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

#include "vpninterfaceitem.h"
//#include "uiutils.h"
#include "activatableitem.h"

//#include <QGraphicsGridLayout>
#include <QLabel>

//#include <solid/control/wirelessaccesspoint.h>
//#include <solid/control/wirelessnetworkinterface.h>
#include <solid/control/networkinterface.h>
//#include <solid/control/networkipv4config.h>
//#include <solid/control/networkmanager.h>

#include "remoteactivatable.h"
#include "remoteactivatablelist.h"
#include "remoteinterfaceconnection.h"


VpnInterfaceItem::VpnInterfaceItem(Solid::Control::NetworkInterface * iface, RemoteActivatableList* activatables, InterfaceItem::NameDisplayMode mode, QGraphicsWidget* parent)
: InterfaceItem(iface, activatables, mode, parent)
{
    m_icon->nativeWidget()->setPixmap(KIcon("secure-card").pixmap(48,48));
    m_ifaceNameLabel->setText(i18nc("VPN connections interface", "<b>Virtual Private Network</b>"));
    m_connectionNameLabel->setText(i18nc("initial label for VPN connection name", "Not Connected"));
}

VpnInterfaceItem::~VpnInterfaceItem()
{
}

RemoteInterfaceConnection* VpnInterfaceItem::currentConnection()
{
    foreach (RemoteActivatable* activatable, m_activatables->activatables()) {
        if (activatable->activatableType() == Knm::Activatable::VpnInterfaceConnection) {
            RemoteInterfaceConnection* remoteconnection = static_cast<RemoteInterfaceConnection*>(activatable);
            if (remoteconnection) {
                //if (remoteconnection->activationState() == Knm::InterfaceConnection::Activated
                //            || remoteconnection->activationState() == Knm::InterfaceConnection::Activating) {
                    return remoteconnection;
                //}
            }

        }
    }
}

QString VpnInterfaceItem::connectionName()
{
    return QString("It's a VPN");
}



// vim: sw=4 sts=4 et tw=100
