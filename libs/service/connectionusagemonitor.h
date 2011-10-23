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

#ifndef CONNECTIONUSAGEMONITOR_H
#define CONNECTIONUSAGEMONITOR_H

#include "interfaceconnection.h"

#include "activatableobserver.h"

namespace Knm
{
    class Activatable;
}

class ActivatableList;
class ConnectionList;

class ConnectionUsageMonitorPrivate;

/**
 * Watch activatables for state changes.  Update Connection's timestamps accordingly
 */
class KNM_EXPORT ConnectionUsageMonitor : public QObject, public ActivatableObserver
{
Q_OBJECT
public:
    ConnectionUsageMonitor(ConnectionList * connectionList, ActivatableList * activatableList, QObject * parent);
    virtual ~ConnectionUsageMonitor();
    /**
     * Begin listening to a connection's state changes
     */
    void handleAdd(Knm::Activatable *);
    /**
     * dummy impl
     */
    void handleUpdate(Knm::Activatable *);
    /**
     * dummy impl
     */
    void handleRemove(Knm::Activatable *);

protected Q_SLOTS:
    /**
     * When an activatable goes Active, update the timestamp on the corresponding connection
     */
    void handleActivationStateChange(Knm::InterfaceConnection::ActivationState, Knm::InterfaceConnection::ActivationState);

    /**
     * Listen to interfaces' access point change signals
     */
    void networkInterfaceAdded(const QString& uni);
    /**
     * When an interface roams, update seen-bssids
     */
    void networkInterfaceAccessPointChanged(const QString & apiUni);

private:
    Q_DECLARE_PRIVATE(ConnectionUsageMonitor)
    ConnectionUsageMonitorPrivate * d_ptr;
};

#endif // CONNECTIONUSAGEMONITOR_H
