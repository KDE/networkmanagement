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

#ifndef WIREDNETWORKINTERFACEACTIVATABLEPROVIDER_H
#define WIREDNETWORKINTERFACEACTIVATABLEPROVIDER_H

#include "networkinterfaceactivatableprovider.h"

class ConnectionList;
class ActivatableList;
class WiredNetworkInterfaceActivatableProviderPrivate;
namespace Solid
{
    namespace Control
    {
        class WiredNetworkInterfaceNm09;
    } // namespace Control
} // namespace Solid

/**
 * Specialized Activatable provider for wireless interfaces
 * Encapsulates logic governing when and how to show Activatables for them.
 */
class KNM_EXPORT WiredNetworkInterfaceActivatableProvider : public NetworkInterfaceActivatableProvider
{
Q_OBJECT
public:
    WiredNetworkInterfaceActivatableProvider(ConnectionList * connectionList, ActivatableList * activatableList, Solid::Control::WiredNetworkInterfaceNm09 * interface, QObject * parent);
    virtual ~WiredNetworkInterfaceActivatableProvider();
public slots:
    /**
     * If the connection is relevant to this network interface, and the wireless network is present,
     * create a WiredNetworkInterfaceConnection
     * @reimp NetworkInterfaceActivatableProvider
     */
    void handleAdd(Knm::Connection *);
protected Q_SLOTS:
    /**
     * Handle changes in the carrier state of this network interface by adding or removing
     * InterfaceConnections
     */
    void handleCarrierChange(bool);
protected:
    virtual bool needsActivatableForUnconfigured() const;
private:
    Q_DECLARE_PRIVATE(WiredNetworkInterfaceActivatableProvider)
};
#endif // WIREDNETWORKINTERFACEACTIVATABLEPROVIDER_H
