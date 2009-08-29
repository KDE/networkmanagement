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

#include <QMultiHash>
#include <QUuid>

#include <KDebug>
#include <solid/control/networkmanager.h>

#include <interfaceconnection.h>
#include <vpninterfaceconnection.h>

#include "activatablelist.h"

#include "nmdbussettingsservice.h"

#include "nm-active-connectioninterface.h"

class NMDBusActiveConnectionMonitorPrivate
{
public:
    ActivatableList * activatableList;
    QHash<QString, OrgFreedesktopNetworkManagerConnectionActiveInterface *> activeConnections;
};

NMDBusActiveConnectionMonitor::NMDBusActiveConnectionMonitor(ActivatableList * activatables, QObject * parent)
: QObject(parent), d_ptr(new NMDBusActiveConnectionMonitorPrivate)
{
    Q_D(NMDBusActiveConnectionMonitor);
    d->activatableList = activatables;

    connect(Solid::Control::NetworkManager::notifier(),
            SIGNAL(activeConnectionsChanged()),
            this, SLOT(activeConnectionListChanged()));

    connect(Solid::Control::NetworkManager::notifier(),
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

    QStringList currentActiveConnections = Solid::Control::NetworkManager::activeConnections();

    // delete any stale interfaces
    foreach (OrgFreedesktopNetworkManagerConnectionActiveInterface * active, d->activeConnections) {
        if (!currentActiveConnections.contains(active->path())) {
            //kDebug() << "removing stale active connection" << active->path();
            d->activeConnections.remove(active->path());
            delete active;
        }
    }

    // create an interface to any active connections we're not already tracking
    // and update their interfaceconnections

    foreach (QString activeConnectionPath, currentActiveConnections) {
        if (!d->activeConnections.contains(activeConnectionPath)) {
            //kDebug() << "Adding active connection interface for " << activeConnectionPath;
            OrgFreedesktopNetworkManagerConnectionActiveInterface * active = new OrgFreedesktopNetworkManagerConnectionActiveInterface("org.freedesktop.NetworkManager", activeConnectionPath, QDBusConnection::systemBus(), this);
            d->activeConnections.insert(activeConnectionPath, active);
            connect(active, SIGNAL(PropertiesChanged(const QVariantMap&)), this, SLOT(activeConnectionChanged(const QVariantMap&)));

            // put the service and the object path into a list of active connections
            //kDebug() << "Connection active at" << active->serviceName() << active->connection().path();

            // update active interfaceConnections' state
            activeConnectionChangedInternal(active, active->state());
        }
    }

    // Finally, reset state on all interfaceconnections that do not have an active Connection.Active in the list
    // This is necessary because when NM sets Unknown state on its Connection.Active objects, they
    // return invalid service and connection values, so we can't look up the interfaceconnection to
    // change back

    QList<Knm::Activatable *> activatables = d->activatableList->activatables();
    // check whether each activatable is for one of the active connections
    foreach (Knm::Activatable * activatable, activatables) {
        Knm::InterfaceConnection * interfaceConnection = qobject_cast<Knm::InterfaceConnection*>(activatable);
        if (interfaceConnection) {
            // short cut: only try to update the active InterfaceConnections
            if (interfaceConnection->activationState() == Knm::InterfaceConnection::Unknown) {
                continue;
            }
            //kDebug() <<  "Looking for a Connection.Active for" << interfaceConnection;
            // comparison criteria: service, object path, and device
            bool icActive = false;
            QString icService = interfaceConnection->property("NMDBusService").toString()
;
            QString icDBusObjectPath = interfaceConnection->property("NMDBusObjectPath").toString();
            QDBusObjectPath icDevice = QDBusObjectPath(interfaceConnection->deviceUni());
            foreach (OrgFreedesktopNetworkManagerConnectionActiveInterface * active, d->activeConnections) {

#if 0
                // Just debug
                kDebug() << "NMDBusService" << interfaceConnection->property("NMDBusService").toString()
                    << "==" << active->serviceName() << ":" << (interfaceConnection->property("NMDBusService") == active->serviceName()
                            )<< ",\n"
                    << "NMDBusObjectPath" <<  interfaceConnection->property("NMDBusObjectPath").toString()
                    << "==" << active->connection().path() << ":" << (interfaceConnection->property("NMDBusObjectPath") == active->connection().path()) << ",\n"
                    << interfaceConnection->deviceUni() << "in";
                foreach (QDBusObjectPath path, active->devices()) {
                    kDebug() << "  " << path.path();
                }
                kDebug() << active->devices().contains(QDBusObjectPath(interfaceConnection->deviceUni()));
                kDebug() << "IC is a VPNIC: " << (interfaceConnection->activatableType() == Knm::Activatable::VpnInterfaceConnection);
                // debug ends
#endif
                if (active->serviceName() == icService
                        && active->connection().path() == icDBusObjectPath
                        && (interfaceConnection->activatableType() == Knm::Activatable::VpnInterfaceConnection
                            || active->devices().contains(icDevice))) {
                    //kDebug() << "Found a Connection.Active for this InterfaceConnection, not setting it Unknown";
                    icActive = true;
                    break;
                }
            }
            if (!icActive) {
                //kDebug() << "Making interfaceconnection Unknown:" << Knm::InterfaceConnection::Unknown;
                interfaceConnection->setActivationState(Knm::InterfaceConnection::Unknown);
                interfaceConnection->setHasDefaultRoute(false);
            }
        }
    }
}

void NMDBusActiveConnectionMonitor::activeConnectionChanged(const QVariantMap& changedProps)
{
    OrgFreedesktopNetworkManagerConnectionActiveInterface * active = qobject_cast<OrgFreedesktopNetworkManagerConnectionActiveInterface*>(sender());
    if (active) {
        activeConnectionChangedInternal(active, changedProps);
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
            if (candidate->property("NMDBusService") == connectionActive->serviceName()
                    && candidate->property("NMDBusObjectPath") == connectionActive->connection().path()
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

void NMDBusActiveConnectionMonitor::activeConnectionChangedInternal(OrgFreedesktopNetworkManagerConnectionActiveInterface * iface, const QVariantMap & changedProps)
{
    Knm::InterfaceConnection * interfaceConnection = interfaceConnectionForConnectionActive(iface);
    if (interfaceConnection) {
        interfaceConnection->setProperty("NMDBusActiveConnectionObject", iface->path());

        const QString defaultKey = QLatin1String("Default");
        const QString stateKey = QLatin1String("State");
        if (changedProps.contains(defaultKey)) {
            interfaceConnection->setHasDefaultRoute(changedProps[defaultKey].toBool());
        }
        if (changedProps.contains(stateKey)) {
            interfaceConnection->setActivationState((Knm::InterfaceConnection::ActivationState)changedProps[stateKey].toUInt());
        }
    }
}

void NMDBusActiveConnectionMonitor::activeConnectionChangedInternal(OrgFreedesktopNetworkManagerConnectionActiveInterface * iface, uint state)
{
    Knm::InterfaceConnection * interfaceConnection = interfaceConnectionForConnectionActive(iface);
    if (interfaceConnection) {
        interfaceConnection->setProperty("NMDBusActiveConnectionObject", iface->path());
        interfaceConnection->setActivationState((Knm::InterfaceConnection::ActivationState)state);
    }
}

void NMDBusActiveConnectionMonitor::handleAdd(Knm::Activatable * added)
{
    // are we tracking an active connection for this activatable?
    Q_D(NMDBusActiveConnectionMonitor);
    Knm::InterfaceConnection * interfaceConnection = qobject_cast<Knm::InterfaceConnection*>(added);
    if (interfaceConnection) {
        // look for a corresponding Connection.Active object on the bus
        foreach (OrgFreedesktopNetworkManagerConnectionActiveInterface * active, d->activeConnections) {
            if (interfaceConnection->property("NMDBusService") == active->serviceName()
                    && interfaceConnection->property("NMDBusObjectPath") == active->connection().path()
                    /* check the device matches */
                    && active->devices().contains(QDBusObjectPath(interfaceConnection->deviceUni()))
               ) {
                //kDebug() << "Updating interfaceconnection to" << active->state();
                interfaceConnection->setActivationState((Knm::InterfaceConnection::ActivationState)active->state());
                interfaceConnection->setHasDefaultRoute(active->getDefault());

            } else {
#if 0
                kDebug() << "NMDBusService" << interfaceConnection->property("NMDBusService")
                    << "NMDBusObjectPath" <<  interfaceConnection->property("NMDBusObjectPath")
                    << interfaceConnection->deviceUni() << "in";
                foreach (QDBusObjectPath path, active->devices()) {
                    kDebug() << "  " << path.path();
                }
#endif
            }
        }
    }
}

void NMDBusActiveConnectionMonitor::handleUpdate(Knm::Activatable *)
{
}

void NMDBusActiveConnectionMonitor::handleRemove(Knm::Activatable *)
{
}

void NMDBusActiveConnectionMonitor::networkingStatusChanged(Solid::Networking::Status status)
{
    if (status == Solid::Networking::Unknown) {
        Q_D(NMDBusActiveConnectionMonitor);
        if (status == Solid::Networking::Unknown) {
            // the manager probably exited, clean our state
            qDeleteAll(d->activeConnections);
            d->activeConnections.clear();
        }
    }
}

// vim: sw=4 sts=4 et tw=100
