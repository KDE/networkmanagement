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
                
    PlasmaComponents.TabBar {
        width: 300
        height: 30
        anchors.horizontalCenter: parent.horizontalCenter
        PlasmaComponents.TabButton { tab: tab1content; text: "Connections"}
        PlasmaComponents.TabButton { tab: tab2content; text: "VPN"}
        PlasmaComponents.TabButton { tab: tab3content; text: "Shared Connections"}
    }
    
    PlasmaComponents.TabGroup {
        height: 170
        width: parent.width - parent.spacing
        Flickable {
            id: tab1content
            width: parent.width
            height: 170
            ConnectionsListWidget{
                widgetHeight: parent.height
                widgetWidth: parent.width
            }

        }
        Text {
            id: tab2content
            text: "VPN Tab"
            horizontalAlignment: "AlignHCenter"
            verticalAlignment: "AlignVCenter"
        }
        Text {
            id: tab3content
            text: "Shared Connections Tab"
            horizontalAlignment: "AlignHCenter"
            verticalAlignment: "AlignVCenter"
        }
    }
} 
