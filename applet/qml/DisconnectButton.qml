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
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets
import org.kde.qtextracomponents 0.1

Item {
    id: disconnectButton
    
    property bool status;
    
    width: 16
    height: 16
    opacity: (status) ? 1 : 0.2
    
    
    property alias showIcon: menuIconElement.visible

    signal hoverButton()
    signal disconnectClicked()
    
    QIconItem {
        id: menuIconElement

        icon: QIcon("user-offline")
        width: parent.width
        height: parent.height

    }
    
    
    
    MouseArea {
        id: buttonMouseArea
        anchors.fill: parent
        hoverEnabled: true
        
        onEntered: {
            if(status) {
                parent.opacity = 0.7; 
            }
            hoverButton();
        }
        
        onExited:  {
            if(status) {
                parent.opacity = 1.0
            }
        }
        
        onClicked: {
            disconnectClicked();
        }
    }
    
    scale: {
        if(status) {
            buttonMouseArea.pressed ? 0.90 : 1.00
        }
    }
}
