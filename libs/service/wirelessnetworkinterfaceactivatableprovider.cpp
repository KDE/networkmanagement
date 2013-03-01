/*
Copyright 2009 Will Stephenson <wstephenson@kde.org>
Copyright 2011-2013 Lamarque V. Souza <lamarque@kde.org>

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

#include <QMultiHash>

#include <solid/control/networkmanager.h>

#include <hiddenwirelessinterfaceconnection.h>
#include <interfaceconnection.h>
#include <wirelessinterfaceconnection.h>
#include <wirelessinterfaceconnectionhelpers.h>
#include <wirelessnetwork.h>
#include <wirelessnetworkinterfaceenvironment.h>

#include "activatablelist.h"
#include "connectionlist.h"
#include "unconfiguredinterface.h"

/** WICs are added on connection add if visible, removed/updated etc
 *           added on network appeared if connection exists, removed etc
 * HiddenWICs are always added on connection add if no other exists
 *
 * WNIs are added on network appeared if no connection exists
 */
class WirelessNetworkInterfaceActivatableProviderPrivate : public NetworkInterfaceActivatableProviderPrivate
{
public:
    WirelessNetworkInterfaceActivatableProviderPrivate(ConnectionList * theConnectionList, ActivatableList * theActivatableList, Solid::Control::WirelessNetworkInterfaceNm09 * theInterface)
        : NetworkInterfaceActivatableProviderPrivate(theConnectionList, theActivatableList, theInterface)
    { }

    Solid::Control::WirelessNetworkInterfaceEnvironment * environment;

    // essid to WirelessNetwork - only 1 exists per network
    QHash<QString, Knm::WirelessNetwork *> wirelessNetworks;

    Solid::Control::WirelessNetworkInterfaceNm09 * wirelessInterface() const
    {
        return qobject_cast<Solid::Control::WirelessNetworkInterfaceNm09*>(interface);
    }
};

WirelessNetworkInterfaceActivatableProvider::WirelessNetworkInterfaceActivatableProvider(ConnectionList * connectionList, ActivatableList * activatableList, Solid::Control::WirelessNetworkInterfaceNm09 * interface, QObject * parent)
: NetworkInterfaceActivatableProvider(*new WirelessNetworkInterfaceActivatableProviderPrivate(connectionList, activatableList, interface), parent)
{
    Q_D(WirelessNetworkInterfaceActivatableProvider);
    d->environment = new Solid::Control::WirelessNetworkInterfaceEnvironment(interface);

    QObject::connect(d->environment, SIGNAL(networkAppeared(QString)), this, SLOT(networkAppeared(QString)));
    QObject::connect(d->environment, SIGNAL(networkDisappeared(QString)), this, SLOT(networkDisappeared(QString)));
    connect(Solid::Control::NetworkManagerNm09::notifier(), SIGNAL(wirelessHardwareEnabledChanged(bool)),
                this, SLOT(wirelessEnabledChanged(bool)));
    connect(Solid::Control::NetworkManagerNm09::notifier(), SIGNAL(wirelessEnabledChanged(bool)),
                this, SLOT(wirelessEnabledChanged(bool)));
    // try to create a connectable for each wireless network we can see
    // this is slightly inefficient because the NetworkInterfaceActivatableProvider ctor
    // already ran and created WirelessInterfaceConnections as needed, but a hash lookup in
    // networkAppeared prevents duplicate WirelessInterfaceConnections, so this
    // iteration creates any WirelessNetworks needed for unconfigured networks.
    foreach (const QString &network, d->environment->networks()) {
        networkAppeared(network);
    }
}

WirelessNetworkInterfaceActivatableProvider::~WirelessNetworkInterfaceActivatableProvider()
{
    Q_D(WirelessNetworkInterfaceActivatableProvider);

    // remove WICs
    QMultiHash<QString, Knm::InterfaceConnection*>::iterator i = d->activatables.begin();
    while (i != d->activatables.end()) {
        Knm::InterfaceConnection * ic = i.value();

        if (ic->activatableType() == Knm::Activatable::WirelessInterfaceConnection ) {
            Knm::WirelessInterfaceConnection * wic = static_cast<Knm::WirelessInterfaceConnection*>(ic);

            if (d->activatableList) {
                d->activatableList->removeActivatable(ic);
            }

            i = d->activatables.erase(i);
            delete wic;
        } else {
            ++i;
        }
    }
    if (d->activatableList) {
        // remove all WirelessNetwork
        QHash<QString, Knm::WirelessNetwork *>::iterator w = d->wirelessNetworks.begin();
        while (w != d->wirelessNetworks.end()) {
            Knm::WirelessNetwork * wni = w.value();
            d->activatableList->removeActivatable(wni);
            delete wni;
            ++w;
        }
    }
}

void WirelessNetworkInterfaceActivatableProvider::handleAdd(Knm::Connection * addedConnection)
{
    Q_D(WirelessNetworkInterfaceActivatableProvider);

    // check that WICs for this connection exist
    bool ourWicFound = false;
    bool ourHiddenWicFound = false;

    Knm::WirelessSetting * wirelessSetting = dynamic_cast<Knm::WirelessSetting *>(addedConnection->setting(Knm::Setting::Wireless));

    if (wirelessSetting) {

        if (matches(addedConnection->type(), d->interface->type())) {
            if (hardwareAddressMatches(addedConnection, d->interface)) {

                foreach (Knm::InterfaceConnection * ic, d->activatables) {
                    if (ic->activatableType() == Knm::Activatable::WirelessInterfaceConnection && ic->connectionUuid() == addedConnection->uuid()) {
                        ourWicFound = true;
                        ourHiddenWicFound = true;
                    }
                    else if (ic->activatableType() == Knm::Activatable::HiddenWirelessInterfaceConnection && ic->connectionUuid() == addedConnection->uuid()) {
                        ourHiddenWicFound = true;
                    }
                }

                // create WirelessInterfaceConnections only where the network is present
                if (!ourWicFound && (d->environment->networks().contains(wirelessSetting->ssid()) || addedConnection->isShared() || wirelessSetting->mode() == Knm::WirelessSetting::EnumMode::adhoc || wirelessSetting->mode() == Knm::WirelessSetting::EnumMode::apMode)) {
                    kDebug() << "Adding WIC:" << wirelessSetting->ssid() <<  addedConnection->uuid() << addedConnection->name() << d->interface->uni() << wirelessSetting->channel();

                    Knm::WirelessInterfaceConnection * ifaceConnection =
                        Knm::WirelessInterfaceConnectionHelpers::buildWirelessInterfaceConnection(
                                d->wirelessInterface(), addedConnection, d->interface->uni(), this);

                    Solid::Control::WirelessNetwork * network = d->environment->findNetwork(wirelessSetting->ssid());

                    if (network) {
                        connect(network, SIGNAL(signalStrengthChanged(int)), ifaceConnection, SLOT(setStrength(int)));
                    }
                    else if (wirelessSetting->mode() == Knm::WirelessSetting::EnumMode::adhoc ||
                             wirelessSetting->mode() == Knm::WirelessSetting::EnumMode::apMode) {
                        ifaceConnection->setStrength(-1);
                        ourHiddenWicFound = true;
                    }

                    // remove any WirelessNetwork created previously
                    Knm::WirelessNetwork * knmNetwork = d->wirelessNetworks.take(wirelessSetting->ssid());
                    if (knmNetwork) {
                        d->activatableList->removeActivatable(knmNetwork);
                        delete knmNetwork;
                    }

                    // register the InterfaceConnection
                    d->activatables.insert(addedConnection->uuid().toString(), ifaceConnection);
                    d->activatableList->addActivatable(ifaceConnection);
                }
                else if (!ourHiddenWicFound) {
                    // create a HiddenWirelessInterfaceConnection for this connection as well
                    // this allows adhoc and hidden wireless networks' connections to be activated
                    // see HiddenWIC's docu for why this is needed
                    kDebug() << "Adding HIDDENWIC:" << wirelessSetting->ssid() <<  addedConnection->uuid() << addedConnection->name() << d->interface->uni();
                    Knm::HiddenWirelessInterfaceConnection * hiddenWic =
                        Knm::WirelessInterfaceConnectionHelpers::buildHiddenWirelessInterfaceConnection(
                                d->wirelessInterface(), addedConnection, d->interface->uni(), this);
                    d->activatables.insert(addedConnection->uuid().toString(), hiddenWic);
                    d->activatableList->addActivatable(hiddenWic);
                }
            }
        }
        maintainActivatableForUnconfigured();
    }
}

void WirelessNetworkInterfaceActivatableProvider::handleRemove(Knm::Connection * removedConnection)
{
    Q_D(WirelessNetworkInterfaceActivatableProvider);
    // let our superclass update its state too.  This includes deleting any
    // WirelessInterfaceConnections created by our handleAdd() reimpl
    NetworkInterfaceActivatableProvider::handleRemove(removedConnection);

    // try to create a WirelessNetwork - this will do nothing if other connections for this network
    // still exist
    Knm::WirelessSetting * wirelessSetting = dynamic_cast<Knm::WirelessSetting *>(removedConnection->setting(Knm::Setting::Wireless));
    // d->interface may be null if NM has just stopped and this provider has not been unregistered yet.
    // d->environment is a child of d->interface, so it is an invalid pointer in that situation.
    if (wirelessSetting && d->interface) {
        if (d->environment->networks().contains(wirelessSetting->ssid())) {
            networkAppeared(wirelessSetting->ssid());
        }
    }
}

void WirelessNetworkInterfaceActivatableProvider::networkAppeared(const QString & ssid)
{
    kDebug() << ssid;
    Q_D(WirelessNetworkInterfaceActivatableProvider);
    // try all connections to see if they are for this network
    foreach (const QString &uuid, d->connectionList->connections()) {
        Knm::Connection * connection = d->connectionList->findConnection(uuid);
        // it is safe to call this multiple times with a connection that is already known
        handleAdd(connection);
    }
    // if we still don't have an activatable for this network, we have no connection
    bool hasConnection = false;
    foreach (Knm::InterfaceConnection * ic, d->activatables) {
        if (ic->activatableType() == Knm::Activatable::WirelessInterfaceConnection || ic->activatableType() == Knm::Activatable::HiddenWirelessInterfaceConnection) {
            Knm::WirelessInterfaceConnection * wic = static_cast<Knm::WirelessInterfaceConnection*>(ic);
            if (wic->ssid() == ssid) {
                hasConnection = true;
                break;
            }
        }
    }

    if (!hasConnection) {
        // create a wirelessnetwork, register it, tell the list
        // get the info on the network
        Solid::Control::WirelessNetwork * network = d->environment->findNetwork(ssid);
        int strength = 0;
        Solid::Control::AccessPointNm09::Capabilities caps = 0;
        Solid::Control::AccessPointNm09::WpaFlags wpaFlags = 0;
        Solid::Control::AccessPointNm09::WpaFlags rsnFlags = 0;
        if (network) {
            strength = network->signalStrength();
            Solid::Control::AccessPointNm09 * ap = d->wirelessInterface()->findAccessPoint(network->referenceAccessPoint());
            if (ap) {
                caps = ap->capabilities();
                wpaFlags = ap->wpaFlags();
                rsnFlags = ap->rsnFlags();
                Knm::WirelessNetwork * wirelessNetworkItem = new Knm::WirelessNetwork(ssid, strength, d->wirelessInterface()->wirelessCapabilities(), caps, wpaFlags, rsnFlags, ap->mode(), d->interface->uni(), this);
                connect(network, SIGNAL(signalStrengthChanged(int)), wirelessNetworkItem, SLOT(setStrength(int)));
                d->wirelessNetworks.insert(ssid, wirelessNetworkItem);
                d->activatableList->addActivatable(wirelessNetworkItem);
            }
        }
    }
}

void WirelessNetworkInterfaceActivatableProvider::networkDisappeared(const QString & ssid)
{
    Q_D(WirelessNetworkInterfaceActivatableProvider);

    // remove any WICs that use this SSID
    QMultiHash<QString, Knm::InterfaceConnection*>::iterator i = d->activatables.begin();
    while (i != d->activatables.end()) {

        Knm::InterfaceConnection * ic = i.value();

        if (ic->activatableType() == Knm::Activatable::WirelessInterfaceConnection ) {
            Knm::WirelessInterfaceConnection * wic = static_cast<Knm::WirelessInterfaceConnection*>(ic);

            if (wic->ssid() == ssid && wic->operationMode() != Solid::Control::WirelessNetworkInterfaceNm09::Adhoc) {
                d->activatableList->removeActivatable(ic);
                i = d->activatables.erase(i);
                delete wic;
            } else {
                ++i;
            }
        } else {
            ++i;
        }
    }
    // remove any WirelessNetwork
    Knm::WirelessNetwork * wni = d->wirelessNetworks.take(ssid);
    if (wni) {
        d->activatableList->removeActivatable(wni);
        delete wni;
    }
}

void WirelessNetworkInterfaceActivatableProvider::wirelessEnabledChanged(bool status)
{
    Q_D(WirelessNetworkInterfaceActivatableProvider);
    kDebug() << "triggered with status " << status;
    if (!status)
    {
        foreach (const QString &uuid, d->connectionList->connections()) {
            Knm::Connection * connection = d->connectionList->findConnection(uuid);
            Knm::WirelessSetting * wirelessSetting = dynamic_cast<Knm::WirelessSetting *>(connection->setting(Knm::Setting::Wireless));
            if (wirelessSetting && (wirelessSetting->mode() == Knm::WirelessSetting::EnumMode::adhoc ||
                                    wirelessSetting->mode() == Knm::WirelessSetting::EnumMode::apMode))
            {
                handleRemove(connection);
            }
        }
    }
    else
    {
        foreach (const QString &uuid, d->connectionList->connections()) {
            Knm::Connection * connection = d->connectionList->findConnection(uuid);
            Knm::WirelessSetting * wirelessSetting = dynamic_cast<Knm::WirelessSetting *>(connection->setting(Knm::Setting::Wireless));
            if (wirelessSetting && (wirelessSetting->mode() == Knm::WirelessSetting::EnumMode::adhoc ||
                                    wirelessSetting->mode() == Knm::WirelessSetting::EnumMode::apMode))
            {
                handleAdd(connection);
            }
        }
    }
}

bool WirelessNetworkInterfaceActivatableProvider::needsActivatableForUnconfigured() const
{
    bool needed =  Solid::Control::NetworkManagerNm09::isWirelessEnabled()
        && Solid::Control::NetworkManagerNm09::isWirelessHardwareEnabled();
    return needed;
}

// vim: sw=4 sts=4 et tw=100
