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

#include "../libs/types.h"
//#include "vpnconnectiongroup.h"

#include "ui_nmConfig.h"

#include <plasma/popupapplet.h>

#include <Plasma/ToolTipManager>

#include <solid/control/networkinterface.h>

namespace Plasma
{
    class Applet;
    class Extender;
} // namespace Plasma

class InterfaceGroup;
class RemoteActivatableList;

class NetworkManagerApplet : public Plasma::PopupApplet
{
Q_OBJECT
public:
    NetworkManagerApplet(QObject * parent, const QVariantList & args);
    ~NetworkManagerApplet();
    /* reimp Plasma::Applet */
    void init();
    /* reimp Plasma::PopupApplet */
    void initExtenderItem(Plasma::ExtenderItem *);
    /* reimp Plasma::Applet */
    void paintInterface(QPainter *painter, const QStyleOptionGraphicsItem * option, const QRect & rect);
    /* reimp Plasma::Applet */
    //QSizeF sizeHint(const Qt::SizeHint which, const QSizeF& constraint) const;
    /* reimp Plasma::Applet */
    //Qt::Orientations expandingDirections() const;
    /* reimp Plasma::Applet */
    void constraintsEvent(Plasma::Constraints constraints);
    virtual QList<QAction*> contextualActions();

    static QString connectionStateToString(Solid::Control::NetworkInterface::ConnectionState state);

    void showWired(bool show);
    void showWireless(bool show);
    void showVpn(bool show);
    void showCellular(bool show);
    void showGeneral(bool show);

public Q_SLOTS:
    /** slots called when a connection in the popup is clicked */
    void activateConnection(const QString&);
    void deactivateConnection(const QString&);

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
private:
    bool hasInterfaceOfType(Solid::Control::NetworkInterface::Type type);
    void updateIcons();
    void paintDefaultInterface(Solid::Control::NetworkInterface*, QPainter *painter, const QStyleOptionGraphicsItem * option, const QRect & rect);
    void paintWiredInterface(Solid::Control::NetworkInterface*, QPainter *painter, const QStyleOptionGraphicsItem * option, const QRect & rect);
    void paintWirelessInterface(Solid::Control::NetworkInterface*, QPainter *painter, const QStyleOptionGraphicsItem * option, const QRect & rect);

    Solid::Control::NetworkInterfaceList sortInterfacesByImportance(const Solid::Control::NetworkInterfaceList& interfaces) const;
    bool m_iconPerDevice;
    Plasma::Svg *m_svg;
    Plasma::Svg *m_wirelessSvg;
    QPixmap m_pixmapWiredConnected;
    QPixmap m_pixmapWiredDisconnected;
    Solid::Control::NetworkInterfaceList m_interfaces;
    QString m_elementName;
    Plasma::ToolTipContent m_toolTip;
    // Configuration dialog
    Ui::nmConfig ui;

    InterfaceGroup * m_wifiGroup;
    bool m_showWired;
    bool m_showWireless;
    bool m_showVpn;
    bool m_showCellular;
    bool m_showGeneral;
    int m_numberWirelessShown;
    RemoteActivatableList * m_activatableList;
};

#endif




