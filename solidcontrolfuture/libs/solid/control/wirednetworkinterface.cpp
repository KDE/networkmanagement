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

#include "wirednetworkinterface.h"
#include "wirednetworkinterface_p.h"

#include "soliddefs_p.h"
#include "ifaces/wirednetworkinterface.h"

Solid::Control::WiredNetworkInterfaceNm09::WiredNetworkInterfaceNm09(QObject *backendObject)
    : NetworkInterfaceNm09(*new WiredNetworkInterfaceNm09Private(this), backendObject)
{
    Q_D(WiredNetworkInterfaceNm09);
    d->setBackendObject(backendObject);
}

Solid::Control::WiredNetworkInterfaceNm09::WiredNetworkInterfaceNm09(const WiredNetworkInterfaceNm09 &networkinterface)
    : NetworkInterfaceNm09(*new WiredNetworkInterfaceNm09Private(this), networkinterface)
{
    Q_D(WiredNetworkInterfaceNm09);
    d->setBackendObject(networkinterface.d_ptr->backendObject());
}

Solid::Control::WiredNetworkInterfaceNm09::WiredNetworkInterfaceNm09(WiredNetworkInterfaceNm09Private &dd, QObject *backendObject)
    : NetworkInterfaceNm09(dd, backendObject)
{
}

Solid::Control::WiredNetworkInterfaceNm09::WiredNetworkInterfaceNm09(WiredNetworkInterfaceNm09Private &dd, const WiredNetworkInterfaceNm09 &network)
    : NetworkInterfaceNm09(dd, network.d_ptr->backendObject())
{
}

Solid::Control::WiredNetworkInterfaceNm09::~WiredNetworkInterfaceNm09()
{

}

Solid::Control::NetworkInterfaceNm09::Type Solid::Control::WiredNetworkInterfaceNm09::type() const
{
    return Ethernet;
}

QString Solid::Control::WiredNetworkInterfaceNm09::hardwareAddress() const
{
    Q_D(const WiredNetworkInterfaceNm09);
    return_SOLID_CALL(Ifaces::WiredNetworkInterfaceNm09 *, d->backendObject(), QString(), hardwareAddress());
}

QString Solid::Control::WiredNetworkInterfaceNm09::permanentHardwareAddress() const
{
    Q_D(const WiredNetworkInterfaceNm09);
    return_SOLID_CALL(Ifaces::WiredNetworkInterfaceNm09 *, d->backendObject(), QString(), permanentHardwareAddress());
}

int Solid::Control::WiredNetworkInterfaceNm09::bitRate() const
{
    Q_D(const WiredNetworkInterfaceNm09);
    return_SOLID_CALL(Ifaces::WiredNetworkInterfaceNm09 *, d->backendObject(), 0, bitRate());
}

bool Solid::Control::WiredNetworkInterfaceNm09::carrier() const
{
    Q_D(const WiredNetworkInterfaceNm09);
    return_SOLID_CALL(Ifaces::WiredNetworkInterfaceNm09 *, d->backendObject(), false, carrier());
}

void Solid::Control::WiredNetworkInterfaceNm09Private::setBackendObject(QObject *object)
{
    NetworkInterfaceNm09Private::setBackendObject(object);

    if (object) {
        QObject::connect(object, SIGNAL(bitRateChanged(int)),
                         parent(), SIGNAL(bitRateChanged(int)));
        QObject::connect(object, SIGNAL(carrierChanged(bool)),
                         parent(), SIGNAL(carrierChanged(bool)));
    }
}

void Solid::Control::WiredNetworkInterfaceNm09::_k_destroyed(QObject *object)
{
    Q_UNUSED(object);
    // nothing to do yet
}
#include "wirednetworkinterface.moc"
