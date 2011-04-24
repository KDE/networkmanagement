/*
Copyright 2009 Will Stephenson <wstephenson@kde.org>
Copyright 2010 Lamarque Souza <lamarque@gmail.com>

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

#ifdef COMPILE_MODEM_MANAGER_SUPPORT

#include "gsmnetworkinterfaceactivatableprovider.h"
#include "networkinterfaceactivatableprovider_p.h"

#include <solid/control/modemmanager.h>
#include <solid/control/networkbtinterface.h>

#include <gsminterfaceconnection.h>
#include <gsminterfaceconnectionhelpers.h>

#include "activatablelist.h"
#include "connectionlist.h"
#include "interfaceconnection.h"

class GsmNetworkInterfaceActivatableProviderPrivate : public NetworkInterfaceActivatableProviderPrivate
{
public:
    GsmNetworkInterfaceActivatableProviderPrivate(ConnectionList * theConnectionList, ActivatableList * theActivatableList, Solid::Control::GsmNetworkInterface * theInterface)
        : NetworkInterfaceActivatableProviderPrivate(theConnectionList, theActivatableList, theInterface)
    { }

    Solid::Control::GsmNetworkInterface * gsmInterface() const
    {
        return qobject_cast<Solid::Control::GsmNetworkInterface*>(interface);
    }
};

GsmNetworkInterfaceActivatableProvider::GsmNetworkInterfaceActivatableProvider(ConnectionList * connectionList, ActivatableList * activatableList, Solid::Control::GsmNetworkInterface * interface, QObject * parent)
: NetworkInterfaceActivatableProvider(*new GsmNetworkInterfaceActivatableProviderPrivate(connectionList, activatableList, interface), parent)
{
}

GsmNetworkInterfaceActivatableProvider::~GsmNetworkInterfaceActivatableProvider()
{

}

void GsmNetworkInterfaceActivatableProvider::handleAdd(Knm::Connection * addedConnection)
{
    Q_D(GsmNetworkInterfaceActivatableProvider);

    kDebug() << addedConnection->uuid();
    if (!d->activatables.contains(addedConnection->uuid())) {
        if (hardwareAddressMatches(addedConnection, d->interface)) {
            Solid::Control::NetworkInterface::Type type = d->interface->type();
            Solid::Control::BtNetworkInterface *btIface = qobject_cast<Solid::Control::BtNetworkInterface *>(d->interface);
            if (btIface) {
                kDebug() << btIface->uni() << " is actually a gsm bluetooth device";
                type = btIface->type();
            }
            if (matches(addedConnection->type(), type)) {
                Knm::GsmInterfaceConnection * ifaceConnection =
                    Knm::GsmInterfaceConnectionHelpers::buildGsmInterfaceConnection(
                            d->gsmInterface(), addedConnection, d->interface->uni(), this);

                d->activatables.insert(addedConnection->uuid(), ifaceConnection);
                d->activatableList->addActivatable(ifaceConnection);
            } else {
                kDebug() << "connection type mismatch: " << addedConnection->type() << type;
            }
        } else {
            kDebug() << "hardware address mismatch!";
        }
    }
    maintainActivatableForUnconfigured();
}

#endif
// vim: sw=4 sts=4 et tw=100
