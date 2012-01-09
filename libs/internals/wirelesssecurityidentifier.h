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

#ifndef WIRELESSSECURITYIDENTIFIER_H
#define WIRELESSSECURITYIDENTIFIER_H

#include <solid/control/wirelessaccesspoint.h>
#include <solid/control/wirelessnetworkinterface.h>

#include "../knm_export.h"

namespace Knm
{
class KNM_EXPORT WirelessSecurity
{
public:
    enum Type { Unknown = -1, None, StaticWep, Leap, DynamicWep, WpaPsk, WpaEap, Wpa2Psk, Wpa2Eap };

    static bool possible(WirelessSecurity::Type, Solid::Control::WirelessNetworkInterfaceNm09::Capabilities, bool haveAp, bool adHoc, Solid::Control::AccessPointNm09::Capabilities apCaps, Solid::Control::AccessPointNm09::WpaFlags apWpa, Solid::Control::AccessPointNm09::WpaFlags apRsn);

    static QString shortToolTip(WirelessSecurity::Type);
    static QString label(WirelessSecurity::Type);

    static QString iconName(WirelessSecurity::Type);

    static WirelessSecurity::Type best(Solid::Control::WirelessNetworkInterfaceNm09::Capabilities, bool haveAp, bool adHoc, Solid::Control::AccessPointNm09::Capabilities apCaps, Solid::Control::AccessPointNm09::WpaFlags apWpa, Solid::Control::AccessPointNm09::WpaFlags apRsn);
private:
    static bool interfaceSupportsApCiphers(Solid::Control::WirelessNetworkInterfaceNm09::Capabilities, Solid::Control::AccessPointNm09::WpaFlags ciphers, WirelessSecurity::Type);
};

} // namespace Knm

#endif // WIRELESSSECURITYIDENTIFIER_H
