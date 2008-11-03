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

#include <QByteArray>
#include <QCryptographicHash>

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
    QByteArray wep128PassphraseHash(QByteArray input);
    QByteArray bin2hex(QByteArray bytes);

    Ui_Wireless80211Security ui;
    QHash<int, SecurityWidget *> securityWidgetHash;
    int noSecurityIndex;
    int staticWepHexIndex;
    int wpaPskIndex;
};

QByteArray Wireless80211SecurityWidget::Private::wep128PassphraseHash(QByteArray input)
{
    QByteArray md5_data, digest;

    if (input.isEmpty()) {
        return QByteArray();
    }

    md5_data.reserve(65);
    digest.reserve(16);

    /* Get at least 64 bytes */
    for (int i = 0; i < 64; i++)
        md5_data[i] = input[i % input.length()];

    /* Null terminate md5 seed data and hash it */
    md5_data[64] = 0;
    digest = QCryptographicHash::hash(md5_data, QCryptographicHash::Md5);
    return bin2hex(digest);
}

QByteArray Wireless80211SecurityWidget::Private::bin2hex(QByteArray bytes)
{
    static char hex_digits[] = "0123456789abcdef";
    QByteArray result;
    int final_len = 26; //for 128-bit encryption

    if (bytes.length() < 1) {
        return result;
    }
    if (bytes.length() > 255) { // Arbitrary limit
        return result;
    }

    result.reserve(bytes.length()*2+1);
    for (int i = 0; i < bytes.length(); i++)
    {
        result[2*i] = hex_digits[(bytes[i] >> 4) & 0xf];
        result[2*i+1] = hex_digits[bytes[i] & 0xf];
    }
    /* Cut converted key off at the correct length for this cipher type */
    if (final_len > -1)
        result[final_len] = '\0';

    return result;
}

Wireless80211SecurityWidget::Wireless80211SecurityWidget(const QString& connectionId, QWidget * parent)
: SettingWidget(connectionId, parent), d(new Wireless80211SecurityWidget::Private)
{
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
    if (!configXml()->config()->hasGroup(NM_SETTING_WIRELESS_SECURITY_SETTING_NAME)) {
        kDebug() << "Unsecured wireless network";
        d->ui.cmbType->setCurrentIndex(d->noSecurityIndex);
    } else {
        KConfigSkeletonItem * item = configXml()->findItem(settingName(), QLatin1String("keymgmt"));
        if ( item) {
            if (item->property().toString() == Wireless80211SecurityWidget::KEY_MGMT_NONE) {
                kDebug() << "WEP";
                d->ui.cmbType->setCurrentIndex(d->staticWepHexIndex);
                SecurityWidget * sw = d->securityWidgetHash.value(d->staticWepHexIndex);
                sw->readConfig();
            } else if (item->property().toString() == Wireless80211SecurityWidget::KEY_MGMT_WPA_PSK) {
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

#include "802_11_wireless_security_widget.moc"

// vim: sw=4 sts=4 et tw=100
