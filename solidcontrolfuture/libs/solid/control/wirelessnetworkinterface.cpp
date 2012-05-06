/*  This file is part of the KDE project
    Copyright (C) 2006 Will Stephenson <wstephenson@kde.org>
    Copyright (C) 2007 Kevin Ottens <ervin@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "wirelessnetworkinterface.h"
#include "wirelessnetworkinterface_p.h"

#include "soliddefs_p.h"
#include "wirelessaccesspoint.h"
#include "ifaces/wirelessaccesspoint.h"
#include "ifaces/wirelessnetworkinterface.h"

Solid::Control::WirelessNetworkInterfaceNm09::WirelessNetworkInterfaceNm09(QObject *backendObject)
    : NetworkInterfaceNm09(*new WirelessNetworkInterfaceNm09Private(this), backendObject)
{
    Q_D(WirelessNetworkInterfaceNm09);
    d->setBackendObject(backendObject);
    makeConnections( backendObject );
    d->readAccessPoints();
}

Solid::Control::WirelessNetworkInterfaceNm09::WirelessNetworkInterfaceNm09(const WirelessNetworkInterfaceNm09 &networkinterface)
    : NetworkInterfaceNm09(*new WirelessNetworkInterfaceNm09Private(this), networkinterface)
{
    Q_D(WirelessNetworkInterfaceNm09);
    d->setBackendObject(networkinterface.d_ptr->backendObject());
    makeConnections( networkinterface.d_ptr->backendObject() );
    d->readAccessPoints();
}

Solid::Control::WirelessNetworkInterfaceNm09::WirelessNetworkInterfaceNm09(WirelessNetworkInterfaceNm09Private &dd, QObject *backendObject)
    : NetworkInterfaceNm09(dd, backendObject)
{
    Q_D(WirelessNetworkInterfaceNm09);
    makeConnections( backendObject );
    d->readAccessPoints();
}

Solid::Control::WirelessNetworkInterfaceNm09::WirelessNetworkInterfaceNm09(WirelessNetworkInterfaceNm09Private &dd, const WirelessNetworkInterfaceNm09 &networkinterface)
    : NetworkInterfaceNm09(dd, networkinterface.d_ptr->backendObject())
{
    Q_D(WirelessNetworkInterfaceNm09);
    makeConnections( networkinterface.d_ptr->backendObject() );
    d->readAccessPoints();
}

Solid::Control::WirelessNetworkInterfaceNm09::~WirelessNetworkInterfaceNm09()
{

}

Solid::Control::NetworkInterfaceNm09::Type Solid::Control::WirelessNetworkInterfaceNm09::type() const
{
    return Wifi;
}

void Solid::Control::WirelessNetworkInterfaceNm09::makeConnections(QObject * source)
{
    connect(source, SIGNAL(accessPointAppeared(QString)),
            this, SLOT(_k_accessPointAdded(QString)));
    connect(source, SIGNAL(accessPointDisappeared(QString)),
            this, SLOT(_k_accessPointRemoved(QString)));
    connect(source, SIGNAL(bitRateChanged(int)),
            this, SIGNAL(bitRateChanged(int)));
    connect(source, SIGNAL(activeAccessPointChanged(QString)),
            this, SIGNAL(activeAccessPointChanged(QString)));
    connect(source, SIGNAL(modeChanged(Solid::Control::WirelessNetworkInterfaceNm09::OperationMode)),
            this, SIGNAL(modeChanged(Solid::Control::WirelessNetworkInterfaceNm09::OperationMode)));
}

QString Solid::Control::WirelessNetworkInterfaceNm09::hardwareAddress() const
{
    Q_D(const WirelessNetworkInterfaceNm09);
    return_SOLID_CALL(Ifaces::WirelessNetworkInterfaceNm09 *, d->backendObject(), QString(), hardwareAddress());
}

QString Solid::Control::WirelessNetworkInterfaceNm09::permanentHardwareAddress() const
{
    Q_D(const WirelessNetworkInterfaceNm09);
    return_SOLID_CALL(Ifaces::WirelessNetworkInterfaceNm09 *, d->backendObject(), QString(), permanentHardwareAddress());
}

QString Solid::Control::WirelessNetworkInterfaceNm09::activeAccessPoint() const
{
    Q_D(const WirelessNetworkInterfaceNm09);
    return_SOLID_CALL(Ifaces::WirelessNetworkInterfaceNm09 *, d->backendObject(), QString(), activeAccessPoint());
}

int Solid::Control::WirelessNetworkInterfaceNm09::bitRate() const
{
    Q_D(const WirelessNetworkInterfaceNm09);
    return_SOLID_CALL(Ifaces::WirelessNetworkInterfaceNm09 *, d->backendObject(), 0, bitRate());
}

Solid::Control::WirelessNetworkInterfaceNm09::OperationMode Solid::Control::WirelessNetworkInterfaceNm09::mode() const
{
    Q_D(const WirelessNetworkInterfaceNm09);
    return_SOLID_CALL(Ifaces::WirelessNetworkInterfaceNm09 *, d->backendObject(), (Solid::Control::WirelessNetworkInterfaceNm09::OperationMode)0, mode());
}

Solid::Control::WirelessNetworkInterfaceNm09::Capabilities Solid::Control::WirelessNetworkInterfaceNm09::wirelessCapabilities() const
{
    Q_D(const WirelessNetworkInterfaceNm09);
    return_SOLID_CALL(Ifaces::WirelessNetworkInterfaceNm09 *, d->backendObject(), (Solid::Control::WirelessNetworkInterfaceNm09::Capabilities)0, wirelessCapabilities());
}

Solid::Control::AccessPointNm09 * Solid::Control::WirelessNetworkInterfaceNm09::findAccessPoint(const QString  & uni) const
{
    const AccessPointNm09Pair pair = findRegisteredAccessPoint(uni);
    return pair.first;
}

void Solid::Control::WirelessNetworkInterfaceNm09Private::setBackendObject(QObject *object)
{
    NetworkInterfaceNm09Private::setBackendObject(object);

    if (object) {
    }
}

Solid::Control::AccessPointNm09List Solid::Control::WirelessNetworkInterfaceNm09::accessPoints() const
{
    Q_D(const WirelessNetworkInterfaceNm09);
    return d->apMap.keys();
}

void Solid::Control::WirelessNetworkInterfaceNm09::_k_accessPointAdded(const QString & uni)
{
    Q_D(WirelessNetworkInterfaceNm09);
    AccessPointNm09Map::Iterator mapIt = d->apMap.find(uni);

    if (mapIt != d->apMap.end())
    {
        // Oops, I'm not sure it should happen...
        // But for an unknown reason it does
        // We were deleting the existing items here, which caused problems later
        // when the client calls find AccessPoint and gets a frontend object with a
        // null private object - dangling pointer
    } else {
        mapIt = d->apMap.insert(uni, AccessPointNm09Pair(0, 0));
    }

    emit accessPointAppeared(uni);
}

void Solid::Control::WirelessNetworkInterfaceNm09::_k_accessPointRemoved(const QString & uni)
{
    Q_D(WirelessNetworkInterfaceNm09);
    AccessPointNm09Pair pair = d->apMap.take(uni);

    if (pair.first!= 0)
    {
        delete pair.first;
        delete pair.second;
    }

    emit accessPointDisappeared(uni);
}

void Solid::Control::WirelessNetworkInterfaceNm09::_k_destroyed(QObject *object)
{
    Q_D(WirelessNetworkInterfaceNm09);
    Ifaces::AccessPointNm09 *ap = qobject_cast<Ifaces::AccessPointNm09 *>(object);

    if (ap!=0)
    {
        QString uni = ap->uni();
        AccessPointNm09Pair pair = d->apMap.take(uni);
        delete pair.first;
    }
}

Solid::Control::AccessPointNm09Pair
Solid::Control::WirelessNetworkInterfaceNm09::findRegisteredAccessPoint(const QString &uni) const
{
    Q_D(const WirelessNetworkInterfaceNm09);

    AccessPointNm09Map::ConstIterator mapIt = d->apMap.constFind(uni);
    if (mapIt != d->apMap.constEnd() && mapIt.value().second) {
        return mapIt.value();
    } else {
        AccessPointNm09Pair pair = d->createAP(uni);
        if (pair.first && pair.second) {
            d->apMap[uni] = pair;
        }
        return pair;
    }

    return AccessPointNm09Pair(0, 0);
}

void Solid::Control::WirelessNetworkInterfaceNm09Private::readAccessPoints()
{
    Ifaces::WirelessNetworkInterfaceNm09 * t = qobject_cast<Ifaces::WirelessNetworkInterfaceNm09 *>(backendObject());
    if (t != 0)
    {
        const MacAddressNm09List unis = t->accessPoints();
        Q_FOREACH (const QString & uni, unis) {
            apMap[uni] = AccessPointNm09Pair(0, 0);
        }
    }
}

Solid::Control::AccessPointNm09Pair
Solid::Control::WirelessNetworkInterfaceNm09Private::createAP(const QString &uni) const
{
    Ifaces::WirelessNetworkInterfaceNm09 *device = qobject_cast<Ifaces::WirelessNetworkInterfaceNm09 *>(backendObject());
    AccessPointNm09 *ap = 0;

    if (device!=0) {
        Ifaces::AccessPointNm09 *iface = qobject_cast<Ifaces::AccessPointNm09 *>(device->createAccessPoint(uni));

        if (qobject_cast<Ifaces::AccessPointNm09 *>(iface)!=0) {
            ap = new AccessPointNm09(iface);
        }

        if (ap != 0) {
            AccessPointNm09Pair pair(ap, iface);
            QObject::connect(iface, SIGNAL(destroyed(QObject*)),
                             parent(), SLOT(_k_destroyed(QObject*)));

            return pair;
        }
    }
    return AccessPointNm09Pair(0, 0);
}

#include "wirelessnetworkinterface.moc"
