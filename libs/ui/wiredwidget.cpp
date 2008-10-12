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

#include "wiredwidget.h"
#include <nm-setting-wired.h>
#include "ui_wired.h"

class WiredWidget::Private
{
public:
    Ui_Settings8023Ethernet ui;
};

WiredWidget::WiredWidget(const QString& connectionId, QWidget * parent)
: SettingWidget(connectionId, parent), d(new WiredWidget::Private)
{
    d->ui.setupUi(this);
    init();
}

WiredWidget::~WiredWidget()
{
    delete d;
}

QString WiredWidget::settingName() const
{
    return QLatin1String(NM_SETTING_WIRED_SETTING_NAME);
}

// vim: sw=4 sts=4 et tw=100
