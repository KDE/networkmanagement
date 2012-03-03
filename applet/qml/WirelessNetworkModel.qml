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

ListModel {
    ListElement {
        modelValue: 50
        modelNetworkName: "WLAN 0"
        modelProtectedNetwork: true
    }
    ListElement {
        modelValue: 60
        modelNetworkName: "WLAN 1"
        modelProtectedNetwork: true
    }
    ListElement {
        modelValue: 40
        modelNetworkName: "WLAN 2"
        modelProtectedNetwork: false
    }
    ListElement {
        modelValue: 40
        modelNetworkName: "WLAN 3"
        modelProtectedNetwork: true
    }
    ListElement {
        modelValue: 40
        modelNetworkName: "WLAN 4"
        modelProtectedNetwork: true
    }
    ListElement {
        modelValue: 40
        modelNetworkName: "WLAN 5"
        modelProtectedNetwork: false
    }
    ListElement {
        modelValue: 40
        modelNetworkName: "WLAN 6"
        modelProtectedNetwork: true
    }
}
