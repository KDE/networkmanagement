/*
Copyright 2009 Will Stephenson <wstephenson@kde.org>
Copyright 2012 Lamarque V. Souza <lamarque@kde.org>

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

#ifndef REMOTEWIRELESSINTERFACECONNECTION_H
#define REMOTEWIRELESSINTERFACECONNECTION_H

#include "remoteinterfaceconnection.h"
#include "remotewirelessobject.h"

#include <solid/control/wirelessaccesspoint.h>
#include <solid/control/wirelessnetworkinterface.h>

#include "knmclient_export.h"

class RemoteWirelessInterfaceConnectionPrivate;

class KNMCLIENT_EXPORT RemoteWirelessInterfaceConnection : public RemoteInterfaceConnection, public RemoteWirelessObject
{
Q_OBJECT
Q_PROPERTY(QString ssid READ ssid)
Q_PROPERTY(int strength READ strength)
Q_PROPERTY(uint interfaceCapabilities READ interfaceCapabilities)
Q_PROPERTY(uint apCapabilities READ apCapabilities)
Q_PROPERTY(uint wpaFlags READ wpaFlags)
Q_PROPERTY(uint rsnFlags READ rsnFlags)
Q_PROPERTY(uint operationMode READ operationMode)

friend class RemoteActivatableList;

public:
    virtual ~RemoteWirelessInterfaceConnection();
    QString ssid() const;
    int strength() const;
    Solid::Control::WirelessNetworkInterfaceNm09::Capabilities interfaceCapabilities() const;
    Solid::Control::AccessPointNm09::Capabilities apCapabilities() const;
    Solid::Control::AccessPointNm09::WpaFlags wpaFlags() const;
    Solid::Control::AccessPointNm09::WpaFlags rsnFlags() const;
    Solid::Control::WirelessNetworkInterfaceNm09::OperationMode operationMode() const;
Q_SIGNALS:
    void strengthChanged(int);
protected Q_SLOTS:
    void wicPropertiesChanged(const QVariantMap &properties);
protected:
    RemoteWirelessInterfaceConnection(const QVariantMap & properties, QObject * parent);
    Q_DECLARE_PRIVATE(RemoteWirelessInterfaceConnection)
};

#endif // REMOTEWIRELESSINTERFACECONNECTION_H
