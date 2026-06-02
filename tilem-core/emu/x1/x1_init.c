/*
 * libtilemcore - Graphing calculator emulation library
 *
 * Copyright (C) 2001 Solignac Julien
 * Copyright (C) 2004-2009 Benjamin Moody
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <tilem.h>

#include "x1.h"

// Forward declaration of Qt debug function
extern void tilem_qt_debug(const char* msg);

void x1_reset(TilemCalc* calc)
{
	char debugbuf[256];

	tilem_qt_debug(">>> x1_reset called");

	calc->hwregs[PORT3] = 0x08;
	calc->hwregs[PORT2] = 0x00;
	calc->hwregs[PORT5] = 0x00;
	calc->hwregs[PORT6] = 0x00;

	calc->mempagemap[0] = 0x00;
	calc->mempagemap[1] = 0x01;
	calc->mempagemap[2] = 0x00;
	calc->mempagemap[3] = 0x02;

	if (calc->hwregs[HW_VERSION] != 2)
		calc->lcd.rowstride = 12;

	tilem_z80_set_speed(calc, 2000);

	/* FIXME: measure actual frequency */
	tilem_z80_set_timer(calc, TIMER_INT, 6000, 6000, 1);

	// Apply PORT3 value to lcd.active and poweronhalt
	calc->lcd.active = calc->poweronhalt = ((calc->hwregs[PORT3] & 8) >> 3);

	snprintf(debugbuf, sizeof(debugbuf),
	         "<<< x1_reset complete - PORT3=0x%02X lcd.active=%d poweronhalt=%d",
	         calc->hwregs[PORT3], calc->lcd.active, calc->poweronhalt);
	tilem_qt_debug(debugbuf);
}
