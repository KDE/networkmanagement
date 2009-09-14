/*
Copyright 2008 Frederik Gladhorn <gladhorn@kde.org>

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

#include "wirelessnetwork.h"

using namespace Knm;

WirelessNetwork::WirelessNetwork(const QString & ssid, int strength, Solid::Control::WirelessNetworkInterface::Capabilities interfaceCapabilities, Solid::Control::AccessPoint::Capabilities apCapabilities, Solid::Control::AccessPoint::WpaFlags wpaFlags, Solid::Control::AccessPoint::WpaFlags rsnFlags, Solid::Control::WirelessNetworkInterface::OperationMode mode, const QString & deviceUni, QObject * parent)
: Activatable(Activatable::WirelessNetwork, deviceUni, parent), WirelessObject(ssid, strength, interfaceCapabilities, apCapabilities, wpaFlags, rsnFlags, mode)
{
}

WirelessNetwork::~WirelessNetwork()
{

}

void WirelessNetwork::setStrength(int strength)
{
    if (strength != m_strength) {
        WirelessObject::setStrength(strength);
        emit strengthChanged(strength);
    }
}
