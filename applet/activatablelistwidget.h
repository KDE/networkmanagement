/*
Copyright 2008,2009 Sebastian Kügler <sebas@kde.org>
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

#ifndef ACTIVATABLELISTWIDGET_H
#define ACTIVATABLELISTWIDGET_H

#include <QGraphicsWidget>

#include "activatable.h"
#include <Plasma/ScrollWidget>
#include <QtNetworkManager/manager.h>

class QGraphicsLinearLayout;
class ActivatableItem;
class RemoteActivatableList;
class RemoteActivatable;
class HiddenWirelessNetworkItem;
class ShowMoreItem;

class ActivatableListWidget: public Plasma::ScrollWidget
{
Q_OBJECT
public:
    enum FilterType {NormalConnections = 0x1, VPNConnections = 0x2, SharedConnections = 0x4, SavedConnections = 0x8};
    Q_DECLARE_FLAGS(FilterTypes, FilterType)

    explicit ActivatableListWidget(RemoteActivatableList* activatables, QGraphicsWidget* parent = 0);
    virtual ~ActivatableListWidget();

    void init();
    void addType(Knm::Activatable::ActivatableType type);
    void removeType(Knm::Activatable::ActivatableType type);
    bool accept(RemoteActivatable* activatable) const;
    void setShowAllTypes(bool show, bool refresh = false);
    void setHasWireless(bool hasWireless); // Used to decide whether or not to show the hidden item
    void setFilter(FilterTypes);
    FilterTypes& getFilter() { return m_filter; }

Q_SIGNALS:
    void showInterfaceDetails(QString);
    void showMoreClicked();

public Q_SLOTS:
    void activatableAdded(RemoteActivatable*, int);
    void activatableRemoved(RemoteActivatable*);
    void listDisappeared();
    void listAppeared();
    void deactivateConnection(const QString& deviceUni);
    void addInterface(NetworkManager::Device*);
    void clearInterfaces();
    void toggleVpn();
    void filter();

private Q_SLOTS:
    void deleteItem();
    void hoverEnter(const QString& uni);
    void hoverLeave(const QString& uni);
    void vpnHoverEnter();
    void vpnHoverLeave();
    void connectToHiddenNetwork(const QString &ssid);

private:
    void createItem(RemoteActivatable* conn, int index);
    void createHiddenItem();
    void updateShowMoreItem();
    int m_connectionType;
    QList<Knm::Activatable::ActivatableType> m_types;
    QHash<QString, NetworkManager::Device::Type> m_interfaces;

    QHash<RemoteActivatable*, ActivatableItem*> m_itemIndex;
    HiddenWirelessNetworkItem* m_hiddenItem;
    ShowMoreItem* m_showMoreItem;
    RemoteActivatableList* m_activatables;
    QGraphicsLinearLayout* m_layout;
    QGraphicsWidget* m_widget;

    bool m_showAllTypes;
    bool m_vpn;
    bool m_hasWireless; // Used to determine whether or not to show hidden config
    FilterTypes m_filter;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(ActivatableListWidget::FilterTypes)
#endif // ACTIVATABLELISTWIDGET_H
