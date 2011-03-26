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

#include "gsmwidget.h"
#include "settingwidget_p.h"
#include "ui_gsm.h"
#include "connection.h"
#include "settings/gsm.h"

class GsmWidgetPrivate : public SettingWidgetPrivate
{
public:
    Ui_Gsm ui;
    Knm::GsmSetting * setting;
};

GsmWidget::GsmWidget(Knm::Connection * connection, QWidget * parent)
: SettingWidget(*new GsmWidgetPrivate, connection, parent)
{
    Q_D(GsmWidget);
    d->ui.setupUi(this);
    d->setting = static_cast<Knm::GsmSetting *>(connection->setting(Knm::Setting::Gsm));
    connect(d->ui.chkShowPass, SIGNAL(stateChanged(int)), this, SLOT(chkShowPassToggled()));
    d->ui.pin->setEchoMode(QLineEdit::Password);
    d->ui.puk->setEchoMode(QLineEdit::Password);
    /* Not used yet*/
    d->ui.lblBand->setVisible(false);
    d->ui.band->setVisible(false);
    d->ui.lblNetwork->setVisible(false);
    d->ui.network->setVisible(false);
}

GsmWidget::~GsmWidget()
{
}

void GsmWidget::readConfig()
{
    Q_D(GsmWidget);
    d->ui.number->setText(d->setting->number());
    d->ui.username->setText(d->setting->username());
    d->ui.apn->setText(d->setting->apn());
    d->ui.network->setText(d->setting->networkid());
    d->ui.type->setCurrentIndex(qBound(0, d->setting->networktype() + 1, d->ui.type->count() - 1));
    d->ui.band->setValue(d->setting->band());
    d->ui.password->setEchoMode(QLineEdit::Password);
}

void GsmWidget::chkShowPassToggled()
{
    Q_D(GsmWidget);
    bool on = d->ui.chkShowPass->isChecked();
    d->ui.password->setEchoMode(on ? QLineEdit::Normal : QLineEdit::Password);
    d->ui.pin->setEchoMode(on ? QLineEdit::Normal : QLineEdit::Password);
    d->ui.puk->setEchoMode(on ? QLineEdit::Normal : QLineEdit::Password);
}


void GsmWidget::writeConfig()
{
    Q_D(GsmWidget);
    d->setting->setNumber(d->ui.number->text());
    d->setting->setUsername(d->ui.username->text());
    d->setting->setPassword(d->ui.password->text());
    d->setting->setApn(d->ui.apn->text());
    d->setting->setNetworkid(d->ui.network->text());
    d->setting->setNetworktype(d->ui.type->currentIndex() - 1);
    d->setting->setBand(d->ui.band->value());
    d->setting->setPin(d->ui.pin->text());
    d->setting->setPuk(d->ui.puk->text());
}

void GsmWidget::readSecrets()
{
    Q_D(GsmWidget);
    d->ui.password->setText(d->setting->password());
    d->ui.pin->setText(d->setting->pin());
    d->ui.puk->setText(d->setting->puk());
}

void GsmWidget::validate()
{

}

void GsmWidget::setNetworkIds(const QList<QVariant> networkIds)
{
    //Q_D(GsmWidget);

    QString temp;

    foreach (const QVariant &v, networkIds) {
        temp.append(v.toString() + ",");
    }
    temp.remove(temp.size()-1, 1);

    // TODO: nm-applet-0.8 does not save network-id list, which probably means
    // NetworkManager does not use this list yet.
    //d->setting->setNetworkid(temp);
    readConfig();
}

void GsmWidget::setApnInfo(const QMap<QString, QVariant> apnInfo)
{
    Q_D(GsmWidget);

    d->setting->setNumber(apnInfo["number"].toString());

    if (!apnInfo["username"].isNull()) {
        d->setting->setUsername(apnInfo["username"].toString());
    }
    if (!apnInfo["password"].isNull()) {
        d->setting->setPassword(apnInfo["password"].toString());
    }

    d->setting->setApn(apnInfo["apn"].toString());

    readConfig();
    d->ui.password->setText(d->setting->password());
}

// vim: sw=4 sts=4 et tw=100
