import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import QtQuick.Shapes
import QtQuick.Effects

// Top header bar. Layout: [narwhal icon] [title] [theme toggle] ... [Load] [Save] [Unload]
//
// Save and Unload are disabled until a vault is loaded (bound to vaultLoaded).
// Load is always enabled so the user can open a vault from any state.
//
// All three vault buttons use the neutral iconBtn palette (same hue, different
// states) because they're utilities rather than primary actions like Add/Edit/Delete.

Item {
    id: root
    implicitHeight: headerRow.implicitHeight
    implicitWidth: headerRow.implicitWidth

    signal loadClicked()
    signal saveClicked()
    signal unloadClicked()

    property bool vaultLoaded: false

    // Title bar drag area: wraps the header row so unhandled clicks
    // (on empty space between buttons) bubble up here and start a
    // native window drag. Interactive children consume their own events.
    MouseArea {
        id: dragArea
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton
        onPressed: function(mouse) {
            Backend.startWindowDrag()
        }
        onDoubleClicked: function(mouse) {
            var w = root.Window.window
            if (w.visibility === Window.Maximized)
                w.showNormal()
            else
                w.showMaximized()
        }

    RowLayout {
        id: headerRow
        anchors.fill: parent
        spacing: Theme.spacingMedium

    // App identity icon. Clicking triggers a sonar-pulse rainbow easter egg:
    // a ring expands outward from the icon while its color sweeps through
    // the spectrum, then returns to the theme accent.
    SvgIcon {
        id: narwhalIcon
        source: Theme.iconNarwhal
        width: Theme.px(32)
        height: Theme.px(32)
        color: Theme.accent

        property bool active: false

        // Sonar rings - GPU-rendered vector arcs via QtQuick.Shapes.
        // Each ring is a donut (two concentric circular paths with OddEvenFill)
        // filled with a conic gradient. No Canvas rasterization, no onPaint —
        // the scene graph handles anti-aliasing natively.
        Shape {
            id: ringWarm
            anchors.centerIn: parent
            width: 0; height: width; opacity: 0
            readonly property real strokeW: 4.5
            readonly property real outerR: Math.max(1, width / 2)
            readonly property real innerR: Math.max(0, outerR - strokeW)
            readonly property real cx: width / 2
            readonly property real cy: width / 2
            layer.enabled: true; layer.samples: 4; layer.smooth: true
            layer.textureSize: Qt.size(Math.max(1, width * 5), Math.max(1, height * 5))

            ShapePath {
                fillRule: ShapePath.OddEvenFill
                strokeWidth: -1
                fillGradient: ConicalGradient {
                    centerX: ringWarm.cx; centerY: ringWarm.cy; angle: 0
                    GradientStop { position: 0.00; color: "#ff0044" }
                    GradientStop { position: 0.16; color: "#ff8800" }
                    GradientStop { position: 0.33; color: "#ffee00" }
                    GradientStop { position: 0.50; color: "#00dd66" }
                    GradientStop { position: 0.66; color: "#0088ff" }
                    GradientStop { position: 0.83; color: "#aa00ff" }
                    GradientStop { position: 1.00; color: "#ff0044" }
                }
                startX: ringWarm.cx + ringWarm.outerR; startY: ringWarm.cy
                PathArc { x: ringWarm.cx - ringWarm.outerR; y: ringWarm.cy; radiusX: ringWarm.outerR; radiusY: ringWarm.outerR }
                PathArc { x: ringWarm.cx + ringWarm.outerR; y: ringWarm.cy; radiusX: ringWarm.outerR; radiusY: ringWarm.outerR }
                PathMove { x: ringWarm.cx + ringWarm.innerR; y: ringWarm.cy }
                PathArc { x: ringWarm.cx - ringWarm.innerR; y: ringWarm.cy; radiusX: ringWarm.innerR; radiusY: ringWarm.innerR }
                PathArc { x: ringWarm.cx + ringWarm.innerR; y: ringWarm.cy; radiusX: ringWarm.innerR; radiusY: ringWarm.innerR }
            }
        }
        Shape {
            id: ringMid
            anchors.centerIn: parent
            width: 0; height: width; opacity: 0
            readonly property real strokeW: 3.0
            readonly property real outerR: Math.max(1, width / 2)
            readonly property real innerR: Math.max(0, outerR - strokeW)
            readonly property real cx: width / 2
            readonly property real cy: width / 2
            layer.enabled: true; layer.samples: 4; layer.smooth: true
            layer.textureSize: Qt.size(Math.max(1, width * 5), Math.max(1, height * 5))

            ShapePath {
                fillRule: ShapePath.OddEvenFill
                strokeWidth: -1
                fillGradient: ConicalGradient {
                    centerX: ringMid.cx; centerY: ringMid.cy; angle: 120
                    GradientStop { position: 0.00; color: "#00ff88" }
                    GradientStop { position: 0.17; color: "#00ddcc" }
                    GradientStop { position: 0.33; color: "#22aaff" }
                    GradientStop { position: 0.50; color: "#6644ff" }
                    GradientStop { position: 0.67; color: "#aa44ff" }
                    GradientStop { position: 0.83; color: "#44ddaa" }
                    GradientStop { position: 1.00; color: "#00ff88" }
                }
                startX: ringMid.cx + ringMid.outerR; startY: ringMid.cy
                PathArc { x: ringMid.cx - ringMid.outerR; y: ringMid.cy; radiusX: ringMid.outerR; radiusY: ringMid.outerR }
                PathArc { x: ringMid.cx + ringMid.outerR; y: ringMid.cy; radiusX: ringMid.outerR; radiusY: ringMid.outerR }
                PathMove { x: ringMid.cx + ringMid.innerR; y: ringMid.cy }
                PathArc { x: ringMid.cx - ringMid.innerR; y: ringMid.cy; radiusX: ringMid.innerR; radiusY: ringMid.innerR }
                PathArc { x: ringMid.cx + ringMid.innerR; y: ringMid.cy; radiusX: ringMid.innerR; radiusY: ringMid.innerR }
            }
        }
        Shape {
            id: ringCool
            anchors.centerIn: parent
            width: 0; height: width; opacity: 0
            readonly property real strokeW: 2.0
            readonly property real outerR: Math.max(1, width / 2)
            readonly property real innerR: Math.max(0, outerR - strokeW)
            readonly property real cx: width / 2
            readonly property real cy: width / 2
            layer.enabled: true; layer.samples: 4; layer.smooth: true
            layer.textureSize: Qt.size(Math.max(1, width * 5), Math.max(1, height * 5))

            ShapePath {
                fillRule: ShapePath.OddEvenFill
                strokeWidth: -1
                fillGradient: ConicalGradient {
                    centerX: ringCool.cx; centerY: ringCool.cy; angle: 240
                    GradientStop { position: 0.00; color: "#44ffcc" }
                    GradientStop { position: 0.17; color: "#2288ff" }
                    GradientStop { position: 0.33; color: "#6644ff" }
                    GradientStop { position: 0.50; color: "#cc44ff" }
                    GradientStop { position: 0.67; color: "#ff44aa" }
                    GradientStop { position: 0.83; color: "#ff8866" }
                    GradientStop { position: 1.00; color: "#44ffcc" }
                }
                startX: ringCool.cx + ringCool.outerR; startY: ringCool.cy
                PathArc { x: ringCool.cx - ringCool.outerR; y: ringCool.cy; radiusX: ringCool.outerR; radiusY: ringCool.outerR }
                PathArc { x: ringCool.cx + ringCool.outerR; y: ringCool.cy; radiusX: ringCool.outerR; radiusY: ringCool.outerR }
                PathMove { x: ringCool.cx + ringCool.innerR; y: ringCool.cy }
                PathArc { x: ringCool.cx - ringCool.innerR; y: ringCool.cy; radiusX: ringCool.innerR; radiusY: ringCool.innerR }
                PathArc { x: ringCool.cx + ringCool.innerR; y: ringCool.cy; radiusX: ringCool.innerR; radiusY: ringCool.innerR }
            }
        }

        // Conic aurora gradient overlay masked to the narwhal silhouette.
        // A Shape renders the gradient (GPU, no rasterization), and
        // MultiEffect masks it to the SVG icon's alpha channel.
        Item {
            id: auroraGradient
            anchors.centerIn: parent
            width: parent.width; height: parent.height
            visible: false
            layer.enabled: true; layer.smooth: true
            layer.textureSize: Qt.size(Math.max(1, width * 5), Math.max(1, height * 5))

            Shape {
                anchors.fill: parent
                ShapePath {
                    strokeWidth: -1
                    fillGradient: ConicalGradient {
                        centerX: auroraGradient.width / 2
                        centerY: auroraGradient.height / 2
                        angle: 30
                        GradientStop { position: 0.00; color: "#00ff88" }
                        GradientStop { position: 0.15; color: "#00ddcc" }
                        GradientStop { position: 0.30; color: "#4488ff" }
                        GradientStop { position: 0.50; color: "#aa44ff" }
                        GradientStop { position: 0.70; color: "#ff44aa" }
                        GradientStop { position: 0.85; color: "#ff8844" }
                        GradientStop { position: 1.00; color: "#00ff88" }
                    }
                    startX: 0; startY: 0
                    PathLine { x: auroraGradient.width; y: 0 }
                    PathLine { x: auroraGradient.width; y: auroraGradient.height }
                    PathLine { x: 0; y: auroraGradient.height }
                    PathLine { x: 0; y: 0 }
                }
            }
        }

        Item {
            id: auroraMask
            anchors.centerIn: parent
            width: parent.width; height: parent.height
            visible: false
            layer.enabled: true; layer.smooth: true
            layer.textureSize: Qt.size(Math.max(1, width * 5), Math.max(1, height * 5))

            Image {
                anchors.fill: parent
                source: Theme.iconNarwhal
                sourceSize: Qt.size(parent.width, parent.height)
            }
        }

        MultiEffect {
            id: iconAurora
            anchors.centerIn: parent
            width: narwhalIcon.width; height: narwhalIcon.height
            source: auroraGradient
            maskEnabled: true
            maskSource: auroraMask
            visible: false
        }

        SequentialAnimation {
            id: easterEgg

            // Nudge each ring's center by a small random offset so the ripples
            // don't share a single perfectly concentric origin — like droplets
            // on a water surface that land slightly apart.
            ScriptAction {
                script: {
                    function jitter() { return (Math.random() - 0.5) * Theme.px(5); }
                    ringWarm.anchors.horizontalCenterOffset = jitter();
                    ringWarm.anchors.verticalCenterOffset   = jitter();
                    ringMid.anchors.horizontalCenterOffset   = jitter();
                    ringMid.anchors.verticalCenterOffset     = jitter();
                    ringCool.anchors.horizontalCenterOffset   = jitter();
                    ringCool.anchors.verticalCenterOffset     = jitter();
                }
            }

            ParallelAnimation {
                // Ring 1 - full rainbow, expands furthest, thickest stroke
                NumberAnimation { target: ringWarm; property: "width";   from: Theme.px(12); to: Theme.px(180); duration: 2200; easing.type: Easing.OutCubic }
                NumberAnimation { target: ringWarm; property: "opacity"; from: 0.70;          to: 0;             duration: 2200; easing.type: Easing.InQuad }

                // Ring 2 - aurora, staggered 400ms, mid reach, gradient rotated 120°
                SequentialAnimation {
                    PauseAnimation { duration: 400 }
                    ParallelAnimation {
                        NumberAnimation { target: ringMid; property: "width";   from: Theme.px(8); to: Theme.px(120); duration: 1900; easing.type: Easing.OutCubic }
                        NumberAnimation { target: ringMid; property: "opacity"; from: 0.50;          to: 0;             duration: 1900; easing.type: Easing.InQuad }
                    }
                }

                // Ring 3 - cool aurora, staggered 800ms, least reach, gradient rotated 240°
                SequentialAnimation {
                    PauseAnimation { duration: 800 }
                    ParallelAnimation {
                        NumberAnimation { target: ringCool; property: "width";   from: Theme.px(6); to: Theme.px(72); duration: 1600; easing.type: Easing.OutCubic }
                        NumberAnimation { target: ringCool; property: "opacity"; from: 0.35;          to: 0;            duration: 1600; easing.type: Easing.InQuad }
                    }
                }

                // Conic aurora gradient on icon - smooth fade in, hold, fade out.
                SequentialAnimation {
                    PropertyAction  { target: iconAurora; property: "visible"; value: true }
                    PropertyAction  { target: iconAurora; property: "opacity"; value: 0 }
                    NumberAnimation { target: iconAurora; property: "opacity"; to: 0.5; duration: 300; easing.type: Easing.OutQuad }
                    PauseAnimation  { duration: 2000 }
                    NumberAnimation { target: iconAurora; property: "opacity"; to: 0; duration: 500; easing.type: Easing.InQuad }
                    PropertyAction  { target: iconAurora; property: "visible"; value: false }
                }
            }

            // Reset ring offsets so they return to centered.
            ScriptAction {
                script: {
                    ringWarm.anchors.horizontalCenterOffset = 0;
                    ringWarm.anchors.verticalCenterOffset   = 0;
                    ringMid.anchors.horizontalCenterOffset   = 0;
                    ringMid.anchors.verticalCenterOffset     = 0;
                    ringCool.anchors.horizontalCenterOffset   = 0;
                    ringCool.anchors.verticalCenterOffset     = 0;
                }
            }

            onFinished: {
                narwhalIcon.active = false;
            }
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                if (narwhalIcon.active) return;
                narwhalIcon.active = true;
                easterEgg.start();
            }
        }
    }

    // Title block
    ColumnLayout {
        spacing: 2

        Text {
            text: "seal"
            font.family: Theme.fontFamily
            font.pixelSize: Theme.fontSizeTitle
            font.bold: true
            color: Theme.accent
        }
        Text {
            text: "Password Manager"
            font.family: Theme.fontFamily
            font.pixelSize: Theme.fontSizeSubtitle
            color: Theme.accent2Dim
        }
    }

    // Theme toggle: sun in dark mode, moon in light mode.
    SvgIcon {
        source: Theme.dark ? Theme.iconSun : Theme.iconMoon
        width: Theme.iconSizeMedium
        height: Theme.iconSizeMedium
        color: themeArea.containsMouse ? Theme.accent : Theme.accentDim
        Behavior on color { ColorAnimation { duration: Theme.hoverDuration } }

        MouseArea {
            id: themeArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: Theme.toggle()
        }
    }

    // Spacer pushes vault control buttons to the right edge.
    Item { Layout.fillWidth: true }

    // Inline component matching TintedButton styling from ActionBar.
    // Uses the iconBtn palette with same-hue reduced-opacity disabled states.
    component HeaderButton: Button {
        id: hBtn
        property string faIcon: ""
        property color tintTop:       Theme.iconBtnTop
        property color tintEnd:       Theme.iconBtnEnd
        property color tintHoverTop:  Theme.iconBtnHoverTop
        property color tintHoverEnd:  Theme.iconBtnHoverEnd
        property color tintPressed:   Theme.iconBtnPressed
        property color tintText:      Theme.textIcon
        property color tintTextHover: Theme.textSecondary
        readonly property color _tintBorder: Qt.rgba(tintEnd.r, tintEnd.g, tintEnd.b, Math.min(tintEnd.a + 0.18, 1.0))
        leftPadding: 14
        rightPadding: 14

        // Disabled: same hue at reduced opacity.
        readonly property color _disText: Qt.rgba(tintText.r, tintText.g, tintText.b, 0.32)
        readonly property color _disTop:  Qt.rgba(tintTop.r, tintTop.g, tintTop.b, tintTop.a * 0.35)
        readonly property color _disEnd:  Qt.rgba(tintEnd.r, tintEnd.g, tintEnd.b, tintEnd.a * 0.35)

        HoverHandler { id: hBtnHover; cursorShape: Qt.PointingHandCursor }

        contentItem: Row {
            spacing: 6
            anchors.centerIn: parent
            SvgIcon {
                source: hBtn.faIcon
                width: Theme.iconSizeMedium
                height: Theme.iconSizeMedium
                color: !hBtn.enabled ? hBtn._disText : hBtn.hovered ? hBtn.tintTextHover : hBtn.tintText
                visible: hBtn.faIcon !== ""
                anchors.verticalCenter: parent.verticalCenter
                Behavior on color { ColorAnimation { duration: Theme.hoverDuration } }
            }
            Text {
                text: hBtn.text
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeMedium
                font.weight: Font.DemiBold
                color: !hBtn.enabled ? hBtn._disText : hBtn.hovered ? hBtn.tintTextHover : hBtn.tintText
                anchors.verticalCenter: parent.verticalCenter
                Behavior on color { ColorAnimation { duration: Theme.hoverDuration } }
            }
        }

        scale: pressed ? 0.97 : 1.0
        Behavior on scale { NumberAnimation { duration: 200; easing.type: Easing.OutBack; easing.overshoot: 2.0 } }

        background: Rectangle {
            implicitWidth: 100
            implicitHeight: 38
            radius: Theme.radiusMedium
            clip: true
            gradient: Gradient {
                GradientStop { position: 0; color: !hBtn.enabled ? hBtn._disTop : hBtn.pressed ? hBtn.tintPressed : hBtn.hovered ? hBtn.tintHoverTop : hBtn.tintTop; Behavior on color { ColorAnimation { duration: Theme.hoverDuration } } }
                GradientStop { position: 1; color: !hBtn.enabled ? hBtn._disEnd : hBtn.pressed ? hBtn.tintPressed : hBtn.hovered ? hBtn.tintHoverEnd : hBtn.tintEnd; Behavior on color { ColorAnimation { duration: Theme.hoverDuration } } }
            }
            border.width: 1
            border.color: !hBtn.enabled ? Theme.borderDim
                        : hBtn.hovered ? Theme.borderHover
                        : hBtn._tintBorder
            Behavior on border.color { ColorAnimation { duration: Theme.hoverDuration } }

            RippleEffect { id: hBtnRipple; baseColor: Qt.rgba(hBtn.tintText.r, hBtn.tintText.g, hBtn.tintText.b, 0.30) }
        }
        onPressed: hBtnRipple.trigger(hBtnHover.point.position.x, hBtnHover.point.position.y)
    }

    HeaderButton {
        text: "Load"
        faIcon: Theme.iconFolderOpen
        onClicked: root.loadClicked()
    }

    HeaderButton {
        text: "Save"
        faIcon: Theme.iconFloppyDisk
        enabled: root.vaultLoaded
        onClicked: root.saveClicked()
    }

    HeaderButton {
        text: "Unload"
        faIcon: Theme.iconEject
        enabled: root.vaultLoaded
        onClicked: root.unloadClicked()
    }
    }  // RowLayout
    }  // MouseArea
}  // Item root
