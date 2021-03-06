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

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents


Item {
    id: listWidget

    property int widgetHeight
    property int widgetWidth

    signal showTraffic(int index)

    height: 290

    ListView {
        id: scrollList

        spacing: 12
        anchors.fill: parent
        clip: true
        model: interfacesListModel
        delegate: InterfaceItem {
            //width: parent.width - 10
            interfaceType: type
            name: interfaceName
            enabledInterface: interfaceEnabled
            connection: interfaceConnection
            isDefault: defaultRoute
            deviceUni: interfaceDeviceUni
            visibleInterface: interfaceVisible
            widgetIcon: interfaceIcon

            Component.onCompleted: {
                listWidget.height = Math.max(290, scrollList.count * (height + scrollList.spacing))
            }
            onInterfaceClicked: {
                showTraffic(index)
            }
        }

        Rectangle {
            anchors.fill: parent
            color: "#00000000"
        }
    }
}
