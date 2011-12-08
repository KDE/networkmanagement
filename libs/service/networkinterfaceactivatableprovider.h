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

#ifndef NETWORKINTERFACEACTIVATABLEPROVIDER_H
#define NETWORKINTERFACEACTIVATABLEPROVIDER_H

#include <QObject>
#include "connectionhandler.h"

#include <QtNetworkManager/device.h>
#include <QtNetworkManager/wireddevice.h>
#include <QtNetworkManager/wirelessdevice.h>
#include <QtNetworkManager/modemdevice.h>

#include "connection.h"
#include "setting.h"
#include "settings/802-3-ethernet.h"
#include "settings/802-11-wireless.h"


#include "knm_export.h"

namespace Knm
{
    class Activatable;
} // namespace Knm

namespace NetworkManager
{
    class Device;
} // namespace NetworkManager

class ConnectionList;
class ActivatableList;

class NetworkInterfaceActivatableProviderPrivate;
/**
 * Monitors a network interface and the connection list and provides and maintains InterfaceConnections for it to the
 * ActivatableList
 */
class KNM_EXPORT NetworkInterfaceActivatableProvider : public QObject, virtual public ConnectionHandler
{
Q_OBJECT
public:
    static bool hardwareAddressMatches(Knm::Connection * connection, NetworkManager::Device * iface);
    static bool matches(Knm::Connection::Type connType, NetworkManager::Device::Type ifaceType, NetworkManager::ModemDevice::Capabilities modemCaps = NetworkManager::ModemDevice::NoCapability);

    NetworkInterfaceActivatableProvider(ConnectionList * connectionList, ActivatableList * activatableList, NetworkManager::Device * interface, QObject * parent);
    virtual ~NetworkInterfaceActivatableProvider();

    /**
     * Sets up initial activatable list
     */
    virtual void init();

    /**
     * @reimp ConnectionHandler
     */
    virtual void handleAdd(Knm::Connection *);
    /**
     * @reimp ConnectionHandler
     */
    virtual void handleUpdate(Knm::Connection *);
    /**
     * @reimp ConnectionHandler
     */
    virtual void handleRemove(Knm::Connection *);

protected Q_SLOTS:
    void _k_destroyed(QObject *);

protected:
    NetworkInterfaceActivatableProvider(NetworkInterfaceActivatableProviderPrivate& dd, QObject * parent);
    NetworkInterfaceActivatableProviderPrivate * d_ptr;
    /**
     * Adds or removes a dummy activatable for unconfigured devices
     */
    void maintainActivatableForUnconfigured();
    /**
     * Indicate if the requirements for activating a connection are present
     */
    virtual bool needsActivatableForUnconfigured() const;

private:
    Q_DECLARE_PRIVATE(NetworkInterfaceActivatableProvider)
};

#endif // NETWORKINTERFACEACTIVATABLEPROVIDER_H
