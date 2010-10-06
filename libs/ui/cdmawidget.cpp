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

#include "cdmawidget.h"
#include "settingwidget_p.h"

#include "connection.h"
#include "settings/cdma.h"
#include "ui_cdma.h"

class CdmaWidgetPrivate : public SettingWidgetPrivate
{
public:
    Ui_Cdma ui;
    Knm::CdmaSetting * setting;
};

CdmaWidget::CdmaWidget(Knm::Connection * connection, QWidget * parent)
: SettingWidget(*new CdmaWidgetPrivate, connection, parent)
{
    Q_D(CdmaWidget);
    d->ui.setupUi(this);
    d->setting = static_cast<Knm::CdmaSetting *>(connection->setting(Knm::Setting::Cdma));
    connect(d->ui.chkShowPass, SIGNAL(stateChanged(int)), this, SLOT(chkShowPassToggled()));
    d->ui.password->setEchoMode(QLineEdit::Password);
}

CdmaWidget::~CdmaWidget()
{
}

void CdmaWidget::chkShowPassToggled()
{
    Q_D(CdmaWidget);
    bool on = d->ui.chkShowPass->isChecked();
    d->ui.password->setEchoMode(on ? QLineEdit::Normal : QLineEdit::Password);
}

void CdmaWidget::readConfig()
{
    Q_D(CdmaWidget);
    d->ui.number->setText(d->setting->number());
    d->ui.username->setText(d->setting->username());
}

void CdmaWidget::writeConfig()
{
    Q_D(CdmaWidget);
    d->setting->setNumber(d->ui.number->text());
    d->setting->setUsername(d->ui.username->text());
    d->setting->setPassword(d->ui.password->text());
}

void CdmaWidget::readSecrets()
{
    Q_D(CdmaWidget);
    d->ui.password->setText(d->setting->password());
}

void CdmaWidget::setCdmaInfo(const QVariantMap info)
{
    Q_D(CdmaWidget);
    d->setting->setNumber(info["number"].toString());

    if (!info["username"].isNull()) {
        d->setting->setUsername(info["username"].toString());
    }
    if (!info["password"].isNull()) {
        d->setting->setPassword(info["password"].toString());
    }

    // TODO: save the sids.
    readConfig();
}

void CdmaWidget::validate()
{
}

// vim: sw=4 sts=4 et tw=100
