#ifdef QT_QML_DEBUG
#include <QtQuick>
#endif

#include <QScopedPointer>
#include <QGuiApplication>
#include <QQuickView>
#include <QQmlEngine>
#include <QQmlContext>
#include <QStandardPaths>
#include <sailfishapp.h>

// TilEm C++ classes
#include "calc.h"
#include "calcscreen.h"
#include "calcthread.h"
#include "calclink.h"
#include "skin.h"
#include "skinimage.h"
#include "qmllogger.h"

int main(int argc, char *argv[])
{
    // REVERT TO QQuickView (Oct 14, 2025 21:06 UTC)
    // QQmlApplicationEngine causes XCB platform plugin errors on SailfishOS
    // SailfishOS apps MUST use SailfishApp::createView() for proper Wayland support
    QScopedPointer<QGuiApplication> app(SailfishApp::application(argc, argv));
    QScopedPointer<QQuickView> view(SailfishApp::createView());

    // Fill the letterbox above/below the skin with black instead of the default
    // white (the 9:16 skin is centred on the taller 9:21 screen).
    view->setColor(QColor(0, 0, 0));

    // Register TilEm C++ types with QML BEFORE loading QML
    qmlRegisterType<Calc>("harbour.tilem", 1, 0, "Calc");
    qmlRegisterType<CalcScreen>("harbour.tilem", 1, 0, "CalcScreen");
    // CalcLink requires Calc* in constructor, created by Calc itself, not directly from QML
    qmlRegisterType<Skin>("harbour.tilem", 1, 0, "Skin");
    qmlRegisterType<SkinImage>("harbour.tilem", 1, 0, "SkinImage");

    // Register QmlLogger singleton for proper QML logging (console.log doesn't appear in journalctl)
    QmlLogger *qmlLogger = new QmlLogger();
    view->rootContext()->setContextProperty("QmlLogger", qmlLogger);

    qDebug() << "[MAIN] QmlLogger registered in QML context";

    // Set up data directories
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    view->rootContext()->setContextProperty("appDataDir", dataDir);

    qDebug() << "[MAIN] Data directory:" << dataDir;

    // FIX (Oct 14, 2025 21:30 UTC): Load simple Item root, not Page
    // QQuickView + Page = crashes (Page expects PageStack which doesn't exist)
    // Solution: Load Main.qml (simple Item) instead of CalcPage.qml (Page)
    QString romFile = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/Documents/TI83.ROM";
    view->rootContext()->setContextProperty("romFile", romFile);

    qDebug() << "[MAIN] ROM file path:" << romFile;

    // Load Main.qml (Item root) which works correctly with QQuickView
    view->setSource(SailfishApp::pathTo("qml/Main.qml"));

    qDebug() << "[MAIN] QML loaded from:" << view->source();

    // Check for QML errors
    if (view->status() == QQuickView::Error) {
        qCritical() << "[MAIN] QML Error! Failed to load main QML file";
        QList<QQmlError> errors = view->errors();
        for (const QQmlError &error : errors) {
            qCritical() << "[MAIN] QML Error:" << error.toString();
        }
        return -1;
    }

    // Print QML errors and warnings to journal for debugging
    QObject::connect(view->engine(), &QQmlEngine::warnings,
        [](const QList<QQmlError> &warnings) {
            for (const QQmlError &warning : warnings) {
                qWarning() << "[MAIN] QML Warning:" << warning.toString();
            }
        });

    qDebug() << "[MAIN] Showing window...";
    view->show();

    qDebug() << "[MAIN] Starting event loop...";
    return app->exec();
}
