/*
 * libtilemcore - Graphing calculator emulation library
 *
 * Copyright (C) 2009 Benjamin Moody
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef _TILEM_Z80_H
#define _TILEM_Z80_H

/* Internal Z80 data structures */

struct _TilemZ80Timer {
	int next, prev;
	dword count;
	dword period;
	TilemZ80TimerFunc callback;
	void* callbackdata;
};

struct _TilemZ80Breakpoint {
	int next, prev;
	int type;
	dword start;
	dword end;
	dword mask;
	TilemZ80BreakpointFunc testfunc;
	void* testdata;
};

/* Useful definitions */

/* ARM64 FIX: All Z80 registers are 16-bit or 8-bit. On ARM64, accessing via .d (32-bit dword)
 * breaks register operations because .d doesn't properly map to .w.l (16-bit word) or .b.h/.b.l (8-bit bytes).
 * MUST use .w.l for all 16-bit register pairs to ensure correct behavior on 64-bit architectures.
 */
#define AF (calc->z80.r.af.w.l)   /* ARM64 fix: use .w.l (16-bit) */
#define BC (calc->z80.r.bc.w.l)   /* ARM64 fix: use .w.l (16-bit) */
#define DE (calc->z80.r.de.w.l)   /* ARM64 fix: use .w.l (16-bit) */
#define HL (calc->z80.r.hl.w.l)   /* ARM64 fix: use .w.l (16-bit) */
#define AF2 (calc->z80.r.af2.w.l) /* ARM64 fix: use .w.l (16-bit) */
#define BC2 (calc->z80.r.bc2.w.l) /* ARM64 fix: use .w.l (16-bit) */
#define DE2 (calc->z80.r.de2.w.l) /* ARM64 fix: use .w.l (16-bit) */
#define HL2 (calc->z80.r.hl2.w.l) /* ARM64 fix: use .w.l (16-bit) */
#define IX (calc->z80.r.ix.w.l)   /* ARM64 fix: use .w.l (16-bit) */
#define IY (calc->z80.r.iy.w.l)   /* ARM64 fix: use .w.l (16-bit) */
#define SP (calc->z80.r.sp.w.l)   /* ARM64 fix: use .w.l (16-bit) */
#define PC (calc->z80.r.pc.w.l)   /* ARM64 fix: use .w.l (16-bit) */
#define IR (calc->z80.r.ir.w.l)   /* ARM64 fix: use .w.l (16-bit) */

#define A (calc->z80.r.af.b.h)
#define F (calc->z80.r.af.b.l)
#define B (calc->z80.r.bc.b.h)
#define C (calc->z80.r.bc.b.l)
#define D (calc->z80.r.de.b.h)
#define E (calc->z80.r.de.b.l)
#define H (calc->z80.r.hl.b.h)
#define L (calc->z80.r.hl.b.l)
#define IXh (calc->z80.r.ix.b.h)
#define IXl (calc->z80.r.ix.b.l)
#define IYh (calc->z80.r.iy.b.h)
#define IYl (calc->z80.r.iy.b.l)
#define I (calc->z80.r.ir.b.h)
#define Rh (calc->z80.r.r7)
#define Rl (calc->z80.r.ir.b.l)
#define R ((Rl & 0x7f) | Rh)

#ifdef DISABLE_Z80_WZ_REGISTER
# define WZ temp_wz.d
# define WZ2 temp_wz2.d
# define W temp_wz.b.h
# define Z temp_wz.b.l
#else
# define WZ (calc->z80.r.wz.d)
# define WZ2 (calc->z80.r.wz2.d)
# define W (calc->z80.r.wz.b.h)
# define Z (calc->z80.r.wz.b.l)
#endif

#define IFF1 (calc->z80.r.iff1)
#define IFF2 (calc->z80.r.iff2)
#define IM (calc->z80.r.im)

#define FLAG_S 0x80
#define FLAG_Z 0x40
#define FLAG_Y 0x20
#define FLAG_H 0x10
#define FLAG_X 0x08
#define FLAG_P 0x04
#define FLAG_V FLAG_P
#define FLAG_N 0x02
#define FLAG_C 0x01

#define BCw (calc->z80.r.bc.w.l)
#define DEw (calc->z80.r.de.w.l)
#define HLw (calc->z80.r.hl.w.l)
#define SPw (calc->z80.r.sp.w.l)
#define IXw (calc->z80.r.ix.w.l)
#define IYw (calc->z80.r.iy.w.l)

#endif
