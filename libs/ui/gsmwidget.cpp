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

#include "gsmwidget.h"

#include <nm-setting-gsm.h>

#include "ui_gsm.h"

class GsmWidget::Private
{
public:
    Ui_Gsm ui;
};

GsmWidget::GsmWidget(const QString& connectionId, QWidget * parent)
: SettingWidget(connectionId, parent), d(new GsmWidget::Private)
{
    d->ui.setupUi(this);
    init();
}

GsmWidget::~GsmWidget()
{
    delete d;
}

QString GsmWidget::label() const
{
    return i18nc("Label for GSM Cellular (3G) network connection", "Mobile Broadband");
}

QString GsmWidget::settingName() const
{
    return QLatin1String(NM_SETTING_GSM_SETTING_NAME);
}

// vim: sw=4 sts=4 et tw=100
