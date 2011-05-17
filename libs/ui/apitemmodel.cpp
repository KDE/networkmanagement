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
    return s_numColumns;
}

QVariant ApItemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_accessPoints.size() || index.row() < 0 || index.column() >= s_numColumns || index.column() < 0)
        return QVariant();

    Solid::Control::AccessPointNm09 *accessPoint = m_networkInterface->findAccessPoint(m_accessPoints.value(index.row()));
    if (!accessPoint) {
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
    if (role == Qt::DisplayRole) {
        switch (orientation) {
            case Qt::Horizontal:
                if (section < 0 || section >= s_numColumns) {
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
                        return QVariant("MAC Address");
                    default:
                        return QVariant();
                }
            case Qt::Vertical:
                return QVariant(section);
        }
    }
    return QVariant();
}

void ApItemModel::setNetworkInterface(const QString &uni)
{
    if (uni.isEmpty() || (m_networkInterface != 0 && m_networkInterface->uni() == uni)) {
        return;
    }

    kDebug() << "Requesting the interface: " << uni;
    Solid::Control::NetworkInterfaceNm09 *networkInterface = Solid::Control::NetworkManagerNm09::findNetworkInterface(uni);
    if (networkInterface == 0) {
        kDebug() << "Could not create a valid network interface.";
        m_networkInterface=0;
        return;
    } else if (networkInterface->type() != Solid::Control::NetworkInterfaceNm09::Wifi) {
        kDebug() << "Network Interface is not of type IEEE 80211";
        m_networkInterface=0;
        return;
    }
    m_networkInterface = static_cast<Solid::Control::WirelessNetworkInterfaceNm09*>(networkInterface);
    scan();
}

void ApItemModel::scan()
{
    m_accessPoints.clear();

    kDebug() << "Scan complete.";
    m_accessPoints.clear();
    reset();

    if (m_networkInterface == 0) {
        kDebug() << "Primary interface not set.";
    }
    m_accessPoints = m_networkInterface->accessPoints();

    kDebug() << m_accessPoints.size() << " access points were found.";
    if (m_accessPoints.size() == 0) {
        kDebug() << "No access points found.";
    }
}

void ApItemModel::accessPointAdded(const QString & uni)
{
    if (!m_accessPoints.contains(uni)) {
        int newIndex = m_accessPoints.count();
        beginInsertRows(QModelIndex(), newIndex, newIndex);
        m_accessPoints.append(uni);
        endInsertRows();
    }
}

void ApItemModel::accessPointRemoved(const QString & uni)
{
    int i = m_accessPoints.indexOf(uni);
    if (i > -1) {
        beginRemoveRows(QModelIndex(), i, i);
        m_accessPoints.takeAt(i);
        endRemoveRows();
    }
}

