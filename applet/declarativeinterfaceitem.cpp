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
    m_iface(iface),
    m_currentConnection(0),
    m_activatables(activatables),
    m_nameMode(mode),
    m_enabled(false),
    m_visible(false),
    m_hasDefaultRoute(false),
    m_starting(true)
{

    if (m_iface) {
        connect(m_iface.data(), SIGNAL(stateChanged(NetworkManager::Device::State,NetworkManager::Device::State,NetworkManager::Device::StateChangeReason)),
                this, SLOT(handleConnectionStateChange(NetworkManager::Device::State,NetworkManager::Device::State,NetworkManager::Device::StateChangeReason)));
    }

    setNameDisplayMode(mode);

    connect(m_activatables, SIGNAL(disappeared()), this, SLOT(serviceDisappeared()));
    connect(m_activatables, SIGNAL(appeared()), this, SLOT(serviceAppeared()));
    connect(m_activatables, SIGNAL(activatableAdded(RemoteActivatable*,int)), SLOT(activatableAdded(RemoteActivatable*)));
    connect(m_activatables, SIGNAL(activatableRemoved(RemoteActivatable*)), SLOT(activatableRemoved(RemoteActivatable*)));

    if (m_iface) {
        m_state = NetworkManager::Device::UnknownState;
        stateChanged(static_cast<NetworkManager::Device::State>(m_iface.data()->state()));

        if (m_iface.data()->type() == NetworkManager::Device::Ethernet) {
            m_type = "wired";
        } else if (m_iface.data()->type() == NetworkManager::Device::Wifi) {
            m_type = "wifi";
            setConnectionInfo();
        } else if (m_iface.data()->type() == NetworkManager::Device::Modem) {
            m_type = "modem";
        } else if (m_iface.data()->type() == NetworkManager::Device::Bluetooth) {
            m_type = "bluetooth";
        }
        m_starting = false;
        setInterfaceIcon();
    } else {
        m_type = "vpn";
        serviceAppeared();
        setConnectionInfo();
    }
}

void DeclarativeInterfaceItem::serviceDisappeared()
{
    if (m_type == "vpn") {
        m_vpnActivatables.clear();
        currentConnectionChanged();
    } else {
        m_currentConnection = 0;
    }
}

void DeclarativeInterfaceItem::serviceAppeared()
{
    if (m_type == "vpn") {
        foreach(RemoteActivatable * remote, m_activatables->activatables()) {
            activatableAdded(remote);
        }
    }
}

void DeclarativeInterfaceItem::setEnabled(bool enable)
{
    m_enabled = enable;
}

bool DeclarativeInterfaceItem::enabled()
{
    return m_enabled;
}

bool DeclarativeInterfaceItem::isVisible()
{
    return m_visible;
}

void DeclarativeInterfaceItem::activatableAdded(RemoteActivatable * activatable)
{
    if (m_type == "vpn") {
        if (accept(activatable)) {
            m_vpnActivatables << activatable;
            RemoteInterfaceConnection* remoteconnection = static_cast<RemoteInterfaceConnection*>(activatable);
            if (remoteconnection) {
                connect(remoteconnection, SIGNAL(changed()), SLOT(currentConnectionChanged()));
            }
            currentConnectionChanged();
        }
    } else {
        if (m_iface && RemoteActivatableList::isConnectionForInterface(activatable, m_iface.data())) {
            updateCurrentConnection(qobject_cast<RemoteInterfaceConnection*>(activatable));

            /* Sometimes the activatableAdded signal arrives after the stateChanged
            signal, so update the interface state here but do not search for current connection
            since it is already known. */
            stateChanged(m_iface.data()->state(), false);
        }
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
    if (m_type != "vpn") {
        if (activatable == m_currentConnection) {
            m_currentConnection = 0;
        }
    } else {
        if (m_vpnActivatables.contains(activatable)) {
            m_vpnActivatables.removeAll(activatable);
            currentConnectionChanged();
        }
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
    setInterfaceIcon();
    kDebug() << "Default Route changed!!" << changed;
    emit itemChanged();
}

bool DeclarativeInterfaceItem::defaultRoute()
{
    return m_hasDefaultRoute;
}

void DeclarativeInterfaceItem::setConnectionInfo()
{
    if (m_type == "vpn") {
        bool showDisconnect = false;
        if (m_currentConnection) {
            m_interfaceTitle = m_currentConnection->connectionName();
            if (m_currentConnection->activationState() == Knm::InterfaceConnection::Activated) {
                m_connectionName = "Connected";
                showDisconnect = true;
            } else if (m_currentConnection->activationState() == Knm::InterfaceConnection::Activating) {
                m_connectionName = "Connecting...";
                showDisconnect = true;
            } else {
                m_connectionName = "Impossible!";
            }
        } else {
            m_interfaceTitle = "Virtual Private Network";
            m_connectionName = "Not Connected...";
        }
        if (!m_vpnActivatables.isEmpty()) {
            //kDebug() << m_vpnActivatables.count() << "VPN connections have become available!";
            m_visible = true;
        } else {
            //kDebug() << "hidding VPN widget:" << m_vpnActivatables.count();
            m_visible = false;
        }
        m_enabled = showDisconnect;
        emit itemChanged();
    } else {
        if (m_iface) {
            currentConnectionChanged();
            stateChanged(static_cast<NetworkManager::Device::State>(m_iface.data()->state()));
        }
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

    m_connectionName = lname;
    setInterfaceIcon();

    kDebug() << "m_icon is: " << m_icon;

    emit stateChanged();
    emit itemChanged();
}

bool DeclarativeInterfaceItem::accept(RemoteActivatable * activatable) const
{
    if (activatable->activatableType() == Knm::Activatable::VpnInterfaceConnection) {
        return true;
    }
    return false;
}

NetworkManager::Device* DeclarativeInterfaceItem::interface()
{
    return m_iface.data();
}

void DeclarativeInterfaceItem::currentConnectionChanged()
{
    if (m_type == "vpn") {
        int vpns = 0;
        foreach(RemoteActivatable * activatable, m_activatables->activatables()) {
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
    } else {
        updateCurrentConnection(m_activatables->connectionForInterface(m_iface.data()));
    }
}

void DeclarativeInterfaceItem::activeConnectionsChanged()
{
    if (m_type != "vpn")
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

void DeclarativeInterfaceItem::setInterfaceIcon()
{
    m_icon = UiUtils::iconName(m_iface.data());
    emit itemChanged();
}

QString DeclarativeInterfaceItem::icon()
{
    return m_icon;
}

QString DeclarativeInterfaceItem::type() const
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
    if (interface()) {
        return interface()->uni();
    }
    return QString();
}

bool DeclarativeInterfaceItem::equals(const DeclarativeInterfaceItem *item)
{
    if (item) {
        if (item->type() == "vpn" && m_type == item->type())
            return true;
    }
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
