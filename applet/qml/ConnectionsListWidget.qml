/***************************************************************************
 *                                                                         *
 *   Copyright 2012 Arthur de Souza Ribeiro <arthurdesribeiro@gmail.com>   *
 *   Copyright 2012-2013 Lamarque V. Souza <lamarque@kde.org>              *
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

    height: widgetHeight
    width: widgetWidth

    ListView {
        id: scrollList

        anchors.fill: parent
        clip: true
        model: connectionsListModel
        spacing: 4
        delegate: ConnectionItem {
            id: connectionItem
            width: parent.width - 20
            networkUuid: connectionUuid
            connectionName: networkType == "wirelessNetwork" ? ssid : model["connectionName"]
            connectionType: networkType
            hidden: hiddenNetwork
            wirelessSignalStrength: strength
            wirelessNetworkIcon: protectedIcon
            activationState: model["activationState"]
            isHovered: hoverEntered
            connectionIcon: model["connectionIcon"]
            defaultRoute: hasDefaultRoute
            signalQuality: model["signalQuality"]
            accessTechnology: model["accessTechnology"]
            shoreMoreChecked: model["showMoreChecked"]
            networkCount: model["networkCount"]
            onDisconnectNetwork: {
                connectionsListModel.disconnectFrom(uuidProperty)
            }
            onConnectNetwork: {
                connectionsListModel.connectTo(index)
            }
            onConnectToHiddenNetwork: {
                connectionsListModel.connectToHiddenNetwork(ssid)
            }
            onShowMoreClicked: {
                connectionsListModel.showMoreClicked()
            }
        }

        Rectangle {
            anchors.fill: parent
            color: "#00000000"
        }

        PlasmaComponents.ScrollBar {
            id: scrollBar
            orientation: Qt.Vertical
            flickableItem: scrollList
            //stepSize: 40
            scrollButtonInterval: 50
            anchors {
                top: scrollList.top
                right: scrollList.right
                bottom: scrollList.bottom
            }
        }
    }
}

