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

#ifndef GSMWIDGET_H
#define GSMWIDGET_H

#include "settingwidget.h"

#include "knm_export.h"

class GsmWidgetPrivate;
class KNM_EXPORT GsmWidget : public SettingWidget
{
Q_OBJECT
Q_DECLARE_PRIVATE(GsmWidget)
public:
    explicit GsmWidget(Knm::Connection *, QWidget * parent = 0 );
    virtual ~GsmWidget();
    QString settingName() const;
    void readConfig();
    void writeConfig();
    void readSecrets();
    void setApnInfo(const QMap<QString, QVariant> apnInfo);
    void setNetworkIds(const QList<QVariant> networkIds);
protected Q_SLOTS:
    void validate();
    void chkShowPassToggled();
    void passwordStorageTypeChanged(int);
    void pinStorageTypeChanged(int);
};

#endif
