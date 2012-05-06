/*
Copyright 2010 Sebastian Kügler <sebas@kde.org>

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

// Qt
#include <QLabel>
#include <QTimer>

// KDE
#include <KDebug>
#include <KIcon>

// Network Management
#include <solid/control/networkinterface.h>
#include "vpninterfaceitem.h"
#include "activatableitem.h"
#include "remoteactivatable.h"
#include "remoteactivatablelist.h"
#include "remoteinterfaceconnection.h"


VpnInterfaceItem::VpnInterfaceItem(Solid::Control::NetworkInterfaceNm09 * iface, RemoteActivatableList* activatables, InterfaceItem::NameDisplayMode mode, QGraphicsWidget* parent)
: InterfaceItem(iface, activatables, mode, parent)
{
    m_icon->nativeWidget()->setPixmap(KIcon("secure-card").pixmap(48,48));
    m_connectionNameLabel->setText(i18nc("initial label for VPN connection name", "Not Available"));

    // Catch all kinds of signals to update the VPN widget

    connect(m_activatables, SIGNAL(activatableAdded(RemoteActivatable*,int)),
            SLOT(activatableAdded(RemoteActivatable*)));
    connect(m_activatables, SIGNAL(activatableRemoved(RemoteActivatable*)),
            SLOT(activatableRemoved(RemoteActivatable*)));

    connect(m_activatables, SIGNAL(appeared()), SLOT(listAppeared()));
    connect(m_activatables, SIGNAL(disappeared()), SLOT(listDisappeared()));

    //connect(this, SIGNAL(stateChanged()), this, SLOT(currentConnectionChanged()));

    connect(m_disconnectButton, SIGNAL(clicked()), this, SLOT(disconnectCurrentConnection()));

    // Update state
    listAppeared();
    setConnectionInfo();
}

VpnInterfaceItem::~VpnInterfaceItem()
{
}

RemoteInterfaceConnection* VpnInterfaceItem::currentConnection()
{
    return m_currentConnection;
}

void VpnInterfaceItem::disconnectCurrentConnection()
{
    if (m_currentConnection) {
        kDebug() << "deactivating:" << m_currentConnection->connectionName();
        m_currentConnection->deactivate();
    }
}

QString VpnInterfaceItem::connectionName()
{
    if (m_currentConnection) {
        return m_currentConnection->connectionName();
    } else {
        return QString("VPN");
    }
}


QString VpnInterfaceItem::currentIpAddress()
{
    return i18nc("ip of vpn interface item", "not available");
}

void VpnInterfaceItem::setConnectionInfo()
{
    bool showDisconnect = false;
    if (m_currentConnection) {
        m_ifaceNameLabel->setText(i18nc("VPN label in interfaces", "<b>%1</b>", m_currentConnection->connectionName()));
        if (m_currentConnection->activationState() == Knm::InterfaceConnection::Activated) {
            m_connectionNameLabel->setText(i18nc("VPN state label", "Connected"));
            showDisconnect = true;
        } else if (m_currentConnection->activationState() == Knm::InterfaceConnection::Activating) {
            m_connectionNameLabel->setText(i18nc("VPN state label", "Connecting..."));
            showDisconnect = true;
        } else {
            m_connectionNameLabel->setText("Impossible!");
        }
    } else {
        m_ifaceNameLabel->setText(i18nc("VPN connections interface", "<b>Virtual Private Network</b>"));
        m_connectionNameLabel->setText(i18nc("VPN state label", "Not Connected..."));
    }
    m_disconnectButton->setVisible(showDisconnect);
    setEnabled(showDisconnect);
    if (!m_vpnActivatables.isEmpty()) {
        //kDebug() << m_vpnActivatables.count() << "VPN connections have become available!";
        show();
    } else {
        //kDebug() << "hiding VPN widget:" << m_vpnActivatables.count();
        hide();
    }
    setEnabled(showDisconnect);
}

void VpnInterfaceItem::currentConnectionChanged()
{
    int vpns = 0;
    foreach (RemoteActivatable* activatable, m_activatables->activatables()) {
        if (activatable->activatableType() == Knm::Activatable::VpnInterfaceConnection) {
            RemoteInterfaceConnection* remoteconnection = static_cast<RemoteInterfaceConnection*>(activatable);
            if (remoteconnection) {
                if (remoteconnection->activationState() == Knm::InterfaceConnection::Activated
                            || remoteconnection->activationState() == Knm::InterfaceConnection::Activating) {
                    vpns++;
                    if (m_currentConnection != remoteconnection) {
                        m_currentConnection = remoteconnection;
                    }
                }
            }
        }
    }
    if (!vpns) {
        m_currentConnection = 0;
    }
    setConnectionInfo();
}

void VpnInterfaceItem::listAppeared()
{
    foreach (RemoteActivatable* remote, m_activatables->activatables()) {
        activatableAdded(remote);
    }
}

void VpnInterfaceItem::listDisappeared()
{
    m_vpnActivatables.clear();
    currentConnectionChanged();
}

void VpnInterfaceItem::activatableAdded(RemoteActivatable * added)
{
    if (accept(added)) {
        m_vpnActivatables << added;
        RemoteInterfaceConnection* remoteconnection = static_cast<RemoteInterfaceConnection*>(added);
        if (remoteconnection) {
            connect(remoteconnection, SIGNAL(changed()), SLOT(currentConnectionChanged()));
        }
        currentConnectionChanged();
    }
}

void VpnInterfaceItem::activatableRemoved(RemoteActivatable * removed)
{
    if (m_vpnActivatables.contains(removed)) {
        m_vpnActivatables.removeAll(removed);
        currentConnectionChanged();
    }
}

bool VpnInterfaceItem::accept(RemoteActivatable * activatable) const
{
    if (activatable->activatableType() == Knm::Activatable::VpnInterfaceConnection) {
        return true;
    }
    return false;
}

void VpnInterfaceItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if (!m_vpnActivatables.isEmpty())
        emit hoverEnter();

    InterfaceItem::hoverEnterEvent(event);
}

void VpnInterfaceItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if (!m_vpnActivatables.isEmpty())
        emit hoverLeave();

    InterfaceItem::hoverLeaveEvent(event);
}
// vim: sw=4 sts=4 et tw=100
