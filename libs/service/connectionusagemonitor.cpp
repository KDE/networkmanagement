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

#include "connectionusagemonitor.h"

#include <KDebug>

#include <NetworkManagerQt/manager.h>
#include <NetworkManagerQt/wirelessdevice.h>
#include <NetworkManagerQt/accesspoint.h>

// libs/internals includes
#include "settings/802-11-wireless.h"
// libs/service includes
#include "wirelessinterfaceconnection.h"
#include "activatablelist.h"
#include "connectionlist.h"


class ConnectionUsageMonitorPrivate
{
public:
    ConnectionList * connectionList;
    ActivatableList * activatableList;
};

ConnectionUsageMonitor::ConnectionUsageMonitor(ConnectionList * connectionList, ActivatableList * activatableList, QObject * parent)
: QObject(parent), d_ptr(new ConnectionUsageMonitorPrivate)
{
    Q_D(ConnectionUsageMonitor);
    d->connectionList = connectionList;
    d->activatableList = activatableList;

    QObject::connect(NetworkManager::notifier(), SIGNAL(deviceAdded(QString)),
            this, SLOT(deviceAdded(QString)));

    NetworkManager::Device::List allInterfaces = NetworkManager::networkInterfaces();
    foreach (const NetworkManager::Device::Ptr &interface, allInterfaces) {
        deviceAdded(interface->uni());
    }
}

ConnectionUsageMonitor::~ConnectionUsageMonitor()
{
    delete d_ptr;
}

void ConnectionUsageMonitor::handleAdd(Knm::Activatable * added)
{
    Knm::InterfaceConnection * ic = qobject_cast<Knm::InterfaceConnection*>(added);
    if (ic) {
        connect(ic, SIGNAL(activationStateChanged(Knm::InterfaceConnection::ActivationState,Knm::InterfaceConnection::ActivationState)),
                this, SLOT(handleActivationStateChange(Knm::InterfaceConnection::ActivationState,Knm::InterfaceConnection::ActivationState)));
    }
}

void ConnectionUsageMonitor::handleUpdate(Knm::Activatable *)
{
}

void ConnectionUsageMonitor::handleRemove(Knm::Activatable *)
{
}

void ConnectionUsageMonitor::handleActivationStateChange(Knm::InterfaceConnection::ActivationState oldState, Knm::InterfaceConnection::ActivationState newState)
{
    Q_UNUSED(oldState)
    Q_D(ConnectionUsageMonitor);
    Knm::InterfaceConnection * ic = qobject_cast<Knm::InterfaceConnection*>(sender());
    if (ic) {
        if (newState == Knm::InterfaceConnection::Activated) {
            Knm::Connection * connection = d->connectionList->findConnection(ic->connectionUuid());
            if (connection) {
                // update timestamp
                connection->setTimestamp(QDateTime::currentDateTime());
                // update with the BSSID of the device's AP
                NetworkManager::Device::Ptr networkInterface
                    = NetworkManager::findNetworkInterface(ic->deviceUni());
                if (networkInterface) {
                    if (networkInterface->type() == NetworkManager::Device::Wifi) {
                        NetworkManager::WirelessDevice::Ptr wifiDevice =
                            networkInterface.objectCast<NetworkManager::WirelessDevice>();

                        NetworkManager::AccessPoint::Ptr ap = wifiDevice->activeAccessPoint();
                        if (ap) {
                            Knm::WirelessSetting * ws
                                = static_cast<Knm::WirelessSetting * >(connection->setting(Knm::Setting::Wireless));

                            if (ws) {
                                QStringList seenBssids = ws->seenbssids();
                                if (!seenBssids.contains(ap->hardwareAddress()) && !ap->hardwareAddress().isEmpty()) {
                                    seenBssids.append(ap->hardwareAddress());
                                    ws->setSeenbssids(seenBssids);
                                }
                            }
                        }
                    }
                }
                d->connectionList->updateConnection(connection);
            }
        }
    }
}

void ConnectionUsageMonitor::deviceAdded(const QString& uni)
{
    NetworkManager::Device::Ptr interface = NetworkManager::findNetworkInterface(uni);

    if (interface && interface->type() == NetworkManager::Device::Wifi) {
        NetworkManager::WirelessDevice::Ptr wifiDevice =
            interface.objectCast<NetworkManager::WirelessDevice>();
        if (wifiDevice)
            connect(wifiDevice.data(), SIGNAL(activeAccessPointChanged(QString)),
                    this, SLOT(networkInterfaceAccessPointChanged(QString)));
    }
}

void ConnectionUsageMonitor::networkInterfaceAccessPointChanged(const QString & apiUni)
{
    Q_D(ConnectionUsageMonitor);
    NetworkManager::WirelessDevice * wifiDevice = qobject_cast<NetworkManager::WirelessDevice *>(sender());
    if (wifiDevice && static_cast<NetworkManager::Device::State>(wifiDevice->state()) == NetworkManager::Device::Activated) {
        NetworkManager::AccessPoint::Ptr ap = wifiDevice->findAccessPoint(apiUni);
        if (ap) {
            // find the activatable
            foreach (Knm::Activatable * activatable, d->activatableList->activatables()) {
                Knm::WirelessInterfaceConnection * ic = qobject_cast<Knm::WirelessInterfaceConnection*>(activatable);
                if (ic) {
                    if (ic->activationState() == Knm::InterfaceConnection::Activated && ic->deviceUni() == wifiDevice->uni()) {
                        // find the connection
                        Knm::Connection * connection = d->connectionList->findConnection(ic->connectionUuid());
                        if (connection) {
                            if (connection->type() == Knm::Connection::Wireless) {
                                Knm::WirelessSetting * ws = static_cast<Knm::WirelessSetting * >(connection->setting(Knm::Setting::Wireless));

                                if (ws) {
                                    if (ws->ssid() == ap->ssid()) {
                                        QStringList seenBssids = ws->seenbssids();
                                        if (!seenBssids.contains(ap->hardwareAddress())) {
                                            seenBssids.append(ap->hardwareAddress());
                                            ws->setSeenbssids(seenBssids);
                                            //kDebug() << "Updating connection" << connection->uuid() << "with" << seenBssids;
                                            d->connectionList->updateConnection(connection);
                                        }
                                    } else {
                                        kDebug() << "SSIDs do not match!" << ws->ssid() << ap->ssid();
                                    }
                                }
                            } else {
                                kDebug() << "connection not wireless!";
                            }
                        } else {
                            kDebug() << "connection not found";
                        }
                    }
                }
            }
        }
    }
}

// vim: sw=4 sts=4 et tw=100
