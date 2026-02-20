import QtQuick
import QtQuick.Layouts

Item {
    id: root

    required property int index
    required property string platform
    required property string maskedUsername
    required property string maskedPassword
    required property int recordIndex
    required property bool selected
    property bool isHovered: mouseArea.containsMouse

    signal clicked()

    implicitHeight: 48

    Rectangle {
        anchors.fill: parent
        color: root.selected ? Theme.selectionActive
             : root.isHovered ? Theme.selectionHover
             : "transparent"
        border.width: 0

        Behavior on color { ColorAnimation { duration: Theme.hoverDuration } }

        // Bottom separator
        Rectangle {
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            height: 1
            color: Theme.borderDim
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 14
            anchors.rightMargin: 14
            spacing: 0

            // Platform column
            Text {
                Layout.preferredWidth: 200
                text: root.platform
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeMedium
                color: root.isHovered ? Theme.accentBright : Theme.accent
                elide: Text.ElideRight
                Behavior on color { ColorAnimation { duration: Theme.hoverDuration } }
            }

            // Username column
            Text {
                Layout.preferredWidth: 200
                text: root.maskedUsername
                font.family: Theme.fontMono
                font.pixelSize: Theme.fontSizeMedium
                color: root.isHovered ? Theme.textSubtle : Theme.textMuted
                Behavior on color { ColorAnimation { duration: Theme.hoverDuration } }
            }

            // Password column
            Text {
                Layout.fillWidth: true
                text: root.maskedPassword
                font.family: Theme.fontMono
                font.pixelSize: Theme.fontSizeMedium
                color: root.isHovered ? Theme.textSubtle : Theme.textMuted
                Behavior on color { ColorAnimation { duration: Theme.hoverDuration } }
            }
        }

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: root.clicked()
        }
    }
}
