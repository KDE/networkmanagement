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
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.qtextracomponents 0.1

Item {
    id: wirelessItem
    width: 300
    height: 30
    
    property string uuid;
    property string networkName;
    property string wifiStatus;
    property double signalStrengthValue;
    property bool connected;
    property string protectedNetworkIcon;
    
    signal disconnect(string uuidProperty)
    signal connectionClicked(int index)
    
    Rectangle {
        anchors.leftMargin: 2
        anchors.rightMargin: 2
        anchors.fill: parent
        color: "#00000000"
        
        ButtonShadow {
            id: shadow
            anchors.fill: parent
            state: "hidden"
        }
        
        MouseArea {
            id: wirelessWidgetArea
            hoverEnabled: true
            anchors.fill: parent
            
            onEntered: shadow.state = "hover"
            
            onExited: shadow.state = "hidden"
            
            onClicked: connectionClicked(index);
        }
        
        
    }
    
    Row {
        width: parent.width
        height: parent.height
        anchors.right: parent.right
        anchors.rightMargin: 10
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        spacing: 10
        
        Row {
            id: row2
            height: parent.heght - 20
            anchors.verticalCenter: parent.verticalCenter
            spacing: 5
            
            WirelessNetworkIcon {
                status: wifiStatus
            }

            PlasmaComponents.Label {
                text: networkName
                font.weight: {
                    if(wifiStatus == "connected") Font.Bold 
                    else Font.Normal
                }
                font.italic: {
                    if(wifiStatus == "connecting") true 
                    else false
                }
            }
        }
     
        Row {
            id: row3
            height: parent.heght - 20
            anchors.right: parent.right
            spacing: 10
            anchors.verticalCenter: parent.verticalCenter
         
            PlasmaComponents.ProgressBar {
                id: signalStrength
                
                orientation: Qt.Horizontal
                minimumValue: 0
                maximumValue: 120
                value: signalStrengthValue
                height: 10
                width: 68
                
                visible: signalStrengthValue > 0
                anchors.verticalCenter: parent.verticalCenter
            }
            
            QIconItem {
                id: protectIcon

                icon: QIcon(protectedNetworkIcon)
                width: 22
                height: 22
                visible: true
                anchors.verticalCenter: parent.verticalCenter
            }

            DisconnectButton {
                anchors.verticalCenter: parent.verticalCenter
                onHoverButton: {
                    shadow.state = "hover"
                }
                
                onDisconnectClicked: {
                    disconnect(uuid);
                }
                
                status: connected
            }

        }
    }


/**
    MouseArea {
        id: wirelessWidgetArea
        hoverEnabled: true
        anchors.fill: parent
    }
    
    **/
}
