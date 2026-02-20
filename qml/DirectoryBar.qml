import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

RowLayout {
    id: root
    spacing: Theme.spacingSmall

    signal encryptDirClicked()
    signal decryptDirClicked()

    component SecondaryButton: Button {
        id: secBtn
        property string faIcon: ""
        leftPadding: 14
        rightPadding: 14

        HoverHandler { cursorShape: Qt.PointingHandCursor }

        contentItem: Row {
            spacing: 6
            anchors.centerIn: parent
            SvgIcon {
                source: secBtn.faIcon
                width: Theme.iconSizeMedium
                height: Theme.iconSizeMedium
                color: secBtn.hovered ? Theme.textPrimary : Theme.textGhost
                visible: secBtn.faIcon !== ""
                anchors.verticalCenter: parent.verticalCenter
                Behavior on color { ColorAnimation { duration: Theme.hoverDuration } }
            }
            Text {
                text: secBtn.text
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeMedium
                font.weight: Font.Medium
                color: secBtn.hovered ? Theme.textPrimary : Theme.textGhost
                anchors.verticalCenter: parent.verticalCenter
                Behavior on color { ColorAnimation { duration: Theme.hoverDuration } }
            }
        }

        background: Rectangle {
            implicitWidth: 150
            implicitHeight: 38
            radius: Theme.radiusMedium
            gradient: Gradient {
                GradientStop { position: 0; color: secBtn.pressed ? Theme.ghostBtnPressed : secBtn.hovered ? Theme.ghostBtnHoverTop : Theme.ghostBtnTop; Behavior on color { ColorAnimation { duration: Theme.hoverDuration } } }
                GradientStop { position: 1; color: secBtn.pressed ? Theme.ghostBtnPressed : secBtn.hovered ? Theme.ghostBtnHoverEnd : Theme.ghostBtnEnd; Behavior on color { ColorAnimation { duration: Theme.hoverDuration } } }
            }
            border.width: 1
            border.color: secBtn.pressed ? Theme.borderPressed
                        : secBtn.hovered ? Theme.borderFocusHover
                        : Theme.borderSubtle

            Behavior on border.color { ColorAnimation { duration: Theme.hoverDuration } }
        }
    }

    SecondaryButton {
        text: "Encrypt Directory"
        faIcon: Theme.iconLock
        onClicked: root.encryptDirClicked()
    }

    SecondaryButton {
        text: "Decrypt Directory"
        faIcon: Theme.iconLockOpen
        onClicked: root.decryptDirClicked()
    }

    Item { Layout.fillWidth: true }
}
