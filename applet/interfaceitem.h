/*
Copyright 2008,2009 Will Stephenson <wstephenson@kde.org>
Copyright 2008, 2009 Sebastian Kügler <sebas@kde.org>

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

#include <solid/control/networkinterface.h>

#include "interfaceconnection.h"

#include <Plasma/Frame>
#include <Plasma/IconWidget>
#include <Plasma/Label>
#include <Plasma/Meter>
#include <Plasma/CheckBox>

class QGraphicsGridLayout;
class QGraphicsLinearLayout;
namespace Solid
{
    namespace Control
    {
        class NetworkInterface;
    }
}


/**
 * Represents a single network interface
 * Displays status, updates itself
 * Allows deactivating any active connection
 */
class InterfaceItem : public Plasma::Frame
{
Q_OBJECT
public:
    enum NameDisplayMode {InterfaceName, HardwareName};
    InterfaceItem(Solid::Control::NetworkInterface* iface, NameDisplayMode mode = InterfaceName, QGraphicsWidget* parent = 0);
    virtual ~InterfaceItem();

    void setNameDisplayMode(NameDisplayMode);
    NameDisplayMode nameDisplayMode() const;

    virtual QString connectionName();
    QString label();

public Q_SLOTS:
    void activeConnectionsChanged();
    void connectionStateChanged(Solid::Control::NetworkInterface::ConnectionState);
    virtual void setEnabled(bool enable);
    // also updates the connection info
    virtual void setActive(bool active);

protected Q_SLOTS:
    /**
     * Remove any connections that were provided by this service
     * from our active connection list
     */
    void handleConnectionStateChange(int new_state);
    void handleConnectionStateChange(int new_state, int old_state, int reason);
    void pppStats(uint in, uint out);

Q_SIGNALS:
    void stateChanged();
    void disconnectInterface();

protected:
    /**
     * Fill in interface type connection info
     */
    virtual void setConnectionInfo();
    /**
     * Give us a pixmap for an icon
     */
    virtual QPixmap statePixmap(const QString &icon);

    /**
    * The current IP address when the connection is active.
    */
    virtual QString currentIpAddress();

    Solid::Control::NetworkInterface* m_iface;

    QGraphicsGridLayout* m_layout;
    QGraphicsLinearLayout* m_infoLayout;
    Plasma::IconWidget* m_icon;
    Plasma::IconWidget* m_disconnectButton;
    Plasma::Label* m_ifaceNameLabel;
    Plasma::Label* m_connectionNameLabel;
    QGraphicsLinearLayout* m_connectionInfoLayout;
    Plasma::Label* m_connectionInfoLabel;
    Plasma::Label* m_connectionInfoStrengthLabel;
    Plasma::IconWidget* m_connectionInfoIcon;
    NameDisplayMode m_nameMode;
    bool m_enabled;

    QString m_interfaceName;
    bool m_disconnect;
};
#endif // APPLET_INTERFACEWIDGET_H
