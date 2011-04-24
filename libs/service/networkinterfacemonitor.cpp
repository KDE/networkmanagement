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

#include <KToolInvocation>
#include <KStandardDirs>

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
#include "gsmnetworkinterfaceactivatableprovider.h"
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
            SIGNAL(networkInterfaceAdded(const QString&)),
            this, SLOT(networkInterfaceAdded(const QString&)));
    QObject::connect(Solid::Control::NetworkManager::notifier(),
            SIGNAL(networkInterfaceRemoved(const QString&)),
            this, SLOT(networkInterfaceRemoved(const QString&)));

    foreach (Solid::Control::NetworkInterface * iface, Solid::Control::NetworkManager::networkInterfaces()) {
        networkInterfaceAdded(iface->uni());
    }
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
        } else if (iface->type() == Solid::Control::NetworkInterface::Bluetooth) {
            kDebug() << "Bluetooth interface added";
            provider = new GsmNetworkInterfaceActivatableProvider(d->connectionList, d->activatableList, qobject_cast<Solid::Control::GsmNetworkInterface*>(iface), this);
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

#ifdef COMPILE_MODEM_MANAGER_SUPPORT
        if (iface->type() == Solid::Control::NetworkInterface::Gsm ||
            iface->type() == Solid::Control::NetworkInterface::Cdma) {

            bool hasCellular = false;
            foreach (const QString uuid, d->connectionList->connections()) {
                const Knm::Connection *c = d->connectionList->findConnection(uuid);
                if ((c->type() == Knm::Connection::Gsm && iface->type() == Solid::Control::NetworkInterface::Gsm) ||
                    (c->type() == Knm::Connection::Cdma && iface->type() == Solid::Control::NetworkInterface::Cdma)) {
                    hasCellular = true;
                    break;
                }
            }

            if (!hasCellular) {
                QStringList args;
                args << QLatin1String("create") << QLatin1String("--type") << QLatin1String("cellular");
                KToolInvocation::kdeinitExec(KGlobal::dirs()->findResource("exe", "networkmanagement_configshell"), args);
            }
        }
#endif
    }
}

void NetworkInterfaceMonitor::networkInterfaceRemoved(const QString & uni)
{
    Q_D(NetworkInterfaceMonitor);
    NetworkInterfaceActivatableProvider * provider = d->providers.take(uni);
    d->connectionList->unregisterConnectionHandler(provider);
    delete provider;
}

// vim: sw=4 sts=4 et tw=100
