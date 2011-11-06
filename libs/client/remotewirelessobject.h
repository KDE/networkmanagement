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

#ifndef REMOTEWIRELESSOBJECT_H
#define REMOTEWIRELESSOBJECT_H

#include <solid/control/wirelessnetworkinterface.h>
#include <solid/control/wirelessaccesspoint.h>

class RemoteWirelessObject
{
public:
    virtual ~RemoteWirelessObject();
    virtual QString ssid() const = 0;
    virtual int strength() const = 0;
    virtual Solid::Control::WirelessNetworkInterfaceNm09::Capabilities interfaceCapabilities() const = 0;
    virtual Solid::Control::AccessPointNm09::Capabilities apCapabilities() const = 0;
    virtual Solid::Control::AccessPointNm09::WpaFlags wpaFlags() const = 0;
    virtual Solid::Control::AccessPointNm09::WpaFlags rsnFlags() const = 0;
    virtual Solid::Control::WirelessNetworkInterfaceNm09::OperationMode operationMode() const = 0;
    virtual void strengthChanged(int) = 0;
};

#endif // REMOTEWIRELESSOBJECT_H
