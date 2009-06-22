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

#include <QMultiHash>
#include <QUuid>

#include <KDebug>
#include <solid/control/networkmanager.h>

#include "interfaceconnection.h"

#include "activatablelist.h"

#include "nmdbussettingsservice.h"

#include "nm-active-connectioninterface.h"

class NMDBusActiveConnectionMonitorPrivate
{
public:
    ActivatableList * activatableList;
    QHash<QString, OrgFreedesktopNetworkManagerConnectionActiveInterface *> activeConnections;
    // HACK necessary because after changing a Connection.Active's state to Unknown(0), NM deletes the
    // object so we can no longer get the connection path to reset the state of the InterfaceConnections
    // so we keep a list of those we've changed, and when removing a stale OFDNMCActiveInterface,
    // reset the state of all of those and then remove the entry in this hash
    QHash<QString, QList<Knm::InterfaceConnection *> > modifiedConnections;
    NMDBusSettingsService * service;
};

NMDBusActiveConnectionMonitor::NMDBusActiveConnectionMonitor(NMDBusSettingsService * service, ActivatableList * activatables, QObject * parent)
: ActivatableObserver(parent), d_ptr(new NMDBusActiveConnectionMonitorPrivate)
{
    Q_D(NMDBusActiveConnectionMonitor);
    d->activatableList = activatables;
    d->service = service;

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
    // what if this is called before our activatableList is up to date?
    // Then we update the activatables as they come in
    Q_D(NMDBusActiveConnectionMonitor);
    kDebug() << d->activeConnections.count() << "active connections known";

    QStringList currentActiveConnections = Solid::Control::NetworkManager::activeConnections();

    // delete any stale interfaces
    foreach (OrgFreedesktopNetworkManagerConnectionActiveInterface * active, d->activeConnections) {
        if (!currentActiveConnections.contains(active->path())) {
            kDebug() << "removing stale active connection" << active->path();
            d->activeConnections.remove(active->path());

            kDebug() << "resetting state of changed InterfaceConnections";
            QList<Knm::InterfaceConnection*> modifiedConnections = d->modifiedConnections.take(active->path());
            foreach (Knm::InterfaceConnection * ic, modifiedConnections) {
                ic->setActivationState(Knm::InterfaceConnection::Unknown);
            }

            delete active;
        }
    }
    // create an interface to any active connections we're not already tracking
    foreach (QString activeConnectionPath, currentActiveConnections) {
        if (!d->activeConnections.contains(activeConnectionPath)) {
            kDebug() << "Adding active connection interface for " << activeConnectionPath;
            OrgFreedesktopNetworkManagerConnectionActiveInterface * active = new OrgFreedesktopNetworkManagerConnectionActiveInterface("org.freedesktop.NetworkManager", activeConnectionPath, QDBusConnection::systemBus(), this);
            d->activeConnections.insert(activeConnectionPath, active);
            connect(active, SIGNAL(PropertiesChanged(const QVariantMap&)), this, SLOT(activeConnectionChanged(const QVariantMap&)));

            // update activatables' state
            activeConnectionChangedInternal(active, active->state());
        }
    }
}

void NMDBusActiveConnectionMonitor::activeConnectionChanged(const QVariantMap& props)
{
    OrgFreedesktopNetworkManagerConnectionActiveInterface * active = qobject_cast<OrgFreedesktopNetworkManagerConnectionActiveInterface*>(sender());
    if (active) {
        activeConnectionChangedInternal(active, active->state());
    }
}

void NMDBusActiveConnectionMonitor::activeConnectionChangedInternal(OrgFreedesktopNetworkManagerConnectionActiveInterface * iface, uint state)
{
    Q_D(NMDBusActiveConnectionMonitor);
    kDebug() << iface << state;
    if (iface->serviceName() == QLatin1String("org.freedesktop.NetworkManagerUserSettings")) {
        // get the uuid for the connection from the service, then get the interfaceconnections
        // derived from it
        kDebug() << "iface says connection is at path:" << iface->connection().path();
        kDebug() << "which has UUID:" << d->service->uuidForPath(QDBusObjectPath(iface->connection().path()));
        QList<Knm::InterfaceConnection*> interfaceConnections = d->activatableList->interfaceConnectionsForUuid(d->service->uuidForPath(QDBusObjectPath(iface->connection().path())));
        kDebug() << "There are " << interfaceConnections.count() << "InterfaceConnections to update";

        foreach (Knm::InterfaceConnection* ic, interfaceConnections) {
            // TODO proper conversion between NM activation state and
            // InterfaceConnection::Activationstate
            ic->setActivationState((Knm::InterfaceConnection::ActivationState)state);
        }
        d->modifiedConnections.insert(iface->path(), interfaceConnections);
    } else {
        kDebug() << "TODO: handle InterfaceConnection updates for system settings. serviceName=" << iface->serviceName();
    }
}

void NMDBusActiveConnectionMonitor::handleAdd(Knm::Activatable * added)
{
    // are we tracking an active connection for this activatable?
    Q_D(NMDBusActiveConnectionMonitor);
    Knm::InterfaceConnection * interfaceConnection = qobject_cast<Knm::InterfaceConnection*>(added);
    if (interfaceConnection) {
        foreach (OrgFreedesktopNetworkManagerConnectionActiveInterface * active, d->activeConnections) {
            // on the user service?
            QUuid uuid = d->service->uuidForPath(active->connection());
            if (uuid == interfaceConnection->connectionUuid() 
                    && active->devices().contains(QDBusObjectPath(interfaceConnection->deviceUni()))) {
                // this interfaceconnection is already active
                interfaceConnection->setActivationState((Knm::InterfaceConnection::ActivationState)active->state());
                QList<Knm::InterfaceConnection*> modified = d->modifiedConnections[active->path()];
                modified.append(interfaceConnection);
                d->modifiedConnections.insert(active->path(), modified);
            }
        }
    }
}

void NMDBusActiveConnectionMonitor::handleUpdate(Knm::Activatable * )
{

}

void NMDBusActiveConnectionMonitor::handleRemove(Knm::Activatable * removed)
{
    // slow but hash is mostly empty
    // iterate each list of modified ICs
    // if found, remove it and reinsert
    Q_D(NMDBusActiveConnectionMonitor);
    Knm::InterfaceConnection * ic = qobject_cast<Knm::InterfaceConnection*>(removed);
    if (ic) {
        QMutableHashIterator<QString, QList<Knm::InterfaceConnection *> > it(d->modifiedConnections);
        while (it.hasNext()) {
            it.next();
            QList<Knm::InterfaceConnection *> modifiedList = it.value();
            if (modifiedList.removeOne(ic)) {
                it.setValue(modifiedList);
            }
        }
    }
}

void NMDBusActiveConnectionMonitor::networkingStatusChanged(Solid::Networking::Status status)
{
    if (status == Solid::Networking::Unknown) {
        Q_D(NMDBusActiveConnectionMonitor);
        if (status == Solid::Networking::Unknown) {
            // the manager probably exited, clean our state
            qDeleteAll(d->activeConnections);
            d->activeConnections.clear();
            d->modifiedConnections.clear();
        }
    }
}

// vim: sw=4 sts=4 et tw=100
