import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

RowLayout {
    id: root
    spacing: Theme.spacingSmall

    property bool hasSelection: false
    property bool isFillArmed: false
    property int fillCountdownSeconds: 0

    signal addClicked()
    signal editClicked()
    signal deleteClicked()
    signal fillClicked()
    signal cancelFillClicked()

    // ── Primary button helper ───────────────────────────────
    component PrimaryButton: Button {
        id: btn
        property string faIcon: ""
        leftPadding: 14
        rightPadding: 14

        HoverHandler { cursorShape: Qt.PointingHandCursor }

        contentItem: Row {
            spacing: 6
            anchors.centerIn: parent
            SvgIcon {
                source: btn.faIcon
                width: Theme.iconSizeMedium
                height: Theme.iconSizeMedium
                color: btn.enabled ? Theme.textOnAccent : Theme.textDisabled
                visible: btn.faIcon !== ""
                anchors.verticalCenter: parent.verticalCenter
            }
            Text {
                text: btn.text
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeMedium
                font.weight: Font.DemiBold
                color: btn.enabled ? Theme.textOnAccent : Theme.textDisabled
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        background: Rectangle {
            implicitWidth: 120
            implicitHeight: 38
            radius: Theme.radiusMedium
            gradient: Gradient {
                GradientStop { position: 0; color: !btn.enabled ? Theme.btnDisabledTop : btn.pressed ? Theme.btnPressTop : btn.hovered ? Theme.btnHoverTop : Theme.btnGradTop; Behavior on color { ColorAnimation { duration: Theme.hoverDuration } } }
                GradientStop { position: 1; color: !btn.enabled ? Theme.btnDisabledBot : btn.pressed ? Theme.btnPressBot : btn.hovered ? Theme.btnHoverBot : Theme.btnGradBot; Behavior on color { ColorAnimation { duration: Theme.hoverDuration } } }
            }
            border.width: 1
            border.color: !btn.enabled ? Theme.borderDim
                        : btn.hovered ? Theme.borderBright
                        : Theme.borderBtn

            Behavior on border.color { ColorAnimation { duration: Theme.hoverDuration } }
        }
    }

    // ── Secondary button helper ─────────────────────────────
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
                color: secBtn.enabled ? (secBtn.hovered ? Theme.textPrimary : Theme.textGhost) : Theme.textDisabled
                visible: secBtn.faIcon !== ""
                anchors.verticalCenter: parent.verticalCenter
                Behavior on color { ColorAnimation { duration: Theme.hoverDuration } }
            }
            Text {
                text: secBtn.text
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeMedium
                font.weight: Font.Medium
                color: secBtn.enabled ? (secBtn.hovered ? Theme.textPrimary : Theme.textGhost) : Theme.textDisabled
                anchors.verticalCenter: parent.verticalCenter
                Behavior on color { ColorAnimation { duration: Theme.hoverDuration } }
            }
        }

        background: Rectangle {
            implicitWidth: 120
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
            opacity: secBtn.enabled ? 1.0 : 0.5

            Behavior on border.color { ColorAnimation { duration: Theme.hoverDuration } }
        }
    }

    PrimaryButton {
        text: "Add"
        faIcon: Theme.iconPlus
        onClicked: root.addClicked()
    }

    PrimaryButton {
        text: "Edit"
        faIcon: Theme.iconPen
        enabled: root.hasSelection
        onClicked: root.editClicked()
    }

    SecondaryButton {
        text: "Delete"
        faIcon: Theme.iconTrash
        enabled: root.hasSelection
        onClicked: root.deleteClicked()
    }

    Item { implicitWidth: 24 }

    // ── Fill button ───────────────────────────────────────
    Button {
        id: fillBtn
        leftPadding: 14
        rightPadding: 14
        enabled: root.isFillArmed || (root.hasSelection && !root.isBusy)
        onClicked: {
            if (root.isFillArmed)
                root.cancelFillClicked();
            else
                root.fillClicked();
        }

        HoverHandler { cursorShape: Qt.PointingHandCursor }

        contentItem: Row {
            spacing: 6
            anchors.centerIn: parent
            SvgIcon {
                source: root.isFillArmed ? "" : Theme.iconCrosshairs
                width: Theme.iconSizeMedium
                height: Theme.iconSizeMedium
                color: fillBtn.enabled ? Theme.textOnAccent : Theme.textDisabled
                visible: !root.isFillArmed
                anchors.verticalCenter: parent.verticalCenter
            }
            Text {
                text: root.isFillArmed
                    ? "Cancel (" + root.fillCountdownSeconds + "s)"
                    : "Fill"
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeMedium
                font.weight: Font.DemiBold
                color: fillBtn.enabled ? Theme.textOnAccent : Theme.textDisabled
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        background: Rectangle {
            implicitWidth: 120
            implicitHeight: 38
            radius: Theme.radiusMedium
            gradient: Gradient {
                GradientStop {
                    position: 0
                    color: root.isFillArmed
                        ? (fillBtn.pressed ? Theme.fillArmedPressTop : fillBtn.hovered ? Theme.fillArmedHoverTop : Theme.fillArmedTop)
                        : (!fillBtn.enabled ? Theme.btnDisabledTop : fillBtn.pressed ? Theme.btnPressTop : fillBtn.hovered ? Theme.btnHoverTop : Theme.btnGradTop)
                    Behavior on color { ColorAnimation { duration: Theme.hoverDuration } }
                }
                GradientStop {
                    position: 1
                    color: root.isFillArmed
                        ? (fillBtn.pressed ? Theme.fillArmedPressEnd : fillBtn.hovered ? Theme.fillArmedHoverEnd : Theme.fillArmedEnd)
                        : (!fillBtn.enabled ? Theme.btnDisabledBot : fillBtn.pressed ? Theme.btnPressBot : fillBtn.hovered ? Theme.btnHoverBot : Theme.btnGradBot)
                    Behavior on color { ColorAnimation { duration: Theme.hoverDuration } }
                }
            }
            border.width: 1
            border.color: root.isFillArmed
                ? Theme.borderFillArmed
                : (!fillBtn.enabled ? Theme.borderDim
                    : fillBtn.hovered ? Theme.borderBright
                    : Theme.borderBtn)

            Behavior on border.color { ColorAnimation { duration: Theme.hoverDuration } }
        }
    }

    Item { Layout.fillWidth: true }
}
