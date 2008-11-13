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

#include "connectionlist.h"

#include <solid/control/networkinterface.h>
#include <solid/control/networkmanager.h>

#include "nm-exported-connectioninterface.h"

class WirelessEnvironmentMerged;
class WirelessNetworkItem;

/** Represents a group of network interfaces of the same type
 * displays either a generic name of interface type (when representing a single interface)
 * or a specific interface name or model
 */
class InterfaceGroup : public ConnectionList
{
Q_OBJECT
public:
    InterfaceGroup(Solid::Control::NetworkInterface::Type type, NetworkManagerSettings * userSettings, NetworkManagerSettings * systemSettings, QWidget * parent = 0);
    virtual ~InterfaceGroup();
    Solid::Control::NetworkInterface::Type interfaceType() const;
    bool accept(RemoteConnection *) const;
    void setupHeader();
    void setupFooter();
protected Q_SLOTS:
    // reimplemented from ConnecitonList
    void activateConnection(AbstractConnectableItem*);
    void connectToWirelessNetwork(AbstractConnectableItem*);
    virtual void interfaceAdded(const QString&);
    virtual void interfaceRemoved(const QString&);
    void wirelessNetworkAppeared(const QString&);
    void wirelessNetworkDisappeared(const QString&);

Q_SIGNALS:
        void updateLayout();


private:
    void addInterfaceInternal(Solid::Control::NetworkInterface *);
    void addNetworkInternal(const QString &ssid);
    // we only show the top N strongest networks, this controls which are visible
    void updateWirelessNetworkLayout();
    // reimplemented from ConnecitonList
    ConnectionItem * createItem(RemoteConnection* conn);
    // list of interfaces
    QHash<QString, InterfaceItem *> m_interfaces;
    QHash<QString, WirelessNetworkItem *> m_networks;
    Solid::Control::NetworkInterface::Type m_type;
    WirelessEnvironmentMerged * m_wirelessEnvironment;
    QVBoxLayout * m_interfaceLayout;
    QVBoxLayout * m_networkLayout;
};
#endif // INTERFACEGROUP_H
