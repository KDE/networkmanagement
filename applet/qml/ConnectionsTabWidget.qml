import QtQuick 1.0
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents

Column {
                
    PlasmaComponents.TabBar {
        width: 300
        height: 30
        anchors.horizontalCenter: parent.horizontalCenter
        PlasmaComponents.TabButton { tab: tab1content; text: "Connections"}
        PlasmaComponents.TabButton { tab: tab2content; text: "VPN"}
        PlasmaComponents.TabButton { tab: tab3content; text: "Shared Connections"}
    }
    
    PlasmaComponents.TabGroup {
        height: 170
        width: parent.width - parent.spacing
        Flickable {
            id: tab1content
            width: parent.width
            height: 170
            ConnectionsListWidget{
                widgetHeight: parent.height
                widgetWidth: parent.width
            }

        }
        Text {
            id: tab2content
            text: "VPN Tab"
            horizontalAlignment: "AlignHCenter"
            verticalAlignment: "AlignVCenter"
        }
        Text {
            id: tab3content
            text: "Shared Connections Tab"
            horizontalAlignment: "AlignHCenter"
            verticalAlignment: "AlignVCenter"
        }
    }
} 
