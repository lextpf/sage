import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Rectangle {
    id: root

    property var model
    property int selectedRow: -1

    signal rowClicked(int row)

    radius: Theme.radiusLarge
    gradient: Gradient {
        GradientStop { position: 0; color: Theme.bgCard }
        GradientStop { position: 1; color: Theme.bgCardEnd }
    }
    border.width: 1
    border.color: Theme.borderSubtle
    clip: true

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // ── Header row ──────────────────────────────────────
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 44
            gradient: Gradient {
                GradientStop { position: 0; color: Theme.bgHeaderTop }
                GradientStop { position: 1; color: Theme.bgHeaderEnd }
            }
            // Top corners follow the parent radius
            radius: Theme.radiusLarge

            // Mask the bottom-left and bottom-right corners
            Rectangle {
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                height: Theme.radiusLarge
                color: "transparent"
                gradient: Gradient {
                    GradientStop { position: 0; color: Theme.bgHeaderEnd }
                    GradientStop { position: 1; color: Theme.bgHeaderEnd }
                }
            }

            // Bottom border
            Rectangle {
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                height: 2
                color: Theme.borderHighlight
            }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 14
                anchors.rightMargin: 14
                spacing: 0

                Row {
                    Layout.preferredWidth: 200
                    spacing: 6

                    SvgIcon {
                        source: Theme.iconService
                        width: Theme.iconSizeSmall
                        height: Theme.iconSizeSmall
                        color: Theme.accent
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    Text {
                        text: "SERVICE"
                        font.family: Theme.fontFamily
                        font.pixelSize: Theme.fontSizeSmall
                        font.bold: true
                        font.letterSpacing: 1.0
                        color: Theme.accent
                    }
                }
                Row {
                    Layout.preferredWidth: 200
                    spacing: 6

                    SvgIcon {
                        source: Theme.iconUsername
                        width: Theme.iconSizeSmall
                        height: Theme.iconSizeSmall
                        color: Theme.accent
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    Text {
                        text: "USERNAME"
                        font.family: Theme.fontFamily
                        font.pixelSize: Theme.fontSizeSmall
                        font.bold: true
                        font.letterSpacing: 1.0
                        color: Theme.accent
                    }
                }
                Row {
                    Layout.fillWidth: true
                    spacing: 6

                    SvgIcon {
                        source: Theme.iconPassword
                        width: Theme.iconSizeSmall
                        height: Theme.iconSizeSmall
                        color: Theme.accent
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    Text {
                        text: "PASSWORD"
                        font.family: Theme.fontFamily
                        font.pixelSize: Theme.fontSizeSmall
                        font.bold: true
                        font.letterSpacing: 1.0
                        color: Theme.accent
                    }
                }
            }
        }

        // ── List view ───────────────────────────────────────
        ListView {
            id: listView
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: root.model
            clip: true
            boundsBehavior: Flickable.StopAtBounds

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
                contentItem: Rectangle {
                    implicitWidth: 6
                    radius: 3
                    color: Theme.scrollThumb
                }
                background: Item {}
            }

            delegate: AccountRow {
                width: listView.width
                selected: root.selectedRow === index
                onClicked: root.rowClicked(index)
            }

            // Empty state
            Text {
                anchors.centerIn: parent
                visible: listView.count === 0
                text: "No accounts loaded"
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeMedium
                color: Theme.textMuted
            }
        }
    }
}
