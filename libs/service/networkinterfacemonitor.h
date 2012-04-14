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

#ifndef NETWORKINTERFACEMONITOR_H
#define NETWORKINTERFACEMONITOR_H

#include <QObject>
#include <QDBusPendingCallWatcher>

#include "knm_export.h"

class ConnectionList;
class ActivatableList;
class NetworkInterfaceMonitorPrivate;

/**
 * Monitors network hardware and maintains NetworkInterfaceActivatableProviders for them
 */
class KNM_EXPORT NetworkInterfaceMonitor : public QObject
{
Q_OBJECT
Q_DECLARE_PRIVATE(NetworkInterfaceMonitor)
public:
    NetworkInterfaceMonitor(ConnectionList * connectionList, ActivatableList * activatableList, QObject * parent);
    virtual ~NetworkInterfaceMonitor();
public Q_SLOTS:
    void networkInterfaceAdded(const QString &);
    void networkInterfaceRemoved(const QString &);
    void modemInterfaceAdded(const QString&);
    void requestPin(const QString &);
    void onSendPinArrived(QDBusPendingCallWatcher *);
private:
    NetworkInterfaceMonitorPrivate * d_ptr;
};

#endif // NETWORKINTERFACEMONITOR_H
