#include "connectionitem.h"

#include <kdebug.h>

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
    if(remote) {
        connect(remote, SIGNAL(activationStateChanged(Knm::InterfaceConnection::ActivationState,Knm::InterfaceConnection::ActivationState)),
                SLOT(activationStateChanged(Knm::InterfaceConnection::ActivationState,Knm::InterfaceConnection::ActivationState)));
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

    kDebug() << "mudou o estado da conexao";

    switch (newState) {
        case Knm::InterfaceConnection::Activated:
            m_connected = true;
            kDebug() << "mudou para ativado";
            break;
        case Knm::InterfaceConnection::Unknown:
            m_connected = false;
            kDebug() << "mudou para desconectado";
            break;
        case Knm::InterfaceConnection::Activating:
            m_connected = true;
            kDebug() << "mudou para ativando";
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
    if((item) && item->interfaceConnection() == interfaceConnection()) return true;
    return false;
}
