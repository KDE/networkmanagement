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

QStringList WirelessEnvironment::wirelessNetworks() const
{
    Q_D(const WirelessEnvironment);
    return d->networks.keys();
}

AbstractWirelessNetwork * WirelessEnvironment::findWirelessNetwork(const QString & ssid) const
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
    kDebug() << d->iface->interfaceName() << " found " << uni;
    accessPointAppearedInternal(uni);
}

void WirelessEnvironment::accessPointAppearedInternal(const QString &uni)
{
    Q_D(WirelessEnvironment);
    Solid::Control::AccessPoint * ap = d->iface->findAccessPoint(uni);
    QString ssid = ap->ssid();
    if (ssid.isEmpty()) {
        kDebug() << "ignoring hidden AP with BSSID:" << ap->hardwareAddress();
    } else if (!d->networks.contains(ssid)) {
        WirelessNetwork * net = new WirelessNetwork(ap, d->iface, 0);
        d->networks.insert(ssid, net);
        //connect(net, SIGNAL(strengthChanged(const
        connect(net, SIGNAL(disappeared(const QString&)), SLOT(networkDisappeared(const QString&)));
        emit wirelessNetworkAppeared(ssid);
    }
}

void WirelessEnvironment::networkDisappeared(const QString &ssid)
{
    Q_D(WirelessEnvironment);
    kDebug() << ssid;
    WirelessNetwork * net = d->networks.take(ssid);
    delete net;
    emit wirelessNetworkDisappeared(ssid);
}

void WirelessEnvironment::wirelessEnabledChanged(bool enabled)
{
    Q_D(WirelessEnvironment);
    if (!enabled) {
        foreach (WirelessNetwork * network, d->networks) {
            emit wirelessNetworkDisappeared(network->ssid());
            delete network;
        }
    }
}

class WirelessEnvironmentMergedPrivate
{
public:
    QList<WirelessEnvironment*> environments;
    QHash<QString, WirelessNetworkMerged*> networks;
};

WirelessEnvironmentMerged::WirelessEnvironmentMerged(QObject * parent)
    : AbstractWirelessEnvironment(parent), d_ptr(new WirelessEnvironmentMergedPrivate)
{

}

WirelessEnvironmentMerged::~WirelessEnvironmentMerged()
{
    delete d_ptr;
}

QStringList WirelessEnvironmentMerged::wirelessNetworks() const
{
    Q_D(const WirelessEnvironmentMerged);
    return d->networks.keys();
}

AbstractWirelessNetwork * WirelessEnvironmentMerged::findWirelessNetwork(const QString &ssid) const
{
    Q_D(const WirelessEnvironmentMerged);
    return d->networks.value(ssid);
}

void WirelessEnvironmentMerged::addWirelessEnvironment(WirelessEnvironment * wEnv)
{
    Q_D(WirelessEnvironmentMerged);
    d->environments.append(wEnv);

    foreach (QString ssid, wEnv->wirelessNetworks()) {
        addWirelessNetworkInternal(qobject_cast<WirelessNetwork*>(wEnv->findWirelessNetwork(ssid)));
    }

    // connect signals
    connect(wEnv, SIGNAL(wirelessNetworkAppeared(const QString&)),
            SLOT(onWirelessNetworkAppeared(const QString&)));
    connect(wEnv, SIGNAL(destroyed(QObject*)),
            SLOT(onWirelessEnvironmentDestroyed(QObject*)));
}

// TODO make slots on our Private object
void WirelessEnvironmentMerged::onWirelessEnvironmentDestroyed(QObject * obj)
{
    Q_D(WirelessEnvironmentMerged);
    d->environments.removeAll(qobject_cast<WirelessEnvironment*>(obj));
}

void WirelessEnvironmentMerged::onWirelessNetworkAppeared(const QString &ssid)
{
    Q_D(WirelessEnvironmentMerged);
    WirelessEnvironment * sourceEnvt = qobject_cast<WirelessEnvironment*>(sender());
    if (sourceEnvt) {
        WirelessNetwork * newNetwork = qobject_cast<WirelessNetwork*>(sourceEnvt->findWirelessNetwork(ssid));
        WirelessNetworkMerged * ourNetwork = d->networks.value(ssid);
        if (ourNetwork) {
            ourNetwork->addWirelessNetworkInternal(newNetwork);
        } else {
            addWirelessNetworkInternal(newNetwork, false);
        }
    }
}

void WirelessEnvironmentMerged::addWirelessNetworkInternal(WirelessNetwork * newNetwork, bool quietly)
{
    Q_D(WirelessEnvironmentMerged);
    WirelessNetworkMerged * ourNetwork = new WirelessNetworkMerged(newNetwork, this);
    QString ssid = ourNetwork->ssid();
    d->networks.insert(ssid, ourNetwork);

    connect(ourNetwork, SIGNAL(disappeared(const QString&)),
            SLOT(onWirelessNetworkDisappeared(const QString&)));


    if (!quietly) {
        KNotification::event(Event::NetworkAppeared, i18nc("Notification text when a wireless network interface was found","Wireless network %1 found", ssid), QPixmap(), 0, KNotification::CloseOnTimeout, KComponentData("knetworkmanager", "knetworkmanager", KComponentData::SkipMainComponentRegistration));
    }

    emit wirelessNetworkAppeared(ssid);
}

void WirelessEnvironmentMerged::onWirelessNetworkDisappeared(const QString &ssid)
{
    Q_D(WirelessEnvironmentMerged);
    delete d->networks.take(ssid);
    KNotification::event(Event::NetworkDisappeared, i18nc("Notification text when a wireless network interface disappeared","Wireless network %1 disappeared", ssid), QPixmap(), 0, KNotification::CloseOnTimeout, KComponentData("knetworkmanager", "knetworkmanager", KComponentData::SkipMainComponentRegistration));
    emit wirelessNetworkDisappeared(ssid);
}

// vim: sw=4 sts=4 et tw=100
