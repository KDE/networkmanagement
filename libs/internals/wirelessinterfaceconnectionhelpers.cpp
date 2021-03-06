/*
Copyright 2009 Paul Marchouk <pmarchouk@gmail.com>
Copyright 2011-2013 Lamarque V. Souza <lamarque@kde.org>

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

#include "wirelessinterfaceconnectionhelpers.h"
#include "wirelessinterfaceconnectionhelpers_p.h"

#include <memory>

#include "connection.h"
#include "settings/802-11-wireless.h"
#include "settings/802-11-wireless-security.h"
#include "hiddenwirelessinterfaceconnection.h"
#include "wirelessinterfaceconnection.h"

using namespace Knm;

Knm::WirelessInterfaceConnection* WirelessInterfaceConnectionHelpers::buildWirelessInterfaceConnection(
        const NetworkManager::WirelessDevice::Ptr &interface,
        Knm::Connection *connection, const QString & deviceUni, QObject * parent)
{
    WirelessInterfaceConnectionBuilder builder(interface, connection, deviceUni, parent);

    return builder.build();
}

Knm::HiddenWirelessInterfaceConnection* WirelessInterfaceConnectionHelpers::buildHiddenWirelessInterfaceConnection(
        const NetworkManager::WirelessDevice::Ptr &interface,
        Knm::Connection *connection, const QString & deviceUni, QObject * parent)
{
    HiddenWirelessInterfaceConnectionBuilder builder(interface, connection, deviceUni, parent);

    return builder.build();
}

void WirelessInterfaceConnectionHelpers::syncWirelessInterfaceConnection(WirelessInterfaceConnection *ic, Knm::Connection *c)
{
    WirelessInterfaceConnectionSync sync;

    sync.sync(ic, c);
}

WirelessInterfaceConnectionBuilder::WirelessInterfaceConnectionBuilder(const NetworkManager::WirelessDevice::Ptr &interface,
                    Knm::Connection *connection,
                    const QString &deviceUni,
                    QObject *parent)
: InterfaceConnectionBuilder(connection, deviceUni, parent), m_interface(interface)
{
}

WirelessInterfaceConnectionBuilder::~WirelessInterfaceConnectionBuilder()
{
}

Knm::WirelessInterfaceConnection* WirelessInterfaceConnectionBuilder::build()
{
    WirelessInterfaceConnection *ic =
            new WirelessInterfaceConnection(Activatable::WirelessInterfaceConnection,
                                            m_deviceUni, m_parent);

    if (ic) {
        init(ic);
    }

    return ic;
}

void WirelessInterfaceConnectionBuilder::init(WirelessInterfaceConnection *ic)
{
    InterfaceConnectionBuilder::init(ic);

    Knm::WirelessSetting * wirelessSetting = dynamic_cast<Knm::WirelessSetting *>(m_connection->setting(Knm::Setting::Wireless));
    if (wirelessSetting) {
        int strength = -1;
        NetworkManager::AccessPoint::Capabilities caps = 0;
        NetworkManager::AccessPoint::WpaFlags wpaFlags = 0;
        NetworkManager::AccessPoint::WpaFlags rsnFlags = 0;
        NetworkManager::AccessPoint::OperationMode mode
            = NetworkManager::AccessPoint::Infra;

        // show connections where the network is present OR adhoc connections
        NetworkManager::WirelessNetwork::Ptr network = m_interface->findNetwork(wirelessSetting->ssid());
        if (network) {
            strength = network->signalStrength();
            NetworkManager::AccessPoint::Ptr ap = network->referenceAccessPoint();
            if (ap) {
                caps = ap->capabilities();
                wpaFlags = ap->wpaFlags();
                rsnFlags = ap->rsnFlags();
                mode = ap->mode();
            }
        }
        else if (wirelessSetting->mode() == Knm::WirelessSetting::EnumMode::adhoc ||
                 wirelessSetting->mode() == Knm::WirelessSetting::EnumMode::apMode) {
                mode = (wirelessSetting->mode() == Knm::WirelessSetting::EnumMode::adhoc) ? NetworkManager::AccessPoint::Adhoc : NetworkManager::AccessPoint::ApMode;
                Knm::WirelessSecuritySetting * wirelessSecuritySetting = dynamic_cast<Knm::WirelessSecuritySetting *>(m_connection->setting(Knm::Setting::WirelessSecurity));
                switch( wirelessSecuritySetting->securityType())
                {
                    case Knm::WirelessSecuritySetting::EnumSecurityType::StaticWep:
                    case Knm::WirelessSecuritySetting::EnumSecurityType::Leap:
                    case Knm::WirelessSecuritySetting::EnumSecurityType::DynamicWep:
                        caps |= NetworkManager::AccessPoint::Privacy;
                        break;
                    case Knm::WirelessSecuritySetting::EnumSecurityType::WpaPsk:
                        wpaFlags |= NetworkManager::AccessPoint::KeyMgmtPsk | NetworkManager::AccessPoint::PairTkip | NetworkManager::AccessPoint::PairCcmp;
                        break;
                    case Knm::WirelessSecuritySetting::EnumSecurityType::WpaEap:
                        wpaFlags |= NetworkManager::AccessPoint::KeyMgmt8021x;
                        break;
                    case Knm::WirelessSecuritySetting::EnumSecurityType::Wpa2Psk:
                        rsnFlags |= NetworkManager::AccessPoint::KeyMgmtPsk | NetworkManager::AccessPoint::PairTkip | NetworkManager::AccessPoint::PairCcmp;
                        break;
                    case Knm::WirelessSecuritySetting::EnumSecurityType::Wpa2Eap:
                        rsnFlags |= NetworkManager::AccessPoint::KeyMgmt8021x;
                        break;
                    case Knm::WirelessSecuritySetting::EnumSecurityType::None:
                    default:
                        break;
                }
            }

        ic->m_interfaceCapabilities = m_interface->wirelessCapabilities();
        ic->m_ssid = wirelessSetting->ssid();
        ic->m_strength = strength;
        ic->m_apCapabilities = caps;
        ic->m_wpaFlags = wpaFlags;
        ic->m_rsnFlags = rsnFlags;
        ic->m_operationMode = mode;
    }
}

HiddenWirelessInterfaceConnectionBuilder::HiddenWirelessInterfaceConnectionBuilder(const NetworkManager::WirelessDevice::Ptr &interface,
                    Knm::Connection *connection,
                    const QString &deviceUni,
                    QObject *parent)
: WirelessInterfaceConnectionBuilder(interface, connection, deviceUni, parent)
{
}

HiddenWirelessInterfaceConnectionBuilder::~HiddenWirelessInterfaceConnectionBuilder()
{
}

Knm::HiddenWirelessInterfaceConnection* HiddenWirelessInterfaceConnectionBuilder::build()
{
    HiddenWirelessInterfaceConnection *ic =
        new HiddenWirelessInterfaceConnection(Activatable::HiddenWirelessInterfaceConnection,
                m_deviceUni, m_parent);

    if (ic) {
        init(ic);
    }

    return ic;
}

WirelessInterfaceConnectionSync::WirelessInterfaceConnectionSync()
{
}

WirelessInterfaceConnectionSync::~WirelessInterfaceConnectionSync()
{
}

void WirelessInterfaceConnectionSync::sync(Knm::WirelessInterfaceConnection * interfaceConnection,
                                           Knm::Connection *connection)
{
    InterfaceConnectionSync::sync(interfaceConnection, connection);
}
