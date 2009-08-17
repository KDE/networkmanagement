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

#include "wirelessobject.h"

Knm::WirelessObject::WirelessObject(const QString & ssid, int strength, Solid::Control::WirelessNetworkInterface::Capabilities interfaceCapabilities, Solid::Control::AccessPoint::Capabilities apCapabilities, Solid::Control::AccessPoint::WpaFlags wpaFlags, Solid::Control::AccessPoint::WpaFlags rsnFlags, Solid::Control::WirelessNetworkInterface::OperationMode mode)
: m_ssid(ssid),
    m_strength(strength),
    m_interfaceCapabilities(interfaceCapabilities),
    m_apCapabilities(apCapabilities),
    m_wpaFlags(wpaFlags),
    m_rsnFlags(rsnFlags),
    m_operationMode(mode)
{

}

Knm::WirelessObject::WirelessObject()
{}

Knm::WirelessObject::~WirelessObject()
{

}

QString Knm::WirelessObject::ssid() const
{
    return m_ssid;
}

int Knm::WirelessObject::strength() const
{
    return m_strength;
}

void Knm::WirelessObject::setStrength(int strength)
{
    m_strength = strength;
}

Solid::Control::WirelessNetworkInterface::Capabilities Knm::WirelessObject::interfaceCapabilities() const
{
    return m_interfaceCapabilities;
}

Solid::Control::AccessPoint::Capabilities Knm::WirelessObject::apCapabilities() const
{
    return m_apCapabilities;
}


Solid::Control::AccessPoint::WpaFlags Knm::WirelessObject::wpaFlags() const
{
    return m_wpaFlags;
}

Solid::Control::AccessPoint::WpaFlags Knm::WirelessObject::rsnFlags() const
{
    return m_rsnFlags;
}

Solid::Control::WirelessNetworkInterface::OperationMode Knm::WirelessObject::operationMode() const
{
    return m_operationMode;
}

// vim: sw=4 sts=4 et tw=100
