/*
Copyright 2012 Arthur de Souza Ribeiro <arthurdesribeiro@gmail.com>

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

#ifndef INTERFACES_LIST_MODEL_H
#define INTERFACES_LIST_MODEL_H

#include "declarativeinterfaceitem.h"
#include <QAbstractListModel>

class InterfacesListModel : public QAbstractListModel
{

Q_OBJECT

public:
    enum Roles {
        InterfaceNameRole = Qt::UserRole+1,
        TypeRole,
        EnabledRole,
        ConnectionRole,
        DefaultRouteRole,
        DeviceUniRole
    };

    InterfacesListModel(QObject *parent=0);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    int rowCount(const QModelIndex & parent = QModelIndex()) const;

    void appendRow(DeclarativeInterfaceItem *item);

    void appendRows(const QList<DeclarativeInterfaceItem*> &items);

    bool removeRow(int row, const QModelIndex &parent = QModelIndex());

    bool removeItem(DeclarativeInterfaceItem *act);

    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

    QModelIndex indexFromItem(const DeclarativeInterfaceItem *item) const;

Q_SIGNALS:
    void updateTraffic(NetworkManager::Device * device);

protected Q_SLOTS:
    void itemChanged();
    void loadTraffic(int index);

private:
    QList<DeclarativeInterfaceItem *> interfaces;
};

#endif
