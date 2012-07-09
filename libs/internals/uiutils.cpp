/*
Copyright 2008-2010 Sebastian Kügler <sebas@kde.org>

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
#include "uiutils_p.h"

#include "paths.h"

#include "settings/802-11-wireless.h"

// KDE
#include <KDebug>
#include <KIconLoader>
#include <KLocale>
#include <kdeversion.h>

#include <solid/control/networkmanager.h>
#include <solid/control/networkinterface.h>
#include <solid/control/wirelessaccesspoint.h>
#include <solid/control/wirelessnetworkinterface.h>
#include <solid/control/wirednetworkinterface.h>

// Qt
#include <QSizeF>

K_GLOBAL_STATIC(UiUtilsPrivate, s_UiUtilsPrivate)

QString UiUtils::interfaceTypeLabel(const Solid::Control::NetworkInterfaceNm09::Type type, const Solid::Control::NetworkInterfaceNm09 *iface)
{
    QString deviceText;
    switch (type) {
        case Solid::Control::NetworkInterfaceNm09::Ethernet:
            deviceText = i18nc("title of the interface widget in nm's popup", "Wired Ethernet");
            break;
        case Solid::Control::NetworkInterfaceNm09::Wifi:
            deviceText = i18nc("title of the interface widget in nm's popup", "Wireless 802.11");
            break;
        case Solid::Control::NetworkInterfaceNm09::Bluetooth:
            deviceText = i18nc("title of the interface widget in nm's popup", "Mobile Broadband");
            break;
        case Solid::Control::NetworkInterfaceNm09::Modem: {
            const Solid::Control::ModemNetworkInterfaceNm09 *nmModemIface = qobject_cast<const Solid::Control::ModemNetworkInterfaceNm09 *>(iface);
            if (nmModemIface) {
                Solid::Control::ModemNetworkInterfaceNm09::ModemCapabilities subType = nmModemIface->subType();
                switch(subType) {
                    case Solid::Control::ModemNetworkInterfaceNm09::Pots:
                         deviceText = i18nc("title of the interface widget in nm's popup", "Serial Modem");
                         break;
                    case Solid::Control::ModemNetworkInterfaceNm09::GsmUmts:
                    case Solid::Control::ModemNetworkInterfaceNm09::CdmaEvdo:
                    case Solid::Control::ModemNetworkInterfaceNm09::Lte:
                         deviceText = i18nc("title of the interface widget in nm's popup", "Mobile Broadband");
                         break;
                    case Solid::Control::ModemNetworkInterfaceNm09::None:
                         kWarning() << "Unhandled modem sub type: Solid::Control::ModemNetworkInterfaceNm09::None";
                         break;
                }
            }
        }
            break;
        default:
            deviceText = i18nc("title of the interface widget in nm's popup", "Wired Ethernet");
            break;
    }
    return deviceText;
}

QString UiUtils::iconName(Solid::Control::NetworkInterfaceNm09 *iface)
{
    if (!iface) {
        return QString("dialog-error");
    }
    QString icon;

    switch (iface->type()) {
        case Solid::Control::NetworkInterfaceNm09::Ethernet: {
            icon = "network-wired";

            Solid::Control::WiredNetworkInterfaceNm09 *wiredIface = qobject_cast<Solid::Control::WiredNetworkInterfaceNm09*>(iface);
            if (wiredIface && wiredIface->carrier()) {
                icon = "network-wired-activated";
            }
            break;
        }
        case Solid::Control::NetworkInterfaceNm09::Wifi: {
            QString strength = "00";
            Solid::Control::WirelessNetworkInterfaceNm09 *wiface = qobject_cast<Solid::Control::WirelessNetworkInterfaceNm09*>(iface);

            if (wiface) {
                QString uni = wiface->activeAccessPoint();
                //QString uni = wiface->activeAccessPoint()->signalStrength();
                //int s =
                Solid::Control::AccessPointNm09 *ap = wiface->findAccessPoint(uni);
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
        }
        case Solid::Control::NetworkInterfaceNm09::Bluetooth:
            icon = "preferences-system-bluetooth";
            break;
        case Solid::Control::NetworkInterfaceNm09::Modem:
            icon = "phone";
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
    int s = qMin(size.width(), size.height());
    // return the biggest fitting icon size from KIconLoader
    if (s >= KIconLoader::SizeEnormous) { // 128
        s = KIconLoader::SizeEnormous;
    } else if (s >= KIconLoader::SizeHuge) { // 64
        s = KIconLoader::SizeHuge;
    } else if (s >= KIconLoader::SizeLarge) { // 48
        s = KIconLoader::SizeLarge;
    } else if (s >= KIconLoader::SizeMedium) { // 32
        s = KIconLoader::SizeMedium;
    } else if (s >= KIconLoader::SizeSmallMedium) { // 22
        s = KIconLoader::SizeSmallMedium;
    } else { // 16
        s = KIconLoader::SizeSmall;
    }
    return s;
}

QString UiUtils::connectionStateToString(Solid::Control::NetworkInterfaceNm09::ConnectionState state, const QString &connectionName)
{
    QString stateString;
    switch (state) {
        case Solid::Control::NetworkInterfaceNm09::UnknownState:
            stateString = i18nc("description of unknown network interface state", "Unknown");
            break;
        case Solid::Control::NetworkInterfaceNm09::Unmanaged:
            stateString = i18nc("description of unmanaged network interface state", "Unmanaged");
            break;
        case Solid::Control::NetworkInterfaceNm09::Unavailable:
            stateString = i18nc("description of unavailable network interface state", "Unavailable");
            break;
        case Solid::Control::NetworkInterfaceNm09::Disconnected:
            stateString = i18nc("description of unconnected network interface state", "Not connected");
            break;
        case Solid::Control::NetworkInterfaceNm09::Preparing:
            stateString = i18nc("description of preparing to connect network interface state", "Preparing to connect");
            break;
        case Solid::Control::NetworkInterfaceNm09::Configuring:
            stateString = i18nc("description of configuring hardware network interface state", "Configuring interface");
            break;
        case Solid::Control::NetworkInterfaceNm09::NeedAuth:
            stateString = i18nc("description of waiting for authentication network interface state", "Waiting for authorization");
            break;
        case Solid::Control::NetworkInterfaceNm09::IPConfig:
            stateString = i18nc("network interface doing dhcp request in most cases", "Setting network address");
            break;
        case Solid::Control::NetworkInterfaceNm09::IPCheck:
            stateString = i18nc("is other action required to fully connect? captive portals, etc.", "Checking further connectivity");
            break;
        case Solid::Control::NetworkInterfaceNm09::Secondaries:
            stateString = i18nc("a secondary connection (e.g. VPN) has to be activated first to continue", "Waiting for a secondary connection");
            break;
        case Solid::Control::NetworkInterfaceNm09::Activated:
            if (connectionName.isEmpty()) {
                stateString = i18nc("network interface connected state label", "Connected");
            } else {
                stateString = i18nc("network interface connected state label", "Connected to %1", connectionName);
            }
            break;
        case Solid::Control::NetworkInterfaceNm09::Deactivating:
            stateString = i18nc("network interface disconnecting state label", "Deactivating connection");
            break;
        case Solid::Control::NetworkInterfaceNm09::Failed:
            stateString = i18nc("network interface connection failed state label", "Connection Failed");
            break;
        default:
            stateString = i18nc("interface state", "Error: Invalid state");
    }
    return stateString;
}

QString UiUtils::connectionStateToString(Knm::InterfaceConnection::ActivationState state, const QString &connectionName)
{
    Solid::Control::NetworkInterfaceNm09::ConnectionState s = Solid::Control::NetworkInterfaceNm09::UnknownState;

    switch(state) {
    case Knm::InterfaceConnection::Unknown:
        s = Solid::Control::NetworkInterfaceNm09::UnknownState;
        break;
    case Knm::InterfaceConnection::Activating:
        s = Solid::Control::NetworkInterfaceNm09::Configuring;
        break;
    case Knm::InterfaceConnection::Activated:
        s = Solid::Control::NetworkInterfaceNm09::Activated;
        break;
    }

    return connectionStateToString(s, connectionName);
}

Solid::Device* UiUtils::findSolidDevice(const QString & uni)
{
    Solid::Control::NetworkInterfaceNm09 * iface = Solid::Control::NetworkManagerNm09::findNetworkInterface(uni);

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
    Solid::Control::NetworkInterfaceNm09 * iface = Solid::Control::NetworkManagerNm09::findNetworkInterface(uni);
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
                    label = i18nc("Format for <Vendor> <Product>", "%1 - %2", dev->vendor(), dev->product());
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
            label = UiUtils::interfaceTypeLabel(iface->type(), iface);
        }
    }
    if (dev) {
        delete dev;
    }
    return label;
}

QString UiUtils::interfaceNameLabel(const QString & uni)
{
    KNetworkManagerServicePrefs::instance(Knm::NETWORKMANAGEMENT_RCFILE);

    return interfaceNameLabel(uni, static_cast<KNetworkManagerServicePrefs::InterfaceNamingChoices>(KNetworkManagerServicePrefs::self()->interfaceNamingStyle()));
}

qreal UiUtils::interfaceState(const Solid::Control::NetworkInterfaceNm09 *interface)
{
    if (!interface) {
        return 0;
    }

    // from libs/types.h
    switch (interface->connectionState()) {
        case Solid::Control::NetworkInterfaceNm09::Preparing:
            return 0.15;
            break;
        case Solid::Control::NetworkInterfaceNm09::Configuring:
            return 0.30;
            break;
        case Solid::Control::NetworkInterfaceNm09::NeedAuth:
            return 0.45;
            break;
        case Solid::Control::NetworkInterfaceNm09::IPConfig:
            return 0.60;
            break;
        case Solid::Control::NetworkInterfaceNm09::IPCheck:
            return 0.75;
            break;
        case Solid::Control::NetworkInterfaceNm09::Secondaries:
            return 0.90;
            break;
        case Solid::Control::NetworkInterfaceNm09::Activated:
            return 1.0;
            break;
        default:
            return 0;
            break;
    }
    return 0;
}

QString UiUtils::operationModeToString(Solid::Control::WirelessNetworkInterfaceNm09::OperationMode mode)
{
    QString modeString;
    switch (mode) {
        case Solid::Control::WirelessNetworkInterfaceNm09::Unassociated:
            modeString = i18nc("wireless network operation mode", "Unassociated");
            break;
        case Solid::Control::WirelessNetworkInterfaceNm09::Adhoc:
            modeString = i18nc("wireless network operation mode", "Adhoc");
            break;
        case Solid::Control::WirelessNetworkInterfaceNm09::Managed:
            modeString = i18nc("wireless network operation mode", "Managed");
            break;
        case Solid::Control::WirelessNetworkInterfaceNm09::Master:
            modeString = i18nc("wireless network operation mode", "Master");
            break;
        case Solid::Control::WirelessNetworkInterfaceNm09::Repeater:
            modeString = i18nc("wireless network operation mode", "Repeater");
            break;
        default:
            modeString = I18N_NOOP("INCORRECT MODE FIX ME");
    }
    return modeString;
}

QStringList UiUtils::wpaFlagsToStringList(Solid::Control::AccessPointNm09::WpaFlags flags)
{
    /* for testing purposes
    flags = Solid::Control::AccessPointNm09::PairWep40
            | Solid::Control::AccessPointNm09::PairWep104
            | Solid::Control::AccessPointNm09::PairTkip
            | Solid::Control::AccessPointNm09::PairCcmp
            | Solid::Control::AccessPointNm09::GroupWep40
            | Solid::Control::AccessPointNm09::GroupWep104
            | Solid::Control::AccessPointNm09::GroupTkip
            | Solid::Control::AccessPointNm09::GroupCcmp
            | Solid::Control::AccessPointNm09::KeyMgmtPsk
            | Solid::Control::AccessPointNm09::KeyMgmt8021x; */

    QStringList flagList;

    if (flags.testFlag(Solid::Control::AccessPointNm09::PairWep40))
        flagList.append(i18nc("wireless network cipher", "Pairwise WEP40"));
    if (flags.testFlag(Solid::Control::AccessPointNm09::PairWep104))
        flagList.append(i18nc("wireless network cipher", "Pairwise WEP104"));
    if (flags.testFlag(Solid::Control::AccessPointNm09::PairTkip))
        flagList.append(i18nc("wireless network cipher", "Pairwise TKIP"));
    if (flags.testFlag(Solid::Control::AccessPointNm09::PairCcmp))
        flagList.append(i18nc("wireless network cipher", "Pairwise CCMP"));
    if (flags.testFlag(Solid::Control::AccessPointNm09::GroupWep40))
        flagList.append(i18nc("wireless network cipher", "Group WEP40"));
    if (flags.testFlag(Solid::Control::AccessPointNm09::GroupWep104))
        flagList.append(i18nc("wireless network cipher", "Group WEP104"));
    if (flags.testFlag(Solid::Control::AccessPointNm09::GroupTkip))
        flagList.append(i18nc("wireless network cipher", "Group TKIP"));
    if (flags.testFlag(Solid::Control::AccessPointNm09::GroupCcmp))
        flagList.append(i18nc("wireless network cipher", "Group CCMP"));
    if (flags.testFlag(Solid::Control::AccessPointNm09::KeyMgmtPsk))
        flagList.append(i18nc("wireless network cipher", "PSK"));
    if (flags.testFlag(Solid::Control::AccessPointNm09::KeyMgmt8021x))
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

QString UiUtils::macAddressAsString(const QByteArray & ba)
{
    QStringList mac;

    for (int i=0; i < ba.size(); i++)
    {
        mac << QString("%1").arg((quint8)ba[i], 2, 16, QLatin1Char('0')).toUpper();
    }

    return mac.join(":");
}

QByteArray UiUtils::macAddressFromString( const QString & s)
{
    QStringList macStringList = s.split(":");
    QByteArray ba;
    if (!s.isEmpty())
    {
        ba.resize(6);
        int i = 0;

        foreach (const QString & macPart, macStringList)
            ba[i++] = macPart.toUInt(0, 16);
    }
    return ba;
}

QPair<int, int> UiUtils::findBandAndChannel(int freq)
{
    UiUtilsPrivate *priv = s_UiUtilsPrivate;
    QPair<int, int> pair;

    if (freq < 2500) {
        pair.first = Knm::WirelessSetting::EnumBand::bg;
        pair.second = 0;
        int i = 0;
        QList<QPair<int, int> > bFreqs = priv->getBFreqs();
        while (i < bFreqs.size()) {
            if (bFreqs.at(i).second <= freq) {
                pair.second = bFreqs.at(i).first;
            } else {
                break;
            }
            i++;
        }
        return pair;
    }
    pair.second = 0;
    int i = 0;
    QList<QPair<int, int> > aFreqs = priv->getAFreqs();
    while (i < aFreqs.size()) {
        if (aFreqs.at(i).second <= freq) {
            pair.second = aFreqs.at(i).first;
        } else {
            break;
        }
        i++;
    }

    pair.first = Knm::WirelessSetting::EnumBand::a;

    return pair;
}

QString UiUtils::wirelessBandToString(int band)
{
    switch (band)
    {
        case Knm::WirelessSetting::EnumBand::a:
            return QLatin1String("a");
            break;
        case Knm::WirelessSetting::EnumBand::bg:
            return QLatin1String("b/g");
            break;
    }
    return QString();
}

// vim: sw=4 sts=4 et tw=100
