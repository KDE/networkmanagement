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

#include <KDebug>

#include "ui_connection.h"
#include "connection.h"

class ConnectionWidget::Private
{
public:
    Ui_ConnectionSettings ui;
    QString setName;
};

ConnectionWidget::ConnectionWidget(Knm::Connection * connection, QWidget * parent)
    : SettingWidget(connection, parent), d(new ConnectionWidget::Private())
{
    d->ui.setupUi(this);
    kDebug() << "Connection id is " << connection->uuid();
}

ConnectionWidget::~ConnectionWidget()
{
    delete d;
}

QTabWidget * ConnectionWidget::connectionSettingsWidget()
{
    return d->ui.tabwidget;
}

void ConnectionWidget::readConfig()
{
    d->ui.id->setText(connection()->name());
    d->ui.autoconnect->setChecked(connection()->autoConnect());
}

void ConnectionWidget::writeConfig()
{
    connection()->setName(d->ui.id->text());
    connection()->setAutoConnect(d->ui.autoconnect->isChecked());
}

// vim: sw=4 sts=4 et tw=100
