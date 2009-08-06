/*
Copyright 2008 Helmut Schaa <helmut.schaa@googlemail.com>
Copyright 2009 Will Stephenson <wstephenson@kde.org>

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

#include "ttlswidget.h"

#include <nm-setting-8021x.h>

#include <connection.h>

#include "eapmethodstack.h"
#include "eapmethodsimple.h"
#include "eapmethodinnerauth_p.h"

TtlsWidget::TtlsWidget(Knm::Connection* connection, QWidget * parent)
: EapMethodInnerAuth(connection, parent)
{
    Q_D(EapMethodInnerAuth);
    setupUi(this);

    int dummyIndex;
    d->innerAuth->registerEapMethod(new EapMethodSimple(EapMethodSimple::Pap, connection, d->innerAuth),
            i18nc("PAP inner auth method", "PAP"), dummyIndex);
    d->innerAuth->registerEapMethod(new EapMethodSimple(EapMethodSimple::MsChap, connection, d->innerAuth),
            i18nc("MSCHAP inner auth method", "MSCHAP"), dummyIndex);
    d->innerAuth->registerEapMethod(new EapMethodSimple(EapMethodSimple::MsChapV2, connection, d->innerAuth),
            i18nc("MSCHAPv2 inner auth method", "MSCHAPv2"), dummyIndex);
    d->innerAuth->registerEapMethod(new EapMethodSimple(EapMethodSimple::Chap, connection, d->innerAuth),
            i18nc("CHAP inner auth method", "CHAP"), dummyIndex);
    d->innerAuth->registerEapMethod(new EapMethodSimple(EapMethodSimple::Pap, connection, d->innerAuth),
            i18nc("PAP inner auth method", "PAP"), dummyIndex);
    gridLayout->addWidget(d->innerAuth, 2, 0, 2, 2);
    // add eepmethodstack for inner auth
}

TtlsWidget::~TtlsWidget()
{
}

bool TtlsWidget::validate() const
{
    return true;
}

void TtlsWidget::readConfig()
{
#if 0
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
#endif
}

void TtlsWidget::writeConfig()
{
#if 0
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
#endif
}

void TtlsWidget::readSecrets()
{
#if 0
    QString password = d->setting->password();
    if (!password.isEmpty())
        d->ui.password->setText(password);
#endif
}


// vim: sw=4 sts=4 et tw=100
