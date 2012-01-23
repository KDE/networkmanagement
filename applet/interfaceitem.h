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
#include "remoteactivatable.h"
#include "types.h"

#include <Plasma/Frame>
#include <Plasma/IconWidget>
#include <Plasma/Label>
#include <Plasma/Meter>
#include <Plasma/CheckBox>
#include <Plasma/IconWidget>

class QGraphicsGridLayout;
class QGraphicsLinearLayout;
namespace Solid
{
    namespace Control
    {
        class NetworkInterface;
    }
}

class RemoteInterfaceConnection;
class RemoteActivatableList;


/**
 * Represents a single network interface
 * Displays status, updates itself
 * Allows deactivating any active connection
 */
class InterfaceItem : public Plasma::IconWidget
{
Q_OBJECT
public:
    enum NameDisplayMode {InterfaceName, HardwareName};
    InterfaceItem(Solid::Control::NetworkInterfaceNm09* iface, RemoteActivatableList* activatables, NameDisplayMode mode = InterfaceName,  QGraphicsWidget* parent = 0);
    virtual ~InterfaceItem();

    void setNameDisplayMode(NameDisplayMode);
    NameDisplayMode nameDisplayMode() const;
    Solid::Control::NetworkInterfaceNm09* interface();
    virtual QString connectionName();
    QString label();
    virtual void setActivatableList(RemoteActivatableList* activatables);
    virtual QString currentIpAddress();
    void disappear();

public Q_SLOTS:
    void activeConnectionsChanged();
    virtual void connectionStateChanged(Solid::Control::NetworkInterfaceNm09::ConnectionState, bool updateConnection = true);
    virtual void setEnabled(bool enable);
    // also updates the connection info
    virtual void setActive(bool active);

Q_SIGNALS:
    void disappearAnimationFinished();

protected Q_SLOTS:
    /**
     * Remove any connections that were provided by this service
     * from our active connection list
     */
    void handleConnectionStateChange(int new_state);
    void handleConnectionStateChange(int new_state, int old_state, int reason);
    void handleHasDefaultRouteChanged(bool);
    void pppStats(uint in, uint out);
    void slotClicked();
    virtual void setConnectionInfo();

Q_SIGNALS:
    void stateChanged();
    void disconnectInterfaceRequested(const QString& deviceUni);
    void clicked(Solid::Control::NetworkInterfaceNm09*);
    void hoverEnter(const QString& uni = QString());
    void hoverLeave(const QString& uni = QString());

protected:
    /**
     * Fill in interface type connection info
     */
    /**
     * Give us a pixmap for an icon
     */
    virtual QPixmap interfacePixmap(const QString &icon = QString());

    /**
    * The current IP address when the connection is active.
    */
   //----- virtual QString currentIpAddress();
    virtual RemoteInterfaceConnection* currentConnection();
    void showItem(QGraphicsWidget* widget, bool show = true);

    RemoteInterfaceConnection* m_currentConnection;

    QWeakPointer<Solid::Control::NetworkInterfaceNm09> m_iface;
    RemoteActivatableList* m_activatables;

    QGraphicsGridLayout* m_layout;
    QGraphicsLinearLayout* m_infoLayout;
    Plasma::Label* m_icon;
    QPixmap* m_pixmap;
    Plasma::IconWidget* m_disconnectButton;
    Plasma::Label* m_ifaceNameLabel;
    Plasma::Label* m_connectionNameLabel;
    QGraphicsLinearLayout* m_connectionInfoLayout;
    Plasma::Label* m_connectionInfoLabel;
    Plasma::Label* m_connectionInfoStrengthLabel;
    Plasma::Label* m_connectionInfoIcon;
    NameDisplayMode m_nameMode;
    bool m_enabled;
    Solid::Control::NetworkInterfaceNm09::ConnectionState m_state;
    QString m_interfaceName;
    bool m_disconnect;
    bool m_hasDefaultRoute;
    QSize m_pixmapSize;
    bool m_starting;

    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

protected Q_SLOTS:
    virtual void currentConnectionChanged();

private Q_SLOTS:
    void emitDisconnectInterfaceRequest();
    void serviceDisappeared();
    void activatableAdded(RemoteActivatable*);
    void activatableRemoved(RemoteActivatable*);
    void updateCurrentConnection(RemoteInterfaceConnection *);
};
#endif // APPLET_INTERFACEWIDGET_H
