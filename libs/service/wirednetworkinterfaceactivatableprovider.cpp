/*
Copyright 2009 Will Stephenson <wstephenson@kde.org>

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

#include "wirednetworkinterfaceactivatableprovider.h"
#include "networkinterfaceactivatableprovider_p.h"

#include <QHash>
#include <solid/control/wirednetworkinterface.h>

#include "activatable.h"
#include "activatablelist.h"
#include "connectionlist.h"

class WiredNetworkInterfaceActivatableProviderPrivate : public NetworkInterfaceActivatableProviderPrivate
{
public:
    WiredNetworkInterfaceActivatableProviderPrivate(ConnectionList * theConnectionList, ActivatableList * theActivatableList, Solid::Control::WiredNetworkInterface * theInterface)
        : NetworkInterfaceActivatableProviderPrivate(theConnectionList, theActivatableList, theInterface)
    { }

    Solid::Control::WiredNetworkInterface * wiredInterface()
    {
        return qobject_cast<Solid::Control::WiredNetworkInterface*>(interface);
    }
};

WiredNetworkInterfaceActivatableProvider::WiredNetworkInterfaceActivatableProvider(ConnectionList * connectionList, ActivatableList * activatableList, Solid::Control::WiredNetworkInterface * interface, QObject * parent)
: NetworkInterfaceActivatableProvider(*new WiredNetworkInterfaceActivatableProviderPrivate( connectionList, activatableList, interface), parent)
{
    connect(interface, SIGNAL(carrierChanged(bool)), this, SLOT(handleCarrierChange(bool)));
}

WiredNetworkInterfaceActivatableProvider::~WiredNetworkInterfaceActivatableProvider()
{

}

void WiredNetworkInterfaceActivatableProvider::handleAdd(Knm::Connection * addedConnection)
{
    Q_D(WiredNetworkInterfaceActivatableProvider);
    if (d->wiredInterface()->carrier()) {
        NetworkInterfaceActivatableProvider::handleAdd(addedConnection);
    }
}

void WiredNetworkInterfaceActivatableProvider::handleCarrierChange(bool carrier)
{
    Q_D(WiredNetworkInterfaceActivatableProvider);
    if (carrier) {
        foreach (QString uuid, d->connectionList->connections()) {
            Knm::Connection * connection = d->connectionList->findConnection(uuid);
            handleAdd(connection);
        }
    } else {
        foreach (Knm::Activatable * activatable, d->activatables) {
            d->activatableList->removeActivatable(activatable);
            delete activatable;
        }
        d->activatables.clear();
    }
}


// vim: sw=4 sts=4 et tw=100
