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

#include "wirelessenvironment.h"

#include <KNotification>
#include <KLocale>

#include <solid/control/wirelessaccesspoint.h>
#include <solid/control/wirelessnetworkinterface.h>

#include "events.h"
#include "wirelessnetwork.h"

class WirelessEnvironment::Private
{
public:
    QHash<QString, WirelessNetwork*> networks;
    Solid::Control::WirelessNetworkInterface * iface;
};

WirelessEnvironment::WirelessEnvironment(Solid::Control::WirelessNetworkInterface * iface, QObject * parent)
: d(new Private)
{
    d->iface = iface;
    foreach (QString apUni, iface->accessPoints()) {
        accessPointAppearedInternal(apUni, true);
    }
    // for managing our list of wireless networks
    connect(iface, SIGNAL(accessPointAppeared(const QString&)),
            SLOT(accessPointAppeared(const QString&)));
}

WirelessEnvironment::~WirelessEnvironment()
{
    delete d;
}

QStringList WirelessEnvironment::wirelessNetworks() const
{
    return d->networks.keys();
}

WirelessNetwork * WirelessEnvironment::findWirelessNetwork(const QString & ssid) const
{
    WirelessNetwork * net = 0;
    if (d->networks.contains(ssid)) {
        net = d->networks.value(ssid);
    }
    return net;
}

void WirelessEnvironment::accessPointAppeared(const QString &uni)
{
    accessPointAppearedInternal(uni);
    emit wirelessNetworksChanged();
}

void WirelessEnvironment::accessPointAppearedInternal(const QString &uni, bool quietly)
{
    Solid::Control::AccessPoint * ap = d->iface->findAccessPoint(uni);
    QString ssid = ap->ssid();
    if (!d->networks.contains(ssid)) {
        WirelessNetwork * net = new WirelessNetwork(ssid, d->iface, 0);
        d->networks.insert(ssid, net);
        //connect(net, SIGNAL(strengthChanged(const
        connect(net, SIGNAL(disappeared(const QString&)), SLOT(networkDisappeared(const QString&)));
        net->accessPointAppeared(uni);
        emit wirelessNetworksChanged();
        if (!quietly) {
            KNotification::event(Event::NetworkAppeared, i18nc("Notification text when a wireless network interface was found","Wireless network %1 found", ssid), QPixmap(), 0, KNotification::CloseOnTimeout, KComponentData("knetworkmanager", "knetworkmanager", KComponentData::SkipMainComponentRegistration));
        }
    }
}

void WirelessEnvironment::networkDisappeared(const QString &ssid)
{
    WirelessNetwork * net = d->networks.take(ssid);
    KNotification::event(Event::NetworkDisappeared, i18nc("Notification text when a wireless network interface disappeared","Wireless network %1 disappeared", ssid), QPixmap(), 0, KNotification::CloseOnTimeout, KComponentData("knetworkmanager", "knetworkmanager", KComponentData::SkipMainComponentRegistration));
    delete net;
    emit wirelessNetworksChanged();
}


// vim: sw=4 sts=4 et tw=100
