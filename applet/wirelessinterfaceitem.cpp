/*
Copyright 2008,2009 Will Stephenson <wstephenson@kde.org>
Copyright 2008-2010 Sebastian K?gler <sebas@kde.org>

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

#include "wirelessinterfaceitem.h"
#include "wirelessstatus.h"
#include "uiutils.h"
#include "remotewirelessinterfaceconnection.h"

#include <QGraphicsGridLayout>
#include <QLabel>

#include <KIcon> 

#include <solid/control/wirelessaccesspoint.h>
#include <solid/control/wirelessnetworkinterface.h>
#include <solid/control/networkinterface.h>
#include <solid/control/networkipv4confignm09.h>
#include <solid/control/networkmanager.h>

WirelessInterfaceItem::WirelessInterfaceItem(Solid::Control::WirelessNetworkInterfaceNm09 * iface, RemoteActivatableList* activatables, InterfaceItem::NameDisplayMode mode, QGraphicsWidget* parent)
: InterfaceItem(iface, activatables, mode, parent)
{
    connect(iface, SIGNAL(activeAccessPointChanged(QString)),
            SLOT(updateInfo()));

    m_wirelessStatus = new WirelessStatus(iface);
    connect(m_wirelessStatus, SIGNAL(strengthChanged(int)), this, SLOT(updateInfo()));
    setConnectionInfo();
}

WirelessInterfaceItem::~WirelessInterfaceItem()
{
    delete m_wirelessStatus;
}

void WirelessInterfaceItem::updateInfo()
{
    setConnectionInfo();
}

void WirelessInterfaceItem::connectButtonClicked()
{
    // TODO
}

void WirelessInterfaceItem::setConnectionInfo()
{
    if (!m_iface) {
        return;
    }
    InterfaceItem::setConnectionInfo(); // Sets the labels
    switch (m_iface.data()->connectionState()) {
        case Solid::Control::NetworkInterfaceNm09::Unavailable:
        case Solid::Control::NetworkInterfaceNm09::Disconnected:
        case Solid::Control::NetworkInterfaceNm09::Failed:
            m_connectionInfoIcon->hide();
            break;
        default:
        {
            if (!currentConnection()) {
                m_connectionInfoIcon->hide();
            } else {
                RemoteWirelessInterfaceConnection* wremote =
                                    static_cast<RemoteWirelessInterfaceConnection*>(currentConnection());
                if (wremote) {
                    m_wirelessStatus->init(wremote);
                }
                if (!m_wirelessStatus->securityIcon().isEmpty()) {
                    m_connectionInfoIcon->nativeWidget()->setPixmap(KIcon(m_wirelessStatus->securityIcon()).pixmap(16, 16));

                    m_connectionInfoIcon->setToolTip(m_wirelessStatus->securityTooltip());
                    m_connectionInfoIcon->show();
                } else {
                    m_connectionInfoIcon->hide();
                }
            }
            break;
        }
    }
}

// vim: sw=4 sts=4 et tw=100
