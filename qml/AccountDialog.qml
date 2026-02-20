import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Popup {
    id: root

    property string dialogTitle: "Add Account"
    property string initialService: ""
    property string initialUsername: ""
    property string initialPassword: ""
    property int editIndex: -1  // -1 = add mode

    signal accepted(string service, string username, string password, int editIdx)

    modal: true
    anchors.centerIn: parent
    width: 440
    padding: 0
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    background: Rectangle {
        color: Theme.bgDialog
        radius: Theme.radiusLarge
        border.width: 1
        border.color: Theme.borderMedium
    }

    // Overlay dimming
    Overlay.modal: Rectangle {
        color: Theme.bgOverlay
    }

    contentItem: ColumnLayout {
        spacing: 0

        // ── Header ──────────────────────────────────────────
        Text {
            Layout.fillWidth: true
            Layout.topMargin: 24
            Layout.leftMargin: 24
            Layout.rightMargin: 24
            Layout.bottomMargin: 16
            text: root.dialogTitle
            font.family: Theme.fontFamily
            font.pixelSize: Theme.px(16)
            font.bold: true
            color: Theme.textPrimary
        }

        // ── Fields ──────────────────────────────────────────
        GridLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 24
            Layout.rightMargin: 24
            columns: 2
            columnSpacing: 12
            rowSpacing: 12

            Row {
                spacing: 6
                SvgIcon {
                    source: Theme.iconService
                    width: Theme.iconSizeMedium
                    height: Theme.iconSizeMedium
                    color: Theme.textSecondary
                    anchors.verticalCenter: parent.verticalCenter
                }
                Text {
                    text: "Service:"
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontSizeMedium
                    color: Theme.textSecondary
                }
            }
            TextField {
                id: serviceField
                Layout.fillWidth: true
                text: root.initialService
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeMedium
                color: Theme.textPrimary
                placeholderText: "e.g. GitHub"
                placeholderTextColor: Theme.textPlaceholder
                selectByMouse: true

                background: Rectangle {
                    implicitHeight: 38
                    radius: Theme.radiusSmall
                    color: serviceField.activeFocus ? Theme.bgInputFocus : Theme.bgInput
                    border.width: 1
                    border.color: serviceField.activeFocus ? Theme.borderFocus : Theme.borderInput
                }
            }

            Row {
                spacing: 6
                SvgIcon {
                    source: Theme.iconUsername
                    width: Theme.iconSizeMedium
                    height: Theme.iconSizeMedium
                    color: Theme.textSecondary
                    anchors.verticalCenter: parent.verticalCenter
                }
                Text {
                    text: "Username:"
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontSizeMedium
                    color: Theme.textSecondary
                }
            }
            TextField {
                id: usernameField
                Layout.fillWidth: true
                text: root.initialUsername
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeMedium
                color: Theme.textPrimary
                selectByMouse: true

                background: Rectangle {
                    implicitHeight: 38
                    radius: Theme.radiusSmall
                    color: usernameField.activeFocus ? Theme.bgInputFocus : Theme.bgInput
                    border.width: 1
                    border.color: usernameField.activeFocus ? Theme.borderFocus : Theme.borderInput
                }
            }

            Row {
                spacing: 6
                SvgIcon {
                    source: Theme.iconPassword
                    width: Theme.iconSizeMedium
                    height: Theme.iconSizeMedium
                    color: Theme.textSecondary
                    anchors.verticalCenter: parent.verticalCenter
                }
                Text {
                    text: "Password:"
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontSizeMedium
                    color: Theme.textSecondary
                }
            }
            TextField {
                id: passwordField
                Layout.fillWidth: true
                text: root.initialPassword
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeMedium
                color: Theme.textPrimary
                echoMode: showPassword ? TextInput.Normal : TextInput.Password
                passwordCharacter: "\u2981"
                selectByMouse: true
                rightPadding: 36

                property bool showPassword: eyeArea.containsMouse

                background: Rectangle {
                    implicitHeight: 38
                    radius: Theme.radiusSmall
                    color: passwordField.activeFocus ? Theme.bgInputFocus : Theme.bgInput
                    border.width: 1
                    border.color: passwordField.activeFocus ? Theme.borderFocus : Theme.borderInput
                }

                SvgIcon {
                    anchors.right: parent.right
                    anchors.rightMargin: 8
                    anchors.verticalCenter: parent.verticalCenter
                    source: passwordField.showPassword ? Theme.iconEyeSlash : Theme.iconEye
                    color: eyeArea.containsMouse ? Theme.accent : Theme.textMuted
                    width: Theme.iconSizeMedium
                    height: Theme.iconSizeMedium

                    MouseArea {
                        id: eyeArea
                        anchors.fill: parent
                        anchors.margins: -4
                        cursorShape: Qt.PointingHandCursor
                        hoverEnabled: true
                        acceptedButtons: Qt.NoButton
                    }
                }
            }
        }

        // ── Buttons ─────────────────────────────────────────
        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: 24
            Layout.bottomMargin: 20
            Layout.leftMargin: 24
            Layout.rightMargin: 24
            spacing: Theme.spacingSmall

            Item { Layout.fillWidth: true }

            Button {
                id: cancelButton
                text: "Cancel"
                onClicked: root.close()

                HoverHandler { cursorShape: Qt.PointingHandCursor }

                contentItem: Text {
                    text: parent.text
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontSizeMedium
                    font.weight: Font.Medium
                    color: cancelButton.hovered ? Theme.textPrimary : Theme.textGhost
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    Behavior on color { ColorAnimation { duration: Theme.hoverDuration } }
                }
                background: Rectangle {
                    implicitWidth: 90
                    implicitHeight: 34
                    radius: Theme.radiusMedium
                    gradient: Gradient {
                        GradientStop { position: 0; color: cancelButton.pressed ? Theme.ghostBtnPressed : cancelButton.hovered ? Theme.ghostBtnHoverTop : Theme.ghostBtnTop; Behavior on color { ColorAnimation { duration: Theme.hoverDuration } } }
                        GradientStop { position: 1; color: cancelButton.pressed ? Theme.ghostBtnPressed : cancelButton.hovered ? Theme.ghostBtnHoverEnd : Theme.ghostBtnEnd; Behavior on color { ColorAnimation { duration: Theme.hoverDuration } } }
                    }
                    border.width: 1
                    border.color: cancelButton.pressed ? Theme.borderPressed
                                : cancelButton.hovered ? Theme.borderFocusHover
                                : Theme.borderSubtle
                    Behavior on border.color { ColorAnimation { duration: Theme.hoverDuration } }
                }
            }

            Button {
                id: okButton
                text: "OK"
                onClicked: {
                    root.accepted(
                        serviceField.text.trim(),
                        usernameField.text.trim(),
                        passwordField.text,
                        root.editIndex
                    );
                    root.close();
                }

                HoverHandler { cursorShape: Qt.PointingHandCursor }

                contentItem: Text {
                    text: parent.text
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontSizeMedium
                    font.weight: Font.DemiBold
                    color: Theme.textOnAccent
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    implicitWidth: 90
                    implicitHeight: 34
                    radius: Theme.radiusMedium
                    gradient: Gradient {
                        GradientStop { position: 0; color: okButton.pressed ? Theme.btnPressTop : okButton.hovered ? Theme.btnHoverTop : Theme.btnGradTop; Behavior on color { ColorAnimation { duration: Theme.hoverDuration } } }
                        GradientStop { position: 1; color: okButton.pressed ? Theme.btnPressBot : okButton.hovered ? Theme.btnHoverBot : Theme.btnGradBot; Behavior on color { ColorAnimation { duration: Theme.hoverDuration } } }
                    }
                    border.width: 1
                    border.color: okButton.hovered ? Theme.borderBright : Theme.borderBtn
                    Behavior on border.color { ColorAnimation { duration: Theme.hoverDuration } }
                }
            }
        }
    }

    onOpened: serviceField.forceActiveFocus()

    function resetFields() {
        serviceField.text = initialService;
        usernameField.text = initialUsername;
        passwordField.text = initialPassword;
    }

    onAboutToShow: resetFields()
}
