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

#include "ipv4widget.h"

#include <nm-setting-ip4-config.h>

#include <KDebug>
#include <KConfigSkeletonItem>
#include <KLocale>

#include "configxml.h"
#include "ui_ipv4.h"

class IpV4Widget::Private
{
public:
    Ui_SettingsIp4Config ui;
};

IpV4Widget::IpV4Widget(const QString& connectionId, QWidget * parent)
    : SettingWidget(connectionId, parent), d(new IpV4Widget::Private)
{
    d->ui.setupUi(this);
    init();
}

IpV4Widget::~IpV4Widget()
{
    delete d;
}

QString IpV4Widget::label() const
{
    return i18nc("Label for IPv4 address settings", "IP Address");
}

QString IpV4Widget::settingName() const
{
    return QLatin1String("ipv4");
}

void IpV4Widget::readConfig()
{
    kDebug();
    KConfigSkeletonItem * methodItem = configXml()->findItem(settingName(), QLatin1String(NM_SETTING_IP4_CONFIG_METHOD));
    Q_ASSERT(methodItem);
    QString method = methodItem->property().toString();
    if ( method == QLatin1String(NM_SETTING_IP4_CONFIG_METHOD_AUTO)) {
        d->ui.method->setCurrentIndex(0);
    } else if ( method == QLatin1String(NM_SETTING_IP4_CONFIG_METHOD_LINK_LOCAL)) {
        d->ui.method->setCurrentIndex(1);
    } else if ( method == QLatin1String(NM_SETTING_IP4_CONFIG_METHOD_MANUAL)) {
        d->ui.method->setCurrentIndex(2);
    } else if ( method == QLatin1String(NM_SETTING_IP4_CONFIG_METHOD_SHARED)) {
        d->ui.method->setCurrentIndex(3);
    } else {
        kDebug() << "Unrecognised value for method:" << method;
    }
    // TODO a lot, for ip addresses, routes etc
    //KConfigGroup group(configXml()->config(), settingName());
    //group.readEntry
}

void IpV4Widget::writeConfig()
{
    // save method
    KConfigGroup group(configXml()->config(), settingName());
    switch ( d->ui.method->currentIndex()) {
        case 0:
            group.writeEntry(NM_SETTING_IP4_CONFIG_METHOD, NM_SETTING_IP4_CONFIG_METHOD_AUTO);
            break;
        case 1:
            group.writeEntry(NM_SETTING_IP4_CONFIG_METHOD, NM_SETTING_IP4_CONFIG_METHOD_LINK_LOCAL);
            break;
        case 2:
            group.writeEntry(NM_SETTING_IP4_CONFIG_METHOD, NM_SETTING_IP4_CONFIG_METHOD_MANUAL);
            break;
        case 3:
            group.writeEntry(NM_SETTING_IP4_CONFIG_METHOD, NM_SETTING_IP4_CONFIG_METHOD_SHARED);
            break;
        default:
            kDebug() << "Unrecognised combo box index for method:" << d->ui.method->currentIndex();
            break;
    }
}

// vim: sw=4 sts=4 et tw=100
