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

#include <nm-setting-wireless-security.h>
#include <solid/control/wirelessaccesspoint.h>

#include <KDebug>

#include "configxml.h"
#include "ui_802_11_wireless_security.h"
#include "wepwidget.h"
#include "wpapskwidget.h"

const QString Wireless80211SecurityWidget::KEY_MGMT_NONE = QLatin1String("none");
const QString Wireless80211SecurityWidget::KEY_MGMT_802_1X = QLatin1String("ieee8021x");
const QString Wireless80211SecurityWidget::KEY_MGMT_WPA_NONE = QLatin1String("wpa-none");
const QString Wireless80211SecurityWidget::KEY_MGMT_WPA_PSK = QLatin1String("wpa-psk");
const QString Wireless80211SecurityWidget::KEY_MGMT_WPA_EAP = QLatin1String("wpa-eap");

class Wireless80211SecurityWidget::Private
{
public:
    Ui_Wireless80211Security ui;
    QHash<int, SecurityWidget *> securityWidgetHash;
    int noSecurityIndex;
    int staticWepHexIndex;
    int wpaPskIndex;
    int security;
};

Wireless80211SecurityWidget::Wireless80211SecurityWidget(bool setDefaults, const QString& connectionId,
                                                         uint caps, uint wpa, uint rsn, QWidget * parent)
    : SettingWidget(connectionId, parent), d(new Wireless80211SecurityWidget::Private)
{
    Q_UNUSED( rsn );
    d->noSecurityIndex = -1;
    d->staticWepHexIndex = -1;
    d->wpaPskIndex = -1;
    d->ui.setupUi(this);
    init();
    // cache ap and device capabilities here
    // populate cmbType with appropriate wireless security types
    int index = 0;
    d->ui.cmbType->insertItem(index, i18nc("Label for no wireless security", "None"));
    d->noSecurityIndex = index++;

    // Fixme: add distinct types of WEP
    d->ui.cmbType->insertItem(index, i18nc("Label for WEP wireless security", "WEP"));
    SecurityWidget * sw = new WepWidget(WepWidget::Hex, configXml()->config(), connectionId, this);
    d->securityWidgetHash.insert(index, sw);
    d->ui.stackedWidget->insertWidget(index, sw);
    d->staticWepHexIndex = index++;

    d->ui.cmbType->insertItem(index, i18nc("Label for WPA-PSK wireless security", "WPA-PSK"));
    sw = new WpaPskWidget(configXml()->config(), connectionId, this);
    d->securityWidgetHash.insert(index, sw);
    d->ui.stackedWidget->insertWidget(index, sw);
    d->wpaPskIndex = index++;

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

bool Wireless80211SecurityWidget::hasSecrets() const
{
    kDebug() << d->ui.cmbType->currentIndex() << d->noSecurityIndex;
    return d->ui.cmbType->currentIndex() != d->noSecurityIndex;
}

QString Wireless80211SecurityWidget::settingName() const
{
    return QLatin1String(NM_SETTING_WIRELESS_SECURITY_SETTING_NAME);
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
    SecurityWidget * sw = d->securityWidgetHash.value(d->ui.cmbType->currentIndex());
    if (sw) {
        sw->writeConfig();
    }
}

void Wireless80211SecurityWidget::readConfig()
{
    if ( d->security != -1 )
        return;

    if (!configXml()->config()->hasGroup(NM_SETTING_WIRELESS_SECURITY_SETTING_NAME)) {
        kDebug() << "Unsecured wireless network";
        d->ui.cmbType->setCurrentIndex(d->noSecurityIndex);
    } else {
        KConfigSkeletonItem * item = configXml()->findItem(settingName(), QLatin1String("keymgmt"));
        if ( item) {
            KCoreConfigSkeleton::ItemEnum *enumItem = (KCoreConfigSkeleton::ItemEnum*)item;
            QString itemString = enumItem->choices()[item->property().toInt()].name;
            kDebug() << "keymgt = " << itemString;
            if (itemString == Wireless80211SecurityWidget::KEY_MGMT_NONE) {
                kDebug() << "WEP";
                d->ui.cmbType->setCurrentIndex(d->staticWepHexIndex);
                SecurityWidget * sw = d->securityWidgetHash.value(d->staticWepHexIndex);
                sw->readConfig();
            } else if (itemString == Wireless80211SecurityWidget::KEY_MGMT_WPA_PSK) {
                kDebug() << "WPA-PSK";
                d->ui.cmbType->setCurrentIndex(d->wpaPskIndex);
                SecurityWidget * sw = d->securityWidgetHash.value(d->wpaPskIndex);
                sw->readConfig();
            } else {
                kDebug() << "Key management setting not found!";
            }
        }
    }
}

QVariantMap Wireless80211SecurityWidget::secrets() const
{
    QVariantMap s;
    SecurityWidget * sw = qobject_cast<SecurityWidget*>(d->ui.stackedWidget->currentWidget());
    if (sw) {
        s = sw->secrets();
    } else {
        kDebug() << "currentWidget was not a SecurityWidget";
    }
    return s;
}

#include "802_11_wireless_security_widget.moc"

// vim: sw=4 sts=4 et tw=100
