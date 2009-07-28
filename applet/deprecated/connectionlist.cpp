/*
Copyright 2008,2009 Will Stephenson <wstephenson@kde.org>

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

#include "connectionlist.h"

#include <NetworkManager.h>

#include <QGraphicsLinearLayout>

#include <Plasma/Extender>
#include <KDebug>

#include "activatableitem.h"
#include "remoteactivatable.h"
#include "remoteactivatablelist.h"

ConnectionList::ConnectionList(RemoteActivatableList * activatableList, Plasma::Extender * ext)
: Plasma::ExtenderItem(ext), m_activatables(activatableList),
    m_connectionLayout(new QGraphicsLinearLayout(Qt::Vertical))
{
    m_widget = new QGraphicsWidget(this);
    m_layout = new QGraphicsLinearLayout(Qt::Vertical, m_widget);
    m_layout->addItem(m_connectionLayout);
    m_widget->setLayout(m_layout);
    setWidget(m_widget);
}

ConnectionList::~ConnectionList()
{
}

void ConnectionList::init()
{
    // adds items from subclasses above our layout
    setupHeader();

    connect(m_activatables, SIGNAL(activatableAdded(RemoteActivatable*)), 
            SLOT(activatableAdded(RemoteActivatable *)));
    connect(m_activatables, SIGNAL(activatableRemoved(RemoteActivatable*)), 
            SLOT(activatableRemoved(RemoteActivatable *)));

    connect(m_activatables, SIGNAL(appeared()), SLOT(getList()));
    connect(m_activatables, SIGNAL(disappeared()), SLOT(listDisappeared()));

    getList();

    // adds items from subclasses below our layout
    setupFooter();
    m_layout->addStretch(5);
}

QGraphicsItem * ConnectionList::widget()
{
    return m_widget;
}

bool ConnectionList::getList()
{
    bool added = false;

    foreach (RemoteActivatable * activatable, m_activatables->activatables()) {
        added |= registerActivatable(activatable);
    }

    if (added) {
        emit connectionListUpdated();
    }
    return added;
}

void ConnectionList::listDisappeared()
{
    //remove all connections from this service
    QHash<RemoteActivatable*, ActivatableItem*>::iterator i = m_connections.begin();
    while (i != m_connections.end()) {
        ActivatableItem * item = i.value();
        m_connectionLayout->removeItem(item);
        i = m_connections.erase(i);
        delete item;
    }
    emit connectionListUpdated();
}

bool ConnectionList::registerActivatable(RemoteActivatable * activatable)
{
    bool changed = false;
    if (!m_connections.contains(activatable)) {
        // let subclass decide
        if (accept(activatable)) {
            kDebug() << "adding activatable";
            ActivatableItem * ci = createItem(activatable);

            m_connections.insert(activatable, ci);
            m_connectionLayout->addItem(ci);
            m_connectionLayout->invalidate();
            m_layout->invalidate();
            changed = true;
        }
    }
    return changed;
}

void ConnectionList::activatableAdded(RemoteActivatable * added)
{
    if (registerActivatable(added)) {
        kDebug();
        emit connectionListUpdated();
    }
}

void ConnectionList::activatableRemoved(RemoteActivatable * removed)
{
    // look up the ActivatableItem and remove it
    if (m_connections.contains(removed)) {
        ActivatableItem * item = m_connections.value(removed);
        m_connectionLayout->removeItem(item);
        m_connections.remove(removed);
        delete item;
        kDebug();
        emit connectionListUpdated();
    }
}

ActivatableItem * ConnectionList::createItem(RemoteActivatable * activatable)
{
    ActivatableItem * ci;
    //ci = new ActivatableItem(activatable, this);
#warning Instantiate correct type Item
    ci->setupItem();
    return ci;
}

void ConnectionList::reassess()
{
    if (getList()) {
        kDebug() << "PING";
        emit connectionListUpdated();
    }
}

bool ConnectionList::isEmpty()
{
    return m_connections.count() == 0;
}

// vim: sw=4 sts=4 et tw=100

