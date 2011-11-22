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

#ifndef NMSYSTEMSETTINGSACTIVATABLEPROVIDER_H
#define NMSYSTEMSETTINGSACTIVATABLEPROVIDER_H

#include "activatableobserver.h"
#include "connectionhandler.h"
#include "connectionlist.h"

#include <QDBusObjectPath>
#include <QDBusPendingCallWatcher>

#include "knm_export.h"

namespace Knm
{
    class Activatable;
    class Connection;
} // namespace Knm

typedef QMap<QString,QVariantMap> QVariantMapMap;

class ConnectionList;
class RemoteConnection;

class NMDBusSettingsConnectionProviderPrivate;

/**
 * Class to cache and access a remote NetworkManagerSettings service
 */
class KNM_EXPORT NMDBusSettingsConnectionProvider : public QObject, public ActivatableObserver
// encapsulate
//: public OrgFreedesktopNetworkManagerSettingsInterface
{
Q_OBJECT
public:
    NMDBusSettingsConnectionProvider(ConnectionList * connectionList, QObject * parent = 0);
    virtual ~NMDBusSettingsConnectionProvider();

    /**
     * tag activatables coming from our connections with dbus object path etc
     */
    void handleAdd(Knm::Activatable * activatable);
    void handleUpdate(Knm::Activatable * activatable);
    void handleRemove(Knm::Activatable * activatable);

    void updateConnection(const QString &uuid, Knm::Connection *newConnection);
    void addConnection(Knm::Connection *newConnection);
    bool getConnectionSecrets(Knm::Connection *con);
    void removeConnection(const QString &uuid);
    ConnectionList * connectionList();

Q_SIGNALS:
    void connectionsChanged();
    void getConnectionSecretsCompleted(bool, const QString &, const QString &);
    void addConnectionCompleted(bool, const QString &);

private Q_SLOTS:
    void onConnectionAdded(const QDBusObjectPath&);
    // should probably be handled in RemoteConnection
    void onRemoteConnectionRemoved();
    // should probably be handled in RemoteConnection
    void onRemoteConnectionUpdated();
    void serviceOwnerChanged(const QString&, const QString&, const QString&);
    void onConnectionSecretsArrived(QDBusPendingCallWatcher *watcher);
    void onConnectionAddArrived(QDBusPendingCallWatcher *watcher);
    void interfaceConnectionActivated();
    void interfaceConnectionDeactivated();
    void onVpnConnectionActivated(QDBusPendingCallWatcher *watcher);
private:
    enum Operation {Add, Remove, Update};

    void initialiseAndRegisterRemoteConnection(const QString & path);
    void makeConnections(RemoteConnection*);

    void clearConnections();
    void initConnections();
    Q_DECLARE_PRIVATE(NMDBusSettingsConnectionProvider)
    NMDBusSettingsConnectionProviderPrivate * d_ptr;
};

#endif // NMSYSTEMSETTINGSACTIVATABLEPROVIDER_H
