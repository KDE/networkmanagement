/*
Copyright 2009 Sebastian Kügler <sebas@kde.org>

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

#ifndef NMEXTENDERITEM_H
#define NMEXTENDERITEM_H

#include <QGraphicsWidget>
//#include <QHash>
//#include <QPair>

#include <Plasma/ExtenderItem>
#include <Plasma/Extender>
#include <Plasma/TabBar>

#include <solid/control/networkinterface.h>
//class AbstractConnectableItem;
//class NetworkManagerSettings;
class QGraphicsLinearLayout;
class ActivatableItem;

class RemoteActivatable;
class RemoteActivatableList;

class InterfaceItem;

class NMExtenderItem: public Plasma::ExtenderItem
{
Q_OBJECT
public:
    NMExtenderItem(RemoteActivatableList *, Plasma::Extender * ext = 0);
    virtual ~NMExtenderItem();

    void init();
    virtual QGraphicsItem * widget();

public Q_SLOTS:
    //virtual void activate(ActivatableItem*) = 0;
    void activatableAdded(RemoteActivatable *);
    void activatableRemoved(RemoteActivatable *);
    void listDisappeared();
    void interfaceAdded(const QString&);
    void interfaceRemoved(const QString&);

Q_SIGNALS:
    void connectionListUpdated();

private:
    void addInterfaceInternal(Solid::Control::NetworkInterface *);


    RemoteActivatableList* m_activatables;
    // list of interfaces [uni] = iface
    QHash<QString, InterfaceItem *> m_interfaces;
    QHash<QString, int> m_tabIndex;

    QGraphicsWidget* m_widget;
    QGraphicsLinearLayout* m_mainLayout;
    QGraphicsLinearLayout* m_interfaceLayout;
    Plasma::TabBar* m_connectionTabs;
};
#endif // NMEXTENDERITEM_H
