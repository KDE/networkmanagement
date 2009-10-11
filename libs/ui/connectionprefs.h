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

#ifndef CONNECTIONPREFS_H
#define CONNECTIONPREFS_H

#include <KCModule>

#include "knm_export.h"

class ConnectionWidget;
class SettingWidget;
class SettingInterface;

namespace Knm
{
    class Connection;
    class ConnectionPersistence;
} // namespace Knm

/**
 * Base class for connection configuration widgets.  Exists to provide
 * name() and type() so these values can be cached in the main
 * knetworkmanagerrc file
 */
class KNM_EXPORT ConnectionPreferences : public KCModule
{
Q_OBJECT
public:
    /**
     * args contains 1) The connection id
     * Subsequent args are type dependent.
     * Hint: @see WirelessPreferences
     */
    ConnectionPreferences(const KComponentData&, QWidget * parent = 0, const QVariantList & args = QVariantList());
    virtual ~ConnectionPreferences();

    Knm::Connection * connection() const;

    // if this returns true in derived classes
    // it signals that the defaults are good enough to connect
    virtual bool needsEdits() const = 0;

public Q_SLOTS:
    // reimplemented from KCModule, to call embedded settingswidgets' load/save methods
    virtual void load();
    virtual void save();
protected Q_SLOTS:
    void gotSecrets(uint result);
protected:
    /**
     * Add widgets to the tabs in the connection widget
     */
    int addToTabWidget(SettingWidget *);

    /**
     * Add widgets to the KConfig magic but _not_ to the tab widget
     */
    void addSettingWidget(SettingWidget *);
    /** Main widget for connection info UI, common to all connections **/
    ConnectionWidget * m_contents;
    /** All setting widgets except m_contents, including connectionTypeWidget*/
    QList<SettingWidget*> m_settingWidgets;
    Knm::Connection * m_connection;
    Knm::ConnectionPersistence * m_connectionPersistence;
};

#endif // CONNECTIONPREFS_H
