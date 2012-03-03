import QtQuick 1.0
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.qtextracomponents 0.1

Item {
    id: wirelessInterfaceItem
    width: 240
    height: 50

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

            onEntered: shadow.state = "hover"

            onExited: shadow.state = "hidden"
        }


    }

    Row {
        width: parent.width
        height: parent.height
        anchors.right: parent.right
        anchors.rightMargin: 10
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        spacing: 10

        Row {
            id: row2
            height: parent.heght - 20
            anchors.verticalCenter: parent.verticalCenter
            spacing: 5

            QIconItem {
                id: connectionIcon

                icon: QIcon("network-wireless")
                width: 48
                height: 48
                visible: true
                anchors.verticalCenter: parent.verticalCenter
            }

            Column {
                spacing: 10
                Text {
                    text: "Wireless 802.11"
                    font.bold: true
                }
                Text {
                    text: i18n("Not connected")
                }
            }
        }
    }
}
 
