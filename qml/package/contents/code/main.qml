// -*- coding: iso-8859-1 -*-
/*
 *   Author: Sebastian Kügler <sebas@kde.org>
 *   Date: Sun Nov 7 2010, 18:51:24
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import Qt 4.7
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.graphicslayouts 4.7 as GraphicsLayouts

Item {
    width: 200
    height: 400
    anchors.fill: parent

    PlasmaCore.DataSource {
          id: networkEngineSource
          engine: "networkmanagement"
          interval: 0
          connectedSources: ["connections"]
          onDataChanged: {
              console.log("data changed...");
          }
          onSourceAdded: {
             if (source != "networkStatus") {
                //console.log("QML addedd ......." + source)
                connectSource(source)
             }
          }
          onSourceRemoved: {
             if (source != "networkStatus") {
                //console.log("QML removed ......." + source)
                disconnectSource(source)
             }
          }
          Component.onCompleted: {
              //connectedSources = sources
              //connectSource("connections")
          }
    }

    PlasmaCore.Theme {
        id: theme
    }

    PlasmaWidgets.Label {
        id: header
        text: i18n("Available Connections")
        font.pixelSize: 20
        anchors { top: parent.top; left: parent.left; right: parent.right; }
    }

    ListView {
        id: list
        //height: 200
        anchors { top: header.bottom; left: parent.left; right: parent.right; bottom: parent.bottom; margins: 8}
        clip: true
        spacing: 10

        model: PlasmaCore.DataModel {
            dataSource: networkEngineSource
        }

        delegate: Item {
            property int collapsedHeight: 38
            property int expandedHeight: 72
            property string iconString: (typeof securityIcon != "undefined") ? securityIcon : "security-low"
            //property string iconString: "security-medium";

            id: citem
            height: collapsedHeight
            width: parent.width
            anchors.margins: 4

            PlasmaWidgets.Frame {
                id: itemFrame
                anchors.fill: parent
                //anchors.top: citem.top
                //anchors.bottom: citem.bottom
                height: citem.height
                //width: citem.width
            }

            PlasmaWidgets.IconWidget {
                id: strengthIconWidget
                height: collapsedHeight
                width: collapsedHeight
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.margins: 4

                Component.onCompleted: {
                    //if (typeof networkEngineSource.data[DataEngineSource]["securityIcon"] != "undefined") {
                    //    setIcon(networkEngineSource.data[DataEngineSource]["securityIcon"]);
                    //} else {
                        setIcon("network-wireless");
                    //}
                }
                onClicked: citem.state = (citem.state == "expanded") ? "collapsed" : "expanded"

            }

            PlasmaWidgets.IconWidget {
                id: "securityIconWidget"
                height: collapsedHeight
                width: collapsedHeight
                //height: 40
                //width: 50
                anchors.top: citem.top
                anchors.right: citem.right
                //anchors.left: mainText.right
                //anchors.left: parent.left
                //anchors.bottom: parent.bottom
                scale: 0.8
                opacity: 0.3

                Component.onCompleted: {
                    try {
                        //if (typeof networkEngineSource.data[DataEngineSource]["securityIcon"] != "undefined") {
                            //setIcon(networkEngineSource.data[DataEngineSource]["securityIcon"]);
                            setIcon(securityIcon);
                        //}
                    } catch (TypeError) {
                        //print("oops");
                        print(" TypeError" + mainText.text);
                    } finally {
                        print(" Exception ignored in " + mainText.text);
                    }
                }
                onClicked: citem.state = (citem.state == "expanded") ? "collapsed" : "expanded"

            }

            PlasmaWidgets.Label {
                id: mainText
                text: {
                    print(" Hmm ... " + activatableType);
                    if (activatableType != "" && isInterfaceConnection(activatableType)) {
                        print("ifaceconnnection :)");
                        if (connectionName) {
                            var t = connectionName;
                            //if (networkEngineSource.data[DataEngineSource]["activationState"] == "Activated") {
                            //    t = t + " (Connected)";
                            //}
                            return t;
                        } else {
                            return "empty";
                        }
                    } else if ("WirelessNetwork" == activatableType) {
                        //return citem.ssid;
                        return "ssid";
                    } else {
                        return citem.activatableType;
                    }
                    return "empty";
                }

                anchors.top: parent.top
                anchors.left: strengthIconWidget.right
                anchors.right: parent.right
                //anchors.right: securityIconWidget.left
            }
            PlasmaWidgets.Label {
                id: infoText
                text: {
                    if (isInterfaceConnection(activatableType)) {
                        //var conType = networkEngineSource.data[DataEngineSource]["connectionType"];
                        try {
                            var conType = connectionType;
                            // Wired, Wireless, Gsm, Cdma, Vpn or Pppoe
                            if (conType == "Wireless") {
                                return i18n("Remembered Wifi Connection");
                            } else if (conType == "Wired") {
                                return i18n("Wired Connection");
                            } else if (conType == "Gsm") {
                                return i18n("Mobile Broadband");
                            } else if (conType == "Cdma") {
                                return i18n("Mobile Internet");
                            } else if (conType == "Pppoe") {
                                return i18n("Modem Connection");
                            } else if (conType == "Vpn") {
                                return i18n("Virtual Private Network");
                            }
                        } catch (ReferenceError) {
                            print("Exception caught in infoText.Label connectionType");
                        }
                    }
                    return "unknown type";
                }
                //text: connectionType
                font.pixelSize: 12
                opacity: 0.4
                anchors.top: mainText.bottom
                anchors.left: mainText.left
                anchors.right: parent.right
            }

            PlasmaWidgets.PushButton {
                id: connectButton
                text: "Connect"
                z: 100
                height: 0
                //setIcon("kmail")
                anchors.top: infoText.bottom
                anchors.bottom: parent.bottom
                anchors.right: securityIconWidget.left
                opacity: 0.0

                onClicked: {
                    console.log("activate: " + mainText.text)
                    //setIcon("network-connect")
                }
            }

            Component.onCompleted: {
                //console.log("item completed" + mainText.text + "|" + index);
            }

            states: [
                State {
                    id: expanded
                    name: "expanded";
                    //when: mouseArea.pressed
                    PropertyChanges {
                        target: citem
                        height: expandedHeight
                    }
                    PropertyChanges {
                        target: connectButton
                        //visible: true
                        opacity: 1.0
                    }
                    PropertyChanges {
                        //animation: buttonAnimation
                        target: securityIconWidget
                        scale: 1.0
                        opacity: 1.0
                    }
                },

                State {
                    id: collapsed
                    name: "collapsed";
                    PropertyChanges {
                        target: citem
                        height: collapsedHeight
                    }
                    PropertyChanges {
                        target: connectButton
                        opacity: 0.0
                    }
                    PropertyChanges {
                        //animation: buttonAnimation
                        scale: 0.8
                        target: securityIconWidget
                        opacity: 0.3
                    }
                }
            ]

            transitions: [
                Transition {
                    PropertyAnimation {
                        properties: "height,opacity,scale"
                        duration: 400;
                        easing.type: Easing.InOutElastic;
                        easing.amplitude: 2.0; easing.period: 1.5
                    }
                }
            ]

            MouseArea {
                anchors.fill: parent

                onClicked: {
                    citem.state = (citem.state == "expanded") ? "collapsed" : "expanded"
                    console.log("MA clicked, citem state" + citem.state );
                }
            }

            function iconForSignalStrength(strength) {
                if (strength > 80) {
                    return "network-wireless-connnected-100";
                } else if (strength > 60) {
                    return "network-wireless-connnected-75";
                } else if (strength > 30) {
                    return "network-wireless-connnected-50";
                } else if (strength > 10) {
                    return "network-wireless-connnected-25";
                }
                return "network-wireless-connnected-00";
            }

            function isInterfaceConnection(myType) {
                var cons = ["InterfaceConnection", "WirelessInterfaceConnection", "VpnInterfaceConnection", "GsmInterfaceConnection"];
                //print("T:" + myType + " " + cons.indexOf(myType));
                return cons.indexOf(myType) >= 0;
            }

        }
    }

    PlasmaWidgets.Label {
        id: statusText
        text: "Connected to some.network."
        anchors.top: list.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
    }
}
