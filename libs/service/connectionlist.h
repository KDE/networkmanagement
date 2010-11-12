/*
Copyright (C) 2008 Christopher Blauvelt <cblauvelt@gmail.com>
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

#ifndef CONNECTION_LIST_H
#define CONNECTION_LIST_H

#include <QObject>
#include <QStringList>

#include "knm_export.h"

//#include <NetworkManager.h>

//DBus specific includes
//#include <QtDBus/QtDBus>
//#include <QDBusObjectPath>

//#include <KConfigGroup>

//#include "busconnection.h"
//#include "marshalarguments.h"

namespace Knm {
class Connection;
}

class ConnectionHandler;
class ConnectionListPrivate;

/**
 * The basic list of connection objects
 *
 * NB this class used to be NetworkSettings and contain NetworkManagerSettings implementation; see the svn history for
 * removed code
 * * ListConnections impl
 * * updating connections timestamps when they become active
 * * NewConnection!
 * * connectionUpdated!
 * * nextObjectPath
 * * busConnectionForInterface
 */
class KNM_EXPORT ConnectionList : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ConnectionList)
    public:
        ConnectionList(QObject *parent = 0);
        virtual ~ConnectionList();

        /**
         * Register a connection handler.  ConnectionHandlers encapsulate peripheral functionality.
         * ConnectionHandlers are called in a defined order.
         * @param handler the handler to register.
         * @param insertAfter the point in the connection handler chain to insert at.  Inserts at end if 0
         * is passed or if insertAfter is not found
         *
         * TODO: do we need separate handler lists for different operations?
         */
        void registerConnectionHandler(ConnectionHandler * handler, ConnectionHandler * insertAfter = 0);

        /**
         * Remove a connection handler
         */
        void unregisterConnectionHandler(ConnectionHandler * handler);

        /**
         * Get the list of UUIDs of known connections
         */
        QStringList connections() const;

        /**
         * Add a Connection.  The connectionList owns the added Connection and takes responsibility
         * for deleting it.
         * Registered ConnectionHandlers are called to handle the add in order
         * @return object path of the new connection
         */
        void addConnection(Knm::Connection *);

        /**
         * Replace a connection.  The existing connection object with the same UUID is deleted.
         * This method should not be necessary longer term
         * Registered ConnectionHandlers are called to handle the replace in order
         */
        void replaceConnection(Knm::Connection * update);

        /**
         * Update a connection in place
         */
        void updateConnection(Knm::Connection * update);

        /**
         * Remove a connection 
         * Registered ConnectionHandlers are called to handle the delete in order
         */
        void removeConnection(Knm::Connection *);

        /**
         * Remove a connection by UUID
         * @param uuid the UUID of the connection to remove. No-op if not found
         */
        void removeConnection(const QString & uuid);

        /**
         * Get a connection by UUID
         * @return 0 if not found
         */
        Knm::Connection * findConnection(const QString & uuid) const;
    private:
        ConnectionListPrivate * d_ptr;
};

#endif
