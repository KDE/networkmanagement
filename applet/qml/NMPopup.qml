/***************************************************************************
 *                                                                         *
 *   Copyright 2012 Lamarque V. Souza <lamarque@kde.org>                   *
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
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets
import org.kde.qtextracomponents 0.1
import InterfaceDetails 0.1
import Separator 0.1

Item {
    id: main
    property int minimumWidth: 320
    property int minimumHeight: 290
    property string previousState;

    property int iconSize: 22
        
    signal enableWireless(bool status)
    signal enableMobile(bool status)
    signal settingsClicked()
    signal noDeviceSelected()
    signal adjustSize(int width, int height)
    
    function showDetailsWidget() {
        previousState = main.state;
        main.state = "HideInterfaceList";
    }

    Component.onCompleted: {
    }

    PlasmaCore.Svg {
        id: iconsSvg
        imagePath: "widgets/configuration-icons"
    }


    onMinimumWidthChanged: {
        adjustSize(minimumWidth, minimumHeight)
    }
    
    onMinimumHeightChanged: {
        adjustSize(minimumWidth, minimumHeight)
    }

    Row {
        anchors.horizontalCenter: parent.horizontalCenter

        Column {
            id: leftColumn
            anchors.top: parent.top
            anchors.topMargin: 5
            anchors.left: parent.left
            anchors.leftMargin: 10
            spacing: 10
            opacity: 0
    
            PlasmaComponents.Label {
                id: interfaceTitle
                text: i18n("Interfaces")
                font.weight : Font.Bold
                font.pixelSize: theme.defaultFont.pointSize + 6
            }
            //WirelessInterfaceItem{}
            //WiredInterfaceItem{}
            //VpnInterfaceItem {}
            
            InterfacesListWidget {
                id: interfaceList
                widgetHeight: 230
                widgetWidth: 260
                onShowTraffic: {
                    interfacesListModel.loadTraffic(index);
                }
            }
            InterfaceDetailsWidget {
                id: interfaceDetails
                objectName: "interfaceDetails"
                visible: false
                onVisibleChanged: {
                    setUpdateEnabled(visible);
                }
                onBack: {
                    console.log("previousState1 is: " + previousState);
                    main.state = previousState;
                    noDeviceSelected();
                }    
                onDisconnectInterfaceRequested: {
                    connectionsListModel.deactivateConnection(deviceUni);
                }
            }
        }
    
        Column {
            id: separator
            visible: false
            width: 8
            height: leftColumn.height
            Separator {
                orientation: Qt.Vertical
                height: parent.height - 2*4
            }
        }
    
        Column {
            anchors.right: parent.right
            anchors.rightMargin: 10
            anchors.leftMargin: 10
            spacing: 2
    
            ConnectionsTabWidget {
                id: tabWidget
                height: 200;
            }
    
            PlasmaComponents.CheckBox {
                height: 30
                text: i18n("Enable Wireless")
    
                visible: wirelessVisible
                
                onCheckedChanged: {
                    enableWireless(checked)
                }
                checked: wirelessChecked
                enabled: wirelessEnabled
                
            }
    
            PlasmaComponents.CheckBox {
                height: 30
                text: i18n("Enable Mobile Broadband")
                
                visible: mobileVisible
                checked: mobileChecked
                enabled: mobileEnabled
                
                onCheckedChanged: {
                    enableMobile(checked)
                }
            }
    
            Row {
                spacing: 10
                PlasmaComponents.ToolButton {
                    text: i18n("Settings...")
                    iconSource: "configure"
                    onClicked: {
                        settingsClicked();
                    }
                }
                PlasmaComponents.ToolButton {
                    id: showConnectionButton
                    text: i18n("Show Connections")
                    iconSource: "format-list-unordered"
                    onClicked: {
                        if(main.state != "ShowInterfaceList" && main.state != "HideInterfaceList") {
                            main.state = "ShowInterfaceList"
                        } else {
                            main.state = "InitialState"
                            noDeviceSelected();
                            minimumWidth: 320
                            minimumHeight: 290
                            width: 320
                            adjustSize(320, 290)
                        }
                    }
                }
            }
        }
    }

    states: [
        State {
            name: "InitialState"

            PropertyChanges {
                target: interfaceDetails
                visible: false
            }
            PropertyChanges {
                target: interfaceTitle
                visible: false
            }
            PropertyChanges {
                target: interfaceList
                visible: false
            }

            PropertyChanges {
                target: separator
                visible: false
            }
        },

        State {
            name: "ShowInterfaceList"

            PropertyChanges {
                target: showConnectionButton
                text: i18n("Hide Connections")
            }

            PropertyChanges {
                target: main
                minimumWidth: 600
                minimumHeight: 290
//                adjustSize(600, 290)
            }

            PropertyChanges {
                target: leftColumn
                opacity: 1
            }
            PropertyChanges {
                target: interfaceDetails
                visible: false
            }
            
            PropertyChanges {
                target: interfaceList
                visible: true
            }
            
            PropertyChanges {
                target: interfaceTitle
                visible: true
            }

            PropertyChanges {
                target: separator
                visible: true
            }
        },

        State {
            name: "HideInterfaceList"

            PropertyChanges {
                target: showConnectionButton
                text: i18n("Hide Connections")
            }

            PropertyChanges {
                target: main
                minimumWidth: 650
                minimumHeight: 370
//                adjustSize(650, 370)
            }

            PropertyChanges {
                target: leftColumn
                opacity: 1
            }
            
            PropertyChanges {
                target: interfaceDetails
                visible: true
            }
            
            PropertyChanges {
                target: interfaceList
                visible: false
            }
            
            PropertyChanges {
                target: interfaceTitle
                visible: false
            }
            
            PropertyChanges {
                target: tabWidget
                height: 260
            }

            PropertyChanges {
                target: separator
                visible: true
            }
        }
    ]

}
