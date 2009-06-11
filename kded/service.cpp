/*
Copyright 2009 Dario Freddi <drf54321@gmail.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License or (at your option) version 3 or any later version
accepted by the membership of KDE e.V. (or its successor approved
by the membership of KDE e.V.), which shall act as a proxy
defined in Section 14 of version 3 of the license.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "service.h"

#include <KCModuleInfo>
#include <KCModuleProxy>
#include <KServiceTypeTrader>
#include <KStandardDirs>

#include "networkmanagementadaptor.h"
#include "knmserviceprefs.h"

#include "wirelessnetwork.h"
#include "wirelessnetworkinterfaceenvironment.h"
#include "connectable.h"
#include "connection.h"
#include "connectionpersistence.h"
#include "settings/802-11-wireless.h"
#include "interfaceconnection.h"
#include "wirelessinterfaceconnection.h"
#include "wirelessnetworkitem.h"

#include <solid/control/networkinterface.h>
#include <solid/control/networkmanager.h>

K_PLUGIN_FACTORY(NetworkManagementServiceFactory,
                 registerPlugin<NetworkManagementService>();
    )
K_EXPORT_PLUGIN(NetworkManagementServiceFactory("networkmanagement"))

const QString NetworkManagementService::BASE_DBUS_PATH = QString::fromLatin1("/modules/networkmanagement/Connectables/");

NetworkManagementService::NetworkManagementService(QObject * parent, const QVariantList&)
        : KDEDModule(parent)
        , m_counter(0)
{
    (void) new NetworkmanagementAdaptor( this );
    QDBusConnection::sessionBus().registerService( "org.kde.networkmanagement" ) ;

    // Load configuration first
    KNetworkManagerServicePrefs::instance(Knm::ConnectionPersistence::NETWORKMANAGEMENT_RCFILE);
    QStringList connectionIds;
    connectionIds = KNetworkManagerServicePrefs::self()->connections();
    // 2) restore each connection
    foreach (QString connectionId, connectionIds) {
        Knm::Connection * connection = restoreConnection(connectionId);
        if (connection) {
            m_connections[connectionId] = connection;
        }
    }

    // Let's start tracking the devices
    foreach (Solid::Control::NetworkInterface *iface, Solid::Control::NetworkManager::networkInterfaces()) {
        networkInterfaceAdded(iface);
    }
}

NetworkManagementService::~NetworkManagementService()
{
}

void NetworkManagementService::reparseConfiguration(const QStringList& changedConnections)
{
    KNetworkManagerServicePrefs::self()->readConfig();
    QStringList addedConnections, deletedConnections;
    // figure out which connections were added
    QStringList existingConnections = m_connections.keys();
    QStringList onDiskConnections = KNetworkManagerServicePrefs::self()->connections();
    qSort(existingConnections);
    qSort(onDiskConnections);
    kDebug() << "existing connections are:" << existingConnections;
    kDebug() << "on-disk connections are:" << onDiskConnections;

    foreach (QString connectionId, onDiskConnections) {
        if (!existingConnections.contains(connectionId)) {
            addedConnections.append(connectionId);
        }
    }
    // figure out which connections were deleted
    foreach (QString connectionId, existingConnections) {
        if (!onDiskConnections.contains(connectionId)) {
            deletedConnections.append(connectionId);
        }
    }
    kDebug() << "added connections:" << addedConnections;
    kDebug() << "changed connections:" << changedConnections;
    kDebug() << "deleted connections:" << deletedConnections;

    // update the service
    foreach (QString connectionId, deletedConnections) {
        Knm::Connectable *conn = 0;

        foreach (Knm::Connectable *item, m_connectables.keys()) {
            if (item->connectableType() == Knm::Connectable::WirelessConnection ||
                item->connectableType() == Knm::Connectable::Connection) {
                Knm::InterfaceConnection *c = qobject_cast<Knm::InterfaceConnection*>(conn);
                if (c->connectionUni() == connectionId) {
                    conn = c;
                    break;
                }
            }
        }

        if (!conn) {
            continue;
        }

        if (conn->connectableType() == Knm::Connectable::WirelessConnection) {
            // In this case, we should "downgrade" it to a WirelessNetworkItem
            Knm::WirelessInterfaceConnection *wc = qobject_cast<Knm::WirelessInterfaceConnection*>(conn);
            QString network = wc->network();
            deleteConnectableAndNotify(wc);

            Knm::WirelessNetworkItem *item = new Knm::WirelessNetworkItem();
            item->setEssid(network);
            registerConnectableAndNotify(item);
        } else {
            // Otherwise, let's just erase it
            deleteConnectableAndNotify(conn);
        }

        m_connections.remove(connectionId);
    }

    foreach (const QString connectionId, changedConnections) {
        // First things first, let's remove the old connection
        foreach (Knm::Connectable *item, m_connectables.keys()) {
            if (item->connectableType() == Knm::Connectable::WirelessConnection ||
                    item->connectableType() == Knm::Connectable::Connection) {
                Knm::InterfaceConnection *c = qobject_cast<Knm::InterfaceConnection*>(item);
                if (c->connectionUni() == connectionId) {
                    deleteConnectableAndNotify(c);
                    break;
                }
            }
        }

        // Now let's just readd it
        Knm::Connection * connection = restoreConnection(connectionId);

        if (connection->type() == Knm::Connection::Wireless) {
            // Let's look for the ssid
            Knm::Setting *set = connection->setting(Knm::Setting::Wireless);
            if (!set) {
                // Something weird is going on: let's pass by
                continue;
            }
            Knm::WirelessSetting *settings = dynamic_cast<Knm::WirelessSetting*>(set);

            Knm::WirelessInterfaceConnection *wc = processNewWirelessNetwork(settings->ssid());

            if (wc != 0) {
                // Add the network to our hash, for each interface
                foreach (Solid::Control::NetworkInterface *iface, Solid::Control::NetworkManager::networkInterfaces()) {
                    if (connection->type() == solidDeviceToConnectionType(iface->uni())) {
                        Knm::WirelessInterfaceConnection *wc = processNewWirelessNetwork(settings->ssid());
                        wc->setDeviceUni(iface->uni());

                        registerConnectableAndNotify(wc);
                    }
                }
            } else {
                // Add the network to our hash, for each interface
                foreach (Solid::Control::NetworkInterface *iface, Solid::Control::NetworkManager::networkInterfaces()) {
                    if (connection->type() == solidDeviceToConnectionType(iface->uni())) {
                        Knm::InterfaceConnection *conn = new Knm::InterfaceConnection();
                        conn->setDeviceUni(iface->uni());
                        conn->setConnectionUni(connection->uuid().toString());
                        conn->setConnectionName(connection->name());
                        conn->setConnectionType(connection->type());

                        registerConnectableAndNotify(conn);
                    }
                }
            }
        }
    }

    foreach (QString connectionId, addedConnections) {
        kDebug() << "adding connection with id: " << connectionId;
        Knm::Connection * connection = restoreConnection(connectionId);

        if (connection->type() == Knm::Connection::Wireless) {
            // Let's look for a connectable that is actually sharing the same ssid
            Knm::Setting *set = connection->setting(Knm::Setting::Wireless);
            if (!set) {
                // Something weird is going on: let's pass by
                continue;
            }
            Knm::WirelessSetting *settings = dynamic_cast<Knm::WirelessSetting*>(set);

            foreach (Knm::Connectable *conn, m_connectables.keys()) {
                if (conn->connectableType() == Knm::Connectable::WirelessNetworkItem) {
                    // Let's have a check
                    Knm::WirelessNetworkItem *wni = qobject_cast<Knm::WirelessNetworkItem*>(conn);
                    if (wni->essid() == settings->ssid()) {
                        // We have our match, so let's do it
                        Knm::WirelessInterfaceConnection *wic = processNewWirelessNetwork(settings->ssid());

                        if (wic != 0) {
                            // Bingo. Let's remove the network from the list and create
                            // the connection

                            deleteConnectableAndNotify(wni);
                            delete wic;

                            // Add the network to our hash, for each interface
                            foreach (Solid::Control::NetworkInterface *iface, Solid::Control::NetworkManager::networkInterfaces()) {
                                if (connection->type() == solidDeviceToConnectionType(iface->uni())) {
                                    Knm::WirelessInterfaceConnection *wic = processNewWirelessNetwork(settings->ssid());
                                    wic->setDeviceUni(iface->uni());

                                    registerConnectableAndNotify(wic);
                                }
                            }
                        }
                    }
                } else {
                    // Add the network to our hash, for each interface
                    foreach (Solid::Control::NetworkInterface *iface, Solid::Control::NetworkManager::networkInterfaces()) {
                        if (connection->type() == solidDeviceToConnectionType(iface->uni())) {
                            Knm::InterfaceConnection *conn = new Knm::InterfaceConnection();
                            conn->setDeviceUni(iface->uni());
                            conn->setConnectionUni(connection->uuid().toString());
                            conn->setConnectionName(connection->name());
                            conn->setConnectionType(connection->type());

                            registerConnectableAndNotify(conn);
                        }
                    }
                }
            }
        }
    }
}

QList<QDBusObjectPath> NetworkManagementService::ListConnectables()
{
    return m_connectables.values();
}

void NetworkManagementService::networkInterfaceAdded(const QString &uni)
{
    networkInterfaceAdded(Solid::Control::NetworkManager::findNetworkInterface(uni));
}

void NetworkManagementService::networkInterfaceAdded(Solid::Control::NetworkInterface *iface)
{
    if (!iface) {
        return;
    }

    // do we care if an interface's state changes?  Possibly for detecting connectable state changes
    // uncomment the connect below when we have a need for it
    //connect(iface, SIGNAL(connectionStateChanged(int)), this, SLOT(connectionStateChanged(int)));

    if (iface->type() == Solid::Control::NetworkInterface::Ieee80211) {
        m_environments[iface->uni()] = new Solid::Control::WirelessNetworkInterfaceEnvironment(
                    qobject_cast<Solid::Control::WirelessNetworkInterface*>(iface));

        connect(m_environments[iface->uni()], SIGNAL(networkAppeared(const QString&)),
                this, SLOT(wirelessNetworkAppeared(const QString&)));
        connect(m_environments[iface->uni()], SIGNAL(networkDisappeared(const QString&)),
                this, SLOT(wirelessNetworkDisappeared(const QString&)));
        //connect(m_environments[iface->uni()], SIGNAL(wirelessNetworkChanged(const QString&)),
        //        this, SLOT(wirelessNetworkChanged(const QString&)));

        // Let's just get all the Wireless Networks from the interface environment
        QStringList networks = m_environments[iface->uni()]->networks();

        // Now, we basically need to check if any of those networks are actually configured
        // and we need to associate them to a connection

        QList<Knm::WirelessInterfaceConnection*> wcons;

        foreach (const QString &network, networks) {
            Knm::WirelessInterfaceConnection *wc = processNewWirelessNetwork(network);

            if (wc != 0) {
                // Bingo. Let's remove the network from the list and create
                // the connection

                networks.removeOne(network);

                // Add the network to our hash
                wc->setDeviceUni(iface->uni());

                registerConnectableAndNotify(wc);
            }
        }

        // At this point, we should be having our connections and networks correctly
        // stored. It's time to stream some signals, since up to now we streamed signals
        // just for the connection items as we were creating them. Now it's time for network
        // items

        foreach (const QString &network, networks) {
            // Add the network to our hash
            Knm::WirelessNetworkItem *item = new Knm::WirelessNetworkItem();
            item->setEssid(network);
            item->setDeviceUni(iface->uni());

            registerConnectableAndNotify(item);
        }
    } else {
        // If the interface is actually not wireless, we just have to expose the
        // existing connections

        foreach (Knm::Connection *connection, m_connections.values()) {
            if (connection->type() == solidDeviceToConnectionType(iface->uni())) {
                // It actually looks like our interface is capable of using this connection:
                // let's actually create and expose it
                Knm::InterfaceConnection *conn = new Knm::InterfaceConnection();
                conn->setDeviceUni(iface->uni());
                conn->setConnectionUni(connection->uuid().toString());
                conn->setConnectionName(connection->name());
                conn->setConnectionType(connection->type());

                registerConnectableAndNotify(conn);
            }
        }
    }
}

void NetworkManagementService::networkInterfaceRemoved(const QString &uni)
{
    // Roll out and delete all the existing Connectables belonging to it

    if (m_environments.contains(uni)) {
        Solid::Control::WirelessNetworkInterfaceEnvironment *env = m_environments[uni];
        Q_UNUSED(env);

        m_environments.remove(uni);
    }

    foreach (Knm::Connectable *conn, m_connectables.keys()) {
        if (conn->deviceUni() == uni) {
            // Remove it
            deleteConnectableAndNotify(conn);
        }
    }
}

void NetworkManagementService::wirelessNetworkAppeared(const QString &uni)
{
    Solid::Control::WirelessNetworkInterfaceEnvironment *env = qobject_cast<Solid::Control::WirelessNetworkInterfaceEnvironment*>(sender());

    if (!env) {
        return;
    }

    Solid::Control::WirelessNetwork *network = env->findNetwork(uni);

    if (!network) {
        return;
    }

    Knm::Connectable *conn;

    Knm::WirelessInterfaceConnection *wc = processNewWirelessNetwork(network->ssid());

    if (wc != 0) {
        conn = wc;
    } else {
        Knm::WirelessNetworkItem *item = new Knm::WirelessNetworkItem();
        item->setEssid(network->ssid());
        conn = item;
    }

    conn->setDeviceUni(env->interface()->uni());

    // Add the network to our hash
    registerConnectableAndNotify(conn);
}

void NetworkManagementService::wirelessNetworkDisappeared(const QString &uni)
{
    foreach (Knm::Connectable *conn, m_connectables.keys()) {
        if (conn->connectableType() == Knm::Connectable::WirelessConnection) {
            Knm::WirelessInterfaceConnection *wc = qobject_cast<Knm::WirelessInterfaceConnection*>(conn);

            if (wc->network() == uni) {
                deleteConnectableAndNotify(conn);
            }
        } else if (conn->connectableType() == Knm::Connectable::WirelessNetworkItem) {
            Knm::WirelessNetworkItem *wni = qobject_cast<Knm::WirelessNetworkItem*>(conn);

            if (wni->essid() == uni) {
                deleteConnectableAndNotify(conn);
            }
        }
    }
}

Knm::WirelessInterfaceConnection *NetworkManagementService::processNewWirelessNetwork(const QString &ssid)
{
    foreach (Knm::Connection *connection, m_connections.values()) {
        Knm::Setting *set = connection->setting(Knm::Setting::Wireless);
        if (set != 0) {
            // Ok, let's retrieve the SSID and set up the connection
            Knm::WirelessSetting *wset = dynamic_cast<Knm::WirelessSetting*>(set);

            if (ssid == wset->ssid()) {
                // Bingo. Let's create the connection

                Knm::WirelessInterfaceConnection *conn = new Knm::WirelessInterfaceConnection();
                conn->setNetwork(ssid);
                conn->setConnectionUni(connection->uuid().toString());
                conn->setConnectionName(connection->name());
                conn->setConnectionType(connection->type());

                return conn;
            }
        }
    }
    return 0;
}

Knm::Connection * NetworkManagementService::restoreConnection(const QString & connectionId)
{
    kDebug() << connectionId;
    m_config = connectionFileForUuid(connectionId);
    Knm::Connection * connection = 0;
    if (!m_config.isNull()) {
        // restore from disk
        Knm::ConnectionPersistence cp(m_config,
                (KNetworkManagerServicePrefs::self()->storeInWallet() ?
                 Knm::ConnectionPersistence::Secure :
                 Knm::ConnectionPersistence::PlainText));
        cp.load();
        connection = cp.connection();
    } else {
        kError() << "Config file for connection" << connectionId << "not found!";
    }

    return connection;
}

KSharedConfig::Ptr NetworkManagementService::connectionFileForUuid(const QString & uuid)
{
    KSharedConfig::Ptr config;
    if (!uuid.isEmpty()) {
        QString configFile = KStandardDirs::locate("data",
                Knm::ConnectionPersistence::CONNECTION_PERSISTENCE_PATH + uuid);
        if (!configFile.isEmpty()) {
            config = KSharedConfig::openConfig(configFile, KConfig::NoGlobals);
            kDebug() << config->name() << " is at " << configFile;
        }
    }
    return config;
}

Knm::Connection::Type NetworkManagementService::solidDeviceToConnectionType(const QString &uni)
{
    Solid::Control::NetworkInterface *iface = Solid::Control::NetworkManager::findNetworkInterface(uni);

    if (!iface) {
        return Knm::Connection::Wired;
    }

    switch (iface->type()) {
        case Solid::Control::NetworkInterface::Ieee8023:
            return Knm::Connection::Wired;
        case Solid::Control::NetworkInterface::Ieee80211:
            return Knm::Connection::Wireless;
        case Solid::Control::NetworkInterface::Gsm:
            return Knm::Connection::Gsm;
        case Solid::Control::NetworkInterface::Cdma:
            return Knm::Connection::Cdma;
        case Solid::Control::NetworkInterface::Serial:
            return Knm::Connection::Pppoe;
        default:
            return Knm::Connection::Wired;
    }
}

void NetworkManagementService::registerConnectableAndNotify(Knm::Connectable *item)
{
    ++m_counter;
    QString path = QString("%1%2").arg(BASE_DBUS_PATH).arg(m_counter);
    kDebug() << path <<  QDBusConnection::sessionBus().registerObject(path, item);
    QDBusObjectPath dbuspath = QDBusObjectPath(path);
    m_connectables[item] = dbuspath;
    emit ConnectableAdded(dbuspath);
}

void NetworkManagementService::deleteConnectableAndNotify(Knm::Connectable *item)
{
    emit ConnectableRemoved(m_connectables[item]);
    m_connectables.remove(item);
    delete item;
}

#include "service.moc"
