/*
Copyright 2008, 2009 Will Stephenson <wstephenson@kde.org>
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

#include "networkmanager.h"

#include <QAction>
#include <QIcon>
#include <QPaintEngine>
#include <QPainter>
#include <QDesktopWidget>
#include <QGraphicsPixmapItem>
#include <QTimeLine>

#include <QGraphicsBlurEffect>

#include <KCModuleProxy>
#include <KCModuleInfo>
#include <KIcon>
#include <KIconLoader>
#include <KToolInvocation>
#include <KConfigDialog>
#include <KDebug>
#include <KLocale>
#include <KNotification>
#include <KPushButton>
#include <kdeversion.h>

#include <solid/device.h>
#include <solid/networking.h>
#include <solid/control/networking.h>
#include <solid/control/networkinterface.h>
#include <solid/control/wirednetworkinterface.h>
#include <solid/control/wirelessnetworkinterface.h>
#include <solid/control/networkmanager.h>

#include <Plasma/Animator>
#include <Plasma/CheckBox>
#include <Plasma/Containment>

#include "../libs/types.h"
#include "knmserviceprefs.h"
#include "remoteactivatablelist.h"
#include "nmpopup.h"
#include "uiutils.h"



K_EXPORT_PLASMA_APPLET(networkmanagement, NetworkManagerApplet)

/* for qSort()ing */
bool networkInterfaceLessThan(Solid::Control::NetworkInterfaceNm09 * if1, Solid::Control::NetworkInterfaceNm09 * if2);
bool networkInterfaceSameConnectionStateLessThan(Solid::Control::NetworkInterfaceNm09 * if1, Solid::Control::NetworkInterfaceNm09 * if2);

NetworkManagerApplet::NetworkManagerApplet(QObject * parent, const QVariantList & args)
    : Plasma::PopupApplet(parent, args),
        m_activatables(0),
        m_popup(0),
        m_panelContainment(true),
        m_totalActiveVpnConnections(0),
        m_activeInterface(0)
{
    KGlobal::locale()->insertCatalog("libknetworkmanager");
    KGlobal::locale()->insertCatalog("solidcontrol");

    setHasConfigurationInterface(true);
    setPopupIcon("networkmanager");
    //setPassivePopup(true); // FIXME: disable, only true for testing ...
    m_overlayTimeline.setEasingCurve(QEasingCurve::OutExpo);
    m_currentState = Solid::Control::NetworkInterfaceNm09::UnknownState;
    connect(&m_overlayTimeline, SIGNAL(valueChanged(qreal)), this, SLOT(repaint()));

    m_svg = new Plasma::Svg(this);
    m_svg->setImagePath("icons/network");
    m_svg->setContainsMultipleImages(true);
    m_meterBgSvg = new Plasma::FrameSvg(this);
    m_meterBgSvg->setImagePath("widgets/bar_meter_horizontal");
    m_meterBgSvg->setElementPrefix("bar-inactive");
    m_meterFgSvg = new Plasma::FrameSvg(this);
    m_meterFgSvg->setImagePath("widgets/bar_meter_horizontal");
    m_meterFgSvg->setElementPrefix("bar-active");
    setStatus(Plasma::ActiveStatus);
    m_interfaces = Solid::Control::NetworkManagerNm09::networkInterfaces();
    if (activeInterface()) {
        m_currentState = static_cast<Solid::Control::NetworkInterfaceNm09::ConnectionState>(activeInterface()->connectionState());
    }
    interfaceConnectionStateChanged();
    m_activatables = new RemoteActivatableList(this);
    connect(m_activatables, SIGNAL(activatableAdded(RemoteActivatable*)), this, SLOT(activatableAdded(RemoteActivatable*)));
    connect(m_activatables, SIGNAL(activatableRemoved(RemoteActivatable*)), this, SLOT(activatableRemoved(RemoteActivatable*)));
    connect(m_activatables, SIGNAL(disappeared()), this, SLOT(activatablesDisappeared()));
    updatePixmap();
}

NetworkManagerApplet::~NetworkManagerApplet()
{
}

QString NetworkManagerApplet::svgElement(Solid::Control::NetworkInterfaceNm09 *iface)
{
    if (iface->type() != Solid::Control::NetworkInterfaceNm09::Wifi
        && iface->type() != Solid::Control::NetworkInterfaceNm09::Ethernet) {
        return QString();
    }
    QString icon;

    int _s = qMin(contentsRect().width(), contentsRect().height());
    int s;
    // Figure out the maximum size of the element
    // those pixelsizes are taken from the svg
    if (_s >= 19) {
        s = 19;
    }
    if (_s >= 24) {
        s = 24;
    }
    if (_s >= 38) {
        s = 38;
    }
    if (_s >= 50) {
        s = 50;
    }
    if (_s >= 76) {
        s = 76;
    }
    // For our fixed sizes, we want a fixed rect to render into
    if (_s >= 19 && _s <= 76) {
        m_contentSquare = QRect(contentsRect().x() + (contentsRect().width() - s) / 2,
                                contentsRect().y() + (contentsRect().height() - s) / 2,
                                s, s);
    } else { // .... otherwise, for free scaling, we just want a square that fits in
        m_contentSquare = QRect(contentsRect().x() + (contentsRect().width() - _s) / 2,
                                contentsRect().y() + (contentsRect().height() - _s) / 2,
                                _s, _s);
    }

    if (iface->type() == Solid::Control::NetworkInterfaceNm09::Ethernet) {
        if (iface->connectionState() == Solid::Control::NetworkInterfaceNm09::Activated) {
            icon = "network-wired-activated";
        } else {
            icon = "network-wired";
        }
        return icon;
    }

    // Now figure out which exact element we'll use
    QString strength = "00";
    Solid::Control::WirelessNetworkInterfaceNm09 *wiface = qobject_cast<Solid::Control::WirelessNetworkInterfaceNm09*>(iface);

    if (wiface) {
        QString uni = wiface->activeAccessPoint();
        Solid::Control::AccessPointNm09 *ap = wiface->findAccessPoint(uni);
        if (ap) {
            int str = ap->signalStrength();
            if (str < 13) {
                strength = '0';
            } else if (str < 30) {
                strength = "20";
            } else if (str < 50) {
                strength = "40";
            } else if (str < 70) {
                strength = "60";
            } else if (str < 90) {
                strength = "80";
            } else {
                strength = "100";
            }
        } else {
                strength = '0';
        }
    } else {
        return QString("dialog-error");
    }
    QString w = QString::number(s);

    // The format in the SVG looks like this: wireless-signal-<strenght>
    icon = QString("network-wireless-%1").arg(strength);
    //kDebug() << "Icon:" << icon;
    return icon;
}

void NetworkManagerApplet::setupInterfaceSignals()
{
    foreach (Solid::Control::NetworkInterfaceNm09* interface, m_interfaces) {
        // be aware of state changes
        QObject::disconnect(interface, SIGNAL(connectionStateChanged(int, int, int)), this, SLOT(interfaceConnectionStateChanged()));
        QObject::disconnect(interface, SIGNAL(connectionStateChanged(int)), this, SLOT(interfaceConnectionStateChanged()));
        QObject::disconnect(interface, SIGNAL(linkUpChanged(bool)));

        //connect(iface, SIGNAL(connectionStateChanged(int,int,int)), this, SLOT(handleConnectionStateChange(int,int,int)));
        connect(interface, SIGNAL(connectionStateChanged(int,int,int)), this, SLOT(interfaceConnectionStateChanged()));
        //connect(iface, SIGNAL(linkUpChanged(bool)), this, SLOT(switchToDefaultTab()));

        QObject::connect(interface, SIGNAL(connectionStateChanged(int)), this, SLOT(interfaceConnectionStateChanged()));
        QObject::connect(interface, SIGNAL(linkUpChanged(bool)), this, SLOT(interfaceConnectionStateChanged()));

        // Interface type-specific connections
        if (interface->type() == Solid::Control::NetworkInterfaceNm09::Ethernet) {
            Solid::Control::WiredNetworkInterfaceNm09* wirediface =
                            static_cast<Solid::Control::WiredNetworkInterfaceNm09*>(interface);
            connect(wirediface, SIGNAL(carrierChanged(bool)), this, SLOT(interfaceConnectionStateChanged()));
        } else if (interface->type() == Solid::Control::NetworkInterfaceNm09::Wifi) {
            Solid::Control::WirelessNetworkInterfaceNm09* wirelessiface =
                            static_cast<Solid::Control::WirelessNetworkInterfaceNm09*>(interface);
            connect(wirelessiface, SIGNAL(activeAccessPointChanged(const QString&)), SLOT(interfaceConnectionStateChanged()));
            QString uni = wirelessiface->activeAccessPoint();
            Solid::Control::AccessPointNm09 *ap = wirelessiface->findAccessPoint(uni);
            if (ap) {
                connect(ap, SIGNAL(signalStrengthChanged(int)), SLOT(interfaceConnectionStateChanged()));
                connect(ap, SIGNAL(destroyed(QObject*)), SLOT(interfaceConnectionStateChanged()));
            }
        }
    }
}

void NetworkManagerApplet::init()
{
    Plasma::Containment * c = containment();

    /* When applet is not in panel the tooltip always appears when hovering
       any point of the popup's area, which is annoying. */
    if (c && (c->containmentType() == Plasma::Containment::PanelContainment ||
              c->containmentType() == Plasma::Containment::CustomPanelContainment)) {
        Plasma::ToolTipManager::self()->registerWidget(this);
        m_panelContainment = true;
    } else {
        m_panelContainment = false;
    }

    // bogus, just to make sure we have some remotely sensible value
    m_contentSquare = contentsRect().toRect();
    //kDebug();
    QObject::connect(Solid::Control::NetworkManagerNm09::notifier(), SIGNAL(networkInterfaceAdded(const QString&)),
            this, SLOT(networkInterfaceAdded(const QString&)));
    QObject::connect(Solid::Control::NetworkManagerNm09::notifier(), SIGNAL(networkInterfaceRemoved(const QString&)),
            this, SLOT(networkInterfaceRemoved(const QString&)));

    QObject::connect(Solid::Control::NetworkManagerNm09::notifier(), SIGNAL(statusChanged(Solid::Networking::Status)),
                     this, SLOT(managerStatusChanged(Solid::Networking::Status)));

    m_activatables->init();
    setupInterfaceSignals();

    // Just to make sure the kded module is loaded.
    QDBusInterface kded(QLatin1String("org.kde.kded"), QLatin1String("/kded"),
                        QLatin1String("org.kde.kded"), QDBusConnection::sessionBus());

    kded.call(QLatin1String("loadModule"), QLatin1String("networkmanagement"));
    QObject::connect(m_activatables, SIGNAL(appeared()), this, SLOT(finishInitialization()));
    finishInitialization();
}

void NetworkManagerApplet::finishInitialization()
{
    // Finishes kded module initialization.
    QDBusInterface networkmanagement(QLatin1String("org.kde.networkmanagement"), QLatin1String("/org/kde/networkmanagement"),
                                     QLatin1String("org.kde.networkmanagement"), QDBusConnection::sessionBus());

    networkmanagement.call(QLatin1String("FinishInitialization"));
}

QGraphicsWidget* NetworkManagerApplet::graphicsWidget()
{
    if (!m_popup) {
        m_popup = new NMPopup(m_activatables, this);
        connect(m_popup, SIGNAL(configNeedsSaving()), this, SIGNAL(configNeedsSaving()));
    }
    return m_popup;
}

void NetworkManagerApplet::createConfigurationInterface(KConfigDialog *parent)
{
    // Add the networkmanager KCM pages to the applet's configdialog
    m_kcmNM = new KCModuleProxy("kcm_networkmanagement");
    m_kcmNMTray = new KCModuleProxy("kcm_networkmanagement_tray");
    parent->addPage(m_kcmNM, m_kcmNM->moduleInfo().moduleName(),
                    m_kcmNM->moduleInfo().icon());
    parent->addPage(m_kcmNMTray, m_kcmNMTray->moduleInfo().moduleName(),
                    m_kcmNMTray->moduleInfo().icon());

    connect(parent, SIGNAL(applyClicked()), this, SLOT(saveConfiguration()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(saveConfiguration()));
}

void NetworkManagerApplet::saveConfiguration()
{
    // kcm_networkmanagement implicitly saves connection definition after
    // editing is finished, so no need to call its save() method
    // FIXME This just writes out changed values to ini file. kded module
    // still continues to use old value
    m_kcmNMTray->save();
}

void NetworkManagerApplet::constraintsEvent(Plasma::Constraints constraints)
{
    // update the pixmap when a new size from kiconloader fits in, this makes sure the
    // icon is only displayed in sizes provides by KIconLoader, so we don't get blurry
    // icons
    if (constraints & (Plasma::SizeConstraint | Plasma::FormFactorConstraint)) {
        setAspectRatioMode(Plasma::Square);
        if (UiUtils::iconSize(contentsRect().size()) != UiUtils::iconSize(m_pixmap.size())) {
            updatePixmap();
        }
    }
}

void NetworkManagerApplet::updatePixmap()
{
    int s = UiUtils::iconSize(contentsRect().size());
    m_pixmap = KIcon(UiUtils::iconName(activeInterface())).pixmap(s, s);
    update();
}

void NetworkManagerApplet::paintInterface(QPainter * p, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    Q_UNUSED( p );
    Q_UNUSED( option );

    if (!m_panelContainment) {
        /* To make applet's size matches the popup's size. The applet is the tray icon, which is 16x16 pixels size by default.*/
        adjustSize();
        return;
    }

    Solid::Control::NetworkInterfaceNm09* interface = activeInterface();
    bool useSvg = false;
    if (interface) {
        useSvg = interface->type() == Solid::Control::NetworkInterfaceNm09::Wifi || interface->type() == Solid::Control::NetworkInterfaceNm09::Ethernet;
    }

    /* I am using setPopupIcon at the end of this method to make the usual system tray icon's hover and click
     * effects work. However, setPopupIcon creates a Plasma::IconWidget object that draws itself over
     * contentsRect and, consequentely, over the overlays created by paintStatusOverlay and paintNeedAuthOverlay.
     * I am creating a new icon with the overlays applied to overcome this problem.
     * TODO: I have not done any performance test, but it is pretty clear this needs improvements.
     *       At least valgrind does not indicate any memory leak in plasma-desktop because of this code.
     *       smem also does not indicate visible memory leak in Xorg process, which is where the pixmap cache is
     *       stored.
     **/
    int s = qMin(contentsRect.width(), contentsRect.height());
    QRect rect(0, 0, s, s);
    QPixmap newIcon(QSize(s, s));
    newIcon.fill(Qt::transparent);
    QPainter painter;
    painter.begin(&newIcon);

    if (useSvg) {
        QString el = svgElement(interface);
        m_svg->paint(&painter, rect, el);
    } else {
        painter.drawPixmap(QPoint(0,0), m_pixmap);
    }

    paintStatusOverlay(&painter, rect);
    paintNeedAuthOverlay(&painter, rect);
    painter.end();
    setPopupIcon(newIcon);
}

inline void NetworkManagerApplet::paintNeedAuthOverlay(QPainter *p, QRect &rect)
{
    // Needs authentication, show this in the panel
    if (!activeInterface()) {
        kDebug() << "No active interface";
        return;
    }
    /*
    kDebug() << "Painting overlay ...>" << activeInterface()->connectionState();
    */
    if (activeInterface() && activeInterface()->connectionState() == Solid::Control::NetworkInterfaceNm09::NeedAuth) {
        //kDebug() << "Needing auth ...>";
        int iconSize = (int)2*(rect.width()/3);

        //kDebug() << "Security:iconSize" << iconSize;
        QPixmap icon = KIcon("dialog-password").pixmap(iconSize);
        QPoint pos = QPoint(rect.right() - iconSize,
                            rect.bottom() - iconSize);

        p->drawPixmap(pos, icon);
    }
}

inline void NetworkManagerApplet::paintStatusOverlay(QPainter *p, QRect &rect)
{
    if (m_totalActiveVpnConnections > 0) {
        int iconSize = (int)2*(rect.width()/3);
        QPixmap pix = KIcon("object-locked").pixmap(iconSize);
        p->drawPixmap(rect.right() - pix.width(), rect.bottom() - pix.height(), pix);
    }

    int oldOpacity = p->opacity();
    qreal opacity = m_overlayTimeline.currentValue();
    if (!qFuzzyCompare(opacity, 1) && !m_previousStatusOverlay.isNull()) {
        p->setOpacity(1 - opacity);
        p->drawPixmap(rect.left(), rect.bottom() - m_previousStatusOverlay.height(), m_previousStatusOverlay);
    }
    if (!m_statusOverlay.isNull()) {
        p->setOpacity(opacity);
        p->drawPixmap(rect.left(), rect.bottom() - m_statusOverlay.height(), m_statusOverlay);
    }
    p->setOpacity(oldOpacity);
}

void NetworkManagerApplet::repaint()
{
    update();
}

/* Slots to react to changes from the daemon */
void NetworkManagerApplet::networkInterfaceAdded(const QString & uni)
{
    Q_UNUSED(uni);
    // update the tray icon
    m_interfaces = Solid::Control::NetworkManagerNm09::networkInterfaces();

    setupInterfaceSignals();
    interfaceConnectionStateChanged();
    updatePixmap();
}

void NetworkManagerApplet::networkInterfaceRemoved(const QString & uni)
{
    Q_UNUSED(uni);
    // update the tray icon
    m_interfaces = Solid::Control::NetworkManagerNm09::networkInterfaces();
    KConfigGroup cg = config();

    setupInterfaceSignals();
    interfaceConnectionStateChanged();
    updatePixmap();
    // kill any animations involving this interface
}

Solid::Control::NetworkInterfaceNm09* NetworkManagerApplet::activeInterface()
{
    if (!m_interfaces.isEmpty()) {
        qSort(m_interfaces.begin(), m_interfaces.end(), networkInterfaceLessThan);
        return m_interfaces.first();
    } else {
        return 0;
    }

}

void NetworkManagerApplet::interfaceConnectionStateChanged()
{
    //kDebug() << " +++ +++ +++ Connection State Changed +++ +++ +++";
    if (activeInterface()) {
        //kDebug() << "busy ... ?";
        Solid::Control::NetworkInterfaceNm09::ConnectionState state = static_cast<Solid::Control::NetworkInterfaceNm09::ConnectionState>(activeInterface()->connectionState());
        switch (state) {
            case Solid::Control::NetworkInterfaceNm09::Preparing:
            case Solid::Control::NetworkInterfaceNm09::Configuring:
            case Solid::Control::NetworkInterfaceNm09::IPConfig:
            case Solid::Control::NetworkInterfaceNm09::IPCheck:
            case Solid::Control::NetworkInterfaceNm09::Secondaries:
            case Solid::Control::NetworkInterfaceNm09::Deactivating:
                if (m_currentState != state) {
                    setStatusOverlay(generateProgressStatusOverlay());
                }
                //setBusy(true);
                break;
            case Solid::Control::NetworkInterfaceNm09::NeedAuth:
                //setBusy(false);
                break;
            case Solid::Control::NetworkInterfaceNm09::Activated:
                //setBusy(false);
                if (m_currentState != state) {
                    // We want to show the full circle a bit
                    setStatusOverlay(generateProgressStatusOverlay());
                    setStatusOverlay("task-complete");
                    QTimer::singleShot(4000, this, SLOT(clearActivatedOverlay()));
                }
                break;
            case Solid::Control::NetworkInterfaceNm09::UnknownState:
                kDebug() << "UnknownState! should this happen?";
            case Solid::Control::NetworkInterfaceNm09::Unmanaged:
            case Solid::Control::NetworkInterfaceNm09::Unavailable:
            case Solid::Control::NetworkInterfaceNm09::Failed:
                if (m_currentState != state) {
                    setStatusOverlay("dialog-error");
                }
                break;
            case Solid::Control::NetworkInterfaceNm09::Disconnected:
                if (m_currentState != state) {
                    setStatusOverlay("dialog-cancel");
                }
                break;
        }
        m_currentState = state;
    }
    updatePixmap();
}

void NetworkManagerApplet::toolTipAboutToShow()
{
    Solid::Control::NetworkInterfaceNm09List interfaces
        = Solid::Control::NetworkManagerNm09::networkInterfaces();
    if (interfaces.isEmpty()) {
        m_toolTip = Plasma::ToolTipContent(QString(),
                                        i18nc("Tooltip sub text", "No network interfaces"),
                                        KIcon("networkmanager").pixmap(IconSize(KIconLoader::Desktop))
                                        );
    } else {
        qSort(interfaces.begin(), interfaces.end(), networkInterfaceLessThan);
        bool hasActive = false;
        bool iconChanged = false;
        QString icon = "networkmanager";
        QStringList lines;
        foreach (Solid::Control::NetworkInterfaceNm09 *iface, interfaces) {
            if (iface->connectionState() != Solid::Control::NetworkInterfaceNm09::Unavailable) {
                if (!lines.isEmpty()) {
                    lines << QString();
                }
                hasActive = true;

                QString deviceText = UiUtils::interfaceNameLabel(iface->uni());
                lines << QString::fromLatin1("<b>%1</b>").arg(deviceText);

                QString connectionName;
                RemoteInterfaceConnection *conn = m_activatables->connectionForInterface(iface);
                if (conn) {
                    connectionName = conn->connectionName();
                }

                lines << QString("%1").arg(UiUtils::connectionStateToString(static_cast<Solid::Control::NetworkInterfaceNm09::ConnectionState>(iface->connectionState()), connectionName));
                /*
                Solid::Control::IPv4Config ip4Config = iface->ipV4Config();
                QList<Solid::Control::IPv4Address> addresses = ip4Config.addresses();
                if (!addresses.isEmpty()) {
                    QHostAddress addr(addresses.first().address());
                    QString currentIp = addr.toString();
                    lines << i18nc("Display of the IP (network) address in the tooltip", "<font size=\"-1\">Address: %1</font>", currentIp);
                }
                */
                // Show the first active connection's icon, otherwise the networkmanager icon
                if (!iconChanged && iface->connectionState() == Solid::Control::NetworkInterfaceNm09::Activated) {
                    icon = UiUtils::iconName(iface);
                    iconChanged = true; // we only want the first one
                }
            }
        }

        QString subText;
        QString text;

        /* VPN connections require at least one non-vpn active connection.
           If there is no non-vpn active connection then there is no active VPN connection too.*/
        if (hasActive) {
            if (!m_activeVpnConnections.isEmpty()) {
                lines << QString();
                lines << QString::fromLatin1("<b>%1</b>").arg(i18n("VPN Connections"));
                QMap<QUuid, QWeakPointer<RemoteInterfaceConnection> >::iterator i = m_activeVpnConnections.begin();
                while (i != m_activeVpnConnections.end()) {
                    RemoteInterfaceConnection *ic = i.value().data();
                    if (!ic) {
                        i = m_activeVpnConnections.erase(i);
                    } else {
                        lines << QString("%1").arg(UiUtils::connectionStateToString(ic->activationState(), ic->connectionName()));
                        i++;
                    }
                }
            }
            subText = lines.join(QLatin1String("<br>"));
        } else {
            text = i18nc("tooltip, all interfaces are down", "Disconnected");

            if (m_popup->hasWireless() && !Solid::Control::NetworkManagerNm09::isWirelessEnabled()) {
                subText = i18nc("tooltip, wireless is disabled in software", "Wireless disabled in software");
            }
            if (!Solid::Control::NetworkManagerNm09::isNetworkingEnabled()) {
                subText = i18nc("tooltip, all interfaces are down", "Networking disabled");
            }
            if (m_popup->hasWireless() && !Solid::Control::NetworkManagerNm09::isWirelessHardwareEnabled()) {
                subText = i18nc("tooltip, wireless is disabled by hardware", "Wireless disabled by hardware");
            }

        }
        m_toolTip = Plasma::ToolTipContent(text,
                                           subText,
                                           KIcon(icon).pixmap(IconSize(KIconLoader::Desktop))
                                           );
    }
    Plasma::ToolTipManager::self()->setContent(this, m_toolTip);
}


bool networkInterfaceLessThan(Solid::Control::NetworkInterfaceNm09 *if1, Solid::Control::NetworkInterfaceNm09 * if2)
{
    /*
     * status merging algorithm
     * In descending order of importance:
     * - Connecting devices
     *   - Cellular devices (because of cost)
     *   - = PPP devices
     *   - Ethernet devices
     *   - Wireless devices
     * - Connected devices
     *   - order as above
     * - Disconnected devices
     *   - order as above
     */
    enum {  Connecting,
            Connected,
            Disconnected,
            Unavailable }
        if2status = Unavailable,
        if1status = Unavailable;

    switch (if1->connectionState()) {
        case Solid::Control::NetworkInterfaceNm09::Preparing:
        case Solid::Control::NetworkInterfaceNm09::Configuring:
        case Solid::Control::NetworkInterfaceNm09::NeedAuth:
        case Solid::Control::NetworkInterfaceNm09::IPConfig:
        case Solid::Control::NetworkInterfaceNm09::IPCheck:
        case Solid::Control::NetworkInterfaceNm09::Secondaries:
            if1status = Connecting;
            break;
        case Solid::Control::NetworkInterfaceNm09::Activated:
            if1status = Connected;
            break;
        case Solid::Control::NetworkInterfaceNm09::Disconnected:
            if1status = Disconnected;
            break;
        default: // all kind of unavailable
            break;
    }
    switch (if2->connectionState()) {
        case Solid::Control::NetworkInterfaceNm09::Preparing:
        case Solid::Control::NetworkInterfaceNm09::Configuring:
        case Solid::Control::NetworkInterfaceNm09::NeedAuth:
        case Solid::Control::NetworkInterfaceNm09::IPConfig:
        case Solid::Control::NetworkInterfaceNm09::IPCheck:
        case Solid::Control::NetworkInterfaceNm09::Secondaries:
            if2status = Connecting;
            break;
        case Solid::Control::NetworkInterfaceNm09::Activated:
            if2status = Connected;
            break;
        case Solid::Control::NetworkInterfaceNm09::Disconnected:
            if2status = Disconnected;
            break;
        default: // all kind of disconnected
            break;
    }

    bool lessThan = false;
    switch (if1status) {
        case Connecting:
            lessThan = (if2status != Connecting || networkInterfaceSameConnectionStateLessThan(if1, if2));
            //return true;//
            break;
        case Connected:
            if ( if2status == Connecting) {
               return false;
            }
            lessThan = ((if2status != Connected) || networkInterfaceSameConnectionStateLessThan(if1, if2));
            break;
        case Disconnected:
            lessThan = false;
            if ( if2status == Disconnected) {
                lessThan = networkInterfaceSameConnectionStateLessThan(if1, if2);
            }
            if (if2status == Unavailable) {
                lessThan = true;
            }
            break;
        case Unavailable:
            lessThan = false;
            if ( if2status == Unavailable) {
                lessThan = networkInterfaceSameConnectionStateLessThan(if1, if2);
            }
            break;
    }

    return lessThan;
}

bool networkInterfaceSameConnectionStateLessThan(Solid::Control::NetworkInterfaceNm09 * if1, Solid::Control::NetworkInterfaceNm09 * if2)
{
    bool lessThan = false;
    switch (if1->type() ) {
        case Solid::Control::NetworkInterfaceNm09::Ethernet:
            switch (if2->type()) {
                case Solid::Control::NetworkInterfaceNm09::Ethernet:
                    lessThan = if1->uni() < if2->uni();
                    break;
                case Solid::Control::NetworkInterfaceNm09::Wifi:
                    lessThan = true;
                    break;
                case Solid::Control::NetworkInterfaceNm09::Modem:
                default:
                    lessThan = false;
                    break;
            }
            break;
        case Solid::Control::NetworkInterfaceNm09::Wifi:
            switch (if2->type()) {
                case Solid::Control::NetworkInterfaceNm09::Ethernet:
                    lessThan = false;
                    break;
                case Solid::Control::NetworkInterfaceNm09::Wifi:
                    lessThan = if1->uni() < if2->uni();
                    break;
                case Solid::Control::NetworkInterfaceNm09::Modem:
                    lessThan = false;
                    break;
                default:
                    lessThan = true;
                    break;
            }
            break;
        case Solid::Control::NetworkInterfaceNm09::Modem:
            switch (if2->type()) {
                case Solid::Control::NetworkInterfaceNm09::Ethernet:
                case Solid::Control::NetworkInterfaceNm09::Wifi:
                    lessThan = true;
                    break;
                case Solid::Control::NetworkInterfaceNm09::Modem:
                    lessThan = if1->uni() < if2->uni();
                    break;
                default:
                    lessThan = true;
                    break;
            }
            break;
        default:
            lessThan = false;
        }
    return lessThan;
}

void NetworkManagerApplet::managerWirelessEnabledChanged(bool)
{
    setupInterfaceSignals();
}

void NetworkManagerApplet::managerWirelessHardwareEnabledChanged(bool enabled)
{
    Q_UNUSED( enabled );
    setupInterfaceSignals();
    updatePixmap();
}

void NetworkManagerApplet::userNetworkingEnabledChanged(bool enabled)
{
    kDebug() << enabled;
    Solid::Control::NetworkManagerNm09::setNetworkingEnabled(enabled);
    setupInterfaceSignals();
}

void NetworkManagerApplet::userWirelessEnabledChanged(bool enabled)
{
    kDebug() << enabled;
    Solid::Control::NetworkManagerNm09::setWirelessEnabled(enabled);
    setupInterfaceSignals();
}

void NetworkManagerApplet::managerStatusChanged(Solid::Networking::Status status)
{
    //kDebug() << "managerstatuschanged";
    if (Solid::Networking::Unknown == status ) {
        // FIXME: Do something smart
    } else {
        // ...
    }
    setupInterfaceSignals();
    updatePixmap();
}

bool NetworkManagerApplet::hasInterfaceOfType(Solid::Control::NetworkInterfaceNm09::Type type)
{
    foreach (Solid::Control::NetworkInterfaceNm09 * interface, m_interfaces) {
        if (interface->type() == type) {
            return true;
        }
    }
    return false;
}

void NetworkManagerApplet::setStatusOverlay(const QPixmap& pix)
{
    m_previousStatusOverlay = m_statusOverlay;
    m_statusOverlay = pix;
    if (m_overlayTimeline.state() == QTimeLine::Running) {
        m_overlayTimeline.stop();
    }
    m_overlayTimeline.start();
}

void NetworkManagerApplet::setStatusOverlay(const QString& name)
{
    int i_s = (int)contentsRect().width()/4;
    int size = UiUtils::iconSize(QSizeF(i_s, i_s));
    QPixmap pix = KIcon(name).pixmap(size);
    setStatusOverlay(pix);
}

QPixmap NetworkManagerApplet::generateProgressStatusOverlay()
{
    int width = contentsRect().width();
    int height = qMax(width / 4, 4);

    QPixmap pix(width, height);
    pix.fill(Qt::transparent);
    qreal state = UiUtils::interfaceState(activeInterface());

    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    m_meterBgSvg->resizeFrame(pix.size());
    m_meterBgSvg->paintFrame(&p, pix.rect());

    QRectF innerRect = pix.rect();
    innerRect.setWidth(innerRect.width() * state);
    m_meterFgSvg->resizeFrame(innerRect.size());
    m_meterFgSvg->paintFrame(&p, innerRect);

    return pix;
}

void NetworkManagerApplet::clearActivatedOverlay()
{
    if (activeInterface() && static_cast<Solid::Control::NetworkInterfaceNm09::ConnectionState>(activeInterface()->connectionState()) == Solid::Control::NetworkInterfaceNm09::Activated) {
        // Clear the overlay, but only if we are still activated
        setStatusOverlay(QPixmap());
    }
}

void NetworkManagerApplet::activatableAdded(RemoteActivatable *activatable)
{
    if (activatable->activatableType() == Knm::Activatable::VpnInterfaceConnection) {
        RemoteInterfaceConnection *ic = static_cast<RemoteInterfaceConnection*>(activatable);
        connect(ic, SIGNAL(activationStateChanged(Knm::InterfaceConnection::ActivationState, Knm::InterfaceConnection::ActivationState)),
                this, SLOT(vpnActivationStateChanged(Knm::InterfaceConnection::ActivationState, Knm::InterfaceConnection::ActivationState)));
        QMetaObject::invokeMethod(ic, "activationStateChanged", Q_ARG(Knm::InterfaceConnection::ActivationState, ic->oldActivationState()), Q_ARG(Knm::InterfaceConnection::ActivationState, ic->activationState()));
    }
}

void NetworkManagerApplet::vpnActivationStateChanged(Knm::InterfaceConnection::ActivationState oldState, Knm::InterfaceConnection::ActivationState newState)
{
    RemoteInterfaceConnection *ic = static_cast<RemoteInterfaceConnection*>(sender());
    QUuid id = ic->connectionUuid();
    switch (newState)
    {
        case Knm::InterfaceConnection::Activated:
            m_totalActiveVpnConnections++;
            if (!m_activeVpnConnections.contains(id))
                m_activeVpnConnections.insert(id, QWeakPointer<RemoteInterfaceConnection>(ic));
            break;
        case Knm::InterfaceConnection::Activating:
            m_activeVpnConnections.insert(id, QWeakPointer<RemoteInterfaceConnection>(ic));
            break;
        case Knm::InterfaceConnection::Unknown:
            m_activeVpnConnections.remove(id);
            if (oldState == Knm::InterfaceConnection::Activated)
                m_totalActiveVpnConnections--;
            break;
    }
    kDebug() << newState << m_totalActiveVpnConnections;
    update();
}

void NetworkManagerApplet::activatableRemoved(RemoteActivatable *activatable)
{
    if (activatable->activatableType() == Knm::Activatable::VpnInterfaceConnection) {
        RemoteInterfaceConnection *ic = static_cast<RemoteInterfaceConnection*>(activatable);
        m_activeVpnConnections.remove(ic->connectionUuid());
        kDebug() << "activatable removed" << m_activeVpnConnections.count();
    }
}

void NetworkManagerApplet::activatablesDisappeared()
{
    m_totalActiveVpnConnections = 0;
    update();
}

#include "networkmanager.moc"
