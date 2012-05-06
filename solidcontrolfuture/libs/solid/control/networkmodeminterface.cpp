/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>

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
#include <KDebug>

#include "networkmodeminterface.h"
#include "networkmodeminterface_p.h"

#include "frontendobject_p.h"
#include "soliddefs_p.h"
#include "ifaces/networkmodeminterface.h"
#include "ifaces/modemgsmnetworkinterface.h"

Solid::Control::ModemNetworkInterfaceNm09::ModemNetworkInterfaceNm09(QObject *backendObject)
    : NetworkInterfaceNm09(*new ModemNetworkInterfaceNm09Private(this), backendObject)
{
    Q_D(ModemNetworkInterfaceNm09);
    d->setBackendObject(backendObject);
    makeConnections( backendObject );
}

Solid::Control::ModemNetworkInterfaceNm09::ModemNetworkInterfaceNm09(const ModemNetworkInterfaceNm09 &networkinterface)
    : NetworkInterfaceNm09(*new ModemNetworkInterfaceNm09Private(this), networkinterface)
{
    Q_D(ModemNetworkInterfaceNm09);
    d->setBackendObject(networkinterface.d_ptr->backendObject());
    makeConnections( networkinterface.d_ptr->backendObject() );
}

Solid::Control::ModemNetworkInterfaceNm09::ModemNetworkInterfaceNm09(ModemNetworkInterfaceNm09Private &dd, QObject *backendObject)
    : NetworkInterfaceNm09(dd, backendObject)
{
    makeConnections( backendObject );
}

Solid::Control::ModemNetworkInterfaceNm09::ModemNetworkInterfaceNm09(ModemNetworkInterfaceNm09Private &dd, const ModemNetworkInterfaceNm09 &networkinterface)
    : NetworkInterfaceNm09(dd, networkinterface.d_ptr->backendObject())
{
    makeConnections( networkinterface.d_ptr->backendObject() );
}

Solid::Control::ModemNetworkInterfaceNm09::~ModemNetworkInterfaceNm09()
{
}

Solid::Control::NetworkInterfaceNm09::Type Solid::Control::ModemNetworkInterfaceNm09::type() const
{
    return Modem;
}

Solid::Control::ModemNetworkInterfaceNm09::ModemCapability Solid::Control::ModemNetworkInterfaceNm09::subType() const
{
    ModemCapabilities modemCaps = currentCapabilities();

    if (modemCaps & Solid::Control::ModemNetworkInterfaceNm09::Lte) {
        return Lte;
    } else if (modemCaps & Solid::Control::ModemNetworkInterfaceNm09::CdmaEvdo) {
        return CdmaEvdo;
    } else if (modemCaps & Solid::Control::ModemNetworkInterfaceNm09::GsmUmts) {
        return GsmUmts;
    } else if (modemCaps & Solid::Control::ModemNetworkInterfaceNm09::Pots) {
        return Pots;
    }
    kDebug() << "Modem subtype not supported";
    return None;
}

Solid::Control::ModemNetworkInterfaceNm09::ModemCapabilities Solid::Control::ModemNetworkInterfaceNm09::modemCapabilities() const
{
    Q_D(const ModemNetworkInterfaceNm09);
    return_SOLID_CALL(Ifaces::ModemNetworkInterfaceNm09 *, d->backendObject(), ModemCapabilities(), modemCapabilities());
}

Solid::Control::ModemNetworkInterfaceNm09::ModemCapabilities Solid::Control::ModemNetworkInterfaceNm09::currentCapabilities() const
{
    Q_D(const ModemNetworkInterfaceNm09);
    return_SOLID_CALL(Ifaces::ModemNetworkInterfaceNm09 *, d->backendObject(), ModemCapabilities(), currentCapabilities());
}


Solid::Control::ModemGsmNetworkInterface * Solid::Control::ModemNetworkInterfaceNm09::getModemNetworkIface()
{
    Q_D(const ModemNetworkInterfaceNm09);
    Ifaces::ModemNetworkInterfaceNm09 *t = qobject_cast<Ifaces::ModemNetworkInterfaceNm09 *>(d->backendObject());
    if (t != 0)
    {
        return t->getModemNetworkIface();
    }
    return 0;
}

Solid::Control::ModemGsmCardInterface * Solid::Control::ModemNetworkInterfaceNm09::getModemCardIface()
{
    Q_D(const ModemNetworkInterfaceNm09);
    Ifaces::ModemNetworkInterfaceNm09 *t = qobject_cast<Ifaces::ModemNetworkInterfaceNm09 *>(d->backendObject());
    if (t != 0)
    {
        return t->getModemCardIface();
    }
    return 0;
}

void Solid::Control::ModemNetworkInterfaceNm09::setModemCardIface(Solid::Control::ModemGsmCardInterface * iface)
{
    Q_D(const ModemNetworkInterfaceNm09);
    Ifaces::ModemNetworkInterfaceNm09 *t = qobject_cast<Ifaces::ModemNetworkInterfaceNm09 *>(d->backendObject());
    if (t != 0)
    {
        t->setModemCardIface(iface);
    }
}

void Solid::Control::ModemNetworkInterfaceNm09::setModemNetworkIface(Solid::Control::ModemGsmNetworkInterface * iface)
{
    Q_D(const ModemNetworkInterfaceNm09);
    Ifaces::ModemNetworkInterfaceNm09 *t = qobject_cast<Ifaces::ModemNetworkInterfaceNm09 *>(d->backendObject());
    if (t != 0)
    {
        t->setModemNetworkIface(iface);
    }
}

void Solid::Control::ModemNetworkInterfaceNm09::makeConnections(QObject * source)
{
}

void Solid::Control::ModemNetworkInterfaceNm09Private::setBackendObject(QObject *object)
{
    NetworkInterfaceNm09Private::setBackendObject(object);
    if (object) {
        QObject::connect(object, SIGNAL(modemCapabilitiesChanged(Solid::Control::ModemNetworkInterfaceNm09::ModemCapabilities)),
                         parent(), SIGNAL(modemCapabilitiesChanged(Solid::Control::ModemNetworkInterfaceNm09::ModemCapabilities)));
        QObject::connect(object, SIGNAL(currentCapabilitiesChanged(Solid::Control::ModemNetworkInterfaceNm09::ModemCapabilities)),
                         parent(), SIGNAL(currentCapabilitiesChanged(Solid::Control::ModemNetworkInterfaceNm09::ModemCapabilities)));
    }
}

void Solid::Control::ModemNetworkInterfaceNm09::_k_destroyed(QObject *object)
{
    Q_UNUSED(object);
}
// vim: sw=4 sts=4 et tw=100
