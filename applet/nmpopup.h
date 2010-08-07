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

#include <Plasma/CheckBox>
#include <Plasma/Frame>
#include <Plasma/IconWidget>
#include <Plasma/Label>
#include <Plasma/PushButton>
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
class VpnInterfaceItem;

class NMPopup: public QGraphicsWidget
{
Q_OBJECT
public:
    NMPopup(RemoteActivatableList *, QGraphicsWidget* parent = 0);
    virtual ~NMPopup();

    void init();
    Solid::Control::NetworkInterface* defaultInterface();
    bool available(int state);

    QHash<QString, InterfaceItem*> m_interfaces;

public Q_SLOTS:
    void interfaceAdded(const QString&);
    void interfaceRemoved(const QString&);
    void managerWirelessEnabledChanged(bool);
    void managerWirelessHardwareEnabledChanged(bool);
    void wirelessEnabledToggled(bool checked);
    void networkingEnabledToggled(bool checked);
    void manageConnections();
    void showMore();
    void showMore(bool);
    void handleConnectionStateChange(int new_state, int old_state, int reason);
    void toggleInterfaceTab();
    void deleteInterfaceItem();

Q_SIGNALS:
    void configNeedsSaving();

private:
    void addInterfaceInternal(Solid::Control::NetworkInterface *);
    void addVpnInterface();
    QSizeF sizeHint ( Qt::SizeHint which, const QSizeF & constraint = QSizeF() ) const;

    RemoteActivatableList* m_activatables;

    QGraphicsWidget* m_widget;
    QGraphicsGridLayout* m_mainLayout;
    // Interfaces label
    Plasma::Label* m_leftLabel;
    // Container for interface overview and interface details widgets
    Plasma::TabBar* m_leftWidget;
    // Overall layout for interface overview
    QGraphicsLinearLayout* m_leftLayout;
    // Inner layout for interface list
    QGraphicsLinearLayout* m_interfaceLayout;

    InterfaceDetailsWidget* m_interfaceDetailsWidget;

    // Connections Label
    Plasma::Label* m_rightLabel;
    // Container for connection List
    QGraphicsWidget* m_rightWidget;

    QGraphicsLinearLayout* m_rightLayout;

    Plasma::CheckBox* m_networkingCheckBox;
    Plasma::CheckBox* m_rfCheckBox;
    Plasma::PushButton* m_connectionsButton;
    Plasma::PushButton* m_showMoreButton;

    ActivatableListWidget* m_connectionList;
    VpnInterfaceItem* m_vpnItem;
};

#endif // NMPOPUP_H
