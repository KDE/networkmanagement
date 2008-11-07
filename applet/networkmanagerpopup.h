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

#ifndef NETWORKMANAGERPOPUP_H
#define NETWORKMANAGERPOPUP_H

#include <QGraphicsWidget>

#include <solid/networking.h>
#include <solid/control/networking.h>
#include <solid/control/networkinterface.h>

#include "../libs/types.h"

class QGraphicsLinearLayout;
class QSignalMapper;

namespace Plasma
{
class CheckBox;
class IconWidget;
class Label;
} // namespace Plasma

class InterfaceGroup;
class NetworkManagerSettings;
class OrgFreedesktopNetworkManagerSettingsConnectionInterface;

/**
 * Widget that pops up when the menu is clicked
 */
class NetworkManagerPopup : public QGraphicsWidget
{
Q_OBJECT
public:
    NetworkManagerPopup(QGraphicsItem *parent);
    virtual ~NetworkManagerPopup();
public Q_SLOTS:
    /** slots called when a connection in the popup is clicked */
    void activateConnection(const QString&);
    void deactivateConnection(const QString&);

    /** Hack to update the layouts */
    void updateLayout();

    /**
     * Update the popup and notify on device changes
     */
    //void networkInterfaceAdded(const QString&);
    //void networkInterfaceRemoved(const QString&);

    /**
     * Update the popup and notify on wireless changes
     */
    //void accessPointAppeared(const QString &);
    //void accessPointDisappeared(const QString &);

    /**
     * Update the popup and notify on configuration changes
     */
    //void connectionAdded();
    //void connectionRemoved();

    //void overallStatusChanged(Solid::Networking::Status);
    /**
     * Handle signals from NM if wireless was disabled in software
     */
    void managerWirelessEnabledChanged(bool);
    /**
     * Handle signals from NM if wireless was disabled in hardware
     */
    void managerWirelessHardwareEnabledChanged(bool);
    /**
     * Handle clicks to enable/disable enabled
     */
    void userNetworkingEnabledChanged(bool);
    /**
     * Handle clicks to enable/disable wireless
     */
    void userWirelessEnabledChanged(bool);
    /**
     * Show our KCModule
     */
    void manageConnections();
    /**
     * React to manager status changes
     */
    void managerStatusChanged(Solid::Networking::Status);
private:
    NetworkManagerSettings * m_userSettings;
    NetworkManagerSettings * m_systemSettings;
    QGraphicsLinearLayout * m_layout;
    QGraphicsLinearLayout * m_connectionLayout;
    InterfaceGroup * m_ethernetGroup;
    InterfaceGroup * m_wifiGroup;
    InterfaceGroup * m_gsmGroup;
    Plasma::Label * m_notRunning;
    Plasma::Label * m_lblRfkill;
    Plasma::CheckBox * m_btnEnableNetworking;
    Plasma::CheckBox * m_btnEnableWireless;
    Plasma::IconWidget * m_btnManageConnections;
    QSignalMapper * m_connectionActivationSignalMapper;
    QSignalMapper * m_connectionDeactivationSignalMapper;
};
#endif // NETWORKMANAGERPOPUP_H
