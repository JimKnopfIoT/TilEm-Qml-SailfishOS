# TilEm-Qml — SailfishOS port (working)

TI-83 calculator emulator running on **SailfishOS** (tested on Sony Xperia 10 III,
aarch64, SailfishOS 5.1.0.7 and 5.1.0.8; built against the
`SailfishOS-5.0.0.62-aarch64` target).

This is a port of [labsin/TilEm-Qml](https://github.com/labsin/TilEm-Qml)
(itself based on the [TilEm](https://github.com/labsin/TilEm) Z80 core) to the
SailfishOS / harbour Qt environment. **The emulator boots and runs** (cursor,
"Mem cleared", calculations work).

## The decisive fix (the long-hunted "ARM64 bug")

`tilem-core/config.h` defined the endianness macro as `#define WORDS_BIGENDIAN 0`,
but `tilem-core/emu/tilem.h` selects the Z80 register-union layout with
`#ifdef WORDS_BIGENDIAN` — and `#ifdef` is **true whenever the macro is defined,
even to `0`**. On the little-endian ARM64 device the *big-endian* register union
was compiled, swapping the byte accessors so that `IN`/`OUT (n),A` built the wrong
port → `OUT (03),x` never reached PORT3 → the ROM could never set the interrupt
mask / turn the LCD on → reboot loop.

**Fix:** `config.h` → `#undef WORDS_BIGENDIAN` (so the little-endian branch
compiles). Supporting change kept: `tilem-core/emu/z80.h` uses `.w.l` (16-bit)
register macros.

## Build & deploy

```bash
./build-and-deploy.sh        # mb2 build → RPM → scp → install on device
```

The build uses `harbour-tilem.pro` (`SUBDIRS = tilem-core src`); the app sources
are in `src/` and `backend/modules/TilEm/`.

## ROM

**A real TI-83 ROM is required to run the emulator** — none is included (copyright).
You must supply your own and place it in your home directory's `Documents` folder,
named exactly `TI83.ROM`:

```
~/Documents/TI83.ROM
```

(On the device this is typically `/home/defaultuser/Documents/TI83.ROM`. The path
is derived at startup from the home directory, so `~/Documents/TI83.ROM` is what
matters.)

## Skins

All calculator skins are bundled under `data/skins/*.skn` and install to
`/usr/share/harbour-tilem/skins/`.
