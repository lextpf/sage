import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

RowLayout {
    id: root
    spacing: Theme.spacingMedium

    signal loadClicked()
    signal saveClicked()
    signal unloadClicked()

    property bool vaultLoaded: false

    // Fingerprint icon
    SvgIcon {
        source: Theme.iconFingerprint
        width: Theme.px(32)
        height: Theme.px(32)
        color: Theme.accent
    }

    // Title block
    ColumnLayout {
        spacing: 2

        Text {
            text: "sage"
            font.family: Theme.fontFamily
            font.pixelSize: Theme.fontSizeTitle
            font.bold: true
            color: Theme.accent
        }
        Text {
            text: "Password Manager"
            font.family: Theme.fontFamily
            font.pixelSize: Theme.fontSizeSubtitle
            color: Theme.accentDim
        }
    }

    Item { Layout.fillWidth: true }

    // Vault control buttons
    Button {
        id: loadBtn
        text: "Load"
        leftPadding: 12
        rightPadding: 12
        onClicked: root.loadClicked()

        HoverHandler { cursorShape: Qt.PointingHandCursor }

        contentItem: Row {
            spacing: 6
            anchors.centerIn: parent
            SvgIcon {
                source: Theme.iconFolderOpen
                width: Theme.iconSizeSmall
                height: Theme.iconSizeSmall
                color: loadBtn.hovered ? Theme.textSecondary : Theme.textIcon
                anchors.verticalCenter: parent.verticalCenter
                Behavior on color { ColorAnimation { duration: Theme.hoverDuration } }
            }
            Text {
                text: "Load"
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeSmall
                font.weight: Font.Medium
                color: loadBtn.hovered ? Theme.textSecondary : Theme.textIcon
                anchors.verticalCenter: parent.verticalCenter
                Behavior on color { ColorAnimation { duration: Theme.hoverDuration } }
            }
        }

        background: Rectangle {
            implicitWidth: 86
            implicitHeight: 32
            radius: Theme.radiusSmall
            gradient: Gradient {
                GradientStop { position: 0; color: loadBtn.pressed ? Theme.iconBtnPressed : loadBtn.hovered ? Theme.iconBtnHoverTop : Theme.iconBtnTop; Behavior on color { ColorAnimation { duration: Theme.hoverDuration } } }
                GradientStop { position: 1; color: loadBtn.pressed ? Theme.iconBtnPressed : loadBtn.hovered ? Theme.iconBtnHoverEnd : Theme.iconBtnEnd; Behavior on color { ColorAnimation { duration: Theme.hoverDuration } } }
            }
            border.width: 1
            border.color: loadBtn.hovered ? Theme.borderHover : Theme.borderSoft
            Behavior on border.color { ColorAnimation { duration: Theme.hoverDuration } }
        }
    }

    Button {
        id: saveBtn
        text: "Save"
        leftPadding: 12
        rightPadding: 12
        enabled: root.vaultLoaded
        onClicked: root.saveClicked()

        HoverHandler { cursorShape: Qt.PointingHandCursor }

        contentItem: Row {
            spacing: 6
            anchors.centerIn: parent
            SvgIcon {
                source: Theme.iconFloppyDisk
                width: Theme.iconSizeSmall
                height: Theme.iconSizeSmall
                color: saveBtn.enabled ? (saveBtn.hovered ? Theme.textSecondary : Theme.textIcon) : Theme.textDisabled
                anchors.verticalCenter: parent.verticalCenter
                Behavior on color { ColorAnimation { duration: Theme.hoverDuration } }
            }
            Text {
                text: "Save"
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeSmall
                font.weight: Font.Medium
                color: saveBtn.enabled ? (saveBtn.hovered ? Theme.textSecondary : Theme.textIcon) : Theme.textDisabled
                anchors.verticalCenter: parent.verticalCenter
                Behavior on color { ColorAnimation { duration: Theme.hoverDuration } }
            }
        }

        background: Rectangle {
            implicitWidth: 86
            implicitHeight: 32
            radius: Theme.radiusSmall
            gradient: Gradient {
                GradientStop { position: 0; color: saveBtn.pressed ? Theme.iconBtnPressed : saveBtn.hovered ? Theme.iconBtnHoverTop : Theme.iconBtnTop; Behavior on color { ColorAnimation { duration: Theme.hoverDuration } } }
                GradientStop { position: 1; color: saveBtn.pressed ? Theme.iconBtnPressed : saveBtn.hovered ? Theme.iconBtnHoverEnd : Theme.iconBtnEnd; Behavior on color { ColorAnimation { duration: Theme.hoverDuration } } }
            }
            border.width: 1
            border.color: saveBtn.hovered ? Theme.borderHover : Theme.borderSoft
            opacity: saveBtn.enabled ? 1.0 : 0.5
            Behavior on border.color { ColorAnimation { duration: Theme.hoverDuration } }
        }
    }

    Button {
        id: unloadBtn
        text: "Unload"
        leftPadding: 12
        rightPadding: 12
        enabled: root.vaultLoaded
        onClicked: root.unloadClicked()

        HoverHandler { cursorShape: Qt.PointingHandCursor }

        contentItem: Row {
            spacing: 6
            anchors.centerIn: parent
            SvgIcon {
                source: Theme.iconEject
                width: Theme.iconSizeSmall
                height: Theme.iconSizeSmall
                color: unloadBtn.enabled ? (unloadBtn.hovered ? Theme.textSecondary : Theme.textIcon) : Theme.textDisabled
                anchors.verticalCenter: parent.verticalCenter
                Behavior on color { ColorAnimation { duration: Theme.hoverDuration } }
            }
            Text {
                text: "Unload"
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeSmall
                font.weight: Font.Medium
                color: unloadBtn.enabled ? (unloadBtn.hovered ? Theme.textSecondary : Theme.textIcon) : Theme.textDisabled
                anchors.verticalCenter: parent.verticalCenter
                Behavior on color { ColorAnimation { duration: Theme.hoverDuration } }
            }
        }

        background: Rectangle {
            implicitWidth: 96
            implicitHeight: 32
            radius: Theme.radiusSmall
            gradient: Gradient {
                GradientStop { position: 0; color: unloadBtn.pressed ? Theme.iconBtnPressed : unloadBtn.hovered ? Theme.iconBtnHoverTop : Theme.iconBtnTop; Behavior on color { ColorAnimation { duration: Theme.hoverDuration } } }
                GradientStop { position: 1; color: unloadBtn.pressed ? Theme.iconBtnPressed : unloadBtn.hovered ? Theme.iconBtnHoverEnd : Theme.iconBtnEnd; Behavior on color { ColorAnimation { duration: Theme.hoverDuration } } }
            }
            border.width: 1
            border.color: unloadBtn.hovered ? Theme.borderHover : Theme.borderSoft
            opacity: unloadBtn.enabled ? 1.0 : 0.5
            Behavior on border.color { ColorAnimation { duration: Theme.hoverDuration } }
        }
    }
}
