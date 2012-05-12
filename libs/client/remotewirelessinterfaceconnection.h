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

#include <QtNetworkManager/accesspoint.h>
#include <QtNetworkManager/wirelessdevice.h>

#include "knmclient_export.h"

class RemoteWirelessInterfaceConnectionPrivate;

class KNMCLIENT_EXPORT RemoteWirelessInterfaceConnection : public RemoteInterfaceConnection, public RemoteWirelessObject
{
Q_OBJECT
Q_PROPERTY(QString ssid READ ssid NOTIFY ssidChanged)
Q_PROPERTY(int strength READ strength NOTIFY strengthChanged)
Q_PROPERTY(uint interfaceCapabilities READ interfaceCapabilities NOTIFY interfaceCapabilitiesChanged)
Q_PROPERTY(uint apCapabilities READ apCapabilities NOTIFY apCapabilitiesChanged)
Q_PROPERTY(uint wpaFlags READ wpaFlags NOTIFY wpaFlagsChanged)
Q_PROPERTY(uint rsnFlags READ rsnFlags NOTIFY rsnFlagsChanged)
Q_PROPERTY(uint operationMode READ operationMode NOTIFY operationModeChanged)

friend class RemoteActivatableList;

public:
    virtual ~RemoteWirelessInterfaceConnection();
    QString ssid() const;
    int strength() const;
    NetworkManager::WirelessDevice::Capabilities interfaceCapabilities() const;
    NetworkManager::AccessPoint::Capabilities apCapabilities() const;
    NetworkManager::AccessPoint::WpaFlags wpaFlags() const;
    NetworkManager::AccessPoint::WpaFlags rsnFlags() const;
    NetworkManager::WirelessDevice::OperationMode operationMode() const;
Q_SIGNALS:
    void strengthChanged(int);
    void ssidChanged();
    void interfaceCapabilitiesChanged();
    void apCapabilitiesChanged();
    void wpaFlagsChanged();
    void rsnFlagsChanged();
    void operationModeChanged();
protected Q_SLOTS:
    void wicPropertiesChanged(const QVariantMap &properties);
protected:
    RemoteWirelessInterfaceConnection(const QVariantMap & properties, QObject * parent);
    Q_DECLARE_PRIVATE(RemoteWirelessInterfaceConnection)
};

#endif // REMOTEWIRELESSINTERFACECONNECTION_H
