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

#include "nmdbussettingsconnectionprovider.h"

#include <NetworkManager.h>

#include <QHash>

#include <KDebug>

// knminternals includes
#include "connection.h"
#include "interfaceconnection.h"

// knmservice includes
#include "activatablelist.h"
#include "connectionlist.h"

#include "connectiondbus.h"
#include "remoteconnection.h"
#include "nm-settingsinterface.h"
#include "nm-exported-connectioninterface.h"

class NMDBusSettingsConnectionProviderPrivate
{
public:
    ConnectionList * connectionList;
    // hash of object path to object
    QHash<QString, QPair<Knm::Connection*, RemoteConnection*> > connections;
    QHash<QUuid, QDBusObjectPath> uuidToPath;
    OrgFreedesktopNetworkManagerSettingsInterface * iface;
    QString serviceName;
};

NMDBusSettingsConnectionProvider::NMDBusSettingsConnectionProvider(ConnectionList * connectionList, const QString & service, QObject * parent)
    : QObject(parent), d_ptr(new NMDBusSettingsConnectionProviderPrivate)
{
    Q_D(NMDBusSettingsConnectionProvider);
    d->connectionList = connectionList;
    d->iface = new OrgFreedesktopNetworkManagerSettingsInterface(service,
            QLatin1String(NM_DBUS_PATH_SETTINGS),
            QDBusConnection::systemBus(), parent);
    d->serviceName = service;

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
    QDBusPendingReply<QList<QDBusObjectPath> > reply = d->iface->ListConnections();
    if (reply.isValid()) {
        QList<QDBusObjectPath> connections = reply.value();
        foreach (const QDBusObjectPath &op, connections) {
            kDebug() << op.path();
            initialiseAndRegisterRemoteConnection(op.path());
        }
    }
}

void NMDBusSettingsConnectionProvider::initialiseAndRegisterRemoteConnection(const QString & path)
{
    Q_D(NMDBusSettingsConnectionProvider);
    if (d->connections.contains(path)) {
        kDebug() << "Service" << d->iface->service() << "is reporting the same connection more than once!";
    } else {
        RemoteConnection * connectionIface = new RemoteConnection(d->iface->service(), path, this);
        makeConnections(connectionIface);
        Knm::Connection * connection = new Knm::Connection(QUuid(), Knm::Connection::Wired);
        ConnectionDbus dbusConverter(connection);
        dbusConverter.fromDbusMap(connectionIface->GetSettings());

        d->connections.insert(path, QPair<Knm::Connection*, RemoteConnection*>(connection,connectionIface));
        d->uuidToPath.insert(connection->uuid(), QDBusObjectPath(path));

        kDebug() << connection->uuid();

        connection->setOrigin(QLatin1String("NMDBusSettingsConnectionProvider"));

        //TODO: make a better check here
        if (d->serviceName.contains("NetworkManagerUserSettings"))
            connection->setScope(Knm::Connection::User);
        else if (d->serviceName.contains("NetworkManagerSystemSettings"))
            connection->setScope(Knm::Connection::System);

        d->connectionList->addConnection(connection);
    }
}

void NMDBusSettingsConnectionProvider::makeConnections(RemoteConnection * connectionIface)
{
    connect(connectionIface, SIGNAL(Removed()), this, SLOT(onRemoteConnectionRemoved()));
    connect(connectionIface, SIGNAL(Updated(const QVariantMapMap&)),
            this, SLOT(onRemoteConnectionUpdated(const QVariantMapMap&)));
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
    if (d->connections.contains(connection->path())) {
        QPair<Knm::Connection *, RemoteConnection *> removed = d->connections.take(removedPath);
        d->uuidToPath.remove(removed.first->uuid());
        delete removed.second;
        d->connectionList->removeConnection(removed.first);

        emit connectionsChanged();
    }
}

void NMDBusSettingsConnectionProvider::onRemoteConnectionUpdated(const QVariantMapMap& updatedSettings)
{
    Q_D(NMDBusSettingsConnectionProvider);
    RemoteConnection * connection = static_cast<RemoteConnection*>(sender());
    kDebug() << connection->path();
    if (d->connections.contains(connection->path())) {
        QPair<Knm::Connection *, RemoteConnection *> updated = d->connections.value(connection->path());
        ConnectionDbus dbusConverter(updated.first);
        dbusConverter.fromDbusMap(updatedSettings);
        d->connectionList->updateConnection(updated.first);

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
    QHashIterator <QString, QPair<Knm::Connection*, RemoteConnection*> > it (d->connections);
    while (it.hasNext()) {
        it.next();
        QPair<Knm::Connection*, RemoteConnection*> toDelete = it.value();
        d->connectionList->removeConnection(toDelete.first);
        delete toDelete.second;
    }
    d->connections.clear();
    d->uuidToPath.clear();
}

void NMDBusSettingsConnectionProvider::handleAdd(Knm::Activatable * added)
{
    Q_D(NMDBusSettingsConnectionProvider);
    Knm::InterfaceConnection * interfaceConnection = qobject_cast<Knm::InterfaceConnection*>(added);
    if (interfaceConnection) {
        // if derived from one of our connections, tag it with the service and object path of the
        // connection
        if (d->uuidToPath.contains(interfaceConnection->connectionUuid())) {
            kDebug() << "tagging InterfaceConnection " << interfaceConnection->connectionName() << "from" << d->serviceName << d->uuidToPath[interfaceConnection->connectionUuid()].path();
            interfaceConnection->setProperty("NMDBusService", d->serviceName );
            interfaceConnection->setProperty("NMDBusObjectPath", d->uuidToPath[interfaceConnection->connectionUuid()].path());
        }
    }
}

void NMDBusSettingsConnectionProvider::handleUpdate(Knm::Activatable *)
{

}
void NMDBusSettingsConnectionProvider::handleRemove(Knm::Activatable *)
{

}

// vim: sw=4 sts=4 et tw=100
