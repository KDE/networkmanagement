import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents


Item {
    id: listWidget;

    property int widgetHeight;
    property int widgetWidth;

    height: widgetHeight
    width: widgetWidth

    ListView {
        id: scrollList

        width: parent.width
        height: parent.height
        clip: true
        model: WirelessNetworkModel {}
        delegate: WirelessNetworkItem {
            width: parent.width - 10
            networkName: modelNetworkName;
            signalStrengthValue: modelValue;
            protectedNetwork: modelProtectedNetwork
        }

        Rectangle {
            anchors.fill: parent
            color: "#00000000"
        }

        PlasmaComponents.ScrollBar {
            id: scrollBar
            orientation: Qt.Vertical
            flickableItem: scrollList
            //stepSize: 40
            scrollButtonInterval: 50
            anchors {
                top: scrollList.top
                right: scrollList.right
                bottom: scrollList.bottom
            }
        }
    }
}

