/*
Copyright 2009 Will Stephenson <wstephenson@kde.org>

 possible() incorporates code from nm-utils.c by
 Ray Strode <rstrode@redhat.com>
 Dan Williams <dcbw@redhat.com>
 Tambet Ingo <tambet@gmail.com>
 (C) Copyright 2005 - 2008 Red Hat, Inc.

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

#include "wirelesssecurityidentifier.h"

#include <KDebug>
#include <KLocale>

bool Knm::WirelessSecurity::interfaceSupportsApCiphers(Solid::Control::WirelessNetworkInterfaceNm09::Capabilities interfaceCaps, Solid::Control::AccessPointNm09::WpaFlags apCiphers, Knm::WirelessSecurity::Type type )
{
    bool havePair = false;
    bool haveGroup = true;
    if ( type == WirelessSecurity::StaticWep ) {
        havePair = true;
    } else {
        if ( interfaceCaps.testFlag(Solid::Control::WirelessNetworkInterfaceNm09::Wep40) && apCiphers.testFlag(Solid::Control::AccessPointNm09::PairWep40)) {
            havePair = true;
        }
        if ( interfaceCaps.testFlag(Solid::Control::WirelessNetworkInterfaceNm09::Wep104) && apCiphers.testFlag(Solid::Control::AccessPointNm09::PairWep104)) {
            havePair = true;
        }
        if ( interfaceCaps.testFlag(Solid::Control::WirelessNetworkInterfaceNm09::Tkip) && apCiphers.testFlag(Solid::Control::AccessPointNm09::PairTkip)) {
            havePair = true;
        }
        if ( interfaceCaps.testFlag(Solid::Control::WirelessNetworkInterfaceNm09::Ccmp) && apCiphers.testFlag(Solid::Control::AccessPointNm09::PairCcmp)) {
            havePair = true;
        }
    }

    if ( interfaceCaps.testFlag(Solid::Control::WirelessNetworkInterfaceNm09::Wep40) && apCiphers.testFlag(Solid::Control::AccessPointNm09::GroupWep40)) {
        haveGroup = true;
    }
    if ( interfaceCaps.testFlag(Solid::Control::WirelessNetworkInterfaceNm09::Wep104) && apCiphers.testFlag(Solid::Control::AccessPointNm09::GroupWep104)) {
        haveGroup = true;
    }
    if (type != StaticWep) {
        if ( interfaceCaps.testFlag(Solid::Control::WirelessNetworkInterfaceNm09::Tkip) && apCiphers.testFlag(Solid::Control::AccessPointNm09::GroupTkip)) {
            haveGroup = true;
        }
        if ( interfaceCaps.testFlag(Solid::Control::WirelessNetworkInterfaceNm09::Ccmp) && apCiphers.testFlag(Solid::Control::AccessPointNm09::GroupCcmp)) {
            haveGroup = true;
        }
    }

    return (havePair && haveGroup);
}

// Keep this in sync with NetworkManager/libnm-util/nm-utils.c:nm_utils_security_valid()
bool Knm::WirelessSecurity::possible(Knm::WirelessSecurity::Type type, Solid::Control::WirelessNetworkInterfaceNm09::Capabilities interfaceCaps, bool haveAp, bool adhoc, Solid::Control::AccessPointNm09::Capabilities apCaps, Solid::Control::AccessPointNm09::WpaFlags apWpa, Solid::Control::AccessPointNm09::WpaFlags apRsn)
{
    bool good = TRUE;

    //kDebug() << "type(" << type << ") interfaceCaps(" << interfaceCaps << ") haveAp(" << haveAp << ") adhoc(" << adhoc << ") apCaps(" << apCaps << ") apWpa(" << apWpa << " apRsn(" << apRsn << ")";

    if (!haveAp) {
        if (type == Knm::WirelessSecurity::None)
            return true;
        if ((type == Knm::WirelessSecurity::StaticWep)
                || ((type == Knm::WirelessSecurity::DynamicWep) && !adhoc)
                || ((type == Knm::WirelessSecurity::Leap) && !adhoc)) {
            if (interfaceCaps.testFlag(Solid::Control::WirelessNetworkInterfaceNm09::Wep40) ||
                interfaceCaps.testFlag(Solid::Control::WirelessNetworkInterfaceNm09::Wep104)) {
                return true;
            }
        }

        // apCaps.testFlag(Privacy) == true for StaticWep, Leap and DynamicWep
        // see libs/internals/wirelessinterfaceconnectionhelpers.cpp
        if (type == Knm::WirelessSecurity::WpaPsk
                || ((type == Knm::WirelessSecurity::WpaEap) && !adhoc)) {
            if (interfaceCaps.testFlag(Solid::Control::WirelessNetworkInterfaceNm09::Wpa) &&
                !apCaps.testFlag(Solid::Control::AccessPointNm09::Privacy)) {
                return true;
            }
        }
        if (type == Knm::WirelessSecurity::Wpa2Psk
                || ((type == Knm::WirelessSecurity::Wpa2Eap) && !adhoc)) {
            if (interfaceCaps.testFlag(Solid::Control::WirelessNetworkInterfaceNm09::Rsn) &&
                !apCaps.testFlag(Solid::Control::AccessPointNm09::Privacy)) {
                return true;
            }
        }
        return false;
    }

    switch (type) {
        case Knm::WirelessSecurity::None:
            Q_ASSERT (haveAp);
            if (apCaps.testFlag(Solid::Control::AccessPointNm09::Privacy))
                return false;
            if (apWpa || apRsn)
                return false;
            break;
        case Knm::WirelessSecurity::Leap: /* require PRIVACY bit for LEAP? */
            if (adhoc)
                return false;
            /* Fall through */
        case Knm::WirelessSecurity::StaticWep:
            Q_ASSERT (haveAp);
            if (!apCaps.testFlag(Solid::Control::AccessPointNm09::Privacy))
                return false;
            if (apWpa || apRsn) {
                if (!interfaceSupportsApCiphers (interfaceCaps, apWpa, StaticWep))
                    if (!interfaceSupportsApCiphers (interfaceCaps, apRsn, StaticWep))
                        return false;
            }
            break;
        case Knm::WirelessSecurity::DynamicWep:
            if (adhoc)
                return false;
            Q_ASSERT (haveAp);
            if (apRsn || !(apCaps.testFlag(Solid::Control::AccessPointNm09::Privacy)))
                return false;
            /* Some APs broadcast minimal WPA-enabled beacons that must be handled */
            if (apWpa) {
                if (!apWpa.testFlag(Solid::Control::AccessPointNm09::KeyMgmt8021x))
                    return false;
                if (!interfaceSupportsApCiphers (interfaceCaps, apWpa, DynamicWep))
                    return false;
            }
            break;
        case Knm::WirelessSecurity::WpaPsk:
            if (!interfaceCaps.testFlag(Solid::Control::WirelessNetworkInterfaceNm09::Wpa)) {
                return false;
            }
            if (haveAp) {
                /* Ad-Hoc WPA APs won't necessarily have the PSK flag set */
                if (adhoc) {
                    if (apWpa.testFlag(Solid::Control::AccessPointNm09::GroupTkip) &&
                        interfaceCaps.testFlag(Solid::Control::WirelessNetworkInterfaceNm09::Tkip)) {
                        return true;
                    }
                    if (apWpa.testFlag(Solid::Control::AccessPointNm09::GroupCcmp) &&
                        interfaceCaps.testFlag(Solid::Control::WirelessNetworkInterfaceNm09::Ccmp)) {
                        return true;
                    }
                } else if (apWpa.testFlag(Solid::Control::AccessPointNm09::KeyMgmtPsk)) {
                    if (apWpa.testFlag(Solid::Control::AccessPointNm09::PairTkip) &&
                        interfaceCaps.testFlag(Solid::Control::WirelessNetworkInterfaceNm09::Tkip)) {
                        return true;
                    }
                    if (apWpa.testFlag(Solid::Control::AccessPointNm09::PairCcmp) &&
                        interfaceCaps.testFlag(Solid::Control::WirelessNetworkInterfaceNm09::Ccmp)) {
                        return true;
                    }
                }
                return false;
            }
            break;
        case Knm::WirelessSecurity::Wpa2Psk:
            if (!interfaceCaps.testFlag(Solid::Control::WirelessNetworkInterfaceNm09::Rsn))
                return false;
            if (haveAp) {
                /* Ad-Hoc WPA APs won't necessarily have the PSK flag set */
                if (apRsn.testFlag(Solid::Control::AccessPointNm09::KeyMgmtPsk) || adhoc) {
                    if (apRsn.testFlag(Solid::Control::AccessPointNm09::PairTkip) &&
                        interfaceCaps.testFlag(Solid::Control::WirelessNetworkInterfaceNm09::Tkip)) {
                        return true;
                    }
                    if (apRsn.testFlag(Solid::Control::AccessPointNm09::PairCcmp) &&
                        interfaceCaps.testFlag(Solid::Control::WirelessNetworkInterfaceNm09::Ccmp)) {
                        return true;
                    }
                }
                return false;
            }
            break;
        case Knm::WirelessSecurity::WpaEap:
            if (adhoc)
                return false;
            if (!interfaceCaps.testFlag(Solid::Control::WirelessNetworkInterfaceNm09::Wpa))
                return false;
            if (haveAp) {
                if (!apWpa.testFlag(Solid::Control::AccessPointNm09::KeyMgmt8021x))
                    return false;
                /* Ensure at least one WPA cipher is supported */
                if (!interfaceSupportsApCiphers (interfaceCaps, apWpa, WpaEap))
                    return false;
            }
            break;
        case Knm::WirelessSecurity::Wpa2Eap:
            if (adhoc)
                return false;
            if (!interfaceCaps.testFlag(Solid::Control::WirelessNetworkInterfaceNm09::Rsn))
                return false;
            if (haveAp) {
                if (!apRsn.testFlag(Solid::Control::AccessPointNm09::KeyMgmt8021x))
                    return false;
                /* Ensure at least one WPA cipher is supported */
                if (!interfaceSupportsApCiphers (interfaceCaps, apRsn, Wpa2Eap))
                    return false;
            }
            break;
        default:
            good = false;
            break;
    }

    return good;
}

Knm::WirelessSecurity::Type Knm::WirelessSecurity::best(Solid::Control::WirelessNetworkInterfaceNm09::Capabilities interfaceCaps, bool haveAp, bool adHoc, Solid::Control::AccessPointNm09::Capabilities apCaps, Solid::Control::AccessPointNm09::WpaFlags apWpa, Solid::Control::AccessPointNm09::WpaFlags apRsn)
{
    QList<Knm::WirelessSecurity::Type> types;

    // The ordering of this list is a pragmatic combination of security level and popularity.
    // Therefore static WEP is before LEAP and Dynamic WEP because there is no way to detect
    // if an AP is capable of Dynamic WEP and showing Dynamic WEP first would confuse
    // Static WEP users.
    types << Knm::WirelessSecurity::Wpa2Eap << Knm::WirelessSecurity::Wpa2Psk << Knm::WirelessSecurity::WpaEap << Knm::WirelessSecurity::WpaPsk << Knm::WirelessSecurity::StaticWep << Knm::WirelessSecurity::DynamicWep << Knm::WirelessSecurity::Leap << Knm::WirelessSecurity::None;

    foreach (Knm::WirelessSecurity::Type type, types) {
        if (possible(type, interfaceCaps, haveAp, adHoc, apCaps, apWpa, apRsn)) {
            return type;
        }
    }
    return Knm::WirelessSecurity::Unknown;
}

QString Knm::WirelessSecurity::label(Knm::WirelessSecurity::Type type)
{
    QString tip;
    switch (type) {
        case None:
            tip = i18nc("@label no security", "Insecure");
            break;
        case StaticWep:
            tip = i18nc("@label WEP security", "WEP");
            break;
        case Leap:
            tip = i18nc("@label LEAP security", "LEAP");
            break;
        case DynamicWep:
            tip = i18nc("@label Dynamic WEP security", "Dynamic WEP");
            break;
        case WpaPsk:
            tip = i18nc("@label WPA-PSK security", "WPA-PSK");
            break;
        case WpaEap:
            tip = i18nc("@label WPA-EAP security", "WPA-EAP");
            break;
        case Wpa2Psk:
            tip = i18nc("@label WPA2-PSK security", "WPA2-PSK");
            break;
        case Wpa2Eap:
            tip = i18nc("@label WPA2-EAP security", "WPA2-EAP");
            break;
        default:
        case Unknown:
            tip = i18nc("@label unknown security", "Unknown security type");
            break;
    }
    return tip;
}

QString Knm::WirelessSecurity::shortToolTip(Knm::WirelessSecurity::Type type)
{
    QString tip;
    switch (type) {
        case None:
            tip = i18nc("@info:tooltip no security", "Insecure");
            break;
        case StaticWep:
            tip = i18nc("@info:tooltip WEP security", "WEP");
            break;
        case Leap:
            tip = i18nc("@info:tooltip LEAP security", "LEAP");
            break;
        case DynamicWep:
            tip = i18nc("@info:tooltip Dynamic WEP security", "Dynamic WEP");
            break;
        case WpaPsk:
            tip = i18nc("@info:tooltip WPA-PSK security", "WPA-PSK");
            break;
        case WpaEap:
            tip = i18nc("@info:tooltip WPA-EAP security", "WPA-EAP");
            break;
        case Wpa2Psk:
            tip = i18nc("@info:tooltip WPA2-PSK security", "WPA2-PSK");
            break;
        case Wpa2Eap:
            tip = i18nc("@info:tooltip WPA2-EAP security", "WPA2-EAP");
            break;
        default:
        case Unknown:
            tip = i18nc("@info:tooltip unknown security", "Unknown security type");
            break;
    }
    return tip;
}

QString Knm::WirelessSecurity::iconName(Knm::WirelessSecurity::Type type)
{
    QString icon;
    switch (type) {
        case None:
            icon = QLatin1String("security-low");
            break;
        case StaticWep:
            icon = QLatin1String("security-medium");
            break;
        case Leap:
            icon = QLatin1String("security-medium");
            break;
        case DynamicWep:
            icon = QLatin1String("security-medium");
            break;
        case WpaPsk:
            icon = QLatin1String("security-high");
            break;
        case WpaEap:
            icon = QLatin1String("security-high");
            break;
        case Wpa2Psk:
            icon = QLatin1String("security-high");
            break;
        case Wpa2Eap:
            icon = QLatin1String("security-high");
            break;
        default:
        case Unknown:
            icon = QLatin1String("security-low");
            break;
    }
    return icon;
}

// vim: sw=4 sts=4 et tw=100
