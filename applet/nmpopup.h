/*
Copyright 2009 Sebastian Kügler <sebas@kde.org>
Copyright 2011 Lamarque Souza <lamarque@kde.org>

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

#include <Plasma/CheckBox>
#include <Plasma/Frame>
#include <Plasma/IconWidget>
#include <Plasma/Label>
#include <Plasma/PushButton>
#include <Plasma/TabBar>

#include <solid/networking.h>
#include <QtNetworkManager/device.h>

#include "activatable.h"
#include "connectionstabbar.h"

class QGraphicsLinearLayout;
class QGraphicsGridLayout;

class RemoteActivatable;
class RemoteActivatableList;

class ActivatableListWidget;
class InterfaceItem;
class InterfaceDetailsWidget;
class TypeListWidget;
class VpnInterfaceItem;

class NMPopup: public QGraphicsWidget
{
Q_OBJECT
public:
    explicit NMPopup(RemoteActivatableList *, QGraphicsWidget* parent = 0);
    virtual ~NMPopup();

    void init();
    NetworkManager::Device* defaultInterface();
    bool available(int state);
    bool hasWireless();

    QHash<QString, InterfaceItem*> m_interfaces;

public Q_SLOTS:
    void interfaceAdded(const QString&);
    void interfaceRemoved(const QString&);
    void managerWirelessEnabledChanged(bool);
    void managerWirelessHardwareEnabledChanged(bool);
    void wirelessEnabledToggled(bool checked);
    void managerWwanEnabledChanged(bool);
    void managerWwanHardwareEnabledChanged(bool);
    void wwanEnabledToggled(bool checked);
    void networkingEnabledToggled(bool checked);
    void managerNetworkingEnabledChanged(bool);
    void manageConnections();
    void showMore();
    void showMore(bool);
    void handleConnectionStateChange(NetworkManager::Device::State new_state, NetworkManager::Device::State old_state, NetworkManager::Device::StateChangeReason reason);
    void toggleInterfaceTab();
    void untoggleInterfaceTab();
    void currentTabChanged(int);
    void currentInnerTabChanged(int);
    void deleteInterfaceItem();

Q_SIGNALS:
    void configNeedsSaving();
    void showMoreChecked(bool);

private Q_SLOTS:
    void readConfig();
    void checkShowMore(RemoteActivatable *);
    void uncheckShowMore(RemoteActivatable *);
    void showInterfaceDetails(const QString &);
    void connectToAnotherNetwork();

private:
    void addInterfaceInternal(NetworkManager::Device *);
    void addVpnInterface();
    void updateHasWireless(bool checked = true);
    void updateHasWwan();

    RemoteActivatableList* m_activatables;
    bool m_hasWirelessInterface;
    bool m_showMoreChecked, m_oldShowMoreChecked;
    int wicCount;
    QGraphicsWidget* m_widget;
    QGraphicsLinearLayout* m_mainLayout;
    QGraphicsGridLayout* m_tab1Layout;
    QGraphicsGridLayout* m_connectionTabLayout;

    InterfaceDetailsWidget* m_interfaceDetailsWidget;
    TypeListWidget * m_typeListWidget;
    // Container for connection List and interface details widgets
    Plasma::TabBar* m_mainTabBar;
    ConnectionsTabBar* m_connectionsTabBar;
//    Plasma::Label* m_title;

    Plasma::CheckBox* m_wifiCheckBox;
    Plasma::CheckBox* m_wwanCheckBox;
    Plasma::PushButton* m_connectionsButton;
    Plasma::PushButton* m_showMoreButton;
    Plasma::IconWidget* m_advancedSettingsButton;
    Plasma::IconWidget* m_connectToAnotherNetwork;

    ActivatableListWidget* m_connectionList;
    InterfaceItem* m_currentIfaceItem;
    VpnInterfaceItem* m_vpnItem;
    QGraphicsWidget * m_typeListWiget;

    friend class NetworkManagerApplet;
};

#endif // NMPOPUP_H
