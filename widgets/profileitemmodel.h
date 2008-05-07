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

#ifndef PROFILEITEMMODEL_H
#define PROFILEITEMMODEL_H

#include "networkprofile.h"

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QList>

#include <KConfigGroup>

class ProfileItemModel : public QAbstractItemModel
{
    Q_OBJECT

    public:
        ProfileItemModel(QObject *parent=0);
        ~ProfileItemModel();

        void updateConfig(const KConfigGroup &config);
        
        QModelIndex index(int row, int column, const QModelIndex &parent=QModelIndex()) const;
        QModelIndex parent(const QModelIndex &index) const;
        int rowCount(const QModelIndex &parent=QModelIndex()) const;
        int columnCount(const QModelIndex &parent=QModelIndex()) const;
        QVariant data(const QModelIndex &index, int role=Qt::DisplayRole) const;

        bool setData(const QModelIndex &index, const QVariant &value, int role);
        bool insertRows(int row, int count, const QModelIndex &parent=QModelIndex());
        bool removeRows(int row, int count, const QModelIndex &parent=QModelIndex());
        
    private:
        QList<NetworkProfile> m_profileList;
        KConfigGroup m_config;
};

#endif
