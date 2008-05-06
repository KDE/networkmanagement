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

#ifndef IFACEITEMMODEL_H
#define IFACEITEMMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

#include <solid/control/networkinterface.h>
#include <solid/control/networkmanager.h>
#include <solid/networkinterface.h>
//#include <solid/devicemanager.h>

class IfaceItemModel : public QAbstractItemModel
{
    Q_OBJECT

    public:
        enum FilterType {Ieee8023 = 1, Ieee80211=2};
        enum UserRoles {TypeRole = Qt::UserRole, UniRole};

        Q_DECLARE_FLAGS(FilterTypes, FilterType)
        
        IfaceItemModel(QObject *parent=0);
        ~IfaceItemModel();

        QModelIndex index(int row, int column, const QModelIndex &parent=QModelIndex()) const;
        QModelIndex parent(const QModelIndex &index) const;
        int rowCount(const QModelIndex &parent=QModelIndex()) const;
        int columnCount(const QModelIndex &parent=QModelIndex()) const;
        QVariant data(const QModelIndex &index, int role=Qt::DisplayRole) const;
        void sort(int column=0, Qt::SortOrder order=Qt::DescendingOrder);
        void filter(FilterTypes types);
        void moveIndexUp(const QModelIndex &index);
        void moveIndexDown(const QModelIndex &index);

        QString priorityInterface() const;
        QString priorityInterface(FilterTypes types) const;

    private:
        Solid::Control::NetworkInterfaceList m_ifaceList;
        Solid::Control::NetworkInterfaceList m_priorityList;
        QList<Solid::Control::NetworkInterface::Type> m_priorityTypeList;
};

#endif
