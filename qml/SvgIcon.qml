import QtQuick
import QtQuick.Controls.impl as Impl

// IconImage wrapper with color tinting. Renders SVG at exact sourceSize.

Impl.IconImage {
    sourceSize: Qt.size(width, height)
    fillMode: Image.PreserveAspectFit
}
