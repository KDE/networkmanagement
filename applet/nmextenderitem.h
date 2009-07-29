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

#include <Plasma/CheckBox>
#include <Plasma/ExtenderItem>
#include <Plasma/Extender>
#include <Plasma/Frame>
#include <Plasma/IconWidget>
#include <Plasma/TabBar>

#include <solid/control/networkinterface.h>
//class AbstractConnectableItem;
//class NetworkManagerSettings;
class QGraphicsLinearLayout;
class QGraphicsGridLayout;
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
    void interfaceAdded(const QString&);
    void interfaceRemoved(const QString&);
    void switchTab(const QString&);

    void managerWirelessEnabledChanged(bool);
    void managerWirelessHardwareEnabledChanged(bool);
    void wirelessEnabledToggled(bool checked);
    void manageConnections();

Q_SIGNALS:
    void connectionListUpdated();

private:
    void addInterfaceInternal(Solid::Control::NetworkInterface *);
    void createTab(InterfaceItem * item, Solid::Control::NetworkInterface* iface, const QString &name, const QString &icon = 0);

    RemoteActivatableList* m_activatables;
    // list of interfaces [uni] = iface
    QHash<QString, InterfaceItem *> m_interfaces;
    QHash<QString, int> m_tabIndex;


    QGraphicsWidget* m_widget;
    QGraphicsLinearLayout* m_mainLayout;
    Plasma::Frame* m_leftWidget;
    Plasma::Frame* m_rightWidget;
    QGraphicsWidget* m_interfaceWidget;
    QGraphicsLinearLayout* m_leftLayout;
    QGraphicsLinearLayout* m_rightLayout;
    QGraphicsLinearLayout* m_interfaceLayout;

    Plasma::CheckBox* m_rfCheckBox;
    Plasma::IconWidget* m_connectionsButton;

    Plasma::TabBar* m_connectionTabs;
};
#endif // NMEXTENDERITEM_H
