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

#include <KLocale>

#include "networkinterfacemonitor.h"

#include <solid/control/networkinterface.h>
#include <solid/control/networkmanager.h>

#include <connection.h>
#include "activatablelist.h"
#include "connectionlist.h"
#include "networkinterfaceactivatableprovider.h"
#include "wirednetworkinterfaceactivatableprovider.h"
#include "wirelessnetworkinterfaceactivatableprovider.h"

#ifdef COMPILE_MODEM_MANAGER_SUPPORT
#include <solid/control/modemmanager.h>

#include "gsmnetworkinterfaceactivatableprovider.h"
#include "pindialog.h"
#endif

class NetworkInterfaceMonitorPrivate
{
public:
    // relates Solid::Control::NetworkInterface identifiers to NetworkInterfaceActivatableProviders
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

    QObject::connect(Solid::Control::NetworkManager::notifier(),
            SIGNAL(networkInterfaceAdded(QString)),
            this, SLOT(networkInterfaceAdded(QString)));
    QObject::connect(Solid::Control::NetworkManager::notifier(),
            SIGNAL(networkInterfaceRemoved(QString)),
            this, SLOT(networkInterfaceRemoved(QString)));

    foreach (Solid::Control::NetworkInterface * iface, Solid::Control::NetworkManager::networkInterfaces()) {
        networkInterfaceAdded(iface->uni());
    }

#ifdef COMPILE_MODEM_MANAGER_SUPPORT
    dialog = 0;
    QObject::connect(Solid::Control::ModemManager::notifier(),
            SIGNAL(modemInterfaceAdded(QString)),
            this, SLOT(modemInterfaceAdded(QString)));

    foreach (Solid::Control::ModemInterface * iface, Solid::Control::ModemManager::modemInterfaces()) {
        modemInterfaceAdded(iface->udi());
    }
#endif
}

NetworkInterfaceMonitor::~NetworkInterfaceMonitor()
{
}

void NetworkInterfaceMonitor::networkInterfaceAdded(const QString & uni)
{
    Q_D(NetworkInterfaceMonitor);
    kDebug();

    Solid::Control::NetworkInterface * iface = Solid::Control::NetworkManager::findNetworkInterface(uni);
    if (iface && !d->providers.contains(uni)) {
        NetworkInterfaceActivatableProvider * provider;
        if (iface->type() == Solid::Control::NetworkInterface::Ieee80211) {
            kDebug() << "Wireless interface added";
            provider = new WirelessNetworkInterfaceActivatableProvider(d->connectionList, d->activatableList, qobject_cast<Solid::Control::WirelessNetworkInterface*>(iface), this);
        } else if (iface->type() == Solid::Control::NetworkInterface::Ieee8023) {
            kDebug() << "Wired interface added";
            provider = new WiredNetworkInterfaceActivatableProvider(d->connectionList, d->activatableList, qobject_cast<Solid::Control::WiredNetworkInterface*>(iface), this);
#ifdef COMPILE_MODEM_MANAGER_SUPPORT
#ifdef NM_0_8
        } else if (iface->type() == Solid::Control::NetworkInterface::Bluetooth) {
            kDebug() << "Bluetooth interface added";
            provider = new GsmNetworkInterfaceActivatableProvider(d->connectionList, d->activatableList, qobject_cast<Solid::Control::GsmNetworkInterface*>(iface), this);
#endif
        } else if (iface->type() == Solid::Control::NetworkInterface::Gsm) {
            kDebug() << "Gsm interface added";
            provider = new GsmNetworkInterfaceActivatableProvider(d->connectionList, d->activatableList, qobject_cast<Solid::Control::GsmNetworkInterface*>(iface), this);
#endif
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

#ifdef COMPILE_MODEM_MANAGER_SUPPORT
void NetworkInterfaceMonitor::modemInterfaceAdded(const QString & udi)
{
    Solid::Control::ModemGsmCardInterface * modem = qobject_cast<Solid::Control::ModemGsmCardInterface *>(Solid::Control::ModemManager::findModemInterface(udi, Solid::Control::ModemInterface::GsmCard));

    if (!modem) {
        return;
    }

    connect(modem, SIGNAL(unlockRequiredChanged(QString)), SLOT(requestPin(QString)));

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

    Solid::Control::ModemGsmCardInterface * modem = qobject_cast<Solid::Control::ModemGsmCardInterface *>(sender());
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
    if (dialog->type() == PinDialog::Pin) {
        modem->sendPin(dialog->pin());
    } else if (dialog->type() == PinDialog::PinPuk) {
        modem->sendPuk(dialog->puk(), dialog->pin());
    }

OUT:
    delete dialog;
    dialog = 0;
}
#endif
// vim: sw=4 sts=4 et tw=100
