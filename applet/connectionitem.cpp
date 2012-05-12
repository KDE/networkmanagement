#include "connectionitem.h"

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
    }
}

QString ConnectionItem::ssid()
{
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

    return QString();
}

int ConnectionItem::signalStrength()
{
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

void ConnectionItem::activationStateChanged(Knm::InterfaceConnection::ActivationState oldState, Knm::InterfaceConnection::ActivationState newState)
{
    Q_UNUSED(oldState);

    switch (newState) {
        case Knm::InterfaceConnection::Activated:
            m_connected = true;
            break;
        case Knm::InterfaceConnection::Unknown:
            m_connected = false;
            break;
        case Knm::InterfaceConnection::Activating:
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
    return null;
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
    if(item->interfaceConnection() == interfaceConnection()) return true;
    return false;
}
