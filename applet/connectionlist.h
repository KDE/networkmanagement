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

#ifndef CONNECTIONLIST_H
#define CONNECTIONLIST_H

#include <QGraphicsWidget>
#include <QHash>
#include <QPair>

#include <Plasma/ExtenderItem>
#include <Plasma/Extender>

class AbstractConnectableItem;
class NetworkManagerSettings;
class QGraphicsLinearLayout;
class ActivatableItem;

class RemoteActivatable;
class RemoteActivatableList;

class ConnectionList: public Plasma::ExtenderItem
{
Q_OBJECT
public:
    ConnectionList(RemoteActivatableList *, Plasma::Extender * ext = 0);
    virtual ~ConnectionList();

    void init(); // fill connection list, after ctor has run so subclasses are initialised
    /**
     * called before the main connection list layout is added.
     * Add any items that should appear below it here
     */
    virtual void setupHeader() = 0; // puts the interfaceitems at the top if needed
    /**
     * called after the main connection list layout is added.
     * Add any items that should appear below it here
     */
    virtual void setupFooter() = 0; // puts the interfaceitems at the top if needed
    virtual bool accept(RemoteActivatable *) const = 0; // do type specific checks
    virtual ActivatableItem * createItem(RemoteActivatable *); // instantiate type-specific connectionitem
    virtual QGraphicsItem * widget();
    bool isEmpty();
    void reassess();

public Q_SLOTS:

    //void reassess();
    virtual void activate(ActivatableItem*) = 0;
    void activatableAdded(RemoteActivatable *);
    void activatableRemoved(RemoteActivatable *);
    /**
     * examine all connections from this service
     * @return true if a connection was added
     */
    bool getList();
    void listDisappeared();
Q_SIGNALS:
    void connectionListUpdated();
protected:
    /**
     * examine a connection
     * @return true if the connection was added to the list
     */
    bool registerActivatable(RemoteActivatable*);
    RemoteActivatableList * m_activatables;
    QGraphicsLinearLayout * m_layout;
    QGraphicsWidget * m_widget;
    QHash<RemoteActivatable*, ActivatableItem *> m_connections;
    RemoteActivatable* m_activeConnection;

private:
    void addSettingsService(NetworkManagerSettings*);
    QGraphicsLinearLayout * m_connectionLayout;
};
#endif // CONNECTIONLIST_H
