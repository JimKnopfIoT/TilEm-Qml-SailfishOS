import QtQuick 2.0
import Sailfish.Silica 1.0
import "pages"

ApplicationWindow {
    id: window

    // CRITICAL FIX (Oct 14, 2025 21:02 UTC): QQmlApplicationEngine requires explicit visible: true
    // Unlike QQuickView which shows automatically, QQmlApplicationEngine doesn't call show()
    // ApplicationWindow must set visible: true to appear on screen
    visible: true

    property string appDir: StandardPaths.home + "/.local/share/harbour-tilem"

    initialPage: Component {
        CalcPage {
            romFile: StandardPaths.home + "/Documents/TI83.ROM"
        }
    }
    cover: Qt.resolvedUrl("cover/CoverPage.qml")
    allowedOrientations: defaultAllowedOrientations
}
