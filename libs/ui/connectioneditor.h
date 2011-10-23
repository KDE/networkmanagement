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

#ifndef CONNECTIONEDITOR_H
#define CONNECTIONEDITOR_H

#include <QObject>
#include <QStringList>

#include "knm_export.h"

#include "internals/connection.h"

namespace Knm {
    class Connection;
}

class ConnectionPreferences;
typedef  QList<QVariant> QVariantList;

/**
 * Control object to manage adding/editing connections
 */
class KNM_EXPORT ConnectionEditor : public QObject
{
Q_OBJECT
public:
    ConnectionEditor(QObject * parent);
    virtual ~ConnectionEditor();

    Knm::Connection *createConnection(bool useDefaults, Knm::Connection::Type type,
            const QVariantList &otherArgs = QVariantList(), const bool autoAccept = false);

    Knm::Connection *editConnection(Knm::Connection::Type type,
            const QVariantList &otherArgs = QVariantList());

    Knm::Connection *editConnection(Knm::Connection *con);

    /**
     * Construct an editor widget for the given connection type.
     */
    ConnectionPreferences * editorForConnectionType(bool setDefaults, QWidget * parent,
                                                    Knm::Connection::Type type,
                                                    const QVariantList & args) const;

    ConnectionPreferences * editorForConnectionType(bool setDefaults, QWidget * parent,
                                                    Knm::Connection *con) const;
};
#endif // CONNECTIONEDITOR_H
