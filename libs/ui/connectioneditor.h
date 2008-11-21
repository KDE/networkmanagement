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

class ConnectionPreferences;
typedef  QList<QVariant> QVariantList;

/**
 * Control object to manage adding/editing connections
 */
class KNM_EXPORT ConnectionEditor : public QObject
{
Q_OBJECT
public:
    enum ConnectionType { Wired, Wireless, Cellular, Vpn, Pppoe };
    ConnectionEditor(QObject * parent);
    virtual ~ConnectionEditor();
    /**
     * Get the connection type for a given string (matches nm-setting.h SETTING_NAME strings)
     * @return connection type enum, Wireless if not found
     */
    void addConnection(ConnectionEditor::ConnectionType connectionType,
            const QVariantList &otherArgs = QVariantList());
    ConnectionEditor::ConnectionType connectionTypeForString(const QString&) const;
    /**
     * Construct an editor widget for the given connection type.
     */
    ConnectionPreferences * editorForConnectionType(QWidget * parent, ConnectionEditor::ConnectionType type, const QVariantList & args) const;
    /**
     * Tell the UserSettings service to reload its configuration (via DBUS)
     * Provide a list of changed connection IDs so the service can notify NetworkManager
     */
    void updateService(const QStringList& changedConnections = QStringList()) const;
Q_SIGNALS:
    void connectionsChanged();
private:
};
#endif // CONNECTIONEDITOR_H
