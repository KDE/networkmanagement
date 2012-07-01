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

ConnectionItem::ConnectionItem(RemoteActivatable *activatable, QObject *parent) :
    QObject(parent),
    m_activatable(activatable)
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
    if(remote) {
        connect(remote, SIGNAL(activationStateChanged(Knm::InterfaceConnection::ActivationState,Knm::InterfaceConnection::ActivationState)),
                SLOT(activationStateChanged(Knm::InterfaceConnection::ActivationState,Knm::InterfaceConnection::ActivationState)));
    }
}

QString ConnectionItem::protectedIcon()
{
    if(m_activatable) {
        bool isShared = false;
        if (m_activatable) {
            isShared = m_activatable->isShared();
        }
        RemoteWirelessNetwork *rwic;
        RemoteWirelessInterfaceConnection *rwic2;
        RemoteWirelessObject *wobj = 0;
        rwic = qobject_cast<RemoteWirelessNetwork *>(m_activatable);
        if(rwic) {
            wobj = rwic;
        }
        rwic2 = qobject_cast<RemoteWirelessInterfaceConnection *>(m_activatable);
        if(rwic2) {
            wobj = rwic2;
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
        RemoteWirelessNetwork *rwic;
        RemoteWirelessInterfaceConnection *rwic2;
        rwic = qobject_cast<RemoteWirelessNetwork *>(m_activatable);
        if(rwic) {
            return rwic->ssid();
        }
        rwic2 = qobject_cast<RemoteWirelessInterfaceConnection *>(m_activatable);
        if(rwic2) {
            return rwic2->ssid();
        }
    }

    return "";
}

QString ConnectionItem::connectionUuid()
{
    RemoteWirelessInterfaceConnection *rwic2;

    if(m_activatable) {
        rwic2 = qobject_cast<RemoteWirelessInterfaceConnection *>(m_activatable);
        if(rwic2) {
            return rwic2->connectionUuid();
        }
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

void ConnectionItem::connectNetwork() {
    if(m_activatable) {
        RemoteInterfaceConnection * remote = interfaceConnection();
        if (remote && (remote->activationState() == Knm::InterfaceConnection::Activating ||
                       remote->activationState() == Knm::InterfaceConnection::Activated)) {
            // Show interface details
        } else {
            QTimer::singleShot(0, m_activatable, SLOT(activate()));
        }
    }
    QTimer::singleShot(0, this, SLOT(notifyNetworkingState()));
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
        RemoteWirelessNetwork *rwic;
        RemoteWirelessInterfaceConnection *rwic2;
        rwic = qobject_cast<RemoteWirelessNetwork *>(m_activatable);
        if(rwic) {
            return rwic->strength();
        }
        rwic2 = qobject_cast<RemoteWirelessInterfaceConnection *>(m_activatable);
        if(rwic2) {
            return rwic2->strength();
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
    return m_activatable->deviceUni();
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
