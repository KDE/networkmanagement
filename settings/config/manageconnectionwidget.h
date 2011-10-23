/*
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

#ifndef NM09_MANAGE_CONNECTION_WIDGET_H
#define NM09_MANAGE_CONNECTION_WIDGET_H

#include <QHash>

#include <KCModule>

#include "connectioneditor.h"
#include "connectionlist.h"
#include "nmdbussettingsconnectionprovider.h"

#include "internals/settings/ipv4.h"

#include "ui_manageconnectionwidget.h"
#include "mobileconnectionwizard.h"

class QMenu;
class QTreeWidget;
class QTreeWidgetItem;


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
     * Called when GetSecrets call of Connection.Secrets interface is arrived
     */
    void editGotSecrets(bool valid, const QString &errorMessage, const QString &);
    void addGotConnection(bool valid, const QString &errorMessage);
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
     * Imports a connection of the selected subtype
     */
    void importClicked();
    /**
     * Exports a connection of the selected subtype
     */
    void exportClicked();
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

    bool event(QEvent *ev);

    Ui_ManageConnectionWidget mConnEditUi;
    QTreeWidget * mWiredList;
    Knm::Connection * mEditConnection;
    QMenu * mWiredMenu;
    QMenu * mWirelessMenu;
    QMenu * mCellularMenu;
    QMenu * mVpnMenu;
    ConnectionEditor * mEditor;
    QHash<QString,QTreeWidgetItem*> mUuidItemHash;
    QTimer * mLastUsedTimer;
    MobileConnectionWizard * mMobileConnectionWizard;


    /**
     * ConnectionList for user-wide and system-wide settings
     */
    ConnectionList * mConnections;

    /**
     * ConnectionProvider to add/remove/update user wide settings
     */
    NMDBusSettingsConnectionProvider *mUserSettings;

    /**
     * ConnectionProvider to add/remove/update system wide settings
     */
    NMDBusSettingsConnectionProvider *mSystemSettings;

    /**
     * Connect add/edit/delete button signals to relevant slots
     */
    void connectButtonSet(AddEditDeleteButtonSet*, QTreeWidget*);

    /**
     * This variable will contain all file extensions supported for
     * import by all VPN plugins. The extensions are separated by space,
     * e.g '*.pcf *.ovpn' and are used in KFileDialog on selecting file to import
     */
    QString mSupportedExtns;
};

#endif // NM09_MANAGE_CONNECTION_WIDGET_H
