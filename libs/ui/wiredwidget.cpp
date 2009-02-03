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

#include "connection.h"
#include "ui_wired.h"

#include "settings/802-3-ethernet.h"

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
}

WiredWidget::~WiredWidget()
{
    delete d;
}

void WiredWidget::readConfig()
{
    if (!d->setting->macaddress().isEmpty())
        d->ui.macAddress->setText(d->setting->macaddress());
    d->ui.mtu->setValue(d->setting->mtu());
}

void WiredWidget::writeConfig()
{
    d->setting->setMtu(d->ui.mtu->value());
    if (d->ui.macAddress->text() == QLatin1String(":::::")) {
        d->setting->setMacaddress(QByteArray());
    } else {
        d->setting->setMacaddress(d->ui.macAddress->text().toAscii());
    }
}

// vim: sw=4 sts=4 et tw=100
