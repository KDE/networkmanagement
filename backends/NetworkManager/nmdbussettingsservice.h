/*
Copyright 2008,2009 Will Stephenson <wstephenson@kde.org>

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

#ifndef NMDBUSSETTINGSSERVICE_H
#define NMDBUSSETTINGSSERVICE_H

#include "activatableobserver.h"
#include "connectionhandler.h"

#include <QList>
#include <QUuid>

#include <QDBusObjectPath>

#include "knm_export.h"

class ConnectionList;
class NMDBusSettingsServicePrivate;

/**
 * NetworkManager specific component
 * Maintains the UserSettings service on the system bus and exposes Connections as DBus object
 * Should be registered as a ConnectionHandler before the connection list is initialised so it sees
 * the Connections with handleAdd()
 */
class KNM_EXPORT NMDBusSettingsService : public ActivatableObserver, public ConnectionHandler
{
Q_OBJECT
Q_CLASSINFO("D-Bus Interface", "org.freedesktop.NetworkManagerSettings")

public:
    NMDBusSettingsService(QObject * parent = 0);
    virtual ~NMDBusSettingsService();
    static const QString SERVICE_USER_SETTINGS;
    static const QString SERVICE_SYSTEM_SETTINGS;
    void handleAdd(Knm::Connection *);
    void handleUpdate(Knm::Connection *);
    void handleRemove(Knm::Connection *);

    // experimental
    //QDBusObjectPath pathForConnection(const QUuid &uuid);
    QUuid uuidForPath(const QDBusObjectPath&) const;

public Q_SLOTS:
    void handleAdd(Knm::Activatable *);
    void handleUpdate(Knm::Activatable *);
    void handleRemove(Knm::Activatable *);
    Q_SCRIPTABLE QList<QDBusObjectPath> ListConnections() const;
Q_SIGNALS:
    Q_SCRIPTABLE void NewConnection(const QDBusObjectPath&);
private Q_SLOTS:
    // DBus service management
    void serviceOwnerChanged(const QString& service, const QString& oldOwner, const QString& newOwner);
    void serviceRegistered(const QString&);
    void serviceUnregistered(const QString&);
    // activate connections
    void interfaceConnectionActivated();

private:
    Q_DECLARE_PRIVATE(NMDBusSettingsService)
    void registerService();
    QString nextObjectPath();
    NMDBusSettingsServicePrivate * d_ptr;
};

#endif // BACKENDS_NETWORKMANAGER_NMDBUSSETTINGSSERVICE_H
