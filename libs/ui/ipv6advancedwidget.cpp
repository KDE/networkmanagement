/*
Copyright 2011 Ilia Kats <ilia-kats@gmx.net>, based on work by Will Stephenson <wstephenson@kde.org> and Paul Marchouk <pmarchouk@gmail.com>

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

#include "ipv6advancedwidget.h"

#include <KLineEdit>
#include <QStandardItemModel>
#include <QNetworkAddressEntry>

#include <KDebug>

#include "ui_ipv6advanced.h"

#include "simpleipv6addressvalidator.h"
#include "intdelegate.h"
#include "ipv6delegate.h"

class IpV6AdvancedWidget::Private
{
public:
    Private() : model(0,3)
    {
        QStandardItem * headerItem = new QStandardItem(i18nc("Header text for IPv6 address", "Address"));
        model.setHorizontalHeaderItem(0, headerItem);
        headerItem = new QStandardItem(i18nc("Header text for IPv6 netmask", "Netmask"));
        model.setHorizontalHeaderItem(1, headerItem);
        headerItem = new QStandardItem(i18nc("Header text for IPv6 gateway", "Gateway"));
        model.setHorizontalHeaderItem(2, headerItem);
    }
    Ui_AdvancedSettingsIp6Config ui;
    QStandardItemModel model;
};

IpV6AdvancedWidget::IpV6AdvancedWidget(QWidget * parent)
: QWidget(parent), d(new IpV6AdvancedWidget::Private())
{
    d->ui.setupUi(this);
    layout()->setMargin(0);
    d->ui.tableViewAddresses->setModel(&d->model);
    d->ui.tableViewAddresses->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    d->ui.tableViewAddresses->horizontalHeader()->setStretchLastSection(true);

    IpV6Delegate *ipDelegate = new IpV6Delegate(this);
    IntDelegate *prefixDelegate = new IntDelegate (0, 128, this);
    d->ui.tableViewAddresses->setItemDelegateForColumn(0, ipDelegate);
    d->ui.tableViewAddresses->setItemDelegateForColumn(1, prefixDelegate);
    d->ui.tableViewAddresses->setItemDelegateForColumn(2, ipDelegate);

    d->ui.pushButtonAdd->setIcon(KIcon("list-add"));
    d->ui.pushButtonRemove->setIcon(KIcon("list-remove"));

    connect(d->ui.pushButtonAdd, SIGNAL(clicked()), this, SLOT(addIPAddress()));
    connect(d->ui.pushButtonRemove, SIGNAL(clicked()), this, SLOT(removeIPAddress()));

    connect(d->ui.tableViewAddresses->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this,
            SLOT(selectionChanged(QItemSelection)));

    connect(&d->model, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(tableViewItemChanged(QStandardItem*)));
}

IpV6AdvancedWidget::~IpV6AdvancedWidget()
{
    delete d;
}

void IpV6AdvancedWidget::setAdditionalAddresses(const QList<NetworkManager::IpAddress> &list)
{
    d->model.removeRows(0, d->model.rowCount());
    foreach (const NetworkManager::IpAddress &address, list) {
        QList<QStandardItem *> item;
        item << new QStandardItem(address.ip().toString())
             << new QStandardItem(QString::number(address.prefixLength(), 10))
             << new QStandardItem(address.gateway().toString());

        d->model.appendRow(item);
    }
}

QList<NetworkManager::IpAddress> IpV6AdvancedWidget::additionalAddresses()
{
    QList<NetworkManager::IpAddress> list;

    for (int i = 0, rowCount = d->model.rowCount(); i < rowCount; i++) {
        NetworkManager::IpAddress address;
        address.setIp(QHostAddress(d->model.item(i, 0)->text()));
        address.setPrefixLength(d->model.item(i, 1)->text().toInt());
        address.setGateway(QHostAddress(d->model.item(i, 2)->text()));

        list << address;
    }
    return list;
}

void IpV6AdvancedWidget::addIPAddress()
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

void IpV6AdvancedWidget::removeIPAddress()
{
    QItemSelectionModel * selectionModel = d->ui.tableViewAddresses->selectionModel();
    if (selectionModel->hasSelection()) {
        QModelIndexList indexes = selectionModel->selectedIndexes();
        d->model.takeRow(indexes[0].row());
    }
    d->ui.pushButtonRemove->setEnabled(false);
}

void IpV6AdvancedWidget::selectionChanged(const QItemSelection & selected)
{
    kDebug() << "selectionChanged";
    d->ui.pushButtonRemove->setEnabled(!selected.isEmpty());
}

extern quint32 suggestNetmask(Q_IPV6ADDR ip);

void IpV6AdvancedWidget::tableViewItemChanged(QStandardItem *item)
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
            quint32 netmask = suggestNetmask(addr.toIPv6Address());
            if (netmask) {
                netmaskItem->setText(QString::number(netmask,10));
            }
        }
    }
}
