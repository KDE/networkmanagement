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

#include "vpninterfaceitem.h"
//#include "uiutils.h"
#include "activatableitem.h"

//#include <QGraphicsGridLayout>
#include <QLabel>
#include <QTimer>

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
    m_connectionNameLabel->setText(i18nc("initial label for VPN connection name", "Not Connected"));

    // Catch all kinds of signals to update the VPN widget
    connect(m_activatables, SIGNAL(activatableAdded(RemoteActivatable*)),
            SLOT(currentConnectionChanged()));
    connect(m_activatables, SIGNAL(activatableRemoved(RemoteActivatable*)),
            SLOT(currentConnectionChanged()));
    connect(m_activatables, SIGNAL(appeared()), SLOT(currentConnectionChanged()));
    connect(m_activatables, SIGNAL(disappeared()), SLOT(currentConnectionChanged()));

    connect(this, SIGNAL(stateChanged()), this, SLOT(currentConnectionChanged()));
    connect(this, SIGNAL(clicked()), this, SLOT(currentConnectionChanged()));

    connect(m_disconnectButton, SIGNAL(clicked()), this, SLOT(disconnectCurrentConnection()));

    // Update state
    currentConnectionChanged();
    setConnectionInfo();
}

VpnInterfaceItem::~VpnInterfaceItem()
{
}

RemoteInterfaceConnection* VpnInterfaceItem::currentConnection()
{
    return m_currentConnection;
    // Returns the first VPN connection it finds
    // FIXME: needs refinement to select the most interesting connection (device uni or status)
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
    return 0;
}

void VpnInterfaceItem::disconnectCurrentConnection()
{
    if (m_currentConnection) {
        kDebug() << "deactivating:" << m_currentConnection->connectionName();
        m_currentConnection->deactivate();
    }
    //currentConnectionChanged();
    //setConnectionInfo();
    QTimer::singleShot(3000, this, SLOT(currentConnectionChanged()));
}

QString VpnInterfaceItem::connectionName()
{
    if (m_currentConnection) {
        return m_currentConnection->connectionName();
    } else {
        return QString("VPN");
    }
}

void VpnInterfaceItem::setConnectionInfo()
{
    kDebug();
    bool showDisconnect = false;
    if (m_currentConnection) {
        m_ifaceNameLabel->setText(i18nc("VPN label in interfaces", "<b>%1</b>", m_currentConnection->connectionName()));
        if (m_currentConnection->activationState() == Knm::InterfaceConnection::Activated) {
            m_connectionNameLabel->setText(i18nc("VPN state label", "Connected"));
            kDebug() << "active..." << m_currentConnection->connectionName();
            showDisconnect = true;
        } else if (m_currentConnection->activationState() == Knm::InterfaceConnection::Activating) {
            m_connectionNameLabel->setText(i18nc("VPN state label", "Connecting..."));
            kDebug() << "activating..." << m_currentConnection->connectionName();
            showDisconnect = true;
        } else {
            m_connectionNameLabel->setText("Impossible!");
            kDebug() << "puzzled..." << m_currentConnection->connectionName();
        }

        //m_connectionNameLabel->setText(i18nc("VPN state label", "Not Connected"));
    } else {
        kDebug() << "Current vpn connection == 0";
        m_ifaceNameLabel->setText(i18nc("VPN connections interface", "<b>Virtual Private Network</b>"));
        m_connectionNameLabel->setText(i18nc("VPN state label", "Not Connected..."));
        kDebug() << "no connection...";
    }
    if (!showDisconnect) {
        //m_disconnectButton->setIcon("dialog-ok");
        //m_disconnectButton->setToolTip(i18n("Connect"));
        m_disconnectButton->hide();
    } else {
        m_disconnectButton->setIcon(KIcon("dialog-close"));
        m_disconnectButton->setToolTip(i18nc("tooltip on disconnect icon", "Disconnect"));
        m_disconnectButton->show();
    }

    kDebug() << "Set to:" << m_connectionNameLabel->text();
}

void VpnInterfaceItem::currentConnectionChanged()
{
    kDebug() << "VPN: currentConnectionChanged()!";
    int vpns = 0;
    foreach (RemoteActivatable* activatable, m_activatables->activatables()) {
        if (activatable->activatableType() == Knm::Activatable::VpnInterfaceConnection) {
            RemoteInterfaceConnection* remoteconnection = static_cast<RemoteInterfaceConnection*>(activatable);
            if (remoteconnection) {
                if (remoteconnection->activationState() == Knm::InterfaceConnection::Activated
                            || remoteconnection->activationState() == Knm::InterfaceConnection::Activating) {
                    kDebug() << "active or activating VPN connection" << remoteconnection->connectionName();

                    vpns++;
                    if (m_currentConnection != remoteconnection) {
                        if (m_currentConnection) {
                            // disconnect
                            disconnect(m_currentConnection, SIGNAL(activationStateChanged(Knm::InterfaceConnection::ActivationState)));
                            disconnect(m_currentConnection, SIGNAL(hasDefaultRouteChanged(bool)));
                        }

                        m_currentConnection = remoteconnection;

                        //    void activationStateChanged(Knm::InterfaceConnection::ActivationState);
                        //    void hasDefaultRouteChanged(bool);
                        connect(m_currentConnection, SIGNAL(activationStateChanged(Knm::InterfaceConnection::ActivationState)),
                                this, SLOT(setConnectionInfo()));
                        connect(m_currentConnection, SIGNAL(hasDefaultRouteChanged(bool)),
                                this, SLOT(setConnectionInfo()));
                        kDebug() << "VPN connection changed" << remoteconnection->connectionName();
                        //setConnectionInfo();
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

void VpnInterfaceItem::connectionStateChanged(Solid::Control::NetworkInterface::ConnectionState)
{
    kDebug() << "fake...";
}



// vim: sw=4 sts=4 et tw=100
