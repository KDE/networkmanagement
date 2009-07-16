#ifndef NMDBUSSETTINGSSERVICE_CPP
#define NMDBUSSETTINGSSERVICE_CPP
/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>

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

#include "nmdbussettingsservice.h"

#include <NetworkManager.h>

#include <QHash>
#include <QUuid>

#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusConnectionInterface>

#include <KDebug>
#include <KLocale>

#include <solid/control/networkmanager.h>

#include <connection.h>
#include <interfaceconnection.h>
#include <vpninterfaceconnection.h>

#include "busconnection.h"
#include "exportedconnection.h"

#include "nm-active-connectioninterface.h"

class NMDBusSettingsServicePrivate
{
public:
    bool active;
    uint nextConnectionId;
    QHash<QDBusObjectPath, BusConnection *> pathToConnections;
    QHash<QUuid, BusConnection *> uuidToConnections;
    QHash<QUuid, QDBusObjectPath> uuidToPath;
};

const QString NMDBusSettingsService::SERVICE_USER_SETTINGS = QLatin1String(NM_DBUS_SERVICE_USER_SETTINGS);
const QString NMDBusSettingsService::SERVICE_SYSTEM_SETTINGS = QLatin1String(NM_DBUS_SERVICE_SYSTEM_SETTINGS);

NMDBusSettingsService::NMDBusSettingsService(QObject * parent)
: QObject(parent), d_ptr(new NMDBusSettingsServicePrivate)
{
    Q_D(NMDBusSettingsService);
    d->active = false;
    d->nextConnectionId = 0;

    if ( !QDBusConnection::systemBus().interface()->registerService( SERVICE_USER_SETTINGS, QDBusConnectionInterface::QueueService, QDBusConnectionInterface::AllowReplacement ) ) {
        // trouble;
        kDebug() << "Unable to register service" << QDBusConnection::systemBus().lastError();
        d->active = false;
    }
    kDebug() << "registered" << SERVICE_USER_SETTINGS;

    connect(QDBusConnection::systemBus().interface(), SIGNAL(serviceRegistered(const QString&)),
            SLOT(serviceRegistered(const QString&)));
    connect(QDBusConnection::systemBus().interface(), SIGNAL(serviceUnregistered(const QString&)),
            SLOT(serviceUnregistered(const QString&)));
    connect( QDBusConnection::systemBus().interface(),
            SIGNAL(serviceOwnerChanged(const QString&, const QString&, const QString & ) ),
            SLOT(serviceOwnerChanged(const QString&, const QString&, const QString & ) ) );

    //declare types
    qDBusRegisterMetaType<QList<QDBusObjectPath> >();

    QDBusConnection dbus = QDBusConnection::systemBus();
    if (dbus.registerObject(QLatin1String(NM_DBUS_PATH_SETTINGS), this, QDBusConnection::ExportScriptableContents)) {
        kDebug() << "Registered settings object " << NM_DBUS_PATH_SETTINGS;
    } else {
        kDebug() << "Unable to register settings object " << NM_DBUS_PATH_SETTINGS;
    }

}

NMDBusSettingsService::~NMDBusSettingsService()
{
    if ( !QDBusConnection::systemBus().unregisterService( "org.freedesktop.NetworkManagerUserSettings" ) ) {
        // trouble;
        kDebug() << "Unable to unregister service";
    }
}

QUuid NMDBusSettingsService::uuidForPath(const QDBusObjectPath& path) const
{
    Q_D(const NMDBusSettingsService);
    BusConnection * busConn = 0;
    if (d->pathToConnections.contains(path)) {
        busConn = d->pathToConnections[path];
        return busConn->connection()->uuid();
    }
    return QUuid();
}

void NMDBusSettingsService::serviceOwnerChanged( const QString& service,const QString& oldOwner, const QString& newOwner )
{
    Q_D(NMDBusSettingsService);
    if (!oldOwner.isEmpty() && service == QLatin1String("org.freedesktop.NetworkManager")) {
        kDebug() << "NetworkManager stopped";
    }
    if (!newOwner.isEmpty() && service == QLatin1String("org.freedesktop.NetworkManager")) {
        kDebug() << "NetworkManager started!";
    }
    if (newOwner.isEmpty() && service == SERVICE_USER_SETTINGS && !d->active) {
        kDebug() << "User settings service was released, trying to register it ourselves";
        if (QDBusConnection::systemBus().interface()->registerService(SERVICE_USER_SETTINGS, QDBusConnectionInterface::QueueService, QDBusConnectionInterface::AllowReplacement)) {
            d->active = true;
        }
    }
}

void NMDBusSettingsService::serviceRegistered(const QString & name)
{
    Q_D(NMDBusSettingsService);
    if (name == SERVICE_USER_SETTINGS) {
        kDebug() << "service registered";
        d->active = true;
    }
}

void NMDBusSettingsService::serviceUnregistered(const QString & name)
{
    Q_D(NMDBusSettingsService);
    if (name == SERVICE_USER_SETTINGS) {
        kDebug() << "service lost, queueing reregistration";
        QDBusConnection::systemBus().interface()->registerService( SERVICE_USER_SETTINGS, QDBusConnectionInterface::QueueService, QDBusConnectionInterface::AllowReplacement );
        d->active = false;
    }
}

void NMDBusSettingsService::handleAdd(Knm::Connection * added)
{
    Q_D(NMDBusSettingsService);

    // only handle connections that come from local storage, not those from the system settings
    // service
    if (added->origin() == QLatin1String("ConnectionListPersistence")) {
        // put it on our bus 
        QDBusObjectPath objectPath;
        BusConnection * busConn = new BusConnection(added, this);
        new ConnectionAdaptor(busConn);
        new SecretsAdaptor(busConn);

        objectPath = QDBusObjectPath(nextObjectPath());
        // important - make sure all 3 hashes are up to date!
        d->pathToConnections.insert(objectPath, busConn);
        d->uuidToConnections.insert(added->uuid(), busConn);
        d->uuidToPath.insert(added->uuid(), objectPath);

        QDBusConnection::systemBus().registerObject(objectPath.path(), busConn, QDBusConnection::ExportAdaptors);
        emit NewConnection(objectPath);
    }
}

void NMDBusSettingsService::handleUpdate(Knm::Connection * updated)
{
    Q_D(NMDBusSettingsService);

    if (d->uuidToConnections.contains(updated->uuid())) {
        BusConnection * busConn = d->uuidToConnections[updated->uuid()];
        if (busConn) {
            busConn->updateInternal(updated);
        }
    }
}

void NMDBusSettingsService::handleRemove(Knm::Connection * removed)
{
    Q_D(NMDBusSettingsService);
    BusConnection * busConn = d->uuidToConnections.take(removed->uuid());
    if (busConn) {
        QDBusObjectPath key = d->pathToConnections.key(busConn);
        d->uuidToPath.remove(removed->uuid());
        d->pathToConnections.remove(key);
    }
    busConn->Delete();
}

void NMDBusSettingsService::handleAdd(Knm::Activatable * added)
{
    Q_D(NMDBusSettingsService);
    Knm::InterfaceConnection * ic = qobject_cast<Knm::InterfaceConnection*>(added);
    if (ic) {
        // listen to the IC
        kDebug() << ic->connectionUuid();
        //if (ic->activatableType() != Knm::Activatable::VpnInterfaceConnection) {
            connect(ic, SIGNAL(activated()), this, SLOT(interfaceConnectionActivated()));
        //}

        // if derived from one of our connections, tag it with the service and object path of the
        // connection.  The system settings monitor NMDBusSettingsConnectionProvider does this for
        // its connections.
        if (d->uuidToPath.contains(ic->connectionUuid())) {
            kDebug() << "tagging local InterfaceConnection " << ic->connectionName() << SERVICE_USER_SETTINGS << d->uuidToPath[ic->connectionUuid()].path();
            ic->setProperty("NMDBusService", SERVICE_USER_SETTINGS);
            ic->setProperty("NMDBusObjectPath", d->uuidToPath[ic->connectionUuid()].path());
        }
    }
    //if (ic->activatableType() == Knm::Activatable::VpnInterfaceConnection) {
}

void NMDBusSettingsService::interfaceConnectionActivated()
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
            foreach (QString activeConnectionPath, Solid::Control::NetworkManager::activeConnections()) {
                OrgFreedesktopNetworkManagerConnectionActiveInterface activeConnection("org.freedesktop.NetworkManager", activeConnectionPath, QDBusConnection::systemBus());

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

        Solid::Control::NetworkManager::activateConnection(deviceToActivateOn,
                QString::fromLatin1("%1 %2")
                .arg(ic->property("NMDBusService").toString(), ic->property("NMDBusObjectPath").toString()),
                extraArguments);
    }
}

void NMDBusSettingsService::handleUpdate(Knm::Activatable *)
{

}
void NMDBusSettingsService::handleRemove(Knm::Activatable *)
{

}

QString NMDBusSettingsService::nextObjectPath()
{
    Q_D(NMDBusSettingsService);
    return QString::fromLatin1("%1/%2").arg(QLatin1String(NM_DBUS_PATH_SETTINGS)).arg(d->nextConnectionId++);
}

QList<QDBusObjectPath> NMDBusSettingsService::ListConnections() const
{
    Q_D(const NMDBusSettingsService);
    QList<QDBusObjectPath> pathList = d->pathToConnections.keys();
    kDebug() << "There are " << pathList.count() << " known connections";
    return pathList;
}

#if QT_VERSION < 0x040500
inline bool operator==(const QDBusObjectPath &lhs, const QDBusObjectPath &rhs)
{ return lhs.path() == rhs.path(); }

inline bool operator!=(const QDBusObjectPath &lhs, const QDBusObjectPath &rhs)
{ return lhs.path() != rhs.path(); }

inline bool operator<(const QDBusObjectPath &lhs, const QDBusObjectPath &rhs)
{ return lhs.path() < rhs.path(); }
#endif

#endif // NMDBUSSETTINGSSERVICE_CPP
