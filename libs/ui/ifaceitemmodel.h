/*
Copyright (C) 2008 Christopher Blauvelt <cblauvelt@gmail.com>

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

#ifndef IFACEITEMMODEL_H
#define IFACEITEMMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

#include <solid/control/networkinterface.h>
#include <solid/control/wirednetworkinterface.h>
#include <solid/control/wirelessnetworkinterface.h>
#include <solid/control/networkmanager.h>
#include <solid/networkinterface.h>

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

        static const int m_numColumns = 3;
};

#endif
