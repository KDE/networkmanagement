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

#ifndef APITEMMODEL_H
#define APITEMMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

#include <solid/control/networkmanager.h>
#include <solid/control/networkinterface.h>
#include <solid/control/wirelessnetworkinterface.h>
#include <solid/control/wirelessaccesspoint.h>

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
        void sort(int column=0, Qt::SortOrder order=Qt::DescendingOrder);

        void setNetworkInterface(const QString &uni);
        Solid::Control::WirelessNetworkInterface* networkInterface() const;

        void scan();

    Q_SIGNALS:
        void scanComplete();

    private Q_SLOTS:
        void onScanComplete();

    private:
        static bool isSignalStrengthLesser(Solid::Control::AccessPoint *first, Solid::Control::AccessPoint *second);
        static bool isSignalStrengthGreater(Solid::Control::AccessPoint *first, Solid::Control::AccessPoint *second);
        
        Solid::Control::AccessPointList m_ssids;
        QList<Solid::Control::AccessPoint*> m_accessPoints;
        Solid::Control::WirelessNetworkInterface *m_networkInterface;

        static const int m_numColumns = 4;
};

#endif

