/*
Copyright 2008 Sebastian Kügler <sebas@kde.org>

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

#ifndef ACTIVATABLELISTWIDGET_H
#define ACTIVATABLELISTWIDGET_H

#include <QGraphicsWidget>

#include <solid/control/networkinterface.h>

#include <Plasma/ScrollWidget>

class QGraphicsLinearLayout;
class ActivatableItem;
class RemoteActivatableList;
class RemoteActivatable;

class ActivatableListWidget: public Plasma::ScrollWidget
{
Q_OBJECT
public:
    ActivatableListWidget(RemoteActivatableList* activatables, QGraphicsWidget* parent = 0);
    virtual ~ActivatableListWidget();

    void init();
    bool accept(RemoteActivatable* activatable) const;

public Q_SLOTS:
    //virtual void activate(ActivatableItem*) = 0;
    void activatableAdded(RemoteActivatable *);
    void activatableRemoved(RemoteActivatable *);
    void listDisappeared();
    void listAppeared();

Q_SIGNALS:
    void connectionListUpdated();

private:
    ActivatableItem* createItem(RemoteActivatable* conn);
    int m_connectionType;

    QHash<RemoteActivatable*, ActivatableItem*> m_itemIndex;
    RemoteActivatableList* m_activatables;
    //Solid::Control::NetworkInterface* m_iface;
    QGraphicsLinearLayout* m_layout;
    QGraphicsWidget* m_widget;

};
#endif // ACTIVATABLELISTWIDGET_H
