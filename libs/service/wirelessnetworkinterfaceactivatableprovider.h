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

#ifndef WIRELESSNETWORKINTERFACEACTIVATABLEPROVIDER_H
#define WIRELESSNETWORKINTERFACEACTIVATABLEPROVIDER_H

#include "networkinterfaceactivatableprovider.h"

class ConnectionList;
class ActivatableList;
class WirelessNetworkInterfaceActivatableProviderPrivate;
namespace Solid
{
    namespace Control
    {
        class WirelessNetworkInterfaceNm09;
    } // namespace Control
} // namespace Solid

/**
 * Specialized Activatable provider for wireless interfaces
 * Encapsulates logic governing when and how to show Activatables for them.
 */
class KNM_EXPORT WirelessNetworkInterfaceActivatableProvider : public NetworkInterfaceActivatableProvider
{
Q_OBJECT
public:
    WirelessNetworkInterfaceActivatableProvider(ConnectionList * connectionList, ActivatableList * activatableList, Solid::Control::WirelessNetworkInterfaceNm09 * interface, QObject * parent);
    virtual ~WirelessNetworkInterfaceActivatableProvider();
public slots:
    /**
     * If the connection is relevant to this network interface, and the wireless network is present,
     * create a WirelessNetworkInterfaceConnection
     * @reimp NetworkInterfaceActivatableProvider
     */
    void handleAdd(Knm::Connection *);

    /**
     * remove any references we hold dependent on this connection and create a WirelessNetwork
     * if its network is visible.
     * @reimp NetworkInterfaceActivatableProvider
     */
    void handleRemove(Knm::Connection *);

    /**
     * When a wireless network appears, check if the ConnectionList has a connection for it.
     * If so, create a WirelessNetworkInterfaceConnection.
     * Otherwise, create a WirelessNetwork
     */

    void networkAppeared(const QString & ssid);

    /**
     * Remove any Activatables related to this network
     */
    void networkDisappeared(const QString & ssid);
    void wirelessEnabledChanged(bool);
protected:
    virtual bool needsActivatableForUnconfigured() const;
private:
    Q_DECLARE_PRIVATE(WirelessNetworkInterfaceActivatableProvider)
};
#endif // WIRELESSNETWORKINTERFACEACTIVATABLEPROVIDER_H
