/*
Copyright 2008,2009 Will Stephenson <wstephenson@kde.org>

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

#include <QNetworkAddressEntry>

#include "ui_ipv4.h"

#include "connection.h"
#include "settings/ipv4.h"
#include "simpleipv4addressvalidator.h"

class IpV4Widget::Private
{
public:
    Private() : setting(0), isAdvancedModeOn(false)
    {
    }
    enum MethodIndex { AutomaticMethodIndex = 0, LinkLocalMethodIndex, ManualMethodIndex, SharedMethodIndex };
    Ui_SettingsIp4Config ui;
    Knm::Ipv4Setting * setting;
    bool isAdvancedModeOn;
};

IpV4Widget::IpV4Widget(Knm::Connection * connection, QWidget * parent)
    : SettingWidget(connection, parent), d(new IpV4Widget::Private)
{
    d->ui.setupUi(this);

    d->ui.advancedSettings->setVisible(false);
    d->ui.address->setValidator(new SimpleIpV4AddressValidator(this));
    // unable to check netmask strictly until user finish the input
    d->ui.netMask->setValidator(new SimpleIpV4AddressValidator(this));
    d->ui.gateway->setValidator(new SimpleIpV4AddressValidator(this));

    connect(d->ui.address, SIGNAL(editingFinished()), this, SLOT(addressEditingFinished()));

    connect(d->ui.pushButtonSettingsMode, SIGNAL(clicked()), this, SLOT(settingsModeClicked()));

    d->setting = static_cast<Knm::Ipv4Setting*>(connection->setting(Knm::Setting::Ipv4));
    connect(d->ui.method, SIGNAL(currentIndexChanged(int)), this, SLOT(methodChanged(int)));
    methodChanged(d->AutomaticMethodIndex);
    switchSettingsMode();
}

IpV4Widget::~IpV4Widget()
{
    delete d;
}

void IpV4Widget::readConfig()
{
    switch (d->setting->method()) {
        case Knm::Ipv4Setting::EnumMethod::Automatic:
            d->ui.method->setCurrentIndex(d->AutomaticMethodIndex);
            break;
        case Knm::Ipv4Setting::EnumMethod::LinkLocal:
            d->ui.method->setCurrentIndex(d->LinkLocalMethodIndex);
            break;
        case Knm::Ipv4Setting::EnumMethod::Manual:
            d->ui.method->setCurrentIndex(d->ManualMethodIndex);
            break;
        case Knm::Ipv4Setting::EnumMethod::Shared:
            d->ui.method->setCurrentIndex(d->SharedMethodIndex);
            break;
        default:
            kDebug() << "Unrecognised value for method:" << d->setting->method();
            break;
    }

    // ip addresses
    QList<Solid::Control::IPv4Address> addrList = d->setting->addresses();
    if (!addrList.isEmpty())
    {
        // show only the fisrt IP address, the rest addresses will be shown
        // via "Advanced..."
        QNetworkAddressEntry entry;
        // we need to set up IP before prefix/netmask manipulation
        entry.setIp(QHostAddress(addrList[0].address()));
        entry.setPrefixLength(addrList[0].netMask());

        d->ui.address->setText(QHostAddress(addrList[0].address()).toString());
        d->ui.netMask->setText(entry.netmask().toString());
        if (addrList[0].gateway()) {
            d->ui.gateway->setText(QHostAddress(addrList[0].gateway()).toString());
        }

        // remove first item
        addrList.removeFirst();
        // put the rest to advanced settings
        d->ui.advancedSettings->setAdditionalAddresses(addrList);
    }

    // dns
    QStringList dnsList;
    foreach (QHostAddress dns, d->setting->dns()) {
       dnsList << dns.toString();
    }
    d->ui.dns->setText(dnsList.join(","));
    // dns search list
    if (!d->setting->dnssearch().isEmpty()) {
        d->ui.dnsSearch->setText(d->setting->dnssearch().join(","));
    }
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
    QList<Solid::Control::IPv4Address> addresses = d->ui.advancedSettings->additionalAddresses();

    // update only the first item, the rest items are already updated
    QNetworkAddressEntry entry;
    // we need to set up IP before prefix/netmask manipulation
    entry.setIp(QHostAddress(d->ui.address->text()));
    entry.setNetmask(QHostAddress(d->ui.netMask->text()));

    QHostAddress gateway(d->ui.gateway->text());
    if (entry.ip() != QHostAddress::Null) {
        Solid::Control::IPv4Address addr(entry.ip().toIPv4Address(),
                                         entry.prefixLength(), gateway.toIPv4Address());

        addresses.prepend(addr);
    }
    d->setting->setAddresses(addresses);

    // dns
    QList<QHostAddress> dnsList;
    QStringList dnsInput = d->ui.dns->text().split(',');
    foreach (QString dns, dnsInput) {
        QHostAddress dnsAddr(dns);
        if (dnsAddr != QHostAddress::Null) {
            //kDebug() << "Address parses to: " << dnsAddr.toString();
            dnsList << dnsAddr;
        }
    }
    d->setting->setDns(dnsList);
    // dns search list
    if (!d->ui.dnsSearch->text().isEmpty()) {
        d->setting->setDnssearch(d->ui.dnsSearch->text().split(','));
    }
}

void IpV4Widget::methodChanged(int currentIndex)
{
    if (currentIndex == d->AutomaticMethodIndex ||
        currentIndex == d->LinkLocalMethodIndex) {
        d->ui.address->setEnabled(false);
        d->ui.addressLabel->setEnabled(false);
        d->ui.netMask->setEnabled(false);
        d->ui.netMaskLabel->setEnabled(false);
        d->ui.gateway->setEnabled(false);
        d->ui.gatewayLabel->setEnabled(false);
        d->ui.dns->setEnabled(false);
        d->ui.dnsLabel->setEnabled(false);
        d->ui.dnsSearch->setEnabled(false);
        d->ui.dnsSearchLabel->setEnabled(false);
        d->ui.pushButtonSettingsMode->setVisible(false);
    }
    else if (currentIndex == d->ManualMethodIndex) {
        d->ui.address->setEnabled(true);
        d->ui.addressLabel->setEnabled(true);
        d->ui.netMask->setEnabled(true);
        d->ui.netMaskLabel->setEnabled(true);
        d->ui.gateway->setEnabled(true);
        d->ui.gatewayLabel->setEnabled(true);
        d->ui.dns->setEnabled(true);
        d->ui.dnsLabel->setEnabled(true);
        d->ui.dnsSearch->setEnabled(true);
        d->ui.dnsSearchLabel->setEnabled(true);
        d->ui.pushButtonSettingsMode->setVisible(true);
    }
    else if (currentIndex == d->SharedMethodIndex) {
        d->ui.address->setEnabled(false);
        d->ui.addressLabel->setEnabled(false);
        d->ui.netMask->setEnabled(false);
        d->ui.netMaskLabel->setEnabled(false);
        d->ui.gateway->setEnabled(false);
        d->ui.gatewayLabel->setEnabled(false);
        d->ui.dns->setEnabled(true);
        d->ui.dnsLabel->setEnabled(true);
        d->ui.dnsSearch->setEnabled(true);
        d->ui.dnsSearchLabel->setEnabled(true);
        d->ui.pushButtonSettingsMode->setVisible(false);
    }
}

quint32 suggestNetmask(quint32 ip)
{
    /*
        A   0       0.0.0.0 	127.255.255.255  255.0.0.0 	/8
        B   10      128.0.0.0 	191.255.255.255  255.255.0.0 	/16
        C   110     192.0.0.0 	223.255.255.255  255.255.255.0 	/24
        D   1110    224.0.0.0 	239.255.255.255  not defined 	not defined
        E   1111    240.0.0.0 	255.255.255.254  not defined 	not defined
    */
    quint32 netmask = 0;

    if (!(ip & 0x80000000)) {
        // test 0 leading bit
        netmask = 0xFF000000;
    }
    else if (!(ip & 0x40000000)) {
        // test 10 leading bits
        netmask = 0xFFFF0000;
    }
    else if (!(ip & 0x20000000)) {
        // test 110 leading bits
        netmask = 0xFFFFFF00;
    }

    return netmask;
}

void IpV4Widget::addressEditingFinished()
{
    if (d->ui.netMask->text().isEmpty()) {
        QHostAddress addr(d->ui.address->text());
        quint32 netmask = suggestNetmask(addr.toIPv4Address());
        if (netmask) {
            QHostAddress v(netmask);
            d->ui.netMask->setText(v.toString());
        }
    }
}

void IpV4Widget::settingsModeClicked()
{
    d->isAdvancedModeOn ^= true; // XOR toggles value;

    switchSettingsMode();
}

void IpV4Widget::switchSettingsMode()
{
    QString text;

    if (false == d->isAdvancedModeOn) {
        text = i18nc("@action:button Additional IPv4 addresses (aliases)","&Additional Addresses");
    }
    else {
        text = i18nc("@action:button Basic IPv4 settings","&Basic settings");
    }

    d->ui.pushButtonSettingsMode->setText(text);
    d->ui.basicSettingsWidget->setVisible(!d->isAdvancedModeOn);
    d->ui.advancedSettings->setVisible(d->isAdvancedModeOn);
}

// vim: sw=4 sts=4 et tw=100
