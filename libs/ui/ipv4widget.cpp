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

#include <QLineEdit>
#include <QStandardItem>
#include <QStandardItemModel>

#include <KDebug>

#include "ui_ipv4.h"

#include "connection.h"
#include "settings/ipv4.h"

class IpV4Widget::Private
{
public:
    Private() : setting(0), model(0,3)
    {
        QStandardItem * headerItem = new QStandardItem(i18nc("Header text for IPv4 address", "Address"));
        model.setHorizontalHeaderItem(0, headerItem);
        headerItem = new QStandardItem(i18nc("Header text for IPv4 netmask", "Prefix"));
        model.setHorizontalHeaderItem(1, headerItem);
        headerItem = new QStandardItem(i18nc("Header text for IPv4 gateway", "Gateway"));
        model.setHorizontalHeaderItem(2, headerItem);
    }
    enum MethodIndex { AutomaticMethodIndex = 0, LinkLocalMethodIndex, ManualMethodIndex, SharedMethodIndex };
    Ui_SettingsIp4Config ui;
    Knm::Ipv4Setting * setting;
    QStandardItemModel model;
};

Ipv4Delegate::Ipv4Delegate(QObject * parent) : QItemDelegate(parent) {}
Ipv4Delegate::~Ipv4Delegate() {}

QWidget * Ipv4Delegate::createEditor(QWidget *parent, const QStyleOptionViewItem &,
        const QModelIndex &) const
{
    QLineEdit *editor = new QLineEdit(parent);
    editor->setInputMask(QLatin1String("000.000.000.000;_"));

    return editor;
}

void Ipv4Delegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QString value = index.model()->data(index, Qt::EditRole).toString();

    QLineEdit *le = static_cast<QLineEdit*>(editor);
    le->setText(value);
}

void Ipv4Delegate::setModelData(QWidget *editor, QAbstractItemModel *model,
        const QModelIndex &index) const
{
    QLineEdit *le = static_cast<QLineEdit*>(editor);

    model->setData(index, le->text(), Qt::EditRole);
}

void Ipv4Delegate::updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &) const
{
    editor->setGeometry(option.rect);

}

IpV4Widget::IpV4Widget(Knm::Connection * connection, QWidget * parent)
    : SettingWidget(connection, parent), d(new IpV4Widget::Private)
{
    d->ui.setupUi(this);
    d->ui.addresses->setModel(&d->model);
    d->ui.addresses->setItemDelegateForColumn(0, new Ipv4Delegate(this));
    d->ui.addresses->setItemDelegateForColumn(2, new Ipv4Delegate(this));
    d->setting = static_cast<Knm::Ipv4Setting*>(connection->setting(Knm::Setting::Ipv4));
    connect(d->ui.addresses->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this,
            SLOT(selectionChanged(const QItemSelection&)));
    connect(d->ui.btnAddAddress, SIGNAL(clicked()), this, SLOT(addIpClicked()));
    connect(d->ui.btnRemoveAddress, SIGNAL(clicked()), this, SLOT(removeIpClicked()));
    connect(d->ui.method, SIGNAL(currentIndexChanged(int)), this, SLOT(methodChanged(int)));
    methodChanged(d->AutomaticMethodIndex);
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
    foreach (Solid::Control::IPv4Address addr, addrList) {
        QList<QStandardItem*> fields;
        fields << new QStandardItem(QHostAddress(addr.address()).toString()) << new QStandardItem(QString::number(addr.netMask())) << new QStandardItem(QHostAddress(addr.gateway()).toString());
        d->model.appendRow(fields);
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
    while (d->model.rowCount()) {
        QList<QStandardItem*> row = d->model.takeRow(0);
        QHostAddress ip(row[0]->text());
        QHostAddress gateway(row[2]->text());
        if (ip == QHostAddress::Null
                || gateway == QHostAddress::Null) {
            continue;
        }
        Solid::Control::IPv4Address addr(ip.toIPv4Address(), row[1]->text().toUInt(), gateway.toIPv4Address());
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

void IpV4Widget::methodChanged(int currentIndex)
{
    kDebug() << currentIndex;
    if (currentIndex == d->AutomaticMethodIndex) {
        d->ui.addresses->setEnabled(false);
        d->ui.dns->setEnabled(false);
        d->ui.dnsSearch->setEnabled(false);
        d->ui.btnAddAddress->setEnabled(false);
        d->ui.btnRemoveAddress->setEnabled(false);
    }
    else if (currentIndex == d->LinkLocalMethodIndex) {
        d->ui.addresses->setEnabled(false);
        d->ui.dns->setEnabled(false);
        d->ui.dnsSearch->setEnabled(false);
        d->ui.btnAddAddress->setEnabled(false);
        d->ui.btnRemoveAddress->setEnabled(false);
    }
    else if (currentIndex == d->ManualMethodIndex) {
        d->ui.addresses->setEnabled(true);
        d->ui.dns->setEnabled(true);
        d->ui.dnsSearch->setEnabled(true);
        d->ui.btnAddAddress->setEnabled(true);
        d->ui.btnRemoveAddress->setEnabled(d->ui.addresses->selectionModel()->hasSelection());
    }
    else if (currentIndex == d->SharedMethodIndex) {
        d->ui.addresses->setEnabled(false);
        d->ui.dns->setEnabled(true);
        d->ui.dnsSearch->setEnabled(true);
        d->ui.btnAddAddress->setEnabled(false);
        d->ui.btnRemoveAddress->setEnabled(false);
    }
}

void IpV4Widget::addIpClicked()
{
    QList<QStandardItem *> item;
    item << new QStandardItem << new QStandardItem << new QStandardItem;
    d->model.appendRow(item);
    // TODO select new row and enable editor on IP address
}

void IpV4Widget::removeIpClicked()
{
#if 1
    QItemSelectionModel * selectionModel = d->ui.addresses->selectionModel();
    if (selectionModel->hasSelection()) {
        QModelIndexList indexes = selectionModel->selectedIndexes();
        d->model.takeRow(indexes[0].row());
    }
    d->ui.btnRemoveAddress->setEnabled(false);
//QList<QTreeWidgetItem*> items = d->ui.addresses->selectedItems();
    //if (items.count()) {
    //    delete items.first();
    //}
#endif
}

void IpV4Widget::selectionChanged(const QItemSelection & selected)
{
    d->ui.btnRemoveAddress->setEnabled(!selected.isEmpty());
}
// vim: sw=4 sts=4 et tw=100
