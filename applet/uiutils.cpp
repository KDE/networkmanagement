/*
Copyright 2008, 2009 Sebastian Kügler <sebas@kde.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License or (at your option) version 3 or any later version
accepted by the membership of KDE e.V. (or its successor approved
by the membership of KDE e.V.), which shall act as a proxy
defined in Section 14 of version 3 of the license.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// Own
#include "uiutils.h"

// KDE
#include <KDebug>
#include <KLocale>
#include <solid/control/networkinterface.h>

// Qt
#include <QPixmap>
#include <QSizeF>

QString UiUtils::stateDescription()
{
    return i18n("UiUtils: connecting");
}


QString UiUtils::descriptiveInterfaceName(const Solid::Control::NetworkInterface::Type type)
{
    QString deviceText;
    switch (type) {
        case Solid::Control::NetworkInterface::Ieee8023:
            deviceText = i18nc("title of the interface widget in nm's popup", "Wired Ethernet");
            break;
        case Solid::Control::NetworkInterface::Ieee80211:
            deviceText = i18nc("title of the interface widget in nm's popup", "Wireless 802.11");
            break;
        case Solid::Control::NetworkInterface::Serial:
            deviceText = i18nc("title of the interface widget in nm's popup", "Serial Modem");
            break;
        case Solid::Control::NetworkInterface::Gsm:
        case Solid::Control::NetworkInterface::Cdma:
            deviceText = i18nc("title of the interface widget in nm's popup", "Mobile Broadband");
            break;
        default:
            deviceText = i18nc("title of the interface widget in nm's popup", "Wired Ethernet");
            break;
    }
    return deviceText;
}

QString UiUtils::iconName(const Solid::Control::NetworkInterface::Type type)
{
    QString icon;
    switch (type) {
        case Solid::Control::NetworkInterface::Ieee8023:
            icon = "network-wired";
            break;
        case Solid::Control::NetworkInterface::Ieee80211:
            icon = "network-wireless";
            break;
        case Solid::Control::NetworkInterface::Serial:
            icon = "modem";
            break;
        case Solid::Control::NetworkInterface::Gsm:
        case Solid::Control::NetworkInterface::Cdma:
            icon = "phone";
            break;
        default:
            icon = "network-wired";
            break;
    }
    return icon;
}

QPixmap UiUtils::interfacePixmap(const QSizeF size, const Solid::Control::NetworkInterface::Type type)
{
    return QPixmap(size.toSize());
}

// vim: sw=4 sts=4 et tw=100
