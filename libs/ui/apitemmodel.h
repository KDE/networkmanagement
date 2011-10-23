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

#ifndef APITEMMODEL_H
#define APITEMMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

#include <solid/control/networkmanager.h>
#include <solid/control/networkinterface.h>
#include <solid/control/wirelessnetworkinterface.h>
#include <solid/control/wirelessaccesspoint.h>
#include "wirelessnetworkinterfaceenvironment.h"

class ApItemModel : public QAbstractItemModel
{
    Q_OBJECT

    public:
        enum UserRoles { SignalStrength=Qt::UserRole, MacAddress, ConnectionType, EncryptionRole };

        ApItemModel(QString uni, QObject *parent=0);
        ~ApItemModel();

        QModelIndex index(int row, int column, const QModelIndex &parent=QModelIndex()) const;
        QModelIndex parent(const QModelIndex &index) const;
        int rowCount(const QModelIndex &parent=QModelIndex()) const;
        int columnCount(const QModelIndex &parent=QModelIndex()) const;
        QVariant data(const QModelIndex &index, int role=Qt::DisplayRole) const;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
        void setNetworkInterface(const QString &uni);

        void scan();

    private Q_SLOTS:
        void accessPointAdded(const QString &);
        void accessPointRemoved(const QString &);

    private:
        QStringList m_accessPoints;
        Solid::Control::WirelessNetworkInterfaceNm09 *m_networkInterface;

        static const int s_numColumns = 4;
};

#endif

