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


#include "vpnconnectiongroup.h"

#include <NetworkManager.h>

#include <KDebug>

#include <solid/control/networkmanager.h>

#include "../libs/types.h"
#include "connectionitem.h"
#include "nm-active-connectioninterface.h"
#include "nm-setting-vpn.h"
#include "remoteconnection.h"

VpnConnectionGroup::VpnConnectionGroup(NetworkManagerSettings * userSettings, NetworkManagerSettings * systemSettings, QWidget * parent)
    : ConnectionList(userSettings, systemSettings, parent)
{
}

VpnConnectionGroup::~VpnConnectionGroup()
{

}

void VpnConnectionGroup::setupHeader()
{
}

bool VpnConnectionGroup::accept(RemoteConnection* connection) const
{
    QVariantMapMap settings = connection->settings();
    return settings.contains(QLatin1String(NM_SETTING_VPN_SETTING_NAME));
}

void VpnConnectionGroup::activateConnection(AbstractConnectableItem* item)
{
    QStringList actives = Solid::Control::NetworkManager::activeConnections();
    QString active, device;
    foreach ( QString acon, actives )
    {
        QDBusInterface con("org.freedesktop.NetworkManager", acon, "org.freedesktop.NetworkManager.Connection.Active", QDBusConnection::systemBus());
        int state = con.property( "State" ).toInt();

        bool isDefault = con.property( "Default" ).toBool();
        if ( isDefault && state == NM_ACTIVE_CONNECTION_STATE_ACTIVATED )
        {
            active = acon;
            QList<QDBusObjectPath> devs = con.property( "Devices" ).value<QList<QDBusObjectPath> >();
            device = devs[0].path(); // pick the first one
            break;
        }
    }

    kDebug() << "active" << active << "device" << device;
    if ( active.isEmpty() || device.isEmpty() )
        return;

    ConnectionItem * ci = qobject_cast<ConnectionItem*>(item);

    kDebug() << "Activating VPN connection" << ci->connection()->path() << "from" << ci->connection()->service() << "on" << device << "connection" << active;
    QVariantMap map;
    map.insert( "extra_connection_parameter", active );
    Solid::Control::NetworkManager::activateConnection(device,
                                                       ci->connection()->service() + " " + ci->connection()->path(),
                                                       map );

}

void VpnConnectionGroup::setupFooter()
{
}

// vim: sw=4 sts=4 et tw=100
