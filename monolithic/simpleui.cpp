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

#include <KAction>
#include <KDebug>
#include <KLocale>
#include <KMenu>

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
    m_notificationItem->setTitle("Network Management");
    m_notificationItem->setIconByName("networkmanager");
    m_popup = new KMenu("Title", 0);
    m_notificationItem->setAssociatedWidget(m_popup);
}

SimpleUi::~SimpleUi()
{
}

void SimpleUi::handleAdd(Knm::Activatable * activatable)
{
    QString actionText;
    Knm::InterfaceConnection * ic;
    Knm::WirelessInterfaceConnection * wic;
    Knm::WirelessNetworkItem * wni;

    switch (activatable->activatableType()) {
        case Knm::Activatable::Connection:
            ic = qobject_cast<Knm::InterfaceConnection*>(activatable);
            actionText = ic->connectionName();
            break;
        case Knm::Activatable::WirelessConnection:
            wic = qobject_cast<Knm::WirelessInterfaceConnection*>(activatable);
            actionText = wic->connectionName();
            break;
        case Knm::Activatable::WirelessNetworkItem:
            wni = qobject_cast<Knm::WirelessNetworkItem*>(activatable);
            actionText = wni->ssid();
            break;
    }
    KAction * newAct = new KAction(actionText, this);
    newAct->setData(QVariant::fromValue(activatable));
    newAct->setIcon(KIcon(iconForActivatable(activatable)));
    m_actions.insert(activatable, newAct);
    m_popup->addAction(newAct);
    connect(newAct, SIGNAL(triggered(bool)), this, SLOT(activatableActionTriggered()));
}

void SimpleUi::handleUpdate(Knm::Activatable * changed)
{
    // TODO implement
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
    Knm::InterfaceConnection * ic;
    Knm::WirelessInterfaceConnection * wic;
    Knm::WirelessNetworkItem * wni;

    switch (activatable->activatableType()) {
        case Knm::Activatable::Connection:
            iconName = QLatin1String("network-wired");
            break;
        case Knm::Activatable::WirelessConnection:
            wic = qobject_cast<Knm::WirelessInterfaceConnection*>(activatable);
            iconName = QLatin1String("preferences-other");
            break;
        case Knm::Activatable::WirelessNetworkItem:
            wni = qobject_cast<Knm::WirelessNetworkItem*>(activatable);
            iconName = QLatin1String("network-wireless");
            break;
    }
    return iconName;

}
    // vim: sw=4 sts=4 et tw=100
