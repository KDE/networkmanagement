/*
Copyright 2009 Will Stephenson <wstephenson@kde.org>
Copyright 2010 Sebastian Kügler <sebas@kde.org>

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

#ifndef OTHERSETTINGSWIDGET_H
#define OTHERSETTINGSWIDGET_H

#include <QWidget>

#include <solid/control/networkinterface.h>

#include "ui_othersettings.h"


class OtherSettingsWidgetPrivate;

class OtherSettingsWidget : public QWidget
{
Q_OBJECT
Q_DECLARE_PRIVATE(OtherSettingsWidget)
public:
    OtherSettingsWidget(QWidget * parent = 0);
    virtual ~OtherSettingsWidget();
public Q_SLOTS:
    void configureNotifications();
private:
    OtherSettingsWidgetPrivate * d_ptr;
};

#endif // OTHERSETTINGSWIDGET_H
