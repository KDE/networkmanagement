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
#include <libnm-qt/manager.h>
#include <libnm-qt/wireddevice.h>

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
};

WiredWidget::WiredWidget(Knm::Connection * connection, QWidget * parent)
: SettingWidget(*new WiredWidgetPrivate, connection, parent)
{
    Q_D(WiredWidget);
    d->ui.setupUi(this);
    d->setting = static_cast<Knm::WiredSetting *>(connection->setting(Knm::Setting::Wired));
    d->ui.mtu->setSuffix(ki18np(" byte", " bytes"));
    foreach (NetworkManager::Device * iface, NetworkManager::NetworkManager::networkInterfaces()) {
        if (iface->type() == NetworkManager::Device::Ieee8023) {
            QString deviceText = UiUtils::interfaceNameLabel(iface->uni(), KNetworkManagerServicePrefs::SystemNames);
            NetworkManager::WiredDevice * wired = static_cast<NetworkManager::WiredDevice*>(iface);
            d->ui.cmbMacAddress->addItem(deviceText, wired->hardwareAddress().toLatin1());
        }
    }
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
            d->ui.cmbMacAddress->addItem(i18nc("@item:inlist item for hardware that is currently not attached to the machine with MAC address", "Disconnected interface (%1)", QLatin1String(d->setting->macaddress())));
            d->ui.cmbMacAddress->setCurrentIndex(d->ui.cmbMacAddress->count() - 1);
        } else {
            d->ui.cmbMacAddress->setCurrentIndex(i);
        }
    }
    if (d->setting->mtu()) {
        d->ui.mtu->setValue(d->setting->mtu());
    }
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
}

void WiredWidget::validate()
{

}
// vim: sw=4 sts=4 et tw=100
