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

#include <KAction>
#include <KDebug>
#include <KLocale>
#include <KMenu>
#include <KStandardAction>
#include <KToolInvocation>

#include "activatable.h"
#include "activatabledebug.h"
#include "activatablelist.h"
#include "wirelessinterfaceconnection.h"
#include "wirelessnetworkitem.h"

Q_DECLARE_METATYPE(Knm::Activatable *);

SimpleUi::SimpleUi(ActivatableList * list, QObject * parent)
    : ActivatableObserver(parent)
{
    m_notificationItem = new Experimental::KNotificationItem(this);
    m_notificationItem->setCategory(Experimental::KNotificationItem::Hardware);
    m_notificationItem->setTitle(i18nc("Popup title", "Network Management"));
    m_notificationItem->setIconByName("networkmanager");
    m_popup = new KMenu("Title", 0);
    m_notificationItem->setAssociatedWidget(m_popup);
    KMenu * menu = m_notificationItem->contextMenu();
    KAction * prefsAction = KStandardAction::preferences(this, SLOT(slotPreferences()), this);
    prefsAction->setText(i18nc("Preferences action title", "Manage Connections..."));
    menu->addAction(prefsAction);

    foreach (Knm::Activatable * activatable, list->activatables()) {
        handleAdd(activatable);
    }
}

SimpleUi::~SimpleUi()
{
}

void SimpleUi::handleAdd(Knm::Activatable * activatable)
{
    KAction * newAct = new KAction(this);
    newAct->setData(QVariant::fromValue(activatable));
    newAct->setIcon(KIcon(iconForActivatable(activatable)));
    if (activatable->activatableType() != Knm::Activatable::WirelessNetworkItem) {
        newAct->setCheckable(true);
    }
    updateActionState(activatable, newAct);
    m_actions.insert(activatable, newAct);
    m_popup->addAction(newAct);
    connect(newAct, SIGNAL(triggered(bool)), this, SLOT(activatableActionTriggered()));
}

void SimpleUi::handleUpdate(Knm::Activatable * changed)
{
    QAction * action = m_actions[changed];
    updateActionState(changed, action);
}

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

void SimpleUi::handleRemove(Knm::Activatable * removed)
{
    QAction * removedAction = m_actions.take(removed);
    delete removedAction;
}

void SimpleUi::activatableActionTriggered()
{
    QAction * triggeredAction = qobject_cast<QAction*>(sender());
    if (triggeredAction) {
        Knm::Activatable * triggeredActivatable = triggeredAction->data().value<Knm::Activatable*>();
        kDebug() << ActivatableDebug::activatableToString(triggeredActivatable) << "was activated!";
        triggeredActivatable->activate();
    }
}

QString SimpleUi::iconForActivatable(Knm::Activatable * activatable)
{
    QString iconName;
//X     Knm::InterfaceConnection * ic;
//X     Knm::WirelessInterfaceConnection * wic;
//X     Knm::WirelessNetworkItem * wni;

    switch (activatable->activatableType()) {
        case Knm::Activatable::InterfaceConnection:
            iconName = QLatin1String("network-wired");
            break;
        case Knm::Activatable::WirelessInterfaceConnection:
//X             wic = qobject_cast<Knm::WirelessInterfaceConnection*>(activatable);
            iconName = QLatin1String("preferences-other");
            break;
        case Knm::Activatable::WirelessNetworkItem:
//X             wni = qobject_cast<Knm::WirelessNetworkItem*>(activatable);
            iconName = QLatin1String("network-wireless");
            break;
    }
    return iconName;

}

void SimpleUi::slotPreferences()
{
    QStringList args;
    args << "kcm_networkmanagement";
    KToolInvocation::kdeinitExec("kcmshell4", args);
}

// vim: sw=4 sts=4 et tw=100
