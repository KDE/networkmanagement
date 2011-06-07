/*
Copyright 2008,2009 Will Stephenson <wstephenson@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <unistd.h>

#include "nmdbussettingsconnectionprovider.h"

#include <NetworkManager.h>

#include <QHash>
#include <QUuid>

#include <KDebug>
#include <KAuth/Action>
#include <kauthactionreply.h>
#include <KMessageBox>
#include <KLocale>

// knminternals includes
#include "connection.h"
#include "interfaceconnection.h"
#include "vpninterfaceconnection.h"

// knmservice includes
#include "activatablelist.h"
#include "connectionlist.h"

#include "settings/ipv4.h"

#include "connectiondbus.h"
#include "remoteconnection.h"
#include "nm-settingsinterface.h"
#include "nm-settings-connectioninterface.h"
#include "nm-active-connectioninterface.h"
#include "nm-manager-interface.h"
#include "nm-device-interface.h"

#include <solid/control/networkmanager.h>
#include <solid/control/networkmodeminterface.h>
#include <solid/control/modeminterface.h>

class NMDBusSettingsConnectionProviderPrivate
{
public:
    ConnectionList * connectionList;
    // hash of object path to object
    QHash<QString, RemoteConnection*> connections;
    QHash<QUuid, QString> uuidToPath;
    QMap<QUuid, QVariantMapMap> secretsToSave;
    QHash<QUuid, Knm::Connection *> secretsToGet;
    OrgFreedesktopNetworkManagerSettingsInterface * iface;
    QString serviceName;
};

NMDBusSettingsConnectionProvider::NMDBusSettingsConnectionProvider(ConnectionList * connectionList, QObject * parent)
    : QObject(parent), d_ptr(new NMDBusSettingsConnectionProviderPrivate)
{
    Q_D(NMDBusSettingsConnectionProvider);
    if (connectionList) {
        d->connectionList = connectionList;
    } else {
        d->connectionList = new ConnectionList(this);
    }
    d->iface = new OrgFreedesktopNetworkManagerSettingsInterface(NM_DBUS_SERVICE,
            NM_DBUS_PATH_SETTINGS,
            QDBusConnection::systemBus(), parent);
    d->serviceName = NM_DBUS_SERVICE;

    // For VPN connections.
    qDBusRegisterMetaType<QStringMap>();

    initConnections();
    // signal is from parent class
    connect(d->iface, SIGNAL(NewConnection(const QDBusObjectPath&)),
            this, SLOT(onConnectionAdded(const QDBusObjectPath&)));
    // clean our connections out if the service goes away
    connect(QDBusConnection::systemBus().interface(),
            SIGNAL(serviceOwnerChanged(const QString&,const QString&,const QString&)),
            SLOT(serviceOwnerChanged(const QString&,const QString&,const QString&)));
}

NMDBusSettingsConnectionProvider::~NMDBusSettingsConnectionProvider()
{
}

void NMDBusSettingsConnectionProvider::initConnections()
{
    kDebug();
    Q_D(NMDBusSettingsConnectionProvider);
    // ListConnections() is asynchronous; we need to wait until completion, else it won't work
    QDBusPendingReply<QList<QDBusObjectPath> > reply = d->iface->ListConnections();
    reply.waitForFinished();
    if (!reply.isError()) {
        QList<QDBusObjectPath> connections = reply.value();
        foreach (const QDBusObjectPath &op, connections) {
            kDebug() << op.path();
            initialiseAndRegisterRemoteConnection(op.path());
        }
    } else {
        kDebug() << "Error in ListConnections() D-Bus call:" << reply.error();
    }


}

void NMDBusSettingsConnectionProvider::initialiseAndRegisterRemoteConnection(const QString & path)
{
    Q_D(NMDBusSettingsConnectionProvider);
    if (!d->uuidToPath.key(path, QUuid()).isNull()) {
        kDebug() << "Service" << d->iface->service() << "is reporting the same connection more than once!";
    } else {
        RemoteConnection * connectionIface = new RemoteConnection(d->iface->service(), path, this);
        makeConnections(connectionIface);
        Knm::Connection * connection = new Knm::Connection(QUuid(), Knm::Connection::Wired);
        ConnectionDbus dbusConverter(connection);
        dbusConverter.fromDbusMap(connectionIface->GetSettings());

        d->connections.insert(connection->uuid(),connectionIface);
        d->uuidToPath.insert(connection->uuid(), path);

        kDebug() << connection->uuid();

        connection->setOrigin(QLatin1String("NMDBusSettingsConnectionProvider"));

        d->connectionList->addConnection(connection);
    }
}

void NMDBusSettingsConnectionProvider::makeConnections(RemoteConnection * connectionIface)
{
    connect(connectionIface, SIGNAL(Removed()), this, SLOT(onRemoteConnectionRemoved()));
    connect(connectionIface, SIGNAL(Updated()),
            this, SLOT(onRemoteConnectionUpdated()));
}

void NMDBusSettingsConnectionProvider::onConnectionAdded(const QDBusObjectPath& op)
{
    initialiseAndRegisterRemoteConnection(op.path());
    emit connectionsChanged();
}

void NMDBusSettingsConnectionProvider::onRemoteConnectionRemoved()
{
    Q_D(NMDBusSettingsConnectionProvider);
    RemoteConnection * connection = static_cast<RemoteConnection*>(sender());
    QString removedPath = connection->path();
    kDebug() << removedPath;
    QUuid uuid = d->uuidToPath.key(removedPath, QUuid());
    if (!uuid.isNull()) {
        RemoteConnection *removed = d->connections.take(uuid.toString());
        d->uuidToPath.remove(uuid.toString());
        delete removed;
        Knm::Connection *con = d->connectionList->findConnection(uuid.toString());
        con->removeCertificates();
        d->connectionList->removeConnection(con);

        emit connectionsChanged();
    }
}

void NMDBusSettingsConnectionProvider::onRemoteConnectionUpdated()
{
    Q_D(NMDBusSettingsConnectionProvider);
    RemoteConnection * connection = static_cast<RemoteConnection*>(sender());
    kDebug() << connection->path();
    QUuid uuid = d->uuidToPath.key(connection->path(), QUuid());
    if (!uuid.isNull() && d->connections.contains(uuid.toString())) {
        Knm::Connection *con = d->connectionList->findConnection(uuid.toString());
        ConnectionDbus dbusConverter(con);
        dbusConverter.fromDbusMap(connection->GetSettings());
        d->connectionList->updateConnection(con);

        emit connectionsChanged();
    }
}

void NMDBusSettingsConnectionProvider::serviceOwnerChanged(const QString & changedService, const QString & oldOwner, const QString & newOwner)
{
    Q_D(NMDBusSettingsConnectionProvider);
    //kDebug() << changedService << service() << oldOwner << newOwner;
    if (changedService == d->iface->service()) {
        if (!oldOwner.isEmpty() && newOwner.isEmpty()) {
            clearConnections();
            //emit disappeared(this);
        } else if (oldOwner.isEmpty() && !newOwner.isEmpty()) {
            initConnections();
            //emit appeared(this);
        } else if (!oldOwner.isEmpty() && !newOwner.isEmpty()) {
            clearConnections();
            //emit disappeared(this);
            initConnections();
            //emit appeared(this);
        }
    }
}

void NMDBusSettingsConnectionProvider::clearConnections()
{
    Q_D(NMDBusSettingsConnectionProvider);
    foreach (const QString &key, d->connections.keys()) {
        // Remove it from d->connections first to prevent a crash because
        // of the "delete toDelete.second" emmiting a Delete signal captured by
        // NMDBusSettingsConnectionProvider::onRemoteConnectionRemoved(), which deletes
        // toDelete.second again.
        RemoteConnection *toDelete = d->connections.take(key);
        d->connectionList->removeConnection(key);
        delete toDelete;
    }
    // Just to make sure d->connections is really clear.
    d->connections.clear();
    d->uuidToPath.clear();
    emit connectionsChanged();
}

void NMDBusSettingsConnectionProvider::handleAdd(Knm::Activatable * added)
{
    Q_D(NMDBusSettingsConnectionProvider);
    Knm::InterfaceConnection * interfaceConnection = qobject_cast<Knm::InterfaceConnection*>(added);
    if (interfaceConnection) {
        connect(interfaceConnection, SIGNAL(activated()), this, SLOT(interfaceConnectionActivated()));
        connect(interfaceConnection, SIGNAL(deactivated()), this, SLOT(interfaceConnectionDeactivated()));
        // if derived from one of our connections, tag it with the service and object path of the
        // connection
        if (d->uuidToPath.contains(interfaceConnection->connectionUuid())) {
            kDebug() << "tagging InterfaceConnection " << interfaceConnection->connectionName() << "from" << d->serviceName << d->uuidToPath[interfaceConnection->connectionUuid()];
            interfaceConnection->setProperty("NMDBusService", d->serviceName );
            interfaceConnection->setProperty("NMDBusObjectPath", d->uuidToPath[interfaceConnection->connectionUuid()]);
        }
    }
}

void NMDBusSettingsConnectionProvider::interfaceConnectionActivated()
{
    Knm::InterfaceConnection * ic = qobject_cast<Knm::InterfaceConnection*>(sender());
    if (ic) {
        QString deviceToActivateOn;
        QVariantMap extraArguments;

        Knm::VpnInterfaceConnection * vpn = qobject_cast<Knm::VpnInterfaceConnection*>(ic);
        if (vpn) {
            // look up the active connection (a real connection, not this vpn that is being activated)
            // because NM needs its details to bring up the VPN
            QString activeConnPath;
            foreach (const QString &activeConnectionPath, Solid::Control::NetworkManagerNm09::activeConnections()) {
                OrgFreedesktopNetworkManagerConnectionActiveInterface activeConnection(NM_DBUS_SERVICE, activeConnectionPath, QDBusConnection::systemBus());

                if ( activeConnection.getDefault() && activeConnection.state() == NM_ACTIVE_CONNECTION_STATE_ACTIVATED) {
                    activeConnPath = activeConnection.path();
                    QList<QDBusObjectPath> devs = activeConnection.devices();
                    if (!devs.isEmpty()) {
                        deviceToActivateOn = devs.first().path();
                    }
                }
            }

            kDebug() << "active" << activeConnPath << "device" << deviceToActivateOn;

            if ( activeConnPath.isEmpty() || deviceToActivateOn.isEmpty() )
                return;

            extraArguments.insert( "extra_connection_parameter", activeConnPath );
        } else {
            deviceToActivateOn = ic->deviceUni();
        }

        // Enable modem before connecting.
        Solid::Control::ModemNetworkInterfaceNm09 *iface = qobject_cast<Solid::Control::ModemNetworkInterfaceNm09 *>(Solid::Control::NetworkManagerNm09::findNetworkInterface(deviceToActivateOn));
        if (iface) {
            Solid::Control::ModemGsmCardInterface *modem = iface->getModemCardIface();
            if (modem && !modem->enabled()) {
                // Try to pin-unlock the modem.
                QMetaObject::invokeMethod(modem, "unlockRequiredChanged", Qt::DirectConnection,
                                          Q_ARG(QString, modem->unlockRequired()));
                kDebug() << "Trying to enable modem";
                modem->enable(true);
            }
        }

        // Now activate the connection
        OrgFreedesktopNetworkManagerInterface nmIface(QLatin1String(NM_DBUS_SERVICE), QLatin1String(NM_DBUS_PATH), QDBusConnection::systemBus());
        nmIface.ActivateConnection(QDBusObjectPath(ic->property("NMDBusObjectPath").toString()), QDBusObjectPath(deviceToActivateOn), QDBusObjectPath("/"));
    }
}

void NMDBusSettingsConnectionProvider::interfaceConnectionDeactivated()
{
    Knm::InterfaceConnection * ic = qobject_cast<Knm::InterfaceConnection*>(sender());
    Solid::Control::NetworkInterfaceNm09 *iface = Solid::Control::NetworkManagerNm09::findNetworkInterface(ic->deviceUni());
    if (iface) {
        iface->disconnectInterface();
    } else { // VPN connections do have NetworkInterface objects.
        Solid::Control::NetworkManagerNm09::deactivateConnection(ic->property("NMDBusActiveConnectionObject").toString());
    }
}

void NMDBusSettingsConnectionProvider::handleUpdate(Knm::Activatable *)
{

}
void NMDBusSettingsConnectionProvider::handleRemove(Knm::Activatable *)
{

}

void NMDBusSettingsConnectionProvider::updateConnection(const QString &uuid, Knm::Connection *newConnection)
{
    Q_D(NMDBusSettingsConnectionProvider);

    if ( d->uuidToPath.contains(QUuid(uuid))) {

        QString objPath = d->uuidToPath.value(QUuid(uuid));

        if (!d->connections.contains(uuid))
        {
            kWarning() << "Connection could not found!" << uuid << objPath;
            return;
        }

        RemoteConnection *remote = d->connections.value(uuid);
        kDebug() << "Updating connection "<< remote->id() << objPath;

        newConnection->saveCertificates();
        newConnection->setSecrets();
        ConnectionDbus converter(newConnection);
        QVariantMapMap map = converter.toDbusMap();

        remote->Update(map);

        // FIXME: if connection's name (id in NM termonology) changed in the Update call above,
        // NM will leave the old connection file intact and create/update a new connection file
        // in /etc/NetworkManager/system-connections/ with the same uuid, which is wrong in my oppinion.
        // Furthermore the old connection is not shown in kcm's because we use the uuid as connection identifier.

        // don't do any processing on d->connections and d->connectionList here
        // because onRemoteConnectionUpdated() method will take care of them
        //

        return;
    }

    kWarning() << "Connection could not found!"<< uuid;

}

void NMDBusSettingsConnectionProvider::addConnection(Knm::Connection *newConnection)
{
    Q_D(NMDBusSettingsConnectionProvider);
    newConnection->saveCertificates();
    newConnection->setSecrets();
    ConnectionDbus converter(newConnection);
    QVariantMapMap map = converter.toDbusMap();
    kDebug() << "Adding connection " << newConnection->name() << newConnection->uuid().toString();
    // WARNING: this debug message print secrets, do not commit it uncommented.
    //kDebug() << "Here is the map: " << map;

    if(newConnection && newConnection->name().isEmpty())
        kWarning() << "Trying to add connection without a name!";



    d->secretsToSave.insert(newConnection->uuid(), map);
    QDBusPendingCall reply = d->iface->AddConnection(map);
    //do not check if reply is valid or not because it's an async call and invalid till reply is really arrived

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, 0);

    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(onConnectionAddArrived(QDBusPendingCallWatcher*)));
}

void NMDBusSettingsConnectionProvider::onConnectionAddArrived(QDBusPendingCallWatcher *watcher)
{
    QDBusPendingReply<QDBusObjectPath> reply = *watcher;

    if (!reply.isValid())
    {
        kWarning() << "Adding connection failed:" << reply.error().message();
        emit addConnectionCompleted(false, reply.error().message());
    }
    else
    {
        Q_D(NMDBusSettingsConnectionProvider);
        QDBusObjectPath objPath = reply.argumentAt<0>();
        emit addConnectionCompleted(true, QString());

        // Hack to force NetworkManager to call the secrets agent to save this connections's secrets.
        // This does not work for VPN connections.
        // TODO: change this to a dbus call to the kded module.
        QString uuid = d->uuidToPath.key(objPath.path(), QUuid()).toString();
        RemoteConnection *remote = d->connections.value(uuid);
        QVariantMapMap map = d->secretsToSave.take(uuid);
        remote->Update(map);

        kDebug() << "Connection added successfully: " << objPath.path() << uuid;
    }

    watcher->deleteLater();
}


bool NMDBusSettingsConnectionProvider::getConnectionSecrets(Knm::Connection *con)
{
    Q_D(NMDBusSettingsConnectionProvider);

    if (!con->hasSecrets())
    {
        kDebug() << "Connection seems not to have any secret information. Ignoring...";
        return false;
    }

    QUuid uuid = con->uuid();
    if ( !d->uuidToPath.contains(uuid)){
        kWarning() << "Secrets requested but connection not found!";
        return false;
    }

    QString objPath = d->uuidToPath.value(uuid);

    OrgFreedesktopNetworkManagerSettingsConnectionInterface secretIface(d->serviceName, objPath, QDBusConnection::systemBus(), this);

    kDebug() << "Getting connection secrets for " << uuid.toString();

    QStringList secretSettings = con->secretSettings();

    kDebug() << "Settings containing secret values are " << secretSettings;

    if (secretSettings.count() != 1)
        kWarning() << "This connection has more than 1 secret setting, not supported yet :/";


    QDBusPendingReply<QVariantMapMap> reply = secretIface.GetSecrets(secretSettings.at(0));
    //do not check if reply is valid or not because it's an async call and invalid till reply is really arrived

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);

    //Ugly hack to access Knm::Connection pointer in the onConnectionSecretsArrived slot
    watcher->setProperty("connection", con->uuid().toString());
    d->secretsToGet.insert(con->uuid(), con);

    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(onConnectionSecretsArrived(QDBusPendingCallWatcher*)));

    return true;
}

void NMDBusSettingsConnectionProvider::onConnectionSecretsArrived(QDBusPendingCallWatcher *watcher)
{
    Q_D(NMDBusSettingsConnectionProvider);

    if (!watcher)
        return;

    QDBusPendingReply<QVariantMapMap> reply = *watcher;

    Knm::Connection *con = d->secretsToGet.value(watcher->property("connection").toString());
    if (reply.isValid())
    {
        QVariantMapMap set = reply.argumentAt<0>();
        // WARNING: this print secrets, do not commit it uncommented.
        //kDebug() << "Got secrets, yay! " << set;
        kDebug() << "Got secrets, yay! ";

        Knm::Connection *con = d->secretsToGet.value(watcher->property("connection").toString());
        if (!con)
        {
            kWarning() << "Connection not found!" << watcher->property("connection").toString();
            return;
        }

        ConnectionDbus dbusConverter(con);
        dbusConverter.fromDbusSecretsMap(set); //update secretSettings in connection
        emit getConnectionSecretsCompleted(true, QString(), con->uuid());
    }
    else
    {
        kWarning () << "Secret fetching failed:" << reply.error().message();
        emit getConnectionSecretsCompleted(false, reply.error().message(), con->uuid());
    }

    watcher->deleteLater();
}

void NMDBusSettingsConnectionProvider::removeConnection(const QString &uuid)
{
    Q_D(NMDBusSettingsConnectionProvider);

    if ( d->uuidToPath.contains(QUuid(uuid))) {

        if (!d->connections.contains(uuid))
        {
            QString objPath = d->uuidToPath.value(QUuid(uuid));
            kWarning() << "Connection could not found!" << uuid << objPath;
            return;
        }

        RemoteConnection *remote = d->connections.value(uuid);

        kDebug() << "Removing connection "<< remote->id() << uuid;
        remote->Delete();

        // don't do any processing on d->connections and d->connectionList here
        // because onRemoteConnectionRemoved() method will take care of them
        //

        return;
    }

    kWarning() << "Connection could not found!"<< uuid;
}

// vim: sw=4 sts=4 et tw=100
