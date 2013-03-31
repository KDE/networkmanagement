/*
Copyright 2012 Arthur de Souza Ribeiro <arthurdesribeiro@gmail.com>
Copyright 2012-2013 Lamarque V. Souza <lamarque@kde.org>

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
#include <KLocale>
#include <KToolInvocation>
#include <KStandardDirs>

#include "remoteactivatable.h"
#include "remoteactivatablelist.h"
#include "remoteinterfaceconnection.h"
#include "remotewirelessinterfaceconnection.h"
#include "remotewirelessnetwork.h"
#include "remotegsminterfaceconnection.h"
#include "knmserviceprefs.h"

#include <QtNetworkManager/manager.h>
#include <QtNetworkManager/wirelessnetworkinterfaceenvironment.h>

ConnectionsListModel::ConnectionsListModel(RemoteActivatableList *activatables, QObject *parent)
    : QAbstractListModel(parent),
      m_showMoreItem(new ConnectionItem(0, ConnectionItem::ShowMore, this)),
      m_device(0),
      m_activatables(activatables),
      hiddenInserted(false),
      m_vpn(false),
      m_moreNetworks(0),
      m_hasWireless(false)
{
    QHash<int, QByteArray> roles;
    roles[DeviceUniRole] = "deviceUni";
    roles[IsSharedRole] = "isShared";
    roles[ConnectionTypeRole] = "networkType";
    roles[ConnectionUuidRole] = "connectionUuid";
    roles[ConnectionNameRole] = "connectionName";
    roles[ActivationStateRole] = "activationState";
    roles[OldActivationStateRole] = "oldActivationState";
    roles[HasDefaultRouteRole] = "hasDefaultRoute";
    roles[SsidRole] = "ssid";
    roles[StrengthRole] = "strength";
    roles[ProtectedIconRole] = "protectedIcon";
    roles[HiddenRole] = "hiddenNetwork";
    roles[HoverEnterRole] = "hoverEntered";
    roles[NetworkIconRole] = "netIcon";
    roles[SignalQualityRole] = "signalQuality";
    roles[AccessTechnologyRole] = "accessTechnology";
    roles[ShowMoreCheckedRole] = "showMoreChecked";
    roles[NetworkCountRole] = "networkCount";
    setRoleNames(roles);

    connect(m_activatables, SIGNAL(activatableAdded(RemoteActivatable*,int)), SLOT(activatableAdded(RemoteActivatable*)));
    connect(m_activatables, SIGNAL(activatableRemoved(RemoteActivatable*)), SLOT(activatableRemoved(RemoteActivatable*)));
    connect(m_showMoreItem, SIGNAL(itemChanged()), this, SLOT(itemChanged()));

    m_showMoreChecked = false;
    m_oldShowMoreChecked = true;
    wicCount = 0; // number of wireless networks which user explicitly configured using the kcm module.

    KConfigGroup config(KNetworkManagerServicePrefs::self()->config(), QLatin1String("General"));
    m_oldShowMoreChecked = config.readEntry(QLatin1String("ShowMoreConnections"), true);
    showMore(m_oldShowMoreChecked);
}

void ConnectionsListModel::activatableAdded(RemoteActivatable *activatable)
{
    uncheckShowMore(activatable);
    updateConnectionsList();
}

void ConnectionsListModel::activatableRemoved(RemoteActivatable *activatable)
{
    checkShowMore(activatable);
    updateConnectionsList();
}

void ConnectionsListModel::showMoreClicked()
{
    // Simulate button toggling.
    m_showMoreChecked = !m_showMoreChecked;
    m_oldShowMoreChecked = m_showMoreChecked;
    showMore(m_oldShowMoreChecked);

    KConfigGroup config(KNetworkManagerServicePrefs::self()->config(), QLatin1String("General"));
    config.writeEntry(QLatin1String("ShowMoreConnections"), m_oldShowMoreChecked);
    config.sync();
}

void ConnectionsListModel::showMore(bool show)
{
    if (!NetworkManager::isWirelessEnabled()) {
        show = false;
    } else if (wicCount == 0) {
        show = true;
    }

    if (show) {
        currentFilter &= ~SavedConnections;
    } else {
        currentFilter |= SavedConnections;
    }

    m_showMoreChecked = show;
    m_showMoreItem->setShowMoreChecked(m_showMoreChecked);
    updateConnectionsList();
    kDebug() << "Show == " << show << " wicCount == " << wicCount;
}

void ConnectionsListModel::checkShowMore(RemoteActivatable * ra)
{
    RemoteWirelessInterfaceConnection * wic = qobject_cast<RemoteWirelessInterfaceConnection*>(ra);
    if (wic) {
        if (wic->operationMode() == NetworkManager::WirelessDevice::Adhoc &&
            wic->activationState() == Knm::InterfaceConnection::Unknown) {
            return;
        }
        if (wicCount > 0) {
            wicCount--;
        }
        if (wicCount == 0 && !m_showMoreItem->showMoreChecked()) {
            // There is no wireless network around which the user has explicitly configured
            // so temporaly show all wifi available networks.
            showMore(true);
        }
    }
}

void ConnectionsListModel::uncheckShowMore(RemoteActivatable *ra)
{
    RemoteWirelessInterfaceConnection * wic = qobject_cast<RemoteWirelessInterfaceConnection*>(ra);
    if (wic) {
        if (wic->operationMode() == NetworkManager::WirelessDevice::Adhoc &&
            wic->activationState() == Knm::InterfaceConnection::Unknown) {
            return;
        }
        wicCount++;
        if (m_oldShowMoreChecked != m_showMoreItem->showMoreChecked()) {
            // One wireless network explicitly configured by the user appeared, reset "Show More" button
            // state to the value before the checkShowMore method above took action.
            showMore(m_oldShowMoreChecked);
        }
    }
}

QVariant ConnectionsListModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= connections.size() || !index.isValid())
        return QVariant();

    if (connections.at(index.row())) {
        switch (role) {
        case DeviceUniRole:
            return connections.at(index.row())->deviceUni();
        case IsSharedRole:
            return connections.at(index.row())->isShared();
        case ConnectionTypeRole:
            return connections.at(index.row())->connectionType();
        case ConnectionUuidRole:
            return connections.at(index.row())->connectionUuid();
        case ConnectionNameRole:
            return connections.at(index.row())->connectionName();
        case ActivationStateRole:
            return connections.at(index.row())->activationState();
        case OldActivationStateRole:
            //return connections.at(index.row())->oldActivationState();
            return QVariant();
        case SsidRole:
            return connections.at(index.row())->ssid();
        case StrengthRole:
            return connections.at(index.row())->signalStrength();
        case ProtectedIconRole:
            return connections.at(index.row())->protectedIcon();
        case HiddenRole:
            return connections.at(index.row())->hidden();
        case HoverEnterRole:
            return connections.at(index.row())->hover();
        case NetworkIconRole:
            return connections.at(index.row())->connectionIcon();
        case HasDefaultRouteRole:
            return connections.at(index.row())->defaultRoute();
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
        case SignalQualityRole:
            return connections.at(index.row())->signalQuality();
        case AccessTechnologyRole:
            return connections.at(index.row())->accessTechnology();
        case ShowMoreCheckedRole:
            return connections.at(index.row())->showMoreChecked();
        case NetworkCountRole:
            return connections.at(index.row())->networkCount();
        default:
            return QVariant();
        }
    }

    return QVariant();
}

void ConnectionsListModel::disconnectFrom(QVariant uuid)
{
    QString connectionId = uuid.toString();

    if (!connectionId.isEmpty()) {
        foreach(ConnectionItem * item, connections) {
            if (item && item->connectionUuid() == connectionId) {
                item->disconnect();
            }
        }
    }
}

void ConnectionsListModel::deactivateConnection(const QString& deviceUni)
{
    if (!deviceUni.isEmpty()) {
        foreach(ConnectionItem * item, connections) {
            if (item && item->interfaceConnection() && item->interfaceConnection()->deviceUni() == deviceUni) {
                item->disconnect();
            }
        }
    }
}

void ConnectionsListModel::hoverEnterConnections(QString deviceUni)
{
    if (!deviceUni.isEmpty()) {
        foreach(ConnectionItem * item, connections) {
            RemoteInterfaceConnection *conn = item->interfaceConnection();
            if (conn && conn->deviceUni() == deviceUni) {
                item->hoverEnter();
            }
        }
    }
}

void ConnectionsListModel::hoverLeftConnections(QString deviceUni)
{
    if (!deviceUni.isEmpty()) {
        foreach(ConnectionItem * item, connections) {
            RemoteInterfaceConnection *conn = item->interfaceConnection();
            if (conn && conn->deviceUni() == deviceUni) {
                item->hoverLeft();
            }
        }
    }
}

void ConnectionsListModel::hoverEnterVpn()
{
    foreach(ConnectionItem * item, connections) {
        RemoteInterfaceConnection *conn = item->interfaceConnection();
        if (conn && conn->connectionType() == Knm::Connection::Vpn) {
            item->hoverEnter();
        }
    }
}

void ConnectionsListModel::hoverLeftVpn()
{
    foreach(ConnectionItem * item, connections) {
        RemoteInterfaceConnection *conn = item->interfaceConnection();
        if (conn && conn->connectionType() == Knm::Connection::Vpn) {
            item->hoverLeft();
        }
    }
}

void ConnectionsListModel::connectTo(int index)
{
    if (index > -1 && index < connections.size()) {
        ConnectionItem *item = connections.at(index);
        if (item) {
            item->connectNetwork();
        }
    }
}

int ConnectionsListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return connections.size();
}

void ConnectionsListModel::appendRow(ConnectionItem *item)
{
    const int start = rowCount();
    beginInsertRows(QModelIndex(), start, start);

    connect(item, SIGNAL(itemChanged()), this, SLOT(itemChanged()));
    connect(item, SIGNAL(showInterfaceDetails(QString)), this, SIGNAL(showInterfaceDetails(QString)));

    connections.append(item);
    endInsertRows();
}

void ConnectionsListModel::insertHiddenItem()
{
    if (!hiddenInserted) {
        beginInsertRows(QModelIndex(), 0, 0);
        kDebug() << "adding hidden item";
        connections.insert(0, new ConnectionItem(0, ConnectionItem::HiddenNetwork));
        hiddenInserted = true;
        endInsertRows();
    }
}

void ConnectionsListModel::removeHiddenItem()
{
    if (hiddenInserted) {
        removeRow(0);
        hiddenInserted = false;
        kDebug() << "Will remove hidden item";
    }
}

void ConnectionsListModel::updateShowMoreItem()
{
    int index = connections.indexOf(m_showMoreItem);
    if (index >= 0) {
        beginRemoveRows(QModelIndex(), index, index);
        connections.removeAt(index);
        endRemoveRows();
    }

    /* m_showMoreItem only makes sense for wifi and NormalConnections, so disabled it for VPN and Shared connections.
     * Also disables it when there is no more connections to show and when the m_showMoreItem is not toggled.
     * When m_showMoreItem is toggled m_moreNetworks is zero, so do not hide m_showMoreItem in that case. */
    if (!currentFilter.testFlag(NormalConnections) ||
        (m_showMoreItem->networkCount() < 1 && currentFilter.testFlag(SavedConnections)) ||
        !m_hasWireless) {
        return;
    }

    // If SavedConnection is set then m_showMoreItem is always *not* toggled.
    m_showMoreItem->setShowMoreChecked(!currentFilter.testFlag(SavedConnections));

    index = rowCount();
    beginInsertRows(QModelIndex(), index, index);
    connections.append(m_showMoreItem);
    endInsertRows();
}

void ConnectionsListModel::appendRows(const QList<ConnectionItem*> &items)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount() + items.size() - 1);
    foreach(ConnectionItem * item, items) {
        connect(item, SIGNAL(itemChanged()), this, SLOT(itemChanged()));
        connections.append(item);
    }
    endInsertRows();
}

void ConnectionsListModel::itemChanged()
{
    ConnectionItem * item = qobject_cast<ConnectionItem *>(sender());

    if (!item) {
        return;
    }
    QModelIndex index = indexFromItem(item);
    if (index.isValid()) {
        emit dataChanged(index, index);
    }
}

bool ConnectionsListModel::removeRow(int row, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    if (row < 0 || row >= connections.size()) return false;
    beginRemoveRows(QModelIndex(), row, row);
    ConnectionItem * c = connections.takeAt(row);
    QObject::disconnect(c, 0, this, 0);
    c->deleteLater();
    endRemoveRows();
    return true;
}

bool ConnectionsListModel::removeItem(ConnectionItem *act)
{
    int row = 0;
    foreach(ConnectionItem * item, connections) {
        if (item->equals(act)) {
            if (item->hidden()) hiddenInserted = false;
            return removeRow(row);
        }
        row++;
    }
    return false;
}

bool ConnectionsListModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    if (row < 0 || (row + count) >= connections.size()) return false;
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    for (int i = 0; i < count; ++i) {
        delete connections.takeAt(row + i);
    }
    endRemoveRows();
    return true;
}

QModelIndex ConnectionsListModel::indexFromItem(const ConnectionItem *item) const
{
    if (item) {
        if (item == m_showMoreItem) {
            return index(connections.indexOf(m_showMoreItem));
        }
        for (int row = 0; row < connections.size(); ++row) {
            if (connections.at(row)->equals(item)) return index(row);
        }
    }
    return QModelIndex();
}

void ConnectionsListModel::setFilter(QVariant tabName)
{
    if (tabName == i18n("Connections")) {
        currentFilter |= NormalConnections;
        currentFilter &= ~VpnConnections;
        currentFilter &= ~SharedConnections;
    } else if (tabName == i18n("VPN")) {
        currentFilter &= ~NormalConnections;
        currentFilter |= VpnConnections;
        currentFilter &= ~SharedConnections;
    } else if (tabName == i18n("Shared Connections")) {
        currentFilter &= ~VpnConnections;
        currentFilter &= ~SharedConnections;
        currentFilter |= SharedConnections;
    }

    updateConnectionsList();
}

void ConnectionsListModel::setDeviceToFilter(NetworkManager::Device* device, const bool vpn)
{
    m_device = device;

    // VpnInterfaceItems do not have NetworkManager::Device associated to them.
    if (m_device || vpn) {
        currentFilter |= FilterDevice;
    } else {
        currentFilter &= ~FilterDevice;
    }
    m_vpn = vpn;
    updateConnectionsList();
}

void ConnectionsListModel::updateConnectionsList()
{
    foreach(ConnectionItem * item, connections) {
        removeItem(item);
    }

    removeHiddenItem();

    if (m_device) {
        if ((currentFilter.testFlag(NormalConnections) && NetworkManager::isWirelessEnabled() && m_device->type() == NetworkManager::Device::Wifi) && !m_vpn) {
            insertHiddenItem();
        }
    } else {
        if ((currentFilter.testFlag(NormalConnections) && NetworkManager::isWirelessEnabled()) && !m_vpn) {
            insertHiddenItem();
        }
    }

    m_showMoreItem->networkCount() = 0;
    foreach(RemoteActivatable * activatable, m_activatables->activatables()) {
        if (m_device && activatable->deviceUni() != m_device->uni()) {
            continue;
        }
        if (currentFilter.testFlag(NormalConnections)) {
            if (activatable->activatableType() == Knm::Activatable::VpnInterfaceConnection ||
                activatable->isShared()) {
                continue;
            }
            if (activatable->activatableType() == Knm::Activatable::WirelessNetwork) {
                m_showMoreItem->networkCount()++; // number of suppressed networks in m_showMoreItem.
                if (currentFilter.testFlag(SavedConnections) || !NetworkManager::isWirelessEnabled()) {
                    continue;
                }
            }
            if (activatable->activatableType() == Knm::Activatable::WirelessInterfaceConnection) {
                RemoteWirelessInterfaceConnection * wic = static_cast<RemoteWirelessInterfaceConnection*>(activatable);
                if (!NetworkManager::isWirelessEnabled() ||
                    (wic->operationMode() == NetworkManager::WirelessDevice::Adhoc &&
                     wic->activationState() != Knm::InterfaceConnection::Activated)) {
                    m_showMoreItem->networkCount()++; // number of suppressed networks in m_showMoreItem.
                    continue;
                }
            }

            ConnectionItem *item = new ConnectionItem(activatable);
            appendRow(item);
        } else if (currentFilter.testFlag(VpnConnections) && activatable->activatableType() == Knm::Activatable::VpnInterfaceConnection) {
            ConnectionItem *item = new ConnectionItem(activatable);
            appendRow(item);
        } else if (currentFilter.testFlag(SharedConnections)) {
            if (activatable->isShared() && !((activatable->activatableType() == Knm::Activatable::WirelessInterfaceConnection ||
                                              activatable->activatableType() == Knm::Activatable::WirelessNetwork) && !NetworkManager::isWirelessEnabled()) && !m_vpn) {
                ConnectionItem *item = new ConnectionItem(activatable);
                appendRow(item);
            }
        }
    }

    updateShowMoreItem();
}

void ConnectionsListModel::connectToHiddenNetwork(QVariant ssidParam)
{
    QString ssid = ssidParam.toString();
    kDebug() << "ssid is: " << ssid;
    NetworkManager::WirelessDevice * wiface = 0;
    foreach(NetworkManager::Device * iface, NetworkManager::networkInterfaces()) {
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
