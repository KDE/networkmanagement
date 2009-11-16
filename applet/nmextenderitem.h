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

#include "activatable.h"

class QGraphicsLinearLayout;
class QGraphicsGridLayout;
class ActivatableItem;

class RemoteActivatable;
class RemoteActivatableList;

class ActivatableListWidget;
class InterfaceItem;

class NMExtenderItem: public Plasma::ExtenderItem
{
Q_OBJECT
public:
    NMExtenderItem(RemoteActivatableList *, Plasma::Extender * ext = 0);
    virtual ~NMExtenderItem();

    void init();
    virtual QGraphicsItem * widget();
    Solid::Control::NetworkInterface* defaultInterface();
    bool available(int state);

public Q_SLOTS:
    void interfaceAdded(const QString&);
    void interfaceRemoved(const QString&);
    void switchTab(int type); // Takes networkinterface type
    void switchToDefaultTab();
    void managerWirelessEnabledChanged(bool);
    void managerWirelessHardwareEnabledChanged(bool);
    void wirelessEnabledToggled(bool checked);
    void networkingEnabledToggled(bool checked);
    void manageConnections();
    void handleConnectionStateChange(int new_state, int old_state, int reason);

Q_SIGNALS:
    void connectionListUpdated();
    void configNeedsSaving();

private:
    void addInterfaceInternal(Solid::Control::NetworkInterface *);
    void createTab(Knm::Activatable::ActivatableType type);

    RemoteActivatableList* m_activatables;
    // uni, interfaceitem mapping
    QHash<QString, InterfaceItem *> m_interfaces;
    // ActivatableType, index of tab mapping
    QHash<int, int> m_tabIndex;

    Plasma::TabBar* m_connectionTabs;

    QGraphicsWidget* m_widget;
    QGraphicsGridLayout* m_mainLayout;
    Plasma::Frame* m_leftWidget;
    Plasma::Frame* m_rightWidget;
    QGraphicsWidget* m_interfaceWidget;
    QGraphicsLinearLayout* m_leftLayout;
    QGraphicsLinearLayout* m_rightLayout;
    QGraphicsLinearLayout* m_interfaceLayout;

    Plasma::CheckBox* m_networkingCheckBox;
    Plasma::CheckBox* m_rfCheckBox;
    Plasma::IconWidget* m_connectionsButton;

    ActivatableListWidget* m_connectionList;
    ActivatableListWidget* m_wirelessList;
};

#endif // NMEXTENDERITEM_H
