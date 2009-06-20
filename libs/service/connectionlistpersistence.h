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

#ifndef CONNECTIONLISTPERSISTENCE_H
#define CONNECTIONLISTPERSISTENCE_H

#include <QObject>
#include "connectionhandler.h"

#include <QMap>
#include <QPair>
#include <QString>
#include <QVariant>

#include <KSharedConfig>

#include "knm_export.h"
#include "marshalarguments.h"
#include <QtGui/qwindowdefs.h> // krazy:exclude=includes (for WId)

class ConnectionList;
class ConnectionListPersistencePrivate;
#include "connection.h"

/**
 * This class extracts stored connections and adds them to the connectionlist
 * 1) get the names of all the connections from the main config file
 * 2) open each connection's file and create a Knm::Connection object
 *
 * 3) Write changed connections back to disk (this is why this class implements ConnectionHandler
 */
class KNM_EXPORT ConnectionListPersistence : public QObject, public ConnectionHandler
{
Q_OBJECT
Q_DECLARE_PRIVATE(ConnectionListPersistence)
public:
    static const QString NETWORKMANAGEMENT_RCFILE;
    static const QString CONNECTION_PERSISTENCE_PATH;

    /**
     * Constructor
     * @param filename to load connection ids from - needed for testing.  By default
     * NETWORKMANAGEMENT_RCFILE is used.
     * @param persistencePath absolute path ending in '/' to search for connection detail files in - needed for
     * testing. By default CONNECTION_PERSISTENCE_PATH is used.
     */
    ConnectionListPersistence(ConnectionList * service, const QString & rcfile = QString(), const QString & persistencePath = QString());

    ~ConnectionListPersistence();

    /**
     * Call to read the connection list from disk
     */
    void init();

    // noop implementations
    void handleAdd(Knm::Connection *);
    void handleRemove(Knm::Connection *);

    /**
     * Takes care of writing changes back to disk
     */
    void handleUpdate(Knm::Connection *);

    /**
     * Reread connections from disk
     * @param changedConnections indicates which connections have been changed
     */
    void configure(const QStringList & changedConnections);

    // UNUSED move
    void start(WId wid);

    // UNUSED move
    void stop();
private:
    // map from a) keys that have been munged to be legal variable names
    // to b) actual networkmanager parameter keys
    QString convertKey(const QString &) const;
    // utility method to do last minute value conversions, if required
    QVariant convertValue(const QString& key, const QVariant& value) const;
    // locate the specified connection's config file
    KSharedConfig::Ptr connectionFileForUuid(const QString & uuid);
    // restore the given connection from storage to a map
    Knm::Connection * restoreConnection(const QString & connectionId);
private:
    ConnectionListPersistencePrivate * d_ptr;
};

#endif // SETTINGS_KCONFIGTOSERVICE_H
