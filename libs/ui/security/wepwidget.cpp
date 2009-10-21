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

#include <nm-setting-wireless-security.h>

//#include "802_11_wireless_security_widget.h"
//#include "secretstoragehelper.h"
#include "ui_wep.h"
#include "settings/802-11-wireless-security.h"
#include "connection.h"

static const int ASCII_MIN = ' ';
static const int ASCII_MAX = '~';

class WepWidget::Private
{
public:
    WepWidget::KeyFormat format;
    Ui_Wep ui;
    QStringList keys;
    int keyIndex;
    Knm::WirelessSecuritySetting * setting;
    QRegExpValidator * hexKeyValidator;
};

WepWidget::WepWidget(KeyFormat format, Knm::Connection * connection, QWidget * parent)
: SecurityWidget(connection, parent), d(new WepWidget::Private)
{
    d->format = format;
    d->keys << "" << "" << "" << "";
    d->keyIndex = 0;
    d->setting = static_cast<Knm::WirelessSecuritySetting *>(connection->setting(Knm::Setting::WirelessSecurity));
    QString hexRegExp = "([0-9]|[a-f]|[A-F]){10,26}";
    QString asciiRegExp = "[-";
    for (char ch = ASCII_MIN; ch <= ASCII_MAX; ++ch) {
        if (ch != '-') {
            if (ch == '\\' || ch == ']') {
                asciiRegExp.append('\\');
            }
            asciiRegExp.append(ch);
        }
    }
    asciiRegExp.append("]{5,13}");
    QRegExp regExp(QString("^(%1|%2)$").arg(hexRegExp).arg(asciiRegExp));
    d->hexKeyValidator = new QRegExpValidator(regExp, this);

    d->ui.setupUi(this);
    d->ui.passphrase->setEchoMode(QLineEdit::Password);
    d->ui.key->setEchoMode(QLineEdit::Password);
    d->ui.key->setValidator(d->hexKeyValidator);

    connect(d->ui.keyType, SIGNAL(currentIndexChanged(int)), this, SLOT(keyTypeChanged(int)));

    if (d->format == WepWidget::Passphrase) {
        keyTypeChanged(0);
    } else {
        keyTypeChanged(1);
    }

    connect(d->ui.weptxkeyindex, SIGNAL(currentIndexChanged(int)), this, SLOT(keyIndexChanged(int)));
    connect(d->ui.chkShowPass, SIGNAL(toggled(bool)), this, SLOT(chkShowPassToggled(bool)));
}

WepWidget::~WepWidget()
{
    delete d;
}

void WepWidget::keyTypeChanged(int index)
{
    switch (index) {
        case 0: //passphrase
            d->ui.passphraseLabel->show();
            d->ui.passphrase->show();
            d->ui.keyLabel->hide();
            d->ui.key->hide();
            d->format = WepWidget::Passphrase;
            break;
        case 1: //hex key
            d->ui.passphraseLabel->hide();
            d->ui.passphrase->hide();
            d->ui.keyLabel->show();
            d->ui.key->show();
            d->format = WepWidget::Hex;
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
    if (d->ui.keyType->currentIndex() == 1) {
        return d->ui.key->hasAcceptableInput();
    }
    else {
        return true;
    }
}

void WepWidget::readConfig()
{
    // tx index
    d->keyIndex = d->setting->weptxkeyindex();
    disconnect(d->ui.weptxkeyindex, SIGNAL(currentIndexChanged(int)), this, SLOT(keyIndexChanged(int)));
    d->ui.weptxkeyindex->setCurrentIndex(d->keyIndex <= 3 ? d->keyIndex : 0 );
    connect(d->ui.weptxkeyindex, SIGNAL(currentIndexChanged(int)), this, SLOT(keyIndexChanged(int)));

    d->ui.chkShowPass->setChecked(false);

    // auth alg
    if (d->setting->authalg()  == Knm::WirelessSecuritySetting::EnumAuthalg::shared) {
        d->ui.authalg->setCurrentIndex( 1 );
    } else {
        d->ui.authalg->setCurrentIndex( 0 );
    }
}

void WepWidget::writeConfig()
{
    d->keys.insert(d->ui.weptxkeyindex->currentIndex(), d->ui.key->text());

    d->setting->setWeptxkeyindex(d->ui.weptxkeyindex->currentIndex());

    // keys
    if (d->format == WepWidget::Passphrase)
    {
        QString passphrase = d->ui.passphrase->text();
        d->setting->setWeppassphrase(passphrase);
        d->setting->setWepkey0(QString());
        d->setting->setWepkey1(QString());
        d->setting->setWepkey2(QString());
        d->setting->setWepkey3(QString());
    } else {
        d->setting->setWeppassphrase(QString());
        d->setting->setWepkey0(d->keys[0]);
        d->setting->setWepkey1(d->keys[1]);
        d->setting->setWepkey2(d->keys[2]);
        d->setting->setWepkey3(d->keys[3]);
    }

    QString authAlg;
    if (d->ui.authalg->currentIndex() == 0 ) {
        d->setting->setAuthalg(Knm::WirelessSecuritySetting::EnumAuthalg::open);
    } else {
        d->setting->setAuthalg(Knm::WirelessSecuritySetting::EnumAuthalg::shared);
    }
}


void WepWidget::readSecrets()
{
    // keys
    d->keys.replace(0, d->setting->wepkey0());
    d->keys.replace(1, d->setting->wepkey1());
    d->keys.replace(2, d->setting->wepkey2());
    d->keys.replace(3, d->setting->wepkey3());

    // passphrase
    if(d->keys.value(d->keyIndex).isEmpty()) {
        d->ui.keyType->setCurrentIndex(0);
    } else {
        d->ui.keyType->setCurrentIndex(1);
    }

    d->ui.key->setText(d->keys.value(d->keyIndex));
    d->ui.passphrase->setText(d->setting->weppassphrase());
}

// vim: sw=4 sts=4 et tw=100
