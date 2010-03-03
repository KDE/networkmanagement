/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>
Copyright 2008, 2009 Sebastian K?gler <sebas@kde.org>

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

#include "activatableitem.h"
#include <kdebug.h>
#include "remoteactivatable.h"

#include <QPainter>
#include <KIcon>

ActivatableItem::ActivatableItem(RemoteActivatable *remote, QGraphicsItem * parent) : Plasma::IconWidget(parent),
    m_activatable(remote),
    m_hasDefaultRoute(false)
{
    setDrawBackground(true);
    setTextBackgroundColor(QColor(Qt::transparent));
    RemoteInterfaceConnection *remoteconnection = interfaceConnection();
    if (remoteconnection) {
        connect(remoteconnection, SIGNAL(hasDefaultRouteChanged(bool)),
                SLOT(handleHasDefaultRouteChanged(bool)));
        connect(remoteconnection, SIGNAL(activationStateChanged(Knm::InterfaceConnection::ActivationState)),
                SLOT(activationStateChanged(Knm::InterfaceConnection::ActivationState)));
    }
}

ActivatableItem::~ActivatableItem()
{
}

void ActivatableItem::emitClicked()
{
    if (m_activatable) {
        m_activatable->activate();
    }
    emit clicked(this);
}

RemoteInterfaceConnection * ActivatableItem::interfaceConnection() const
{
    return qobject_cast<RemoteInterfaceConnection*>(m_activatable);
}

void ActivatableItem::handleHasDefaultRouteChanged(bool has)
{
    // do something nice to show that this connection has the default route
    m_hasDefaultRoute = has;
    update();
}

void ActivatableItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Plasma::IconWidget::paint(painter, option, widget);
    if (m_hasDefaultRoute) {
        painter->drawPixmap(QRect(4,4,10,10), KIcon("face-smile").pixmap(QSize(8,8)));
    }
}

void ActivatableItem::activationStateChanged(Knm::InterfaceConnection::ActivationState state)
{
    // Update the view of the connection, manipulate font based on activation state.
    QFont f = font();
    switch (state) {
        //Knm::InterfaceConnectihon::ActivationState
        case Knm::InterfaceConnection::Activated:
            f.setBold(true);
            f.setItalic(false);
            break;
        case Knm::InterfaceConnection::Unknown:
            f.setBold(false);
            f.setItalic(false);
            break;
        case Knm::InterfaceConnection::Activating:
            f.setBold(false);
            f.setItalic(true);
    }
    setFont(f);
}

// vim: sw=4 sts=4 et tw=100
