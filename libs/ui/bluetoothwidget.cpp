/*
Copyright 2011 Lamarque Souza <lamarque@gmail.com>

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

#include <nm-setting-bluetooth.h>

#include "bluetoothwidget.h"
#include "settingwidget_p.h"
#include "ui_bluetooth.h"
#include "connection.h"
#include "settings/bluetooth.h"

#include <uiutils.h>

class BluetoothWidgetPrivate : public SettingWidgetPrivate
{
public:
    Ui_Bluetooth ui;
    Knm::BluetoothSetting * setting;
};

BluetoothWidget::BluetoothWidget(Knm::Connection * connection, QWidget * parent)
: SettingWidget(*new BluetoothWidgetPrivate, connection, parent)
{
    Q_D(BluetoothWidget);
    d->ui.setupUi(this);
    d->setting = static_cast<Knm::BluetoothSetting *>(connection->setting(Knm::Setting::Bluetooth));
    d->ui.networkname->setEnabled(false);
    d->ui.bdaddr->setEnabled(false);
    d->ui.networktype->setEnabled(false);

    d->ui.networktype->setItemData(DunIndex, QString(NM_SETTING_BLUETOOTH_TYPE_DUN));
    d->ui.networktype->setItemData(PanuIndex, QString(NM_SETTING_BLUETOOTH_TYPE_PANU));
}

BluetoothWidget::~BluetoothWidget()
{
}

void BluetoothWidget::readConfig()
{
    Q_D(BluetoothWidget);
    d->ui.networkname->setText(d->setting->networkname());
    d->ui.bdaddr->setText(UiUtils::macAddressAsString(d->setting->bdaddr()));
    d->ui.networktype->setCurrentIndex(networktypeIndexFromString(d->setting->networktype()));
}

void BluetoothWidget::writeConfig()
{
    Q_D(BluetoothWidget);
    d->setting->setNetworkname(d->ui.networkname->text());
    d->setting->setBdaddr(UiUtils::macAddressFromString(d->ui.bdaddr->text()));
    d->setting->setNetworktype(d->ui.networktype->itemData(d->ui.networktype->currentIndex()).toString());
}

/*void BluetoothWidget::readSecrets()
{
}*/

void BluetoothWidget::validate()
{
}

BluetoothWidget::NetworktypeIndex BluetoothWidget::networktypeIndexFromString(const QString &networktype) const
{
    if (networktype == NM_SETTING_BLUETOOTH_TYPE_PANU) {
        return PanuIndex;
    }
    return DunIndex;
}

// vim: sw=4 sts=4 et tw=100
