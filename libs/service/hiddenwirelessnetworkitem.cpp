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

#include "hiddenwirelessnetworkitem.h"

HiddenWirelessNetworkItem::HiddenWirelessNetworkItem(const QString & deviceUni, QObject * parent)
: WirelessNetworkItem(QString(), -1, (Solid::Control::AccessPoint::Capabilities)0,(Solid::Control::AccessPoint::WpaFlags)0, (Solid::Control::AccessPoint::WpaFlags)0, deviceUni, parent)
{
}

void HiddenWirelessNetworkItem::setSsid(const QString & ssid)
{
    m_ssid = ssid;
}
// vim: sw=4 sts=4 et tw=100
