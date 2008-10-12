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

#include "802_11_wirelesswidget.h"

#include <nm-setting-wireless.h>
#include <KDebug>

#include "configxml.h"

#include "ui_802-11-wireless.h"

const QString Wireless80211Widget::INFRA_MODE = QLatin1String("infrastructure");
const QString Wireless80211Widget::ADHOC_MODE = QLatin1String("adhoc");

class Wireless80211Widget::Private
{
    public:
    Ui_Wireless80211Config ui;
};

Wireless80211Widget::Wireless80211Widget(const QString& connectionId, QWidget * parent) : SettingWidget(connectionId, parent), d(new Wireless80211Widget::Private)
{
    d->ui.setupUi(this);
    init();
}

Wireless80211Widget::~Wireless80211Widget()
{
    delete d;
}

QString Wireless80211Widget::settingName() const
{
    return QLatin1String(NM_SETTING_WIRELESS_SETTING_NAME);
}

void Wireless80211Widget::readConfig()
{
    kDebug();
    KConfigSkeletonItem * item = configXml()->findItem(settingName(), QLatin1String(NM_SETTING_WIRELESS_MODE));
    Q_ASSERT(item);
    QString mode = item->property().toString();
    if ( mode == QLatin1String("infrastructure")) {
        d->ui.cmbMode->setCurrentIndex(0);
    } else if ( mode == QLatin1String("adhoc")) {
        d->ui.cmbMode->setCurrentIndex(1);
    } else if ( !mode.isEmpty()) {
        kDebug() << "Found unrecognised mode value: " << mode;
    }
}

void Wireless80211Widget::writeConfig()
{
    kDebug();
    // save method
    KConfigGroup group(configXml()->config(), settingName());
    switch ( d->ui.cmbMode->currentIndex()) {
        case 0:
            group.writeEntry(NM_SETTING_WIRELESS_MODE, INFRA_MODE);
            break;
        case 1:
            group.writeEntry(NM_SETTING_WIRELESS_MODE, ADHOC_MODE);
            break;
    }
}

// vim: sw=4 sts=4 et tw=100
