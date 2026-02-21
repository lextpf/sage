import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Encrypt/Decrypt directory buttons. Always enabled (vault-independent).

RowLayout {
    id: root
    spacing: Theme.spacingSmall

    signal encryptDirClicked()
    signal decryptDirClicked()

    Button {
        id: encryptBtn
        text: "Encrypt Directory"
        leftPadding: 12
        rightPadding: 12
        onClicked: root.encryptDirClicked()

        HoverHandler { id: encryptHover; cursorShape: Qt.PointingHandCursor }

        contentItem: Row {
            spacing: 6
            anchors.centerIn: parent
            SvgIcon {
                source: Theme.iconLock
                width: Theme.iconSizeSmall
                height: Theme.iconSizeSmall
                color: encryptBtn.hovered ? Theme.textSecondary : Theme.textIcon
                anchors.verticalCenter: parent.verticalCenter
                Behavior on color { ColorAnimation { duration: Theme.hoverDuration } }
            }
            Text {
                text: encryptBtn.text
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeSmall
                font.weight: Font.Medium
                color: encryptBtn.hovered ? Theme.textSecondary : Theme.textIcon
                anchors.verticalCenter: parent.verticalCenter
                Behavior on color { ColorAnimation { duration: Theme.hoverDuration } }
            }
        }

        scale: pressed ? 0.97 : 1.0
        Behavior on scale { NumberAnimation { duration: 200; easing.type: Easing.OutBack; easing.overshoot: 2.0 } }

        background: Rectangle {
            implicitWidth: 86
            implicitHeight: 32
            radius: Theme.radiusSmall
            gradient: Gradient {
                GradientStop { position: 0; color: encryptBtn.pressed ? Theme.iconBtnPressed : encryptBtn.hovered ? Theme.iconBtnHoverTop : Theme.iconBtnTop; Behavior on color { ColorAnimation { duration: Theme.hoverDuration } } }
                GradientStop { position: 1; color: encryptBtn.pressed ? Theme.iconBtnPressed : encryptBtn.hovered ? Theme.iconBtnHoverEnd : Theme.iconBtnEnd; Behavior on color { ColorAnimation { duration: Theme.hoverDuration } } }
            }
            border.width: 1
            border.color: encryptBtn.hovered ? Theme.borderHover : Theme.borderSoft
            Behavior on border.color { ColorAnimation { duration: Theme.hoverDuration } }

            RippleEffect { id: encryptRipple; baseColor: Qt.rgba(Theme.iconBtnHoverTop.r, Theme.iconBtnHoverTop.g, Theme.iconBtnHoverTop.b, 0.40) }
        }
        onPressed: encryptRipple.trigger(encryptHover.point.position.x, encryptHover.point.position.y)
    }

    Button {
        id: decryptBtn
        text: "Decrypt Directory"
        leftPadding: 12
        rightPadding: 12
        onClicked: root.decryptDirClicked()

        HoverHandler { id: decryptHover; cursorShape: Qt.PointingHandCursor }

        contentItem: Row {
            spacing: 6
            anchors.centerIn: parent
            SvgIcon {
                source: Theme.iconLockOpen
                width: Theme.iconSizeSmall
                height: Theme.iconSizeSmall
                color: decryptBtn.hovered ? Theme.textSecondary : Theme.textIcon
                anchors.verticalCenter: parent.verticalCenter
                Behavior on color { ColorAnimation { duration: Theme.hoverDuration } }
            }
            Text {
                text: decryptBtn.text
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeSmall
                font.weight: Font.Medium
                color: decryptBtn.hovered ? Theme.textSecondary : Theme.textIcon
                anchors.verticalCenter: parent.verticalCenter
                Behavior on color { ColorAnimation { duration: Theme.hoverDuration } }
            }
        }

        scale: pressed ? 0.97 : 1.0
        Behavior on scale { NumberAnimation { duration: 200; easing.type: Easing.OutBack; easing.overshoot: 2.0 } }

        background: Rectangle {
            implicitWidth: 86
            implicitHeight: 32
            radius: Theme.radiusSmall
            gradient: Gradient {
                GradientStop { position: 0; color: decryptBtn.pressed ? Theme.iconBtnPressed : decryptBtn.hovered ? Theme.iconBtnHoverTop : Theme.iconBtnTop; Behavior on color { ColorAnimation { duration: Theme.hoverDuration } } }
                GradientStop { position: 1; color: decryptBtn.pressed ? Theme.iconBtnPressed : decryptBtn.hovered ? Theme.iconBtnHoverEnd : Theme.iconBtnEnd; Behavior on color { ColorAnimation { duration: Theme.hoverDuration } } }
            }
            border.width: 1
            border.color: decryptBtn.hovered ? Theme.borderHover : Theme.borderSoft
            Behavior on border.color { ColorAnimation { duration: Theme.hoverDuration } }

            RippleEffect { id: decryptRipple; baseColor: Qt.rgba(Theme.iconBtnHoverTop.r, Theme.iconBtnHoverTop.g, Theme.iconBtnHoverTop.b, 0.40) }
        }
        onPressed: decryptRipple.trigger(decryptHover.point.position.x, decryptHover.point.position.y)
    }

    Item { Layout.fillWidth: true }
}
