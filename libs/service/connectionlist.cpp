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

#include "connectionlist.h"

#include <QHash>
#include <QList>

#include <KDebug>

// internals includes
#include "connection.h"

#include "connectionhandler.h"

class ConnectionListPrivate
{
    public:
        QList<ConnectionHandler *> connectionHandlers;
        QHash<QString, Knm::Connection *> connections;
};

ConnectionList::ConnectionList(QObject * parent)
: QObject(parent), d_ptr(new ConnectionListPrivate)
{
}

ConnectionList::~ConnectionList()
{
    Q_D(ConnectionList);
    foreach (Knm::Connection * connection, d->connections) {
        delete connection;
    }
    delete d;
}

void ConnectionList::registerConnectionHandler(ConnectionHandler * handler, ConnectionHandler * insertAfter)
{
    Q_D(ConnectionList);
    if (handler) {
        // each handler may only be registered once
        if (d->connectionHandlers.contains(handler)) {
            return;
        }

        // inserts at end if insertAfter not found (therefore if it is 0)
        QMutableListIterator<ConnectionHandler*> i(d->connectionHandlers);
        i.findNext(insertAfter);
        i.insert(handler);
    }
}

void ConnectionList::unregisterConnectionHandler(ConnectionHandler * handler)
{
    Q_D(ConnectionList);
    if (handler && !d->connectionHandlers.isEmpty()) {
        d->connectionHandlers.removeOne(handler);
    }
}

QStringList ConnectionList::connections() const
{
    Q_D(const ConnectionList);
    return d->connections.keys();
}

void ConnectionList::addConnection(Knm::Connection * connection)
{
    Q_D(ConnectionList);
    if (connection && !d->connections.contains(connection->uuid())) {
        kDebug() << connection->uuid();
        d->connections.insert(connection->uuid(), connection);
        foreach (ConnectionHandler * connHandler, d->connectionHandlers) {
            connHandler->handleAdd(connection);
        }
    }
}

void ConnectionList::replaceConnection(Knm::Connection * connection)
{
    Q_D(ConnectionList);
    Knm::Connection * existing = 0;
    if (connection) {
        if ((existing = findConnection(connection->uuid()))) {
            delete existing;
            d->connections.insert(connection->uuid(), connection);
            foreach (ConnectionHandler * connHandler, d->connectionHandlers) {
                connHandler->handleUpdate(connection);
            }
        }
    }
}

void ConnectionList::updateConnection(Knm::Connection * connection)
{
    Q_D(ConnectionList);
    foreach (ConnectionHandler * connHandler, d->connectionHandlers) {
        connHandler->handleUpdate(connection);
    }
}

void ConnectionList::removeConnection(const QString & uuid)
{
    removeConnection(findConnection(uuid));
}


void ConnectionList::removeConnection(Knm::Connection * connection)
{
    Q_D(ConnectionList);
    kDebug() << connection->uuid();
    if (connection && d->connections.contains(connection->uuid())) {
        d->connections.take(connection->uuid());
        // reverse iterate the handlers
        QListIterator<ConnectionHandler*> it(d->connectionHandlers);
        it.toBack();
        while (it.hasPrevious()) {
            ConnectionHandler * connHandler = it.previous();
            connHandler->handleRemove(connection);
        }
    }
    delete connection;
}

Knm::Connection * ConnectionList::findConnection(const QString & uuid) const
{
    Q_D(const ConnectionList);
    if (d->connections.contains(uuid)) {
        return d->connections[uuid];
    } else {
        return 0;
    }
}

