/*
Copyright (C) 2008 Christopher Blauvelt <cblauvelt@gmail.com>

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
#include "ifaceitemmodel.h"

#include <QTableView>
#include <QHeaderView>

#include <KDebug>

ScanWidget::ScanWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    //populate the interfaces combobox
    m_ifaceModel = new IfaceItemModel(m_interface);
    m_ifaceModel->filter(IfaceItemModel::Ieee80211); //we only want wifi devices
    m_interface->setModel(m_ifaceModel);
    m_interface->setModelColumn(2);//Interface name

    //setup scanview if it doesn't already exist
    m_scanView = new ApItemView(this);
    m_scanModel = new NetworkItemModel(m_ifaceModel->data(m_ifaceModel->index(m_interface->currentIndex(),0),IfaceItemModel::UniRole).toString());
    m_scanDelegate = new ApItemDelegate(m_scanView);
    m_scanSelectionModel = new QItemSelectionModel(m_scanModel);

    m_scanView->setModel(m_scanModel);
    m_scanView->setItemDelegate(m_scanDelegate);
    m_scanView->setSelectionModel(m_scanSelectionModel);
    m_stack->insertWidget(0, m_scanView);

    m_detailsView = new QTableView(this);
    m_detailsView->setModel(m_scanModel);
    m_detailsView->setSelectionModel(m_scanSelectionModel);
    QHeaderView* tableHeader = m_detailsView->horizontalHeader();
    tableHeader->setResizeMode(QHeaderView::Stretch);
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
    QModelIndex modelIndex = m_ifaceModel->index(index,0);
    if(!modelIndex.isValid()) {
        kDebug() << "Interface could not be loaded.";
        return;
    }

    kDebug() << "Loading: " << m_ifaceModel->data(modelIndex, IfaceItemModel::UniRole);
    m_scanModel->setNetworkInterface(m_ifaceModel->data(modelIndex, IfaceItemModel::UniRole).toString());
}

