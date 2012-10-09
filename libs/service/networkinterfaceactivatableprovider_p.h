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
#ifndef NETWORKINTERFACEACTIVATABLEPROVIDER_P_H
#define NETWORKINTERFACEACTIVATABLEPROVIDER_P_H

#include "networkinterfaceactivatableprovider.h"

#include <QMultiHash>
#include <QPointer>

namespace Knm
{
    class InterfaceConnection;
    class UnconfiguredInterface;
} // namespace Knm

class NetworkInterfaceActivatableProviderPrivate
{
Q_DECLARE_PUBLIC(NetworkInterfaceActivatableProvider)
public:
    NetworkInterfaceActivatableProviderPrivate(ConnectionList * connectionList, ActivatableList * activatableList, Solid::Control::NetworkInterfaceNm09 * interface);
    virtual ~NetworkInterfaceActivatableProviderPrivate();
    QPointer<Solid::Control::NetworkInterfaceNm09> interface;
    ConnectionList * connectionList;
    QPointer<ActivatableList> activatableList;
    // map connection UUID to activatable
    QMultiHash<QString, Knm::InterfaceConnection*> activatables;
    // special activatable for devices with no connections
    Knm::UnconfiguredInterface * unconfiguredActivatable;
protected:
    NetworkInterfaceActivatableProvider *q_ptr;
};

#endif // NETWORKINTERFACEACTIVATABLEPROVIDER_P_H
