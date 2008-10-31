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

#ifndef CONNECTIONLIST_H
#define CONNECTIONLIST_H

#include <QGraphicsWidget>
#include <QHash>
#include <QPair>

class NetworkManagerSettings;
class QGraphicsLinearLayout;
class ConnectionItem;
class InterfaceItem;
class RemoteConnection;
class WirelessEnvironment;

typedef QPair<QString,QString> QStringPair;
typedef QHash<QStringPair, ConnectionItem*> ServiceConnectionHash;

class ConnectionList : public QGraphicsWidget
{
Q_OBJECT
public:
    ConnectionList(NetworkManagerSettings * userSettings, NetworkManagerSettings * systemSettings, QGraphicsWidget * parent = 0);
    virtual ~ConnectionList();

    void init(); // fill connection list, after ctor has run so subclasses are initialised
    virtual void setupHeader() = 0; // puts the interfaceitems at the top if needed
    virtual bool accept(RemoteConnection *) const = 0; // do type specific checks
    virtual ConnectionItem * createItem(RemoteConnection * conn); // instantiate type-specific connectionitem
public Q_SLOTS:

// from IG
    void reassess();
    virtual void activateConnection(ConnectionItem*) = 0;
    void connectionAddedToService(NetworkManagerSettings *, const QString&);
    void connectionRemovedFromService(NetworkManagerSettings *, const QString&);
    void serviceAppeared(NetworkManagerSettings*);
    void serviceDisappeared(NetworkManagerSettings*);

protected:
    void processConnection(NetworkManagerSettings * service, const QString& connectionPath); // check if already exists, accept() checks
    NetworkManagerSettings * m_userSettings;
    NetworkManagerSettings * m_systemSettings;
    QGraphicsLinearLayout * m_layout;
private:
    void addSettingsService(NetworkManagerSettings*);
    // list of connection objects for this interface type
    ServiceConnectionHash m_connections;
    QGraphicsLinearLayout * m_connectionLayout;
};
#endif // CONNECTIONLIST_H
