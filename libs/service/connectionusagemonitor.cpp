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

#include <solid/control/networkmanager.h>
#include <solid/control/wirelessnetworkinterface.h>
#include <solid/control/wirelessaccesspoint.h>

// libs/internals includes
#include "settings/802-11-wireless.h"
// libs/service includes
#include "interfaceconnection.h"
#include "activatablelist.h"
#include "connectionlist.h"

class ConnectionUsageMonitorPrivate
{
public:
    ConnectionList * connectionList;
    ActivatableList * activatableList;
};

ConnectionUsageMonitor::ConnectionUsageMonitor(ConnectionList * connectionList, ActivatableList * activatableList, QObject * parent)
: ActivatableObserver(parent), d_ptr(new ConnectionUsageMonitorPrivate)
{
    Q_D(ConnectionUsageMonitor);
    d->connectionList = connectionList;
    d->activatableList = activatableList;
    foreach (Knm::Activatable * activatable, d->activatableList->activatables()) {
        handleAdd(activatable);
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
        // listen to the IC
        connect(ic, SIGNAL(activationStateChanged(Knm::InterfaceConnection::ActivationState)), this, SLOT(handleActivationStateChange(Knm::InterfaceConnection::ActivationState)));
        }
}

void ConnectionUsageMonitor::handleUpdate(Knm::Activatable *)
{
}

void ConnectionUsageMonitor::handleRemove(Knm::Activatable *)
{
}

void ConnectionUsageMonitor::handleActivationStateChange(Knm::InterfaceConnection::ActivationState state)
{
    Q_D(ConnectionUsageMonitor);
    Knm::InterfaceConnection * ic = qobject_cast<Knm::InterfaceConnection*>(sender());
    if (ic) {
        if (state == Knm::InterfaceConnection::Activated) {
            Knm::Connection * connection = d->connectionList->findConnection(ic->connectionUuid());
            if (connection) {
                // update timestamp
                connection->setTimestamp(QDateTime::currentDateTime());
                // update with the BSSID of the device's AP
                Solid::Control::NetworkInterface * networkInterface
                    = Solid::Control::NetworkManager::findNetworkInterface(ic->deviceUni());
                if (networkInterface) {
                    if (networkInterface->type() == Solid::Control::NetworkInterface::Ieee80211) {
                        Solid::Control::WirelessNetworkInterface * wifiDevice =
                            qobject_cast<Solid::Control::WirelessNetworkInterface *>(networkInterface);

                        Solid::Control::AccessPoint * ap = wifiDevice->findAccessPoint(wifiDevice->activeAccessPoint());
                        Knm::WirelessSetting * ws
                            = static_cast<Knm::WirelessSetting * >(connection->setting(Knm::Setting::Wireless));

                        if (ws) {
                            QStringList seenBssids = ws->seenbssids();
                            if (!seenBssids.contains(ap->hardwareAddress())) {
                                seenBssids.append(ap->hardwareAddress());
                                ws->setSeenbssids(seenBssids);
                            }
                        }
                    }
                }
                d->connectionList->updateConnection(connection);
            }
        }
    }
}

// vim: sw=4 sts=4 et tw=100
