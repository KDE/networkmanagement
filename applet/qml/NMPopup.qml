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

Item {
    id: main
    property int minimumWidth: 320
    property int minimumHeight: 280

    property int iconSize: 22

    Component.onCompleted: {
    }

    PlasmaCore.Svg {
        id: iconsSvg
        imagePath: "widgets/configuration-icons"
    }



    Column {
        id: leftColumn
        anchors.top: parent.top
        anchors.topMargin: 5
        anchors.left: parent.left
        anchors.leftMargin: 10
        spacing: 10
        opacity: 0

        PlasmaComponents.Label {
            text: i18n("Interfaces")
            font.weight : Font.Bold
            font.pixelSize: theme.defaultFont.pointSize + 6
        }
        WirelessInterfaceItem{}
        WiredInterfaceItem{}
        VpnInterfaceItem {}
    }

    Column {
        anchors.right: parent.right
        anchors.rightMargin: 20
        spacing: 10

        ConnectionsTabWidget {}

        PlasmaComponents.CheckBox {
            height: 30
            text: i18n("Enable Wireless")

            onCheckedChanged: {
                if (checked)
                    console.log("CheckBox checked");
                else
                    console.log("CheckBox unchecked");
            }
            onClicked: {
                console.log("CheckBox clicked");
            }
        }

        Row {
            spacing: 10
            PlasmaComponents.ToolButton {
                text: i18n("  Settings...")
                iconSource: "configure"
            }
            PlasmaComponents.ToolButton {
                id: showConnectionButton
                text: i18n("  Show Connections")
                iconSource: "format-list-unordered"
                onClicked: {
                    if(parent.parent.parent.state != "State1") {
                        parent.parent.parent.state = "State1"
                    } else {
                        parent.parent.parent.state = "base state"
                        minimumWidth: 320
                        width: 320
                    }
                }
            }
        }
    }

    states: [
        State {
            name: "State1"

            PropertyChanges {
                target: showConnectionButton
                text: i18n("   Hide Connections")
            }

            PropertyChanges {
                target: main
                minimumWidth: 600
            }

            PropertyChanges {
                target: leftColumn
                opacity: 1
            }
        }
    ]

}
