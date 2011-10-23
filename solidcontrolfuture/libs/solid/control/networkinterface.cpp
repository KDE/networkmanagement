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

#include "networkinterface.h"
#include "networkinterface_p.h"

#include "soliddefs_p.h"
#include "ifaces/networkinterface.h"

Solid::Control::NetworkInterfaceNm09::NetworkInterfaceNm09(QObject *backendObject)
    : QObject(), d_ptr(new NetworkInterfaceNm09Private(this))
{
    Q_D(NetworkInterfaceNm09); d->setBackendObject(backendObject);
}

Solid::Control::NetworkInterfaceNm09::NetworkInterfaceNm09(const NetworkInterfaceNm09 &other)
    : QObject(), d_ptr(new NetworkInterfaceNm09Private(this))
{
    Q_D(NetworkInterfaceNm09);
    d->setBackendObject(other.d_ptr->backendObject());
}

Solid::Control::NetworkInterfaceNm09::NetworkInterfaceNm09(NetworkInterfaceNm09Private &dd, QObject *backendObject)
    : QObject(), d_ptr(&dd)
{
    Q_UNUSED(backendObject);
}

Solid::Control::NetworkInterfaceNm09::NetworkInterfaceNm09(NetworkInterfaceNm09Private &dd, const NetworkInterfaceNm09 &other)
    : d_ptr(&dd)
{
    Q_UNUSED(other);
}

Solid::Control::NetworkInterfaceNm09::~NetworkInterfaceNm09()
{
    delete d_ptr;
}

Solid::Control::NetworkInterfaceNm09::Type Solid::Control::NetworkInterfaceNm09::type() const
{
    return UnknownType;
}

QString Solid::Control::NetworkInterfaceNm09::uni() const
{
    Q_D(const NetworkInterfaceNm09);
    return_SOLID_CALL(Ifaces::NetworkInterfaceNm09 *, d->backendObject(), QString(), uni());
}

QString Solid::Control::NetworkInterfaceNm09::udi() const
{
    Q_D(const NetworkInterfaceNm09);
    return_SOLID_CALL(Ifaces::NetworkInterfaceNm09 *, d->backendObject(), QString(), udi());
}

QString Solid::Control::NetworkInterfaceNm09::interfaceName() const
{
    Q_D(const NetworkInterfaceNm09);
    return_SOLID_CALL(Ifaces::NetworkInterfaceNm09 *, d->backendObject(), QString(), interfaceName());
}

QString Solid::Control::NetworkInterfaceNm09::ipInterfaceName() const
{
    Q_D(const NetworkInterfaceNm09);
    return_SOLID_CALL(Ifaces::NetworkInterfaceNm09 *, d->backendObject(), QString(), ipInterfaceName());
}

QString Solid::Control::NetworkInterfaceNm09::activeConnection() const
{
    Q_D(const NetworkInterfaceNm09);
    return_SOLID_CALL(Ifaces::NetworkInterfaceNm09 *, d->backendObject(), QString(), activeConnection());
}

QString Solid::Control::NetworkInterfaceNm09::driver() const
{
    Q_D(const NetworkInterfaceNm09);
    return_SOLID_CALL(Ifaces::NetworkInterfaceNm09 *, d->backendObject(), QString(), driver());
}

bool Solid::Control::NetworkInterfaceNm09::firmwareMissing() const
{
    Q_D(const NetworkInterfaceNm09);
    return_SOLID_CALL(Ifaces::NetworkInterfaceNm09 *, d->backendObject(), false, firmwareMissing());
}

void Solid::Control::NetworkInterfaceNm09::disconnectInterface() const
{
    Q_D(const NetworkInterfaceNm09);
    SOLID_CALL(Ifaces::NetworkInterfaceNm09 *, d->backendObject(), disconnectInterface());
}

Solid::Control::IPv4ConfigNm09 Solid::Control::NetworkInterfaceNm09::ipV4Config() const
{
    Q_D(const NetworkInterfaceNm09);
    return_SOLID_CALL(Ifaces::NetworkInterfaceNm09 *, d->backendObject(), Solid::Control::IPv4ConfigNm09(), ipV4Config() );
}

bool Solid::Control::NetworkInterfaceNm09::isActive() const
{
    Q_D(const NetworkInterfaceNm09);
    return_SOLID_CALL(Ifaces::NetworkInterfaceNm09 *, d->backendObject(), false, isActive());
}

Solid::Control::NetworkInterfaceNm09::ConnectionState Solid::Control::NetworkInterfaceNm09::connectionState() const
{
    Q_D(const NetworkInterfaceNm09);
    return_SOLID_CALL(Ifaces::NetworkInterfaceNm09 *, d->backendObject(), UnknownState, connectionState());
}

int Solid::Control::NetworkInterfaceNm09::designSpeed() const
{
    Q_D(const NetworkInterfaceNm09);
    return_SOLID_CALL(Ifaces::NetworkInterfaceNm09 *, d->backendObject(), 0, designSpeed());
}

Solid::Control::NetworkInterfaceNm09::Capabilities Solid::Control::NetworkInterfaceNm09::capabilities() const
{
    Q_D(const NetworkInterfaceNm09);
    return_SOLID_CALL(Ifaces::NetworkInterfaceNm09 *, d->backendObject(), Capabilities(), capabilities());
}

void Solid::Control::NetworkInterfaceNm09Private::setBackendObject(QObject *object)
{
    FrontendObjectNm09Private::setBackendObject(object);

    if (object) {
        QObject::connect(object, SIGNAL(connectionStateChanged(int)),
                         parent(), SIGNAL(connectionStateChanged(int)));
        QObject::connect(object, SIGNAL(connectionStateChanged(int,int,int)),
                         parent(), SIGNAL(connectionStateChanged(int,int,int)));
    }
}


#include "networkinterface.moc"
