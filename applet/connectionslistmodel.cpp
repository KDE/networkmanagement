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

#include "connectionslistmodel.h"

#include <KDebug>
#include <QtNetworkManager/manager.h>
#include <KToolInvocation>
#include <KStandardDirs>

#include "remoteactivatable.h"
#include "remoteactivatablelist.h"
#include "remoteinterfaceconnection.h"
#include "remotewirelessinterfaceconnection.h"
#include "remotewirelessnetwork.h"
#include "remotegsminterfaceconnection.h"
#include "activatableitem.h"

#include <QtNetworkManager/wirelessnetworkinterfaceenvironment.h>

ConnectionsListModel::ConnectionsListModel(RemoteActivatableList *activatables, QObject *parent)
    : QAbstractListModel(parent),
      m_activatables(activatables),
      hiddenInserted(false)
{
    QHash<int, QByteArray> roles;
    roles[DeviceUniRole] = "deviceUni";
    roles[ActivatableTypeRole] = "activatableType";
    roles[IsSharedRole] = "isShared";
    roles[ConnectionTypeRole] = "networkType";
    roles[ConnectionUuidRole] = "connectionUuid";
    roles[ConnectionNameRole] = "connectionName";
    roles[ActivationStateRole] = "activationState";
    roles[OldActivationStateRole] = "oldActivationState";
    roles[HasDefaultRouteRole] = "hasDefaultRoute";
    roles[SsidRole] = "ssid";
    roles[StrengthRole] = "strength";
    roles[ConnectedRole] = "connectedNetwork";
    roles[StatusRole] = "networkStatus";
    roles[ProtectedIconRole] = "protectedIcon";
    roles[HiddenRole] = "hiddenNetwork";
    roles[WiredNetworkNameRole] = "wiredName";
    setRoleNames(roles);
}

QVariant ConnectionsListModel::data(const QModelIndex &index, int role) const
{
    if(index.row() < 0 || index.row() >= connections.size() || !index.isValid())
        return QVariant();

    if(connections.at(index.row())) {
        switch(role) {
            case DeviceUniRole:
                return connections.at(index.row())->deviceUni();
            case ActivatableTypeRole:
                return connections.at(index.row())->activatableType();
            case IsSharedRole:
                return connections.at(index.row())->isShared();
            case ConnectionTypeRole:
                return connections.at(index.row())->connectionType();
            case ConnectionUuidRole:
                return connections.at(index.row())->connectionUuid();
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
            case StatusRole:
                return connections.at(index.row())->status();
            case ProtectedIconRole:
                return connections.at(index.row())->protectedIcon();
            case HiddenRole:
                return connections.at(index.row())->hidden();
            case WiredNetworkNameRole:
                return connections.at(index.row())->wiredName();
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
    }

    return QVariant();
}

void ConnectionsListModel::disconnectFrom(QVariant uuid) {
    QString connectionId = uuid.toString();
    if(connectionId != "") {
        foreach (ConnectionItem *item, connections) {
            if (item && item->connectionUuid() == connectionId) {
                item->disconnect();
            }
        }
    }

}

void ConnectionsListModel::connectTo(int index) {
    if(index > -1 && index < connections.size()) {
        ConnectionItem *item = connections.at(index);
        if(item != 0) {
            item->connectNetwork();
        }
    }
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

void ConnectionsListModel::insertHiddenItem() {
    if(!hiddenInserted) {
        beginInsertRows(QModelIndex(), 0, 0);
        kDebug() << "adding hidden item";
        connections.insert(0, new ConnectionItem(0, true));
        hiddenInserted = true;
        endInsertRows();
    }
}

void ConnectionsListModel::removeHiddenItem() {
    if(hiddenInserted) {
        removeRow(0);
        hiddenInserted = false;
        kDebug() << "Will remove hidden item";
    }
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
    ConnectionItem * item = qobject_cast<ConnectionItem *>(sender());
    if (!item) {
        return;
    }
    QModelIndex index = indexFromItem(item);
    if(index.isValid()) {
        emit dataChanged(index, index);
    }
}

bool ConnectionsListModel::removeRow(int row, const QModelIndex &parent) {
    Q_UNUSED(parent);
    if(row < 0 || row >= connections.size()) return false;
    beginRemoveRows(QModelIndex(), row, row);
    ConnectionItem * c = connections.takeAt(row);
    QObject::disconnect(c, 0, this, 0);
    c->deleteLater();
    endRemoveRows();
    return true;
}

bool ConnectionsListModel::removeItem(ConnectionItem *act) {
    int row = 0;
    foreach (ConnectionItem *item, connections) {
        if(item->equals(act)) {
            if(item->hidden()) hiddenInserted = false;
            return removeRow(row);
        }
        row++;
    }
    return false;
}

bool ConnectionsListModel::removeRows(int row, int count, const QModelIndex &parent) {
    Q_UNUSED(parent);
    if(row < 0 || (row+count) >= connections.size()) return false;
    beginRemoveRows(QModelIndex(), row, row+count-1);
    for(int i=0; i<count; ++i) {
      delete connections.takeAt(row+i);
    }
    endRemoveRows();
    return true;
}

QModelIndex ConnectionsListModel::indexFromItem(const ConnectionItem *item) const
{
    if(item) {
        for(int row=0; row < connections.size(); ++row) {
            if(connections.at(row)->equals(item)) return index(row);
        }
    }
    return QModelIndex();
}

void ConnectionsListModel::setFilter(QVariant tabName)
{
    if(tabName == "Connections") {
        currentFilter = NormalConnections;
    } else if (tabName == "VPN") {
        currentFilter = VpnConnections;
    } else if (tabName == "Shared Connections") {
        currentFilter = SharedConnections;
    }

    updateConnectionsList();
}

void ConnectionsListModel::updateConnectionsList()
{
    foreach (ConnectionItem *item, connections) {
        kDebug() << "removing connection item";
        removeItem(item);
    }

    removeHiddenItem();


    if(currentFilter == NormalConnections && NetworkManager::isWirelessEnabled()) {
        insertHiddenItem();
    }

    foreach (RemoteActivatable *activatable, m_activatables->activatables()) {
        switch(currentFilter) {
            case NormalConnections:
                if(!activatable->isShared() && !(activatable->activatableType() == Knm::Activatable::VpnInterfaceConnection)) {
                    ConnectionItem *item = new ConnectionItem(activatable);
                    appendRow(item);
                    kDebug() << "adding connection item";
                }
                break;
            case VpnConnections:
                if(activatable->activatableType() == Knm::Activatable::VpnInterfaceConnection) {
                    ConnectionItem *item = new ConnectionItem(activatable);
                    appendRow(item);
                }
                break;
            case SharedConnections:
                if(activatable->isShared() && !((activatable->activatableType() == Knm::Activatable::WirelessInterfaceConnection ||
                                                 activatable->activatableType() == Knm::Activatable::WirelessNetwork) && !NetworkManager::isWirelessEnabled())) {
                    ConnectionItem *item = new ConnectionItem(activatable);
                    appendRow(item);
                }
                break;
        }
    }
}

void ConnectionsListModel::connectToHiddenNetwork(QVariant ssidParam)
{
    QString ssid = ssidParam.toString();
    kDebug() << "ssid is: " << ssid;
    NetworkManager::WirelessDevice * wiface = 0;
    foreach (NetworkManager::Device * iface, NetworkManager::networkInterfaces()) {
        if (iface->type() == NetworkManager::Device::Wifi && iface->state() > NetworkManager::Device::Unavailable) {
            wiface = qobject_cast<NetworkManager::WirelessDevice *>(iface);
            break;
        }
    }

    if (!wiface) {
        return;
    }

    QStringList args;
    QString moduleArgs;

    NetworkManager::WirelessNetworkInterfaceEnvironment envt(wiface);
    NetworkManager::WirelessNetwork * network = envt.findNetwork(ssid);

    if (network) {
        moduleArgs = QString::fromLatin1("%1 %2")
            .arg(wiface->uni())
            .arg(network->referenceAccessPoint());

    } else {
        moduleArgs = QString::fromLatin1("%1 %2")
            .arg(wiface->uni())
            .arg(ssid);
    }

    args << QLatin1String("create") << QLatin1String("--type") << QLatin1String("802-11-wireless") << QLatin1String("--specific-args") << moduleArgs << QLatin1String("wifi_pass");
    kDebug() << "invoking networkmanagement_configshell" << args;
    int ret = KToolInvocation::kdeinitExec(KGlobal::dirs()->findResource("exe", "networkmanagement_configshell"), args);
    kDebug() << ret << args;
}
