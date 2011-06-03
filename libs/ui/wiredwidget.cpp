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

#include "wiredwidget.h"
#include "settingwidget_p.h"

#include <QComboBox>

#include <kdeversion.h>
#include <Solid/Device>
#include <solid/control/networkmanager.h>
#include <solid/control/wirednetworkinterface.h>

#include <connection.h>
#include <settings/802-3-ethernet.h>
#include <uiutils.h>

#include "ui_wired.h"

#include "knmserviceprefs.h"



class WiredWidgetPrivate : public SettingWidgetPrivate
{
public:
    Ui_Settings8023Ethernet ui;
    Knm::WiredSetting * setting;
    enum DuplexIndex {Half = 0, Full = 1};
};

WiredWidget::WiredWidget(Knm::Connection * connection, QWidget * parent)
: SettingWidget(*new WiredWidgetPrivate, connection, parent)
{
    Q_D(WiredWidget);
    d->ui.setupUi(this);
    d->setting = static_cast<Knm::WiredSetting *>(connection->setting(Knm::Setting::Wired));
    d->ui.mtu->setSuffix(ki18np(" byte", " bytes"));
    foreach (Solid::Control::NetworkInterfaceNm09 * iface, Solid::Control::NetworkManagerNm09::networkInterfaces()) {
        if (iface->type() == Solid::Control::NetworkInterfaceNm09::Ethernet) {
            QString deviceText = UiUtils::interfaceNameLabel(iface->uni(), KNetworkManagerServicePrefs::SystemNames);
            Solid::Control::WiredNetworkInterfaceNm09 * wired = static_cast<Solid::Control::WiredNetworkInterfaceNm09*>(iface);
            d->ui.cmbMacAddress->addItem(deviceText, UiUtils::macAddressFromString(wired->permanentHardwareAddress()));
        }
    }
    connect(d->ui.clonedMacAddressRandom, SIGNAL(clicked()), this, SLOT(generateRandomClonedMac()));
}

WiredWidget::~WiredWidget()
{
}

void WiredWidget::readConfig()
{
    Q_D(WiredWidget);
    if (!d->setting->macaddress().isEmpty()) {
        int i = d->ui.cmbMacAddress->findData(d->setting->macaddress());
        if (i == -1) {
            d->ui.cmbMacAddress->addItem(i18nc("@item:inlist item for hardware that is currently not attached to the machine with MAC address", "Disconnected interface (%1)", UiUtils::macAddressAsString(d->setting->macaddress())));
            d->ui.cmbMacAddress->setCurrentIndex(d->ui.cmbMacAddress->count() - 1);
        } else {
            d->ui.cmbMacAddress->setCurrentIndex(i);
        }
    }
    if (!d->setting->clonedmacaddress().isEmpty()) {
        d->ui.clonedMacAddress->setText(UiUtils::macAddressAsString(d->setting->clonedmacaddress()));
    }
    if (d->setting->mtu()) {
        d->ui.mtu->setValue(d->setting->mtu());
    }
    d->ui.speed->setValue(d->setting->speed());
    switch (d->setting->duplex())
    {
        case Knm::WiredSetting::EnumDuplex::half:
            d->ui.cmbDuplex->setCurrentIndex(WiredWidgetPrivate::Half);
            break;
        case Knm::WiredSetting::EnumDuplex::full:
        default:
            d->ui.cmbDuplex->setCurrentIndex(WiredWidgetPrivate::Full);
            break;
    }
    d->ui.chkAutoNegotiate->setChecked(d->setting->autonegotiate());
}

void WiredWidget::writeConfig()
{
    Q_D(WiredWidget);
    d->setting->setMtu(d->ui.mtu->value());
    int i = d->ui.cmbMacAddress->currentIndex();
    if ( i == 0) {
        d->setting->setMacaddress(QByteArray());
    } else {
        d->setting->setMacaddress(d->ui.cmbMacAddress->itemData(i).toByteArray());
    }
    if (d->ui.clonedMacAddress->text() != QString::fromLatin1(":::::")) {
        d->setting->setClonedmacaddress(UiUtils::macAddressFromString(d->ui.clonedMacAddress->text()));
    } else {
        d->setting->setClonedmacaddress(QByteArray());
    }
    d->setting->setSpeed(d->ui.speed->value());
    switch (d->ui.cmbDuplex->currentIndex())
    {
        case WiredWidgetPrivate::Half:
            d->setting->setDuplex(Knm::WiredSetting::EnumDuplex::half);
            break;
        case WiredWidgetPrivate::Full:
        default:
            d->setting->setDuplex(Knm::WiredSetting::EnumDuplex::full);
            break;
    }
    d->setting->setAutonegotiate(d->ui.chkAutoNegotiate->isChecked());
}

void WiredWidget::validate()
{

}

void WiredWidget::generateRandomClonedMac()
{
    Q_D(WiredWidget);
    QByteArray mac;
    mac.resize(6);
    for (int i = 0; i < 6; i++) {
        int random = qrand() % 255;
        mac[i] = random;
    }
    d->ui.clonedMacAddress->setText(UiUtils::macAddressAsString(mac));
}

// vim: sw=4 sts=4 et tw=100
