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

class QGraphicsLinearLayout;
namespace Plasma
{
class Label;
class PushButton;
} // namespace Plasma

class NetworkManagerPopup : public QGraphicsWidget
{
Q_OBJECT
public:
    NetworkManagerPopup(QGraphicsItem *parent);
    virtual ~NetworkManagerPopup();
private Q_SLOTS:
    void networkInterfaceAdded(const QString&);
    void networkInterfaceRemoved(const QString&);
    void overallStatusChanged(Solid::Networking::Status);
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
private:
    QGraphicsLinearLayout * m_layout;
    QGraphicsLinearLayout * m_connectionLayout;
    Plasma::Label * m_lblRfkill;
    Plasma::PushButton * m_btnEnableNetworking;
    Plasma::PushButton * m_btnEnableWireless;
    Plasma::PushButton * m_btnManageConnections;
};
#endif // NETWORKMANAGERPOPUP_H
