/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>
Copyright 2008 Sebastian Kügler <sebas@kde.org>

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

#include <QIcon>
#include <QPainter>

#if KDE_IS_VERSION(4,1,70)
#else
#include <QVBoxLayout>
#include <Plasma/Corona>
#include <Plasma/Dialog>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <KGlobalSettings>
#include <QGraphicsGridLayout>
#endif

#include <KIcon>
#include <KIconLoader>
#include <KToolInvocation>
#include <KConfigDialog>

#include <solid/control/networkmanager.h>
#include <solid/control/networkinterface.h>

#include "interfacegroup.h"

// to go:
#include "interfaceitem.h"
#include "networkmanagerpopup.h"

K_EXPORT_PLASMA_APPLET(networkmanager, NetworkManagerApplet)

/* for qSort()ing */
bool networkInterfaceLessThan(Solid::Control::NetworkInterface * if1, Solid::Control::NetworkInterface * if2);
bool networkInterfaceSameConnectionStateLessThan(Solid::Control::NetworkInterface * if1, Solid::Control::NetworkInterface * if2);

NetworkManagerApplet::NetworkManagerApplet(QObject * parent, const QVariantList & args)
#if KDE_IS_VERSION(4,1,70)
: Plasma::PopupApplet(parent, args), m_iconPerDevice(false), m_svg(0)
#else
: Plasma::Applet(parent, args), m_iconPerDevice(false), m_svg(0), m_dialog(0)
#endif
{
    setHasConfigurationInterface(false);
#if KDE_IS_VERSION(4,1,70)
    setPopupIcon(QIcon());
    setPassivePopup(true);
#endif
    updateToolTip();
    setAspectRatioMode(Plasma::ConstrainedSquare);
    setHasConfigurationInterface(true);
    m_svg = new Plasma::Svg(this);
    m_svg->setImagePath("networkmanager/networkmanager");

    m_wirelessSvg = new Plasma::Svg(this);
    m_wirelessSvg->setImagePath("networkmanager/networkmanager-wireless");

    m_interfaces = Solid::Control::NetworkManager::networkInterfaces();
    interfaceConnectionStateChanged();

    m_popup = new NetworkManagerPopup(this);

    KConfigGroup cg = config();
    m_popup->showWired(cg.readEntry("showWired", true));
    m_popup->showWireless(cg.readEntry("showWireless", true));
    m_popup->showVpn(cg.readEntry("showVpn", true));
    m_popup->showGsm(cg.readEntry("showGsm", true));

    QObject::connect(m_popup, SIGNAL(manageConnections()),
            this, SLOT(manageConnections()));
}

NetworkManagerApplet::~NetworkManagerApplet()
{

}

void NetworkManagerApplet::init()
{
    KConfigGroup cg = config();
    m_iconPerDevice = cg.readEntry("IconPerDevice", false);
    m_svg->resize(contentsRect().size());
    QObject::connect(Solid::Control::NetworkManager::notifier(), SIGNAL(networkInterfaceAdded(const QString&)),
            this, SLOT(networkInterfaceAdded(const QString&)));
    QObject::connect(Solid::Control::NetworkManager::notifier(), SIGNAL(networkInterfaceRemoved(const QString&)),
            this, SLOT(networkInterfaceRemoved(const QString&)));
    foreach (Solid::Control::NetworkInterface * interface,
            Solid::Control::NetworkManager::networkInterfaces()) {
        QObject::connect(interface, SIGNAL(connectionStateChanged(int)), this, SLOT(interfaceConnectionStateChanged()));
    }
}

void NetworkManagerApplet::constraintsEvent(Plasma::Constraints constraints)
{
   if (constraints & (Plasma::SizeConstraint | Plasma::FormFactorConstraint)) {
        m_svg->resize(contentsRect().size().toSize());
        m_wirelessSvg->resize(contentsRect().size().toSize());
        updateIcons();
    }
}

void NetworkManagerApplet::updateIcons()
{
    m_pixmapWiredConnected = KIcon("network-connect").pixmap(contentsRect().size().toSize());
    m_pixmapWiredDisconnected = KIcon("network-disconnect").pixmap(contentsRect().size().toSize());
}

void NetworkManagerApplet::createConfigurationInterface(KConfigDialog *parent)
{

    QWidget *widget = new QWidget(parent);
    ui.setupUi(widget);
    parent->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
    parent->addPage(widget, parent->windowTitle(), Applet::icon());
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));

    ui.showWired->setChecked(m_popup->wiredShown());
    ui.showWireless->setChecked(m_popup->wirelessShown());
    ui.showVpn->setChecked(m_popup->vpnShown());
    ui.showGsm->setChecked(m_popup->gsmShown());
}

void NetworkManagerApplet::configAccepted()
{
    KConfigGroup cg = config();

    if (m_popup->wiredShown() != ui.showVpn->isChecked()) {
        m_popup->showWired(!m_popup->wiredShown());
        cg.writeEntry("showWired", m_popup->wiredShown());
    }
}

void NetworkManagerApplet::paintInterface(QPainter * p, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    // i can't figure out how to do layouting of multiple items in constraintsEvent properly,
    // so only have 1 rather than hack something ugly that will be thrown out later
    if (!m_interfaces.isEmpty()) {
        Solid::Control::NetworkInterface *interface = m_interfaces.first();
        //kDebug() << "most interesting interface to paint: " << iface->uni() << " with icon " << m_elementName;

        // Call the correct method to paint the applet, depending on the kind of connection
        switch (interface->type() ) {
            case Solid::Control::NetworkInterface::Ieee80211:
                paintWirelessInterface(interface, p, option, contentsRect);
                break;
            case Solid::Control::NetworkInterface::Ieee8023:
                paintWiredInterface(interface, p, option, contentsRect);
                break;
            case Solid::Control::NetworkInterface::Serial:
            case Solid::Control::NetworkInterface::Gsm:
            case Solid::Control::NetworkInterface::Cdma:
            default:
                paintDefaultInterface(interface, p, option, contentsRect);
                break;
        }
    }
}

void NetworkManagerApplet::paintDefaultInterface(Solid::Control::NetworkInterface* interface, QPainter * p, const QStyleOptionGraphicsItem * option, const QRect &contentsRect)
{
    Q_UNUSED(option);
    Q_UNUSED(interface);
    //kDebug() << " ============== Default Interface";
    m_svg->paint(p, contentsRect, m_elementName);
}

void NetworkManagerApplet::paintWiredInterface(Solid::Control::NetworkInterface* interface, QPainter * p, const QStyleOptionGraphicsItem * option, const QRect &contentsRect)
{
    Q_UNUSED( option );
    if (interface->connectionState() == Solid::Control::NetworkInterface::Activated) {
        p->drawPixmap(contentsRect.topLeft(), m_pixmapWiredConnected);
    } else {
        p->drawPixmap(contentsRect.topLeft(), m_pixmapWiredDisconnected);
    }
}

void NetworkManagerApplet::paintWirelessInterface(Solid::Control::NetworkInterface* interface, QPainter * p, const QStyleOptionGraphicsItem * option, const QRect &contentsRect)
{
    Q_UNUSED(option);
    //kDebug() << interface->type();
    /* TODO:
        enum ConnectionState{ UnknownState, Unmanaged, Unavailable, Disconnected, Preparing,
                    Configuring, NeedAuth, IPConfig, Activated, Failed };
        make use of this information...
    */
    //kDebug() << " ============== Wireless Interface";
    switch (interface->connectionState()) {
        case Solid::Control::NetworkInterface::UnknownState:
            //kDebug() << " ... UnknownState";
            m_wirelessSvg->paint(p, contentsRect, "antenna");
            break;
        case Solid::Control::NetworkInterface::Unmanaged:
            //kDebug() << " ... Unmanaged";
            m_wirelessSvg->paint(p, contentsRect, "antenna");
            break;
        case Solid::Control::NetworkInterface::Unavailable:
            //kDebug() << " ... Unavailable";
            m_wirelessSvg->paint(p, contentsRect, "antenna");
            break;
        case Solid::Control::NetworkInterface::Disconnected:
            //kDebug() << " ... Disconnected";
            m_wirelessSvg->paint(p, contentsRect, "antenna");
            break;
        case Solid::Control::NetworkInterface::Preparing:
            //kDebug() << " ... Preparing";
            break;
        case Solid::Control::NetworkInterface::Configuring:
            //kDebug() << " ... Configuring";
            break;
        case Solid::Control::NetworkInterface::NeedAuth:
            //kDebug() << " ... NeedAuth";
            break;
        case Solid::Control::NetworkInterface::IPConfig:
            //kDebug() << " ... IPConfig";
            break;
        case Solid::Control::NetworkInterface::Activated:
            //kDebug() << " ... Activated";
            m_wirelessSvg->paint(p, contentsRect, "connected");
            m_wirelessSvg->paint(p, contentsRect, "antenna");
            break;
        case Solid::Control::NetworkInterface::Failed:
            //kDebug() << " ... Failed";
            m_wirelessSvg->paint(p, contentsRect, "antenna");
            break;
        default:
            //kDebug() << "dunno ...";
            break;
    }
}

QGraphicsWidget * NetworkManagerApplet::graphicsWidget()
{
    return m_popup;
}

/* Slots to react to changes from the daemon */
void NetworkManagerApplet::networkInterfaceAdded(const QString & uni)
{
    Q_UNUSED(uni);
    // update the tray icon
    m_interfaces = Solid::Control::NetworkManager::networkInterfaces();
    interfaceConnectionStateChanged();
    update();
}

void NetworkManagerApplet::networkInterfaceRemoved(const QString & uni)
{
    Q_UNUSED(uni);
    // update the tray icon
    m_interfaces = Solid::Control::NetworkManager::networkInterfaces();
    interfaceConnectionStateChanged();
    update();
    // kill any animations involving this interface
}

void NetworkManagerApplet::interfaceConnectionStateChanged()
{
    //kDebug() << "Updating connection state ...";
    //Solid::Control::NetworkInterface * interface = static_cast<Solid::Control::NetworkInterface *>(sender());
    // update appearance
    QString elementNameToPaint;
    if (!m_interfaces.isEmpty()) {
        qSort(m_interfaces.begin(), m_interfaces.end(), networkInterfaceLessThan);
        Solid::Control::NetworkInterface * interface = m_interfaces.first();
        switch (interface->type() ) {
            case Solid::Control::NetworkInterface::Ieee8023:
                elementNameToPaint = "wired";
                break;
            case Solid::Control::NetworkInterface::Ieee80211:
                elementNameToPaint = "wireless";
                break;
            case Solid::Control::NetworkInterface::Serial:
                elementNameToPaint = "ppp";
                break;
            case Solid::Control::NetworkInterface::Gsm:
            case Solid::Control::NetworkInterface::Cdma:
                elementNameToPaint = "cellular";
                break;
            default:
                elementNameToPaint = "wired";
                break;
        }
        if (interface->connectionState() == Solid::Control::NetworkInterface::Activated) {
            elementNameToPaint += "_connected";
        } else {
            elementNameToPaint += "_disconnected";
        }
        //kDebug() << "busy ... ?";
        switch (interface->connectionState()) {
            case Solid::Control::NetworkInterface::Preparing:
            case Solid::Control::NetworkInterface::Configuring:
            case Solid::Control::NetworkInterface::NeedAuth:
            case Solid::Control::NetworkInterface::IPConfig:
                setBusy(true);
                break;
            default:
                setBusy(false);
                break;
        }
    } else {
        elementNameToPaint = "nointerfaces";
    }

    if (elementNameToPaint != m_elementName) {
        m_elementName = elementNameToPaint;
        update();
    }

    updateToolTip();
}


void NetworkManagerApplet::updateToolTip()
{
    Solid::Control::NetworkInterfaceList interfaces
        = Solid::Control::NetworkManager::networkInterfaces();
    if (interfaces.isEmpty()) {
        m_toolTip = Plasma::ToolTipContent(i18nc("Tooltip main title text", "Networks"),
                i18nc("Tooltip sub text", "No network interfaces"),
                KIcon("networkmanager").pixmap(IconSize(KIconLoader::Desktop))
                );
    } else {
        QString subText;
        qSort(interfaces.begin(), interfaces.end(), networkInterfaceLessThan);
        foreach (Solid::Control::NetworkInterface *iface, interfaces) {
            if (!subText.isEmpty()) {
                subText += QLatin1String("<br>");
            }
            subText += QString::fromLatin1("<b>%1</b>: %2").arg(iface->interfaceName()).arg(connectionStateToString(iface->connectionState()));
            m_toolTip = Plasma::ToolTipContent(i18nc("Tooltip main title text", "Networks"),
                subText,
                KIcon("networkmanager").pixmap(IconSize(KIconLoader::Desktop))
                );
            Plasma::ToolTipManager::self()->setContent(this, m_toolTip);
        }
    }
}

QString NetworkManagerApplet::connectionStateToString(Solid::Control::NetworkInterface::ConnectionState state)
{
    QString stateString;
    switch (state) {
        case Solid::Control::NetworkInterface::UnknownState:
            stateString = i18nc("description of unknown network interface state", "Unknown");
            break;
        case Solid::Control::NetworkInterface::Unmanaged:
            stateString = i18nc("description of unmanaged network interface state", "Unmanaged");
            break;
        case Solid::Control::NetworkInterface::Unavailable:
            stateString = i18nc("description of unavailable network interface state", "Unavailable");
            break;
        case Solid::Control::NetworkInterface::Disconnected:
            stateString = i18nc("description of unconnected network interface state", "Not connected");
            break;
        case Solid::Control::NetworkInterface::Preparing:
            stateString = i18nc("description of preparing to connect network interface state", "Preparing to connect");
            break;
        case Solid::Control::NetworkInterface::Configuring:
            stateString = i18nc("description of configuring hardware network interface state", "Configuring interface");
            break;
        case Solid::Control::NetworkInterface::NeedAuth:
            stateString = i18nc("description of waiting for authentication network interface state", "Waiting for authorization");
            break;
        case Solid::Control::NetworkInterface::IPConfig:
            stateString = i18nc("network interface doing dhcp request in most cases", "Setting network address");
            break;
        case Solid::Control::NetworkInterface::Activated:
            stateString = i18nc("network interface connected state label", "Connected");
            break;
        case Solid::Control::NetworkInterface::Failed:
            stateString = i18nc("network interface connection failed state label", "Connection Failed");
            break;
        default:
            stateString = I18N_NOOP("UNKNOWN STATE FIX ME");
    }
    return stateString;
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
    bool lessThan = false;
    Solid::Control::NetworkInterface::ConnectionState if2State = if2->connectionState();
    switch (if1->connectionState()) {
        case Solid::Control::NetworkInterface::Preparing:
        case Solid::Control::NetworkInterface::Configuring:
        case Solid::Control::NetworkInterface::NeedAuth:
        case Solid::Control::NetworkInterface::IPConfig:
            if ( if2State == Solid::Control::NetworkInterface::Preparing
                    || if2State == Solid::Control::NetworkInterface::Configuring
                    || if2State == Solid::Control::NetworkInterface::NeedAuth
                    || if2State == Solid::Control::NetworkInterface::IPConfig
               ) {
                lessThan = networkInterfaceSameConnectionStateLessThan(if1, if2);
            } else {
                lessThan = true;
            }
            break;
        case Solid::Control::NetworkInterface::Activated:
            switch (if2->connectionState()) {
                case Solid::Control::NetworkInterface::Preparing:
                case Solid::Control::NetworkInterface::Configuring:
                case Solid::Control::NetworkInterface::NeedAuth:
                case Solid::Control::NetworkInterface::IPConfig:
                    lessThan = false;
                    break;
                case Solid::Control::NetworkInterface::Activated:
                    lessThan = networkInterfaceSameConnectionStateLessThan(if1, if2);
                    break;
                default:
                    lessThan = true;
            }
            break;
        default:
            lessThan = networkInterfaceSameConnectionStateLessThan(if1, if2);

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
    args << "kcm_knetworkmanager";
    KToolInvocation::kdeinitExec("kcmshell4", args);
    hidePopup();
}


#if KDE_IS_VERSION(4,1,70)
#else

void NetworkManagerApplet::setBusy(bool busy)
{
    Q_UNUSED(busy);
    // We can reimplement that with some animation later on,
    // for now, leave it like this so the applet still compiles
    // with KDE 4.1.
}

void NetworkManagerApplet::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->buttons() == Qt::LeftButton) {
        m_clicked = scenePos().toPoint();
        event->setAccepted(true);
        return;
    }
    Applet::mousePressEvent(event);
}

void NetworkManagerApplet::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if ((m_clicked - scenePos().toPoint()).manhattanLength() <
            KGlobalSettings::dndEventDelay()) {
        showLegacyPopup(event);
    }
}

void NetworkManagerApplet::showLegacyPopup(QGraphicsSceneMouseEvent *event)
{
#if 0
    Q_UNUSED(event);

    if (m_dialog == 0) {
        QGraphicsWidget * gWidget = graphicsWidget();

        m_dialog = new Plasma::Dialog();
        //m_dialog->setWindowFlags(Qt::Popup);
        if (gWidget) {
            Plasma::Corona *corona = qobject_cast<Plasma::Corona *>(gWidget->scene());

            //could that cast ever fail??
            if (corona) {
                //duplicated from  Corona::addOffscreenWidget()
                QGraphicsWidget *offscreenWidget = new QGraphicsWidget(0);
                corona->addItem(offscreenWidget);
                QGraphicsGridLayout * offscreenLayout = new QGraphicsGridLayout(offscreenWidget);
                offscreenWidget->setPos(-10000, -10000);
                offscreenWidget->setLayout(offscreenLayout);
                offscreenLayout->addItem(gWidget, 0, 0);
                gWidget->update();
                // end dupe
                gWidget->resize(gWidget->preferredSize());
                gWidget->setMinimumSize(gWidget->preferredSize());
                // duplicated from Dialog::setGraphicsWidget()
                QVBoxLayout *lay = new QVBoxLayout(m_dialog);
                lay->setMargin(0);
                lay->setSpacing(0);
                QGraphicsView * view = new QGraphicsView(m_dialog);
                view->setFrameShape(QFrame::NoFrame);
                view->viewport()->setAutoFillBackground(false);
                m_dialog->layout()->addWidget(view);
                view->setScene(gWidget->scene());
                //m_dialog->adjustView();
                // duped from DialogPrivate::adjustView()
                QSize prevSize = m_dialog->size();

                kDebug() << "Widget size:" << gWidget->size()
                    << "| Widget size hint:" << gWidget->effectiveSizeHint(Qt::PreferredSize)
                    << "| Widget bounding rect:" << gWidget->boundingRect();

                QRectF boundingRect = gWidget->boundingRect();
                boundingRect.setSize(gWidget->effectiveSizeHint(Qt::PreferredSize));

                //reposition and resize the view.
                view->setSceneRect(gWidget->mapToScene(boundingRect).boundingRect());
                view->resize(view->mapFromScene(view->sceneRect()).boundingRect().size());
                view->centerOn(gWidget);

                //set the sizehints correctly:
                int left, top, right, bottom;
                m_dialog->getContentsMargins(&left, &top, &right, &bottom);

                m_dialog->setMinimumSize(qMin(int(gWidget->minimumSize().width()) + left + right, QWIDGETSIZE_MAX),
                        qMin(int(gWidget->minimumSize().height()) + top + bottom, QWIDGETSIZE_MAX));
                m_dialog->resize(qMin(int(gWidget->minimumSize().width()) + left + right, QWIDGETSIZE_MAX),
                        qMin(int(gWidget->minimumSize().height()) + top + bottom, QWIDGETSIZE_MAX));
                m_dialog->setMaximumSize(qMin(int(gWidget->maximumSize().width()) + left + right, QWIDGETSIZE_MAX),
                        qMin(int(gWidget->maximumSize().height()) + top + bottom, QWIDGETSIZE_MAX));
                m_dialog->updateGeometry();

                //end of dupe from adjustView();
                m_dialog->adjustSize();
            }
        }
    }
    if (m_dialog->isVisible()) {
        m_dialog->hide();
    } else {
        kDebug();
        m_dialog->show();
    }
#endif
}

#endif
#include "networkmanager.moc"
