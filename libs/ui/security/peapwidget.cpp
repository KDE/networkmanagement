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

#include "peapwidget.h"
#include "ui_security_peap.h"
#include "secretstoragehelper.h"

class PeapWidget::Private
{
public:
    Ui_Peap ui;
    KConfig* config;
};

PeapWidget::PeapWidget(KConfig* config, const QString & connectionId, QWidget * parent)
: EapWidget(connectionId, parent), d(new PeapWidget::Private)
{
    d->ui.setupUi(this);
    d->config = config;

    d->ui.cacert->setMode(KFile::LocalOnly);

    chkShowPassToggled(false);
    connect(d->ui.chkShowPassword, SIGNAL(toggled(bool)), this, SLOT(chkShowPassToggled(bool)));
}

PeapWidget::~PeapWidget()
{
}

void PeapWidget::chkShowPassToggled(bool on)
{
    d->ui.password->setEchoMode(on ? QLineEdit::Normal : QLineEdit::Password);
}

bool PeapWidget::validate() const
{
    return true;
}

void PeapWidget::readConfig()
{
    KConfigGroup cg(d->config, NM_SETTING_802_1X_SETTING_NAME);

    QString identity;
    identity = cg.readEntry("identity");
    if (!identity.isEmpty())
        d->ui.identity->setText(identity);

    QString anonymousidentity = cg.readEntry("anonymousidentity");
    if (!anonymousidentity.isEmpty())
        d->ui.anonymousidentity->setText(anonymousidentity);

    QString capath = cg.readEntry("capath");
    if (!capath.isEmpty())
        d->ui.cacert->setUrl(capath);

    QString phase2autheap = cg.readEntry("phase2autheap", "pap");
    if (phase2autheap == "pap")
        d->ui.phase2autheap->setCurrentIndex(0);
    else if (phase2autheap == "mschap")
        d->ui.phase2autheap->setCurrentIndex(1);
    else if (phase2autheap == "mschapv2")
        d->ui.phase2autheap->setCurrentIndex(2);
    else if (phase2autheap == "chap")
        d->ui.phase2autheap->setCurrentIndex(3);

    // secrets
    SecretStorageHelper secrets(m_connectionId, cg.name());

    QString password;
    secrets.readSecret("password", password);
    if (!password.isEmpty())
        d->ui.password->setText(password);
}

void PeapWidget::writeConfig()
{
    KConfigGroup cg(d->config, NM_SETTING_802_1X_SETTING_NAME);

    cg.writeEntry("identity", d->ui.identity->text());
    cg.writeEntry("anonymousidentity", d->ui.anonymousidentity->text());
    cg.writeEntry("capath", d->ui.cacert->url().directory() + "/" + d->ui.cacert->url().fileName());

    switch(d->ui.phase2autheap->currentIndex())
    {
        case 0:
            cg.writeEntry("phase2autheap", "pap");
            break;
        case 1:
            cg.writeEntry("phase2autheap", "mschap");
            break;
        case 2:
            cg.writeEntry("phase2autheap", "mschapv2");
            break;
        case 3:
            cg.writeEntry("phase2autheap", "chap");
            break;
    }

    // secrets
    SecretStorageHelper secrets(m_connectionId, cg.name());

    secrets.writeSecret("password", d->ui.password->text());
}

QVariantMap PeapWidget::secrets() const
{
    QVariantMap ourSecrets;
    ourSecrets.insert("password", d->ui.password->text());
    return ourSecrets;
}
// vim: sw=4 sts=4 et tw=100
