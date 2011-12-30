/*
Copyright 2009 Will Stephenson <wstephenson@kde.org>

 possible() incorporates code from nm-utils.c by
 Ray Strode <rstrode@redhat.com>
 Dan Williams <dcbw@redhat.com>
 Tambet Ingo <tambet@gmail.com>
 (C) Copyright 2005 - 2008 Red Hat, Inc. //krazy:exclude=copyright

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

#include <KLocale>

bool Knm::WirelessSecurity::interfaceSupportsApCiphers(NetworkManager::WirelessDevice::Capabilities interfaceCaps, NetworkManager::AccessPoint::WpaFlags apCiphers, Knm::WirelessSecurity::Type type )
{
    bool havePair = false;
    bool haveGroup = true;
    if ( type == WirelessSecurity::StaticWep ) {
        havePair = true;
    } else {
        if ( interfaceCaps.testFlag(NetworkManager::WirelessDevice::Wep40) && apCiphers.testFlag(NetworkManager::AccessPoint::PairWep40)) {
            havePair = true;
        }
        if ( interfaceCaps.testFlag(NetworkManager::WirelessDevice::Wep104) && apCiphers.testFlag(NetworkManager::AccessPoint::PairWep104)) {
            havePair = true;
        }
        if ( interfaceCaps.testFlag(NetworkManager::WirelessDevice::Tkip) && apCiphers.testFlag(NetworkManager::AccessPoint::PairTkip)) {
            havePair = true;
        }
        if ( interfaceCaps.testFlag(NetworkManager::WirelessDevice::Ccmp) && apCiphers.testFlag(NetworkManager::AccessPoint::PairCcmp)) {
            havePair = true;
        }
    }

    if ( interfaceCaps.testFlag(NetworkManager::WirelessDevice::Wep40) && apCiphers.testFlag(NetworkManager::AccessPoint::GroupWep40)) {
        haveGroup = true;
    }
    if ( interfaceCaps.testFlag(NetworkManager::WirelessDevice::Wep104) && apCiphers.testFlag(NetworkManager::AccessPoint::GroupWep104)) {
        haveGroup = true;
    }
    if (type != StaticWep) {
        if ( interfaceCaps.testFlag(NetworkManager::WirelessDevice::Tkip) && apCiphers.testFlag(NetworkManager::AccessPoint::GroupTkip)) {
            haveGroup = true;
        }
        if ( interfaceCaps.testFlag(NetworkManager::WirelessDevice::Ccmp) && apCiphers.testFlag(NetworkManager::AccessPoint::GroupCcmp)) {
            haveGroup = true;
        }
    }

    return havePair && haveGroup;
}

bool Knm::WirelessSecurity::possible(Knm::WirelessSecurity::Type type, NetworkManager::WirelessDevice::Capabilities interfaceCaps, bool haveAp, bool adhoc, NetworkManager::AccessPoint::Capabilities apCaps, NetworkManager::AccessPoint::WpaFlags apWpa, NetworkManager::AccessPoint::WpaFlags apRsn)
{
    bool good = true;

    if (!haveAp) {
        if (type == Knm::WirelessSecurity::None)
            return true;
        if ((type == Knm::WirelessSecurity::StaticWep)
                || ((type == Knm::WirelessSecurity::DynamicWep) && !adhoc)
                || ((type == Knm::WirelessSecurity::Leap) && !adhoc)) {
            if (interfaceCaps & (NetworkManager::WirelessDevice::Wep40 | NetworkManager::WirelessDevice::Wep104))
                return true;
        }
        if (type == Knm::WirelessSecurity::WpaPsk
                || ((type == Knm::WirelessSecurity::WpaEap) && !adhoc)) {
            if (interfaceCaps & (NetworkManager::WirelessDevice::Wpa)) {
                return true;
            }
        }
        if (type == Knm::WirelessSecurity::Wpa2Psk
                || ((type == Knm::WirelessSecurity::Wpa2Eap) && !adhoc)) {
            if (interfaceCaps & (NetworkManager::WirelessDevice::Rsn)) {
                return true;
            }
        }
        return false;
    }

    switch (type) {
        case Knm::WirelessSecurity::None:
            Q_ASSERT (haveAp);
            if (apCaps & NetworkManager::AccessPoint::Privacy)
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
            if (!(apCaps & NetworkManager::AccessPoint::Privacy))
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
            if (apRsn || !(apCaps & NetworkManager::AccessPoint::Privacy))
                return false;
            /* Some APs broadcast minimal WPA-enabled beacons that must be handled */
            if (apWpa) {
                if (!(apWpa & NetworkManager::AccessPoint::KeyMgmt8021x))
                    return false;
                if (!interfaceSupportsApCiphers (interfaceCaps, apWpa, DynamicWep))
                    return false;
            }
            break;
        case Knm::WirelessSecurity::WpaPsk:
            if (!(interfaceCaps & NetworkManager::WirelessDevice::Wpa)) {
                return false;
            }
            if (haveAp) {
                /* Ad-Hoc WPA APs won't necessarily have the PSK flag set */
                if (adhoc) {
                    if ((apWpa & NetworkManager::AccessPoint::GroupTkip) &&
                        (interfaceCaps & NetworkManager::WirelessDevice::Tkip)) {
                        return true;
                    }
                    if ((apWpa & NetworkManager::AccessPoint::GroupCcmp) &&
                        (interfaceCaps & NetworkManager::WirelessDevice::Ccmp)) {
                        return true;
                    }
                } else if (apWpa & NetworkManager::AccessPoint::KeyMgmtPsk) {
                    if ((apWpa & NetworkManager::AccessPoint::PairTkip) &&
                        (interfaceCaps & NetworkManager::WirelessDevice::Tkip)) {
                        return true;
                    }
                    if ((apWpa & NetworkManager::AccessPoint::PairCcmp) &&
                        (interfaceCaps & NetworkManager::WirelessDevice::Ccmp)) {
                        return true;
                    }
                }
                return false;
            }
            break;
        case Knm::WirelessSecurity::Wpa2Psk:
            if (!(interfaceCaps & NetworkManager::WirelessDevice::Rsn))
                return false;
            if (haveAp) {
                /* Ad-Hoc WPA APs won't necessarily have the PSK flag set */
                if ((apRsn & NetworkManager::AccessPoint::KeyMgmtPsk) || adhoc) {
                    if (   (apRsn & NetworkManager::AccessPoint::PairTkip)
                            && (interfaceCaps & NetworkManager::WirelessDevice::Tkip))
                        return true;
                    if (   (apRsn & NetworkManager::AccessPoint::PairCcmp)
                            && (interfaceCaps & NetworkManager::WirelessDevice::Ccmp))
                        return true;
                }
                return false;
            }
            break;
        case Knm::WirelessSecurity::WpaEap:
            if (adhoc)
                return false;
            if (!(interfaceCaps & NetworkManager::WirelessDevice::Wpa))
                return false;
            if (haveAp) {
                if (!(apWpa & NetworkManager::AccessPoint::KeyMgmt8021x))
                    return false;
                /* Ensure at least one WPA cipher is supported */
                if (!interfaceSupportsApCiphers (interfaceCaps, apWpa, WpaEap))
                    return false;
            }
            break;
        case Knm::WirelessSecurity::Wpa2Eap:
            if (adhoc)
                return false;
            if (!(interfaceCaps & NetworkManager::WirelessDevice::Rsn))
                return false;
            if (haveAp) {
                if (!(apRsn & NetworkManager::AccessPoint::KeyMgmt8021x))
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

Knm::WirelessSecurity::Type Knm::WirelessSecurity::best(NetworkManager::WirelessDevice::Capabilities interfaceCaps, bool haveAp, bool adHoc, NetworkManager::AccessPoint::Capabilities apCaps, NetworkManager::AccessPoint::WpaFlags apWpa, NetworkManager::AccessPoint::WpaFlags apRsn)
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
