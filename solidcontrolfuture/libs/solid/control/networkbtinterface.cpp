/*
Copyright 2011 Lamarque Souza <lamarque@gmail.com>

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

#include <KLocale>

#include "networkbtinterface.h"
#include "networkbtinterface_p.h"

#include "frontendobject_p.h"
#include "soliddefs_p.h"
#include "ifaces/networkbtinterface.h"

Solid::Control::BtNetworkInterfaceNm09::BtNetworkInterfaceNm09(QObject *backendObject)
    : ModemNetworkInterfaceNm09(*new BtNetworkInterfaceNm09Private(this), backendObject)
{
    Q_D(BtNetworkInterfaceNm09);
    d->setBackendObject(backendObject);
    makeConnections( backendObject );
}

Solid::Control::BtNetworkInterfaceNm09::BtNetworkInterfaceNm09(const BtNetworkInterfaceNm09 &networkinterface)
    : ModemNetworkInterfaceNm09(*new BtNetworkInterfaceNm09Private(this), networkinterface)
{
    Q_D(BtNetworkInterfaceNm09);
    d->setBackendObject(networkinterface.d_ptr->backendObject());
    makeConnections( networkinterface.d_ptr->backendObject() );
}

Solid::Control::BtNetworkInterfaceNm09::BtNetworkInterfaceNm09(BtNetworkInterfaceNm09Private &dd, QObject *backendObject)
    : ModemNetworkInterfaceNm09(dd, backendObject)
{
    makeConnections( backendObject );
}

Solid::Control::BtNetworkInterfaceNm09::BtNetworkInterfaceNm09(BtNetworkInterfaceNm09Private &dd, const BtNetworkInterfaceNm09 &networkinterface)
    : ModemNetworkInterfaceNm09(dd, networkinterface.d_ptr->backendObject())
{
    makeConnections( networkinterface.d_ptr->backendObject() );
}

Solid::Control::BtNetworkInterfaceNm09::~BtNetworkInterfaceNm09()
{
}

Solid::Control::NetworkInterfaceNm09::Type Solid::Control::BtNetworkInterfaceNm09::type() const
{
    return Bluetooth;
}

void Solid::Control::BtNetworkInterfaceNm09::makeConnections(QObject * source)
{
    connect(source, SIGNAL(networkNameChanged(QString)),
            this, SIGNAL(networkNameChanged(QString)));
}

Solid::Control::BtNetworkInterfaceNm09::Capabilities Solid::Control::BtNetworkInterfaceNm09::btCapabilities() const
{
    Q_D(const BtNetworkInterfaceNm09);
    return_SOLID_CALL(Ifaces::BtNetworkInterfaceNm09 *, d->backendObject(), (Solid::Control::BtNetworkInterfaceNm09::Capabilities)0, btCapabilities());
}

QString Solid::Control::BtNetworkInterfaceNm09::hardwareAddress() const
{
    Q_D(const BtNetworkInterfaceNm09);
    return_SOLID_CALL(Ifaces::BtNetworkInterfaceNm09 *, d->backendObject(), QString(), hardwareAddress());
}

QString Solid::Control::BtNetworkInterfaceNm09::name() const
{
    Q_D(const BtNetworkInterfaceNm09);
    return_SOLID_CALL(Ifaces::BtNetworkInterfaceNm09 *, d->backendObject(), QString(), name());
}

void Solid::Control::BtNetworkInterfaceNm09Private::setBackendObject(QObject *object)
{
    ModemNetworkInterfaceNm09Private::setBackendObject(object);
}

void Solid::Control::BtNetworkInterfaceNm09::_k_destroyed(QObject *object)
{
    Q_UNUSED(object);
}
// vim: sw=4 sts=4 et tw=100
