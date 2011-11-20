/*
Copyright 2009 Will Stephenson <wstephenson@kde.org>
Copyright 2010 Maurus Rohrer <maurus.rohrer@gmail.com>

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

#ifndef STRONGSWANWIDGET_H
#define STRONGSWANWIDGET_H

#include "settingwidget.h"

#include "ui_strongswanprop.h"
#include "settings/vpn.h"


namespace Knm
{
    class Connection;
} // namespace Knm

class StrongswanSettingWidgetPrivate;

class StrongswanSettingWidget : public SettingWidget
{
Q_OBJECT
Q_DECLARE_PRIVATE(StrongswanSettingWidget)
public:
    StrongswanSettingWidget(Knm::Connection *, QWidget * parent = 0);
    ~StrongswanSettingWidget();
    void readConfig();
    void writeConfig();
    void readSecrets();
protected Q_SLOTS:
    void userPasswordTypeChanged(int);
    void privateKeyPasswordTypeChanged(int);
    void pinTypeChanged(int);
    void methodChanged(int);
    void validate();
    void showPasswordsChanged(bool show);
private:
    void fillOnePasswordCombo(QComboBox * combo, const QString & key, const QStringMap & data, bool hasPassword);
    uint handleOnePasswordType(const QComboBox * combo, const QString & key, QStringMap & data);
    StrongswanSettingWidgetPrivate * d_ptr;
};

#endif // STRONGSWANWIDGET_H
