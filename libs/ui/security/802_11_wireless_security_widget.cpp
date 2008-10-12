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

#include <KDebug>

#include "configxml.h"
#include "ui_802_11_wireless_security.h"
#include "wepwidget.h"

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
};

Wireless80211SecurityWidget::Wireless80211SecurityWidget(const QString& connectionId, QWidget * parent)
: SettingWidget(connectionId, parent), d(new Wireless80211SecurityWidget::Private)
{
    d->noSecurityIndex = -1;
    d->staticWepHexIndex = - 1;
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
    connect(d->ui.cmbType, SIGNAL(currentIndexChanged(int)), this, SLOT(securityTypeChanged(int)));
}

Wireless80211SecurityWidget::~Wireless80211SecurityWidget()
{
    delete d;
}

QString Wireless80211SecurityWidget::settingName() const
{
    return QLatin1String(NM_SETTING_WIRELESS_SECURITY_SETTING_NAME);
}

void Wireless80211SecurityWidget::securityTypeChanged(int index)
{
    // None 0 -> 0
    // Wep 1,2,3 -> 1
    // Leap 4 -> 2
    // Dynamic WEP 5 -> 4
    // WPA PSK 6 ->  3
    // WPA EAP 7 -> 4
    switch (index) {
        case 0: // None
            d->ui.stackedWidget->setCurrentIndex(0);
            break;
        case 1: // WEP
        case 2:
        case 3:
            d->ui.stackedWidget->setCurrentIndex(1);
            break;
        case 4: // LEAP
            d->ui.stackedWidget->setCurrentIndex(2);
            break;
        case 5:
            d->ui.stackedWidget->setCurrentIndex(4);
            break;
        case 6:
            d->ui.stackedWidget->setCurrentIndex(3);
            break;
        case 7:
            d->ui.stackedWidget->setCurrentIndex(4);
            break;
        default:
            break;
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
    if (!configXml()->config()->hasGroup(NM_SETTING_WIRELESS_SECURITY_SETTING_NAME)) {
        kDebug() << "Unsecured wireless network";
        d->ui.cmbType->setCurrentIndex(d->noSecurityIndex);
    }

    KConfigSkeletonItem * item = configXml()->findItem(settingName(), QLatin1String("keymgmt"));
    if ( item) {
        if (item->property().toString() == QLatin1String("none")) {
            kDebug() << "WEP";
            d->ui.cmbType->setCurrentIndex(d->staticWepHexIndex);
            SecurityWidget * sw = d->securityWidgetHash.value(d->staticWepHexIndex);
            sw->readConfig();
        } else {
            kDebug() << "Key management setting not found!";
        }
    }
#if 0
    // First, figure out what type of security is stored.
    //
    KConfigSkeletonItem * item = configXml()->findItem(settingName(), QLatin1String(NM_SETTING_WIRELESS_SECURITY_KEY_MGMT));
    Q_ASSERT(item);
    QString keyMgmt = item->property().toString();
    kDebug() << keyMgmt;
    SecurityWidget * sw = d->securityWidgetHash->value(d->ui.cmbType->currentIndex());
    if (sw) {
        sw->writeConfig();
    }
#endif
}

#include "802_11_wireless_security_widget.moc"

// vim: sw=4 sts=4 et tw=100
