/*
Copyright 2009 Will Stephenson <wstephenson@kde.org>
Copyright 2009 Paul Marchouk <pmarchouk@gmail.com>

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

#include <QLineEdit>
#include <QStandardItemModel>
#include <QNetworkAddressEntry>
//#include <QHostAddress>

#include <KDebug>

#include "ui_ipv4advanced.h"

#include "ipv4advancedwidget.h"
#include "simpleipv4addressvalidator.h"
#include "ipv4delegate.h"

class IpV4AdvancedWidget::Private
{
public:
    Private() : model(0,3)
    {
        QStandardItem * headerItem = new QStandardItem(i18nc("Header text for IPv4 address", "Address"));
        model.setHorizontalHeaderItem(0, headerItem);
        headerItem = new QStandardItem(i18nc("Header text for IPv4 netmask", "Netmask"));
        model.setHorizontalHeaderItem(1, headerItem);
        headerItem = new QStandardItem(i18nc("Header text for IPv4 gateway", "Gateway"));
        model.setHorizontalHeaderItem(2, headerItem);
    }
    Ui_AdvancedSettingsIp4Config ui;
    QStandardItemModel model;
};

IpV4AdvancedWidget::IpV4AdvancedWidget(QWidget * parent)
: QWidget(parent), d(new IpV4AdvancedWidget::Private())
{
    d->ui.setupUi(this);
    layout()->setMargin(0);
    d->ui.tableViewAddresses->setModel(&d->model);
    d->ui.tableViewAddresses->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    d->ui.tableViewAddresses->horizontalHeader()->setStretchLastSection(true);

    IpV4Delegate *ipDelegate = new IpV4Delegate(this);
    d->ui.tableViewAddresses->setItemDelegateForColumn(0, ipDelegate);
    d->ui.tableViewAddresses->setItemDelegateForColumn(1, ipDelegate);
    d->ui.tableViewAddresses->setItemDelegateForColumn(2, ipDelegate);

    d->ui.pushButtonAdd->setIcon(KIcon("list-add"));
    d->ui.pushButtonRemove->setIcon(KIcon("list-remove"));

    connect(d->ui.pushButtonAdd, SIGNAL(clicked()), this, SLOT(addIPAddress()));
    connect(d->ui.pushButtonRemove, SIGNAL(clicked()), this, SLOT(removeIPAddress()));

    connect(d->ui.tableViewAddresses->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this,
            SLOT(selectionChanged(QItemSelection)));

    connect(&d->model, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(tableViewItemChanged(QStandardItem*)));
}

IpV4AdvancedWidget::~IpV4AdvancedWidget()
{
    delete d;
}

void IpV4AdvancedWidget::setAdditionalAddresses(const QList<Solid::Control::IPv4AddressNm09> &list)
{
    d->model.removeRows(0, d->model.rowCount());
    foreach (const Solid::Control::IPv4AddressNm09 &addr, list) {
        QList<QStandardItem *> item;
        QNetworkAddressEntry entry;
        // we need to set up IP before prefix/netmask manipulation
        entry.setIp(QHostAddress(addr.address()));
        entry.setPrefixLength(addr.netMask());

        item << new QStandardItem(entry.ip().toString())
             << new QStandardItem(entry.netmask().toString());

        QString gateway;
        if (addr.gateway()) {
            gateway = QHostAddress(addr.gateway()).toString();
        }
        item << new QStandardItem(gateway);

        d->model.appendRow(item);
    }
}

QList<Solid::Control::IPv4AddressNm09> IpV4AdvancedWidget::additionalAddresses()
{
    QList<Solid::Control::IPv4AddressNm09> list;

    for (int i = 0, rowCount = d->model.rowCount(); i < rowCount; i++) {
        QHostAddress ip, mask, gw;
        QNetworkAddressEntry entry;

        ip.setAddress(d->model.item(i, 0)->text());
        entry.setIp(ip);
        mask.setAddress(d->model.item(i, 1)->text());
        entry.setNetmask(mask);
        gw.setAddress(d->model.item(i, 2)->text());

        list.append(Solid::Control::IPv4AddressNm09(ip.toIPv4Address(),
                                                entry.prefixLength(),
                                                gw.toIPv4Address()));
    }
    return list;
}

void IpV4AdvancedWidget::addIPAddress()
{
    QList<QStandardItem *> item;
    item << new QStandardItem << new QStandardItem << new QStandardItem;
    d->model.appendRow(item);

    int rowCount = d->model.rowCount();
    if (rowCount > 0) {
        d->ui.tableViewAddresses->selectRow(rowCount - 1);

        QItemSelectionModel * selectionModel = d->ui.tableViewAddresses->selectionModel();
        QModelIndexList list = selectionModel->selectedIndexes();
        if (list.size()) {
            // QTableView is configured to select only rows.
            // So, list[0] - IP address.
            d->ui.tableViewAddresses->edit(list[0]);
        }
    }
}

void IpV4AdvancedWidget::removeIPAddress()
{
    QItemSelectionModel * selectionModel = d->ui.tableViewAddresses->selectionModel();
    if (selectionModel->hasSelection()) {
        QModelIndexList indexes = selectionModel->selectedIndexes();
        d->model.takeRow(indexes[0].row());
    }
    d->ui.pushButtonRemove->setEnabled(false);
}

void IpV4AdvancedWidget::selectionChanged(const QItemSelection & selected)
{
    kDebug() << "selectionChanged";
    d->ui.pushButtonRemove->setEnabled(!selected.isEmpty());
}

extern quint32 suggestNetmask(quint32 ip);

void IpV4AdvancedWidget::tableViewItemChanged(QStandardItem *item)
{
    if (item->text().isEmpty()) {
        return;
    }

    int column = item->column();
    if (column == 0) { // ip
        int row = item->row();

        QStandardItem *netmaskItem = d->model.item(row, column + 1); // netmask
        if (netmaskItem && netmaskItem->text().isEmpty()) {
            QHostAddress addr(item->text());
            quint32 netmask = suggestNetmask(addr.toIPv4Address());
            if (netmask) {
                QHostAddress v(netmask);
                netmaskItem->setText(v.toString());
            }
        }
    }
}
