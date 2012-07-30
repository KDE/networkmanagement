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

#include "declarativeinterfaceitem.h"
#include "uiutils.h"
#include "remoteinterfaceconnection.h"
#include "remoteactivatablelist.h"

#include <arpa/inet.h>

#include <Solid/Device>
#include <QtNetworkManager/wireddevice.h>
#include <QtNetworkManager/ipv4config.h>
#include <QtNetworkManager/manager.h>
#include <NetworkManager/NetworkManager.h>

#include "knmserviceprefs.h"

DeclarativeInterfaceItem::DeclarativeInterfaceItem(NetworkManager::Device * iface, RemoteActivatableList* activatables,  NameDisplayMode mode, QObject *parent) : QObject(parent),
    m_currentConnection(0),
    m_iface(iface),
    m_activatables(activatables),
    m_nameMode(mode),
    m_enabled(false),
    m_hasDefaultRoute(false),
    m_starting(true)
{
    connect(m_activatables, SIGNAL(disappeared()), this, SLOT(serviceDisappeared()));
    connect(m_activatables, SIGNAL(activatableAdded(RemoteActivatable*,int)), SLOT(activatableAdded(RemoteActivatable*)));
    connect(m_activatables, SIGNAL(activatableRemoved(RemoteActivatable*)), SLOT(activatableRemoved(RemoteActivatable*)));

    if (m_iface) {
        connect(m_iface.data(), SIGNAL(stateChanged(NetworkManager::Device::State,NetworkManager::Device::State,NetworkManager::Device::StateChangeReason)),
                this, SLOT(handleConnectionStateChange(NetworkManager::Device::State,NetworkManager::Device::State,NetworkManager::Device::StateChangeReason)));
    }

    setNameDisplayMode(mode);

    if (m_iface) {
        if (m_iface.data()->type() == NetworkManager::Device::Ethernet) {
            NetworkManager::WiredDevice* wirediface =
                            static_cast<NetworkManager::WiredDevice*>(m_iface.data());
            m_type = "wired";
            //connect(wirediface, SIGNAL(carrierChanged(bool)), this, SLOT(setActive(bool)));
        } else if (m_iface.data()->type() == NetworkManager::Device::Wifi) {
            m_type = "wifi";
        } else if (m_iface.data()->type() == NetworkManager::Device::Modem) {
            m_type = "modem";
        }
        m_state = NetworkManager::Device::UnknownState;
        stateChanged(static_cast<NetworkManager::Device::State>(m_iface.data()->state()));

        if (m_iface.data()->type() == NetworkManager::Device::Ethernet) {
            m_type = "wired";
        } else if (m_iface.data()->type() == NetworkManager::Device::Wifi) {
            m_type = "wifi";
        } else if (m_iface.data()->type() == NetworkManager::Device::Modem) {
            m_type = "modem";
        }
    }



    m_starting = false;
}

void DeclarativeInterfaceItem::serviceDisappeared()
{
    m_currentConnection = 0;
}

void DeclarativeInterfaceItem::setEnabled(bool enable)
{
    m_enabled = enable;
}

bool DeclarativeInterfaceItem::enabled()
{
    return m_enabled;
}

void DeclarativeInterfaceItem::activatableAdded(RemoteActivatable * activatable)
{
    if (m_iface && RemoteActivatableList::isConnectionForInterface(activatable, m_iface.data())) {
        updateCurrentConnection(qobject_cast<RemoteInterfaceConnection*>(activatable));

        /* Sometimes the activatableAdded signal arrives after the stateChanged
           signal, so update the interface state here but do not search for current connection
           since it is already known. */
        stateChanged(m_iface.data()->state(), false);
    }
}

void DeclarativeInterfaceItem::setNameDisplayMode(NameDisplayMode mode)
{
    m_nameMode = mode;
    if (m_iface) {
        m_interfaceName = UiUtils::interfaceNameLabel(m_iface.data()->uni());
    }
    if (m_nameMode == InterfaceName) {
        m_interfaceTitle = m_interfaceName;
    } else if (m_nameMode == HardwareName) {
        if (m_iface) {
            m_interfaceTitle = m_iface.data()->interfaceName();
        } else {
            m_interfaceTitle = "Network Interface";
        }

    } else {
        m_interfaceTitle = "Unknown Network Interface";
    }
}

void DeclarativeInterfaceItem::handleConnectionStateChange(NetworkManager::Device::State new_state, NetworkManager::Device::State old_state, NetworkManager::Device::StateChangeReason reason)
{
    Q_UNUSED(old_state);
    Q_UNUSED(reason);
    stateChanged((NetworkManager::Device::State)new_state);
}

void DeclarativeInterfaceItem::activatableRemoved(RemoteActivatable * activatable)
{
    if (activatable == m_currentConnection) {
        m_currentConnection = 0;
    }
}

void DeclarativeInterfaceItem::updateCurrentConnection(RemoteInterfaceConnection * ic)
{
    if (ic) {
        if (m_currentConnection) {
            QObject::disconnect(m_currentConnection, 0, this, 0);
        }
        m_currentConnection = ic;

        connect(m_currentConnection, SIGNAL(hasDefaultRouteChanged(bool)),
                                     SLOT(handleHasDefaultRouteChanged(bool)));
        handleHasDefaultRouteChanged(m_currentConnection->hasDefaultRoute());
        return;
    }
    handleHasDefaultRouteChanged(false);
    m_currentConnection = 0;
    return;
}

void DeclarativeInterfaceItem::handleHasDefaultRouteChanged(bool changed)
{
    m_hasDefaultRoute = changed;
    kDebug() << "Default Route changed!!" << changed;
    emit itemChanged();
}

bool DeclarativeInterfaceItem::defaultRoute()
{
    return m_hasDefaultRoute;
}

void DeclarativeInterfaceItem::setConnectionInfo()
{
    if (m_iface) {
        currentConnectionChanged();
        stateChanged(static_cast<NetworkManager::Device::State>(m_iface.data()->state()));
    }
}

void DeclarativeInterfaceItem::stateChanged(NetworkManager::Device::State state, bool updateConnection)
{
    if (m_state == state) {
        return;
    }
    m_state = state;
    m_disconnect = false;

    // Name and info labels
    QString lname;

    if (updateConnection) {
        currentConnectionChanged();
    }
    if (m_currentConnection) {
        lname = UiUtils::connectionStateToString(state, m_currentConnection->connectionName());
    } else {
        lname = UiUtils::connectionStateToString(state, QString());
        // to allow updating connection's name in the next call of stateChanged()
        // even if the state has not changed.
        m_state = NetworkManager::Device::UnknownState;
    }

    switch (state) {
        case NetworkManager::Device::Unavailable:
            if (m_iface.data()->type() == NetworkManager::Device::Ethernet) {
                lname = "Cable Unplugged";
            }
            setEnabled(false); // FIXME: tone down colors using an animation
            break;
        case NetworkManager::Device::Disconnected:
        case NetworkManager::Device::Deactivating:
            setEnabled(true);
            break;
        case NetworkManager::Device::Preparing:
        case NetworkManager::Device::ConfiguringHardware:
        case NetworkManager::Device::NeedAuth:
        case NetworkManager::Device::ConfiguringIp:
        case NetworkManager::Device::CheckingIp:
        case NetworkManager::Device::WaitingForSecondaries:
        case NetworkManager::Device::Activated:
            setEnabled(true);
            m_disconnect = true;
            break;
        case NetworkManager::Device::Unmanaged:
        case NetworkManager::Device::Failed:
        case NetworkManager::Device::UnknownState:
            setEnabled(false);
            break;
    }

    //m_icon->nativeWidget()->setPixmap(interfacePixmap());

    kDebug() << "State changed" << lname;
    m_connectionName = lname;

    emit stateChanged();
    emit itemChanged();
}

NetworkManager::Device* DeclarativeInterfaceItem::interface()
{
    return m_iface.data();
}

void DeclarativeInterfaceItem::currentConnectionChanged()
{
    updateCurrentConnection(m_activatables->connectionForInterface(m_iface.data()));
}

void DeclarativeInterfaceItem::activeConnectionsChanged()
{
    setConnectionInfo();
}

QWeakPointer<NetworkManager::Device> DeclarativeInterfaceItem::iface()
{
    return m_iface;
}

QString DeclarativeInterfaceItem::connectionName()
{
    // Default active connection's name is empty, room for improvement?
    if (m_currentConnection) {
        return m_currentConnection->connectionName();
    }
    return QString();
}

QString DeclarativeInterfaceItem::type()
{
    return m_type;
}

QString DeclarativeInterfaceItem::interfaceTitle()
{
    return m_interfaceTitle;
}

QString DeclarativeInterfaceItem::connection()
{
    return m_connectionName;
}

QString DeclarativeInterfaceItem::deviceUni()
{
    if(interface()) {
        return interface()->uni();
    }
    return QString();
}

bool DeclarativeInterfaceItem::equals(const DeclarativeInterfaceItem *item)
{
    if (!item || !item->m_iface) {
        return false;
    }

    if (!m_iface) {
        return false;
    }

    if (m_iface == item->m_iface) {
        return true;
    }

    return false;
}
