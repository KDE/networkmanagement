/*
Copyright 2008-2010 Sebastian Kügler <sebas@kde.org>
Copyright 2011 Will Stephenson <wstephenson@kde.org>

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
#include <kdeversion.h>
#include <Solid/Device>

#include <libnm-qt/manager.h>

// Qt
#include <QSizeF>

QString UiUtils::interfaceTypeLabel(const NetworkManager::Device::Type type)
{
    QString deviceText;
    switch (type) {
        case NetworkManager::Device::Ethernet:
            deviceText = i18nc("title of the interface widget in nm's popup", "Wired Ethernet");
            break;
        case NetworkManager::Device::Wifi:
            deviceText = i18nc("title of the interface widget in nm's popup", "Wireless 802.11");
            break;
        case NetworkManager::Device::Modem:
            deviceText = i18nc("title of the interface widget in nm's popup", "Serial Modem");
            break;
        case NetworkManager::Device::Bluetooth:
            deviceText = i18nc("title of the interface widget in nm's popup", "Bluetooth");
            break;
        default:
            deviceText = i18nc("title of the interface widget in nm's popup", "Wired Ethernet");
            break;
    }
    return deviceText;
}

QString UiUtils::iconName(NetworkManager::Device *iface)
{
    if (!iface) {
        return QString("dialog-error");
    }
    QString icon;
    QString strength = "00";
    NetworkManager::WirelessDevice *wiface = qobject_cast<NetworkManager::WirelessDevice*>(iface);

    switch (iface->type()) {
        case NetworkManager::Device::Ethernet:
            icon = "network-wired";
            break;
        case NetworkManager::Device::Wifi:

            if (wiface) {
                QString uni = wiface->activeAccessPoint();
                //QString uni = wiface->activeAccessPoint()->signalStrength();
                //int s =
                NetworkManager::AccessPoint *ap = wiface->findAccessPoint(uni);
                if (ap) {
                    int s = ap->signalStrength();
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
                } else {
                        strength = "00";
                }
            }
            icon = "network-wireless-connected-" + strength;
            break;
        case NetworkManager::Device::Modem:
            icon = "modem";
            break;
        case NetworkManager::Device::Bluetooth:
            icon = "bluetooth";
            break;
        default:
            icon = "network-wired";
            break;
    }
    //kDebug() << "icon:" << icon;
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
    } else if (c >= KIconLoader::SizeSmallMedium) { // 22
        s = KIconLoader::SizeSmallMedium;
    } else { // 16
        s = KIconLoader::SizeSmall;
    }
    return s;
}

QString UiUtils::connectionStateToString(NetworkManager::Device::State state, const QString &connectionName)
{
    QString stateString;
    switch (state) {
        case NetworkManager::Device::UnknownState:
            stateString = i18nc("description of unknown network interface state", "Unknown");
            break;
        case NetworkManager::Device::Unmanaged:
            stateString = i18nc("description of unmanaged network interface state", "Unmanaged");
            break;
        case NetworkManager::Device::Unavailable:
            stateString = i18nc("description of unavailable network interface state", "Unavailable");
            break;
        case NetworkManager::Device::Disconnected:
            stateString = i18nc("description of unconnected network interface state", "Not connected");
            break;
        case NetworkManager::Device::Preparing:
            stateString = i18nc("description of preparing to connect network interface state", "Preparing to connect");
            break;
        case NetworkManager::Device::ConfiguringHardware:
            stateString = i18nc("description of configuring hardware network interface state", "Configuring interface");
            break;
        case NetworkManager::Device::NeedAuth:
            stateString = i18nc("description of waiting for authentication network interface state", "Waiting for authorization");
            break;
        case NetworkManager::Device::ConfiguringIp:
            stateString = i18nc("network interface doing dhcp request in most cases", "Setting network address");
            break;
        case NetworkManager::Device::CheckingIp:
            stateString = i18nc("is other action required to fully connect? captive portals, etc.", "Checking further connectivity");
            break;
        case NetworkManager::Device::WaitingForSecondaries:
            stateString = i18nc("a secondary connection (e.g. VPN) has to be activated first to continue", "Waiting for a secondary connection");
            break;
        case NetworkManager::Device::Activated:
            if (connectionName.isEmpty()) {
                stateString = i18nc("network interface connected state label", "Connected");
            } else {
                stateString = i18nc("network interface connected state label", "Connected to %1", connectionName);
            }
            break;
        case NetworkManager::Device::Deactivating:
            stateString = i18nc("network interface disconnecting state label", "Deactivating connection");
            break;
        case NetworkManager::Device::Failed:
            stateString = i18nc("network interface connection failed state label", "Connection Failed");
            break;
        default:
            stateString = i18nc("interface state", "Error: Invalid state");
    }
    return stateString;
}

Solid::Device* UiUtils::findSolidDevice(const QString & uni)
{
    NetworkManager::Device * iface = NetworkManager::findNetworkInterface(uni);

    if (!iface) {
        return 0;
    }

    QList<Solid::Device> list = Solid::Device::listFromQuery(QString::fromLatin1("NetworkInterface.ifaceName == '%1'").arg(iface->interfaceName()));
    QList<Solid::Device>::iterator it = list.begin();

    if (it != list.end()) {
        Solid::Device* dev = new Solid::Device(*it);
        return dev;
    }

    return 0;
}

QString UiUtils::interfaceNameLabel(const QString & uni, const KNetworkManagerServicePrefs::InterfaceNamingChoices interfaceNamingStyle)
{
    QString label;
    NetworkManager::Device * iface = NetworkManager::findNetworkInterface(uni);
    Solid::Device* dev = findSolidDevice(uni);

    switch (interfaceNamingStyle) {
        case KNetworkManagerServicePrefs::SystemNames:
            if (iface) {
                label = iface->interfaceName();
            }
            break;
        case KNetworkManagerServicePrefs::DescriptiveNames:
            if (dev) {
                label = dev->description();
                //kDebug() << "Vendor, Product:" << dev->vendor() << dev->product();
            }
            break;
        case KNetworkManagerServicePrefs::VendorProductNames:
            if (dev) {
                if (!dev->vendor().isEmpty() && !dev->product().isEmpty()) {
                    label = QString(i18nc("Format for <Vendor> <Product>", "%1 - %2", dev->vendor(), dev->product()));
                }
            }
            break;
        case KNetworkManagerServicePrefs::TypeNames:
            break;
    }

    if (label.isEmpty()) {
        // if we don't get sensible information from Solid,
        // let's try to use the type of the interface
        if (iface) {
            label = UiUtils::interfaceTypeLabel(iface->type());
        }
    }
    if (dev) {
        delete dev;
    }
    return label;
}

QString UiUtils::interfaceNameLabel(const QString & uni)
{
    KNetworkManagerServicePrefs::instance(Knm::ConnectionPersistence::NETWORKMANAGEMENT_RCFILE);

    return interfaceNameLabel(uni, static_cast<KNetworkManagerServicePrefs::InterfaceNamingChoices>(KNetworkManagerServicePrefs::self()->interfaceNamingStyle()));
}

qreal UiUtils::interfaceState(const NetworkManager::Device *interface)
{
    if (!interface) {
        return 0;
    }

    // from libs/types.h
    switch (interface->state()) {
        case NetworkManager::Device::Preparing:
            return 0.15;
            break;
        case NetworkManager::Device::ConfiguringHardware:
            return 0.30;
            break;
        case NetworkManager::Device::NeedAuth:
            return 0.45;
            break;
        case NetworkManager::Device::ConfiguringIp:
            return 0.60;
            break;
        case NetworkManager::Device::CheckingIp:
            return 0.75;
            break;
        case NetworkManager::Device::WaitingForSecondaries:
            return 0.90;
            break;
        case NetworkManager::Device::Activated:
            return 1.0;
            break;
        default:
            return 0;
            break;
    }
    return 0;
}

QString UiUtils::operationModeToString(NetworkManager::WirelessDevice::OperationMode mode)
{
    QString modeString;
    switch (mode) {
        case NetworkManager::WirelessDevice::Unassociated:
            modeString = i18nc("wireless network operation mode", "Unassociated");
            break;
        case NetworkManager::WirelessDevice::Adhoc:
            modeString = i18nc("wireless network operation mode", "Adhoc");
            break;
        case NetworkManager::WirelessDevice::Managed:
            modeString = i18nc("wireless network operation mode", "Managed");
            break;
        case NetworkManager::WirelessDevice::Master:
            modeString = i18nc("wireless network operation mode", "Master");
            break;
        case NetworkManager::WirelessDevice::Repeater:
            modeString = i18nc("wireless network operation mode", "Repeater");
            break;
        default:
            modeString = I18N_NOOP("INCORRECT MODE FIX ME");
    }
    return modeString;
}

QStringList UiUtils::wpaFlagsToStringList(NetworkManager::AccessPoint::WpaFlags flags)
{
    /* for testing purposes
    flags = NetworkManager::AccessPoint::PairWep40
            | NetworkManager::AccessPoint::PairWep104
            | NetworkManager::AccessPoint::PairTkip
            | NetworkManager::AccessPoint::PairCcmp
            | NetworkManager::AccessPoint::GroupWep40
            | NetworkManager::AccessPoint::GroupWep104
            | NetworkManager::AccessPoint::GroupTkip
            | NetworkManager::AccessPoint::GroupCcmp
            | NetworkManager::AccessPoint::KeyMgmtPsk
            | NetworkManager::AccessPoint::KeyMgmt8021x; */

    QStringList flagList;

    if (flags.testFlag(NetworkManager::AccessPoint::PairWep40))
        flagList.append(i18nc("wireless network cipher", "Pairwise WEP40"));
    if (flags.testFlag(NetworkManager::AccessPoint::PairWep104))
        flagList.append(i18nc("wireless network cipher", "Pairwise WEP104"));
    if (flags.testFlag(NetworkManager::AccessPoint::PairTkip))
        flagList.append(i18nc("wireless network cipher", "Pairwise TKIP"));
    if (flags.testFlag(NetworkManager::AccessPoint::PairCcmp))
        flagList.append(i18nc("wireless network cipher", "Pairwise CCMP"));
    if (flags.testFlag(NetworkManager::AccessPoint::GroupWep40))
        flagList.append(i18nc("wireless network cipher", "Group WEP40"));
    if (flags.testFlag(NetworkManager::AccessPoint::GroupWep104))
        flagList.append(i18nc("wireless network cipher", "Group WEP104"));
    if (flags.testFlag(NetworkManager::AccessPoint::GroupTkip))
        flagList.append(i18nc("wireless network cipher", "Group TKIP"));
    if (flags.testFlag(NetworkManager::AccessPoint::GroupCcmp))
        flagList.append(i18nc("wireless network cipher", "Group CCMP"));
    if (flags.testFlag(NetworkManager::AccessPoint::KeyMgmtPsk))
        flagList.append(i18nc("wireless network cipher", "PSK"));
    if (flags.testFlag(NetworkManager::AccessPoint::KeyMgmt8021x))
        flagList.append(i18nc("wireless network cipher", "802.1x"));

    return flagList;
}

QString UiUtils::connectionSpeed(double bitrate)
{
    QString out;
    if (bitrate < 1000) {
        out = i18nc("connection speed", "%1 Bit/s", QString::number(bitrate));
    } else if (bitrate < 1000000) {
        out = i18nc("connection speed", "%1 MBit/s", QString::number(bitrate/1000));
    } else {
        out = i18nc("connection speed", "%1 GBit/s", QString::number(bitrate/1000000));
    }
    return out;
}

// vim: sw=4 sts=4 et tw=100
