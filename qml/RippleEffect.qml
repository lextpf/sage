import QtQuick

// Ink ripple effect. Child of button background so it renders behind text.

Item {
    id: ripple
    anchors.fill: parent
    clip: true

    property color baseColor: Theme.rippleColor
    property real cx: 0
    property real cy: 0
    // Diagonal ensures circle covers every corner.
    property real maxRadius: Math.sqrt(width * width + height * height)

    // Resets and restarts animation from the press point.
    function trigger(pressX, pressY) {
        cx = pressX; cy = pressY;
        circle.scale = 0; circle.opacity = 1;
        anim.restart();
    }

    // Circle centered on (cx, cy), scaled from 0 to 1.
    Rectangle {
        id: circle
        x: ripple.cx - ripple.maxRadius
        y: ripple.cy - ripple.maxRadius
        width: ripple.maxRadius * 2
        height: ripple.maxRadius * 2
        radius: ripple.maxRadius
        color: ripple.baseColor
        scale: 0; opacity: 0
    }

    // Fade starts 80ms after scale for a brief fully-visible moment.
    ParallelAnimation {
        id: anim
        NumberAnimation { target: circle; property: "scale"; to: 1.0; duration: 420; easing.type: Easing.OutCubic }
        SequentialAnimation {
            PauseAnimation { duration: 80 }
            NumberAnimation { target: circle; property: "opacity"; to: 0.0; duration: 340; easing.type: Easing.OutCubic }
        }
    }
}
