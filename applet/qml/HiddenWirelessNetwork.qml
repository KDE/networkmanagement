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
    id: hiddenWirelessItem
    width: 300
    height: 30
    
    property string uuid;
    property string wifiStatus;
    property bool connected;
    property string protectedNetworkIcon;
    
    signal enterPressed(string networkName);
    
    Rectangle {
        id: content
        
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
            
            onClicked: parent.parent.state = "State1"
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
                id: icon
                status: "not connected"
            }

            PlasmaComponents.Label {
                id: description
                text: i18n("<hidden network>")
            }
        }
 
    }
    
    PlasmaComponents.TextField {
        id: networkNameEdit
        placeholderText: i18n("Enter network name and press <enter>")
        anchors.leftMargin: 2
        anchors.rightMargin: 2
        anchors.fill: parent
        visible: false
        onAccepted: {
            if(networkNameEdit.text != "") {
                enterPressed(networkNameEdit.text);
                networkNameEdit.text = "";
            }
            hiddenWirelessItem.state = "base state"
        }
    }
        
    states: [
        State {
            name: "State1"

            PropertyChanges {
                target: networkNameEdit
                visible: true
            }
            
            PropertyChanges {
                target: content
                visible: false
            }
            
            PropertyChanges {
                target: icon
                visible: false
            }
            
            PropertyChanges {
                target: description
                visible: false
            }
            
            PropertyChanges {
                target: row2
                visible: false
            }
        }
    ]
}