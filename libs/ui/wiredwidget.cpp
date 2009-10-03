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

#include <QComboBox>

#include <kdeversion.h>
#include <Solid/Device>
#include <solid/control/networkmanager.h>
#include <solid/control/wirednetworkinterface.h>

#include <connection.h>
#include <settings/802-3-ethernet.h>

#include "ui_wired.h"

#include "knmserviceprefs.h"



class WiredWidget::Private
{
public:
    Ui_Settings8023Ethernet ui;
    Knm::WiredSetting * setting;
};

WiredWidget::WiredWidget(Knm::Connection * connection, QWidget * parent)
: SettingWidget(connection, parent), d(new WiredWidget::Private)
{
    d->ui.setupUi(this);
    d->setting = static_cast<Knm::WiredSetting *>(connection->setting(Knm::Setting::Wired));
    d->ui.mtu->setSuffix(ki18np(" byte", " bytes"));
    foreach (Solid::Control::NetworkInterface * iface, Solid::Control::NetworkManager::networkInterfaces()) {
        if (iface->type() == Solid::Control::NetworkInterface::Ieee8023) {
            Solid::Device * dev = new Solid::Device(iface->uni());
            QString deviceText;
            KNetworkManagerServicePrefs::instance(Knm::ConnectionPersistence::NETWORKMANAGEMENT_RCFILE);
    
            if (KNetworkManagerServicePrefs::self()->interfaceNamingStyle() == KNetworkManagerServicePrefs::DescriptiveNames) {
        
#if KDE_IS_VERSION(4,3,60)
                deviceText = dev->description();
#else
                deviceText = dev->product();
#endif
            } else {                
                deviceText = iface->interfaceName();
            }
            Solid::Control::WiredNetworkInterface * wired = static_cast<Solid::Control::WiredNetworkInterface*>(iface);
            d->ui.cmbMacAddress->addItem(i18nc("@item:inlist Solid Device Name (kernel interface name)", "%1 (%2)", deviceText, wired->interfaceName()), wired->hardwareAddress().toLatin1());
        }
    }
}

WiredWidget::~WiredWidget()
{
    delete d;
}

void WiredWidget::readConfig()
{
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
    d->setting->setMtu(d->ui.mtu->value());
    int i = d->ui.cmbMacAddress->currentIndex();
    if ( i == 0) {
        d->setting->setMacaddress(QByteArray());
    } else {
        d->setting->setMacaddress(d->ui.cmbMacAddress->itemData(i).toByteArray());
    }
}

// vim: sw=4 sts=4 et tw=100
