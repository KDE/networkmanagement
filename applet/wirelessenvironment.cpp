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

#include <QMutableHashIterator>

#include <KDebug>
#include <KNotification>
#include <KLocale>

#include <solid/control/networkmanager.h>
#include <solid/control/wirelessaccesspoint.h>
#include <solid/control/wirelessnetworkinterface.h>

#include "events.h"
#include "wirelessnetwork.h"

AbstractWirelessEnvironment::AbstractWirelessEnvironment(QObject *parent)
:QObject(parent)
{

}

AbstractWirelessEnvironment::~AbstractWirelessEnvironment()
{

}

class WirelessEnvironmentPrivate
{
public:
    virtual ~WirelessEnvironmentPrivate() {}
    QHash<QString, WirelessNetwork*> networks;
    Solid::Control::WirelessNetworkInterface * iface;
};

WirelessEnvironment::WirelessEnvironment(Solid::Control::WirelessNetworkInterface * iface, QObject * parent)
: AbstractWirelessEnvironment(parent), d_ptr(new WirelessEnvironmentPrivate)
{
    Q_D(WirelessEnvironment);
    d->iface = iface;
    foreach (QString apUni, iface->accessPoints()) {
        accessPointAppearedInternal(apUni);
    }
    // for managing our list of wireless networks
    connect(iface, SIGNAL(accessPointAppeared(const QString&)),
            SLOT(accessPointAppeared(const QString&)));
    connect(Solid::Control::NetworkManager::notifier(), SIGNAL(wirelessEnabledChanged(bool)),
            SLOT(wirelessEnabledChanged(bool)));
}

WirelessEnvironment::~WirelessEnvironment()
{
    delete d_ptr;
}

QStringList WirelessEnvironment::networks() const
{
    Q_D(const WirelessEnvironment);
    return d->networks.keys();
}

AbstractWirelessNetwork * WirelessEnvironment::findNetwork(const QString & ssid) const
{
    Q_D(const WirelessEnvironment);
    WirelessNetwork * net = 0;
    if (d->networks.contains(ssid)) {
        net = d->networks.value(ssid);
    }
    return net;
}

void WirelessEnvironment::accessPointAppeared(const QString &uni)
{
    Q_D(WirelessEnvironment);
    //kDebug() << d->iface->interfaceName() << " found " << uni;
    accessPointAppearedInternal(uni);
}

void WirelessEnvironment::accessPointAppearedInternal(const QString &uni)
{
    Q_D(WirelessEnvironment);
    Solid::Control::AccessPoint * ap = d->iface->findAccessPoint(uni);
    QString ssid = ap->ssid();
    if (ssid.isEmpty()) {
        //kDebug() << "ignoring hidden AP with BSSID:" << ap->hardwareAddress();
    } else if (!d->networks.contains(ssid)) {
        WirelessNetwork * net = new WirelessNetwork(ap, d->iface, 0);
        d->networks.insert(ssid, net);
        connect(net, SIGNAL(noAccessPoints(const QString&)), SLOT(removeNetwork(const QString&)));
        emit networkAppeared(ssid);
    }
}

void WirelessEnvironment::removeNetwork(const QString &ssid)
{
    Q_D(WirelessEnvironment);
    //kDebug() << ssid;
    WirelessNetwork * net = d->networks.value(ssid);
    if ( net ) {
        emit networkDisappeared(ssid);
        delete net;
    }
    d->networks.remove(ssid);
}

void WirelessEnvironment::wirelessEnabledChanged(bool enabled)
{
    Q_D(WirelessEnvironment);
    if (!enabled) {
        QMutableHashIterator<QString, WirelessNetwork*> i (d->networks);
        while (i.hasNext()) {
            i.next();
            QString deletedSsid = i.key();
            //delete i.value();
            i.remove();
            emit networkDisappeared(deletedSsid);
        }
    }
}

// vim: sw=4 sts=4 et tw=100
