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
#include "wirelessenvironment.h"

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

WiredConnectionInspector::~WiredConnectionInspector()
{}

bool WiredConnectionInspector::accept(RemoteConnection * connection)
{
    kDebug() << connection;
    kDebug() << connection->type();
    kDebug() << m_iface;
    kDebug() << m_iface->carrier();
    return (connection->type() == Solid::Control::NetworkInterface::Ieee8023 && m_iface->carrier()) && ConnectionInspector::accept(connection);
}

WirelessConnectionInspector::WirelessConnectionInspector(Solid::Control::WirelessNetworkInterface* iface, WirelessEnvironment * envt)
    : m_iface(iface), m_envt(envt)
{
}

WirelessConnectionInspector::~WirelessConnectionInspector()
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
                if ( m_envt->findNetwork(ssid) != 0 ) {
                    acceptable = true;
                }
            }
        }
    }
    return acceptable && ConnectionInspector::accept(connection);
}

GsmConnectionInspector::GsmConnectionInspector()
{
}

GsmConnectionInspector::~GsmConnectionInspector()
{
}

bool GsmConnectionInspector::accept(RemoteConnection * connection)
{
    return connection->type() == Solid::Control::NetworkInterface::Gsm && ConnectionInspector::accept(connection);
}

CdmaConnectionInspector::CdmaConnectionInspector()
{
}

CdmaConnectionInspector::~CdmaConnectionInspector()
{
}

bool CdmaConnectionInspector::accept(RemoteConnection * connection)
{
    return connection->type() == Solid::Control::NetworkInterface::Cdma && ConnectionInspector::accept(connection);
}

PppoeConnectionInspector::PppoeConnectionInspector()
{
}

PppoeConnectionInspector::~PppoeConnectionInspector()
{
}

bool PppoeConnectionInspector::accept(RemoteConnection * connection)
{
    return connection->type() == Solid::Control::NetworkInterface::Serial && ConnectionInspector::accept(connection);
}
