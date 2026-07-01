import QtQuick 2.0
import harbour.tilem 1.0

// FIX (Oct 14, 2025 21:30 UTC): Direct Item root for QQuickView
// QQuickView + Page = crashes (Page expects PageStack context)
// Solution: Use simple Item as root with calculator components directly
Item {
    id: root
    width: 540
    height: 960

    property int pressCount: 0
    property int lastKeycode: -1

    Component.onCompleted: {
        QmlLogger.log("=== Root Item completed - all children ready ===")
        // NOW it's safe to load ROM (all components are constructed)
        if (typeof romFile !== 'undefined' && romFile !== "") {
            QmlLogger.log("Loading ROM: " + romFile)
            calcObj.load(romFile)
            QmlLogger.log("ROM loaded successfully")
            // FIX (Oct 14, 2025 21:56 UTC): QML Timers don't work with QQuickView on SailfishOS
            // Solution: Use C++ side to auto-press ON button after ROM boot
            // For now, user must manually tap ON button on screen to wake calculator
            QmlLogger.log("*** Calculator loaded in sleep mode - tap ON button to wake ***")
        }
    }

    Calc {
        id: calcObj

        // FIX (Oct 14, 2025 21:35 UTC): Don't load ROM in Component.onCompleted
        // Loading ROM starts calc thread which may access CalcScreen before it's ready
        // ROM loading moved to root Item's Component.onCompleted (after all children ready)

        onModelNameChanged: {
            QmlLogger.log("Model detected: " + modelName)
            var skinPath = "/usr/share/harbour-tilem/skins/" + modelName + ".skn"
            QmlLogger.log("Looking for skin: " + skinPath)
            skinId.skinFile = skinPath
        }
    }

    Skin {
        id: skinId
        skinFile: "/usr/share/harbour-tilem/skins/ti84p.skn"

        onSkinFileChanged: QmlLogger.log("Skin file: " + skinFile)
        Component.onCompleted: QmlLogger.log("Skin loaded: " + width + "x" + height)
    }

    SkinImage {
        id: skinImageId
        skin: skinId
        anchors.fill: parent
        keepAspect: true
        z: 0
        enabled: false

        Component.onCompleted: QmlLogger.log("SkinImage ready: " + width + "x" + height)
    }

    CalcScreen {
        id: calcScreen
        calc: calcObj
        x: skinImageId.lcdX
        y: skinImageId.lcdY
        width: (skinId && skinId.lcdW > 0) ? (skinId.lcdW * skinImageId.scaleX) : 240
        height: (skinId && skinId.lcdH > 0) ? (skinId.lcdH * skinImageId.scaleY) : 160
        z: 10
        enabled: false
    }

    // Touch handling
    MouseArea {
        anchors.fill: parent
        z: 1000000

        function keycode(mouse) {
            if (!skinImageId || !skinImageId.skin) {
                QmlLogger.log("ERROR: skinImageId or skin is null!")
                return -1
            }
            var nx = skinImageId.normalizeX(mouse.x)
            var ny = skinImageId.normalizeY(mouse.y)
            QmlLogger.log("Normalized coords: " + nx + ", " + ny)
            var kc = skinImageId.skin.keyCode(nx, ny)
            QmlLogger.log("Keycode result: " + kc)
            return kc
        }

        onPressed: {
            QmlLogger.log("Touch at: " + mouse.x + ", " + mouse.y)
            root.pressCount++
            QmlLogger.log("pressCount incremented to: " + root.pressCount)

            var kc = keycode(mouse)
            root.lastKeycode = kc
            QmlLogger.log("Keycode: " + kc)

            if (kc >= 0) {
                QmlLogger.log("Calling pressKey(" + kc + ")")
                calcObj.pressKey(kc)
            } else {
                QmlLogger.log("Invalid keycode, not calling pressKey")
            }
        }

        onReleased: {
            var kc = keycode(mouse)
            if (kc >= 0) {
                calcObj.releaseKey(kc)
            }
        }
    }
}
