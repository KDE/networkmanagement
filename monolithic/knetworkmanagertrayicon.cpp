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
    Solid::Control::NetworkInterface::Types displayedTypes;
    SortedActivatableList * sortedList;
    Experimental::KNotificationItem * notificationItem;
    KMenu * popup;
    QVBoxLayout * popupLayout;
    QHash<Knm::Activatable *, QWidgetAction *> actions;
    QStringList deviceUnis;

};

KNetworkManagerTrayIcon::KNetworkManagerTrayIcon(Solid::Control::NetworkInterface::Types types, const QString & id, ActivatableList * list, QObject * parent)
    : KNotificationItem(id, parent), d_ptr(new KNetworkManagerTrayIconPrivate)
{
    Q_D(KNetworkManagerTrayIcon);
    d->displayedTypes = types;

    // don't try and make this our child or it crashes on app exit due to widgets it manages
    // not liking there being no QApplication anymore.
    setStandardActionsEnabled(false);
    setCategory(Experimental::KNotificationItem::Hardware);
    setTitle(i18nc("Popup title", "Network Management"));
    setIconByName("networkmanager");
    d->popup = new KMenu("Title", 0);

//    d->notificationItem->setAssociatedWidget(d->popup);
    //KMenu * menu = d->notificationItem->contextMenu();
    setContextMenu(d->popup);
    KAction * prefsAction = KStandardAction::preferences(this, SLOT(slotPreferences()), this);
    prefsAction->setText(i18nc("Preferences action title", "Manage Connections..."));
    //menu->addAction(prefsAction);

    d->sortedList = new SortedActivatableList(types, this);

    // HACK - insert a SortedActivatableList before us, so we can use its sort
    list->unregisterObserver(this);
    list->registerObserver(d->sortedList);
    list->registerObserver(this, d->sortedList);

    fillPopup();
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

// vim: sw=4 sts=4 et tw=100
