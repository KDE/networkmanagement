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

#include "simpleui.h"

#include <knotificationitem-1/knotificationitem.h>

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

SimpleUi::SimpleUi(ActivatableList * list, QObject * parent)
    : ActivatableObserver(parent)
{
    // leak, you fucker
    m_notificationItem = new Experimental::KNotificationItem(0);
    m_notificationItem->setCategory(Experimental::KNotificationItem::Hardware);
    m_notificationItem->setTitle(i18nc("Popup title", "Network Management"));
    m_notificationItem->setIconByName("networkmanager");
    m_popup = new KMenu("Title", 0);

    m_notificationItem->setAssociatedWidget(m_popup);
    KMenu * menu = m_notificationItem->contextMenu();
    KAction * prefsAction = KStandardAction::preferences(this, SLOT(slotPreferences()), this);
    prefsAction->setText(i18nc("Preferences action title", "Manage Connections..."));
    menu->addAction(prefsAction);

    m_sortedList = new SortedActivatableList(list, this);

    //HACK
    list->unregisterObserver(this);
    list->registerObserver(m_sortedList);
    list->registerObserver(this, m_sortedList);

    fillPopup();
#if 0
    foreach (Knm::Activatable * activatable, list->activatables()) {
        handleAdd(activatable);
    }
#endif
}

SimpleUi::~SimpleUi()
{
}

void SimpleUi::handleAdd(Knm::Activatable *)
{
    // FIXME could cache QWidgetActions here...
    fillPopup();
}

void SimpleUi::fillPopup()
{
    m_deviceUnis.clear();

    m_popup->clear();
    foreach (Knm::Activatable * activatable, m_sortedList->activatables()) {
        QWidgetAction * newAct = new QWidgetAction(this);
        newAct->setData(QVariant::fromValue(activatable));
        ActivatableItem * widget = 0;
        if (activatable->activatableType() == Knm::Activatable::InterfaceConnection) {
            Knm::InterfaceConnection * ic = static_cast<Knm::InterfaceConnection*>(activatable);
            kDebug() << ic->connectionName();
            widget = new InterfaceConnectionItem(ic, m_popup);
            //connect(newAct, SIGNAL(triggered(bool)), this, SLOT(activatableActionTriggered()));
        } else if ( activatable->activatableType() == Knm::Activatable::WirelessInterfaceConnection) {
            Knm::WirelessInterfaceConnection * wic = static_cast<Knm::WirelessInterfaceConnection*>(activatable);
            kDebug() << wic->connectionName();
            widget = new WirelessInterfaceConnectionItem(wic, m_popup);
        } else if ( activatable->activatableType() == Knm::Activatable::WirelessNetworkItem) {
            Knm::WirelessNetworkItem * wni = static_cast<Knm::WirelessNetworkItem*>(activatable);
            kDebug() << wni->ssid();
            widget = new WirelessNetworkItemItem(wni, m_popup);
        } else if ( activatable->activatableType() == Knm::Activatable::UnconfiguredInterface) {
            Knm::UnconfiguredInterface * unco = static_cast<Knm::UnconfiguredInterface*>(activatable);
            kDebug() << unco->deviceUni();
            widget = new UnconfiguredInterfaceItem(unco, m_popup);
        }

        if (widget) {
            if (!m_deviceUnis.contains(activatable->deviceUni())) {
                widget->setFirst(true);
                m_deviceUnis.append(activatable->deviceUni());
                m_popup->addSeparator();
            }
            newAct->setDefaultWidget(widget);
            m_actions.insert(activatable, newAct);
            m_popup->insertAction(0, newAct);
        }
    }
}

void SimpleUi::handleUpdate(Knm::Activatable *)
{
    fillPopup();
    //QAction * action = m_actions[changed];
    //updateActionState(changed, action);
}
#if 0
void SimpleUi::updateActionState(Knm::Activatable * activatable, QAction * action)
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

void SimpleUi::handleRemove(Knm::Activatable * removed)
{
    QWidgetAction * removedAction = m_actions.take(removed);
    delete removedAction;
}

void SimpleUi::activatableActionTriggered()
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

void SimpleUi::slotPreferences()
{
    QStringList args;
    args << "kcm_networkmanagement";
    KToolInvocation::kdeinitExec("kcmshell4", args);
}

// vim: sw=4 sts=4 et tw=100
