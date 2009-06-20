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

#include "networkinterfaceconnectableprovider.h"

#include <QHash>

#include "connection.h"
#include "connectionlist.h"
#include "interfaceconnection.h"

#include "connectablelist.h"

class NetworkInterfaceConnectableProviderPrivate
{
public:
    Solid::Control::NetworkInterface * interface;
    ConnectableList * connectableList;
    QHash<Knm::Connection *, Knm::Connectable*> connectables;
};

NetworkInterfaceConnectableProvider::NetworkInterfaceConnectableProvider(ConnectionList * connectionList, ConnectableList * connectableList, Solid::Control::NetworkInterface * interface, QObject * parent)
    : QObject(parent), d_ptr(new NetworkInterfaceConnectableProviderPrivate)
{
    Q_D(NetworkInterfaceConnectableProvider);
    d->interface = interface;
    d->connectableList = connectableList;
    // assess all connections
    foreach (QString uuid, connectionList->connections()) {
        Knm::Connection * connection = connectionList->findConnection(uuid);
        handleAdd(connection);
    }
}

NetworkInterfaceConnectableProvider::~NetworkInterfaceConnectableProvider()
{
    Q_D(NetworkInterfaceConnectableProvider);
    foreach (Knm::Connectable* connectable, d->connectables) {
        d->connectableList->removeConnectable(connectable);
    }
    // all connectables we own are deleted since they are child QObjects
}

bool NetworkInterfaceConnectableProvider::matches(Knm::Connection::Type connType, Solid::Control::NetworkInterface::Type ifaceType)
{
     return ( (connType == Knm::Connection::Wired && ifaceType == Solid::Control::NetworkInterface::Ieee8023)
             || (connType == Knm::Connection::Wireless && ifaceType == Solid::Control::NetworkInterface::Ieee80211)
             || (connType == Knm::Connection::Wireless && ifaceType == Solid::Control::NetworkInterface::Ieee80211)
             || (connType == Knm::Connection::Gsm && ifaceType == Solid::Control::NetworkInterface::Gsm)
             || (connType == Knm::Connection::Cdma && ifaceType == Solid::Control::NetworkInterface::Cdma)
             || (connType == Knm::Connection::Pppoe && ifaceType == Solid::Control::NetworkInterface::Serial)
             );
}

bool hardwareAddressMatches(Knm::Connection * connection, Solid::Control::NetworkInterface * iface)
{
    bool matches = true;
    if (connection->type() == Knm::Connection::Wired) {
        Knm::WiredSetting * wiredSetting = dynamic_cast<Knm::WiredSetting *>(connection->setting(Knm::Setting::Wired));
        Solid::Control::WiredNetworkInterface * wiredIface = dynamic_cast<Solid::Control::WiredNetworkInterface *>(iface);

        if (wiredSetting && wiredIface) {

            // only settings which contain a valid macaddress are interesting
            if (!wiredSetting->macaddress().isEmpty()) {
                matches = (QString(wiredSetting->macaddress()) == wiredIface->hardwareAddress());
            }
        }
    } else if (connection->type() == Knm::Connection::Wireless) {
        Knm::WirelessSetting * wirelessSetting = dynamic_cast<Knm::WirelessSetting *>(connection->setting(Knm::Setting::Wireless));
        Solid::Control::WirelessNetworkInterface * wirelessIface = dynamic_cast<Solid::Control::WirelessNetworkInterface *>(iface);

        if (wirelessSetting && wirelessIface) {

            // only settings which contain a valid macaddress are interesting
            if (!wirelessSetting->macaddress().isEmpty()) {
                matches = (QString(wirelessSetting->macaddress()) == wirelessIface->hardwareAddress());
            }
        }
    }
    return matches;
}
void NetworkInterfaceConnectableProvider::handleAdd(Knm::Connection * addedConnection)
{
    Q_D(NetworkInterfaceConnectableProvider);
    // check type
    if (matches(addedConnection->type(), d->interface->type())) {
        
        if (!d->connectables.contains(addedConnection)) {
            Knm::InterfaceConnection * ifaceConnection = new Knm::InterfaceConnection(addedConnection->uuid(), addedConnection->name(), d->interface->uni(), this);
            d->connectables.insert(addedConnection, ifaceConnection);
            d->connectableList->addConnectable(ifaceConnection);
        }
    }
}

void NetworkInterfaceConnectableProvider::handleUpdate(Knm::Connection * updatedConnection)
{
    Q_D(NetworkInterfaceConnectableProvider);
    if (d->connectables.contains(updatedConnection)) {
        Knm::InterfaceConnection * ifaceConnection = dynamic_cast<Knm::InterfaceConnection *>(d->connectables[updatedConnection]);
        // assume only the name changed here
        ifaceConnection->setConnectionName(updatedConnection->name());
    }
}

void NetworkInterfaceConnectableProvider::handleRemove(Knm::Connection * removedConnection)
{
    Q_D(NetworkInterfaceConnectableProvider);
    if (d->connectables.contains(removedConnection)) {
        Knm::Connectable * connectable = d->connectables[removedConnection];
        d->connectableList->removeConnectable(connectable);
    }
}

// vim: sw=4 sts=4 et tw=100
