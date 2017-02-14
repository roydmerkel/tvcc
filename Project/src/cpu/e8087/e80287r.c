/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/e8086/e80286r.c                                      *
 * Created:     2003-10-07 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2013 Hampa Hug <hampa@hampa.ch>                     *
 *****************************************************************************/

/*****************************************************************************
 * This program is free software. You can redistribute it and / or modify it *
 * under the terms of the GNU General Public License version 2 as  published *
 * by the Free Software Foundation.                                          *
 *                                                                           *
 * This program is distributed in the hope  that  it  will  be  useful,  but *
 * WITHOUT  ANY   WARRANTY,   without   even   the   implied   warranty   of *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU  General *
 * Public License for more details.                                          *
 *****************************************************************************/


#include "e8087.h"
#include "intrnl87.h"

#include <stdlib.h>
#include <stdio.h>


#define E287_MSW_PE 0x0001
#define E287_MSW_MP 0x0002
#define E287_MSW_EM 0x0004
#define E287_MSW_TS 0x0008

/* WARNING: This file is used as a placeholder for the upcoming Intel 8087 FPU. Please do not use this file in any production emulator. */
/* OP 0F 01 00: SGDT mem */
static
unsigned op_0f_01_00 (e8087_t *c)
{
	e87_get_ea_ptr (c, c->pq + 2);

	if (!c->ea.is_mem) {
		return (e87_undefined (c));
	}

	e87_set_ea16 (c, 0x0000);
	e87_set_clk_ea (c, 11, 11);

	return (c->ea.cnt + 2);
}

/* OP 0F 01 01: SIDT mem */
static
unsigned op_0f_01_01 (e8087_t *c)
{
	e87_get_ea_ptr (c, c->pq + 2);

	if (!c->ea.is_mem) {
		return (e87_undefined (c));
	}

	e87_set_ea16 (c, 0x0000);
	e87_set_clk_ea (c, 11, 11);

	return (c->ea.cnt + 2);
}

/* OP 0F 01 04: SMSW r/m16 */
static
unsigned op_0f_01_04 (e8087_t *c)
{
	e87_get_ea_ptr (c, c->pq + 2);
	e87_set_ea16 (c, 0x0000);

	e87_set_clk_ea (c, 2, 3);

	return (c->ea.cnt + 2);
}

/* OP 0F 01 06: LMSW r/m16 */
static
unsigned op_0f_01_06 (e8087_t *c)
{
	unsigned short val;

	e87_get_ea_ptr (c, c->pq + 2);
	val = e87_get_ea16 (c);

	if (val & E287_MSW_EM) {
		c->cpu |= E87_CPU_INT7;
	}
	else {
		c->cpu &= ~E87_CPU_INT7;
	}

	e87_set_clk_ea (c, 3, 6);

	return (c->ea.cnt + 2);
}

/* OP 0F 01 */
static
unsigned op_0f_01 (e8087_t *c)
{
	switch ((c->pq[2] >> 3) & 0x07) {
		case 0x00:
			return (op_0f_01_00 (c));

		case 0x01:
			return (op_0f_01_01 (c));

		case 0x04:
			return (op_0f_01_04 (c));

		case 0x06:
			return (op_0f_01_06 (c));
	}

	return (e87_undefined (c));
}

/* OP 0F */
static
unsigned op_0f (e8087_t *c)
{
	if (c->pq[1] == 0x01) {
		return (op_0f_01 (c));
	}

	return (e87_undefined (c));
}

void e87_set_80287 (e8087_t *c)
{
	e87_set_80187 (c);

	c->cpu |= (E87_CPU_INT6 | E87_CPU_FLAGS286 | E87_CPU_PUSH_FIRST);

	c->op[0x0f] = &op_0f;
}
