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

Item {
    id: connectionItem
    width: 300
    height: 30

    property string networkUuid
    property string connectionName
    property string status
    property string iconNetwork
    property double wirelessSignalStrength
    property string wirelessNetworkIcon
    property string connectionType
    property bool hidden
    property bool isHovered
    property bool defaultRoute

    signal disconnectNetwork(string uuidProperty)
    signal connectNetwork(int index)
    signal connectToHiddenNetwork(string ssid)

    Component {
        id: wiredNetworkItemComponent

        WiredNetworkItem {
            id: wiredItem

            width: connectionItem.width
            uuid: connectionItem.networkUuid
            networkName: connectionItem.connectionName
            wiredStatus: connectionItem.status
            hoverEnter: connectionItem.isHovered
            routeDefault: connectionItem.defaultRoute

            onDisconnect: {
                connectionItem.disconnectNetwork(uuid)
            }
            onConnectionClicked: {
                connectionItem.connectNetwork(index)
            }
        }
    }

    Component {
        id: hiddenWirelessNetworkComponent

        HiddenWirelessNetwork {
            id: hiddenItem
            width: connectionItem.width
            onEnterPressed: {
                connectionItem.connectToHiddenNetwork(networkName)
            }
        }
    }

    Component {
        id: wirelessNetworkItemComponent

        WirelessNetworkItem {
            id: wirelessItem

            width: connectionItem.width
            uuid: connectionItem.networkUuid
            networkName: connectionItem.connectionName
            signalStrengthValue: connectionItem.wirelessSignalStrength
            protectedNetworkIcon: connectionItem.wirelessNetworkIcon
            wifiStatus: connectionItem.status
            hoverEnter: connectionItem.isHovered
            routeDefault: connectionItem.defaultRoute

            onDisconnect: {
                connectionItem.disconnectNetwork(uuid)
            }
            onConnectionClicked: {
                connectionItem.connectNetwork(index)
            }
        }
    }

    Component {
        id: vpnNetworkItemComponent

        VpnNetworkItem {
            id: vpnItem

            width: connectionItem.width
            uuid: connectionItem.networkUuid
            networkName: connectionItem.connectionName
            wiredStatus: connectionItem.status
            hoverEnter: connectionItem.isHovered
            networkIcon: connectionItem.iconNetwork
            onDisconnect: {
                connectionItem.disconnectNetwork(uuid)
            }
            onConnectionClicked: {
                connectionItem.connectNetwork(index)
            }
        }
    }

    Component.onCompleted: {
        if (connectionType == "wireless") {
            if (hidden) {
                hiddenWirelessNetworkComponent.createObject(connectionItem)
            } else {
                wirelessNetworkItemComponent.createObject(connectionItem)
            }
        } else if (connectionType == "wired") {
            wiredNetworkItemComponent.createObject(connectionItem)
        } else if (connectionType == "vpn") {
            vpnNetworkItemComponent.createObject(connectionItem)
        }
    }
}
