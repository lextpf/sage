#ifdef USE_QT_UI

#include "QmlMain.h"
#include "Backend.h"

#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>
#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>
#include <QtQuick/QQuickWindow>
#include <QtCore/QCoreApplication>
#include <QtQuickControls2/QQuickStyle>

#include <windows.h>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

#include <algorithm>
#include <iostream>

static void applyDarkTitleBar(QQuickWindow* window)
{
    if (!window)
        return;
    HWND hwnd = (HWND)window->winId();
    if (!hwnd)
        return;

    BOOL darkMode = TRUE;
    DwmSetWindowAttribute(hwnd, 20, &darkMode, sizeof(darkMode)); // DWMWA_USE_IMMERSIVE_DARK_MODE

    COLORREF darkColor = RGB(10, 13, 18); // matches Theme.bgDeep
    DwmSetWindowAttribute(hwnd, 34, &darkColor, sizeof(darkColor)); // DWMWA_CAPTION_COLOR
    DwmSetWindowAttribute(hwnd, 35, &darkColor, sizeof(darkColor)); // DWMWA_BORDER_COLOR

    COLORREF lightTextColor = RGB(245, 247, 251); // matches Theme.textPrimary
    DwmSetWindowAttribute(hwnd, 36, &lightTextColor, sizeof(lightTextColor)); // DWMWA_TEXT_COLOR
}

static qreal computeUiScale()
{
    QScreen* screen = QGuiApplication::primaryScreen();
    if (!screen)
        return 1.0;

    // Match the existing QWidget baseline: 1920 physical pixels = 1.0 scale.
    const qreal physicalWidth = static_cast<qreal>(screen->size().width()) * screen->devicePixelRatio();
    if (physicalWidth <= 1920.0)
        return 1.0;

    const qreal rawScale = physicalWidth / 1920.0;
    // Text-only boost: increase readability on 4K without resizing the whole layout.
    const qreal textScale = 1.0 + (rawScale - 1.0) * 0.45;
    return std::clamp(textScale, 1.0, 1.5);
}

int RunQMLMode(int argc, char* argv[])
{
    QQuickStyle::setStyle("Basic"); // non-native style so custom theming takes full effect

    QGuiApplication app(argc, argv);
    app.setApplicationName("sage");
    app.setOrganizationName("sage");
    const qreal uiScale = computeUiScale();

    sage::Backend backend;
    QQmlApplicationEngine engine;
    QObject::connect( // abort on QML load failure instead of showing a blank window
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        [] { QCoreApplication::exit(1); },
        Qt::QueuedConnection
    );

    engine.rootContext()->setContextProperty("UiScale", uiScale); // DPI-aware text scale factor
    engine.rootContext()->setContextProperty("Backend", &backend); // vault + crypto operations
    engine.loadFromModule("sage", "Main");

    if (engine.rootObjects().isEmpty())
    {
        std::cerr << "Failed to load QML" << std::endl;
        return 1;
    }

    // Win32 title bar must be styled after the window is created
    QObject* rootObject = engine.rootObjects().first();
    QQuickWindow* window = qobject_cast<QQuickWindow*>(rootObject);
    if (window)
    {
        applyDarkTitleBar(window);
    }

    return app.exec();
}

#endif // USE_QT_UI
