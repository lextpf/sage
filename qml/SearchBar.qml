import QtQuick
import QtQuick.Controls

// Live search field. Typing here instantly updates Backend.searchFilter, which
// triggers VaultListModel::setFilter() in C++ to re-filter the visible rows.
// The model uses case-insensitive substring matching against platform names.
//
// Layout: [search icon] [text input] [active filter chip] [X clear button]
// The status chip and clear button appear only when the field has text.

TextField {
    id: root
    property int resultCount: 0
    readonly property bool filtering: text.trim().length > 0
    readonly property string resultLabel: resultCount === 1 ? "1 match" : resultCount + " matches"
    property real sheenProgress: -0.35

    // Debounced search signal. Fires 200 ms after the last keystroke so C++
    // model filtering is not triggered on every single character typed.
    signal searchRequested(string text)

    Timer {
        id: _debounce
        interval: 200
        onTriggered: root.searchRequested(root.text)
    }
    onTextChanged: _debounce.restart()
    onActiveFocusChanged: if (!activeFocus) sheenProgress = -0.35

    placeholderText: "Search credentials..."
    placeholderTextColor: Theme.textPlaceholder
    color: Theme.textPrimary
    font.family: Theme.fontFamily
    font.pixelSize: Theme.fontSizeLarge
    selectByMouse: true
    selectionColor: Theme.btnGradBot
    selectedTextColor: Theme.textOnAccent

    // Padding reserves space for search icon (left) plus status chip and clear button (right).
    leftPadding: searchIcon.width + 24
    rightPadding: 16
                + (clearBtn.visible ? clearBtn.width + 16 : 0)
                + (filterChip.visible ? filterChip.implicitWidth + 10 : 0)
    topPadding: 10
    bottomPadding: 10

    SequentialAnimation on sheenProgress {
        running: root.activeFocus
        loops: Animation.Infinite
        NumberAnimation { from: -0.35; to: 1.15; duration: 1750; easing.type: Easing.InOutQuad }
        PauseAnimation { duration: 650 }
    }

    SvgIcon {
        id: searchIcon
        source: Theme.iconSearch
        width: Theme.iconSizeMedium
        height: Theme.iconSizeMedium
        color: root.activeFocus ? Theme.accent
             : root.filtering ? Theme.textSecondary
             : searchHover.hovered ? Theme.textSubtle
             : Theme.textPlaceholder
        anchors.left: parent.left
        anchors.leftMargin: 12
        anchors.verticalCenter: parent.verticalCenter

        Behavior on color { ColorAnimation { duration: Theme.hoverDuration } }
    }

    Rectangle {
        id: filterChip
        visible: root.filtering
        opacity: visible ? 1 : 0
        implicitWidth: chipText.implicitWidth + 16
        implicitHeight: 22
        radius: implicitHeight / 2
        anchors.right: clearBtn.left
        anchors.rightMargin: 8
        anchors.verticalCenter: parent.verticalCenter
        gradient: Gradient {
            GradientStop { position: 0; color: Theme.statusChipTop }
            GradientStop { position: 1; color: Theme.statusChipEnd }
        }
        border.width: 1
        border.color: Theme.statusChipBorder
        clip: true

        Rectangle {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 1
            color: Theme.surfaceHighlight
            opacity: 0.65
        }

        Text {
            id: chipText
            anchors.centerIn: parent
            text: root.resultLabel
            font.family: Theme.fontFamily
            font.pixelSize: Theme.fontSizeSmall
            font.weight: Font.DemiBold
            color: Theme.statusChipText
        }

        Behavior on opacity { NumberAnimation { duration: 120 } }
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
        clip: true

        Rectangle {
            width: parent.width * 0.72
            height: parent.height * 1.4
            radius: height / 2
            x: parent.width * 0.06
            y: -height * 0.45
            color: Theme.surfaceGlow
            opacity: root.activeFocus ? 0.32 : searchHover.hovered ? 0.18 : 0.04
            Behavior on opacity { NumberAnimation { duration: Theme.hoverDuration } }
        }

        Rectangle {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 1
            color: Theme.surfaceHighlight
            opacity: root.activeFocus ? 0.85 : searchHover.hovered ? 0.45 : 0.16
            Behavior on opacity { NumberAnimation { duration: Theme.hoverDuration } }
        }

        Rectangle {
            width: parent.width * 0.42
            height: parent.height * 1.8
            radius: width / 2
            x: -width + (parent.width + width * 2) * root.sheenProgress
            y: -height * 0.24
            rotation: 18
            opacity: root.activeFocus ? 0.52 : 0.0
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop { position: 0.0; color: Qt.rgba(Theme.focusSheen.r, Theme.focusSheen.g, Theme.focusSheen.b, 0.0) }
                GradientStop { position: 0.50; color: Theme.focusSheen }
                GradientStop { position: 1.0; color: Qt.rgba(Theme.focusSheen.r, Theme.focusSheen.g, Theme.focusSheen.b, 0.0) }
            }
            Behavior on opacity { NumberAnimation { duration: Theme.hoverDuration } }
        }

        Behavior on color { ColorAnimation { duration: Theme.hoverDuration } }
        Behavior on border.color { ColorAnimation { duration: Theme.hoverDuration } }
    }
}
