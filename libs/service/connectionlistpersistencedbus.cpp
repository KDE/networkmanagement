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

#include "connectionlistpersistencedbus.h"

#include <QDBusConnection>

#include "secretstorage.h"

class ConnectionListPersistenceDBusPrivate
{
public:
    ConnectionListPersistence * connectionListPersistence;
};

ConnectionListPersistenceDBus::ConnectionListPersistenceDBus(ConnectionListPersistence * connectionListPersistence, QObject * parent)
: QObject(parent), d_ptr(new ConnectionListPersistenceDBusPrivate)
{
    Q_D(ConnectionListPersistenceDBus);
    d->connectionListPersistence = connectionListPersistence;
    QDBusConnection::sessionBus().registerService("org.kde.networkmanagement");
    QDBusConnection::sessionBus().registerObject("/connections", connectionListPersistence, QDBusConnection::ExportScriptableSlots);
}

ConnectionListPersistenceDBus::~ConnectionListPersistenceDBus()
{
    QDBusConnection::sessionBus().unregisterObject("/connections");
    QDBusConnection::sessionBus().unregisterService("org.kde.networkmanagement");
}
