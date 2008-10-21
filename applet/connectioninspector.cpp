/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>

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

#include "connectioninspector.h"
#include <nm-setting-wireless.h>

#include <KDebug>

#include <solid/control/wirednetworkinterface.h>
#include <solid/control/wirelessaccesspoint.h>
#include <solid/control/wirelessnetworkinterface.h>

#include "remoteconnection.h"

ConnectionInspector::~ConnectionInspector()
{
}

bool ConnectionInspector::accept( RemoteConnection * connection)
{
    return !connection->active();
}

WiredConnectionInspector::WiredConnectionInspector(Solid::Control::WiredNetworkInterface* iface)
: m_iface(iface)
{ }

bool WiredConnectionInspector::accept(RemoteConnection * connection)
{
    return (connection->type() == Solid::Control::NetworkInterface::Ieee8023 && m_iface->carrier()) && ConnectionInspector::accept(connection);
}

WirelessConnectionInspector::WirelessConnectionInspector(Solid::Control::WirelessNetworkInterface* iface)
    : m_iface(iface)
{
}

bool WirelessConnectionInspector::accept(RemoteConnection * connection)
{
    bool acceptable = false;
    if (connection->type() == Solid::Control::NetworkInterface::Ieee80211) {
        // check if the essid in the connection matches one of the access points returned by NM
        // on this device.
        // If an AP is hiding the essid, but one of the Settings services provides a connection
        // with this essid, NM will add the essid to the AP object, so we can use this technique
        // even for hidden networks

        QString ssid;
        QVariantMapMap settings = connection->settings();
        if ( settings.contains(QLatin1String(NM_SETTING_WIRELESS_SETTING_NAME))) {
            QVariantMap connectionSetting = settings.value(QLatin1String(NM_SETTING_WIRELESS_SETTING_NAME));
            if (connectionSetting.contains(QLatin1String(NM_SETTING_WIRELESS_SSID))) {
                ssid = connectionSetting.value(QLatin1String(NM_SETTING_WIRELESS_SSID)).toString();
                foreach (QString accessPointUni, m_iface->accessPoints()) {
                    Solid::Control::AccessPoint * ap = m_iface->findAccessPoint(accessPointUni);
                    kDebug() << "Checking AP essid: " << ap->ssid() << " vs connection essid: " << ssid;
                    if (ap->ssid() == ssid) {
                        acceptable = true;
                    }
                }
            }
        }
    }
    return acceptable && ConnectionInspector::accept(connection);
}

bool GsmConnectionInspector::accept(RemoteConnection * connection)
{
    return connection->type() == Solid::Control::NetworkInterface::Gsm && ConnectionInspector::accept(connection);
}

bool CdmaConnectionInspector::accept(RemoteConnection * connection)
{
    return connection->type() == Solid::Control::NetworkInterface::Cdma && ConnectionInspector::accept(connection);
}

bool PppoeConnectionInspector::accept(RemoteConnection * connection)
{
    return connection->type() == Solid::Control::NetworkInterface::Serial && ConnectionInspector::accept(connection);
}

ConnectionInspector *ConnectionInspectorFactory::connectionInspector(Solid::Control::NetworkInterface* iface)
{
    ConnectionInspector * inspector = 0;
    if (!m_inspectors.contains(iface)) {
        switch (iface->type()) {
            case Solid::Control::NetworkInterface::Ieee8023:
                inspector = new WiredConnectionInspector(static_cast<Solid::Control::WiredNetworkInterface*>(iface));
                break;
            case Solid::Control::NetworkInterface::Ieee80211:
                inspector = new WirelessConnectionInspector(static_cast<Solid::Control::WirelessNetworkInterface*>(iface));
                break;
            case Solid::Control::NetworkInterface::Serial:
                inspector = new PppoeConnectionInspector;
                break;
            case Solid::Control::NetworkInterface::Gsm:
                inspector = new GsmConnectionInspector;
                break;
            case Solid::Control::NetworkInterface::Cdma:
                inspector = new CdmaConnectionInspector;
                break;
            default:
                kDebug() << "Unhandled network interface type : " << iface->type();
        }
        m_inspectors.insert(iface, inspector);
    } else {
        inspector = m_inspectors.value(iface);
    }
    Q_ASSERT(inspector);
    return inspector;
}

