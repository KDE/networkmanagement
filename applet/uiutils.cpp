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
#include <KIconLoader>
#include <KLocale>

#include <solid/control/networkinterface.h>
#include <solid/control/wirelessaccesspoint.h>
#include <solid/control/wirelessnetworkinterface.h>

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

QString UiUtils::iconName(Solid::Control::NetworkInterface *iface)
{
    QString icon;
    QString strength = "00";
    Solid::Control::WirelessNetworkInterface *wiface = qobject_cast<Solid::Control::WirelessNetworkInterface*>(iface);

    switch (iface->type()) {
        case Solid::Control::NetworkInterface::Ieee8023:
            icon = "network-wired";
            break;
        case Solid::Control::NetworkInterface::Ieee80211:

            if (wiface) {
                QString uni = wiface->activeAccessPoint();
                //QString uni = wiface->activeAccessPoint()->signalStrength();
                //int s =
                int s = wiface->findAccessPoint(uni)->signalStrength();
                if (s < 13) {
                    strength = "00";
                } else if (s < 38) {
                    strength = "25";
                } else if (s < 63) {
                    strength = "50";
                } else if (s < 88) {
                    strength = "75";
                } else if (s >= 88) {
                    strength = "100";
                }
            }
            icon = "network-wireless-connected-" + strength;
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
    kDebug() << "icon:" << icon;
    return icon;
}

int UiUtils::iconSize(const QSizeF size)
{
    int c = qMin(size.width(), size.height());
    // return the biggest fitting icon size from KIconLoader
    int s;
    if (c >= KIconLoader::SizeEnormous) { // 128
        s = KIconLoader::SizeEnormous;
    } else if (c >= KIconLoader::SizeHuge) { // 64
        s = KIconLoader::SizeHuge;
    } else if (c >= KIconLoader::SizeLarge) { // 48
        s = KIconLoader::SizeLarge;
    } else if (c >= KIconLoader::SizeMedium) { // 32
        s = KIconLoader::SizeMedium;
    } else if (c >= KIconLoader::SizeSmallMedium) { // 32
        s = KIconLoader::SizeSmallMedium;
    } else { // 16
        s = KIconLoader::SizeSmall;
    }
    return s;
}


QPixmap UiUtils::interfacePixmap(const QSizeF size, const Solid::Control::NetworkInterface *iface)
{

    return QPixmap(size.toSize());
}

QString UiUtils::connectionStateToString(int state)
{
    QString stateString;
    switch (state) {
        case Solid::Control::NetworkInterface::UnknownState:
            stateString = i18nc("description of unknown network interface state", "Unknown");
            break;
        case Solid::Control::NetworkInterface::Unmanaged:
            stateString = i18nc("description of unmanaged network interface state", "Unmanaged");
            break;
        case Solid::Control::NetworkInterface::Unavailable:
            stateString = i18nc("description of unavailable network interface state", "Unavailable");
            break;
        case Solid::Control::NetworkInterface::Disconnected:
            stateString = i18nc("description of unconnected network interface state", "Not connected");
            break;
        case Solid::Control::NetworkInterface::Preparing:
            stateString = i18nc("description of preparing to connect network interface state", "Preparing to connect");
            break;
        case Solid::Control::NetworkInterface::Configuring:
            stateString = i18nc("description of configuring hardware network interface state", "Configuring interface");
            break;
        case Solid::Control::NetworkInterface::NeedAuth:
            stateString = i18nc("description of waiting for authentication network interface state", "Waiting for authorization");
            break;
        case Solid::Control::NetworkInterface::IPConfig:
            stateString = i18nc("network interface doing dhcp request in most cases", "Setting network address");
            break;
        case Solid::Control::NetworkInterface::Activated:
            stateString = i18nc("network interface connected state label", "Connected");
            break;
        case Solid::Control::NetworkInterface::Failed:
            stateString = i18nc("network interface connection failed state label", "Connection Failed");
            break;
        default:
            stateString = I18N_NOOP("UNKNOWN STATE FIX ME");
    }
    return stateString;
}

// vim: sw=4 sts=4 et tw=100
