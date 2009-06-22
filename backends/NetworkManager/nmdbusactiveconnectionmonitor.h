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

#ifndef BACKENDS_NETWORKMANAGER_NMDBUSACTIVECONNECTIONMONITOR_H
#define BACKENDS_NETWORKMANAGER_NMDBUSACTIVECONNECTIONMONITOR_H

#include "activatableobserver.h"

#include <QVariantMap>

#include <Solid/Networking>

#include "knm_export.h"

class ActivatableList;
class NMDBusSettingsService;
class NMDBusSystemSettingsMonitor;

class NMDBusActiveConnectionMonitorPrivate;
class OrgFreedesktopNetworkManagerConnectionActiveInterface;

/**
 * Watches the list of active connections and updates InterfaceConnections' state
 */
class KNM_EXPORT NMDBusActiveConnectionMonitor : public ActivatableObserver
{
Q_OBJECT
public:
    NMDBusActiveConnectionMonitor(NMDBusSettingsService * service, ActivatableList * activatables, QObject * parent = 0);
    ~NMDBusActiveConnectionMonitor();
public Q_SLOTS:
    /**
     * Used to keep track of InterfaceConnections
     */
    void handleAdd(Knm::Activatable *);
    void handleUpdate(Knm::Activatable *);
    /**
     * We keep references to InterfaceConnections we have modified.  This cleans that list in case
     * an InterfaceConnection is removed while we have modified it.
     */
    void handleRemove(Knm::Activatable *);

private Q_SLOTS:
    /**
     * Start tracking a Connection.Active
     */
    void activeConnectionListChanged();

    /**
     * Update the state on affected activatables
     */
    void activeConnectionChanged(const QVariantMap &);

    /**
     * Clean state if NetworkManager exits
     */
    void networkingStatusChanged(Solid::Networking::Status);

private:
    void activeConnectionChangedInternal(OrgFreedesktopNetworkManagerConnectionActiveInterface *, uint);
    Q_DECLARE_PRIVATE(NMDBusActiveConnectionMonitor);
    NMDBusActiveConnectionMonitorPrivate * d_ptr;
};

#endif // BACKENDS_NETWORKMANAGER_NMDBUSACTIVECONNECTIONMONITOR_H
