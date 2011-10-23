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

#include "networkinterface.h"
#include "networkinterface_p.h"

#include <arpa/inet.h>

#include <KDebug>

#include "dbus/nm-ip4-configinterface.h"
#include "manager.h"
#include "networkmanagerdefinitions.h"

NMNetworkInterfacePrivate::NMNetworkInterfacePrivate( const QString & path, QObject * owner ) : deviceIface(NM_DBUS_SERVICE, path, QDBusConnection::systemBus()), uni(path), designSpeed(0), manager(0)/*, propHelper(owner)*/
{
    Q_UNUSED(owner);
    //isLinkUp = deviceIface.isLinkUp();
    driver = deviceIface.driver();
    interfaceName = deviceIface.interface();
    ipV4Address = deviceIface.ip4Address();    
    managed = deviceIface.managed();
    udi = deviceIface.udi();
    firmwareMissing = deviceIface.firmwareMissing();

    //TODO set active connections based on active connection list on the manager; find out if
    //signal needed
    //activeConnection = deviceIface.activeConnection();
    //propHelper.registerProperty(NM_DEVICE_UDI, PropertySignalPair("uni",0));
}

NMNetworkInterfacePrivate::~NMNetworkInterfacePrivate()
{

}

NMNetworkInterface::NMNetworkInterface(const QString & path, NMNetworkManagerNm09 * manager, QObject * parent) : QObject(parent), d_ptr(new NMNetworkInterfacePrivate(path, this))
{
    Q_D(NMNetworkInterface);
    init();
    d->manager = manager;
}

NMNetworkInterface::NMNetworkInterface(NMNetworkInterfacePrivate & dd, NMNetworkManagerNm09 * manager, QObject * parent) : QObject(parent), d_ptr(&dd)
{
    qDBusRegisterMetaType<UIntList>();
    qDBusRegisterMetaType<UIntListList>();
    Q_D(NMNetworkInterface);
    init();
    d->manager = manager;
}

void NMNetworkInterface::init()
{
    Q_D(NMNetworkInterface);
    d->capabilities = convertCapabilities(d->deviceIface.capabilities());
    d->connectionState = convertState(d->deviceIface.state());

    connect(&d->deviceIface, SIGNAL(StateChanged(uint,uint,uint)), this, SLOT(stateChanged(uint,uint,uint)));
}

NMNetworkInterface::~NMNetworkInterface()
{
    delete d_ptr;
}

QString NMNetworkInterface::uni() const
{
    Q_D(const NMNetworkInterface);
    return d->uni;
}

void NMNetworkInterface::setUni(const QVariant & uni)
{
    Q_D(NMNetworkInterface);
    d->uni = uni.toString();
}

QString NMNetworkInterface::interfaceName() const
{
    Q_D(const NMNetworkInterface);
    return d->interfaceName;
}

void NMNetworkInterface::setInterfaceName(const QVariant & name)
{
    Q_D(NMNetworkInterface);
    d->interfaceName = name.toString();
}

QString NMNetworkInterface::ipInterfaceName() const
{
    Q_D(const NMNetworkInterface);
    return d->deviceIface.ipInterface();
}

QString NMNetworkInterface::driver() const
{
    Q_D(const NMNetworkInterface);
    return d->driver;
}

bool NMNetworkInterface::firmwareMissing() const
{
    Q_D(const NMNetworkInterface);
    return d->firmwareMissing;
}

QString NMNetworkInterface::activeConnection() const
{
    Q_D(const NMNetworkInterface);
    return d->deviceIface.activeConnection().path();
}

void NMNetworkInterface::setDriver(const QVariant & driver)
{
    Q_D(NMNetworkInterface);
    d->driver = driver.toString();
}

QString NMNetworkInterface::udi() const
{
    Q_D(const NMNetworkInterface);
    return d->udi;
}

uint NMNetworkInterface::ipV4Address() const
{
    Q_D(const NMNetworkInterface);
    return d->ipV4Address;
}

Solid::Control::IPv4ConfigNm09 NMNetworkInterface::ipV4Config() const
{
    Q_D(const NMNetworkInterface);
    if (d->connectionState != Solid::Control::NetworkInterfaceNm09::Activated) {
        return Solid::Control::IPv4ConfigNm09();
    } else {
        // ask the daemon for the details
        QDBusObjectPath ipV4ConfigPath = d->deviceIface.ip4Config();
        OrgFreedesktopNetworkManagerIP4ConfigInterface iface(NM_DBUS_SERVICE, ipV4ConfigPath.path(), QDBusConnection::systemBus());
        if (iface.isValid()) {
            //convert ipaddresses into object
            UIntListList addresses = iface.addresses();
            QList<Solid::Control::IPv4AddressNm09> addressObjects;
            foreach (const UIntList & addressList, addresses) {
                if ( addressList.count() == 3 ) {
                    Solid::Control::IPv4AddressNm09 addr(htonl(addressList[0]), addressList[1], htonl(addressList[2]));
                    addressObjects.append(addr);
                }
            }
            //convert routes into objects
            UIntListList routes = iface.routes();
            QList<Solid::Control::IPv4RouteNm09> routeObjects;
            foreach (const UIntList & routeList, routes) {
                if ( routeList.count() == 4 ) {
                    Solid::Control::IPv4RouteNm09 addr(routeList[0], routeList[1], routeList[2], routeList[3]);
                    routeObjects.append(addr);
                }
            }
            // nameservers' IP addresses are always in network byte order
            return Solid::Control::IPv4ConfigNm09(addressObjects,
                iface.nameservers(), iface.domains(),
                routeObjects);
        } else {
            return Solid::Control::IPv4ConfigNm09();
        }
    }
}

bool NMNetworkInterface::isActive() const
{
    Q_D(const NMNetworkInterface);
    return !(d->connectionState == Solid::Control::NetworkInterfaceNm09::Unavailable
            || d->connectionState == Solid::Control::NetworkInterfaceNm09::Disconnected
            || d->connectionState == Solid::Control::NetworkInterfaceNm09::Failed );
}

bool NMNetworkInterface::managed() const
{
    Q_D(const NMNetworkInterface);
    return d->managed;
}

void NMNetworkInterface::disconnectInterface()
{
    Q_D(NMNetworkInterface);
    d->deviceIface.Disconnect();
}

void NMNetworkInterface::setManaged(const QVariant & driver)
{
    Q_D(NMNetworkInterface);
    d->driver = driver.toBool();
}

Solid::Control::NetworkInterfaceNm09::ConnectionState NMNetworkInterface::connectionState() const
{
    Q_D(const NMNetworkInterface);
    return d->connectionState;
}

void NMNetworkInterface::setConnectionState(const QVariant & state)
{
    Q_D(NMNetworkInterface);
    d->connectionState = convertState(state.toUInt());
}

int NMNetworkInterface::designSpeed() const
{
    Q_D(const NMNetworkInterface);
    return d->designSpeed;
}
/*
bool NMNetworkInterface::isLinkUp() const
{
    Q_D(const NMNetworkInterface);
    return d->isLinkUp;
}
*/
Solid::Control::NetworkInterfaceNm09::Capabilities NMNetworkInterface::capabilities() const
{
    Q_D(const NMNetworkInterface);
    return d->capabilities;
}

QVariant NMNetworkInterface::capabilitiesV() const
{
    Q_D(const NMNetworkInterface);
    return QVariant(d->capabilities);
}

void NMNetworkInterface::setCapabilitiesV(const QVariant & caps)
{
    Q_D(NMNetworkInterface);
    d->capabilities = convertCapabilities(caps.toUInt());
}

Solid::Control::NetworkInterfaceNm09::Capabilities NMNetworkInterface::convertCapabilities(uint theirCaps)
{
    Solid::Control::NetworkInterfaceNm09::Capabilities ourCaps
        = (Solid::Control::NetworkInterfaceNm09::Capabilities) theirCaps;
    return ourCaps;
}

Solid::Control::NetworkInterfaceNm09::ConnectionState NMNetworkInterface::convertState(uint theirState)
{
    Solid::Control::NetworkInterfaceNm09::ConnectionState ourState = (Solid::Control::NetworkInterfaceNm09::ConnectionState)theirState;
    return ourState;
}

void NMNetworkInterface::stateChanged(uint new_state, uint old_state, uint reason)
{
    Q_D(NMNetworkInterface);
    d->connectionState = convertState(new_state);
    emit connectionStateChanged(d->connectionState, convertState(old_state), reason);
}

#include "networkinterface.moc"

