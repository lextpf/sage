import QtQuick
import QtQuick.Controls.impl as Impl

// Thin wrapper around Qt's IconImage for runtime SVG color tinting.
//
// Why not plain Image? Image renders SVGs at their intrinsic size and rasterizes
// at load time, so recoloring requires a different asset per color. IconImage
// (from QtQuick.Controls.impl) supports a `color` property that tints the SVG
// at render time, allowing a single monochrome SVG asset to be reused across
// themes and states (normal, hover, disabled, accent, etc.).
//
// sourceSize is locked to width x height so the SVG rasterizes at the exact
// display size - no blurry upscaling or wasted overdraw from oversized textures.

Impl.IconImage {
    sourceSize: Qt.size(width, height)
    fillMode: Image.PreserveAspectFit
}
