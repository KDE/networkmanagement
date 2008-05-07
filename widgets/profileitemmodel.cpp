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

#include "profileitemmodel.h"

#include <KIcon>
#include <KDebug>

ProfileItemModel::ProfileItemModel(QObject *parent)
    : QAbstractItemModel(parent),
      m_profileList(),
      m_config()
{
}

ProfileItemModel::~ProfileItemModel()
{
}

void ProfileItemModel::updateConfig(const KConfigGroup &config)
{
    m_profileList.clear();
    reset();
    foreach (const QString &groupName, config.groupList()) {
        kDebug() << "Adding profile: " << groupName;
        const KConfigGroup configGroup = config.group(groupName);
        int profileType = configGroup.readEntry("ProfileType", (int)NetworkProfile::Default);
        m_profileList << NetworkProfile(groupName, (NetworkProfile::Type)profileType);
    }
    emit dataChanged(index(0,0), index(rowCount()-1, 0));
}

QModelIndex ProfileItemModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    if (row < 0 || row > rowCount()-1 || column != 0) {
        return QModelIndex();
    }
    return createIndex(row, column);
}

QModelIndex ProfileItemModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return QModelIndex();
}

int ProfileItemModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_profileList.size();
}

int ProfileItemModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}

QVariant ProfileItemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_profileList.size() || index.row() < 0)
        return QVariant();

    switch (role) {
        case Qt::DisplayRole:
            return m_profileList.value(index.row()).name();
        case Qt::DecorationRole:
            return KIcon(m_profileList.value(index.row()).icon());
        default:
            return QVariant();
    }
}

bool ProfileItemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.row() < 0 || index.row() >= m_profileList.size()) {
        return false;
    }

    switch (role) {
        case Qt::DisplayRole:
            m_profileList[index.row()].setName(value.toString());
            break;
        case Qt::DecorationRole:
            m_profileList[index.row()].setType((NetworkProfile::Type)value.toInt());
            break;
        default:
            return false;
    }
    emit dataChanged(index,index);
    return true;
}

bool ProfileItemModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if (row < 0 || row > m_profileList.size()) {
        return false;
    }
    
    beginInsertRows(parent, row, row+count);
    for(int index=0; index < count; index++) {
        m_profileList.insert(row+index, NetworkProfile());
    }
    endInsertRows();
    return true;
}
bool ProfileItemModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (row < 0 || row >= m_profileList.size()) {
        return false;
    }

    beginRemoveRows(parent, row, row+count);
    for(int index=0; index < count; index++) {
        m_profileList.removeAt(row+index);
    }
    endRemoveRows();
    return true;
}

#include "profileitemmodel.moc"
