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

#include "networkinterfaceactivatableprovider.h"

#include <QHash>

#include "connection.h"
#include "connectionlist.h"
#include "interfaceconnection.h"

#include "activatablelist.h"

#include "networkinterfaceactivatableprovider_p.h"

NetworkInterfaceActivatableProviderPrivate::~NetworkInterfaceActivatableProviderPrivate()
{

}

NetworkInterfaceActivatableProvider::NetworkInterfaceActivatableProvider(ConnectionList * connectionList, ActivatableList * activatableList, Solid::Control::NetworkInterface * interface, QObject * parent)
    : QObject(parent), d_ptr(new NetworkInterfaceActivatableProviderPrivate)
{
    Q_D(NetworkInterfaceActivatableProvider);
    d->interface = interface;
    d->activatableList = activatableList;
    d->connectionList = connectionList;
}

NetworkInterfaceActivatableProvider::NetworkInterfaceActivatableProvider(NetworkInterfaceActivatableProviderPrivate &dd, ConnectionList * connectionList, ActivatableList * activatableList, Solid::Control::NetworkInterface * interface, QObject * parent)
    : QObject(parent), d_ptr(&dd)
{
    Q_D(NetworkInterfaceActivatableProvider);
    d->interface = interface;
    d->activatableList = activatableList;
    d->connectionList = connectionList;
    connect(d->activatableList, SIGNAL(destroyed()), this, SLOT(activatableListDestroyed()));
}

void NetworkInterfaceActivatableProvider::init()
{
    Q_D(NetworkInterfaceActivatableProvider);
    // assess all connections
    foreach (QString uuid, d->connectionList->connections()) {
        Knm::Connection * connection = d->connectionList->findConnection(uuid);
        handleAdd(connection);
    }
}

NetworkInterfaceActivatableProvider::~NetworkInterfaceActivatableProvider()
{
    Q_D(NetworkInterfaceActivatableProvider);
    if (d->activatableList) {
        foreach (Knm::Activatable* activatable, d->activatables) {
            d->activatableList->removeActivatable(activatable);
        }
    }
    // all activatables we own are deleted since they are child QObjects
}

bool NetworkInterfaceActivatableProvider::matches(Knm::Connection::Type connType, Solid::Control::NetworkInterface::Type ifaceType)
{
     return ( (connType == Knm::Connection::Wired && ifaceType == Solid::Control::NetworkInterface::Ieee8023)
             || (connType == Knm::Connection::Wireless && ifaceType == Solid::Control::NetworkInterface::Ieee80211)
             || (connType == Knm::Connection::Wireless && ifaceType == Solid::Control::NetworkInterface::Ieee80211)
             || (connType == Knm::Connection::Gsm && ifaceType == Solid::Control::NetworkInterface::Gsm)
             || (connType == Knm::Connection::Cdma && ifaceType == Solid::Control::NetworkInterface::Cdma)
             || (connType == Knm::Connection::Pppoe && ifaceType == Solid::Control::NetworkInterface::Serial)
             );
}

bool NetworkInterfaceActivatableProvider::hardwareAddressMatches(Knm::Connection * connection, Solid::Control::NetworkInterface * iface)
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

void NetworkInterfaceActivatableProvider::handleAdd(Knm::Connection * addedConnection)
{
    Q_D(NetworkInterfaceActivatableProvider);
    // check type
    if (!d->activatables.contains(addedConnection->uuid())) {
        if (hardwareAddressMatches(addedConnection, d->interface)) {
            if (matches(addedConnection->type(), d->interface->type())) {
                Knm::InterfaceConnection * ifaceConnection = new Knm::InterfaceConnection(addedConnection->uuid(), addedConnection->name(), d->interface->uni(), this);
                d->activatables.insert(addedConnection->uuid(), ifaceConnection);
                d->activatableList->addActivatable(ifaceConnection);
            }
        }
    }
}

void NetworkInterfaceActivatableProvider::handleUpdate(Knm::Connection * updatedConnection)
{
    Q_D(NetworkInterfaceActivatableProvider);
    if (d->activatables.contains(updatedConnection->uuid())) {
        Knm::InterfaceConnection * ifaceConnection = dynamic_cast<Knm::InterfaceConnection *>(d->activatables[updatedConnection->uuid()]);
        // assume only the name changed here
        ifaceConnection->setConnectionName(updatedConnection->name());
    }
}

void NetworkInterfaceActivatableProvider::handleRemove(Knm::Connection * removedConnection)
{
    Q_D(NetworkInterfaceActivatableProvider);
    if (d->activatables.contains(removedConnection->uuid())) {
        Knm::Activatable * activatable = d->activatables[removedConnection->uuid()];
        d->activatableList->removeActivatable(activatable);
        delete activatable;
    }
}

void NetworkInterfaceActivatableProvider::activatableListDestroyed()
{
    Q_D(NetworkInterfaceActivatableProvider);
    d->activatableList = 0;
}
// vim: sw=4 sts=4 et tw=100
