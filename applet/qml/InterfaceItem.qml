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
    id: interfaceItem
    width: 240
    height: 50
    
    property string interfaceType;
    property string name;
    property string connection;
    property bool enabledInterface;
    property bool isDefault;
        
    Component {
        id: wiredInterfaceComponent

        WiredInterfaceItem {
            id: wiredItem

            width: interfaceItem.width;
            opacity: enabledInterface ? 1.0 : 0.7
            connectionDescription: connection
            interfaceTitle: name
            defaultRoute: isDefault;
        }
    }
    
    Component {
        id: modemInterfaceComponent

        ModemInterfaceItem {
            id: modemItem
            width: interfaceItem.width;
            
            interfaceTitle: name
            opacity: enabledInterface ? 1.0 : 0.7
            connectionDescription: connection
            //defaultRoute: isDefault;
        }
    }
    
    Component {
       id: wirelessInterfaceComponent

       WirelessInterfaceItem {
           id: wirelessItem

           width: interfaceItem.width;
           interfaceTitle: name
           opacity: enabledInterface ? 1.0 : 0.7
           connectionDescription: connection
           defaultRoute: isDefault;
       }
    }
    
    Component.onCompleted: {
        if (interfaceType == "wifi") {
            wirelessInterfaceComponent.createObject(interfaceItem);
        } else if (interfaceType == "wired") {
            wiredInterfaceComponent.createObject(interfaceItem);
        } else if (interfaceType == "modem") {
            modemInterfaceComponent.createObject(interfaceItem);
        }
    }
}
