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

#include "wirelesssecuritysettingwidget.h"

#include <solid/control/wirelessaccesspoint.h>

#include <KDebug>

#include <connection.h>
#include <connectionpersistence.h>
#include <settings/802-1x.h>
#include <settings/802-11-wireless.h>
#include <settings/802-11-wireless-security.h>

#include "ui_wirelesssecurity.h"
#include "securitywidget.h"

#include "nullsecuritywidget.h"
#include "wepwidget.h"
#include "wpapskwidget.h"
#include "wpaeapwidget.h"

class WirelessSecuritySettingWidgetPrivate
{
Q_DECLARE_PUBLIC(WirelessSecuritySettingWidget)
public:
    WirelessSecuritySettingWidgetPrivate(WirelessSecuritySettingWidget * parent)
        : q_ptr(parent), noSecurityIndex(-1), staticWepIndex(-1), wpaPskIndex(-1), wpaEapIndex(-1), currentSecurity(-1),
        wpaEapWidget(0), settingWireless(0), settingSecurity(0), persistence(0)
    {
    }

    void registerSecurityType(SecurityWidget * securityWidget, const QString & label, int & index)
    {
        ui.cboType->addItem(label);
        index = ui.securityWidgets->addWidget(securityWidget);
    }

    void setCurrentSecurityWidget(int index)
    {
        if (index >= 0 && index < ui.securityWidgets->count()) {
            ui.cboType->setCurrentIndex(index);
        }
    }

    SecurityWidget * currentSecurityWidget() const
    {
        return static_cast<SecurityWidget*>(ui.securityWidgets->currentWidget());
    }

    WirelessSecuritySettingWidget * q_ptr;

    Ui_WirelessSecurity ui;
    //QHash<int, SecurityWidget *> securityWidgetHash;
    int noSecurityIndex;
    int staticWepIndex;
    int wpaPskIndex;
    int wpaEapIndex;
    int currentSecurity;
    WpaEapWidget * wpaEapWidget;
    Knm::WirelessSetting * settingWireless;
    Knm::WirelessSecuritySetting * settingSecurity;
    Knm::Security8021xSetting * setting8021x;
    Knm::ConnectionPersistence * persistence;
};

WirelessSecuritySettingWidget::WirelessSecuritySettingWidget(bool setDefaults, Knm::Connection * connection,
                                                         uint caps, uint wpa, uint rsn, QWidget * parent)
    : SettingWidget(connection, parent), d_ptr(new WirelessSecuritySettingWidgetPrivate(this))
{
    Q_D(WirelessSecuritySettingWidget);

    d->ui.setupUi(this);
    QObject::connect(d->ui.cboType, SIGNAL(currentIndexChanged(int)), d->ui.securityWidgets, SLOT(setCurrentIndex(int)));

    d->settingWireless = static_cast<Knm::WirelessSetting *>(connection->setting(Knm::Setting::Wireless));
    d->settingSecurity = static_cast<Knm::WirelessSecuritySetting *>(connection->setting(Knm::Setting::WirelessSecurity));
    d->setting8021x = static_cast<Knm::Security8021xSetting *>(connection->setting(Knm::Setting::Security8021x));

    // cache ap and device capabilities here
    Solid::Control::AccessPoint::WpaFlags wpaFlags( wpa );
    Solid::Control::AccessPoint::WpaFlags rsnFlags( rsn );

    // populate cboType with appropriate wireless security types
    // TODO this is probably too simplistic and does not check things that nm-applet checks like
    // adhoc mode, whether the network interfaces supports the auth modes the AP is supporting
    // see page-wireless-security.c in network-manager-applet
    // and ibnm-util/nm-utils.c nm_utils_security_valid()

    // insecure
    if (!setDefaults || caps || wpa || rsn) {
        d->registerSecurityType(new NullSecurityWidget(connection, this), i18nc("Label for no wireless security", "None"), d->noSecurityIndex);
    }

    // WEP
    if (!setDefaults || caps || ( wpaFlags.testFlag( Solid::Control::AccessPoint::PairWep40 ) ||
             wpaFlags.testFlag( Solid::Control::AccessPoint::PairWep104 ) )
       ) {
         d->registerSecurityType(new WepWidget(WepWidget::Passphrase, connection, this), i18nc("Label for WEP wireless security", "WEP"), d->staticWepIndex);
    }

    // WPA
    if ( !setDefaults || wpaFlags.testFlag( Solid::Control::AccessPoint::KeyMgmtPsk )
            || wpaFlags.testFlag( Solid::Control::AccessPoint::PairTkip)
            || rsnFlags.testFlag( Solid::Control::AccessPoint::KeyMgmtPsk )
            || rsnFlags.testFlag( Solid::Control::AccessPoint::PairTkip )
            || rsnFlags.testFlag( Solid::Control::AccessPoint::PairCcmp ) ) {
        d->registerSecurityType(new WpaPskWidget(connection, this), i18nc("Label for WPA-PSK wireless security", "WPA-PSK"), d->wpaPskIndex);
    }
    // what for EAP?
    if (!setDefaults
            || wpaFlags.testFlag(Solid::Control::AccessPoint::KeyMgmt8021x)
            || rsnFlags.testFlag(Solid::Control::AccessPoint::KeyMgmt8021x)) {
        d->registerSecurityType(new WpaEapWidget(connection, this), i18nc("Label for WPA-EAP wireless security", "WPA-EAP"), d->wpaEapIndex);
    }

    // select best available security by default
    if (d->ui.cboType->count()) {
        d->ui.cboType->setCurrentIndex(d->ui.cboType->count() - 1);
    }
}

WirelessSecuritySettingWidget::~WirelessSecuritySettingWidget()
{
    delete d_ptr;
}

void WirelessSecuritySettingWidget::writeConfig()
{
    Q_D(WirelessSecuritySettingWidget);
    d->settingWireless->setSecurity(d->settingSecurity->name());
    if (d->ui.cboType->currentIndex() == d->noSecurityIndex) {
        d->setting8021x->setEnabled(false);
        d->settingSecurity->setSecurityType(Knm::WirelessSecuritySetting::EnumSecurityType::None);
        d->settingWireless->setSecurity("");
    }
    if (d->ui.cboType->currentIndex() == d->staticWepIndex) {
        d->setting8021x->setEnabled(false);
        d->settingSecurity->setSecurityType(Knm::WirelessSecuritySetting::EnumSecurityType::WEP40); // FIXME
        d->settingSecurity->setKeymgmt(Knm::WirelessSecuritySetting::EnumKeymgmt::None);
    }
    else if (d->ui.cboType->currentIndex() == d->wpaPskIndex) {
        d->setting8021x->setEnabled(false);
        d->settingSecurity->setSecurityType(Knm::WirelessSecuritySetting::EnumSecurityType::WPAPSK); // FIXME
        d->settingSecurity->setKeymgmt(Knm::WirelessSecuritySetting::EnumKeymgmt::WPAPSK);
    }
    else if (d->ui.cboType->currentIndex() == d->wpaEapIndex) {
        d->setting8021x->setEnabled(true);
        d->settingSecurity->setSecurityType(Knm::WirelessSecuritySetting::EnumSecurityType::WPAEAP); // FIXME
        d->settingSecurity->setKeymgmt(Knm::WirelessSecuritySetting::EnumKeymgmt::WPAEAP);
    }

    SecurityWidget * sw = d->currentSecurityWidget();
    if (sw) {
        sw->writeConfig();
    }
}

void WirelessSecuritySettingWidget::readConfig()
{
    Q_D(WirelessSecuritySettingWidget);
//X     if ( d->currentSecurity != -1 )
//X         return;
//X 
    SecurityWidget * sw = 0;
    switch (d->settingSecurity->securityType()) {
        case Knm::WirelessSecuritySetting::EnumSecurityType::None:
            d->setCurrentSecurityWidget(d->noSecurityIndex);
            break;
        case Knm::WirelessSecuritySetting::EnumSecurityType::WEP40:
            d->setCurrentSecurityWidget(d->staticWepIndex);
            break;
        case Knm::WirelessSecuritySetting::EnumSecurityType::WEP128:
            d->setCurrentSecurityWidget(d->staticWepIndex);
            break;
        case Knm::WirelessSecuritySetting::EnumSecurityType::DynamicWEP:
            break;
        case Knm::WirelessSecuritySetting::EnumSecurityType::WPAPSK:
            d->setCurrentSecurityWidget(d->wpaPskIndex);
            break;
        case Knm::WirelessSecuritySetting::EnumSecurityType::WPAEAP:
            d->setCurrentSecurityWidget(d->wpaEapIndex);
            break;
    }

    sw = d->currentSecurityWidget();
    sw->readConfig();
}

void WirelessSecuritySettingWidget::readSecrets()
{
    Q_D(WirelessSecuritySettingWidget);
    SecurityWidget * sw = d->currentSecurityWidget();
    sw->readSecrets();
}

// vim: sw=4 sts=4 et tw=100
