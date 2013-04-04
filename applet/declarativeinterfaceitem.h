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

#ifndef APPLET_DECLARATIVE_INTERFACEITEM_H
#define APPLET_DECLARATIVE_INTERFACEITEM_H

#include <QObject>
#include <QDBusObjectPath>
#include <QGraphicsWidget>

#include <QtNetworkManager/device.h>

#include "interfaceconnection.h"
#include "remoteactivatable.h"

namespace NetworkManager
{
class Device;
}

class RemoteInterfaceConnection;
class RemoteActivatableList;

class DeclarativeInterfaceItem : public QObject
{
    Q_OBJECT
public:
    enum NameDisplayMode {InterfaceName, HardwareName};
    DeclarativeInterfaceItem(const NetworkManager::Device::Ptr &iface, RemoteActivatableList* activatables, NameDisplayMode mode = InterfaceName,  QObject* parent = 0);

    NetworkManager::Device::Ptr interface() const;
    bool equals(const DeclarativeInterfaceItem *item);
    QString connectionName();
    QString connection();
    QString type() const;
    QString interfaceTitle();
    QString deviceUni();
    QString icon();
    bool enabled();
    bool isVisible();
    bool defaultRoute();
    void setNameDisplayMode(NameDisplayMode mode);

Q_SIGNALS:
    void stateChanged();
    void itemChanged();

public Q_SLOTS:
    void activeConnectionsChanged();
    void currentConnectionChanged();
    void serviceDisappeared();
    void serviceAppeared();
    void activatableAdded(RemoteActivatable*);
    void activatableRemoved(RemoteActivatable*);
    void updateCurrentConnection(RemoteInterfaceConnection *);
    void stateChanged(NetworkManager::Device::State, bool updateConnection = true);
    void handleConnectionStateChange(NetworkManager::Device::State new_state, NetworkManager::Device::State old_state, NetworkManager::Device::StateChangeReason reason);
    void handleHasDefaultRouteChanged(bool);
    void setConnectionInfo();

private:
    void setEnabled(bool enabled);
    bool accept(RemoteActivatable* activatable) const;
    void setInterfaceIcon();

    QList<RemoteActivatable*> m_vpnActivatables;
    NetworkManager::Device::Ptr m_iface;

    RemoteInterfaceConnection* m_currentConnection;
    RemoteActivatableList* m_activatables;

    NameDisplayMode m_nameMode;
    bool m_enabled;
    NetworkManager::Device::State m_state;
    QString m_interfaceName;
    QString m_connectionName;
    QString m_interfaceTitle;
    QString m_icon;
    QString m_type;
    bool m_disconnect;
    bool m_visible;
    bool m_hasDefaultRoute;
    bool m_starting;
};

#endif
