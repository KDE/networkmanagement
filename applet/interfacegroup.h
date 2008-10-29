/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>

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

#ifndef INTERFACEGROUP_H
#define INTERFACEGROUP_H

#include <QGraphicsWidget>
#include <QHash>
#include <QPair>

#include <solid/control/networkinterface.h>
#include <solid/control/networkmanager.h>

#include "nm-exported-connectioninterface.h"


class NetworkManagerSettings;
class QGraphicsLinearLayout;
class ConnectionItem;
class InterfaceItem;
class RemoteConnection;
class WirelessEnvironment;

typedef QPair<QString,QString> QStringPair;
typedef QHash<QStringPair, ConnectionItem*> ServiceConnectionHash;

/** Represents a group of network interfaces of the same type
 * displays either a generic name of interface type (when representing a single interface)
 * or a specific interface name or model
 */
class InterfaceGroup : public QGraphicsWidget
{
Q_OBJECT
public:
    InterfaceGroup(Solid::Control::NetworkInterface::Type type, NetworkManagerSettings * userSettings, NetworkManagerSettings * systemSettings, QGraphicsWidget * parent = 0);
    virtual ~InterfaceGroup();
    Solid::Control::NetworkInterface::Type interfaceType() const;
protected slots:
    virtual void activateConnection(ConnectionItem*);
    virtual void interfaceAdded(const QString&);
    virtual void interfaceRemoved(const QString&);
    void connectionAddedToService(NetworkManagerSettings *, const QString&);
    void connectionRemovedFromService(NetworkManagerSettings *, const QString&);
    void serviceAppeared(NetworkManagerSettings*);
    void serviceDisappeared(NetworkManagerSettings*);
    void reassessConnectionList();
private:
    void addInterfaceInternal(Solid::Control::NetworkInterface *);
    void addConnectionInternal(NetworkManagerSettings*, const QString &connectionPath);
    void addSettingsService(NetworkManagerSettings*);
    // list of connection objects for this interface type
    ServiceConnectionHash m_connections;
    // list of interfaces
    QHash<QString, InterfaceItem *> m_interfaces;
    QGraphicsLinearLayout * m_layout;
    Solid::Control::NetworkInterface::Type m_type;
    NetworkManagerSettings * m_userSettings;
    NetworkManagerSettings * m_systemSettings;
    WirelessEnvironment * m_wirelessEnvironment;
};
#endif // INTERFACEGROUP_H
