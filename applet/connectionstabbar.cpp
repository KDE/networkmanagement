/*
Copyright 2011 Lamarque V. Souza <lamarque@kde.org>

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

#include "connectionstabbar.h"

#include <QGraphicsSceneEvent>

#include <KDebug>

ConnectionsTabBar::ConnectionsTabBar(QGraphicsWidget * parent): Plasma::TabBar(parent)
{
   //setAcceptHoverEvents(true);
   //nativeWidget()->setMouseTracking(true);
}

bool ConnectionsTabBar::event(QEvent *event)
{
    //kDebug() << "Lamarque" << event->type() << nativeWidget()->hasMouseTracking();
    if (event->type() == QEvent::GraphicsSceneHoverEnter ||
        event->type() == QEvent::GraphicsSceneMouseMove) {
        QGraphicsSceneHoverEvent *he = static_cast<QGraphicsSceneHoverEvent *>(event);
        //kDebug() << "Lamarque entrou" << event->type();
        for (int i = 0; i < count(); ++i) {
            QRectF area = nativeWidget()->tabRect(i);
            if (area.contains(he->pos())) {
                if (i != currentIndex()) {
                    setCurrentIndex(i);
                }
                break;
            }
        }
    }
    return Plasma::TabBar::event(event);
}

// without this no mouseMoveEvent()'s are reported
void ConnectionsTabBar::mousePressEvent(QGraphicsSceneMouseEvent *ev)
{
    //kDebug() << "Lamarque";
}

void ConnectionsTabBar::mouseReleaseEvent(QGraphicsSceneMouseEvent *ev)
{
    //kDebug() << "Lamarque";
}


void ConnectionsTabBar::mouseMoveEvent(QGraphicsSceneMouseEvent *ev)
{
    //kDebug() << "Lamarque 1 event->type() == " << ev->type();
    event(ev);
}

bool ConnectionsTabBar::sceneEvent ( QEvent * event )
{
    //kDebug() << "Lamarque";
    return Plasma::TabBar::sceneEvent(event);
}
