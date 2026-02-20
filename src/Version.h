#pragma once

/// Major version number.
#define SAGE_VERSION_MAJOR 0
/// Minor version number.
#define SAGE_VERSION_MINOR 1
/// Patch version number.
#define SAGE_VERSION_PATCH 0
/// Release version number.
#define SAGE_VERSION_RELEASE 0

/**
 * Stringify version components.
 *
 * @param major Major version.
 * @param minor Minor version.
 * @param patch Patch version.
 * @param release Release version.
 *
 * @see SAGE_VERSION
 */
#define SAGE_VERSION_STRINGIFY(major, minor, patch, release) \
    #major "." #minor "." #patch "." #release

/// Complete version string (e.g., "0.1.0.0").
#define SAGE_VERSION \
    SAGE_VERSION_STRINGIFY( \
        SAGE_VERSION_MAJOR, \
        SAGE_VERSION_MINOR, \
        SAGE_VERSION_PATCH, \
        SAGE_VERSION_RELEASE)
