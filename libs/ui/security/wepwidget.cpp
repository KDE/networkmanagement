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
#include <QValidator>

#include <KDebug>

#include "wepwidget.h"
#include "securitywidget_p.h"

#include <nm-setting-wireless-security.h>

//#include "802_11_wireless_security_widget.h"
//#include "secretstoragehelper.h"
#include "ui_wep.h"
#include "settings/802-11-wireless-security.h"
#include "connection.h"

static const int ASCII_MIN = ' ';
static const int ASCII_MAX = '~';

class WepWidgetPrivate : public SecurityWidgetPrivate
{
public:
    WepWidget::KeyFormat format;
    Ui_Wep ui;
    QStringList keys;
    int keyIndex;
    Knm::WirelessSecuritySetting * setting;
    QRegExpValidator * wepKeyValidator;
};

WepWidget::WepWidget(KeyFormat format, Knm::Connection * connection, QWidget * parent)
: SecurityWidget(*new WepWidgetPrivate, connection, parent)
{
    Q_D(WepWidget);
    d->format = format;
    d->keys << "" << "" << "" << "";
    d->keyIndex = 0;
    d->setting = static_cast<Knm::WirelessSecuritySetting *>(connection->setting(Knm::Setting::WirelessSecurity));
    QString hexRegExp = "([0-9]|[a-f]|[A-F]){10}|([0-9]|[a-f]|[A-F]){26}"; // 10 or 26 Hex digits
    QString asciiRegExp = "[-";
    for (char ch = ASCII_MIN; ch <= ASCII_MAX; ++ch) {
        if (ch != '-') {
            if (ch == '\\' || ch == ']') {
                asciiRegExp.append('\\');
            }
            asciiRegExp.append(ch);
        }
    }
    asciiRegExp.append("]");
    asciiRegExp = asciiRegExp + "{5}|" + asciiRegExp + "{13}";  // 5 or 13 ASCII characters
    QRegExp regExp(QString("^(%1|%2)$").arg(hexRegExp).arg(asciiRegExp));
    d->wepKeyValidator = new QRegExpValidator(regExp, this);

    d->ui.setupUi(this);
    d->ui.key->setEchoMode(QLineEdit::Password);

    connect(d->ui.keyType, SIGNAL(currentIndexChanged(int)), this, SLOT(keyTypeChanged(int)));

    if (d->format == WepWidget::Passphrase) {
        // currentIndex is already 0, calling d->ui.keyType->setCurrentIndex(0) has no effect.
        // so call the slot here.
        keyTypeChanged(0);
    } else {
        d->ui.keyType->setCurrentIndex(1);
    }

    connect(d->ui.weptxkeyindex, SIGNAL(currentIndexChanged(int)), this, SLOT(keyIndexChanged(int)));
    connect(d->ui.chkShowPass, SIGNAL(toggled(bool)), this, SLOT(chkShowPassToggled(bool)));
    // Validate the key when changed, which is used to enable/disable OK button
    connect(d->ui.key, SIGNAL(textChanged(QString)), this, SLOT(validateKey(QString)));
}

WepWidget::~WepWidget()
{
}

void WepWidget::keyTypeChanged(int index)
{
    Q_D(WepWidget);
    switch (index) {
        case 0: //passphrase
            d->ui.keyLabel->setText(i18n("&Passphrase:"));
            d->format = WepWidget::Passphrase;
            d->ui.key->setMaxLength(64);
            break;
        case 1: //hex/ascii key
            d->ui.keyLabel->setText(i18n("&Key:"));
            d->format = WepWidget::Hex;
            d->ui.key->setMaxLength(26);
            break;
    }
    // Ensure any existing key is validated too
    validateKey(d->ui.key->text());
}

void WepWidget::keyIndexChanged(int index)
{
    Q_D(WepWidget);
    d->keys.replace(d->keyIndex, d->ui.key->text());
    if (d->keys.count() > index) {
        d->ui.key->setText(d->keys[index]);
    }
    d->keyIndex = index;
}

void WepWidget::validateKey(QString key)
{
    Q_UNUSED(key);
    emit valid( validate() );
}

void WepWidget::chkShowPassToggled(bool on)
{
    Q_D(WepWidget);
    d->ui.key->setEchoMode(on ? QLineEdit::Normal : QLineEdit::Password);
}

bool WepWidget::validate() const
{
    Q_D(const WepWidget);
    if (d->ui.keyType->currentIndex() == 1) {   // Hex/ASCII key
        QString key = d->ui.key->text();
        int keyLength = key.length();
        return (d->wepKeyValidator->validate(key, keyLength) == QValidator::Acceptable);
    } else {  // Passphrase
        return (!d->ui.key->text().isEmpty() && d->ui.key->text().length() <= 64);
    }
}

void WepWidget::readConfig()
{
    Q_D(WepWidget);
    // tx index
    d->keyIndex = d->setting->weptxkeyindex();
    disconnect(d->ui.weptxkeyindex, SIGNAL(currentIndexChanged(int)), this, SLOT(keyIndexChanged(int)));
    d->ui.weptxkeyindex->setCurrentIndex(d->keyIndex <= 3 ? d->keyIndex : 0 );
    connect(d->ui.weptxkeyindex, SIGNAL(currentIndexChanged(int)), this, SLOT(keyIndexChanged(int)));

    switch (d->setting->wepKeyType()) {
        case Knm::WirelessSecuritySetting::Passphrase: d->ui.keyType->setCurrentIndex(0);
          break;
        default: d->ui.keyType->setCurrentIndex(1);
    }

    d->ui.chkShowPass->setChecked(false);

    // auth alg
    if (d->setting->authalg() == Knm::WirelessSecuritySetting::EnumAuthalg::shared) {
        d->ui.authalg->setCurrentIndex( 1 );
    } else {
        d->ui.authalg->setCurrentIndex( 0 );
    }
}

void WepWidget::writeConfig()
{
    Q_D(WepWidget);
    d->keys[d->ui.weptxkeyindex->currentIndex()] = d->ui.key->text();

    d->setting->setWeptxkeyindex(d->ui.weptxkeyindex->currentIndex());

    // keys
    d->setting->setWepkey0(d->keys[0]);
    d->setting->setWepkey1(d->keys[1]);
    d->setting->setWepkey2(d->keys[2]);
    d->setting->setWepkey3(d->keys[3]);
    if (!d->connection->permissions().isEmpty()) {
        d->setting->setWepkeyflags(Knm::Setting::AgentOwned);
    } else {
        d->setting->setWepkeyflags(Knm::Setting::None);
    }
    if (d->format == WepWidget::Passphrase) {
        d->setting->setWepKeyType(Knm::WirelessSecuritySetting::Passphrase);
    } else {
        d->setting->setWepKeyType(Knm::WirelessSecuritySetting::Hex);
    }

    if (d->ui.authalg->currentIndex() == 0 ) {
        d->setting->setAuthalg(Knm::WirelessSecuritySetting::EnumAuthalg::open);
    } else {
        d->setting->setAuthalg(Knm::WirelessSecuritySetting::EnumAuthalg::shared);
    }
}


void WepWidget::readSecrets()
{
    Q_D(WepWidget);
    // keys
    d->keys.replace(0, d->setting->wepkey0());
    d->keys.replace(1, d->setting->wepkey1());
    d->keys.replace(2, d->setting->wepkey2());
    d->keys.replace(3, d->setting->wepkey3());

    switch (d->setting->wepKeyType()) {
        case Knm::WirelessSecuritySetting::Passphrase: d->ui.keyType->setCurrentIndex(0);
          break;
        default: d->ui.keyType->setCurrentIndex(1);
    }

    d->ui.key->setText(d->keys.value(d->keyIndex));
}

// vim: sw=4 sts=4 et tw=100
