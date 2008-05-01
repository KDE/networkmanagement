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

ProfileItemModel::ProfileItemModel(QObject *parent)
    : QAbstractItemModel(parent),
      m_profileList()
{
    m_profileList << NetworkProfile("Home", NetworkProfile::Home) << NetworkProfile("Work", NetworkProfile::Work) << NetworkProfile("Cafe", NetworkProfile::Cafe);
}

ProfileItemModel::~ProfileItemModel()
{
}

QModelIndex ProfileItemModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    if (row >= 0 && row < rowCount() && column >= 0 && column < columnCount()) {
        return createIndex(row, column);
    }
    return QModelIndex();
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

#include "profileitemmodel.moc"
