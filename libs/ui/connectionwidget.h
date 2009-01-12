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

class KNM_EXPORT ConnectionWidget : public SettingWidget
{
public:
    ConnectionWidget(const QString& connectionId, QWidget * parent = 0);
    ~ConnectionWidget();
    QTabWidget * connectionSettingsWidget();
    QString settingName() const;

    void writeConfig();
    /**
     * set the user-visible, non-unique connection name
     */
    void setConnectionName(const QString& name);
    QString connectionName() const;
private:
    class Private;
    Private * d;
};

#endif // CONNECTIONWIDGET_H
