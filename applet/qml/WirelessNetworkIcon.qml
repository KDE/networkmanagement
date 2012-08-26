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
import org.kde.qtextracomponents 0.1

Item {
    id: wirelessIcon

    width: 16
    height: 16

    property string status
    property bool route

    QIconItem {
        id: connectionIcon

        // TODO: change icon depending on signal strength, like in the C++ plasmoid.
        icon: QIcon("network-wireless-connected-100")
        width: 16
        height: 16
        visible: true
        anchors.verticalCenter: parent.verticalCenter
    }

    QIconItem {
        id: connectionIcon2

        icon: QIcon("network-defaultroute.png")
        width: 8
        height: 8
        visible: route
        anchors.verticalCenter: parent.verticalCenter
    }
}
