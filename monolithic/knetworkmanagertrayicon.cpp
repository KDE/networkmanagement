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

#include <QApplication>
#include <QClipboard>
#include <QHostAddress>
#include <QPointer>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidgetAction>

#include <KAction>
#include <KDebug>
#include <KLocale>
#include <KMenu>
#include <KStandardAction>
#include <KToggleAction>
#include <KToolInvocation>

#include <libnm-qt/manager.h>
#include <libnm-qt/modemdevice.h>

#include <connectioninfodialog.h>

#include <activatable.h>
#include <activatabledebug.h>
#include <sortedactivatablelist.h>
#include <interfaceconnection.h>
#include <uiutils.h>
#include <unconfiguredinterface.h>
#include <vpninterfaceconnection.h>
#include <wirelessinterfaceconnection.h>
#include <wirelessnetwork.h>

#include "interfaceconnectionitem.h"
#include "otherwirelessnetworkdialog.h"
#include "vpninterfaceconnectionitem.h"
#include "wirelessinterfaceconnectionitem.h"
#include "unconfiguredinterfaceitem.h"
#include "wirelessnetworkitem.h"


class KNetworkManagerTrayIconPrivate
{
public:
    KNetworkManagerTrayIconPrivate()
        : list(0),
        iconName(QLatin1String("networkmanager")),
        networkingEnableAction(0),
        flightModeAction(0),
        prefsAction(0),
        copyIpAddrAction(0),
        propertiesAction(0),
        active(true)
    { }
    NetworkManager::Device::Types interfaceTypes;
    SortedActivatableList * list;
    QHash<Knm::Activatable *, QWidgetAction *> actions;
    QStringList deviceUnis;
    QString iconName;
    KAction * networkingEnableAction;
    KAction * flightModeAction;
    KAction * prefsAction;
    KAction * copyIpAddrAction;
    KAction * propertiesAction;
    KAction * otherWirelessNetworksAction;
    OtherWirelessNetworkDialog * otherWirelessNetworksDialog;
    bool active;
    // used for updating the tray icon, and indicates whether we are currently displaying a wireless
    // network interface's state in the tray
    QPointer<NetworkManager::Device> displayedNetworkInterface;
    QPointer<NetworkManager::AccessPoint> activeAccessPoint;
    QHostAddress hoveredActionIpAddress;
    QHash<Knm::Activatable *, QPointer<ConnectionInfoDialog> > connectionPropertiesDialogs;
    Knm::InterfaceConnection *hoveredActionInterfaceConnection;
};

/* for qSort()ing */
bool networkInterfaceLessThan(NetworkManager::Device * if1, NetworkManager::Device * if2);
bool networkInterfaceSameConnectionStateLessThan(NetworkManager::Device * if1, NetworkManager::Device * if2);

KNetworkManagerTrayIcon::KNetworkManagerTrayIcon(NetworkManager::Device::Types types, const QString & id, SortedActivatableList * list, bool serviceAvailable, QObject * parent)
    : PARENT_ICON_CLASS(id, parent), d_ptr(new KNetworkManagerTrayIconPrivate)
{
    Q_D(KNetworkManagerTrayIcon);
    d->interfaceTypes = types;
    d->active = serviceAvailable;

    setStandardActionsEnabled(false);
    setCategory(PARENT_ICON_CLASS::Hardware);
    setTitle(i18nc("@title:window KNotificationItem tray icon title", "Network Management"));
    setIconByName(d->iconName);

    setAssociatedWidget(contextMenu());
    setStatus( (!d->active || NetworkManager::status() == Solid::Networking::Unknown )? PARENT_ICON_CLASS::Passive : PARENT_ICON_CLASS::Active);

    d->networkingEnableAction = new KAction(i18nc("@action:inmenu turns off networking", "Enable networking"), this);
    d->networkingEnableAction->setCheckable(true);
    d->networkingEnableAction->setChecked(NetworkManager::isNetworkingEnabled());
    connect(d->networkingEnableAction, SIGNAL(toggled(bool)), this, SLOT(enableNetworking(bool)));
    connect(NetworkManager::notifier(), SIGNAL(networkingEnabledChanged(bool)),
            this, SLOT(networkingEnabledChanged()));

    if (types.testFlag(NetworkManager::Device::Wifi)) {
        d->flightModeAction = new KAction(i18nc("@action:inmenu turns off wireless networking", "Enable wireless"), this);
        d->flightModeAction->setCheckable(true);
        d->flightModeAction->setChecked(NetworkManager::isWirelessEnabled());
        d->flightModeAction->setEnabled(NetworkManager::isWirelessHardwareEnabled());
        connect(d->flightModeAction, SIGNAL(toggled(bool)), this, SLOT(enableWireless(bool)));
        connect(NetworkManager::notifier(), SIGNAL(wirelessHardwareEnabledChanged(bool)),
                this, SLOT(wirelessEnabledChanged()));
        connect(NetworkManager::notifier(), SIGNAL(wirelessEnabledChanged(bool)),
                this, SLOT(wirelessEnabledChanged()));
    }

    d->prefsAction = KStandardAction::preferences(this, SLOT(slotPreferences()), this);
    d->prefsAction->setText(i18nc("@action:inmenu Preferences action title", "Manage Connections..."));
    d->otherWirelessNetworksDialog = new OtherWirelessNetworkDialog(list, contextMenu());
    d->otherWirelessNetworksDialog->resize(370, 390);
    list->registerObserver(d->otherWirelessNetworksDialog);

    d->list = list;

    d->otherWirelessNetworksAction = new KAction(i18nc("@action:inmenu show dialog to connect to new or hidden network", "Connect To &Other Network..."), this);

    connect (d->otherWirelessNetworksAction, SIGNAL(triggered()), this, SLOT(showOtherWirelessDialog()));

    fillPopup();

    // hide the icon when network management is unavailable
    QObject::connect(NetworkManager::notifier(),
                        SIGNAL(statusChanged(Solid::Networking::Status)),
                        this,
                        SLOT(networkingStatusChanged(Solid::Networking::Status)));

    // listen for new devices
    QObject::connect(NetworkManager::notifier(),
            SIGNAL(networkInterfaceAdded(const QString&)),
            this,
            SLOT(networkInterfaceAdded(const QString&)));

    QObject::connect(NetworkManager::notifier(),
            SIGNAL(networkInterfaceRemoved(const QString&)),
            this,
            SLOT(updateInterfaceToDisplay()));

    // listen to existing devices' state changes
    foreach (NetworkManager::Device * iface,
            NetworkManager::networkInterfaces()) {
        if (d->interfaceTypes.testFlag(iface->type())) {
            kDebug() << "connecting" << iface->interfaceName() << "'s signals";
            QObject::connect(iface, SIGNAL(stateChanged(int,int,int)), this, SLOT(handleConnectionStateChange(int,int,int)));
        }
    }

    // show sub-context menu on connection actions
    contextMenu()->contextMenu();
    connect( contextMenu(), SIGNAL(aboutToShowContextMenu(KMenu*,QAction*,QMenu*)), this, SLOT(aboutToShowMenuContextMenu(KMenu*,QAction*,QMenu*)));

    updateInterfaceToDisplay();
}

KNetworkManagerTrayIcon::~KNetworkManagerTrayIcon()
{
}

void KNetworkManagerTrayIcon::handleAdd(Knm::Activatable *)
{
    fillPopup();
}

void KNetworkManagerTrayIcon::fillPopup()
{
    Q_D(KNetworkManagerTrayIcon);
    d->deviceUnis.clear();

    // build a list of wireless devices, so we can put its UnconfiguredInterface in the submenu
    QStringList wirelessDeviceUnis;
    foreach (NetworkManager::Device * interface, NetworkManager::networkInterfaces()) {
        if (interface->type() == NetworkManager::Device::Wifi) {
            wirelessDeviceUnis.append(interface->uni());
        }
    }

    // clear the menu without deleting useful actions
    foreach (QAction * action, contextMenu()->actions()) {
        contextMenu()->removeAction(action);
        // throw away separators, easier than tracking them
        if (action->isSeparator()) {
            action->deleteLater();
        }
    }

    // if not active, add a warning notice, show Networking and Wireless check buttons and stop
    if (NetworkManager::status() == Solid::Networking::Unknown) {
        QString passiveText = i18nc("@action:inmenu Disable action text used when the NetworkManager daemon is not running", "Network Management disabled");
        QString passiveTooltip = i18nc("@info:tooltip NetworkManager is not running, this client cannot do anything", "The system Network Management service is not running");

        QAction * passiveAction = new QAction(passiveText, this);
        passiveAction->setToolTip(passiveTooltip);
        passiveAction->setEnabled(false);
        contextMenu()->addAction(passiveAction);

        // show "Enable" check buttons
        contextMenu()->addAction(d->networkingEnableAction);
        if (!wirelessDeviceUnis.isEmpty() /*TODO Bluetooth too */ && d->interfaceTypes.testFlag(NetworkManager::Device::Wifi)) {
            contextMenu()->addAction(d->flightModeAction);
        }
        return;
    }

    // fill the menu
    QAction * insertionPointForConnectToOtherWireless = 0;

    // list of wireless unconfigured interface items.  We put these in the main menu if there are
    // NO wirelessinterfaceconnections, otherwise at the bottom of the wireless network submenu
    QList<QWidgetAction *> wirelessUnconfiguredInterfaceItems;
    // counter of wirelessinterfaceconnections, to tell where to put the wireless unconfigured
    // interface items
    uint wirelessInterfaceConnectionCount = 0;

    foreach (Knm::Activatable * activatable, d->list->activatables()) {
        QWidgetAction * action = 0;
        ActivatableItem * widget = 0;

        if (d->actions.contains(activatable)) {
            action = d->actions[activatable];
            widget = qobject_cast<ActivatableItem*>(action->defaultWidget());
        } else {
            action = new QWidgetAction(this);
            if (activatable->activatableType() == Knm::Activatable::InterfaceConnection) {
                Knm::InterfaceConnection * ic = static_cast<Knm::InterfaceConnection*>(activatable);
                kDebug() << "IC" << ic->connectionName();
                widget = new InterfaceConnectionItem(ic, 0);
            } else if ( activatable->activatableType() == Knm::Activatable::WirelessInterfaceConnection) {
                Knm::WirelessInterfaceConnection * wic = static_cast<Knm::WirelessInterfaceConnection*>(activatable);
                kDebug() << "WIC" << wic->connectionName();
                widget = new WirelessInterfaceConnectionItem(wic, 0);
                if (wic->operationMode() == NetworkManager::WirelessDevice::Adhoc)
                {
                    widget->setVisible(false);
                    connect(wic,SIGNAL(activated()),this,SLOT(adHocActivated()));
                    connect(wic,SIGNAL(deactivated()),this,SLOT(adHocDeactivated()));
                    connect (this, SIGNAL(adhocActivated(bool)), action, SLOT(setVisible(bool)));
                    connect (this, SIGNAL(adhocDeactivated(bool)), action, SLOT(setVisible(bool)));
                }
            } else if ( activatable->activatableType() == Knm::Activatable::UnconfiguredInterface) {
                Knm::UnconfiguredInterface * unco = static_cast<Knm::UnconfiguredInterface*>(activatable);
                kDebug() << "UCI" << unco->deviceUni();
                widget = new UnconfiguredInterfaceItem(unco, 0);
                widget->setObjectName(unco->deviceUni());
                NetworkManager::Device * iface
                    = NetworkManager::findNetworkInterface(unco->deviceUni());
                if (iface) {
                    if (iface->type() == NetworkManager::Device::Wifi) {
                        connect (widget, SIGNAL(clicked()), this, SLOT(showOtherWirelessDialog()));
                    }
                }
            } else if (activatable->activatableType() == Knm::Activatable::VpnInterfaceConnection) {
                Knm::VpnInterfaceConnection * vpn = static_cast<Knm::VpnInterfaceConnection*>(activatable);
                kDebug() << "VPN" << vpn->connectionName();
                widget = new VpnInterfaceConnectionItem(vpn, 0);
            }
            action->setDefaultWidget(widget);
            d->actions.insert(activatable, action);
        }

        if (action && widget) {
            if (wirelessDeviceUnis.contains(activatable->deviceUni()) && activatable->activatableType() == Knm::Activatable::UnconfiguredInterface) {
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
                            if (!insertionPointForConnectToOtherWireless ) {
                                insertionPointForConnectToOtherWireless = sepAction;
                            }
                        }
                    }
                    d->deviceUnis.append(activatable->deviceUni());
                }
                if (activatable->activatableType() == Knm::Activatable::WirelessInterfaceConnection && static_cast<Knm::WirelessInterfaceConnection*>(activatable)->operationMode() != NetworkManager::WirelessDevice::Adhoc) {
                    wirelessInterfaceConnectionCount++;
                }

                contextMenu()->addAction(action);
            }
        }
    }

    if (!wirelessInterfaceConnectionCount && NetworkManager::isWirelessEnabled()
            && NetworkManager::isWirelessHardwareEnabled()) {
        foreach (QWidgetAction * action, wirelessUnconfiguredInterfaceItems) {
            contextMenu()->insertSeparator(insertionPointForConnectToOtherWireless);
            contextMenu()->insertAction(insertionPointForConnectToOtherWireless, action);
        }
    }

    if (d->interfaceTypes.testFlag(NetworkManager::Device::Wifi)
            && NetworkManager::isWirelessEnabled()
            && NetworkManager::isWirelessHardwareEnabled() && wirelessInterfaceConnectionCount) {
        contextMenu()->insertAction(insertionPointForConnectToOtherWireless, d->otherWirelessNetworksAction);
    }

    // add the housekeeping actions
    contextMenu()->addSeparator();

    contextMenu()->addAction(d->networkingEnableAction);
    if (!wirelessDeviceUnis.isEmpty() /*TODO Bluetooth too */ && d->interfaceTypes.testFlag(NetworkManager::Device::Wifi)) {
        contextMenu()->addAction(d->flightModeAction);
    }
    contextMenu()->addSeparator();
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

void KNetworkManagerTrayIcon::adHocActivated()
{
    emit adhocActivated(true);
}

void KNetworkManagerTrayIcon::adHocDeactivated()
{
    emit adhocDeactivated(false);
}

void KNetworkManagerTrayIcon::slotPreferences()
{
    QStringList args;
    args << "--icon" << "networkmanager" << "kcm_networkmanagement" << "kcm_networkmanagement_tray";
    KToolInvocation::kdeinitExec("kcmshell4", args);
}

void KNetworkManagerTrayIcon::networkInterfaceAdded(const QString & uni)
{
    Q_D(KNetworkManagerTrayIcon);
    NetworkManager::Device * iface = NetworkManager::findNetworkInterface(uni);
    if (iface) {
        if (d->interfaceTypes.testFlag(iface->type())) {
            kDebug() << "connecting" << iface->interfaceName() << "'s signals";
            QObject::connect(iface, SIGNAL(stateChanged(int,int,int)), this, SLOT(handleConnectionStateChange(int,int,int)));
        }
    }
    //update our state
    updateInterfaceToDisplay();
}

void KNetworkManagerTrayIcon::handleConnectionStateChange(int new_state, int old_state, int reason)
{
    NetworkManager::Device * iface = qobject_cast<NetworkManager::Device*>(sender());
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
    NetworkManager::DeviceList interfaces = NetworkManager::networkInterfaces();
    QMutableListIterator<NetworkManager::Device *> it(interfaces);
    while (it.hasNext()) {
        NetworkManager::Device * iface = it.next();
        if (!d->interfaceTypes.testFlag(iface->type())) {
            it.remove();
        }
    }

    kDebug() << "interfaces considered:";
    foreach (NetworkManager::Device * iface, interfaces) {
        kDebug() << iface << iface->type() << iface->state();

    }

    // get the icon name for the state of the most interesting interface
    if (!interfaces.isEmpty()) {
        qSort(interfaces.begin(), interfaces.end(), networkInterfaceLessThan);

        NetworkManager::Device * interface = interfaces.first();
        NetworkManager::WirelessDevice * wirelessIface = qobject_cast<NetworkManager::WirelessDevice*>(interface);

        // disconnect the prior displayed interface's signal strength signals
        if (!d->displayedNetworkInterface.isNull()) {
            NetworkManager::WirelessDevice * displayedWirelessIface
                = qobject_cast<NetworkManager::WirelessDevice*>(d->displayedNetworkInterface.data());

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
        NetworkManager::ModemDevice * modemDev = 0;
        switch (d->displayedNetworkInterface->type() ) {
            case NetworkManager::Device::Ethernet:
                iconName = QLatin1String("network-wired");
                break;
            case NetworkManager::Device::Wifi:
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
            case NetworkManager::Device::Modem:
                modemDev = qobject_cast<NetworkManager::ModemDevice *>(d->displayedNetworkInterface);
                if (modemDev) {
                    if (modemDev->currentCapabilities() & NetworkManager::ModemDevice::CdmaEvdo || modemDev->currentCapabilities() & NetworkManager::ModemDevice::GsmUmts)
                        iconName = QLatin1String("phone");
                    else
                        iconName = QLatin1String("modem");
                }
                break;
            default:
                iconName = "network-wired";
                break;
        }

        switch (d->displayedNetworkInterface->state()) {
            case NetworkManager::Device::Preparing:
                overlayName = QLatin1String("busy-phase1");
                overlayName = QLatin1String("emblem-link");
                break;
            case NetworkManager::Device::ConfiguringHardware:
                overlayName = QLatin1String("busy-phase2");
                overlayName = QLatin1String("emblem-link");
                break;
            case NetworkManager::Device::NeedAuth:
                overlayName = QLatin1String("busy-phase2");
                overlayName = QLatin1String("emblem-link");
                break;
            case NetworkManager::Device::ConfiguringIp:
                overlayName = QLatin1String("busy-phase3");
                overlayName = QLatin1String("emblem-link");
                break;
            case NetworkManager::Device::Activated:
                if (iconName == QLatin1String("network-wired"))
                    iconName = QLatin1String("network-wired-activated");
                else

                    overlayName = QLatin1String("emblem-mounted");
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


    if (iconName != d->iconName) {
        d->iconName = iconName;
        setIconByName(d->iconName);
    }
    setOverlayIconByName(overlayName);
}

void KNetworkManagerTrayIcon::updateToolTip()
{
    Q_D(KNetworkManagerTrayIcon);
    QString tip;
    if (d->displayedNetworkInterface) {
        tip = UiUtils::connectionStateToString(static_cast<NetworkManager::Device::State>(d->displayedNetworkInterface->state()));
    } else {
        tip = "<qt>Networking <b>information</b> not available</qt>";
    }
    setToolTip(d->iconName, tip, QString());
}

void KNetworkManagerTrayIcon::networkingStatusChanged(Solid::Networking::Status status)
{
    if (status == Solid::Networking::Unknown) {
        setStatus(PARENT_ICON_CLASS::Passive);
        fillPopup();
    } else {
        setStatus(PARENT_ICON_CLASS::Active);
    }
}

void KNetworkManagerTrayIcon::enableNetworking(bool enabled)
{
    kDebug() << enabled;
    NetworkManager::setNetworkingEnabled(enabled);
}

void KNetworkManagerTrayIcon::enableWireless(bool enabled)
{
    kDebug() << enabled;
    NetworkManager::setWirelessEnabled(enabled);
}

void KNetworkManagerTrayIcon::wirelessEnabledChanged()
{
    Q_D(KNetworkManagerTrayIcon);
    d->flightModeAction->setEnabled(NetworkManager::isWirelessHardwareEnabled());

    d->flightModeAction->setChecked(NetworkManager::isWirelessEnabled());
    fillPopup();
}

void KNetworkManagerTrayIcon::networkingEnabledChanged()
{
    Q_D(KNetworkManagerTrayIcon);
    d->networkingEnableAction->setChecked(NetworkManager::isNetworkingEnabled());
}

void KNetworkManagerTrayIcon::setActive(bool active)
{
    Q_D(KNetworkManagerTrayIcon);
    d->active = active;
    fillPopup();
}

void KNetworkManagerTrayIcon::activeAccessPointChanged(const QString & uni)
{
    kDebug();
    Q_D(KNetworkManagerTrayIcon);

    if (!d->displayedNetworkInterface.isNull()) {
        NetworkManager::WirelessDevice * wirelessIface
            = qobject_cast<NetworkManager::WirelessDevice*>(
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

void KNetworkManagerTrayIcon::aboutToShowMenuContextMenu(KMenu * menu, QAction * action, QMenu * ctxMenu)
{
    Q_D(KNetworkManagerTrayIcon);
    kDebug() << menu <<  action << ctxMenu;
    if (menu == contextMenu()) {
        QWidgetAction * widgetAction = qobject_cast<QWidgetAction*>(action);
        if (widgetAction) {
            InterfaceConnectionItem * ici = qobject_cast<InterfaceConnectionItem*>(widgetAction->defaultWidget());
            if (ici) {
                Knm::InterfaceConnection * ic = ici->interfaceConnection();
                if ( ic ) {
                    if (!d->copyIpAddrAction) {
                        d->copyIpAddrAction = KStandardAction::copy(this, SLOT(copyIpAddress()), this);
                        d->copyIpAddrAction->setText(i18nc("@action:inmenu copy ip address to clipboard", "Copy IP Address"));
                        d->propertiesAction = new KAction(i18nc("@action:inmenu interface connection properties", "Properties"), this);
                        connect(d->propertiesAction, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)), this, SLOT(showConnectionProperties()));
                    }
                    d->hoveredActionInterfaceConnection = ic;
                    QString deviceUni = ic->deviceUni();
                    NetworkManager::Device * iface = NetworkManager::findNetworkInterface(deviceUni);
                    if (ic->activationState() == Knm::InterfaceConnection::Activated) {
                        QHostAddress addr;
                        if (iface) {
                            NetworkManager::IPv4Config cfg = iface->ipV4Config();
                            if (!cfg.addresses().isEmpty()) {
                                addr = QHostAddress(cfg.addresses().first().address());
                            }
                        }
                        d->hoveredActionIpAddress = addr;
                        d->copyIpAddrAction->setEnabled(true);
                    } else {
                        d->hoveredActionIpAddress = QHostAddress();
                        d->copyIpAddrAction->setEnabled(false);
                    }
                    ctxMenu->addAction(d->copyIpAddrAction);
                    ctxMenu->addAction(d->propertiesAction);
                } else {
                    kDebug() << "InterfaceconnectionItem did not have an InterfaceConnection";
                }
            } else {
                ctxMenu->removeAction(d->copyIpAddrAction);
                QTimer::singleShot(0, ctxMenu, SLOT(hide()));

            }
        } else {
            ctxMenu->removeAction(d->copyIpAddrAction);
            QTimer::singleShot(0, ctxMenu, SLOT(hide()));
        }
    }
}

void KNetworkManagerTrayIcon::copyIpAddress()
{
    Q_D(KNetworkManagerTrayIcon);
    QClipboard * clip = QApplication::clipboard();
    clip->setText(d->hoveredActionIpAddress.toString());
    d->hoveredActionIpAddress = QHostAddress();
}

void KNetworkManagerTrayIcon::showConnectionProperties()
{
    Q_D(KNetworkManagerTrayIcon);
    ConnectionInfoDialog *connectionInfoDialog;
    
    connectionInfoDialog = d->connectionPropertiesDialogs[d->hoveredActionInterfaceConnection];
    if (connectionInfoDialog == 0) {
        connectionInfoDialog = new ConnectionInfoDialog(d->hoveredActionInterfaceConnection);
        d->connectionPropertiesDialogs[d->hoveredActionInterfaceConnection] = connectionInfoDialog;
        connectionInfoDialog->show();
    } else {
        connectionInfoDialog->show();
        connectionInfoDialog->raise();
    }
    contextMenu()->hide();
}

void KNetworkManagerTrayIcon::showOtherWirelessDialog()
{
    Q_D(KNetworkManagerTrayIcon);
    d->otherWirelessNetworksDialog->show();
}

bool networkInterfaceLessThan(NetworkManager::Device *if1, NetworkManager::Device * if2)
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
    switch (if1->state()) {
        case NetworkManager::Device::Preparing:
        case NetworkManager::Device::ConfiguringHardware:
        case NetworkManager::Device::NeedAuth:
        case NetworkManager::Device::ConfiguringIp:
            if1status = Connecting;
            break;
        case NetworkManager::Device::Activated:
            if1status = Connected;
            break;
        default: // all kind of disconnected
            break;
    }
    switch (if2->state()) {
        case NetworkManager::Device::Preparing:
        case NetworkManager::Device::ConfiguringHardware:
        case NetworkManager::Device::NeedAuth:
        case NetworkManager::Device::ConfiguringIp:
            if2status = Connecting;
            break;
        case NetworkManager::Device::Activated:
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

bool networkInterfaceSameConnectionStateLessThan(NetworkManager::Device * if1, NetworkManager::Device * if2)
{
    bool lessThan = false;
    switch (if1->type() ) {
        case NetworkManager::Device::Ethernet:
            switch (if2->type()) {
                case NetworkManager::Device::Ethernet:
                    lessThan = if1->uni() < if2->uni();
                    break;
                case NetworkManager::Device::Wifi:
                    lessThan = true;
                    break;
                case NetworkManager::Device::Modem:
                default:
                    lessThan = false;
                    break;
            }
            break;
        case NetworkManager::Device::Wifi:
            switch (if2->type()) {
                case NetworkManager::Device::Ethernet:
                    lessThan = false;
                    break;
                case NetworkManager::Device::Wifi:
                    lessThan = if1->uni() < if2->uni();
                    break;
                case NetworkManager::Device::Modem:
                    lessThan = false;
                    break;
                default:
                    lessThan = true;
                    break;
            }
            break;
        case NetworkManager::Device::Modem:
            switch (if2->type()) {
                case NetworkManager::Device::Ethernet:
                case NetworkManager::Device::Wifi:
                    lessThan = true;
                    break;
                case NetworkManager::Device::Modem:
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
