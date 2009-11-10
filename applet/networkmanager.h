/*
Copyright 2008,2009 Will Stephenson <wstephenson@kde.org>

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

#ifndef PLASMA_NETWORKMANAGER_APPLET_H
#define PLASMA_NETWORKMANAGER_APPLET_H

#include <kdeversion.h>

#include <plasma/svg.h>
#include <solid/networking.h>
#include <solid/control/networking.h>
#include <solid/control/networkinterface.h>
#include <solid/control/wirelessaccesspoint.h>

#include "../libs/types.h"
//#include "vpnconnectiongroup.h"

#include "ui_nmConfig.h"

#include <plasma/popupapplet.h>

#include <Plasma/ToolTipManager>

namespace Plasma
{
    class Applet;
    class Extender;
} // namespace Plasma

class NMExtenderItem;
class RemoteActivatableList;

class NetworkManagerApplet : public Plasma::PopupApplet
{
Q_OBJECT
public:
    NetworkManagerApplet(QObject * parent, const QVariantList & args);
    ~NetworkManagerApplet();
    /* reimp Plasma::Applet */
    void init();
    /* reimp Plasma::Applet */
    void paintInterface(QPainter *painter, const QStyleOptionGraphicsItem * option, const QRect & rect);
    /* reimp Plasma::Applet */
    //QSizeF sizeHint(const Qt::SizeHint which, const QSizeF& constraint) const;
    /* reimp Plasma::Applet */
    //Qt::Orientations expandingDirections() const;
    /* reimp Plasma::Applet */
    void constraintsEvent(Plasma::Constraints constraints);
    virtual QList<QAction*> contextualActions();
    virtual void initExtenderItem(Plasma::ExtenderItem *);

    void loadExtender();

public Q_SLOTS:
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
     * React to manager status changes
     */
    void managerStatusChanged(Solid::Networking::Status);

signals:
    /**
     * Tell the applet to show our KCModule
     */
    //void manageConnections();

public Q_SLOTS:

protected Q_SLOTS:
    // called by Plasma::ToolTipManager
    void toolTipAboutToShow();
    void configAccepted();
protected:
    void createConfigurationInterface(KConfigDialog *parent);
    /**
     * Reimplemented from Plasma::PopupApplet
     */
    void popupEvent(bool);
private Q_SLOTS:
    void networkInterfaceAdded(const QString& = QString());
    void networkInterfaceRemoved(const QString&);
    void interfaceConnectionStateChanged();
    void manageConnections();
    // used to let the user easily hide VPN
    void hideVpnGroup();
    void updatePixmap();
    void positionPixmap();

private:
    bool hasInterfaceOfType(Solid::Control::NetworkInterface::Type type);
    Solid::Control::NetworkInterface* activeInterface();
    void setupInterface();

    void paintPixmap(QPainter *painter, QPixmap pixmap,
                     const QRectF &rect, qreal opacity = 1.0);

    Solid::Control::NetworkInterfaceList sortInterfacesByImportance(const Solid::Control::NetworkInterfaceList& interfaces) const;
    bool m_iconPerDevice;
    Solid::Control::NetworkInterfaceList m_interfaces;
    Plasma::ToolTipContent m_toolTip;
    // Configuration dialog
    Ui::nmConfig ui;
    RemoteActivatableList * m_activatableList;
    NMExtenderItem* m_extenderItem;

    QGraphicsPixmapItem *m_pixmapItem;

    // For tracking which status we should show
    Solid::Control::NetworkInterface *m_activeInterface;
    Solid::Control::AccessPoint *m_accessPoint;
};

#endif




