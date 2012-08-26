/***************************************************************************
 *                                                                         *
 *   Copyright 2012 Arthur de Souza Ribeiro <arthurdesribeiro@gmail.com>   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 1.0
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents

Column {
    spacing: 8

    PlasmaComponents.TabBar {
        width: 300
        height: 30
        anchors.horizontalCenter: parent.horizontalCenter
        PlasmaComponents.TabButton { tab: tabContent; text: i18n("Connections")}
        PlasmaComponents.TabButton { tab: tabContent; text: i18n("VPN")}
        PlasmaComponents.TabButton { tab: tabContent; text: i18n("Shared Connections")}

        onCurrentTabChanged: {
            connectionsListModel.setFilter(currentTab.text)
        }
    }

    PlasmaComponents.TabGroup {
        height: parent.height - 30
        width: parent.width - parent.spacing
        Flickable {
            id: tabContent
            width: parent.width
            height: parent.height - 30
            ConnectionsListWidget{
                widgetHeight: parent.height
                widgetWidth: parent.width
            }

        }
    }
}
