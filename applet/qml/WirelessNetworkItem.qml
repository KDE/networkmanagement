import QtQuick 1.0
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.qtextracomponents 0.1

Item {
    id: wirelessItem
    width: 300
    height: 30
    
    property string networkName: "Wireless Network 1"
    property double signalStrengthValue: 0
    property bool protectedNetwork: true
    
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

                icon: QIcon("network-wireless-connected-100")
                width: 16
                height: 16
                visible: true
                anchors.verticalCenter: parent.verticalCenter
            }

            Text {
                text: networkName
            }
        }
     
        Row {
            id: row3
            height: parent.heght - 20
            anchors.right: parent.right
            spacing: 10
            anchors.verticalCenter: parent.verticalCenter
         
            PlasmaComponents.ProgressBar {
                id: signalStrength
                
                orientation: Qt.Horizontal
                minimumValue: 0
                maximumValue: 100
                value: signalStrengthValue
                height: 10
                width: 68
                
                anchors.verticalCenter: parent.verticalCenter
            }
            
            QIconItem {
                id: protectIcon

                icon: (protectedNetwork) ? QIcon("security-high") : QIcon("security-low")
                width: 22
                height: 22
                visible: true
                anchors.verticalCenter: parent.verticalCenter
            }

            DisconnectButton {
                anchors.verticalCenter: parent.verticalCenter
                onHoverButton: {
                    shadow.state = "hover"
                }
            }

        }
    }


/**
    MouseArea {
        id: wirelessWidgetArea
        hoverEnabled: true
        anchors.fill: parent
    }
    
    **/
}
