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
#include "settingwidget_p.h"

#include <KDebug>

#include <QNetworkAddressEntry>

#include "ui_ipv4.h"

#include "connection.h"
#include "settings/ipv4.h"
#include "simpleipv4addressvalidator.h"
#include "listvalidator.h"
#include "editlistdialog.h"

//void removeEmptyItems(QStringList &list);

class IpV4WidgetPrivate : public SettingWidgetPrivate
{
public:
    IpV4WidgetPrivate() : setting(0), isAdvancedModeOn(false)
    {
    }

    // Make sure that this order is same as the combobox shown in ipv4.ui file
    enum MethodIndex { AutomaticMethodIndex = 0, AutomaticOnlyIPMethodIndex, ManualMethodIndex, LinkLocalMethodIndex, SharedMethodIndex, DisabledMethodIndex};
    Ui_SettingsIp4Config ui;
    Knm::Ipv4Setting * setting;
    bool isAdvancedModeOn;
};

IpV4Widget::IpV4Widget(Knm::Connection * connection, QWidget * parent)
    : SettingWidget(*new IpV4WidgetPrivate, connection, parent)
{
    Q_D(IpV4Widget);
    d->ui.setupUi(this);
    for(int index=0; index < d->ui.stackedWidget->count(); ++index) {
        d->ui.stackedWidget->widget(index)->layout()->setMargin(0);
    }

    QString str_auto;
    QString str_auto_only;
    Knm::Connection::Type connType = connection->type();

    kDebug() << connection->name() << connection->uuid().toString() << connection->typeAsString(connection->type());

    if (Knm::Connection::Vpn == connType) {
        str_auto = i18nc("@item:inlistbox IPv4 settings configuration method",
                         "Automatic (VPN)");
        str_auto_only = i18nc("@item:inlistbox IPv4 settings configuration method",
                              "Automatic (VPN) addresses only");
    }
    else if (Knm::Connection::Gsm == connType
             || Knm::Connection::Cdma == connType) {
        str_auto = i18nc("@item:inlistbox IPv4 settings configuration method",
                         "Automatic (PPP)");
        str_auto_only = i18nc("@item:inlistbox IPv4 settings configuration method",
                              "Automatic (PPP) addresses only");
    }
    else if (Knm::Connection::Pppoe == connType) {
        str_auto = i18nc("@item:inlistbox IPv4 settings configuration method",
                         "Automatic (DSL)");
        str_auto_only = i18nc("@item:inlistbox IPv4 settings configuration method",
                              "Automatic (DSL) addresses only");
    }
    else {
        str_auto = i18nc("@item:inlistbox IPv4 settings configuration method",
                         "Automatic (DHCP)");
        str_auto_only = i18nc("@item:inlistbox IPv4 settings configuration method",
                              "Automatic (DHCP) addresses only");
    }
    d->ui.method->setItemText(0, str_auto);
    d->ui.method->setItemText(1, str_auto_only);

    d->ui.address->setValidator(new SimpleIpV4AddressValidator(this));
    // unable to check netmask strictly until user finish the input
    d->ui.netMask->setValidator(new SimpleIpV4AddressValidator(this));
    d->ui.gateway->setValidator(new SimpleIpV4AddressValidator(this));

    ListValidator *dnsEntriesValidator = new ListValidator(this);
    dnsEntriesValidator->setInnerValidator(new SimpleIpV4AddressValidator(dnsEntriesValidator));
    d->ui.dns->setValidator(dnsEntriesValidator);

    ListValidator *dnsSearchEntriesValidator = new ListValidator(this);
    dnsSearchEntriesValidator->setInnerValidator(new QRegExpValidator(QRegExp("\\S+"), this));
    d->ui.dnsSearch->setValidator(dnsSearchEntriesValidator);

    connect(d->ui.address, SIGNAL(editingFinished()), this, SLOT(addressEditingFinished()));

    connect(d->ui.dnsMorePushButton, SIGNAL(clicked()), this, SLOT(showDnsEditor()));
    connect(d->ui.dnsSearchMorePushButton, SIGNAL(clicked()), this, SLOT(showDnsSearchEditor()));

    d->setting = static_cast<Knm::Ipv4Setting*>(connection->setting(Knm::Setting::Ipv4));

    kDebug() << "Method is" << d->setting->method() << d->setting->dhcpclientid();

    connect(d->ui.method, SIGNAL(currentIndexChanged(int)), this, SLOT(methodChanged(int)));
    if (d->setting->method() == Knm::Ipv4Setting::EnumMethod::Shared) {
        methodChanged(d->SharedMethodIndex);
    } else {
        methodChanged(d->AutomaticMethodIndex);
    }
}

IpV4Widget::~IpV4Widget()
{
}

void IpV4Widget::readConfig()
{
    kDebug() << "Reading IPv4 settings...";

    Q_D(IpV4Widget);
    // The following flags are used to not fill disabled fields.
    // Setting and handling them is quite redundant, but it's necessary
    // when we have a connection config in inconsistent state.
    bool addressPartEnabled = false;
    bool advancedSettingsPartEnabled = false;
    bool dnsPartEnabled = false;

    switch (d->setting->method()) {
        case Knm::Ipv4Setting::EnumMethod::Automatic:
            kDebug() << "Method: Automatic";
            if (d->setting->ignoredhcpdns()) {
                d->ui.method->setCurrentIndex(d->AutomaticOnlyIPMethodIndex);
                dnsPartEnabled = true;
            }
            else {
                d->ui.method->setCurrentIndex(d->AutomaticMethodIndex);
                dnsPartEnabled = true;
            }
            advancedSettingsPartEnabled = true;
            break;
        case Knm::Ipv4Setting::EnumMethod::LinkLocal:
            kDebug() << "Method: LinkLocal";
            d->ui.method->setCurrentIndex(d->LinkLocalMethodIndex);
            break;
        case Knm::Ipv4Setting::EnumMethod::Manual:
            kDebug() << "Method: Manual";
            d->ui.method->setCurrentIndex(d->ManualMethodIndex);
            advancedSettingsPartEnabled = addressPartEnabled = dnsPartEnabled = true;
            break;
        case Knm::Ipv4Setting::EnumMethod::Shared:
            kDebug() << "Method: Shared";
            d->ui.method->setCurrentIndex(d->SharedMethodIndex);
            break;
        case Knm::Ipv4Setting::EnumMethod::Disabled:
            kDebug() << "Method: Shared";
            d->ui.method->setCurrentIndex(d->DisabledMethodIndex);
            break;
        default:
            kDebug() << "Unrecognised value for method:" << d->setting->method();
            break;
    }

    // ip addresses
    if (advancedSettingsPartEnabled) {
        QList<Solid::Control::IPv4AddressNm09> addrList = d->setting->addresses();
        if (!addrList.isEmpty())
        {
            if (addressPartEnabled)
            {
                // show only the first IP address, the rest addresses will be shown
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
            }
             // put the rest to advanced settings
            d->ui.advancedSettings->setAdditionalAddresses(addrList);
        }
    }

    // dns
    if (dnsPartEnabled) {
        QStringList dnsList;
        foreach (const QHostAddress &dns, d->setting->dns()) {
           dnsList << dns.toString();
        }
        d->ui.dns->setText(dnsList.join(QLatin1String(", ")));
        // dns search list
        if (!d->setting->dnssearch().isEmpty()) {
            d->ui.dnsSearch->setText(d->setting->dnssearch().join(QLatin1String(", ")));
        }
    }
    // dhcp client ID
    d->ui.dhcpClientId->setText(d->setting->dhcpclientid());

    // routing
    if (advancedSettingsPartEnabled)
    {
        d->ui.routesSettings->setNeverDefault(d->setting->neverdefault());
        d->ui.routesSettings->setIgnoreAutoRoutes(d->setting->ignoreautoroute());
        d->ui.routesSettings->setRoutes(d->setting->routes());
    }

    //required or not
    d->ui.cbMayFail->setChecked(!d->setting->mayfail());
}

void IpV4Widget::writeConfig()
{
    Q_D(IpV4Widget);
    // save method
    switch ( d->ui.method->currentIndex()) {
        case IpV4WidgetPrivate::AutomaticOnlyIPMethodIndex:
            d->setting->setMethod(Knm::Ipv4Setting::EnumMethod::Automatic);
            d->setting->setIgnoredhcpdns(true);
            break;
        case IpV4WidgetPrivate::AutomaticMethodIndex:
            d->setting->setMethod(Knm::Ipv4Setting::EnumMethod::Automatic);
            d->setting->setIgnoredhcpdns(false);
            break;
        case IpV4WidgetPrivate::LinkLocalMethodIndex:
            d->setting->setMethod(Knm::Ipv4Setting::EnumMethod::LinkLocal);
            d->setting->setIgnoredhcpdns(false);
            break;
        case IpV4WidgetPrivate::ManualMethodIndex:
            d->setting->setMethod(Knm::Ipv4Setting::EnumMethod::Manual);
            d->setting->setIgnoredhcpdns(false);
            break;
        case IpV4WidgetPrivate::SharedMethodIndex:
            d->setting->setMethod(Knm::Ipv4Setting::EnumMethod::Shared);
            d->setting->setIgnoredhcpdns(false);
            break;
        case IpV4WidgetPrivate::DisabledMethodIndex:
            d->setting->setMethod(Knm::Ipv4Setting::EnumMethod::Disabled);
            d->setting->setIgnoredhcpdns(false);
            break;
        default:
            kDebug() << "Unrecognised combo box index for method:" << d->ui.method->currentIndex();
            break;
    }

    // addresses
    QList<Solid::Control::IPv4AddressNm09> addresses = d->ui.advancedSettings->additionalAddresses();
    // update only the first item, the rest items are already updated
    QNetworkAddressEntry entry;
    // we need to set up IP before prefix/netmask manipulation
    entry.setIp(QHostAddress(d->ui.address->text()));
    entry.setNetmask(QHostAddress(d->ui.netMask->text()));

    QHostAddress gateway(d->ui.gateway->text());
    if (entry.ip() != QHostAddress::Null) {
        Solid::Control::IPv4AddressNm09 addr(entry.ip().toIPv4Address(),
                                         entry.prefixLength(), gateway.toIPv4Address());

        addresses.prepend(addr);
    }

    d->setting->setAddresses(addresses);


    QList<QHostAddress> dnsList;
    QStringList dnsSearchEntries;

    // dns
    QString tempStr = d->ui.dns->text().remove(QLatin1Char(' '));
    QStringList dnsInput = tempStr.split(QLatin1Char(','), QString::SkipEmptyParts);
    foreach (const QString &dns, dnsInput) {
        QHostAddress dnsAddr(dns);
        if (dnsAddr != QHostAddress::Null) {
            //kDebug() << "Address parses to: " << dnsAddr.toString();
            dnsList << dnsAddr;
        }
    }
    // dns search list
    if (!d->ui.dnsSearch->text().isEmpty()) {
        QString tempStr = d->ui.dnsSearch->text().remove(QLatin1Char(' '));
        dnsSearchEntries = tempStr.split(QLatin1Char(','), QString::SkipEmptyParts);
    }

    d->setting->setDns(dnsList);
    d->setting->setDnssearch(dnsSearchEntries);

    // dhcp client ID
    d->setting->setDhcpclientid(d->ui.dhcpClientId->text());
    // routing
    d->setting->setNeverdefault(d->ui.routesSettings->neverdefault());
    d->setting->setIgnoreautoroute(d->ui.routesSettings->ignoreautoroutes());
    d->setting->setRoutes(d->ui.routesSettings->routes());

    //required or not
    d->setting->setMayfail(!d->ui.cbMayFail->isChecked());
}

void IpV4Widget::methodChanged(int currentIndex)
{
    Q_D(IpV4Widget);
    bool addressPartEnabled = false;
    bool advancedSettingsPartEnabled = true;
    bool dnsPartEnabled = false;
    bool dhcpClientIdEnabled = false;
    bool methodAuto = false;

    if (IpV4WidgetPrivate::ManualMethodIndex == currentIndex) {
        addressPartEnabled = true;
        dnsPartEnabled = true;
    } else if (IpV4WidgetPrivate::AutomaticOnlyIPMethodIndex == currentIndex) {
        dnsPartEnabled = true;
        dhcpClientIdEnabled = true;
    } else if (IpV4WidgetPrivate::AutomaticMethodIndex == currentIndex) {
        dnsPartEnabled = true;
        dhcpClientIdEnabled = true;
        methodAuto = true;
    }
    else {
        advancedSettingsPartEnabled = false;
    }

    if (!addressPartEnabled && advancedSettingsPartEnabled)
    {
        QList<Solid::Control::IPv4AddressNm09> addresses = d->ui.advancedSettings->additionalAddresses();
        QNetworkAddressEntry entry;
        // we need to set up IP before prefix/netmask manipulation
        entry.setIp(QHostAddress(d->ui.address->text()));
        entry.setNetmask(QHostAddress(d->ui.netMask->text()));

        QHostAddress gateway(d->ui.gateway->text());
        if (entry.ip() != QHostAddress::Null)
        {
            Solid::Control::IPv4AddressNm09 addr(entry.ip().toIPv4Address(),
                                            entry.prefixLength(), gateway.toIPv4Address());
            addresses.prepend(addr);
        }
        d->ui.advancedSettings->setAdditionalAddresses(addresses);
    }
    else if (addressPartEnabled && advancedSettingsPartEnabled)
    {
        QList<Solid::Control::IPv4AddressNm09> addresses = d->ui.advancedSettings->additionalAddresses();
        if (!addresses.isEmpty())
        {
            Solid::Control::IPv4AddressNm09 addr = addresses.takeFirst();
            QNetworkAddressEntry entry;
            // we need to set up IP before prefix/netmask manipulation
            entry.setIp(QHostAddress(addr.address()));
            entry.setPrefixLength(addr.netMask());
            kDebug()<<entry.netmask().toString();
            QHostAddress gateway(addr.gateway());

            d->ui.address->setText(entry.ip().toString());
            d->ui.netMask->setText(entry.netmask().toString());
            d->ui.gateway->setText(gateway.toString());

            d->ui.advancedSettings->setAdditionalAddresses(addresses);
        }
    }
    if (!addressPartEnabled)
    {
        d->ui.address->clear();
        d->ui.netMask->clear();
        d->ui.gateway->clear();
    }
    if (!advancedSettingsPartEnabled)
    {
        d->ui.advancedSettings->setAdditionalAddresses(QList<Solid::Control::IPv4AddressNm09>());
    }
    if (methodAuto) {
        d->ui.dnsLabel->setText(i18nc("@info","Additional &DNS Servers:"));
        d->ui.dnsSearchLabel->setText(i18nc("@info","Additional &Search Domains:"));
    } else {
        d->ui.dnsLabel->setText(i18nc("@info","&DNS Servers:"));
        d->ui.dnsSearchLabel->setText(i18nc("@info","&Search Domains:"));
    }
    d->ui.advancedSettings->setEnabled(advancedSettingsPartEnabled);
    d->ui.routesSettings->setEnabled(advancedSettingsPartEnabled);
    d->ui.address->setEnabled(addressPartEnabled);
    d->ui.addressLabel->setEnabled(addressPartEnabled);
    d->ui.netMask->setEnabled(addressPartEnabled);
    d->ui.netMaskLabel->setEnabled(addressPartEnabled);
    d->ui.gateway->setEnabled(addressPartEnabled);
    d->ui.gatewayLabel->setEnabled(addressPartEnabled);

    if (!dnsPartEnabled) {
        d->ui.dns->clear();
        d->ui.dnsSearch->clear();
    }
    d->ui.dns->setEnabled(dnsPartEnabled);
    d->ui.dnsLabel->setEnabled(dnsPartEnabled);
    d->ui.dnsSearch->setEnabled(dnsPartEnabled);
    d->ui.dnsSearchLabel->setEnabled(dnsPartEnabled);
    d->ui.dnsSearchMorePushButton->setEnabled(dnsPartEnabled);
    d->ui.dnsMorePushButton->setEnabled(dnsPartEnabled);

    d->ui.labelDhcpClientId->setEnabled(dhcpClientIdEnabled);
    d->ui.dhcpClientId->setEnabled(dhcpClientIdEnabled);
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
    Q_D(IpV4Widget);
    if (d->ui.netMask->text().isEmpty()) {
        QHostAddress addr(d->ui.address->text());
        quint32 netmask = suggestNetmask(addr.toIPv4Address());
        if (netmask) {
            QHostAddress v(netmask);
            d->ui.netMask->setText(v.toString());
        }
    }
}

void IpV4Widget::dnsEdited(QStringList items)
{
    Q_D(IpV4Widget);
    d->ui.dns->setText(items.join(QLatin1String(", ")));
}

void IpV4Widget::dnsSearchEdited(QStringList items)
{
    Q_D(IpV4Widget);
    d->ui.dnsSearch->setText(items.join(QLatin1String(", ")));
}

void IpV4Widget::showDnsEditor()
{
    Q_D(IpV4Widget);
    EditListDialog * dnsEditor = new EditListDialog;
    // at first remove space characters
    QString dnsEntries = d->ui.dns->text().remove(QLatin1Char(' '));
    dnsEditor->setItems(dnsEntries.split(QLatin1Char(','), QString::SkipEmptyParts));
    connect(dnsEditor, SIGNAL(itemsEdited(QStringList)), this, SLOT(dnsEdited(QStringList)));
    dnsEditor->setCaption(i18n("DNS Servers"));
    dnsEditor->setModal(true);
    dnsEditor->setValidator(new SimpleIpV4AddressValidator(dnsEditor));
    dnsEditor->show();
}

void IpV4Widget::showDnsSearchEditor()
{
    Q_D(IpV4Widget);
    EditListDialog * dnsSearchEditor = new EditListDialog;
    // at first remove space characters
    QString dnsSearchEntries = d->ui.dnsSearch->text().remove(QLatin1Char(' '));
    dnsSearchEditor->setItems(dnsSearchEntries.split(QLatin1Char(','), QString::SkipEmptyParts));
    connect(dnsSearchEditor, SIGNAL(itemsEdited(QStringList)), this, SLOT(dnsSearchEdited(QStringList)));
    dnsSearchEditor->setCaption(i18n("Search domains"));
    dnsSearchEditor->setModal(true);
    dnsSearchEditor->show();
}

void IpV4Widget::setDns(const QList<QVariant> dnsList)
{
    if (dnsList.isEmpty()) {
        return;
    }

    Q_D(IpV4Widget);
    QList<QHostAddress> temp;
    foreach (const QVariant &dns, dnsList) {
        QHostAddress dnsAddr(dns.toString());
        if (dnsAddr != QHostAddress::Null) {
            temp << dnsAddr;
        }
    }

    d->setting->setMethod(Knm::Ipv4Setting::EnumMethod::Automatic);
    d->setting->setIgnoredhcpdns(true);
    d->setting->setDns(temp);
    readConfig();
}

void IpV4Widget::validate()
{

}
// vim: sw=4 sts=4 et tw=100
