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

IfaceItemModel::IfaceItemModel(QObject *parent)
    : QAbstractItemModel(parent),
      m_ifaceList(),
      m_priorityList()
{
    m_ifaceList = Solid::Control::NetworkManager::networkInterfaces();
    m_priorityList << Solid::Control::NetworkInterface::Ieee8023 << Solid::Control::NetworkInterface::Ieee80211 << Solid::Control::NetworkInterface::UnknownType;
}

IfaceItemModel::~IfaceItemModel()
{
}

void IfaceItemModel::init()
{
}

QModelIndex IfaceItemModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent)
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
    return m_ifaceList.size();
}

int IfaceItemModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}

QVariant IfaceItemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_ifaceList.size() || index.row() < 0)
        return QVariant();

    switch (role) {
        case Qt::DisplayRole:
            switch (m_ifaceList.value(index.row()).type()) {
                case Solid::Control::NetworkInterface::Ieee80211:
                    return QString("Wifi");
                case Solid::Control::NetworkInterface::Ieee8023:
                    return QString("Ethernet");
                default:
                    return QString("Unknown");
            }
        case Qt::DecorationRole:
            switch (m_ifaceList.value(index.row()).type()) {
                case Solid::Control::NetworkInterface::Ieee80211:
                    return KIcon("network-wireless");
                case Solid::Control::NetworkInterface::Ieee8023:
                    return KIcon("network-wired");
                default:
                    return KIcon("Unknown");
            }
        case Type:
            return QVariant(m_ifaceList.value(index.row()).type());
        default:
            return QVariant();
    }
}

void IfaceItemModel::sort(int column, Qt::SortOrder order)
{
    Q_UNUSED(column)
    Q_UNUSED(order)
    
    QList<Solid::Control::NetworkInterface> sortedList;
    foreach (const Solid::Control::NetworkInterface::Type &type, m_priorityList) {
        foreach (const Solid::Control::NetworkInterface &iface, m_ifaceList) {
            if (iface.type() == type) {
                sortedList << iface;
            }
        }
    }
    m_ifaceList = sortedList;
}

#include "ifaceitemmodel.moc"

