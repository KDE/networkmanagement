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
#include <kfiledialog.h>
#include <KUser>

#include "eapmethodstack.h"
#include "eapmethodsimple.h"
#include "eapmethodinnerauth_p.h"
#include "settings/802-1x.h"

TtlsWidget::TtlsWidget(Knm::Connection* connection, QWidget * parent)
: EapMethodInnerAuth(connection, parent)
{
    Q_D(EapMethodInnerAuth);
    setupUi(this);

    d->innerAuth->registerEapMethod(Knm::Security8021xSetting::EnumPhase2auth::pap,
            new EapMethodSimple(EapMethodSimple::Pap, connection, d->innerAuth),
            i18nc("PAP inner auth method", "PAP"));
    d->innerAuth->registerEapMethod(Knm::Security8021xSetting::EnumPhase2auth::mschap,
            new EapMethodSimple(EapMethodSimple::MsChap, connection, d->innerAuth),
            i18nc("MSCHAP inner auth method", "MSCHAP"));
    d->innerAuth->registerEapMethod(Knm::Security8021xSetting::EnumPhase2auth::mschapv2,
            new EapMethodSimple(EapMethodSimple::MsChapV2, connection, d->innerAuth),
            i18nc("MSCHAPv2 inner auth method", "MSCHAPv2"));
    d->innerAuth->registerEapMethod(Knm::Security8021xSetting::EnumPhase2auth::chap,
            new EapMethodSimple(EapMethodSimple::Chap, connection, d->innerAuth),
            i18nc("CHAP inner auth method", "CHAP"));
    gridLayout->addWidget(d->innerAuth, 3, 0, 2, 2);

    connect(chkUseSystemCaCerts,SIGNAL(toggled(bool)),this,SLOT(toggleSystemCa(bool)));
    connect(caCertLoad,SIGNAL(clicked()),this,SLOT(loadCert()));
    caCertLoad->setIcon(KIcon("document-open"));
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
    Q_D(EapMethodInnerAuth);
    leAnonIdentity->setText(d->setting->anonymousidentity());

    if (d->setting->useSystemCaCerts()) {
        chkUseSystemCaCerts->setChecked(true);
        caCertLoad->setEnabled(false);
    } else {
        chkUseSystemCaCerts->setChecked(false);
        if (!d->setting->cacert().isEmpty()) {
            setText(true);
        } else {
            setText(false);
        }
    }

    if (d->setting->phase2autheap() != Knm::Security8021xSetting::EnumPhase2autheap::none) {
        d->innerAuth->setCurrentEapMethod(d->setting->phase2autheap());
    } else {
        d->innerAuth->setCurrentEapMethod(d->setting->phase2auth());
    }
    d->innerAuth->readConfig();
}

void TtlsWidget::writeConfig()
{
    Q_D(EapMethodInnerAuth);
    // make the Setting TTLS
    d->setting->setEapFlags(Knm::Security8021xSetting::ttls);
    // TTLS specific config
    d->setting->setAnonymousidentity(leAnonIdentity->text());

    KUrl url;

    if (chkUseSystemCaCerts->isChecked()) {
        d->setting->setUseSystemCaCerts(true);
        d->setting->addToCertToDelete(Knm::Security8021xSetting::CACert);
    } else {
        d->setting->setUseSystemCaCerts(false);
    }

    d->innerAuth->writeConfig();
}

void TtlsWidget::readSecrets()
{
    Q_D(EapMethodInnerAuth);
    d->innerAuth->readSecrets();
}

void TtlsWidget::loadCert()
{
    Q_D(EapMethodInnerAuth);
    QString newcert = KFileDialog::getOpenFileName(KUser().homeDir(),"",this,i18nc("File chooser dialog title for certificate loading","Load Certificate"));
    if (!newcert.isEmpty()) {
        d->setting->setCacerttoimport(newcert);
        setText(true);
    }
}

void TtlsWidget::toggleSystemCa(bool toggled)
{
    Q_D(EapMethodInnerAuth);
    if (toggled)
        setText(false);
    else if (!d->setting->cacert().isEmpty() || !d->setting->cacerttoimport().isEmpty())
        setText(true);
}

void TtlsWidget::setText(bool loaded)
{
    if (loaded) {
        caCertLoad->setText(i18nc("Text to display on certificate button a certificate is already loaded","Load new"));
        caCertLoadedLabel->setText(i18nc("Text to display on CA certificate LED label when certificate is already loaded","Loaded"));
        caCertLed->setState(KLed::On);
    } else {
        caCertLoad->setText(i18nc("Text to display on CA certificate button when no certificate is loaded yet","Load"));
        caCertLoadedLabel->setText("");
        caCertLed->setState(KLed::Off);
    }
}


// vim: sw=4 sts=4 et tw=100
