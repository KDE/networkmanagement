/*
Copyright 2012 Arthur de Souza Ribeiro <arthurdesribeiro@gmail.com>

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

#include "connectionitem.h"

#include "activatableitem.h"
#include <kdebug.h>
#include "remoteactivatable.h"

#include <QPainter>
#include <QGraphicsSceneHoverEvent>
#include <QSize>

#include <KIcon>
#include <KNotification>
#include <KGlobalSettings>

#include <Plasma/Animation>
#include <Plasma/PushButton>

#include <QtNetworkManager/manager.h>
#include <QtNetworkManager/wirelessdevice.h>

#include <activatable.h>
#include <remotewirelessobject.h>
#include <remotewirelessinterfaceconnection.h>
#include <remotewirelessnetwork.h>
#include <wirelesssecurityidentifier.h>

#include "activatableitem.h"
//#include "wirelessinterfaceconnectionitem.h"
#include "wirelessnetworkitem.h"

#include "../libs/service/events.h"

K_GLOBAL_STATIC_WITH_ARGS(KComponentData, s_networkManagementComponentData, ("networkmanagement", "networkmanagement", KComponentData::SkipMainComponentRegistration))
static const int m_iconSize = 48;

ConnectionItem::ConnectionItem(RemoteActivatable *activatable, bool hidden, QObject *parent) :
    QObject(parent),
    m_activatable(activatable),
    m_hidden(hidden),
    m_hoverEnter(false)
{
    m_connected = false;
    connect(m_activatable, SIGNAL(strengthChanged(int)), this, SLOT(handlePropertiesChanges(int)));
    connect(m_activatable, SIGNAL(activationStateChanged(Knm::InterfaceConnection::ActivationState,Knm::InterfaceConnection::ActivationState)),
            SLOT(activationStateChanged(Knm::InterfaceConnection::ActivationState,Knm::InterfaceConnection::ActivationState)));

    RemoteInterfaceConnection * remote = interfaceConnection();
    if (remote && (remote->activationState() == Knm::InterfaceConnection::Activating ||
                   remote->activationState() == Knm::InterfaceConnection::Activated)) {
        m_connected = true;
        if(remote->activationState() == Knm::InterfaceConnection::Activated) {
            m_status = "connected";
        } else {
            m_status = "connecting";
        }

    }

    if(m_activatable) {
        if(m_activatable->activatableType() == Knm::Activatable::WirelessNetwork ||
                m_activatable->activatableType() == Knm::Activatable::WirelessInterfaceConnection) {
            m_type = "wireless";
        } else if (m_activatable->activatableType() == Knm::Activatable::InterfaceConnection) {
            m_type = "wired";
        } else if (m_activatable->activatableType() == Knm::Activatable::VpnInterfaceConnection) {
            m_type = "vpn";
        }
    }

    if(hidden) {
        m_type = "wireless";
    }

    connect(remote, SIGNAL(activationStateChanged(Knm::InterfaceConnection::ActivationState,Knm::InterfaceConnection::ActivationState)),
                SLOT(activationStateChanged(Knm::InterfaceConnection::ActivationState,Knm::InterfaceConnection::ActivationState)));
}

QString ConnectionItem::protectedIcon()
{
    if(m_activatable) {
        bool isShared = false;
        if (m_activatable) {
            isShared = m_activatable->isShared();
        }
        RemoteWirelessObject *wobj = 0;

        if (m_activatable->activatableType() == Knm::Activatable::WirelessNetwork){
            RemoteWirelessNetwork *rwic = qobject_cast<RemoteWirelessNetwork *>(m_activatable);
            if(rwic) {
                wobj = rwic;
            }
        } else if (m_activatable->activatableType() == Knm::Activatable::WirelessInterfaceConnection) {
            RemoteWirelessInterfaceConnection *rwic = qobject_cast<RemoteWirelessInterfaceConnection *>(m_activatable);
            if(rwic) {
                wobj = rwic;
            }
        }

        if(wobj) {
            Knm::WirelessSecurity::Type best = Knm::WirelessSecurity::best(wobj->interfaceCapabilities(), !isShared, (wobj->operationMode() == NetworkManager::WirelessDevice::Adhoc), wobj->apCapabilities(), wobj->wpaFlags(), wobj->rsnFlags());
            return Knm::WirelessSecurity::iconName(best);
        }
    }
    return QString();
}

QString ConnectionItem::ssid()
{
    if(m_activatable) {
        if (m_activatable->activatableType() == Knm::Activatable::WirelessNetwork){
            RemoteWirelessNetwork *rwic = qobject_cast<RemoteWirelessNetwork *>(m_activatable);
            if(rwic) {
                return rwic->ssid();
            }
        } else if (m_activatable->activatableType() == Knm::Activatable::WirelessInterfaceConnection) {
            RemoteWirelessInterfaceConnection *rwic = qobject_cast<RemoteWirelessInterfaceConnection *>(m_activatable);
            if(rwic) {
                return rwic->ssid();
            }
        }
    }

    return "";
}

QString ConnectionItem::wiredName()
{
    RemoteInterfaceConnection *remoteconnection = interfaceConnection();
    if (remoteconnection) {
        return remoteconnection->connectionName();
    }
    return "";
}

QString ConnectionItem::connectionType()
{
    return m_type;
}

bool ConnectionItem::hidden()
{
    return m_hidden;
}

QString ConnectionItem::connectionUuid()
{
    RemoteWirelessInterfaceConnection *rwic2;

    if(m_activatable) {
        if(m_activatable->activatableType() == Knm::Activatable::WirelessNetwork ||
                m_activatable->activatableType() == Knm::Activatable::WirelessInterfaceConnection) {
            rwic2 = qobject_cast<RemoteWirelessInterfaceConnection *>(m_activatable);
            if(rwic2)
                return rwic2->connectionUuid();
        } else {
            RemoteInterfaceConnection *remoteconnection = interfaceConnection();
            if(remoteconnection)
                return remoteconnection->connectionUuid();
        }
    }

    return QString();
}

QString ConnectionItem::connectionIcon()
{
    RemoteInterfaceConnection *remoteconnection = interfaceConnection();
    if (remoteconnection) {
        return remoteconnection->iconName();
    }
    return QString();
}

void ConnectionItem::disconnect() {
    if (m_activatable) {
        RemoteInterfaceConnection * remote = interfaceConnection();
        if (remote && (remote->activationState() == Knm::InterfaceConnection::Activating ||
                       remote->activationState() == Knm::InterfaceConnection::Activated)) {
            remote->deactivate();
        }
    }
}

void ConnectionItem::connectNetwork()
{
    if(m_activatable) {
        RemoteInterfaceConnection * remote = interfaceConnection();
        if (remote && (remote->activationState() == Knm::InterfaceConnection::Activating ||
                       remote->activationState() == Knm::InterfaceConnection::Activated)) {
            emit showInterfaceDetails(remote->deviceUni());
        } else {
            QTimer::singleShot(0, m_activatable, SLOT(activate()));
        }
    }
    QTimer::singleShot(0, this, SLOT(notifyNetworkingState()));
}

void ConnectionItem::hoverEnter()
{
    m_hoverEnter = true;
    emit itemChanged();
}

void ConnectionItem::hoverLeft()
{
    m_hoverEnter = false;
    emit itemChanged();
}

bool ConnectionItem::hover()
{
    return m_hoverEnter;
}

void ConnectionItem::notifyNetworkingState()
{
    if (!NetworkManager::isNetworkingEnabled()) {
        KNotification::event(Event::NetworkingDisabled, i18nc("@info:status Notification when the networking subsystem (NetworkManager, etc) is disabled", "Networking system disabled"), QPixmap(), 0, KNotification::CloseOnTimeout, *s_networkManagementComponentData)->sendEvent();
    } else if (!NetworkManager::isWirelessEnabled() &&
               m_activatable &&
               m_activatable->activatableType() == Knm::Activatable::WirelessInterfaceConnection) {
        KNotification::event(Event::RfOff, i18nc("@info:status Notification for radio kill switch turned off", "Wireless hardware disabled"), KIcon("network-wireless").pixmap(QSize(m_iconSize,m_iconSize)), 0, KNotification::CloseOnTimeout, *s_networkManagementComponentData)->sendEvent();
    }
}

int ConnectionItem::signalStrength()
{
    if(m_activatable) {
        if (m_activatable->activatableType() == Knm::Activatable::WirelessNetwork){
            RemoteWirelessNetwork *rwic = qobject_cast<RemoteWirelessNetwork *>(m_activatable);
            if(rwic) {
                return rwic->strength();
            }
        } else if (m_activatable->activatableType() == Knm::Activatable::WirelessInterfaceConnection) {
            RemoteWirelessInterfaceConnection *rwic = qobject_cast<RemoteWirelessInterfaceConnection *>(m_activatable);
            if(rwic) {
                return rwic->strength();
            }
        }
    }

    return 0;
}

void ConnectionItem::handlePropertiesChanges(int strength)
{
    Q_UNUSED(strength);
    emit itemChanged();
}

bool ConnectionItem::connected()
{
    return m_connected;
}

QString ConnectionItem::status()
{
    return m_status;
}

void ConnectionItem::activationStateChanged(Knm::InterfaceConnection::ActivationState oldState, Knm::InterfaceConnection::ActivationState newState)
{
    Q_UNUSED(oldState);

    switch (newState) {
        case Knm::InterfaceConnection::Activated:
            m_status = "connected";
            m_connected = true;
            break;
        case Knm::InterfaceConnection::Unknown:
            m_status = "not connected";
            m_connected = false;
            break;
        case Knm::InterfaceConnection::Activating:
            m_status = "connecting";
            m_connected = true;
            break;
    }

    emit itemChanged();
}

RemoteInterfaceConnection* ConnectionItem::interfaceConnection() const
{
    if(m_activatable) {
        return qobject_cast<RemoteInterfaceConnection*>(m_activatable);
    }
    return 0;
}

RemoteActivatable * ConnectionItem::activatable() const
{
    return m_activatable;
}

QString ConnectionItem::deviceUni()
{
    if(m_activatable)
        return m_activatable->deviceUni();
    return QString();
}

QString ConnectionItem::activatableType()
{
    return QString();
}

bool ConnectionItem::isShared()
{
    return m_activatable->isShared();
}

bool ConnectionItem::equals(const ConnectionItem *item)
{
    if (!item || !item->activatable()) {
        return false;
    }

    if (!m_activatable) {
        return false;
    }

    if (m_activatable == item->activatable()) {
        return true;
    }

    RemoteInterfaceConnection * a = interfaceConnection();
    RemoteInterfaceConnection * b = item->interfaceConnection();

    if (a && b && a->connectionUuid() == b->connectionUuid()) {
        return true;
    }

    return false;
}
