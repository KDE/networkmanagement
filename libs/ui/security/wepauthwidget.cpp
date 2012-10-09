/*
Copyright 2011 Ilia Kats <ilia-kats@gmx.net>

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

#include "wepauthwidget.h"

#include <QWidget>
#include <QValidator>
#include <QFormLayout>
#include <QLabel>
#include <QRegExp>
#include <QRegExpValidator>
#include <KLineEdit>
#include <KLocale>

#include <settings/802-11-wireless-security.h>
#include <connection.h>

static const int ASCII_MIN = ' ';
static const int ASCII_MAX = '~';

class WepAuthWidget::Private
{
public:
    Knm::WirelessSecuritySetting * setting;
    QRegExpValidator * validator;
    QFormLayout *layout;
    KLineEdit *pw;
};

WepAuthWidget::WepAuthWidget(Knm::Connection * connection, QWidget * parent)
: SecurityWidget(connection, parent), d(new WepAuthWidget::Private)
{
    d->setting = static_cast<Knm::WirelessSecuritySetting *>(connection->setting(Knm::Setting::WirelessSecurity));
    d->layout = new QFormLayout(this);
    this->setLayout(d->layout);
}

WepAuthWidget::WepAuthWidget(Knm::Connection * connection, QFormLayout *layout, QWidget * parent)
: SecurityWidget(connection, parent), d(new WepAuthWidget::Private)
{
    d->setting = static_cast<Knm::WirelessSecuritySetting *>(connection->setting(Knm::Setting::WirelessSecurity));
    d->layout = layout;
}

WepAuthWidget::~WepAuthWidget()
{
    delete d;
}

bool WepAuthWidget::validate() const
{
    return d->pw->hasAcceptableInput();
}

void WepAuthWidget::readConfig()
{
    QLabel *label = new QLabel(this);
    d->pw = new KLineEdit(this);
    d->pw->setPasswordMode(true);
    QRegExp regExp;
    switch (d->setting->wepKeyType()) {
        case Knm::WirelessSecuritySetting::Passphrase:
        {
            label->setText(i18n("Passphrase:"));
            QString asciiRegExp = "[-";
            for (char ch = ASCII_MIN; ch <= ASCII_MAX; ++ch) {
                if (ch != '-') {
                    if (ch == '\\' || ch == ']') {
                        asciiRegExp.append('\\');
                    }
                    asciiRegExp.append(ch);
                }
            }
            asciiRegExp.append(']');
            asciiRegExp = asciiRegExp + "{5}|" + asciiRegExp + "{13}"; // 5 or 13 ASCII chars
            regExp = QRegExp(asciiRegExp);
            break;
        }
        case Knm::WirelessSecuritySetting::Hex:
            label->setText(i18n("Key:"));
            regExp = QRegExp("([0-9]|[a-f]|[A-F]){10}|([0-9]|[a-f]|[A-F]){26}"); // 10 or 26 Hex
            break;
        case Knm::WirelessSecuritySetting::None:
        case Knm::WirelessSecuritySetting::COUNT:
            break;
    }
    d->pw->setValidator(new QRegExpValidator(regExp, this));

    d->layout->addRow(label, d->pw);
    d->pw->setFocus(Qt::OtherFocusReason);
    //Validate the keys when changed, required to channel validation back to ConnectionSecretsJob::Ok button
    connect(d->pw, SIGNAL(textChanged(QString)), SLOT(validateKey(QString)));
}

void WepAuthWidget::readSecrets()
{
    switch (d->setting->weptxkeyindex()) {
        case 0:
            d->pw->setText(d->setting->wepkey0());
            break;
        case 1:
            d->pw->setText(d->setting->wepkey1());
            break;
        case 2:
            d->pw->setText(d->setting->wepkey2());
            break;
        case 3:
            d->pw->setText(d->setting->wepkey3());
            break;
    }
}

void WepAuthWidget::writeConfig()
{
    switch (d->setting->weptxkeyindex()) {
        case 0:
            d->setting->setWepkey0(d->pw->text());
            break;
        case 1:
            d->setting->setWepkey1(d->pw->text());
            break;
        case 2:
            d->setting->setWepkey2(d->pw->text());
            break;
        case 3:
            d->setting->setWepkey3(d->pw->text());
            break;
    }
}

void WepAuthWidget::setShowPasswords(bool on)
{
    d->pw->setPasswordMode(!on);
}

void WepAuthWidget::validateKey(const QString &key)
{
    Q_UNUSED(key);
    emit valid(validate());
}
