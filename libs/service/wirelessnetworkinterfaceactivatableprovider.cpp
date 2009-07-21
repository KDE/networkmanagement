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

#include "wirelessnetworkinterfaceactivatableprovider.h"
#include "networkinterfaceactivatableprovider_p.h"

#include <QHash>

#include <solid/control/networkmanager.h>

#include <wirelessinterfaceconnection.h>
#include <wirelessnetwork.h>
#include <wirelessnetworkinterfaceenvironment.h>

#include "activatablelist.h"
#include "connectionlist.h"
#include "unconfiguredinterface.h"

class WirelessNetworkInterfaceActivatableProviderPrivate : public NetworkInterfaceActivatableProviderPrivate
{
public:
    WirelessNetworkInterfaceActivatableProviderPrivate(ConnectionList * theConnectionList, ActivatableList * theActivatableList, Solid::Control::WirelessNetworkInterface * theInterface)
        : NetworkInterfaceActivatableProviderPrivate(theConnectionList, theActivatableList, theInterface)
    { }

    Solid::Control::WirelessNetworkInterfaceEnvironment * environment;

    // essid to activatable
    QHash<QString, Knm::Activatable *> wirelessActivatables;

    Solid::Control::WirelessNetworkInterface * wirelessInterface() const
    {
        return qobject_cast<Solid::Control::WirelessNetworkInterface*>(interface);
    }
};

WirelessNetworkInterfaceActivatableProvider::WirelessNetworkInterfaceActivatableProvider(ConnectionList * connectionList, ActivatableList * activatableList, Solid::Control::WirelessNetworkInterface * interface, QObject * parent)
: NetworkInterfaceActivatableProvider(*new WirelessNetworkInterfaceActivatableProviderPrivate(connectionList, activatableList, interface), parent)
{
    Q_D(WirelessNetworkInterfaceActivatableProvider);
    d->environment = new Solid::Control::WirelessNetworkInterfaceEnvironment(interface);
    QObject::connect(d->environment, SIGNAL(networkAppeared(const QString &)), this, SLOT(networkAppeared(const QString&)));
    QObject::connect(d->environment, SIGNAL(networkDisappeared(const QString &)), this, SLOT(networkDisappeared(const QString&)));
    // try to create a connectable for each wireless network we can see
    // this is slightly inefficient because the NetworkInterfaceActivatableProvider ctor
    // already ran and created WirelessInterfaceConnections as needed, but a hash lookup in
    // networkAppeared prevents duplicate WirelessInterfaceConnections, so this
    // iteration creates any WirelessNetworks needed for unconfigured networks.
    foreach (QString network, d->environment->networks()) {
        networkAppeared(network);
    }
}

WirelessNetworkInterfaceActivatableProvider::~WirelessNetworkInterfaceActivatableProvider()
{

}

void WirelessNetworkInterfaceActivatableProvider::handleAdd(Knm::Connection * addedConnection)
{
    Q_D(WirelessNetworkInterfaceActivatableProvider);
    // check type
    if (!d->activatables.contains(addedConnection->uuid())) {
        if (matches(addedConnection->type(), d->interface->type())) {
            if (hardwareAddressMatches(addedConnection, d->interface)) {
                // check that the network is present
                // SSID match is sufficient, matching on SeenBSSIDs would prevent connecting to a
                // known network in a different location
                Knm::WirelessSetting * wirelessSetting = dynamic_cast<Knm::WirelessSetting *>(addedConnection->setting(Knm::Setting::Wireless));
                if (wirelessSetting) {
                    // show connections where the network is present OR adhoc connections
                    if (wirelessSetting->mode() == Knm::WirelessSetting::EnumMode::adhoc || d->environment->networks().contains(wirelessSetting->ssid())) {
                        //kDebug() << d->environment->networks();
                        kDebug() << wirelessSetting->ssid() <<  addedConnection->uuid() << addedConnection->name() << d->interface->uni();

                        // get the info on the network
                        Solid::Control::WirelessNetwork * network = d->environment->findNetwork(wirelessSetting->ssid());
                        int strength = -1;
                        Solid::Control::AccessPoint::Capabilities caps = 0;
                        Solid::Control::AccessPoint::WpaFlags wpaFlags = 0;
                        Solid::Control::AccessPoint::WpaFlags rsnFlags = 0;

                        if (network) {
                            strength = network->signalStrength();
                            Solid::Control::AccessPoint * ap = d->wirelessInterface()->findAccessPoint(network->referenceAccessPoint());
                            if (ap) {
                                caps = ap->capabilities();
                                wpaFlags = ap->wpaFlags();
                                rsnFlags = ap->rsnFlags();
                            }

                        }

                        Knm::WirelessInterfaceConnection * ifaceConnection = new Knm::WirelessInterfaceConnection(
                                wirelessSetting->ssid(), strength, caps, wpaFlags, rsnFlags, addedConnection->uuid(), addedConnection->name(),
                                d->interface->uni(), this);

                        if (network) {
                            connect(network, SIGNAL(signalStrengthChanged(int)), ifaceConnection, SLOT(setStrength(int)));
                        }

                        // remove any WirelessNetwork created previously
                        Knm::WirelessNetwork * wni = qobject_cast<Knm::WirelessNetwork*>(d->wirelessActivatables.take(wirelessSetting->ssid()));
                        if (wni) {
                            d->activatableList->removeActivatable(wni);
                        }
                        delete wni;

                        // register the InterfaceConnection
                        d->activatables.insert(addedConnection->uuid(), ifaceConnection);
                        d->wirelessActivatables.insert(wirelessSetting->ssid(), ifaceConnection);
                        d->activatableList->addActivatable(ifaceConnection);
                    }
                }
            }
        }
        maintainActivatableForUnconfigured();
    }
}

void WirelessNetworkInterfaceActivatableProvider::handleRemove(Knm::Connection * removedConnection)
{
    Q_D(WirelessNetworkInterfaceActivatableProvider);
    // check type
    // if we know this connection, remove any references to its WirelessInterfaceConnection
    if (d->activatables.contains(removedConnection->uuid())) {
        Knm::WirelessSetting * wirelessSetting = dynamic_cast<Knm::WirelessSetting *>(removedConnection->setting(Knm::Setting::Wireless));
        if (wirelessSetting) {
            // remove it any reference we hold
            d->wirelessActivatables.take(wirelessSetting->ssid());
            // if the network is present, create a WirelessNetwork for it
            if (d->environment->networks().contains(wirelessSetting->ssid())) {
                networkAppeared(wirelessSetting->ssid());
            }
        }
        // let our superclass update its state too.  This includes deleting any
        // WirelessInterfaceConnection created by our handleAdd() reimpl
        NetworkInterfaceActivatableProvider::handleRemove(removedConnection);
    }
}

void WirelessNetworkInterfaceActivatableProvider::networkAppeared(const QString & ssid)
{
    Q_D(WirelessNetworkInterfaceActivatableProvider);
    // save time by checking if we already know this network
    if (!d->wirelessActivatables.contains(ssid)) {
        // try all connections to see if they are for this network
        foreach (QString uuid, d->connectionList->connections()) {
            Knm::Connection * connection = d->connectionList->findConnection(uuid);
            // it is safe to call this multiple times with a connection that is already known
            handleAdd(connection);
        }
        // if we still don't have an activatable for this network, we have no connection
        bool hasConnection = false;
        if (d->wirelessActivatables.contains(ssid)) {
            Knm::WirelessInterfaceConnection * wirelessInterfaceConnection
                = qobject_cast<Knm::WirelessInterfaceConnection*>(d->wirelessActivatables[ssid]);
            if (wirelessInterfaceConnection) {
                hasConnection = true;
            }
        }
        if (!hasConnection) {
            // create a wirelessnetwork, register it, tell the list
            // get the info on the network
            Solid::Control::WirelessNetwork * network = d->environment->findNetwork(ssid);
            int strength = 0;
            Solid::Control::AccessPoint::Capabilities caps = 0;
            Solid::Control::AccessPoint::WpaFlags wpaFlags = 0;
            Solid::Control::AccessPoint::WpaFlags rsnFlags = 0;
            if (network) {
                strength = network->signalStrength();
                Solid::Control::AccessPoint * ap = d->wirelessInterface()->findAccessPoint(network->referenceAccessPoint());
                if (ap) {
                    caps = ap->capabilities();
                    wpaFlags = ap->wpaFlags();
                    rsnFlags = ap->rsnFlags();
                    Knm::WirelessNetwork * wirelessNetworkItem = new Knm::WirelessNetwork(ssid, strength, caps, wpaFlags, rsnFlags, d->interface->uni(), this);
                    connect(network, SIGNAL(signalStrengthChanged(int)), wirelessNetworkItem, SLOT(setStrength(int)));
                    d->wirelessActivatables.insert(ssid, wirelessNetworkItem);
                    d->activatableList->addActivatable(wirelessNetworkItem);

                }
            }
        }
    }
}

void WirelessNetworkInterfaceActivatableProvider::networkDisappeared(const QString & ssid)
{
    Q_D(WirelessNetworkInterfaceActivatableProvider);
    if (d->wirelessActivatables.contains(ssid)) {
        Knm::Activatable * activatable = d->wirelessActivatables[ssid];
        Knm::WirelessInterfaceConnection * wirelessInterfaceConnection = qobject_cast<Knm::WirelessInterfaceConnection*>(activatable);
        if (wirelessInterfaceConnection) {
            d->activatables.take(wirelessInterfaceConnection->connectionUuid());
        }
        d->wirelessActivatables.take(ssid);
        d->activatableList->removeActivatable(activatable);
        delete activatable;
    }
}

bool WirelessNetworkInterfaceActivatableProvider::needsActivatableForUnconfigured() const
{
    bool needed =  Solid::Control::NetworkManager::isWirelessEnabled()
        && Solid::Control::NetworkManager::isWirelessHardwareEnabled();
    return needed;
}

// vim: sw=4 sts=4 et tw=100
