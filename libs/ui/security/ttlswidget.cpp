/*
Copyright 2008 Helmut Schaa <helmut.schaa@googlemail.com>

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

#include <nm-setting-8021x.h>

#include "ttlswidget.h"
#include "ui_security_ttls.h"
#include "secretstoragehelper.h"
#include "settings/802-1x.h"
#include "connection.h"

class TtlsWidget::Private
{
public:
    Ui_Ttls ui;
    Knm::Security8021xSetting* setting;
};

TtlsWidget::TtlsWidget(Knm::Connection* connection, QWidget * parent)
: EapWidget(connection, parent), d(new TtlsWidget::Private)
{
    d->ui.setupUi(this);
    d->setting = static_cast<Knm::Security8021xSetting *>(connection->setting(Knm::Setting::Security8021x));

    chkShowPassToggled(false);
    connect(d->ui.chkShowPassword, SIGNAL(toggled(bool)), this, SLOT(chkShowPassToggled(bool)));
}

TtlsWidget::~TtlsWidget()
{
}

void TtlsWidget::chkShowPassToggled(bool on)
{
    d->ui.password->setEchoMode(on ? QLineEdit::Normal : QLineEdit::Password);
}

bool TtlsWidget::validate() const
{
    return true;
}

void TtlsWidget::readConfig()
{
    QString identity;
    identity = d->setting->identity();
    if (!identity.isEmpty())
        d->ui.identity->setText(identity);

    QString anonymousidentity = d->setting->anonymousidentity();
    if (!anonymousidentity.isEmpty())
        d->ui.anonidentity->setText(anonymousidentity);

    QString capath = d->setting->capath();
    if (!capath.isEmpty())
        d->ui.cacert->setUrl(capath);

    int phase2autheap = d->setting->phase2autheap();
    if (phase2autheap == Knm::Security8021xSetting::EnumPhase2autheap::pap)
        d->ui.phase2autheap->setCurrentIndex(0);
    else if (phase2autheap == Knm::Security8021xSetting::EnumPhase2autheap::mschap)
        d->ui.phase2autheap->setCurrentIndex(1);
    else if (phase2autheap == Knm::Security8021xSetting::EnumPhase2autheap::mschapv2)
        d->ui.phase2autheap->setCurrentIndex(2);
    else if (phase2autheap == Knm::Security8021xSetting::EnumPhase2autheap::chap)
        d->ui.phase2autheap->setCurrentIndex(3);

    QString password = d->setting->password();
    if (!password.isEmpty())
        d->ui.password->setText(password);
}

void TtlsWidget::writeConfig()
{
    d->setting->setIdentity(d->ui.identity->text());
    d->setting->setAnonymousidentity(d->ui.anonidentity->text());
    if (!d->ui.cacert->url().directory().isEmpty() && !d->ui.cacert->url().fileName().isEmpty())
        d->setting->setCapath(d->ui.cacert->url().directory() + "/" + d->ui.cacert->url().fileName());

    switch(d->ui.phase2autheap->currentIndex())
    {
        case 0:
            d->setting->setPhase2autheap(Knm::Security8021xSetting::EnumPhase2autheap::pap);
            break;
        case 1:
            d->setting->setPhase2autheap(Knm::Security8021xSetting::EnumPhase2autheap::mschap);
            break;
        case 2:
            d->setting->setPhase2autheap(Knm::Security8021xSetting::EnumPhase2autheap::mschapv2);
            break;
        case 3:
            d->setting->setPhase2autheap(Knm::Security8021xSetting::EnumPhase2autheap::chap);
            break;
    }

    d->setting->setPassword(d->ui.password->text());
}

// vim: sw=4 sts=4 et tw=100
