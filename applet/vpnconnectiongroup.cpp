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
    // tell the manager to activate the connection
    // which device??
    // HACK - take the first one
    ConnectionItem * ci = qobject_cast<ConnectionItem*>(item);
    Solid::Control::NetworkInterfaceList activeInterfaces;
    foreach (Solid::Control::NetworkInterface * iface, Solid::Control::NetworkManager::networkInterfaces()) {
        if (iface->connectionState() == Solid::Control::NetworkInterface::Activated) {
            activeInterfaces.append(iface);
        }
    }
    if (activeInterfaces.count() == 1) {
        kDebug() << "Activating VPN connection " << ci->connection()->path() << " from " << ci->connection()->service() << " on " << activeInterfaces[0]->uni();
        Solid::Control::NetworkManager::activateConnection(activeInterfaces[0]->uni(), ci->connection()->service() + " " + ci->connection()->path(), QVariantMap());
    } else if (activeInterfaces.count() > 1) {
        // determine which interface holds the default route
        kDebug() << "More than one interface is active...";
        Solid::Control::NetworkInterface * interfaceToActivate = 0;
        QStringList activeConnections = Solid::Control::NetworkManager::activeConnections();
        foreach (QString conn, activeConnections) {
            OrgFreedesktopNetworkManagerConnectionActiveInterface candidate(NM_DBUS_SERVICE,
                    conn, QDBusConnection::systemBus(), 0);
            if (candidate.getDefault()) {
                foreach (QDBusObjectPath activeDevicePath, candidate.devices()) {
                    foreach (Solid::Control::NetworkInterface * iface, activeInterfaces) {
                        if (iface->uni() == activeDevicePath.path()) {
                            interfaceToActivate = iface;
                            break;
                        }
                    }
                }
                break;
            }
        }
        if (interfaceToActivate) {
            Solid::Control::NetworkManager::activateConnection(interfaceToActivate->uni(), ci->connection()->service() + " " + ci->connection()->path(), QVariantMap());
        } else {
            kDebug() << "couldn't identify the interface with the default route, not activating the connection";
        }
    }
}

void VpnConnectionGroup::setupFooter()
{
}

// vim: sw=4 sts=4 et tw=100
