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

#include "networkmanagementadaptor.h"

#include "externals/wirelessnetwork.h"
#include "externals/wirelessnetworkinterfaceenvironment.h"
#include "externals/connectable.h"
#include "internals/connection.h"
#include "externals/wirelessconnection.h "

#include <solid/control/networkinterface.h>
#include <solid/control/networkmanager.h>

K_PLUGIN_FACTORY(NetworkManagementServiceFactory,
                 registerPlugin<NetworkManagementService>();
    )
K_EXPORT_PLUGIN(NetworkManagementServiceFactory("networkmanagement"))

const static QString BASE_DBUS_PATH = "/modules/networkmanagement/Connectables/Connectable";

NetworkManagementService::NetworkManagementService(QObject * parent, const QVariantList&)
        : KDEDModule(parent)
        , m_counter(0)
{
    (void) new NetworkmanagementAdaptor( this );
    QDBusConnection::sessionBus().registerService( "org.kde.networkmanagement" ) ;
    QDBusConnection::sessionBus().registerObject( "/modules/networkmanagement", this );

    // Load configuration first
    reparseConfiguration();

    // Let's start tracking the devices

    foreach (Solid::Control::NetworkInterface *iface, Solid::Control::NetworkManager::networkInterfaces()) {
        networkInterfaceAdded(iface);
    }
}

NetworkManagementService::~NetworkManagementService()
{

}

void NetworkManagementService::reparseConfiguration()
{
    foreach (Knm::Internals::Connection *conn, m_connections) {
        conn->deleteLater();
    }

    m_connections.clear();

    QStringList connectionIds;
    connectionIds = KNetworkManagerServicePrefs::self()->connections();
    // 2) restore each connection
    foreach (QString connectionId, connectionIds) {
        Knm::Internals::Connection * connection = restoreConnection(connectionId);
        if (connection) {
            m_connections.append(connection);
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

    connect(iface, SIGNAL(connectionStateChanged(int)), this, SLOT(connectionStateChanged(int)));

    if (iface->type() == Solid::Control::NetworkInterface::Ieee80211) {
        m_environments[iface->uni()] = new Solid::Control::WirelessNetworkInterfaceEnvironment(
                    qobject_cast<Solid::Control::WirelessNetworkInterface>(iface));

        connect(qobject_cast<Solid::Control::WirelessNetworkInterface>(iface),
                SIGNAL(activeAccessPointChanged(const QString&)),
                this, SLOT(activeAccessPointChanged(const QString&)));
        connect(m_environments[iface->uni()], SIGNAL(wirelessNetworkAppeared(const QString&)),
                this, SLOT(wirelessNetworkAppeared(const QString&)));
        connect(m_environments[iface->uni()], SIGNAL(wirelessNetworkDisappeared(const QString&)),
                this, SLOT(wirelessNetworkDisappeared(const QString&)));
        connect(m_environments[iface->uni()], SIGNAL(wirelessNetworkChanged(const QString&)),
                this, SLOT(wirelessNetworkChanged(const QString&)));

        // Let's just get all the Wireless Networks from the interface environment
        QList<Solid::Control::WirelessNetwork*> networks = m_environments[iface->uni()]->wirelessNetworks();

        // Now, we basically need to check if any of those networks are actually configured
        // and we need to associate them to a connection

        QList<Knm::Externals::WirelessConnection*> wcons;

        foreach (Solid::Control::WirelessNetwork *network, networks) {
            Knm::Externals::WirelessConnection *wc = processNewWirelessNetwork(network);

            if (wc != 0) {
                // Bingo. Let's remove the network from the list and create
                // the connection

                networks.removeOne(network);

                // Add the network to our hash
                ++m_counter;
                QString path = QString("%1%2").arg(BASE_DBUS_PATH).arg(m_counter);
                QDBusConnection::sessionBus().registerObject(path, wc);
                m_connectables[wc] = path;
                emit ConnectableAdded(path);
            }
        }

        // At this point, we should be having our connections and networks correctly
        // stored. It's time to stream some signals, since up to now we streamed signals
        // just for the connection items as we were creating them. Now it's time for network
        // items

        foreach (Solid::Control::WirelessNetwork *network, networks) {
            // Add the network to our hash
            ++m_counter;
            QString path = QString("%1%2").arg(BASE_DBUS_PATH).arg(m_counter);
            QDBusConnection::sessionBus().registerObject(path, network);
            m_connectables[network] = path;
            emit ConnectableAdded(path);
        }
    }
}

void NetworkManagementService::networkInterfaceRemoved(const QString &uni)
{
    // Roll out and delete all the existing Connectables belonging to it

    if (m_environment.contains(uni)) {
        Solid::Control::WirelessNetworkEnvironment *env = m_environments[uni];

        m_environments.removeOne(uni);
        env->deleteLater();
    }

    foreach (Connectable *conn, m_connectables.keys()) {
        if (conn->deviceUni() == uni) {
            // Remove it
            emit ConnectableRemoved(m_connectables[conn]);
            m_connectables.removeOne(conn);
            conn->deleteLater();
        }
    }
}

void NetworkManagementService::wirelessNetworkAppeared(const QString &uni)
{
    Solid::Control::WirelessNetworkEnvironment *env = qobject_cast<Solid::Control::WirelessNetworkEnvironment>(sender());

    if (!env) {
        return;
    }

    Solid::Control::WirelessNetwork *network = env->findWirelessNetwork(uni);

    if (!network) {
        return;
    }

    Connectable *conn = network;

    Knm::Externals::WirelessConnection *wc = processNewWirelessNetwork(network);

    if (wc != 0) {
        conn = wc;
    }

    // Add the network to our hash
    ++m_counter;
    QString path = QString("%1%2").arg(BASE_DBUS_PATH).arg(m_counter);
    QDBusConnection::sessionBus().registerObject(path, conn);
    m_connectables[conn] = path;
    emit ConnectableAdded(path);
}

void NetworkManagementService::wirelessNetworkDisappeared(const QString &uni)
{
    foreach (Connectable *conn, m_connectables.keys()) {
        if (conn->connectionType() = "Wireless") {
            Knm::Externals::WirelessConnection *wc = qobject_cast<Knm::Externals::WirelessConnection>(conn);

            if (wc->network() == uni) {
                emit ConnectableRemoved(m_connectables[conn]);
                conn->deleteLater();
            }
        } else if (conn->connectionType() = "WirelessNetworkItem") {
            WirelessNetworkItem *wni = qobject_cast<WirelessNetworkItem>(conn);

            if (wni->essid() == uni) {
                emit ConnectableRemoved(m_connectables[conn]);
                conn->deleteLater();
            }
        }
    }
}

Knm::Externals::WirelessConnection *NetworkManagementService::processNewWirelessNetwork(Solid::Control::WirelessNetwork *network)
{
    foreach (Knm::Internals::Connection *connection, m_connections) {
        Knm::Internals::Setting *set = connection->setting(Knm::Internals::Setting::Wireless);
        if (set != 0) {
            // Ok, let's retrieve the SSID and set up the connection
            Knm::Internals::WirelessSetting *wset = qobject_cast<Knm::Internals::WirelessSetting*>(set);

            if (network->ssid() == wset->ssid()) {
                // Bingo. Let's create the connection

                Knm::Externals::WirelessConnection *conn = new Knm::Externals::WirelessConnection();
                conn->setNetwork(network->uuid());

                return conn;
            }
        }
    }
    return 0;
}

Knm::Internals::Connection * NetworkManagementService::restoreConnection(const QString & connectionId)
{
    kDebug() << connectionId;
    m_config = connectionFileForUuid(connectionId);
    Knm::Internals::Connection * connection = 0;
    if (!m_config.isNull()) {
        // restore from disk
        Knm::Internals::ConnectionPersistence cp(m_config,
                (KNetworkManagerServicePrefs::self()->storeInWallet() ?
                 Knm::Internals::ConnectionPersistence::Secure :
                 Knm::Internals::ConnectionPersistence::PlainText));
        cp.load();
        connection = cp.connection();
    } else {
        kError() << "Config file for connection" << connectionId << "not found!";
    }

    return connection;
}

#include "service.moc"
