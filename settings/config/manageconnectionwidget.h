/*
Copyright 2008,2009 Will Stephenson <wstephenson@kde.org>

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

#ifndef NM07_MANAGE_CONNECTION_WIDGET_H
#define NM07_MANAGE_CONNECTION_WIDGET_H

#include <QHash>

#include <KCModule>

#include "connectioneditor.h"

#include "ui_manageconnectionwidget.h"

class QMenu;
class QTreeWidget;
class QTreeWidgetItem;

class TraySettingsWidget;

class ManageConnectionWidget : public KCModule
{
Q_OBJECT
public:
    ManageConnectionWidget(QWidget * parent = 0, const QVariantList & args = QVariantList());
    virtual ~ManageConnectionWidget();
    virtual void save();
    virtual void load();
public slots:
    /**
     * Check if a newly added or removed device should enable or disable tabs
     */
    void updateTabStates();

    Q_SCRIPTABLE void createConnection(const QString & connectionType, const QVariantList &args);
    /**
     * Update connection items last used timestamp if they become active
     */
    void activeConnectionsChanged();
private slots:
    /** 
     * Add a new connection 
     */
    void addClicked();
    /**
     * Edit the selected connection
     */
    void editClicked();
    /**
     * Edit given item
     */
    void editItem(QTreeWidgetItem*);
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
     * adds a connection of the selected subtype
     */
    void connectionTypeMenuTriggered(QAction* action);
    /**
     * Reparse knetworkmanagerrc (the main connection file) and rebuild the list of connections
     */
    void restoreConnections();
    /**
     * Update the relative last used labels (called once a minute on a timer, cheesy I know)
     */
    void updateLastUsed();

    /**
     * Used by the tray settings widget to indicate when its unmanaged widgets change
     */
    void otherSettingsChanged();
private:
    /**
     * Get the connection type of the currently selected index
     * @return connection type enum, Wireless if not found
     */
    Knm::Connection::Type connectionTypeForCurrentIndex() const;
    /**
     * Get the selected item on the current tab.
     * Returns 0 if no selection.
     */
    QTreeWidgetItem * selectedItem() const;

    /**
     * Format a date as something relative to now
     */
    QString formatDateRelative(const QDateTime & lastUsed);
    void updateLastUsed(QTreeWidget *);

    Ui_ManageConnectionWidget mConnEditUi;
    QTreeWidget * mWiredList;
    QMenu * mCellularMenu;
    QMenu * mVpnMenu;
    ConnectionEditor * mEditor;
    QHash<QString,QTreeWidgetItem*> mUuidItemHash;
    QTimer * mLastUsedTimer;
    TraySettingsWidget * mTraySettingsWidget;
};

#endif // NM07_MANAGE_CONNECTION_WIDGET_H
