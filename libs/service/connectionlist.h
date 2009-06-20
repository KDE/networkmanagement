/*  This file is part of the KDE project
    Copyright (C) 2008 Christopher Blauvelt <cblauvelt@gmail.com>
    Copyright (C) 2008,2009 Will Stephenson <wstephenson@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

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
         * Update a connection.  The existing connection is deleted.
         * Registered ConnectionHandlers are called to handle the update in order
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
    Q_SIGNALS:
    private:
        ConnectionListPrivate * d_ptr;
};

#endif
