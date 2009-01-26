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

#include "802_11_wireless_security_widget.h"

#include <solid/control/wirelessaccesspoint.h>

#include <KDebug>

#include "connection.h"
#include "settings/802-11-wireless-security.h"
#include "ui_802_11_wireless_security.h"
#include "securitywidget.h"

#include "wepwidget.h"
#include "wpapskwidget.h"
#include "wpaeapwidget.h"

class Wireless80211SecurityWidget::Private
{
public:
    Ui_Wireless80211Security ui;
    QHash<int, SecurityWidget *> securityWidgetHash;
    int noSecurityIndex;
    int staticWepHexIndex;
    int wpaPskIndex;
    int wpaEapIndex;
    int security;
    WpaEapWidget * wpaeapwid;
    Knm::WirelessSecuritySetting * setting;
};

Wireless80211SecurityWidget::Wireless80211SecurityWidget(bool setDefaults, Knm::Connection * connection,
                                                         uint caps, uint wpa, uint rsn, QWidget * parent)
    : SettingWidget(connection, parent), d(new Wireless80211SecurityWidget::Private)
{
    Q_UNUSED( rsn );
    d->noSecurityIndex = -1;
    d->staticWepHexIndex = -1;
    d->wpaPskIndex = -1;
    d->ui.setupUi(this);

    d->setting = static_cast<Knm::WirelessSecuritySetting *>(connection->setting(Knm::Setting::WirelessSecurity));

    // cache ap and device capabilities here
    // populate cmbType with appropriate wireless security types
    int index = 0;
    d->ui.cmbType->insertItem(index, i18nc("Label for no wireless security", "None"));
    d->noSecurityIndex = index++;

    // Fixme: add distinct types of WEP
    d->ui.cmbType->insertItem(index, i18nc("Label for WEP wireless security", "WEP"));
    SecurityWidget * sw = new WepWidget(WepWidget::Hex, connection, this);
    d->securityWidgetHash.insert(index, sw);
    d->ui.stackedWidget->insertWidget(index, sw);
    d->staticWepHexIndex = index++;

    d->ui.cmbType->insertItem(index, i18nc("Label for WPA-PSK wireless security", "WPA-PSK"));
    sw = new WpaPskWidget(connection, this);
    d->securityWidgetHash.insert(index, sw);
    d->ui.stackedWidget->insertWidget(index, sw);
    d->wpaPskIndex = index++;

    d->ui.cmbType->insertItem(index, i18nc("Label for WPA-EAP wireless security", "WPA-EAP"));
    sw = d->wpaeapwid = new WpaEapWidget(connection, this);
    d->securityWidgetHash.insert(index, sw);
    d->ui.stackedWidget->insertWidget(index, sw);
    d->wpaEapIndex = index++;
    
    Solid::Control::AccessPoint::WpaFlags wpaFlags( wpa );
    Solid::Control::AccessPoint::WpaFlags rsnFlags( rsn );

    d->security = -1;

    if ( setDefaults )
    {
        d->security = 0;

        if ( caps )
            d->security = d->staticWepHexIndex;

        // TODO: this was done by a clueless (coolo)
        if ( wpaFlags.testFlag( Solid::Control::AccessPoint::PairWep40 ) ||
             wpaFlags.testFlag( Solid::Control::AccessPoint::PairWep104 ) )
            d->security = d->staticWepHexIndex;

        if ( wpaFlags.testFlag( Solid::Control::AccessPoint::KeyMgmtPsk ) ||
             wpaFlags.testFlag( Solid::Control::AccessPoint::PairTkip ) )
            d->security = d->wpaPskIndex;

        if ( rsnFlags.testFlag( Solid::Control::AccessPoint::KeyMgmtPsk ) ||
             rsnFlags.testFlag( Solid::Control::AccessPoint::PairTkip ) || 
             rsnFlags.testFlag( Solid::Control::AccessPoint::PairCcmp ) )
            d->security = d->wpaPskIndex;

        d->ui.cmbType->setCurrentIndex( d->security );
        securityTypeChanged( d->security );
    }

    connect(d->ui.cmbType, SIGNAL(currentIndexChanged(int)), this, SLOT(securityTypeChanged(int)));
}

Wireless80211SecurityWidget::~Wireless80211SecurityWidget()
{
    delete d;
}

SettingInterface* Wireless80211SecurityWidget::wpaEapWidget()
{
    return 0; //d->wpaeapwid;
}

void Wireless80211SecurityWidget::securityTypeChanged(int index)
{
    d->ui.stackedWidget->setCurrentWidget(d->securityWidgetHash.value(index));

    // hide the security widgets
    if (index == 0) {
        d->ui.stackedWidget->currentWidget()->hide();
    } else {
        //show them
        d->ui.stackedWidget->currentWidget()->show();
    }
}

void Wireless80211SecurityWidget::writeConfig()
{
    if (d->ui.cmbType->currentIndex() == d->noSecurityIndex) {
        d->setting->setSecurityType(Knm::WirelessSecuritySetting::EnumSecurityType::None);
    }
    if (d->ui.cmbType->currentIndex() == d->staticWepHexIndex) {
        d->setting->setSecurityType(Knm::WirelessSecuritySetting::EnumSecurityType::WEP40); // FIXME
        d->setting->setKeymgmt(Knm::WirelessSecuritySetting::EnumKeymgmt::None);
    }
    else if (d->ui.cmbType->currentIndex() == d->wpaEapIndex) {
        d->setting->setSecurityType(Knm::WirelessSecuritySetting::EnumSecurityType::WPAEAP); // FIXME
        d->setting->setKeymgmt(Knm::WirelessSecuritySetting::EnumKeymgmt::WPAEAP);
    }
    else if (d->ui.cmbType->currentIndex() == d->wpaPskIndex) {
        d->setting->setSecurityType(Knm::WirelessSecuritySetting::EnumSecurityType::WPAPSK); // FIXME
        d->setting->setKeymgmt(Knm::WirelessSecuritySetting::EnumKeymgmt::WPAPSK);
    }

    SecurityWidget * sw = d->securityWidgetHash.value(d->ui.cmbType->currentIndex());
    if (sw) {
        sw->writeConfig();
    }
}

void Wireless80211SecurityWidget::readConfig()
{
    if ( d->security != -1 )
        return;

    switch (d->setting->securityType()) {
        case Knm::WirelessSecuritySetting::EnumSecurityType::None:
            d->security = d->noSecurityIndex;
            break;
        case Knm::WirelessSecuritySetting::EnumSecurityType::WEP40:
            d->security = d->staticWepHexIndex;
            //SecurityWidget * sw = d->securityWidgetHash.value(d->staticWepHexIndex);
            //sw->readConfig();            
            break;
        case Knm::WirelessSecuritySetting::EnumSecurityType::WEP128:
            d->security = d->staticWepHexIndex;
            //SecurityWidget * sw = d->securityWidgetHash.value(d->staticWepHexIndex);
            //sw->readConfig();            
            break;
        case Knm::WirelessSecuritySetting::EnumSecurityType::DynamicWEP:
            break;
        case Knm::WirelessSecuritySetting::EnumSecurityType::WPAPSK:
            d->security = d->wpaPskIndex;
            //SecurityWidget * sw = d->securityWidgetHash.value(d->wpaPskIndex);
            //sw->readConfig();
            break;
        case Knm::WirelessSecuritySetting::EnumSecurityType::WPAEAP:
            d->security = d->wpaEapIndex;
            SecurityWidget * sw = d->securityWidgetHash.value(d->wpaEapIndex);
            sw->readConfig();            
            break;
    }
    d->ui.cmbType->setCurrentIndex( d->security );
    securityTypeChanged( d->security );
}

#include "802_11_wireless_security_widget.moc"

// vim: sw=4 sts=4 et tw=100
