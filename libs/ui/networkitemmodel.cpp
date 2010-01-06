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

#include "networkitemmodel.h"

#include <QStringList>
#include <QMap>

#include <KIcon>
#include <KDebug>

NetworkItemModel::NetworkItemModel(const QString & uni, QObject *parent)
    : QAbstractItemModel(parent),
      m_networkInterface(0)
{
    setNetworkInterface(uni);
}

NetworkItemModel::~NetworkItemModel()
{
}

QModelIndex NetworkItemModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return createIndex(row, column);
}

QModelIndex NetworkItemModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return QModelIndex();
}

int NetworkItemModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_networks.size();
}

int NetworkItemModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    //columns are: essid(QString), signal strength(int), encrypted(bool/QString), mac address(QString)
    return s_numColumns;
}

QVariant NetworkItemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_networks.size() || index.row() < 0 || index.column() >= s_numColumns || index.column() < 0)
        return QVariant();

    Solid::Control::WirelessNetwork * network  = m_environment->findNetwork(m_networks.value(index.row()));
    if (!network) {
        kDebug() << "Network could not be found.";
        return QVariant();
    }
    switch (index.column()) {
        case 0:
            switch (role){
                case Qt::DisplayRole:
                    return network->ssid();
                case Qt::DecorationRole:
                    return KIcon("network-wireless");
                case SignalStrength:
                    return network->signalStrength();
                case EncryptionRole:
                    return (m_networkInterface->findAccessPoint(network->referenceAccessPoint())->wpaFlags() != 0) ? QString("object-locked") : QString("object-unlocked");
                case MacAddress:
                    return m_networkInterface->findAccessPoint(network->referenceAccessPoint())->hardwareAddress();
                default:
                    return QVariant();
            }
        case 1:
            if (role ==  Qt::DisplayRole)
                return network->signalStrength();
            else
                return QVariant();
        case 2:
            if (role ==  Qt::DisplayRole)
                return (m_networkInterface->findAccessPoint(network->referenceAccessPoint())->wpaFlags() != 0) ? QString("Yes") : QString("No");
            else
                return QVariant();
        case 3:
            if (role ==  Qt::DisplayRole)
                return m_networkInterface->findAccessPoint(network->referenceAccessPoint())->hardwareAddress();
            else
                return QVariant();
        default:
            return QVariant();
    }
}

QVariant NetworkItemModel::headerData(int section, Qt::Orientation orientation, int role) const
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

void NetworkItemModel::setNetworkInterface(const QString &uni)
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
    m_networkInterface = static_cast<Solid::Control::WirelessNetworkInterface*>(networkInterface);
    m_environment = new Solid::Control::WirelessNetworkInterfaceEnvironment(m_networkInterface);
    scan();
}

void NetworkItemModel::scan()
{
    m_networks.clear();

    kDebug() << "Scan complete.";
    reset();

    if (m_networkInterface == 0) {
        kDebug() << "Primary interface not set.";
    }
    m_networks = m_environment->networks();

    kDebug() << m_networks.size() << " access points were found.";
    if (m_networks.size() == 0) {
        kDebug() << "No access points found.";
    }
}

void NetworkItemModel::networkAdded(const QString & uni)
{
    if (!m_networks.contains(uni)) {
        int newIndex = m_networks.count();
        beginInsertRows(QModelIndex(), newIndex, newIndex);
        m_networks.append(uni);
        endInsertRows();
    }
}

void NetworkItemModel::networkRemoved(const QString & uni)
{
    int i = m_networks.indexOf(uni);
    if (i > -1) {
        beginRemoveRows(QModelIndex(), i, i);
        m_networks.takeAt(i);
        endRemoveRows();
    }
}

