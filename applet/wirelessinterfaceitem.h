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

#ifndef WIRELESSINTERFACEITEM_H
#define WIRELESSINTERFACEITEM_H

#include <QHash>
#include "interfaceitem.h"

namespace Solid
{
namespace Control
{
    class AccessPoint;
    class WirelessNetworkInterface;
}
}

class WirelessEnvironment;
class WirelessNetwork;

/** Represents a wireless network interface in the popup
 * Provides custom UI for wireless connection status
 * Maintains a list of wireless networks visible to this network interface,
 * used by the interface's group to determine which connections are appropriate
 */
class WirelessInterfaceItem : public InterfaceItem
{
Q_OBJECT
public:
    WirelessInterfaceItem(Solid::Control::WirelessNetworkInterface * iface, NetworkManagerSettings * userSettings, NetworkManagerSettings * systemSettings, InterfaceItem::NameDisplayMode mode, QGraphicsItem* parent = 0);
    virtual ~WirelessInterfaceItem();
    WirelessEnvironment * wirelessEnvironment() const;
signals:
    void wirelessNetworksChanged();
public slots:
    void activeAccessPointChanged(const QString&);
    void activeSignalStrengthChanged(int);
    void accessPointDestroyed(QObject *);
private:
    void setConnectionInfo();
    Solid::Control::WirelessNetworkInterface * m_wirelessIface;
    Solid::Control::AccessPoint * m_activeAccessPoint;
    WirelessEnvironment * m_environment;
};
#endif // WIRELESSINTERFACEITEM_H
