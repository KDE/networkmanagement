/*
Copyright 2009 Will Stephenson <wstephenson@kde.org>

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

#include "knetworkmanagertrayicon.h"

#include <QPointer>
#include <QVBoxLayout>
#include <QWidgetAction>

#include <KAction>
#include <KDebug>
#include <KLocale>
#include <KMenu>
#include <KStandardAction>
#include <KToggleAction>
#include <KToolInvocation>

#include <solid/control/networkmanager.h>

#include <activatable.h>
#include <activatabledebug.h>
#include <sortedactivatablelist.h>
#include <interfaceconnection.h>
#include <unconfiguredinterface.h>
#include <vpninterfaceconnection.h>
#include <wirelessinterfaceconnection.h>
#include <wirelessnetwork.h>

#include "interfaceconnectionitem.h"
#include "vpninterfaceconnectionitem.h"
#include "wirelessinterfaceconnectionitem.h"
#include "unconfiguredinterfaceitem.h"
#include "wirelessnetworkitem.h"

Q_DECLARE_METATYPE(Knm::Activatable *)

class KNetworkManagerTrayIconPrivate
{
public:
    KNetworkManagerTrayIconPrivate()
        : list(0),
        iconName(QLatin1String("networkmanager")),
        flightModeAction(0),
        prefsAction(0),
        wirelessNetworkItemMenu(0), active(true)
    { }
    Solid::Control::NetworkInterface::Types interfaceTypes;
    SortedActivatableList * list;
    QHash<Knm::Activatable *, QWidgetAction *> actions;
    QStringList deviceUnis;
    QString iconName;
    KAction * flightModeAction;
    KAction * prefsAction;
    KMenu * wirelessNetworkItemMenu;
    bool active;
    // used for updating the tray icon, and indicates whether we are currently displaying a wireless
    // network interface's state in the tray
    QPointer<Solid::Control::NetworkInterface> displayedNetworkInterface;
    QPointer<Solid::Control::AccessPoint> activeAccessPoint;
};

/* for qSort()ing */
bool networkInterfaceLessThan(Solid::Control::NetworkInterface * if1, Solid::Control::NetworkInterface * if2);
bool networkInterfaceSameConnectionStateLessThan(Solid::Control::NetworkInterface * if1, Solid::Control::NetworkInterface * if2);

KNetworkManagerTrayIcon::KNetworkManagerTrayIcon(Solid::Control::NetworkInterface::Types types, const QString & id, SortedActivatableList * list, bool active, QObject * parent)
    : KNotificationItem(id, parent), d_ptr(new KNetworkManagerTrayIconPrivate)
{
    Q_D(KNetworkManagerTrayIcon);
    d->interfaceTypes = types;
    d->active = active;
    d->wirelessNetworkItemMenu = new KMenu(contextMenu());

    setStandardActionsEnabled(false);
    setCategory(Experimental::KNotificationItem::Hardware);
    setTitle(i18nc("@title:window KNotificationItem tray icon title", "Network Management"));
    setIconByName(d->iconName);

    setAssociatedWidget(contextMenu());
    setStatus( (!active || Solid::Control::NetworkManager::status() == Solid::Networking::Unknown )? KNotificationItem::Passive : KNotificationItem::Active);

    if (types.testFlag(Solid::Control::NetworkInterface::Ieee80211)) {
        d->flightModeAction = new KAction(i18nc("@action:inmenu turns off wireless networking", "Enable wireless"), this);
        d->flightModeAction->setCheckable(true);
        d->flightModeAction->setChecked(Solid::Control::NetworkManager::isWirelessEnabled());
        d->flightModeAction->setEnabled(Solid::Control::NetworkManager::isWirelessHardwareEnabled());
        connect(d->flightModeAction, SIGNAL(toggled(bool)), this, SLOT(enableWireless(bool)));
        connect(Solid::Control::NetworkManager::notifier(), SIGNAL(wirelessHardwareEnabledChanged(bool)),
                this, SLOT(wirelessEnabledChanged()));
        connect(Solid::Control::NetworkManager::notifier(), SIGNAL(wirelessEnabledChanged(bool)),
                this, SLOT(wirelessEnabledChanged()));
    }

    d->prefsAction = KStandardAction::preferences(this, SLOT(slotPreferences()), this);
    d->prefsAction->setText(i18nc("@action:inmenu Preferences action title", "Manage Connections..."));

    d->list = list;

    fillPopup();

    // hide the icon when network management is unavailable
    QObject::connect(Solid::Control::NetworkManager::notifier(),
                        SIGNAL(statusChanged(Solid::Networking::Status)),
                        this,
                        SLOT(networkingStatusChanged(Solid::Networking::Status)));

    // listen for new devices
    QObject::connect(Solid::Control::NetworkManager::notifier(),
            SIGNAL(networkInterfaceAdded(const QString&)),
            this,
            SLOT(networkInterfaceAdded(const QString&)));

    QObject::connect(Solid::Control::NetworkManager::notifier(),
            SIGNAL(networkInterfaceRemoved(const QString&)),
            this,
            SLOT(updateInterfaceToDisplay()));

    // listen to existing devices' state changes
    foreach (Solid::Control::NetworkInterface * iface,
            Solid::Control::NetworkManager::networkInterfaces()) {
        if (d->interfaceTypes.testFlag(iface->type())) {
            kDebug() << "connecting" << iface->interfaceName() << "'s signals";
            QObject::connect(iface, SIGNAL(connectionStateChanged(int,int,int)), this, SLOT(handleConnectionStateChange(int,int,int)));
        }
    }

    updateInterfaceToDisplay();
}

KNetworkManagerTrayIcon::~KNetworkManagerTrayIcon()
{
}

void KNetworkManagerTrayIcon::handleAdd(Knm::Activatable *)
{
    // FIXME could cache QWidgetActions here...
    fillPopup();
}

void KNetworkManagerTrayIcon::fillPopup()
{
    Q_D(KNetworkManagerTrayIcon);
    d->deviceUnis.clear();

    // build a list of wireless devices, so we can put its UnconfiguredInterface in the submenu
    QStringList wirelessDeviceUnis;
    foreach (Solid::Control::NetworkInterface * interface, Solid::Control::NetworkManager::networkInterfaces()) {
        if (interface->type() == Solid::Control::NetworkInterface::Ieee80211) {
            wirelessDeviceUnis.append(interface->uni());
        }
    }

    // clear the menu without deleting useful actions
    foreach (QAction * action, contextMenu()->actions()) {
        contextMenu()->removeAction(action);
        // throw away separators, easier than tracking them
        if (action->isSeparator()) {
            delete action;
        }
    }

    foreach (QAction * action, d->wirelessNetworkItemMenu->actions()) {
        d->wirelessNetworkItemMenu->removeAction(action);
    }

    QAction * insertionPointForWirelessNetworkSubmenu = 0;

    // counter to tell us how to label the wireless network item menu
    uint wirelessNetworkItemCount = 0;

    // list of wireless unconfigured interface items.  We put these in the main menu if there are
    // NO wirelessinterfaceconnections, otherwise at the bottom of the wireless network submenu
    QList<QWidgetAction *> wirelessUnconfiguredInterfaceItems;
    // counter of wirelessinterfaceconnections, to tell where to put the wireless unconfigured
    // interface items
    uint wirelessInterfaceConnectionCount = 0;

    // if not active only add a warning notice
    if (!d->active) {
        QAction * passiveAction = new QAction(i18nc("@action:inmenu Disable action text used when it is not possible to actively control networking", "Applet in passive mode"), this);
        passiveAction->setToolTip(i18nc("@info:tooltip when it is not possible to actively control networking", "Another applet is currently responsible for Network Management"));
        passiveAction->setEnabled(false);
        contextMenu()->addAction(passiveAction);
        return;
    }

    foreach (Knm::Activatable * activatable, d->list->activatables()) {
        QWidgetAction * action = 0;
        ActivatableItem * widget = 0;

        if (d->actions.contains(activatable)) {
            action = d->actions[activatable];
            widget = qobject_cast<ActivatableItem*>(action->defaultWidget());
        } else {
            action = new QWidgetAction(this);
            action->setData(QVariant::fromValue(activatable));
            if (activatable->activatableType() == Knm::Activatable::InterfaceConnection) {
                Knm::InterfaceConnection * ic = static_cast<Knm::InterfaceConnection*>(activatable);
                kDebug() << "IC" << ic->connectionName();
                widget = new InterfaceConnectionItem(ic, 0);
            } else if ( activatable->activatableType() == Knm::Activatable::WirelessInterfaceConnection) {
                Knm::WirelessInterfaceConnection * wic = static_cast<Knm::WirelessInterfaceConnection*>(activatable);
                kDebug() << "WIC" << wic->connectionName();
                widget = new WirelessInterfaceConnectionItem(wic, 0);
            } else if ( activatable->activatableType() == Knm::Activatable::WirelessNetwork) {
                Knm::WirelessNetwork * wni = static_cast<Knm::WirelessNetwork*>(activatable);
                kDebug() << "WNI" << wni->ssid();
                widget = new WirelessNetworkItem(wni, 0);
            } else if ( activatable->activatableType() == Knm::Activatable::UnconfiguredInterface) {
                Knm::UnconfiguredInterface * unco = static_cast<Knm::UnconfiguredInterface*>(activatable);
                kDebug() << "UCI" << unco->deviceUni();
                widget = new UnconfiguredInterfaceItem(unco, 0);
                widget->setObjectName(unco->deviceUni());
            } else if (activatable->activatableType() == Knm::Activatable::VpnInterfaceConnection) {
                Knm::VpnInterfaceConnection * vpn = static_cast<Knm::VpnInterfaceConnection*>(activatable);
                kDebug() << "VPN" << vpn->connectionName();
                widget = new VpnInterfaceConnectionItem(vpn, 0);
            }
            action->setDefaultWidget(widget);
            d->actions.insert(activatable, action);
        }

        if (action && widget) {
            // put all wireless network items and wireless unconfigured items into a submenu
            if (activatable->activatableType() == Knm::Activatable::WirelessNetwork) {

                wirelessNetworkItemCount++;

                d->wirelessNetworkItemMenu->addAction(action);

            } else if (wirelessDeviceUnis.contains(activatable->deviceUni()) && activatable->activatableType() == Knm::Activatable::UnconfiguredInterface) {
                // append to list of wireless unconfigured interfaces to be added later
                wirelessUnconfiguredInterfaceItems.append(action);
            } else {
                // If we have not seen any activatables for this device before, set its First flag for emphasis
                // Precede it with a separator if it is not the first action in the menu
                //
                // if there are wireless network items in the submenu, and we have reached a non-wireless (network or interfaceconnection)
                // item, mark the separator as the place to insert the submenu, as long as we have not yet done this
                if (d->deviceUnis.contains(activatable->deviceUni())) {
                    widget->setFirst(false);
                } else {
                    widget->setFirst(true);
                    if (!contextMenu()->actions().isEmpty()) {
                        QAction * sepAction = contextMenu()->addSeparator();

                        if (activatable->activatableType() != Knm::Activatable::WirelessInterfaceConnection) {
                            if (!d->wirelessNetworkItemMenu->actions().isEmpty() && !insertionPointForWirelessNetworkSubmenu ) {
                                insertionPointForWirelessNetworkSubmenu = sepAction;
                            }
                        }
                    }
                    d->deviceUnis.append(activatable->deviceUni());
                }
                if (activatable->activatableType() == Knm::Activatable::WirelessInterfaceConnection) {
                    wirelessInterfaceConnectionCount++;
                }

                contextMenu()->addAction(action);
            }
        }
    }
    // insert the wireless network items submenu at the right place
    if (wirelessNetworkItemCount) {
        d->wirelessNetworkItemMenu->setTitle(i18ncp("@title:menu Wireless network item menu title when additional networks are present", "%1 additional network", "%1 additional networks", wirelessNetworkItemCount));
        d->wirelessNetworkItemMenu->setEnabled(true);
    } else {
        d->wirelessNetworkItemMenu->setTitle(i18nc("@title:menu Wireless network item menu title when no networks found", "Other networks"));
    }

    if (d->interfaceTypes.testFlag(Solid::Control::NetworkInterface::Ieee80211)
            && Solid::Control::NetworkManager::isWirelessEnabled()
            && Solid::Control::NetworkManager::isWirelessHardwareEnabled()) {

        contextMenu()->insertAction(insertionPointForWirelessNetworkSubmenu, d->wirelessNetworkItemMenu->menuAction());

    }


    // insert the unconfigured wireless interface items at the right place in the right menu (see
    // above)
    QMenu * targetMenu = 0;
    QAction * insertionPoint = 0;
    if (wirelessInterfaceConnectionCount) {
        targetMenu = d->wirelessNetworkItemMenu;
    } else {
        targetMenu = contextMenu();
        insertionPoint = d->wirelessNetworkItemMenu->menuAction();
    }
    foreach (QWidgetAction * action, wirelessUnconfiguredInterfaceItems) {
        targetMenu->insertAction(insertionPoint, action);
    }

    // add the housekeeping actions
    contextMenu()->addSeparator();
    if (d->interfaceTypes.testFlag(Solid::Control::NetworkInterface::Ieee80211)) {
        contextMenu()->addAction(d->flightModeAction);
    }
    contextMenu()->addAction(d->prefsAction);
}

void KNetworkManagerTrayIcon::handleUpdate(Knm::Activatable *)
{
    fillPopup();
}

void KNetworkManagerTrayIcon::handleRemove(Knm::Activatable * removed)
{
    Q_D(KNetworkManagerTrayIcon);
    QWidgetAction * removedAction = d->actions.take(removed);
    delete removedAction;
}

void KNetworkManagerTrayIcon::slotPreferences()
{
    QStringList args;
    args << "kcm_networkmanagement";
    KToolInvocation::kdeinitExec("kcmshell4", args);
}

void KNetworkManagerTrayIcon::networkInterfaceAdded(const QString & uni)
{
    Q_D(KNetworkManagerTrayIcon);
    Solid::Control::NetworkInterface * iface = Solid::Control::NetworkManager::findNetworkInterface(uni);
    if (iface) {
        if (d->interfaceTypes.testFlag(iface->type())) {
            kDebug() << "connecting" << iface->interfaceName() << "'s signals";
            QObject::connect(iface, SIGNAL(connectionStateChanged(int,int,int)), this, SLOT(handleConnectionStateChange(int,int,int)));
        }
    }
    //update our state
    updateInterfaceToDisplay();
}

void KNetworkManagerTrayIcon::handleConnectionStateChange(int new_state, int old_state, int reason)
{
    Solid::Control::NetworkInterface * iface = qobject_cast<Solid::Control::NetworkInterface*>(sender());
    if (iface) {
        kDebug() << iface->interfaceName() << "has changed state from" << old_state << "to" << new_state << "because of reason" << reason;
    }
    updateInterfaceToDisplay();
}

void KNetworkManagerTrayIcon::updateInterfaceToDisplay()
{
    Q_D(KNetworkManagerTrayIcon);
    kDebug();
    // update appearance
    // get current list of interfaces we are displaying
    Solid::Control::NetworkInterfaceList interfaces = Solid::Control::NetworkManager::networkInterfaces();
    QMutableListIterator<Solid::Control::NetworkInterface *> it(interfaces);
    while (it.hasNext()) {
        Solid::Control::NetworkInterface * iface = it.next();
        if (!d->interfaceTypes.testFlag(iface->type())) {
            it.remove();
        }
    }

    kDebug() << "interfaces considered:";
    foreach (Solid::Control::NetworkInterface * iface, interfaces) {
        kDebug() << iface << iface->type() << iface->connectionState();

    }

    // get the icon name for the state of the most interesting interface
    if (!interfaces.isEmpty()) {
        qSort(interfaces.begin(), interfaces.end(), networkInterfaceLessThan);

        Solid::Control::NetworkInterface * interface = interfaces.first();
        Solid::Control::WirelessNetworkInterface * wirelessIface = qobject_cast<Solid::Control::WirelessNetworkInterface*>(interface);

        // disconnect the prior displayed interface's signal strength signals
        if (!d->displayedNetworkInterface.isNull()) {
            Solid::Control::WirelessNetworkInterface * displayedWirelessIface
                = qobject_cast<Solid::Control::WirelessNetworkInterface*>(d->displayedNetworkInterface.data());

            QObject::disconnect(displayedWirelessIface,
                    SIGNAL(activeAccessPointChanged(const QString &)),
                    this,
                    SLOT(activeAccessPointChanged(const QString &)));
        }
        if (!d->activeAccessPoint.isNull()) {
            QObject::disconnect(d->activeAccessPoint.data(), 0, this, 0);
        }

        // set the new displayed interface
        kDebug() << "interface to display:" << interface;
        d->displayedNetworkInterface = interface;

        // if wireless listen to its signal strength signals
        if (wirelessIface) {
            QObject::connect(wirelessIface,
                    SIGNAL(activeAccessPointChanged(const QString &)),
                    this,
                    SLOT(activeAccessPointChanged(const QString &)));

            activeAccessPointChanged(wirelessIface->activeAccessPoint());
        } else {
            d->activeAccessPoint = 0;
        }
    }
    updateTrayIcon();
    updateToolTip();
}

void KNetworkManagerTrayIcon::updateTrayIcon()
{
    Q_D(KNetworkManagerTrayIcon);

    QString iconName, overlayName;

    if (!d->displayedNetworkInterface.isNull()) {

        switch (d->displayedNetworkInterface->type() ) {
            case Solid::Control::NetworkInterface::Ieee8023:
                iconName = QLatin1String("network-wired");
                break;
            case Solid::Control::NetworkInterface::Ieee80211:
                if (!d->activeAccessPoint.isNull()) {
                    int strength = d->activeAccessPoint->signalStrength();
                    if ( strength > 80 )
                        iconName = QLatin1String("network-wireless-100");
                    else if ( strength > 55 )
                        iconName = QLatin1String("network-wireless-75");
                    else if ( strength > 30 )
                        iconName = QLatin1String("network-wireless-50");
                    else if ( strength > 5 )
                        iconName = QLatin1String("network-wireless-25");
                    else
                        iconName = QLatin1String("network-wireless-0");
                } else {
                    iconName = QLatin1String("network-wireless");
                }
                break;
            case Solid::Control::NetworkInterface::Serial:
                iconName = QLatin1String("modem");
                break;
            case Solid::Control::NetworkInterface::Gsm:
            case Solid::Control::NetworkInterface::Cdma:
                //TODO make this work for cellular signal strength
                iconName = "phone";
                break;
            default:
                iconName = "network-wired";
                break;
        }

        switch (d->displayedNetworkInterface->connectionState()) {
            case Solid::Control::NetworkInterface::Preparing:
                overlayName = QLatin1String("busy-phase1");
                overlayName = QLatin1String("emblem-mounted");
                break;
            case Solid::Control::NetworkInterface::Configuring:
                overlayName = QLatin1String("busy-phase2");
                overlayName = QLatin1String("emblem-mounted");
                break;
            case Solid::Control::NetworkInterface::NeedAuth:
                overlayName = QLatin1String("busy-phase2");
                overlayName = QLatin1String("emblem-mounted");
                break;
            case Solid::Control::NetworkInterface::IPConfig:
                overlayName = QLatin1String("busy-phase3");
                overlayName = QLatin1String("emblem-mounted");
                break;
            case Solid::Control::NetworkInterface::Activated:
                overlayName = QLatin1String("checkbox");
                break;
            default:
                break;
        }
    } else {
        iconName = "networkmanager";
    }

    foreach (Knm::Activatable * activatable, d->list->activatables()) {
        if (activatable->activatableType() == Knm::Activatable::VpnInterfaceConnection) {
            Knm::InterfaceConnection * ic = static_cast<Knm::InterfaceConnection*>(activatable);
            if (ic->activationState() == Knm::InterfaceConnection::Activated) {
                overlayName = QLatin1String("nm-vpn-lock"); //TODO real icon name
            }
        }
    }

    kDebug() << "suggested icon name:" << iconName;

    if (iconName != d->iconName) {
        d->iconName = iconName;
        kDebug() << "setting icon:" << iconName;
        setIconByName(d->iconName);
    }
    kDebug() << "setting overlay:" << overlayName;
    setOverlayIconByName(overlayName);
}

void KNetworkManagerTrayIcon::updateToolTip()
{
    Q_D(KNetworkManagerTrayIcon);
    QString tip;
    if (d->displayedNetworkInterface) {
        switch (d->displayedNetworkInterface->connectionState()) {
            case Solid::Control::NetworkInterface::UnknownState:
                tip = i18nc("@info:tooltip status string for network interface in unknown state state ", "Unknown");
                break;
            case Solid::Control::NetworkInterface::Unmanaged:
                tip = i18nc("@info:tooltip status string for network interface not managed by networking subsystem", "Not managed");
                break;
            case Solid::Control::NetworkInterface::Unavailable:
                tip = i18nc("@info:tooltip status string for network interface not available for use", "Unavailable");
                break;
            case Solid::Control::NetworkInterface::Disconnected:
                tip = i18nc("@info:tooltip status string for disconnected network interface", "Not connected");
                break;
            case Solid::Control::NetworkInterface::Preparing:
                tip = i18nc("@info:tooltip status string for network interface preparing to connect", "Preparing to connect");
                break;
            case Solid::Control::NetworkInterface::Configuring:
                tip = i18nc("@info:tooltip status string for network interface being configured prior to connection", "Configuring interface");
                break;
            case Solid::Control::NetworkInterface::NeedAuth:
                tip = i18nc("@info:tooltip status string for network interface awaiting authorizatoin", "Waiting for authorization");
                break;
            case Solid::Control::NetworkInterface::IPConfig:
                tip = i18nc("@info:tooltip status string for network interface getting IP address", "Obtaining network address");
                break;
            case Solid::Control::NetworkInterface::Activated:
                tip = i18nc("@info:tooltip status string for active network interface", "Active");
                break;
            case Solid::Control::NetworkInterface::Failed:
                tip = i18nc("@info:tooltip status string for network interface with failed connection", "Connection failed");
                break;
        }
    } else {
        tip = "<qt>Networking <b>information</b> not available</qt>";
    }
    setToolTip(d->iconName, tip, QString());
}

void KNetworkManagerTrayIcon::networkingStatusChanged(Solid::Networking::Status status)
{
    if (status == Solid::Networking::Unknown) {
        setStatus(Experimental::KNotificationItem::Passive);
    } else {
        setStatus(Experimental::KNotificationItem::Active);
    }
}

void KNetworkManagerTrayIcon::enableWireless(bool enabled)
{
    kDebug() << enabled;
    Solid::Control::NetworkManager::setWirelessEnabled(enabled);
}

void KNetworkManagerTrayIcon::wirelessEnabledChanged()
{
    Q_D(KNetworkManagerTrayIcon);
    d->flightModeAction->setEnabled(Solid::Control::NetworkManager::isWirelessHardwareEnabled());

    d->flightModeAction->setChecked(Solid::Control::NetworkManager::isWirelessEnabled());
    if (!(Solid::Control::NetworkManager::isWirelessHardwareEnabled() && Solid::Control::NetworkManager::isWirelessEnabled())) {
        contextMenu()->removeAction(d->wirelessNetworkItemMenu->menuAction());
    }
}

void KNetworkManagerTrayIcon::setActive(bool active)
{
    Q_D(KNetworkManagerTrayIcon);
    d->active = active;
    setStatus( active ? KNotificationItem::Active : KNotificationItem::Passive);
    fillPopup();
}

void KNetworkManagerTrayIcon::activeAccessPointChanged(const QString & uni)
{
    kDebug();
    Q_D(KNetworkManagerTrayIcon);

    if (!d->displayedNetworkInterface.isNull()) {
        Solid::Control::WirelessNetworkInterface * wirelessIface
            = qobject_cast<Solid::Control::WirelessNetworkInterface*>(
                    d->displayedNetworkInterface.data());
        if (wirelessIface) {
            d->activeAccessPoint = wirelessIface->findAccessPoint(uni);

            if (!d->activeAccessPoint.isNull()) {
                QObject::connect(d->activeAccessPoint.data(), SIGNAL(signalStrengthChanged(int)),
                        this, SLOT(updateTrayIcon()));
            }
        }
    }
    updateTrayIcon();
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

// vim: sw=4 sts=4 et tw=100
