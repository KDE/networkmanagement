// -*- coding: iso-8859-1 -*-
/*
 *   Copyright 2011 Sebastian Kügler <sebas@kde.org>
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
    height: 200
    anchors.fill: parent

    PlasmaCore.DataSource {
          id: networkEngineSource
          engine: "networkmanagement"
          interval: 0
          connectedSources: ["connections"]
          onDataChanged: {
                console.log("data changed...")
          }
          onSourceAdded: {
            print ("source added:" + source);
             if (source != "networkStatus") {
                console.log("QML addedd ......." + source)
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
        anchors { top: parent.top; left: parent.left; right: parent.right }
        id: titleLabel
        text: i18n("<h2>Connect to the Internet</h2>")
        anchors.margins: 8
    }

    ListView {
        id: list
        //height: 300

        anchors.top: titleLabel.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: statusText.top
        //anchors.fill: parent
        clip: true
        spacing: 10

        model: PlasmaCore.DataModel {
            dataSource: networkEngineSource
        }

        delegate: Item {
            property int collapsedHeight: 38
            property int expandedHeight: 72
            //property string iconString: (typeof securityIcon != "undefined") ? securityIcon : "security-low"
            property string iconString: "security-medium";

            id: citem
            height: collapsedHeight
            width: parent.width
            anchors.margins: 4

            PlasmaWidgets.Frame {
                id: itemFrame
                anchors.top: citem.top
                anchors.bottom: citem.bottom
                height: citem.height
                width: citem.width
            }

            PlasmaWidgets.IconWidget {
                id: strengthIconWidget
                height: collapsedHeight
                width: collapsedHeight
                anchors.top: parent.top
                anchors.left: parent.left

                Component.onCompleted: {
                    //if (typeof networkEngineSource.data[DataEngineSource]["securityIcon"] != "undefined") {
                    //    setIcon(networkEngineSource.data[DataEngineSource]["securityIcon"]);
                    //} else {
                        setIcon(iconName);
                        print ("XXXXX we're done");
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
                            setIcon(networkEngineSource.data[DataEngineSource]["securityIcon"]);
                        //}
                    } catch (TypeError) {
                        //print("oops");
                        print(" TypeError" + mainText.text);
                    } finally {
                        //print(" Exception ignored in " + mainText.text);
                    }
                }
                onClicked: citem.state = (citem.state == "expanded") ? "collapsed" : "expanded"

            }

            Text {
                id: mainText
                text: {
                    if (isInterfaceConnection(activatableType)) {
                        //var t = networkEngineSource.data[DataEngineSource]["connectionName"];
                        var t = connectionName;
                        if (activationState == "Activated") {
                            t = t + " (Connected)";
                        }
                        return t;
                    } else if ("WirelessNetwork" == activatableType) {
                        return ssid;
                    } else {
                        return activatableType;
                    }
                }
                anchors.top: parent.top
                anchors.left: strengthIconWidget.right
                //anchors.right: securityIconWidget.left
            }
            Text {
                id: infoText;
                text: {
                    if (isInterfaceConnection(activatableType)) {
                        // Wired, Wireless, Gsm, Cdma, Vpn or Pppoe
                        if (connectionType == "Wireless") {
                            return i18n("Remembered Wifi Connection");
                        } else if (connectionType == "Wired") {
                            return i18n("Wired Connection");
                        } else if (connectionType == "Gsm") {
                            return i18n("Mobile Broadband");
                        } else if (connectionType == "Cdma") {
                            return i18n("Mobile Internet");
                        } else if (connectionType == "Pppoe") {
                            return i18n("Modem Connection");
                        } else if (connectionType == "Vpn") {
                            return i18n("Virtual Private Network");
                        }
                    }
                    return "";
                }
                //text: connectionType
                font.pixelSize: mainText.font.pixelSize - 2
                opacity: 0.4
                anchors.top: mainText.bottom
                anchors.left: mainText.left
                //anchors.bottom: parent.top
            }

            PlasmaWidgets.PushButton {
                id: connectButton
                text: i18n("Connect")
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
                console.log("XXXX item completed" + mainText.text + "|" + index);
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

            function isInterfaceConnection(activatableType) {
                var cons = ["InterfaceConnection", "WirelessInterfaceConnection", "VpnInterfaceConnection", "GsmInterfaceConnection"];
                //print(activatableType);
                return cons.indexOf(activatableType) >= 0;
            }
        }
    }

    Rectangle {
        anchors.fill: list
        color: theme.backgroundColor
        radius: 12
        opacity: 0.4
        border.width: 1
        border.color: theme.backgroundColor
        //border.opacity: 0.1
    }

    PlasmaWidgets.Label {
        id: statusText
        text: "statuslabel ....."
        //anchors.top: list.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
    }
}
