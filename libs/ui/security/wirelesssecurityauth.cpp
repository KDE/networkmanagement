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

#include "wirelesssecurityauth.h"
#include "settingwidget_p.h"

#include <connection.h>
#include <settings/802-11-wireless.h>
#include <settings/802-11-wireless-security.h>

#include <wirelesssecurityidentifier.h>

#include "securitywidget.h"

#include "nullsecuritywidget.h"
#include "wepauthwidget.h"
#include "leapauthwidget.h"
#include "wpaauthwidget.h"

#include <QFormLayout>
#include <QCheckBox>
#include <KLocale>

class WirelessSecurityAuthWidgetPrivate : public SettingWidgetPrivate
{
public:
    Knm::WirelessSetting * settingWireless;
    Knm::WirelessSecuritySetting * settingSecurity;
    SecurityWidget *settingWidget;
    QFormLayout *layout;
};

WirelessSecurityAuthWidget::WirelessSecurityAuthWidget(Knm::Connection * connection, QWidget * parent)
: SettingWidget(*new WirelessSecurityAuthWidgetPrivate, connection, parent)
{
    Q_D(WirelessSecurityAuthWidget);

    d->layout = new QFormLayout(this);
    this->setLayout(d->layout);
    d->settingWireless = static_cast<Knm::WirelessSetting *>(connection->setting(Knm::Setting::Wireless));
    d->settingSecurity = static_cast<Knm::WirelessSecuritySetting *>(connection->setting(Knm::Setting::WirelessSecurity));
}

WirelessSecurityAuthWidget::~WirelessSecurityAuthWidget()
{
}

void WirelessSecurityAuthWidget::readConfig()
{
    Q_D(WirelessSecurityAuthWidget);
    if (!d->settingWireless) {
        kWarning() << "Wireless setting is null. That should not happen.";
        return;
    }
    if (d->settingWireless->security().isEmpty()) {
        d->settingWidget = new NullSecurityWidget(d->connection, this);
    } else {
        switch (d->settingSecurity->securityType()) {
            case Knm::WirelessSecuritySetting::EnumSecurityType::StaticWep:
                d->settingWidget = new WepAuthWidget(d->connection, d->layout, this);
                break;
            case Knm::WirelessSecuritySetting::EnumSecurityType::Leap:
                d->settingWidget = new LeapAuthWidget(d->connection, d->layout, this);
                break;
            case Knm::WirelessSecuritySetting::EnumSecurityType::WpaPsk:
            case Knm::WirelessSecuritySetting::EnumSecurityType::Wpa2Psk:
                d->settingWidget = new WpaAuthWidget(d->connection, d->layout, this);
                break;
            // every other security type does not have secrets in 802-11-wireless-security setting,
            // this object should not even have been created
            default:
                d->settingWidget = new NullSecurityWidget(d->connection, this);
                break;
        }
        d->settingWidget->readConfig();
        d->layout->addWidget(d->settingWidget);
    }

    QCheckBox *showPasswords = new QCheckBox(this);
    showPasswords->setText(i18n("&Show password"));
    d->layout->setWidget(d->layout->rowCount(), QFormLayout::FieldRole, showPasswords);
    connect(showPasswords, SIGNAL(toggled(bool)), d->settingWidget, SLOT(setShowPasswords(bool)));
    d->layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
}

void WirelessSecurityAuthWidget::writeConfig()
{
    Q_D(WirelessSecurityAuthWidget);
    d->settingWidget->writeConfig();
}

void WirelessSecurityAuthWidget::validate()
{
    Q_D(WirelessSecurityAuthWidget);
    emit valid(d->settingWidget->validate());
}
