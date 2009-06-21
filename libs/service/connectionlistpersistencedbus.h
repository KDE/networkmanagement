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

#ifndef CONNECTIONLISTPERSISTENCEDBUS_H
#define CONNECTIONLISTPERSISTENCEDBUS_H

#include <QObject>

#include "knm_export.h"

class ConnectionListPersistence;
class ConnectionListPersistenceDBusPrivate;

/**
 * Exposes the configure() method to the session bus so that the configuration UI processes can tell
 * service implementations to reload
 */
class KNM_EXPORT ConnectionListPersistenceDBus : public QObject
{
Q_OBJECT
Q_DECLARE_PRIVATE(ConnectionListPersistenceDBus)
public:
    ConnectionListPersistenceDBus(ConnectionListPersistence * connectionListPersistence, QObject * parent);
    ~ConnectionListPersistenceDBus();
private:
    ConnectionListPersistenceDBusPrivate * d_ptr;
};
// vim: sw=4 sts=4 et tw=100
#endif // CONNECTIONLISTPERSISTENCEDBUS_H
