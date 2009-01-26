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
    connect(d->ui.btnAddAddress, SIGNAL(clicked()), this, SLOT(addIpClicked()));
    connect(d->ui.btnRemoveAddress, SIGNAL(clicked()), this, SLOT(removeIpClicked()));
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

    // ip addresses
    QList<QTreeWidgetItem*> items;
    QList<Solid::Control::IPv4Address> addrList = d->setting->addresses();
    foreach (Solid::Control::IPv4Address addr, d->setting->addresses()) {
        QStringList fields;
        fields << QHostAddress(addr.address()).toString() << QString::number(addr.netMask()) << QHostAddress(addr.gateway()).toString();
        QTreeWidgetItem * item = new QTreeWidgetItem(d->ui.addresses, fields);
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
        items.append(item);
    }

    // dns
    QStringList dnsList;
    foreach (QHostAddress dns, d->setting->dns()) {
       dnsList << dns.toString();
    }
    d->ui.dns->setText(dnsList.join(","));
    // dns search list
    d->ui.dnsSearch->setText(d->setting->dnssearch().join(","));
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

    // addresses
    QList<Solid::Control::IPv4Address> addresses;
    while (QTreeWidgetItem* item = d->ui.addresses->takeTopLevelItem(0)) {
        QHostAddress ip(item->text(0));
        QHostAddress gateway(item->text(2));
        if (ip == QHostAddress::Null
                || gateway == QHostAddress::Null) {
            continue;
        }
        Solid::Control::IPv4Address addr(ip.toIPv4Address(), item->text(1).toUInt(), gateway.toIPv4Address());
        addresses.append(addr);
    }
    d->setting->setAddresses(addresses);

    // dns
    QList<QHostAddress> dnsList;
    QStringList dnsInput = d->ui.dns->text().split(',');
    foreach (QString dns, dnsInput) {
        QHostAddress dnsAddr(dns);
        if (dnsAddr != QHostAddress::Null) {
            kDebug() << "Address parses to: " << dnsAddr.toString();
            dnsList << dnsAddr;
        }
    }
    d->setting->setDns(dnsList);
    // dns search list
    d->setting->setDnssearch(d->ui.dnsSearch->text().split(','));
}

void IpV4Widget::addIpClicked()
{
    QTreeWidgetItem * item = new QTreeWidgetItem(d->ui.addresses);
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
    item->setSelected(true);
}

void IpV4Widget::removeIpClicked()
{
    QList<QTreeWidgetItem*> items = d->ui.addresses->selectedItems();
    if (items.count()) {
        delete items.first();
    }
}

// vim: sw=4 sts=4 et tw=100
