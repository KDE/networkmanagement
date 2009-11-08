/*
Copyright 2008,2009 Will Stephenson <wstephenson@kde.org>
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
#include <QPaintEngine>
#include <QPainter>
#include <QDesktopWidget>

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
#include <solid/control/networkmanager.h>

#include <Plasma/CheckBox>
#include <Plasma/Extender>
#include <Plasma/ExtenderItem>


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
    : Plasma::PopupApplet(parent, args), m_iconPerDevice(false)
{
    setHasConfigurationInterface(false);
    setPopupIcon(QIcon());
    setPassivePopup(true); // FIXME: disable, only true for testing ...

    Plasma::ToolTipManager::self()->registerWidget(this);
    setAspectRatioMode(Plasma::ConstrainedSquare);
    setHasConfigurationInterface(true);

    m_interfaces = Solid::Control::NetworkManager::networkInterfaces();
    interfaceConnectionStateChanged();

    m_activatableList = new RemoteActivatableList(this);

    // TODO: read config into m_extenderItem ...
    // Now it is safe to create ExtenderItems and therefore InterfaceGroups

}

NetworkManagerApplet::~NetworkManagerApplet()
{
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
        // TODO: optimize: doesn't need to happen on *every* resize
        // only for icon size jumps
        updatePixmap();
    }
}

void NetworkManagerApplet::updatePixmap()
{
    // Todo
    //m_pixmap = UiUtils::interfacePix
}

void NetworkManagerApplet::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *widget = new QWidget(parent);
    ui.setupUi(widget);
    parent->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
    parent->addPage(widget, parent->windowTitle(), Applet::icon());
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));

    ui.showWired->setChecked(m_extenderItem->m_showWired);
    ui.showWireless->setChecked(m_extenderItem->m_showWireless);
    ui.showVpn->setChecked(m_extenderItem->m_showVpn);
    ui.showCellular->setChecked(m_extenderItem->m_showCellular);
}

void NetworkManagerApplet::configAccepted()
{
    KConfigGroup cg = config();

    if (m_extenderItem->m_showWired != ui.showWired->isChecked()) {
        m_extenderItem->showWired(!m_extenderItem->m_showWired);
        cg.writeEntry("showWired", m_extenderItem->m_showWired);
        m_extenderItem->showWired(m_extenderItem->m_showWired);
        kDebug() << "Wired Changed" << m_extenderItem->m_showWired;
    }
    if (m_extenderItem->m_showWireless != ui.showWireless->isChecked()) {
        m_extenderItem->showWireless(!m_extenderItem->m_showWireless);
        cg.writeEntry("showWireless", m_extenderItem->m_showWireless);
        kDebug() << "Wireless Changed" << m_extenderItem->m_showWireless;
    }
    if (m_extenderItem->m_showCellular != ui.showCellular->isChecked()) {
        m_extenderItem->showCellular(!m_extenderItem->m_showCellular);
        cg.writeEntry("showCellular", m_extenderItem->m_showCellular);
        kDebug() << "Gsm Changed" << m_extenderItem->m_showCellular;
    }
    if (m_extenderItem->m_showVpn != ui.showVpn->isChecked()) {
        m_extenderItem->showVpn(!m_extenderItem->m_showVpn);
        cg.writeEntry("showVpn", m_extenderItem->m_showVpn);
        kDebug() << "VPN Changed" << m_extenderItem->m_showVpn;
    }

    Plasma::Applet::configNeedsSaving();
    kDebug() << "config done";
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
    int _i = UiUtils::iconSize(contentsRect.size());
    QSize s(_i, _i);
    QPixmap pixmap;

    if (!m_interfaces.isEmpty()) {
        Solid::Control::NetworkInterface *interface = m_interfaces.first();
        pixmap = KIcon(UiUtils::iconName(interface)).pixmap(s);
    } else {
        pixmap = KIcon("dialog-error").pixmap(s);
    }
    paintPixmap(p, pixmap, contentsRect);
}


void NetworkManagerApplet::paintPixmap(QPainter *painter, QPixmap &pixmap, const QRectF &rect, qreal opacity)
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

/* Slots to react to changes from the daemon */
void NetworkManagerApplet::networkInterfaceAdded(const QString & uni)
{
    Q_UNUSED(uni);
    // update the tray icon
    m_interfaces = Solid::Control::NetworkManager::networkInterfaces();
    foreach (Solid::Control::NetworkInterface* interface, m_interfaces) {

        // be aware of state changes
        QObject::disconnect(interface, SIGNAL(connectionStateChanged(int)), this, SLOT(interfaceConnectionStateChanged()));
        QObject::connect(interface, SIGNAL(connectionStateChanged(int)), this, SLOT(interfaceConnectionStateChanged()));
    }

    interfaceConnectionStateChanged();
    update();
}

void NetworkManagerApplet::networkInterfaceRemoved(const QString & uni)
{
    Q_UNUSED(uni);
    // update the tray icon
    m_interfaces = Solid::Control::NetworkManager::networkInterfaces();
    foreach (Solid::Control::NetworkInterface * interface, m_interfaces) {
        QObject::disconnect(interface, SIGNAL(connectionStateChanged(int)), this, SLOT(interfaceConnectionStateChanged()));
        QObject::connect(interface, SIGNAL(connectionStateChanged(int)), this, SLOT(interfaceConnectionStateChanged()));
    }

    // update extender visibility
    KConfigGroup cg = config();

    interfaceConnectionStateChanged();
    update();
    // kill any animations involving this interface
}

void NetworkManagerApplet::interfaceConnectionStateChanged()
{
    if (!m_interfaces.isEmpty()) {
        qSort(m_interfaces.begin(), m_interfaces.end(), networkInterfaceLessThan);
        Solid::Control::NetworkInterface * interface = m_interfaces.first();
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

                QString deviceText;
                KNetworkManagerServicePrefs::instance(Knm::ConnectionPersistence::NETWORKMANAGEMENT_RCFILE);

                if (KNetworkManagerServicePrefs::self()->interfaceNamingStyle() == KNetworkManagerServicePrefs::DescriptiveNames) {
                    deviceText = UiUtils::descriptiveInterfaceName(iface->type());
                } else {
                    deviceText = iface->interfaceName();
                }

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
    enum { Connecting, Connected, Disconnected } if2status = Disconnected, if1status = Disconnected;
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
        default: // all kind of disconnected
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
        default: // all kind of disconnected
            break;
    }
    switch (if1status) {
        case Connecting:
            return if2status != Connecting || networkInterfaceSameConnectionStateLessThan(if1, if2);
            break;
        case Connected:
            if ( if2status == Connecting)
               return false;
            return if2status != Connected || networkInterfaceSameConnectionStateLessThan(if1, if2);
            break;
        case Disconnected:
            if ( if2status == Disconnected)
                return networkInterfaceSameConnectionStateLessThan(if1, if2);
            return false;
            break;
    }
    // satisfy compiler
    return false;
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

void NetworkManagerApplet::managerWirelessEnabledChanged(bool )
{
}

void NetworkManagerApplet::managerWirelessHardwareEnabledChanged(bool enabled)
{
    Q_UNUSED( enabled );
    // This isn't necessary since all interfaceitems
    // should react to changes by themselves.
}

void NetworkManagerApplet::userNetworkingEnabledChanged(bool enabled)
{
    kDebug() << enabled;
    Solid::Control::NetworkManager::setNetworkingEnabled(enabled);
}

void NetworkManagerApplet::userWirelessEnabledChanged(bool enabled)
{
    kDebug() << enabled;
    Solid::Control::NetworkManager::setWirelessEnabled(enabled);
}

void NetworkManagerApplet::managerStatusChanged(Solid::Networking::Status status)
{
    if (Solid::Networking::Unknown == status ) {
        // FIXME: Do something smart
    } else {
        // ...
    }
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

void NetworkManagerApplet::hideVpnGroup()
{
    m_extenderItem->m_showVpn = false;
    KConfigGroup cg = config();
    cg.writeEntry("showVpn", m_extenderItem->m_showVpn);
    m_extenderItem->showVpn(false);
    Plasma::Applet::configNeedsSaving();
}

void NetworkManagerApplet::popupEvent(bool show)
{
    // Notify the wireless extender of popup events so it can revert its hidden wireless network
    // item to button mode
    if (show && m_extenderItem) {
        m_extenderItem->switchToDefaultTab();
    }
    return;
}

#include "networkmanager.moc"
