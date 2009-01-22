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

#include <KDebug>

#include "ui_ipv4.h"

#include "connection.h"
#include "settings/ipv4.h"

class IpV4Widget::Private
{
public:
    Ui_SettingsIp4Config ui;
    Knm::Ipv4Setting * setting;
};

IpV4Widget::IpV4Widget(Knm::Connection * connection, QWidget * parent)
    : SettingWidget(connection, parent), d(new IpV4Widget::Private)
{
    d->ui.setupUi(this);
    d->setting = static_cast<Knm::Ipv4Setting*>(connection->setting(Knm::Setting::Ipv4));
}

IpV4Widget::~IpV4Widget()
{
    delete d;
}

void IpV4Widget::readConfig()
{
    kDebug();
    switch (d->setting->method()) {
        case Knm::Ipv4Setting::EnumMethod::Automatic:
            d->ui.method->setCurrentIndex(0);
            break;
        case Knm::Ipv4Setting::EnumMethod::LinkLocal:
            d->ui.method->setCurrentIndex(1);
            break;
        case Knm::Ipv4Setting::EnumMethod::Manual:
            d->ui.method->setCurrentIndex(2);
            break;
        case Knm::Ipv4Setting::EnumMethod::Shared:
            d->ui.method->setCurrentIndex(3);
            break;
        default:
            kDebug() << "Unrecognised value for method:" << d->setting->method();
            break;
    }
    // TODO a lot, for ip addresses, routes etc
}

void IpV4Widget::writeConfig()
{
    // save method
    switch ( d->ui.method->currentIndex()) {
        case 0:
            d->setting->setMethod(Knm::Ipv4Setting::EnumMethod::Automatic);
            break;
        case 1:
            d->setting->setMethod(Knm::Ipv4Setting::EnumMethod::LinkLocal);
            break;
        case 2:
            d->setting->setMethod(Knm::Ipv4Setting::EnumMethod::Manual);
            break;
        case 3:
            d->setting->setMethod(Knm::Ipv4Setting::EnumMethod::Shared);
            break;
        default:
            kDebug() << "Unrecognised combo box index for method:" << d->ui.method->currentIndex();
            break;
    }
}

// vim: sw=4 sts=4 et tw=100
