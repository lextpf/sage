import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Embedded terminal panel for interactive CLI commands.
//
// Replaces the main vault UI (SearchBar, AccountsTable, ActionBar)
// when CLI mode is toggled via the window chrome button. Commands are dispatched
// to Backend.executeCliCommand() and output is received via Backend.cliOutputReady.

Item {
    id: root

    // Accumulated output text. Using a single string + TextArea instead of
    // a ListView so the user can select and copy arbitrary spans of output.
    // Capped at _maxLines to prevent unbounded growth during long sessions.
    property string outputText: ""
    property int _lineCount: 0
    readonly property int _maxLines: 500
    readonly property int _trimTarget: 400

    function _appendOutput(line) {
        if (outputText.length > 0)
            outputText += "\n"
        outputText += line
        _lineCount++

        // Trim oldest lines in batches to avoid O(n) split/join on every append.
        if (_lineCount > _maxLines) {
            var lines = outputText.split("\n")
            outputText = lines.slice(lines.length - _trimTarget).join("\n")
            _lineCount = _trimTarget
        }

        Qt.callLater(function() {
            outputScroll.ScrollBar.vertical.position = 1.0 - outputScroll.ScrollBar.vertical.size
        })
    }

    Connections {
        target: Backend
        function onCliOutputReady(text) { root._appendOutput(text) }
        function onCliOutputCleared() {
            root.outputText = ""
            root._lineCount = 0
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Scrollable, selectable output area
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: Theme.radiusMedium
            color: Theme.bgInput
            border.width: 1
            border.color: Theme.borderSubtle
            Behavior on color { ColorAnimation { duration: Theme.hoverDuration } }
            Behavior on border.color { ColorAnimation { duration: Theme.hoverDuration } }

            ScrollView {
                id: outputScroll
                anchors.fill: parent
                anchors.margins: 10
                clip: true

                ScrollBar.vertical: ScrollBar {
                    policy: ScrollBar.AsNeeded
                    contentItem: Rectangle {
                        implicitWidth: 6
                        radius: 3
                        color: parent.hovered ? Theme.scrollThumbHover : Theme.scrollThumb
                        Behavior on color { ColorAnimation { duration: Theme.hoverDuration } }
                    }
                }

                TextArea {
                    id: outputArea
                    text: root.outputText
                    readOnly: true
                    selectByMouse: true
                    selectionColor: Theme.btnGradBot
                    selectedTextColor: Theme.textOnAccent
                    font.family: Theme.fontMono
                    font.pixelSize: Theme.fontSizeMedium
                    color: Theme.textPrimary
                    wrapMode: Text.WrapAnywhere
                    textFormat: Text.PlainText

                    background: Item {}
                }
            }
        }

        // Input area
        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: Theme.spacingSmall
            spacing: 8

            Text {
                text: "seal>"
                font.family: Theme.fontMono
                font.pixelSize: Theme.fontSizeMedium
                font.weight: Font.DemiBold
                color: Theme.accent
                Layout.alignment: Qt.AlignVCenter
                Behavior on color { ColorAnimation { duration: 350 } }
            }

            TextField {
                id: inputField
                Layout.fillWidth: true
                placeholderText: "Type a command... (:help for commands)"
                placeholderTextColor: Theme.textPlaceholder
                color: Theme.textPrimary
                font.family: Theme.fontMono
                font.pixelSize: Theme.fontSizeMedium
                selectByMouse: true
                selectionColor: Theme.btnGradBot
                selectedTextColor: Theme.textOnAccent

                background: Rectangle {
                    implicitHeight: 36
                    radius: Theme.radiusSmall
                    color: inputField.activeFocus ? Theme.bgInputFocus : Theme.bgInput
                    border.width: 1
                    border.color: inputField.activeFocus ? Theme.borderFocus
                                : inputField.hovered ? Theme.borderHover
                                : Theme.borderMedium
                    Behavior on color { ColorAnimation { duration: Theme.hoverDuration } }
                    Behavior on border.color { ColorAnimation { duration: Theme.hoverDuration } }
                }

                Keys.onReturnPressed: submitCommand()
                Keys.onEnterPressed: submitCommand()

                function submitCommand() {
                    var cmd = inputField.text.trim()
                    if (cmd.length === 0) return
                    root._appendOutput("seal> " + inputField.text)
                    Backend.executeCliCommand(inputField.text)
                    inputField.text = ""
                }
            }
        }
    }

    // Focus the input field when the panel becomes visible
    onVisibleChanged: {
        if (visible) inputField.forceActiveFocus()
    }
}
