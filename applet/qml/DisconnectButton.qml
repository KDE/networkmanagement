import QtQuick 1.0
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets
import org.kde.qtextracomponents 0.1

Item {
    id: disconnectButton
    
    width: 16
    height: 16
    
    signal hoverButton()
    
    QIconItem {
        id: menuIconElement

        icon: QIcon("user-offline")
        width: parent.width
        height: parent.height
        visible: true

    }
    
    MouseArea {
        id: buttonMouseArea
        anchors.fill: parent
        hoverEnabled: true
        
        onEntered: {parent.opacity = 0.7; hoverButton()}
        
        onExited:  parent.opacity = 1.0
    }
    
    scale: buttonMouseArea.pressed ? 0.90 : 1.00
}