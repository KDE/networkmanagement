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

class KNM_EXPORT WirelessSecurity
{
public:
    enum Type { Unknown, None, StaticWep, Leap, DynamicWep, WpaPsk, WpaEap, Wpa2Psk, Wpa2Eap };

    static bool possible(WirelessSecurity::Type, Solid::Control::WirelessNetworkInterface::Capabilities, bool haveAp, bool adHoc, Solid::Control::AccessPoint::Capabilities apCaps, Solid::Control::AccessPoint::WpaFlags apWpa, Solid::Control::AccessPoint::WpaFlags apRsn);

    static QString shortToolTip(WirelessSecurity::Type);

    static QString iconName(WirelessSecurity::Type);

    static WirelessSecurity::Type best(Solid::Control::WirelessNetworkInterface::Capabilities, bool haveAp, bool adHoc, Solid::Control::AccessPoint::Capabilities apCaps, Solid::Control::AccessPoint::WpaFlags apWpa, Solid::Control::AccessPoint::WpaFlags apRsn);
private:
    static bool interfaceSupportsApCiphers(Solid::Control::WirelessNetworkInterface::Capabilities, Solid::Control::AccessPoint::WpaFlags ciphers, WirelessSecurity::Type);
};


#endif // WIRELESSSECURITYIDENTIFIER_H
