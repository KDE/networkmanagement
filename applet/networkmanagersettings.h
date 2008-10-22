/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>

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

#ifndef NM_SETTINGS_CLIENT_H
#define NM_SETTINGS_CLIENT_H

#include "nm-settingsinterface.h"

#include <QDBusObjectPath>
#include <QHash>

#include "nm-exported-connectioninterface.h"

class RemoteConnection;

/**
 * Class to cache and access a remote NetworkManagerSettings service
 */
class NetworkManagerSettings : public OrgFreedesktopNetworkManagerSettingsInterface
{
Q_OBJECT
public:
    NetworkManagerSettings(const QString & service, QObject * parent);
    virtual ~NetworkManagerSettings();
    QStringList connections() const;
    RemoteConnection * findConnection(const QString&) const;
Q_SIGNALS:
    void connectionAdded(NetworkManagerSettings *, const QString&);
    void connectionRemoved(NetworkManagerSettings *, const QString&);
    void connectionUpdated(NetworkManagerSettings *, const QString&);
    /**
     * Emitted when the service this object represents leaves the system bus
     */
    void disappeared(NetworkManagerSettings*);
    /**
     * Emitted when the service this object represents joins the system bus
     */
    void appeared(NetworkManagerSettings*);
private Q_SLOTS:
    void onConnectionAdded(const QDBusObjectPath&);
    void onConnectionRemoved();
    void onConnectionUpdated(const QVariantMapMap&);
    void serviceOwnerChanged(const QString&, const QString&, const QString&);
private:
    void makeConnections(RemoteConnection*);
    QHash<QString, RemoteConnection*> m_connections;
    QString m_service;
};

#endif
