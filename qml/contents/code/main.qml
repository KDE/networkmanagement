// -*- coding: iso-8859-1 -*-
/*
 *   Author: Marco Martin <mart@kde.org>
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
              //console.log("data changed...")
          }
          onSourceAdded: {
             if (source != "networkStatus") {
                console.log("QML addedd ......." + source)
                connectSource(source)
             }
          }
          onSourceRemoved: {
             if (source != "networkStatus") {
                console.log("QML removed ......." + source)
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

    ListView {
        id: list
        //height: 200
        /*
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: statusText
        */
        anchors.fill: parent
        clip: true
        spacing: 10

        model: PlasmaCore.DataModel {
            dataSource: networkEngineSource
        }

        delegate: Item {
            property int collapsedHeight: 38
            property int expandedHeight: 96
            property string iconString: (typeof securityIcon != "undefined") ? securityIcon : "security-medium"
            //property string iconString: "security-medium";

            id: citem
            height: collapsedHeight
            anchors.margins: 4

            PlasmaWidgets.Frame {
                id: itemFrame
                anchors.fill: parent
            }

            PlasmaWidgets.IconWidget {
                id: strengthIconWidget
                height: collapsedHeight
                width: collapsedHeight
                anchors.top: parent.top
                anchors.left: parent.left

                Component.onCompleted: {
                    setIcon("network-wireless");
                }
                onClicked: citem.state = (citem.state == "expanded") ? "collapsed" : "expanded"

            }

            PlasmaWidgets.IconWidget {
                id: securityIconWidget
                height: collapsedHeight
                width: collapsedHeight
                anchors.top: parent.top
                anchors.right: parent.right
                //anchors.left: parent.left
                //anchors.bottom: parent.bottom
                opacity: .8

                Component.onCompleted: {
                    //securityIconWidget.setIcon(networkEngineSource.data[DataEngineSource]["securityIcon"]);
                    //console.log("new sec icon" + networkEngineSource.data[DataEngineSource]["securityIcon"]);
                    if (typeof networkEngineSource.data[DataEngineSource]["securityIcon"] != "undefined") {
                        setIcon: networkEngineSource.data[DataEngineSource]["securityIcon"];
                        print("ICON:" + networkEngineSource.data[DataEngineSource]["securityIcon"]);
                    }
                }
                onClicked: citem.state = (citem.state == "expanded") ? "collapsed" : "expanded"

            }

            Text {
                id: mainText
                text: networkEngineSource.data[DataEngineSource]["connectionName"];
                anchors.top: parent.top
                anchors.left: strengthIconWidget.right
                anchors.right: parent.right
            }
            Text {
                id: infoText;
                //text: "Wireless Network"
                //text: i18n("Security: ") + networkEngineSource.data[DataEngineSource]["securityToolTip"]
                text: {
                    if (typeof networkEngineSource.data[DataEngineSource]["connectionType"] != "undefined") {
                        return networkEngineSource.data[DataEngineSource]["connectionType"];
                    }
                    return "";
                }
                //text: connectionType
                font.pixelSize: mainText.font.pixelSize - 2
                opacity: 0.4
                anchors.top: mainText.bottom
                anchors.left: strengthIconWidget.right
                //anchors.bottom: parent.top
            }

            PlasmaWidgets.PushButton {
                id: connectButton
                text: "Connect"
                height: 0
                //setIcon: "kmail"
                anchors.top: infoText.bottom
                anchors.bottom: parent.bottom
                anchors.left: strengthIconWidget.right
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
                        opacity: .9
                    }
                    PropertyChanges {
                        target: securityIconWidget
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
                        target: securityIconWidget
                        opacity: 0.0
                    }
                }
            ]

            transitions: [
                Transition {
                    PropertyAnimation {
                        properties: "height"
                        duration: 500;
                        easing.type: Easing.InOutElastic;
                        easing.amplitude: 2.0; easing.period: 1.5
                    }
                },

                Transition {
                    PropertyAnimation {
                        //id: buttonAnimation;
                        properties: "opacity"
                        duration: 2000;
                        //easing.type: Easing.InOutElastic;
                        //easing.amplitude: 2.0; easing.period: 1.5
                    }
                }
            ]
            /*
            MouseArea {
                anchors.fill: parent

                onClicked: {
                    citem.state = (citem.state == "expanded") ? "collapsed" : "expanded"
                    console.log("MA clicked, citem state" + citem.state );
                }
            }
            */
        }
    }

    Text {
        id: statusText
        text: "statuslabel ....."
        anchors.top: list.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
    }
}
