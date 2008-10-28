/*  This file is part of the KDE project
    Copyright (C) 2008 Christopher Blauvelt <cblauvelt@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "scanwidget.h"
#include "ifaceitemmodel.h"

#include <QTableView>

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
    m_scanModel = new ApItemModel(m_ifaceModel->data(m_ifaceModel->index(m_interface->currentIndex(),0),IfaceItemModel::UniRole).toString());
    m_scanDelegate = new ApItemDelegate(m_scanView);
    m_scanSelectionModel = new QItemSelectionModel(m_scanModel);

    m_scanView->setModel(m_scanModel);
    m_scanView->setItemDelegate(m_scanDelegate);
    m_scanView->setSelectionModel(m_scanSelectionModel);
    m_stack->insertWidget(0, m_scanView);

    m_detailsView = new QTableView(this);
    m_detailsView->setModel(m_scanModel);
    m_detailsView->setSelectionModel(m_scanSelectionModel);
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

#include "scanwidget.moc"
