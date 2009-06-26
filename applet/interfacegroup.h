/*
Copyright 2008,2009 Will Stephenson <wstephenson@kde.org

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

#include <Plasma/Extender>

#include <solid/control/networkinterface.h>
#include <solid/control/networkmanager.h>


class HiddenWirelessNetworkItem;
class InterfaceItem;
class RemoteActivatable;
class RemoteActivatableList;

/** Represents a group of network interfaces of the same type
 * displays either a generic name of interface type (when representing a single interface)
 * or a specific interface name or model
 */
class InterfaceGroup : public ConnectionList
{
Q_OBJECT
public:
    InterfaceGroup(Solid::Control::NetworkInterface::Type type, RemoteActivatableList * list, Plasma::Extender * ext = 0);
    virtual ~InterfaceGroup();
    Solid::Control::NetworkInterface::Type interfaceType() const;
    bool accept(RemoteActivatable*) const;
    void setupHeader();
    void setupFooter();
    void setNetworksLimit( int wlans );

    /**
     * Inform the widget of a popupEvent in the containing applet popup.
     */
    virtual void popupEvent(bool show);

public Q_SLOTS:
    void setEnabled(bool enabled);
protected Q_SLOTS:
    // reimplemented from ConnectionList
    void activate(ActivatableItem*);
    //void connectToWirelessNetwork(AbstractConnectableItem*);
    virtual void interfaceAdded(const QString&);
    virtual void interfaceRemoved(const QString&);
    // used to re-populate the list of wireless networks, if necessary
    void refreshConnectionsAndNetworks();
    void updateNetworks();
    void updateConnections();
Q_SIGNALS:
    void updateLayout();

private:
    void addInterfaceInternal(Solid::Control::NetworkInterface *);
    void addWirelessNetworkInternal(const QString &ssid);
    // we only show the top N strongest networks, this controls which are visible
    //QList<AbstractWirelessNetwork*> networksToShow();
    // reimplemented from ConnectionList
    ActivatableItem * createItem(RemoteActivatable* conn);
    // list of interfaces
    QHash<QString, InterfaceItem *> m_interfaces;
    //QHash<QString, WirelessNetworkItem *> m_networks;
    HiddenWirelessNetworkItem * m_hiddenItem;
    Solid::Control::NetworkInterface::Type m_type;
    QGraphicsLinearLayout * m_interfaceLayout;
    QGraphicsLinearLayout * m_networkLayout;
    bool m_enabled;
    int m_numberOfWlans;
};
#endif // INTERFACEGROUP_H
