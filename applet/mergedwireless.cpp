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

#include "mergedwireless.h"
#include <QHash>
#include <QStringList>
#include <KDebug>
#include <KIcon>
#include <KLocale>
#include <KNotification>

#include "events.h"

class WirelessNetworkMerged::Private
{
public:
    QHash<WirelessEnvironment*, WirelessNetwork*> networks;
    QString ssid;
    WirelessNetwork * reference;
};

WirelessNetworkMerged::WirelessNetworkMerged(WirelessEnvironment *envt, WirelessNetwork *network, QObject *parent)
    : AbstractWirelessNetwork(parent), d(new WirelessNetworkMerged::Private)
{
    d->reference = 0;
    addWirelessNetworkInternal(envt, network);
}

WirelessNetworkMerged::~WirelessNetworkMerged()
{
    //kDebug() << ssid();
    delete d;
}

QString WirelessNetworkMerged::ssid() const
{
    return d->ssid;
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

void WirelessNetworkMerged::addWirelessNetworkInternal(WirelessEnvironment * envt, WirelessNetwork * network)
{
    if (d->reference == 0) {
        d->reference = network;
        d->ssid = network->ssid();
    }
    if ( network->ssid() == d->ssid ) {
        d->networks.insert(envt, network);
        connect(network, SIGNAL(strengthChanged(const QString&,int)), SLOT(onStrengthChanged(const QString&,int)));
        connect(envt, SIGNAL(networkDisappeared(const QString&)), SLOT(disappeared(const QString)));
        if (network) {
            onStrengthChanged(network->ssid(), network->strength());
        }
    }
}

void WirelessNetworkMerged::onStrengthChanged(const QString &, int strength)
{
    WirelessNetwork * network = qobject_cast<WirelessNetwork*>(sender());
    if (d->reference == 0) {
        d->reference = network;
        emit strengthChanged(d->reference->ssid(), strength);
    } else {
        if (network == d->reference) {
            emit strengthChanged(d->reference->ssid(), strength);
        } else if (strength > d->reference->strength()) {
            d->reference = network;
            emit strengthChanged(d->reference->ssid(), strength);
        }
    }
}

void WirelessNetworkMerged::disappeared(const QString& ssid)
{
    if (ssid == d->ssid) {
        //kDebug() << ssid;
        WirelessEnvironment * envt = qobject_cast<WirelessEnvironment*>(sender());
        WirelessNetwork * disappearedNetwork = d->networks.take(envt);
        if (d->networks.isEmpty()) {
            emit noAccessPoints(ssid);
        } else if (d->reference && d->reference == disappearedNetwork) {
            d->reference = d->networks.begin().value();
            foreach (WirelessNetwork * candidate, d->networks) {
                if (candidate->strength() > d->reference->strength()) {
                    d->reference = candidate;
                }
            }
            emit strengthChanged(d->reference->ssid(), d->reference->strength());
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
    m_newNetworkTimer = new QTimer(this);
    connect(m_newNetworkTimer, SIGNAL(timeout()), this, SLOT(notifyNewNetwork()));
    m_newNetworks = QStringList();

    m_disappearedNetworkTimer = new QTimer(this);
    connect(m_disappearedNetworkTimer, SIGNAL(timeout()), this, SLOT(notifyDisappearedNetwork()));
    m_disappearedNetworks = QStringList();
}

WirelessEnvironmentMerged::~WirelessEnvironmentMerged()
{
    delete d_ptr;
}

QStringList WirelessEnvironmentMerged::networks() const
{
    Q_D(const WirelessEnvironmentMerged);
    return d->networks.keys();
}

AbstractWirelessNetwork * WirelessEnvironmentMerged::findNetwork(const QString &ssid) const
{
    Q_D(const WirelessEnvironmentMerged);
    return d->networks.value(ssid);
}

void WirelessEnvironmentMerged::addWirelessEnvironment(WirelessEnvironment * wEnv)
{
    Q_D(WirelessEnvironmentMerged);
    d->environments.append(wEnv);

    foreach (QString ssid, wEnv->networks()) {
        addNetworkInternal(wEnv, qobject_cast<WirelessNetwork*>(wEnv->findNetwork(ssid)));
    }

    // connect signals
    connect(wEnv, SIGNAL(networkAppeared(const QString&)),
            SLOT(onNetworkAppeared(const QString&)));
    connect(wEnv, SIGNAL(destroyed(QObject*)),
            SLOT(onWirelessEnvironmentDestroyed(QObject*)));
}

// TODO make slots on our Private object
void WirelessEnvironmentMerged::onWirelessEnvironmentDestroyed(QObject * obj)
{
    Q_D(WirelessEnvironmentMerged);
    d->environments.removeAll(qobject_cast<WirelessEnvironment*>(obj));
}

void WirelessEnvironmentMerged::onNetworkAppeared(const QString &ssid)
{
    Q_D(WirelessEnvironmentMerged);
    WirelessEnvironment * sourceEnvt = qobject_cast<WirelessEnvironment*>(sender());
    if (sourceEnvt) {
        WirelessNetwork * newNetwork = qobject_cast<WirelessNetwork*>(sourceEnvt->findNetwork(ssid));
        //kDebug() << ssid << d->networks.keys();
        WirelessNetworkMerged * ourNetwork = d->networks.value(ssid);
        if (ourNetwork) {
            ourNetwork->addWirelessNetworkInternal(sourceEnvt, newNetwork);
        } else {
            addNetworkInternal(sourceEnvt, newNetwork, false);
        }
    }
}

void WirelessEnvironmentMerged::disappeared(const QString &ssid)
{
    Q_D(WirelessEnvironmentMerged);
    delete d->networks.take(ssid);

    m_disappearedNetworks.append(ssid);
    m_disappearedNetworkTimer->start(500);
    emit networkDisappeared(ssid);
}

void WirelessEnvironmentMerged::addNetworkInternal(WirelessEnvironment * source, WirelessNetwork * newNetwork, bool quietly)
{
    Q_D(WirelessEnvironmentMerged);
    WirelessNetworkMerged * ourNetwork = new WirelessNetworkMerged(source, newNetwork, this);
    QString ssid = ourNetwork->ssid();
    d->networks.insert(ssid, ourNetwork);

    connect(ourNetwork, SIGNAL(noAccessPoints(const QString&)),
            SLOT(disappeared(const QString&)));
    m_newNetworks.append(ssid);
    if (!quietly) {
        m_newNetworkTimer->start(500);
    }

    emit networkAppeared(ssid);
}

void WirelessEnvironmentMerged::notifyNewNetwork()
{
    if (m_newNetworks.count() == 1) {
        KNotification::event(Event::NetworkAppeared, i18nc("Notification text when a wireless network interface was found","Wireless network %1 found", m_newNetworks[0]), QPixmap(), 0, KNotification::CloseOnTimeout, KComponentData("networkmanagement", "networkmanagement", KComponentData::SkipMainComponentRegistration));
    } else {
        KNotification::event(Event::NetworkAppeared, i18nc("Notification text when multiple wireless networks are found","<b>New wireless networks:</b><br /> %1", m_newNetworks.join(", ")), KIcon("network-wireless").pixmap(QSize(48,48)), 0, KNotification::CloseOnTimeout, KComponentData("networkmanagement", "networkmanagement", KComponentData::SkipMainComponentRegistration));
    }
    m_newNetworkTimer->stop();
    m_newNetworks.clear();
}

void WirelessEnvironmentMerged::notifyDisappearedNetwork()
{
    if (m_disappearedNetworks.count() == 1) {
        KNotification::event(Event::NetworkDisappeared, i18nc("Notification text when a wireless network interface disappeared","Wireless network %1 disappeared", m_disappearedNetworks[0]), QPixmap(), 0, KNotification::CloseOnTimeout, KComponentData("networkmanagement", "networkmanagement", KComponentData::SkipMainComponentRegistration));

    } else {
        KNotification::event(Event::NetworkDisappeared, i18nc("Notification text when multiple wireless networks have disappeared","<b>Wireless networks have disappeared:</b><br /> %1", m_disappearedNetworks.join(", ")), KIcon("network-wireless").pixmap(QSize(48,48)), 0, KNotification::CloseOnTimeout, KComponentData("networkmanagement", "networkmanagement", KComponentData::SkipMainComponentRegistration));
    }
    m_disappearedNetworkTimer->stop();
    m_disappearedNetworks.clear();
}

// vim: sw=4 sts=4 et tw=100
