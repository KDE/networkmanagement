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

#ifndef PPPOEWIDGET_H
#define PPPOEWIDGET_H

#include "settingwidget.h"

#include "knm_export.h"

class PppoeWidgetPrivate;
class KNM_EXPORT PppoeWidget : public SettingWidget
{
Q_OBJECT
Q_DECLARE_PRIVATE(PppoeWidget)
public:
    PppoeWidget(Knm::Connection *, QWidget * parent = 0 );
    virtual ~PppoeWidget();
    QString settingName() const;
    void readConfig();
    void writeConfig();
    void readSecrets();
protected Q_SLOTS:
    void validate();
    void chkShowPassToggled();
};

#endif
