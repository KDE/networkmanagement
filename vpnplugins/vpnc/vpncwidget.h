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

#ifndef VPNCWIDGET_H
#define VPNCWIDGET_H

#include "settingwidget.h"

#include "settings/vpn.h"

namespace Knm
{
    class Connection;
} // namespace Knm
class QComboBox;
class VpncSettingWidgetPrivate;

class VpncSettingWidget : public SettingWidget
{
Q_OBJECT
Q_DECLARE_PRIVATE(VpncSettingWidget)
public:
    VpncSettingWidget(Knm::Connection *, QWidget * parent = 0);
    ~VpncSettingWidget();
    void readConfig();
    void writeConfig();
    void readSecrets();
protected Q_SLOTS:
    void userPasswordTypeChanged(int);
    void groupPasswordTypeChanged(int);
    void natTraversalChanged(int);
    void validate();
    void showPasswordsChanged(bool);
private:
    void fillOnePasswordCombo(QComboBox * combo, Knm::Setting::secretsTypes type);
    uint handleOnePasswordType(const QComboBox * combo, const QString & key, QStringMap & data);
    VpncSettingWidgetPrivate * d_ptr;
};

#endif // VPNCWIDGET_H
