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
        m_activeInterface(0)
{
    setHasConfigurationInterface(false);
    setPopupIcon(QIcon());
    setPassivePopup(true); // FIXME: disable, only true for testing ...
    m_overlayTimeline.setEasingCurve(QEasingCurve::OutExpo);
    m_currentState = 0;
    connect(&m_overlayTimeline, SIGNAL(valueChanged(qreal)), this, SLOT(repaint()));

    Plasma::ToolTipManager::self()->registerWidget(this);
    setAspectRatioMode(Plasma::ConstrainedSquare);

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
            kDebug() << "connect carrierChanged";
            connect(wirediface, SIGNAL(carrierChanged(bool)), this, SLOT(interfaceConnectionStateChanged()));
        } else if (interface->type() == Solid::Control::NetworkInterface::Ieee80211) {
            Solid::Control::WirelessNetworkInterface* wirelessiface =
                            static_cast<Solid::Control::WirelessNetworkInterface*>(interface);
            connect(wirelessiface, SIGNAL(activeAccessPointChanged(const QString&)), SLOT(interfaceConnectionStateChanged()));
            QString uni = wirelessiface->activeAccessPoint();
            Solid::Control::AccessPoint *ap = wirelessiface->findAccessPoint(uni);
            if (ap) {
                kDebug() << "connect ap";
                connect(ap, SIGNAL(signalStrengthChanged(int)), SLOT(interfaceConnectionStateChanged()));
                connect(ap, SIGNAL(destroyed(QObject*)), SLOT(interfaceConnectionStateChanged()));
            }
        }
    }

    kDebug() << "...................................";
    //updatePixmap(); // do we really want to do this here?

}

void NetworkManagerApplet::init()
{
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


    m_extenderItem = new NMExtenderItem(m_activatableList, extender());
    connect(m_extenderItem, SIGNAL(configNeedsSaving()), this, SIGNAL(configNeedsSaving()));
}


void NetworkManagerApplet::initExtenderItem(Plasma::ExtenderItem * eItem)
{
    // Let's just load a new one, hackish but works for now
    if (eItem->name() == "nmextenderitem") {
        eItem->destroy();
    }
    return;
}

void NetworkManagerApplet::constraintsEvent(Plasma::Constraints constraints)
{
   if (constraints & (Plasma::SizeConstraint | Plasma::FormFactorConstraint)) {
        if (UiUtils::iconSize(contentsRect().size()) != UiUtils::iconSize(m_pixmap.size())) {
            int _i = UiUtils::iconSize(m_pixmap.size());
            kDebug() << "cevent" << UiUtils::iconSize(contentsRect().size()) << UiUtils::iconSize(QSize(_i, _i));
            updatePixmap();
        } else {
            //positionPixmap();
        }
    }
}

void NetworkManagerApplet::updatePixmap()
{
    m_pixmap = KIcon(UiUtils::iconName(activeInterface())).pixmap(contentsRect().size().toSize());
    update();
}

QList<QAction*> NetworkManagerApplet::contextualActions()
{
    QAction* configAction = new QAction(KIcon("networkmanager"), i18n("Manage Connections..."), this);
    connect(configAction, SIGNAL(triggered(bool)), this, SLOT(manageConnections()));
    QList<QAction*> tempActions;
    tempActions << configAction;
    return tempActions;
}

void NetworkManagerApplet::paintInterface(QPainter * p, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    Q_UNUSED( option );

    paintPixmap(p, m_pixmap, contentsRect);
    paintProgress(p);
}

void NetworkManagerApplet::paintProgress(QPainter *p)
{
    bool bar = false;
    qreal state = UiUtils::interfaceState(activeInterface());

    qreal opacity = m_overlayTimeline.currentValue();
    if (opacity == 0) {
        return;
    } else if (state == 1) {
        //kDebug() << "painting OK overlay with opacity: " << opacity;
        paintOkOverlay(p, contentsRect(), opacity);
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
        int top = 90 * 16;
        int progress = -360 * 16 * state;
        QPen pen(fgColor, 2); // color and line width

        //kDebug() << "progress circle" << top << progress;
        p->setPen(pen);
        p->setBrush(QBrush(bgColor));

        p->drawArc(contentsRect(), top, progress);
    }
}

void NetworkManagerApplet::paintOkOverlay(QPainter *p, const QRectF &rect, qreal opacity)
{
    QColor color = QColor("#37B237");
    if (UiUtils::interfaceState(activeInterface()) == 0) {
        color = QColor("#B23636");
    }

    color.setAlphaF(opacity * 0.6);
    QPen pen(color, 2); // green, px width
    p->setPen(pen);
    p->drawEllipse(rect);
}

void NetworkManagerApplet::paintPixmap(QPainter *painter, QPixmap pixmap, const QRectF &rect, qreal opacity)
{
    int size = pixmap.size().width();
    QPointF iconOrigin = QPointF(rect.left() + (rect.width() - size) / 2,
                                 rect.top() + (rect.height() - size) / 2);

    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    painter->setRenderHint(QPainter::Antialiasing);

    if (!painter->paintEngine()->hasFeature(QPaintEngine::ConstantOpacity)) {
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
    } else {
        // FIXME: Works, but makes hw acceleration impossible, use above code path
        qreal old = painter->opacity();
        painter->setOpacity(opacity);
        painter->drawPixmap(iconOrigin, pixmap);
        painter->setOpacity(old);
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
        int state = activeInterface()->connectionState();
        switch (state) {
            case Solid::Control::NetworkInterface::Preparing:
            case Solid::Control::NetworkInterface::Configuring:
            case Solid::Control::NetworkInterface::NeedAuth:
            case Solid::Control::NetworkInterface::IPConfig:
                if (m_currentState != state) {
                    m_overlayTimeline.start();
                    m_overlayTimeline.setDuration(2000);
                    m_overlayTimeline.setDirection(QTimeLine::Forward);
                    m_overlayTimeline.start();
                }
                setBusy(true);
                break;
            default:
                setBusy(false);
                if (m_currentState != state) {
                    m_overlayTimeline.stop();
                    m_overlayTimeline.setDirection(QTimeLine::Backward);
                    m_overlayTimeline.setDuration(4000);
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
        QString subText;
        qSort(interfaces.begin(), interfaces.end(), networkInterfaceLessThan);
        bool hasActive = false;
        bool iconChanged = false;
        QString icon = "networkmanager";
        foreach (Solid::Control::NetworkInterface *iface, interfaces) {
            if (!subText.isEmpty()) {
                subText += QLatin1String("<br><br>");
            }
            if (iface->connectionState() != Solid::Control::NetworkInterface::Unavailable) {
                hasActive = true;

                QString deviceText = UiUtils::interfaceNameLabel(iface->uni());

                QString ifaceName = iface->interfaceName();
                subText += QString::fromLatin1("<b>%1</b>: %2").arg(deviceText).arg(UiUtils::connectionStateToString(iface->connectionState()));
                Solid::Control::IPv4Config ip4Config = iface->ipV4Config();
                QList<Solid::Control::IPv4Address> addresses = ip4Config.addresses();
                if (!addresses.isEmpty()) {
                    QHostAddress addr(addresses.first().address());
                    QString currentIp = addr.toString();
                    subText += QString::fromLatin1("<br>") + i18nc("Display of the IP (network) address in the tooltip", "<font size=\"-1\">Address: %1</font>", currentIp);
                }
                // Show the first active connection's icon, otherwise the networkmanager icon
                if (!iconChanged && iface->connectionState() == Solid::Control::NetworkInterface::Activated) {
                    icon = UiUtils::iconName(iface);
                    iconChanged = true; // we only want the first one
                }
            }
        }
        if (!hasActive) {
            subText += i18nc("tooltip, all interfaces are down", "Disconnected");
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

void NetworkManagerApplet::manageConnections()
{
    //kDebug() << "opening connection management dialog";
    QStringList args;
    args << "kcm_networkmanagement";
    KToolInvocation::kdeinitExec("kcmshell4", args);
    hidePopup();
}

void NetworkManagerApplet::loadExtender()
{
    Plasma::ExtenderItem *eItem = extender()->item("networkmanagement");
    if (eItem) {
        eItem->destroy(); // Apparently, we need to "refresh the extenderitem
    }
    eItem = new NMExtenderItem(m_activatableList, extender());
    eItem->setName("networkmanagement");
    eItem->setTitle(i18nc("Label for extender","Network Management"));
    eItem->widget();
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
