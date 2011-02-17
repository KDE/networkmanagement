/*
Copyright 2008,2010 Will Stephenson <wstephenson@kde.org>

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
#include "settingwidget_p.h"

#include <KDebug>
#include <kdeversion.h>

#include <Solid/Device>
#include <solid/control/networkmanager.h>
#include <solid/control/wirelessnetworkinterface.h>

#include <connection.h>
#include <settings/802-11-wireless.h>
#include <uiutils.h>

#include "scanwidget.h"
#include "ui_802-11-wireless.h"


class Wireless80211WidgetPrivate : public SettingWidgetPrivate
{
public:
    Ui_Wireless80211Config ui;
    Knm::WirelessSetting * setting;
    QString originalSsid;
    QString proposedSsid;
};

Wireless80211Widget::Wireless80211Widget(Knm::Connection* connection, const QString &ssid, QWidget * parent)
    : SettingWidget(*new Wireless80211WidgetPrivate, connection, parent)
{
    Q_D(Wireless80211Widget);
    d->ui.setupUi(this);
    d->valid = false; //until there is a SSID at least
    // setup validation
    connect(d->ui.ssid, SIGNAL(textChanged(const QString&)), SLOT(validate()));
    d->proposedSsid = ssid;
    d->setting = static_cast<Knm::WirelessSetting *>(connection->setting(Knm::Setting::Wireless));
    d->ui.ssid->setText(d->proposedSsid);

    d->ui.mtu->setSuffix(ki18np(" byte", " bytes"));
    connect(d->ui.btnScan, SIGNAL(clicked()), SLOT(scanClicked()));
    foreach (Solid::Control::NetworkInterface * iface, Solid::Control::NetworkManager::networkInterfaces()) {
        if (iface->type() == Solid::Control::NetworkInterface::Ieee80211) {

            Solid::Control::WirelessNetworkInterface * wiface = static_cast<Solid::Control::WirelessNetworkInterface*>(iface);
            d->ui.cmbMacAddress->addItem(UiUtils::interfaceNameLabel(iface->uni()), wiface->hardwareAddress().toLatin1());
        }
    }
}

Wireless80211Widget::~Wireless80211Widget()
{
}

void Wireless80211Widget::readConfig()
{
    Q_D(Wireless80211Widget);
    //kDebug() << kBacktrace();
    switch(d->setting->mode())
    {
        case Knm::WirelessSetting::EnumMode::adhoc:
            d->ui.cmbMode->setCurrentIndex(1);
            break;
        case Knm::WirelessSetting::EnumMode::infrastructure:
        default:
            d->ui.cmbMode->setCurrentIndex(0);
      }
    // need to check that ssids containing international characters are restored correctly
    if (!d->setting->ssid().isEmpty()) {
        d->ui.ssid->setText(QString::fromAscii(d->setting->ssid()));
        d->originalSsid = QString::fromAscii(d->setting->ssid());
    }
    d->ui.bssid->setText(QString::fromAscii(d->setting->bssid()));
    if (!d->setting->macaddress().isEmpty()) {
        int i = d->ui.cmbMacAddress->findData(d->setting->macaddress());
        if (i == -1) {
            d->ui.cmbMacAddress->addItem(i18nc("@item:inlist item for hardware that is currently not attached to the machine with MAC address", "Disconnected interface (%1)", QLatin1String(d->setting->macaddress())));
            d->ui.cmbMacAddress->setCurrentIndex(d->ui.cmbMacAddress->count() - 1);
        } else {
            d->ui.cmbMacAddress->setCurrentIndex(i);
        }
    }
    d->ui.mtu->setValue(d->setting->mtu());
}

void Wireless80211Widget::writeConfig()
{
    Q_D(Wireless80211Widget);
    kDebug();

    d->setting->setSsid(d->ui.ssid->text().toAscii());
    kDebug() << d->setting->ssid();
    switch ( d->ui.cmbMode->currentIndex()) {
        case 0:
            d->setting->setMode(Knm::WirelessSetting::EnumMode::infrastructure);
            break;
        case 1:
            d->setting->setMode(Knm::WirelessSetting::EnumMode::adhoc);
            break;
    }

    int i = d->ui.cmbMacAddress->currentIndex();
    if ( i == 0) {
        d->setting->setMacaddress(QByteArray());
    } else {
        d->setting->setMacaddress(d->ui.cmbMacAddress->itemData(i).toByteArray());
    }

    if (d->ui.bssid->text() != QString::fromLatin1(":::::")) {
        d->setting->setBssid(d->ui.bssid->text().toAscii());
    } else {
        d->setting->setBssid(QByteArray());
    }
    d->setting->setMtu(d->ui.mtu->value());
}

void Wireless80211Widget::scanClicked()
{
    Q_D(Wireless80211Widget);
    KDialog scanDialog;
    scanDialog.setCaption(i18nc("@title:window wireless network scan dialog",
                "Available Networks"));
    scanDialog.setButtons( KDialog::Ok | KDialog::Cancel);
    ScanWidget scanWid;
    scanDialog.setMainWidget(&scanWid);
    if (scanDialog.exec() == QDialog::Accepted) {
        d->ui.ssid->setText(scanWid.currentAccessPoint());
	emit ssidSelected(scanWid.currentAccessPoint());
    }
}

QByteArray Wireless80211Widget::selectedInterfaceHardwareAddress() const
{
    Q_D(const Wireless80211Widget);
    QByteArray hwAddr;
    int i = d->ui.cmbMacAddress->currentIndex();
    if ( i != 0) {
        hwAddr = d->ui.cmbMacAddress->itemData(i).toByteArray();
    }
    return hwAddr;
}

QString Wireless80211Widget::enteredSsid() const
{
    Q_D(const Wireless80211Widget);
    return d->ui.ssid->text();
}

bool Wireless80211Widget::enteredSsidIsDirty() const
{
    Q_D(const Wireless80211Widget);
    return (d->ui.ssid->text() != d->originalSsid);
}

void Wireless80211Widget::setEnteredSsidClean()
{
    Q_D(Wireless80211Widget);
    d->originalSsid = d->ui.ssid->text();
}

void Wireless80211Widget::validate()
{
    Q_D(Wireless80211Widget);
    d->valid = (d->ui.ssid->text().length() > 0 && d->ui.ssid->text().length() < 33);
    emit valid(d->valid);
}
// vim: sw=4 sts=4 et tw=100
