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

#include "apitemmodel.h"

#include <QString>
#include <QStringList>
#include <QMap>

#include <KIcon>
#include <KDebug>

ApItemModel::ApItemModel(QString uni, QObject *parent)
    : QAbstractItemModel(parent),
      m_accessPoints(),
      m_uni(uni)
{
}

ApItemModel::~ApItemModel()
{
}

void ApItemModel::init()
{
    connect(this, SIGNAL(scanComplete()), this, SLOT(onScanComplete()));
    scan();
}

QModelIndex ApItemModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return createIndex(row, column);
}

QModelIndex ApItemModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return QModelIndex();
}

int ApItemModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_accessPoints.size();
}

int ApItemModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}

QVariant ApItemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_accessPoints.size() || index.row() < 0)
        return QVariant();

    switch (role){
        case Qt::DisplayRole:
            return m_accessPoints.value(index.row()).essid();
        case Qt::DecorationRole:
            return KIcon("network-wireless");
        case SignalStrength:
            return m_accessPoints.value(index.row()).signalStrength();
        case MacAddress:
            return m_accessPoints.value(index.row()).macAddress();
        case EncryptionRole:
            return (m_accessPoints.value(index.row()).encrypted()) ? QString("object-locked") : QString("object-unlocked");
        default:
            return QVariant();
    }
}

void ApItemModel::setNewtorkDevice(const QString &uni)
{
    m_uni = uni;
}

QString ApItemModel::networkDevice() const
{
    return m_uni;
}

void ApItemModel::sort(int column, Qt::SortOrder order)
{
    Q_UNUSED(column)
    if (order == Qt::DescendingOrder) {
        qSort(m_accessPoints.begin(), m_accessPoints.end(), qGreater<AccessPoint>());
    } else {
        qSort(m_accessPoints);
    }
}

void ApItemModel::scan()
{
    m_accessPoints.clear();

    emit scanComplete();
}

void ApItemModel::onScanComplete()
{
    //Used for testing
    /*AccessPoint ap("linksys", AccessPoint::Home, 89, "some:address", false);
    m_accessPoints << ap;

    ap.setData("Starbucks", AccessPoint::Cafe, 67, "some:other:address", false);
    m_accessPoints << ap;

    ap.setData("LAX", AccessPoint::Airport, 78, "some:airport:address", true);
    m_accessPoints << ap;

    ap.setData("Neighbor's Wifi", AccessPoint::Wireless, 45, "other:address", false);
    m_accessPoints << ap;*/
    Solid::Control::NetworkInterface iface(m_uni);
    if (!iface.isValid()) {
        kDebug() << "Could not create a valid network interface.";
        return;
    } else if (iface.type() != Solid::Control::NetworkInterface::Ieee80211) {
        kDebug() << "Network Interface is not of type IEEE 80211";
    }

    Solid::Control::NetworkList netList = iface.networks();
    if (netList.size() == 0) {
        kDebug() << "No networks found.";
    }
    foreach (Solid::Control::Network *network, netList) {
        Solid::Control::WirelessNetwork *wifiNet = (Solid::Control::WirelessNetwork*)network;
        AccessPoint ap(wifiNet->essid(), wifiNet->signalStrength(), wifiNet->bssList()[0], wifiNet->isEncrypted());
        m_accessPoints << ap;
    }
    sort();
}

#include "apitemmodel.moc"

