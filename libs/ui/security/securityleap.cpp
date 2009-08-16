/*
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

#include "securityleap.h"

#include <QWidget>

//#include <nm-setting-wireless-security.h>
//X #include <nm-setting-connection.h>
//X #include <nm-setting-wireless.h>
//X #include <nm-setting-8021x.h>
//X 
#include <KDebug>
#include <wpasecretidentifier.h>

//#include "802_11_wireless_security_widget.h"
#include "ui_leap.h"
#include "settings/802-11-wireless.h"
#include "settings/802-11-wireless-security.h"
#include "connection.h"


class LeapWidget::Private
{
public:
    Ui_Leap ui;
    Knm::WirelessSetting* wsetting;
    Knm::WirelessSecuritySetting* setting;
};

LeapWidget::LeapWidget(Knm::Connection* connection, QWidget * parent)
: SecurityWidget(connection, parent), d(new LeapWidget::Private)
{
    d->ui.setupUi(this);
    d->setting = static_cast<Knm::WirelessSecuritySetting *>(connection->setting(Knm::Setting::WirelessSecurity));
    d->wsetting = static_cast<Knm::WirelessSetting *>(connection->setting(Knm::Setting::Wireless));

    connect(d->ui.chkShowPass, SIGNAL(toggled(bool)), this, SLOT(chkShowPassToggled(bool)));
    d->ui.lePassword->setPasswordMode(true);
}

LeapWidget::~LeapWidget()
{
    delete d;
}

void LeapWidget::chkShowPassToggled(bool on)
{
    d->ui.lePassword->setPasswordMode(!on);
}

bool LeapWidget::validate() const
{
    return !(d->ui.lePassword->text().isEmpty() || d->ui.leUserName->text().isEmpty());
}

void LeapWidget::readConfig()
{
    d->ui.leUserName->setText(d->setting->leapusername());
    d->ui.chkShowPass->setChecked(false);
}

void LeapWidget::writeConfig()
{
    d->setting->setLeapusername(d->ui.leUserName->text());
    d->setting->setLeappassword(d->ui.lePassword->text());
    d->setting->setAuthalg(Knm::WirelessSecuritySetting::EnumAuthalg::leap);
    d->setting->setKeymgmt(Knm::WirelessSecuritySetting::EnumKeymgmt::Ieee8021x);
}

void LeapWidget::readSecrets()
{
    d->ui.lePassword->setText(d->setting->leappassword());
}

// vim: sw=4 sts=4 et tw=100
