import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.tilem 1.0

Page {
    id: page
    // CRITICAL FIX (Oct 14, 2025 21:15 UTC): romFile now comes from context property, not Page property
    // main.cpp sets this via view->rootContext()->setContextProperty("romFile", ...)
    property bool pageInitialized: false  // Track if we've run initialization
    property int pressCount: 0  // DEBUG: Count button presses
    property int lastKeycode: -1  // DEBUG: Last keycode pressed

    // EMERGENCY TEST (Oct 14, 2025 20:49 UTC): Check if Page blocks touch
    // SailFish Page may not propagate touch events to children by default
    focus: true
    Keys.onPressed: console.log("!!!!! KEY PRESSED: " + event.key)

    // ARM64 FIX (Oct 13, 2025 10:00 UTC): Component.onCompleted NEVER fires on SailfishOS!
    // Workaround: Use onStatusChanged with PageStatus.Active to trigger initialization
    // This is the only reliable lifecycle hook that works on SailfishOS ApplicationWindow

    // REMOVED (Oct 14, 2025 21:25 UTC): onStatusChanged moved to onWidthChanged
    // ROM loading in onStatusChanged was causing crash because Item wrapper wasn't initialized yet
    // PageStatus.Active may fire before Item children (calcObj) are fully constructed
    // Moved ROM loading to onWidthChanged which fires after all children are ready

    // VISUAL TEST: Full-screen overlay to confirm QML rendering
    // ARM64 FIX (Oct 13, 2025 10:30 UTC): DISABLED - blocks touch events
    // Even without MouseArea, high z-index Rectangle blocks event delivery
    Rectangle {
        id: visualTest
        anchors.fill: parent
        color: "green"
        opacity: 0.3
        z: 999999
        visible: false  // DISABLED - was blocking touch input

        Text {
            anchors.centerIn: parent
            text: "QML RENDERING WORKS\nTap anywhere to test touch"
            font.pixelSize: 40
            color: "white"
            font.bold: true
        }

        Component.onCompleted: {
            QmlLogger.log("=== VISUAL TEST OVERLAY CREATED ===")
        }
    }

    // REMOVED (Oct 14, 2025 21:18 UTC): onRomFileChanged doesn't work with context properties
    // Context properties set via setContextProperty() are not QML properties and don't emit change signals
    // ROM loading now happens in onStatusChanged when page becomes active

    // Hide page header for full calculator display
    allowedOrientations: Orientation.All

    // CRITICAL FIX (Oct 14, 2025 20:50 UTC): Wrap all content in Item
    // SailfishOS Page may not properly propagate touch events to direct children
    // Solution: Use Item wrapper like harbour-machines does
    Item {
        anchors.fill: parent

    Calc {
        id: calcObj

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

        onSkinFileChanged: QmlLogger.log("Skin file changed to: " + skinFile)
        onSkinSettingsLoaded: QmlLogger.log("Skin settings loaded!")
        onWidthChanged: QmlLogger.log("Skin width: " + width)
        onHeightChanged: QmlLogger.log("Skin height: " + height)
        Component.onCompleted: QmlLogger.log("Skin completed, file: " + skinFile + " size: " + width + "x" + height)
    }

    // DISABLED (Oct 9, 2025 21:30 UTC): IconButton overlays removed for testing
    // These require Wayland/Lipstick theme image provider which may cause crashes
    // if compositor features were removed during testing.
    //
    // Reset button in top-right corner (PullDownMenu doesn't work without SilicaFlickable)
    // IconButton {
    //     anchors.top: parent.top
    //     anchors.right: parent.right
    //     anchors.topMargin: Theme.paddingLarge
    //     anchors.rightMargin: Theme.paddingLarge
    //     icon.source: "image://theme/icon-m-refresh"
    //     z: 200
    //
    //     onClicked: {
    //         remorse.execute(qsTr("Resetting"), function() {
    //             calcObj.reset()
    //         })
    //     }
    // }
    //
    // // Close button in top-left corner
    // IconButton {
    //     anchors.top: parent.top
    //     anchors.left: parent.left
    //     anchors.topMargin: Theme.paddingLarge
    //     anchors.leftMargin: Theme.paddingLarge
    //     icon.source: "image://theme/icon-m-back"
    //     z: 200
    //
    //     onClicked: pageStack.pop()
    // }

    SkinImage {
        id: skinImageId
        skin: skinId
        anchors.fill: parent
        keepAspect: true  // FIX (Oct 13, 2025 22:15 UTC): CRITICAL - false breaks touch coordinate mapping!
        z: 0  // Background layer
        enabled: false  // FIX (Oct 13, 2025): Disable all input - visual only!

        onWidthChanged: QmlLogger.log("SkinImage width: " + width)
        onHeightChanged: QmlLogger.log("SkinImage height: " + height)
        onHasImageChanged: QmlLogger.log("SkinImage hasImage: " + hasImage)
        Component.onCompleted: {
            QmlLogger.log("SkinImage completed - size: " + width + "x" + height)
            QmlLogger.log("Skin object size: " + skinId.width + "x" + skinId.height)
        }
    }

    CalcScreen {
        id: calcScreen
        calc: calcObj
        x: skinImageId.lcdX
        y: skinImageId.lcdY
        // FIX (Oct 12, 2025 21:53 UTC): Use skin file's LCD area dimensions with correct scaling
        // skinId.lcdW and skinId.lcdH are the dimensions of the gray LCD area in the skin
        // IMPORTANT: Use scaleX for width and scaleY for height (they can be different!)
        width: (skinId && skinId.lcdW > 0) ? (skinId.lcdW * skinImageId.scaleX) : 240
        height: (skinId && skinId.lcdH > 0) ? (skinId.lcdH * skinImageId.scaleY) : 160
        z: 10  // LCD on top of skin image

        // CRITICAL: Don't accept mouse events - let them fall through to MouseArea
        enabled: false  // Disable all input events, only render

        onWidthChanged: QmlLogger.log("CalcScreen width: " + width)
        onHeightChanged: QmlLogger.log("CalcScreen height: " + height)
    }

    // DEBUG (Oct 13, 2025): Red rectangle removed - no longer needed after z-index fix

    // DEBUG TEST: Visual feedback for button presses
    Rectangle {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        width: 250
        height: 100
        color: "red"
        opacity: 0.8
        z: 500000
        visible: true

        Column {
            anchors.centerIn: parent
            spacing: 5

            Text {
                text: "BTNS: " + skinId.getButtonCount() + " | PRESSES: " + page.pressCount
                color: "white"
                font.pixelSize: 14
                font.bold: true
            }

            Text {
                text: "LAST KEY: " + page.lastKeycode
                color: "yellow"
                font.pixelSize: 14
                font.bold: true
            }
        }
    }

    // DEBUG (Oct 13, 2025 11:00 UTC): Visual button hitbox overlay
    // Shows where the skin file says buttons are located
    // FIX (Oct 13, 2025 11:20 UTC): Don't use translateX/Y - button coords are already in skin space
    // Just apply scaling and offset directly
    Repeater {
        model: skinId.getButtonCount()
        delegate: Rectangle {
            x: skinImageId.xOffset() + (skinId.getButtonX(index) * skinImageId.scaleX)
            y: skinImageId.yOffset() + (skinId.getButtonY(index) * skinImageId.scaleY)
            width: skinId.getButtonWidth(index) * skinImageId.scaleX
            height: skinId.getButtonHeight(index) * skinImageId.scaleY
            color: "transparent"
            border.color: "cyan"
            border.width: 2
            z: 500000  // Below MouseArea but above everything else
            opacity: 0.7
            visible: true  // Set to false to disable overlay

            Text {
                anchors.centerIn: parent
                text: index
                color: "yellow"
                font.pixelSize: 12
                font.bold: true
            }
        }
    }

    // ARM64 FIX (Oct 12, 2025 23:21 UTC): MouseArea MUST be direct child of Page, not SkinImage
    // When MouseArea is child of SkinImage, touch events don't reach it because SkinImage
    // (a custom C++ QQuickPaintedItem) doesn't propagate mouse events to children by default.
    // Solution: Make MouseArea a sibling of SkinImage at Page level, with high z-index.
    // EMERGENCY FIX (Oct 14, 2025 20:42 UTC): Simplify to bare minimum MouseArea for testing
    // Remove all complex properties and just test basic touch detection
    MouseArea {
        id: testMouseArea
        anchors.fill: parent
        z: 9999999

        onClicked: {
            console.log("!!!!! TOUCH DETECTED AT: " + mouse.x + ", " + mouse.y)
            page.pressCount++
        }
    }

    MouseArea {
        id: mainMouseArea
        anchors.fill: parent
        z: 1000000  // FIX (Oct 13, 2025): Use harbour-machines pattern - extremely high z-index
        enabled: true  // Explicitly enable
        visible: true  // CRITICAL FIX (Oct 13, 2025 13:40 UTC): Force visible - was inheriting false from somewhere
        hoverEnabled: false  // DISABLE - may be causing issues
        preventStealing: false  // DISABLE - may be blocking events

        property bool mouseAreaInitialized: false

        function keycode(mouse) {
            QmlLogger.log("===== COORDINATE TRANSFORM DIAGNOSTICS =====")
            QmlLogger.log("Raw touch: mouse.x=" + mouse.x + " mouse.y=" + mouse.y)
            QmlLogger.log("Page dimensions: " + page.width + "x" + page.height)
            QmlLogger.log("SkinImage: width=" + skinImageId.width + " height=" + skinImageId.height)
            QmlLogger.log("SkinImage draw: drawWidth=" + skinImageId.drawWidth + " drawHeight=" + skinImageId.drawHeight)
            QmlLogger.log("SkinImage scale: scaleX=" + skinImageId.scaleX + " scaleY=" + skinImageId.scaleY)
            QmlLogger.log("SkinImage offset: xOffset=" + skinImageId.xOffset() + " yOffset=" + skinImageId.yOffset())
            QmlLogger.log("Skin raw dimensions: " + skinId.width + "x" + skinId.height)

            var nx = skinImageId.normalizeX(mouse.x)
            var ny = skinImageId.normalizeY(mouse.y)
            QmlLogger.log("Normalized coordinates: nx=" + nx + " ny=" + ny)

            QmlLogger.log("First button: x=" + skinId.getButtonX(0) + " y=" + skinId.getButtonY(0) + " w=" + skinId.getButtonWidth(0) + " h=" + skinId.getButtonHeight(0))

            var kc = skinImageId.skin.keyCode(nx, ny)
            QmlLogger.log("KeyCode from skin: " + kc)
            QmlLogger.log("===========================================")
            return kc
        }

        onPressed: {
            QmlLogger.log("=== MOUSEAREA PRESSED! Touch event received! ===")
            QmlLogger.log("Mouse position: " + mouse.x + ", " + mouse.y)

            // Get keycode from touch position
            var kc = keycode(mouse)
            QmlLogger.log("Keycode: " + kc)

            // DEBUG: Update visual feedback
            page.pressCount++
            page.lastKeycode = kc

            if (kc >= 0) {
                QmlLogger.log("Calling calcObj.pressKey(" + kc + ")")
                calcObj.pressKey(kc)
            } else {
                QmlLogger.log("Invalid keycode, ignoring press")
            }
        }

        onReleased: {
            QmlLogger.log("=== MOUSEAREA RELEASED! ===")

            // Get keycode from release position
            var kc = keycode(mouse)
            if (kc >= 0) {
                QmlLogger.log("Calling calcObj.releaseKey(" + kc + ")")
                calcObj.releaseKey(kc)
            }
        }

        // ARM64 FIX (Oct 13, 2025 10:05 UTC): Component.onCompleted workaround
        // Use onVisibleChanged instead - this DOES fire on SailfishOS
        onVisibleChanged: {
            QmlLogger.log("MouseArea visible changed: " + visible)

            if (visible && !mouseAreaInitialized) {
                QmlLogger.log("*** MouseArea initialization (Component.onCompleted workaround) ***")
                QmlLogger.log("MouseArea dimensions: " + width + "x" + height + " z:" + z)
                QmlLogger.log("MouseArea enabled: " + enabled + " parent: " + parent)
                mouseAreaInitialized = true
                QmlLogger.log("*** MouseArea ready for touch input ***")
            }
        }

        onEnabledChanged: {
            QmlLogger.log("MouseArea enabled changed: " + enabled)
        }

        // DIAGNOSTIC (Oct 13, 2025): Additional logging for touch debugging
        onWidthChanged: {
            QmlLogger.log("MouseArea width changed: " + width)
        }

        onHeightChanged: {
            QmlLogger.log("MouseArea height changed: " + height)
        }

        onContainsMouseChanged: {
            QmlLogger.log("=== MouseArea containsMouse changed: " + containsMouse)
        }

        onPressedChanged: {
            QmlLogger.log("=== MouseArea pressed changed: " + pressed)
        }
    }


    // ARM64 FIX (Oct 13, 2025 19:05 UTC): Component.onCompleted workaround using width signal
    // Component.onCompleted doesn't fire and Timers don't start in SailfishOS ApplicationWindow
    // Solution: Use onWidthChanged which DOES fire when Page gets its geometry
    property bool initRan: false

    onWidthChanged: {
        if (!initRan && width > 0) {
            QmlLogger.log("=== INITIALIZATION via onWidthChanged (Component.onCompleted workaround) ===")
            QmlLogger.log("Page width: " + width + " height: " + height)
            QmlLogger.log("Page visible: " + visible + " enabled: " + enabled)

            initRan = true

            // FIX (Oct 14, 2025 21:25 UTC): Load ROM here instead of onStatusChanged
            // onStatusChanged fires before Item wrapper is fully initialized, causing crash
            // onWidthChanged fires after all children are constructed
            QmlLogger.log("=== LOADING ROM FILE: " + romFile + " ===")
            if (romFile !== "" && typeof romFile !== 'undefined') {
                QmlLogger.log("Calling calcObj.load() with: " + romFile)
                calcObj.load(romFile)
                QmlLogger.log("calcObj.load() completed")
            } else {
                QmlLogger.log("ERROR: romFile is empty or undefined!")
            }

            // ARM64 FIX (Oct 13, 2025 19:20 UTC): Can't use Timers, they don't work
            // ROM needs ~540ms for POST to complete before pressing ON
            // Solution: Don't auto-press at all - let user press ON manually
            // The calculator is now properly initialized and ready for input
            QmlLogger.log("*** Initialization complete - calculator ready for manual ON press ***")
        }
    }

    // NOV 11 2025: Removed timerInjectionTimer - DEAD CODE
    // - Added Oct 14, 2025 for manual Timer1 injection
    // - Disabled Oct 14, 2025 (running: false)
    // - Never executed after Oct 14
    // - Hardware timers now work correctly with 40-second period

    // ARM64 FIX (Oct 14, 2025 19:15 UTC): DISABLE auto-press ON button
    // ROOT CAUSE: Auto-press at 1.5s + Timer1 injection at 2s = interrupt storm
    // The ON key interrupt (0x01) is pending when Timer1 (0x02) arrives, creating
    // interrupts=0x03 which triggers infinite interrupt loop on slow ARM64.
    //
    // SOLUTION: Let user manually press ON button when ready. With 2-second Timer1
    // interval, ROM has enough time to process ON key interrupt before next Timer1.
    //
    // DISABLED - User must manually press ON to boot
    Timer {
        id: autoPressTimer
        interval: 1500  // Wait for ROM initialization
        running: false  // DISABLED - prevents interrupt storm
        repeat: false
        onTriggered: {
            QmlLogger.log("=== AUTO-PRESSING ON BUTTON (keycode 41 / 0x29) ===")
            calcObj.pressKey(41)
            QmlLogger.log("*** Starting releaseTimer with 100ms interval")
            releaseTimer.start()
            QmlLogger.log("*** releaseTimer.running=" + releaseTimer.running)
        }
    }

    Timer {
        id: releaseTimer
        interval: 100
        running: false
        repeat: false
        onTriggered: {
            QmlLogger.log("=== AUTO-RELEASING ON BUTTON (keycode 41) ===")
            calcObj.releaseKey(41)
        }
    }

    // Remorse popup for destructive actions
    RemorsePopup {
        id: remorse
    }

    } // End of Item wrapper

}
