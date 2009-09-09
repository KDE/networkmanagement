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

#include <solid/control/networkmanager.h>
#include <solid/control/networkinterface.h>
#include <solid/control/wirelessnetworkinterface.h>
#include <solid/control/wirelessaccesspoint.h>

#include <KDebug>

#include <connection.h>
#include <connectionpersistence.h>
#include <settings/802-1x.h>
#include <settings/802-11-wireless.h>
#include <settings/802-11-wireless-security.h>

#include <wirelesssecurityidentifier.h>

#include "ui_wirelesssecurity.h"
#include "securitywidget.h"

#include "nullsecuritywidget.h"
#include "wepwidget.h"
#include "securityleap.h"
#include "securitydynamicwep.h"
#include "wpapskwidget.h"
#include "wpaeapwidget.h"

class WirelessSecuritySettingWidgetPrivate
{
Q_DECLARE_PUBLIC(WirelessSecuritySettingWidget)
public:
    WirelessSecuritySettingWidgetPrivate(WirelessSecuritySettingWidget * parent)
        : q_ptr(parent), noSecurityIndex(-1), staticWepIndex(-1), leapIndex(-1), dynamicWepIndex(-1), wpaPskIndex(-1), wpaEapIndex(-1), currentSecurity(-1),
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
    int leapIndex;
    int dynamicWepIndex;
    int wpaPskIndex;
    int wpaEapIndex;
    int currentSecurity;
    WpaEapWidget * wpaEapWidget;
    Knm::WirelessSetting * settingWireless;
    Knm::WirelessSecuritySetting * settingSecurity;
    Knm::Security8021xSetting * setting8021x;
    Knm::ConnectionPersistence * persistence;
};

WirelessSecuritySettingWidget::WirelessSecuritySettingWidget(
        Knm::Connection * connection,
        Solid::Control::WirelessNetworkInterface * iface,
        Solid::Control::AccessPoint * ap,
        QWidget * parent)
: SettingWidget(connection, parent), d_ptr(new WirelessSecuritySettingWidgetPrivate(this))
{
    Q_D(WirelessSecuritySettingWidget);

    d->ui.setupUi(this);
    QObject::connect(d->ui.cboType, SIGNAL(currentIndexChanged(int)), d->ui.securityWidgets, SLOT(setCurrentIndex(int)));

    d->settingWireless = static_cast<Knm::WirelessSetting *>(connection->setting(Knm::Setting::Wireless));
    d->settingSecurity = static_cast<Knm::WirelessSecuritySetting *>(connection->setting(Knm::Setting::WirelessSecurity));
    d->setting8021x = static_cast<Knm::Security8021xSetting *>(connection->setting(Knm::Setting::Security8021x));

    // cache ap and device capabilities here
    Solid::Control::WirelessNetworkInterface::Capabilities ifaceCaps(0);
    bool adhoc = false;
    Solid::Control::AccessPoint::Capabilities apCaps(0);
    Solid::Control::AccessPoint::WpaFlags apWpa(0);
    Solid::Control::AccessPoint::WpaFlags apRsn(0);


    if (iface) {
        ifaceCaps = iface->wirelessCapabilities();
        if (ap) {
            apCaps = ap->capabilities();
            adhoc = (ap->mode() == Solid::Control::WirelessNetworkInterface::Adhoc);
            apWpa = ap->wpaFlags();
            apRsn = ap->rsnFlags();
        }
    } else {
        foreach (Solid::Control::NetworkInterface * iface, Solid::Control::NetworkManager::networkInterfaces()) {
            if (iface->type() == Solid::Control::NetworkInterface::Ieee80211) {
                Solid::Control::WirelessNetworkInterface * wirelessIface = qobject_cast<Solid::Control::WirelessNetworkInterface*>(iface);
                if (wirelessIface) {
                    ifaceCaps |= wirelessIface->wirelessCapabilities();
                }
            }
        }
    }

    // populate cboType with appropriate wireless security types


    // insecure
    if (Knm::WirelessSecurity::possible(Knm::WirelessSecurity::None, ifaceCaps, (ap != 0), adhoc, apCaps, apWpa, apRsn)) {
        d->registerSecurityType(new NullSecurityWidget(connection, this), i18nc("Label for no wireless security", "None"), d->noSecurityIndex);
    }

    // WEP
    if (Knm::WirelessSecurity::possible(Knm::WirelessSecurity::StaticWep, ifaceCaps, (ap != 0), adhoc, apCaps, apWpa, apRsn)) {
        d->registerSecurityType(new WepWidget(WepWidget::Passphrase, connection, this), i18nc("Label for WEP wireless security", "WEP"), d->staticWepIndex);
    }

    // LEAP
    if (Knm::WirelessSecurity::possible(Knm::WirelessSecurity::Leap, ifaceCaps, (ap != 0), adhoc, apCaps, apWpa, apRsn)) {
        d->registerSecurityType(new LeapWidget(connection, this), i18nc("Label for LEAP wireless security", "LEAP"), d->leapIndex);
    }

    // Dynamic WEP
    if (Knm::WirelessSecurity::possible(Knm::WirelessSecurity::DynamicWep, ifaceCaps, (ap != 0), adhoc, apCaps, apWpa, apRsn)) {
        d->registerSecurityType(new SecurityDynamicWep(connection, this), i18nc("Label for Dynamic WEP wireless security", "Dynamic WEP (802.1x)"), d->dynamicWepIndex);
    }

    // WPA-PSK
    if (Knm::WirelessSecurity::possible(Knm::WirelessSecurity::WpaPsk, ifaceCaps, (ap != 0), adhoc, apCaps, apWpa, apRsn)
            || Knm::WirelessSecurity::possible(Knm::WirelessSecurity::Wpa2Psk, ifaceCaps, (ap != 0), adhoc, apCaps, apWpa, apRsn)
       ) {
        d->registerSecurityType(new WpaPskWidget(connection, this), i18nc("Label for WPA-PSK wireless security", "WPA/WPA2 Personal"), d->wpaPskIndex);
    }
    // WPA-EAP
    if (Knm::WirelessSecurity::possible(Knm::WirelessSecurity::WpaEap, ifaceCaps, (ap != 0), adhoc, apCaps, apWpa, apRsn)
            || Knm::WirelessSecurity::possible(Knm::WirelessSecurity::Wpa2Eap, ifaceCaps, (ap != 0), adhoc, apCaps, apWpa, apRsn)
                ) {
        d->registerSecurityType(new WpaEapWidget(connection, this), i18nc("Label for WPA-EAP wireless security", "WPA/WPA2 Enterprise"), d->wpaEapIndex);
    }
}

WirelessSecuritySettingWidget::~WirelessSecuritySettingWidget()
{
    delete d_ptr;
}

void WirelessSecuritySettingWidget::readConfig()
{
    Q_D(WirelessSecuritySettingWidget);
    SecurityWidget * sw = 0;
    if (d->settingWireless->security().isEmpty()) {
        d->setCurrentSecurityWidget(d->noSecurityIndex);
    } else {
        if (d->settingSecurity->isNull()) {
            //HACK - default new connections without an AP or those which support both EAP and PSK to WPA-PSK.
            if (d->wpaPskIndex >= 0) {
                d->settingSecurity->setSecurityType(Knm::WirelessSecuritySetting::EnumSecurityType::WpaPsk);
            } else if (d->wpaEapIndex >= 0) {
                d->settingSecurity->setSecurityType(Knm::WirelessSecuritySetting::EnumSecurityType::WpaEap);
            } else if (d->staticWepIndex >= 0) {
                d->settingSecurity->setSecurityType(Knm::WirelessSecuritySetting::EnumSecurityType::StaticWep);
            }
        }
        switch (d->settingSecurity->securityType()) {
            case Knm::WirelessSecuritySetting::EnumSecurityType::None:
                d->setCurrentSecurityWidget(d->noSecurityIndex);
                break;
            case Knm::WirelessSecuritySetting::EnumSecurityType::StaticWep:
                d->setCurrentSecurityWidget(d->staticWepIndex);
                break;
            case Knm::WirelessSecuritySetting::EnumSecurityType::Leap:
                d->setCurrentSecurityWidget(d->leapIndex);
                break;
            case Knm::WirelessSecuritySetting::EnumSecurityType::DynamicWep:
                d->setCurrentSecurityWidget(d->dynamicWepIndex);
                break;
            case Knm::WirelessSecuritySetting::EnumSecurityType::WpaPsk:
                d->setCurrentSecurityWidget(d->wpaPskIndex);
                break;
            case Knm::WirelessSecuritySetting::EnumSecurityType::WpaEap:
                d->setCurrentSecurityWidget(d->wpaEapIndex);
                break;
        }

        sw = d->currentSecurityWidget();
        sw->readConfig();
    }
}

void WirelessSecuritySettingWidget::readSecrets()
{
    Q_D(WirelessSecuritySettingWidget);
    SecurityWidget * sw = d->currentSecurityWidget();
    sw->readSecrets();
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
    else if (d->ui.cboType->currentIndex() == d->staticWepIndex) {
        d->setting8021x->setEnabled(false);
        d->settingSecurity->setSecurityType(Knm::WirelessSecuritySetting::EnumSecurityType::StaticWep); // FIXME
        d->settingSecurity->setKeymgmt(Knm::WirelessSecuritySetting::EnumKeymgmt::None);
    }
    else if (d->ui.cboType->currentIndex() == d->leapIndex) {
        d->setting8021x->setEnabled(false);
        d->settingSecurity->setSecurityType(Knm::WirelessSecuritySetting::EnumSecurityType::Leap); // FIXME
    }
    else if (d->ui.cboType->currentIndex() == d->dynamicWepIndex) {
        d->setting8021x->setEnabled(true);
        d->settingSecurity->setSecurityType(Knm::WirelessSecuritySetting::EnumSecurityType::DynamicWep); // FIXME
        d->settingSecurity->setKeymgmt(Knm::WirelessSecuritySetting::EnumKeymgmt::WPAEAP);
    }
    else if (d->ui.cboType->currentIndex() == d->wpaPskIndex) {
        d->setting8021x->setEnabled(false);
        d->settingSecurity->setSecurityType(Knm::WirelessSecuritySetting::EnumSecurityType::WpaPsk); // FIXME
        d->settingSecurity->setKeymgmt(Knm::WirelessSecuritySetting::EnumKeymgmt::WPAPSK);
    }
    else if (d->ui.cboType->currentIndex() == d->wpaEapIndex) {
        d->setting8021x->setEnabled(true);
        d->settingSecurity->setSecurityType(Knm::WirelessSecuritySetting::EnumSecurityType::WpaEap); // FIXME
        d->settingSecurity->setKeymgmt(Knm::WirelessSecuritySetting::EnumKeymgmt::WPAEAP);
    }

    SecurityWidget * sw = d->currentSecurityWidget();
    if (sw) {
        sw->writeConfig();
    }
    d->settingSecurity->setInitialized();
    d->setting8021x->setInitialized();
}

// vim: sw=4 sts=4 et tw=100
