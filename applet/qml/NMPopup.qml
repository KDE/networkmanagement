/***************************************************************************
 *                                                                         *
 *   Copyright 2012 Lamarque V. Souza <lamarque@kde.org>                   *
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
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets
import org.kde.qtextracomponents 0.1
import InterfaceDetails 0.1
import Separator 0.1

Item {
    id: main

    property int minimumWidth: Math.max(320, mainRow.width + 2*4)
    property int minimumHeight: Math.max(290, mainRow.height + 2*4)
    property string previousState
    property int iconSize: 22

    function updateSize() {
        adjustSize(minimumWidth, minimumHeight)
    }

    signal enableWireless(bool status)
    signal enableMobile(bool status)
    signal settingsClicked()
    signal noDeviceSelected()
    signal adjustSize(int width, int height)

    Component.onCompleted: {
        main.state = warningLabel === "" ? "InitialState" : "ShowInterfaceList"
    }

    onMinimumWidthChanged: {
        adjustSize(minimumWidth, minimumHeight)
    }

    onMinimumHeightChanged: {
        adjustSize(minimumWidth, minimumHeight)
    }

    function showDetailsWidget() {
        previousState = main.state
        main.state = "ShowInterfaceDetails"
    }

    PlasmaCore.Svg {
        id: iconsSvg
        imagePath: "widgets/configuration-icons"
    }

    Row {
        id: mainRow

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 8
        anchors.bottomMargin: 4
        spacing: 4

        Column {
            id: leftColumn
            anchors.top: parent.top
            anchors.leftMargin: 8
            spacing: 4
            visible: false

            PlasmaComponents.Label {
                id: interfaceTitle
                visible: false
                text: warningLabel !== "" ? warningLabel : i18n("Interfaces")
                font.weight: Font.Bold
                font.pixelSize: theme.defaultFont.pointSize + 6

                onTextChanged: {
                    if (warningLabel !== "") {
                        main.state = "ShowInterfaceList"
                    }
                }
            }

            InterfacesListWidget {
                id: interfaceList
                visible: false
                width: 260
                onShowTraffic: {
                    interfacesListModel.loadTraffic(index)
                }
            }
            InterfaceDetailsWidget {
                id: interfaceDetails
                visible: false
                objectName: "interfaceDetails"
                onVisibleChanged: {
                    setUpdateEnabled(visible)
                }
                onBack: {
                    console.log("previousState is: " + previousState)
                    main.state = previousState
                    noDeviceSelected()
                }
                onDisconnectInterfaceRequested: {
                    connectionsListModel.deactivateConnection(deviceUni)
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
            id: rightColumn

            height: leftColumn.visible ? Math.max(tabWidget.height + controls.height + 28, leftColumn.height) : (tabWidget.height + controls.height + 28)
            spacing: 8

            ConnectionsTabWidget {
                id: tabWidget
                height: leftColumn.visible ? (leftColumn.height - controls.height - 28) : 208
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Column {
                id: controls
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 4
                width: parent.width
                spacing: 4

                PlasmaComponents.CheckBox {
                    text: i18n("Enable Wireless")

                    visible: wirelessVisible
                    checked: wirelessChecked
                    enabled: wirelessEnabled

                    onCheckedChanged: {
                        enableWireless(checked)
                    }
                }

                PlasmaComponents.CheckBox {
                    text: i18n("Enable Mobile Broadband")

                    visible: mobileVisible
                    checked: mobileChecked
                    enabled: mobileEnabled

                    onCheckedChanged: {
                        enableMobile(checked)
                    }
                }

                Row {
                    width: parent.width

                    PlasmaComponents.ToolButton {
                        text: i18n("Settings...")
                        iconSource: "configure"
                        anchors.verticalCenter: parent.verticalCenter
                        onClicked: {
                            settingsClicked()
                        }
                    }
                    PlasmaComponents.ToolButton {
                        id: showConnectionButton
                        text: i18n("Show Interfaces")
                        iconSource: "format-list-unordered"
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        onClicked: {
                            if (main.state !== "ShowInterfaceList" && main.state !== "ShowInterfaceDetails") {
                                main.state = "ShowInterfaceList"
                            } else {
                                main.state = "InitialState"
                            }
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
                target: showConnectionButton
                text: i18n("Show Interfaces")
            }

            PropertyChanges {
                target: leftColumn
                visible: false
            }
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
                text: i18n("Hide Interfaces")
            }

            PropertyChanges {
                target: leftColumn
                visible: true
            }
            PropertyChanges {
                target: interfaceDetails
                visible: false
            }
            PropertyChanges {
                target: interfaceTitle
                visible: true
            }
            PropertyChanges {
                target: interfaceList
                visible: true
            }

            PropertyChanges {
                target: separator
                visible: true
            }
        },

        State {
            name: "ShowInterfaceDetails"

            PropertyChanges {
                target: showConnectionButton
                text: i18n("Hide Interfaces")
            }

            PropertyChanges {
                target: leftColumn
                visible: true
            }
            PropertyChanges {
                target: interfaceDetails
                visible: true
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
                visible: true
            }
        }
    ]
}
