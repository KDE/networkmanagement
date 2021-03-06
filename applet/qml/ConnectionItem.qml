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

Item {
    id: connectionItem
    width: parent.width
    height: 32

    property string networkUuid
    property string connectionName
    property string activationState
    property string connectionIcon
    property double wirelessSignalStrength
    property string wirelessNetworkIcon
    property string connectionType
    property bool hidden
    property bool isHovered
    property bool defaultRoute
    property int signalQuality
    property string accessTechnology
    property bool shoreMoreChecked
    property int networkCount

    signal disconnectNetwork(string uuidProperty)
    signal connectNetwork(int index)
    signal connectToHiddenNetwork(string ssid)
    signal showMoreClicked()

    Component {
        id: wiredNetworkItemComponent

        WiredNetworkItem {
            id: wiredItem

            width: connectionItem.width
            uuid: connectionItem.networkUuid
            connectionName: connectionItem.connectionName
            activationState: connectionItem.activationState
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
                connectionItem.connectToHiddenNetwork(connectionName)
            }
        }
    }

    Component {
        id: wirelessNetworkItemComponent

        WirelessNetworkItem {
            id: wirelessItem

            width: connectionItem.width
            uuid: connectionItem.networkUuid
            connectionName: connectionItem.connectionName
            connectionIcon: connectionItem.connectionIcon
            signalStrength: connectionItem.wirelessSignalStrength
            protectedNetworkIcon: connectionItem.wirelessNetworkIcon
            activationState: connectionItem.activationState
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
            connectionName: connectionItem.connectionName
            connectionIcon: connectionItem.connectionIcon
            activationState: connectionItem.activationState
            hoverEnter: connectionItem.isHovered
            onDisconnect: {
                connectionItem.disconnectNetwork(uuid)
            }
            onConnectionClicked: {
                connectionItem.connectNetwork(index)
            }
        }
    }

    Component {
        id: gsmNetworkItemComponent

        GsmNetworkItem {
            id: gsmItem

            width: connectionItem.width
            uuid: connectionItem.networkUuid
            connectionName: connectionItem.connectionName
            connectionIcon: connectionItem.connectionIcon
            activationState: connectionItem.activationState
            signalQuality: connectionItem.signalQuality
            accessTechnology: connectionItem.accessTechnology
            hoverEnter: connectionItem.isHovered
            onDisconnect: {
                connectionItem.disconnectNetwork(uuid)
            }
            onConnectionClicked: {
                connectionItem.connectNetwork(index)
            }
        }
    }

    Component {
        id: showMoreItemComponent

        ShowMoreItem {
            id: showMoreItem

            width: connectionItem.width
            hoverEnter: connectionItem.isHovered
            checked: connectionItem.shoreMoreChecked
            networkCount: connectionItem.networkCount
        }
    }

    Component.onCompleted: {
        if (connectionType == "wireless" || connectionType == "wirelessNetwork") {
            if (hidden) {
                hiddenWirelessNetworkComponent.createObject(connectionItem)
            } else {
                wirelessNetworkItemComponent.createObject(connectionItem)
            }
        } else if (connectionType == "wired") {
            wiredNetworkItemComponent.createObject(connectionItem)
        } else if (connectionType == "vpn") {
            vpnNetworkItemComponent.createObject(connectionItem)
        } else if (connectionType == "gsm") {
            gsmNetworkItemComponent.createObject(connectionItem)
        } else if (connectionType == "showMore") {
            showMoreItemComponent.createObject(connectionItem)
        } else {
            console.log("Unhandled connection type " + connectionType)
        }
    }
}
