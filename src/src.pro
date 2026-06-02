TARGET = harbour-tilem

# Qt modules for SailfishOS
QT += quick qml gui
CONFIG += c++11 link_pkgconfig
PKGCONFIG += sailfishapp

# ============================================================================
# ARM64 Performance Optimizations - Added 2025-10-08
# Phase 1: Compiler flags for Cortex-A53/A73 (Expected: 10-30% speedup)
# ============================================================================
QMAKE_CXXFLAGS_RELEASE += -O3 -march=armv8-a -mtune=cortex-a53
QMAKE_CXXFLAGS_RELEASE += -fomit-frame-pointer
QMAKE_CXXFLAGS_RELEASE += -funroll-loops -finline-functions -fstrict-aliasing
QMAKE_CXXFLAGS_RELEASE += -fno-tree-vectorize
# Note: -flto removed - causes linking issues with static library in mb2 build

# Ensure release flags are used even in debug builds for performance testing
QMAKE_CXXFLAGS += $$QMAKE_CXXFLAGS_RELEASE
# ============================================================================

SOURCES += \
    src/main.cpp \
    ../backend/modules/TilEm/calc.cpp \
    ../backend/modules/TilEm/calcthread.cpp \
    ../backend/modules/TilEm/calclink.cpp \
    ../backend/modules/TilEm/calcscreen.cpp \
    ../backend/modules/TilEm/skin.cpp \
    ../backend/modules/TilEm/skinimage.cpp \
    ../backend/modules/TilEm/settings.cpp \
    ../backend/modules/TilEm/skinops.c

HEADERS += \
    ../backend/modules/TilEm/calc.h \
    ../backend/modules/TilEm/calcthread.h \
    ../backend/modules/TilEm/calclink.h \
    ../backend/modules/TilEm/calcscreen.h \
    ../backend/modules/TilEm/skin.h \
    ../backend/modules/TilEm/skinimage.h \
    ../backend/modules/TilEm/settings.h \
    ../backend/modules/TilEm/skinops.h \
    ../backend/modules/TilEm/linkbuffer.h \
    ../backend/modules/TilEm/scancodes.h \
    ../backend/modules/TilEm/qmllogger.h

INCLUDEPATH += \
    ../backend/modules/TilEm \
    ../tilem-core \
    ../tilem-core/emu \
    $$OUT_PWD/../tilem-core

# Link against the tilem emulator core library
LIBS += -L$$OUT_PWD/../tilem-core/emu -ltilememu

# Find required libraries
CONFIG += link_pkgconfig
PKGCONFIG += \
    glib-2.0 \
    gobject-2.0 \
    gdk-pixbuf-2.0

QML_FILES = \
    qml/harbour-tilem.qml \
    qml/pages/CalcPage.qml \
    qml/pages/FolderPage.qml \
    qml/pages/AboutPage.qml \
    qml/cover/CoverPage.qml

OTHER_FILES += \
    $$QML_FILES \
    harbour-tilem.desktop

SAILFISHAPP_ICONS = 86x86 108x108 128x128 172x172

# Install binary
target.path = /usr/bin
INSTALLS += target

# Install skins
skins.path = /usr/share/$$TARGET/skins
skins.files = ../data/skins/*.skn
INSTALLS += skins

# Install keybindings
keybindings.path = /usr/share/$$TARGET/data
keybindings.files = ../data/keybindings.ini
INSTALLS += keybindings

# Install QML files
qml.path = /usr/share/$$TARGET/qml
qml.files = qml/*
INSTALLS += qml

# Install desktop file
desktop.path = /usr/share/applications
desktop.files = harbour-tilem.desktop
INSTALLS += desktop

DEFINES += APP_VERSION=\\\"$$VERSION\\\"
DEFINES += APP_RELEASE=\\\"$$RELEASE\\\"
