/*
    Copyright 2013 Jan Grulich <jgrulich@redhat.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KNM_OPENSWAN_WIDGET_H
#define KNM_OPENSWAN_WIDGET_H

#include "settingwidget.h"
#include "settings/vpn.h"

namespace Knm
{
    class Connection;
} // namespace Knm

namespace Ui
{
class OpenswanWidget;
}
class OpenswanWidgetPrivate;

class OpenswanWidget : public SettingWidget
{
Q_OBJECT
Q_DECLARE_PRIVATE(OpenswanWidget)
public:
    enum PasswordStorageType { Store = 0, AlwaysAsk, NotRequired };

    OpenswanWidget(Knm::Connection *, QWidget *parent = 0);
    ~OpenswanWidget();

    void readConfig();
    void readSecrets();
    void writeConfig();
protected Q_SLOTS:
    void validate() {};
private slots:
    void userPasswordTypeChanged(int index);
    void groupPasswordTypeChanged(int index);
    void showPassword(bool show);

private:
    OpenswanWidgetPrivate *d_ptr;
};

#endif // KNM_OPENSWAN_WIDGET_H
