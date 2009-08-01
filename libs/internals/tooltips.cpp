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

#include "tooltips.h"

#include <QHash>

#include <KGlobal>
#include <KLocale>

class ToolTipsPrivate
{
public:
    ToolTipsPrivate();
    ~ToolTipsPrivate();
public:
    QHash<QString, QString> tipKeysToLabels;
    QHash<QString, QString> tipKeysToToolTips;
};

K_GLOBAL_STATIC(ToolTipsPrivate, myToolTipsPrivate)

ToolTipsPrivate::ToolTipsPrivate()
{
    /* first the tooltips for the tooltips */
    tipKeysToToolTips[QLatin1String("interface:type")] = i18nc("@info:tooltip", "Interface type");
    tipKeysToToolTips[QLatin1String("interface:name")] = i18nc("@info:tooltip", "System device name");
    tipKeysToToolTips[QLatin1String("interface:driver")] = i18nc("@info:tooltip", "System driver name");
    tipKeysToToolTips[QLatin1String("interface:status")] = i18nc("@info:tooltip", "Network interface status");
    tipKeysToToolTips[QLatin1String("interface:hardwareaddress")] = i18nc("@info:tooltip", "Hardware address of a network interface");
    tipKeysToToolTips[QLatin1String("interface:bitrate")] = i18nc("@info:tooltip", "Network interface current bit rate");
    tipKeysToToolTips[QLatin1String("interface:designspeed")] = i18nc("@info:tooltip", "Maximum speed of the network interface");
    tipKeysToToolTips[QLatin1String("ipv4:address")] = i18nc("@info:tooltip", "IPv4 network address");
    tipKeysToToolTips[QLatin1String("ipv4:nameservers")] = i18nc("@info:tooltip", "Network name servers");
    tipKeysToToolTips[QLatin1String("ipv4:domains")] = i18nc("@info:tooltip", "Network domains");
    tipKeysToToolTips[QLatin1String("ipv4:routes")] = i18nc("@info:tooltip", "Network routes");
    //tipKeysToToolTips[QLatin1String("wired:carrier")] =
    tipKeysToToolTips[QLatin1String("wireless:strength")] = i18nc("@info:tooltip", "Signal strength of the wifi network");
    tipKeysToToolTips[QLatin1String("wireless:ssid")] = i18nc("@info:tooltip", "Name of the wireless network in use");
    tipKeysToToolTips[QLatin1String("wireless:mode")] = i18nc("@info:tooltip", "Operation mode of wireless network");
    tipKeysToToolTips[QLatin1String("wireless:accesspoint")] = i18nc("@info:tooltip", "Hardware address of the active access point");
    tipKeysToToolTips[QLatin1String("wireless:frequency")] = i18nc("@info:tooltip", "The radio channel frequency that the access point is operating on");
    tipKeysToToolTips[QLatin1String("wireless:security")] = i18nc("@info:tooltip", "Subjective network security level description");
    tipKeysToToolTips[QLatin1String("wireless:wpaflags")] = i18nc("@info:tooltip", "Flags describing the access point's capabilities according to WPA (Wifi Protected Access)");
    tipKeysToToolTips[QLatin1String("wireless:rsnflags")] = i18nc("@info:tooltip", "Flags describing the access point's capabilities according to RSN (Robust Secure Network)");

    /* now the labels */
    tipKeysToLabels[QLatin1String("interface:type")] = i18nc(" interface type", "Type");
    tipKeysToLabels[QLatin1String("interface:name")] = i18nc(" network device name eg eth0", "Interface");
    tipKeysToLabels[QLatin1String("interface:driver")] = i18nc(" system driver name", "Driver");
    tipKeysToLabels[QLatin1String("interface:status")] = i18nc(" network interface status", "Status");
    tipKeysToLabels[QLatin1String("interface:hardwareaddress")] = i18nc(" this is the hardware address of a network interface",
            "Hardware address (Wired)");
    tipKeysToLabels[QLatin1String("interface:bitrate")] = i18nc(" network connection bit rate","Bit rate");
    tipKeysToLabels[QLatin1String("interface:designspeed")] = i18nc(" The network device's maximum speed",
            "Max speed");
    tipKeysToLabels[QLatin1String("ipv4:address")] = i18nc(" IPv4 address", "IP address");
    tipKeysToLabels[QLatin1String("ipv4:nameservers")] = i18nc(" network name servers", "Name servers");
    tipKeysToLabels[QLatin1String("ipv4:domains")] = i18nc(" network domains", "Domains");
    tipKeysToLabels[QLatin1String("ipv4:routes")] = i18nc(" network routes", "Routes");
    //tipKeysToLabels[QLatin1String("wired:carrier")] =
    tipKeysToLabels[QLatin1String("wireless:strength")] = i18nc(" The signal strength of the wifi network",
            "Strength");
    tipKeysToLabels[QLatin1String("wireless:ssid")] = i18nc(" SSID is a friendly name that identifies a 802.11 WLAN.",
            "SSID");
    tipKeysToLabels[QLatin1String("wireless:mode")] = i18nc(" the operation mode of wi-fi network","Mode");
    tipKeysToLabels[QLatin1String("wireless:accesspoint")] = i18nc(" Active access point MAC address",
            "Access point");
    tipKeysToLabels[QLatin1String("wireless:frequency")] = i18nc(" the frequency of the radio channel that the access point is operating on",
            "Frequency");
    tipKeysToLabels[QLatin1String("wireless:security")] = i18nc(" network security level, e.g. high, low", "Security");
    tipKeysToLabels[QLatin1String("wireless:wpaflags")] = i18nc(" Flags describing the access point's capabilities according to WPA (Wifi Protected Access)",
            "WPA flags");
    tipKeysToLabels[QLatin1String("wireless:rsnflags")] = i18nc("Flags describing the access point's capabilities according to RSN (Robust Secure Network)",
            "RSN(WPA2) flags");
}

ToolTipsPrivate::~ToolTipsPrivate()
{

}

QStringList Knm::ToolTips::allKeys()
{
    return myToolTipsPrivate->tipKeysToLabels.keys();
}

QString Knm::ToolTips::labelForKey(const QString & key)
{
    return myToolTipsPrivate->tipKeysToLabels.value(key);
}

QString Knm::ToolTips::toolTipForKey(const QString & key)
{
    return myToolTipsPrivate->tipKeysToToolTips.value(key);
}

// vim: sw=4 sts=4 et tw=100
