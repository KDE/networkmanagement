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

#ifndef NMPOPUP_H
#define NMPOPUP_H

#include <QGraphicsWidget>
//#include <QHash>
//#include <QPair>

#include <Plasma/CheckBox>
//#include <Plasma/ExtenderItem>
//#include <Plasma/Extender>
#include <Plasma/Frame>
#include <Plasma/IconWidget>
#include <Plasma/TabBar>

#include <solid/control/networkinterface.h>

#include "activatable.h"

class QGraphicsLinearLayout;
class QGraphicsGridLayout;

class RemoteActivatable;
class RemoteActivatableList;

class ActivatableListWidget;
class InterfaceItem;
class InterfaceDetailsWidget;

class NMPopup: public QGraphicsWidget
{
Q_OBJECT
public:
    NMPopup(RemoteActivatableList *, QGraphicsWidget* parent = 0);
    virtual ~NMPopup();

    void init();
    //virtual QGraphicsItem * widget();
    Solid::Control::NetworkInterface* defaultInterface();
    bool available(int state);

public Q_SLOTS:
    void interfaceAdded(const QString&);
    void interfaceRemoved(const QString&);
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
    Plasma::TabBar* m_leftWidget;
    Plasma::Frame* m_rightWidget;
    QGraphicsWidget* m_interfaceWidget;
    QGraphicsLinearLayout* m_leftLayout;
    QGraphicsLinearLayout* m_rightLayout;
    QGraphicsLinearLayout* m_interfaceLayout;
    InterfaceDetailsWidget* m_interfaceDetailsWidget;

    Plasma::CheckBox* m_networkingCheckBox;
    Plasma::CheckBox* m_rfCheckBox;
    Plasma::IconWidget* m_connectionsButton;

    ActivatableListWidget* m_connectionList;
    ActivatableListWidget* m_wirelessList;
};

#endif // NMPOPUP_H
