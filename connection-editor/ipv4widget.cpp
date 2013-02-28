/*
    Copyright (c) 2013 Lukas Tinkl <ltinkl@redhat.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QDialog>
#include <QStandardItemModel>
#include <QItemSelection>
#include <QNetworkAddressEntry>

#include "ipv4widget.h"
#include "ui_ipv4.h"
#include "ui/ipv4delegate.h"

quint32 suggestNetmask(quint32 ip)
{
    /*
        A   0       0.0.0.0 <-->127.255.255.255  255.0.0.0 <--->/8
        B   10      128.0.0.0 <>191.255.255.255  255.255.0.0 <->/16
        C   110     192.0.0.0 <>223.255.255.255  255.255.255.0 >/24
        D   1110    224.0.0.0 <>239.255.255.255  not defined <->not defined
        E   1111    240.0.0.0 <>255.255.255.254  not defined <->not defined
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

class IPv4Widget::Private
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
    QStandardItemModel model;
};


IPv4Widget::IPv4Widget(NetworkManager::Settings::Setting* setting, QWidget* parent, Qt::WindowFlags f):
    SettingWidget(setting, parent, f),
    m_ui(new Ui::IPv4Widget),
    m_ipv4Setting(0),
    d(new IPv4Widget::Private())
{
    m_ui->setupUi(this);

    m_ui->tableViewAddresses->setModel(&d->model);
    //m_ui->tableViewAddresses->setSelectionModel(new QItemSelectionModel(m_ui->tableViewAddresses->model()));
    m_ui->tableViewAddresses->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    m_ui->tableViewAddresses->horizontalHeader()->setStretchLastSection(true);

    IpV4Delegate *ipDelegate = new IpV4Delegate(this);
    m_ui->tableViewAddresses->setItemDelegateForColumn(0, ipDelegate);
    m_ui->tableViewAddresses->setItemDelegateForColumn(1, ipDelegate);
    m_ui->tableViewAddresses->setItemDelegateForColumn(2, ipDelegate);

    connect(m_ui->btnAdd, SIGNAL(clicked()), this, SLOT(slotAddIPAddress()));
    connect(m_ui->btnRemove, SIGNAL(clicked()), this, SLOT(slotRemoveIPAddress()));

    connect(m_ui->tableViewAddresses->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(selectionChanged(QItemSelection)));

    connect(&d->model, SIGNAL(itemChanged(QStandardItem*)),
            this, SLOT(tableViewItemChanged(QStandardItem*)));

    if (setting) {
        m_ipv4Setting = static_cast<NetworkManager::Settings::Ipv4Setting *>(setting);
        loadConfig(m_ipv4Setting);
    }

    connect(m_ui->method, SIGNAL(currentIndexChanged(int)),
            SLOT(slotModeComboChanged(int)));
    slotModeComboChanged(m_ui->method->currentIndex());

    connect(m_ui->btnRoutes, SIGNAL(clicked()),
            SLOT(slotRoutesDialog()));
}

IPv4Widget::~IPv4Widget()
{
    delete d;
}

void IPv4Widget::loadConfig(NetworkManager::Settings::Setting * setting)
{
    Q_UNUSED(setting)

    // method
    m_ui->method->setCurrentIndex(static_cast<int>(m_ipv4Setting->method()));

    // dns
    QStringList tmp;
    foreach (const QHostAddress & addr, m_ipv4Setting->dns()) {
        tmp.append(addr.toString());
    }
    m_ui->dns->setText(tmp.join(","));
    m_ui->dnsSearch->setText(m_ipv4Setting->dnsSearch().join(","));

    m_ui->dhcpClientId->setText(m_ipv4Setting->dhcpClientId());

    // addresses
    foreach (const NetworkManager::IPv4Address &addr, m_ipv4Setting->addresses()) {
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

    // may-fail
    m_ui->ipv4RequiredCB->setChecked(!m_ipv4Setting->mayFail());
}

QVariantMap IPv4Widget::setting() const
{
    // method
    m_ipv4Setting->setMethod(static_cast<NetworkManager::Settings::Ipv4Setting::ConfigMethod>(m_ui->method->currentIndex()));

    // dns
    if (m_ui->dns->isEnabled() && !m_ui->dns->text().isEmpty()) {
        QStringList tmp = m_ui->dns->text().split(',');
        QList<QHostAddress> tmpAddrList;
        foreach (const QString & str, tmp) {
            QHostAddress addr(str);
            if (!addr.isNull())
                tmpAddrList.append(addr);
        }
        m_ipv4Setting->setDns(tmpAddrList);
    }
    if (m_ui->dnsSearch->isEnabled() && !m_ui->dnsSearch->text().isEmpty()) {
        m_ipv4Setting->setDnsSearch(m_ui->dnsSearch->text().split(','));
    }

    // dhcp id
    if (m_ui->dhcpClientId->isEnabled() && !m_ui->dhcpClientId->text().isEmpty()) {
        m_ipv4Setting->setDhcpClientId(m_ui->dhcpClientId->text());
    }

    // addresses
    if (m_ui->tableViewAddresses->isEnabled()) {
        QList<NetworkManager::IPv4Address> list;
        for (int i = 0, rowCount = d->model.rowCount(); i < rowCount; i++) {
            QHostAddress ip, mask, gw;
            QNetworkAddressEntry entry;

            ip.setAddress(d->model.item(i, 0)->text());
            entry.setIp(ip);
            mask.setAddress(d->model.item(i, 1)->text());
            entry.setNetmask(mask);
            gw.setAddress(d->model.item(i, 2)->text());

            list.append(NetworkManager::IPv4Address(ip.toIPv4Address(),
                                                    entry.prefixLength(),
                                                    gw.toIPv4Address()));
        }
        if (!list.isEmpty()) {
            m_ipv4Setting->setAddresses(list);
        }
    }

    // may-fail
    if (m_ui->ipv4RequiredCB->isEnabled()) {
        m_ipv4Setting->setMayFail(!m_ui->ipv4RequiredCB->isChecked());
    }

    return m_ipv4Setting->toMap();
}

void IPv4Widget::slotModeComboChanged(int index)
{
    if (index == 0) {  // Automatic
        m_ui->dns->setEnabled(true);
        m_ui->dnsMorePushButton->setEnabled(true);
        m_ui->dnsSearch->setEnabled(true);
        m_ui->dnsSearchMorePushButton->setEnabled(true);
        m_ui->dhcpClientId->setEnabled(true);
        m_ui->ipv4RequiredCB->setEnabled(true);
        m_ui->btnRoutes->setEnabled(true);

        m_ui->tableViewAddresses->setVisible(false);
        m_ui->btnAdd->setVisible(false);
        m_ui->btnRemove->setVisible(false);
    } else if (index == 2) {  // Manual
        m_ui->dns->setEnabled(true);
        m_ui->dnsMorePushButton->setEnabled(true);
        m_ui->dnsSearch->setEnabled(true);
        m_ui->dnsSearchMorePushButton->setEnabled(true);
        m_ui->dhcpClientId->setEnabled(false);
        m_ui->ipv4RequiredCB->setEnabled(true);
        m_ui->btnRoutes->setEnabled(true);

        m_ui->tableViewAddresses->setVisible(true);
        m_ui->btnAdd->setVisible(true);
        m_ui->btnRemove->setVisible(true);
    } else if (index == 1|| index == 3) {  // Link-local or Shared
        m_ui->dns->setEnabled(false);
        m_ui->dnsMorePushButton->setEnabled(false);
        m_ui->dnsSearch->setEnabled(false);
        m_ui->dnsSearchMorePushButton->setEnabled(false);
        m_ui->dhcpClientId->setEnabled(false);
        m_ui->ipv4RequiredCB->setEnabled(true);
        m_ui->btnRoutes->setEnabled(false);

        m_ui->tableViewAddresses->setVisible(false);
        m_ui->btnAdd->setVisible(false);
        m_ui->btnRemove->setVisible(false);
    } else if (index == 4) {  // Disabled
        m_ui->dns->setEnabled(false);
        m_ui->dnsMorePushButton->setEnabled(false);
        m_ui->dnsSearch->setEnabled(false);
        m_ui->dnsSearchMorePushButton->setEnabled(false);
        m_ui->dhcpClientId->setEnabled(false);
        m_ui->ipv4RequiredCB->setEnabled(false);
        m_ui->btnRoutes->setEnabled(false);

        m_ui->tableViewAddresses->setVisible(false);
        m_ui->btnAdd->setVisible(false);
        m_ui->btnRemove->setVisible(false);
    }
}

void IPv4Widget::slotAddIPAddress()
{
    QList<QStandardItem *> item;
    item << new QStandardItem << new QStandardItem << new QStandardItem;
    d->model.appendRow(item);

    int rowCount = d->model.rowCount();
    if (rowCount > 0) {
        m_ui->tableViewAddresses->selectRow(rowCount - 1);

        QItemSelectionModel * selectionModel = m_ui->tableViewAddresses->selectionModel();
        QModelIndexList list = selectionModel->selectedIndexes();
        if (!list.isEmpty()) {
            // QTableView is configured to select only rows.
            // So, list[0] - IP address.
            m_ui->tableViewAddresses->edit(list[0]);
        }
    }
}

void IPv4Widget::slotRemoveIPAddress()
{
    QItemSelectionModel * selectionModel = m_ui->tableViewAddresses->selectionModel();
    if (selectionModel->hasSelection()) {
        QModelIndexList indexes = selectionModel->selectedIndexes();
        d->model.takeRow(indexes[0].row());
    }
    m_ui->btnRemove->setEnabled(m_ui->tableViewAddresses->selectionModel()->hasSelection());
}

void IPv4Widget::selectionChanged(const QItemSelection & selected)
{
    m_ui->btnRemove->setEnabled(!selected.isEmpty());
}

void IPv4Widget::tableViewItemChanged(QStandardItem *item)
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

void IPv4Widget::slotRoutesDialog()
{
    IpV4RoutesWidget * dlg = new IpV4RoutesWidget(this);
    dlg->setRoutes(m_ipv4Setting->routes());
    dlg->setNeverDefault(m_ipv4Setting->neverDefault());
    if (m_ui->method->currentIndex() == 1) {  // manual
        dlg->setIgnoreAutoRoutesCheckboxEnabled(false);
    } else {
        dlg->setIgnoreAutoRoutes(m_ipv4Setting->ignoreAutoRoutes());
    }
    if (dlg->exec() == QDialog::Accepted) {
        m_ipv4Setting->setRoutes(dlg->routes());
        m_ipv4Setting->setNeverDefault(dlg->neverdefault());
        m_ipv4Setting->setIgnoreAutoRoutes(dlg->ignoreautoroutes());
    }
    delete dlg;
}
