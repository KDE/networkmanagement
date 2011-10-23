/*
Copyright 2011 Ilia Kats <ilia-kats@gmx.net>

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

#ifndef SECURITY8021XWIDGET_H
#define SECURITY8021XWIDGET_H

#include "settingwidget.h"

namespace Knm
{
    class Connection;
} // namespace Knm
class Security8021xAuthWidgetPrivate;

class KNM_EXPORT Security8021xAuthWidget : public SettingWidget
{
Q_OBJECT
Q_DECLARE_PRIVATE(Security8021xAuthWidget)
public:
    Security8021xAuthWidget(Knm::Connection* connection, QWidget * parent = 0 );
    Security8021xAuthWidget(Knm::Connection* connection, const QStringList &secrets, QWidget * parent = 0 );
    ~Security8021xAuthWidget();
    void readConfig(){}
    void writeConfig();
    void readSecrets(){}
protected Q_SLOTS:
    void validate() {}
private Q_SLOTS:
    void showPasswordsToggled(bool);
};

#endif
