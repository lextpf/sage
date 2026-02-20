import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Popup {
    id: root

    property string errorMessage: ""

    signal accepted(string password)
    signal ocrRequested()

    modal: true
    anchors.centerIn: parent
    width: 420
    padding: 0
    closePolicy: Popup.CloseOnEscape

    background: Rectangle {
        color: Theme.bgDialog
        radius: Theme.radiusLarge
        border.width: 1
        border.color: Theme.borderMedium
    }

    Overlay.modal: Rectangle {
        color: Theme.bgOverlay
    }

    onAboutToShow: {
        passwordField.text = "";
        passwordField.forceActiveFocus();
    }

    contentItem: ColumnLayout {
        spacing: 0

        // Title row
        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: 24
            Layout.leftMargin: 24
            Layout.rightMargin: 24
            spacing: 10

            SvgIcon {
                source: Theme.iconLock
                color: Theme.accent
                width: Theme.iconSizeMedium
                height: Theme.iconSizeMedium
            }

            Text {
                text: "Enter Master Password"
                font.family: Theme.fontFamily
                font.pixelSize: Theme.px(16)
                font.bold: true
                color: Theme.textPrimary
            }
        }

        Text {
            Layout.fillWidth: true
            Layout.topMargin: 8
            Layout.leftMargin: 24
            Layout.rightMargin: 24
            text: "Type your password or use webcam OCR."
            font.family: Theme.fontFamily
            font.pixelSize: Theme.fontSizeMedium
            color: Theme.textSecondary
            wrapMode: Text.WordWrap
        }

        Text {
            Layout.fillWidth: true
            Layout.topMargin: 6
            Layout.leftMargin: 24
            Layout.rightMargin: 24
            visible: root.errorMessage !== ""
            text: root.errorMessage
            font.family: Theme.fontFamily
            font.pixelSize: Theme.fontSizeMedium
            font.weight: Font.Medium
            color: "#e05555"
            wrapMode: Text.WordWrap
        }

        // Password field
        TextField {
            id: passwordField
            Layout.fillWidth: true
            Layout.topMargin: 16
            Layout.leftMargin: 24
            Layout.rightMargin: 24
            echoMode: showPassword ? TextInput.Normal : TextInput.Password
            passwordCharacter: "\u2981"
            placeholderText: "Password"
            placeholderTextColor: Theme.textMuted
            font.family: Theme.fontFamily
            font.pixelSize: Theme.fontSizeLarge
            color: Theme.textPrimary
            leftPadding: 12
            rightPadding: 36

            property bool showPassword: eyeArea.containsMouse

            background: Rectangle {
                implicitHeight: 38
                radius: Theme.radiusSmall
                color: passwordField.activeFocus ? Theme.bgInputFocus : Theme.bgInput
                border.width: 1
                border.color: passwordField.activeFocus ? Theme.borderFocus : Theme.borderSubtle
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

            Keys.onReturnPressed: {
                if (passwordField.text.length > 0) {
                    var pw = passwordField.text;
                    root.close();
                    root.accepted(pw);
                }
            }
            Keys.onEnterPressed: {
                if (passwordField.text.length > 0) {
                    var pw = passwordField.text;
                    root.close();
                    root.accepted(pw);
                }
            }
        }

        // Buttons
        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: 24
            Layout.bottomMargin: 20
            Layout.leftMargin: 24
            Layout.rightMargin: 24
            spacing: Theme.spacingSmall

            // Cancel button (left)
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

            Item { Layout.fillWidth: true }

            // OCR button
            Button {
                id: ocrButton
                onClicked: {
                    root.close();
                    root.ocrRequested();
                }

                HoverHandler { cursorShape: Qt.PointingHandCursor }

                contentItem: RowLayout {
                    spacing: 6
                    SvgIcon {
                        source: Theme.iconCamera
                        color: ocrButton.hovered ? Theme.textPrimary : Theme.textGhost
                        width: Theme.iconSizeSmall
                        height: Theme.iconSizeSmall
                        Behavior on color { ColorAnimation { duration: Theme.hoverDuration } }
                    }
                    Text {
                        text: "OCR"
                        font.family: Theme.fontFamily
                        font.pixelSize: Theme.fontSizeMedium
                        font.weight: Font.Medium
                        color: ocrButton.hovered ? Theme.textPrimary : Theme.textGhost
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        Behavior on color { ColorAnimation { duration: Theme.hoverDuration } }
                    }
                }
                background: Rectangle {
                    implicitWidth: 90
                    implicitHeight: 34
                    radius: Theme.radiusMedium
                    gradient: Gradient {
                        GradientStop { position: 0; color: ocrButton.pressed ? Theme.ghostBtnPressed : ocrButton.hovered ? Theme.ghostBtnHoverTop : Theme.ghostBtnTop; Behavior on color { ColorAnimation { duration: Theme.hoverDuration } } }
                        GradientStop { position: 1; color: ocrButton.pressed ? Theme.ghostBtnPressed : ocrButton.hovered ? Theme.ghostBtnHoverEnd : Theme.ghostBtnEnd; Behavior on color { ColorAnimation { duration: Theme.hoverDuration } } }
                    }
                    border.width: 1
                    border.color: ocrButton.pressed ? Theme.borderPressed
                                : ocrButton.hovered ? Theme.borderFocusHover
                                : Theme.borderSubtle
                    Behavior on border.color { ColorAnimation { duration: Theme.hoverDuration } }
                }
            }

            // OK button
            Button {
                id: okButton
                text: "OK"
                enabled: passwordField.text.length > 0
                onClicked: {
                    var pw = passwordField.text;
                    root.close();
                    root.accepted(pw);
                }

                HoverHandler { cursorShape: Qt.PointingHandCursor }

                contentItem: Text {
                    text: parent.text
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontSizeMedium
                    font.weight: Font.DemiBold
                    color: okButton.enabled ? Theme.textOnAccent : Theme.textDisabled
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    implicitWidth: 90
                    implicitHeight: 34
                    radius: Theme.radiusMedium
                    gradient: Gradient {
                        GradientStop { position: 0; color: okButton.enabled ? (okButton.pressed ? Theme.btnPressTop : okButton.hovered ? Theme.btnHoverTop : Theme.btnGradTop) : Theme.btnDisabledTop; Behavior on color { ColorAnimation { duration: Theme.hoverDuration } } }
                        GradientStop { position: 1; color: okButton.enabled ? (okButton.pressed ? Theme.btnPressBot : okButton.hovered ? Theme.btnHoverBot : Theme.btnGradBot) : Theme.btnDisabledBot; Behavior on color { ColorAnimation { duration: Theme.hoverDuration } } }
                    }
                    border.width: 1
                    border.color: okButton.enabled ? (okButton.hovered ? Theme.borderBright : Theme.borderBtn) : Theme.borderSubtle
                    Behavior on border.color { ColorAnimation { duration: Theme.hoverDuration } }
                }
            }
        }
    }
}
