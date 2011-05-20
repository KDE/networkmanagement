/*
Copyright 2008,2009 Will Stephenson <wstephenson@kde.org>
Copyright 2008-2010 Sebastian Kügler <sebas@kde.org>

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

class QAction;
class KCModuleProxy;

#include <kdeversion.h>
#include <QTimeLine>

#include <plasma/framesvg.h>
#include <plasma/svg.h>
#include <solid/networking.h>
#include <libnm-qt/device.h>
#include <libnm-qt/accesspoint.h>

#include <libnm-qt/generic-types.h>

#include <Plasma/PopupApplet>

#include <Plasma/ToolTipManager>

namespace Plasma
{
    class Applet;
} // namespace Plasma

class NMPopup;
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
    void paintInterface(QPainter *painter, const QStyleOptionGraphicsItem * option,
                                            const QRect & rect);
    /* reimp Plasma::Applet */
    //QSizeF sizeHint(const Qt::SizeHint which, const QSizeF& constraint) const;
    /* reimp Plasma::Applet */
    //Qt::Orientations expandingDirections() const;
    /* reimp Plasma::Applet */
    void constraintsEvent(Plasma::Constraints constraints);
    QGraphicsWidget *graphicsWidget();

    //virtual void initExtenderItem(Plasma::ExtenderItem *);

    //void loadExtender();

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
    void configChanged();

protected:
    void createConfigurationInterface(KConfigDialog *parent);

protected Q_SLOTS:
    // called by Plasma::ToolTipManager
    void toolTipAboutToShow();

private Q_SLOTS:
    void networkInterfaceAdded(const QString& = QString());
    void networkInterfaceRemoved(const QString&);
    void interfaceConnectionStateChanged();
    void updatePixmap();
    void repaint();
    void clearActivatedOverlay();
    // Request KCM module to persist changes
    void saveConfiguration();

private:
    bool hasInterfaceOfType(NetworkManager::Device::Type type);
    NetworkManager::Device* activeInterface();
    void setupInterfaceSignals();
    QString svgElement(NetworkManager::Device *iface);

    void paintPixmap(QPainter* painter, QPixmap pixmap,
                     const QRectF &rect, qreal opacity = 1.0);
    void paintStatusOverlay(QPainter* p);
    void paintNeedAuthOverlay(QPainter* p);
    QPixmap generateProgressStatusOverlay();
    void setStatusOverlay(const QPixmap&);
    void setStatusOverlay(const QString&);

    bool m_iconPerDevice;
    NetworkManager::DeviceList m_interfaces;
    Plasma::ToolTipContent m_toolTip;

    RemoteActivatableList* m_activatables;
    NMPopup* m_popup;

    QPixmap m_pixmap;

    // For tracking which status we should show
    NetworkManager::Device* m_activeInterface;
    NetworkManager::AccessPoint* m_accessPoint;

    // Timeline controlling a connection progress overlay on the main icon
    QTimeLine m_overlayTimeline;
    QPixmap m_previousStatusOverlay;
    QPixmap m_statusOverlay;
    NetworkManager::Device::State m_currentState;

    ///embedded KCM modules in the configuration dialog
    KCModuleProxy* m_kcmNM;
    KCModuleProxy* m_kcmNMTray;

    Plasma::Svg* m_svg;
    Plasma::FrameSvg* m_meterBgSvg;
    Plasma::FrameSvg* m_meterFgSvg;
    QRect m_contentSquare;

};

#endif




