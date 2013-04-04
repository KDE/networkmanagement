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

#ifndef CONNECTIONSLISTMODEL_H
#define CONNECTIONSLISTMODEL_H

#include <QAbstractListModel>
#include "remoteactivatablelist.h"
#include "remoteactivatable.h"
#include "remotewirelessnetwork.h"
#include "connectionitem.h"
#include <KDebug>


class ConnectionsListModel : public QAbstractListModel
{
    Q_OBJECT

public:

    enum Roles {
        DeviceUniRole = Qt::UserRole + 1,
        ActivatableTypeRole,
        IsSharedRole,
        ConnectionTypeRole,
        ConnectionUuidRole,
        ConnectionNameRole,
        ActivationStateRole,
        OldActivationStateRole,
        HasDefaultRouteRole,
        SsidRole,
        StrengthRole,
        InterfaceCapabilitiesRole,
        ApCapabilitiesRole,
        WpaFlagsRole,
        RsnFlagsRole,
        OperationModeRole,
        StatusRole,
        ProtectedIconRole,
        HiddenRole,
        HoverEnterRole,
        ConnectionIconRole,
        SignalQualityRole,
        AccessTechnologyRole,
        ShowMoreCheckedRole,
        NetworkCountRole
    };

    enum FilterType {NormalConnections = 0x1, VpnConnections = 0x2, SharedConnections = 0x4, SavedConnections = 0x8, FilterDevice = 0x10};
    Q_DECLARE_FLAGS(FilterTypes, FilterType)

    explicit ConnectionsListModel(RemoteActivatableList *activatables, QObject *parent = 0);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    int rowCount(const QModelIndex & parent = QModelIndex()) const;

    void appendRow(ConnectionItem *item);
    void appendRows(const QList<ConnectionItem*> &items);

    void insertHiddenItem();
    void removeHiddenItem();

    void updateShowMoreItem();

    bool removeRow(int row, const QModelIndex &parent = QModelIndex());

    bool removeItem(ConnectionItem *act);

    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

    QModelIndex indexFromItem(const ConnectionItem *item) const;

    void updateConnectionsList();

    void setDeviceToFilter(const NetworkManager::Device::Ptr &device = NetworkManager::Device::Ptr(), const bool vpn = false);

    void setHasWireless(const bool hasWireless) { m_hasWireless = hasWireless; }

Q_SIGNALS:
    void showInterfaceDetails(QString);

public slots:
    void itemChanged();
    void disconnectFrom(QVariant uuid);
    void connectTo(int index);
    void connectToHiddenNetwork(QVariant ssid);
    void setFilter(QVariant tabName);
    void deactivateConnection(const QString& deviceUni);
    void hoverEnterConnections(QString deviceUni);
    void hoverLeftConnections(QString deviceUni);
    void hoverEnterVpn();
    void hoverLeftVpn();
    void activatableAdded(RemoteActivatable *activatable);
    void activatableRemoved(RemoteActivatable *activatable);
    void showMoreClicked();
    void showMore(const bool);
    void checkShowMore(RemoteActivatable *);
    void uncheckShowMore(RemoteActivatable *);

private:
    QList<ConnectionItem *> connections;
    ConnectionItem *m_showMoreItem;
    NetworkManager::Device::Ptr m_device;
    RemoteActivatableList* m_activatables;
    bool hiddenInserted;
    bool m_vpn;
    FilterTypes currentFilter;
    int m_moreNetworks;
    bool m_showMoreChecked;
    bool m_oldShowMoreChecked;
    bool m_oldShowInterfaceList;
    int wicCount;
    bool m_hasWireless;
};

#endif // CONNECTIONSLISTMODEL_H
