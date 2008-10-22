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

#include <solid/control/wirelessaccesspoint.h>
#include <solid/control/wirelessnetworkinterface.h>

#include "wirelessnetwork.h"
class WirelessNetwork::Private
{
public:
    QString ssid;
    Solid::Control::WirelessNetworkInterface * iface;
    int strength;
    QHash<QString, Solid::Control::AccessPoint *> aps;
};

WirelessNetwork::WirelessNetwork(const QString& ssid, Solid::Control::WirelessNetworkInterface * iface, QObject * parent)
: QObject(parent), d(new Private)
{
    d->ssid = ssid;
    d->iface = iface;
    d->strength = 0;
    connect(iface, SIGNAL(accessPointAppeared(const QString &)),
            SLOT(accessPointAppeared(const QString &)));
    connect(iface, SIGNAL(accessPointDisappeared(const QString&)),
            SLOT(accessPointDisappeared(const QString&)));
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
            connect(ap, SIGNAL(signalStrengthChanged(int)),
                    SLOT(updateStrength()));
            d->aps.insert(uni, ap);
            updateStrength();
        }
    }
}

void WirelessNetwork::accessPointDisappeared(const QString &uni)
{
    d->aps.remove(uni);
    if (d->aps.isEmpty()) {
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
}

// vim: sw=4 sts=4 et tw=100
