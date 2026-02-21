import QtQuick
import QtQuick.Controls

// Live search field. Bound to Backend.searchFilter for instant vault filtering.

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

    // Padding reserves space for search icon (left) and clear button (right).
    leftPadding: searchIcon.width + 24
    rightPadding: clearBtn.visible ? clearBtn.width + 16 : 16
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

    // Clear button. -6 margin expands hit area without enlarging the icon.
    SvgIcon {
        id: clearBtn
        source: Theme.iconXmark
        width: Theme.iconSizeSmall
        height: Theme.iconSizeSmall
        color: clearArea.containsMouse ? Theme.textSecondary : Theme.textMuted
        anchors.right: parent.right
        anchors.rightMargin: 10
        anchors.verticalCenter: parent.verticalCenter
        visible: root.text.length > 0
        opacity: visible ? 1 : 0

        Behavior on color { ColorAnimation { duration: Theme.hoverDuration } }
        Behavior on opacity { NumberAnimation { duration: 120 } }

        MouseArea {
            id: clearArea
            anchors.fill: parent
            anchors.margins: -6
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            // Clear text and re-focus so the user can immediately start a new search.
            onClicked: { root.text = ""; root.forceActiveFocus(); }
        }
    }

    // IBeamCursor on hover signals editable text.
    HoverHandler {
        id: searchHover
        cursorShape: Qt.IBeamCursor
    }

    // Three-state border: focused > hovered > idle.
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
