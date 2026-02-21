import QtQuick
import QtQuick.Layouts

// Single row in the accounts list. Only masked strings reach QML; no real credentials.

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
        // Priority: selected > hovered > zebra stripe > transparent.
        color: root.selected ? Theme.selectionActive
             : root.isHovered ? Theme.selectionHover
             : (root.index % 2 === 1) ? Theme.rowAlt
             : "transparent"
        border.width: 0

        Behavior on color { ColorAnimation { duration: Theme.hoverDuration } }

        // Top selection glow
        Rectangle {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 3
            opacity: root.selected ? 1.0 : 0.0
            Behavior on opacity { NumberAnimation { duration: 200 } }
            gradient: Gradient {
                orientation: Gradient.Vertical
                GradientStop { position: 0.0; color: Theme.selectionGlow }
                GradientStop { position: 1.0; color: Theme.selectionGlowEdge }
            }
        }

        // Bottom selection glow (mirrored)
        Rectangle {
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            height: 3
            opacity: root.selected ? 1.0 : 0.0
            Behavior on opacity { NumberAnimation { duration: 200 } }
            gradient: Gradient {
                orientation: Gradient.Vertical
                GradientStop { position: 0.0; color: Theme.selectionGlowEdge }
                GradientStop { position: 1.0; color: Theme.selectionGlow }
            }
        }

        // Left accent stripe for selected row
        Rectangle {
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            width: 3
            color: Theme.selectionStripe
            opacity: root.selected ? 1.0 : 0.0
            Behavior on opacity { NumberAnimation { duration: 200 } }
        }

        // Row separator
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

            // Platform name (accent color for easy scanning)
            Text {
                Layout.preferredWidth: 200
                text: root.platform
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeMedium
                color: root.isHovered ? Theme.accentBright : Theme.accent
                elide: Text.ElideRight
                Behavior on color { ColorAnimation { duration: Theme.hoverDuration } }
            }

            // Monospace for consistent masked character alignment
            Text {
                Layout.preferredWidth: 200
                text: root.maskedUsername
                font.family: Theme.fontMono
                font.pixelSize: Theme.fontSizeMedium
                color: root.isHovered ? Theme.textSubtle : Theme.textMuted
                Behavior on color { ColorAnimation { duration: Theme.hoverDuration } }
            }

            Text {
                Layout.fillWidth: true
                text: root.maskedPassword
                font.family: Theme.fontMono
                font.pixelSize: Theme.fontSizeMedium
                color: root.isHovered ? Theme.textSubtle : Theme.textMuted
                Behavior on color { ColorAnimation { duration: Theme.hoverDuration } }
            }
        }

        // Full-row click/hover area
        MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: root.clicked()
        }
    }
}
