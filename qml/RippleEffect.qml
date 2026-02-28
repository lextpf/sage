import QtQuick

// Material Design-inspired ink ripple effect for button press feedback.
//
// How it works: a circle is placed at the press coordinates and scaled from
// 0 to 1 over 420ms. The fade-out starts 80ms later so the ripple is briefly
// fully visible at its maximum size before disappearing - this creates the
// characteristic "ink splash" feel. The parent clips the circle so it stays
// within the button bounds.
//
// Usage: embed as a child of a button's `background` Rectangle and call
// trigger(pressX, pressY) from the button's onPressed handler. The ripple
// renders behind the button's contentItem (text/icon) because it lives
// inside the background.

Item {
    id: ripple
    anchors.fill: parent
    clip: true

    property color baseColor: Theme.rippleColor
    property real cx: 0
    property real cy: 0
    // maxRadius uses the diagonal so the circle covers every corner of
    // the rectangular parent regardless of where the press originated.
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
