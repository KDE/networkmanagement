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

#include <QWidget>

#include <KDebug>

#include "wepwidget.h"

#include <nm-setting-wireless-security.h>

#include "802_11_wireless_security_widget.h"
#include "secretstoragehelper.h"
#include "ui_wep.h"

class WepWidget::Private
{
public:

    WepWidget::KeyFormat format;
    Ui_Wep ui;
    QStringList keys;
    int keyIndex;
    KConfig * config;
};

WepWidget::WepWidget(KeyFormat format, KConfig * config, const QString & connectionId, QWidget * parent)
: SecurityWidget(connectionId, parent), d(new WepWidget::Private)
{
    d->format = format;
    d->keys << "" << "" << "" << "";
    d->keyIndex = 0;
    d->config = config;
    d->ui.setupUi(this);
    d->ui.passphrase->setEchoMode(QLineEdit::Password);
    d->ui.key->setEchoMode(QLineEdit::Password);
    keyTypeChanged(0);//initialize for passphrase

    connect(d->ui.keyType, SIGNAL(currentIndexChanged(int)), this, SLOT(keyTypeChanged(int)));
    connect(d->ui.weptxkeyindex, SIGNAL(currentIndexChanged(int)), this, SLOT(keyIndexChanged(int)));
    connect(d->ui.chkShowPass, SIGNAL(toggled(bool)), this, SLOT(chkShowPassToggled(bool)));
}

WepWidget::~WepWidget()
{
    delete d;
}

void WepWidget::keyTypeChanged(int type)
{
    switch (type) {
        case 0: //passphrase
            d->ui.passphraseLabel->show();
            d->ui.passphrase->show();
            d->ui.keyLabel->hide();
            d->ui.key->hide();
            break;
        case 1: //hex key
            d->ui.passphraseLabel->hide();
            d->ui.passphrase->hide();
            d->ui.keyLabel->show();
            d->ui.key->show();
            break;
    }
}

void WepWidget::keyIndexChanged(int index)
{
    d->keys.replace(d->keyIndex, d->ui.key->text());
    if (d->keys.count() > index) {
        d->ui.key->setText(d->keys[index]);
    }
    d->keyIndex = index;
}

void WepWidget::chkShowPassToggled(bool on)
{
    d->ui.passphrase->setEchoMode(on ? QLineEdit::Normal : QLineEdit::Password);
    d->ui.key->setEchoMode(on ? QLineEdit::Normal : QLineEdit::Password);
}

bool WepWidget::validate() const
{
    return true;
}

void WepWidget::readConfig()
{
    KConfigGroup cg(d->config, NM_SETTING_WIRELESS_SECURITY_SETTING_NAME);
    // tx index
    uint txKeyIndex = cg.readEntry("weptxkeyidx", 0);
    d->keyIndex = txKeyIndex;
    disconnect(d->ui.weptxkeyindex, SIGNAL(currentIndexChanged(int)), this, SLOT(keyIndexChanged(int)));
    d->ui.weptxkeyindex->setCurrentIndex(txKeyIndex < 3 ? txKeyIndex : 0 );
    connect(d->ui.weptxkeyindex, SIGNAL(currentIndexChanged(int)), this, SLOT(keyIndexChanged(int)));
    // keys
    SecretStorageHelper secrets(m_connectionId, cg.name());
    for (int i = 0; i < 4; i++) {
        QString fieldName = QString::fromLatin1("wep-key%1").arg(i);
        QString secret;
        secrets.readSecret(fieldName, secret);
        if (!secret.isEmpty()) {
            d->keys.replace(i, secret);
        }
    }
    d->ui.key->setText(d->keys.value(txKeyIndex));
    d->ui.chkShowPass->setChecked(false);

    //passphrase
    QString passphrase;
    secrets.readSecret("wep-passphrase", passphrase);
    if (!passphrase.isEmpty()) {
        d->ui.passphrase->setText(passphrase);
    }

    // auth alg
    QString authAlg = cg.readEntry("authalg", "open");
    if (authAlg == QLatin1String("shared")) {
        d->ui.authalg->setCurrentIndex( 1 );
    } else {
        d->ui.authalg->setCurrentIndex( 0 );
    }
}

void WepWidget::writeConfig()
{
    d->keys.insert(d->ui.weptxkeyindex->currentIndex(), d->ui.key->text());

    KConfigGroup cg(d->config, NM_SETTING_WIRELESS_SECURITY_SETTING_NAME);
    cg.writeEntry("keymgmt", Wireless80211SecurityWidget::KEY_MGMT_NONE);
    cg.writeEntry("weptxkeyidx", d->ui.weptxkeyindex->currentIndex());
    // keys
    SecretStorageHelper secrets(m_connectionId, cg.name());

    for (int i = 0; i < d->keys.count(); i++) {
        QString fieldName = QString::fromLatin1("wep-key%1").arg(i);
        if (!d->keys[i].isEmpty()) {
            secrets.writeSecret(fieldName, d->keys[i]);
        }
    }
    QString passphrase = d->ui.passphrase->text();
    secrets.writeSecret("wep-passphrase", passphrase);
    QString authAlg;
    if (d->ui.authalg->currentIndex() == 0 ) {
        authAlg = AUTH_ALG_OPEN;
    } else {
        authAlg = AUTH_ALG_SHARED;
    }
    cg.writeEntry("authalg", authAlg);
}

QVariantMap WepWidget::secrets() const
{
    QVariantMap ourSecrets;
    d->keys.insert(d->ui.weptxkeyindex->currentIndex(), d->ui.key->text());

    for (int i = 0; i < d->keys.count(); i++) {
        QString fieldName = QString::fromLatin1("wep-key%1").arg(i);
        if (!d->keys[i].isEmpty()) {
            ourSecrets.insert(fieldName, QVariant(d->keys[i]));
        }
    }
    kDebug() << ourSecrets;
    return ourSecrets;
}

#include "wepwidget.moc"
// vim: sw=4 sts=4 et tw=100
