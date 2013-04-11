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

#ifndef WIRELESSOBJECT_H
#define WIRELESSOBJECT_H

#include <QString>
#include <QtNetworkManager/wirelessdevice.h>

#include "knminternals_export.h"

namespace Knm
{
class KNMINTERNALS_EXPORT WirelessObject
{
public:
    WirelessObject(const QString & ssid, int strength, NetworkManager::WirelessDevice::Capabilities interfaceCapabilities, NetworkManager::AccessPoint::Capabilities apCapabilities, NetworkManager::AccessPoint::WpaFlags wpaFlags, NetworkManager::AccessPoint::WpaFlags rsnFlags, NetworkManager::AccessPoint::OperationMode mode);
    virtual ~WirelessObject();
    QString ssid() const;
    int strength() const;
    virtual void setStrength(int strength);
    NetworkManager::WirelessDevice::Capabilities interfaceCapabilities() const;
    NetworkManager::AccessPoint::Capabilities apCapabilities() const;
    NetworkManager::AccessPoint::WpaFlags wpaFlags() const;
    NetworkManager::AccessPoint::WpaFlags rsnFlags() const;
    NetworkManager::AccessPoint::OperationMode operationMode() const;
    virtual void strengthChanged(int) = 0;

protected:
    WirelessObject();

protected:
    QString m_ssid;
    int m_strength;
    NetworkManager::WirelessDevice::Capabilities m_interfaceCapabilities;
    NetworkManager::AccessPoint::Capabilities m_apCapabilities;
    NetworkManager::AccessPoint::WpaFlags m_wpaFlags;
    NetworkManager::AccessPoint::WpaFlags m_rsnFlags;
    NetworkManager::AccessPoint::OperationMode m_operationMode;
};

} // namespace Knm

#endif // WIRELESSOBJECT_H
