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

#include "leapauthwidget.h"

#include <QWidget>
#include <QFormLayout>
#include <QLabel>
#include <KLineEdit>
#include <KLocale>

#include <settings/802-11-wireless-security.h>
#include <connection.h>

class LeapAuthWidget::Private
{
public:
    Knm::WirelessSecuritySetting * setting;
    QFormLayout *layout;
    KLineEdit *pw;
};

LeapAuthWidget::LeapAuthWidget(Knm::Connection * connection, QWidget * parent)
: SecurityWidget(connection, parent), d(new LeapAuthWidget::Private)
{
    d->setting = static_cast<Knm::WirelessSecuritySetting *>(connection->setting(Knm::Setting::WirelessSecurity));
    d->layout = new QFormLayout(this);
    this->setLayout(d->layout);
    setupUi();
}

LeapAuthWidget::LeapAuthWidget(Knm::Connection * connection, QFormLayout *layout, QWidget * parent)
: SecurityWidget(connection, parent), d(new LeapAuthWidget::Private)
{
    d->setting = static_cast<Knm::WirelessSecuritySetting *>(connection->setting(Knm::Setting::WirelessSecurity));
    d->layout = layout;
    setupUi();
}

LeapAuthWidget::~LeapAuthWidget()
{
    delete d;
}

void LeapAuthWidget::setupUi()
{
    QLabel *label = new QLabel(this);
    label->setText(i18n("Password:"));
    d->pw = new KLineEdit(this);
    d->pw->setPasswordMode(true);
    d->layout->addRow(label, d->pw);
    d->pw->setFocus(Qt::OtherFocusReason);
}

void LeapAuthWidget::emitValid()
{
    emit valid(validate());
}

bool LeapAuthWidget::validate() const
{
    return true;
}

void LeapAuthWidget::readSecrets()
{
    d->pw->setText(d->setting->leappassword());
}

void LeapAuthWidget::writeConfig()
{
    d->setting->setLeappassword(d->pw->text());
}

void LeapAuthWidget::setShowPasswords(bool on)
{
    d->pw->setPasswordMode(!on);
}
