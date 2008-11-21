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

#ifndef APPLET_INTERFACEITEM_H
#define APPLET_INTERFACEITEM_H

#include <QDBusObjectPath>
#include <QGraphicsWidget>

#include <Plasma/IconWidget>
#include <Plasma/Label>
#include <Plasma/Meter>

class QGraphicsGridLayout;
class QGraphicsLinearLayout;
namespace Solid
{
    namespace Control
    {
        class NetworkInterface;
    } // namespace Control
} // namespace Solid

namespace Plasma
{
    class IconWidget;
    class Label;
} // namespace Plasma

class ConnectionInspector;
class NetworkManagerSettings;
class RemoteConnection;
class WirelessNetwork;
typedef QPair<NetworkManagerSettings*, RemoteConnection*> ActiveConnectionPair;
/**
 * Represents a single network interface
 * Displays status, updates itself
 * Allows deactivating any active connection
 */
class InterfaceItem : public QGraphicsWidget
{
Q_OBJECT
public:
    enum NameDisplayMode {InterfaceName, HardwareName};
    InterfaceItem(Solid::Control::NetworkInterface * iface, NetworkManagerSettings * userSettings, NetworkManagerSettings * systemSettings, NameDisplayMode mode = InterfaceName, QGraphicsItem* parent = 0);
    virtual ~InterfaceItem();
    void setNameDisplayMode(NameDisplayMode);
    NameDisplayMode nameDisplayMode() const;
    /**
     * Each interface item maintains a connection inspector to approve connections
     */
    void setConnectionInspector(ConnectionInspector *);
    ConnectionInspector * connectionInspector() const;
    uint activeConnectionCount() const;
    virtual QString ssid();

public slots:
    void activeConnectionsChanged();
    void connectionStateChanged(int);
    virtual void connectButtonClicked() = 0;

protected Q_SLOTS:
    /**
     * Remove any connections that were provided by this service
     * from our active connection list
     */
    void serviceDisappeared(NetworkManagerSettings *service);
Q_SIGNALS:
    void stateChanged();
protected:
    /**
     * Fill in interface type connection info
     */
    virtual void setConnectionInfo();
    /**
     * The interface is unavailable for connections, update the UI
     */
    virtual void setUnavailable();
    /**
     * The interface is inactive but could be activated
     */
    virtual void setInactive();
    /**
     * The interface is currently active, update the UI
     */
    virtual void setActiveConnection(int);
    /**
     * The active connection's ssid or empty
     */

    /**
     * Returns all available connections for the device type.
     */
    QList<RemoteConnection*> availableConnections() const;

    Solid::Control::NetworkInterface * m_iface;
    NetworkManagerSettings * m_userSettings;
    NetworkManagerSettings * m_systemSettings;
    //QGraphicsLinearLayout * m_layout; // use with nested linear based layouts, when they work
    QGraphicsGridLayout * m_layout;
    QGraphicsLinearLayout * m_infoLayout;
    Plasma::IconWidget * m_icon;
    Plasma::Label * m_ifaceNameLabel;
    Plasma::Label * m_connectionNameLabel;
    QGraphicsLinearLayout * m_connectionInfoLayout;
    Plasma::Label * m_connectionInfoLabel;
    Plasma::Meter * m_strengthMeter;
    Plasma::Label * m_connectionInfoStrengthLabel;
    Plasma::IconWidget * m_connectionInfoIcon;
    Plasma::IconWidget * m_connectButton;
    NameDisplayMode m_nameMode;

    QList<ActiveConnectionPair> m_activeConnections;
    ConnectionInspector * m_connectionInspector;
};
#endif // APPLET_INTERFACEWIDGET_H
