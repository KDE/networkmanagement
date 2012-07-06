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
    
    property string networkUuid;
    property string wiredNetworkName;
    property string wirelessNetworkName;
    property string status;
    property double wirelessSignalStrength;
    property bool networkConnected;
    property string wirelessNetworkIcon;
    property string connectionType;
    property bool hidden;
    
    signal disconnectNetwork(string uuidProperty)
    signal connectNetwork(int index)
    
    WiredNetworkItem {
        id: wiredItem
        
        width: parent.width;
        uuid: networkUuid;
        networkName: wiredNetworkName;
        connected: networkConnected;
        wiredStatus: status;
        onDisconnect: {
            disconnectNetwork(uuid);
        }
        onConnectionClicked: {
            connectNetwork(index);
        }
        visible: false
    }
    
    HiddenWirelessNetwork {
        id: hiddenItem
        width: parent.width;
        visible: false
    }
    
    WirelessNetworkItem {
        id: wirelessItem
        width: parent.width;
        uuid: networkUuid;
        networkName: wirelessNetworkName;
        signalStrengthValue: wirelessSignalStrength;
        protectedNetworkIcon: wirelessNetworkIcon;
        connected: networkConnected;
        wifiStatus: status;
        onDisconnect: {
            console.log("uuid eh: " + uuid);
            disconnectNetwork(uuid);
        }
        onConnectionClicked: {
            connectNetwork(index);
        }
        visible: false
    }
    
    Component.onCompleted: {
        if (connectionType == "wireless") {
            if(hidden) {
                hiddenItem.visible = true;
            } else {
                wirelessItem.visible = true;
            }
        } else if (connectionType == "wired") {
            wiredItem.visible = true;
        }
    }
}
