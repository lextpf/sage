import QtQuick

Rectangle {
    id: root

    property string statusText: "Ready"
    property bool fillArmed: false

    implicitHeight: 36
    gradient: Gradient {
        GradientStop { position: 0; color: Theme.bgFooterTop }
        GradientStop { position: 1; color: Theme.bgFooterEnd }
    }

    // Top border
    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 1
        color: Theme.borderDim
    }

    Text {
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: 20
        text: root.statusText
        font.family: Theme.fontFamily
        font.pixelSize: Theme.fontSizeSmall
        font.weight: Font.Medium
        color: Theme.textSubtle
    }

    Rectangle {
        id: armedIndicator
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: 20
        width: 8
        height: 8
        radius: 4
        color: Theme.fillArmedDot
        visible: root.fillArmed

        SequentialAnimation on opacity {
            running: armedIndicator.visible
            loops: Animation.Infinite
            NumberAnimation { to: 0.3; duration: 800; easing.type: Easing.InOutSine }
            NumberAnimation { to: 1.0; duration: 800; easing.type: Easing.InOutSine }
        }
    }
}
