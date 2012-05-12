#include "connectionslistmodel.h"

ConnectionsListModel::ConnectionsListModel(QObject *parent)
    : QAbstractListModel(parent)
{
    QHash<int, QByteArray> roles;
    roles[DeviceUniRole] = "deviceUni";
    roles[ActivatableTypeRole] = "activatableType";
    roles[IsSharedRole] = "isShared";
    roles[ConnectionTypeRole] = "connectionType";
    roles[ConnectionUuidRole] = "connectionUuid";
    roles[ConnectionNameRole] = "connectionName";
    roles[ActivationStateRole] = "activationState";
    roles[OldActivationStateRole] = "oldActivationState";
    roles[HasDefaultRouteRole] = "hasDefaultRoute";
    roles[SsidRole] = "ssid";
    roles[StrengthRole] = "strength";
    roles[ConnectedRole] = "connectedNetwork";
    setRoleNames(roles);
}

QVariant ConnectionsListModel::data(const QModelIndex &index, int role) const
{
    if(index.row() < 0 || index.row() >= connections.size() || !index.isValid())
        return QVariant();

    switch(role) {
        case DeviceUniRole:
            return connections.at(index.row())->deviceUni();
        case ActivatableTypeRole:
            return connections.at(index.row())->activatableType();
        case IsSharedRole:
            return connections.at(index.row())->isShared();
        case ConnectionTypeRole:
            //return connections.at(index.row())->connectionType();
            return QVariant();
        case ConnectionUuidRole:
            //return connections.at(index.row())->connectionUuid().toString();
            return QVariant();
        case ConnectionNameRole:
            //return connections.at(index.row())->connectionName();
            return QVariant();
        case ActivationStateRole:
            //return connections.at(index.row())->activationStateRole();
            return QVariant();
        case OldActivationStateRole:
            //return connections.at(index.row())->oldActivationState();
            return QVariant();
        case HasDefaultRouteRole:
            //return connections.at(index.row())->hasDefaultRoute();
            return QVariant();
        case SsidRole:
            return connections.at(index.row())->ssid();
        case StrengthRole:
            return connections.at(index.row())->signalStrength();
        case ConnectedRole:
            return connections.at(index.row())->connected();
        /**
        case InterfaceCapabilitiesRole:
            RemoteWirelessInterfaceConnection *rwic = qobject_cast<RemoteWirelessInterfaceConnection *>connections.at(index.row());
            rwic->
            return ;
        case ApCapabilitiesRole:
            RemoteWirelessInterfaceConnection *rwic = qobject_cast<RemoteWirelessInterfaceConnection *>connections.at(index.row());
            break;
        case WpaFlagsRole:
            RemoteWirelessInterfaceConnection *rwic = qobject_cast<RemoteWirelessInterfaceConnection *>connections.at(index.row());
            break;
        case RsnFlagsRole:
            RemoteWirelessInterfaceConnection *rwic = qobject_cast<RemoteWirelessInterfaceConnection *>connections.at(index.row());
            break;
        case OperationModeRole:
            RemoteWirelessInterfaceConnection *rwic = qobject_cast<RemoteWirelessInterfaceConnection *>connections.at(index.row());
            break;
            **/
        default:
            return QVariant();
    }

    return QVariant();
}

int ConnectionsListModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return connections.size();
}

void ConnectionsListModel::appendRow(ConnectionItem *item) {
    beginInsertRows(QModelIndex(), rowCount(), rowCount()+1);

    connect(item, SIGNAL(itemChanged()), this, SLOT(itemChanged()));

    connections.append(item);
    endInsertRows();
}

void ConnectionsListModel::appendRows(const QList<ConnectionItem*> &items) {
    beginInsertRows(QModelIndex(), rowCount(), rowCount()+items.size()-1);
    foreach (ConnectionItem *item, items) {
        connect(item, SIGNAL(itemChanged()), this, SLOT(itemChanged()));
        connections.append(item);
    }
    endInsertRows();
}

void ConnectionsListModel::itemChanged() {
    QModelIndex index = indexFromItem((ConnectionItem*)sender());
    if(index.isValid()) {
        emit dataChanged(index, index);
    }
}

bool ConnectionsListModel::removeRow(int row, const QModelIndex &parent) {
    Q_UNUSED(parent);
    if(row < 0 || row >= connections.size()) return false;
    beginRemoveRows(QModelIndex(), row, row);
    connections.removeAt(row);
    endRemoveRows();
    return true;
}

bool ConnectionsListModel::removeItem(ConnectionItem *act) {
    int row = -1;
    int i = 0;
    foreach (ConnectionItem *item, connections) {
        if(item->equals(act)) {
            row = i;
            break;
        }
        i++;
    }
    if (row > 0) {
        return this->removeRow(row);
    }
    return false;
}

bool ConnectionsListModel::removeRows(int row, int count, const QModelIndex &parent) {
    Q_UNUSED(parent);
    if(row < 0 || (row+count) >= connections.size()) return false;
    beginRemoveRows(QModelIndex(), row, row+count-1);
    for(int i=0; i<count; ++i) {
      delete connections.takeAt(row);
    }
    endRemoveRows();
    return true;
}

QModelIndex ConnectionsListModel::indexFromItem(const ConnectionItem *item) const
{
  for(int row=0; row < connections.size(); ++row) {
      if(connections.at(row)->equals(item)) return index(row);
  }
  return QModelIndex();
}
