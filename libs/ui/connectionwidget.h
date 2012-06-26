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

#ifndef CONNECTIONWIDGET_H
#define CONNECTIONWIDGET_H

#include <QWidget>

#include "settingwidget.h"

#include "knm_export.h"

class QTabWidget;
class QDBusPendingCallWatcher;
namespace Knm
{
    class Connection;
} // namespace Knm

class ConnectionWidgetPrivate;
class KNM_EXPORT ConnectionWidget : public SettingWidget
{
Q_OBJECT
Q_DECLARE_PRIVATE(ConnectionWidget)
public:
    /**
     * @param defaultName a default name to set on the connection, if the supplied connection does
     * not have a name.
     */
    ConnectionWidget(QWidget * parent = 0);
    ~ConnectionWidget();

    QTabWidget * connectionSettingsWidget();
    QString settingName() const;

    void readConfig();
    void writeConfig();
    /**
     * set the user-visible, non-unique connection name
     */
    void setConnectionName(const QString& name);
    QString connectionName() const;
    /**
     * guarantee that the connection name is not empty
     */
    void validate();

public Q_SLOTS:
    void setDefaultName(const QString & defaultName);

private Q_SLOTS:
    void buttonPermissionsClicked();
    void getZonesCallFinished(QDBusPendingCallWatcher* watcher);
};

#endif // CONNECTIONWIDGET_H
