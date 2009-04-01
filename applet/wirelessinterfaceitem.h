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
#include <QString>
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
class AbstractWirelessNetwork;

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
    /**
     * SSID of any active network, or an invalid QString if none
     */
    QString ssid();
    /** @reimp InterfaceItem */
    virtual void setEnabled(bool enable);
    /**
     * Checks whether this interface is using the supplied wireless network
     * At the moment it only checks on SSID, but this should be extended to check on BSSIDs in the
     * network, as SSID is an insufficient uniqueness guarantee
     */
    bool isUsing(const AbstractWirelessNetwork *) const;
public Q_SLOTS:
    void activeAccessPointChanged(const QString&);
    void activeSignalStrengthChanged(int);
    void accessPointDestroyed(QObject *);
    void connectButtonClicked();

private:
    void setConnectionInfo();

    /// Returns a list of connections that can be applied to available access points.
    QList<RemoteConnection*> appropriateConnections(const QList<RemoteConnection*> &connections, const QList<Solid::Control::AccessPoint*> accesspoints) const;
    /// returns a list of available accesspoint objects
    QList<Solid::Control::AccessPoint*> availableAccessPoints() const;

    Solid::Control::WirelessNetworkInterface * m_wirelessIface;
    Solid::Control::AccessPoint * m_activeAccessPoint;
    WirelessEnvironment * m_environment;
};
#endif // WIRELESSINTERFACEITEM_H
