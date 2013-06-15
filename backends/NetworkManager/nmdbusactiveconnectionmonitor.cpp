/*
Copyright 2009 Will Stephenson <wstephenson@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "nmdbusactiveconnectionmonitor.h"

#include <NetworkManager.h>
#include <NetworkManagerVPN.h>

#include <QMultiHash>
#include <QUuid>

#include <KDebug>
#include <solid/control/networkmanager.h>

#include <interfaceconnection.h>
#include <vpninterfaceconnection.h>

#include "activatablelist.h"

#include "nm-active-connectioninterface.h"
#include "nm-vpn-connectioninterface.h"

NMDBusActiveConnectionProxy::NMDBusActiveConnectionProxy(Knm::InterfaceConnection * interfaceConnection,
        OrgFreedesktopNetworkManagerConnectionActiveInterface * activeConnectionIface)
: m_activeConnectionIface(activeConnectionIface), m_interfaceConnection(interfaceConnection)
{
    m_activeConnectionIface->setParent(this);

    connect(m_activeConnectionIface, SIGNAL(PropertiesChanged(QVariantMap)), SLOT(handlePropertiesChanged(QVariantMap)));

    m_interfaceConnection->setProperty("NMDBusActiveConnectionObject", m_activeConnectionIface->path());
    kDebug() << "default:" << m_activeConnectionIface->getDefault() << "state:" << m_activeConnectionIface->state();
    m_interfaceConnection->setHasDefaultRoute(m_activeConnectionIface->getDefault());
    setState(m_activeConnectionIface->state());
}

NMDBusActiveConnectionProxy::~NMDBusActiveConnectionProxy()
{
    m_interfaceConnection->setActivationState(Knm::InterfaceConnection::Unknown);
    m_interfaceConnection->setHasDefaultRoute(false);
}


Knm::InterfaceConnection * NMDBusActiveConnectionProxy::interfaceConnection() const
{
    return m_interfaceConnection;
}

void NMDBusActiveConnectionProxy::handlePropertiesChanged(const QVariantMap & changedProps)
{
    const QString defaultKey = QLatin1String("Default");
    const QString stateKey = QLatin1String("State");
    if (changedProps.contains(defaultKey)) {
        m_interfaceConnection->setHasDefaultRoute(changedProps[defaultKey].toBool());
    }
    if (changedProps.contains(stateKey)) {
        setState(changedProps[stateKey].toUInt());
    }
}

void NMDBusActiveConnectionProxy::setState(uint nmState)
{
    Knm::InterfaceConnection::ActivationState aState = Knm::InterfaceConnection::Unknown;
    switch (nmState) {
        case NM_ACTIVE_CONNECTION_STATE_UNKNOWN:
            break;
        case NM_ACTIVE_CONNECTION_STATE_ACTIVATING:
            aState = Knm::InterfaceConnection::Activating;
            break;
        case NM_ACTIVE_CONNECTION_STATE_ACTIVATED:
            aState = Knm::InterfaceConnection::Activated;
            break;
        case NM_ACTIVE_CONNECTION_STATE_DEACTIVATING:
            aState = Knm::InterfaceConnection::Deactivating;
            break;
        case NM_ACTIVE_CONNECTION_STATE_DEACTIVATED:
            aState = Knm::InterfaceConnection::Deactivated;
            break;
        default:
            kDebug() << "Unhandled activation state" << nmState;
    }
    kDebug() << "state:" << aState;
    m_interfaceConnection->setActivationState(aState);
}

NMDBusVPNConnectionProxy::NMDBusVPNConnectionProxy(Knm::InterfaceConnection * interfaceConnection,
        OrgFreedesktopNetworkManagerConnectionActiveInterface * activeConnectionIface)
: NMDBusActiveConnectionProxy(interfaceConnection, activeConnectionIface)
{
    m_vpnConnectionIface = new OrgFreedesktopNetworkManagerVPNConnectionInterface(
            activeConnectionIface->service(),
            activeConnectionIface->path(),
            QDBusConnection::systemBus(),
            this);
    connect(m_vpnConnectionIface, SIGNAL(PropertiesChanged(QVariantMap)), this, SLOT(handleVPNPropertiesChanged(QVariantMap)));
    setState(m_vpnConnectionIface->vpnState());
}

void NMDBusVPNConnectionProxy::handleVPNPropertiesChanged(const QVariantMap & changedProps)
{
    const QString stateKey = QLatin1String("VpnState");
    if (changedProps.contains(stateKey)) {
        setState(changedProps[stateKey].toUInt());
    }
}

void NMDBusVPNConnectionProxy::setState(uint vpnState)
{
    Knm::InterfaceConnection::ActivationState aState = Knm::InterfaceConnection::Unknown;
    switch (vpnState) {
        case NM_VPN_CONNECTION_STATE_UNKNOWN:
        case NM_VPN_CONNECTION_STATE_FAILED:
        case NM_VPN_CONNECTION_STATE_DISCONNECTED:
            break;
        case NM_VPN_CONNECTION_STATE_PREPARE:
        case NM_VPN_CONNECTION_STATE_NEED_AUTH:
        case NM_VPN_CONNECTION_STATE_CONNECT:
        case NM_VPN_CONNECTION_STATE_IP_CONFIG_GET:
            aState = Knm::InterfaceConnection::Activating;
            break;
        case NM_VPN_CONNECTION_STATE_ACTIVATED:
            aState = Knm::InterfaceConnection::Activated;
    }
    kDebug() << "state:" << aState;
    m_interfaceConnection->setActivationState(aState);
}

class NMDBusActiveConnectionMonitorPrivate
{
public:
    ActivatableList * activatableList;
    QHash<QString, NMDBusActiveConnectionProxy *> activeConnections;
};

NMDBusActiveConnectionMonitor::NMDBusActiveConnectionMonitor(ActivatableList * activatables, QObject * parent)
: QObject(parent), d_ptr(new NMDBusActiveConnectionMonitorPrivate)
{
    Q_D(NMDBusActiveConnectionMonitor);
    d->activatableList = activatables;

    connect(Solid::Control::NetworkManagerNm09::notifier(),
            SIGNAL(activeConnectionsChanged()),
            this, SLOT(activeConnectionListChanged()));

    connect(Solid::Control::NetworkManagerNm09::notifier(),
            SIGNAL(statusChanged(Solid::Networking::Status)),
            this, SLOT(networkingStatusChanged(Solid::Networking::Status)));

    activeConnectionListChanged();
}

NMDBusActiveConnectionMonitor::~NMDBusActiveConnectionMonitor()
{
    delete d_ptr;
}

void NMDBusActiveConnectionMonitor::activeConnectionListChanged()
{
    // update all InterfaceConnections we know about
    Q_D(NMDBusActiveConnectionMonitor);

    QStringList currentActiveConnections = Solid::Control::NetworkManagerNm09::activeConnections();

    // delete any stale interfaces
    foreach (const QString &key, d->activeConnections.keys()) {
        if (!currentActiveConnections.contains(key)) {
            NMDBusActiveConnectionProxy * stale = d->activeConnections.take(key);
            kDebug() << "removing stale active connection" << key;
            delete stale;
        }
    }

    // create an interface to any active connections we're not already tracking
    // and update their interfaceconnections
    foreach (const QString &activeConnectionPath, currentActiveConnections) {
        if (!d->activeConnections.contains(activeConnectionPath)) {
            kDebug() << "Adding active connection interface for " << activeConnectionPath;

            OrgFreedesktopNetworkManagerConnectionActiveInterface * active = new OrgFreedesktopNetworkManagerConnectionActiveInterface(NM_DBUS_INTERFACE, activeConnectionPath, QDBusConnection::systemBus(), 0);

            Knm::InterfaceConnection * ic = interfaceConnectionForConnectionActive(active);
            if (ic) {
                NMDBusActiveConnectionProxy* proxy;
                if (active->vpn()) {
                    proxy = new NMDBusVPNConnectionProxy(ic, active);
                } else {
                    proxy = new NMDBusActiveConnectionProxy(ic, active);
                }
                d->activeConnections.insert(activeConnectionPath, proxy);
            }
            // put the object path into a list of active connections
            kDebug() << "Connection active at" << active->connection().path() << (active->vpn() ? "is" : "is not") << "a VPN connection";
        }
    }
}

Knm::InterfaceConnection * NMDBusActiveConnectionMonitor::interfaceConnectionForConnectionActive(OrgFreedesktopNetworkManagerConnectionActiveInterface * connectionActive)
{
    Q_D(NMDBusActiveConnectionMonitor);
    Knm::InterfaceConnection * ic = 0;
    QList<Knm::Activatable *> activatables = d->activatableList->activatables();
    // check whether each interfaceconnection is for the changed active connection
    foreach (Knm::Activatable * activatable, activatables) {
        Knm::InterfaceConnection * candidate = qobject_cast<Knm::InterfaceConnection*>(activatable);
        // ignore HiddenWICs, we don't set status on these
        if (candidate && candidate->activatableType() != Knm::Activatable::HiddenWirelessInterfaceConnection) {
              if (candidate->property("NMDBusObjectPath") == connectionActive->connection().path()
                    && (candidate->activatableType() == Knm::Activatable::VpnInterfaceConnection
                        || connectionActive->devices().contains(QDBusObjectPath(candidate->deviceUni())))
                    ) {
                ic = candidate;
                break;
            }
        }
    }

    return ic;
}

void NMDBusActiveConnectionMonitor::handleAdd(Knm::Activatable *)
{
}

void NMDBusActiveConnectionMonitor::handleUpdate(Knm::Activatable *)
{
}

void NMDBusActiveConnectionMonitor::handleRemove(Knm::Activatable * removed)
{
    Q_D(NMDBusActiveConnectionMonitor);
    // we remove proxies if their interfaceconnection is removed so that we don't need a fixed order
    // ActiveConnection removal and InterfaceConnection removal events
    QMutableHashIterator<QString, NMDBusActiveConnectionProxy*> i(d->activeConnections);
    while (i.hasNext()) {
        i.next();
        NMDBusActiveConnectionProxy * candidate = i.value();
        if (candidate->interfaceConnection() == removed) {
            i.remove();
            kDebug() << "removing active connection because its connection was removed";
            delete candidate;
        }
    }
}

void NMDBusActiveConnectionMonitor::networkingStatusChanged(Solid::Networking::Status status)
{
    if (status == Solid::Networking::Unknown) {
        Q_D(NMDBusActiveConnectionMonitor);
        // the manager probably exited, clean our state
        qDeleteAll(d->activeConnections);
        d->activeConnections.clear();
    }
}

// vim: sw=4 sts=4 et tw=100
