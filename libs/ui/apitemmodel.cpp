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
      m_networkInterface(0)
{
    connect(this, SIGNAL(scanComplete()), this, SLOT(onScanComplete()));
    setNetworkInterface(uni);
}

ApItemModel::~ApItemModel()
{
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
    //columns are: essid(QString), signal strength(int), encrypted(bool/QString), mac address(QString)
    return m_numColumns;
}

QVariant ApItemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_accessPoints.size() || index.row() < 0 || index.column() >= m_numColumns || index.column() < 0)
        return QVariant();

    Solid::Control::AccessPoint *accessPoint = m_accessPoints.value(index.row());
    if (accessPoint == 0) {
        kDebug() << "Access point could not be found.";
        return QVariant();
    }
    switch (index.column()) {
        case 0:
            switch (role){
                case Qt::DisplayRole:
                    return accessPoint->ssid();
                case Qt::DecorationRole:
                    return KIcon("network-wireless");
                case SignalStrength:
                    return accessPoint->signalStrength();
                case EncryptionRole:
                    return (accessPoint->wpaFlags() != 0) ? QString("object-locked") : QString("object-unlocked");
                case MacAddress:
                    return accessPoint->hardwareAddress();
                default:
                    return QVariant();
            }
        case 1:
            return accessPoint->signalStrength();
        case 2:
            return (accessPoint->wpaFlags() != 0) ? QString("Yes") : QString("No");
        case 3:
            return accessPoint->hardwareAddress();
        default:
            return QVariant();
    }
}

QVariant ApItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch (orientation) {
        case Qt::Horizontal:
            if (section < 0 || section >= m_numColumns) {
                kDebug() << "Section is out of bounds: " << section;
                return QVariant();
            }

            switch (section) {
                case 0:
                    return QVariant("Name");
                case 1:
                    return QVariant("Signal Strength");
                case 2:
                    return QVariant("Encrypted");
                case 3:
                    return QVariant("Mac Address");
                default:
                    return QVariant();
            }
        case Qt::Vertical:
            return QVariant(section);
    }
    return QVariant();
}

void ApItemModel::setNetworkInterface(const QString &uni)
{
    if (uni.isEmpty() || (m_networkInterface != 0 && m_networkInterface->uni() == uni)) {
        return;
    }

    kDebug() << "Requesting the interface: " << uni;
    Solid::Control::NetworkInterface *networkInterface = Solid::Control::NetworkManager::findNetworkInterface(uni);
    if (networkInterface == 0) {
        kDebug() << "Could not create a valid network interface.";
        m_networkInterface=0;
        return;
    } else if (networkInterface->type() != Solid::Control::NetworkInterface::Ieee80211) {
        kDebug() << "Network Interface is not of type IEEE 80211";
        m_networkInterface=0;
        return;
    }
    m_networkInterface = (Solid::Control::WirelessNetworkInterface*)networkInterface;
    scan();
}

Solid::Control::WirelessNetworkInterface* ApItemModel::networkInterface() const
{
    return m_networkInterface;
}

void ApItemModel::sort(int column, Qt::SortOrder order)
{
    Q_UNUSED(column)
    if (order == Qt::DescendingOrder) {
        qSort(m_accessPoints.begin(), m_accessPoints.end(), ApItemModel::isSignalStrengthGreater);
    } else {
        qSort(m_accessPoints);
    }
}

bool ApItemModel::isSignalStrengthGreater(Solid::Control::AccessPoint *first, Solid::Control::AccessPoint *second)
{
    return (first->signalStrength() > second->signalStrength());
}

bool ApItemModel::isSignalStrengthLesser(Solid::Control::AccessPoint *first, Solid::Control::AccessPoint *second)
{
    return (first->signalStrength() < second->signalStrength());
}

void ApItemModel::scan()
{
    m_accessPoints.clear();

    emit scanComplete();
}

void ApItemModel::onScanComplete()
{
    kDebug() << "Scan complete.";
    m_ssids.clear();
    m_accessPoints.clear();
    reset();

    if (m_networkInterface == 0) {
        kDebug() << "Primary interface not set.";
    }
    Solid::Control::AccessPointList apList = m_networkInterface->accessPoints();
    kDebug() << apList.size() << " access points were found.";
    if (apList.size() == 0) {
        kDebug() << "No networks found.";
    }
    
    foreach (const QString &ap, apList) {
        kDebug() << "Proccessing ap: " << ap;
        Solid::Control::AccessPoint *accesspoint = m_networkInterface->findAccessPoint(ap);
        if (accesspoint == 0) {
            continue;
        }
        kDebug() << "Adding to AP list.";
        
        m_ssids << ap;
        m_accessPoints << accesspoint;
    }
    sort();
}

#include "apitemmodel.moc"

