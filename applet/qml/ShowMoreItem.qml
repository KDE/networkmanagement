/***************************************************************************
 *                                                                         *
 *   Copyright 2013 Lamarque V. Souza <lamarque@kde.org>                   *
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
    id: main
    anchors.fill: parent

    property bool hoverEnter
    property bool checked: false
    property int networkCount: 0

    signal showMoreClicked()

    onCheckedChanged: {
        console.log("Lamarque " << checked);
    }

    onHoverEnterChanged: {
        if (hoverEnter) {
            shadow.state = "hover"
        } else {
            shadow.state = "hidden"
        }
    }

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
            hoverEnabled: true
            anchors.fill: parent

            onEntered: shadow.state = "hover"

            onExited: shadow.state = "hidden"

            onClicked: {
                connectionItem.showMoreClicked()
            }
        }
    }

    Row {
        id: mainRow
        anchors.right: parent.right
        anchors.rightMargin: 8
        anchors.left: parent.left
        anchors.leftMargin: 8
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        spacing: 8

        Row {
            id: row2
            height: parent.heght - 20
            anchors.verticalCenter: parent.verticalCenter
            spacing: 4

            ShowMoreIcon {
                id: icon
                anchors.verticalCenter: parent.verticalCenter
                checked: main.checked
            }

            PlasmaComponents.Label {
                text: main.checked ? i18nc("text when this item is toggled", "Show less networks...") : i18ncp("text when this item not toggled", "Show one more network", "Show %1 more networks...", networkCount)
                elide: Text.ElideRight
                width: mainRow.width - icon.width - 8
            }
        }
    }
}
