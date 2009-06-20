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

#include "networkinterfacemonitor.h"

#include <solid/control/networkinterface.h>
#include <solid/control/networkmanager.h>

#include "connectablelist.h"
#include "connectionlist.h"
#include "networkinterfaceconnectableprovider.h"

class NetworkInterfaceMonitorPrivate
{
public:
    // relates Solid::Control::NetworkInterface identifiers to NetworkInterfaceConnectableProviders
    QHash<QString, NetworkInterfaceConnectableProvider *> providers;
    ConnectionList * connectionList;
    ConnectableList * connectableList;
};

NetworkInterfaceMonitor::NetworkInterfaceMonitor(ConnectionList * connectionList, ConnectableList * connectableList, QObject * parent)
    :QObject(parent), d_ptr(new NetworkInterfaceMonitorPrivate)
{
    Q_D(NetworkInterfaceMonitor);
    d->connectionList = connectionList;
    d->connectableList = connectableList;

    QObject::connect(Solid::Control::NetworkManager::notifier(),
            SIGNAL(networkInterfaceAdded(const QString&)),
            this, SLOT(networkInterfaceAdded(const QString&)));
    QObject::connect(Solid::Control::NetworkManager::notifier(),
            SIGNAL(networkInterfaceRemoved(const QString&)),
            this, SLOT(networkInterfaceRemoved(const QString&)));

    foreach (Solid::Control::NetworkInterface * iface, Solid::Control::NetworkManager::networkInterfaces()) {
        networkInterfaceAdded(iface->uni());
    }
}

NetworkInterfaceMonitor::~NetworkInterfaceMonitor()
{
}

void NetworkInterfaceMonitor::networkInterfaceAdded(const QString & uni)
{
    Q_D(NetworkInterfaceMonitor);
    Solid::Control::NetworkInterface * iface = Solid::Control::NetworkManager::findNetworkInterface(uni);
    if (iface && !d->providers.contains(uni)) {
        d->providers.insert(uni, new NetworkInterfaceConnectableProvider(d->connectionList, d->connectableList, iface, this));
    }
}

void NetworkInterfaceMonitor::networkInterfaceRemoved(const QString & uni)
{
    Q_D(NetworkInterfaceMonitor);
    NetworkInterfaceConnectableProvider * provider = d->providers.take(uni);
    delete provider;
}

// vim: sw=4 sts=4 et tw=100
