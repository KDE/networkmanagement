/*
Copyright 2011 Lamarque Souza <lamarque@gmail.com>

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

#ifndef BLUETOOTHWIDGET_H
#define BLUETOOTHWIDGET_H

#include "settingwidget.h"

#include "knm_export.h"

class BluetoothWidgetPrivate;
class KNM_EXPORT BluetoothWidget : public SettingWidget
{
Q_OBJECT
Q_DECLARE_PRIVATE(BluetoothWidget)
public:
    BluetoothWidget(Knm::Connection *, QWidget * parent = 0 );
    virtual ~BluetoothWidget();
    QString settingName() const;
    void readConfig();
    void writeConfig();
    //void readSecrets();
protected Q_SLOTS:
    void validate();
protected:
    enum NetworktypeIndex { DunIndex = 0, PanuIndex };

    NetworktypeIndex networktypeIndexFromString(const QString &networktype) const;
};

#endif
