/*
Copyright 2009 Will Stephenson <wstephenson@kde.org>
Copyright 2010 Lamarque Souza <lamarque@gmail.com>

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

#ifdef COMPILE_MODEM_MANAGER_SUPPORT

#ifndef GSMNETWORKINTERFACEACTIVATABLEPROVIDER_H
#define GSMNETWORKINTERFACEACTIVATABLEPROVIDER_H

#include "networkinterfaceactivatableprovider.h"

class ConnectionList;
class ActivatableList;
class GsmNetworkInterfaceActivatableProviderPrivate;
namespace NetworkMangager
{
    class ModemDevice;
}

/**
 * Specialized Activatable provider for gsm interfaces
 * Encapsulates logic governing when and how to show Activatables for them.
 */
class KNM_EXPORT GsmNetworkInterfaceActivatableProvider : public NetworkInterfaceActivatableProvider
{
Q_OBJECT
public:
    GsmNetworkInterfaceActivatableProvider(ConnectionList * connectionList, ActivatableList * activatableList, NetworkManager::ModemDevice * interface, QObject * parent);
    virtual ~GsmNetworkInterfaceActivatableProvider();
public slots:
    /**
     * Create a GsmNetworkInterfaceConnection
     * @reimp NetworkInterfaceActivatableProvider
     */
    void handleAdd(Knm::Connection *);

private:
    Q_DECLARE_PRIVATE(GsmNetworkInterfaceActivatableProvider)
};
#endif // GSMNETWORKINTERFACEACTIVATABLEPROVIDER_H

#endif
