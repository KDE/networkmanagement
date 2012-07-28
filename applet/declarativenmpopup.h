/*
Copyright 2012 Arthur de Souza Ribeiro <arthurdesribeiro@gmail.com>

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

#ifndef DECLARATIVE_NM_POPUP_H
#define DECLARATIVE_NM_POPUP_H

#include <Plasma/DeclarativeWidget>
#include "connectionslistmodel.h"
#include "interfaceslistmodel.h"
#include "declarativeinterfaceitem.h"

class DeclarativeNMPopup : public Plasma::DeclarativeWidget
{
    Q_OBJECT

public:
    DeclarativeNMPopup(RemoteActivatableList * activatableList, QGraphicsWidget *parent=0);

    void connectionAdded(ConnectionItem *connection);
    void connectionRemoved(ConnectionItem *connection);
    void updateHasWireless(bool checked = true);
    void updateHasWwan();

    QHash<QString, DeclarativeInterfaceItem*> m_interfaces;

private:
    void addInterfaceInternal(NetworkManager::Device *);

public Q_SLOTS:
    void updateWireless(bool checked);
    void updateMobile(bool checked);
    void manageConnections();
    void showInterfaceDetails(const QString & uni);

private Q_SLOTS:
    void readConfig();
    void managerWirelessEnabledChanged(bool);
    void managerWirelessHardwareEnabledChanged(bool);
    void managerWwanHardwareEnabledChanged(bool);
    void managerWwanEnabledChanged(bool);
    void interfaceRemoved(const QString& uni);
    void interfaceAdded(const QString& uni);
    void manageUpdateTraffic(NetworkManager::Device *device);

    void qmlCreationFinished();
private:
    ConnectionsListModel *listModel;
    InterfacesListModel *interfaceListModel;
    bool m_hasWirelessInterface;
    RemoteActivatableList* m_activatables;
};

#endif
