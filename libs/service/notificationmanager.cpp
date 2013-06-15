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
#include <QSet>
#include <QTimer>

#include <KDebug>
#include <KIcon>
#include <KLocale>
#include <kdeversion.h>
#include "kglobal.h"
#include <KToolInvocation>
#include <KStandardDirs>
#include <KIconLoader>

#include <Solid/Device>
#include <solid/control/networkmanager.h>
#include <solid/control/wirelessnetworkinterface.h>

#include <interfaceconnection.h>
#include <vpninterfaceconnection.h>
#include <uiutils.h>
#include <wirelessnetworkinterfaceenvironment.h>

#include "events.h"
#include "../internals/connection.h"

static const int iconSize = 48;

K_GLOBAL_STATIC_WITH_ARGS(KComponentData, s_networkManagementComponentData, ("networkmanagement", "libknetworkmanager", KComponentData::SkipMainComponentRegistration))

InterfaceNotificationHost::InterfaceNotificationHost(Solid::Control::NetworkInterfaceNm09 * interface, NotificationManager * manager) : QObject(manager), m_manager(manager), m_interface(interface), m_suppressStrengthNotification(false)
{
    // Keep a record for when it is removed
    m_interfaceNameLabel = UiUtils::interfaceNameLabel(interface->uni());

    // For the notification icon
    m_type = Knm::Connection::typeFromSolidType(interface);

    QObject::connect(interface, SIGNAL(connectionStateChanged(int,int,int)),
            this, SLOT(interfaceConnectionStateChanged(int,int,int)));
}

InterfaceNotificationHost::~InterfaceNotificationHost()
{
}

void InterfaceNotificationHost::addInterfaceConnection(Knm::InterfaceConnection * ic)
{
    if (ic) {
        m_interfaceConnections.insert(ic);
        connect(ic, SIGNAL(activationStateChanged(Knm::InterfaceConnection::ActivationState,Knm::InterfaceConnection::ActivationState)),
                this, SLOT(interfaceConnectionActivationStateChanged(Knm::InterfaceConnection::ActivationState,Knm::InterfaceConnection::ActivationState)));

        switch (ic->connectionType()) {
            case Knm::Connection::Wireless: connect(ic, SIGNAL(strengthChanged(int)), this, SLOT(strengthChanged(int))); break;
            case Knm::Connection::Gsm: connect(ic, SIGNAL(signalQualityChanged(int)), this, SLOT(strengthChanged(int))); break;
            default: break;
        }
    }
}

QString InterfaceNotificationHost::label() const
{
    return m_interfaceNameLabel;
}

Knm::Connection::Type InterfaceNotificationHost::type() const
{
    return m_type;
}

void InterfaceNotificationHost::removeInterfaceConnection(Knm::InterfaceConnection * ic)
{
    if (ic) {
        disconnect(ic, 0, this, 0);
    }

    m_interfaceConnections.remove(ic);
    m_activating.remove(ic);
}

void InterfaceNotificationHost::interfaceConnectionActivationStateChanged(Knm::InterfaceConnection::ActivationState oldState, Knm::InterfaceConnection::ActivationState newState)
{
    Q_UNUSED(oldState)
    kDebug() << newState;

    Knm::InterfaceConnection * ic = qobject_cast<Knm::InterfaceConnection *>(sender());

    switch (newState) {
        case Knm::InterfaceConnection::Activating:
            kDebug() << ic->connectionName() << "is activating";
            m_activating.insert(ic);
            KNotification::event(Event::Connecting, m_interfaceNameLabel, i18nc("@info:status Notification text when connecting","Activating %1", ic->connectionName()), KIcon(Knm::Connection::iconName(ic->connectionType())).pixmap(QSize(iconSize,iconSize)), 0, KNotification::CloseOnTimeout, m_manager->componentData());
            break;
        case Knm::InterfaceConnection::Activated:
            m_activating.remove(ic);
            KNotification::event(Event::Connected, m_interfaceNameLabel, i18nc("@info:status Notification text when a connection has been activated","%1 activated", ic->connectionName()), KIcon(Knm::Connection::iconName(ic->connectionType())).pixmap(QSize(iconSize,iconSize)), 0, KNotification::CloseOnTimeout, m_manager->componentData());
            break;
        case Knm::InterfaceConnection::Deactivating:
            // Ignored.
            break;
        case Knm::InterfaceConnection::Deactivated:
            m_activating.remove(ic);
            KNotification::event(Event::Disconnected, m_interfaceNameLabel, i18nc("@info:status Notification text when deactivating a connection","%1 deactivated", ic->connectionName()), KIcon(Knm::Connection::iconName(ic->connectionType())).pixmap(QSize(iconSize,iconSize)), 0, KNotification::CloseOnTimeout, m_manager->componentData());
            break;
        case Knm::InterfaceConnection::Unknown:
            m_activating.remove(ic);
            if (ic->connectionType() != Knm::Connection::Wireless || Solid::Control::NetworkManagerNm09::isWirelessHardwareEnabled()) {
                if (ic->oldActivationState() == Knm::InterfaceConnection::Activating) {
                    KNotification::event(Event::ConnectFailed, m_interfaceNameLabel, i18nc("@info:status Notification text when connection has failed","Connection %1 failed", ic->connectionName()), KIcon(Knm::Connection::iconName(ic->connectionType())).pixmap(QSize(iconSize,iconSize)), 0, KNotification::CloseOnTimeout, m_manager->componentData());
                }
            }
            break;
    }
}

void InterfaceNotificationHost::strengthChanged(int strength)
{
    if (m_suppressStrengthNotification || strength > 30) {
        return;
    }
    Knm::InterfaceConnection * ic = qobject_cast<Knm::InterfaceConnection *>(sender());

    if (ic->activationState() == Knm::InterfaceConnection::Activated) {
        m_suppressStrengthNotification = true;
	// ignore strengh notifications for 5 minutes.
        QTimer::singleShot(5 * 60 * 1000, this, SLOT(enableStrengthNotification()));
        KNotification::event(Event::LowSignal, m_interfaceNameLabel, i18nc("@info:status Notification text when wireless/gsm signal is low","Low signal on %1", ic->connectionName()), KIcon(Knm::Connection::iconName(ic->connectionType())).pixmap(QSize(iconSize,iconSize)), 0, KNotification::CloseOnTimeout, m_manager->componentData());
    }
}

void InterfaceNotificationHost::enableStrengthNotification()
{
    m_suppressStrengthNotification = false;
}

void InterfaceNotificationHost::interfaceConnectionStateChanged(int new_state, int, int reason)
{
    kDebug() << new_state << reason;
    QString title;
    QString text;
    KNotification::NotificationFlag flag = KNotification::Persistent;

    QString identifier = UiUtils::interfaceNameLabel(m_interface->uni());
    QString stateString = UiUtils::connectionStateToString((Solid::Control::NetworkInterfaceNm09::ConnectionState)new_state);
    /*
    // need to keep the notification object around to reset it during connection cycles, but
    // delete it at the end of a connection cycle
    // keep a map of interface to KNotification
    // if not end of connection cycle, look for a
    // if set and not end of connection cycle, reuse this notification
    bool keepNotification = false;

    if (new_state == Solid::Control::NetworkInterfaceNm09::Preparing
    || new_state == Solid::Control::NetworkInterfaceNm09::Configuring
    || new_state == Solid::Control::NetworkInterfaceNm09::NeedAuth
    || new_state == Solid::Control::NetworkInterfaceNm09::IPConfig) {
    keepNotification = true;
    }
    */
    // if it's a connecting state, get the name of the InterfaceConnection that is activating
    // identify which connection is actually activating.  This is ugly because NM should signal
    // when it activates
    Knm::InterfaceConnection * activatingConnection = 0;
    if (!m_activating.isEmpty()) {
        activatingConnection = *(m_activating.begin());
    }

    //X     QSetIterator<Knm::InterfaceConnection*> it(m_interfaceConnections);
    //X
    //X     while (it.hasNext()) {
    //X         Knm::InterfaceConnection * ic = it.next();
    //X         if (ic->activationState() == Knm::InterfaceConnection::Activating) {
    //X             activatingConnection = ic;
    //X             break;
    //X         }

    switch (new_state) {
        case Solid::Control::NetworkInterfaceNm09::Preparing:
        case Solid::Control::NetworkInterfaceNm09::Configuring:
        case Solid::Control::NetworkInterfaceNm09::NeedAuth:
        case Solid::Control::NetworkInterfaceNm09::IPConfig:
        case Solid::Control::NetworkInterfaceNm09::IPCheck:
        case Solid::Control::NetworkInterfaceNm09::Secondaries:
            if (activatingConnection) {
                title = i18nc("@info:status interface (%2) status notification title when a connection (%1) is activating",
                        "Activating %1 on %2", activatingConnection->connectionName(), identifier);
            } else {
                title = identifier;
            }
            flag = KNotification::Persistent;
            break;
        case Solid::Control::NetworkInterfaceNm09::Activated:
        default:
            if (activatingConnection) {
                title = i18nc("@info:status interface (%2) status notification title when a connection (%1) has successfully activated",
                        "%1 on %2", activatingConnection->connectionName(), identifier);
            } else {
                title = identifier;
            }
            flag = KNotification::CloseOnTimeout;
            break;
    }

    switch (reason) {
        case Solid::Control::NetworkInterfaceNm09::NoReason:
        case Solid::Control::NetworkInterfaceNm09::UnknownReason:
            text = stateString;
            break;
        case Solid::Control::NetworkInterfaceNm09::NowManagedReason:
            text = i18nc("@info:status Notification when an interface changes state (%1) due to NowManagedReason","%1 because it is now being managed", stateString);
            break;
        case Solid::Control::NetworkInterfaceNm09::NowUnmanagedReason:
            text = i18nc("@info:status Notification when an interface changes state (%1) due to NowUnmanagedReason","%1 because it is no longer being managed", stateString );
            break;
        case Solid::Control::NetworkInterfaceNm09::ConfigFailedReason:
            text = i18nc("@info:status Notification when an interface changes state (%1) due to ConfigFailedReason","%1 because configuration failed", stateString);
            break;
        case Solid::Control::NetworkInterfaceNm09::ConfigUnavailableReason:
            text = i18nc("@info:status Notification when an interface changes state (%1) due to ConfigUnavailableReason","%1 because the configuration is unavailable", stateString);
            break;
        case Solid::Control::NetworkInterfaceNm09::ConfigExpiredReason:
            text = i18nc("@info:status Notification when an interface changes state (%1) due to ConfigExpiredReason","%1 because the configuration has expired", stateString);
            break;
        case Solid::Control::NetworkInterfaceNm09::NoSecretsReason:
            text = i18nc("@info:status Notification when an interface changes state (%1) due to NoSecretsReason","%1 because secrets were not provided", stateString);
            break;
        case Solid::Control::NetworkInterfaceNm09::AuthSupplicantDisconnectReason:
            text = i18nc("@info:status Notification when an interface changes state (%1) due to AuthSupplicantDisconnectReason","%1 because the authorization supplicant disconnected", stateString);
            break;
        case Solid::Control::NetworkInterfaceNm09::AuthSupplicantConfigFailedReason:
            text = i18nc("@info:status Notification when an interface changes state (%1) due to AuthSupplicantConfigFailedReason","%1 because the authorization supplicant's configuration failed", stateString);
            break;
        case Solid::Control::NetworkInterfaceNm09::AuthSupplicantFailedReason:
            text = i18nc("@info:status Notification when an interface changes state (%1) due to AuthSupplicantFailedReason","%1 because the authorization supplicant failed", stateString);
            break;
        case Solid::Control::NetworkInterfaceNm09::AuthSupplicantTimeoutReason:
            text = i18nc("@info:status Notification when an interface changes state (%1) due to AuthSupplicantTimeoutReason","%1 because the authorization supplicant timed out", stateString);
            break;
        case Solid::Control::NetworkInterfaceNm09::PppStartFailedReason:
            text = i18nc("@info:status Notification when an interface changes state (%1) due to PppStartFailedReason","%1 because PPP failed to start", stateString);
            break;
        case Solid::Control::NetworkInterfaceNm09::PppDisconnectReason:
            text = i18nc("@info:status Notification when an interface changes state (%1) due to PppDisconnectReason","%1 because PPP disconnected", stateString);
            break;
        case Solid::Control::NetworkInterfaceNm09::PppFailedReason:
            text = i18nc("@info:status Notification when an interface changes state (%1) due to PppFailedReason","%1 because PPP failed", stateString);
            break;
        case Solid::Control::NetworkInterfaceNm09::DhcpStartFailedReason:
            text = i18nc("@info:status Notification when an interface changes state (%1) due to DhcpStartFailedReason","%1 because DHCP failed to start", stateString);
            break;
        case Solid::Control::NetworkInterfaceNm09::DhcpErrorReason:
            text = i18nc("@info:status Notification when an interface changes state (%1) due to DhcpErrorReason","%1 because a DHCP error occurred", stateString);
            break;
        case Solid::Control::NetworkInterfaceNm09::DhcpFailedReason:
            text = i18nc("@info:status Notification when an interface changes state (%1) due to DhcpFailedReason","%1 because DHCP failed ", stateString);
            break;
        case Solid::Control::NetworkInterfaceNm09::SharedStartFailedReason:
            text = i18nc("@info:status Notification when an interface changes state (%1) due to SharedStartFailedReason","%1 because the shared service failed to start", stateString);
            break;
        case Solid::Control::NetworkInterfaceNm09::SharedFailedReason:
            text = i18nc("@info:status Notification when an interface changes state (%1) due to SharedFailedReason","%1 because the shared service failed", stateString);
            break;
        case Solid::Control::NetworkInterfaceNm09::AutoIpStartFailedReason:
            text = i18nc("@info:status Notification when an interface changes state (%1) due to AutoIpStartFailedReason","%1 because the auto IP service failed to start", stateString);
            break;
        case Solid::Control::NetworkInterfaceNm09::AutoIpErrorReason:
            text = i18nc("@info:status Notification when an interface changes state (%1) due to AutoIpErrorReason","%1 because the auto IP service reported an error", stateString);
            break;
        case Solid::Control::NetworkInterfaceNm09::AutoIpFailedReason:
            text = i18nc("@info:status Notification when an interface changes state (%1) due to AutoIpFailedReason","%1 because the auto IP service failed", stateString);
            break;
        case Solid::Control::NetworkInterfaceNm09::ModemBusyReason:
            text = i18nc("@info:status Notification when an interface changes state (%1) due to ModemBusyReason","%1 because the modem is busy", stateString);
            break;
        case Solid::Control::NetworkInterfaceNm09::ModemNoDialToneReason:
            text = i18nc("@info:status Notification when an interface changes state (%1) due to ModemNoDialToneReason","%1 because the modem has no dial tone", stateString);
            break;
        case Solid::Control::NetworkInterfaceNm09::ModemNoCarrierReason:
            text = i18nc("@info:status Notification when an interface changes state (%1) due to ModemNoCarrierReason","%1 because the modem shows no carrier", stateString);
            break;
        case Solid::Control::NetworkInterfaceNm09::ModemDialTimeoutReason:
            text = i18nc("@info:status Notification when an interface changes state (%1) due to ModemDialTimeoutReason","%1 because the modem dial timed out", stateString);
            break;
        case Solid::Control::NetworkInterfaceNm09::ModemInitFailedReason:
            text = i18nc("@info:status Notification when an interface changes state (%1) due to ModemInitFailedReason","%1 because the modem could not be initialized", stateString);
            break;
        case Solid::Control::NetworkInterfaceNm09::GsmApnSelectFailedReason:
            text = i18nc("@info:status Notification when an interface changes state (%1) due to GsmApnSelectFailedReason","%1 because the GSM APN could not be selected", stateString);
            break;
        case Solid::Control::NetworkInterfaceNm09::GsmNotSearchingReason:
            text = i18nc("@info:status Notification when an interface changes state (%1) due to GsmNotSearchingReason","%1 because the GSM modem is not searching", stateString);
            break;
        case Solid::Control::NetworkInterfaceNm09::GsmRegistrationDeniedReason:
            text = i18nc("@info:status Notification when an interface changes state (%1) due to GsmRegistrationDeniedReason","%1 because GSM network registration was denied", stateString);
            break;
        case Solid::Control::NetworkInterfaceNm09::GsmRegistrationTimeoutReason:
            text = i18nc("@info:status Notification when an interface changes state (%1) due to GsmRegistrationTimeoutReason","%1 because GSM network registration timed out", stateString);
            break;
        case Solid::Control::NetworkInterfaceNm09::GsmRegistrationFailedReason:
            text = i18nc("@info:status Notification when an interface changes state (%1) due to GsmRegistrationFailedReason","%1 because GSM registration failed", stateString);
            break;
        case Solid::Control::NetworkInterfaceNm09::GsmPinCheckFailedReason:
            text = i18nc("@info:status Notification when an interface changes state (%1) due to GsmPinCheckFailedReason","%1 because the GSM PIN check failed", stateString);
            break;
        case Solid::Control::NetworkInterfaceNm09::FirmwareMissingReason:
            text = i18nc("@info:status Notification when an interface changes state (%1) due to FirmwareMissingReason","%1 because firmware is missing", stateString);
            break;
        case Solid::Control::NetworkInterfaceNm09::DeviceRemovedReason:
            text = i18nc("@info:status Notification when an interface changes state (%1) due to DeviceRemovedReason","%1 because the device was removed", stateString);
            break;
        case Solid::Control::NetworkInterfaceNm09::SleepingReason:
            text = i18nc("@info:status Notification when an interface changes state (%1) due to SleepingReason","%1 because the networking system is now sleeping", stateString);
            break;
        case Solid::Control::NetworkInterfaceNm09::ConnectionRemovedReason:
            text = i18nc("@info:status Notification when an interface changes state (%1) due to ConnectionRemovedReason","%1 because the connection was removed", stateString);
            break;
        case Solid::Control::NetworkInterfaceNm09::UserRequestedReason:
            text = i18nc("@info:status Notification when an interface changes state (%1) due to UserRequestedReason","%1 by request", stateString);
            break;
        case Solid::Control::NetworkInterfaceNm09::CarrierReason:
            text = i18nc("@info:status Notification when an interface changes state (%1) due to CarrierReason","%1 because the cable was disconnected", stateString);
            break;
    }
    performInterfaceNotification(title, text, KIcon(Knm::Connection::iconName(m_type)).pixmap(QSize(iconSize,iconSize)), flag);
}

void InterfaceNotificationHost::performInterfaceNotification(const QString & title, const QString & text, const QPixmap & pixmap, KNotification::NotificationFlag flag)
{
    kDebug() << title << text << flag;
    KNotification * notification = 0;

    if (m_notification.isNull()) {
        notification = new KNotification(Event::InterfaceStateChange, 0, flag);
        notification->setComponentData(m_manager->componentData());
        if (flag == KNotification::Persistent)
            m_notification = QPointer<KNotification>(notification);
        //notification->addContext(QLatin1String("connectiontype"), /*need to get the connection being activated on this device...*/);
    } else {
        notification = m_notification.data();
        // if existing notification and we have a reason to kill it
        if (notification->flags().testFlag(KNotification::Persistent) && flag == KNotification::CloseOnTimeout) {
            // workaround for setFlags not doing this...
            QTimer::singleShot(10000, notification, SLOT(close()));
        }
        notification->setFlags(flag);
    }

    if (!title.isEmpty())
        notification->setTitle(title);

    notification->setText(text);
    notification->setPixmap(pixmap);

    notification->sendEvent();
}

class NotificationManagerPrivate
{
public:
    bool suppressHardwareEvents;
    QTimer * newNetworkTimer;
    QTimer * disappearedNetworkTimer;
    QStringList newWirelessNetworks;
    QStringList disappearedWirelessNetworks;
    QHash<QString, InterfaceNotificationHost *> interfaceHosts;
    ConnectionList * connectionList;
};

NotificationManager::NotificationManager(ConnectionList *connectionList, QObject * parent)
: QObject(parent), d_ptr (new NotificationManagerPrivate)
{
    Q_D(NotificationManager);
    d->connectionList = connectionList;
    d->suppressHardwareEvents = true;
    d->newNetworkTimer = new QTimer(this);
    d->disappearedNetworkTimer = new QTimer(this);
    connect(d->newNetworkTimer, SIGNAL(timeout()), this, SLOT(notifyNewNetworks()));
    connect(d->disappearedNetworkTimer, SIGNAL(timeout()), this, SLOT(notifyDisappearedNetworks()));

    // status
    QObject::connect(Solid::Control::NetworkManagerNm09::notifier(), SIGNAL(statusChanged(Solid::Networking::Status)),
            this, SLOT(statusChanged(Solid::Networking::Status)));

    // rfkill
    QObject::connect(Solid::Control::NetworkManagerNm09::notifier(), SIGNAL(wirelessHardwareEnabledChanged(bool)),
            this, SLOT(wirelessHardwareEnabledChanged(bool)));

    // interfaces
    QObject::connect(Solid::Control::NetworkManagerNm09::notifier(), SIGNAL(networkInterfaceAdded(QString)),
            this, SLOT(networkInterfaceAdded(QString)));
    QObject::connect(Solid::Control::NetworkManagerNm09::notifier(), SIGNAL(networkInterfaceRemoved(QString)),
            this, SLOT(networkInterfaceRemoved(QString)));

    foreach (Solid::Control::NetworkInterfaceNm09* interface, Solid::Control::NetworkManagerNm09::networkInterfaces()) {
        networkInterfaceAdded(interface->uni());
    }
    d->suppressHardwareEvents = false;
}

NotificationManager::~NotificationManager()
{
    delete d_ptr;
}

KComponentData NotificationManager::componentData() const
{
    return *s_networkManagementComponentData;
}

void NotificationManager::handleAdd(Knm::Activatable * activatable)
{
    Q_D(NotificationManager);
    Knm::InterfaceConnection * ic = qobject_cast<Knm::InterfaceConnection *>(activatable);
    if (ic) {
        InterfaceNotificationHost * host = d->interfaceHosts.value(activatable->deviceUni());
        if (host) {
            host->addInterfaceConnection(ic);
        }
    }
}

void NotificationManager::handleUpdate(Knm::Activatable *activatable)
{
    if (activatable->activatableType() == Knm::Activatable::VpnInterfaceConnection) {
        Knm::VpnInterfaceConnection * ic = qobject_cast<Knm::VpnInterfaceConnection *>(activatable);
        if (ic) {
            switch (ic->activationState()) {
                case Knm::InterfaceConnection::Activating:
                    kDebug() << ic->connectionName() << "is activating";
                    KNotification::event(Event::Connecting, QString(), i18nc("@info:status Notification text when connecting","Activating %1", ic->connectionName()), KIcon(Knm::Connection::iconName(ic->connectionType())).pixmap(QSize(iconSize,iconSize)), 0, KNotification::CloseOnTimeout, componentData());
                    break;
                case Knm::InterfaceConnection::Activated:
                    kDebug() << ic->connectionName() << "is activated";
                    KNotification::event(Event::Connected, QString(), i18nc("@info:status Notification text when a connection has been activated","%1 activated", ic->connectionName()), KIcon(Knm::Connection::iconName(ic->connectionType())).pixmap(QSize(iconSize,iconSize)), 0, KNotification::CloseOnTimeout, componentData());
                    break;
                case Knm::InterfaceConnection::Deactivating:
                    // Ignored.
                    break;
                case Knm::InterfaceConnection::Deactivated:
                    KNotification::event(Event::Disconnected, QString(), i18nc("@info:status Notification text when deactivating a connection","%1 deactivated", ic->connectionName()), KIcon(Knm::Connection::iconName(ic->connectionType())).pixmap(QSize(iconSize,iconSize)), 0, KNotification::CloseOnTimeout, componentData());
                    break;
                case Knm::InterfaceConnection::Unknown:
                    kDebug() << ic->connectionName() << "is unknown";
                    if (ic->oldActivationState() == Knm::InterfaceConnection::Activating) {
                        KNotification::event(Event::ConnectFailed, QString(), i18nc("@info:status Notification text when connection has failed","Connection %1 failed", ic->connectionName()), KIcon(Knm::Connection::iconName(ic->connectionType())).pixmap(QSize(iconSize,iconSize)), 0, KNotification::CloseOnTimeout, componentData());
                    }
            }
        }
    }
}

void NotificationManager::handleRemove(Knm::Activatable * activatable)
{
    Q_D(NotificationManager);
    Knm::InterfaceConnection * ic = qobject_cast<Knm::InterfaceConnection *>(activatable);
    if (ic) {
        InterfaceNotificationHost * host = d->interfaceHosts.value(activatable->deviceUni());
        if (host) {
            host->removeInterfaceConnection(ic);
        }
    }
}

void NotificationManager::createCellularConnection()
{
    QStringList args;
    args << QLatin1String("create") << QLatin1String("--type") << QLatin1String("cellular");
    KToolInvocation::kdeinitExec(KGlobal::dirs()->findResource("exe", "networkmanagement_configshell"), args);
}

void NotificationManager::networkInterfaceAdded(const QString & uni)
{
    Q_D(NotificationManager);
    kDebug() << uni;
    if (!d->interfaceHosts.contains(uni)) {

        kDebug() << "adding notification host";
        Solid::Control::NetworkInterfaceNm09 * iface = Solid::Control::NetworkManagerNm09::findNetworkInterface(uni);
        if (iface) {
            InterfaceNotificationHost * host = new InterfaceNotificationHost(iface, this);

            d->interfaceHosts.insert(uni, host);

            // notify hardware added
            if (!d->suppressHardwareEvents) {
                if (iface->type() == Solid::Control::NetworkInterfaceNm09::Modem) {
                    Solid::Control::ModemNetworkInterfaceNm09 * nmModemIface = qobject_cast<Solid::Control::ModemNetworkInterfaceNm09 *>(iface);
                    if (nmModemIface) {
                        // KNotification::CloseOnTimeout sometimes breaks the activation of slot createCellularConnection,
                        // so using Persistent here and closing the notification using QTimer::singleShot() below.
                        KNotification *notification= new KNotification(Event::HwAdded, 0, KNotification::Persistent);
                        notification->setComponentData(componentData());
                        notification->setText(i18nc("@info:status Notification for hardware added", "%1 attached", host->label()));
                        notification->setActions(( QStringList() << i18nc("@action", "Create Connection" ) << i18nc("@action", "Close" )) );
                        notification->setPixmap(KIcon(Knm::Connection::iconName(Knm::Connection::typeFromSolidType(iface))).pixmap(QSize(iconSize,iconSize)));
                        QObject::connect(notification,SIGNAL(activated()), this , SLOT(createCellularConnection()) );
                        QObject::connect(notification,SIGNAL(action1Activated()), this, SLOT(createCellularConnection()) );
                        QObject::connect(notification,SIGNAL(action2Activated()), notification, SLOT(close()) );
                        QObject::connect(notification,SIGNAL(ignored()), notification, SLOT(close()) );
                        notification->sendEvent();
                        QTimer::singleShot(10000, notification, SLOT(close()));
                        return;
                    }
                }
                KNotification::event(Event::HwAdded, i18nc("@info:status Notification for hardware added", "%1 attached", host->label()), KIcon(Knm::Connection::iconName(Knm::Connection::typeFromSolidType(iface))).pixmap(QSize(iconSize,iconSize)), 0, KNotification::CloseOnTimeout, componentData());
            }

            // if wireless, listen for new networks
            if (iface->type() == Solid::Control::NetworkInterfaceNm09::Wifi) {
                Solid::Control::WirelessNetworkInterfaceNm09 * wireless = qobject_cast<Solid::Control::WirelessNetworkInterfaceNm09*>(iface);

                if (wireless) {
                    // this is a bit wasteful because WirelessNetworkInterfaceActivatableProvider is also
                    // creating these objects, but I expect these will move into Solid and become singletons
                    Solid::Control::WirelessNetworkInterfaceEnvironment * environment = new Solid::Control::WirelessNetworkInterfaceEnvironment(wireless);

                    QObject::connect(environment, SIGNAL(networkAppeared(QString)),
                            this, SLOT(networkAppeared(QString)));
                    QObject::connect(environment, SIGNAL(networkDisappeared(QString)),
                            this, SLOT(networkDisappeared(QString)));
                }
            }
        }
    }
}

void NotificationManager::networkInterfaceRemoved(const QString &uni)
{
    Q_D(NotificationManager);

    kDebug() << uni;

    InterfaceNotificationHost * host = d->interfaceHosts.take(uni);

    kDebug() << d->interfaceHosts.keys();

    if (!d->suppressHardwareEvents) {
        QString notificationText;
        Knm::Connection::Type type = Knm::Connection::Wired;
        if (host) {
            notificationText = i18nc("@info:status Notification for hardware removed giving vendor supplied product name", "%1 removed", host->label());
            type = host->type();
            delete host;
        } else {
            notificationText = i18nc("@info:status Notification for hardware removed used if we don't have its user-visible name", "Network interface removed");
        }

        KNotification::event(Event::HwRemoved, notificationText, KIcon(Knm::Connection::iconName(type)).pixmap(QSize(iconSize,iconSize)), 0, KNotification::CloseOnTimeout, componentData());
    }
}

void NotificationManager::networkAppeared(const QString & ssid)
{
    Q_D(NotificationManager);
    Solid::Control::WirelessNetworkInterfaceEnvironment * environment = qobject_cast<Solid::Control::WirelessNetworkInterfaceEnvironment *>(sender());
    if (environment && environment->interface()->activeAccessPoint() == "/") {
        d->newWirelessNetworks.append(ssid);
        d->newNetworkTimer->start(500);
    }
}

void NotificationManager::networkDisappeared(const QString & ssid)
{
    Q_D(NotificationManager);
    Solid::Control::WirelessNetworkInterfaceEnvironment * environment = qobject_cast<Solid::Control::WirelessNetworkInterfaceEnvironment *>(sender());
    if (environment && environment->interface()->activeAccessPoint() == "/") {
        d->disappearedWirelessNetworks.append(ssid);
        d->disappearedNetworkTimer->start(500);
    }
}

void NotificationManager::notifyNewNetworks()
{
    Q_D(NotificationManager);
    if (d->newWirelessNetworks.isEmpty()) {
        return;
    } else if (d->newWirelessNetworks.count() == 1) {
        KNotification::event(Event::NetworkAppeared, i18nc("@info:status Notification text when a single wireless network was found","Wireless network %1 found", d->newWirelessNetworks[0]), KIcon("network-wireless").pixmap(QSize(iconSize,iconSize)), 0, KNotification::CloseOnTimeout, KComponentData("knetworkmanager", "knetworkmanager", KComponentData::SkipMainComponentRegistration));
    } else {
        KNotification::event(Event::NetworkAppeared, i18ncp("@info:status Notification text when multiple wireless networks are found. %2 is a list of networks, and the %1 value (not printed) is just used to determine the plural form of network.",
							    "<b>New wireless network:</b><br /> %2",
							    "<b>New wireless networks:</b><br /> %2",
							    d->newWirelessNetworks.count(), // the %1 parameter, used only to choose between plural forms on the word network
							    d->newWirelessNetworks.join(", ")), KIcon("network-wireless").pixmap(QSize(iconSize,iconSize)), 0, KNotification::CloseOnTimeout, componentData());
    }
    d->newNetworkTimer->stop();
    d->newWirelessNetworks.clear();
}

void NotificationManager::notifyDisappearedNetworks()
{
    Q_D(NotificationManager);
    if (d->disappearedWirelessNetworks.isEmpty()) {
        return;
    } else if (d->disappearedWirelessNetworks.count() == 1) {
        KNotification::event(Event::NetworkDisappeared, i18nc("@info:status Notification text when a single wireless network disappeared","Wireless network %1 disappeared", d->disappearedWirelessNetworks[0]), KIcon("network-wireless").pixmap(QSize(iconSize,iconSize)), 0, KNotification::CloseOnTimeout, KComponentData("knetworkmanager", "knetworkmanager", KComponentData::SkipMainComponentRegistration));

    } else {
        KNotification::event(Event::NetworkDisappeared, i18ncp("@info:status Notification text when multiple wireless networks have disappeared.  %2 is a list of networks, and the %1 value (not printed) is just used to determine the plural form of network.",
							       "<b>Wireless network disappeared:</b><br /> %2",
							       "<b>Wireless networks disappeared:</b><br /> %2",
							       d->disappearedWirelessNetworks.count(), // the %1 parameter, used only to choose between plural forms on the word network
							       d->disappearedWirelessNetworks.join(", ")), KIcon("network-wireless").pixmap(QSize(iconSize,iconSize)), 0, KNotification::CloseOnTimeout, componentData());
    }
    d->disappearedNetworkTimer->stop();
    d->disappearedWirelessNetworks.clear();
}

void NotificationManager::wirelessHardwareEnabledChanged(bool enabled)
{
    if (enabled) {
        KNotification::event(Event::RfOn, i18nc("@info:status Notification for radio kill switch turned on", "Wireless hardware enabled"), KIcon("network-wireless").pixmap(QSize(iconSize,iconSize)), 0, KNotification::CloseOnTimeout, componentData());
    } else {
        Q_D(NotificationManager);
        d->newNetworkTimer->stop();
        d->newWirelessNetworks.clear();
        d->disappearedNetworkTimer->stop();
        d->disappearedWirelessNetworks.clear();
        KNotification::event(Event::RfOff, i18nc("@info:status Notification for radio kill switch turned off", "Wireless hardware disabled"), KIcon("network-wireless").pixmap(QSize(iconSize,iconSize)), 0, KNotification::CloseOnTimeout, componentData());
    }
}

void NotificationManager::statusChanged(Solid::Networking::Status status)
{
    if (status == Solid::Networking::Unknown) {
        KNotification::event(Event::NetworkingDisabled, i18nc("@info:status Notification when the networking subsystem (NetworkManager, etc) is disabled", "Networking system disabled"), QPixmap(), 0, KNotification::CloseOnTimeout, componentData());
    } else {
        Solid::Control::NetworkManagerNm09::Notifier * n = qobject_cast<Solid::Control::NetworkManagerNm09::Notifier *>(sender());

        /* If the signal does not come from a Solid::Control::NetworkManager::Notifier object then it is from a Monolithic Knm object. */
        if (n == NULL and status == Solid::Networking::Connected)
            KNotification::event(Event::AlreadyRunning, i18nc("@info:status Notification when the networking subsystem (NetworkManager, etc) is already running", "Networking system already running"), QPixmap(), 0, KNotification::CloseOnTimeout, componentData());
    }
}

void NotificationManager::performNotification(const QString &eventId, const QString &title, const QString &text, const QString &icon)
{
    KNotification::event(eventId, title, text, KIcon(icon).pixmap(QSize(iconSize,iconSize)), 0, KNotification::CloseOnTimeout, *s_networkManagementComponentData);
}
// vim: sw=4 sts=4 et tw=100
