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
    enum PasswordStorage {Store = 0, AlwaysAsk, NotRequired};
};

LeapWidget::LeapWidget(Knm::Connection* connection, QWidget * parent)
: SecurityWidget(connection, parent), d(new LeapWidget::Private)
{
    d->ui.setupUi(this);
    d->setting = static_cast<Knm::WirelessSecuritySetting *>(connection->setting(Knm::Setting::WirelessSecurity));
    d->wsetting = static_cast<Knm::WirelessSetting *>(connection->setting(Knm::Setting::Wireless));

    connect(d->ui.chkShowPass, SIGNAL(toggled(bool)), this, SLOT(chkShowPassToggled(bool)));
    connect(d->ui.cmbPasswordStorage, SIGNAL(currentIndexChanged(int)), this, SLOT(passwordStorageChanged(int)));
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

void LeapWidget::passwordStorageChanged(int type)
{
    switch (type)
    {
        case Private::Store:
            d->ui.lePassword->setEnabled(true);
            break;
        default:
            d->ui.lePassword->setEnabled(false);
            break;
    }
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
    d->setting->setAuthalg(Knm::WirelessSecuritySetting::EnumAuthalg::leap);
    d->setting->setKeymgmt(Knm::WirelessSecuritySetting::EnumKeymgmt::Ieee8021x);
    switch (d->ui.cmbPasswordStorage->currentIndex()) {
        case Private::Store:
            d->setting->setLeappassword(d->ui.lePassword->text());
            d->setting->setLeappasswordflags(Knm::Setting::AgentOwned);
            break;
        case Private::AlwaysAsk:
            d->setting->setLeappasswordflags(Knm::Setting::NotSaved);
            break;
        case Private::NotRequired:
            d->setting->setLeappasswordflags(Knm::Setting::NotRequired);
            break;
    }
}

void LeapWidget::readSecrets()
{
    if (d->setting->leappasswordflags() & Knm::Setting::AgentOwned || d->setting->leappasswordflags() & Knm::Setting::None) {
        d->ui.lePassword->setText(d->setting->leappassword());
        d->ui.cmbPasswordStorage->setCurrentIndex(Private::Store);
    } else if (d->setting->leappasswordflags() & Knm::Setting::NotSaved) {
        d->ui.cmbPasswordStorage->setCurrentIndex(Private::AlwaysAsk);
    } else if (d->setting->leappasswordflags() & Knm::Setting::NotRequired){
        d->ui.cmbPasswordStorage->setCurrentIndex(Private::NotRequired);
    }
}

// vim: sw=4 sts=4 et tw=100
