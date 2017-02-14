/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/e8087/disasm.c                                       *
 * Created:     2002-05-20 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2002-2009 Hampa Hug <hampa@hampa.ch>                     *
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


#include <string.h>

#include "e8087.h"
#include "intrnl87.h"

/* WARNING: This file is used as a placeholder for the upcoming Intel 8087 FPU. Please do not use this file in any production emulator. */
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
 * Disassembler functions
 **************************************************************************/


static void e87_disasm_op (e87_disasm_t *op, unsigned char *src);


static
char *d_tab_reg8[8] = {
	"AL", "CL", "DL", "BL", "AH", "CH", "DH", "BH"
};

static
char *d_tab_reg16[8] = {
	"AX", "CX", "DX", "BX", "SP", "BP", "SI", "DI"
};

static
char *d_tab_sreg[4] = {
	"ES", "CS", "SS", "DS"
};

static
const char *d_tab_segpre[5] = {
	"", "ES:", "CS:", "SS:", "DS:"
};

typedef struct {
	unsigned   cnt;
	const char *arg;
} d_tab_ea_t;

static
d_tab_ea_t d_tab_ea[24] = {
	{ 0, "BX+SI" },
	{ 0, "BX+DI" },
	{ 0, "BP+SI" },
	{ 0, "BP+DI" },
	{ 0, "SI" },
	{ 0, "DI" },
	{ 2, NULL },
	{ 0, "BX" },
	{ 1, "BX+SI" },
	{ 1, "BX+DI" },
	{ 1, "BP+SI" },
	{ 1, "BP+DI" },
	{ 1, "SI" },
	{ 1, "DI" },
	{ 1, "BP" },
	{ 1, "BX" },
	{ 2, "BX+SI" },
	{ 2, "BX+DI" },
	{ 2, "BP+SI" },
	{ 2, "BP+DI" },
	{ 2, "SI" },
	{ 2, "DI" },
	{ 2, "BP" },
	{ 2, "BX" }
};


static
void disasm_reg8 (char *dst, unsigned r)
{
	strcpy (dst, d_tab_reg8[r & 7]);
}

static
void disasm_reg16 (char *dst, unsigned r)
{
	strcpy (dst, d_tab_reg16[r & 7]);
}

static
void disasm_sreg (char *dst, unsigned r)
{
	strcpy (dst, d_tab_sreg[r & 3]);
}

static
void disasm_uint16 (char *dst, unsigned short v)
{
	sprintf (dst, "%04X", v);
}

static
void disasm_imm8 (char *dst, const unsigned char *src)
{
	sprintf (dst, "%02X", src[0]);
}

static
void disasm_simm8 (char *dst, const unsigned char *src)
{
	if (src[0] < 0x80) {
		sprintf (dst, "+%02X", src[0]);
	}
	else {
		sprintf (dst, "-%02X", (~src[0] + 1) & 0xff);
	}
}

static
void disasm_imm16 (char *dst, const unsigned char *src)
{
	sprintf (dst, "%04X", e87_mk_uint16 (src[0], src[1]));
}

static
void disasm_imm32 (char *dst, const unsigned char *src)
{
	sprintf (dst, "%04X:%04X",
		e87_mk_uint16 (src[2], src[3]),
		e87_mk_uint16 (src[0], src[1])
	);
}

static
void disasm_addr16 (e87_disasm_t *op, char *dst, const unsigned char *src)
{
	sprintf (dst, "[%s%04X]",
		d_tab_segpre[op->seg],
		e87_mk_uint16 (src[0], src[1])
	);

	op->seg = 0;
}

static
void disasm_addr16_index (e87_disasm_t *op, char *dst, const char *reg)
{
	sprintf (dst, "[%s%s]", d_tab_segpre[op->seg], reg);
}

static
unsigned disasm_ea (e87_disasm_t *op, char *str, const unsigned char *src, int s16)
{
	unsigned   val;
	unsigned   ea;
	d_tab_ea_t *eat;
	const char *seg;

	if ((src[0] & 0xc0) == 0xc0) {
		if (s16) {
			disasm_reg16 (str, src[0] & 7);
		}
		else {
			disasm_reg8 (str, src[0] & 7);
		}

		return (1);
	}

	ea = (src[0] & 7) | ((src[0] & 0xc0) >> 3);
	eat = &d_tab_ea[ea];

	seg = d_tab_segpre[op->seg];

	op->seg = 0;

	strcpy (str, s16 ? "WORD " : "BYTE ");
	str += strlen (str);

	if (eat->arg == NULL) {
		sprintf (str, "[%s%04X]", seg,
			e87_mk_uint16 (src[1], src[2])
		);
		return (3);
	}

	switch (eat->cnt) {
	case 0:
		sprintf (str, "[%s%s]", seg, eat->arg);
		break;

	case 1:
		sprintf (str, "[%s%s%c%02X]", seg, eat->arg,
			(src[1] & 0x80) ? '-' : '+',
			((src[1] & 0x80) ? (~src[1] + 1) : src[1]) & 0xff
		);
		break;

	case 2:
		val = e87_mk_uint16 (src[1], src[2]);

		sprintf (str, "[%s%s%c%04X]", seg, eat->arg,
			(val & 0x8000) ? '-' : '+',
			((val & 0x8000) ? (~val + 1) : val) & 0xffff
		);
		break;

	default:
		strcpy (str, "<bad>");
		break;
	}

	return (eat->cnt + 1);
}

static
unsigned disasm_ea8 (e87_disasm_t *op, char *str, const unsigned char *src)
{
	return (disasm_ea (op, str, src, 0));
}

static
unsigned disasm_ea16 (e87_disasm_t *op, char *str, const unsigned char *src)
{
	return (disasm_ea (op, str, src, 1));
}


static
void dop_11011_ud (e87_disasm_t *op, unsigned char *src)
{
	op->dat_n = 1;
	op->arg_n = 1;

	strcpy (op->op, "DB");
	disasm_imm8 (op->arg1, src);
}

static
void dop_11011_const (e87_disasm_t *op, const char *sop, unsigned cnt)
{
	op->dat_n = cnt;
	op->arg_n = 0;

	strcpy (op->op, sop);
}

static
void dop_11011_ea8_reg8 (e87_disasm_t *op, unsigned char *src, char *sop)
{
	unsigned cnt;

	strcpy (op->op, sop);
	cnt = disasm_ea8 (op, op->arg1, src + 1);
	disasm_reg8 (op->arg2, (src[1] >> 3) & 7);

	op->dat_n = 1 + cnt;
	op->arg_n = 2;
}

static
void dop_11011_ea16_reg16 (e87_disasm_t *op, unsigned char *src, char *sop)
{
	unsigned cnt;

	strcpy (op->op, sop);
	cnt = disasm_ea16 (op, op->arg1, src + 1);
	disasm_reg16 (op->arg2, (src[1] >> 3) & 7);

	op->dat_n = 1 + cnt;
	op->arg_n = 2;
}

static
void dop_11011_reg8_ea8 (e87_disasm_t *op, unsigned char *src, char *sop)
{
	unsigned cnt;

	strcpy (op->op, sop);
	disasm_reg8 (op->arg1, (src[1] >> 3) & 7);
	cnt = disasm_ea8 (op, op->arg2, src + 1);

	op->dat_n = 1 + cnt;
	op->arg_n = 2;
}

static
void dop_11011_reg16_ea16 (e87_disasm_t *op, unsigned char *src, char *sop)
{
	unsigned cnt;

	strcpy (op->op, sop);
	disasm_reg16 (op->arg1, (src[1] >> 3) & 7);
	cnt = disasm_ea16 (op, op->arg2, src + 1);

	op->dat_n = 1 + cnt;
	op->arg_n = 2;
}

static
void dop_11011_ea8 (e87_disasm_t *op, unsigned char *src, char *sop)
{
	unsigned cnt;

	strcpy (op->op, sop);
	cnt = disasm_ea8 (op, op->arg1, src + 1);

	op->dat_n = 1 + cnt;
	op->arg_n = 1;
}

static
void dop_11011_ea16 (e87_disasm_t *op, unsigned char *src, char *sop)
{
	unsigned cnt;

	strcpy (op->op, sop);
	cnt = disasm_ea16 (op, op->arg1, src + 1);

	op->dat_n = 1 + cnt;
	op->arg_n = 1;
}

static
void dop_11011_seg_prefix (e87_disasm_t *op, unsigned char *src, const char *sop, unsigned i)
{
	unsigned tmp;

	tmp = op->seg;

	op->seg = i;

	e87_disasm_op (op, src + 1);

	if (op->seg == 0) {
		op->dat_n += 1;
	}
	else {
		dop_11011_const (op, sop, 1);
	}

	op->seg = tmp;
}


/* DOP 00: ADD r/m8, reg8 */
static
void dop_11011_00 (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_ea8_reg8 (op, src, "ADD");
}

/* DOP 01: ADD r/m16, reg16 */
static
void dop_11011_01 (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_ea16_reg16 (op, src, "ADD");
}

/* DOP 02: ADD reg8, r/m8 */
static
void dop_11011_02 (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_reg8_ea8 (op, src, "ADD");
}

/* DOP 03: ADD reg16, r/m16 */
static void dop_11011_03 (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_reg16_ea16 (op, src, "ADD");
}

/* DOP 04: ADD AL, data8 */
static void dop_11011_04 (e87_disasm_t *op, unsigned char *src)
{
	op->dat_n = 2;
	op->arg_n = 2;

	strcpy (op->op, "ADD");
	strcpy (op->arg1, "AL");
	disasm_imm8 (op->arg2, src + 1);
}

/* DOP 05: ADD AX, data16 */
static void dop_11011_05 (e87_disasm_t *op, unsigned char *src)
{
	op->dat_n = 3;
	op->arg_n = 2;

	strcpy (op->op, "ADD");
	strcpy (op->arg1, "AX");
	disasm_imm16 (op->arg2, src + 1);
}

/* DOP 06: PUSH ES */
static void dop_11011_06 (e87_disasm_t *op, unsigned char *src)
{
	op->dat_n = 1;
	op->arg_n = 1;

	strcpy (op->op, "PUSH");
	strcpy (op->arg1, "ES");
}

/* DOP 07: POP ES */
static void dop_11011_07 (e87_disasm_t *op, unsigned char *src)
{
	op->dat_n = 1;
	op->arg_n = 1;

	strcpy (op->op, "POP");
	strcpy (op->arg1, "ES");
}

/* DOP 08: OR r/m8, reg8 */
static void dop_11011_08 (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_ea8_reg8 (op, src, "OR");
}

/* DOP 09: OR r/m16, reg16 */
static void dop_11011_09 (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_ea16_reg16 (op, src, "OR");
}

/* DOP 0A: OR reg8, r/m8 */
static void dop_11011_0a (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_reg8_ea8 (op, src, "OR");
}

/* DOP 0B: OR reg16, r/m16 */
static void dop_11011_0b (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_reg16_ea16 (op, src, "OR");
}

/* DOP 0C: OR AL, data8 */
static void dop_11011_0c (e87_disasm_t *op, unsigned char *src)
{
	op->dat_n = 2;
	op->arg_n = 2;

	strcpy (op->op, "OR");
	strcpy (op->arg1, "AL");
	disasm_imm8 (op->arg2, src + 1);
}

/* DOP 0D: OR AX, data16 */
static void dop_11011_0d (e87_disasm_t *op, unsigned char *src)
{
	op->dat_n = 3;
	op->arg_n = 2;

	strcpy (op->op, "OR");
	strcpy (op->arg1, "AX");
	disasm_imm16 (op->arg2, src + 1);
}

/* DOP 0E: PUSH CS */
static void dop_11011_0e (e87_disasm_t *op, unsigned char *src)
{
	op->dat_n = 1;
	op->arg_n = 1;

	strcpy (op->op, "PUSH");
	strcpy (op->arg1, "CS");
}

/* DOP 0F: POP ES */
static void dop_11011_0f (e87_disasm_t *op, unsigned char *src)
{
	op->dat_n = 1;
	op->arg_n = 1;

	strcpy (op->op, "POP");
	strcpy (op->arg1, "CS");
}

/* DOP 10: ADC r/m8, reg8 */
static void dop_11011_10 (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_ea8_reg8 (op, src, "ADC");
}

/* DOP 11: ADC r/m16, reg16 */
static void dop_11011_11 (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_ea16_reg16 (op, src, "ADC");
}

/* DOP 12: ADC reg8, r/m8 */
static void dop_11011_12 (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_reg8_ea8 (op, src, "ADC");
}

/* DOP 13: ADC reg16, r/m16 */
static void dop_11011_13 (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_reg16_ea16 (op, src, "ADC");
}

/* DOP 14: ADC AL, data8 */
static void dop_11011_14 (e87_disasm_t *op, unsigned char *src)
{
	op->dat_n = 2;
	op->arg_n = 2;

	strcpy (op->op, "ADC");
	strcpy (op->arg1, "AL");
	disasm_imm8 (op->arg2, src + 1);
}

/* DOP 15: ADC AX, data16 */
static void dop_11011_15 (e87_disasm_t *op, unsigned char *src)
{
	op->dat_n = 3;
	op->arg_n = 2;

	strcpy (op->op, "ADC");
	strcpy (op->arg1, "AX");
	disasm_imm16 (op->arg2, src + 1);
}

/* DOP 16: PUSH SS */
static void dop_11011_16 (e87_disasm_t *op, unsigned char *src)
{
	op->dat_n = 1;
	op->arg_n = 1;

	strcpy (op->op, "PUSH");
	strcpy (op->arg1, "SS");
}

/* DOP 17: POP SS */
static void dop_11011_17 (e87_disasm_t *op, unsigned char *src)
{
	op->dat_n = 1;
	op->arg_n = 1;

	strcpy (op->op, "POP");
	strcpy (op->arg1, "SS");
}

/* DOP 18: SBB r/m8, reg8 */
static
void dop_11011_18 (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_ea8_reg8 (op, src, "SBB");
}

/* DOP 19: SBB r/m16, reg16 */
static
void dop_11011_19 (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_ea16_reg16 (op, src, "SBB");
}

/* DOP 1A: SBB reg8, r/m8 */
static
void dop_11011_1a (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_reg8_ea8 (op, src, "SBB");
}

/* DOP 1B: SBB reg16, r/m16 */
static void dop_11011_1b (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_reg16_ea16 (op, src, "SBB");
}

/* DOP 1C: SBB AL, data8 */
static void dop_11011_1c (e87_disasm_t *op, unsigned char *src)
{
	op->dat_n = 2;
	op->arg_n = 2;

	strcpy (op->op, "SBB");
	strcpy (op->arg1, "AL");
	disasm_imm8 (op->arg2, src + 1);
}

/* DOP 1D: SBB AX, data16 */
static void dop_11011_1d (e87_disasm_t *op, unsigned char *src)
{
	op->dat_n = 3;
	op->arg_n = 2;

	strcpy (op->op, "SBB");
	strcpy (op->arg1, "AX");
	disasm_imm16 (op->arg2, src + 1);
}

/* DOP 1E: PUSH DS */
static void dop_11011_1e (e87_disasm_t *op, unsigned char *src)
{
	op->dat_n = 1;
	op->arg_n = 1;

	strcpy (op->op, "PUSH");
	strcpy (op->arg1, "DS");
}

/* DOP 1F: POP SS */
static void dop_11011_1f (e87_disasm_t *op, unsigned char *src)
{
	op->dat_n = 1;
	op->arg_n = 1;

	strcpy (op->op, "POP");
	strcpy (op->arg1, "DS");
}

/* DOP 20: AND r/m8, reg8 */
static void dop_11011_20 (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_ea8_reg8 (op, src, "AND");
}

/* DOP 21: AND r/m16, reg16 */
static void dop_11011_21 (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_ea16_reg16 (op, src, "AND");
}

/* DOP 22: AND reg8, r/m8 */
static void dop_11011_22 (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_reg8_ea8 (op, src, "AND");
}

/* DOP 23: AND reg16, r/m16 */
static void dop_11011_23 (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_reg16_ea16 (op, src, "AND");
}

/* DOP 24: AND AL, data8 */
static void dop_11011_24 (e87_disasm_t *op, unsigned char *src)
{
	op->dat_n = 2;
	op->arg_n = 2;

	strcpy (op->op, "AND");
	strcpy (op->arg1, "AL");
	disasm_imm8 (op->arg2, src + 1);
}

/* DOP 25: AND AX, data16 */
static void dop_11011_25 (e87_disasm_t *op, unsigned char *src)
{
	op->dat_n = 3;
	op->arg_n = 2;

	strcpy (op->op, "AND");
	strcpy (op->arg1, "AX");
	disasm_imm16 (op->arg2, src + 1);
}

/* DOP 26: ES: */
static
void dop_11011_26 (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_seg_prefix (op, src, "ES:", 1);
}

/* DOP 27: DAA */
static
void dop_11011_27 (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_const (op, "DAA", 1);
}

/* DOP 28: SUB r/m8, reg8 */
static void dop_11011_28 (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_ea8_reg8 (op, src, "SUB");
}

/* DOP 29: SUB r/m16, reg16 */
static void dop_11011_29 (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_ea16_reg16 (op, src, "SUB");
}

/* DOP 2A: SUB reg8, r/m8 */
static void dop_11011_2a (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_reg8_ea8 (op, src, "SUB");
}

/* DOP 2B: SUB reg16, r/m16 */
static void dop_11011_2b (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_reg16_ea16 (op, src, "SUB");
}

/* DOP 2C: SUB AL, data8 */
static void dop_11011_2c (e87_disasm_t *op, unsigned char *src)
{
	op->dat_n = 2;
	op->arg_n = 2;

	strcpy (op->op, "SUB");
	strcpy (op->arg1, "AL");
	disasm_imm8 (op->arg2, src + 1);
}

/* DOP 2D: SUB AX, data16 */
static void dop_11011_2d (e87_disasm_t *op, unsigned char *src)
{
	op->dat_n = 3;
	op->arg_n = 2;

	strcpy (op->op, "SUB");
	strcpy (op->arg1, "AX");
	disasm_imm16 (op->arg2, src + 1);
}

/* DOP 2E: CS: */
static
void dop_11011_2e (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_seg_prefix (op, src, "CS:", 2);
}

/* DOP 2F: DAS */
static
void dop_11011_2f (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_const (op, "DAS", 1);
}

/* DOP 30: XOR r/m8, reg8 */
static void dop_11011_30 (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_ea8_reg8 (op, src, "XOR");
}

/* DOP 31: XOR r/m16, reg16 */
static void dop_11011_31 (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_ea16_reg16 (op, src, "XOR");
}

/* DOP 32: XOR reg8, r/m8 */
static void dop_11011_32 (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_reg8_ea8 (op, src, "XOR");
}

/* DOP 33: XOR reg16, r/m16 */
static void dop_11011_33 (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_reg16_ea16 (op, src, "XOR");
}

/* DOP 34: XOR AL, data8 */
static void dop_11011_34 (e87_disasm_t *op, unsigned char *src)
{
	op->dat_n = 2;
	op->arg_n = 2;

	strcpy (op->op, "XOR");
	strcpy (op->arg1, "AL");
	disasm_imm8 (op->arg2, src + 1);
}

/* DOP 35: XOR AX, data16 */
static void dop_11011_35 (e87_disasm_t *op, unsigned char *src)
{
	op->dat_n = 3;
	op->arg_n = 2;

	strcpy (op->op, "XOR");
	strcpy (op->arg1, "AX");
	disasm_imm16 (op->arg2, src + 1);
}

/* DOP 36: SS: */
static
void dop_11011_36 (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_seg_prefix (op, src, "SS:", 3);
}

/* DOP 37: AAA */
static
void dop_11011_37 (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_const (op, "AAA", 1);
}

/* DOP 38: CMP r/m8, reg8 */
static void dop_11011_38 (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_ea8_reg8 (op, src, "CMP");
}

/* DOP 39: CMP r/m16, reg16 */
static void dop_11011_39 (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_ea16_reg16 (op, src, "CMP");
}

/* DOP 3A: CMP reg8, r/m8 */
static void dop_11011_3a (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_reg8_ea8 (op, src, "CMP");
}

/* DOP 3B: CMP reg16, r/m16 */
static void dop_11011_3b (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_reg16_ea16 (op, src, "CMP");
}

/* DOP 3C: CMP AL, data8 */
static void dop_11011_3c (e87_disasm_t *op, unsigned char *src)
{
	op->dat_n = 2;
	op->arg_n = 2;

	strcpy (op->op, "CMP");
	strcpy (op->arg1, "AL");
	disasm_imm8 (op->arg2, src + 1);
}

/* DOP 3D: CMP AX, data16 */
static void dop_11011_3d (e87_disasm_t *op, unsigned char *src)
{
	op->dat_n = 3;
	op->arg_n = 2;

	strcpy (op->op, "CMP");
	strcpy (op->arg1, "AX");
	disasm_imm16 (op->arg2, src + 1);
}

/* DOP 3E: DS: */
static
void dop_11011_3e (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_seg_prefix (op, src, "DS:", 4);
}

/* DOP 3F: AAS */
static
void dop_11011_3f (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_const (op, "AAS", 1);
}

/* DOP 4x: INC reg16 */
static void dop_11011_40 (e87_disasm_t *op, unsigned char *src)
{
	op->dat_n = 1;
	op->arg_n = 1;

	strcpy (op->op, "INC");
	disasm_reg16 (op->arg1, src[0] & 7);
}

/* DOP 4x: DEC reg16 */
static void dop_11011_48 (e87_disasm_t *op, unsigned char *src)
{
	op->dat_n = 1;
	op->arg_n = 1;

	strcpy (op->op, "DEC");
	disasm_reg16 (op->arg1, src[0] & 7);
}

/* DOP 5x: PUSH reg16 */
static void dop_11011_50 (e87_disasm_t *op, unsigned char *src)
{
	op->dat_n = 1;
	op->arg_n = 1;

	strcpy (op->op, "PUSH");
	disasm_reg16 (op->arg1, src[0] & 7);
}

/* DOP 5x: POP reg16 */
static void dop_11011_58 (e87_disasm_t *op, unsigned char *src)
{
	op->dat_n = 1;
	op->arg_n = 1;

	strcpy (op->op, "POP");
	disasm_reg16 (op->arg1, src[0] & 7);
}

static char *d_tab_70[16] = {
"JO", "JNO", "JC",  "JNC", "JZ", "JNZ", "JBE", "JA",
"JS", "JNS", "JPE", "JPO", "JL", "JGE", "JLE", "JG"
};

/* DOP 60: PUSHA */
static
void dop_11011_60 (e87_disasm_t *op, unsigned char *src)
{
	op->flags |= E87_DFLAGS_187;
	dop_11011_const (op, "PUSHA", 1);
}

/* DOP 61: POPA */
static
void dop_11011_61 (e87_disasm_t *op, unsigned char *src)
{
	op->flags |= E87_DFLAGS_187;
	dop_11011_const (op, "POPA", 1);
}

/* DOP 62: BOUND */
static
void dop_11011_62 (e87_disasm_t *op, unsigned char *src)
{
	op->flags |= E87_DFLAGS_187;

	dop_11011_reg16_ea16 (op, src, "BOUND");
}

/* DOP 66: PCE HOOK */
static
void dop_11011_66 (e87_disasm_t *op, unsigned char *src)
{
	if (src[1] != 0x66) {
		dop_11011_ud (op, src);
		return;
	}

	op->dat_n = 4;
	op->arg_n = 2;

	strcpy (op->op, "PCEH");
	disasm_imm8 (op->arg1, src + 2);
	disasm_imm8 (op->arg2, src + 3);
}

/* DOP 68: PUSH imm16 */
static
void dop_11011_68 (e87_disasm_t *op, unsigned char *src)
{
	op->flags |= E87_DFLAGS_187;

	op->dat_n = 3;
	op->arg_n = 1;

	strcpy (op->op, "PUSH");
	disasm_imm16 (op->arg1, src + 1);
}

/* DOP 6A: PUSH imm8 */
static
void dop_11011_6a (e87_disasm_t *op, unsigned char *src)
{
	op->flags |= E87_DFLAGS_187;

	op->dat_n = 2;
	op->arg_n = 1;

	strcpy (op->op, "PUSH");
	disasm_simm8 (op->arg1, src + 1);
}

/* DOP 6C: INSB */
static
void dop_11011_6c (e87_disasm_t *op, unsigned char *src)
{
	op->flags |= E87_DFLAGS_187;
	dop_11011_const (op, "INSB", 1);
}

/* DOP 6D: INSW */
static
void dop_11011_6d (e87_disasm_t *op, unsigned char *src)
{
	op->flags |= E87_DFLAGS_187;
	dop_11011_const (op, "INSW", 1);
}

/* DOP 6E: OUTB */
static
void dop_11011_6e (e87_disasm_t *op, unsigned char *src)
{
	op->flags |= E87_DFLAGS_187;
	dop_11011_const (op, "OUTSB", 1);
}

/* DOP 6F: OUTW */
static
void dop_11011_6f (e87_disasm_t *op, unsigned char *src)
{
	op->flags |= E87_DFLAGS_187;
	dop_11011_const (op, "OUTSW", 1);
}

/* DOP 7x: Jxx imm8 */
static void dop_11011_70 (e87_disasm_t *op, unsigned char *src)
{
	unsigned short t;

	op->dat_n = 2;
	op->arg_n = 1;

	t = 2 + e87_add_sint8 (op->ip, src[1]);

	strcpy (op->op, d_tab_70[src[0] & 15]);
	disasm_uint16 (op->arg1, t);
}

static char *d_tab_80[16] = {
	"ADD", "OR",  "ADC", "SBB",
	"AND", "SUB", "XOR", "CMP"
};

/* DOP 80: xxx r/m8, imm8 */
static void dop_11011_80 (e87_disasm_t *op, unsigned char *src)
{
	unsigned cnt;

	strcpy (op->op, d_tab_80[(src[1] >> 3) & 7]);
	cnt = disasm_ea8 (op, op->arg1, src + 1);
	disasm_imm8 (op->arg2, src + 1 + cnt);

	op->dat_n = 2 + cnt;
	op->arg_n = 2;
}

/* DOP 81: xxx r/m16, imm16 */
static void dop_11011_81 (e87_disasm_t *op, unsigned char *src)
{
	unsigned cnt;

	strcpy (op->op, d_tab_80[(src[1] >> 3) & 7]);
	cnt = disasm_ea16 (op, op->arg1, src + 1);
	disasm_imm16 (op->arg2, src + 1 + cnt);

	op->dat_n = 3 + cnt;
	op->arg_n = 2;
}

/* DOP 83: xxx r/m16, imm8 */
static void dop_11011_83 (e87_disasm_t *op, unsigned char *src)
{
	unsigned cnt;

	strcpy (op->op, d_tab_80[(src[1] >> 3) & 7]);
	cnt = disasm_ea16 (op, op->arg1, src + 1);
	disasm_simm8 (op->arg2, src + 1 + cnt);

	op->dat_n = 2 + cnt;
	op->arg_n = 2;
}

/* DOP 84: TEST r/m8, reg8 */
static
void dop_11011_84 (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_ea8_reg8 (op, src, "TEST");
}

/* DOP 85: TEST r/m16, reg16 */
static
void dop_11011_85 (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_ea16_reg16 (op, src, "TEST");
}

/* DOP 87: XCHG r/m8, reg8 */
static void dop_11011_87 (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_ea8_reg8 (op, src, "XCHG");
}

/* DOP 87: XCHG r/m16, reg16 */
static void dop_11011_87 (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_ea16_reg16 (op, src, "XCHG");
}

/* DOP 88: MOV r/m8, reg8 */
static void dop_11011_88 (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_ea8_reg8 (op, src, "MOV");
}

/* DOP 89: MOV r/m16, reg16 */
static void dop_11011_89 (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_ea16_reg16 (op, src, "MOV");
}

/* DOP 8A: MOV reg8, r/m8 */
static void dop_11011_8a (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_reg8_ea8 (op, src, "MOV");
}

/* DOP 8B: MOV reg16, r/m16 */
static void dop_11011_8b (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_reg16_ea16 (op, src, "MOV");
}

/* DOP 8C: MOV r/m16, sreg */
static void dop_11011_8c (e87_disasm_t *op, unsigned char *src)
{
	unsigned cnt;

	strcpy (op->op, "MOV");
	cnt = disasm_ea16 (op, op->arg1, src + 1);
	disasm_sreg (op->arg2, (src[1] >> 3) & 3);

	op->dat_n = 1 + cnt;
	op->arg_n = 2;
}

/* DOP 8D: LEA reg16, r/m16 */
static void dop_11011_8d (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_reg16_ea16 (op, src, "LEA");
}

/* DOP 8E: MOV sreg, r/m16 */
static void dop_11011_8e (e87_disasm_t *op, unsigned char *src)
{
	unsigned cnt;

	strcpy (op->op, "MOV");
	disasm_sreg (op->arg1, (src[1] >> 3) & 3);
	cnt = disasm_ea16 (op, op->arg2, src + 1);

	op->dat_n = cnt + 1;
	op->arg_n = 2;
}

/* DOP 8F: POP r/m16 */
static void dop_11011_8f (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_ea16 (op, src, "POP");
}

/* DOP 90: NOP */
static
void dop_11011_90 (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_const (op, "NOP", 1);
}

/* DOP 9x: XCHG AX, reg16 */
static void dop_11011_91 (e87_disasm_t *op, unsigned char *src)
{
	strcpy (op->op, "XCHG");
	strcpy (op->arg1, "AX");
	disasm_reg16 (op->arg2, src[0] & 7);

	op->dat_n = 1;
	op->arg_n = 2;
}

/* DOP 98: CBW */
static void dop_11011_98 (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_const (op, "CBW", 1);
}

/* DOP 99: CWD */
static void dop_11011_99 (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_const (op, "CWD", 1);
}

/* DOP 9A: CALL imm32 */
static
void dop_11011_9a (e87_disasm_t *op, unsigned char *src)
{
	op->flags |= E87_DFLAGS_CALL;

	op->dat_n = 5;
	op->arg_n = 1;

	strcpy (op->op, "CALL");
	disasm_imm32 (op->arg1, src + 1);
}

/* DOP 9B: WAIT */
static void dop_11011_9b (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_const (op, "WAIT", 1);
}

/* DOP 9C: PUSHF */
static void dop_11011_9c (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_const (op, "PUSHF", 1);
}

/* DOP 9D: POPF */
static void dop_11011_9d (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_const (op, "POPF", 1);
}

/* DOP 9E: SAHF */
static void dop_11011_9e (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_const (op, "SAHF", 1);
}

/* DOP 9F: LAHF */
static void dop_11011_9f (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_const (op, "LAHF", 1);
}

/* DOP A0: MOV AL, [data16] */
static void dop_11011_a0 (e87_disasm_t *op, unsigned char *src)
{
	op->dat_n = 3;
	op->arg_n = 2;

	strcpy (op->op, "MOV");
	strcpy (op->arg1, "AL");
	disasm_addr16 (op, op->arg2, src + 1);
}

/* DOP A1: MOV AX, [data16] */
static void dop_11011_a1 (e87_disasm_t *op, unsigned char *src)
{
	op->dat_n = 3;
	op->arg_n = 2;

	strcpy (op->op, "MOV");
	strcpy (op->arg1, "AX");
	disasm_addr16 (op, op->arg2, src + 1);
}

/* DOP A2: MOV [data16], AL */
static void dop_11011_a2 (e87_disasm_t *op, unsigned char *src)
{
	op->dat_n = 3;
	op->arg_n = 2;

	strcpy (op->op, "MOV");
	disasm_addr16 (op, op->arg1, src + 1);
	strcpy (op->arg2, "AL");
}

/* DOP A3: MOV [data16], AX */
static void dop_11011_a3 (e87_disasm_t *op, unsigned char *src)
{
	op->dat_n = 3;
	op->arg_n = 2;

	strcpy (op->op, "MOV");
	disasm_addr16 (op, op->arg1, src + 1);
	strcpy (op->arg2, "AX");
}

/* DOP A4: MOVSB */
static
void dop_11011_a4 (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_const (op, "MOVSB", 1);

	if (op->seg != 0) {
		strcpy (op->arg1, "[ES:DI]");
		disasm_addr16_index (op, op->arg2, "SI");
		op->arg_n = 2;
		op->seg = 0;
	}
}

/* DOP A5: MOVSW */
static
void dop_11011_a5 (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_const (op, "MOVSW", 1);

	if (op->seg != 0) {
		strcpy (op->arg1, "[ES:DI]");
		disasm_addr16_index (op, op->arg2, "SI");
		op->arg_n = 2;
		op->seg = 0;
	}
}

/* DOP A6: CMPSB */
static
void dop_11011_a6 (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_const (op, "CMPSB", 1);

	if (op->seg != 0) {
		strcpy (op->arg1, "[ES:DI]");
		disasm_addr16_index (op, op->arg2, "SI");
		op->arg_n = 2;
		op->seg = 0;
	}
}

/* DOP A7: CMPSW */
static
void dop_11011_a7 (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_const (op, "CMPSW", 1);

	if (op->seg != 0) {
		strcpy (op->arg1, "[ES:DI]");
		disasm_addr16_index (op, op->arg2, "SI");
		op->arg_n = 2;
		op->seg = 0;
	}
}

/* DOP A8: TEST AL, data8 */
static void dop_11011_a8 (e87_disasm_t *op, unsigned char *src)
{
	op->dat_n = 2;
	op->arg_n = 2;

	strcpy (op->op, "TEST");
	strcpy (op->arg1, "AL");
	disasm_imm8 (op->arg2, src + 1);
}

/* DOP A9: TEST AX, data16 */
static void dop_11011_a9 (e87_disasm_t *op, unsigned char *src)
{
	op->dat_n = 3;
	op->arg_n = 2;

	strcpy (op->op, "TEST");
	strcpy (op->arg1, "AX");
	disasm_imm16 (op->arg2, src + 1);
}

/* DOP AA: STOSB */
static
void dop_11011_aa (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_const (op, "STOSB", 1);
}

/* DOP AB: STOSW */
static
void dop_11011_ab (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_const (op, "STOSW", 1);
}

/* DOP AC: LODSB */
static
void dop_11011_ac (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_const (op, "LODSB", 1);

	if (op->seg != 0) {
		disasm_addr16_index (op, op->arg1, "SI");
		op->arg_n = 1;
		op->seg = 0;
	}
}

/* DOP AD: LODSW */
static
void dop_11011_ad (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_const (op, "LODSW", 1);

	if (op->seg != 0) {
		disasm_addr16_index (op, op->arg1, "SI");
		op->arg_n = 1;
		op->seg = 0;
	}
}

/* DOP AE: SCASB */
static
void dop_11011_ae (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_const (op, "SCASB", 1);
}

/* DOP AF: SCASW */
static
void dop_11011_af (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_const (op, "SCASW", 1);
}

/* DOP Bx: MOV reg8, imm8 */
static void dop_11011_b0 (e87_disasm_t *op, unsigned char *src)
{
	op->dat_n = 2;
	op->arg_n = 2;

	strcpy (op->op, "MOV");
	disasm_reg8 (op->arg1, src[0] & 7);
	disasm_imm8 (op->arg2, src + 1);
}

/* DOP Bx: MOV reg16, imm16 */
static void dop_11011_b8 (e87_disasm_t *op, unsigned char *src)
{
	op->dat_n = 3;
	op->arg_n = 2;

	strcpy (op->op, "MOV");
	disasm_reg16 (op->arg1, src[0] & 7);
	disasm_imm16 (op->arg2, src + 1);
}

static char *d_tab_c0[8] = {
	"ROL", "ROR", "RCL", "RCR",
	"SHL", "SHR", "???",  "SAR"
};

/* DOP C0: ROL, ROR, RCL, RCR, SHL, SHR, SAR r/m8, imm8 */
static void dop_11011_c0 (e87_disasm_t *op, unsigned char *src)
{
	unsigned xop, cnt;

	op->flags |= E87_DFLAGS_187;

	xop = (src[1] >> 3) & 7;

	strcpy (op->op, d_tab_c0[xop]);
	cnt = disasm_ea8 (op, op->arg1, src + 1);
	disasm_imm8 (op->arg2, src + cnt + 1);

	op->dat_n = 2 + cnt;
	op->arg_n = 2;
}

/* DOP C1: ROL, ROR, RCL, RCR, SHL, SHR, SAR r/m16, imm8 */
static void dop_11011_c1 (e87_disasm_t *op, unsigned char *src)
{
	unsigned xop, cnt;

	op->flags |= E87_DFLAGS_187;

	xop = (src[1] >> 3) & 7;

	strcpy (op->op, d_tab_c0[xop]);
	cnt = disasm_ea16 (op, op->arg1, src + 1);
	disasm_imm8 (op->arg2, src + cnt + 1);

	op->dat_n = 2 + cnt;
	op->arg_n = 2;
}

/* DOP C2: RET imm16 */
static void dop_11011_c2 (e87_disasm_t *op, unsigned char *src)
{
	op->dat_n = 3;
	op->arg_n = 1;

	strcpy (op->op, "RETN");
	disasm_imm16 (op->arg1, src + 1);
}

/* DOP C3: RET */
static
void dop_11011_c3 (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_const (op, "RETN", 1);
}

/* DOP C4: LES reg16, r/m16 */
static void dop_11011_c4 (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_reg16_ea16 (op, src, "LES");
}

/* DOP C5: LDS reg16, r/m16 */
static void dop_11011_c5 (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_reg16_ea16 (op, src, "LDS");
}

/* DOP C6: MOV r/m8, imm8 */
static void dop_11011_c6 (e87_disasm_t *op, unsigned char *src)
{
	strcpy (op->op, "MOV");
	op->dat_n = disasm_ea8 (op, op->arg1, src + 1);
	disasm_imm8 (op->arg2, src + 1 + op->dat_n);

	op->dat_n += 2;
	op->arg_n = 2;
}

/* DOP C7: MOV r/m16, imm16 */
static void dop_11011_c7 (e87_disasm_t *op, unsigned char *src)
{
	strcpy (op->op, "MOV");
	op->dat_n = disasm_ea16 (op, op->arg1, src + 1);
	disasm_imm16 (op->arg2, src + 1 + op->dat_n);

	op->dat_n += 3;
	op->arg_n = 2;
}

/* DOP C8: ENTER imm16, imm8 */
static
void dop_11011_c8 (e87_disasm_t *op, unsigned char *src)
{
	op->flags |= E87_DFLAGS_187;

	strcpy (op->op, "ENTER");
	disasm_imm16 (op->arg1, src + 1);
	disasm_imm8 (op->arg2, src + 3);

	op->dat_n = 4;
	op->arg_n = 2;
}

/* DOP C9: LEAVE */
static
void dop_11011_c9 (e87_disasm_t *op, unsigned char *src)
{
	op->flags |= E87_DFLAGS_187;
	dop_11011_const (op, "LEAVE", 1);
}

/* DOP CA: RETF imm16 */
static
void dop_11011_ca (e87_disasm_t *op, unsigned char *src)
{
	op->dat_n = 3;
	op->arg_n = 1;

	strcpy (op->op, "RETF");
	disasm_imm16 (op->arg1, src + 1);
}

/* DOP CB: RETF */
static
void dop_11011_cb (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_const (op, "RETF", 1);
}

/* DOP CC: INT3 */
static
void dop_11011_cc (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_const (op, "INT3", 1);
	op->flags |= E87_DFLAGS_CALL;
}

/* DOP CD: INT imm8 */
static
void dop_11011_cd (e87_disasm_t *op, unsigned char *src)
{
	op->flags |= E87_DFLAGS_CALL;

	op->dat_n = 2;
	op->arg_n = 1;

	strcpy (op->op, "INT");
	disasm_imm8 (op->arg1, src + 1);
}

/* DOP CE: INTO */
static
void dop_11011_ce (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_const (op, "INTO", 1);
	op->flags |= E87_DFLAGS_CALL;
}

/* DOP CF: IRET */
static
void dop_11011_cf (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_const (op, "IRET", 1);
}

static char *d_tab_d0[8] = {
	"ROL", "ROR", "RCL", "RCR",
	"SHL", "SHR", "???",  "SAR"
};

/* DOP D0: ROL, ROR, RCL, RCR, SHL, SHR, SAR r/m8, 1 */
static void dop_11011_d0 (e87_disasm_t *op, unsigned char *src)
{
	unsigned xop, cnt;

	xop = (src[1] >> 3) & 7;

	strcpy (op->op, d_tab_d0[xop]);
	cnt = disasm_ea8 (op, op->arg1, src + 1);
	strcpy (op->arg2, "1");

	op->dat_n = 1 + cnt;
	op->arg_n = 2;
}

/* DOP D1: ROL, ROR, RCL, RCR, SHL, SHR, SAR r/m16, 1 */
static void dop_11011_d1 (e87_disasm_t *op, unsigned char *src)
{
	unsigned xop, cnt;

	xop = (src[1] >> 3) & 7;

	strcpy (op->op, d_tab_d0[xop]);
	cnt = disasm_ea16 (op, op->arg1, src + 1);
	strcpy (op->arg2, "1");

	op->dat_n = 1 + cnt;
	op->arg_n = 2;
}

static char *d_tab_d2[8] = {
	"ROL", "ROR", "RCL", "RCR",
	"SHL", "SHR", "???",  "SAR"
};

/* DOP D2: ROL, ROR, RCL, RCR, SHL, SHR, SAR r/m8, CL */
static void dop_11011_d2 (e87_disasm_t *op, unsigned char *src)
{
	unsigned xop, cnt;

	xop = (src[1] >> 3) & 7;

	strcpy (op->op, d_tab_d2[xop]);
	cnt = disasm_ea8 (op, op->arg1, src + 1);
	disasm_reg8 (op->arg2, E87_REG_CL);

	op->dat_n = 1 + cnt;
	op->arg_n = 2;
}

/* DOP D3: ROL, ROR, RCL, RCR, SHL, SHR, SAR r/m16, CL */
static void dop_11011_d3 (e87_disasm_t *op, unsigned char *src)
{
	unsigned xop, cnt;

	xop = (src[1] >> 3) & 7;

	strcpy (op->op, d_tab_d2[xop]);
	cnt = disasm_ea16 (op, op->arg1, src + 1);
	disasm_reg8 (op->arg2, E87_REG_CL);

	op->dat_n = 1 + cnt;
	op->arg_n = 2;
}

/* DOP D4: AAM imm8 */
static void dop_11011_d4 (e87_disasm_t *op, unsigned char *src)
{
	strcpy (op->op, "AAM");
	disasm_imm8 (op->arg1, src + 1);

	op->dat_n = 2;
	op->arg_n = 1;
}

/* DOP D7: XLAT */
static
void dop_11011_d7 (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_const (op, "XLAT", 1);
}

static char *d_tab_e0[4] = {
	"LOOPNZ", "LOOPZ", "LOOP", "JCXZ"
};

/* DOP E0-E3: LOOPxx imm8 */
static void dop_11011_e0 (e87_disasm_t *op, unsigned char *src)
{
	unsigned short t;

	op->flags |= E87_DFLAGS_LOOP;

	op->dat_n = 2;
	op->arg_n = 1;

	t = 2 + e87_add_sint8 (op->ip, src[1]);

	strcpy (op->op, d_tab_e0[src[0] - 0xe0]);
	disasm_uint16 (op->arg1, t);
}

/* DOP E4: IN AL, imm8 */
static void dop_11011_e4 (e87_disasm_t *op, unsigned char *src)
{
	op->dat_n = 2;
	op->arg_n = 2;

	strcpy (op->op, "IN");
	strcpy (op->arg1, "AL");
	disasm_imm8 (op->arg2, src + 1);
}

/* DOP E5: IN AX, imm8 */
static void dop_11011_e5 (e87_disasm_t *op, unsigned char *src)
{
	op->dat_n = 2;
	op->arg_n = 2;

	strcpy (op->op, "IN");
	strcpy (op->arg1, "AX");
	disasm_imm8 (op->arg2, src + 1);
}

/* DOP E6: OUT imm8, AL */
static void dop_11011_e6 (e87_disasm_t *op, unsigned char *src)
{
	op->dat_n = 2;
	op->arg_n = 2;

	strcpy (op->op, "OUT");
	disasm_imm8 (op->arg1, src + 1);
	strcpy (op->arg2, "AL");
}

/* DOP E7: OUT imm8, AX */
static void dop_11011_e7 (e87_disasm_t *op, unsigned char *src)
{
	op->dat_n = 2;
	op->arg_n = 2;

	strcpy (op->op, "OUT");
	disasm_imm8 (op->arg1, src + 1);
	strcpy (op->arg2, "AX");
}

/* DOP E8: CALL imm16 */
static
void dop_11011_e8 (e87_disasm_t *op, unsigned char *src)
{
	unsigned short t;

	op->flags |= E87_DFLAGS_CALL;

	op->dat_n = 3;
	op->arg_n = 1;

	t = (op->ip + e87_mk_uint16 (src[1], src[2]) + 3) & 0xffff;

	strcpy (op->op, "CALL");
	disasm_uint16 (op->arg1, t);
}

/* DOP E9: JMP imm16 */
static void dop_11011_e9 (e87_disasm_t *op, unsigned char *src)
{
	unsigned short t;

	op->dat_n = 3;
	op->arg_n = 1;

	t = (op->ip + e87_mk_uint16 (src[1], src[2]) + 3) & 0xffff;

	strcpy (op->op, "JMPN");
	disasm_uint16 (op->arg1, t);
}

/* DOP EA: JMP imm32 */
static void dop_11011_ea (e87_disasm_t *op, unsigned char *src)
{
	op->dat_n = 5;
	op->arg_n = 1;

	strcpy (op->op, "JMPF");
	disasm_imm32 (op->arg1, src + 1);
}

/* DOP EB: JMP imm8 */
static void dop_11011_eb (e87_disasm_t *op, unsigned char *src)
{
	unsigned short t;

	op->dat_n = 2;
	op->arg_n = 1;

	t = e87_add_sint8 (op->ip + 2, src[1]);

	strcpy (op->op, "JMPS");
	disasm_uint16 (op->arg1, t);
}

/* DOP EC: IN AL, DX */
static void dop_11011_ec (e87_disasm_t *op, unsigned char *src)
{
	op->dat_n = 1;
	op->arg_n = 2;

	strcpy (op->op, "IN");
	strcpy (op->arg1, "AL");
	strcpy (op->arg2, "DX");
}

/* DOP ED: IN AX, DX */
static void dop_11011_ed (e87_disasm_t *op, unsigned char *src)
{
	op->dat_n = 1;
	op->arg_n = 2;

	strcpy (op->op, "IN");
	strcpy (op->arg1, "AX");
	strcpy (op->arg2, "DX");
}

/* DOP EE: OUT DX, AL */
static
void dop_11011_ee (e87_disasm_t *op, unsigned char *src)
{
	op->dat_n = 1;
	op->arg_n = 2;

	strcpy (op->op, "OUT");
	strcpy (op->arg1, "DX");
	strcpy (op->arg2, "AL");
}

/* DOP EF: OUT DX, AX */
static
void dop_11011_ef (e87_disasm_t *op, unsigned char *src)
{
	op->dat_n = 1;
	op->arg_n = 2;

	strcpy (op->op, "OUT");
	strcpy (op->arg1, "DX");
	strcpy (op->arg2, "AX");
}

/* DOP F0: LOCK */
static
void dop_11011_f0 (e87_disasm_t *op, unsigned char *src)
{
	char tmp[256];

	e87_disasm (op, src + 1, op->ip);

	strcpy (tmp, op->op);
	strcpy (op->op, "LOCK ");
	strcat (op->op, tmp);

	op->dat_n += 1;
}

/* DOP F2: REPNE */
static
void dop_11011_f2 (e87_disasm_t *op, unsigned char *src)
{
	char tmp[256];

	e87_disasm (op, src + 1, op->ip);

	strcpy (tmp, op->op);
	strcpy (op->op, "REPNE ");
	strcat (op->op, tmp);

	op->dat_n += 1;
}

/* DOP F3: REP */
static
void dop_11011_f3 (e87_disasm_t *op, unsigned char *src)
{
	char tmp[256];

	e87_disasm (op, src + 1, op->ip);

	strcpy (tmp, op->op);
	strcpy (op->op, "REP ");
	strcat (op->op, tmp);

	op->dat_n += 1;
}

/* DOP F4: HLT */
static
void dop_11011_f4 (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_const (op, "HLT", 1);
}

/* DOP F5: CMC */
static
void dop_11011_f5 (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_const (op, "CMC", 1);
}

/* DOP F6: xxx r/m8 */
static
void dop_11011_f6 (e87_disasm_t *op, unsigned char *src)
{
	unsigned xop, cnt;

	xop = (src[1] >> 3) & 7;

	switch (xop) {
		case 0:
			strcpy (op->op, "TEST");
			cnt = disasm_ea8 (op, op->arg1, src + 1);
			disasm_imm8 (op->arg2, src + 1 + cnt);
			op->dat_n = cnt + 2;
			op->arg_n = 2;
			return;

		case 1:
			dop_11011_ud (op, src);
			return;

		case 2:
			dop_11011_ea8 (op, src, "NOT");
			return;

		case 3:
			dop_11011_ea8 (op, src, "NEG");
			return;

		case 4:
			dop_11011_ea8 (op, src, "MUL");
			return;

		case 5:
			dop_11011_ea8 (op, src, "IMUL");
			return;

		case 6:
			dop_11011_ea8 (op, src, "DIV");
			return;

		case 7:
			dop_11011_ea8 (op, src, "IDIV");
			return;

		default:
			dop_11011_ud (op, src);
			return;
	}
}

/* DOP F7: xxx r/m16 */
static
void dop_11011_f7 (e87_disasm_t *op, unsigned char *src)
{
	unsigned xop, cnt;

	xop = (src[1] >> 3) & 7;

	switch (xop) {
		case 0:
			strcpy (op->op, "TEST");
			cnt = disasm_ea16 (op, op->arg1, src + 1);
			disasm_imm16 (op->arg2, src + 1 + cnt);
			op->dat_n = cnt + 3;
			op->arg_n = 2;
			return;

		case 1:
			dop_11011_ud (op, src);
			return;

		case 2:
			dop_11011_ea16 (op, src, "NOT");
			return;

		case 3:
			dop_11011_ea16 (op, src, "NEG");
			return;

		case 4:
			dop_11011_ea16 (op, src, "MUL");
			return;

		case 5:
			dop_11011_ea16 (op, src, "IMUL");
			return;

		case 6:
			dop_11011_ea16 (op, src, "DIV");
			return;

		case 7:
			dop_11011_ea16 (op, src, "IDIV");
			return;

		default:
			dop_11011_ud (op, src);
			return;
	}
}

static char *d_tab_f8[6] = {
	"CLC", "STC", "CLI", "STI", "CLD", "STD"
};

/* DOP F8-FD: CLC, STC, CLI, STI, CLD, STD */
static void dop_11011_f8 (e87_disasm_t *op, unsigned char *src)
{
	op->dat_n = 1;
	op->arg_n = 0;

	strcpy (op->op, d_tab_f8[src[0] - 0xf8]);
}

/* DOP FE: INC, DEC r/m8 */
static void dop_11011_fe (e87_disasm_t *op, unsigned char *src)
{
	unsigned xop, cnt;

	xop = (src[1] >> 3) & 7;

	switch (xop) {
		case 0: /* INC r/m8 */
			strcpy (op->op, "INC");
			cnt = disasm_ea8 (op, op->arg1, src + 1);
			op->dat_n = cnt + 1;
			op->arg_n = 1;
			return;

		case 1: /* DEC r/m8 */
			strcpy (op->op, "DEC");
			cnt = disasm_ea8 (op, op->arg1, src + 1);
			op->dat_n = cnt + 1;
			op->arg_n = 1;
			return;

		default:
			dop_11011_ud (op, src);
			return;
	}
}

/* DOP FF: xxx r/m16 */
static
void dop_11011_ff (e87_disasm_t *op, unsigned char *src)
{
	unsigned xop;

	xop = (src[1] >> 3) & 7;

	switch (xop) {
		case 0:
			dop_11011_ea16 (op, src, "INC");
			return;

		case 1:
			dop_11011_ea16 (op, src, "DEC");
			return;

		case 2:
			dop_11011_ea16 (op, src, "CALL");
			op->flags |= E87_DFLAGS_CALL;
			return;

		case 3:
			dop_11011_ea16 (op, src, "CALLF");
			op->flags |= E87_DFLAGS_CALL;
			return;

		case 4:
			dop_11011_ea16 (op, src, "JMP");
			return;

		case 5:
			dop_11011_ea16 (op, src, "JMPF");
			return;

		case 6:
			dop_11011_ea16 (op, src, "PUSH");
			return;

		case 7:
			dop_11011_ud (op, src);
			return;

		default:
			dop_11011_ud (op, src);
			return;
	}
}


typedef void (*e87_disasm_f) (e87_disasm_t *op, unsigned char *src);

static
e87_disasm_f dop_11011_list[256] = {
	&dop_11011_00, &dop_11011_01, &dop_11011_02, &dop_11011_03, &dop_11011_04, &dop_11011_05, &dop_11011_06, &dop_11011_07,
	&dop_11011_08, &dop_11011_09, &dop_11011_0a, &dop_11011_0b, &dop_11011_0c, &dop_11011_0d, &dop_11011_0e, &dop_11011_0f,
	&dop_11011_10, &dop_11011_11, &dop_11011_12, &dop_11011_13, &dop_11011_14, &dop_11011_15, &dop_11011_16, &dop_11011_17,
	&dop_11011_18, &dop_11011_19, &dop_11011_1a, &dop_11011_1b, &dop_11011_1c, &dop_11011_1d, &dop_11011_1e, &dop_11011_1f,
	&dop_11011_20, &dop_11011_21, &dop_11011_22, &dop_11011_23, &dop_11011_24, &dop_11011_25, &dop_11011_26, &dop_11011_27, /* 20 */
	&dop_11011_28, &dop_11011_29, &dop_11011_2a, &dop_11011_2b, &dop_11011_2c, &dop_11011_2d, &dop_11011_2e, &dop_11011_2f,
	&dop_11011_30, &dop_11011_31, &dop_11011_32, &dop_11011_33, &dop_11011_34, &dop_11011_35, &dop_11011_36, &dop_11011_37, /* 30 */
	&dop_11011_38, &dop_11011_39, &dop_11011_3a, &dop_11011_3b, &dop_11011_3c, &dop_11011_3d, &dop_11011_3e, &dop_11011_3f,
	&dop_11011_40, &dop_11011_40, &dop_11011_40, &dop_11011_40, &dop_11011_40, &dop_11011_40, &dop_11011_40, &dop_11011_40, /* 40 */
	&dop_11011_48, &dop_11011_48, &dop_11011_48, &dop_11011_48, &dop_11011_48, &dop_11011_48, &dop_11011_48, &dop_11011_48,
	&dop_11011_50, &dop_11011_50, &dop_11011_50, &dop_11011_50, &dop_11011_50, &dop_11011_50, &dop_11011_50, &dop_11011_50, /* 50 */
	&dop_11011_58, &dop_11011_58, &dop_11011_58, &dop_11011_58, &dop_11011_58, &dop_11011_58, &dop_11011_58, &dop_11011_58,
	&dop_11011_60, &dop_11011_61, &dop_11011_62, &dop_11011_ud, &dop_11011_ud, &dop_11011_ud, &dop_11011_66, &dop_11011_ud, /* 60 */
	&dop_11011_68, &dop_11011_ud, &dop_11011_6a, &dop_11011_ud, &dop_11011_6c, &dop_11011_6d, &dop_11011_6e, &dop_11011_6f,
	&dop_11011_70, &dop_11011_70, &dop_11011_70, &dop_11011_70, &dop_11011_70, &dop_11011_70, &dop_11011_70, &dop_11011_70, /* 70 */
	&dop_11011_70, &dop_11011_70, &dop_11011_70, &dop_11011_70, &dop_11011_70, &dop_11011_70, &dop_11011_70, &dop_11011_70,
	&dop_11011_80, &dop_11011_81, &dop_11011_80, &dop_11011_83, &dop_11011_84, &dop_11011_85, &dop_11011_87, &dop_11011_87, /* 80 */
	&dop_11011_88, &dop_11011_89, &dop_11011_8a, &dop_11011_8b, &dop_11011_8c, &dop_11011_8d, &dop_11011_8e, &dop_11011_8f,
	&dop_11011_90, &dop_11011_91, &dop_11011_91, &dop_11011_91, &dop_11011_91, &dop_11011_91, &dop_11011_91, &dop_11011_91, /* 90 */
	&dop_11011_98, &dop_11011_99, &dop_11011_9a, &dop_11011_9b, &dop_11011_9c, &dop_11011_9d, &dop_11011_9e, &dop_11011_9f,
	&dop_11011_a0, &dop_11011_a1, &dop_11011_a2, &dop_11011_a3, &dop_11011_a4, &dop_11011_a5, &dop_11011_a6, &dop_11011_a7, /* A0 */
	&dop_11011_a8, &dop_11011_a9, &dop_11011_aa, &dop_11011_ab, &dop_11011_ac, &dop_11011_ad, &dop_11011_ae, &dop_11011_af,
	&dop_11011_b0, &dop_11011_b0, &dop_11011_b0, &dop_11011_b0, &dop_11011_b0, &dop_11011_b0, &dop_11011_b0, &dop_11011_b0, /* B0 */
	&dop_11011_b8, &dop_11011_b8, &dop_11011_b8, &dop_11011_b8, &dop_11011_b8, &dop_11011_b8, &dop_11011_b8, &dop_11011_b8,
	&dop_11011_c0, &dop_11011_c1, &dop_11011_c2, &dop_11011_c3, &dop_11011_c4, &dop_11011_c5, &dop_11011_c6, &dop_11011_c7, /* C0 */
	&dop_11011_c8, &dop_11011_c9, &dop_11011_ca, &dop_11011_cb, &dop_11011_cc, &dop_11011_cd, &dop_11011_ce, &dop_11011_cf,
	&dop_11011_d0, &dop_11011_d1, &dop_11011_d2, &dop_11011_d3, &dop_11011_d4, &dop_11011_ud, &dop_11011_ud, &dop_11011_d7, /* D0 */
	&dop_11011_ud, &dop_11011_ud, &dop_11011_ud, &dop_11011_ud, &dop_11011_ud, &dop_11011_ud, &dop_11011_ud, &dop_11011_ud,
	&dop_11011_e0, &dop_11011_e0, &dop_11011_e0, &dop_11011_e0, &dop_11011_e4, &dop_11011_e5, &dop_11011_e6, &dop_11011_e7, /* E0 */
	&dop_11011_e8, &dop_11011_e9, &dop_11011_ea, &dop_11011_eb, &dop_11011_ec, &dop_11011_ed, &dop_11011_ee, &dop_11011_ef,
	&dop_11011_f0, &dop_11011_ud, &dop_11011_f2, &dop_11011_f3, &dop_11011_f4, &dop_11011_f5, &dop_11011_f6, &dop_11011_f7, /* F0 */
	&dop_11011_f8, &dop_11011_f8, &dop_11011_f8, &dop_11011_f8, &dop_11011_f8, &dop_11011_f8, &dop_11011_fe, &dop_11011_ff
};

static
void e87_disasm_op (e87_disasm_t *op, unsigned char *src)
{
	dop_11011_list[src[0]] (op, src);
}

void e87_disasm (e87_disasm_t *op, unsigned char *src, unsigned short ip)
{
	unsigned i;

	op->flags = 0;
	op->seg = 0;
	op->ip = ip;

	e87_disasm_op (op, src);

	for (i = 0; i < op->dat_n; i++) {
		op->dat[i] = src[i];
	}
}

void e87_disasm_mem (e8087_t *c, e87_disasm_t *op, unsigned short cs, unsigned short ip)
{
	unsigned i;
	unsigned char src[16];

	for (i = 0; i < 16; i++) {
		src[i] = e87_get_mem8 (c, cs, (ip + i) & 0xffff);
	}

	e87_disasm (op, src, ip);
}

void e87_disasm_cur (e8087_t *c, e87_disasm_t *op)
{
	e87_disasm_mem (c, op, c->sreg[E87_REG_CS], c->ip);
}
