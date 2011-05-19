/*
Copyright (C) 2008 Christopher Blauvelt <cblauvelt@gmail.com>
Copyright 2010 Will Stephenson <wstephenson@kde.org>

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

#include "scanwidget.h"

#include <QTreeView>
#include <QHeaderView>
#include <QSortFilterProxyModel>

#include <KDebug>

#include "uiutils.h"

ScanWidget::ScanWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    //populate the interfaces combobox
    foreach (NetworkManager::Device * iface, NetworkManager::networkInterfaces()) {
        if (iface->type() == NetworkManager::Device::Wifi) {

            NetworkManager::WirelessDevice * wiface = static_cast<NetworkManager::WirelessDevice*>(iface);
            m_interface->addItem(UiUtils::interfaceNameLabel(iface->uni()), wiface->uni());
        }
    }

    m_scanView = new ApItemView(this);
    m_scanModel = new NetworkItemModel(m_interface->itemData(0).toString());
    m_scanDelegate = new ApItemDelegate(m_scanView);
    m_scanSelectionModel = new QItemSelectionModel(m_scanModel);

    m_scanView->setModel(m_scanModel);
    m_scanView->setItemDelegate(m_scanDelegate);
    m_scanView->setSelectionModel(m_scanSelectionModel);
    m_stack->insertWidget(0, m_scanView);

    QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(m_scanModel);
    proxyModel->setDynamicSortFilter(true);
    proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_detailsView = new QTreeView(this);
    m_detailsView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_detailsView->setAllColumnsShowFocus(true);
    m_detailsView->setRootIsDecorated(false);
    m_detailsView->setModel(proxyModel);
    m_detailsView->setSelectionModel(m_scanSelectionModel);
    m_detailsView->setSortingEnabled(true);
    m_stack->insertWidget(1, m_detailsView);

    m_stack->setCurrentWidget(m_scanView);
    connect(m_view, SIGNAL(currentIndexChanged(int)), m_stack, SLOT(setCurrentIndex(int)));
}

ScanWidget::~ScanWidget()
{
}

void ScanWidget::setWirelessInterface(const QString &interface)
{
    m_scanModel->setNetworkInterface(interface);
}

QString ScanWidget::currentAccessPoint() const
{
    QModelIndex index = m_scanSelectionModel->currentIndex();

    if (!index.isValid()) {
        return QString();
    }

    return m_scanModel->data(m_scanModel->index(index.row(),0)).toString();
}

void ScanWidget::onInterfaceChanged(int index)
{
    m_scanModel->setNetworkInterface(m_interface->itemData(index).toString());
}

