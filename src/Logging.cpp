#ifdef USE_QT_UI

#include "Logging.h"

#include <QtCore/QDateTime>
#include <QtCore/QByteArray>

#include <cstdio>

Q_LOGGING_CATEGORY(logBackend, "sage.backend")
Q_LOGGING_CATEGORY(logVault,   "sage.vault")
Q_LOGGING_CATEGORY(logCrypto,  "sage.crypto")
Q_LOGGING_CATEGORY(logFill,    "sage.fill")
Q_LOGGING_CATEGORY(logFile,    "sage.file")
Q_LOGGING_CATEGORY(logApp,     "sage.app")

static void sageMessageHandler(QtMsgType type, const QMessageLogContext& ctx, const QString& msg)
{
    const char* lvl = "???";
    switch (type)
    {
        case QtDebugMsg:    lvl = "DBG"; break;
        case QtInfoMsg:     lvl = "INF"; break;
        case QtWarningMsg:  lvl = "WRN"; break;
        case QtCriticalMsg: lvl = "ERR"; break;
        case QtFatalMsg:    lvl = "FTL"; break;
    }

    const QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    const char* cat = ctx.category ? ctx.category : "default";

    const QByteArray line = QStringLiteral("[%1] [%2] [%3] %4\n")
        .arg(timestamp, QLatin1String(lvl), QLatin1String(cat), msg)
        .toUtf8();

    fwrite(line.constData(), 1, line.size(), stderr);
    fflush(stderr);

    if (type == QtFatalMsg)
        abort();
}

void installSageMessageHandler()
{
    qInstallMessageHandler(sageMessageHandler);
}

#endif // USE_QT_UI
