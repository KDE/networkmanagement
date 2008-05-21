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

#include "ifaceitemmodel.h"

#include <KIcon>
#include <KDebug>

IfaceItemModel::IfaceItemModel(QObject *parent)
    : QAbstractItemModel(parent),
      m_ifaceList(),
      m_priorityTypeList()
{
    m_ifaceList = m_priorityList = Solid::Control::NetworkManager::networkInterfaces();
    m_priorityTypeList << Solid::Control::NetworkInterface::Ieee8023 << Solid::Control::NetworkInterface::Ieee80211 << Solid::Control::NetworkInterface::UnknownType;
}

IfaceItemModel::~IfaceItemModel()
{
}

QModelIndex IfaceItemModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    if (row < 0 || row > rowCount()-1 || column != 0) {
        return QModelIndex();
    }
    return createIndex(row, column);
}

QModelIndex IfaceItemModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return QModelIndex();
}

int IfaceItemModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_priorityList.size();
}

int IfaceItemModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_numColumns;
}

QVariant IfaceItemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_priorityList.size() || index.row() < 0 || index.column() < 0 || index.column() >= m_numColumns) {
        kDebug() << "An out-of-bounds index was requested.";
        return QVariant();
    }

    //kDebug() << "Requesting column: " << index.column();
    switch (index.column()) {
        case 0:
            switch (role) {
                case Qt::DisplayRole:
                    //kDebug() << "Returning: " << m_priorityList.value(index.row())->type();
                    switch (m_priorityList.value(index.row())->type()) {
                        case Solid::Control::NetworkInterface::Ieee80211:
                            return QString("Wireless");
                        case Solid::Control::NetworkInterface::Ieee8023:
                            return QString("Ethernet");
                        default:
                            return QString("Unknown");
                    }
                case Qt::DecorationRole:
                    switch (m_priorityList.value(index.row())->type()) {
                        case Solid::Control::NetworkInterface::Ieee80211:
                            return KIcon("network-wireless");
                        case Solid::Control::NetworkInterface::Ieee8023:
                            return KIcon("network-wired");
                        default:
                            return KIcon("Unknown");
                    }
                case TypeRole:
                    return QVariant(m_priorityList.value(index.row())->type());
                case UniRole:
                    return QVariant(m_priorityList.value(index.row())->uni());
                default:
                    return QVariant();
            }
        case 1:
            switch (role) {
                case Qt::DisplayRole:
                    //kDebug() << "Returning: " << m_priorityList.value(index.row())->uni();
                    return QVariant(m_priorityList.value(index.row())->uni());
                case Qt::DecorationRole:
                    switch (m_priorityList.value(index.row())->type()) {
                        case Solid::Control::NetworkInterface::Ieee80211:
                            return KIcon("network-wireless");
                        case Solid::Control::NetworkInterface::Ieee8023:
                            return KIcon("network-wired");
                        default:
                            return KIcon("Unknown");
                    }
                default:
                    return QVariant();
            }
        case 2:
            switch (role) {
                case Qt::DisplayRole:
                    //kDebug() << "Returning: " << m_priorityList.value(index.row())->interfaceName();
                    return QVariant(m_priorityList.value(index.row())->interfaceName());
                case Qt::DecorationRole:
                    switch (m_priorityList.value(index.row())->type()) {
                        case Solid::Control::NetworkInterface::Ieee80211:
                            return KIcon("network-wireless");
                        case Solid::Control::NetworkInterface::Ieee8023:
                            return KIcon("network-wired");
                        default:
                            return KIcon("Unknown");
                    }
                default:
                    return QVariant();
            }
        default:
            return QVariant();
    }
}

void IfaceItemModel::sort(int column, Qt::SortOrder order)
{
    Q_UNUSED(column)
    Q_UNUSED(order)
    
    Solid::Control::NetworkInterfaceList sortedList;
    foreach (Solid::Control::NetworkInterface::Type type, m_priorityTypeList) {
        foreach (Solid::Control::NetworkInterface *iface, m_priorityList) {
            if (iface->type() == type) {
                sortedList << iface;
            }
        }
    }
    m_priorityList = sortedList;
}

void IfaceItemModel::filter(FilterTypes types)
{
    m_priorityList.clear();
    reset();
    foreach (Solid::Control::NetworkInterface *iface, m_ifaceList) {
        if (types & Ieee8023 && iface->type() == Solid::Control::NetworkInterface::Ieee8023) {
            m_priorityList << iface;
        } else if (types & Ieee80211 && iface->type() == Solid::Control::NetworkInterface::Ieee80211) {
            m_priorityList << iface;
        }
    }
    emit dataChanged(index(0,0), index(rowCount()-1,0));
}

void IfaceItemModel::moveIndexUp(const QModelIndex &index)
{
    if (index.row() == 0) {
        return;
    }

    m_priorityList.swap(index.row(), index.row()-1);
    emit dataChanged(this->index(0,0), this->index(rowCount()-1,0));
}

void IfaceItemModel::moveIndexDown(const QModelIndex &index)
{
    if (index.row() >= m_priorityList.size()) {
        return;
    }

    m_priorityList.swap(index.row(), index.row()+1);
    emit dataChanged(this->index(0,0), this->index(rowCount()-1,0));
}

QString IfaceItemModel::priorityInterface() const
{
    return m_priorityList[0]->uni();
}

QString IfaceItemModel::priorityInterface(FilterTypes types) const
{
    foreach (Solid::Control::NetworkInterface *iface, m_priorityList) {
        if (types & Ieee8023 && iface->type() == Solid::Control::NetworkInterface::Ieee8023) {
            return iface->uni();
        } else if (types & Ieee80211 && iface->type() == Solid::Control::NetworkInterface::Ieee80211) {
            return iface->uni();
        }
    }
}

#include "ifaceitemmodel.moc"

