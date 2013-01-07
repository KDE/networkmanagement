/*
Copyright 2011 Ilia Kats <ilia-kats@gmx.net>, based on work by Will Stephenson <wstephenson@kde.org>

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

#include "ipv6widget.h"
#include "settingwidget_p.h"

#include <KDebug>

#include <QNetworkAddressEntry>

#include "ui_ipv6.h"

#include "connection.h"
#include "settings/ipv6.h"
#include "simpleipv6addressvalidator.h"
#include "listvalidator.h"
#include "intvalidator.h"
#include "editlistdialog.h"

class IpV6WidgetPrivate : public SettingWidgetPrivate
{
public:
    IpV6WidgetPrivate() : setting(0), isAdvancedModeOn(false)
    {
    }

    // Make sure that this order is same as the combobox shown in ipv4.ui file
    enum MethodIndex { AutomaticMethodIndex = 0, AutomaticAddressesOnlyMethodIndex, DhcpMethodIndex, ManualMethodIndex, LinkLocalMethodIndex, SharedMethodIndex, IgnoreMethodIndex };
    Ui_SettingsIp6Config ui;
    Knm::Ipv6Setting * setting;
    bool isAdvancedModeOn;
};

IpV6Widget::IpV6Widget(Knm::Connection * connection, QWidget * parent)
    : SettingWidget(*new IpV6WidgetPrivate, connection, parent)
{
    Q_D(IpV6Widget);
    d->ui.setupUi(this);
    for(int index=0; index < d->ui.stackedWidget->count(); ++index) {
        d->ui.stackedWidget->widget(index)->layout()->setMargin(0);
    }

    d->ui.address->setValidator(new SimpleIpV6AddressValidator(this));
    d->ui.netMask->setValidator(new IntValidator(0, 128, this));
    d->ui.gateway->setValidator(new SimpleIpV6AddressValidator(this));

    ListValidator *dnsEntriesValidator = new ListValidator(this);
    dnsEntriesValidator->setInnerValidator(new SimpleIpV6AddressValidator(dnsEntriesValidator));
    d->ui.dns->setValidator(dnsEntriesValidator);

    ListValidator *dnsSearchEntriesValidator = new ListValidator(this);
    dnsSearchEntriesValidator->setInnerValidator(new QRegExpValidator(QRegExp("\\S+"), this));
    d->ui.dnsSearch->setValidator(dnsSearchEntriesValidator);

    connect(d->ui.address, SIGNAL(editingFinished()), this, SLOT(addressEditingFinished()));

    connect(d->ui.dnsMorePushButton, SIGNAL(clicked()), this, SLOT(showDnsEditor()));
    connect(d->ui.dnsSearchMorePushButton, SIGNAL(clicked()), this, SLOT(showDnsSearchEditor()));

    d->setting = static_cast<Knm::Ipv6Setting*>(connection->setting(Knm::Setting::Ipv6));

    kDebug() << "Method is" << d->setting->method();

    connect(d->ui.method, SIGNAL(currentIndexChanged(int)), this, SLOT(methodChanged(int)));
    methodChanged(d->AutomaticMethodIndex);
}

IpV6Widget::~IpV6Widget()
{
}

void IpV6Widget::readConfig()
{
    kDebug() << "Reading IPv6 settings...";

    Q_D(IpV6Widget);
    // The following flags are used to not fill disabled fields.
    // Setting and handling them is quite redundant, but it's necessary
    // when we have a connection config in inconsistent state.
    bool addressPartEnabled = false;
    bool advancedSettingsPartEnabled = false;
    bool dnsPartEnabled = false;

    switch (d->setting->method()) {
        case Knm::Ipv6Setting::EnumMethod::Automatic:
            kDebug() << "Method: Automatic";
            if (d->setting->ignoredhcpdns()) {
                d->ui.method->setCurrentIndex(d->AutomaticAddressesOnlyMethodIndex);
                dnsPartEnabled = true;
            } else {
                d->ui.method->setCurrentIndex(d->AutomaticMethodIndex);
                dnsPartEnabled = true;
            }
            advancedSettingsPartEnabled = true;
            break;
        case Knm::Ipv6Setting::EnumMethod::Dhcp:
            kDebug() << "Method: Dhcp";
            d->ui.method->setCurrentIndex(d->DhcpMethodIndex);
            break;
        case Knm::Ipv6Setting::EnumMethod::LinkLocal:
            kDebug() << "Method: LinkLocal";
            d->ui.method->setCurrentIndex(d->LinkLocalMethodIndex);
            break;
        case Knm::Ipv6Setting::EnumMethod::Manual:
            kDebug() << "Method: Manual";
            d->ui.method->setCurrentIndex(d->ManualMethodIndex);
            advancedSettingsPartEnabled = addressPartEnabled = dnsPartEnabled = true;
            break;
        case Knm::Ipv6Setting::EnumMethod::Shared:
            kDebug() << "Method: Shared";
            d->ui.method->setCurrentIndex(d->SharedMethodIndex);
            break;
         case Knm::Ipv6Setting::EnumMethod::Ignore:
            kDebug() << "Method: Ignore";
            d->ui.method->setCurrentIndex(d->IgnoreMethodIndex);
            break;
        default:
            kDebug() << "Unrecognised value for method:" << d->setting->method();
            break;
    }

    // ip addresses
    if (advancedSettingsPartEnabled) {
        QList<Solid::Control::IPv6Address> addrList = d->setting->addresses();
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
                d->ui.netMask->setText(QString::number(entry.prefixLength(),10));
                if (!QHostAddress(addrList[0].gateway()).isNull()) {
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

    // routing
    if (advancedSettingsPartEnabled)
    {
        d->ui.routesSettings->setNeverDefault(d->setting->neverdefault());
        d->ui.routesSettings->setIgnoreAutoRoutes(d->setting->ignoreautoroute());
        d->ui.routesSettings->setRoutes(d->setting->routes());
    }

    //required or not
    d->ui.cbMayFail->setChecked(!d->setting->mayfail());

    // privacy
    const Knm::Ipv6Setting::EnumPrivacy::type privacy = d->setting->privacy();
    if (privacy == Knm::Ipv6Setting::EnumPrivacy::Unknown) {
        d->ui.privacy->setCurrentIndex(Knm::Ipv6Setting::EnumPrivacy::Disabled); // unknown and disabled seem to behave the same
    } else {
        d->ui.privacy->setCurrentIndex(privacy);
    }
}

void IpV6Widget::writeConfig()
{
    Q_D(IpV6Widget);
    // save method
    switch ( d->ui.method->currentIndex()) {
        case IpV6WidgetPrivate::AutomaticMethodIndex:
            d->setting->setMethod(Knm::Ipv6Setting::EnumMethod::Automatic);
            d->setting->setIgnoredhcpdns(false);
            break;
        case IpV6WidgetPrivate::AutomaticAddressesOnlyMethodIndex:
            d->setting->setMethod(Knm::Ipv6Setting::EnumMethod::Automatic);
            d->setting->setIgnoredhcpdns(true);
            break;
        case IpV6WidgetPrivate::DhcpMethodIndex:
            d->setting->setMethod(Knm::Ipv6Setting::EnumMethod::Dhcp);
            d->setting->setIgnoredhcpdns(false);
            break;
        case IpV6WidgetPrivate::LinkLocalMethodIndex:
            d->setting->setMethod(Knm::Ipv6Setting::EnumMethod::LinkLocal);
            d->setting->setIgnoredhcpdns(false);
            break;
        case IpV6WidgetPrivate::ManualMethodIndex:
            d->setting->setMethod(Knm::Ipv6Setting::EnumMethod::Manual);
            d->setting->setIgnoredhcpdns(false);
            break;
        case IpV6WidgetPrivate::SharedMethodIndex:
            d->setting->setMethod(Knm::Ipv6Setting::EnumMethod::Shared);
            d->setting->setIgnoredhcpdns(false);
            break;
        case IpV6WidgetPrivate::IgnoreMethodIndex:
            d->setting->setMethod(Knm::Ipv6Setting::EnumMethod::Ignore);
            d->setting->setIgnoredhcpdns(false);
            break;
        default:
            kDebug() << "Unrecognised combo box index for method:" << d->ui.method->currentIndex();
            break;
    }

    // addresses
    QList<Solid::Control::IPv6Address> addresses = d->ui.advancedSettings->additionalAddresses();
    // update only the first item, the rest items are already updated
    QNetworkAddressEntry entry;
    // we need to set up IP before prefix/netmask manipulation
    entry.setIp(QHostAddress(d->ui.address->text()));
    entry.setPrefixLength(d->ui.netMask->text().toInt());

    QHostAddress gateway(d->ui.gateway->text());
    if (entry.ip() != QHostAddress::Null) {
        Solid::Control::IPv6Address addr(entry.ip().toIPv6Address(),
                                         entry.prefixLength(), gateway.toIPv6Address());
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

    // routing
    d->setting->setNeverdefault(d->ui.routesSettings->neverdefault());
    d->setting->setIgnoreautoroute(d->ui.routesSettings->ignoreautoroutes());
    d->setting->setRoutes(d->ui.routesSettings->routes());

    //required or not
    d->setting->setMayfail(!d->ui.cbMayFail->isChecked());

    // privacy
    d->setting->setPrivacy(static_cast<Knm::Ipv6Setting::EnumPrivacy::type>(d->ui.privacy->currentIndex()));
}

void IpV6Widget::methodChanged(int currentIndex)
{
    Q_D(IpV6Widget);
    bool addressPartEnabled = false;
    bool advancedSettingsPartEnabled = true;
    bool dnsPartEnabled = false;
    bool methodAuto = false;

    if (IpV6WidgetPrivate::ManualMethodIndex == currentIndex) {
        addressPartEnabled = true;
        dnsPartEnabled = true;
    } else if (IpV6WidgetPrivate::AutomaticAddressesOnlyMethodIndex == currentIndex) {
        dnsPartEnabled = true;
    } else if (IpV6WidgetPrivate::AutomaticMethodIndex == currentIndex) {
        dnsPartEnabled = true;
        methodAuto = true;
    } else if (IpV6WidgetPrivate::AutomaticMethodIndex != currentIndex &&
               IpV6WidgetPrivate::DhcpMethodIndex != currentIndex) {
        advancedSettingsPartEnabled = false;
    }

    if (!addressPartEnabled && advancedSettingsPartEnabled)
    {
        QList<Solid::Control::IPv6Address> addresses = d->ui.advancedSettings->additionalAddresses();
        QNetworkAddressEntry entry;
        // we need to set up IP before prefix/netmask manipulation
        entry.setIp(QHostAddress(d->ui.address->text()));
        entry.setPrefixLength(d->ui.netMask->text().toUInt());
        QHostAddress gateway(d->ui.gateway->text());
        if (entry.ip() != QHostAddress::Null)
        {
            Solid::Control::IPv6Address addr(entry.ip().toIPv6Address(),
                                         entry.prefixLength(), gateway.toIPv6Address());
            addresses.prepend(addr);
        }
        d->ui.advancedSettings->setAdditionalAddresses(addresses);
    }
    else if (addressPartEnabled && advancedSettingsPartEnabled)
    {
        QList<Solid::Control::IPv6Address> addresses = d->ui.advancedSettings->additionalAddresses();
        if (!addresses.isEmpty())
        {
            Solid::Control::IPv6Address addr = addresses.takeFirst();
            QNetworkAddressEntry entry;
            // we need to set up IP before prefix/netmask manipulation
            entry.setIp(QHostAddress(addr.address()));
            entry.setPrefixLength(addr.netMask());
            kDebug()<<entry.netmask().toString();
            QHostAddress gateway(addr.gateway());

            d->ui.address->setText(entry.ip().toString());
            d->ui.netMask->setText(QString::number(entry.prefixLength(),10));
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
        d->ui.advancedSettings->setAdditionalAddresses(QList<Solid::Control::IPv6Address>());
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
}

quint32 suggestNetmask(Q_IPV6ADDR ip)
{
    Q_UNUSED(ip);

    /*
    TODO: find out common IPv6-netmasks and make a complete function

    */
    quint32 netmask = 64;

    return netmask;
}

void IpV6Widget::addressEditingFinished()
{
    Q_D(IpV6Widget);
    if (d->ui.netMask->text().isEmpty()) {
        QHostAddress addr(d->ui.address->text());
        quint32 netmask = suggestNetmask(addr.toIPv6Address());kDebug();
        if (netmask) {
            d->ui.netMask->setText(QString::number(netmask,10));
        }
    }
}

void IpV6Widget::dnsEdited(QStringList items)
{
    Q_D(IpV6Widget);
    d->ui.dns->setText(items.join(QLatin1String(", ")));
}

void IpV6Widget::dnsSearchEdited(QStringList items)
{
    Q_D(IpV6Widget);
    d->ui.dnsSearch->setText(items.join(QLatin1String(", ")));
}

void IpV6Widget::showDnsEditor()
{
    Q_D(IpV6Widget);
    EditListDialog * dnsEditor = new EditListDialog;
    // at first remove space characters
    QString dnsEntries = d->ui.dns->text().remove(QLatin1Char(' '));
    dnsEditor->setItems(dnsEntries.split(QLatin1Char(','), QString::SkipEmptyParts));
    connect(dnsEditor, SIGNAL(itemsEdited(QStringList)), this, SLOT(dnsEdited(QStringList)));
    dnsEditor->setCaption(i18n("DNS Servers"));
    dnsEditor->setModal(true);
    dnsEditor->setValidator(new SimpleIpV6AddressValidator(dnsEditor));
    dnsEditor->show();
}

void IpV6Widget::showDnsSearchEditor()
{
    Q_D(IpV6Widget);
    EditListDialog * dnsSearchEditor = new EditListDialog;
    // at first remove space characters
    QString dnsSearchEntries = d->ui.dnsSearch->text().remove(QLatin1Char(' '));
    dnsSearchEditor->setItems(dnsSearchEntries.split(QLatin1Char(','), QString::SkipEmptyParts));
    connect(dnsSearchEditor, SIGNAL(itemsEdited(QStringList)), this, SLOT(dnsSearchEdited(QStringList)));
    dnsSearchEditor->setCaption(i18n("Search domains"));
    dnsSearchEditor->setModal(true);
    dnsSearchEditor->show();
}

void IpV6Widget::setDns(const QList<QVariant> dnsList)
{
    if (dnsList.isEmpty()) {
        return;
    }

    Q_D(IpV6Widget);
    QList<QHostAddress> temp;
    foreach (const QVariant &dns, dnsList) {
        QHostAddress dnsAddr(dns.toString());
        if (dnsAddr != QHostAddress::Null) {
            temp << dnsAddr;
        }
    }

    d->setting->setMethod(Knm::Ipv6Setting::EnumMethod::Automatic);
    d->setting->setIgnoredhcpdns(true);
    d->setting->setDns(temp);
    readConfig();
}

void IpV6Widget::validate()
{

}
// vim: sw=4 sts=4 et tw=100
