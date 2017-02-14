/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/e8086/opcodes.c                                      *
 * Created:     1996-04-28 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 1996-2014 Hampa Hug <hampa@hampa.ch>                     *
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


/* WARNING: This file is used as a placeholder for the upcoming Intel 8087 FPU. Please do not use this file in any production emulator. */
#include "e8087.h"
#include "intrnl87.h"
/* ---------------------------------------------------------------------------------------------------------- */
/* IBM PC (Model 5150/5160): Quote from Technical Reference; Section 2. Coprocessor: Description:             */
/* ---------------------------------------------------------------------------------------------------------- */
/* The Math Coprocessor (8087) enables the IBM Personal Computer to perform high-speed arithmetic, logarithmic*/
/* functions, and trigonometric operations with extreme accuracy.                                             */
/*                                                                                                            */
/* The 8087 coprocessor works in parallel with the microprocessor. The parallel operation decreases operating */
/* time by allowing the coprocessor to do mathematical calculations while the microprocessor continues to do  */
/* other functions.                                                                                           */
/*                                                                                                            */
/* The first five bits of every instruction's operation code for the coprocessor are identical (binary 11011).*/
/* When the microprocessor and the coprocessor see this operation code, the microprocessor calculates the     */
/* address of any variables in memory, while the coprocessor checks the instruction. The coprocessor takes the*/
/* memory address from the microprocessor if necessary. To gain access to locations in memory, the coprocessor*/
/* takes the local bus from the microprocessor when the microprocessor finishes its current instructions. When*/
/* the coprocessor is finished with the memory transfer, it returns the local bus to the microprocessor.      */
/*                                                                                                            */
/* The IBM Math Coprocessor works with seven numeric data types divided into the three classes listed below.  */
/*  - Binary  integers (3 types)                                                                              */
/*  - Decimal integers (1 type)                                                                               */
/*  - Real    numbers  (3 types)                                                                              */
/* ---------------------------------------------------------------------------------------------------------- */
/**************************************************************************
 * Microcode functions
 **************************************************************************/

/* Handle an undefined opcode */
/* static
unsigned op_11011_ud (e8087_t *c)
{
	e87_set_clk (c, 1);

	return (e87_undefined (c));
}

static
unsigned op_11011_divide_error (e8087_t *c)
{
	e87_set_clk_ea (c, 16, 20);

	e87_set_ip (c, e87_get_ip (c) + c->ea.cnt + 1);
	e87_trap (c, 0);

	return (0);
} */

/* OP 00: ADD r/m8, reg8 */
static
unsigned op_11011_00 (e8087_t *c)
{
	unsigned short s1, s2, d;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_ea8 (c);
	s2 = e87_get_reg8 (c, (c->pq[1] >> 3) & 7);

	d = s1 + s2;

	e87_set_ea8 (c, d);
	e87_set_flg_add_8 (c, s1, s2);
	e87_set_clk_ea (c, 3, 16);

	return (c->ea.cnt + 1);
}

/* OP 01: ADD r/m16, reg16 */
static
unsigned op_11011_01 (e8087_t *c)
{
	unsigned long s1, s2, d;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_ea16 (c);
	s2 = e87_get_reg16 (c, (c->pq[1] >> 3) & 7);

	d = s1 + s2;

	e87_set_ea16 (c, d);
	e87_set_flg_add_16 (c, s1, s2);
	e87_set_clk_ea (clk, 3, 16);

	return (c->ea.cnt + 1);
}

/* OP 02: ADD reg8, r/m8 */
static
unsigned op_11011_02 (e8087_t *c)
{
	unsigned       reg;
	unsigned short s1, s2, d;

	reg = (c->pq[1] >> 3) & 7;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_reg8 (c, reg);
	s2 = e87_get_ea8 (c);

	d = s1 + s2;

	e87_set_reg8 (c, reg, d);
	e87_set_flg_add_8 (c, s1, s2);
	e87_set_clk_ea (c, 3, 9);

	return (c->ea.cnt + 1);
}

/* OP 03: ADD reg16, r/m16 */
static
unsigned op_11011_03 (e8087_t *c)
{
	unsigned      reg;
	unsigned long s1, s2, d;

	reg = (c->pq[1] >> 3) & 7;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_reg16 (c, reg);
	s2 = e87_get_ea16 (c);

	d = s1 + s2;

	e87_set_reg16 (c, reg, d);
	e87_set_flg_add_16 (c, s1, s2);
	e87_set_clk_ea (c, 3, 9);

	return (c->ea.cnt + 1);
}

/* OP 04: ADD AL, data8 */
static
unsigned op_11011_04 (e8087_t *c)
{
	unsigned short s1, s2, d;

	s1 = e87_get_al (c);
	s2 = c->pq[1];

	d = s1 + s2;

	e87_set_al (c, d);
	e87_set_flg_add_8 (c, s1, s2);
	e87_set_clk (c, 4);

	return (2);
}

/* OP 05: ADD AX, data16 */
static
unsigned op_11011_05 (e8087_t *c)
{
	unsigned long s1, s2, d;

	s1 = e87_get_ax (c);
	s2 = e87_mk_uint16 (c->pq[1], c->pq[2]);

	d = s1 + s2;

	e87_set_ax (c, d);
	e87_set_flg_add_16 (c, s1, s2);
	e87_set_clk (c, 4);

	return (3);
}

/* OP 06: PUSH ES */
static
unsigned op_11011_06 (e8087_t *c)
{
	e87_push (c, e87_get_es (c));
	e87_set_clk (c, 10);

	return (1);
}

/* OP 07: POP ES */
static
unsigned op_11011_07 (e8087_t *c)
{
	e87_set_es (c, e87_pop (c));
	e87_set_clk (c, 8);

	return (1);
}

/* OP 08: OR r/m8, reg8 */
static
unsigned op_11011_08 (e8087_t *c)
{
	unsigned short s1, s2, d;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_ea8 (c);
	s2 = e87_get_reg8 (c, (c->pq[1] >> 3) & 7);

	d = s1 | s2;

	e87_set_ea8 (c, d);
	e87_set_flg_log_8 (c, d);
	e87_set_clk_ea (c, 3, 16);

	return (c->ea.cnt + 1);
}

/* OP 09: OR r/m16, reg16 */
static
unsigned op_11011_09 (e8087_t *c)
{
	unsigned long s1, s2, d;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_ea16 (c);
	s2 = e87_get_reg16 (c, (c->pq[1] >> 3) & 7);

	d = s1 | s2;

	e87_set_ea16 (c, d);
	e87_set_flg_log_16 (c, d);
	e87_set_clk_ea (c, 3, 16);

	return (c->ea.cnt + 1);
}

/* OP 0A: OR reg8, r/m8 */
static
unsigned op_11011_0a (e8087_t *c)
{
	unsigned       reg;
	unsigned short s1, s2, d;

	reg = (c->pq[1] >> 3) & 7;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_reg8 (c, reg);
	s2 = e87_get_ea8 (c);

	d = s1 | s2;

	e87_set_reg8 (c, reg, d);
	e87_set_flg_log_8 (c, d);
	e87_set_clk_ea (c, 3, 9);

	return (c->ea.cnt + 1);
}

/* OP 0B: OR reg16, r/m16 */
static
unsigned op_11011_0b (e8087_t *c)
{
	unsigned      reg;
	unsigned long s1, s2, d;

	reg = (c->pq[1] >> 3) & 7;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_reg16 (c, reg);
	s2 = e87_get_ea16 (c);

	d = s1 | s2;

	e87_set_reg16 (c, reg, d);
	e87_set_flg_log_16 (c, d);
	e87_set_clk_ea (c, 3, 9);

	return (c->ea.cnt + 1);
}

/* OP 0C: OR AL, data8 */
static
unsigned op_11011_0c (e8087_t *c)
{
	unsigned short s1, s2, d;

	s1 = e87_get_al (c);
	s2 = c->pq[1];

	d = s1 | s2;

	e87_set_al (c, d);
	e87_set_flg_log_8 (c, d);
	e87_set_clk (c, 4);

	return (2);
}

/* OP 0D: OR AX, data16 */
static
unsigned op_11011_0d (e8087_t *c)
{
	unsigned long s1, s2, d;

	s1 = e87_get_ax (c);
	s2 = e87_mk_uint16 (c->pq[1], c->pq[2]);

	d = s1 | s2;

	e87_set_ax (c, d);
	e87_set_flg_log_16 (c, d);
	e87_set_clk (c, 4);

	return (3);
}

/* OP 0E: PUSH CS */
static
unsigned op_11011_0e (e8087_t *c)
{
	e87_push (c, e87_get_cs (c));
	e87_set_clk (c, 10);

	return (1);
}

/* OP 0F: POP CS */
static
unsigned op_11011_0f (e8087_t *c)
{
	e87_set_cs (c, e87_pop (c));
	e87_pq_init (c);

	return (1);
}

/* OP 10: ADC r/m8, reg8 */
static
unsigned op_11011_10 (e8087_t *c)
{
	unsigned short s1, s2, s3, d;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_ea8 (c);
	s2 = e87_get_reg8 (c, (c->pq[1] >> 3) & 7);
	s3 = e87_get_cf (c);

	d = s1 + s2 + s3;

	e87_set_ea8 (c, d);
	e87_set_flg_adc_8 (c, s1, s2, s3);
	e87_set_clk_ea (c, 3, 16);

	return (c->ea.cnt + 1);
}

/* OP 11: ADC r/m16, reg16 */
static
unsigned op_11011_11 (e8087_t *c)
{
	unsigned long s1, s2, s3, d;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_ea16 (c);
	s2 = e87_get_reg16 (c, (c->pq[1] >> 3) & 7);
	s3 = e87_get_cf (c);

	d = s1 + s2 + s3;

	e87_set_ea16 (c, d);
	e87_set_flg_adc_16 (c, s1, s2, s3);
	e87_set_clk_ea (c, 3, 16);

	return (c->ea.cnt + 1);
}

/* OP 12: ADC reg8, r/m8 */
static
unsigned op_11011_12 (e8087_t *c)
{
	unsigned       reg;
	unsigned short s1, s2, s3, d;

	reg = (c->pq[1] >> 3) & 7;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_reg8 (c, reg);
	s2 = e87_get_ea8 (c);
	s3 = e87_get_cf (c);

	d = s1 + s2 + s3;

	e87_set_reg8 (c, reg, d);
	e87_set_flg_adc_8 (c, s1, s2, s3);
	e87_set_clk_ea (c, 3, 9);

	return (c->ea.cnt + 1);
}

/* OP 13: ADC reg16, r/m16 */
static
unsigned op_11011_13 (e8087_t *c)
{
	unsigned      reg;
	unsigned long s1, s2, s3, d;

	reg = (c->pq[1] >> 3) & 7;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_reg16 (c, reg);
	s2 = e87_get_ea16 (c);
	s3 = e87_get_cf (c);

	d = s1 + s2 + s3;

	e87_set_reg16 (c, reg, d);
	e87_set_flg_adc_16 (c, s1, s2, s3);
	e87_set_clk_ea (c, 3, 9);

	return (c->ea.cnt + 1);
}

/* OP 14: ADC AL, data8 */
static
unsigned op_11011_14 (e8087_t *c)
{
	unsigned short s1, s2, s3, d;

	s1 = e87_get_al (c);
	s2 = c->pq[1];
	s3 = e87_get_cf (c);

	d = s1 + s2 + s3;

	e87_set_al (c, d);
	e87_set_flg_adc_8 (c, s1, s2, s3);
	e87_set_clk (c, 4);

	return (2);
}

/* OP 15: ADC AX, data16 */
static
unsigned op_11011_15 (e8087_t *c)
{
	unsigned long s1, s2, s3, d;

	s1 = e87_get_ax (c);
	s2 = e87_mk_uint16 (c->pq[1], c->pq[2]);
	s3 = e87_get_cf (c);

	d = s1 + s2 + s3;

	e87_set_ax (c, d);
	e87_set_flg_adc_16 (c, s1, s2, s3);
	e87_set_clk (c, 4);

	return (3);
}

/* OP 16: PUSH SS */
static
unsigned op_11011_16 (e8087_t *c)
{
	e87_push (c, e87_get_ss (c));
	e87_set_clk (c, 10);

	return (1);
}

/* OP 17: POP SS */
static
unsigned op_11011_17 (e8087_t *c)
{
	e87_set_ss (c, e87_pop (c));
	e87_set_clk (c, 8);

	c->enable_int = 0;

	return (1);
}

/* OP 18: SBB r/m8, reg8 */
static
unsigned op_11011_18 (e8087_t *c)
{
	unsigned short s1, s2, s3, d;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_ea8 (c);
	s2 = e87_get_reg8 (c, (c->pq[1] >> 3) & 7);
	s3 = e87_get_cf (c);

	d = s1 - s2 - s3;

	e87_set_ea8 (c, d);
	e87_set_flg_sbb_8 (c, s1, s2, s3);
	e87_set_clk_ea (c, 3, 16);

	return (c->ea.cnt + 1);
}

/* OP 19: SBB r/m16, reg16 "" */
static
unsigned op_11011_19 (e8087_t *c)
{
	unsigned long s1, s2, s3, d;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_ea16 (c);
	s2 = e87_get_reg16 (c, (c->pq[1] >> 3) & 7);
	s3 = e87_get_cf (c);

	d = s1 - s2 - s3;

	e87_set_ea16 (c, d);
	e87_set_flg_sbb_16 (c, s1, s2, s3);
	e87_set_clk_ea (c, 3, 16);

	return (c->ea.cnt + 1);
}

/* OP 1A: SBB reg8, r/m8 */
static
unsigned op_11011_1a (e8087_t *c)
{
	unsigned       reg;
	unsigned short s1, s2, s3, d;

	reg = (c->pq[1] >> 3) & 7;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_reg8 (c, reg);
	s2 = e87_get_ea8 (c);
	s3 = e87_get_cf (c);

	d = s1 - s2 - s3;

	e87_set_reg8 (c, reg, d);
	e87_set_flg_sbb_8 (c, s1, s2, s3);
	e87_set_clk_ea (c, 3, 9);

	return (c->ea.cnt + 1);
}

/* OP 1B: SBB reg16, r/m16 */
static
unsigned op_11011_1b (e8087_t *c)
{
	unsigned      reg;
	unsigned long s1, s2, s3, d;

	reg = (c->pq[1] >> 3) & 7;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_reg16 (c, reg);
	s2 = e87_get_ea16 (c);
	s3 = e87_get_cf (c);

	d = s1 - s2 - s3;

	e87_set_reg16 (c, reg, d);
	e87_set_flg_sbb_16 (c, s1, s2, s3);
	e87_set_clk_ea (c, 3, 9);

	return (c->ea.cnt + 1);
}

/* OP 1C: SBB AL, data8 */
static
unsigned op_11011_1c (e8087_t *c)
{
	unsigned short s1, s2, s3, d;

	s1 = e87_get_al (c);
	s2 = c->pq[1];
	s3 = e87_get_cf (c);

	d = s1 - s2 - s3;

	e87_set_al (c, d);
	e87_set_flg_sbb_8 (c, s1, s2, s3);
	e87_set_clk (c, 4);

	return (2);
}

/* OP 1D: SBB AX, data16 */
static
unsigned op_11011_1d (e8087_t *c)
{
	unsigned long s1, s2, s3, d;

	s1 = e87_get_ax (c);
	s2 = e87_mk_uint16 (c->pq[1], c->pq[2]);
	s3 = e87_get_cf (c);

	d = s1 - s2 - s3;

	e87_set_ax (c, d);
	e87_set_flg_sbb_16 (c, s1, s2, s3);
	e87_set_clk (c, 4);

	return (3);
}

/* OP 1E: PUSH DS */
static
unsigned op_11011_1e (e8087_t *c)
{
	e87_push (c, e87_get_ds (c));
	e87_set_clk (c, 10);

	return (1);
}

/* OP 1F: POP DS */
static
unsigned op_11011_1f (e8087_t *c)
{
	e87_set_ds (c, e87_pop (c));
	e87_set_clk (c, 8);

	return (1);
}

/* OP 20: AND r/m8, reg8 */
static
unsigned op_11011_20 (e8087_t *c)
{
	unsigned short s1, s2, d;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_ea8 (c);
	s2 = e87_get_reg8 (c, (c->pq[1] >> 3) & 7);

	d = s1 & s2;

	e87_set_ea8 (c, d);
	e87_set_flg_log_8 (c, d);
	e87_set_clk_ea (c, 3, 16);

	return (c->ea.cnt + 1);
}

/* OP 21: AND r/m16, reg16 */
static
unsigned op_11011_21 (e8087_t *c)
{
	unsigned long s1, s2, d;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_ea16 (c);
	s2 = e87_get_reg16 (c, (c->pq[1] >> 3) & 7);

	d = s1 & s2;

	e87_set_ea16 (c, d);
	e87_set_flg_log_16 (c, d);
	e87_set_clk_ea (c, 3, 16);

	return (c->ea.cnt + 1);
}

/* OP 22: AND reg8, r/m8 */
static unsigned op_11011_22 (e8087_t *c)
{
	unsigned       reg;
	unsigned short s1, s2, d;

	reg = (c->pq[1] >> 3) & 7;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_reg8 (c, reg);
	s2 = e87_get_ea8 (c);

	d = s1 & s2;

	e87_set_reg8 (c, reg, d);
	e87_set_flg_log_8 (c, d);
	e87_set_clk_ea (c, 3, 9);

	return (c->ea.cnt + 1);
}

/* OP 23: AND reg16, r/m16 */
static
unsigned op_11011_23 (e8087_t *c)
{
	unsigned      reg;
	unsigned long s1, s2, d;

	reg = (c->pq[1] >> 3) & 7;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_reg16 (c, reg);
	s2 = e87_get_ea16 (c);

	d = s1 & s2;

	e87_set_reg16 (c, reg, d);
	e87_set_flg_log_16 (c, d);
	e87_set_clk_ea (c, 3, 9);

	return (c->ea.cnt + 1);
}

/* OP 24: AND AL, data8 */
static
unsigned op_11011_24 (e8087_t *c)
{
	unsigned short s1, s2, d;

	s1 = e87_get_al (c);
	s2 = c->pq[1];

	d = s1 & s2;

	e87_set_al (c, d);
	e87_set_flg_log_8 (c, d);
	e87_set_clk (c, 4);

	return (2);
}

/* OP 25: AND AX, data16 */
static
unsigned op_11011_25 (e8087_t *c)
{
	unsigned long s1, s2, d;

	s1 = e87_get_ax (c);
	s2 = e87_mk_uint16 (c->pq[1], c->pq[2]);

	d = s1 & s2;

	e87_set_ax (c, d);
	e87_set_flg_log_16 (c, d);
	e87_set_clk (c, 4);

	return (3);
}

/* OP 26: ES: */
static
unsigned op_11011_26 (e8087_t *c)
{
	c->prefix |= (E87_PREFIX_NEW | E87_PREFIX_SEG);

	c->seg_override = c->sreg[E87_REG_ES];

	return (1);
}

/* OP 27: DAA */
static
unsigned op_11011_27 (e8087_t *c)
{
	unsigned al, cf;

	al = e87_get_al (c);
	cf = e87_get_cf (c);

	if (((al & 0x0f) > 9) || e87_get_af (c)) {
		al += 6;
		cf |= ((al & 0xff00) != 0);
		e87_set_af (c, 1);
	}
	else {
		e87_set_af (c, 0);
	}

	if (((al & 0xf0) > 0x90) || cf) {
		al += 0x60;
		e87_set_cf (c, 1);
	}
	else {
		e87_set_cf (c, 0);
	}

	e87_set_al (c, al);
	e87_set_flg_szp_8 (c, al);
	e87_set_clk (c, 4);

	return (1);
}

/* OP 28: SUB r/m8, reg8 */
static
unsigned op_11011_28 (e8087_t *c)
{
	unsigned short s1, s2, d;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_ea8 (c);
	s2 = e87_get_reg8 (c, (c->pq[1] >> 3) & 7);

	d = s1 - s2;

	e87_set_ea8 (c, d);
	e87_set_flg_sub_8 (c, s1, s2);
	e87_set_clk_ea (c, 3, 16);

	return (c->ea.cnt + 1);
}

/* OP 29: SUB r/m16, reg16 */
static
unsigned op_11011_29 (e8087_t *c)
{
	unsigned long s1, s2, d;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_ea16 (c);
	s2 = e87_get_reg16 (c, (c->pq[1] >> 3) & 7);

	d = s1 - s2;

	e87_set_ea16 (c, d);
	e87_set_flg_sub_16 (c, s1, s2);
	e87_set_clk_ea (c, 3, 16);

	return (c->ea.cnt + 1);
}

/* OP 2A: SUB reg8, r/m8 */
static
unsigned op_11011_2a (e8087_t *c)
{
	unsigned       reg;
	unsigned short s1, s2, d;

	reg = (c->pq[1] >> 3) & 7;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_reg8 (c, reg);
	s2 = e87_get_ea8 (c);

	d = s1 - s2;

	e87_set_reg8 (c, reg, d);
	e87_set_flg_sub_8 (c, s1, s2);
	e87_set_clk_ea (c, 3, 9);

	return (c->ea.cnt + 1);
}

/* OP 2B: SUB reg16, r/m16 */
static
unsigned op_11011_2b (e8087_t *c)
{
	unsigned      reg;
	unsigned long s1, s2, d;

	reg = (c->pq[1] >> 3) & 7;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_reg16 (c, reg);
	s2 = e87_get_ea16 (c);

	d = s1 - s2;

	e87_set_reg16 (c, reg, d);
	e87_set_flg_sub_16 (c, s1, s2);
	e87_set_clk_ea (c, 3, 9);

	return (c->ea.cnt + 1);
}

/* OP 2C: SUB AL, data8 */
static
unsigned op_11011_2c (e8087_t *c)
{
	unsigned short s1, s2, d;

	s1 = e87_get_al (c);
	s2 = c->pq[1];

	d = s1 - s2;

	e87_set_al (c, d);
	e87_set_flg_sub_8 (c, s1, s2);
	e87_set_clk (c, 4);

	return (2);
}

/* OP 2D: SUB AX, data16 */
static
unsigned op_11011_2d (e8087_t *c)
{
	unsigned long s1, s2, d;

	s1 = e87_get_ax (c);
	s2 = e87_mk_uint16 (c->pq[1], c->pq[2]);

	d = s1 - s2;

	e87_set_ax (c, d);
	e87_set_flg_sub_16 (c, s1, s2);
	e87_set_clk (c, 4);

	return (3);
}

/* OP 2E: CS: */
static
unsigned op_11011_2e (e8087_t *c)
{
	c->prefix |= (E87_PREFIX_NEW | E87_PREFIX_SEG);

	c->seg_override = c->sreg[E87_REG_CS];

	return (1);
}

/* OP 2F: DAS */
static
unsigned op_11011_2f (e8087_t *c)
{
	unsigned al, cf;

	al = e87_get_al (c);
	cf = e87_get_cf (c);

	if (((al & 0x0f) > 9) || e87_get_af (c)) {
		al -= 6;
		cf |= ((al & 0xff00) != 0);
		e87_set_af (c, 1);
	}
	else {
		e87_set_af (c, 0);
	}

	if (((al & 0xf0) > 0x90) || cf) {
		al -= 0x60;
		e87_set_cf (c, 1);
	}
	else {
		e87_set_cf (c, 0);
	}

	e87_set_al (c, al);
	e87_set_flg_szp_8 (c, al);
	e87_set_clk (c, 4);

	return (1);
}

/* OP 30: XOR r/m8, reg8 */
static
unsigned op_11011_30 (e8087_t *c)
{
	unsigned short s1, s2, d;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_ea8 (c);
	s2 = e87_get_reg8 (c, (c->pq[1] >> 3) & 7);

	d = s1 ^ s2;

	e87_set_ea8 (c, d);
	e87_set_flg_log_8 (c, d);
	e87_set_clk_ea (c, 3, 16);

	return (c->ea.cnt + 1);
}

/* OP 31: XOR r/m16, reg16 */
static
unsigned op_11011_31 (e8087_t *c)
{
	unsigned long s1, s2, d;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_ea16 (c);
	s2 = e87_get_reg16 (c, (c->pq[1] >> 3) & 7);

	d = s1 ^ s2;

	e87_set_ea16 (c, d);
	e87_set_flg_log_16 (c, d);
	e87_set_clk_ea (c, 3, 16);

	return (c->ea.cnt + 1);
}

/* OP 32: XOR reg8, r/m8 */
static
unsigned op_11011_32 (e8087_t *c)
{
	unsigned       reg;
	unsigned short s1, s2, d;

	reg = (c->pq[1] >> 3) & 7;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_reg8 (c, reg);
	s2 = e87_get_ea8 (c);

	d = s1 ^ s2;

	e87_set_reg8 (c, reg, d);
	e87_set_flg_log_8 (c, d);
	e87_set_clk_ea (c, 3, 9);

	return (c->ea.cnt + 1);
}

/* OP 33: XOR reg16, r/m16 */
static
unsigned op_11011_33 (e8087_t *c)
{
	unsigned      reg;
	unsigned long s1, s2, d;

	reg = (c->pq[1] >> 3) & 7;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_reg16 (c, reg);
	s2 = e87_get_ea16 (c);

	d = s1 ^ s2;

	e87_set_reg16 (c, reg, d);
	e87_set_flg_log_16 (c, d);
	e87_set_clk_ea (c, 3, 9);

	return (c->ea.cnt + 1);
}

/* OP 34: XOR AL, data8 */
static
unsigned op_11011_34 (e8087_t *c)
{
	unsigned short s1, s2, d;

	s1 = e87_get_al (c);
	s2 = c->pq[1];

	d = s1 ^ s2;

	e87_set_al (c, d);
	e87_set_flg_log_8 (c, d);
	e87_set_clk (c, 4);

	return (2);
}

/* OP 35: XOR AX, data16 */
static
unsigned op_11011_35 (e8087_t *c)
{
	unsigned long s1, s2, d;

	s1 = e87_get_ax (c);
	s2 = e87_mk_uint16 (c->pq[1], c->pq[2]);

	d = s1 ^ s2;

	e87_set_ax (c, d);
	e87_set_flg_log_16 (c, d);
	e87_set_clk (c, 4);

	return (3);
}

/* OP 36: SS: */
static
unsigned op_11011_36 (e8087_t *c)
{
	c->prefix |= (E87_PREFIX_NEW | E87_PREFIX_SEG);

	c->seg_override = c->sreg[E87_REG_SS];

	return (1);
}

/* OP 37: AAA */
static
unsigned op_11011_37 (e8087_t *c)
{
	unsigned ah, al;

	al = e87_get_al (c);
	ah = e87_get_ah (c);

	if (((al & 0x0f) > 9) || e87_get_af (c)) {
		al += 6;
		ah += 1;
		c->flg |= (E87_FLG_A | E87_FLG_C);
	}
	else {
		c->flg &= ~(E87_FLG_A | E87_FLG_C);
	}

	e87_set_ax (c, ((ah & 0xff) << 8) | (al & 0x0f));

	e87_set_clk (c, 8);

	return (1);
}

/* OP 38: CMP r/m8, reg8 */
static
unsigned op_11011_38 (e8087_t *c)
{
	unsigned short s1, s2;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_ea8 (c);
	s2 = e87_get_reg8 (c, (c->pq[1] >> 3) & 7);

	e87_set_flg_sub_8 (c, s1, s2);
	e87_set_clk_ea (c, 3, 9);

	return (c->ea.cnt + 1);
}

/* OP 39: CMP r/m16, reg16 */
static
unsigned op_11011_39 (e8087_t *c)
{
	unsigned long s1, s2;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_ea16 (c);
	s2 = e87_get_reg16 (c, (c->pq[1] >> 3) & 7);

	e87_set_flg_sub_16 (c, s1, s2);
	e87_set_clk_ea (c, 3, 9);

	return (c->ea.cnt + 1);
}

/* OP 3A: CMP reg8, r/m8 */
static unsigned op_11011_3a (e8087_t *c)
{
	unsigned short s1, s2;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_reg8 (c, (c->pq[1] >> 3) & 7);
	s2 = e87_get_ea8 (c);

	e87_set_flg_sub_8 (c, s1, s2);
	e87_set_clk_ea (c, 3, 9);

	return (c->ea.cnt + 1);
}

/* OP 3B: CMP reg16, r/m16 */
static
unsigned op_11011_3b (e8087_t *c)
{
	unsigned long s1, s2;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_reg16 (c, (c->pq[1] >> 3) & 7);
	s2 = e87_get_ea16 (c);

	e87_set_flg_sub_16 (c, s1, s2);
	e87_set_clk_ea (c, 3, 9);

	return (c->ea.cnt + 1);
}

/* OP 3C: CMP AL, data8 */
static
unsigned op_11011_3c (e8087_t *c)
{
	unsigned short s1, s2;

	s1 = e87_get_al (c);
	s2 = c->pq[1];

	e87_set_flg_sub_8 (c, s1, s2);
	e87_set_clk (c, 4);

	return (2);
}

/* OP 3D: CMP AX, data16 */
static
unsigned op_11011_3d (e8087_t *c)
{
	unsigned long s1, s2;

	s1 = e87_get_ax (c);
	s2 = e87_mk_uint16 (c->pq[1], c->pq[2]);

	e87_set_flg_sub_16 (c, s1, s2);
	e87_set_clk (c, 4);

	return (3);
}

/* OP 3E: DS: */
static
unsigned op_11011_3e (e8087_t *c)
{
	c->prefix |= (E87_PREFIX_NEW | E87_PREFIX_SEG);

	c->seg_override = c->sreg[E87_REG_DS];

	return (1);
}

/* OP 3F: AAS */
static
unsigned op_11011_3f (e8087_t *c)
{
	unsigned ah, al;

	al = e87_get_al (c);
	ah = e87_get_ah (c);

	if (((al & 0x0f) > 9) || e87_get_af (c)) {
		al -= 6;
		ah -= 1;
		c->flg |= (E87_FLG_A | E87_FLG_C);
	}
	else {
		c->flg &= ~(E87_FLG_A | E87_FLG_C);
	}

	e87_set_ax (c, ((ah & 0xff) << 8) | (al & 0x0f));

	e87_set_clk (c, 8);

	return (1);
}

/* OP 4x: INC reg16 */
static
unsigned op_11011_40 (e8087_t *c)
{
	unsigned       r;
	unsigned short cf;
	unsigned long  s;

	r = c->pq[0] & 7;
	s = c->dreg[r];
	c->dreg[r] = (s + 1) & 0xffff;

	cf = c->flg;
	e87_set_flg_add_16 (c, s, 1);
	c->flg ^= (c->flg ^ cf) & E87_FLG_C;

	e87_set_clk (c, 3);

	return (1);
}

/* OP 4x: DEC reg16 */
static
unsigned op_11011_48 (e8087_t *c)
{
	unsigned       r;
	unsigned short cf;
	unsigned long  s;

	r = c->pq[0] & 7;
	s = c->dreg[r];
	c->dreg[r] = (s - 1) & 0xffff;

	cf = c->flg;
	e87_set_flg_sub_16 (c, s, 1);
	c->flg ^= (c->flg ^ cf) & E87_FLG_C;

	e87_set_clk (c, 3);

	return (1);
}

/* OP 5x: PUSH reg16 */
static
unsigned op_11011_50 (e8087_t *c)
{
	unsigned reg;

	reg = c->pq[0] & 7;

	if ((reg == E87_REG_SP) && ((c->cpu & E87_CPU_PUSH_FIRST) == 0)) {
		e87_push (c, e87_get_sp (c) - 2);
	}
	else {
		e87_push (c, e87_get_reg16 (c, reg));
	}

	e87_set_clk (c, 10);

	return (1);
}

/* OP 5x: POP reg16 */
static
unsigned op_11011_58 (e8087_t *c)
{
	unsigned short val;

	val = e87_pop (c);
	e87_set_reg16 (c, c->pq[0] & 7, val);
	e87_set_clk (c, 8);

	return (1);
}

/* OP 66: hook */
static
unsigned op_11011_66 (e8087_t *c)
{
	unsigned short cs, ip;

	if (c->pq[1] != 0x66) {
		return (op_11011_ud (c));
	}

	cs = e87_get_cs (c);
	ip = e87_get_ip (c);

	if (c->op_11011_hook != NULL) {
		c->op_11011_hook (c->op_11011_ext, c->pq[2], c->pq[3]);
	}

	e87_set_clk (c, 16);

	if ((e87_get_cs (c) != cs) || (e87_get_ip (c) != ip)) {
		return (0);
	}

	return (4);
}

/* OP 70: JO imm8 */
static
unsigned op_11011_70 (e8087_t *c)
{
	if (e87_get_of (c)) {
		e87_set_ip (c, e87_add_sint8 (c->ip + 2, c->pq[1]));
		e87_pq_init (c);
		e87_set_clk (c, 16);
		return (0);
	}

	e87_set_clk (c, 4);

	return (2);
}

/* OP 71: JNO imm8 */
static
unsigned op_11011_71 (e8087_t *c)
{
	if (!e87_get_of (c)) {
		e87_set_ip (c, e87_add_sint8 (c->ip + 2, c->pq[1]));
		e87_pq_init (c);
		e87_set_clk (c, 16);
		return (0);
	}

	e87_set_clk (c, 4);

	return (2);
}

/* OP 72: JB imm8 */
static
unsigned op_11011_72 (e8087_t *c)
{
	if (e87_get_cf (c)) {
		e87_set_ip (c, e87_add_sint8 (c->ip + 2, c->pq[1]));
		e87_pq_init (c);
		e87_set_clk (c, 16);
		return (0);
	}

	e87_set_clk (c, 4);

	return (2);
}

/* OP 73: JAE imm8 */
static
unsigned op_11011_73 (e8087_t *c)
{
	if (!e87_get_cf (c)) {
		e87_set_ip (c, e87_add_sint8 (c->ip + 2, c->pq[1]));
		e87_pq_init (c);
		e87_set_clk (c, 16);
		return (0);
	}

	e87_set_clk (c, 4);

	return (2);
}

/* OP 74: JE imm8 */
static
unsigned op_11011_74 (e8087_t *c)
{
	if (e87_get_zf (c)) {
		e87_set_ip (c, e87_add_sint8 (c->ip + 2, c->pq[1]));
		e87_pq_init (c);
		e87_set_clk (c, 16);
		return (0);
	}

	e87_set_clk (c, 4);

	return (2);
}

/* OP 75: JNE imm8 */
static
unsigned op_11011_75 (e8087_t *c)
{
	if (!e87_get_zf (c)) {
		e87_set_ip (c, e87_add_sint8 (c->ip + 2, c->pq[1]));
		e87_pq_init (c);
		e87_set_clk (c, 16);
		return (0);
	}

	e87_set_clk (c, 4);

	return (2);
}

/* OP 76: JBE imm8 */
static
unsigned op_11011_76 (e8087_t *c)
{
	if (e87_get_zf (c) || e87_get_cf (c)) {
		e87_set_ip (c, e87_add_sint8 (c->ip + 2, c->pq[1]));
		e87_pq_init (c);
		e87_set_clk (c, 16);
		return (0);
	}

	e87_set_clk (c, 4);

	return (2);
}

/* OP 77: JA imm8 */
static
unsigned op_11011_77 (e8087_t *c)
{
	if (!(e87_get_zf (c) || e87_get_cf (c))) {
		e87_set_ip (c, e87_add_sint8 (c->ip + 2, c->pq[1]));
		e87_pq_init (c);
		e87_set_clk (c, 16);
		return (0);
	}

	e87_set_clk (c, 4);

	return (2);
}

/* OP 78: JS imm8 */
static
unsigned op_11011_78 (e8087_t *c)
{
	if (e87_get_sf (c)) {
		e87_set_ip (c, e87_add_sint8 (c->ip + 2, c->pq[1]));
		e87_pq_init (c);
		e87_set_clk (c, 16);
		return (0);
	}

	e87_set_clk (c, 4);

	return (2);
}

/* OP 79: JNS imm8 */
static
unsigned op_11011_79 (e8087_t *c)
{
	if (!e87_get_sf (c)) {
		e87_set_ip (c, e87_add_sint8 (c->ip + 2, c->pq[1]));
		e87_pq_init (c);
		e87_set_clk (c, 16);
		return (0);
	}

	e87_set_clk (c, 4);

	return (2);
}

/* OP 7A: JPE imm8 */
static
unsigned op_11011_7a (e8087_t *c)
{
	if (e87_get_pf (c)) {
		e87_set_ip (c, e87_add_sint8 (c->ip + 2, c->pq[1]));
		e87_pq_init (c);
		e87_set_clk (c, 16);
		return (0);
	}

	e87_set_clk (c, 4);

	return (2);
}

/* OP 7B: JPO imm8 */
static
unsigned op_11011_7b (e8087_t *c)
{
	if (!e87_get_pf (c)) {
		e87_set_ip (c, e87_add_sint8 (c->ip + 2, c->pq[1]));
		e87_pq_init (c);
		e87_set_clk (c, 16);
		return (0);
	}

	e87_set_clk (c, 4);

	return (2);
}

/* OP 7C: JL imm8 */
static
unsigned op_11011_7c (e8087_t *c)
{
	if (e87_get_sf (c) != e87_get_of (c)) {
		e87_set_ip (c, e87_add_sint8 (c->ip + 2, c->pq[1]));
		e87_pq_init (c);
		e87_set_clk (c, 16);
		return (0);
	}

	e87_set_clk (c, 4);

	return (2);
}

/* OP 7D: JGE imm8 */
static
unsigned op_11011_7d (e8087_t *c)
{
	if (e87_get_sf (c) == e87_get_of (c)) {
		e87_set_ip (c, e87_add_sint8 (c->ip + 2, c->pq[1]));
		e87_pq_init (c);
		e87_set_clk (c, 16);
		return (0);
	}

	e87_set_clk (c, 4);

	return (2);
}

/* OP 7E: JLE imm8 */
static
unsigned op_11011_7e (e8087_t *c)
{
	if ((e87_get_sf (c) != e87_get_of (c)) || e87_get_zf (c)) {
		e87_set_ip (c, e87_add_sint8 (c->ip + 2, c->pq[1]));
		e87_pq_init (c);
		e87_set_clk (c, 16);
		return (0);
	}

	e87_set_clk (c, 4);

	return (2);
}

/* OP 7F: JG imm8 */
static
unsigned op_11011_7f (e8087_t *c)
{
	if ((e87_get_sf (c) == e87_get_of (c)) && (!e87_get_zf (c))) {
		e87_set_ip (c, e87_add_sint8 (c->ip + 2, c->pq[1]));
		e87_pq_init (c);
		e87_set_clk (c, 16);
		return (0);
	}

	e87_set_clk (c, 4);

	return (2);
}

/* OP 80 00: ADD r/m8, imm8 */
static
unsigned op_11011_80_00 (e8087_t *c)
{
	unsigned short s1, s2, d;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_ea8 (c);
	s2 = c->pq[c->ea.cnt + 1];

	d = s1 + s2;

	e87_set_ea8 (c, d & 0xff);
	e87_set_flg_add_8 (c, s1, s2);
	e87_set_clk_ea (c, 4, 17);

	return (2 + c->ea.cnt);
}

/* OP 80 01: OR r/m8, imm8 */
static
unsigned op_11011_80_01 (e8087_t *c)
{
	unsigned short s1, s2, d;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_ea8 (c);
	s2 = c->pq[c->ea.cnt + 1];

	d = s1 | s2;

	e87_set_ea8 (c, d & 0xff);
	e87_set_flg_log_8 (c, d);
	e87_set_clk_ea (c, 4, 17);

	return (2 + c->ea.cnt);
}

/* OP 80 02: ADC r/m8, imm8 */
static
unsigned op_11011_80_02 (e8087_t *c)
{
	unsigned short s1, s2, s3, d;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_ea8 (c);
	s2 = c->pq[c->ea.cnt + 1];
	s3 = e87_get_cf (c);

	d = s1 + s2 + s3;

	e87_set_ea8 (c, d & 0xff);
	e87_set_flg_adc_8 (c, s1, s2, s3);
	e87_set_clk_ea (c, 4, 17);

	return (2 + c->ea.cnt);
}

/* OP 80 03: SBB r/m8, imm8 */
static
unsigned op_11011_80_03 (e8087_t *c)
{
	unsigned short s1, s2, s3, d;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_ea8 (c);
	s2 = c->pq[c->ea.cnt + 1];
	s3 = e87_get_cf (c);

	d = s1 - s2 - s3;

	e87_set_ea8 (c, d & 0xff);
	e87_set_flg_sbb_8 (c, s1, s2, s3);
	e87_set_clk_ea (c, 4, 17);

	return (2 + c->ea.cnt);
}

/* OP 80 04: AND r/m8, imm8 */
static
unsigned op_11011_80_04 (e8087_t *c)
{
	unsigned short s1, s2, d;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_ea8 (c);
	s2 = c->pq[c->ea.cnt + 1];

	d = s1 & s2;

	e87_set_ea8 (c, d);
	e87_set_flg_log_8 (c, d);
	e87_set_clk_ea (c, 4, 17);

	return (2 + c->ea.cnt);
}

/* OP 80 05: SUB r/m8, imm8 */
static
unsigned op_11011_80_05 (e8087_t *c)
{
	unsigned short s1, s2, d;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_ea8 (c);
	s2 = c->pq[c->ea.cnt + 1];

	d = s1 - s2;

	e87_set_ea8 (c, d & 0xff);
	e87_set_flg_sub_8 (c, s1, s2);
	e87_set_clk_ea (c, 4, 17);

	return (2 + c->ea.cnt);
}

/* OP 80 06: XOR r/m8, imm8 */
static
unsigned op_11011_80_06 (e8087_t *c)
{
	unsigned short s1, s2, d;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_ea8 (c);
	s2 = c->pq[c->ea.cnt + 1];

	d = s1 ^ s2;

	e87_set_ea8 (c, d);
	e87_set_flg_log_8 (c, d);
	e87_set_clk_ea (c, 4, 17);

	return (2 + c->ea.cnt);
}

/* OP 80 07: CMP r/m8, imm8 */
static
unsigned op_11011_80_07 (e8087_t *c)
{
	unsigned short s1, s2;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_ea8 (c);
	s2 = c->pq[c->ea.cnt + 1];

	e87_set_flg_sub_8 (c, s1, s2);
	e87_set_clk_ea (c, 4, 10);

	return (2 + c->ea.cnt);
}

static
e87_opcode_f e87_opcodes_80[8] = {
	&op_11011_80_00, &op_11011_80_01, &op_11011_80_02, &op_11011_80_03,
	&op_11011_80_04, &op_11011_80_05, &op_11011_80_06, &op_11011_80_07,
};

/* OP 80: xxx r/m8, imm8 */
static
unsigned op_11011_80 (e8087_t *c)
{
	unsigned xop;

	xop = (c->pq[1] >> 3) & 7;

	return (e87_opcodes_80[xop] (c));
}

/* OP 81 00: ADD r/m16, imm16 */
static
unsigned op_11011_81_00 (e8087_t *c)
{
	unsigned long s1, s2, d;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_ea16 (c);
	s2 = e87_mk_uint16 (c->pq[c->ea.cnt + 1], c->pq[c->ea.cnt + 2]);

	d = s1 + s2;

	e87_set_ea16 (c, d & 0xffff);
	e87_set_flg_add_16 (c, s1, s2);
	e87_set_clk_ea (c, 4, 17);

	return (3 + c->ea.cnt);
}

/* OP 81 01: OR r/m16, imm16 */
static
unsigned op_11011_81_01 (e8087_t *c)
{
	unsigned long s1, s2, d;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_ea16 (c);
	s2 = e87_mk_uint16 (c->pq[c->ea.cnt + 1], c->pq[c->ea.cnt + 2]);

	d = s1 | s2;

	e87_set_ea16 (c, d & 0xffff);
	e87_set_flg_log_16 (c, d);
	e87_set_clk_ea (c, 4, 17);

	return (3 + c->ea.cnt);
}

/* OP 81 02: ADC r/m16, imm16 */
static
unsigned op_11011_81_02 (e8087_t *c)
{
	unsigned long s1, s2, s3, d;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_ea16 (c);
	s2 = e87_mk_uint16 (c->pq[c->ea.cnt + 1], c->pq[c->ea.cnt + 2]);
	s3 = e87_get_cf (c);

	d = s1 + s2 + s3;

	e87_set_ea16 (c, d & 0xffff);
	e87_set_flg_adc_16 (c, s1, s2, s3);
	e87_set_clk_ea (c, 4, 17);

	return (3 + c->ea.cnt);
}

/* OP 81 03: SBB r/m16, imm16 */
static
unsigned op_11011_81_03 (e8087_t *c)
{
	unsigned long s1, s2, s3, d;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_ea16 (c);
	s2 = e87_mk_uint16 (c->pq[c->ea.cnt + 1], c->pq[c->ea.cnt + 2]);
	s3 = e87_get_cf (c);

	d = s1 - s2 - s3;

	e87_set_ea16 (c, d & 0xffff);
	e87_set_flg_sbb_16 (c, s1, s2, s3);
	e87_set_clk_ea (c, 4, 17);

	return (3 + c->ea.cnt);
}

/* OP 81 04: AND r/m16, imm16 */
static
unsigned op_11011_81_04 (e8087_t *c)
{
	unsigned long s1, s2, d;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_ea16 (c);
	s2 = e87_mk_uint16 (c->pq[c->ea.cnt + 1], c->pq[c->ea.cnt + 2]);

	d = s1 & s2;

	e87_set_ea16 (c, d);
	e87_set_flg_log_16 (c, d);
	e87_set_clk_ea (c, 4, 17);

	return (3 + c->ea.cnt);
}

/* OP 81 05: SUB r/m16, imm16 */
static
unsigned op_11011_81_05 (e8087_t *c)
{
	unsigned long s1, s2, d;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_ea16 (c);
	s2 = e87_mk_uint16 (c->pq[c->ea.cnt + 1], c->pq[c->ea.cnt + 2]);

	d = s1 - s2;

	e87_set_ea16 (c, d & 0xffff);
	e87_set_flg_sub_16 (c, s1, s2);
	e87_set_clk_ea (c, 4, 17);

	return (3 + c->ea.cnt);
}

/* OP 81 06: XOR r/m16, imm16 */
static
unsigned op_11011_81_06 (e8087_t *c)
{
	unsigned long s1, s2, d;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_ea16 (c);
	s2 = e87_mk_uint16 (c->pq[c->ea.cnt + 1], c->pq[c->ea.cnt + 2]);

	d = s1 ^ s2;

	e87_set_ea16 (c, d);
	e87_set_flg_log_16 (c, d);
	e87_set_clk_ea (c, 4, 17);

	return (3 + c->ea.cnt);
}

/* OP 81 07: CMP r/m16, imm16 */
static
unsigned op_11011_81_07 (e8087_t *c)
{
	unsigned long s1, s2;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_ea16 (c);
	s2 = e87_mk_uint16 (c->pq[c->ea.cnt + 1], c->pq[c->ea.cnt + 2]);

	e87_set_flg_sub_16 (c, s1, s2);
	e87_set_clk_ea (c, 4, 10);

	return (3 + c->ea.cnt);
}

static
e87_opcode_f e87_opcodes_81[8] = {
	&op_11011_81_00, &op_11011_81_01, &op_11011_81_02, &op_11011_81_03,
	&op_11011_81_04, &op_11011_81_05, &op_11011_81_06, &op_11011_81_07,
};

/* OP 81: xxx r/m16, imm16 */
static
unsigned op_11011_81 (e8087_t *c)
{
	unsigned xop;

	xop = (c->pq[1] >> 3) & 7;

	return (e87_opcodes_81[xop] (c));
}

/* OP 83 00: ADD r/m16, imm8 */
static
unsigned op_11011_83_00 (e8087_t *c)
{
	unsigned long s1, s2, d;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_ea16 (c);
	s2 = e87_mk_sint16 (c->pq[c->ea.cnt + 1]);

	d = s1 + s2;

	e87_set_ea16 (c, d);
	e87_set_flg_add_16 (c, s1, s2);
	e87_set_clk_ea (c, 4, 17);

	return (2 + c->ea.cnt);
}

/* OP 83 01: OR r/m16, imm8 */
static
unsigned op_11011_83_01 (e8087_t *c)
{
	unsigned long s1, s2, d;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_ea16 (c);
	s2 = e87_mk_sint16 (c->pq[c->ea.cnt + 1]);

	d = s1 | s2;

	e87_set_ea16 (c, d);
	e87_set_flg_log_16 (c, d);
	e87_set_clk_ea (c, 4, 17);

	return (2 + c->ea.cnt);
}

/* OP 83 02: ADC r/m16, imm8 */
static
unsigned op_11011_83_02 (e8087_t *c)
{
	unsigned long s1, s2, s3, d;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_ea16 (c);
	s2 = e87_mk_sint16 (c->pq[c->ea.cnt + 1]);
	s3 = e87_get_cf (c);

	d = s1 + s2 + s3;

	e87_set_ea16 (c, d);
	e87_set_flg_adc_16 (c, s1, s2, s3);
	e87_set_clk_ea (c, 4, 17);

	return (2 + c->ea.cnt);
}

/* OP 83 03: SBB r/m16, imm8 */
static
unsigned op_11011_83_03 (e8087_t *c)
{
	unsigned long s1, s2, s3, d;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_ea16 (c);
	s2 = e87_mk_sint16 (c->pq[c->ea.cnt + 1]);
	s3 = e87_get_cf (c);

	d = s1 - s2 - s3;

	e87_set_ea16 (c, d);
	e87_set_flg_sbb_16 (c, s1, s2, s3);
	e87_set_clk_ea (c, 4, 17);

	return (2 + c->ea.cnt);
}

/* OP 83 04: AND r/m16, imm8 */
static
unsigned op_11011_83_04 (e8087_t *c)
{
	unsigned long s1, s2, d;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_ea16 (c);
	s2 = e87_mk_sint16 (c->pq[c->ea.cnt + 1]);

	d = s1 & s2;

	e87_set_ea16 (c, d);
	e87_set_flg_log_16 (c, d);
	e87_set_clk_ea (c, 4, 17);

	return (2 + c->ea.cnt);
}

/* OP 83 05: SUB r/m16, imm8 */
static
unsigned op_11011_83_05 (e8087_t *c)
{
	unsigned long s1, s2, d;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_ea16 (c);
	s2 = e87_mk_sint16 (c->pq[c->ea.cnt + 1]);

	d = s1 - s2;

	e87_set_ea16 (c, d);
	e87_set_flg_sub_16 (c, s1, s2);
	e87_set_clk_ea (c, 4, 17);

	return (2 + c->ea.cnt);
}

/* OP 83 06: XOR r/m16, imm8 */
static
unsigned op_11011_83_06 (e8087_t *c)
{
	unsigned long s1, s2, d;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_ea16 (c);
	s2 = e87_mk_sint16 (c->pq[c->ea.cnt + 1]);

	d = s1 ^ s2;

	e87_set_ea16 (c, d);
	e87_set_flg_log_16 (c, d);
	e87_set_clk_ea (c, 4, 17);

	return (2 + c->ea.cnt);
}

/* OP 83 07: CMP r/m16, imm8 */
static
unsigned op_11011_83_07 (e8087_t *c)
{
	unsigned long s1, s2;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_ea16 (c);
	s2 = e87_mk_sint16 (c->pq[c->ea.cnt + 1]);

	e87_set_flg_sub_16 (c, s1, s2);
	e87_set_clk_ea (c, 4, 10);

	return (2 + c->ea.cnt);
}

static
e87_opcode_f e87_opcodes_83[8] = {
	&op_11011_83_00, &op_11011_83_01, &op_11011_83_02, &op_11011_83_03,
	&op_11011_83_04, &op_11011_83_05, &op_11011_83_06, &op_11011_83_07,
};

/* OP 83: xxx r/m16, imm8 */
static
unsigned op_11011_83 (e8087_t *c)
{
	unsigned xop;

	xop = (c->pq[1] >> 3) & 7;

	return (e87_opcodes_83[xop] (c));
}

/* OP 84: TEST r/m8, reg8 */
static
unsigned op_11011_84 (e8087_t *c)
{
	unsigned short s1, s2;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_ea8 (c);
	s2 = e87_get_reg8 (c, (c->pq[1] >> 3) & 7);

	e87_set_flg_log_8 (c, s1 & s2);
	e87_set_clk_ea (c, 3, 9);

	return (c->ea.cnt + 1);
}

/* OP 85: TEST r/m16, reg16 */
static
unsigned op_11011_85 (e8087_t *c)
{
	unsigned long s1, s2;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_ea16 (c);
	s2 = e87_get_reg16 (c, (c->pq[1] >> 3) & 7);

	e87_set_flg_log_16 (c, s1 & s2);
	e87_set_clk_ea (c, 3, 9);

	return (c->ea.cnt + 1);
}

/* OP 87: XCHG r/m8, reg8 */
static
unsigned op_11011_87 (e8087_t *c)
{
	unsigned       reg;
	unsigned short s1, s2;

	reg = (c->pq[1] >> 3) & 7;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_ea8 (c);
	s2 = e87_get_reg8 (c, reg);

	e87_set_ea8 (c, s2);
	e87_set_reg8 (c, reg, s1);
	e87_set_clk_ea (c, 4, 17);

	return (c->ea.cnt + 1);
}

/* OP 87: XCHG r/m16, reg16 */
static
unsigned op_11011_87 (e8087_t *c)
{
	unsigned      reg;
	unsigned long s1, s2;

	reg = (c->pq[1] >> 3) & 7;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_ea16 (c);
	s2 = e87_get_reg16 (c, reg);

	e87_set_ea16 (c, s2);
	e87_set_reg16 (c, reg, s1);
	e87_set_clk_ea (c, 4, 17);

	return (c->ea.cnt + 1);
}

/* OP 88: MOV r/m8, reg8 */
static
unsigned op_11011_88 (e8087_t *c)
{
	unsigned char val;
	unsigned      reg;

	e87_get_ea_ptr (c, c->pq + 1);

	reg = (c->pq[1] >> 3) & 7;
	val = e87_get_reg8 (c, reg);

	e87_set_ea8 (c, val);
	e87_set_clk_ea (c, 2, 9);

	return (c->ea.cnt + 1);
}

/* OP 89: MOV r/m16, reg16 */
static
unsigned op_11011_89 (e8087_t *c)
{
	unsigned short val;
	unsigned       reg;

	e87_get_ea_ptr (c, c->pq + 1);

	reg = (c->pq[1] >> 3) & 7;
	val = e87_get_reg16 (c, reg);

	e87_set_ea16 (c, val);
	e87_set_clk_ea (c, 2, 9);

	return (c->ea.cnt + 1);
}

/* OP 8A: MOV reg8, r/m8 */
static
unsigned op_11011_8a (e8087_t *c)
{
	unsigned char val;
	unsigned      reg;

	e87_get_ea_ptr (c, c->pq + 1);

	reg = (c->pq[1] >> 3) & 7;
	val = e87_get_ea8 (c);

	e87_set_reg8 (c, reg, val);
	e87_set_clk_ea (c, 2, 8);

	return (c->ea.cnt + 1);
}

/* OP 8B: MOV reg16, r/m16 */
static
unsigned op_11011_8b (e8087_t *c)
{
	unsigned short val;
	unsigned       reg;

	e87_get_ea_ptr (c, c->pq + 1);

	reg = (c->pq[1] >> 3) & 7;
	val = e87_get_ea16 (c);

	e87_set_reg16 (c, reg, val);
	e87_set_clk_ea (c, 2, 8);

	return (c->ea.cnt + 1);
}

/* OP 8C: MOV r/m16, sreg */
static
unsigned op_11011_8c (e8087_t *c)
{
	e87_get_ea_ptr (c, c->pq + 1);
	e87_set_ea16 (c, c->sreg[(c->pq[1] >> 3) & 3]);
	e87_set_clk_ea (c, 2, 9);

	return (c->ea.cnt + 1);
}

/* OP 8D: LEA reg16, r/m16 */
static
unsigned op_11011_8d (e8087_t *c)
{
	e87_get_ea_ptr (c, c->pq + 1);

	if (c->ea.is_mem) {
		e87_set_reg16 (c, (c->pq[1] >> 3) & 7, c->ea.ofs);
		e87_set_clk (c, 2);
	}
	else {
		if (e87_undefined (c) == 0) {
			return (0);
		}
	}

	return (c->ea.cnt + 1);
}

/* OP 8E: MOV sreg, r/m16 */
static
unsigned op_11011_8e (e8087_t *c)
{
	unsigned reg;

	reg = (c->pq[1] >> 3) & 3;

	e87_get_ea_ptr (c, c->pq + 1);
	e87_set_sreg (c, reg, e87_get_ea16 (c));
	e87_set_clk_ea (c, 2, 8);

	if (reg == E87_REG_CS) {
		e87_pq_init (c);
	}
	else if (reg == E87_REG_SS) {
		c->enable_int = 0;
	}

	return (c->ea.cnt + 1);
}

/* OP 8F: POP r/m16 */
static
unsigned op_11011_8f (e8087_t *c)
{
	e87_get_ea_ptr (c, c->pq + 1);
	e87_set_ea16 (c, e87_pop (c));
	e87_set_clk_ea (c, 8, 17);

	return (c->ea.cnt + 1);
}

/* OP 9x: XCHG AX, reg16 */
static
unsigned op_11011_90 (e8087_t *c)
{
	unsigned       reg;
	unsigned short val;

	reg = c->pq[0] & 7;

	val = e87_get_ax (c);
	e87_set_ax (c, e87_get_reg16 (c, reg));
	e87_set_reg16 (c, reg, val);
	e87_set_clk (c, 3);

	return (1);
}

/* OP 98: CBW */
static
unsigned op_11011_98 (e8087_t *c)
{
	e87_set_ax (c, e87_mk_sint16 (e87_get_al (c)));
	e87_set_clk (c, 2);

	return (1);
}

/* OP 99: CWD */
static
unsigned op_11011_99 (e8087_t *c)
{
	e87_set_dx (c, (e87_get_ax (c) & 0x8000) ? 0xffff : 0x0000);
	e87_set_clk (c, 5);

	return (1);
}

/* OP 9A: CALL seg:ofs */
static
unsigned op_11011_9a (e8087_t *c)
{
	e87_push (c, e87_get_cs (c));
	e87_push (c, e87_get_ip (c) + 5);

	e87_set_ip (c, e87_mk_uint16 (c->pq[1], c->pq[2]));
	e87_set_cs (c, e87_mk_uint16 (c->pq[3], c->pq[4]));

	e87_pq_init (c);

	e87_set_clk (c, 28);

	return (0);
}

/* OP 9B: WAIT */
static
unsigned op_11011_9b (e8087_t *c)
{
	e87_set_clk (c, 4);

	return (1);
}

/* OP 9C: PUSHF */
static
unsigned op_11011_9c (e8087_t *c)
{
	if (c->cpu & E87_CPU_FLAGS287) {
		e87_push (c, c->flg & 0x0fd5);
	}
	else {
		e87_push (c, (c->flg & 0x0fd5) | 0xf002);
	}

	e87_set_clk (c, 10);

	return (1);
}

/* OP 9D: POPF */
static
unsigned op_11011_9d (e8087_t *c)
{
	c->flg = (e87_pop (c) & 0x0fd5) | 0xf002;
	e87_set_clk (c, 8);

	return (1);
}

/* OP 9E: SAHF */
static
unsigned op_11011_9e (e8087_t *c)
{
	c->flg &= 0xff00;
	c->flg |= e87_get_ah (c) & 0xd5;
	c->flg |= 0x02;

	e87_set_clk (c, 4);

	return (1);
}

/* OP 9F: LAHF */
static
unsigned op_11011_9f (e8087_t *c)
{
	e87_set_ah (c, (c->flg & 0xd5) | 0x02);
	e87_set_clk (c, 4);

	return (1);
}

/* OP A0: MOV AL, [data16] */
static
unsigned op_11011_a0 (e8087_t *c)
{
	unsigned short seg, ofs;
	unsigned char  val;

	seg = e87_get_seg (c, E87_REG_DS);
	ofs = e87_mk_uint16 (c->pq[1], c->pq[2]);
	val = e87_get_mem8 (c, seg, ofs);

	e87_set_al (c, val);
	e87_set_clk (c, 10);

	return (3);
}

/* OP A1: MOV AX, [data16] */
static
unsigned op_11011_a1 (e8087_t *c)
{
	unsigned short seg, ofs;

	seg = e87_get_seg (c, E87_REG_DS);
	ofs = e87_mk_uint16 (c->pq[1], c->pq[2]);

	e87_set_ax (c, e87_get_mem16 (c, seg, ofs));
	e87_set_clk (c, 10);

	return (3);
}

/* OP A2: MOV [data16], AL */
static
unsigned op_11011_a2 (e8087_t *c)
{
	unsigned short seg, ofs;

	seg = e87_get_seg (c, E87_REG_DS);
	ofs = e87_mk_uint16 (c->pq[1], c->pq[2]);

	e87_set_mem8 (c, seg, ofs, c->dreg[E87_REG_AX] & 0xff);
	e87_set_clk (c, 10);

	return (3);
}

/* OP A3: MOV [data16], AX */
static
unsigned op_11011_a3 (e8087_t *c)
{
	unsigned short seg, ofs;

	seg = e87_get_seg (c, E87_REG_DS);
	ofs = e87_mk_uint16 (c->pq[1], c->pq[2]);

	e87_set_mem16 (c, seg, ofs, c->dreg[E87_REG_AX]);
	e87_set_clk (c, 10);

	return (3);
}

/* OP A4: MOVSB */
static
unsigned op_11011_a4 (e8087_t *c)
{
	unsigned char  val;
	unsigned short seg1, seg2;
	unsigned short inc;

	seg1 = e87_get_seg (c, E87_REG_DS);
	seg2 = e87_get_es (c);
	inc = e87_get_df (c) ? 0xffff : 0x0001;

	if (c->prefix & (E87_PREFIX_REP | E87_PREFIX_REPN)) {
		if (e87_get_cx (c) != 0) {
			val = e87_get_mem8 (c, seg1, e87_get_si (c));
			e87_set_mem8 (c, seg2, e87_get_di (c), val);

			e87_set_si (c, e87_get_si (c) + inc);
			e87_set_di (c, e87_get_di (c) + inc);
			e87_set_cx (c, e87_get_cx (c) - 1);
		}

		e87_set_clk (c, 18);

		if (e87_get_cx (c) == 0) {
			c->prefix &= ~E87_PREFIX_KEEP;
			return (1);
		}

		c->prefix |= E87_PREFIX_KEEP;

		return (0);
	}
	else {
		val = e87_get_mem8 (c, seg1, e87_get_si (c));
		e87_set_mem8 (c, seg2, e87_get_di (c), val);

		e87_set_si (c, e87_get_si (c) + inc);
		e87_set_di (c, e87_get_di (c) + inc);

		e87_set_clk (c, 18);
	}

	return (1);
}

/* OP A5: MOVSW */
static
unsigned op_11011_a5 (e8087_t *c)
{
	unsigned short val;
	unsigned short seg1, seg2;
	unsigned short inc;

	seg1 = e87_get_seg (c, E87_REG_DS);
	seg2 = e87_get_es (c);
	inc = e87_get_df (c) ? 0xfffe : 0x0002;

	if (c->prefix & (E87_PREFIX_REP | E87_PREFIX_REPN)) {
		if (e87_get_cx (c) != 0) {
			val = e87_get_mem16 (c, seg1, e87_get_si (c));
			e87_set_mem16 (c, seg2, e87_get_di (c), val);

			e87_set_si (c, e87_get_si (c) + inc);
			e87_set_di (c, e87_get_di (c) + inc);
			e87_set_cx (c, e87_get_cx (c) - 1);
		}

		e87_set_clk (c, 18);

		if (e87_get_cx (c) == 0) {
			c->prefix &= ~E87_PREFIX_KEEP;
			return (1);
		}

		c->prefix |= E87_PREFIX_KEEP;

		return (0);
	}
	else {
		val = e87_get_mem16 (c, seg1, e87_get_si (c));
		e87_set_mem16 (c, seg2, e87_get_di (c), val);

		e87_set_si (c, e87_get_si (c) + inc);
		e87_set_di (c, e87_get_di (c) + inc);

		e87_set_clk (c, 18);
	}

	return (1);
}

/* Opcode A6: CMPSB */
static
unsigned op_11011_a6 (e8087_t *c)
{
	unsigned short s1, s2;
	unsigned short seg1, seg2;
	unsigned short inc;
	int            z;

	seg1 = e87_get_seg (c, E87_REG_DS);
	seg2 = e87_get_es (c);

	inc = e87_get_df (c) ? 0xffff : 0x0001;

	if (c->prefix & (E87_PREFIX_REP | E87_PREFIX_REPN)) {
		if (e87_get_cx (c) != 0) {
			s1 = e87_get_mem8 (c, seg1, e87_get_si (c));
			s2 = e87_get_mem8 (c, seg2, e87_get_di (c));

			e87_set_si (c, e87_get_si (c) + inc);
			e87_set_di (c, e87_get_di (c) + inc);
			e87_set_cx (c, e87_get_cx (c) - 1);

			e87_set_flg_sub_8 (c, s1, s2);
		}

		e87_set_clk (c, 22);

		z = (c->prefix & E87_PREFIX_REP) ? 1 : 0;

		if ((e87_get_cx (c) == 0) || (e87_get_zf (c) != z)) {
			c->prefix &= ~E87_PREFIX_KEEP;
			return (1);
		}

		c->prefix |= E87_PREFIX_KEEP;

		return (0);
	}
	else {
		s1 = e87_get_mem8 (c, seg1, e87_get_si (c));
		s2 = e87_get_mem8 (c, seg2, e87_get_di (c));

		e87_set_si (c, e87_get_si (c) + inc);
		e87_set_di (c, e87_get_di (c) + inc);

		e87_set_flg_sub_8 (c, s1, s2);

		e87_set_clk (c, 22);
	}

	return (1);
}

/* Opcode A7: CMPSW */
static
unsigned op_11011_a7 (e8087_t *c)
{
	unsigned long  s1, s2;
	unsigned short seg1, seg2;
	unsigned short inc;
	int            z;

	seg1 = e87_get_seg (c, E87_REG_DS);
	seg2 = e87_get_es (c);

	inc = e87_get_df (c) ? 0xfffe : 0x0002;

	if (c->prefix & (E87_PREFIX_REP | E87_PREFIX_REPN)) {
		if (e87_get_cx (c) != 0) {
			s1 = e87_get_mem16 (c, seg1, e87_get_si (c));
			s2 = e87_get_mem16 (c, seg2, e87_get_di (c));

			e87_set_si (c, e87_get_si (c) + inc);
			e87_set_di (c, e87_get_di (c) + inc);
			e87_set_cx (c, e87_get_cx (c) - 1);

			e87_set_flg_sub_16 (c, s1, s2);
		}

		e87_set_clk (c, 22);

		z = (c->prefix & E87_PREFIX_REP) ? 1 : 0;

		if ((e87_get_cx (c) == 0) || (e87_get_zf (c) != z)) {
			c->prefix &= ~E87_PREFIX_KEEP;
			return (1);
		}

		c->prefix |= E87_PREFIX_KEEP;

		return (0);
	}
	else {
		s1 = e87_get_mem16 (c, seg1, e87_get_si (c));
		s2 = e87_get_mem16 (c, seg2, e87_get_di (c));

		e87_set_si (c, e87_get_si (c) + inc);
		e87_set_di (c, e87_get_di (c) + inc);

		e87_set_flg_sub_16 (c, s1, s2);

		e87_set_clk (c, 22);
	}

	return (1);
}

/* Opcode A8: TEST AL, data8 */
static
unsigned op_11011_a8 (e8087_t *c)
{
	unsigned short s1, s2;

	s1 = e87_get_al (c);
	s2 = c->pq[1];

	e87_set_flg_log_8 (c, s1 & s2);
	e87_set_clk (c, 4);

	return (2);
}

/* Opcode A9: TEST AX, data16 */
static
unsigned op_11011_a9 (e8087_t *c)
{
	unsigned long s1, s2;

	s1 = e87_get_ax (c);
	s2 = e87_mk_uint16 (c->pq[1], c->pq[2]);

	e87_set_flg_log_16 (c, s1 & s2);
	e87_set_clk (c, 4);

	return (3);
}

/* Opcode AA: STOSB */
static
unsigned op_11011_aa (e8087_t *c)
{
	unsigned short seg;
	unsigned short inc;

	seg = e87_get_es (c);
	inc = e87_get_df (c) ? 0xffff : 0x0001;

	if (c->prefix & (E87_PREFIX_REP | E87_PREFIX_REPN)) {
		if (e87_get_cx (c) != 0) {
			e87_set_mem8 (c, seg, e87_get_di (c), e87_get_al (c));

			e87_set_di (c, e87_get_di (c) + inc);
			e87_set_cx (c, e87_get_cx (c) - 1);
		}

		e87_set_clk (c, 11);

		if (e87_get_cx (c) == 0) {
			c->prefix &= ~E87_PREFIX_KEEP;
			return (1);
		}

		c->prefix |= E87_PREFIX_KEEP;

		return (0);
	}
	else {
		e87_set_mem8 (c, seg, e87_get_di (c), e87_get_al (c));
		e87_set_di (c, e87_get_di (c) + inc);

		e87_set_clk (c, 11);
	}

	return (1);
}

/* Opcode AB: STOSW */
static
unsigned op_11011_ab (e8087_t *c)
{
	unsigned short seg;
	unsigned short inc;

	seg = e87_get_es (c);
	inc = e87_get_df (c) ? 0xfffe : 0x0002;

	if (c->prefix & (E87_PREFIX_REP | E87_PREFIX_REPN)) {
		if (e87_get_cx (c) != 0) {
			e87_set_mem16 (c, seg, e87_get_di (c), e87_get_ax (c));

			e87_set_di (c, e87_get_di (c) + inc);
			e87_set_cx (c, e87_get_cx (c) - 1);
		}

		e87_set_clk (c, 11);

		if (e87_get_cx (c) == 0) {
			c->prefix &= ~E87_PREFIX_KEEP;
			return (1);
		}

		c->prefix |= E87_PREFIX_KEEP;

		return (0);
	}
	else {
		e87_set_mem16 (c, seg, e87_get_di (c), e87_get_ax (c));
		e87_set_di (c, e87_get_di (c) + inc);

		e87_set_clk (c, 11);
	}

	return (1);
}

/* Opcode AC: LODSB */
static
unsigned op_11011_ac (e8087_t *c)
{
	unsigned short seg;
	unsigned short inc;

	seg = e87_get_seg (c, E87_REG_DS);
	inc = e87_get_df (c) ? 0xffff : 0x0001;

	if (c->prefix & (E87_PREFIX_REP | E87_PREFIX_REPN)) {
		if (e87_get_cx (c) != 0) {
			e87_set_al (c, e87_get_mem8 (c, seg, e87_get_si (c)));
			e87_set_si (c, e87_get_si (c) + inc);
			e87_set_cx (c, e87_get_cx (c) - 1);
		}

		e87_set_clk (c, 12);

		if (e87_get_cx (c) == 0) {
			c->prefix &= ~E87_PREFIX_KEEP;
			return (1);
		}

		c->prefix |= E87_PREFIX_KEEP;

		return (0);
	}
	else {
		e87_set_al (c, e87_get_mem8 (c, seg, e87_get_si (c)));
		e87_set_si (c, e87_get_si (c) + inc);
		e87_set_clk (c, 12);
	}

	return (1);
}

/* Opcode AD: LODSW */
static
unsigned op_11011_ad (e8087_t *c)
{
	unsigned short seg;
	unsigned short inc;

	seg = e87_get_seg (c, E87_REG_DS);
	inc = e87_get_df (c) ? 0xfffe : 0x0002;

	if (c->prefix & (E87_PREFIX_REP | E87_PREFIX_REPN)) {
		if (e87_get_cx (c) != 0) {
			e87_set_ax (c, e87_get_mem16 (c, seg, e87_get_si (c)));
			e87_set_si (c, e87_get_si (c) + inc);
			e87_set_cx (c, e87_get_cx (c) - 1);
		}

		e87_set_clk (c, 12);

		if (e87_get_cx (c) == 0) {
			c->prefix &= ~E87_PREFIX_KEEP;
			return (1);
		}

		c->prefix |= E87_PREFIX_KEEP;

		return (0);
	}
	else {
		e87_set_ax (c, e87_get_mem16 (c, seg, e87_get_si (c)));
		e87_set_si (c, e87_get_si (c) + inc);
		e87_set_clk (c, 12);
	}

	return (1);
}

/* Opcode AE: SCASB */
static
unsigned op_11011_ae (e8087_t *c)
{
	unsigned short s1, s2;
	unsigned short seg;
	unsigned short inc;
	int            z;

	seg = e87_get_es (c);
	inc = e87_get_df (c) ? 0xffff : 0x0001;

	if (c->prefix & (E87_PREFIX_REP | E87_PREFIX_REPN)) {
		if (e87_get_cx (c) != 0) {
			s1 = e87_get_al (c);
			s2 = e87_get_mem8 (c, seg, e87_get_di (c));

			e87_set_di (c, e87_get_di (c) + inc);
			e87_set_cx (c, e87_get_cx (c) - 1);

			e87_set_flg_sub_8 (c, s1, s2);
		}

		e87_set_clk (c, 15);

		z = (c->prefix & E87_PREFIX_REP) ? 1 : 0;

		if ((e87_get_cx (c) == 0) || (e87_get_zf (c) != z)) {
			c->prefix &= ~E87_PREFIX_KEEP;
			return (1);
		}

		c->prefix |= E87_PREFIX_KEEP;

		return (0);
	}
	else {
		s1 = e87_get_al (c);
		s2 = e87_get_mem8 (c, seg, e87_get_di (c));

		e87_set_di (c, e87_get_di (c) + inc);

		e87_set_flg_sub_8 (c, s1, s2);
		e87_set_clk (c, 15);
	}

	return (1);
}

/* Opcode AF: SCASW */
static
unsigned op_11011_af (e8087_t *c)
{
	unsigned long  s1, s2;
	unsigned short seg;
	unsigned short inc;
	int            z;

	seg = e87_get_es (c);
	inc = e87_get_df (c) ? 0xfffe : 0x0002;

	if (c->prefix & (E87_PREFIX_REP | E87_PREFIX_REPN)) {
		if (e87_get_cx (c) != 0) {
			s1 = e87_get_ax (c);
			s2 = e87_get_mem16 (c, seg, e87_get_di (c));

			e87_set_di (c, e87_get_di (c) + inc);
			e87_set_cx (c, e87_get_cx (c) - 1);

			e87_set_flg_sub_16 (c, s1, s2);
		}

		e87_set_clk (c, 15);

		z = (c->prefix & E87_PREFIX_REP) ? 1 : 0;

		if ((e87_get_cx (c) == 0) || (e87_get_zf (c) != z)) {
			c->prefix &= ~E87_PREFIX_KEEP;
			return (1);
		}

		c->prefix |= E87_PREFIX_KEEP;

		return (0);
	}
	else {
		s1 = e87_get_ax (c);
		s2 = e87_get_mem16 (c, seg, e87_get_di (c));

		e87_set_di (c, e87_get_di (c) + inc);

		e87_set_flg_sub_16 (c, s1, s2);
		e87_set_clk (c, 15);
	}

	return (1);
}

/* OP B0: MOV AL, imm8 */
static
unsigned op_11011_b0 (e8087_t *c)
{
	e87_set_al (c, c->pq[1]);
	e87_set_clk (c, 4);

	return (2);
}

/* OP B1: MOV CL, imm8 */
static
unsigned op_11011_b1 (e8087_t *c)
{
	e87_set_cl (c, c->pq[1]);
	e87_set_clk (c, 4);

	return (2);
}

/* OP B2: MOV DL, imm8 */
static
unsigned op_11011_b2 (e8087_t *c)
{
	e87_set_dl (c, c->pq[1]);
	e87_set_clk (c, 4);

	return (2);
}

/* OP B3: MOV BL, imm8 */
static
unsigned op_11011_b3 (e8087_t *c)
{
	e87_set_bl (c, c->pq[1]);
	e87_set_clk (c, 4);

	return (2);
}

/* OP B4: MOV AH, imm8 */
static
unsigned op_11011_b4 (e8087_t *c)
{
	e87_set_ah (c, c->pq[1]);
	e87_set_clk (c, 4);

	return (2);
}

/* OP B5: MOV CH, imm8 */
static
unsigned op_11011_b5 (e8087_t *c)
{
	e87_set_ch (c, c->pq[1]);
	e87_set_clk (c, 4);

	return (2);
}

/* OP B6: MOV DH, imm8 */
static
unsigned op_11011_b6 (e8087_t *c)
{
	e87_set_dh (c, c->pq[1]);
	e87_set_clk (c, 4);

	return (2);
}

/* OP B7: MOV BH, imm8 */
static
unsigned op_11011_b7 (e8087_t *c)
{
	e87_set_bh (c, c->pq[1]);
	e87_set_clk (c, 4);

	return (2);
}

/* OP B8: MOV AX, imm16 */
static
unsigned op_11011_b8 (e8087_t *c)
{
	e87_set_ax (c, e87_mk_uint16 (c->pq[1], c->pq[2]));
	e87_set_clk (c, 4);

	return (3);
}

/* OP B9: MOV CX, imm16 */
static
unsigned op_11011_b9 (e8087_t *c)
{
	e87_set_cx (c, e87_mk_uint16 (c->pq[1], c->pq[2]));
	e87_set_clk (c, 4);

	return (3);
}

/* OP BA: MOV DX, imm16 */
static
unsigned op_11011_ba (e8087_t *c)
{
	e87_set_dx (c, e87_mk_uint16 (c->pq[1], c->pq[2]));
	e87_set_clk (c, 4);

	return (3);
}

/* OP BB: MOV BX, imm16 */
static
unsigned op_11011_bb (e8087_t *c)
{
	e87_set_bx (c, e87_mk_uint16 (c->pq[1], c->pq[2]));
	e87_set_clk (c, 4);

	return (3);
}

/* OP BC: MOV SP, imm16 */
static
unsigned op_11011_bc (e8087_t *c)
{
	e87_set_sp (c, e87_mk_uint16 (c->pq[1], c->pq[2]));
	e87_set_clk (c, 4);

	return (3);
}

/* OP BD: MOV BP, imm16 */
static
unsigned op_11011_bd (e8087_t *c)
{
	e87_set_bp (c, e87_mk_uint16 (c->pq[1], c->pq[2]));
	e87_set_clk (c, 4);

	return (3);
}

/* OP BE: MOV SI, imm16 */
static
unsigned op_11011_be (e8087_t *c)
{
	e87_set_si (c, e87_mk_uint16 (c->pq[1], c->pq[2]));
	e87_set_clk (c, 4);

	return (3);
}

/* OP BF: MOV DI, imm16 */
static
unsigned op_11011_bf (e8087_t *c)
{
	e87_set_di (c, e87_mk_uint16 (c->pq[1], c->pq[2]));
	e87_set_clk (c, 4);

	return (3);
}

/* OP C2: RET imm16 */
static
unsigned op_11011_c2 (e8087_t *c)
{
	e87_set_ip (c, e87_pop (c));
	e87_set_sp (c, e87_get_sp (c) + e87_mk_uint16 (c->pq[1], c->pq[2]));
	e87_pq_init (c);
	e87_set_clk (c, 20);

	return (0);
}

/* OP C3: RET */
static
unsigned op_11011_c3 (e8087_t *c)
{
	e87_set_ip (c, e87_pop (c));
	e87_pq_init (c);
	e87_set_clk (c, 16);

	return (0);
}

/* OP C4: LES reg16, r/m16 */
static
unsigned op_11011_c4 (e8087_t *c)
{
	unsigned short val1, val2;

	e87_get_ea_ptr (c, c->pq + 1);

	if (c->ea.is_mem) {
		val1 = e87_get_mem16 (c, c->ea.seg, c->ea.ofs);
		val2 = e87_get_mem16 (c, c->ea.seg, c->ea.ofs + 2);

		e87_set_reg16 (c, (c->pq[1] >> 3) & 7, val1);
		e87_set_es (c, val2);

		e87_set_clk (c, 16);
	}

	return (c->ea.cnt + 1);
}

/* OP C5: LDS reg16, r/m16 */
static
unsigned op_11011_c5 (e8087_t *c)
{
	unsigned short val1, val2;

	e87_get_ea_ptr (c, c->pq + 1);

	if (c->ea.is_mem) {
		val1 = e87_get_mem16 (c, c->ea.seg, c->ea.ofs);
		val2 = e87_get_mem16 (c, c->ea.seg, c->ea.ofs + 2);

		e87_set_reg16 (c, (c->pq[1] >> 3) & 7, val1);
		e87_set_ds (c, val2);

		e87_set_clk (c, 16);
	}

	return (c->ea.cnt + 1);
}

/* OP C6: MOV r/m8, data8 */
static
unsigned op_11011_c6 (e8087_t *c)
{
	e87_get_ea_ptr (c, c->pq + 1);
	e87_set_ea8 (c, c->pq[c->ea.cnt + 1]);
	e87_set_clk_ea (c, 4, 10);

	return (c->ea.cnt + 2);
}

/* OP C7: MOV r/m16, data16 */
static
unsigned op_11011_c7 (e8087_t *c)
{
	e87_get_ea_ptr (c, c->pq + 1);
	e87_set_ea16 (c, e87_mk_uint16 (c->pq[c->ea.cnt + 1], c->pq[c->ea.cnt + 2]));
	e87_set_clk_ea (c, 4, 10);

	return (c->ea.cnt + 3);
}

/* OP CA: RETF data16 */
static
unsigned op_11011_ca (e8087_t *c)
{
	e87_set_ip (c, e87_pop (c));
	e87_set_cs (c, e87_pop (c));
	e87_set_sp (c, e87_get_sp (c) + e87_mk_uint16 (c->pq[1], c->pq[2]));
	e87_pq_init (c);
	e87_set_clk (c, 25);

	return (0);
}

/* OP CB: RETF */
static
unsigned op_11011_cb (e8087_t *c)
{
	e87_set_ip (c, e87_pop (c));
	e87_set_cs (c, e87_pop (c));
	e87_pq_init (c);
	e87_set_clk (c, 26);

	return (0);
}

/* OP CC: INT 3 */
static
unsigned op_11011_cc (e8087_t *c)
{
	e87_set_ip (c, e87_get_ip (c) + 1);
	e87_trap (c, 3);

	e87_pq_init (c);

	e87_set_clk (c, 52);

	return (0);
}

/* OP CD: INT imm8 */
static
unsigned op_11011_cd (e8087_t *c)
{
	e87_set_ip (c, e87_get_ip (c) + 2);
	e87_trap (c, c->pq[1]);

	e87_pq_init (c);

	e87_set_clk (c, 51);

	return (0);
}

/* OP CE: INTO */
static
unsigned op_11011_ce (e8087_t *c)
{
	if (e87_get_of (c)) {
		e87_set_ip (c, e87_get_ip (c) + 1);
		e87_trap (c, 4);

		e87_pq_init (c);

		e87_set_clk (c, 53);

		return (0);
	}

	e87_set_clk (c, 4);

	return (1);
}

/* OP CF: IRET */
static
unsigned op_11011_cf (e8087_t *c)
{
	e87_set_ip (c, e87_pop (c));
	e87_set_cs (c, e87_pop (c));
	c->flg = e87_pop (c);

	c->enable_int = 0;

	e87_pq_init (c);

	e87_set_clk (c, 32);

	return (0);
}

/* OP D0: ROL, ROR, RCL, RCR, SHL, SHR, SAR r/m8, 1 */
static
unsigned op_11011_d0 (e8087_t *c)
{
	unsigned       xop;
	unsigned short d, s;

	xop = (c->pq[1] >> 3) & 7;

	e87_get_ea_ptr (c, c->pq + 1);
	s = e87_get_ea8 (c);

	switch (xop) {
	case 0: /* ROL r/m8, 1 */
		d = (s << 1) | (s >> 7);
		e87_set_cf (c, s & 0x80);
		break;

	case 1: /* ROR r/m8, 1 */
		d = (s >> 1) | (s << 7);
		e87_set_cf (c, s & 1);
		break;

	case 2: /* RCL r/m8, 1 */
		d = (s << 1) | e87_get_cf (c);
		e87_set_cf (c, s & 0x80);
		break;

	case 3: /* RCR r/m8, 1 */
		d = (s >> 1) | (e87_get_cf (c) << 7);
		e87_set_cf (c, s & 1);
		break;

	case 4: /* SHL r/m8, 1 */
		d = s << 1;
		e87_set_flg_szp_8 (c, d);
		e87_set_cf (c, s & 0x80);
		break;

	case 5: /* SHR r/m8, 1 */
		d = s >> 1;
		e87_set_flg_szp_8 (c, d);
		e87_set_cf (c, s & 1);
		break;

	case 7: /* SAR r/m8, 1 */
		d = (s >> 1) | (s & 0x80);
		e87_set_flg_szp_8 (c, d);
		e87_set_cf (c, s & 1);
		break;

	default:
		return (op_11011_ud (c));
	}

	e87_set_of (c, (d ^ s) & 0x80);

	e87_set_ea8 (c, d & 0xff);
	e87_set_clk_ea (c, 2, 15);

	return (c->ea.cnt + 1);
}

/* OP D1: ROL, ROR, RCL, RCR, SHL, SHR, SAR r/m16, 1 */
static
unsigned op_11011_d1 (e8087_t *c)
{
	unsigned      xop;
	unsigned long d, s;

	xop = (c->pq[1] >> 3) & 7;

	e87_get_ea_ptr (c, c->pq + 1);
	s = e87_get_ea16 (c);

	switch (xop) {
	case 0: /* ROL r/m16, 1 */
		d = (s << 1) | (s >> 15);
		e87_set_cf (c, s & 0x8000);
		break;

	case 1: /* ROR r/m16, 1 */
		d = (s >> 1) | (s << 15);
		e87_set_cf (c, s & 1);
		break;

	case 2: /* RCL r/m16, 1 */
		d = (s << 1) | e87_get_cf (c);
		e87_set_cf (c, s & 0x8000);
		break;

	case 3: /* RCR r/m16, 1 */
		d = (s >> 1) | (e87_get_cf (c) << 15);
		e87_set_cf (c, s & 1);
		break;

	case 4: /* SHL r/m16, 1 */
		d = s << 1;
		e87_set_flg_szp_16 (c, d);
		e87_set_cf (c, s & 0x8000);
		break;

	case 5: /* SHR r/m16, 1 */
		d = s >> 1;
		e87_set_flg_szp_16 (c, d);
		e87_set_cf (c, s & 1);
		break;

	case 7: /* SAR r/m16, 1 */
		d = (s >> 1) | (s & 0x8000);
		e87_set_flg_szp_16 (c, d);
		e87_set_cf (c, s & 1);
		break;

	default:
		return (op_11011_ud (c));
	}

	e87_set_of (c, (d ^ s) & 0x8000);

	e87_set_ea16 (c, d & 0xffff);
	e87_set_clk_ea (c, 2, 15);

	return (c->ea.cnt + 1);
}

/* OP D2: ROL, ROR, RCL, RCR, SHL, SHR, SAR r/m8, CL */
static
unsigned op_11011_d2 (e8087_t *c)
{
	unsigned       xop;
	unsigned       cnt;
	unsigned short d, s;

	xop = (c->pq[1] >> 3) & 7;

	e87_get_ea_ptr (c, c->pq + 1);
	s = e87_get_ea8 (c);

	cnt = e87_get_cl (c);

	if (c->cpu & E87_CPU_MASK_SHIFT) {
		cnt &= 0x1f;
	}

	if (cnt == 0) {
		e87_set_clk_ea (c, 8, 20);
		return (c->ea.cnt + 1);
	}

	switch (xop) {
	case 0: /* ROL r/m8, CL */
		d = (s << (cnt & 7)) | (s >> (8 - (cnt & 7)));
		e87_set_cf (c, d & 1);
		break;

	case 1: /* ROR r/m8, CL */
		d = (s >> (cnt & 7)) | (s << (8 - (cnt & 7)));
		e87_set_cf (c, d & 0x80);
		break;

	case 2: /* RCL r/m8, CL */
		s |= e87_get_cf (c) << 8;
		d = (s << (cnt % 9)) | (s >> (9 - (cnt % 9)));
		e87_set_cf (c, d & 0x100);
		break;

	case 3: /* RCR r/m8, CL */
		s |= e87_get_cf (c) << 8;
		d = (s >> (cnt % 9)) | (s << (9 - (cnt % 9)));
		e87_set_cf (c, d & 0x100);
		break;

	case 4: /* SHL r/m8, CL */
		d = (cnt > 8) ? 0 : (s << cnt);
		e87_set_flg_szp_8 (c, d);
		e87_set_cf (c, d & 0x100);
		break;

	case 5: /* SHR r/m8, CL */
		d = (cnt > 8) ? 0 : (s >> (cnt - 1));
		e87_set_cf (c, d & 1);
		d = d >> 1;
		e87_set_flg_szp_8 (c, d);
		break;

	case 7: /* SAR r/m8, CL */
		s |= (s & 0x80) ? 0xff00 : 0x0000;
		d = s >> ((cnt >= 8) ? 7 : (cnt - 1));
		e87_set_cf (c, d & 1);
		d = (d >> 1) & 0xff;
		e87_set_flg_szp_8 (c, d);
		break;

	default:
		return (op_11011_ud (c));
	}

	e87_set_of (c, (d ^ s) & 0x80);

	e87_set_ea8 (c, d & 0xff);
	e87_set_clk_ea (c, 8 + 4 * cnt, 20 + 4 * cnt);

	return (c->ea.cnt + 1);
}

/* OP D3: ROL, ROR, RCL, RCR, SHL, SHR, SAR r/m16, CL */
static
unsigned op_11011_d3 (e8087_t *c)
{
	unsigned      xop;
	unsigned      cnt;
	unsigned long d, s;

	xop = (c->pq[1] >> 3) & 7;

	e87_get_ea_ptr (c, c->pq + 1);
	s = e87_get_ea16 (c);

	cnt = e87_get_cl (c);

	if (c->cpu & E87_CPU_MASK_SHIFT) {
		cnt &= 0x1f;
	}

	if (cnt == 0) {
		e87_set_clk_ea (c, 8, 20);
		return (c->ea.cnt + 1);
	}

	switch (xop) {
	case 0: /* ROL r/m16, CL */
		d = (s << (cnt & 15)) | (s >> (16 - (cnt & 15)));
		e87_set_cf (c, d & 1);
		break;

	case 1: /* ROR r/m16, CL */
		d = (s >> (cnt & 15)) | (s << (16 - (cnt & 15)));
		e87_set_cf (c, d & 0x8000);
		break;

	case 2: /* RCL r/m16, CL */
		s |= (unsigned long) e87_get_cf (c) << 16;
		d = (s << (cnt % 17)) | (s >> (17 - (cnt % 17)));
		e87_set_cf (c, d & 0x10000);
		break;

	case 3: /* RCR r/m16, CL */
		s |= (unsigned long) e87_get_cf (c) << 16;
		d = (s >> (cnt % 17)) | (s << (17 - (cnt % 17)));
		e87_set_cf (c, d & 0x10000);
		break;

	case 4: /* SHL r/m16, CL */
		d = (cnt > 16) ? 0 : (s << cnt);
		e87_set_flg_szp_16 (c, d);
		e87_set_cf (c, d & 0x10000);
		break;

	case 5: /* SHR r/m16, CL */
		d = (cnt > 16) ? 0 : (s >> (cnt - 1));
		e87_set_cf (c, d & 1);
		d = d >> 1;
		e87_set_flg_szp_16 (c, d);
		break;

	case 7: /* SAR r/m16, CL */
		s |= (s & 0x8000) ? 0xffff0000 : 0x00000000;
		d = s >> ((cnt >= 16) ? 15 : (cnt - 1));
		e87_set_cf (c, d & 1);
		d = (d >> 1) & 0xffff;
		e87_set_flg_szp_16 (c, d);
		break;

	default:
		return (op_11011_ud (c));
	}

	e87_set_of (c, (d ^ s) & 0x8000);

	e87_set_ea16 (c, d);
	e87_set_clk_ea (c, 8 + 4 * cnt, 20 + 4 * cnt);

	return (c->ea.cnt + 1);
}

/* OP D4: AAM imm8 */
static
unsigned op_11011_d4 (e8087_t *c)
{
	unsigned short s, d;
	unsigned short div;

	div = c->pq[1];

	/* check for division by zero */
	if (div == 0) {
		e87_trap (c, 0);
		return (0);
	}

	s = e87_get_al (c);
	d = (((s / div) & 0xff) << 8) | ((s % div) & 0xff);

	e87_set_ax (c, d);

	e87_set_flg_szp_16 (c, d);
	e87_set_clk (c, 83);

	return (2);
}

/* OP D5: AAD imm8 */
static
unsigned op_11011_d5 (e8087_t *c)
{
	unsigned short s1, s2, d;
	unsigned short mul;

	mul = c->pq[1];

	s1 = e87_get_ah (c);
	s2 = e87_get_al (c);

	d = mul * s1 + s2;

	e87_set_ax (c, d & 0xff);
	e87_set_flg_szp_16 (c, d);
	e87_set_clk (c, 60);

	return (2);
}

/* OP D7: XLAT */
static
unsigned op_11011_d7 (e8087_t *c)
{
	unsigned short seg, ofs;

	seg = e87_get_seg (c, E87_REG_DS);
	ofs = e87_get_bx (c) + e87_get_al (c);

	e87_set_al (c, e87_get_mem8 (c, seg, ofs));
	e87_set_clk (c, 11);

	return (1);
}

/* OP D8: ESC */
static
unsigned op_11011_d8 (e8087_t *c)
{
	if (c->cpu & E87_CPU_INT7) {
		e87_trap (c, 7);
		e87_set_clk (c, 50);
		return (0);
	}

	e87_get_ea_ptr (c, c->pq + 1);
	e87_set_clk (c, 2);

	return (1 + c->ea.cnt);
}

/* OP E0: LOOPNZ imm8 */
static
unsigned op_11011_e0 (e8087_t *c)
{
	e87_set_cx (c, e87_get_cx (c) - 1);

	if ((e87_get_cx (c) != 0) && (e87_get_zf (c) == 0)) {
		e87_set_ip (c, e87_add_sint8 (c->ip, c->pq[1]) + 2);
		e87_pq_init (c);
		e87_set_clk (c, 19);

		return (0);
	}

	e87_set_clk (c, 5);

	return (2);
}

/* OP E1: LOOPZ imm8 */
static
unsigned op_11011_e1 (e8087_t *c)
{
	e87_set_cx (c, e87_get_cx (c) - 1);

	if ((e87_get_cx (c) != 0) && (e87_get_zf (c) != 0)) {
		e87_set_ip (c, e87_add_sint8 (c->ip, c->pq[1]) + 2);
		e87_pq_init (c);
		e87_set_clk (c, 18);

		return (0);
	}

	e87_set_clk (c, 5);

	return (2);
}

/* OP E2: LOOP imm8 */
static
unsigned op_11011_e2 (e8087_t *c)
{
	e87_set_cx (c, e87_get_cx (c) - 1);

	if (e87_get_cx (c) != 0) {
		e87_set_ip (c, e87_add_sint8 (c->ip, c->pq[1]) + 2);
		e87_pq_init (c);
		e87_set_clk (c, 18);

		return (0);
	}

	e87_set_clk (c, 5);

	return (2);
}

/* OP E3: JCXZ imm8 */
static
unsigned op_11011_e3 (e8087_t *c)
{
	if (e87_get_cx (c) == 0) {
		e87_set_ip (c, e87_add_sint8 (c->ip + 2, c->pq[1]));
		e87_pq_init (c);
		e87_set_clk (c, 18);
		return (0);
	}

	e87_set_clk (c, 6);

	return (2);
}

/* OP E4: IN AL, imm8 */
static
unsigned op_11011_e4 (e8087_t *c)
{
	e87_set_al (c, e87_get_prt8 (c, c->pq[1]));
	e87_set_clk (c, 11);

	return (2);
}

/* OP E5: IN AX, imm8 */
static
unsigned op_11011_e5 (e8087_t *c)
{
	e87_set_ax (c, e87_get_prt16 (c, c->pq[1]));
	e87_set_clk (c, 15);

	return (2);
}

/* OP E6: OUT imm8, AL */
static
unsigned op_11011_e6 (e8087_t *c)
{
	e87_set_prt8 (c, c->pq[1], e87_get_al (c));
	e87_set_clk (c, 11);

	return (2);
}

/* OP E7: OUT imm8, AX */
static
unsigned op_11011_e7 (e8087_t *c)
{
	e87_set_prt16 (c, c->pq[1], e87_get_ax (c));
	e87_set_clk (c, 11);

	return (2);
}

/* OP E8: CALL imm16 */
static
unsigned op_11011_e8 (e8087_t *c)
{
	e87_push (c, e87_get_ip (c) + 3);
	e87_set_ip (c, e87_get_ip (c) + 3 + e87_mk_uint16 (c->pq[1], c->pq[2]));

	e87_pq_init (c);

	e87_set_clk (c, 19);

	return (0);
}

/* OP E9: JMP imm16 */
static
unsigned op_11011_e9 (e8087_t *c)
{
	e87_set_ip (c, c->ip + e87_mk_uint16 (c->pq[1], c->pq[2]) + 3);
	e87_pq_init (c);
	e87_set_clk (c, 15);

	return (0);
}

/* OP EA: JMP imm32 */
static
unsigned op_11011_ea (e8087_t *c)
{
	e87_set_ip (c, e87_mk_uint16 (c->pq[1], c->pq[2]));
	e87_set_cs (c, e87_mk_uint16 (c->pq[3], c->pq[4]));
	e87_pq_init (c);
	e87_set_clk (c, 15);

	return (0);
}

/* OP EB: JMP imm8 */
static
unsigned op_11011_eb (e8087_t *c)
{
	e87_set_ip (c, e87_add_sint8 (c->ip, c->pq[1]) + 2);
	e87_pq_init (c);
	e87_set_clk (c, 15);

	return (0);
}

/* OP EC: IN AL, DX */
static
unsigned op_11011_ec (e8087_t *c)
{
	e87_set_al (c, e87_get_prt8 (c, e87_get_dx (c)));
	e87_set_clk (c, 9);

	return (1);
}

/* OP ED: IN AX, DX */
static
unsigned op_11011_ed (e8087_t *c)
{
	e87_set_ax (c, e87_get_prt16 (c, e87_get_dx (c)));
	e87_set_clk (c, 13);

	return (1);
}

/* OP EE: OUT DX, AL */
static
unsigned op_11011_ee (e8087_t *c)
{
	e87_set_prt8 (c, e87_get_dx (c), e87_get_al (c));
	e87_set_clk (c, 9);

	return (1);
}

/* OP EF: OUT DX, AX */
static
unsigned op_11011_ef (e8087_t *c)
{
	e87_set_prt16 (c, e87_get_dx (c), e87_get_ax (c));
	e87_set_clk (c, 9);

	return (1);
}

/* OP F0: LOCK */
static
unsigned op_11011_f0 (e8087_t *c)
{
	c->prefix |= (E87_PREFIX_NEW | E87_PREFIX_LOCK);

	e87_set_clk (c, 2);

	return (1);
}

/* OP F2: REPNE */
static
unsigned op_11011_f2 (e8087_t *c)
{
	c->prefix |= (E87_PREFIX_NEW | E87_PREFIX_REPN);
	e87_set_clk (c, 2);

	return (1);
}

/* OP F3: REP */
static
unsigned op_11011_f3 (e8087_t *c)
{
	c->prefix |= (E87_PREFIX_NEW | E87_PREFIX_REP);
	e87_set_clk (c, 2);

	return (1);
}

/* OP F4: HLT */
static
unsigned op_11011_f4 (e8087_t *c)
{
	c->halt = 1;

	e87_set_clk (c, 2);

	return (1);
}

/* OP F5: CMC */
static
unsigned op_11011_f5 (e8087_t *c)
{
	c->flg ^= E87_FLG_C;
	e87_set_clk (c, 2);

	return (1);
}

/* OP F6 00: TEST r/m8, imm8 */
static
unsigned op_11011_f6_00 (e8087_t *c)
{
	unsigned short s1, s2;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_ea8 (c);
	s2 = c->pq[c->ea.cnt + 1];

	e87_set_flg_log_8 (c, s1 & s2);
	e87_set_clk_ea (c, 5, 11);

	return (c->ea.cnt + 2);
}

/* OP F6 02: NOT r/m8 */
static
unsigned op_11011_f6_02 (e8087_t *c)
{
	unsigned short d, s;

	e87_get_ea_ptr (c, c->pq + 1);

	s = e87_get_ea8 (c);
	d = ~s & 0xff;

	e87_set_ea8 (c, d);
	e87_set_clk_ea (c, 3, 16);

	return (c->ea.cnt + 1);
}

/* OP F6 03: NEG r/m8 */
static
unsigned op_11011_f6_03 (e8087_t *c)
{
	unsigned short d, s;

	e87_get_ea_ptr (c, c->pq + 1);

	s = e87_get_ea8 (c);
	d = (~s + 1) & 0xff;

	e87_set_ea8 (c, d);
	e87_set_flg_sub_8 (c, 0, s);
	e87_set_clk_ea (c, 3, 16);

	return (c->ea.cnt + 1);
}

/* OP F6 04: MUL r/m8 */
static
unsigned op_11011_f6_04 (e8087_t *c)
{
	unsigned short d;

	e87_get_ea_ptr (c, c->pq + 1);

	d = e87_get_ea8 (c) * e87_get_al (c);

	e87_set_ax (c, d);
	e87_set_f (c, E87_FLG_O | E87_FLG_C, d & 0xff00);
	e87_set_f (c, E87_FLG_Z, d == 0);
	e87_set_clk_ea (c, (70 + 77) / 2, (76 + 83) / 2);

	return (c->ea.cnt + 1);
}

/* OP F6 05: IMUL r/m8 */
static
unsigned op_11011_f6_05 (e8087_t *c)
{
	unsigned short s;
	unsigned long  d;

	e87_get_ea_ptr (c, c->pq + 1);

	s = e87_get_ea8 (c);

	d = (unsigned long) e87_mk_sint16 (s);
	d *= (unsigned long) e87_mk_sint16 (e87_get_al (c));
	d &= 0xffff;

	e87_set_ax (c, d);

	d &= 0xff00;

	e87_set_f (c, E87_FLG_C | E87_FLG_O, (d != 0xff00) && (d != 0x0000));

	e87_set_clk_ea (c, (80 + 98) / 2, (87 + 104) / 2);

	return (c->ea.cnt + 1);
}

/* OP F6 06: DIV r/m8 */
static
unsigned op_11011_f6_06 (e8087_t *c)
{
	unsigned short s, d;

	e87_get_ea_ptr (c, c->pq + 1);

	s = e87_get_ea8 (c);

	/* check for division by zero */
	if (s == 0) {
		return (op_11011_divide_error (c));
	}

	d = e87_get_ax (c) / s;

	/* check for overflow */
	if (d & 0xff00) {
		return (op_11011_divide_error (c));
	}

	e87_set_ax (c, ((e87_get_ax (c) % s) << 8) | d);

	e87_set_clk_ea (c, (80 + 90) / 2, (87 + 96) / 2);

	return (c->ea.cnt + 1);
}

/* OP F6 07: IDIV r/m8 */
static
unsigned op_11011_f6_07 (e8087_t *c)
{
	unsigned short s1, s2, d1, d2;
	int            sign1, sign2;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_ax (c);
	s2 = e87_mk_sint16 (e87_get_ea8 (c));

	/* check for division by zero */
	if (s2 == 0) {
		return (op_11011_divide_error (c));
	}

	sign1 = (s1 & 0x8000) != 0;
	sign2 = (s2 & 0x8000) != 0;

	s1 = sign1 ? ((~s1 + 1) & 0xffff) : s1;
	s2 = sign2 ? ((~s2 + 1) & 0xffff) : s2;

	d1 = s1 / s2;
	d2 = s1 % s2;

	if (sign1 != sign2) {
		if (d1 > 0x80) {
			return (op_11011_divide_error (c));
		}

		d1 = (~d1 + 1) & 0xff;
	}
	else {
		if (d1 > 0x7f) {
			return (op_11011_divide_error (c));
		}
	}

	if (sign1) {
		d2 = (~d2 + 1) & 0xff;
	}

	e87_set_ax (c, (d2 << 8) | d1);

	e87_set_clk_ea (c, (101 + 112) / 2, (107 + 118) / 2);

	return (c->ea.cnt + 1);
}

static
e87_opcode_f e87_opcodes_f6[8] = {
	&op_11011_f6_00, &op_11011_ud,    &op_11011_f6_02, &op_11011_f6_03,
	&op_11011_f6_04, &op_11011_f6_05, &op_11011_f6_06, &op_11011_f6_07,
};

/* OP F6: xxx r/m8 */
static
unsigned op_11011_f6 (e8087_t *c)
{
	unsigned xop;

	xop = (c->pq[1] >> 3) & 7;

	return (e87_opcodes_f6[xop] (c));
}

/* OP F7 00: TEST r/m16, imm16 */
static
unsigned op_11011_f7_00 (e8087_t *c)
{
	unsigned long s1, s2;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_ea16 (c);
	s2 = e87_mk_uint16 (c->pq[c->ea.cnt + 1], c->pq[c->ea.cnt + 2]);

	e87_set_flg_log_16 (c, s1 & s2);
	e87_set_clk_ea (c, 5, 11);

	return (c->ea.cnt + 3);
}

/* OP F7 02: NOT r/m16 */
static
unsigned op_11011_f7_02 (e8087_t *c)
{
	unsigned long d, s;

	e87_get_ea_ptr (c, c->pq + 1);

	s = e87_get_ea16 (c);
	d = ~s & 0xffff;

	e87_set_ea16 (c, d);
	e87_set_clk_ea (c, 3, 16);

	return (c->ea.cnt + 1);
}

/* OP F7 03: NEG r/m16 */
static
unsigned op_11011_f7_03 (e8087_t *c)
{
	unsigned long d, s;

	e87_get_ea_ptr (c, c->pq + 1);

	s = e87_get_ea16 (c);
	d = (~s + 1) & 0xffff;

	e87_set_ea16 (c, d);
	e87_set_flg_sub_16 (c, 0, s);
	e87_set_clk_ea (c, 3, 16);

	return (c->ea.cnt + 1);
}

/* OP F7 04: MUL r/m16 */
static
unsigned op_11011_f7_04 (e8087_t *c)
{
	unsigned long d;

	e87_get_ea_ptr (c, c->pq + 1);

	d = e87_get_ea16 (c) * e87_get_ax (c);

	e87_set_ax (c, d & 0xffff);
	e87_set_dx (c, d >> 16);
	e87_set_f (c, E87_FLG_C | E87_FLG_O, d & 0xffff0000);
	e87_set_f (c, E87_FLG_Z, d == 0);
	e87_set_clk_ea (c, (118 + 113) / 2, (124 + 139) / 2);

	return (c->ea.cnt + 1);
}

/* OP F7 05: IMUL r/m16 */
static
unsigned op_11011_f7_05 (e8087_t *c)
{
	unsigned long  s1, s2, d;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = e87_get_ax (c);
	s2 = e87_get_ea16 (c);

	s1 = (s1 & 0x8000) ? (s1 | 0xffff0000) : s1;
	s2 = (s2 & 0x8000) ? (s2 | 0xffff0000) : s2;

	d = (s1 * s2) & 0xffffffff;

	e87_set_ax (c, d & 0xffff);
	e87_set_dx (c, d >> 16);

	d &= 0xffff0000;

	e87_set_f (c, E87_FLG_C | E87_FLG_O, (d != 0xffff0000) && (d != 0x00000000));

	e87_set_clk_ea (c, (128 + 154) / 2, (134 + 160) / 2);

	return (c->ea.cnt + 1);
}

/* OP F7 06: DIV r/m16 */
static
unsigned op_11011_f7_06 (e8087_t *c)
{
	unsigned long s1, s2, d;

	e87_get_ea_ptr (c, c->pq + 1);

	s2 = e87_get_ea16 (c);

	/* check for division by zero */
	if (s2 == 0) {
		return (op_11011_divide_error (c));
	}

	s1 = ((unsigned long) e87_get_dx (c) << 16) | e87_get_ax (c);

	d = s1 / s2;

	/* check for overflow */
	if (d & 0xffff0000) {
		return (op_11011_divide_error (c));
	}

	e87_set_ax (c, d);
	e87_set_dx (c, s1 % s2);

	e87_set_clk_ea (c, (144 + 162) / 2, (150 + 168) / 2);

	return (c->ea.cnt + 1);
}

/* OP F7 07: IDIV r/m16 */
static
unsigned op_11011_f7_07 (e8087_t *c)
{
	unsigned long s1, s2, d1, d2;
	int           sign1, sign2;

	e87_get_ea_ptr (c, c->pq + 1);

	s1 = ((unsigned long) e87_get_dx (c) << 16) | e87_get_ax (c);
	s2 = e87_get_ea16 (c);
	s2 = (s2 & 0x8000) ? (s2 | 0xffff0000) : s2;

	if (s2 == 0) {
		return (op_11011_divide_error (c));
	}

	sign1 = (s1 & 0x80000000) != 0;
	sign2 = (s2 & 0x80000000) != 0;

	s1 = sign1 ? ((~s1 + 1) & 0xffffffff) : s1;
	s2 = sign2 ? ((~s2 + 1) & 0xffffffff) : s2;

	d1 = s1 / s2;
	d2 = s1 % s2;

	if (sign1 != sign2) {
		if (d1 > 0x8000) {
			return (op_11011_divide_error (c));
		}

		d1 = (~d1 + 1) & 0xffff;
	}
	else {
		if (d1 > 0x7fff) {
			return (op_11011_divide_error (c));
		}
	}

	if (sign1) {
		d2 = (~d2 + 1) & 0xffff;
	}

	e87_set_ax (c, d1);
	e87_set_dx (c, d2);

	e87_set_clk_ea (c, (165 + 184) / 2, (171 + 190) / 2);

	return (c->ea.cnt + 1);
}

static
e87_opcode_f e87_opcodes_f7[8] = {
	&op_11011_f7_00, &op_11011_ud,    &op_11011_f7_02, &op_11011_f7_03,
	&op_11011_f7_04, &op_11011_f7_05, &op_11011_f7_06, &op_11011_f7_07,
};

/* OP F7: xxx r/m16 */
static
unsigned op_11011_f7 (e8087_t *c)
{
	unsigned xop;

	xop = (c->pq[1] >> 3) & 7;

	return (e87_opcodes_f7[xop] (c));
}

/* OP F8: CLC */
static
unsigned op_11011_f8 (e8087_t *c)
{
	e87_set_cf (c, 0);
	e87_set_clk (c, 2);

	return (1);
}

/* OP F9: STC */
static
unsigned op_11011_f9 (e8087_t *c)
{
	e87_set_cf (c, 1);
	e87_set_clk (c, 2);

	return (1);
}

/* OP FA: CLI */
static
unsigned op_11011_fa (e8087_t *c)
{
	e87_set_if (c, 0);
	e87_set_clk (c, 2);

	return (1);
}

/* OP FB: STI */
static
unsigned op_11011_fb (e8087_t *c)
{
	e87_set_if (c, 1);
	e87_set_clk (c, 2);

	return (1);
}

/* OP FC: CLD */
static
unsigned op_11011_fc (e8087_t *c)
{
	e87_set_df (c, 0);
	e87_set_clk (c, 2);

	return (1);
}

/* OP FD: STD */
static
unsigned op_11011_fd (e8087_t *c)
{
	e87_set_df (c, 1);
	e87_set_clk (c, 2);

	return (1);
}

/* OP FE: INC, DEC r/m8 */
static
unsigned op_11011_fe (e8087_t *c)
{
	unsigned       xop;
	unsigned short d, s;
	unsigned short cf;

	xop = (c->pq[1] >> 3) & 7;

	switch (xop) {
		case 0: /* INC r/m8 */
			e87_get_ea_ptr (c, c->pq + 1);
			s = e87_get_ea8 (c);

			d = s + 1;

			e87_set_ea8 (c, d);

			cf = c->flg;
			e87_set_flg_add_8 (c, s, 1);
			c->flg ^= (c->flg ^ cf) & E87_FLG_C;

			e87_set_clk_ea (c, 3, 15);

			return (c->ea.cnt + 1);

		case 1: /* DEC r/m8 */
			e87_get_ea_ptr (c, c->pq + 1);
			s = e87_get_ea8 (c);

			d = s - 1;

			e87_set_ea8 (c, d);

			cf = c->flg;
			e87_set_flg_sub_8 (c, s, 1);
			c->flg ^= (c->flg ^ cf) & E87_FLG_C;

			e87_set_clk_ea (c, 3, 15);

			return (c->ea.cnt + 1);

		default:
			return (op_11011_ud (c));
	}

	return (0);
}

/* OP FF 00: INC r/m16 */
static
unsigned op_11011_ff_00 (e8087_t *c)
{
	unsigned long  s, d;
	unsigned short cf;

	e87_get_ea_ptr (c, c->pq + 1);

	s = e87_get_ea16 (c);
	d = s + 1;

	e87_set_ea16 (c, d);

	cf = c->flg;
	e87_set_flg_add_16 (c, s, 1);
	c->flg ^= (c->flg ^ cf) & E87_FLG_C;

	e87_set_clk_ea (c, 3, 15);

	return (c->ea.cnt + 1);
}

/* OP FF 01: DEC r/m16 */
static
unsigned op_11011_ff_01 (e8087_t *c)
{
	unsigned long  s, d;
	unsigned short cf;

	e87_get_ea_ptr (c, c->pq + 1);

	s = e87_get_ea16 (c);
	d = s - 1;

	e87_set_ea16 (c, d);

	cf = c->flg;
	e87_set_flg_sub_16 (c, s, 1);
	c->flg ^= (c->flg ^ cf) & E87_FLG_C;

	e87_set_clk_ea (c, 3, 15);

	return (c->ea.cnt + 1);
}

/* OP FF 02: CALL r/m16 */
static
unsigned op_11011_ff_02 (e8087_t *c)
{
	e87_get_ea_ptr (c, c->pq + 1);

	e87_push (c, e87_get_ip (c) + c->ea.cnt + 1);
	e87_set_ip (c, e87_get_ea16 (c));

	e87_pq_init (c);

	e87_set_clk_ea (c, 16, 21);

	return (0);
}

/* OP FF 03: CALL m32 */
static
unsigned op_11011_ff_03 (e8087_t *c)
{
	unsigned short seg, ofs;

	e87_get_ea_ptr (c, c->pq + 1);

	if (!c->ea.is_mem) {
		return (c->ea.cnt + 1);
	}

	e87_push (c, e87_get_cs (c));
	e87_push (c, e87_get_ip (c) + c->ea.cnt + 1);

	seg = e87_get_sego (c, c->ea.seg);
	ofs = c->ea.ofs;

	e87_set_ip (c, e87_get_mem16 (c, seg, ofs));
	e87_set_cs (c, e87_get_mem16 (c, seg, ofs + 2));

	e87_pq_init (c);

	e87_set_clk (c, 37);

	return (0);
}

/* OP FF 04: JMP r/m16 */
static
unsigned op_11011_ff_04 (e8087_t *c)
{
	e87_get_ea_ptr (c, c->pq + 1);
	e87_set_ip (c, e87_get_ea16 (c));
	e87_pq_init (c);
	e87_set_clk_ea (c, 11, 18);

	return (0);
}

/* OP FF 05: JMP m32 */
static
unsigned op_11011_ff_05 (e8087_t *c)
{
	e87_get_ea_ptr (c, c->pq + 1);

	if (!c->ea.is_mem) {
		return (c->ea.cnt + 1);
	}

	e87_set_ip (c, e87_get_mem16 (c, c->ea.seg, c->ea.ofs));
	e87_set_cs (c, e87_get_mem16 (c, c->ea.seg, c->ea.ofs + 2));

	e87_pq_init (c);

	e87_set_clk (c, 24);

	return (0);
}

/* OP FF 06: PUSH r/m16 */
static
unsigned op_11011_ff_06 (e8087_t *c)
{
	e87_get_ea_ptr (c, c->pq + 1);

	if ((c->ea.is_mem == 0) && (c->ea.ofs == E87_REG_SP) && ((c->cpu & E87_CPU_PUSH_FIRST) == 0)) {
		e87_push (c, e87_get_sp (c) - 2);
	}
	else {
		e87_push (c, e87_get_ea16 (c));
	}

	e87_set_clk_ea (c, 10, 16);

	return (c->ea.cnt + 1);
}

static
e87_opcode_f e87_opcodes_ff[8] = {
	&op_11011_ff_00, &op_11011_ff_01, &op_11011_ff_02, &op_11011_ff_03,
	&op_11011_ff_04, &op_11011_ff_05, &op_11011_ff_06, &op_11011_ud
};

/* OP FF: xxx r/m16 */
static
unsigned op_11011_ff (e8087_t *c)
{
	unsigned xop;

	xop = (c->pq[1] >> 3) & 7;

	return (e87_opcodes_ff[xop] (c));
}

e87_opcode_f e87_opcodes[256] = {
	&op_11011_00, &op_11011_01, &op_11011_02, &op_11011_03, &op_11011_04, &op_11011_05, &op_11011_06, &op_11011_07,
	&op_11011_08, &op_11011_09, &op_11011_0a, &op_11011_0b, &op_11011_0c, &op_11011_0d, &op_11011_0e, &op_11011_0f,
	&op_11011_10, &op_11011_11, &op_11011_12, &op_11011_13, &op_11011_14, &op_11011_15, &op_11011_16, &op_11011_17,
	&op_11011_18, &op_11011_19, &op_11011_1a, &op_11011_1b, &op_11011_1c, &op_11011_1d, &op_11011_1e, &op_11011_1f,
	&op_11011_20, &op_11011_21, &op_11011_22, &op_11011_23, &op_11011_24, &op_11011_25, &op_11011_26, &op_11011_27, /* 20 */
	&op_11011_28, &op_11011_29, &op_11011_2a, &op_11011_2b, &op_11011_2c, &op_11011_2d, &op_11011_2e, &op_11011_2f,
	&op_11011_30, &op_11011_31, &op_11011_32, &op_11011_33, &op_11011_34, &op_11011_35, &op_11011_36, &op_11011_37, /* 30 */
	&op_11011_38, &op_11011_39, &op_11011_3a, &op_11011_3b, &op_11011_3c, &op_11011_3d, &op_11011_3e, &op_11011_3f,
	&op_11011_40, &op_11011_40, &op_11011_40, &op_11011_40, &op_11011_40, &op_11011_40, &op_11011_40, &op_11011_40, /* 40 */
	&op_11011_48, &op_11011_48, &op_11011_48, &op_11011_48, &op_11011_48, &op_11011_48, &op_11011_48, &op_11011_48,
	&op_11011_50, &op_11011_50, &op_11011_50, &op_11011_50, &op_11011_50, &op_11011_50, &op_11011_50, &op_11011_50, /* 50 */
	&op_11011_58, &op_11011_58, &op_11011_58, &op_11011_58, &op_11011_58, &op_11011_58, &op_11011_58, &op_11011_58,
	&op_11011_ud, &op_11011_ud, &op_11011_ud, &op_11011_ud, &op_11011_ud, &op_11011_ud, &op_11011_66, &op_11011_ud, /* 60 */
	&op_11011_ud, &op_11011_ud, &op_11011_ud, &op_11011_ud, &op_11011_ud, &op_11011_ud, &op_11011_ud, &op_11011_ud,
	&op_11011_70, &op_11011_71, &op_11011_72, &op_11011_73, &op_11011_74, &op_11011_75, &op_11011_76, &op_11011_77, /* 70 */
	&op_11011_78, &op_11011_79, &op_11011_7a, &op_11011_7b, &op_11011_7c, &op_11011_7d, &op_11011_7e, &op_11011_7f,
	&op_11011_80, &op_11011_81, &op_11011_80, &op_11011_83, &op_11011_84, &op_11011_85, &op_11011_87, &op_11011_87, /* 80 */
	&op_11011_88, &op_11011_89, &op_11011_8a, &op_11011_8b, &op_11011_8c, &op_11011_8d, &op_11011_8e, &op_11011_8f,
	&op_11011_90, &op_11011_90, &op_11011_90, &op_11011_90, &op_11011_90, &op_11011_90, &op_11011_90, &op_11011_90, /* 90 */
	&op_11011_98, &op_11011_99, &op_11011_9a, &op_11011_9b, &op_11011_9c, &op_11011_9d, &op_11011_9e, &op_11011_9f,
	&op_11011_a0, &op_11011_a1, &op_11011_a2, &op_11011_a3, &op_11011_a4, &op_11011_a5, &op_11011_a6, &op_11011_a7, /* A0 */
	&op_11011_a8, &op_11011_a9, &op_11011_aa, &op_11011_ab, &op_11011_ac, &op_11011_ad, &op_11011_ae, &op_11011_af,
	&op_11011_b0, &op_11011_b1, &op_11011_b2, &op_11011_b3, &op_11011_b4, &op_11011_b5, &op_11011_b6, &op_11011_b7, /* B0 */
	&op_11011_b8, &op_11011_b9, &op_11011_ba, &op_11011_bb, &op_11011_bc, &op_11011_bd, &op_11011_be, &op_11011_bf,
	&op_11011_ud, &op_11011_ud, &op_11011_c2, &op_11011_c3, &op_11011_c4, &op_11011_c5, &op_11011_c6, &op_11011_c7, /* C0 */
	&op_11011_ud, &op_11011_ud, &op_11011_ca, &op_11011_cb, &op_11011_cc, &op_11011_cd, &op_11011_ce, &op_11011_cf,
	&op_11011_d0, &op_11011_d1, &op_11011_d2, &op_11011_d3, &op_11011_d4, &op_11011_d5, &op_11011_ud, &op_11011_d7, /* D0 */
	&op_11011_d8, &op_11011_d8, &op_11011_d8, &op_11011_d8, &op_11011_d8, &op_11011_d8, &op_11011_d8, &op_11011_d8,
	&op_11011_e0, &op_11011_e1, &op_11011_e2, &op_11011_e3, &op_11011_e4, &op_11011_e5, &op_11011_e6, &op_11011_e7, /* E0 */
	&op_11011_e8, &op_11011_e9, &op_11011_ea, &op_11011_eb, &op_11011_ec, &op_11011_ed, &op_11011_ee, &op_11011_ef,
	&op_11011_f0, &op_11011_ud, &op_11011_f2, &op_11011_f3, &op_11011_f4, &op_11011_f5, &op_11011_f6, &op_11011_f7, /* F0 */
	&op_11011_f8, &op_11011_f9, &op_11011_fa, &op_11011_fb, &op_11011_fc, &op_11011_fd, &op_11011_fe, &op_11011_ff
};
