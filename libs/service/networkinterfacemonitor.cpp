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

#include <QDBusPendingReply>

#include <KLocale>
#include <KMessageBox>
#include <kdeversion.h>
#include <libmm-qt/manager.h>

#include <connection.h>
#include "activatablelist.h"
#include "connectionlist.h"
#include "networkinterfaceactivatableprovider.h"
#include "wirednetworkinterfaceactivatableprovider.h"
#include "wirelessnetworkinterfaceactivatableprovider.h"
#include "gsmnetworkinterfaceactivatableprovider.h"
#include "pindialog.h"
#include "networkinterfacemonitor.h"

class NetworkInterfaceMonitorPrivate
{
public:
    // relates NetworkManager::Device identifiers to NetworkInterfaceActivatableProviders
    QHash<QString, NetworkInterfaceActivatableProvider *> providers;
    ConnectionList * connectionList;
    ActivatableList * activatableList;
};

NetworkInterfaceMonitor::NetworkInterfaceMonitor(ConnectionList * connectionList, ActivatableList * activatableList, QObject * parent)
    :QObject(parent), d_ptr(new NetworkInterfaceMonitorPrivate)
{
    Q_D(NetworkInterfaceMonitor);
    d->connectionList = connectionList;
    d->activatableList = activatableList;

    QObject::connect(NetworkManager::notifier(),
            SIGNAL(networkInterfaceAdded(const QString&)),
            this, SLOT(networkInterfaceAdded(const QString&)));
    QObject::connect(NetworkManager::notifier(),
            SIGNAL(networkInterfaceRemoved(const QString&)),
            this, SLOT(networkInterfaceRemoved(const QString&)));

    foreach (NetworkManager::Device * iface, NetworkManager::networkInterfaces()) {
        networkInterfaceAdded(iface->uni());
    }

    dialog = 0;
    QObject::connect(ModemManager::notifier(),
            SIGNAL(modemInterfaceAdded(const QString&)),
            this, SLOT(modemInterfaceAdded(const QString&)));

    foreach (ModemManager::ModemInterface * iface, ModemManager::modemInterfaces()) {
        modemInterfaceAdded(iface->udi());
    }
}

NetworkInterfaceMonitor::~NetworkInterfaceMonitor()
{
}

void NetworkInterfaceMonitor::networkInterfaceAdded(const QString & uni)
{
    Q_D(NetworkInterfaceMonitor);
    kDebug();

    NetworkManager::Device * iface = NetworkManager::findNetworkInterface(uni);
    if (iface && !d->providers.contains(uni)) {
        NetworkInterfaceActivatableProvider * provider;
        if (iface->type() == NetworkManager::Device::Wifi) {
            kDebug() << "Wireless interface added";
            provider = new WirelessNetworkInterfaceActivatableProvider(d->connectionList, d->activatableList, qobject_cast<NetworkManager::WirelessDevice*>(iface), this);
        } else if (iface->type() == NetworkManager::Device::Ethernet) {
            kDebug() << "Wired interface added";
            provider = new WiredNetworkInterfaceActivatableProvider(d->connectionList, d->activatableList, qobject_cast<NetworkManager::WiredDevice*>(iface), this);
        } else if (iface->type() == NetworkManager::Device::Bluetooth) {
            kDebug() << "Bluetooth interface added";
            provider = new GsmNetworkInterfaceActivatableProvider(d->connectionList, d->activatableList, qobject_cast<NetworkManager::ModemDevice*>(iface), this);
        } else if (iface->type() == NetworkManager::Device::Modem) {
            kDebug() << "Gsm interface added";
            provider = new GsmNetworkInterfaceActivatableProvider(d->connectionList, d->activatableList, qobject_cast<NetworkManager::ModemDevice*>(iface), this);
        } else {
            kDebug() << "Unknown interface added: uni == " << uni << "(type == " << iface->type() << ")";
            provider = new NetworkInterfaceActivatableProvider(d->connectionList, d->activatableList, iface, this);
        }
        d->connectionList->registerConnectionHandler(provider);
        d->providers.insert(uni, provider);
        provider->init();
    }
}

void NetworkInterfaceMonitor::networkInterfaceRemoved(const QString & uni)
{
    Q_D(NetworkInterfaceMonitor);
    NetworkInterfaceActivatableProvider * provider = d->providers.take(uni);
    d->connectionList->unregisterConnectionHandler(provider);
    delete provider;
}

void NetworkInterfaceMonitor::modemInterfaceAdded(const QString & udi)
{
    ModemManager::ModemGsmCardInterface * modem = qobject_cast<ModemManager::ModemGsmCardInterface *>(ModemManager::findModemInterface(udi, ModemManager::ModemInterface::GsmCard));

    if (!modem) {
        return;
    }

    connect(modem, SIGNAL(unlockRequiredChanged(const QString &)), SLOT(requestPin(const QString &)));

    if (dialog || modem->unlockRequired().isEmpty()) {
        return;
    }

    // Using queued invokation to prevent kded stalling here until user enter the pin.
    QMetaObject::invokeMethod(modem, "unlockRequiredChanged", Qt::QueuedConnection,
                              Q_ARG(QString, modem->unlockRequired()));
}

void NetworkInterfaceMonitor::requestPin(const QString & unlockRequired)
{
    kDebug() << "unlockRequired == " << unlockRequired;
    if (unlockRequired.isEmpty()) {
        return;
    }

    ModemManager::ModemGsmCardInterface * modem = qobject_cast<ModemManager::ModemGsmCardInterface *>(sender());
    if (!modem) {
        return;
    }

    if (dialog) {
        kDebug() << "PinDialog already running";
        return;
    }

    if (unlockRequired == QLatin1String("sim-pin")) {
        dialog = new PinDialog(modem, PinDialog::Pin);
    } else if (unlockRequired == QLatin1String("sim-puk")) {
        dialog = new PinDialog(modem, PinDialog::PinPuk);
    } else {
        kWarning() << "Unhandled unlock request for '" << unlockRequired << "'";
        return;
    }

    if (dialog->exec() != QDialog::Accepted) {
        goto OUT;
    }

    kDebug() << "Sending unlock code";

#if KDE_IS_VERSION(4,6,90)
    {
        QDBusPendingCallWatcher *watcher = 0;
    
        if (dialog->type() == PinDialog::Pin) {
            QDBusPendingCall reply = modem->sendPin(dialog->pin());
            watcher = new QDBusPendingCallWatcher(reply, modem);
        } else if (dialog->type() == PinDialog::PinPuk) {
            QDBusPendingCall reply = modem->sendPuk(dialog->puk(), dialog->pin());
            watcher = new QDBusPendingCallWatcher(reply, modem);
        }
    
        connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher *)), SLOT(onSendPinArrived(QDBusPendingCallWatcher *)));
    }
#else
    if (dialog->type() == PinDialog::Pin) {
        modem->sendPin(dialog->pin());
    } else if (dialog->type() == PinDialog::PinPuk) {
        modem->sendPuk(dialog->puk(), dialog->pin());
    }
#endif

OUT:
    delete dialog;
    dialog = 0;
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
