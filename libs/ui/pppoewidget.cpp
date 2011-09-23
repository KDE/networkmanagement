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

#include "pppoewidget.h"
#include "settingwidget_p.h"

#include "connection.h"
#include "settings/pppoe.h"
#include "ui_pppoe.h"

class PppoeWidgetPrivate : public SettingWidgetPrivate
{
public:
    Ui_Pppoe ui;
    Knm::PppoeSetting * setting;
};

PppoeWidget::PppoeWidget(Knm::Connection * connection, QWidget * parent)
: SettingWidget(*new PppoeWidgetPrivate, connection, parent)
{
    Q_D(PppoeWidget);
    d->ui.setupUi(this);
    d->setting = static_cast<Knm::PppoeSetting *>(connection->setting(Knm::Setting::Pppoe));
    connect(d->ui.chkShowPass, SIGNAL(stateChanged(int)), this, SLOT(chkShowPassToggled()));
    d->ui.password->setEchoMode(QLineEdit::Password);
}

PppoeWidget::~PppoeWidget()
{
}

void PppoeWidget::chkShowPassToggled()
{
    Q_D(PppoeWidget);
    bool on = d->ui.chkShowPass->isChecked();
    d->ui.password->setEchoMode(on ? QLineEdit::Normal : QLineEdit::Password);
}

void PppoeWidget::readConfig()
{
    Q_D(PppoeWidget);
    d->ui.service->setText(d->setting->service());
    d->ui.username->setText(d->setting->username());
}

void PppoeWidget::writeConfig()
{
    Q_D(PppoeWidget);
    d->setting->setService(d->ui.service->text());
    d->setting->setUsername(d->ui.username->text());
    d->setting->setPassword(d->ui.password->text());
    if (!d->connection->permissions().isEmpty()) {
        d->setting->setPasswordflags(Knm::Setting::AgentOwned);
    } else {
        d->setting->setPasswordflags(Knm::Setting::None);
    }
}

void PppoeWidget::readSecrets()
{
    Q_D(PppoeWidget);
    d->ui.password->setText(d->setting->password());
}

void PppoeWidget::validate()
{

}
// vim: sw=4 sts=4 et tw=100
