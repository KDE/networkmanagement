/*
Copyright 2008, 2009 Will Stephenson <wstephenson@kde.org>
Copyright 2008, 2009 Sebastian K?gler <sebas@kde.org>

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
#include <QGraphicsLinearLayout>
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
#include <Plasma/Extender>
#include <Plasma/ExtenderItem>
#include <Plasma/Theme>

#include "../libs/types.h"
#include "knmserviceprefs.h"
#include "remoteactivatablelist.h"

#include "nmextenderitem.h"
#include "uiutils.h"



K_EXPORT_PLASMA_APPLET(networkmanagement, NetworkManagerApplet)

/* for qSort()ing */
bool networkInterfaceLessThan(Solid::Control::NetworkInterface * if1, Solid::Control::NetworkInterface * if2);
bool networkInterfaceSameConnectionStateLessThan(Solid::Control::NetworkInterface * if1, Solid::Control::NetworkInterface * if2);

NetworkManagerApplet::NetworkManagerApplet(QObject * parent, const QVariantList & args)
    : Plasma::PopupApplet(parent, args),
        m_iconPerDevice(false),
        m_extenderItem(0),
        m_activeInterface(0)
{
    setHasConfigurationInterface(true);
    setPopupIcon(QIcon());
    //setPassivePopup(true); // FIXME: disable, only true for testing ...
    m_overlayTimeline.setEasingCurve(QEasingCurve::OutExpo);
    m_currentState = 0;
    connect(&m_overlayTimeline, SIGNAL(valueChanged(qreal)), this, SLOT(repaint()));

    Plasma::ToolTipManager::self()->registerWidget(this);
    setAspectRatioMode(Plasma::ConstrainedSquare);

    m_svg = new Plasma::Svg(this);
    m_svg->setImagePath("widgets/wireless");
    m_svg->setContainsMultipleImages(true);
    setStatus(Plasma::ActiveStatus);
    m_interfaces = Solid::Control::NetworkManager::networkInterfaces();
    if (activeInterface()) {
        m_currentState = activeInterface()->connectionState();
    }
    interfaceConnectionStateChanged();
    m_activatableList = new RemoteActivatableList(this);
    setMinimumSize(16, 16);
    resize(64, 64);
    updatePixmap();
    // TODO: read config into m_extenderItem ...
    // Now it is safe to create ExtenderItems and therefore InterfaceGroups

}

NetworkManagerApplet::~NetworkManagerApplet()
{
}

QString NetworkManagerApplet::svgElement(Solid::Control::NetworkInterface *iface)
{
    if (iface->type() != Solid::Control::NetworkInterface::Ieee80211) {
        return QString();
    }

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
    if (_s >= 19 && _s <= 76) {
        m_contentSquare = QRect(contentsRect().x() + (contentsRect().width() - s) / 2,
                                contentsRect().y() + (contentsRect().height() - s) / 2,
                                s, s);
    } else {
        m_contentSquare = QRect(contentsRect().x() + (contentsRect().width() - _s) / 2,
                                contentsRect().y() + (contentsRect().height() - _s) / 2,
                                _s, _s);

    }

    // Now figure out which exact element we'll use
    QString strength = "00";
    Solid::Control::WirelessNetworkInterface *wiface = qobject_cast<Solid::Control::WirelessNetworkInterface*>(iface);

    if (wiface) {
        QString uni = wiface->activeAccessPoint();
        //QString uni = wiface->activeAccessPoint()->signalStrength();
        //int s =
        Solid::Control::AccessPoint *ap = wiface->findAccessPoint(uni);
        if (ap) {
            int str = ap->signalStrength();
            if (s == 19) {
                // The size 19 in the svg only has 4 states
                if (str < 13) {
                    strength = "00";
                } else if (str < 38) {
                    strength = "25";
                } else if (str < 63) {
                    strength = "50";
                } else if (str < 88) {
                    strength = "75";
                } else if (str >= 88) {
                    strength = "100";
                }
            } else {
                if (str < 13) {
                    strength = "00";
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
            }
        } else {
                strength = "00";
        }
    } else {
        return QString("dialog-error");
    }
    QString w = QString::number(s);

    // <width>-<height>-wireless-signal-<strenght>
    QString icon;
    if (_s < 19 || _s > 76) {
        icon = QString("wireless-signal-%1").arg(strength);
        //m_contentSquare = QRect(contentsRect().x() + (contentsRect().width() - _s) / 2,
        //                contentsRect().y() + (contentsRect().height() - _s) / 2,
        //                _s, _s);

    } else {
        icon = QString("%1-%2-wireless-signal-%3").arg(w, w, strength);
    }
    //kDebug() << "============================ icon:" << icon;
    //m_contentSquare = QRect(contentsRect().x() + (contentsRect().width() - s) / 2,
    //                        contentsRect().y() + (contentsRect().height() - s) / 2,
    //                        s, s);

    return icon;
}

void NetworkManagerApplet::setupInterfaceSignals()
{
    foreach (Solid::Control::NetworkInterface* interface, m_interfaces) {
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
        if (interface->type() == Solid::Control::NetworkInterface::Ieee8023) {
            Solid::Control::WiredNetworkInterface* wirediface =
                            static_cast<Solid::Control::WiredNetworkInterface*>(interface);
            connect(wirediface, SIGNAL(carrierChanged(bool)), this, SLOT(interfaceConnectionStateChanged()));
        } else if (interface->type() == Solid::Control::NetworkInterface::Ieee80211) {
            Solid::Control::WirelessNetworkInterface* wirelessiface =
                            static_cast<Solid::Control::WirelessNetworkInterface*>(interface);
            connect(wirelessiface, SIGNAL(activeAccessPointChanged(const QString&)), SLOT(interfaceConnectionStateChanged()));
            QString uni = wirelessiface->activeAccessPoint();
            Solid::Control::AccessPoint *ap = wirelessiface->findAccessPoint(uni);
            if (ap) {
                connect(ap, SIGNAL(signalStrengthChanged(int)), SLOT(interfaceConnectionStateChanged()));
                connect(ap, SIGNAL(destroyed(QObject*)), SLOT(interfaceConnectionStateChanged()));
            }
        }
    }
}

void NetworkManagerApplet::init()
{
    // bogus, just to make sure we have some remotely sensible value
    m_contentSquare = contentsRect().toRect();
    kDebug();
    KConfigGroup cg = config();
    m_iconPerDevice = cg.readEntry("IconPerDevice", false);
    QObject::connect(Solid::Control::NetworkManager::notifier(), SIGNAL(networkInterfaceAdded(const QString&)),
            this, SLOT(networkInterfaceAdded(const QString&)));
    QObject::connect(Solid::Control::NetworkManager::notifier(), SIGNAL(networkInterfaceRemoved(const QString&)),
            this, SLOT(networkInterfaceRemoved(const QString&)));

    QObject::connect(Solid::Control::NetworkManager::notifier(), SIGNAL(statusChanged(Solid::Networking::Status)),
                     this, SLOT(managerStatusChanged(Solid::Networking::Status)));

    m_activatableList->init();
    setupInterfaceSignals();
    //kDebug() << "Plop.";
    //(void)graphicsWidget();
    //kDebug() << "Plop 2.";
}

QGraphicsWidget* NetworkManagerApplet::graphicsWidget()
{
    if (!m_extenderItem) {
        m_extenderItem = new NMExtenderItem(m_activatableList);
        connect(m_extenderItem, SIGNAL(configNeedsSaving()), this, SIGNAL(configNeedsSaving()));
    }
    return m_extenderItem;

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
}


/*
void NetworkManagerApplet::initExtenderItem(Plasma::ExtenderItem * eItem)
{
    // Let's just load a new one, hackish but works for now
    if (eItem->name() == "nmextenderitem") {
        eItem->destroy();
    }
    return;
}
*/
void NetworkManagerApplet::constraintsEvent(Plasma::Constraints constraints)
{
    // update the pixmap when a new size from kiconloader fits in, this makes sure the
    // icon is only displayed in sizes provides by KIconLoader, so we don't get blurry
    // icons
    if (constraints & (Plasma::SizeConstraint | Plasma::FormFactorConstraint)) {
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
    Q_UNUSED( option );

    if (m_useSvg) {
        QString el = svgElement(activeInterface());
        //<width>-<height>-wireless-signal-<strenght>
        m_svg->paint(p, m_contentSquare, el);
        //kDebug() << "------- using svg" << el << m_contentSquare;
    } else {
        //kDebug() << "------- not using svg";
        paintPixmap(p, m_pixmap, contentsRect);
    }
    paintProgress(p);
    paintOverlay(p);
}

void NetworkManagerApplet::paintProgress(QPainter *p)
{
    bool bar = false;
    qreal state = UiUtils::interfaceState(activeInterface());

    int i_s = (int)contentsRect().width()/4;
    int iconsize = qMax(UiUtils::iconSize(QSizeF(i_s, i_s)), 8);

    //QRectF r = QRectF(contentsRect().width()*.25, contentsRect().height()*.75, contentsRect().width()*.5, contentsRect().height()*.5);
    QRectF r = QRectF(iconsize, iconsize*2, iconsize, iconsize);

    qreal opacity = m_overlayTimeline.currentValue();
    if (opacity == 0) {
        return;
    } else if (state == 1) {
        //kDebug() << "painting OK overlay with opacity: " << opacity;
        //paintOkOverlay(p, contentsRect(), opacity);
        paintOkOverlay(p, r, opacity);
        return;
    }

    QColor fgColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    QColor bgColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor);

    bgColor.setAlphaF(.7 * opacity);
    fgColor.setAlphaF(.6 * opacity);

    //p->translate(0.5, 0.5);

    if (bar) {
        // paint a progress bar
        // height, space and width and position of the bar
        int fh = contentsRect().height();
        int fw = contentsRect().width();
        int h = qMax((qreal)(2.0), (qreal)(fh/20));
        int s = 1;
        int w = (contentsRect().width() - s*2) * state;
        QRectF background = QRectF(QPoint(0, fh - h - s - s ) + contentsRect().topLeft(), QSizeF(fw, h+2*s));
        QRectF progress = QRectF(QPoint(s, fh - h - s) + contentsRect().topLeft(), QSizeF(w, h));
        kDebug() << contentsRect() << background;


        QPen linePen(bgColor);

        p->setPen(linePen);
        p->drawRect(background);

        p->setPen(QPen(fgColor));
        p->setBrush(QBrush(fgColor));
        p->drawRect(progress);

    } else {
        // paint an arc completing a circle
        // 1 degree = 16 ticks, that's how drawArc() works
        // 0 is at 3 o'clock
        p->save();
        int top = 90 * 16;
        int progress = -360 * 16 * state;
        QPen pen(fgColor, 2); // color and line width

        //kDebug() << "progress circle" << top << progress;
        p->setPen(pen);
        p->setBrush(fgColor);
        //p->setBrush(QBrush(bgColor));

        //p->drawArc(contentsRect(), top, progress);
        p->drawPie(r, top, progress);
        p->restore();
    }
}

void NetworkManagerApplet::paintOverlay(QPainter *p)
{
    // Needs authentication, show this in the panel
    if (!activeInterface()) {
        return;
    }
    if (activeInterface()->connectionState() == Solid::Control::NetworkInterface::NeedAuth) {
        int i_s = (int)contentsRect().width()/4;
        int iconsize = qMax(UiUtils::iconSize(QSizeF(i_s, i_s)), 8);
        kDebug() << "iconsize" << iconsize;
        QPixmap icon = KIcon("dialog-password").pixmap(QSize(iconsize, iconsize));
        QPointF pos = QPointF(contentsRect().bottomRight().x() - iconsize,
                            contentsRect().bottomRight().y() - iconsize);
        p->drawPixmap(pos, icon);

    };
}

void NetworkManagerApplet::paintOkOverlay(QPainter *p, const QRectF &rect, qreal opacity)
{
    QColor color = QColor("#37B237"); // GNA! hardcoded colors == teh suck
    if (UiUtils::interfaceState(activeInterface()) == 0) {
        color = QColor("#B23636"); // green; GNA! hardcoded colors == teh suck
    }

    color.setAlphaF(opacity * 0.6);
    QPen pen(color, 2); // green, px width
    p->setPen(pen);
    p->setBrush(color);
    p->drawEllipse(rect);
}

void NetworkManagerApplet::paintPixmap(QPainter *painter, QPixmap pixmap, const QRectF &rect, qreal opacity)
{
    int size = pixmap.size().width();
    QPointF iconOrigin = QPointF(rect.left() + (rect.width() - size) / 2,
                                 rect.top() + (rect.height() - size) / 2);

    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    painter->setRenderHint(QPainter::Antialiasing);

    if (painter->paintEngine()->hasFeature(QPaintEngine::ConstantOpacity)) {
        // NOTE: Works, but makes hw acceleration impossible, use above code path
        //kWarning() << "You don't really want to hit this path, it means slow painting. Your paintengine is not good enough.";
        qreal old = painter->opacity();
        painter->setOpacity(opacity);
        painter->drawPixmap(iconOrigin, pixmap);
        painter->setOpacity(old);
    } else {
        QPixmap temp(QSize(size, size));
        temp.fill(Qt::transparent);

        QPainter p;
        p.begin(&temp);

        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.drawPixmap(QPoint(0,0), pixmap);

        p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        p.fillRect(pixmap.rect(), QColor(0, 0, 0, opacity * 254));
        p.end();

        // draw the pixmap
        painter->drawPixmap(iconOrigin, temp);
    }
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
    m_interfaces = Solid::Control::NetworkManager::networkInterfaces();

    setupInterfaceSignals();
    interfaceConnectionStateChanged();
    updatePixmap();
    update();
}

void NetworkManagerApplet::networkInterfaceRemoved(const QString & uni)
{
    Q_UNUSED(uni);
    // update the tray icon
    m_interfaces = Solid::Control::NetworkManager::networkInterfaces();
    // update extender visibility
    KConfigGroup cg = config();

    setupInterfaceSignals();
    interfaceConnectionStateChanged();
    updatePixmap();
    // kill any animations involving this interface
}

Solid::Control::NetworkInterface* NetworkManagerApplet::activeInterface()
{
    if (!m_interfaces.isEmpty()) {
        qSort(m_interfaces.begin(), m_interfaces.end(), networkInterfaceLessThan);
        if (m_interfaces.first()->type() == Solid::Control::NetworkInterface::Ieee80211) {
            m_useSvg = true;
        } else {
            m_useSvg = false;
        }
        return m_interfaces.first();
    } else {
        m_useSvg = false;
        return 0;
    }

}

void NetworkManagerApplet::interfaceConnectionStateChanged()
{
    //kDebug() << " +++ +++ +++ Connection State Changed +++ +++ +++";
    if (activeInterface()) {
        //kDebug() << "busy ... ?";
        int state = activeInterface()->connectionState();
        switch (state) {
            case Solid::Control::NetworkInterface::Preparing:
            case Solid::Control::NetworkInterface::Configuring:
            case Solid::Control::NetworkInterface::IPConfig:
                if (m_currentState != state) {
                    m_overlayTimeline.stop();
                    m_overlayTimeline.setDuration(2000);
                    m_overlayTimeline.setDirection(QTimeLine::Forward);
                    m_overlayTimeline.start();
                }
                //setBusy(true);
                break;
            case Solid::Control::NetworkInterface::NeedAuth:
                //setBusy(false);
                break;
            default:
                //setBusy(false);
                if (m_currentState != state) {
                    m_overlayTimeline.stop();
                    m_overlayTimeline.setDirection(QTimeLine::Backward);
                    m_overlayTimeline.setDuration(10000);
                    m_overlayTimeline.start();
                }
                break;
        }
        m_currentState = state;
    }
    updatePixmap();
}

void NetworkManagerApplet::toolTipAboutToShow()
{
    Solid::Control::NetworkInterfaceList interfaces
        = Solid::Control::NetworkManager::networkInterfaces();
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
        foreach (Solid::Control::NetworkInterface *iface, interfaces) {
            if (iface->connectionState() != Solid::Control::NetworkInterface::Unavailable) {
                if (!lines.isEmpty()) {
                    lines << QString();
                }
                hasActive = true;

                QString deviceText = UiUtils::interfaceNameLabel(iface->uni());

                QString ifaceName = iface->interfaceName();
                lines << QString::fromLatin1("<b>%1</b>").arg(deviceText);
                lines << UiUtils::connectionStateToString(iface->connectionState());
                Solid::Control::IPv4Config ip4Config = iface->ipV4Config();
                QList<Solid::Control::IPv4Address> addresses = ip4Config.addresses();
                if (!addresses.isEmpty()) {
                    QHostAddress addr(addresses.first().address());
                    QString currentIp = addr.toString();
                    lines << i18nc("Display of the IP (network) address in the tooltip", "<font size=\"-1\">Address: %1</font>", currentIp);
                }
                // Show the first active connection's icon, otherwise the networkmanager icon
                if (!iconChanged && iface->connectionState() == Solid::Control::NetworkInterface::Activated) {
                    icon = UiUtils::iconName(iface);
                    iconChanged = true; // we only want the first one
                }
            }
        }
        QString subText;
        if (hasActive) {
            subText = lines.join(QLatin1String("<br>"));
        } else {
            subText = i18nc("tooltip, all interfaces are down", "Disconnected");
        }
        m_toolTip = Plasma::ToolTipContent(QString(),
                                           subText,
                                           KIcon(icon).pixmap(IconSize(KIconLoader::Desktop))
                                           );
    }
    Plasma::ToolTipManager::self()->setContent(this, m_toolTip);
}


bool networkInterfaceLessThan(Solid::Control::NetworkInterface *if1, Solid::Control::NetworkInterface * if2)
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
        case Solid::Control::NetworkInterface::Preparing:
        case Solid::Control::NetworkInterface::Configuring:
        case Solid::Control::NetworkInterface::NeedAuth:
        case Solid::Control::NetworkInterface::IPConfig:
            if1status = Connecting;
            break;
        case Solid::Control::NetworkInterface::Activated:
            if1status = Connected;
            break;
        case Solid::Control::NetworkInterface::Disconnected:
            if1status = Disconnected;
            break;
        default: // all kind of unavailable
            break;
    }
    switch (if2->connectionState()) {
        case Solid::Control::NetworkInterface::Preparing:
        case Solid::Control::NetworkInterface::Configuring:
        case Solid::Control::NetworkInterface::NeedAuth:
        case Solid::Control::NetworkInterface::IPConfig:
            if2status = Connecting;
            break;
        case Solid::Control::NetworkInterface::Activated:
            if2status = Connected;
            break;
        case Solid::Control::NetworkInterface::Disconnected:
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

bool networkInterfaceSameConnectionStateLessThan(Solid::Control::NetworkInterface * if1, Solid::Control::NetworkInterface * if2)
{
    bool lessThan = false;
    switch (if1->type() ) {
        case Solid::Control::NetworkInterface::Ieee8023:
            switch (if2->type()) {
                case Solid::Control::NetworkInterface::Ieee8023:
                    lessThan = if1->uni() < if2->uni();
                    break;
                case Solid::Control::NetworkInterface::Ieee80211:
                    lessThan = true;
                    break;
                case Solid::Control::NetworkInterface::Serial:
                case Solid::Control::NetworkInterface::Gsm:
                case Solid::Control::NetworkInterface::Cdma:
                default:
                    lessThan = false;
                    break;
            }
            break;
        case Solid::Control::NetworkInterface::Ieee80211:
            switch (if2->type()) {
                case Solid::Control::NetworkInterface::Ieee8023:
                    lessThan = false;
                    break;
                case Solid::Control::NetworkInterface::Ieee80211:
                    lessThan = if1->uni() < if2->uni();
                    break;
                case Solid::Control::NetworkInterface::Serial:
                case Solid::Control::NetworkInterface::Gsm:
                case Solid::Control::NetworkInterface::Cdma:
                    lessThan = false;
                    break;
                default:
                    lessThan = true;
                    break;
            }
            break;
        case Solid::Control::NetworkInterface::Serial:
            switch (if2->type()) {
                case Solid::Control::NetworkInterface::Ieee8023:
                case Solid::Control::NetworkInterface::Ieee80211:
                    lessThan = true;
                    break;
                case Solid::Control::NetworkInterface::Serial:
                    lessThan = if1->uni() < if2->uni();
                    break;
                case Solid::Control::NetworkInterface::Gsm:
                case Solid::Control::NetworkInterface::Cdma:
                    lessThan = false;
                    break;
                default:
                    lessThan = true;
                    break;
            }
            break;
        case Solid::Control::NetworkInterface::Gsm:
            switch (if2->type()) {
                case Solid::Control::NetworkInterface::Ieee8023:
                case Solid::Control::NetworkInterface::Ieee80211:
                case Solid::Control::NetworkInterface::Serial:
                    lessThan = true;
                    break;
                case Solid::Control::NetworkInterface::Gsm:
                    lessThan = if1->uni() < if2->uni();
                    break;
                case Solid::Control::NetworkInterface::Cdma:
                    lessThan = false;
                    break;
                default:
                    lessThan = true;
                    break;
            }
            break;
        case Solid::Control::NetworkInterface::Cdma:
            switch (if2->type()) {
                case Solid::Control::NetworkInterface::Ieee8023:
                case Solid::Control::NetworkInterface::Ieee80211:
                case Solid::Control::NetworkInterface::Serial:
                case Solid::Control::NetworkInterface::Gsm:
                    lessThan = true;
                    break;
                case Solid::Control::NetworkInterface::Cdma:
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

/*
void NetworkManagerApplet::manageConnections()
{
    //kDebug() << "opening connection management dialog";
    QStringList args;
    args << "--icon" << "networkmanager" << "kcm_networkmanagement" << "kcm_networkmanagement_tray";
    KToolInvocation::kdeinitExec("kcmshell4", args);
    hidePopup();
}
void NetworkManagerApplet::loadExtender()
{
    //Plasma::ExtenderItem *eItem = extender()->item("networkmanagement");
    //if (eItem) {
    //    eItem->destroy(); // Apparently, we need to "refresh the extenderitem
    //}
    QGraphicsWidget* eItem = new NMExtenderItem(m_activatableList, extender());
    //eItem->setName("networkmanagement");
    //eItem->setTitle(i18nc("Label for extender","Network Management"));
    //eItem->widget();
}
*/

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
    Solid::Control::NetworkManager::setNetworkingEnabled(enabled);
    setupInterfaceSignals();
}

void NetworkManagerApplet::userWirelessEnabledChanged(bool enabled)
{
    kDebug() << enabled;
    Solid::Control::NetworkManager::setWirelessEnabled(enabled);
    setupInterfaceSignals();
}

void NetworkManagerApplet::managerStatusChanged(Solid::Networking::Status status)
{
    kDebug() << "managerstatuschanged";
    if (Solid::Networking::Unknown == status ) {
        // FIXME: Do something smart
    } else {
        // ...
    }
    setupInterfaceSignals();
    updatePixmap();
}

bool NetworkManagerApplet::hasInterfaceOfType(Solid::Control::NetworkInterface::Type type)
{
    foreach (Solid::Control::NetworkInterface * interface, m_interfaces) {
        if (interface->type() == type) {
            return true;
        }
    }
    return false;
}

#include "networkmanager.moc"
