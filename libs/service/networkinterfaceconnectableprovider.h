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

#ifndef NETWORKINTERFACECONNECTABLEPROVIDER_H
#define NETWORKINTERFACECONNECTABLEPROVIDER_H

#include <QObject>
#include "connectionhandler.h"

#include <solid/control/networkinterface.h>
#include <solid/control/wirednetworkinterface.h>
#include <solid/control/wirelessnetworkinterface.h>

#include "connection.h"
#include "setting.h"
#include "settings/802-3-ethernet.h"
#include "settings/802-11-wireless.h"


#include "knm_export.h"

namespace Knm
{
    class Connectable;
} // namespace Knm

namespace Solid
{
    namespace Control
    {
        class NetworkInterface;
    } // namespace Control
} // namespace Solid

class ConnectionList;
class ConnectableList;

class NetworkInterfaceConnectableProviderPrivate;
/**
 * Monitors a network interface and the connection list and provides and maintains InterfaceConnections for it to the
 * ConnectableList
 */
class KNM_EXPORT NetworkInterfaceConnectableProvider : public QObject, public ConnectionHandler
{
Q_OBJECT
Q_DECLARE_PRIVATE(NetworkInterfaceConnectableProvider)
public:
    static bool matches(Knm::Connection::Type connType, Solid::Control::NetworkInterface::Type ifaceType);
    static bool hardwareAddressMatches(Knm::Connection * connection, Solid::Control::NetworkInterface * iface);

    NetworkInterfaceConnectableProvider(ConnectionList * connectionList, ConnectableList * connectableList, Solid::Control::NetworkInterface * interface, QObject * parent);
    virtual ~NetworkInterfaceConnectableProvider();
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
protected:
    NetworkInterfaceConnectableProviderPrivate * d_ptr;
};

#endif // NETWORKINTERFACECONNECTABLEPROVIDER_H
