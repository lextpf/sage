pragma Singleton
import QtQuick

QtObject {
    function detectTextScale() {
        var screens = (Qt.application && Qt.application.screens) ? Qt.application.screens : [];
        if (!screens || screens.length === 0)
            return 1.0;

        var primary = screens[0];
        var dpr = primary.devicePixelRatio > 0 ? primary.devicePixelRatio : 1.0;
        var physicalWidth = primary.width * dpr;
        if (physicalWidth <= 1920.0)
            return 1.0;

        var rawScale = physicalWidth / 1920.0;
        var textScale = 1.0 + (rawScale - 1.0) * 0.45;
        return Math.max(1.0, Math.min(textScale, 1.5));
    }

    readonly property real textScale: detectTextScale()

    readonly property FontLoader _interFont: FontLoader {
        source: "qrc:/qt/qml/sage/assets/fonts/Inter-VariableFont_opsz,wght.ttf"
    }

    function px(base) {
        return Math.max(1, Math.round(base * textScale));
    }

    // ── Background ──────────────────────────────────────────
    readonly property color bgDeep:       "#0a0d12"
    readonly property color bgCard:       Qt.rgba(0.071, 0.086, 0.118, 0.7)
    readonly property color bgCardEnd:    Qt.rgba(0.055, 0.071, 0.102, 0.85)
    readonly property color bgDialog:     "#0e1218"
    readonly property color bgInput:      Qt.rgba(0.047, 0.063, 0.094, 0.9)
    readonly property color bgInputFocus: Qt.rgba(0.059, 0.078, 0.122, 0.95)
    readonly property color bgOverlay:    Qt.rgba(0, 0, 0, 0.55)

    // ── Region Backgrounds ─────────────────────────────────
    readonly property color bgHeaderTop:  Qt.rgba(0.118, 0.133, 0.180, 0.95)
    readonly property color bgHeaderEnd:  Qt.rgba(0.094, 0.110, 0.149, 0.98)
    readonly property color bgFooterTop:  Qt.rgba(0.055, 0.071, 0.102, 0.95)
    readonly property color bgFooterEnd:  Qt.rgba(0.039, 0.051, 0.071, 1.0)

    // ── Icon Button (header bar) ───────────────────────────
    readonly property color iconBtnTop:        Qt.rgba(0.086, 0.106, 0.149, 0.8)
    readonly property color iconBtnEnd:        Qt.rgba(0.063, 0.078, 0.118, 0.85)
    readonly property color iconBtnHoverTop:   Qt.rgba(0.118, 0.157, 0.243, 0.9)
    readonly property color iconBtnHoverEnd:   Qt.rgba(0.094, 0.125, 0.204, 0.92)
    readonly property color iconBtnPressed:    Qt.rgba(0.055, 0.071, 0.110, 0.95)

    // ── Ghost / Secondary Button ───────────────────────────
    readonly property color ghostBtnTop:        Qt.rgba(0.078, 0.094, 0.141, 0.6)
    readonly property color ghostBtnEnd:        Qt.rgba(0.063, 0.078, 0.118, 0.7)
    readonly property color ghostBtnHoverTop:   Qt.rgba(0.157, 0.220, 0.392, 0.45)
    readonly property color ghostBtnHoverEnd:   Qt.rgba(0.118, 0.165, 0.314, 0.5)
    readonly property color ghostBtnPressed:    Qt.rgba(0.165, 0.329, 0.761, 0.2)

    // ── Disabled Primary Button ────────────────────────────
    readonly property color btnDisabledTop: "#1a2035"
    readonly property color btnDisabledBot: "#151b2c"

    // ── Accent ──────────────────────────────────────────────
    readonly property color accent:        "#8fb5ff"
    readonly property color accentBright:  "#5580ff"
    readonly property color accentDim:     "#5a6175"
    readonly property color btnGradTop:    "#4272f5"
    readonly property color btnGradBot:    "#335fdd"
    readonly property color btnHoverTop:   "#5580ff"
    readonly property color btnHoverBot:   "#4272f5"
    readonly property color btnPressTop:   "#2f54c2"
    readonly property color btnPressBot:   "#2648a8"

    // ── Fill Armed (orange) ─────────────────────────────────
    readonly property color fillArmedTop:      "#d06a00"
    readonly property color fillArmedEnd:      "#b85c00"
    readonly property color fillArmedHoverTop: "#e07700"
    readonly property color fillArmedHoverEnd: "#c86500"
    readonly property color fillArmedPressTop: "#b85c00"
    readonly property color fillArmedPressEnd: "#a05000"
    readonly property color fillArmedDot:      "#ff8800"
    readonly property color borderFillArmed:   Qt.rgba(1.0, 0.6, 0.1, 0.4)

    // ── Text ────────────────────────────────────────────────
    readonly property color textPrimary:   "#f5f7fb"
    readonly property color textSecondary: "#d8dce8"
    readonly property color textMuted:     "#5a6175"
    readonly property color textDisabled:  "#4a5068"
    readonly property color textSubtle:    "#7a8298"
    readonly property color textIcon:      "#99a0b8"
    readonly property color textGhost:     "#c4cae0"
    readonly property color textOnAccent:  "#ffffff"
    readonly property color textPlaceholder: "#444a5e"

    // ── Borders ─────────────────────────────────────────────
    readonly property color borderDim:       Qt.rgba(0.561, 0.710, 1.0, 0.06)
    readonly property color borderSoft:      Qt.rgba(0.561, 0.710, 1.0, 0.10)
    readonly property color borderSubtle:    Qt.rgba(0.561, 0.710, 1.0, 0.12)
    readonly property color borderInput:     Qt.rgba(0.561, 0.710, 1.0, 0.15)
    readonly property color borderMedium:    Qt.rgba(0.561, 0.710, 1.0, 0.18)
    readonly property color borderBtn:       Qt.rgba(0.561, 0.710, 1.0, 0.2)
    readonly property color borderHover:     Qt.rgba(0.561, 0.710, 1.0, 0.22)
    readonly property color borderHighlight: Qt.rgba(0.561, 0.710, 1.0, 0.25)
    readonly property color borderBright:    Qt.rgba(0.561, 0.710, 1.0, 0.35)
    readonly property color borderPressed:   Qt.rgba(0.357, 0.498, 0.961, 0.3)
    readonly property color borderFocusHover: Qt.rgba(0.357, 0.498, 0.961, 0.4)
    readonly property color borderFocus:     Qt.rgba(0.357, 0.498, 0.961, 0.7)

    // ── Selection ───────────────────────────────────────────
    readonly property color selectionBg:     Qt.rgba(0.2, 0.373, 0.867, 0.35)
    readonly property color selectionHover:  Qt.rgba(0.561, 0.710, 1.0, 0.08)
    readonly property color selectionActive: Qt.rgba(0.2, 0.373, 0.867, 0.4)

    // ── Scrollbar ───────────────────────────────────────────
    readonly property color scrollThumb: Qt.rgba(0.235, 0.275, 0.392, 0.5)

    // ── Animation ──────────────────────────────────────────
    readonly property int hoverDuration:  150
    readonly property int pressDuration:  80

    // ── Dimensions ──────────────────────────────────────────
    readonly property int radiusSmall:  8
    readonly property int radiusMedium: 10
    readonly property int radiusLarge:  12

    readonly property int spacingSmall:  10
    readonly property int spacingMedium: 14
    readonly property int spacingLarge:  20
    readonly property int spacingXL:     36

    // ── Fonts ───────────────────────────────────────────────
    readonly property string fontFamily:     _interFont.name !== "" ? _interFont.name : "Inter"
    readonly property string fontMono:       "Consolas"
    readonly property string fontEmoji:      "Segoe UI Emoji"
    readonly property int fontSizeSmall:     px(10)
    readonly property int fontSizeMedium:    px(11)
    readonly property int fontSizeLarge:     px(12)
    readonly property int fontSizeTitle:     px(26)
    readonly property int fontSizeSubtitle:  px(11)
    readonly property int iconSizeSmall:     px(13)
    readonly property int iconSizeMedium:    px(15)

    // ── Icon SVG Paths ──────────────────────────────────
    readonly property string iconLock:         "qrc:/qt/qml/sage/assets/svgs/lock.svg"
    readonly property string iconLockOpen:     "qrc:/qt/qml/sage/assets/svgs/lock-open.svg"
    readonly property string iconPlus:         "qrc:/qt/qml/sage/assets/svgs/plus.svg"
    readonly property string iconPen:          "qrc:/qt/qml/sage/assets/svgs/pen.svg"
    readonly property string iconTrash:        "qrc:/qt/qml/sage/assets/svgs/trash.svg"
    readonly property string iconFloppyDisk:   "qrc:/qt/qml/sage/assets/svgs/floppy-disk.svg"
    readonly property string iconFolderOpen:   "qrc:/qt/qml/sage/assets/svgs/folder-open.svg"
    readonly property string iconEject:        "qrc:/qt/qml/sage/assets/svgs/eject.svg"
    readonly property string iconKeyboard:     "qrc:/qt/qml/sage/assets/svgs/keyboard.svg"
    readonly property string iconKey:          "qrc:/qt/qml/sage/assets/svgs/key.svg"
    readonly property string iconSearch:       "qrc:/qt/qml/sage/assets/svgs/magnifying-glass.svg"
    readonly property string iconShieldHalved: "qrc:/qt/qml/sage/assets/svgs/shield-halved.svg"
    readonly property string iconService:      "qrc:/qt/qml/sage/assets/svgs/database.svg"
    readonly property string iconUsername:     "qrc:/qt/qml/sage/assets/svgs/user.svg"
    readonly property string iconPassword:     "qrc:/qt/qml/sage/assets/svgs/lock.svg"
    readonly property string iconCamera:       "qrc:/qt/qml/sage/assets/svgs/camera-web.svg"
    readonly property string iconFingerprint:  "qrc:/qt/qml/sage/assets/svgs/fingerprint.svg"
    readonly property string iconEye:          "qrc:/qt/qml/sage/assets/svgs/eye.svg"
    readonly property string iconEyeSlash:     "qrc:/qt/qml/sage/assets/svgs/eye-slash.svg"
    readonly property string iconCrosshairs:   "qrc:/qt/qml/sage/assets/svgs/crosshairs.svg"
}
