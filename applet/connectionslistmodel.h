#ifndef CONNECTIONSLISTMODEL_H
#define CONNECTIONSLISTMODEL_H

#include <QAbstractListModel>
#include "remoteactivatablelist.h"
#include "remoteactivatable.h"
#include "remotewirelessnetwork.h"
#include "connectionitem.h"
#include <KDebug>


class ConnectionsListModel : public QAbstractListModel {
    Q_OBJECT

public:

    enum Roles {
        DeviceUniRole = Qt::UserRole+1,
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
        ConnectedRole,
        StatusRole,
        ProtectedIconRole
    };

    ConnectionsListModel(QObject *parent = 0);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    int rowCount(const QModelIndex & parent = QModelIndex()) const;

    void appendRow(ConnectionItem *item);

    void appendRows(const QList<ConnectionItem*> &items);

    bool removeRow(int row, const QModelIndex &parent = QModelIndex());

    bool removeItem(ConnectionItem *act);

    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

    QModelIndex indexFromItem(const ConnectionItem *item) const;

public slots:
    void itemChanged();
    void disconnectFrom(QVariant uuid);
    void connectTo(int index);

private:
    QList<ConnectionItem *> connections;

};

#endif // CONNECTIONSLISTMODEL_H
