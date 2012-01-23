/*
Copyright 2008 Frederik Gladhorn <gladhorn@kde.org>
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

#include "wirelessnetwork.h"

using namespace Knm;

WirelessNetwork::WirelessNetwork(const QString & ssid, int strength, Solid::Control::WirelessNetworkInterfaceNm09::Capabilities interfaceCapabilities, Solid::Control::AccessPointNm09::Capabilities apCapabilities, Solid::Control::AccessPointNm09::WpaFlags wpaFlags, Solid::Control::AccessPointNm09::WpaFlags rsnFlags, Solid::Control::WirelessNetworkInterfaceNm09::OperationMode mode, const QString & deviceUni, QObject * parent)
: Activatable(Activatable::WirelessNetwork, deviceUni, parent), WirelessObject(ssid, strength, interfaceCapabilities, apCapabilities, wpaFlags, rsnFlags, mode)
{
}

WirelessNetwork::~WirelessNetwork()
{

}

void WirelessNetwork::setStrength(int strength)
{
    if (strength == m_strength) {
        return;
    }
    WirelessObject::setStrength(strength);
    emit strengthChanged(strength);
    QVariantMap map;
    map.insert("signalStrength", m_strength);
    emit wnPropertiesChanged(map);
}

QVariantMap WirelessNetwork::toMap()
{
    QVariantMap map = Activatable::toMap();
    map.insert("ssid", m_ssid);
    map.insert("signalStrength", m_strength);
    map.insert("interfaceCapabilities", (uint)m_interfaceCapabilities);
    map.insert("apCapabilities", (uint)m_apCapabilities);
    map.insert("wpaFlags", (uint)m_wpaFlags);
    map.insert("rsnFlags", (uint)m_rsnFlags);
    map.insert("operationMode", (uint)m_operationMode);
    return map;
}
