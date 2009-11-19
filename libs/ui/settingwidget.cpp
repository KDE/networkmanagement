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

#include "settingwidget.h"
#include "settingwidget_p.h"
#include <connection.h>

SettingWidgetPrivate::SettingWidgetPrivate()
: valid(true)
{
}

SettingWidget::SettingWidget(Knm::Connection * connection, QWidget* parent)
: QWidget(parent)
, d_ptr(new SettingWidgetPrivate)
{
    d_ptr->connection = connection;
}

SettingWidget::SettingWidget(SettingWidgetPrivate &dd, Knm::Connection * connection, QWidget* parent)
: QWidget(parent)
, d_ptr(&dd)
{
    d_ptr->connection = connection;
}

SettingWidget::SettingWidget(SettingWidgetPrivate &dd, QWidget* parent)
: QWidget(parent)
, d_ptr(&dd)
{
}

SettingWidget::~SettingWidget()
{
    delete d_ptr;
}

void SettingWidget::setConnection(Knm::Connection * connection)
{
    d_ptr->connection = connection;
}

Knm::Connection * SettingWidget::connection() const
{
    Q_D(const SettingWidget);
    return d->connection;
}

void SettingWidget::readSecrets()
{
    //default impl does nothing
}

bool SettingWidget::isValid() const
{
    Q_D(const SettingWidget);
    return d->valid;
}

void SettingWidget::setValid(bool valid)
{
    Q_D(SettingWidget);
    d->valid = valid;
}

// vim: sw=4 sts=4 et tw=100
