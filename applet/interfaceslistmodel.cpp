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

#include "interfaceslistmodel.h"

#include <KDebug>

InterfacesListModel::InterfacesListModel(QObject *parent)
    : QAbstractListModel(parent)
{
    QHash<int, QByteArray> roles;
    roles[InterfaceNameRole] = "interfaceName";
    roles[TypeRole] = "type";
    roles[EnabledRole] = "interfaceEnabled";
    roles[ConnectionRole] = "interfaceConnection";
    roles[DefaultRouteRole] = "defaultRoute";
    roles[DeviceUniRole] = "interfaceDeviceUni";
    roles[VisibleRole] = "interfaceVisible";
    setRoleNames(roles);
}

QVariant InterfacesListModel::data(const QModelIndex &index, int role) const
{
    if(index.row() < 0 || index.row() >= interfaces.size() || !index.isValid())
        return QVariant();

    if(interfaces.at(index.row())) {
        switch(role) {
            case InterfaceNameRole:
                return interfaces.at(index.row())->interfaceTitle();
            case TypeRole:
                return interfaces.at(index.row())->type();
            case EnabledRole:
                return interfaces.at(index.row())->enabled();
            case ConnectionRole:
                return interfaces.at(index.row())->connection();
            case DefaultRouteRole:
                return interfaces.at(index.row())->defaultRoute();
            case DeviceUniRole:
                return interfaces.at(index.row())->deviceUni();
            case VisibleRole:
                return interfaces.at(index.row())->isVisible();
            default:
                return QVariant();
        }
    }

    return QVariant();
}


int InterfacesListModel::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);
    return interfaces.size();
}

void InterfacesListModel::itemChanged() {
    DeclarativeInterfaceItem * item = qobject_cast<DeclarativeInterfaceItem *>(sender());
    if (!item) {
        return;
    }
    QModelIndex index = indexFromItem(item);
    if(index.isValid()) {
        emit dataChanged(index, index);
    }
}

void InterfacesListModel::appendRow(DeclarativeInterfaceItem *item)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount()+1);

    connect(item, SIGNAL(itemChanged()), this, SLOT(itemChanged()));

    interfaces.append(item);
    
    endInsertRows();
}

void InterfacesListModel::appendRows(const QList<DeclarativeInterfaceItem*> &items)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount()+items.size()-1);
    foreach (DeclarativeInterfaceItem *item, items) {
        connect(item, SIGNAL(itemChanged()), this, SLOT(itemChanged()));
        interfaces.append(item);
    }
    endInsertRows();
}

bool InterfacesListModel::removeRow(int row, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    if(row < 0 || row >= interfaces.size()) return false;
    beginRemoveRows(QModelIndex(), row, row);
    DeclarativeInterfaceItem * c = interfaces.takeAt(row);
    QObject::disconnect(c, 0, this, 0);
    c->deleteLater();
    endRemoveRows();
    return true;
}

bool InterfacesListModel::removeItem(DeclarativeInterfaceItem *act)
{
    int row = 0;
    foreach (DeclarativeInterfaceItem *item, interfaces) {
        if(item->equals(act)) {
            return removeRow(row);
        }
        row++;
    }
    return false;
}

void InterfacesListModel::loadTraffic(int index)
{
    kDebug() << "load traffic plotter with index " << index;
    if(index >= 0 && index < interfaces.size()) {
        emit updateTraffic(interfaces[index]);
        kDebug() << "emitted update traffic plotter signal";
    }
}

bool InterfacesListModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    if(row < 0 || (row+count) >= interfaces.size()) return false;
    beginRemoveRows(QModelIndex(), row, row+count-1);
    for(int i=0; i<count; ++i) {
      delete interfaces.takeAt(row+i);
    }
    endRemoveRows();
    return true;
}

QModelIndex InterfacesListModel::indexFromItem(const DeclarativeInterfaceItem *item) const
{
    if(item) {
        for(int row=0; row < interfaces.size(); ++row) {
            if(interfaces.at(row)->equals(item)) return index(row);
        }
    }
    return QModelIndex();
}
