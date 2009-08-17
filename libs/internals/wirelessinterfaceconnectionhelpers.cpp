/*
Copyright 2009 Paul Marchouk <pmarchouk@gmail.com>

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
#include "wirelessinterfaceconnection.h"
#include "wirelessnetworkinterfaceenvironment.h"

using namespace Knm;

Knm::WirelessInterfaceConnection* WirelessInterfaceConnectionHelpers::buildInterfaceConnection(
        Solid::Control::WirelessNetworkInterface *interface,
        Knm::Connection *connection, const QString & deviceUni, QObject * parent)
{
    WirelessInterfaceConnectionBuilder builder(interface, connection, deviceUni, parent);

    return builder.build();
}

void WirelessInterfaceConnectionHelpers::syncInterfaceConnection(WirelessInterfaceConnection *ic, Knm::Connection *c)
{
    WirelessInterfaceConnectionSync sync;

    sync.sync(ic, c);
}

WirelessInterfaceConnectionBuilder::WirelessInterfaceConnectionBuilder(
                    Solid::Control::WirelessNetworkInterface *interface,
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
        std::auto_ptr<Solid::Control::WirelessNetworkInterfaceEnvironment> apEnvironment(
                new Solid::Control::WirelessNetworkInterfaceEnvironment(m_interface));

        int strength = -1;
        Solid::Control::AccessPoint::Capabilities caps = 0;
        Solid::Control::AccessPoint::WpaFlags wpaFlags = 0;
        Solid::Control::AccessPoint::WpaFlags rsnFlags = 0;
        Solid::Control::WirelessNetworkInterface::OperationMode mode
            = Solid::Control::WirelessNetworkInterface::Master;

        // show connections where the network is present OR adhoc connections
        if (wirelessSetting->mode() == Knm::WirelessSetting::EnumMode::adhoc
            || apEnvironment->networks().contains(wirelessSetting->ssid())) {
            // get the info on the network
            Solid::Control::WirelessNetwork *network = apEnvironment->findNetwork(wirelessSetting->ssid());

            if (network) {
                strength = network->signalStrength();
                Solid::Control::AccessPoint * ap = m_interface->findAccessPoint(network->referenceAccessPoint());
                if (ap) {
                    caps = ap->capabilities();
                    wpaFlags = ap->wpaFlags();
                    rsnFlags = ap->rsnFlags();
                    mode = ap->mode();
                }
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
