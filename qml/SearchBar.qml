import QtQuick
import QtQuick.Controls

TextField {
    id: root

    placeholderText: "Search credentials..."
    placeholderTextColor: Theme.textPlaceholder
    color: Theme.textPrimary
    font.family: Theme.fontFamily
    font.pixelSize: Theme.fontSizeLarge
    selectByMouse: true
    selectionColor: Theme.btnGradBot
    selectedTextColor: Theme.textOnAccent

    leftPadding: searchIcon.width + 24
    rightPadding: 16
    topPadding: 10
    bottomPadding: 10

    SvgIcon {
        id: searchIcon
        source: Theme.iconSearch
        width: Theme.iconSizeMedium
        height: Theme.iconSizeMedium
        color: root.activeFocus ? Theme.textSubtle
             : searchHover.hovered ? Theme.textSubtle
             : Theme.textPlaceholder
        anchors.left: parent.left
        anchors.leftMargin: 12
        anchors.verticalCenter: parent.verticalCenter

        Behavior on color { ColorAnimation { duration: Theme.hoverDuration } }
    }

    HoverHandler {
        id: searchHover
        cursorShape: Qt.IBeamCursor
    }

    background: Rectangle {
        implicitHeight: 42
        radius: Theme.radiusMedium
        color: root.activeFocus ? Theme.bgInputFocus : Theme.bgInput
        border.width: 1
        border.color: root.activeFocus ? Theme.borderFocus
                    : searchHover.hovered ? Theme.borderHover
                    : Theme.borderMedium

        Behavior on color { ColorAnimation { duration: Theme.hoverDuration } }
        Behavior on border.color { ColorAnimation { duration: Theme.hoverDuration } }
    }
}
