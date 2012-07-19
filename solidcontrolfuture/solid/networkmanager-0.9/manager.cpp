/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of
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

#include "manager.h"
#include "manager_p.h"

#include <KDebug>

#include "dbus/nm-deviceinterface.h"
#include "networkmanagerdefinitions.h"
#include "wirednetworkinterface.h"
#include "wirelessnetworkinterface.h"
#include "networkmodeminterface.h"
#include "networkbtinterface.h"
#include "dbus/nm-active-connectioninterface.h"

NMNetworkManagerNm09Private::NMNetworkManagerNm09Private()
    : iface(NM_DBUS_SERVICE, NM_DBUS_PATH, QDBusConnection::systemBus())
{
    kDebug(1441) << NM_DBUS_SERVICE;
}

NMNetworkManagerNm09::NMNetworkManagerNm09(QObject * parent, const QVariantList &) 
{
    qDBusRegisterMetaType<QList<QDBusObjectPath> >();
    d_ptr = new NMNetworkManagerNm09Private;
    Q_D(NMNetworkManagerNm09);
    d->version = d->iface.version();
    parseVersion();
    d->nmState = d->iface.state();
    d->isWirelessHardwareEnabled = d->iface.wirelessHardwareEnabled();
    d->isWirelessEnabled = d->iface.wirelessEnabled();
    d->isWwanHardwareEnabled = d->iface.wwanHardwareEnabled();
    d->isWwanEnabled = d->iface.wwanEnabled();
    d->isNetworkingEnabled = d->iface.networkingEnabled();
    connect( &d->iface, SIGNAL(DeviceAdded(QDBusObjectPath)),
                this, SLOT(deviceAdded(QDBusObjectPath)));
    connect( &d->iface, SIGNAL(DeviceRemoved(QDBusObjectPath)),
                this, SLOT(deviceRemoved(QDBusObjectPath)));
    connect( &d->iface, SIGNAL(PropertiesChanged(QVariantMap)),
                this, SLOT(propertiesChanged(QVariantMap)));
    connect( &d->iface, SIGNAL(StateChanged(uint)),
                this, SLOT(stateChanged(uint)));

    d->iface.connection().connect(QLatin1String("org.freedesktop.DBus"),
            QLatin1String("/org/freedesktop/DBus"), QLatin1String("org.freedesktop.DBus"),
            QLatin1String("NameOwnerChanged"), QLatin1String("sss"),
            this, SLOT(nameOwnerChanged(QString,QString,QString)));

    QDBusReply< QList <QDBusObjectPath> > deviceList = d->iface.GetDevices();
    if (deviceList.isValid())
    {
        kDebug(1441) << "Device list";
        QList <QDBusObjectPath> devices = deviceList.value();
        foreach (const QDBusObjectPath &op, devices)
        {
            d->networkInterfaces.append(op.path());
            kDebug(1441) << "  " << op.path();
        }
    }
    else
        kDebug(1441) << "Error getting device list: " << deviceList.error().name() << ": " << deviceList.error().message();

    kDebug(1441) << "Active connections:";
    QList <QDBusObjectPath> activeConnections = d->iface.activeConnections();
    foreach (const QDBusObjectPath &ac, activeConnections)
    {
        d->activeConnections.append(ac.path());
        kDebug(1441) << "  " << ac.path();
    }
}

NMNetworkManagerNm09::~NMNetworkManagerNm09()
{
    delete d_ptr;
}

void NMNetworkManagerNm09::parseVersion()
{
    Q_D(NMNetworkManagerNm09);
    QStringList sl = d->version.split('.');

    if (sl.size() > 2) {
        d->x = sl[0].toInt();
        d->y = sl[1].toInt();
        d->z = sl[2].toInt();
    } else {
        d->x = -1;
        d->y = -1;
        d->z = -1;
    }
}

int NMNetworkManagerNm09::compareVersion(const QString & version)
{
    int x, y, z;

    QStringList sl = version.split('.');

    if (sl.size() > 2) {
        x = sl[0].toInt();
        y = sl[1].toInt();
        z = sl[2].toInt();
    } else {
        x = -1;
        y = -1;
        z = -1;
    }

    return compareVersion(x, y, z);
}

int NMNetworkManagerNm09::compareVersion(const int x, const int y, const int z) const
{
    Q_D(const NMNetworkManagerNm09);
    if (d->x > x) {
        return 1;
    } else if (d->x < x) {
        return -1;
    } else if (d->y > y) {
        return 1;
    } else if (d->y < y) {
        return -1;
    } else if (d->z > z) {
        return 1;
    } else if (d->z < z) {
        return -1;
    }
    return 0;
}

Solid::Networking::Status NMNetworkManagerNm09::status() const
{
    Q_D(const NMNetworkManagerNm09);
    return convertNMState(d->nmState);
}

QStringList NMNetworkManagerNm09::networkInterfaces() const
{
    Q_D(const NMNetworkManagerNm09);
    return d->networkInterfaces;
}

QObject *NMNetworkManagerNm09::createNetworkInterface(const QString &uni)
{
    kDebug(1441);
    OrgFreedesktopNetworkManagerDeviceInterface devIface(NM_DBUS_SERVICE, uni, QDBusConnection::systemBus());
    uint deviceType = devIface.deviceType();
    NMNetworkInterface * createdInterface = 0;
    switch ( deviceType ) {
        case NM_DEVICE_TYPE_ETHERNET:
            createdInterface = new NMWiredNetworkInterface(uni, this, 0); // these are deleted by the frontend manager
            break;
        case NM_DEVICE_TYPE_WIFI:
            createdInterface = new NMWirelessNetworkInterface(uni, this, 0);
            break;
        case NM_DEVICE_TYPE_MODEM:
            createdInterface = new NMModemNetworkInterface(uni, this, 0);
            break;
        case NM_DEVICE_TYPE_BT:
            createdInterface = new NMBtNetworkInterface(uni, this, 0);
            break;

        /* TODO: add NM_DEVICE_TYPE_WIMAX and NM_DEVICE_TYPE_OLPC_MESH */
        default:
            kDebug(1441) << "Can't create object of type " << deviceType;
            break;
    }

    return createdInterface;
}

bool NMNetworkManagerNm09::isNetworkingEnabled() const
{
    Q_D(const NMNetworkManagerNm09);
    return d->isNetworkingEnabled;
}

bool NMNetworkManagerNm09::isWirelessEnabled() const
{
    Q_D(const NMNetworkManagerNm09);
    return d->isWirelessEnabled;
}

bool NMNetworkManagerNm09::isWirelessHardwareEnabled() const
{
    Q_D(const NMNetworkManagerNm09);
    return d->isWirelessHardwareEnabled;
}

bool NMNetworkManagerNm09::isWwanEnabled() const
{
    Q_D(const NMNetworkManagerNm09);
    return d->isWwanEnabled;
}

bool NMNetworkManagerNm09::isWwanHardwareEnabled() const
{
    Q_D(const NMNetworkManagerNm09);
    return d->isWwanHardwareEnabled;
}

void NMNetworkManagerNm09::activateConnection(const QString & interfaceUni, const QString & connectionUni, const QVariantMap & connectionParameters)
{
    Q_D(NMNetworkManagerNm09);
    QString connectionPath = connectionUni.section(' ', 1, 1);
    // ### FIXME find a better name for the parameter needed for NM 0.7
    QString extra_connection_parameter = connectionParameters.value("extra_connection_parameter").toString();
    if (extra_connection_parameter.isEmpty()) {
        extra_connection_parameter = QLatin1String("/");
    }
    if ( connectionPath.isEmpty() ) {
        return;
    }
    // TODO store error code
    QDBusObjectPath connPath(connectionPath);
    QDBusObjectPath interfacePath(interfaceUni);
    kDebug(1441) << "Activating connection" << connPath.path() << "on interface" << interfacePath.path() << "with extra" << extra_connection_parameter;
    d->iface.ActivateConnection(connPath, interfacePath, QDBusObjectPath(extra_connection_parameter));
}

QString NMNetworkManagerNm09::version() const
{
    Q_D(const NMNetworkManagerNm09);
    return d->version;
}

void NMNetworkManagerNm09::deactivateConnection( const QString & activeConnectionPath )
{
    Q_D(NMNetworkManagerNm09);
    d->iface.DeactivateConnection(QDBusObjectPath(activeConnectionPath));
}

void NMNetworkManagerNm09::setNetworkingEnabled(bool enabled)
{
    Q_D(NMNetworkManagerNm09);

    QDBusPendingReply<> reply = d->iface.Enable(enabled);
    reply.waitForFinished();
    if (reply.isError()) {
        kDebug(1441) << "Enable() D-Bus method return error:" << reply.error();
    }
}

void NMNetworkManagerNm09::setWirelessEnabled(bool enabled)
{
    Q_D(NMNetworkManagerNm09);
    d->iface.setWirelessEnabled(enabled);
}

void NMNetworkManagerNm09::setWwanEnabled(bool enabled)
{
    Q_D(NMNetworkManagerNm09);
    d->iface.setWwanEnabled(enabled);
}

void NMNetworkManagerNm09::deviceAdded(const QDBusObjectPath & objpath)
{
    kDebug(1441);
    Q_D(NMNetworkManagerNm09);
    d->networkInterfaces.append(objpath.path());
    emit networkInterfaceAdded(objpath.path());
}

void NMNetworkManagerNm09::deviceRemoved(const QDBusObjectPath & objpath)
{
    kDebug(1441);
    Q_D(NMNetworkManagerNm09);
    d->networkInterfaces.removeAll(objpath.path());
    emit networkInterfaceRemoved(objpath.path());
}

void NMNetworkManagerNm09::stateChanged(uint state)
{
    Q_D(NMNetworkManagerNm09);
    if ( d->nmState != state ) {
        if (d->nmState == NM_STATE_UNKNOWN) {
            d->isWirelessHardwareEnabled = d->iface.wirelessHardwareEnabled();
            d->isWirelessEnabled = d->iface.wirelessEnabled();
            d->isWwanHardwareEnabled = d->iface.wwanHardwareEnabled();
            d->isWwanEnabled = d->iface.wwanEnabled();
            d->isNetworkingEnabled = d->iface.networkingEnabled();
        }

        // set new state
        d->nmState = state;
        emit statusChanged( convertNMState( state ) );
    }

}

void NMNetworkManagerNm09::propertiesChanged(const QVariantMap &properties)
{
    Q_D(NMNetworkManagerNm09);
    kDebug(1441) << properties.keys();
    QLatin1String activeConnKey("ActiveConnections");
    QLatin1String netEnabledKey("NetworkingEnabled");
    QLatin1String wifiHwKey("WirelessHardwareEnabled");
    QLatin1String wifiEnabledKey("WirelessEnabled");
    QLatin1String wwanHwKey("WwanHardwareEnabled");
    QLatin1String wwanEnabledKey("WwanEnabled");
    QVariantMap::const_iterator it = properties.find(activeConnKey);
    if ( it != properties.end()) {
        QList<QDBusObjectPath> activePaths = qdbus_cast< QList<QDBusObjectPath> >(*it);
        d->activeConnections.clear();
        if ( activePaths.count() ) {
            kDebug(1441) << activeConnKey;
        }
        foreach (const QDBusObjectPath &ac, activePaths)
        {
            d->activeConnections.append(ac.path());
            kDebug(1441) << "  " << ac.path();
        }
        emit activeConnectionsChanged();
    }
    it = properties.find(wifiHwKey);
    if ( it != properties.end()) {
        d->isWirelessHardwareEnabled = it->toBool();
        kDebug(1441) << wifiHwKey << d->isWirelessHardwareEnabled;
        emit wirelessHardwareEnabledChanged(d->isWirelessHardwareEnabled);
    }
    it = properties.find(wifiEnabledKey);
    if ( it != properties.end()) {
        d->isWirelessEnabled = it->toBool();
        kDebug(1441) << wifiEnabledKey << d->isWirelessEnabled;
        emit wirelessEnabledChanged(d->isWirelessEnabled);
    }
    it = properties.find(wwanHwKey);
    if ( it != properties.end()) {
        d->isWwanHardwareEnabled = it->toBool();
        kDebug(1441) << wwanHwKey << d->isWwanHardwareEnabled;
    }
    it = properties.find(wwanEnabledKey);
    if ( it != properties.end()) {
        d->isWwanEnabled = it->toBool();
        kDebug(1441) << wwanEnabledKey << d->isWwanEnabled;
        emit wwanEnabledChanged(d->isWwanEnabled);
    }
    it = properties.find(netEnabledKey);
    if ( it != properties.end()) {
        d->isNetworkingEnabled = it->toBool();
        kDebug(1441) << netEnabledKey << d->isNetworkingEnabled;
        emit networkingEnabledChanged(d->isNetworkingEnabled);
    }
}

Solid::Networking::Status NMNetworkManagerNm09::convertNMState(uint state)
{
    Solid::Networking::Status status = Solid::Networking::Unknown;
    switch (state) {
        case NM_STATE_UNKNOWN:
        case NM_STATE_ASLEEP:
            status = Solid::Networking::Unknown;
            break;
        case NM_STATE_DISCONNECTED:
            status = Solid::Networking::Unconnected;
            break;
        case NM_STATE_DISCONNECTING:
            status = Solid::Networking::Disconnecting;
            break;
        case NM_STATE_CONNECTING:
            status = Solid::Networking::Connecting;
            break;
        case NM_STATE_CONNECTED_LOCAL:
        case NM_STATE_CONNECTED_SITE:
        case NM_STATE_CONNECTED_GLOBAL:
            status = Solid::Networking::Connected;
            break;
    }
    return status;
}

void NMNetworkManagerNm09::nameOwnerChanged(QString name, QString oldOwner, QString newOwner)
{
    if ( name == QLatin1String("org.freedesktop.NetworkManager") ) {
        kDebug(1441) << "name: " << name << ", old owner: " << oldOwner << ", new owner: " << newOwner;
        if ( oldOwner.isEmpty() && !newOwner.isEmpty() ) {
            // NetworkManager started, but we are already listening to StateChanged so we should get
            // its status that way
            ;
        }
        if ( !oldOwner.isEmpty() && newOwner.isEmpty() ) {
            // NetworkManager stopped or crashed, set status Unknown for safety
            stateChanged(NM_STATE_UNKNOWN);
        }
    }
}

QStringList NMNetworkManagerNm09::activeConnections() const
{
    Q_D(const NMNetworkManagerNm09);
    return d->activeConnections;
}

QStringList NMNetworkManagerNm09::activeConnectionsUuid() const
{
    Q_D(const NMNetworkManagerNm09);
    QStringList r;
    foreach(const QString & objPath, d->activeConnections) {
        OrgFreedesktopNetworkManagerConnectionActiveInterface iface(NM_DBUS_SERVICE, objPath, QDBusConnection::systemBus());
        r.append(iface.uuid());
    }
    return r;
}

Solid::Control::NetworkInterfaceNm09::Types NMNetworkManagerNm09::supportedInterfaceTypes() const
{
    return (Solid::Control::NetworkInterfaceNm09::Types) (
           Solid::Control::NetworkInterfaceNm09::Wifi |
           Solid::Control::NetworkInterfaceNm09::Ethernet |
           Solid::Control::NetworkInterfaceNm09::Bluetooth |
           Solid::Control::NetworkInterfaceNm09::Modem
           /* TODO: implement those two and add them to this list.
           Solid::Control::NetworkInterfaceNm09::Wimax |
           Solid::Control::NetworkInterfaceNm09::Olpc */
           );
}

#include "manager.moc"

