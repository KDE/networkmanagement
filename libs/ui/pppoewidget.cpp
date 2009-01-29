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

#include "connection.h"
#include "settings/pppoe.h"
#include "ui_pppoe.h"

class PppoeWidget::Private
{
public:
    Ui_Pppoe ui;
    Knm::PppoeSetting * setting;
};

PppoeWidget::PppoeWidget(Knm::Connection * connection, QWidget * parent)
: SettingWidget(connection, parent), d(new PppoeWidget::Private)
{
    d->ui.setupUi(this);
    d->setting = static_cast<Knm::PppoeSetting *>(connection->setting(Knm::Setting::Pppoe));
    connect(d->ui.chkShowPass, SIGNAL(stateChanged(int)), this, SLOT(chkShowPassToggled()));
    d->ui.password->setEchoMode(QLineEdit::Password);
}

PppoeWidget::~PppoeWidget()
{
    delete d;
}

void PppoeWidget::chkShowPassToggled()
{
    bool on = d->ui.chkShowPass->isChecked();
    d->ui.password->setEchoMode(on ? QLineEdit::Normal : QLineEdit::Password);
}

void PppoeWidget::readConfig()
{
    d->ui.service->setText(d->setting->service());
    d->ui.username->setText(d->setting->username());

}

void PppoeWidget::writeConfig()
{
    d->setting->setService(d->ui.service->text());
    d->setting->setUsername(d->ui.username->text());
    d->setting->setPassword(d->ui.password->text());
}

void PppoeWidget::readSecrets()
{
    d->ui.password->setText(d->setting->password());
}

// vim: sw=4 sts=4 et tw=100
