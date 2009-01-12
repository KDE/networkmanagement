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

#include "connectionwidget.h"

#include <nm-setting-connection.h>

#include <KDebug>
#include <KStandardDirs>

#include "configxml.h"
#include "ui_connection.h"

class ConnectionWidget::Private
{
public:
    Ui_ConnectionSettings ui;
    QString setName;
};

ConnectionWidget::ConnectionWidget(const QString& connectionId, QWidget * parent)
    : SettingWidget(connectionId, parent), d(new ConnectionWidget::Private())
{
    d->ui.setupUi(this);
    init();
    kDebug() << "Connection id is " << connectionId;
}

ConnectionWidget::~ConnectionWidget()
{
    delete d;
}

QTabWidget * ConnectionWidget::connectionSettingsWidget()
{
    return d->ui.tabwidget;
}

QString ConnectionWidget::settingName() const
{
    return QLatin1String(NM_SETTING_CONNECTION_SETTING_NAME);
}

void ConnectionWidget::writeConfig()
{
    SettingWidget::writeConfig();
    kDebug();
    KConfigGroup group(configXml()->config(), settingName());
    group.writeEntry(NM_SETTING_CONNECTION_UUID, connectionId());
    if ( !d->setName.isNull() )
        group.writeEntry(NM_SETTING_CONNECTION_ID, d->setName );
}

void ConnectionWidget::setConnectionName(const QString& name)
{
    KConfigSkeletonItem * configItem = configXml()->findItem(settingName(), "id");
    configItem->setProperty(name);
    d->setName = name;
}

QString ConnectionWidget::connectionName() const
{
    return d->setName;
}

// vim: sw=4 sts=4 et tw=100
