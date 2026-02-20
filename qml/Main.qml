import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: window
    visible: true
    width: 1100
    height: 750
    minimumWidth: 900
    minimumHeight: 600
    title: "sage"
    color: Theme.bgDeep

    // ── Backend connections ──────────────────────────────────
    Connections {
        target: Backend

        function onErrorOccurred(title, message) {
            errorDialog.title = title;
            errorDialog.message = message;
            errorDialog.open();
        }

        function onConfirmDeleteRequested(index, platform) {
            confirmDlg.deleteIndex = index;
            confirmDlg.message = "Are you sure you want to delete the account for '" + platform + "'?";
            confirmDlg.open();
        }

        function onInfoMessage(title, message) {
            infoDialog.title = title;
            infoDialog.message = message;
            infoDialog.open();
        }

        function onPasswordRequired() {
            passwordDlg.errorMessage = "";
            passwordDlg.open();
        }

        function onPasswordRetryRequired(message) {
            passwordDlg.errorMessage = message;
            passwordDlg.open();
        }

        function onOcrCaptureFinished(success) {
            if (!success) {
                passwordDlg.errorMessage = "";
                passwordDlg.open();
            }
        }

        function onEditAccountReady(data) {
            accountDlg.dialogTitle = "Edit Account";
            accountDlg.editIndex = data.editIndex;
            accountDlg.initialService = data.service;
            accountDlg.initialUsername = data.username;
            accountDlg.initialPassword = data.password;
            accountDlg.open();
        }
    }

    // ── Auto-load on startup ────────────────────────────────
    Component.onCompleted: {
        Backend.autoLoadVault();
    }

    onClosing: function(close) {
        Backend.cleanup();
        close.accepted = true;
    }

    // ── Main layout ─────────────────────────────────────────
    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Content area with margins
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: Theme.spacingXL
            Layout.topMargin: 30
            Layout.bottomMargin: 24
            spacing: Theme.spacingLarge

            // Header
            HeaderBar {
                Layout.fillWidth: true
                vaultLoaded: Backend.vaultLoaded

                onLoadClicked: Backend.loadVault()
                onSaveClicked: Backend.saveVault()
                onUnloadClicked: Backend.unloadVault()
            }

            // Search
            SearchBar {
                Layout.fillWidth: true
                onTextChanged: Backend.searchFilter = text
            }

            // Accounts table
            AccountsTable {
                Layout.fillWidth: true
                Layout.fillHeight: true
                model: Backend.vaultModel
                selectedRow: Backend.selectedIndex

                onRowClicked: function(row) {
                    Backend.selectedIndex = (Backend.selectedIndex === row) ? -1 : row;
                }
            }

            // Action buttons
            ActionBar {
                Layout.fillWidth: true
                hasSelection: Backend.hasSelection
                isFillArmed: Backend.isFillArmed
                fillCountdownSeconds: Backend.fillCountdownSeconds

                onAddClicked: {
                    accountDlg.dialogTitle = "Add Account";
                    accountDlg.editIndex = -1;
                    accountDlg.initialService = "";
                    accountDlg.initialUsername = "";
                    accountDlg.initialPassword = "";
                    accountDlg.open();
                }

                onEditClicked: {
                    if (!Backend.hasSelection) return;
                    var realIdx = Backend.vaultModel.recordIndexForRow(Backend.selectedIndex);
                    var data = Backend.decryptAccountForEdit(realIdx);
                    if (!data.service) return;
                    accountDlg.dialogTitle = "Edit Account";
                    accountDlg.editIndex = realIdx;
                    accountDlg.initialService = data.service;
                    accountDlg.initialUsername = data.username;
                    accountDlg.initialPassword = data.password;
                    accountDlg.open();
                }

                onDeleteClicked: {
                    if (!Backend.hasSelection) return;
                    var realIdx = Backend.vaultModel.recordIndexForRow(Backend.selectedIndex);
                    confirmDlg.deleteIndex = realIdx;
                    confirmDlg.message = "Are you sure you want to delete this account?";
                    confirmDlg.open();
                }

                onFillClicked: {
                    if (!Backend.hasSelection) return;
                    var realIdx = Backend.vaultModel.recordIndexForRow(Backend.selectedIndex);
                    Backend.armFill(realIdx);
                }

                onCancelFillClicked: {
                    Backend.cancelFill();
                }
            }

            // Directory buttons
            DirectoryBar {
                Layout.fillWidth: true
                onEncryptDirClicked: Backend.encryptDirectory()
                onDecryptDirClicked: Backend.decryptDirectory()
            }
        }

        // Status footer
        StatusFooter {
            Layout.fillWidth: true
            statusText: Backend.statusText
            fillArmed: Backend.isFillArmed
        }
    }

    // ── Dialogs ─────────────────────────────────────────────

    PasswordDialog {
        id: passwordDlg
        onAccepted: function(password) {
            Backend.submitPassword(password);
        }
        onOcrRequested: {
            Backend.requestOcrCapture();
        }
    }

    AccountDialog {
        id: accountDlg
        onAccepted: function(service, username, password, editIdx) {
            if (editIdx >= 0)
                Backend.editAccount(editIdx, service, username, password);
            else
                Backend.addAccount(service, username, password);
        }
    }

    // Delete confirmation
    ConfirmDialog {
        id: confirmDlg
        title: "Confirm Delete"
        property int deleteIndex: -1

        onConfirmed: {
            if (deleteIndex >= 0)
                Backend.deleteAccount(deleteIndex);
            deleteIndex = -1;
        }
    }

    // Error dialog (reuse ConfirmDialog as info-only)
    ConfirmDialog {
        id: errorDialog
        // Only show OK button for errors
        contentItem: ColumnLayout {
            spacing: 0

            Text {
                Layout.fillWidth: true
                Layout.topMargin: 24
                Layout.leftMargin: 24
                Layout.rightMargin: 24
                text: errorDialog.title
                font.family: Theme.fontFamily
                font.pixelSize: Theme.px(16)
                font.bold: true
                color: Theme.textPrimary
            }

            Text {
                Layout.fillWidth: true
                Layout.topMargin: 12
                Layout.leftMargin: 24
                Layout.rightMargin: 24
                text: errorDialog.message
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeMedium
                color: Theme.textSecondary
                wrapMode: Text.WordWrap
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.topMargin: 24
                Layout.bottomMargin: 20
                Layout.rightMargin: 24
                spacing: Theme.spacingSmall

                Item { Layout.fillWidth: true }

                Button {
                    id: errorOkButton
                    text: "OK"
                    onClicked: errorDialog.close()

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
                            GradientStop { position: 0; color: errorOkButton.pressed ? Theme.btnPressTop : errorOkButton.hovered ? Theme.btnHoverTop : Theme.btnGradTop; Behavior on color { ColorAnimation { duration: Theme.hoverDuration } } }
                            GradientStop { position: 1; color: errorOkButton.pressed ? Theme.btnPressBot : errorOkButton.hovered ? Theme.btnHoverBot : Theme.btnGradBot; Behavior on color { ColorAnimation { duration: Theme.hoverDuration } } }
                        }
                        border.width: 1
                        border.color: errorOkButton.hovered ? Theme.borderBright : Theme.borderBtn
                        Behavior on border.color { ColorAnimation { duration: Theme.hoverDuration } }
                    }
                }
            }
        }
    }

    // Info dialog
    ConfirmDialog {
        id: infoDialog

        contentItem: ColumnLayout {
            spacing: 0

            Text {
                Layout.fillWidth: true
                Layout.topMargin: 24
                Layout.leftMargin: 24
                Layout.rightMargin: 24
                text: infoDialog.title
                font.family: Theme.fontFamily
                font.pixelSize: Theme.px(16)
                font.bold: true
                color: Theme.textPrimary
            }

            Text {
                Layout.fillWidth: true
                Layout.topMargin: 12
                Layout.leftMargin: 24
                Layout.rightMargin: 24
                text: infoDialog.message
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeMedium
                color: Theme.textSecondary
                wrapMode: Text.WordWrap
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.topMargin: 24
                Layout.bottomMargin: 20
                Layout.rightMargin: 24
                spacing: Theme.spacingSmall

                Item { Layout.fillWidth: true }

                Button {
                    id: infoOkButton
                    text: "OK"
                    onClicked: infoDialog.close()

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
                            GradientStop { position: 0; color: infoOkButton.pressed ? Theme.btnPressTop : infoOkButton.hovered ? Theme.btnHoverTop : Theme.btnGradTop; Behavior on color { ColorAnimation { duration: Theme.hoverDuration } } }
                            GradientStop { position: 1; color: infoOkButton.pressed ? Theme.btnPressBot : infoOkButton.hovered ? Theme.btnHoverBot : Theme.btnGradBot; Behavior on color { ColorAnimation { duration: Theme.hoverDuration } } }
                        }
                        border.width: 1
                        border.color: infoOkButton.hovered ? Theme.borderBright : Theme.borderBtn
                        Behavior on border.color { ColorAnimation { duration: Theme.hoverDuration } }
                    }
                }
            }
        }
    }
}
