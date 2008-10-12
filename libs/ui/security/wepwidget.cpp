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

#include "wepwidget.h"

#include <nm-setting-wireless-security.h>

#include "secretstoragehelper.h"
#include "ui_wep.h"

const QString WepWidget::AUTH_ALG_OPEN = QLatin1String("open");
const QString WepWidget::AUTH_ALG_SHARED = QLatin1String("shared");

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
    connect(d->ui.weptxkeyindex, SIGNAL(currentIndexChanged(int)), this, SLOT(keyIndexChanged(int)));
    connect(d->ui.chkShowPass, SIGNAL(toggled(bool)), this, SLOT(chkShowPassToggled(bool)));
}

WepWidget::~WepWidget()
{
    delete d;
}
#if 0
QString Wireless80211SecurityWidget::settingName() const
{
    return QLatin1String(NM_SETTING_WIRELESS_SECURITY_SETTING_NAME);
}
#endif

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
    d->ui.key->setEchoMode(on ? QLineEdit::Normal : QLineEdit::PasswordEchoOnEdit);
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
        QString fieldName = QString::fromLatin1("wepkey%1").arg(i);
        QString secret;
        secrets.readSecret(fieldName, secret);
        if (!secret.isEmpty()) {
            d->keys.replace(i, secret);
        }
    }
    d->ui.key->setText(d->keys.value(txKeyIndex));
    d->ui.chkShowPass->setChecked(false);
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
    cg.writeEntry("keymgmt", "none");
    cg.writeEntry("weptxkeyidx", d->ui.weptxkeyindex->currentIndex());
    // keys
    SecretStorageHelper secrets(m_connectionId, cg.name());

    for (int i = 0; i < d->keys.count(); i++) {
        QString fieldName = QString::fromLatin1("wepkey%1").arg(i);
        if (!d->keys[i].isEmpty()) {
            secrets.writeSecret(fieldName, d->keys[i]);
        }
    }
    QString authAlg;
    if (d->ui.authalg->currentIndex() == 0 ) {
        authAlg = AUTH_ALG_OPEN;
    } else {
        authAlg = AUTH_ALG_SHARED;
    }
    cg.writeEntry("authalg", authAlg);
}

#include "wepwidget.moc"
// vim: sw=4 sts=4 et tw=100
