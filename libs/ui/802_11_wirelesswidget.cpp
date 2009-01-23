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

#include <KDebug>

#include "scanwidget.h"
#include "connection.h"
#include "ui_802-11-wireless.h"
#include "settings/802-11-wireless.h"

const QString Wireless80211Widget::INFRA_MODE = QLatin1String("infrastructure");
const QString Wireless80211Widget::ADHOC_MODE = QLatin1String("adhoc");

class Wireless80211Widget::Private
{
public:
    Ui_Wireless80211Config ui;
    Knm::WirelessSetting * setting;
};

Wireless80211Widget::Wireless80211Widget(Knm::Connection* connection, const QString &ssid, QWidget * parent)
    : SettingWidget(connection, parent), d(new Wireless80211Widget::Private)
{
    d->ui.setupUi(this);
    d->setting = static_cast<Knm::WirelessSetting *>(connection->setting(Knm::Setting::Wireless));
    d->setting->setSsid(ssid.toAscii());
    
    connect(d->ui.btnScan, SIGNAL(clicked()), SLOT(scanClicked()));
}

Wireless80211Widget::~Wireless80211Widget()
{
    delete d;
}

void Wireless80211Widget::readConfig()
{
    kDebug();

    switch(d->setting->mode())
    {
        case Knm::WirelessSetting::EnumMode::adhoc:
            d->ui.cmbMode->setCurrentIndex(1);
            break;
        case Knm::WirelessSetting::EnumMode::infrastructure:
        default:
            d->ui.cmbMode->setCurrentIndex(0);
      }
    d->ui.kcfg_ssid->setText(QString(d->setting->ssid()));
}

void Wireless80211Widget::writeConfig()
{
    kDebug();

    d->setting->setSsid(d->ui.kcfg_ssid->text().toAscii());
    switch ( d->ui.cmbMode->currentIndex()) {
        case 0:
            d->setting->setMode(Knm::WirelessSetting::EnumMode::infrastructure);
            break;
        case 1:
            d->setting->setMode(Knm::WirelessSetting::EnumMode::adhoc);
            break;
    }
}

void Wireless80211Widget::scanClicked()
{
    KDialog scanDialog;
    scanDialog.setCaption(i18n("Available Access Points"));
    scanDialog.setButtons( KDialog::Ok | KDialog::Cancel);
    ScanWidget scanWid;
    scanDialog.setMainWidget(&scanWid);
    if (scanDialog.exec() == QDialog::Accepted) {
        d->ui.kcfg_ssid->setText(scanWid.currentAccessPoint());
    }
}

// vim: sw=4 sts=4 et tw=100
