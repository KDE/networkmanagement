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

#ifndef VPNINTERFACECONNECTIONPROVIDER_H
#define VPNINTERFACECONNECTIONPROVIDER_H

#include <QObject>

#include <Solid/Networking>

#include "connectionhandler.h"

#include "knm_export.h"

class ConnectionList;
class ActivatableList;

class VpnInterfaceConnectionProviderPrivate;

class KNM_EXPORT VpnInterfaceConnectionProvider : public QObject, public ConnectionHandler
{
Q_OBJECT
Q_DECLARE_PRIVATE(VpnInterfaceConnectionProvider)

public:
    VpnInterfaceConnectionProvider(ConnectionList * connectionList, ActivatableList * activatableList, QObject * parent = 0);
    virtual ~VpnInterfaceConnectionProvider();

    void init();

    void handleAdd(Knm::Connection *);
    void handleUpdate(Knm::Connection *);
    void handleRemove(Knm::Connection *);
protected Q_SLOTS:
    void statusChanged(Solid::Networking::Status status);
private:
    VpnInterfaceConnectionProviderPrivate * d_ptr;
};

#endif // VPNINTERFACECONNECTIONPROVIDER_H
