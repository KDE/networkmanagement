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

#include "pppwidget.h"
#include "settingwidget_p.h"

#include "ui_ppp.h"
#include "connection.h"
#include "settings/ppp.h"

#define LCP_ECHO_FAILURE 5
#define LCP_ECHO_INTERVAL 30

class PppWidgetPrivate : public SettingWidgetPrivate
{
public:
    Ui_Ppp ui;
    Knm::PppSetting * setting;
};

PppWidget::PppWidget(Knm::Connection * connection, QWidget * parent)
: SettingWidget(*new PppWidgetPrivate, connection, parent)
{
    Q_D(PppWidget);
    d->ui.setupUi(this);
    d->setting = static_cast<Knm::PppSetting *>(connection->setting(Knm::Setting::Ppp));
}

PppWidget::~PppWidget()
{
}

void PppWidget::readConfig()
{
    Q_D(PppWidget);
    d->ui.auth->setChecked(!d->setting->noauth());
    d->ui.pap->setChecked(!d->setting->refusepap());
    d->ui.chap->setChecked(!d->setting->refusechap());
    d->ui.mschap->setChecked(!d->setting->refusemschap());
    d->ui.mschapv2->setChecked(!d->setting->refusemschapv2());
    d->ui.eap->setChecked(!d->setting->refuseeap());
    d->ui.requiremppe->setChecked(d->setting->requiremppe());
    d->ui.requiremppe128->setChecked(d->setting->requiremppe128());
    d->ui.mppestateful->setChecked(d->setting->mppestateful());
    d->ui.bsdcomp->setChecked(!d->setting->nobsdcomp());
    d->ui.deflate->setChecked(!d->setting->nodeflate());
    d->ui.vjcomp->setChecked(!d->setting->novjcomp());

    d->ui.pppecho->setChecked(d->setting->lcpechointerval() != 0);
}

void PppWidget::writeConfig()
{
    Q_D(PppWidget);
    d->setting->setNoauth(!d->ui.auth->isChecked());
    d->setting->setRefusepap(!d->ui.pap->isChecked());
    d->setting->setRefusechap(!d->ui.chap->isChecked());
    d->setting->setRefusemschap(!d->ui.mschap->isChecked());
    d->setting->setRefusemschapv2(!d->ui.mschapv2->isChecked());
    d->setting->setRefuseeap(!d->ui.eap->isChecked());
    d->setting->setRequiremppe(d->ui.requiremppe->isChecked());
    d->setting->setRequiremppe128(d->ui.requiremppe128->isChecked());
    d->setting->setMppestateful(d->ui.mppestateful->isChecked());
    d->setting->setNobsdcomp(!d->ui.bsdcomp->isChecked());
    d->setting->setNodeflate(!d->ui.deflate->isChecked());
    d->setting->setNovjcomp(!d->ui.vjcomp->isChecked());
    if (d->ui.pppecho->isChecked()) {
        d->setting->setLcpechofailure(LCP_ECHO_FAILURE);
        d->setting->setLcpechointerval(LCP_ECHO_INTERVAL);
    } else {
        d->setting->setLcpechofailure(0);
        d->setting->setLcpechointerval(0);
    }
}

void PppWidget::validate()
{

}

// vim: sw=4 sts=4 et tw=100
