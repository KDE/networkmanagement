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

#include <KDebug>

#include "uiutils.h"

ScanWidget::ScanWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    //populate the interfaces combobox
    foreach (const Solid::Control::NetworkInterfaceNm09 * iface, Solid::Control::NetworkManagerNm09::networkInterfaces()) {
        if (iface->type() == Solid::Control::NetworkInterfaceNm09::Wifi) {

            const Solid::Control::WirelessNetworkInterfaceNm09 * wiface = static_cast<const Solid::Control::WirelessNetworkInterfaceNm09*>(iface);
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
    m_stack->insertWidget(Map, m_scanView);

    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_scanModel);
    m_proxyModel->setDynamicSortFilter(true);
    m_proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_scanProxySelectionModel = new QItemSelectionModel(m_proxyModel);
    m_detailsView = new QTreeView(this);
    m_detailsView->sortByColumn(0, Qt::AscendingOrder);
    m_detailsView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_detailsView->setAllColumnsShowFocus(true);
    m_detailsView->setRootIsDecorated(false);
    m_detailsView->setModel(m_proxyModel);
    m_detailsView->setSelectionModel(m_scanProxySelectionModel);
    m_detailsView->setSortingEnabled(true);
    m_stack->insertWidget(Details, m_detailsView);

    m_stack->setCurrentWidget(m_scanView);
    connect(m_view, SIGNAL(currentIndexChanged(int)), m_stack, SLOT(setCurrentIndex(int)));

    connect(m_scanView, SIGNAL(doubleClicked(QModelIndex)), SIGNAL(doubleClicked()));
    connect(m_detailsView, SIGNAL(doubleClicked(QModelIndex)), SIGNAL(doubleClicked()));
}

ScanWidget::~ScanWidget()
{
}

void ScanWidget::setWirelessInterface(const QString &interface)
{
    m_scanModel->setNetworkInterface(interface);
}

QPair<QString,QString> ScanWidget::currentAccessPoint() const
{
    QPair<QString, QString> accessPoint(QString(""), QString(""));
    QModelIndex index;
    switch (m_stack->currentIndex())
    {
        case Details:
            index = m_proxyModel->mapToSource(m_scanProxySelectionModel->currentIndex());
            break;
        case Map:
        default:
            index = m_scanSelectionModel->currentIndex();
            break;
    }

    if (!index.isValid()) {
        return accessPoint;
    }
    accessPoint.first = m_scanModel->data(m_scanModel->index(index.row(),0)).toString();

    switch (m_useBssid->isChecked())
    {
        case false:
            accessPoint.second = QString();
            break;
        case true:
        default:
            accessPoint.second = m_scanModel->data(m_scanModel->index(index.row(),5)).toString();
            break;
    }

    return accessPoint;
}

QPair<Solid::Control::WirelessNetworkInterfaceNm09 *, Solid::Control::AccessPointNm09 *> ScanWidget::currentAccessPointUni()
{
    QPair<Solid::Control::WirelessNetworkInterfaceNm09 *, Solid::Control::AccessPointNm09 *> pair(0, 0);
    QModelIndex index;

    switch (m_stack->currentIndex())
    {
        case Details:
            index = m_proxyModel->mapToSource(m_scanProxySelectionModel->currentIndex());
            break;
        case Map:
        default:
            index = m_scanSelectionModel->currentIndex();
            break;
    }

    if (!index.isValid()) {
        return pair;
    }

    QString apMac = m_scanModel->data(m_scanModel->index(index.row(),5)).toString();
    if (apMac.isEmpty()) {
        return pair;
    }

    Solid::Control::WirelessNetworkInterfaceNm09 * wiface = qobject_cast<Solid::Control::WirelessNetworkInterfaceNm09 *>(Solid::Control::NetworkManagerNm09::findNetworkInterface(m_interface->itemData(m_interface->currentIndex()).toString()));
    if (wiface) {
        foreach(const QString & uni, wiface->accessPoints()) {
            Solid::Control::AccessPointNm09 * ap = wiface->findAccessPoint(uni);
            if (ap->hardwareAddress() == apMac) {
                pair.first = wiface;
                pair.second = ap;
                break;
            }
        }
    }

    return pair;
}

void ScanWidget::onInterfaceChanged(int index)
{
    m_scanModel->setNetworkInterface(m_interface->itemData(index).toString());
}

