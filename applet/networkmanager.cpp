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
#include <QDesktopWidget>

#include <KIcon>
#include <KIconLoader>
#include <KToolInvocation>
#include <KConfigDialog>
#include <KDebug>
#include <KLocale>
#include <KNotification>
#include <KPushButton>

#include <solid/networking.h>
#include <solid/control/networking.h>
#include <solid/control/networkinterface.h>
#include <solid/control/networkmanager.h>


#include <NetworkManager.h>
#include <nm-setting-cdma.h>
#include <nm-setting-connection.h>
#include <nm-setting-gsm.h>
#include <nm-setting-pppoe.h>
#include <nm-setting-vpn.h>
#include <nm-setting-wired.h>
#include <nm-setting-wireless.h>

#include <Plasma/CheckBox>
#include <Plasma/Extender>
#include <Plasma/ExtenderItem>

#include "interfacegroup.h"
#include "../libs/types.h"
#include "vpnconnectiongroup.h"
#include "networkmanagersettings.h"
#include "interfaceitem.h"
#include "events.h"

K_EXPORT_PLASMA_APPLET(networkmanagement, NetworkManagerApplet)

/* for qSort()ing */
bool networkInterfaceLessThan(Solid::Control::NetworkInterface * if1, Solid::Control::NetworkInterface * if2);
bool networkInterfaceSameConnectionStateLessThan(Solid::Control::NetworkInterface * if1, Solid::Control::NetworkInterface * if2);

NetworkManagerApplet::NetworkManagerApplet(QObject * parent, const QVariantList & args)
    : Plasma::PopupApplet(parent, args), m_iconPerDevice(false), m_svg(0)
{
    setHasConfigurationInterface(false);
    setPopupIcon(QIcon());
    //setPassivePopup(true); // only for testing ...

    updateToolTip();
    setAspectRatioMode(Plasma::ConstrainedSquare);
    setHasConfigurationInterface(true);
    m_svg = new Plasma::Svg(this);
    m_svg->setImagePath("networkmanagement/networkmanagement");

    m_wirelessSvg = new Plasma::Svg(this);
    m_wirelessSvg->setImagePath("networkmanagement/networkmanagement-wireless");

    m_interfaces = Solid::Control::NetworkManager::networkInterfaces();
    interfaceConnectionStateChanged();

    // FIXME:: Add manage connection button
    //QObject::connect(this, SIGNAL(manageConnections()),
    //        this, SLOT(manageConnections()));

    // This MUST happen before any InterfaceGroups are instantiated
    // kickstart the kded module
    QDBusInterface ref( "org.kde.kded", "/modules/knetworkmanager",
                        "org.kde.knetworkmanagerd", QDBusConnection::sessionBus() );

    WId wid = QApplication::desktop()->effectiveWinId();
    kDebug() << wid;
    ref.call( "start", qlonglong( wid ) );
    // not really interesting, for now we only care to kick the load-on-demand
    kDebug() << ref.isValid() << ref.lastError().message() << ref.lastError().name();

    m_userSettings = new NetworkManagerSettings(QLatin1String(NM_DBUS_SERVICE_USER_SETTINGS), this);
    m_userSettings->setObjectName("user-settings-service");
    m_systemSettings = new NetworkManagerSettings(QLatin1String(NM_DBUS_SERVICE_SYSTEM_SETTINGS), this);
    m_systemSettings->setObjectName("system-settings-service");
    // Now it is safe to create ExtenderItems and therefore InterfaceGroups

}

NetworkManagerApplet::~NetworkManagerApplet()
{
    QDBusInterface ref( "org.kde.kded", "/modules/knetworkmanager",
                        "org.kde.knetworkmanagerd", QDBusConnection::sessionBus() );
    // ## used to have NoEventLoop and 3s timeout with dcop
    ref.call( QLatin1String("stop") );
    //ref.call("stop");
    kDebug() << ref.isValid() << ref.lastError().message() << ref.lastError().name();
}

void NetworkManagerApplet::init()
{
    kDebug();
    KConfigGroup cg = config();
    m_iconPerDevice = cg.readEntry("IconPerDevice", false);
    m_svg->resize(contentsRect().size());
    QObject::connect(Solid::Control::NetworkManager::notifier(), SIGNAL(networkInterfaceAdded(const QString&)),
            this, SLOT(networkInterfaceAdded(const QString&)));
    QObject::connect(Solid::Control::NetworkManager::notifier(), SIGNAL(networkInterfaceRemoved(const QString&)),
            this, SLOT(networkInterfaceRemoved(const QString&)));

    // Set up the extender with its various groups.  The first call to extender() triggers calls to
    // initExtenderItem() if there are any detached items.
    extender()->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    // calling this initialises the extenderitems
    networkInterfaceAdded(QString());
    // add VPN last
    showVpn(cg.readEntry("showVpn", true));

    QObject::connect(Solid::Control::NetworkManager::notifier(), SIGNAL(statusChanged(Solid::Networking::Status)),
                     this, SLOT(managerStatusChanged(Solid::Networking::Status)));

}

void NetworkManagerApplet::initExtenderItem(Plasma::ExtenderItem * eItem)
{
    const QString WIRED_EXTENDER_ITEM_NAME = QLatin1String("wired");
    const QString WIRELESS_EXTENDER_ITEM_NAME = QLatin1String("wireless");
    const QString GSM_EXTENDER_ITEM_NAME = QLatin1String("gsm");
    const QString CDMA_EXTENDER_ITEM_NAME = QLatin1String("cdma");
    const QString VPN_EXTENDER_ITEM_NAME = QLatin1String("vpn");

    if (eItem->name() == WIRED_EXTENDER_ITEM_NAME) {
        InterfaceGroup * ethernetGroup = new InterfaceGroup(Solid::Control::NetworkInterface::Ieee8023, m_userSettings, m_systemSettings, eItem);
        ethernetGroup->setObjectName("ethernet-interface-group");
        ethernetGroup->init();
        eItem->setWidget(ethernetGroup);
    } else if (eItem->name() == WIRELESS_EXTENDER_ITEM_NAME) {
        m_wifiGroup = new InterfaceGroup(Solid::Control::NetworkInterface::Ieee80211, m_userSettings, m_systemSettings, eItem);
        m_wifiGroup->setObjectName("wifi-interface-group");
        m_wifiGroup->init();
        KConfigGroup cg = config();
        m_numberOfWlans = cg.readEntry("numberOfWlans", 4);
        m_wifiGroup->setNetworksLimit( m_numberOfWlans );

        eItem->setWidget(m_wifiGroup);
    } else if (eItem->name() == GSM_EXTENDER_ITEM_NAME) {
        InterfaceGroup * gsmGroup = new InterfaceGroup(Solid::Control::NetworkInterface::Gsm, m_userSettings, m_systemSettings, eItem);
        gsmGroup->setObjectName("gsm-interface-group");
        gsmGroup->init();

        eItem->setWidget(gsmGroup);
    } else if (eItem->name() == CDMA_EXTENDER_ITEM_NAME) {
        InterfaceGroup * cdmaGroup = new InterfaceGroup(Solid::Control::NetworkInterface::Cdma, m_userSettings, m_systemSettings, eItem);
        cdmaGroup->setObjectName("cdma-interface-group");
        cdmaGroup->init();

        eItem->setWidget(cdmaGroup);
    } else if (eItem->name() == VPN_EXTENDER_ITEM_NAME) {
        VpnConnectionGroup * vpnGroup = new VpnConnectionGroup(m_userSettings, m_systemSettings, eItem);
        vpnGroup->setObjectName("vpn-interface-group");
        vpnGroup->init();
        eItem->setWidget(vpnGroup);
        connect(vpnGroup, SIGNAL(hideClicked()), SLOT(hideVpnGroup()));
    } else {
        kDebug() << "Unrecognised extender name!  Is the config from the future?";
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

    ui.showWired->setChecked(m_showWired);
    ui.showWireless->setChecked(m_showWireless);
    ui.showVpn->setChecked(m_showVpn);
    ui.showCellular->setChecked(m_showCellular);
    ui.numberOfWlans->setValue(m_numberOfWlans);
}

void NetworkManagerApplet::configAccepted()
{
    KConfigGroup cg = config();

    if (m_showWired != ui.showWired->isChecked()) {
        showWired(!m_showWired);
        cg.writeEntry("showWired", m_showWired);
        kDebug() << "Wired Changed" << m_showWired;
    }
    if (m_showWireless != ui.showWireless->isChecked()) {
        showWireless(!m_showWireless);
        cg.writeEntry("showWireless", m_showWireless);
        kDebug() << "Wireless Changed" << m_showWireless;
    }
    if (m_showCellular != ui.showCellular->isChecked()) {
        showCellular(!m_showCellular);
        cg.writeEntry("showCellular", m_showCellular);
        kDebug() << "Gsm Changed" << m_showCellular;
    }
    if (m_showVpn != ui.showVpn->isChecked()) {
        showVpn(!m_showVpn);
        cg.writeEntry("showVpn", m_showVpn);
        kDebug() << "VPN Changed" << m_showVpn;
    }
    int wlans = ui.numberOfWlans->value();
    if (wlans != m_numberOfWlans) {
        m_numberOfWlans = wlans;
        if (m_wifiGroup) {
            m_wifiGroup->setNetworksLimit( m_numberOfWlans );
        }
        cg.writeEntry("numberOfWlans", m_numberOfWlans);
        kDebug() << "No of WLANS Changed:" << wlans;
    }
    Plasma::Applet::configNeedsSaving();
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
    // i can't figure out how to do layouting of multiple items in constraintsEvent properly,
    // so only have 1 rather than hack something ugly that will be thrown out later
    if (!m_interfaces.isEmpty()) {
        Solid::Control::NetworkInterface *interface = m_interfaces.first();
        //kDebug() << "most interesting interface to paint: " << interface->uni() << " with icon " << m_elementName;

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

/*
QGraphicsWidget * NetworkManagerApplet::graphicsWidget()
{
    return m_popup;
}
*/

/* Slots to react to changes from the daemon */
void NetworkManagerApplet::networkInterfaceAdded(const QString & uni)
{
    Q_UNUSED(uni);
    // update the tray icon
    m_interfaces = Solid::Control::NetworkManager::networkInterfaces();
    foreach (Solid::Control::NetworkInterface * interface,
            m_interfaces) {

        // be aware of state changes
        QObject::disconnect(interface, SIGNAL(connectionStateChanged(int)), this, SLOT(interfaceConnectionStateChanged()));
        QObject::connect(interface, SIGNAL(connectionStateChanged(int)), this, SLOT(interfaceConnectionStateChanged()));
    }

    // update extender visibility
    KConfigGroup cg = config();
    showWired(cg.readEntry("showWired", true));
    showWireless(cg.readEntry("showWireless", true));
    //showPppoe(cg.readEntry("showPppoe", true));
    showCellular(cg.readEntry("showCellular", true));
    //showCdma(cg.readEntry("showCdma", true));

    interfaceConnectionStateChanged();
    update();
}

void NetworkManagerApplet::networkInterfaceRemoved(const QString & uni)
{
    Q_UNUSED(uni);
    // update the tray icon
    m_interfaces = Solid::Control::NetworkManager::networkInterfaces();
    foreach (Solid::Control::NetworkInterface * interface,
            Solid::Control::NetworkManager::networkInterfaces()) {
        QObject::disconnect(interface, SIGNAL(connectionStateChanged(int)), this, SLOT(interfaceConnectionStateChanged()));
        QObject::connect(interface, SIGNAL(connectionStateChanged(int)), this, SLOT(interfaceConnectionStateChanged()));
    }

    // update extender visibility
    KConfigGroup cg = config();
    showWired(cg.readEntry("showWired", true));
    showWireless(cg.readEntry("showWireless", true));
    //showPppoe(cg.readEntry("showPppoe", true));
    showCellular(cg.readEntry("showCellular", true));
    //showCdma(cg.readEntry("showCdma", true));

    interfaceConnectionStateChanged();
    update();
    // kill any animations involving this interface
}

void NetworkManagerApplet::interfaceConnectionStateChanged()
{
    /* Solid::Control::NetworkInterface * interface = dynamic_cast<Solid::Control::NetworkInterface *>(sender());
    if (interface)
       kDebug() << "Updating connection state ..." << interface->uni() << interface->type(); */
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
        m_toolTip = Plasma::ToolTipContent(name(),
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
            m_toolTip = Plasma::ToolTipContent(name(),
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

void NetworkManagerApplet::showWired(bool show)
{
    m_showWired = show;
    Plasma::ExtenderItem *eItem = extender()->item("wired");
    if (show && hasInterfaceOfType(Solid::Control::NetworkInterface::Ieee8023)) {
        if (!eItem) {
            kDebug() << "SHOWING";
            eItem = new Plasma::ExtenderItem(extender());
            eItem->setName("wired");
            eItem->setIcon("network-wired");
            eItem->setTitle(i18nc("Label for ethernet group in popup","Ethernet"));
            initExtenderItem(eItem);
        }
    } else {
        if (eItem) {
            kDebug() << "HIDING";
            eItem->destroy();
        }
    }
}

void NetworkManagerApplet::showWireless(bool show)
{
    m_showWireless = show;
    Plasma::ExtenderItem *eItem = extender()->item("wireless");
    if (show && hasInterfaceOfType(Solid::Control::NetworkInterface::Ieee80211)) {
        if (!eItem) {
            kDebug() << "SHOWING";
            eItem = new Plasma::ExtenderItem(extender());
            eItem->setName("wireless");
            eItem->setIcon("network-wireless");
            eItem->setTitle(i18nc("Label for wifi networks in popup","Wireless"));
            initExtenderItem(eItem);
        }
    } else {
        if (eItem) {
            kDebug() << "HIDING";
            eItem->destroy();
        }
    }
}

void NetworkManagerApplet::showVpn(bool show)
{
    m_showVpn = show;
    Plasma::ExtenderItem *eItem = extender()->item("vpn");
    if (show) {
        if (!eItem) {
            kDebug() << "SHOWING";
            eItem = new Plasma::ExtenderItem(extender());
            eItem->setName("vpn");
            eItem->setIcon("network-server");
            eItem->setTitle(i18nc("Label for vpn connections in popup","VPN Connections"));
            initExtenderItem(eItem);
        }
    } else {
        if (eItem) {
            kDebug() << "HIDING";
            eItem->destroy();
        }
    }
}

void NetworkManagerApplet::showCellular(bool show)
{
    m_showCellular = show;
    Plasma::ExtenderItem *gsmItem = extender()->item("gsm");
    if (show && hasInterfaceOfType(Solid::Control::NetworkInterface::Gsm)) {
        if (!gsmItem) {
            gsmItem = new Plasma::ExtenderItem(extender());
            gsmItem->setName("gsm");
            gsmItem->setIcon("phone");
            gsmItem->setTitle(i18nc("Label for mobile broadband (GSM/CDMA/UMTS/HDSPA etc)","Mobile Broadband"));
            initExtenderItem(gsmItem);
        }
    } else {
        if (gsmItem) {
            gsmItem->destroy();
        }
    }
    Plasma::ExtenderItem *cdmaItem = extender()->item("cdma");
    if (show && hasInterfaceOfType(Solid::Control::NetworkInterface::Cdma)) {
        if (!cdmaItem) {
            cdmaItem = new Plasma::ExtenderItem(extender());
            cdmaItem->setName("cdma");
            cdmaItem->setIcon("phone");
            cdmaItem->setTitle(i18nc("Label for mobile broadband (GSM/CDMA/UMTS/HDSPA etc)","Mobile Broadband"));
            initExtenderItem(cdmaItem);
        }
    } else {
        if (cdmaItem) {
            cdmaItem->destroy();
        }
    }
}

void NetworkManagerApplet::managerWirelessEnabledChanged(bool )
{
}

void NetworkManagerApplet::managerWirelessHardwareEnabledChanged(bool enabled)
{
    if (enabled) {
        KNotification::event(Event::RfOn, i18nc("Notification for radio kill switch turned on", "Wireless hardware enabled"), QPixmap(), 0, KNotification::CloseOnTimeout, KComponentData("networkmanagement", "networkmanagement", KComponentData::SkipMainComponentRegistration));
        //m_lblRfkill->setText(i18nc("Label text when hardware wireless is enabled", "Wireless hardware is enabled"));
    } else {
        //m_lblRfkill->setText(i18nc("Label text when hardware wireless is not enabled", "Wireless hardware is disabled"));
        KNotification::event(Event::RfOff, i18nc("Notification for radio kill switch turned on", "Wireless hardware disabled"), QPixmap(), 0, KNotification::CloseOnTimeout, KComponentData("networkmanagerment", "networkmanagement", KComponentData::SkipMainComponentRegistration));
    }
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

void NetworkManagerApplet::activateConnection(const QString& connection)
{
    kDebug() << connection;
}

void NetworkManagerApplet::deactivateConnection(const QString& connection)
{
    kDebug() << connection;
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
    foreach (Solid::Control::NetworkInterface * interface,
            m_interfaces) {

        if (interface->type() == type) {
            return true;
        }
    }
    return false;
}

void NetworkManagerApplet::hideVpnGroup()
{
    m_showVpn = false;
    KConfigGroup cg = config();
    cg.writeEntry("showVpn", m_showVpn);
    showVpn(false);
    Plasma::Applet::configNeedsSaving();
}
#include "networkmanager.moc"
