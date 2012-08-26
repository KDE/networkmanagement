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
#include "uiutils.h"

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
#include <remotegsminterfaceconnection.h>
#include <wirelesssecurityidentifier.h>

#include "wirelessnetworkitem.h"

#include "../libs/service/events.h"

K_GLOBAL_STATIC_WITH_ARGS(KComponentData, s_networkManagementComponentData, ("networkmanagement", "networkmanagement", KComponentData::SkipMainComponentRegistration))
static const int m_iconSize = 48;

ConnectionItem::ConnectionItem(RemoteActivatable *activatable, bool hidden, QObject *parent) :
    QObject(parent),
    m_activatable(activatable),
    m_hoverEnter(false),
    m_hasDefaultRoute(false),
    m_activationState(QLatin1String("unknown")),
    m_hidden(hidden)
{
    if (m_activatable) {
        RemoteInterfaceConnection * remote = interfaceConnection();

        if (remote) {
            m_hasDefaultRoute = remote->hasDefaultRoute();
            connect(remote, SIGNAL(hasDefaultRouteChanged(bool)),
                    SLOT(handleHasDefaultRouteChanged(bool)));
            connect(remote, SIGNAL(activationStateChanged(Knm::InterfaceConnection::ActivationState,Knm::InterfaceConnection::ActivationState)),
                    SLOT(activationStateChanged(Knm::InterfaceConnection::ActivationState,Knm::InterfaceConnection::ActivationState)));

            if (remote->activationState() == Knm::InterfaceConnection::Activated) {
                m_activationState = "activated";
            } else if (remote->activationState() == Knm::InterfaceConnection::Activating) {
                m_activationState = "activating";
            }
        }

        switch (m_activatable->activatableType()) {
        case Knm::Activatable::WirelessNetwork:
            connect(qobject_cast<RemoteWirelessNetwork *>(m_activatable), SIGNAL(strengthChanged(int)), this, SLOT(handlePropertiesChanges()));
            m_type = "wirelessNetwork";
            break;
        case Knm::Activatable::WirelessInterfaceConnection:
            connect(qobject_cast<RemoteWirelessInterfaceConnection *>(m_activatable), SIGNAL(strengthChanged(int)), this, SLOT(handlePropertiesChanges()));
            m_type = "wireless";
            break;
        case Knm::Activatable::InterfaceConnection:
            m_type = "wired";
            break;
        case Knm::Activatable::VpnInterfaceConnection:
            m_type = "vpn";
            break;
        case Knm::Activatable::GsmInterfaceConnection:
            connect(qobject_cast<RemoteGsmInterfaceConnection *>(m_activatable), SIGNAL(signalQualityChanged(int)), this, SLOT(handlePropertiesChanges()));
            connect(qobject_cast<RemoteGsmInterfaceConnection *>(m_activatable), SIGNAL(accessTechnologyChanged(int)), this, SLOT(handlePropertiesChanges()));
            m_type = "gsm";
            break;
        /* TODO: add HiddenWirelessInterfaceConnection and UnconfiguredInterface, or just get rid of them. */
        }
    }

    if (hidden) {
        m_type = "wirelessNetwork";
    }
}

QString ConnectionItem::protectedIcon()
{
    if (m_activatable) {
        bool isShared = false;
        if (m_activatable) {
            isShared = m_activatable->isShared();
        }
        RemoteWirelessObject *wobj = 0;

        if (m_activatable->activatableType() == Knm::Activatable::WirelessNetwork) {
            RemoteWirelessNetwork *rwic = qobject_cast<RemoteWirelessNetwork *>(m_activatable);
            if (rwic) {
                wobj = rwic;
            }
        } else if (m_activatable->activatableType() == Knm::Activatable::WirelessInterfaceConnection) {
            RemoteWirelessInterfaceConnection *rwic = qobject_cast<RemoteWirelessInterfaceConnection *>(m_activatable);
            if (rwic) {
                wobj = rwic;
            }
        }

        if (wobj) {
            Knm::WirelessSecurity::Type best = Knm::WirelessSecurity::best(wobj->interfaceCapabilities(), !isShared, (wobj->operationMode() == NetworkManager::WirelessDevice::Adhoc), wobj->apCapabilities(), wobj->wpaFlags(), wobj->rsnFlags());
            return Knm::WirelessSecurity::iconName(best);
        }
    }
    return QString();
}

QString ConnectionItem::ssid()
{
    if (m_activatable) {
        if (m_activatable->activatableType() == Knm::Activatable::WirelessNetwork) {
            RemoteWirelessNetwork *rwic = qobject_cast<RemoteWirelessNetwork *>(m_activatable);
            if (rwic) {
                return rwic->ssid();
            }
        } else if (m_activatable->activatableType() == Knm::Activatable::WirelessInterfaceConnection) {
            RemoteWirelessInterfaceConnection *rwic = qobject_cast<RemoteWirelessInterfaceConnection *>(m_activatable);
            if (rwic) {
                return rwic->ssid();
            }
        }
    }

    return "";
}

QString ConnectionItem::connectionName()
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

void ConnectionItem::handleHasDefaultRouteChanged(bool has)
{
    m_hasDefaultRoute = has;
    emit itemChanged();
}

QString ConnectionItem::connectionUuid()
{
    RemoteWirelessInterfaceConnection *rwic2;

    if (m_activatable) {
        if (m_activatable->activatableType() == Knm::Activatable::WirelessNetwork ||
                m_activatable->activatableType() == Knm::Activatable::WirelessInterfaceConnection) {
            rwic2 = qobject_cast<RemoteWirelessInterfaceConnection *>(m_activatable);
            if (rwic2)
                return rwic2->connectionUuid();
        } else {
            RemoteInterfaceConnection *remoteconnection = interfaceConnection();
            if (remoteconnection)
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

void ConnectionItem::disconnect()
{
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
    if (m_activatable) {
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
        KNotification::event(Event::RfOff, i18nc("@info:status Notification for radio kill switch turned off", "Wireless hardware disabled"), KIcon("network-wireless").pixmap(QSize(m_iconSize, m_iconSize)), 0, KNotification::CloseOnTimeout, *s_networkManagementComponentData)->sendEvent();
    }
}

int ConnectionItem::signalStrength()
{
    if (m_activatable) {
        if (m_activatable->activatableType() == Knm::Activatable::WirelessNetwork) {
            RemoteWirelessNetwork *rwic = qobject_cast<RemoteWirelessNetwork *>(m_activatable);
            if (rwic) {
                return rwic->strength();
            }
        } else if (m_activatable->activatableType() == Knm::Activatable::WirelessInterfaceConnection) {
            RemoteWirelessInterfaceConnection *rwic = qobject_cast<RemoteWirelessInterfaceConnection *>(m_activatable);
            if (rwic) {
                return rwic->strength();
            }
        }
    }

    return 0;
}

void ConnectionItem::handlePropertiesChanges()
{
    emit itemChanged();
}

bool ConnectionItem::defaultRoute()
{
    return m_hasDefaultRoute;
}

int ConnectionItem::signalQuality()
{
    if (m_activatable && m_activatable->activatableType() == Knm::Activatable::GsmInterfaceConnection) {
        RemoteGsmInterfaceConnection * giface = qobject_cast<RemoteGsmInterfaceConnection *>(m_activatable);

        if (giface) {
            return giface->getSignalQuality();
        }
    }
    return -1;
}

QString ConnectionItem::accessTechnology()
{
    if (m_activatable && m_activatable->activatableType() == Knm::Activatable::GsmInterfaceConnection) {
        RemoteGsmInterfaceConnection * giface = qobject_cast<RemoteGsmInterfaceConnection *>(m_activatable);

        if (giface) {
            ModemManager::ModemInterface::AccessTechnology tech = static_cast<ModemManager::ModemInterface::AccessTechnology>(giface->getAccessTechnology());
            
            if (tech != ModemManager::ModemInterface::UnknownTechnology) {
                return UiUtils::convertAccessTechnologyToString(tech);
            }
        }
    }
    return QString();
}

QString ConnectionItem::activationState()
{
    return m_activationState;
}

void ConnectionItem::activationStateChanged(Knm::InterfaceConnection::ActivationState oldState, Knm::InterfaceConnection::ActivationState newState)
{
    Q_UNUSED(oldState);

    switch (newState) {
    case Knm::InterfaceConnection::Activated:
        m_activationState = "activated";
        break;
    case Knm::InterfaceConnection::Unknown:
        m_activationState = "unknown";
        break;
    case Knm::InterfaceConnection::Activating:
        m_activationState = "activating";
        break;
    }

    emit itemChanged();
}

RemoteInterfaceConnection* ConnectionItem::interfaceConnection() const
{
    return qobject_cast<RemoteInterfaceConnection*>(m_activatable);
}

RemoteActivatable * ConnectionItem::activatable() const
{
    return m_activatable;
}

QString ConnectionItem::deviceUni()
{
    if (m_activatable)
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
