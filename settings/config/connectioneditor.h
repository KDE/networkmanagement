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

#ifndef NM07_CONNECTION_EDITOR_H
#define NM07_CONNECTION_EDITOR_H

#include <KCModule>

#include "connectioneditor.h"

#include "ui_connectioneditor.h"

class QMenu;
class ConnectionPreferences;

class ConnectionEditor : public KCModule
{
Q_OBJECT
public:
    ConnectionEditor(QWidget * parent = 0, const QVariantList & args = QVariantList());
    virtual ~ConnectionEditor();
    virtual void save();
    virtual void load();
public slots:
    /**
     * Check if a newly added or removed device should enable or disable tabs
     */
    void updateTabStates();
private slots:
    /** 
     * Add a new connection 
     */
    void addClicked();
    /**
     * Edit selected connection
     */
    void editClicked();
    /**
     * Delete selected connection
     */
    void deleteClicked();
    /**
     * When switching to the vpn or cellular tabs, add a menu to the Add button for selecting
     * connection subtypes
     */
    void tabChanged(int);
    /**
     * temporarily stores the selected connection type in m_nextConnectionType until addClicked
     * reads it
     */
    void connectionTypeMenuTriggered(QAction* action);
private:
    /**
     * Tell the UserSettings service to reload its configuration (via DBUS)
     * Provide a list of changed connection IDs so the service can notify NetworkManager
     */
    void updateService(const QStringList& changedConnections = QStringList()) const;
    /**
     * Construct an editor widget for the selected connection tab.
     * Returns 0 if no tab is selected.
     */
    ConnectionPreferences * editorForCurrentIndex(QWidget * parent, const QVariantList & args) const;
    /**
     * Get the selected item on the current tab.
     * Returns 0 if no selection.
     */
    QTreeWidgetItem * selectedItem() const;
    /**
     * Reparse knetworkmanagerrc (the main connection file) and rebuild the list of connections
     */
    void restoreConnections();

    Ui_ConnectionEditor mConnEditUi;
    QTreeWidget * mWiredList;
    QMenu * mCellularMenu;
    QMenu * mVpnMenu;
    QVariant m_nextConnectionType;
};

#endif
