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
    id: vpnInterfaceItem
    anchors.fill: parent

    property string interfaceTitle;
    property string connectionDescription;
    property bool defaultRoute;

    signal vpnInterfaceClicked();
    signal hoverEnter();
    signal hoverLeft();

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

            onClicked: vpnInterfaceClicked();

            onEntered: {
                shadow.state = "hover";
                hoverEnter();
            }

            onExited: {
                shadow.state = "hidden"
                hoverLeft();
            }
        }
    }

    Row {
        width: parent.width
        height: parent.height
        anchors.right: parent.right
        anchors.rightMargin: 8
        anchors.left: parent.left
        anchors.leftMargin: 8
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter

        Row {
            id: row2
            anchors.verticalCenter: parent.verticalCenter
            spacing: 4

            QIconItem {
                id: connectionIcon

                icon: QIcon("secure-card")
                width: 48
                height: 48
                visible: true
                anchors.verticalCenter: parent.verticalCenter
            }

            Column {
                spacing: 4
                PlasmaComponents.Label {
                    text: interfaceTitle
                    font.weight: Font.Bold
                }
                PlasmaComponents.Label {
                    text: connectionDescription
                }
            }
        }
    }
}
