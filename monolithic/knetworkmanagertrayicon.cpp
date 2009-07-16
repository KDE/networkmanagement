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

#include <QSignalMapper>
#include <QVBoxLayout>
#include <QWidgetAction>

#include <KAction>
#include <KDebug>
#include <KLocale>
#include <KMenu>
#include <KStandardAction>
#include <KToolInvocation>

#include <solid/control/networkmanager.h>

#include "activatable.h"
#include "activatabledebug.h"
#include "activatablelist.h"
#include "interfaceconnection.h"
#include "unconfiguredinterface.h"
#include "wirelessinterfaceconnection.h"
#include "wirelessnetworkitem.h"

#include "interfaceconnectionitem.h"
#include "wirelessinterfaceconnectionitem.h"
#include "sortedactivatablelist.h"
#include "unconfiguredinterfaceitem.h"
#include "wirelessnetworkitemitem.h"

// sorting activatables
// by interface type (compare on activatable::deviceUni()
// by activatableType() (interfaceconnections and wirelessinterfaceconnections before wirelessnetworkitems)
// then: for interfaceconnections - by activation state
//   then: alphabetically
//       for wirelessinterfaceconnections - by activation state
//         then: by strength
//           then: alphabetically
// InterfaceConnections)
// by type ([w]ic > wni)
// by signal strength (listen to strengthchanged signals)
//   or alphabetically
// QAction::addAction/removeAction
// resort everything or just find correct place for changed item?
//   (save pointer to last active connection?)
// make lists of 
// k

Q_DECLARE_METATYPE(Knm::Activatable *);

class KNetworkManagerTrayIconPrivate
{
public:
    Solid::Control::NetworkInterface::Types interfaceTypes;
    SortedActivatableList * sortedList;
    Experimental::KNotificationItem * notificationItem;
    KMenu * popup;
    QVBoxLayout * popupLayout;
    QHash<Knm::Activatable *, QWidgetAction *> actions;
    QStringList deviceUnis;
    QString iconName;
    bool showNetworkItems;
};

/* for qSort()ing */
bool networkInterfaceLessThan(Solid::Control::NetworkInterface * if1, Solid::Control::NetworkInterface * if2);
bool networkInterfaceSameConnectionStateLessThan(Solid::Control::NetworkInterface * if1, Solid::Control::NetworkInterface * if2);

KNetworkManagerTrayIcon::KNetworkManagerTrayIcon(Solid::Control::NetworkInterface::Types types, const QString & id, ActivatableList * list, QObject * parent)
    : KNotificationItem(id, parent), d_ptr(new KNetworkManagerTrayIconPrivate)
{
    Q_D(KNetworkManagerTrayIcon);
    d->interfaceTypes = types;
    d->iconName = "networkmanager";
    d->showNetworkItems = false;
    // don't try and make this our child or it crashes on app exit due to widgets it manages
    // not liking there being no QApplication anymore.
    setStandardActionsEnabled(false);
    setCategory(Experimental::KNotificationItem::Hardware);
    setTitle(i18nc("Popup title", "Network Management"));
    setIconByName(d->iconName);
    d->popup = new KMenu("Title", 0);

    //KMenu * menu = d->notificationItem->contextMenu();
    setAssociatedWidget(d->popup);
    setContextMenu(d->popup);
    setStatus(Experimental::KNotificationItem::Active);

    KAction * prefsAction = KStandardAction::preferences(this, SLOT(slotPreferences()), this);
    prefsAction->setText(i18nc("Preferences action title", "Manage Connections..."));
    //menu->addAction(prefsAction);

    d->sortedList = new SortedActivatableList(types, this);

    // HACK - insert a SortedActivatableList before us, so we can use its sort
    list->unregisterObserver(this);
    list->registerObserver(d->sortedList);
    list->registerObserver(this, d->sortedList);

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
            SLOT(updateTrayIcon()));

    // listen to existing devices' state changes
    foreach (Solid::Control::NetworkInterface * iface,
            Solid::Control::NetworkManager::networkInterfaces()) {
        if (d->interfaceTypes.testFlag(iface->type())) {
            kDebug() << "connecting" << iface->interfaceName() << "'s signals";
            QObject::connect(iface, SIGNAL(connectionStateChanged(int,int,int)), this, SLOT(handleConnectionStateChange(int,int,int)));
        }
    }

    // set initial tray icon appearance and tooltip
    updateTrayIcon();
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

    // clear the menu without deleting useful actions
    foreach (QAction * action, d->popup->actions()) {
        d->popup->removeAction(action);
        // throw away separators, easier than tracking them
        if (action->isSeparator()) {
            delete action;
        }
    }

    foreach (Knm::Activatable * activatable, d->sortedList->activatables()) {
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
                kDebug() << ic->connectionName();
                widget = new InterfaceConnectionItem(ic, d->popup);
            } else if ( activatable->activatableType() == Knm::Activatable::WirelessInterfaceConnection) {
                Knm::WirelessInterfaceConnection * wic = static_cast<Knm::WirelessInterfaceConnection*>(activatable);
                kDebug() << wic->connectionName();
                widget = new WirelessInterfaceConnectionItem(wic, d->popup);
            } else if ( activatable->activatableType() == Knm::Activatable::WirelessNetworkItem) {
                Knm::WirelessNetworkItem * wni = static_cast<Knm::WirelessNetworkItem*>(activatable);
                kDebug() << wni->ssid();
                widget = new WirelessNetworkItemItem(wni, d->popup);
            } else if ( activatable->activatableType() == Knm::Activatable::UnconfiguredInterface) {
                Knm::UnconfiguredInterface * unco = static_cast<Knm::UnconfiguredInterface*>(activatable);
                kDebug() << unco->deviceUni();
                widget = new UnconfiguredInterfaceItem(unco, d->popup);
            }
            action->setDefaultWidget(widget);
            d->actions.insert(activatable, action);
        }

        if (!d->deviceUnis.contains(activatable->deviceUni())) {
            widget->setFirst(true);
            d->deviceUnis.append(activatable->deviceUni());
            d->popup->addSeparator();
        }
        d->popup->addAction(action);
    }
}

void KNetworkManagerTrayIcon::handleUpdate(Knm::Activatable *)
{
    fillPopup();
    //QAction * action = d->actions[changed];
    //updateActionState(changed, action);
}
#if 0
void KNetworkManagerTrayIcon::updateActionState(Knm::Activatable * activatable, QAction * action)
{
    QString actionText;
    Knm::InterfaceConnection * ic;
    Knm::WirelessInterfaceConnection * wic;
    Knm::WirelessNetworkItem * wni;

    if (activatable && action) {
        switch (activatable->activatableType()) {
            case Knm::Activatable::InterfaceConnection:
                ic = qobject_cast<Knm::InterfaceConnection*>(activatable);
                action->setChecked((ic->activationState() != Knm::InterfaceConnection::Unknown));
                actionText = QString::fromLatin1("%1").arg(ic->connectionName());
                break;
            case Knm::Activatable::WirelessInterfaceConnection:
                wic = qobject_cast<Knm::WirelessInterfaceConnection*>(activatable);
                action->setChecked((wic->activationState() != Knm::InterfaceConnection::Unknown));
                actionText = QString::fromLatin1("%1 (%2)").arg(wic->connectionName(), QString::number(wic->strength()));
                break;
            case Knm::Activatable::WirelessNetworkItem:
                wni = qobject_cast<Knm::WirelessNetworkItem*>(activatable);
                actionText = QString::fromLatin1("%1 (%2)").arg(wni->ssid(), QString::number(wni->strength()));
                break;
        }
        action->setText(actionText);
    }
}
#endif

void KNetworkManagerTrayIcon::handleRemove(Knm::Activatable * removed)
{
    Q_D(KNetworkManagerTrayIcon);
    QWidgetAction * removedAction = d->actions.take(removed);
    delete removedAction;
}

void KNetworkManagerTrayIcon::activatableActionTriggered()
{
#if 0
    QAction * triggeredAction = qobject_cast<QAction*>(sender());
    if (triggeredAction) {
        Knm::Activatable * triggeredActivatable = triggeredAction->data().value<Knm::Activatable*>();
        kDebug() << ActivatableDebug::activatableToString(triggeredActivatable) << "was activated!";
        triggeredActivatable->activate();
    }
#endif
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
        if (!d->interfaceTypes.testFlag(iface->type())) {
            QObject::connect(iface, SIGNAL(connectionStateChanged(int)), this, SLOT(handleConnectionStateChange(int,int,int)));
        }
    }
    //update our state
    updateTrayIcon();
}

void KNetworkManagerTrayIcon::handleConnectionStateChange(int new_state, int old_state, int reason)
{
    Solid::Control::NetworkInterface * iface = qobject_cast<Solid::Control::NetworkInterface*>(sender());
    kDebug() << iface->interfaceName() << "has changed state from" << old_state << "to" << new_state << "because of reason" << reason;
    updateTrayIcon();
}

void KNetworkManagerTrayIcon::updateTrayIcon()
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
    QString iconName, overlayIconName;
    if (!interfaces.isEmpty()) {
        qSort(interfaces.begin(), interfaces.end(), networkInterfaceLessThan);
        Solid::Control::NetworkInterface * interface = interfaces.first();
        switch (interface->type() ) {
            case Solid::Control::NetworkInterface::Ieee8023:
                iconName = "network-wired";
                break;
            case Solid::Control::NetworkInterface::Ieee80211:
                iconName = "network-wireless";
                break;
            case Solid::Control::NetworkInterface::Serial:
                iconName = "modem";
                break;
            case Solid::Control::NetworkInterface::Gsm:
            case Solid::Control::NetworkInterface::Cdma:
                iconName = "phone";
                break;
            default:
                iconName = "network-wired";
                break;
        }

        switch (interface->connectionState()) {
            case Solid::Control::NetworkInterface::Preparing:
                //overlayIconName = "busy-phase1";
                overlayIconName = "emblem-mounted";
                break;
            case Solid::Control::NetworkInterface::Configuring:
                overlayIconName = "busy-phase2";
                overlayIconName = "emblem-mounted";
                break;
            case Solid::Control::NetworkInterface::NeedAuth:
                overlayIconName = "busy-phase2";
                overlayIconName = "emblem-mounted";
                break;
            case Solid::Control::NetworkInterface::IPConfig:
                overlayIconName = "busy-phase3";
                overlayIconName = "emblem-mounted";
                break;
            case Solid::Control::NetworkInterface::Activated:
                overlayIconName = "checkbox";
                break;
            default:
                break;
        }
    } else {
        iconName = "networkmanager";
    }

    kDebug() << "suggested icon name:" << iconName;

    if (iconName != d->iconName) {
        d->iconName = iconName;
        kDebug() << "setting icon:" << iconName;
        setIconByName(d->iconName);
    }
    kDebug() << "setting overlay:" << overlayIconName;
    setOverlayIconByName(overlayIconName);
}

void KNetworkManagerTrayIcon::networkingStatusChanged(Solid::Networking::Status status)
{
    if (status == Solid::Networking::Unknown) {
        setStatus(Experimental::KNotificationItem::Passive);
    } else {
        setStatus(Experimental::KNotificationItem::Active);
    }
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
