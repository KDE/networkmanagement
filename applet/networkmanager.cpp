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

K_EXPORT_PLASMA_APPLET(networkmanager, NetworkManagerApplet)

/* for qSort()ing */
bool networkInterfaceLessThan(Solid::Control::NetworkInterface * if1, Solid::Control::NetworkInterface * if2);
bool networkInterfaceSameConnectionStateLessThan(Solid::Control::NetworkInterface * if1, Solid::Control::NetworkInterface * if2);

NetworkManagerApplet::NetworkManagerApplet(QObject * parent, const QVariantList & args)
: Plasma::PopupApplet(parent, args), m_iconPerDevice(false), m_svg(0)
{
    m_extender = 0;
    m_wifiGroup = 0;
    m_wiredHeader = 0;
    m_ethernetGroup = 0;
    m_wirelessHeader = 0;
    m_ethernetGroup = 0;
    m_vpnHeader = 0;
    m_vpnGroup = 0;
    m_gsmHeader = 0;
    m_gsmGroup = 0;

    setHasConfigurationInterface(false);
    setPopupIcon(QIcon());
    //setPassivePopup(true); // only for testing ...

    updateToolTip();
    setAspectRatioMode(Plasma::ConstrainedSquare);
    setHasConfigurationInterface(true);
    m_svg = new Plasma::Svg(this);
    m_svg->setImagePath("networkmanager/networkmanager");

    m_wirelessSvg = new Plasma::Svg(this);
    m_wirelessSvg->setImagePath("networkmanager/networkmanager-wireless");

    m_interfaces = Solid::Control::NetworkManager::networkInterfaces();
    interfaceConnectionStateChanged();

    //m_popup = new NetworkManagerPopup(this);
    //m_popup->setExtender(extender());


    //QObject::connect(this, SIGNAL(manageConnections()),
    //        this, SLOT(manageConnections()));
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

    // Set up the extender with its various groups
    extender()->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    m_userSettings = new NetworkManagerSettings(QLatin1String(NM_DBUS_SERVICE_USER_SETTINGS), this);
    m_userSettings->setObjectName("user-settings-service");
    m_systemSettings = new NetworkManagerSettings(QLatin1String(NM_DBUS_SERVICE_SYSTEM_SETTINGS), this);
    m_systemSettings->setObjectName("system-settings-service");

    { // Wired
        Plasma::ExtenderItem *eItem = new Plasma::ExtenderItem(extender());
        eItem->setName("wired");
        eItem->setIcon("network-wired");
        eItem->setTitle(i18nc("Label for ethernet group in popup","Ethernet"));
        m_ethernetGroup = new InterfaceGroup(Solid::Control::NetworkInterface::Ieee8023, m_userSettings, m_systemSettings, eItem);
        m_ethernetGroup->setObjectName("ethernet-interface-group");
        m_ethernetGroup->init();
        eItem->setWidget(m_ethernetGroup);
    }

    { // Wireless

        Plasma::ExtenderItem *eItem = new Plasma::ExtenderItem(extender());
        eItem->setName("wireless");
        eItem->setIcon("network-wireless");
        eItem->setTitle(i18nc("Label for wifi networks in popup","Wireless"));

        m_wifiGroup = new InterfaceGroup(Solid::Control::NetworkInterface::Ieee80211, m_userSettings, m_systemSettings, eItem);
        m_wifiGroup->setObjectName("wifi-interface-group");
        m_wifiGroup->init();

        eItem->setWidget(m_wifiGroup);
    }

    { // GSM
        Plasma::ExtenderItem *eItem = new Plasma::ExtenderItem(extender());
        eItem->setName("gsm");
        eItem->setIcon("phone");
        eItem->setTitle(i18nc("Label for mobile wireless","Mobile Wireless"));
        m_gsmGroup = new InterfaceGroup(Solid::Control::NetworkInterface::Gsm, m_userSettings, m_systemSettings, this);
        m_gsmGroup->setObjectName("gsm-interface-group");
        m_gsmGroup->init();
        eItem->setWidget(m_gsmGroup);
    }

    { // VPN
        Plasma::ExtenderItem *eItem = new Plasma::ExtenderItem(extender());
        eItem->setName("vpn");
        eItem->setIcon("network-server");
        eItem->setTitle(i18nc("Label for vpn connections in popup","VPN Connections"));
        m_vpnGroup = new VpnConnectionGroup(m_userSettings, m_systemSettings, this);
        m_vpnGroup->setObjectName("vpn-interface-group");
        m_vpnGroup->init();
        eItem->setWidget(m_vpnGroup);
    }

    showWired(cg.readEntry("showWired", true));
    //showWired(true);
    showWireless(cg.readEntry("showWireless", true));
    showVpn(cg.readEntry("showVpn", false));
    showGsm(cg.readEntry("showGsm", false));
    m_numberOfWlans = cg.readEntry("numberOfWlans", 4);
    kDebug() << "WIRED???" << m_showWired << cg.readEntry("showWired", true);

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
    ui.showGsm->setChecked(m_showGsm);
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
    if (m_showGsm != ui.showGsm->isChecked()) {
        showGsm(!m_showGsm);
        cg.writeEntry("showGsm", m_showGsm);
        kDebug() << "Gsm Changed" << m_showGsm;
    }
    if (m_showVpn != ui.showVpn->isChecked()) {
        showVpn(!m_showVpn);
        cg.writeEntry("showVpn", m_showVpn);
        kDebug() << "VPN Changed" << m_showVpn;
    }
    int wlans = ui.numberOfWlans->value();
    if (wlans != m_numberOfWlans) {
        m_numberOfWlans = wlans;
        kDebug() << "No of WLANS Changed:" << wlans;
        // FIXME: Update something in the wifigroup / extender
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

void NetworkManagerApplet::showWired(bool show)
{
    m_showWired = show;
    kDebug() << show << m_showWired;
    Plasma::ExtenderItem *item = extender()->item("wired");
    if (item) {
        item->setVisible(show);
    }
}

void NetworkManagerApplet::showWireless(bool show)
{
    m_showWireless = show;
    kDebug() << show << m_showWireless;
    Plasma::ExtenderItem *item = extender()->item("wireless");
    if (item) {
        item->setVisible(show);
    }
}

void NetworkManagerApplet::showVpn(bool show)
{
    m_showVpn = show;
    kDebug() << show << m_showVpn;
    Plasma::ExtenderItem *item = extender()->item("vpn");
    if (item) {
        item->setVisible(show);
    }
}

void NetworkManagerApplet::showGsm(bool show)
{
    kDebug() << show << m_showGsm;
    Plasma::ExtenderItem *item = extender()->item("gsm");
    if (item) {
        item->setVisible(show);
    }
}

void NetworkManagerApplet::managerWirelessEnabledChanged(bool enabled)
{
    m_btnEnableWireless->setChecked(enabled);
    if (m_wifiGroup) {
        m_wifiGroup->enableInterface(enabled);
    }
}

void NetworkManagerApplet::managerWirelessHardwareEnabledChanged(bool enabled)
{
    if (enabled) {
        KNotification::event(Event::RfOn, i18nc("Notification for radio kill switch turned on", "Wireless hardware enabled"), QPixmap(), 0, KNotification::CloseOnTimeout, KComponentData("knetworkmanager", "knetworkmanager", KComponentData::SkipMainComponentRegistration));
        m_lblRfkill->setText(i18nc("Label text when hardware wireless is enabled", "Wireless hardware is enabled"));
    } else {
        m_lblRfkill->setText(i18nc("Label text when hardware wireless is not enabled", "Wireless hardware is disabled"));
        KNotification::event(Event::RfOff, i18nc("Notification for radio kill switch turned on", "Wireless hardware disabled"), QPixmap(), 0, KNotification::CloseOnTimeout, KComponentData("knetworkmanager", "knetworkmanager", KComponentData::SkipMainComponentRegistration));
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
        m_ethernetGroup->hide();
        //m_wifiGroup->hide();
        m_gsmGroup->hide();
        //m_notRunning->show();
    } else {
        m_ethernetGroup->show();
        //m_wifiGroup->show();
        m_gsmGroup->show();
        //m_notRunning->hide();
    }
}


#include "networkmanager.moc"
