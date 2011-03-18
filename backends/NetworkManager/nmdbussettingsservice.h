/*
Copyright 2008,2009 Will Stephenson <wstephenson@kde.org>

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
class KNM_EXPORT NMDBusSettingsService : public QObject, public ActivatableObserver, public ConnectionHandler
{
Q_OBJECT
Q_CLASSINFO("D-Bus Interface", "org.freedesktop.NetworkManager.Settings")

public:
    enum ServiceStatus { Available, AlreadyRunning, AccessDenied, UnknownError };
    NMDBusSettingsService(QObject * parent = 0);
    virtual ~NMDBusSettingsService();
    static const QString SERVICE_USER_SETTINGS;
    static const QString SERVICE_SYSTEM_SETTINGS;
    void handleAdd(Knm::Connection *);
    void handleUpdate(Knm::Connection *);
    void handleRemove(Knm::Connection *);

    void handleAdd(Knm::Activatable *);
    void handleUpdate(Knm::Activatable *);
    void handleRemove(Knm::Activatable *);
    ServiceStatus serviceStatus() const;
public Q_SLOTS:
    Q_SCRIPTABLE QList<QDBusObjectPath> ListConnections() const;
Q_SIGNALS:
    Q_SCRIPTABLE void NewConnection(const QDBusObjectPath&);
    /**
     * Indicate that the dbus service is registered
     */
    void serviceAvailable(bool);
private Q_SLOTS:
    // activate connections
    void interfaceConnectionActivated();
    void interfaceConnectionDeactivated();

private:
    Q_DECLARE_PRIVATE(NMDBusSettingsService)
    QUuid uuidForPath(const QDBusObjectPath&) const;

    void registerService();
    QString nextObjectPath();
    NMDBusSettingsServicePrivate * d_ptr;
};

#endif // BACKENDS_NETWORKMANAGER_NMDBUSSETTINGSSERVICE_H
