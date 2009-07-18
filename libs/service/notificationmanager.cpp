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

#include "notificationmanager.h"

#include <QStringList>
#include <QTimer>

#include <KIcon>
#include <KLocale>
#include <KNotification>

#include <solid/control/networkmanager.h>
#include <solid/control/wirelessnetworkinterface.h>

#include <interfaceconnection.h>
#include <wirelessnetworkinterfaceenvironment.h>

#include "events.h"

class NotificationManagerPrivate
{
public:
    bool suppressHardwareEvents;
    QTimer * newNetworkTimer;
    QTimer * disappearedNetworkTimer;
    QStringList newWirelessNetworks;
    QStringList disappearedWirelessNetworks;
};

NotificationManager::NotificationManager(QObject * parent)
: QObject(parent), d_ptr (new NotificationManagerPrivate)
{
    Q_D(NotificationManager);
    d->suppressHardwareEvents = false;
    d->newNetworkTimer = new QTimer(this);
    d->disappearedNetworkTimer = new QTimer(this);
    connect(d->newNetworkTimer, SIGNAL(timeout()), this, SLOT(notifyNewNetworks()));
    connect(d->disappearedNetworkTimer, SIGNAL(timeout()), this, SLOT(notifyDisappearedNetworks()));

    // status
    QObject::connect(Solid::Control::NetworkManager::notifier(), SIGNAL(statusChanged(Solid::Networking::Status)),
            this, SLOT(statusChanged(solid::Networking::Status)));

    // rfkill
    QObject::connect(Solid::Control::NetworkManager::notifier(), SIGNAL(wirelessHardwareEnabledChanged(bool)),
            this, SLOT(wirelessHardwareEnabledChanged(bool)));

    // interfaces
    QObject::connect(Solid::Control::NetworkManager::notifier(), SIGNAL(networkInterfaceAdded(const QString&)),
            this, SLOT(networkInterfaceAdded(const QString&)));
    QObject::connect(Solid::Control::NetworkManager::notifier(), SIGNAL(networkInterfaceRemoved(const QString&)),
            this, SLOT(networkInterfaceRemoved(const QString&)));

    foreach (Solid::Control::NetworkInterface* interface, Solid::Control::NetworkManager::networkInterfaces()) {
        networkInterfaceAdded(interface->uni());
    }
}

NotificationManager::~NotificationManager()
{
    delete d_ptr;
}

void NotificationManager::handleAdd(Knm::Activatable * activatable)
{
    Knm::InterfaceConnection * ic = qobject_cast<Knm::InterfaceConnection *>(activatable);
    if (ic) {
        connect(ic, SIGNAL(activated()), this, SLOT(interfaceConnectionActivated()));
    }
}

void NotificationManager::handleUpdate(Knm::Activatable *)
{

}

void NotificationManager::handleRemove(Knm::Activatable *)
{

}

void NotificationManager::networkInterfaceAdded(const QString & uni)
{
    Q_D(NotificationManager);
    Solid::Control::NetworkInterface * iface = Solid::Control::NetworkManager::findNetworkInterface(uni);
    if (iface && !d->suppressHardwareEvents) {
        KNotification::event(Event::HwAdded, i18nc("@info:status Notification for hardware added", "%1 attached", iface->interfaceName()), QPixmap(), 0, KNotification::CloseOnTimeout, KComponentData("networkmanagement", "networkmanagement", KComponentData::SkipMainComponentRegistration));
    }

    // if wireless, listen for new networks
    if (iface->type() == Solid::Control::NetworkInterface::Ieee80211) {
        Solid::Control::WirelessNetworkInterface * wireless = qobject_cast<Solid::Control::WirelessNetworkInterface*>(iface);

        if (wireless) {
            // this is a bit wasteful because WirelessNetworkInterfaceActivatableProvider is also
            // creating these objects, but I expect these will move into Solid and become singletons
            Solid::Control::WirelessNetworkInterfaceEnvironment * environment = new Solid::Control::WirelessNetworkInterfaceEnvironment(wireless);

            QObject::connect(environment, SIGNAL(networkAppeared(const QString &)),
                    this, SLOT(networkAppeared(const QString&)));
            QObject::connect(environment, SIGNAL(networkDisappeared(const QString &)),
                    this, SLOT(networkDisappeared(const QString&)));
        }
    }
}

void NotificationManager::networkInterfaceRemoved(const QString &)
{
        KNotification::event(Event::HwRemoved, i18nc("Notification for hardware removed", "Network interface removed"), QPixmap(), 0, KNotification::CloseOnTimeout, KComponentData("networkmanagement", "networkmanagement", KComponentData::SkipMainComponentRegistration));
}

void NotificationManager::networkAppeared(const QString & ssid)
{
    Q_D(NotificationManager);
    d->newWirelessNetworks.append(ssid);
    d->newNetworkTimer->start(500);
}

void NotificationManager::networkDisappeared(const QString & ssid)
{
    Q_D(NotificationManager);
    d->disappearedWirelessNetworks.append(ssid);
    d->disappearedNetworkTimer->start(500);
}

void NotificationManager::notifyNewNetworks()
{
    Q_D(NotificationManager);
    if (d->newWirelessNetworks.count() == 1) {
        KNotification::event(Event::NetworkAppeared, i18nc("@info:status Notification text when a single wireless network was found","Wireless network %1 found", d->newWirelessNetworks[0]), QPixmap(), 0, KNotification::CloseOnTimeout, KComponentData("knetworkmanager", "knetworkmanager", KComponentData::SkipMainComponentRegistration));
    } else {
        KNotification::event(Event::NetworkAppeared, i18nc("@info:status Notification text when multiple wireless networks are found","<b>New wireless networks:</b><br /> %1", d->newWirelessNetworks.join(", ")), KIcon("network-wireless").pixmap(QSize(48,48)), 0, KNotification::CloseOnTimeout, KComponentData("knetworkmanager", "knetworkmanager", KComponentData::SkipMainComponentRegistration));
    }
    d->newNetworkTimer->stop();
    d->newWirelessNetworks.clear();
}

void NotificationManager::notifyDisappearedNetworks()
{
    Q_D(NotificationManager);
    if (d->disappearedWirelessNetworks.count() == 1) {
        KNotification::event(Event::NetworkDisappeared, i18nc("@info:status Notification text when a single wireless network disappeared","Wireless network %1 disappeared", d->disappearedWirelessNetworks[0]), QPixmap(), 0, KNotification::CloseOnTimeout, KComponentData("knetworkmanager", "knetworkmanager", KComponentData::SkipMainComponentRegistration));

    } else {
        KNotification::event(Event::NetworkDisappeared, i18nc("@info:status Notification text when multiple wireless networks have disappeared","<b>Wireless networks disappeared:</b><br /> %1", d->disappearedWirelessNetworks.join(", ")), KIcon("network-wireless").pixmap(QSize(48,48)), 0, KNotification::CloseOnTimeout, KComponentData("knetworkmanager", "knetworkmanager", KComponentData::SkipMainComponentRegistration));
    }
    d->disappearedNetworkTimer->stop();
    d->disappearedWirelessNetworks.clear();
}

void NotificationManager::interfaceConnectionActivated()
{
    Knm::InterfaceConnection * ic = qobject_cast<Knm::InterfaceConnection *>(sender());

    KNotification::event(Event::UserConnectionAttempt, i18nc("Notification text when activating a connection","Connecting %1", ic->connectionName()), QPixmap(), 0, KNotification::CloseOnTimeout, KComponentData("networkmanagement", "networkmanagement", KComponentData::SkipMainComponentRegistration));

}

void NotificationManager::wirelessHardwareEnabledChanged(bool enabled)
{
    if (enabled) {
        KNotification::event(Event::RfOn, i18nc("Notification for radio kill switch turned on", "Wireless hardware enabled"), QPixmap(), 0, KNotification::CloseOnTimeout, KComponentData("networkmanagement", "networkmanagement", KComponentData::SkipMainComponentRegistration));
    } else {
        KNotification::event(Event::RfOff, i18nc("Notification for radio kill switch turned on", "Wireless hardware disabled"), QPixmap(), 0, KNotification::CloseOnTimeout, KComponentData("networkmanagement", "networkmanagement", KComponentData::SkipMainComponentRegistration));
    }
}

void NotificationManager::statusChanged(Solid::Networking::Status status)
{

}

// vim: sw=4 sts=4 et tw=100
