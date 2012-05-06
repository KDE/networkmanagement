/*
Copyright 2008,2009 Will Stephenson <wstephenson@kde.org>

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
#include "wirelessnetworkinterfaceenvironment.h"

#include <QMutableHashIterator>

#include <KDebug>
#include <KNotification>
#include <KLocale>

#include <solid/control/networkmanager.h>
#include <solid/control/wirelessaccesspoint.h>
#include <solid/control/wirelessnetworkinterface.h>

//#include "events.h"
#include "wirelessnetwork.h"
#include "wirelessnetwork_p.h"

namespace Solid
{
namespace Control
{

class WirelessNetworkInterfaceEnvironmentPrivate
{
public:
    virtual ~WirelessNetworkInterfaceEnvironmentPrivate() {}
    QHash<QString, Solid::Control::WirelessNetwork*> networks;
    Solid::Control::WirelessNetworkInterfaceNm09 * iface;
};
} // namespace Control
} // namespace Solid

Solid::Control::WirelessNetworkInterfaceEnvironment::WirelessNetworkInterfaceEnvironment(Solid::Control::WirelessNetworkInterfaceNm09 * iface)
: QObject(iface), d_ptr(new WirelessNetworkInterfaceEnvironmentPrivate)
{
    Q_D(WirelessNetworkInterfaceEnvironment);
    d->iface = iface;
    foreach (const QString &apUni, iface->accessPoints()) {
        accessPointAppearedInternal(apUni);
    }
    // for managing our list of wireless networks
    connect(iface, SIGNAL(accessPointAppeared(QString)),
            SLOT(accessPointAppeared(QString)));
    connect(Solid::Control::NetworkManagerNm09::notifier(), SIGNAL(wirelessEnabledChanged(bool)),
            SLOT(wirelessEnabledChanged(bool)));
}

Solid::Control::WirelessNetworkInterfaceEnvironment::~WirelessNetworkInterfaceEnvironment()
{
    kDebug();
    delete d_ptr;
}

Solid::Control::WirelessNetworkInterfaceNm09 * Solid::Control::WirelessNetworkInterfaceEnvironment::interface() const
{
    Q_D(const WirelessNetworkInterfaceEnvironment);
    return d->iface;
}

QStringList Solid::Control::WirelessNetworkInterfaceEnvironment::networks() const
{
    Q_D(const WirelessNetworkInterfaceEnvironment);
    return d->networks.keys();
}

Solid::Control::WirelessNetwork * Solid::Control::WirelessNetworkInterfaceEnvironment::findNetwork(const QString & ssid) const
{
    Q_D(const WirelessNetworkInterfaceEnvironment);
    WirelessNetwork * net = 0;
    if (d->networks.contains(ssid)) {
        net = d->networks.value(ssid);
    }
    return net;
}

void Solid::Control::WirelessNetworkInterfaceEnvironment::accessPointAppeared(const QString &uni)
{
    Q_UNUSED(uni);
    //kDebug() << d->iface->interfaceName() << " found " << uni;
    accessPointAppearedInternal(uni);
}

void Solid::Control::WirelessNetworkInterfaceEnvironment::accessPointAppearedInternal(const QString &uni)
{
    Q_D(WirelessNetworkInterfaceEnvironment);
    Solid::Control::AccessPointNm09 * ap = d->iface->findAccessPoint(uni);
    QString ssid = ap->ssid();
    //kDebug() << ssid << d->networks.contains(ssid);
    if (ssid.isEmpty()) {
        //kDebug() << "ignoring hidden AP with BSSID:" << ap->hardwareAddress();
    } else if (!d->networks.contains(ssid)) {
        Solid::Control::WirelessNetwork * net = new Solid::Control::WirelessNetwork(ap, d->iface, this);
        d->networks.insert(ssid, net);
        connect(net, SIGNAL(disappeared(QString)), SLOT(removeNetwork(QString)));
        emit networkAppeared(ssid);
    }

    //dump();
}
/*
void Solid::Control::WirelessNetworkInterfaceEnvironment::dump()
{
   Q_D(WirelessNetworkInterfaceEnvironment);
   kDebug() << d->networks.count();
   QHashIterator<QString, WirelessNetwork*> i(d->networks);
   while (i.hasNext()) {
       i.next();
       i.value()->dump();
   }
}
*/
void Solid::Control::WirelessNetworkInterfaceEnvironment::removeNetwork(const QString &ssid)
{
    Q_D(WirelessNetworkInterfaceEnvironment);
    //kDebug() << ssid;
    QHash<QString, WirelessNetwork*>::iterator it = d->networks.find(ssid);
    if ( it == d->networks.end() )
        return;
    WirelessNetwork * net = it.value();
    if ( net ) {
        emit networkDisappeared(ssid);
        delete net;
    }
    d->networks.erase(it);
    //dump();
}

void Solid::Control::WirelessNetworkInterfaceEnvironment::wirelessEnabledChanged(bool enabled)
{
    Q_D(WirelessNetworkInterfaceEnvironment);
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
