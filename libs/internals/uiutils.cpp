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

// KDE
#include <KDebug>
#include <KIconLoader>
#include <KLocale>
#include <kdeversion.h>

#include <solid/control/networkmanager.h>
#include <solid/control/networkinterface.h>
#include <solid/control/wirelessaccesspoint.h>
#include <solid/control/wirelessnetworkinterface.h>

// Qt
#include <QSizeF>

#include "knmserviceprefs.h"

QString UiUtils::interfaceTypeLabel(const Solid::Control::NetworkInterface::Type type)
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
    if (!iface) {
        return QString("dialog-error");
    }
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
                Solid::Control::AccessPoint *ap = wiface->findAccessPoint(uni);
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
    } else if (c >= KIconLoader::SizeSmallMedium) { // 32
        s = KIconLoader::SizeSmallMedium;
    } else { // 16
        s = KIconLoader::SizeSmall;
    }
    return s;
}

QString UiUtils::connectionStateToString(Solid::Control::NetworkInterface::ConnectionState state, const QString &connectionName)
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
            if (connectionName.isEmpty()) {
                stateString = i18nc("network interface connected state label", "Connected");
            } else {
                stateString = i18nc("network interface connected state label", "Connected to %1", connectionName);
            }
            break;
        case Solid::Control::NetworkInterface::Failed:
            stateString = i18nc("network interface connection failed state label", "Connection Failed");
            break;
        default:
            stateString = i18nc("interface state", "Error: Invalid state");
    }
    return stateString;
}

Solid::Device* UiUtils::findSolidDevice(const QString & uni)
{
    Solid::Control::NetworkInterface * iface = Solid::Control::NetworkManager::findNetworkInterface(uni);

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

QString UiUtils::interfaceNameLabel(const QString & uni)
{
    KNetworkManagerServicePrefs::instance(Knm::ConnectionPersistence::NETWORKMANAGEMENT_RCFILE);
    QString label;
    Solid::Control::NetworkInterface * iface = Solid::Control::NetworkManager::findNetworkInterface(uni);
    Solid::Device* dev = findSolidDevice(uni);

    switch (KNetworkManagerServicePrefs::self()->interfaceNamingStyle()) {
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

RemoteInterfaceConnection* UiUtils::connectionForInterface(RemoteActivatableList* activatables, Solid::Control::NetworkInterface *interface)
{
    foreach (RemoteActivatable* activatable, activatables->activatables()) {
        if (activatable->deviceUni() == interface->uni()) {
            RemoteInterfaceConnection* remoteconnection = dynamic_cast<RemoteInterfaceConnection*>(activatable);
            if (remoteconnection) {
                if (remoteconnection->activationState() == Knm::InterfaceConnection::Activated
                            || remoteconnection->activationState() == Knm::InterfaceConnection::Activating) {
                    return remoteconnection;
                }
            }

        }
    }
    return 0;
}


qreal UiUtils::interfaceState(const Solid::Control::NetworkInterface *interface)
{
    if (!interface) {
        return 0;
    }
    switch (interface->connectionState()) {
        case Solid::Control::NetworkInterface::Preparing:
            return 0.20;
            break;
        case Solid::Control::NetworkInterface::Configuring:
            return 0.40;
            break;
        case Solid::Control::NetworkInterface::NeedAuth:
            return 0.60;
            break;
        case Solid::Control::NetworkInterface::IPConfig:
            return 0.80;
            break;
        case Solid::Control::NetworkInterface::Activated:
            return 1.0;
            break;
        default:
            return 0;
            break;
    }
    return 0;
}

QString UiUtils::operationModeToString(Solid::Control::WirelessNetworkInterface::OperationMode mode)
{
    QString modeString;
    switch (mode) {
        case Solid::Control::WirelessNetworkInterface::Unassociated:
            modeString = i18nc("wireless network operation mode", "Unassociated");
            break;
        case Solid::Control::WirelessNetworkInterface::Adhoc:
            modeString = i18nc("wireless network operation mode", "Adhoc");
            break;
        case Solid::Control::WirelessNetworkInterface::Managed:
            modeString = i18nc("wireless network operation mode", "Managed");
            break;
        case Solid::Control::WirelessNetworkInterface::Master:
            modeString = i18nc("wireless network operation mode", "Master");
            break;
        case Solid::Control::WirelessNetworkInterface::Repeater:
            modeString = i18nc("wireless network operation mode", "Repeater");
            break;
        default:
            modeString = I18N_NOOP("INCORRECT MODE FIX ME");
    }
    return modeString;
}

QStringList UiUtils::wpaFlagsToStringList(Solid::Control::AccessPoint::WpaFlags flags)
{
    /* for testing purposes
    flags = Solid::Control::AccessPoint::PairWep40
            | Solid::Control::AccessPoint::PairWep104
            | Solid::Control::AccessPoint::PairTkip
            | Solid::Control::AccessPoint::PairCcmp
            | Solid::Control::AccessPoint::GroupWep40
            | Solid::Control::AccessPoint::GroupWep104
            | Solid::Control::AccessPoint::GroupTkip
            | Solid::Control::AccessPoint::GroupCcmp
            | Solid::Control::AccessPoint::KeyMgmtPsk
            | Solid::Control::AccessPoint::KeyMgmt8021x; */

    QStringList flagList;

    if (flags.testFlag(Solid::Control::AccessPoint::PairWep40))
        flagList.append(i18nc("wireless network cipher", "Pairwise WEP40"));
    if (flags.testFlag(Solid::Control::AccessPoint::PairWep104))
        flagList.append(i18nc("wireless network cipher", "Pairwise WEP104"));
    if (flags.testFlag(Solid::Control::AccessPoint::PairTkip))
        flagList.append(i18nc("wireless network cipher", "Pairwise TKIP"));
    if (flags.testFlag(Solid::Control::AccessPoint::PairCcmp))
        flagList.append(i18nc("wireless network cipher", "Pairwise CCMP"));
    if (flags.testFlag(Solid::Control::AccessPoint::GroupWep40))
        flagList.append(i18nc("wireless network cipher", "Group WEP40"));
    if (flags.testFlag(Solid::Control::AccessPoint::GroupWep104))
        flagList.append(i18nc("wireless network cipher", "Group WEP104"));
    if (flags.testFlag(Solid::Control::AccessPoint::GroupTkip))
        flagList.append(i18nc("wireless network cipher", "Group TKIP"));
    if (flags.testFlag(Solid::Control::AccessPoint::GroupCcmp))
        flagList.append(i18nc("wireless network cipher", "Group CCMP"));
    if (flags.testFlag(Solid::Control::AccessPoint::KeyMgmtPsk))
        flagList.append(i18nc("wireless network cipher", "PSK"));
    if (flags.testFlag(Solid::Control::AccessPoint::KeyMgmt8021x))
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
