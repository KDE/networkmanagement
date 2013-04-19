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

#include "networkinterfacemonitor.h"

#include <QDBusPendingReply>

#include <KLocale>
#include <KMessageBox>
#include <kdeversion.h>
#include <ModemManagerQt/manager.h>

#include <connection.h>
#include "activatablelist.h"
#include "connectionlist.h"
#include "networkinterfaceactivatableprovider.h"
#include "wirednetworkinterfaceactivatableprovider.h"
#include "wirelessnetworkinterfaceactivatableprovider.h"
#include "gsmnetworkinterfaceactivatableprovider.h"
#include "pindialog.h"
#include "knmserviceprefs.h"

class NetworkInterfaceMonitorPrivate
{
public:
    // relates NetworkManager::Device identifiers to NetworkInterfaceActivatableProviders
    QHash<QString, NetworkInterfaceActivatableProvider *> providers;
    ConnectionList * connectionList;
    ActivatableList * activatableList;
    QWeakPointer<PinDialog> dialog;
};

NetworkInterfaceMonitor::NetworkInterfaceMonitor(ConnectionList * connectionList, ActivatableList * activatableList, QObject * parent)
    :QObject(parent), d_ptr(new NetworkInterfaceMonitorPrivate)
{
    Q_D(NetworkInterfaceMonitor);
    d->connectionList = connectionList;
    d->activatableList = activatableList;
    d->dialog.clear();

    QObject::connect(NetworkManager::notifier(),
            SIGNAL(deviceAdded(QString)),
            this, SLOT(deviceAdded(QString)));
    QObject::connect(NetworkManager::notifier(),
            SIGNAL(deviceRemoved(QString)),
            this, SLOT(deviceRemoved(QString)));

    foreach (const NetworkManager::Device::Ptr &iface, NetworkManager::networkInterfaces()) {
        deviceAdded(iface->uni());
    }

    QObject::connect(ModemManager::notifier(),
            SIGNAL(modemAdded(QString)),
            this, SLOT(modemAdded(QString)));

    foreach (const ModemManager::ModemInterface::Ptr &iface, ModemManager::modemInterfaces()) {
        modemAdded(iface->udi());
    }
}

NetworkInterfaceMonitor::~NetworkInterfaceMonitor()
{
}

void NetworkInterfaceMonitor::deviceAdded(const QString & uni)
{
    Q_D(NetworkInterfaceMonitor);
    kDebug();

    NetworkManager::Device::Ptr iface = NetworkManager::findNetworkInterface(uni);
    if (iface && !d->providers.contains(uni)) {
        NetworkInterfaceActivatableProvider * provider;
        if (iface->type() == NetworkManager::Device::Wifi) {
            kDebug() << "Wireless interface added";
            provider = new WirelessNetworkInterfaceActivatableProvider(d->connectionList, d->activatableList, iface.objectCast<NetworkManager::WirelessDevice>(), this);
        } else if (iface->type() == NetworkManager::Device::Ethernet) {
            kDebug() << "Wired interface added";
            provider = new WiredNetworkInterfaceActivatableProvider(d->connectionList, d->activatableList, iface.objectCast<NetworkManager::WiredDevice>(), this);
        } else if (iface->type() == NetworkManager::Device::Bluetooth) {
            kDebug() << "Bluetooth interface added";
            provider = new GsmNetworkInterfaceActivatableProvider(d->connectionList, d->activatableList, iface.objectCast<NetworkManager::ModemDevice>(), this);
        } else if (iface->type() == NetworkManager::Device::Modem) {
            kDebug() << "Gsm interface added";
            provider = new GsmNetworkInterfaceActivatableProvider(d->connectionList, d->activatableList, iface.objectCast<NetworkManager::ModemDevice>(), this);
        } else {
            kDebug() << "Unknown interface added: uni == " << uni << "(type == " << iface->type() << ")";
            provider = new NetworkInterfaceActivatableProvider(d->connectionList, d->activatableList, iface, this);
        }
        d->connectionList->registerConnectionHandler(provider);
        d->providers.insert(uni, provider);
        provider->init();
    }
}

void NetworkInterfaceMonitor::deviceRemoved(const QString & uni)
{
    Q_D(NetworkInterfaceMonitor);
    NetworkInterfaceActivatableProvider * provider = d->providers.take(uni);
    d->connectionList->unregisterConnectionHandler(provider);
    provider->deleteLater();
}

void NetworkInterfaceMonitor::modemAdded(const QString & udi)
{
    Q_D(NetworkInterfaceMonitor);
    ModemManager::ModemGsmCardInterface::Ptr modem;
    modem = ModemManager::findModemInterface(udi, ModemManager::ModemInterface::GsmCard).objectCast<ModemManager::ModemGsmCardInterface>();

    if (!modem) {
        return;
    }

    connect(modem.data(), SIGNAL(unlockRequiredChanged(QString)), SLOT(requestPin(QString)));

    if (d->dialog || modem->unlockRequired().isEmpty()) {
        return;
    }

    KNetworkManagerServicePrefs::self()->readConfig();
    if (KNetworkManagerServicePrefs::self()->askForGsmPin() != KNetworkManagerServicePrefs::OnModemDetection) {
        return;
    }

    // Using queued invocation to prevent kded stalling here until user enters the pin.
    QMetaObject::invokeMethod(modem.data(), "unlockRequiredChanged", Qt::QueuedConnection,
                              Q_ARG(QString, modem->unlockRequired()));
}

void NetworkInterfaceMonitor::requestPin(const QString & unlockRequired)
{
    Q_D(NetworkInterfaceMonitor);
    kDebug() << "unlockRequired == " << unlockRequired;
    if (unlockRequired.isEmpty()) {
        return;
    }

    ModemManager::ModemGsmCardInterface * modem = qobject_cast<ModemManager::ModemGsmCardInterface *>(sender());
    if (!modem) {
        return;
    }

    if (d->dialog) {
        kDebug() << "PinDialog already running";
        return;
    }

    if (unlockRequired == QLatin1String("sim-pin")) {
        d->dialog = new PinDialog(modem, PinDialog::Pin);
    } else if (unlockRequired == QLatin1String("sim-puk")) {
        d->dialog = new PinDialog(modem, PinDialog::PinPuk);
    } else {
        kWarning() << "Unhandled unlock request for '" << unlockRequired << "'";
        return;
    }

    if (d->dialog.data()->exec() != QDialog::Accepted) {
        goto OUT;
    }

    kDebug() << "Sending unlock code";

    {
        QDBusPendingCallWatcher *watcher = 0;
    
        if (d->dialog.data()->type() == PinDialog::Pin) {
            QDBusPendingCall reply = modem->sendPin(d->dialog.data()->pin());
            watcher = new QDBusPendingCallWatcher(reply, modem);
        } else if (d->dialog.data()->type() == PinDialog::PinPuk) {
            QDBusPendingCall reply = modem->sendPuk(d->dialog.data()->puk(), d->dialog.data()->pin());
            watcher = new QDBusPendingCallWatcher(reply, modem);
        }
    
        connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), SLOT(onSendPinArrived(QDBusPendingCallWatcher*)));
    }

OUT:
    if(d->dialog) {
        d->dialog.data()->deleteLater();
    }
    d->dialog.clear();
}

void NetworkInterfaceMonitor::onSendPinArrived(QDBusPendingCallWatcher * watcher)
{
   QDBusPendingReply<> reply = *watcher;

    if (reply.isValid()) {
        // Automatically enabling this for cell phones with expensive data plans is not a good idea.
        //NetworkManager::setWwanEnabled(true);
    } else {
        KMessageBox::error(0, i18nc("Text in GSM PIN/PUK unlock error dialog", "Error unlocking modem: %1", reply.error().message()), i18nc("Title for GSM PIN/PUK unlock error dialog", "PIN/PUK unlock error"));
    }

    watcher->deleteLater();
}

// vim: sw=4 sts=4 et tw=100
