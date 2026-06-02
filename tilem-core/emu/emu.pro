TEMPLATE = lib
TARGET = tilememu
CONFIG += staticlib

# Find required libraries
CONFIG += link_pkgconfig
PKGCONFIG += glib-2.0

INCLUDEPATH += .. $$OUT_PWD/..

# ============================================================================
# ARM64 Performance Optimizations - Updated 2025-10-09 21:00 UTC
# Phase 1: Compiler flags for Cortex-A73 (Expected: 15-30% speedup)
# Research: Comprehensive optimization based on ARM64 emulation best practices
# Fix: Enable tree vectorization for ARM64 NEON SIMD instructions
# ============================================================================

# Core optimization flags
QMAKE_CFLAGS_RELEASE += -O3 -march=armv8-a -mtune=cortex-a73
QMAKE_CFLAGS_RELEASE += -fomit-frame-pointer
QMAKE_CFLAGS_RELEASE += -funroll-loops -finline-functions -fstrict-aliasing

# ARM64 NEON vectorization (Oct 9, 21:00 UTC)
# Changed from -fno-tree-vectorize to -ftree-vectorize to enable SIMD
# This allows GCC to use ARM64 NEON instructions for parallel operations
QMAKE_CFLAGS_RELEASE += -ftree-vectorize

# Fast math optimizations (timer calculations)
QMAKE_CFLAGS_RELEASE += -ffast-math -funsafe-math-optimizations

# Branch optimization
QMAKE_CFLAGS_RELEASE += -freorder-blocks-algorithm=stc
QMAKE_CFLAGS_RELEASE += -freorder-blocks-and-partition

# Aggressive inlining
QMAKE_CFLAGS_RELEASE += -finline-limit=600
QMAKE_CFLAGS_RELEASE += --param inline-unit-growth=50
QMAKE_CFLAGS_RELEASE += --param large-function-growth=200

# LTO (Link Time Optimization) - Oct 9, 21:00 UTC
# Note: Full -flto caused linking issues with static library in mb2 build
# Note 2: -flto=thin not supported by GCC in SailfishOS SDK (tried Oct 9, 21:00)
# Leaving LTO disabled for now
# QMAKE_CFLAGS_RELEASE += -flto=thin
# QMAKE_LFLAGS_RELEASE += -flto=thin

# Ensure release flags are used even in debug builds for performance testing
QMAKE_CFLAGS += $$QMAKE_CFLAGS_RELEASE
# ============================================================================

# Core emulator sources
SOURCES += \
    keypad.c \
    link.c \
    audio.c \
    lcd.c \
    grayimage.c \
    rom.c \
    md5.c \
    flash.c \
    cert.c \
    state.c \
    graylcd.c \
    z80.c \
    calcs.c \
    timers.c \
    monolcd.c \
    graycolor.c

# Model-specific sources
SOURCES += \
    x1/x1_init.c x1/x1_io.c x1/x1_memory.c x1/x1_subcore.c \
    x2/x2_init.c x2/x2_io.c x2/x2_memory.c x2/x2_subcore.c \
    x3/x3_init.c x3/x3_io.c x3/x3_memory.c x3/x3_subcore.c \
    x4/x4_init.c x4/x4_io.c x4/x4_memory.c x4/x4_subcore.c \
    x5/x5_init.c x5/x5_io.c x5/x5_memory.c x5/x5_subcore.c \
    x6/x6_init.c x6/x6_io.c x6/x6_memory.c x6/x6_subcore.c \
    x7/x7_init.c x7/x7_io.c x7/x7_memory.c x7/x7_subcore.c \
    xc/xc_init.c xc/xc_io.c xc/xc_memory.c xc/xc_subcore.c xc/xc_lcd.c \
    xn/xn_init.c xn/xn_io.c xn/xn_memory.c xn/xn_subcore.c \
    xp/xp_init.c xp/xp_io.c xp/xp_memory.c xp/xp_subcore.c \
    xs/xs_init.c xs/xs_io.c xs/xs_memory.c xs/xs_subcore.c \
    xz/xz_init.c xz/xz_io.c xz/xz_memory.c xz/xz_subcore.c

HEADERS += \
    tilem.h \
    tilemint.h \
    scancodes.h \
    graylcd.h \
    z80.h \
    z80main.h \
    z80ed.h \
    z80cb.h \
    z80ddfd.h \
    z80cmds.h \
    gettext.h \
    x1/x1.h \
    x2/x2.h \
    x3/x3.h \
    x4/x4.h \
    x5/x5.h \
    x6/x6.h \
    x7/x7.h \
    xc/xc.h \
    xn/xn.h \
    xp/xp.h \
    xs/xs.h \
    xz/xz.h

# config.h is manually created in tilem-core/config.h
