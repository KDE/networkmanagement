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

#include <QHash>
#include <QStringList>
#include <QTimer>

#include <KDebug>
#include <KIcon>
#include <KLocale>
#include <KNotification>
#include <kdeversion.h>

#include <Solid/Device>
#include <solid/control/networkmanager.h>
#include <solid/control/wirelessnetworkinterface.h>

#include <interfaceconnection.h>
#include <wirelessnetworkinterfaceenvironment.h>

#include "events.h"

#include "knmserviceprefs.h"


class NotificationManagerPrivate
{
public:
    bool suppressHardwareEvents;
    QTimer * newNetworkTimer;
    QTimer * disappearedNetworkTimer;
    QStringList newWirelessNetworks;
    QStringList disappearedWirelessNetworks;
    QHash<Solid::Control::NetworkInterface *, KNotification *> interfaceNotifications;
    // used to keep track of interface names so we can use them when the device has been removed.
    QHash<QString,QString> interfaceNameRecord;
};

NotificationManager::NotificationManager(QObject * parent)
: QObject(parent), d_ptr (new NotificationManagerPrivate)
{
    Q_D(NotificationManager);
    d->suppressHardwareEvents = true;
    d->newNetworkTimer = new QTimer(this);
    d->disappearedNetworkTimer = new QTimer(this);
    connect(d->newNetworkTimer, SIGNAL(timeout()), this, SLOT(notifyNewNetworks()));
    connect(d->disappearedNetworkTimer, SIGNAL(timeout()), this, SLOT(notifyDisappearedNetworks()));

    // status
    QObject::connect(Solid::Control::NetworkManager::notifier(), SIGNAL(statusChanged(Solid::Networking::Status)),
            this, SLOT(statusChanged(Solid::Networking::Status)));

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
    d->suppressHardwareEvents = false;
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

    // Keep a record for when it is removed
    Solid::Device* dev = new Solid::Device(uni);
    QString deviceText;
    KNetworkManagerServicePrefs::instance(Knm::ConnectionPersistence::NETWORKMANAGEMENT_RCFILE);
    
    if (KNetworkManagerServicePrefs::self()->interfaceNamingStyle() == KNetworkManagerServicePrefs::DescriptiveNames) {
        
#if KDE_IS_VERSION(4,3,60)
        deviceText = dev->description();
#else
        deviceText = dev->product();
#endif
    } else {        
        deviceText = iface->interfaceName();        
    }
    d->interfaceNameRecord.insert(uni, deviceText);

    if (iface && !d->suppressHardwareEvents) {

        QObject::connect(iface, SIGNAL(connectionStateChanged(int,int,int)), this, SLOT(interfaceConnectionStateChanged(int,int,int)));

        KNotification::event(Event::HwAdded, i18nc("@info:status Notification for hardware added", "%1 attached", deviceText), QPixmap(), 0, KNotification::CloseOnTimeout, KComponentData("networkmanagement", "networkmanagement", KComponentData::SkipMainComponentRegistration));
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

void NotificationManager::networkInterfaceRemoved(const QString &uni)
{
    Q_D(NotificationManager);
    QString deviceText = d->interfaceNameRecord.take(uni);

    if (!d->suppressHardwareEvents) {
        QString notificationText;
        if (deviceText.isEmpty()) {
            notificationText = i18nc("@info:status Notification for hardware removed used if we don't have its user-visible name", "Network interface removed", deviceText);
        } else {
            notificationText = i18nc("@info:status Notification for hardware removed giving vendor supplied product name", "%1 removed", deviceText);
        }

        KNotification::event(Event::HwRemoved, notificationText, QPixmap(), 0, KNotification::CloseOnTimeout, KComponentData("networkmanagement", "networkmanagement", KComponentData::SkipMainComponentRegistration));
    }
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

    KNotification::event(Event::UserConnectionAttempt, i18nc("@info:status Notification text when activating a connection","Connecting %1", ic->connectionName()), QPixmap(), 0, KNotification::CloseOnTimeout, KComponentData("networkmanagement", "networkmanagement", KComponentData::SkipMainComponentRegistration));

}

void NotificationManager::wirelessHardwareEnabledChanged(bool enabled)
{
    if (enabled) {
        KNotification::event(Event::RfOn, i18nc("@info:status Notification for radio kill switch turned on", "Wireless hardware enabled"), QPixmap(), 0, KNotification::CloseOnTimeout, KComponentData("networkmanagement", "networkmanagement", KComponentData::SkipMainComponentRegistration));
    } else {
        KNotification::event(Event::RfOff, i18nc("@info:status Notification for radio kill switch turned on", "Wireless hardware disabled"), QPixmap(), 0, KNotification::CloseOnTimeout, KComponentData("networkmanagement", "networkmanagement", KComponentData::SkipMainComponentRegistration));
    }
}

void NotificationManager::statusChanged(Solid::Networking::Status status)
{
    if (status == Solid::Networking::Unknown) {
        KNotification::event(Event::NetworkingDisabled, i18nc("@info:status Notification when the networking subsystem (NetworkManager, etc) is disabled", "Networking system disabled"), QPixmap(), 0, KNotification::CloseOnTimeout, KComponentData("networkmanagement", "networkmanagement", KComponentData::SkipMainComponentRegistration));
    }
}

QString NotificationManager::connectionStateToString(Solid::Control::NetworkInterface::ConnectionState state)
{
    QString stateString;
    switch (state) {
        case Solid::Control::NetworkInterface::UnknownState:
            stateString = i18nc("description of unknown network interface state", "is in an unknown state");
            break;
        case Solid::Control::NetworkInterface::Unmanaged:
            stateString = i18nc("description of unmanaged network interface state", "is unmanaged");
            break;
        case Solid::Control::NetworkInterface::Unavailable:
            stateString = i18nc("description of unavailable network interface state", "is unavailable");
            break;
        case Solid::Control::NetworkInterface::Disconnected:
            stateString = i18nc("description of unconnected network interface state", "is not connected");
            break;
        case Solid::Control::NetworkInterface::Preparing:
            stateString = i18nc("description of preparing to connect network interface state", "is preparing to connect");
            break;
        case Solid::Control::NetworkInterface::Configuring:
            stateString = i18nc("description of configuring hardware network interface state", "is being configured");
            break;
        case Solid::Control::NetworkInterface::NeedAuth:
            stateString = i18nc("description of waiting for authentication network interface state", "is waiting for authorization");
            break;
        case Solid::Control::NetworkInterface::IPConfig:
            stateString = i18nc("network interface doing dhcp request in most cases", "is setting network address");
            break;
        case Solid::Control::NetworkInterface::Activated:
            stateString = i18nc("network interface connected state label", "is connected");
            break;
        case Solid::Control::NetworkInterface::Failed:
            stateString = i18nc("network interface connection failed state label", "Connection Failed");
            break;
        default:
            stateString = I18N_NOOP("UNKNOWN STATE FIX ME");
    }
    return stateString;
}

void NotificationManager::interfaceConnectionStateChanged(int new_state, int, int reason)
{
    Q_D(NotificationManager);

    Solid::Control::NetworkInterface * iface = qobject_cast<Solid::Control::NetworkInterface *>(sender());
    if (iface) {
        QString text;
        QString identifier = iface->interfaceName();
        QString stateString = connectionStateToString((Solid::Control::NetworkInterface::ConnectionState)new_state);
        // need to keep the notification object around to reset it during connection cycles, but
        // delete it at the end of a connection cycle
        // keep a map of interface to KNotification
        // if not end of connection cycle, look for a 
        // if set and not end of connection cycle, reuse this notification
        bool keepNotification = false;

        if (new_state == Solid::Control::NetworkInterface::Preparing
                || new_state == Solid::Control::NetworkInterface::Configuring
                || new_state == Solid::Control::NetworkInterface::NeedAuth
                || new_state == Solid::Control::NetworkInterface::IPConfig) {
            keepNotification = false;
        }

        switch (reason) {
            case Solid::Control::NetworkInterface::NoReason:
            case Solid::Control::NetworkInterface::UnknownReason:
                text = i18nc("@info:status Notification when an interface (%1) connection state (%2) changes ", "%1 %2", identifier, stateString );
                break;
            case Solid::Control::NetworkInterface::NowManagedReason:
                text = i18nc("@info:status Notification when an interface (%1) changes state (%2) due to NowManagedReason","Interface %1 is now %2 because it is now being managed", identifier, stateString);
                break;
            case Solid::Control::NetworkInterface::NowUnmanagedReason:
                text = i18nc("@info:status Notification when an interface (%1) changes state (%2) due to NowUnmanagedReason","Interface %1 is now %2 because no longer being managed", identifier, stateString );
                break;
            case Solid::Control::NetworkInterface::ConfigFailedReason:
                text = i18nc("@info:status Notification when an interface (%1) changes state (%2) due to ConfigFailedReason","Interface %1 is now %2 because configuration failed", identifier, stateString);
                break;
            case Solid::Control::NetworkInterface::ConfigUnavailableReason:
                text = i18nc("@info:status Notification when an interface (%1) changes state (%2) due to ConfigUnavailableReason","%1 is now %2 because the configuration is unavailable", identifier, stateString);
                break;
            case Solid::Control::NetworkInterface::ConfigExpiredReason:
                text = i18nc("@info:status Notification when an interface (%1) changes state (%2) due to ConfigExpiredReason","%1 is now %2 because the configuration has expired", identifier, stateString);
                break;
            case Solid::Control::NetworkInterface::NoSecretsReason:
                text = i18nc("@info:status Notification when an interface (%1) changes state (%2) due to NoSecretsReason","%1 is now %2 because secrets were not provided", identifier, stateString);
                break;
            case Solid::Control::NetworkInterface::AuthSupplicantDisconnectReason:
                text = i18nc("@info:status Notification when an interface (%1) changes state (%2) due to AuthSupplicantDisconnectReason","%1 is now %2 because the authorization supplicant disconnected", identifier, stateString);
                break;
            case Solid::Control::NetworkInterface::AuthSupplicantConfigFailedReason:
                text = i18nc("@info:status Notification when an interface (%1) changes state (%2) due to AuthSupplicantConfigFailedReason","%1 is now %2 because the authorization supplicant's configuration failed", identifier, stateString);
                break;
            case Solid::Control::NetworkInterface::AuthSupplicantFailedReason:
                text = i18nc("@info:status Notification when an interface (%1) changes state (%2) due to AuthSupplicantFailedReason","%1 is now %2 because the authorization supplicant failed", identifier, stateString);
                break;
            case Solid::Control::NetworkInterface::AuthSupplicantTimeoutReason:
                text = i18nc("@info:status Notification when an interface (%1) changes state (%2) due to AuthSupplicantTimeoutReason","%1 is now %2 because the authorization supplicant timed out", identifier, stateString);
                break;
            case Solid::Control::NetworkInterface::PppStartFailedReason:
                text = i18nc("@info:status Notification when an interface (%1) changes state (%2) due to PppStartFailedReason","%1 is now %2 because PPP failed to start", identifier, stateString);
                break;
            case Solid::Control::NetworkInterface::PppDisconnectReason:
                text = i18nc("@info:status Notification when an interface (%1) changes state (%2) due to PppDisconnectReason","%1 is now %2 because PPP disconnected", identifier, stateString);
                break;
            case Solid::Control::NetworkInterface::PppFailedReason:
                text = i18nc("@info:status Notification when an interface (%1) changes state (%2) due to PppFailedReason","%1 is now %2 because PPP failed", identifier, stateString);
                break;
            case Solid::Control::NetworkInterface::DhcpStartFailedReason:
                text = i18nc("@info:status Notification when an interface (%1) changes state (%2) due to DhcpStartFailedReason","%1 is now %2 because DHCP failed to start", identifier, stateString);
                break;
            case Solid::Control::NetworkInterface::DhcpErrorReason:
                text = i18nc("@info:status Notification when an interface (%1) changes state (%2) due to DhcpErrorReason","%1 is now %2 because a DHCP error occurred", identifier, stateString);
                break;
            case Solid::Control::NetworkInterface::DhcpFailedReason:
                text = i18nc("@info:status Notification when an interface (%1) changes state (%2) due to DhcpFailedReason","%1 is now %2 because DHCP failed ", identifier, stateString);
                break;
            case Solid::Control::NetworkInterface::SharedStartFailedReason:
                text = i18nc("@info:status Notification when an interface (%1) changes state (%2) due to SharedStartFailedReason","%1 is now %2 because the shared service failed to start", identifier, stateString);
                break;
            case Solid::Control::NetworkInterface::SharedFailedReason:
                text = i18nc("@info:status Notification when an interface (%1) changes state (%2) due to SharedFailedReason","%1 is now %2 because the shared service failed", identifier, stateString);
                break;
            case Solid::Control::NetworkInterface::AutoIpStartFailedReason:
                text = i18nc("@info:status Notification when an interface (%1) changes state (%2) due to AutoIpStartFailedReason","%1 is now %2 because the auto IP service failed to start", identifier, stateString);
                break;
            case Solid::Control::NetworkInterface::AutoIpErrorReason:
                text = i18nc("@info:status Notification when an interface (%1) changes state (%2) due to AutoIpErrorReason","%1 is now %2 because the auto IP service reported an error", identifier, stateString);
                break;
            case Solid::Control::NetworkInterface::AutoIpFailedReason:
                text = i18nc("@info:status Notification when an interface (%1) changes state (%2) due to AutoIpFailedReason","%1 is now %2 because the auto IP service failed", identifier, stateString);
                break;
            case Solid::Control::NetworkInterface::ModemBusyReason:
                text = i18nc("@info:status Notification when an interface (%1) changes state (%2) due to ModemBusyReason","%1 is now %2 because the modem is busy", identifier, stateString);
                break;
            case Solid::Control::NetworkInterface::ModemNoDialToneReason:
                text = i18nc("@info:status Notification when an interface (%1) changes state (%2) due to ModemNoDialToneReason","%1 is now %2 because the modem has no dial tone", identifier, stateString);
                break;
            case Solid::Control::NetworkInterface::ModemNoCarrierReason:
                text = i18nc("@info:status Notification when an interface (%1) changes state (%2) due to ModemNoCarrierReason","%1 is now %2 because the modem shows no carrier", identifier, stateString);
                break;
            case Solid::Control::NetworkInterface::ModemDialTimeoutReason:
                text = i18nc("@info:status Notification when an interface (%1) changes state (%2) due to ModemDialTimeoutReason","%1 is now %2 because the modem dial timed out", identifier, stateString);
                break;
            case Solid::Control::NetworkInterface::ModemInitFailedReason:
                text = i18nc("@info:status Notification when an interface (%1) changes state (%2) due to ModemInitFailedReason","%1 is now %2 because the modem could not be initialized", identifier, stateString);
                break;
            case Solid::Control::NetworkInterface::GsmApnSelectFailedReason:
                text = i18nc("@info:status Notification when an interface (%1) changes state (%2) due to GsmApnSelectFailedReason","%1 is now %2 because the GSM APN could not be selected", identifier, stateString);
                break;
            case Solid::Control::NetworkInterface::GsmNotSearchingReason:
                text = i18nc("@info:status Notification when an interface (%1) changes state (%2) due to GsmNotSearchingReason","%1 is now %2 because the GSM modem is not searching", identifier, stateString);
                break;
            case Solid::Control::NetworkInterface::GsmRegistrationDeniedReason:
                text = i18nc("@info:status Notification when an interface (%1) changes state (%2) due to GsmRegistrationDeniedReason","%1 is now %2 because GSM network registration was denied", identifier, stateString);
                break;
            case Solid::Control::NetworkInterface::GsmRegistrationTimeoutReason:
                text = i18nc("@info:status Notification when an interface (%1) changes state (%2) due to GsmRegistrationTimeoutReason","%1 is now %2 because GSM network registration timed out", identifier, stateString);
                break;
            case Solid::Control::NetworkInterface::GsmRegistrationFailedReason:
                text = i18nc("@info:status Notification when an interface (%1) changes state (%2) due to GsmRegistrationFailedReason","%1 is now %2 because GSM registration failed", identifier, stateString);
                break;
            case Solid::Control::NetworkInterface::GsmPinCheckFailedReason:
                text = i18nc("@info:status Notification when an interface (%1) changes state (%2) due to GsmPinCheckFailedReason","%1 is now %2 because the GSM PIN check failed", identifier, stateString);
                break;
            case Solid::Control::NetworkInterface::FirmwareMissingReason:
                text = i18nc("@info:status Notification when an interface (%1) changes state (%2) due to FirmwareMissingReason","%1 is now %2 because firmware is missing", identifier, stateString);
                break;
            case Solid::Control::NetworkInterface::DeviceRemovedReason:
                text = i18nc("@info:status Notification when an interface (%1) changes state (%2) due to DeviceRemovedReason","%1 is now %2 because the device was removed", identifier, stateString);
                break;
            case Solid::Control::NetworkInterface::SleepingReason:
                text = i18nc("@info:status Notification when an interface (%1) changes state (%2) due to SleepingReason","%1 is now %2 because the networking system is now sleeping", identifier, stateString);
                break;
            case Solid::Control::NetworkInterface::ConnectionRemovedReason:
                text = i18nc("@info:status Notification when an interface (%1) changes state (%2) due to ConnectionRemovedReason","%1 is now %2 because the connection was removed", identifier, stateString);
                break;
            case Solid::Control::NetworkInterface::UserRequestedReason:
                text = i18nc("@info:status Notification when an interface (%1) changes state (%2) due to UserRequestedReason","%1 is now %2 at user request", identifier, stateString);
                break;
            case Solid::Control::NetworkInterface::CarrierReason:
                text = i18nc("@info:status Notification when an interface (%1) changes state (%2) due to CarrierReason","%1 is now %2 because the link changed state", identifier, stateString);
                break;
        }
        KNotification * notification = 0;

        if (d->interfaceNotifications.contains(iface)) {
            notification = d->interfaceNotifications.take(iface);
        } else {
            notification = new KNotification(Event::InterfaceStateChange, 0, KNotification::Persistent);
            notification->setComponentData(KComponentData("networkmanagement", "networkmanagement", KComponentData::SkipMainComponentRegistration));
            //notification->addContext(QLatin1String("connectiontype"), /*need to get the connection being activated on this device...*/);
        }

        notification->setText(text);

        if (keepNotification) {
            d->interfaceNotifications.insert(iface, notification);
        } else {
            notification->setFlags(KNotification::CloseOnTimeout);
        }
        notification->sendEvent();
    }
}

// vim: sw=4 sts=4 et tw=100
