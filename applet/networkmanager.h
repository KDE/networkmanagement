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

#include <QTimeLine>

#include <kdeversion.h>

#include <plasma/framesvg.h>
#include <plasma/svg.h>
#include <NetworkManagerQt/manager.h>
#include <NetworkManagerQt/accesspoint.h>

#include <interfaceconnection.h>

#include <Plasma/PopupApplet>

#include <Plasma/ToolTipManager>

namespace Plasma
{
    class Applet;
} // namespace Plasma

class RemoteActivatable;
class RemoteActivatableList;
class RemoteInterfaceConnection;

class NetworkManagerApplet : public Plasma::PopupApplet
{
Q_OBJECT
public:
    enum SystrayOverlayOption {
        NoOverlay       = 0x00,
        Locked          = 0x01,
        PreviousOverlay = 0x02,
        StatusOverlay   = 0x04,
        NeedAuth        = 0x08,
    };
    Q_DECLARE_FLAGS(SystrayOverlayOptions, SystrayOverlayOption);

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

    //virtual void initExtenderItem(Plasma::ExtenderItem *);

    //void loadExtender();

    QList<QAction*> contextualActions();

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
    void managerStatusChanged(NetworkManager::Status);

protected:
    void createConfigurationInterface(KConfigDialog *parent);

protected Q_SLOTS:
    // called by Plasma::ToolTipManager
    void toolTipAboutToShow();

private Q_SLOTS:
    void deviceAdded(const QString& = QString());
    void deviceRemoved(const QString&);
    void interfaceConnectionStateChanged();
    void updatePixmap();
    void repaint();
    void clearActivatedOverlay();
    void finishInitialization();
    void activatableAdded(RemoteActivatable*);
    void activatableRemoved(RemoteActivatable*);
    void vpnActivationStateChanged(Knm::InterfaceConnection::ActivationState, Knm::InterfaceConnection::ActivationState);
    void activatablesDisappeared();
    // Request KCM module to persist changes
    void saveConfiguration();
    void updateActiveInterface(bool);
    void resetActiveSystrayInterface();
    void _k_destroyed(QObject *);
    void setupAccessPointSignals(const QString &);
    void qmlCreationFinished();
    void updateWireless(bool checked);
private:
    SystrayOverlayOptions m_systrayOverlayOption;

    bool needToUpdateOverlay();
    bool hasInterfaceOfType(NetworkManager::Device::Type type);
    void setupInterfaceSignals();
    QString svgElement(const NetworkManager::Device::Ptr &iface);
    QString m_currentSvgElement;
    QString m_currentPixmapIconName;

    void paintStatusOverlay(QPainter* p, QRect & rect);
    void paintNeedAuthOverlay(QPainter* p, QRect & rect);
    QPixmap generateProgressStatusOverlay(const qreal state);
    QPixmap generateProgressStatusOverlay();
    QPixmap generateVpnProgressStatusOverlay(const RemoteInterfaceConnection *ic);
    void setStatusOverlay(const QPixmap&);
    void setStatusOverlay(const QString&);
    inline void setActiveInterface(const NetworkManager::Device::Ptr &device = NetworkManager::Device::Ptr());
    inline void setActiveSystrayInterface(const NetworkManager::Device::Ptr &device = NetworkManager::Device::Ptr());

    void updateInterfaceList();
    NetworkManager::Device::List m_interfaces;
    Plasma::ToolTipContent m_toolTip;

    RemoteActivatableList* m_activatables;

    bool m_panelContainment;

    QPixmap m_pixmap;
    QMap<QUuid, QWeakPointer<RemoteInterfaceConnection> > m_activeVpnConnections;
    int m_totalActiveVpnConnections;

    // For tracking which status we should show.
    // This one is always the interface which has the default route
    // or the first interface selected after sorting the current
    // interface list using networkInterfaceLessThan().
    // WARNING: do not directly assign this pointer, use setActiveInterface instead.
    NetworkManager::Device::Ptr m_activeInterface;
    // Interface used to update system tray icon. If we have only one interface
    // then this one is always equals to m_activeInterfaceState.
    // WARNING: do not directly assign this pointer, use setActiveSystrayInterface instead.
    NetworkManager::Device::Ptr m_activeSystrayInterface;
    NetworkManager::AccessPoint* m_accessPoint;

    // Timeline controlling a connection progress overlay on the main icon
    QTimeLine m_overlayTimeline;
    QPixmap m_previousStatusOverlay;
    QPixmap m_statusOverlay;

    NetworkManager::Device::State m_activeInterfaceState;
    NetworkManager::Device::State m_activeSystrayInterfaceState;

    QString m_lastActiveInterfaceUni;
    QString m_lastActiveSystrayInterfaceUni;

    ///embedded KCM modules in the configuration dialog
    KCModuleProxy* m_kcmNM;
    KCModuleProxy* m_kcmNMTray;

    Plasma::Svg* m_svg;
    Plasma::Svg* m_svgMobile;
    Plasma::FrameSvg* m_meterBgSvg;
    Plasma::FrameSvg* m_meterFgSvg;

    class Private;
    Private * const d;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(NetworkManagerApplet::SystrayOverlayOptions)

#endif
