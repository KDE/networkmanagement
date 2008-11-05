/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License or (at your option) version 3 or any later version
accepted by the membership of KDE e.V. (or its successor approved
by the membership of KDE e.V.), which shall act as a proxy
defined in Section 14 of version 3 of the license.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <kdebug.h>

#include <solid/control/wirelessaccesspoint.h>
#include <solid/control/wirelessnetworkinterface.h>

#include "wirelessnetwork.h"

AbstractWirelessNetwork::AbstractWirelessNetwork(QObject * parent)
    : QObject(parent)
{

}

AbstractWirelessNetwork::~AbstractWirelessNetwork()
{

}

class WirelessNetwork::Private
{
public:
    QString ssid;
    Solid::Control::WirelessNetworkInterface * iface;
    int strength;
    QHash<QString, Solid::Control::AccessPoint *> aps;
};

WirelessNetwork::WirelessNetwork(Solid::Control::AccessPoint * ap, Solid::Control::WirelessNetworkInterface * iface, QObject * parent)
: AbstractWirelessNetwork(parent), d(new Private)
{
    d->ssid = ap->ssid();
    d->iface = iface;
    d->strength = 0;
    connect(iface, SIGNAL(accessPointAppeared(const QString &)),
            SLOT(accessPointAppeared(const QString &)));
    connect(iface, SIGNAL(accessPointDisappeared(const QString&)),
            SLOT(accessPointDisappeared(const QString&)));

    addAccessPointInternal(ap);
}

WirelessNetwork::~WirelessNetwork()
{
    delete d;
}

QString WirelessNetwork::ssid() const
{
    return d->ssid;
}

int WirelessNetwork::strength() const
{
    return d->strength;
}

void WirelessNetwork::accessPointAppeared(const QString &uni)
{
    if (!d->aps.contains(uni)) {
        Solid::Control::AccessPoint * ap = d->iface->findAccessPoint(uni);
        if (ap->ssid() == d->ssid) {
            addAccessPointInternal(ap);
        }
    }
}

void WirelessNetwork::addAccessPointInternal(Solid::Control::AccessPoint * ap)
{
    connect(ap, SIGNAL(signalStrengthChanged(int)),
            SLOT(updateStrength()));
    d->aps.insert(ap->uni(), ap);
    updateStrength();
}

void WirelessNetwork::accessPointDisappeared(const QString &uni)
{
    d->aps.remove(uni);
    if (d->aps.isEmpty()) {
        kDebug() << uni;
        emit disappeared(d->ssid);
    } else {
        updateStrength();
    }
}

void WirelessNetwork::updateStrength()
{
    int maximumStrength = -1;
    foreach (Solid::Control::AccessPoint* iface, d->aps) {
        maximumStrength = qMax(maximumStrength, iface->signalStrength());
    }
    if (maximumStrength != d->strength) {
        d->strength = maximumStrength;
        emit strengthChanged(d->ssid, d->strength);
    }
    //kDebug() << "update strength" << d->ssid << d->strength;
}

Solid::Control::AccessPoint * WirelessNetwork::referenceAccessPoint() const
{
    int maximumStrength = -1;
    Solid::Control::AccessPoint* strongest = 0;
    foreach (Solid::Control::AccessPoint* iface, d->aps) {
        int oldMax = maximumStrength;
        maximumStrength = qMax(maximumStrength, iface->signalStrength());
        if ( oldMax <= maximumStrength ) {
            strongest = iface;
        }
    }
    return strongest;
}



class WirelessNetworkMerged::Private
{
public:
    QList<WirelessNetwork*> networks;
    WirelessNetwork * reference;
};

WirelessNetworkMerged::WirelessNetworkMerged(WirelessNetwork * network, QObject * parent)
    : AbstractWirelessNetwork(parent), d(new WirelessNetworkMerged::Private)
{
    d->reference = 0;
    addWirelessNetworkInternal(network);
}

WirelessNetworkMerged::~WirelessNetworkMerged()
{
    delete d;
}

QString WirelessNetworkMerged::ssid() const
{
    QString ssid;
    if (d->reference) {
        ssid = d->reference->ssid();
    }
    return ssid;
}


int WirelessNetworkMerged::strength() const
{
    int strength = -1;
    if (d->reference) {
        strength = d->reference->strength();
    }
    return strength;
}

Solid::Control::AccessPoint * WirelessNetworkMerged::referenceAccessPoint() const
{
    if (d->reference) {
        return d->reference->referenceAccessPoint();
    }
    return 0;
}

void WirelessNetworkMerged::addWirelessNetworkInternal(WirelessNetwork * network)
{
    if (d->reference == 0) {
        d->reference = network;
    }
    if ( network->ssid() == d->reference->ssid() ) {
        d->networks.append(network);
        connect(network, SIGNAL(strengthChanged(const QString&,int)), SLOT(onStrengthChanged(const QString&,int)));
        connect(network, SIGNAL(disappeared(const QString&)), SLOT(onDisappeared(const QString&)));
        onStrengthChanged(network->ssid(), network->strength());
    }
}

void WirelessNetworkMerged::onStrengthChanged(const QString &, int strength)
{
    WirelessNetwork * network = qobject_cast<WirelessNetwork*>(sender());
    if (d->reference == 0) {
        d->reference = network;
        emit strengthChanged(d->reference->ssid(), strength);
    } else {
        if (strength > d->reference->strength()) {
            d->reference = network;
            emit strengthChanged(d->reference->ssid(), strength);
        }
    }
}

void WirelessNetworkMerged::onDisappeared(const QString& ssid)
{
    kDebug() << ssid;
    WirelessNetwork * disappearedNetwork = qobject_cast<WirelessNetwork*>(sender());
    d->networks.removeAll(disappearedNetwork);
    if (d->networks.isEmpty()) {
        emit disappeared(ssid); // this relies on this slot ONLY being called from a wireless network with our ssid...
    } else if (d->reference && d->reference == disappearedNetwork) {
        d->reference = d->networks.first();
        foreach (WirelessNetwork * candidate, d->networks) {
            if (candidate->strength() > d->reference->strength()) {
                d->reference = candidate;
            }
        }
        emit strengthChanged(d->reference->ssid(), d->reference->strength());
    }
}

// vim: sw=4 sts=4 et tw=100
