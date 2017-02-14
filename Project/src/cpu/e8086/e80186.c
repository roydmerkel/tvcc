/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/e8086/e80186.c                                           *
 * Created:     2003-08-29 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2010 Hampa Hug <hampa@hampa.ch>                     *
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


#include "e8086.h"
#include "internal.h"
#include <lib/log.h>

#include <stdlib.h>
#include <stdio.h>


void g386_real_mode_get_ea_ptr(e8086_t *c, unsigned char *ea)
{
	unsigned fea;
	c->ea.data = ea;
	fea = (ea[0] & 7) | ((ea[0] & 0xc0) >> 3);
	e86_ea[fea](c);
	c->ea.ofs &= 0xffffU;
	if (c->prefix & E86_PREFIX_SEG) {
		c->ea.seg = c->seg_override;
		c->delay += 2;
	}
}

void g386_virtual_8086_mode_get_ea_ptr(e8086_t *c, unsigned char *ea)
{
	unsigned fea;
	c->ea.data = ea;
	fea = (ea[0] & 7) | ((ea[0] & 0xc0) >> 3);
	e86_ea[fea](c);
	c->ea.ofs &= 0xffffU;
	if (c->prefix & E86_PREFIX_SEG) {
		c->ea.seg = c->seg_override;
		c->delay += 2;
	}
}

void g386_protected_mode_get_ea_ptr(e8086_t *c, unsigned char *ea)
{
	unsigned fea;
	c->ea.data = ea;
	fea = (ea[0] & 7) | ((ea[0] & 0xc0) >> 3);
	e86_ea[fea](c);
	c->ea.ofs &= 0xffffU;
	if (c->prefix & E86_PREFIX_SEG) {
		c->ea.seg = c->seg_override;
		c->delay += 2;
	}
}

void g386_real_mode_get_ea_ptr32(e8086_t *c, unsigned char *ea)
{
	unsigned fea;
	c->ea.data = ea;
	fea = (ea[0] & 7) | ((ea[0] & 0xc000) >> 3);
	e86_ea[fea](c);
	c->ea.ofs &= 0xffffffffU;
	if (c->prefix & E86_PREFIX_SEG) {
		c->ea.seg = c->seg_override;
		c->delay += 2;
	}
}

void g386_virtual_8086_mode_get_ea_ptr32(e8086_t *c, unsigned char *ea)
{
	unsigned fea;
	c->ea.data = ea;
	fea = (ea[0] & 7) | ((ea[0] & 0xc000) >> 3);
	e86_ea[fea](c);
	c->ea.ofs &= 0xffffffffU;
	if (c->prefix & E86_PREFIX_SEG) {
		c->ea.seg = c->seg_override;
		c->delay += 2;
	}
}

void g386_protected_mode_get_ea_ptr32(e8086_t *c, unsigned char *ea)
{
	unsigned fea;
	c->ea.data = ea;
	fea = (ea[0] & 7) | ((ea[0] & 0xc000) >> 3);
	e86_ea[fea](c);
	c->ea.ofs &= 0xffffffffU;
	if (c->prefix & E86_PREFIX_SEG) {
		c->ea.seg = c->seg_override;
		c->delay += 2;
	}
}

unsigned char g386_real_mode_get_ea8(e8086_t *c)
{
	if (c->ea.is_mem) {
		return (e86_get_mem8(c, c->ea.seg, c->ea.ofs));
	}
	return (e86_get_reg8(c, c->ea.ofs));
}

unsigned char g386_virtual_8086_mode_get_ea8(e8086_t *c)
{
	if (c->ea.is_mem) {
		return (e86_get_mem8(c, c->ea.seg, c->ea.ofs));
	}
	return (e86_get_reg8(c, c->ea.ofs));
}

unsigned char g386_protected_mode_get_ea8(e8086_t *c)
{
	if (c->ea.is_mem) {
		return (e86_get_mem8(c, c->ea.seg, c->ea.ofs));
	}
	return (e86_get_reg8(c, c->ea.ofs));
}

unsigned short g386_real_mode_get_ea16(e8086_t *c)
{
	if (c->ea.is_mem) {
		if ((c->cpu & E86_CPU_8BIT) || (c->ea.ofs & 1)) {
			c->delay += 4;
		}
		return (e86_get_mem16(c, c->ea.seg, c->ea.ofs));
	}
	return (e86_get_reg16(c, c->ea.ofs));
}

unsigned short g386_virtual_8086_mode_get_ea16(e8086_t *c)
{
	if (c->ea.is_mem) {
		if ((c->cpu & E86_CPU_8BIT) || (c->ea.ofs & 1)) {
			c->delay += 4;
		}
		return (e86_get_mem16(c, c->ea.seg, c->ea.ofs));
	}
	return (e86_get_reg16(c, c->ea.ofs));
}

unsigned short g386_protected_mode_get_ea16(e8086_t *c)
{
	if (c->ea.is_mem) {
		if ((c->cpu & E86_CPU_8BIT) || (c->ea.ofs & 1)) {
			c->delay += 4;
		}
		return (e86_get_mem16(c, c->ea.seg, c->ea.ofs));
	}
	return (e86_get_reg16(c, c->ea.ofs));
}

unsigned long g386_real_mode_get_ea32(e8086_t *c)
{
	if (c->ea.is_mem) {
		if ((c->cpu & E86_CPU_8BIT) || (c->ea.ofs & 1)) {
			c->delay += 4;
		}
		return (e386_get_mem32(c, c->ea.seg, c->ea.ofs));
	}
	return (e386_get_reg32(c, c->ea.ofs));
}

unsigned long g386_virtual_8086_mode_get_ea32(e8086_t *c)
{
	if (c->ea.is_mem) {
		if ((c->cpu & E86_CPU_8BIT) || (c->ea.ofs & 1)) {
			c->delay += 4;
		}
		return (e386_get_mem32(c, c->ea.seg, c->ea.ofs));
	}
	return (e386_get_reg32(c, c->ea.ofs));
}

unsigned long g386_protected_mode_get_ea32(e8086_t *c)
{
	if (c->ea.is_mem) {
		if ((c->cpu & E86_CPU_8BIT) || (c->ea.ofs & 1)) {
			c->delay += 4;
		}
		return (e386_get_mem32(c, c->ea.seg, c->ea.ofs));
	}
	return (e386_get_reg32(c, c->ea.ofs));
}

void g386_real_mode_set_ea8(e8086_t *c, unsigned char val)
{
	if (c->ea.is_mem) {
		e86_set_mem8(c, c->ea.seg, c->ea.ofs, val);
	}
	else {
		e86_set_reg8(c, c->ea.ofs, val);
	}
}

void g386_virtual_8086_mode_set_ea8(e8086_t *c, unsigned char val)
{
	if (c->ea.is_mem) {
		e86_set_mem8(c, c->ea.seg, c->ea.ofs, val);
	}
	else {
		e86_set_reg8(c, c->ea.ofs, val);
	}
}

void g386_protected_mode_set_ea8(e8086_t *c, unsigned char val)
{
	if (c->ea.is_mem) {
		e86_set_mem8(c, c->ea.seg, c->ea.ofs, val);
	}
	else {
		e86_set_reg8(c, c->ea.ofs, val);
	}
}

void g386_real_mode_set_ea16(e8086_t *c, unsigned short val)
{
	if (c->ea.is_mem) {
		if ((c->cpu & E86_CPU_8BIT) || (c->ea.ofs & 1)) {
			c->delay += 4;
		}
		e86_set_mem16(c, c->ea.seg, c->ea.ofs, val);
	}
	else {
		e86_set_reg16(c, c->ea.ofs, val);
	}
}

void g386_virtual_8086_mode_set_ea16(e8086_t *c, unsigned short val)
{
	if (c->ea.is_mem) {
		if ((c->cpu & E86_CPU_8BIT) || (c->ea.ofs & 1)) {
			c->delay += 4;
		}
		e86_set_mem16(c, c->ea.seg, c->ea.ofs, val);
	}
	else {
		e86_set_reg16(c, c->ea.ofs, val);
	}
}

void g386_protected_mode_set_ea16(e8086_t *c, unsigned short val)
{
	if (c->ea.is_mem) {
		if ((c->cpu & E86_CPU_8BIT) || (c->ea.ofs & 1)) {
			c->delay += 4;
		}
		e86_set_mem16(c, c->ea.seg, c->ea.ofs, val);
	}
	else {
		e86_set_reg16(c, c->ea.ofs, val);
	}
}

void g386_real_mode_set_ea32(e8086_t *c, unsigned long val)
{
	if (c->ea.is_mem) {
		if ((c->cpu & E86_CPU_8BIT) || (c->ea.ofs & 1)) {
			c->delay += 4;
		}
		e386_set_mem32(c, c->ea.seg, c->ea.ofs, val);
	}
	else {
		e386_set_reg32(c, c->ea.ofs, val);
	}
}

void g386_virtual_8086_mode_set_ea32(e8086_t *c, unsigned long val)
{
	if (c->ea.is_mem) {
		if ((c->cpu & E86_CPU_8BIT) || (c->ea.ofs & 1)) {
			c->delay += 4;
		}
		e386_set_mem32(c, c->ea.seg, c->ea.ofs, val);
	}
	else {
		e386_set_reg32(c, c->ea.ofs, val);
	}
}

void g386_protected_mode_set_ea32(e8086_t *c, unsigned long val)
{
	if (c->ea.is_mem) {
		if ((c->cpu & E86_CPU_8BIT) || (c->ea.ofs & 1)) {
			c->delay += 4;
		}
		e386_set_mem32(c, c->ea.seg, c->ea.ofs, val);
	}
	else {
		e386_set_reg32(c, c->ea.ofs, val);
	}
}

/* PUSH (Real Mode) */
/* void e386_real_mode_push(e8086_t *c, unsigned short val) */
void g386_real_mode_push(e8086_t *c, unsigned short val)
{
	/* 16-bit register operand specified */
	e86_set_sp(c, e86_get_sp(c) - 2);
	if (c->e386_address_size_prefix == 1)
	{
		/* 32-bit stack address size */
		e386_set_mem32(c, e386_get_ss(c), e386_get_esp(c), val);
	}
	else
	{
		/* 16-bit stack address size */
		e86_set_mem16(c, e86_get_ss(c), e86_get_sp(c), val);
	}
}

/* PUSH (Virtual 8086 Mode) */
void g386_virtual_8086_mode_push(e8086_t *c, unsigned short val)
{
	/* 16-bit register operand specified */
	e86_set_sp(c, e86_get_sp(c) - 2);
	if (c->e386_address_size_prefix == 1)
	{
		/* 32-bit stack address size */
		e386_set_mem32(c, e386_get_ss(c), e386_get_esp(c), val);
	}
	else
	{
		/* 16-bit stack address size */
		e86_set_mem16(c, e86_get_ss(c), e86_get_sp(c), val);
	}
}

/* PUSH (Protected Mode) */
void g386_protected_mode_push(e8086_t *c, unsigned short val)
{
	/* 16-bit register operand specified */
	e86_set_sp(c, e86_get_sp(c) - 2);
	if (c->e386_address_size_prefix == 1)
	{
		/* 32-bit stack address size */
		e386_set_mem32(c, e386_get_ss(c), e386_get_esp(c), val);
	}
	else
	{
		/* 16-bit stack address size */
		e86_set_mem16(c, e86_get_ss(c), e86_get_sp(c), val);
	}
}

void g386_real_mode_push32(e8086_t *c, unsigned long val)
{
	/* 32-bit register operand specified */
	e86_set_sp(c, e86_get_sp(c) - 4);
	if (c->e386_address_size_prefix == 1)
	{
		/* 32-bit stack address size */
		e386_set_mem32(c, e386_get_ss(c), e386_get_esp(c), val);
	}
	else
	{
		/* 16-bit stack address size */
		e86_set_mem16(c, e86_get_ss(c), e86_get_sp(c), val);
	}
}

/* PUSH (Virtual 8086 Mode) */
void g386_virtual_8086_mode_push32(e8086_t *c, unsigned long val)
{
	/* 32-bit register operand specified */
	e86_set_sp(c, e86_get_sp(c) - 4);
	if (c->e386_address_size_prefix == 1)
	{
		/* 32-bit stack address size */
		e386_set_mem32(c, e386_get_ss(c), e386_get_esp(c), val);
	}
	else
	{
		/* 16-bit stack address size */
		e86_set_mem16(c, e86_get_ss(c), e86_get_sp(c), val);
	}
}

/* PUSH (Protected Mode) */
void g386_protected_mode_push32(e8086_t *c, unsigned long val)
{
	/* 32-bit register operand specified */
	e86_set_sp(c, e86_get_sp(c) - 4);
	if (c->e386_address_size_prefix == 1)
	{
		/* 32-bit stack address size */
		e386_set_mem32(c, e386_get_ss(c), e386_get_esp(c), val);
	}
	else
	{
		/* 16-bit stack address size */
		e86_set_mem16(c, e86_get_ss(c), e86_get_sp(c), val);
	}
}

/* POP WORD or DWORD register (Real Mode) */
unsigned short g386_real_mode_pop(e8086_t *c)
{
	unsigned short sp;
	sp = e86_get_sp(c);
	e86_set_sp(c, sp + 2);
	if (c->e386_address_size_prefix == 1)
	{
		/* 32-bit stack address size */
		return (e386_get_mem32(c, e386_get_ss(c), sp));
	}
	else
	{
		/* 16-bit stack address size */
		return (e86_get_mem16(c, e86_get_ss(c), sp));
	}
}

/* POP WORD or DWORD register (Virtual 8086 Mode) */
unsigned short g386_virtual_8086_mode_pop(e8086_t *c)
{
	unsigned short sp;
	sp = e86_get_sp(c);
	e86_set_sp(c, sp + 2);
	if (c->e386_address_size_prefix == 1)
	{
		/* 32-bit stack address size */
		return (e386_get_mem32(c, e386_get_ss(c), sp));
	}
	else
	{
		/* 16-bit stack address size */
		return (e86_get_mem16(c, e86_get_ss(c), sp));
	}
}

/* POP WORD or DWORD register (Protected Mode) */
unsigned short g386_protected_mode_pop_word_or_dword_register(e8086_t *c)
{
	unsigned short sp;
	sp = e86_get_sp(c);
	e86_set_sp(c, sp + 2);
	if (c->e386_address_size_prefix == 1)
	{
		/* 32-bit stack address size */
		return (e386_get_mem32(c, e386_get_ss(c), sp));
	}
	else
	{
		/* 16-bit stack address size */
		return (e86_get_mem16(c, e86_get_ss(c), sp));
	}
}

/* POP WORD or DWORD register (Real Mode) */
unsigned short g386_real_mode_pop32(e8086_t *c)
{
	unsigned long esp;
	esp = e386_get_esp(c);
	e386_set_esp(c, esp + 4);
	if (c->e386_address_size_prefix == 1)
	{
		/* 32-bit stack address size */
		return (e386_get_mem32(c, e386_get_ss(c), esp));
	}
	else
	{
		/* 16-bit stack address size */
		return (e86_get_mem16(c, e86_get_ss(c), esp));
	}
}

/* POP WORD or DWORD register (Virtual 8086 Mode) */
unsigned short g386_virtual_8086_mode_pop32(e8086_t *c)
{
	unsigned long esp;
	esp = e386_get_esp(c);
	e386_set_esp(c, esp + 4);
	if (c->e386_address_size_prefix == 1)
	{
		/* 32-bit stack address size */
		return (e386_get_mem32(c, e386_get_ss(c), esp));
	}
	else
	{
		/* 16-bit stack address size */
		return (e86_get_mem16(c, e86_get_ss(c), esp));
	}
}

/* POP WORD or DWORD register (Protected Mode) */
unsigned short g386_protected_mode_pop_word_or_dword_register32(e8086_t *c)
{
	unsigned long esp;
	esp = e386_get_esp(c);
	e386_set_esp(c, esp + 4);
	if (c->e386_address_size_prefix == 1)
	{
		/* 32-bit stack address size */
		return (e386_get_mem32(c, e386_get_ss(c), esp));
	}
	else
	{
		/* 16-bit stack address size */
		return (e86_get_mem16(c, e86_get_ss(c), esp));
	}
}

/* POP WORD or DWORD register (Protected Mode) */
unsigned short g386_protected_mode_pop_ds(e8086_t *c)
{
	e86_set_ds(c, g386_protected_mode_pop_word_or_dword_register(c));
	/* unsigned short sp;
	sp = e86_get_sp(c);
	e86_set_sp(c, sp + 2);
	if (c->e386_address_size_prefix == 1)
	{
	/* 32-bit stack address size */
	/* return (e386_get_mem32(c, e386_get_ss(c), sp));
	}
	else
	{
	/* 16-bit stack address size */
	/* return (e86_get_mem16(c, e86_get_ss(c), sp));
	} */
}

/* POP WORD or DWORD register (Protected Mode) */
unsigned long g386_protected_mode_pop_ds32(e8086_t *c)
{
	e386_set_ds(c, g386_protected_mode_pop_word_or_dword_register32(c));
	/* unsigned long esp;
	esp = e386_get_esp(c);
	e386_set_esp(c, esp + 4);
	if (c->e386_address_size_prefix == 1)
	{
	/* 32-bit stack address size */
	/* return (e386_get_mem32(c, e386_get_ss(c), esp));
	}
	else
	{
	/* 16-bit stack address size */
	/* return (e86_get_mem16(c, e86_get_ss(c), esp));
	} */
}

/* POP WORD or DWORD register (Protected Mode) */
unsigned short g386_protected_mode_pop_es(e8086_t *c)
{
	e86_set_es(c, g386_protected_mode_pop_word_or_dword_register(c));
	/* unsigned short sp;
	sp = e86_get_sp(c);
	e86_set_sp(c, sp + 2);
	if (c->e386_address_size_prefix == 1)
	{
	/* 32-bit stack address size */
	/* return (e386_get_mem32(c, e386_get_ss(c), sp));
	}
	else
	{
	/* 16-bit stack address size */
	/* return (e86_get_mem16(c, e86_get_ss(c), sp));
	} */
}

/* POP WORD or DWORD register (Protected Mode) */
unsigned long g386_protected_mode_pop_es32(e8086_t *c)
{
	e86_set_es(c, g386_protected_mode_pop_word_or_dword_register32(c));
	/* unsigned long esp;
	esp = e386_get_esp(c);
	e386_set_esp(c, esp + 4);
	if (c->e386_address_size_prefix == 1)
	{
	/* 32-bit stack address size */
	/* return (e386_get_mem32(c, e386_get_ss(c), esp));
	}
	else
	{
	/* 16-bit stack address size */
	/* return (e86_get_mem16(c, e86_get_ss(c), esp));
	} */
}

/* POP WORD or DWORD register (Protected Mode) */
unsigned short g386_protected_mode_pop_fs(e8086_t *c)
{
	e386_set_fs_16(c, g386_protected_mode_pop_word_or_dword_register(c));
	/* unsigned short sp;
	sp = e86_get_sp(c);
	e86_set_sp(c, sp + 2);
	if (c->e386_address_size_prefix == 1)
	{
	/* 32-bit stack address size */
	/* return (e386_get_mem32(c, e386_get_ss(c), sp));
	}
	else
	{
	/* 16-bit stack address size */
	/* return (e86_get_mem16(c, e86_get_ss(c), sp));
	} */
}

/* POP WORD or DWORD register (Protected Mode) */
unsigned long g386_protected_mode_pop_fs32(e8086_t *c)
{
	e386_set_fs(c, g386_protected_mode_pop_word_or_dword_register32(c));
	/* unsigned long esp;
	esp = e386_get_esp(c);
	e386_set_esp(c, esp + 4);
	if (c->e386_address_size_prefix == 1)
	{
	/* 32-bit stack address size */
	/* return (e386_get_mem32(c, e386_get_ss(c), esp));
	}
	else
	{
	/* 16-bit stack address size */
	/* return (e86_get_mem16(c, e86_get_ss(c), esp));
	} */
}

/* POP WORD or DWORD register (Protected Mode) */
unsigned short g386_protected_mode_pop_gs(e8086_t *c)
{
	e386_set_gs_16(c, g386_protected_mode_pop_word_or_dword_register(c));
	/* unsigned short sp;
	sp = e86_get_sp(c);
	e86_set_sp(c, sp + 2);
	if (c->e386_address_size_prefix == 1)
	{
	/* 32-bit stack address size */
	/* return (e386_get_mem32(c, e386_get_ss(c), sp));
	}
	else
	{
	/* 16-bit stack address size */
	/* return (e86_get_mem16(c, e86_get_ss(c), sp));
	} */
}

/* POP WORD or DWORD register (Protected Mode) */
unsigned long g386_protected_mode_pop_gs32(e8086_t *c)
{
	e386_set_gs(c, g386_protected_mode_pop_word_or_dword_register32(c));
	/* unsigned long esp;
	esp = e386_get_esp(c);
	e386_set_esp(c, esp + 4);
	if (c->e386_address_size_prefix == 1)
	{
	/* 32-bit stack address size */
	/* return (e386_get_mem32(c, e386_get_ss(c), esp));
	}
	else
	{
	/* 16-bit stack address size */
	/* return (e86_get_mem16(c, e86_get_ss(c), esp));
	} */
}

/* POP WORD or DWORD register (Protected Mode) */
unsigned short g386_protected_mode_pop_ss(e8086_t *c)
{
	e86_set_ss(c, g386_protected_mode_pop_word_or_dword_register(c));
	/* unsigned short sp;
	sp = e86_get_sp(c);
	e86_set_sp(c, sp + 2);
	if (c->e386_address_size_prefix == 1)
	{
	/* 32-bit stack address size */
	/* return (e386_get_mem32(c, e386_get_ss(c), sp));
	}
	else
	{
	/* 16-bit stack address size */
	/* return (e86_get_mem16(c, e86_get_ss(c), sp));
	} */
}

/* POP WORD or DWORD register (Protected Mode) */
unsigned long g386_protected_mode_pop_ss32(e8086_t *c)
{
	e386_set_ss(c, g386_protected_mode_pop_word_or_dword_register32(c));
	/* unsigned long esp;
	esp = e386_get_esp(c);
	e386_set_esp(c, esp + 4);
	if (c->e386_address_size_prefix == 1)
	{
	/* 32-bit stack address size */
	/* return (e386_get_mem32(c, e386_get_ss(c), esp));
	}
	else
	{
	/* 16-bit stack address size */
	/* return (e86_get_mem16(c, e86_get_ss(c), esp));
	} */
}

/* Handle an undefined opcode (Real Mode) */
static
unsigned g386_real_mode_op_ud(e8086_t *c)
{
	e86_set_clk(c, 1);
	c->e386_address_size_prefix = 0;
	c->e386_operand_size_prefix = 0;
	return (e86_undefined(c));
}

/* Handle an undefined opcode (Virtual 8086 Mode) */
static
unsigned g386_virtual_8086_mode_op_ud(e8086_t *c)
{
	e86_set_clk(c, 1);
	c->e386_address_size_prefix = 0;
	c->e386_operand_size_prefix = 0;
	return (e86_undefined(c));
}

/* Handle an undefined opcode (Protected Mode) */
static
unsigned g386_protected_mode_op_ud(e8086_t *c)
{
	e86_set_clk(c, 1);
	c->e386_address_size_prefix = 1;
	c->e386_operand_size_prefix = 1;
	return (e86_undefined(c));
}

/* Divide By Zero Error (Real Mode) */
static
unsigned g386_real_mode_op_divide_error(e8086_t *c)
{
	e86_set_clk_ea(c, 16, 20);
	e86_set_ip(c, e86_get_ip(c) + c->ea.cnt + 1);
	e86_trap(c, 0);
	c->e386_address_size_prefix = 0;
	c->e386_operand_size_prefix = 0;
	return (0);
}

/* Divide By Zero Error (Virtual 8086 Mode) */
static
unsigned g386_virtual_8086_mode_op_divide_error(e8086_t *c)
{
	e86_set_clk_ea(c, 16, 20);
	e86_set_ip(c, e86_get_ip(c) + c->ea.cnt + 1);
	e86_trap(c, 0);
	c->e386_address_size_prefix = 0;
	c->e386_operand_size_prefix = 0;
	return (0);
}

/* Divide By Zero Error (Protected Mode) */
static
unsigned g386_protected_mode_op_divide_error(e8086_t *c)
{
	e86_set_clk_ea(c, 16, 20);
	e86_set_ip(c, e86_get_ip(c) + c->ea.cnt + 1);
	e86_trap(c, 0);
	c->e386_address_size_prefix = 1;
	c->e386_operand_size_prefix = 1;
	return (0);
}

 /* OP 60: PUSHA (Real Mode) */
static
unsigned e386_real_mode_op_60(e8086_t *c)
{
	unsigned short tmp;
	tmp = e86_get_sp(c);
	g386_real_mode_push(c, e86_get_ax(c));
	g386_real_mode_push(c, e86_get_cx(c));
	g386_real_mode_push(c, e86_get_dx(c));
	g386_real_mode_push(c, e86_get_bx(c));
	g386_real_mode_push(c, tmp);
	g386_real_mode_push(c, e86_get_bp(c));
	g386_real_mode_push(c, e86_get_si(c));
	g386_real_mode_push(c, e86_get_di(c));
	if (c->memory_clock_cycle_disable == 1)
	{
		/* External memory cycle */
		unsigned char memclk;
		memclk = c->memory_clock_cycle_count;
		e86_set_clk(c, memclk);
	}
	else
	{
		/* CPU memory cycle */
		e86_set_clk(c, 19);
	}
	c->e386_address_size_prefix = 0;
	c->e386_operand_size_prefix = 0;
}

/* OP 60: PUSHA (Virtual 8086 Mode) */
static
unsigned e386_virtual_8086_mode_op_60(e8086_t *c)
{
	unsigned short tmp;
	tmp = e86_get_sp(c);
	g386_virtual_8086_mode_push(c, e86_get_ax(c));
	g386_virtual_8086_mode_push(c, e86_get_cx(c));
	g386_virtual_8086_mode_push(c, e86_get_dx(c));
	g386_virtual_8086_mode_push(c, e86_get_bx(c));
	g386_virtual_8086_mode_push(c, tmp);
	g386_virtual_8086_mode_push(c, e86_get_bp(c));
	g386_virtual_8086_mode_push(c, e86_get_si(c));
	g386_virtual_8086_mode_push(c, e86_get_di(c));
	if (c->memory_clock_cycle_disable == 1)
	{
		/* External memory cycle */
		unsigned char memclk;
		memclk = c->memory_clock_cycle_count;
		e86_set_clk(c, memclk);
	}
	else
	{
		/* CPU memory cycle */
		e86_set_clk(c, 19);
	}
	c->e386_address_size_prefix = 0;
	c->e386_operand_size_prefix = 0;
}

/* OP 60: PUSHA (Protected Mode) */
static
unsigned e386_protected_mode_op_60(e8086_t *c)
{
	unsigned short tmp;
	tmp = e86_get_sp(c);
	g386_protected_mode_push(c, e86_get_ax(c));
	g386_protected_mode_push(c, e86_get_cx(c));
	g386_protected_mode_push(c, e86_get_dx(c));
	g386_protected_mode_push(c, e86_get_bx(c));
	g386_protected_mode_push(c, tmp);
	g386_protected_mode_push(c, e86_get_bp(c));
	g386_protected_mode_push(c, e86_get_si(c));
	g386_protected_mode_push(c, e86_get_di(c));
	if (c->memory_clock_cycle_disable == 1)
	{
		/* External memory cycle */
		unsigned char memclk;
		memclk = c->memory_clock_cycle_count;
		e86_set_clk(c, memclk);
	}
	else
	{
		/* CPU memory cycle */
		e86_set_clk(c, 19);
	}
	c->e386_address_size_prefix = 1;
	c->e386_operand_size_prefix = 1;
}

/* OP 60: PUSHA */
static
unsigned op_60 (e8086_t *c)
{
	if (c->cpumodel >= 0x80386) {
		/* Intel 80386 CPU or higher */
		e286_get_machine_status_word(c);
		e386_get_flags(c);
		if ((e286_get_pe(c) == 0) && (e386_get_vm(c) == 0))
		{
			/* Real Mode */
			e386_real_mode_op_60(c);
			return (1);
		}
		else if (e386_get_vm(c) == 1)
		{
			/* Virtual 8086 Mode */
			e386_virtual_8086_mode_op_60(c);
			return (1);
		}
		else
		{
			/* Protected Mode */
			e386_protected_mode_op_60(c);
			return (1);
		}
	}
	else {
		/* Intel 80186 CPU or  lower */
		unsigned short tmp;
		tmp = e86_get_sp(c);
		e86_push(c, e86_get_ax(c));
		e86_push(c, e86_get_cx(c));
		e86_push(c, e86_get_dx(c));
		e86_push(c, e86_get_bx(c));
		e86_push(c, tmp);
		e86_push(c, e86_get_bp(c));
		e86_push(c, e86_get_si(c));
		e86_push(c, e86_get_di(c));
		if (c->memory_clock_cycle_disable == 1)
		{
			/* External memory cycle */
			unsigned char memclk;
			memclk = c->memory_clock_cycle_count;
			e86_set_clk(c, memclk);
		}
		else
		{
			/* CPU memory cycle */
			e86_set_clk(c, 19);
		}
		return (1);
	}
}

/* OP 61: POPA (Real Mode) */
static
unsigned e386_real_mode_op_61(e8086_t *c)
{
	e86_set_di(c, g386_real_mode_pop(c));
	e86_set_si(c, g386_real_mode_pop(c));
	e86_set_bp(c, g386_real_mode_pop(c));
	g386_real_mode_pop(c);
	e86_set_bx(c, g386_real_mode_pop(c));
	e86_set_dx(c, g386_real_mode_pop(c));
	e86_set_cx(c, g386_real_mode_pop(c));
	e86_set_ax(c, g386_real_mode_pop(c));
	if (c->memory_clock_cycle_disable == 1)
	{
		/* External memory cycle */
		unsigned char memclk;
		memclk = c->memory_clock_cycle_count;
		e86_set_clk(c, memclk);
	}
	else
	{
		/* CPU memory cycle */
		e86_set_clk(c, 19);
	}
	c->e386_address_size_prefix = 0;
	c->e386_operand_size_prefix = 0;
}

/* OP 61: POPA (Virtual 8086 Mode) */
static
unsigned e386_virtual_8086_mode_op_61(e8086_t *c)
{
	e86_set_di(c, g386_virtual_8086_mode_pop(c));
	e86_set_si(c, g386_virtual_8086_mode_pop(c));
	e86_set_bp(c, g386_virtual_8086_mode_pop(c));
	g386_virtual_8086_mode_pop(c);
	e86_set_bx(c, g386_virtual_8086_mode_pop(c));
	e86_set_dx(c, g386_virtual_8086_mode_pop(c));
	e86_set_cx(c, g386_virtual_8086_mode_pop(c));
	e86_set_ax(c, g386_virtual_8086_mode_pop(c));
	if (c->memory_clock_cycle_disable == 1)
	{
		/* External memory cycle */
		unsigned char memclk;
		memclk = c->memory_clock_cycle_count;
		e86_set_clk(c, memclk);
	}
	else
	{
		/* CPU memory cycle */
		e86_set_clk(c, 19);
	}
	c->e386_address_size_prefix = 0;
	c->e386_operand_size_prefix = 0;
}

/* OP 61: POPA (Protected Mode) */
static
unsigned e386_protected_mode_op_61(e8086_t *c)
{
	e86_set_di(c, g386_protected_mode_pop_word_or_dword_register(c));
	e86_set_si(c, g386_protected_mode_pop_word_or_dword_register(c));
	e86_set_bp(c, g386_protected_mode_pop_word_or_dword_register(c));
	g386_protected_mode_pop_word_or_dword_register(c);
	e86_set_bx(c, g386_protected_mode_pop_word_or_dword_register(c));
	e86_set_dx(c, g386_protected_mode_pop_word_or_dword_register(c));
	e86_set_cx(c, g386_protected_mode_pop_word_or_dword_register(c));
	e86_set_ax(c, g386_protected_mode_pop_word_or_dword_register(c));
	if (c->memory_clock_cycle_disable == 1)
	{
		/* External memory cycle */
		unsigned char memclk;
		memclk = c->memory_clock_cycle_count;
		e86_set_clk(c, memclk);
	}
	else
	{
		/* CPU memory cycle */
		e86_set_clk(c, 19);
	}
	c->e386_address_size_prefix = 1;
	c->e386_operand_size_prefix = 1;
}

/* OP 61: POPA */
static
unsigned op_61 (e8086_t *c)
{
	if (c->cpumodel >= 0x80386) {
		/* Intel 80386 CPU or higher */
		e286_get_machine_status_word(c);
		e386_get_flags(c);
		if ((e286_get_pe(c) == 0) && (e386_get_vm(c) == 0))
		{
			/* Real Mode */
			e386_real_mode_op_61(c);
			return (1);
		}
		else if (e386_get_vm(c) == 1)
		{
			/* Virtual 8086 Mode */
			e386_virtual_8086_mode_op_61(c);
			return (1);
		}
		else
		{
			/* Protected Mode */
			e386_protected_mode_op_61(c);
			return (1);
		}
	}
	else {
		/* Intel 80186 CPU or  lower */
		e86_set_di(c, e86_pop(c));
		e86_set_si(c, e86_pop(c));
		e86_set_bp(c, e86_pop(c));
		e86_pop(c);
		e86_set_bx(c, e86_pop(c));
		e86_set_dx(c, e86_pop(c));
		e86_set_cx(c, e86_pop(c));
		e86_set_ax(c, e86_pop(c));
		if (c->memory_clock_cycle_disable == 1)
		{
			/* External memory cycle */
			unsigned char memclk;
			memclk = c->memory_clock_cycle_count;
			e86_set_clk(c, memclk);
		}
		else
		{
			/* CPU memory cycle */
			e86_set_clk(c, 19);
		}
		return (1);
	}
}

/* OP 62: BOUND reg16, r/m16 (Real Mode) */
static
unsigned e386_real_mode_op_62(e8086_t *c)
{
	long           s1, lo, hi;
	unsigned short seg, ofs;
	e86_get_ea_ptr(c, c->pq + 1);
	s1 = e86_get_reg16(c, (c->pq[1] >> 3) & 7);
	if (c->ea.is_mem == 0) {
		fprintf(stderr, "BOUND EA is register\n");
		c->e386_address_size_prefix = 0;
		c->e386_operand_size_prefix = 0;
		return (c->ea.cnt + 1);
	}
	seg = c->ea.seg;
	ofs = c->ea.ofs;
	lo = e86_get_mem16(c, seg, ofs);
	hi = e86_get_mem16(c, seg, ofs + 2);
	s1 = (s1 > 0x7fff) ? (s1 - 0x10000) : s1;
	lo = (lo > 0x7fff) ? (lo - 0x10000) : lo;
	hi = (hi > 0x7fff) ? (hi - 0x10000) : hi;
	if ((s1 < lo) || (s1 >(hi + 2))) {
		e86_trap(c, 5);
		c->e386_address_size_prefix = 0;
		c->e386_operand_size_prefix = 0;
		return (0);
	}
	if (c->memory_clock_cycle_disable == 1)
	{
		/* External memory cycle */
		unsigned char memclk;
		memclk = c->memory_clock_cycle_count;
		e86_set_clk(c, memclk);
	}
	else
	{
		/* CPU memory cycle */
		e86_set_clk(c, (33 + 35) / 2);
	}
	c->e386_address_size_prefix = 0;
	c->e386_operand_size_prefix = 0;
}

/* OP 62: BOUND reg16, r/m16 (Virtual 8086 Mode) */
static
unsigned e386_virtual_8086_mode_op_62(e8086_t *c)
{
	long           s1, lo, hi;
	unsigned short seg, ofs;
	e86_get_ea_ptr(c, c->pq + 1);
	s1 = e86_get_reg16(c, (c->pq[1] >> 3) & 7);
	if (c->ea.is_mem == 0) {
		fprintf(stderr, "BOUND EA is register\n");
		c->e386_address_size_prefix = 0;
		c->e386_operand_size_prefix = 0;
		return (c->ea.cnt + 1);
	}
	seg = c->ea.seg;
	ofs = c->ea.ofs;
	lo = e86_get_mem16(c, seg, ofs);
	hi = e86_get_mem16(c, seg, ofs + 2);
	s1 = (s1 > 0x7fff) ? (s1 - 0x10000) : s1;
	lo = (lo > 0x7fff) ? (lo - 0x10000) : lo;
	hi = (hi > 0x7fff) ? (hi - 0x10000) : hi;
	if ((s1 < lo) || (s1 >(hi + 2))) {
		e86_trap(c, 5);
		c->e386_address_size_prefix = 0;
		c->e386_operand_size_prefix = 0;
		return (0);
	}
	if (c->memory_clock_cycle_disable == 1)
	{
		/* External memory cycle */
		unsigned char memclk;
		memclk = c->memory_clock_cycle_count;
		e86_set_clk(c, memclk);
	}
	else
	{
		/* CPU memory cycle */
		e86_set_clk(c, (33 + 35) / 2);
	}
	c->e386_address_size_prefix = 0;
	c->e386_operand_size_prefix = 0;
}

/* OP 62: BOUND reg16, r/m16 (Protected Mode) */
static
unsigned e386_protected_mode_op_62(e8086_t *c)
{
	long           s1, lo, hi;
	unsigned short seg, ofs;
	e86_get_ea_ptr(c, c->pq + 1);
	s1 = e86_get_reg16(c, (c->pq[1] >> 3) & 7);
	if (c->ea.is_mem == 0) {
		fprintf(stderr, "BOUND EA is register\n");
		c->e386_address_size_prefix = 1;
		c->e386_operand_size_prefix = 1;
		return (c->ea.cnt + 1);
	}
	seg = c->ea.seg;
	ofs = c->ea.ofs;
	lo = e86_get_mem16(c, seg, ofs);
	hi = e86_get_mem16(c, seg, ofs + 2);
	s1 = (s1 > 0x7fff) ? (s1 - 0x10000) : s1;
	lo = (lo > 0x7fff) ? (lo - 0x10000) : lo;
	hi = (hi > 0x7fff) ? (hi - 0x10000) : hi;
	if ((s1 < lo) || (s1 >(hi + 2))) {
		e86_trap(c, 5);
		c->e386_address_size_prefix = 1;
		c->e386_operand_size_prefix = 1;
		return (0);
	}
	if (c->memory_clock_cycle_disable == 1)
	{
		/* External memory cycle */
		unsigned char memclk;
		memclk = c->memory_clock_cycle_count;
		e86_set_clk(c, memclk);
	}
	else
	{
		/* CPU memory cycle */
		e86_set_clk(c, (33 + 35) / 2);
	}
	c->e386_address_size_prefix = 0;
	c->e386_operand_size_prefix = 0;
}

/* OP 62: BOUND reg16, r/m16 */
static
unsigned op_62 (e8086_t *c)
{
	if (c->cpumodel >= 0x80386) {
		/* Intel 80386 CPU or higher */
		e286_get_machine_status_word(c);
		e386_get_flags(c);
		if ((e286_get_pe(c) == 0) && (e386_get_vm(c) == 0))
		{
			/* Real Mode */
			e386_real_mode_op_62(c);
			return (1);
		}
		else if (e386_get_vm(c) == 1)
		{
			/* Virtual 8086 Mode */
			e386_virtual_8086_mode_op_62(c);
			return (1);
		}
		else
		{
			/* Protected Mode */
			e386_protected_mode_op_62(c);
			return (c->ea.cnt + 1);
		}
	}
	else {
		/* Intel 80186 CPU or  lower */
		long           s1, lo, hi;
		unsigned short seg, ofs;
		e86_get_ea_ptr(c, c->pq + 1);
		s1 = e86_get_reg16(c, (c->pq[1] >> 3) & 7);
		if (c->ea.is_mem == 0) {
			fprintf(stderr, "BOUND EA is register\n");
			return (c->ea.cnt + 1);
		}
		seg = c->ea.seg;
		ofs = c->ea.ofs;
		lo = e86_get_mem16(c, seg, ofs);
		hi = e86_get_mem16(c, seg, ofs + 2);
		s1 = (s1 > 0x7fff) ? (s1 - 0x10000) : s1;
		lo = (lo > 0x7fff) ? (lo - 0x10000) : lo;
		hi = (hi > 0x7fff) ? (hi - 0x10000) : hi;
		if ((s1 < lo) || (s1 >(hi + 2))) {
			e86_trap(c, 5);
			return (0);
		}
		if (c->memory_clock_cycle_disable == 1)
		{
			/* External memory cycle */
			unsigned char memclk;
			memclk = c->memory_clock_cycle_count;
			e86_set_clk(c, memclk);
		}
		else
		{
			/* CPU memory cycle */
			e86_set_clk(c, (33 + 35) / 2);
		}
		return (c->ea.cnt + 1);
	}
}

/* OP 68: PUSH imm16 or PUSH imm32 (Real Mode) */
static
unsigned e386_real_mode_op_68(e8086_t *c)
{
	if (c->e386_operand_size_prefix == 1)
	{
		/* 32-bit register operand specified */
		g386_real_mode_push32(c, e86_mk_uint32(c->pq[1], c->pq[2]));
		if (c->memory_clock_cycle_disable == 1)
		{
			/* External memory cycle */
			unsigned char memclk;
			memclk = c->memory_clock_cycle_count;
			e86_set_clk(c, memclk);
		}
		else
		{
			/* CPU memory cycle */
			e86_set_clk(c, 2);
		}
	}
	else
	{
		/* 16-bit register operand specified */
		g386_real_mode_push(c, e86_mk_uint16(c->pq[1], c->pq[2]));
		if (c->memory_clock_cycle_disable == 1)
		{
			/* External memory cycle */
			unsigned char memclk;
			memclk = c->memory_clock_cycle_count;
			e86_set_clk(c, memclk);
		}
		else
		{
			/* CPU memory cycle */
			e86_set_clk(c, 2);
		}
	}
	c->e386_address_size_prefix = 0;
	c->e386_operand_size_prefix = 0;
	/* return (3); */
}

/* OP 68: PUSH imm16 or PUSH imm32 (Virtual 8086 Mode) */
static
unsigned e386_virtual_8086_mode_op_68(e8086_t *c)
{
	if (c->e386_operand_size_prefix == 1)
	{
		/* 32-bit register operand specified */
		g386_real_mode_push32(c, e86_mk_uint32(c->pq[1], c->pq[2]));
		if (c->memory_clock_cycle_disable == 1)
		{
			/* External memory cycle */
			unsigned char memclk;
			memclk = c->memory_clock_cycle_count;
			e86_set_clk(c, memclk);
		}
		else
		{
			/* CPU memory cycle */
			e86_set_clk(c, 2);
		}
	}
	else
	{
		/* 16-bit register operand specified */
		g386_real_mode_push(c, e86_mk_uint16(c->pq[1], c->pq[2]));
		if (c->memory_clock_cycle_disable == 1)
		{
			/* External memory cycle */
			unsigned char memclk;
			memclk = c->memory_clock_cycle_count;
			e86_set_clk(c, memclk);
		}
		else
		{
			/* CPU memory cycle */
			e86_set_clk(c, 2);
		}
	}
	c->e386_address_size_prefix = 0;
	c->e386_operand_size_prefix = 0;
	/* return (3); */
}

/* OP 68: PUSH imm16 or PUSH imm32 (Protected Mode) */
static
unsigned e386_protected_mode_op_68(e8086_t *c)
{
	if (c->e386_operand_size_prefix == 1)
	{
		/* 32-bit register operand specified */
		g386_real_mode_push32(c, e86_mk_uint32(c->pq[1], c->pq[2]));
		if (c->memory_clock_cycle_disable == 1)
		{
			/* External memory cycle */
			unsigned char memclk;
			memclk = c->memory_clock_cycle_count;
			e86_set_clk(c, memclk);
		}
		else
		{
			/* CPU memory cycle */
			e86_set_clk(c, 2);
		}
	}
	else
	{
		/* 16-bit register operand specified */
		g386_real_mode_push(c, e86_mk_uint16(c->pq[1], c->pq[2]));
		if (c->memory_clock_cycle_disable == 1)
		{
			/* External memory cycle */
			unsigned char memclk;
			memclk = c->memory_clock_cycle_count;
			e86_set_clk(c, memclk);
		}
		else
		{
			/* CPU memory cycle */
			e86_set_clk(c, 2);
		}
	}
	c->e386_address_size_prefix = 1;
	c->e386_operand_size_prefix = 1;
	/* return (3); */
}

/* OP 68: PUSH imm16 */
static
unsigned op_68 (e8086_t *c)
{
	if (c->cpumodel >= 0x80386) {
		/* Intel 80386 CPU or higher */
		e286_get_machine_status_word(c);
		e386_get_flags(c);
		if ((e286_get_pe(c) == 0) && (e386_get_vm(c) == 0))
		{
			/* Real Mode */
			e386_real_mode_op_68(c);
			return (3);
		}
		else if (e386_get_vm(c) == 1)
		{
			/* Virtual 8086 Mode */
			e386_virtual_8086_mode_op_68(c);
			return (3);
		}
		else
		{
			/* Protected Mode */
			e386_protected_mode_op_68(c);
			return (3);
		}
	}
	else {
		/* Intel 80186 CPU or  lower */
		e86_push(c, e86_mk_uint16(c->pq[1], c->pq[2]));
		if (c->memory_clock_cycle_disable == 1)
		{
			/* External memory cycle */
			unsigned char memclk;
			memclk = c->memory_clock_cycle_count;
			e86_set_clk(c, memclk);
		}
		else
		{
			/* CPU memory cycle */
			e86_set_clk(c, 3);
		}
		return (3);
	}
}

/* OP 69: IMUL r16, r/m16, imm16 (Real Mode) */
static
unsigned e386_real_mode_op_69(e8086_t *c)
{
	unsigned long  s1, s2, d;
	e86_get_ea_ptr(c, c->pq + 1);
	s1 = e86_get_ea16(c);
	s2 = e86_mk_uint16(c->pq[c->ea.cnt + 1], c->pq[c->ea.cnt + 2]);
	s1 = (s1 & 0x8000) ? (s1 | 0xffff0000) : s1;
	s2 = (s2 & 0x8000) ? (s2 | 0xffff0000) : s2;
	d = (s1 * s2) & 0xffffffff;
	e86_set_reg16(c, (c->pq[1] >> 3) & 7, d & 0xffff);
	d &= 0xffff8000;
	e86_set_f(c, E86_FLG_CF | E86_FLG_OF, (d != 0xffff8000) && (d != 0x00000000));
	e86_set_f(c, E86_FLG_ZF, d == 0);
	if (c->memory_clock_cycle_disable == 1)
	{
		/* External memory cycle */
		unsigned char memclk;
		memclk = c->memory_clock_cycle_count;
		e86_set_clk(c, memclk);
	}
	else
	{
		/* CPU memory cycle */
		e86_set_clk_ea(c, (22 + 25) / 2, (29 + 32) / 2);
	}
	c->e386_address_size_prefix = 0;
	c->e386_operand_size_prefix = 0;
}

/* OP 69: IMUL r16, r/m16, imm16 (Virtual 8086 Mode) */
static
unsigned e386_virtual_8086_mode_op_69(e8086_t *c)
{
	unsigned long  s1, s2, d;
	e86_get_ea_ptr(c, c->pq + 1);
	s1 = e86_get_ea16(c);
	s2 = e86_mk_uint16(c->pq[c->ea.cnt + 1], c->pq[c->ea.cnt + 2]);
	s1 = (s1 & 0x8000) ? (s1 | 0xffff0000) : s1;
	s2 = (s2 & 0x8000) ? (s2 | 0xffff0000) : s2;
	d = (s1 * s2) & 0xffffffff;
	e86_set_reg16(c, (c->pq[1] >> 3) & 7, d & 0xffff);
	d &= 0xffff8000;
	e86_set_f(c, E86_FLG_CF | E86_FLG_OF, (d != 0xffff8000) && (d != 0x00000000));
	e86_set_f(c, E86_FLG_ZF, d == 0);
	if (c->memory_clock_cycle_disable == 1)
	{
		/* External memory cycle */
		unsigned char memclk;
		memclk = c->memory_clock_cycle_count;
		e86_set_clk(c, memclk);
	}
	else
	{
		/* CPU memory cycle */
		e86_set_clk_ea(c, (22 + 25) / 2, (29 + 32) / 2);
	}
	c->e386_address_size_prefix = 0;
	c->e386_operand_size_prefix = 0;
}

/* OP 69: IMUL r16, r/m16, imm16 (Protected Mode) */
static
unsigned e386_protected_mode_op_69(e8086_t *c)
{
	unsigned long  s1, s2, d;
	e86_get_ea_ptr(c, c->pq + 1);
	s1 = e86_get_ea16(c);
	s2 = e86_mk_uint16(c->pq[c->ea.cnt + 1], c->pq[c->ea.cnt + 2]);
	s1 = (s1 & 0x8000) ? (s1 | 0xffff0000) : s1;
	s2 = (s2 & 0x8000) ? (s2 | 0xffff0000) : s2;
	d = (s1 * s2) & 0xffffffff;
	e86_set_reg16(c, (c->pq[1] >> 3) & 7, d & 0xffff);
	d &= 0xffff8000;
	e86_set_f(c, E86_FLG_CF | E86_FLG_OF, (d != 0xffff8000) && (d != 0x00000000));
	e86_set_f(c, E86_FLG_ZF, d == 0);
	if (c->memory_clock_cycle_disable == 1)
	{
		/* External memory cycle */
		unsigned char memclk;
		memclk = c->memory_clock_cycle_count;
		e86_set_clk(c, memclk);
	}
	else
	{
		/* CPU memory cycle */
		e86_set_clk_ea(c, (22 + 25) / 2, (29 + 32) / 2);
	}
	c->e386_address_size_prefix = 1;
	c->e386_operand_size_prefix = 1;
}

/* OP 69: IMUL r16, r/m16, imm16 */
static
unsigned op_69 (e8086_t *c)
{
	if (c->cpumodel >= 0x80386) {
		/* Intel 80386 CPU or higher */
		e286_get_machine_status_word(c);
		e386_get_flags(c);
		if ((e286_get_pe(c) == 0) && (e386_get_vm(c) == 0))
		{
			/* Real Mode */
			e386_real_mode_op_69(c);
			return (c->ea.cnt + 3);
		}
		else if (e386_get_vm(c) == 1)
		{
			/* Virtual 8086 Mode */
			e386_virtual_8086_mode_op_69(c);
			return (c->ea.cnt + 3);
		}
		else
		{
			/* Protected Mode */
			e386_protected_mode_op_69(c);
			return (c->ea.cnt + 3);
		}
	}
	else {
		/* Intel 80186 CPU or  lower */
		unsigned long  s1, s2, d;
		e86_get_ea_ptr(c, c->pq + 1);
		s1 = e86_get_ea16(c);
		s2 = e86_mk_uint16(c->pq[c->ea.cnt + 1], c->pq[c->ea.cnt + 2]);
		s1 = (s1 & 0x8000) ? (s1 | 0xffff0000) : s1;
		s2 = (s2 & 0x8000) ? (s2 | 0xffff0000) : s2;
		d = (s1 * s2) & 0xffffffff;
		e86_set_reg16(c, (c->pq[1] >> 3) & 7, d & 0xffff);
		d &= 0xffff8000;
		e86_set_f(c, E86_FLG_CF | E86_FLG_OF, (d != 0xffff8000) && (d != 0x00000000));
		e86_set_f(c, E86_FLG_ZF, d == 0);
		if (c->memory_clock_cycle_disable == 1)
		{
			/* External memory cycle */
			unsigned char memclk;
			memclk = c->memory_clock_cycle_count;
			e86_set_clk(c, memclk);
		}
		else
		{
			/* CPU memory cycle */
			e86_set_clk_ea(c, (22 + 25) / 2, (29 + 32) / 2);
		}
		return (c->ea.cnt + 3);
	}
}

/* OP 6A: PUSH imm8 (Real Mode) */
static
unsigned e386_real_mode_op_6a(e8086_t *c)
{
	g386_real_mode_push(c, e86_mk_sint16(c->pq[1]));
	if (c->memory_clock_cycle_disable == 1)
	{
		/* External memory cycle */
		unsigned char memclk;
		memclk = c->memory_clock_cycle_count;
		e86_set_clk(c, memclk);
	}
	else
	{
		/* CPU memory cycle */
		e86_set_clk(c, 2);
	}
	c->e386_address_size_prefix = 0;
	c->e386_operand_size_prefix = 0;
}

/* OP 6A: PUSH imm8 (Virtual 8086 Mode) */
static
unsigned e386_virtual_8086_mode_op_6a(e8086_t *c)
{
	g386_virtual_8086_mode_push(c, e86_mk_sint16(c->pq[1]));
	if (c->memory_clock_cycle_disable == 1)
	{
		/* External memory cycle */
		unsigned char memclk;
		memclk = c->memory_clock_cycle_count;
		e86_set_clk(c, memclk);
	}
	else
	{
		/* CPU memory cycle */
		e86_set_clk(c, 2);
	}
	c->e386_address_size_prefix = 0;
	c->e386_operand_size_prefix = 0;
}

/* OP 6A: PUSH imm8 (Protected Mode) */
static
unsigned e386_protected_mode_op_6a(e8086_t *c)
{
	g386_protected_mode_push(c, e86_mk_sint16(c->pq[1]));
	if (c->memory_clock_cycle_disable == 1)
	{
		/* External memory cycle */
		unsigned char memclk;
		memclk = c->memory_clock_cycle_count;
		e86_set_clk(c, memclk);
	}
	else
	{
		/* CPU memory cycle */
		e86_set_clk(c, 2);
	}
	c->e386_address_size_prefix = 1;
	c->e386_operand_size_prefix = 1;
}

/* OP 6A: PUSH imm8 */
static
unsigned op_6a (e8086_t *c)
{
	if (c->cpumodel >= 0x80386) {
		/* Intel 80386 CPU or higher */
		e286_get_machine_status_word(c);
		e386_get_flags(c);
		if ((e286_get_pe(c) == 0) && (e386_get_vm(c) == 0))
		{
			/* Real Mode */
			e386_real_mode_op_6a(c);
			return (2);
		}
		else if (e386_get_vm(c) == 1)
		{
			/* Virtual 8086 Mode */
			e386_virtual_8086_mode_op_6a(c);
			return (2);
		}
		else
		{
			/* Protected Mode */
			e386_protected_mode_op_6a(c);
			return (2);
		}
	}
	else {
		/* Intel 80186 CPU or  lower */
		e86_push(c, e86_mk_sint16(c->pq[1]));
		if (c->memory_clock_cycle_disable == 1)
		{
			/* External memory cycle */
			unsigned char memclk;
			memclk = c->memory_clock_cycle_count;
			e86_set_clk(c, memclk);
		}
		else
		{
			/* CPU memory cycle */
			e86_set_clk(c, 3);
		}
		return (2);
	}
}

/* OP 6B: IMUL r16, r/m16, imm8 (Real Mode) */
static
unsigned e386_real_mode_op_6b(e8086_t *c)
{
	unsigned long  s1, s2, d;
	e86_get_ea_ptr(c, c->pq + 1);
	s1 = e86_get_ea16(c);
	s2 = e86_mk_sint16(c->pq[c->ea.cnt + 1]);
	s1 = (s1 & 0x8000) ? (s1 | 0xffff0000) : s1;
	s2 = (s2 & 0x8000) ? (s2 | 0xffff0000) : s2;
	d = (s1 * s2) & 0xffffffff;
	e86_set_reg16(c, (c->pq[1] >> 3) & 7, d & 0xffff);
	d &= 0xffff8000;
	e86_set_f(c, E86_FLG_CF | E86_FLG_OF, (d != 0xffff8000) && (d != 0x00000000));
	e86_set_f(c, E86_FLG_ZF, d == 0);
	if (c->memory_clock_cycle_disable == 1)
	{
		/* External memory cycle */
		unsigned char memclk;
		memclk = c->memory_clock_cycle_count;
		e86_set_clk(c, memclk);
	}
	else
	{
		/* CPU memory cycle */
		e86_set_clk_ea(c, (22 + 25) / 2, (29 + 32) / 2);
	}
	c->e386_address_size_prefix = 0;
	c->e386_operand_size_prefix = 0;
	/* return (c->ea.cnt + 2); */
}

/* OP 6B: IMUL r16, r/m16, imm8 (Virtual 8086 Mode) */
static
unsigned e386_virtual_8086_mode_op_6b(e8086_t *c)
{
	unsigned long  s1, s2, d;
	e86_get_ea_ptr(c, c->pq + 1);
	s1 = e86_get_ea16(c);
	s2 = e86_mk_sint16(c->pq[c->ea.cnt + 1]);
	s1 = (s1 & 0x8000) ? (s1 | 0xffff0000) : s1;
	s2 = (s2 & 0x8000) ? (s2 | 0xffff0000) : s2;
	d = (s1 * s2) & 0xffffffff;
	e86_set_reg16(c, (c->pq[1] >> 3) & 7, d & 0xffff);
	d &= 0xffff8000;
	e86_set_f(c, E86_FLG_CF | E86_FLG_OF, (d != 0xffff8000) && (d != 0x00000000));
	e86_set_f(c, E86_FLG_ZF, d == 0);
	if (c->memory_clock_cycle_disable == 1)
	{
		/* External memory cycle */
		unsigned char memclk;
		memclk = c->memory_clock_cycle_count;
		e86_set_clk(c, memclk);
	}
	else
	{
		/* CPU memory cycle */
		e86_set_clk_ea(c, (22 + 25) / 2, (29 + 32) / 2);
	}
	c->e386_address_size_prefix = 0;
	c->e386_operand_size_prefix = 0;
	/* return (c->ea.cnt + 2); */
}

/* OP 6B: IMUL r16, r/m16, imm8 (Protected Mode) */
static
unsigned e386_protected_mode_op_6b(e8086_t *c)
{
	unsigned long  s1, s2, d;
	e86_get_ea_ptr(c, c->pq + 1);
	s1 = e86_get_ea16(c);
	s2 = e86_mk_sint16(c->pq[c->ea.cnt + 1]);
	s1 = (s1 & 0x8000) ? (s1 | 0xffff0000) : s1;
	s2 = (s2 & 0x8000) ? (s2 | 0xffff0000) : s2;
	d = (s1 * s2) & 0xffffffff;
	e86_set_reg16(c, (c->pq[1] >> 3) & 7, d & 0xffff);
	d &= 0xffff8000;
	e86_set_f(c, E86_FLG_CF | E86_FLG_OF, (d != 0xffff8000) && (d != 0x00000000));
	e86_set_f(c, E86_FLG_ZF, d == 0);
	if (c->memory_clock_cycle_disable == 1)
	{
		/* External memory cycle */
		unsigned char memclk;
		memclk = c->memory_clock_cycle_count;
		e86_set_clk(c, memclk);
	}
	else
	{
		/* CPU memory cycle */
		e86_set_clk_ea(c, (22 + 25) / 2, (29 + 32) / 2);
	}
	c->e386_address_size_prefix = 1;
	c->e386_operand_size_prefix = 1;
	/* return (c->ea.cnt + 2); */
}

/* OP 6B: IMUL r16, r/m16, imm8 */
static
unsigned op_6b (e8086_t *c)
{
	if (c->cpumodel >= 0x80386) {
		/* Intel 80386 CPU or higher */
		e286_get_machine_status_word(c);
		e386_get_flags(c);
		if ((e286_get_pe(c) == 0) && (e386_get_vm(c) == 0))
		{
			/* Real Mode */
			e386_real_mode_op_6b(c);
			return (c->ea.cnt + 2);
		}
		else if (e386_get_vm(c) == 1)
		{
			/* Virtual 8086 Mode */
			e386_virtual_8086_mode_op_6b(c);
			return (c->ea.cnt + 2);
		}
		else
		{
			/* Protected Mode */
			e386_protected_mode_op_6b(c);
			return (c->ea.cnt + 2);
		}
	}
	else {
		/* Intel 80186 CPU or  lower */
		unsigned long  s1, s2, d;
		e86_get_ea_ptr(c, c->pq + 1);
		s1 = e86_get_ea16(c);
		s2 = e86_mk_sint16(c->pq[c->ea.cnt + 1]);
		s1 = (s1 & 0x8000) ? (s1 | 0xffff0000) : s1;
		s2 = (s2 & 0x8000) ? (s2 | 0xffff0000) : s2;
		d = (s1 * s2) & 0xffffffff;
		e86_set_reg16(c, (c->pq[1] >> 3) & 7, d & 0xffff);
		d &= 0xffff8000;
		e86_set_f(c, E86_FLG_CF | E86_FLG_OF, (d != 0xffff8000) && (d != 0x00000000));
		e86_set_f(c, E86_FLG_ZF, d == 0);
		if (c->memory_clock_cycle_disable == 1)
		{
			/* External memory cycle */
			unsigned char memclk;
			memclk = c->memory_clock_cycle_count;
			e86_set_clk(c, memclk);
		}
		else
		{
			/* CPU memory cycle */
			e86_set_clk_ea(c, (22 + 25) / 2, (29 + 32) / 2);
		}
		return (c->ea.cnt + 2);
	}
}

/* OP 6C: INSB (Real Mode) */
static
unsigned e386_real_mode_op_6c(e8086_t *c)
{
	unsigned short inc;
	inc = e86_get_df(c) ? 0xffff : 0x0001;
	if (c->prefix & (E86_PREFIX_REP | E86_PREFIX_REPN)) {
		while (e86_get_cx(c) > 0) {
			e86_set_mem8(c, e86_get_es(c), e86_get_di(c), e86_get_prt8(c, e86_get_dx(c)));
			e86_set_di(c, e86_get_di(c) + inc);
			e86_set_cx(c, e86_get_cx(c) - 1);
			c->instructions += 1;
			if (c->memory_clock_cycle_disable == 1)
			{
				/* External memory cycle */
				unsigned char memclk;
				memclk = c->memory_clock_cycle_count;
				e86_set_clk(c, memclk);
			}
			else
			{
				/* CPU memory cycle */
				e86_set_clk(c, 8);
			}
		}
	}
	else {
		e86_set_mem8(c, e86_get_es(c), e86_get_di(c), e86_get_prt8(c, e86_get_dx(c)));
		e86_set_di(c, e86_get_di(c) + inc);
		if (c->memory_clock_cycle_disable == 1)
		{
			/* External memory cycle */
			unsigned char memclk;
			memclk = c->memory_clock_cycle_count;
			e86_set_clk(c, memclk);
		}
		else
		{
			/* CPU memory cycle */
			e86_set_clk(c, 8);
		}
	}
	c->e386_address_size_prefix = 0;
	c->e386_operand_size_prefix = 0;
}

/* OP 6C: INSB (Virtual 8086 Mode) */
static
unsigned e386_virtual_8086_mode_op_6c(e8086_t *c)
{
	unsigned short inc;
	inc = e86_get_df(c) ? 0xffff : 0x0001;
	if (c->prefix & (E86_PREFIX_REP | E86_PREFIX_REPN)) {
		while (e86_get_cx(c) > 0) {
			e86_set_mem8(c, e86_get_es(c), e86_get_di(c), e86_get_prt8(c, e86_get_dx(c)));
			e86_set_di(c, e86_get_di(c) + inc);
			e86_set_cx(c, e86_get_cx(c) - 1);
			c->instructions += 1;
			if (c->memory_clock_cycle_disable == 1)
			{
				/* External memory cycle */
				unsigned char memclk;
				memclk = c->memory_clock_cycle_count;
				e86_set_clk(c, memclk);
			}
			else
			{
				/* CPU memory cycle */
				e86_set_clk(c, 8);
			}
		}
	}
	else {
		e86_set_mem8(c, e86_get_es(c), e86_get_di(c), e86_get_prt8(c, e86_get_dx(c)));
		e86_set_di(c, e86_get_di(c) + inc);
		if (c->memory_clock_cycle_disable == 1)
		{
			/* External memory cycle */
			unsigned char memclk;
			memclk = c->memory_clock_cycle_count;
			e86_set_clk(c, memclk);
		}
		else
		{
			/* CPU memory cycle */
			e86_set_clk(c, 8);
		}
	}
	c->e386_address_size_prefix = 0;
	c->e386_operand_size_prefix = 0;
}

/* OP 6C: INSB (Protected Mode) */
static
unsigned e386_protected_mode_op_6c(e8086_t *c)
{
	unsigned short inc;
	inc = e86_get_df(c) ? 0xffff : 0x0001;
	if (c->prefix & (E86_PREFIX_REP | E86_PREFIX_REPN)) {
		while (e86_get_cx(c) > 0) {
			e86_set_mem8(c, e86_get_es(c), e86_get_di(c), e86_get_prt8(c, e86_get_dx(c)));
			e86_set_di(c, e86_get_di(c) + inc);
			e86_set_cx(c, e86_get_cx(c) - 1);
			c->instructions += 1;
			if (c->memory_clock_cycle_disable == 1)
			{
				/* External memory cycle */
				unsigned char memclk;
				memclk = c->memory_clock_cycle_count;
				e86_set_clk(c, memclk);
			}
			else
			{
				/* CPU memory cycle */
				e86_set_clk(c, 8);
			}
		}
	}
	else {
		e86_set_mem8(c, e86_get_es(c), e86_get_di(c), e86_get_prt8(c, e86_get_dx(c)));
		e86_set_di(c, e86_get_di(c) + inc);
		if (c->memory_clock_cycle_disable == 1)
		{
			/* External memory cycle */
			unsigned char memclk;
			memclk = c->memory_clock_cycle_count;
			e86_set_clk(c, memclk);
		}
		else
		{
			/* CPU memory cycle */
			e86_set_clk(c, 8);
		}
	}
	c->e386_address_size_prefix = 1;
	c->e386_operand_size_prefix = 1;
}

/* OP 6C: INSB */
static
unsigned op_6c (e8086_t *c)
{
	if (c->cpumodel >= 0x80386) {
		/* Intel 80386 CPU or higher */
		e286_get_machine_status_word(c);
		e386_get_flags(c);
		if ((e286_get_pe(c) == 0) && (e386_get_vm(c) == 0))
		{
			/* Real Mode */
			e386_real_mode_op_6c(c);
			return (1);
		}
		else if (e386_get_vm(c) == 1)
		{
			/* Virtual 8086 Mode */
			e386_virtual_8086_mode_op_6c(c);
			return (1);
		}
		else
		{
			/* Protected Mode */
			e386_protected_mode_op_6c(c);
			return (1);
		}
	}
	else {
		/* Intel 80186 CPU or  lower */
		unsigned short inc;
		inc = e86_get_df(c) ? 0xffff : 0x0001;
		if (c->prefix & (E86_PREFIX_REP | E86_PREFIX_REPN)) {
			while (e86_get_cx(c) > 0) {
				e86_set_mem8(c, e86_get_es(c), e86_get_di(c), e86_get_prt8(c, e86_get_dx(c)));
				e86_set_di(c, e86_get_di(c) + inc);
				e86_set_cx(c, e86_get_cx(c) - 1);
				c->instructions += 1;
				if (c->memory_clock_cycle_disable == 1)
				{
					/* External memory cycle */
					unsigned char memclk;
					memclk = c->memory_clock_cycle_count;
					e86_set_clk(c, memclk);
				}
				else
				{
					/* CPU memory cycle */
					e86_set_clk(c, 8);
				}
			}
		}
		else {
			e86_set_mem8(c, e86_get_es(c), e86_get_di(c), e86_get_prt8(c, e86_get_dx(c)));
			e86_set_di(c, e86_get_di(c) + inc);
			if (c->memory_clock_cycle_disable == 1)
			{
				/* External memory cycle */
				unsigned char memclk;
				memclk = c->memory_clock_cycle_count;
				e86_set_clk(c, memclk);
			}
			else
			{
				/* CPU memory cycle */
				e86_set_clk(c, 8);
			}
		}
		return (1);
	}
}

/* OP 6D: INSW (Real Mode) */
static
unsigned e386_real_mode_op_6d(e8086_t *c)
{
	unsigned short inc;
	inc = e86_get_df(c) ? 0xfffe : 0x0002;
	if (c->prefix & (E86_PREFIX_REP | E86_PREFIX_REPN)) {
		while (e86_get_cx(c) > 0) {
			e86_set_mem16(c, e86_get_es(c), e86_get_di(c), e86_get_prt16(c, e86_get_dx(c)));
			e86_set_di(c, e86_get_di(c) + inc);
			e86_set_cx(c, e86_get_cx(c) - 1);
			c->instructions += 1;
			if (c->memory_clock_cycle_disable == 1)
			{
				/* External memory cycle */
				unsigned char memclk;
				memclk = c->memory_clock_cycle_count;
				e86_set_clk(c, memclk);
			}
			else
			{
				/* CPU memory cycle */
				e86_set_clk(c, 8);
			}
		}
	}
	else {
		e86_set_mem16(c, e86_get_es(c), e86_get_di(c), e86_get_prt16(c, e86_get_dx(c)));
		e86_set_di(c, e86_get_di(c) + inc);
		if (c->memory_clock_cycle_disable == 1)
		{
			/* External memory cycle */
			unsigned char memclk;
			memclk = c->memory_clock_cycle_count;
			e86_set_clk(c, memclk);
		}
		else
		{
			/* CPU memory cycle */
			e86_set_clk(c, 8);
		}
	}
	c->e386_address_size_prefix = 0;
	c->e386_operand_size_prefix = 0;
	return (1);
}

/* OP 6D: INSW (Virtual 8086 Mode) */
static
unsigned e386_virtual_8086_mode_op_6d(e8086_t *c)
{
	unsigned short inc;
	inc = e86_get_df(c) ? 0xfffe : 0x0002;
	if (c->prefix & (E86_PREFIX_REP | E86_PREFIX_REPN)) {
		while (e86_get_cx(c) > 0) {
			e86_set_mem16(c, e86_get_es(c), e86_get_di(c), e86_get_prt16(c, e86_get_dx(c)));
			e86_set_di(c, e86_get_di(c) + inc);
			e86_set_cx(c, e86_get_cx(c) - 1);
			c->instructions += 1;
			if (c->memory_clock_cycle_disable == 1)
			{
				/* External memory cycle */
				unsigned char memclk;
				memclk = c->memory_clock_cycle_count;
				e86_set_clk(c, memclk);
			}
			else
			{
				/* CPU memory cycle */
				e86_set_clk(c, 8);
			}
		}
	}
	else {
		e86_set_mem16(c, e86_get_es(c), e86_get_di(c), e86_get_prt16(c, e86_get_dx(c)));
		e86_set_di(c, e86_get_di(c) + inc);
		if (c->memory_clock_cycle_disable == 1)
		{
			/* External memory cycle */
			unsigned char memclk;
			memclk = c->memory_clock_cycle_count;
			e86_set_clk(c, memclk);
		}
		else
		{
			/* CPU memory cycle */
			e86_set_clk(c, 8);
		}
	}
	c->e386_address_size_prefix = 0;
	c->e386_operand_size_prefix = 0;
	return (1);
}

/* OP 6D: INSW (Protected Mode) */
static
unsigned e386_protected_mode_op_6d(e8086_t *c)
{
	unsigned short inc;
	inc = e86_get_df(c) ? 0xfffe : 0x0002;
	if (c->prefix & (E86_PREFIX_REP | E86_PREFIX_REPN)) {
		while (e86_get_cx(c) > 0) {
			e86_set_mem16(c, e86_get_es(c), e86_get_di(c), e86_get_prt16(c, e86_get_dx(c)));
			e86_set_di(c, e86_get_di(c) + inc);
			e86_set_cx(c, e86_get_cx(c) - 1);
			c->instructions += 1;
			if (c->memory_clock_cycle_disable == 1)
			{
				/* External memory cycle */
				unsigned char memclk;
				memclk = c->memory_clock_cycle_count;
				e86_set_clk(c, memclk);
			}
			else
			{
				/* CPU memory cycle */
				e86_set_clk(c, 8);
			}
		}
	}
	else {
		e86_set_mem16(c, e86_get_es(c), e86_get_di(c), e86_get_prt16(c, e86_get_dx(c)));
		e86_set_di(c, e86_get_di(c) + inc);
		if (c->memory_clock_cycle_disable == 1)
		{
			/* External memory cycle */
			unsigned char memclk;
			memclk = c->memory_clock_cycle_count;
			e86_set_clk(c, memclk);
		}
		else
		{
			/* CPU memory cycle */
			e86_set_clk(c, 8);
		}
	}
	c->e386_address_size_prefix = 1;
	c->e386_operand_size_prefix = 1;
	return (1);
}

/* OP 6D: INSW */
static
unsigned op_6d (e8086_t *c)
{
	if (c->cpumodel >= 0x80386) {
		/* Intel 80386 CPU or higher */
		e286_get_machine_status_word(c);
		e386_get_flags(c);
		if ((e286_get_pe(c) == 0) && (e386_get_vm(c) == 0))
		{
			/* Real Mode */
			e386_real_mode_op_6d(c);
			return (1);
		}
		else if (e386_get_vm(c) == 1)
		{
			/* Virtual 8086 Mode */
			e386_virtual_8086_mode_op_6d(c);
			return (1);
		}
		else
		{
			/* Protected Mode */
			e386_protected_mode_op_6d(c);
			return (1);
		}
	}
	else {
		/* Intel 80186 CPU or  lower */
		unsigned short inc;
		inc = e86_get_df(c) ? 0xfffe : 0x0002;
		if (c->prefix & (E86_PREFIX_REP | E86_PREFIX_REPN)) {
			while (e86_get_cx(c) > 0) {
				e86_set_mem16(c, e86_get_es(c), e86_get_di(c), e86_get_prt16(c, e86_get_dx(c)));
				e86_set_di(c, e86_get_di(c) + inc);
				e86_set_cx(c, e86_get_cx(c) - 1);
				c->instructions += 1;
				if (c->memory_clock_cycle_disable == 1)
				{
					/* External memory cycle */
					unsigned char memclk;
					memclk = c->memory_clock_cycle_count;
					e86_set_clk(c, memclk);
				}
				else
				{
					/* CPU memory cycle */
					e86_set_clk(c, 8);
				}
			}
		}
		else {
			e86_set_mem16(c, e86_get_es(c), e86_get_di(c), e86_get_prt16(c, e86_get_dx(c)));
			e86_set_di(c, e86_get_di(c) + inc);
			if (c->memory_clock_cycle_disable == 1)
			{
				/* External memory cycle */
				unsigned char memclk;
				memclk = c->memory_clock_cycle_count;
				e86_set_clk(c, memclk);
			}
			else
			{
				/* CPU memory cycle */
				e86_set_clk(c, 8);
			}
		}
		return (1);
	}
}

/* OP 6E: OUTSB (Real Mode) */
/* static
unsigned e386_real_mode_op_6e(e8086_t *c)
{
	unsigned short seg;
	unsigned short inc;
	seg = e86_get_seg(c, E86_REG_DS);
	inc = e86_get_df(c) ? 0xffff : 0x0001;
	if (c->prefix & (E86_PREFIX_REP | E86_PREFIX_REPN)) {
		while (e86_get_cx(c) > 0) {
			e86_set_prt8(c, e86_get_dx(c), e86_get_mem8(c, seg, e86_get_si(c)));
			e86_set_si(c, e86_get_si(c) + inc);
			e86_set_cx(c, e86_get_cx(c) - 1);
			c->instructions += 1;
			if (c->memory_clock_cycle_disable == 1)
			{
				/* External memory cycle */
/* 				unsigned char memclk;
				memclk = c->memory_clock_cycle_count;
				e86_set_clk(c, memclk);
			}
			else
			{
				/* CPU memory cycle */
/* 				e86_set_clk(c, 8);
			}
		}
	}
	else {
		e86_set_prt8(c, e86_get_dx(c), e86_get_mem8(c, seg, e86_get_si(c)));
		e86_set_si(c, e86_get_si(c) + inc);
		if (c->memory_clock_cycle_disable == 1)
		{
			/* External memory cycle */
/* 			unsigned char memclk;
			memclk = c->memory_clock_cycle_count;
			e86_set_clk(c, memclk);
		}
		else
		{
			/* CPU memory cycle */
/* 			e86_set_clk(c, 8);
		}
	}
	c->e386_address_size_prefix = 0;
	c->e386_operand_size_prefix = 0;
	return (1);
} */

/* OP 6E: OUTSB (Virtual 8086 Mode) */
/* static
unsigned e386_virtual_8086_mode_op_6e(e8086_t *c)
{
	unsigned short seg;
	unsigned short inc;
	seg = e86_get_seg(c, E86_REG_DS);
	inc = e86_get_df(c) ? 0xffff : 0x0001;
	if (c->prefix & (E86_PREFIX_REP | E86_PREFIX_REPN)) {
		while (e86_get_cx(c) > 0) {
			e86_set_prt8(c, e86_get_dx(c), e86_get_mem8(c, seg, e86_get_si(c)));
			e86_set_si(c, e86_get_si(c) + inc);
			e86_set_cx(c, e86_get_cx(c) - 1);
			c->instructions += 1;
			if (c->memory_clock_cycle_disable == 1)
			{
				/* External memory cycle */
/* 				unsigned char memclk;
				memclk = c->memory_clock_cycle_count;
				e86_set_clk(c, memclk);
			}
			else
			{
				/* CPU memory cycle */
/* 				e86_set_clk(c, 8);
			}
		}
	}
	else {
		e86_set_prt8(c, e86_get_dx(c), e86_get_mem8(c, seg, e86_get_si(c)));
		e86_set_si(c, e86_get_si(c) + inc);
		if (c->memory_clock_cycle_disable == 1)
		{
			/* External memory cycle */
/* 			unsigned char memclk;
			memclk = c->memory_clock_cycle_count;
			e86_set_clk(c, memclk);
		}
		else
		{
			/* CPU memory cycle */
/* 			e86_set_clk(c, 8);
		}
	}
	c->e386_address_size_prefix = 0;
	c->e386_operand_size_prefix = 0;
	return (1);
} */

/* OP 6E: OUTSB (Protected Mode) */
/* static
unsigned e386_protected_mode_op_6e(e8086_t *c)
{
	unsigned short seg;
	unsigned short inc;
	seg = e86_get_seg(c, E86_REG_DS);
	inc = e86_get_df(c) ? 0xffff : 0x0001;
	if (c->prefix & (E86_PREFIX_REP | E86_PREFIX_REPN)) {
		while (e86_get_cx(c) > 0) {
			e86_set_prt8(c, e86_get_dx(c), e86_get_mem8(c, seg, e86_get_si(c)));
			e86_set_si(c, e86_get_si(c) + inc);
			e86_set_cx(c, e86_get_cx(c) - 1);
			c->instructions += 1;
			if (c->memory_clock_cycle_disable == 1)
			{
				/* External memory cycle */
/* 				unsigned char memclk;
				memclk = c->memory_clock_cycle_count;
				e86_set_clk(c, memclk);
			}
			else
			{
				/* CPU memory cycle */
/* 				e86_set_clk(c, 8);
			}
		}
	}
	else {
		e86_set_prt8(c, e86_get_dx(c), e86_get_mem8(c, seg, e86_get_si(c)));
		e86_set_si(c, e86_get_si(c) + inc);
		if (c->memory_clock_cycle_disable == 1)
		{
			/* External memory cycle */
/* 			unsigned char memclk;
			memclk = c->memory_clock_cycle_count;
			e86_set_clk(c, memclk);
		}
		else
		{
			/* CPU memory cycle */
/* 			e86_set_clk(c, 8);
		}
	}
	c->e386_address_size_prefix = 1;
	c->e386_operand_size_prefix = 1;
	return (1);
} */

/* OP 6E: OUTSB */
static
unsigned op_6e (e8086_t *c)
{
	if (c->cpumodel >= 0x80386) {
		/* Intel 80386 CPU or higher */
		e286_get_machine_status_word(c);
		e386_get_flags(c);
		if ((e286_get_pe(c) == 0) && (e386_get_vm(c) == 0))
		{
			/* Real Mode */
			if (c->e386_address_size_prefix == 1)
			{
				unsigned long  seg;
				unsigned long  inc;
				unsigned long  source;
				unsigned long  cs_segment_limit, ds_segment_limit, es_segment_limit, fs_segment_limit, gs_segment_limit, ss_segment_limit;
				/* 32-bit memory address operand specified */
				if (c->fs_segment == 1)
				{
					/* FS   segment   override   prefix   specified */
					seg = e386_get_fs(c);
				}
				else if (c->gs_segment == 1)
				{
					/* GS   segment   override   prefix   specified */
					seg = e386_get_gs(c);
				}
				else
				{
					/* CS, DS, ES, FS, GS, or SS segments specified */
					seg = e86_get_seg(c, E86_REG_DS);
				}
				inc = e386_get_df(c) ? 0xffffffff : 0x00000001;
				source = e386_get_esi(c);                                   /* Source Operand   */
				cs_segment_limit = c->cs_segment_register_descriptor_limit; /* CS Segment Limit */
				ds_segment_limit = c->ds_segment_register_descriptor_limit; /* DS Segment Limit */
				es_segment_limit = c->es_segment_register_descriptor_limit; /* ES Segment Limit */
				fs_segment_limit = c->fs_segment_register_descriptor_limit; /* FS Segment Limit */
				gs_segment_limit = c->gs_segment_register_descriptor_limit; /* GS Segment Limit */
				ss_segment_limit = c->ss_segment_register_descriptor_limit; /* SS Segment Limit */
				if (((seg == e386_get_cs(c)) && (source > cs_segment_limit)) || ((seg == e386_get_ds(c)) && (source > ds_segment_limit)) || ((seg == e386_get_es(c)) && (source > es_segment_limit)) || ((seg == e386_get_fs(c)) && (source > fs_segment_limit)) || ((seg == e386_get_gs(c)) && (source > gs_segment_limit)) || ((seg == e386_get_ss(c)) && (source > ss_segment_limit)))
				{
					/* Interrupt 13 if outside of effective address range from 0 to 0FFFFH */
					e86_trap(c, 13);
					c->fs_segment = 0;
					c->gs_segment = 0;
					c->e386_address_size_prefix = 0;
					c->e386_operand_size_prefix = 0;
					return (0);
				}
				if (c->prefix & (E86_PREFIX_REP | E86_PREFIX_REPN)) {
					while (e386_get_ecx(c) > 0) {
						e86_set_prt8(c, e386_get_edx(c), e86_get_mem8(c, seg, e386_get_esi(c)));
						e386_set_esi(c, e386_get_esi(c) + inc);
						e386_set_ecx(c, e386_get_ecx(c) - 1);
						c->instructions += 1;
						if (c->memory_clock_cycle_disable == 1)
						{
							/* External memory cycle */
							unsigned char memclk;
							memclk = c->memory_clock_cycle_count;
							e86_set_clk(c, memclk);
						}
						else
						{
							/* CPU memory cycle */
							e86_set_clk(c, 14);
						}
					}
				}
				else {
					e86_set_prt8(c, e386_get_edx(c), e86_get_mem8(c, seg, e386_get_esi(c)));
					e386_set_esi(c, e386_get_esi(c) + inc);
					if (c->memory_clock_cycle_disable == 1)
					{
						/* External memory cycle */
						unsigned char memclk;
						memclk = c->memory_clock_cycle_count;
						e86_set_clk(c, memclk);
					}
					else
					{
						/* CPU memory cycle */
						e86_set_clk(c, 14);
					}
				}
				c->e386_address_size_prefix = 0;
				c->e386_operand_size_prefix = 0;
				return (1);
			}
			else
			{
				/* 16-bit memory address operand specified */
				unsigned short seg;
				unsigned short inc;
				unsigned short source;
				unsigned long  cs_segment_limit, ds_segment_limit, es_segment_limit, fs_segment_limit, gs_segment_limit, ss_segment_limit;
				if (c->fs_segment == 1)
				{
					/* FS   segment   override   prefix   specified */
					seg = e386_get_fs(c);
				}
				else if (c->gs_segment == 1)
				{
					/* GS   segment   override   prefix   specified */
					seg = e386_get_gs(c);
				}
				else
				{
					/* CS, DS, ES, FS, GS, or SS segments specified */
					seg = e86_get_seg(c, E86_REG_DS);
				}
				source = e86_get_si(c);
				inc = e86_get_df(c) ? 0xffff : 0x0001;
				cs_segment_limit = c->cs_segment_register_descriptor_limit; /* CS Segment Limit */
				ds_segment_limit = c->ds_segment_register_descriptor_limit; /* DS Segment Limit */
				es_segment_limit = c->es_segment_register_descriptor_limit; /* ES Segment Limit */
				fs_segment_limit = c->fs_segment_register_descriptor_limit; /* FS Segment Limit */
				gs_segment_limit = c->gs_segment_register_descriptor_limit; /* GS Segment Limit */
				ss_segment_limit = c->ss_segment_register_descriptor_limit; /* SS Segment Limit */
				if (((seg == e86_get_cs(c)) && (source > cs_segment_limit)) || ((seg == e86_get_ds(c)) && (source > ds_segment_limit)) || ((seg == e86_get_es(c)) && (source > es_segment_limit)) || ((seg == e386_get_fs_16(c)) && (source > fs_segment_limit)) || ((seg == e386_get_gs_16(c)) && (source > gs_segment_limit)) || ((seg == e86_get_ss(c)) && (source > ss_segment_limit)))
				{
					/* Interrupt 13 if outside of effective address range from 0 to 0FFFFH */
					e86_trap(c, 13);
					c->fs_segment = 0;
					c->gs_segment = 0;
					c->e386_address_size_prefix = 0;
					c->e386_operand_size_prefix = 0;
					return (0);
				}
				if (c->prefix & (E86_PREFIX_REP | E86_PREFIX_REPN)) {
					while (e86_get_cx(c) > 0) {
						e86_set_prt8(c, e86_get_dx(c), e86_get_mem8(c, seg, e86_get_si(c)));
						e86_set_si(c, e86_get_si(c) + inc);
						e86_set_cx(c, e86_get_cx(c) - 1);
						c->instructions += 1;
						if (c->memory_clock_cycle_disable == 1)
						{
							/* External memory cycle */
							unsigned char memclk;
							memclk = c->memory_clock_cycle_count;
							e86_set_clk(c, memclk);
						}
						else
						{
							/* CPU memory cycle */
							e86_set_clk(c, 14);
						}
					}
				}
				else {
					e86_set_prt8(c, e86_get_dx(c), e86_get_mem8(c, seg, e86_get_si(c)));
					e86_set_si(c, e86_get_si(c) + inc);
					if (c->memory_clock_cycle_disable == 1)
					{
						/* External memory cycle */
						unsigned char memclk;
						memclk = c->memory_clock_cycle_count;
						e86_set_clk(c, memclk);
					}
					else
					{
						/* CPU memory cycle */
						e86_set_clk(c, 14);
					}
				}
				c->e386_address_size_prefix = 0;
				c->e386_operand_size_prefix = 0;
				return (1);
			}
			/* e386_real_mode_op_6e(c); */
			/* return (1); */
		}
		else if (e386_get_vm(c) == 1)
		{
			/* Virtual 8086 Mode */
			if (c->e386_address_size_prefix == 1)
			{
				unsigned long  seg;
				unsigned long  inc;
				unsigned long  source;
				unsigned long  cs_segment_limit, ds_segment_limit, es_segment_limit, fs_segment_limit, gs_segment_limit, ss_segment_limit;
				/* 32-bit memory address operand specified */
				if (c->fs_segment == 1)
				{
					/* FS   segment   override   prefix   specified */
					seg = e386_get_fs(c);
				}
				else if (c->gs_segment == 1)
				{
					/* GS   segment   override   prefix   specified */
					seg = e386_get_gs(c);
				}
				else
				{
					/* CS, DS, ES, FS, GS, or SS segments specified */
					seg = e86_get_seg(c, E86_REG_DS);
				}
				inc = e386_get_df(c) ? 0xffffffff : 0x00000001;
				source = e386_get_esi(c);
				cs_segment_limit = c->cs_segment_register_descriptor_limit; /* CS Segment Limit */
				ds_segment_limit = c->ds_segment_register_descriptor_limit; /* DS Segment Limit */
				es_segment_limit = c->es_segment_register_descriptor_limit; /* ES Segment Limit */
				fs_segment_limit = c->fs_segment_register_descriptor_limit; /* FS Segment Limit */
				gs_segment_limit = c->gs_segment_register_descriptor_limit; /* GS Segment Limit */
				ss_segment_limit = c->ss_segment_register_descriptor_limit; /* SS Segment Limit */
				/* General Protection Fault if corresponding I/O permission bits in Task State Segment equals 1 */
				if (c->prefix & (E86_PREFIX_REP | E86_PREFIX_REPN)) {
					while (e386_get_ecx(c) > 0) {
						e86_set_prt8(c, e386_get_edx(c), e86_get_mem8(c, seg, e386_get_esi(c)));
						e386_set_esi(c, e386_get_esi(c) + inc);
						e386_set_ecx(c, e386_get_ecx(c) - 1);
						c->instructions += 1;
						if (c->memory_clock_cycle_disable == 1)
						{
							/* External memory cycle */
							unsigned char memclk;
							memclk = c->memory_clock_cycle_count;
							e86_set_clk(c, memclk);
						}
						else
						{
							/* CPU memory cycle */
							e86_set_clk(c, 28);
						}
					}
				}
				else {
					e86_set_prt8(c, e386_get_edx(c), e86_get_mem8(c, seg, e386_get_esi(c)));
					e386_set_esi(c, e386_get_esi(c) + inc);
					if (c->memory_clock_cycle_disable == 1)
					{
						/* External memory cycle */
						unsigned char memclk;
						memclk = c->memory_clock_cycle_count;
						e86_set_clk(c, memclk);
					}
					else
					{
						/* CPU memory cycle */
						e86_set_clk(c, 28);
					}
				}
				c->e386_address_size_prefix = 0;
				c->e386_operand_size_prefix = 0;
				return (1);
			}
			else
			{
				/* 16-bit memory address operand specified */
				unsigned short seg;
				unsigned short inc;
				unsigned short source;
				unsigned long  cs_segment_limit, ds_segment_limit, es_segment_limit, fs_segment_limit, gs_segment_limit, ss_segment_limit;
				if (c->fs_segment == 1)
				{
					/* FS   segment   override   prefix   specified */
					seg = e386_get_fs(c);
				}
				else if (c->gs_segment == 1)
				{
					/* GS   segment   override   prefix   specified */
					seg = e386_get_gs(c);
				}
				else
				{
					/* CS, DS, ES, FS, GS, or SS segments specified */
					seg = e86_get_seg(c, E86_REG_DS);
				}
				inc = e86_get_df(c) ? 0xffff : 0x0001;
				source = e86_get_si(c);
				cs_segment_limit = c->cs_segment_register_descriptor_limit; /* CS Segment Limit */
				ds_segment_limit = c->ds_segment_register_descriptor_limit; /* DS Segment Limit */
				es_segment_limit = c->es_segment_register_descriptor_limit; /* ES Segment Limit */
				fs_segment_limit = c->fs_segment_register_descriptor_limit; /* FS Segment Limit */
				gs_segment_limit = c->gs_segment_register_descriptor_limit; /* GS Segment Limit */
				ss_segment_limit = c->ss_segment_register_descriptor_limit; /* SS Segment Limit */
				/* General Protection Fault if corresponding I/O permission bits in Task State Segment equals 1 */
				if (c->prefix & (E86_PREFIX_REP | E86_PREFIX_REPN)) {
					while (e86_get_cx(c) > 0) {
						e86_set_prt8(c, e86_get_dx(c), e86_get_mem8(c, seg, e86_get_si(c)));
						e86_set_si(c, e86_get_si(c) + inc);
						e86_set_cx(c, e86_get_cx(c) - 1);
						c->instructions += 1;
						if (c->memory_clock_cycle_disable == 1)
						{
							/* External memory cycle */
							unsigned char memclk;
							memclk = c->memory_clock_cycle_count;
							e86_set_clk(c, memclk);
						}
						else
						{
							/* CPU memory cycle */
							e86_set_clk(c, 28);
						}
					}
				}
				else {
					e86_set_prt8(c, e86_get_dx(c), e86_get_mem8(c, seg, e86_get_si(c)));
					e86_set_si(c, e86_get_si(c) + inc);
					if (c->memory_clock_cycle_disable == 1)
					{
						/* External memory cycle */
						unsigned char memclk;
						memclk = c->memory_clock_cycle_count;
						e86_set_clk(c, memclk);
					}
					else
					{
						/* CPU memory cycle */
						e86_set_clk(c, 28);
					}
				}
				c->e386_address_size_prefix = 0;
				c->e386_operand_size_prefix = 0;
				return (1);
			}
			/* e386_virtual_8086_mode_op_6e(c); */
			/* return (1); */
		}
		else
		{
			/* Protected Mode */
			if (c->e386_address_size_prefix == 1)
			{
				/* 32-bit memory address operand specified */
				unsigned long  seg;
				unsigned long  inc;
				unsigned long  source;
				unsigned long  cs_segment_limit, ds_segment_limit, es_segment_limit, fs_segment_limit, gs_segment_limit, ss_segment_limit;
				unsigned char  iopl, cpl;
				if (c->fs_segment == 1)
				{
					/* FS   segment   override   prefix   specified */
					seg = e386_get_fs(c);
				}
				else if (c->gs_segment == 1)
				{
					/* GS   segment   override   prefix   specified */
					seg = e386_get_gs(c);
				}
				else
				{
					/* CS, DS, ES, FS, GS, or SS segments specified */
					seg = e86_get_seg(c, E86_REG_DS);
				}
				inc = e386_get_df(c) ? 0xffffffff : 0x00000001;
				source = e386_get_esi(c);
				cs_segment_limit = c->cs_segment_register_descriptor_limit; /* CS Segment Limit */
				ds_segment_limit = c->ds_segment_register_descriptor_limit; /* DS Segment Limit */
				es_segment_limit = c->es_segment_register_descriptor_limit; /* ES Segment Limit */
				fs_segment_limit = c->fs_segment_register_descriptor_limit; /* FS Segment Limit */
				gs_segment_limit = c->gs_segment_register_descriptor_limit; /* GS Segment Limit */
				ss_segment_limit = c->ss_segment_register_descriptor_limit; /* SS Segment Limit */
				iopl = e386_get_iopl(c);
				cpl = c->current_privilege_level;
				if (((seg == e386_get_cs(c)) && (source > cs_segment_limit)) || ((seg == e386_get_ds(c)) && (source > ds_segment_limit)) || ((seg == e386_get_es(c)) && (source > es_segment_limit)))
				{
					/* General Protection Fault if outside of CS, DS, or ES segment limits */
					e86_trap(c, 13);
					c->fs_segment = 0;
					c->gs_segment = 0;
					c->e386_address_size_prefix = 1;
					c->e386_operand_size_prefix = 1;
					return (0);
				}
				if (((seg == e386_get_cs(c)) && (c->cs_segment_register_descriptor_available == 0)) || ((seg == e386_get_ds(c)) && (c->ds_segment_register_descriptor_available == 0)) || ((seg == e386_get_es(c)) && (c->es_segment_register_descriptor_available == 0)))
				{
					/* General Protection Fault if illegal memory operand effective address in CS, DS, or ES segments */
					e86_trap(c, 13);
					c->fs_segment = 0;
					c->gs_segment = 0;
					c->e386_address_size_prefix = 1;
					c->e386_operand_size_prefix = 1;
					return (0);
				}
				if ((seg == e386_get_ss(c)) && (source > ss_segment_limit))
				{
					/* Stack              Fault if outside of            SS segment limit  */
					e86_trap(c, 12);
					c->fs_segment = 0;
					c->gs_segment = 0;
					c->e386_address_size_prefix = 1;
					c->e386_operand_size_prefix = 1;
					return (0);
				}
				if ((seg == e386_get_ss(c)) && (c->ss_segment_register_descriptor_available == 0))
				{
					/* Stack              Fault if illegal memory operand effective address in            SS segment  */
					e86_trap(c, 12);
					c->fs_segment = 0;
					c->gs_segment = 0;
					c->e386_address_size_prefix = 1;
					c->e386_operand_size_prefix = 1;
					return (0);
				}
				/* General Protection Fault if Current Privilege Level is greater than I/O Privilege leval and */
				/* corresponding I/O permission bits in Task State Segment equals 1                            */
				if (c->prefix & (E86_PREFIX_REP | E86_PREFIX_REPN)) {
					while (e386_get_ecx(c) > 0) {
						e86_set_prt8(c, e386_get_edx(c), e86_get_mem8(c, seg, e386_get_esi(c)));
						e386_set_esi(c, e386_get_esi(c) + inc);
						e386_set_ecx(c, e386_get_ecx(c) - 1);
						c->instructions += 1;
						if (c->memory_clock_cycle_disable == 1)
						{
							/* External memory cycle */
							unsigned char memclk;
							memclk = c->memory_clock_cycle_count;
							e86_set_clk(c, memclk);
						}
						else
						{
							/* CPU memory cycle */
							if (cpl > iopl)
							{
								/* Current Privilege Level greater than I/O Privilege Level */
								e86_set_clk(c, 28);
							}
							else
							{
								/* Current Privilege Level greater than I/O Privilege Level */
								e86_set_clk(c, 8);
							}
						}
					}
				}
				else {
					e86_set_prt8(c, e386_get_edx(c), e86_get_mem8(c, seg, e386_get_esi(c)));
					e386_set_esi(c, e386_get_esi(c) + inc);
					if (c->memory_clock_cycle_disable == 1)
					{
						/* External memory cycle */
						unsigned char memclk;
						memclk = c->memory_clock_cycle_count;
						e86_set_clk(c, memclk);
					}
					else
					{
						/* CPU memory cycle */
						if (cpl > iopl)
						{
							/* Current Privilege Level greater than I/O Privilege Level */
							e86_set_clk(c, 28);
						}
						else
						{
							/* Current Privilege Level greater than I/O Privilege Level */
							e86_set_clk(c, 8);
						}
					}
				}
				c->e386_address_size_prefix = 1;
				c->e386_operand_size_prefix = 1;
				return (1);
			}
			else
			{
				/* 16-bit memory address operand specified */
				unsigned short seg;
				unsigned short inc;
				unsigned short source;
				unsigned long  cs_segment_limit, ds_segment_limit, es_segment_limit, fs_segment_limit, gs_segment_limit, ss_segment_limit;
				unsigned char  iopl, cpl;
				if (c->fs_segment == 1)
				{
					/* FS   segment   override   prefix   specified */
					seg = e386_get_fs(c);
				}
				else if (c->gs_segment == 1)
				{
					/* GS   segment   override   prefix   specified */
					seg = e386_get_gs(c);
				}
				else
				{
					/* CS, DS, ES, FS, GS, or SS segments specified */
					seg = e86_get_seg(c, E86_REG_DS);
				}
				inc = e86_get_df(c) ? 0xffff : 0x0001;
				source = e86_get_si(c);
				cs_segment_limit = c->cs_segment_register_descriptor_limit; /* CS Segment Limit */
				ds_segment_limit = c->ds_segment_register_descriptor_limit; /* DS Segment Limit */
				es_segment_limit = c->es_segment_register_descriptor_limit; /* ES Segment Limit */
				fs_segment_limit = c->fs_segment_register_descriptor_limit; /* FS Segment Limit */
				gs_segment_limit = c->gs_segment_register_descriptor_limit; /* GS Segment Limit */
				ss_segment_limit = c->ss_segment_register_descriptor_limit; /* SS Segment Limit */
				iopl = e386_get_iopl(c);
				cpl = c->current_privilege_level;
				if (((seg == e86_get_cs(c)) && (source > cs_segment_limit)) || ((seg == e86_get_ds(c)) && (source > ds_segment_limit)) || ((seg == e86_get_es(c)) && (source > es_segment_limit)))
				{
					/* General Protection Fault if outside of CS, DS, or ES segment limits */
					e86_trap(c, 13);
					c->fs_segment = 0;
					c->gs_segment = 0;
					c->e386_address_size_prefix = 1;
					c->e386_operand_size_prefix = 1;
					return (0);
				}
				if (((seg == e86_get_cs(c)) && (c->cs_segment_register_descriptor_available == 0)) || ((seg == e86_get_ds(c)) && (c->ds_segment_register_descriptor_available == 0)) || ((seg == e86_get_es(c)) && (c->es_segment_register_descriptor_available == 0)))
				{
					/* General Protection Fault if illegal memory operand effective address in CS, DS, or ES segments */
					e86_trap(c, 13);
					c->fs_segment = 0;
					c->gs_segment = 0;
					c->e386_address_size_prefix = 1;
					c->e386_operand_size_prefix = 1;
					return (0);
				}
				if ((seg == e86_get_ss(c)) && (source > ss_segment_limit))
				{
					/* Stack              Fault if outside of            SS segment limit  */
					e86_trap(c, 12);
					c->fs_segment = 0;
					c->gs_segment = 0;
					c->e386_address_size_prefix = 1;
					c->e386_operand_size_prefix = 1;
					return (0);
				}
				if ((seg == e86_get_ss(c)) && (c->ss_segment_register_descriptor_available == 0))
				{
					/* Stack              Fault if illegal memory operand effective address in            SS segment  */
					e86_trap(c, 12);
					c->fs_segment = 0;
					c->gs_segment = 0;
					c->e386_address_size_prefix = 1;
					c->e386_operand_size_prefix = 1;
					return (0);
				}
				/* General Protection Fault if Current Privilege Level is greater than I/O Privilege leval and */
				/* corresponding I/O permission bits in Task State Segment equals 1                            */
				if (c->prefix & (E86_PREFIX_REP | E86_PREFIX_REPN)) {
					while (e86_get_cx(c) > 0) {
						e86_set_prt8(c, e86_get_dx(c), e86_get_mem8(c, seg, e86_get_si(c)));
						e86_set_si(c, e86_get_si(c) + inc);
						e86_set_cx(c, e86_get_cx(c) - 1);
						c->instructions += 1;
						if (c->memory_clock_cycle_disable == 1)
						{
							/* External memory cycle */
							unsigned char memclk;
							memclk = c->memory_clock_cycle_count;
							e86_set_clk(c, memclk);
						}
						else
						{
							/* CPU memory cycle */
							if (cpl > iopl)
							{
								/* Current Privilege Level greater than I/O Privilege Level */
								e86_set_clk(c, 28);
							}
							else
							{
								/* Current Privilege Level greater than I/O Privilege Level */
								e86_set_clk(c, 8);
							}
						}
					}
				}
				else {
					e86_set_prt8(c, e86_get_dx(c), e86_get_mem8(c, seg, e86_get_si(c)));
					e86_set_si(c, e86_get_si(c) + inc);
					if (c->memory_clock_cycle_disable == 1)
					{
						/* External memory cycle */
						unsigned char memclk;
						memclk = c->memory_clock_cycle_count;
						e86_set_clk(c, memclk);
					}
					else
					{
						/* CPU memory cycle */
						if (cpl > iopl)
						{
							/* Current Privilege Level greater than I/O Privilege Level */
							e86_set_clk(c, 28);
						}
						else
						{
							/* Current Privilege Level greater than I/O Privilege Level */
							e86_set_clk(c, 8);
						}
					}
				}
				c->e386_address_size_prefix = 1;
				c->e386_operand_size_prefix = 1;
				return (1);
			}
			/* e386_protected_mode_op_6e(c); */
			/* return (1); */
		}
	}
	else {
		/* Intel 80186 CPU or  lower */
		unsigned short seg;
		unsigned short inc;
		seg = e86_get_seg(c, E86_REG_DS);
		inc = e86_get_df(c) ? 0xffff : 0x0001;
		if (c->prefix & (E86_PREFIX_REP | E86_PREFIX_REPN)) {
			while (e86_get_cx(c) > 0) {
				e86_set_prt8(c, e86_get_dx(c), e86_get_mem8(c, seg, e86_get_si(c)));
				e86_set_si(c, e86_get_si(c) + inc);
				e86_set_cx(c, e86_get_cx(c) - 1);
				c->instructions += 1;
				if (c->memory_clock_cycle_disable == 1)
				{
					/* External memory cycle */
					unsigned char memclk;
					memclk = c->memory_clock_cycle_count;
					e86_set_clk(c, memclk);
				}
				else
				{
					/* CPU memory cycle */
					e86_set_clk(c, 8);
				}
			}
		}
		else {
			e86_set_prt8(c, e86_get_dx(c), e86_get_mem8(c, seg, e86_get_si(c)));
			e86_set_si(c, e86_get_si(c) + inc);
			if (c->memory_clock_cycle_disable == 1)
			{
				/* External memory cycle */
				unsigned char memclk;
				memclk = c->memory_clock_cycle_count;
				e86_set_clk(c, memclk);
			}
			else
			{
				/* CPU memory cycle */
				e86_set_clk(c, 8);
			}
		}
		return (1);
	}
}

/* Opcode 6F: OUTSW or OUTSD (Real Mode) */
/* static
unsigned e386_real_mode_op_6f(e8086_t *c)
{
	unsigned short seg;
	unsigned short inc;
	seg = e86_get_seg(c, E86_REG_DS);
	inc = e86_get_df(c) ? 0xfffe : 0x0002;
	if (c->prefix & (E86_PREFIX_REP | E86_PREFIX_REPN)) {
		while (e86_get_cx(c) > 0) {
			e86_set_prt16(c, e86_get_dx(c), e86_get_mem16(c, seg, e86_get_si(c)));
			e86_set_si(c, e86_get_si(c) + inc);
			e86_set_cx(c, e86_get_cx(c) - 1);
			c->instructions += 1;
			if (c->memory_clock_cycle_disable == 1)
			{
				/* External memory cycle */
/* 				unsigned char memclk;
				memclk = c->memory_clock_cycle_count;
				e86_set_clk(c, memclk);
			}
			else
			{
				/* CPU memory cycle */
/* 				e86_set_clk(c, 8);
			}
		}
	}
	else {
		e86_set_prt16(c, e86_get_dx(c), e86_get_mem16(c, seg, e86_get_si(c)));
		e86_set_si(c, e86_get_si(c) + inc);
		if (c->memory_clock_cycle_disable == 1)
		{
			/* External memory cycle */
/* 			unsigned char memclk;
			memclk = c->memory_clock_cycle_count;
			e86_set_clk(c, memclk);
		}
		else
		{
			/* CPU memory cycle */
/* 			e86_set_clk(c, 8);
		}
	}
	c->e386_address_size_prefix = 0;
	c->e386_operand_size_prefix = 0;
	return (1);
} */

/* Opcode 6F: OUTSW or OUTSD (Virtual 8086 Mode) */
/* static
unsigned e386_virtual_8086_mode_op_6f(e8086_t *c)
{
	unsigned short seg;
	unsigned short inc;
	seg = e86_get_seg(c, E86_REG_DS);
	inc = e86_get_df(c) ? 0xfffe : 0x0002;
	if (c->prefix & (E86_PREFIX_REP | E86_PREFIX_REPN)) {
		while (e86_get_cx(c) > 0) {
			e86_set_prt16(c, e86_get_dx(c), e86_get_mem16(c, seg, e86_get_si(c)));
			e86_set_si(c, e86_get_si(c) + inc);
			e86_set_cx(c, e86_get_cx(c) - 1);
			c->instructions += 1;
			if (c->memory_clock_cycle_disable == 1)
			{
				/* External memory cycle */
/* 				unsigned char memclk;
				memclk = c->memory_clock_cycle_count;
				e86_set_clk(c, memclk);
			}
			else
			{
				/* CPU memory cycle */
/* 				e86_set_clk(c, 8);
			}
		}
	}
	else {
		e86_set_prt16(c, e86_get_dx(c), e86_get_mem16(c, seg, e86_get_si(c)));
		e86_set_si(c, e86_get_si(c) + inc);
		if (c->memory_clock_cycle_disable == 1)
		{
			/* External memory cycle */
/* 			unsigned char memclk;
			memclk = c->memory_clock_cycle_count;
			e86_set_clk(c, memclk);
		}
		else
		{
			/* CPU memory cycle */
/* 			e86_set_clk(c, 8);
		}
	}
	c->e386_address_size_prefix = 0;
	c->e386_operand_size_prefix = 0;
	return (1);
} */

/* Opcode 6F: OUTSW or OUTSD (Protected Mode) */
/* static
unsigned e386_protected_mode_op_6f(e8086_t *c)
{
	unsigned short seg;
	unsigned short inc;
	seg = e86_get_seg(c, E86_REG_DS);
	inc = e86_get_df(c) ? 0xfffe : 0x0002;
	if (c->prefix & (E86_PREFIX_REP | E86_PREFIX_REPN)) {
		while (e86_get_cx(c) > 0) {
			e86_set_prt16(c, e86_get_dx(c), e86_get_mem16(c, seg, e86_get_si(c)));
			e86_set_si(c, e86_get_si(c) + inc);
			e86_set_cx(c, e86_get_cx(c) - 1);
			c->instructions += 1;
			if (c->memory_clock_cycle_disable == 1)
			{
				/* External memory cycle */
/* 				unsigned char memclk;
				memclk = c->memory_clock_cycle_count;
				e86_set_clk(c, memclk);
			}
			else
			{
				/* CPU memory cycle */
/* 				e86_set_clk(c, 8);
			}
		}
	}
	else {
		e86_set_prt16(c, e86_get_dx(c), e86_get_mem16(c, seg, e86_get_si(c)));
		e86_set_si(c, e86_get_si(c) + inc);
		if (c->memory_clock_cycle_disable == 1)
		{
			/* External memory cycle */
/* 			unsigned char memclk;
			memclk = c->memory_clock_cycle_count;
			e86_set_clk(c, memclk);
		}
		else
		{
			/* CPU memory cycle */
/* 			e86_set_clk(c, 8);
		}
	}
	c->e386_address_size_prefix = 1;
	c->e386_operand_size_prefix = 1;
	return (1);
} */

/* Opcode 6F: OUTSW */
static
unsigned op_6f (e8086_t *c)
{
	if (c->cpumodel >= 0x80386) {
		/* Intel 80386 CPU or higher */
		e286_get_machine_status_word(c);
		e386_get_flags(c);
		if ((e286_get_pe(c) == 0) && (e386_get_vm(c) == 0))
		{
			/* Real Mode */
			if (c->e386_address_size_prefix == 1)
			{
				/* 32-bit memory address operand specified */
				unsigned long  seg;
				unsigned long  inc;
				unsigned long  source;
				unsigned long  cs_segment_limit, ds_segment_limit, es_segment_limit, fs_segment_limit, gs_segment_limit, ss_segment_limit;
				if (c->fs_segment == 1)
				{
					/* FS   segment   override   prefix   specified */
					seg = e386_get_fs(c);
				}
				else if (c->gs_segment == 1)
				{
					/* GS   segment   override   prefix   specified */
					seg = e386_get_gs(c);
				}
				else
				{
					/* CS, DS, ES, FS, GS, or SS segments specified */
					seg = e86_get_seg(c, E86_REG_DS);
				}
				if (c->e386_operand_size_prefix == 1)
				{
					/* 32-bit register operand specified */
					inc = e386_get_df(c) ? 0xfffffffe : 0x00000004;
				}
				else
				{
					/* 16-bit register operand specified */
					inc = e386_get_df(c) ? 0xfffffffe : 0x00000002;
				}
				source = e386_get_esi(c);                                   /* Source Operand   */
				cs_segment_limit = c->cs_segment_register_descriptor_limit; /* CS Segment Limit */
				ds_segment_limit = c->ds_segment_register_descriptor_limit; /* DS Segment Limit */
				es_segment_limit = c->es_segment_register_descriptor_limit; /* ES Segment Limit */
				fs_segment_limit = c->fs_segment_register_descriptor_limit; /* FS Segment Limit */
				gs_segment_limit = c->gs_segment_register_descriptor_limit; /* GS Segment Limit */
				ss_segment_limit = c->ss_segment_register_descriptor_limit; /* SS Segment Limit */
				if (((seg == e386_get_cs(c)) && (source > cs_segment_limit)) || ((seg == e386_get_ds(c)) && (source > ds_segment_limit)) || ((seg == e386_get_es(c)) && (source > es_segment_limit)) || ((seg == e386_get_fs(c)) && (source > fs_segment_limit)) || ((seg == e386_get_gs(c)) && (source > gs_segment_limit)) || ((seg == e386_get_ss(c)) && (source > ss_segment_limit)))
				{
					/* Interrupt 13 if outside of effective address range from 0 to 0FFFFH */
					e86_trap(c, 13);
					c->fs_segment = 0;
					c->gs_segment = 0;
					c->e386_address_size_prefix = 0;
					c->e386_operand_size_prefix = 0;
					return (0);
				}
				if (c->prefix & (E86_PREFIX_REP | E86_PREFIX_REPN)) {
					while (e386_get_ecx(c) > 0) {
						e86_set_prt32(c, e386_get_edx(c), e386_get_mem32(c, seg, e386_get_esi(c)));
						e386_set_esi(c, e386_get_esi(c) + inc);
						e386_set_ecx(c, e386_get_ecx(c) - 1);
						c->instructions += 1;
						if (c->memory_clock_cycle_disable == 1)
						{
							/* External memory cycle */
							unsigned char memclk;
							memclk = c->memory_clock_cycle_count;
							e86_set_clk(c, memclk);
						}
						else
						{
							/* CPU memory cycle */
							e86_set_clk(c, 14);
						}
					}
				}
				else {
					e86_set_prt32(c, e386_get_edx(c), e386_get_mem32(c, seg, e386_get_esi(c)));
					e386_set_esi(c, e386_get_esi(c) + inc);
					if (c->memory_clock_cycle_disable == 1)
					{
						/* External memory cycle */
						unsigned char memclk;
						memclk = c->memory_clock_cycle_count;
						e86_set_clk(c, memclk);
					}
					else
					{
						/* CPU memory cycle */
						e86_set_clk(c, 14);
					}
				}
				c->fs_segment = 0;
				c->gs_segment = 0;
				c->e386_address_size_prefix = 0;
				c->e386_operand_size_prefix = 0;
				return (1);
			}
			else
			{
				/* 16-bit memory address operand specified */
				unsigned short seg;
				unsigned short inc;
				unsigned short source;
				unsigned long  cs_segment_limit, ds_segment_limit, es_segment_limit, fs_segment_limit, gs_segment_limit, ss_segment_limit;
				if (c->fs_segment == 1)
				{
					/* FS   segment   override   prefix   specified */
					seg = e386_get_fs(c);
				}
				else if (c->gs_segment == 1)
				{
					/* GS   segment   override   prefix   specified */
					seg = e386_get_gs(c);
				}
				else
				{
					/* CS, DS, ES, FS, GS, or SS segments specified */
					seg = e86_get_seg(c, E86_REG_DS);
				}
				if (c->e386_operand_size_prefix == 1)
				{
					/* 32-bit register operand specified */
					inc = e86_get_df(c) ? 0xfffe : 0x0004;
				}
				else
				{
					/* 16-bit register operand specified */
					inc = e86_get_df(c) ? 0xfffe : 0x0002;
				}
				source = e86_get_si(c);                                     /* Source Operand   */
				cs_segment_limit = c->cs_segment_register_descriptor_limit; /* CS Segment Limit */
				ds_segment_limit = c->ds_segment_register_descriptor_limit; /* DS Segment Limit */
				es_segment_limit = c->es_segment_register_descriptor_limit; /* ES Segment Limit */
				fs_segment_limit = c->fs_segment_register_descriptor_limit; /* FS Segment Limit */
				gs_segment_limit = c->gs_segment_register_descriptor_limit; /* GS Segment Limit */
				ss_segment_limit = c->ss_segment_register_descriptor_limit; /* SS Segment Limit */
				if (((seg == e86_get_cs(c)) && (source > cs_segment_limit)) || ((seg == e86_get_ds(c)) && (source > ds_segment_limit)) || ((seg == e86_get_es(c)) && (source > es_segment_limit)) || ((seg == e386_get_fs_16(c)) && (source > fs_segment_limit)) || ((seg == e386_get_gs_16(c)) && (source > gs_segment_limit)) || ((seg == e86_get_ss(c)) && (source > ss_segment_limit)))
				{
					/* Interrupt 13 if outside of effective address range from 0 to 0FFFFH */
					e86_trap(c, 13);
					c->fs_segment = 0;
					c->gs_segment = 0;
					c->e386_address_size_prefix = 0;
					c->e386_operand_size_prefix = 0;
					return (0);
				}
				if (c->prefix & (E86_PREFIX_REP | E86_PREFIX_REPN)) {
					while (e86_get_cx(c) > 0) {
						e86_set_prt16(c, e86_get_dx(c), e86_get_mem16(c, seg, e86_get_si(c)));
						e86_set_si(c, e86_get_si(c) + inc);
						e86_set_cx(c, e86_get_cx(c) - 1);
						c->instructions += 1;
						if (c->memory_clock_cycle_disable == 1)
						{
							/* External memory cycle */
							unsigned char memclk;
							memclk = c->memory_clock_cycle_count;
							e86_set_clk(c, memclk);
						}
						else
						{
							/* CPU memory cycle */
							e86_set_clk(c, 14);
						}
					}
				}
				else {
					e86_set_prt16(c, e86_get_dx(c), e86_get_mem16(c, seg, e86_get_si(c)));
					e86_set_si(c, e86_get_si(c) + inc);
					if (c->memory_clock_cycle_disable == 1)
					{
						/* External memory cycle */
						unsigned char memclk;
						memclk = c->memory_clock_cycle_count;
						e86_set_clk(c, memclk);
					}
					else
					{
						/* CPU memory cycle */
						e86_set_clk(c, 14);
					}
				}
				c->fs_segment = 0;
				c->gs_segment = 0;
				c->e386_address_size_prefix = 0;
				c->e386_operand_size_prefix = 0;
				return (1);
			}
			/* e386_real_mode_op_6f(c); */
			/* return (1); */
		}
		else if (e386_get_vm(c) == 1)
		{
			/* Virtual 8086 Mode */
			if (c->e386_address_size_prefix == 1)
			{
				/* 32-bit memory address operand specified */
				unsigned long  seg;
				unsigned long  inc;
				unsigned long  source;
				unsigned long  cs_segment_limit, ds_segment_limit, es_segment_limit, fs_segment_limit, gs_segment_limit, ss_segment_limit;
				if (c->fs_segment == 1)
				{
					/* FS   segment   override   prefix   specified */
					seg = e386_get_fs(c);
				}
				else if (c->gs_segment == 1)
				{
					/* GS   segment   override   prefix   specified */
					seg = e386_get_gs(c);
				}
				else
				{
					/* CS, DS, ES, FS, GS, or SS segments specified */
					seg = e86_get_seg(c, E86_REG_DS);
				}
				if (c->e386_operand_size_prefix == 1)
				{
					/* 32-bit register operand specified */
					inc = e386_get_df(c) ? 0xfffffffe : 0x00000004;
				}
				else
				{
					/* 16-bit register operand specified */
					inc = e386_get_df(c) ? 0xfffffffe : 0x00000002;
				}
				source = e386_get_esi(c);                                   /* Source Operand   */
				cs_segment_limit = c->cs_segment_register_descriptor_limit; /* CS Segment Limit */
				ds_segment_limit = c->ds_segment_register_descriptor_limit; /* DS Segment Limit */
				es_segment_limit = c->es_segment_register_descriptor_limit; /* ES Segment Limit */
				fs_segment_limit = c->fs_segment_register_descriptor_limit; /* FS Segment Limit */
				gs_segment_limit = c->gs_segment_register_descriptor_limit; /* GS Segment Limit */
				ss_segment_limit = c->ss_segment_register_descriptor_limit; /* SS Segment Limit */
				/* General Protection Fault if corresponding I/O permission bits in Task State Segment equals 1 */
				if (c->prefix & (E86_PREFIX_REP | E86_PREFIX_REPN)) {
					while (e386_get_ecx(c) > 0) {
						e86_set_prt32(c, e386_get_edx(c), e386_get_mem32(c, seg, e386_get_esi(c)));
						e386_set_esi(c, e386_get_esi(c) + inc);
						e386_set_ecx(c, e386_get_ecx(c) - 1);
						c->instructions += 1;
						if (c->memory_clock_cycle_disable == 1)
						{
							/* External memory cycle */
							unsigned char memclk;
							memclk = c->memory_clock_cycle_count;
							e86_set_clk(c, memclk);
						}
						else
						{
							/* CPU memory cycle */
							e86_set_clk(c, 28);
						}
					}
				}
				else {
					e86_set_prt32(c, e386_get_edx(c), e386_get_mem32(c, seg, e386_get_esi(c)));
					e386_set_esi(c, e386_get_esi(c) + inc);
					if (c->memory_clock_cycle_disable == 1)
					{
						/* External memory cycle */
						unsigned char memclk;
						memclk = c->memory_clock_cycle_count;
						e86_set_clk(c, memclk);
					}
					else
					{
						/* CPU memory cycle */
						e86_set_clk(c, 28);
					}
				}
				c->fs_segment = 0;
				c->gs_segment = 0;
				c->e386_address_size_prefix = 0;
				c->e386_operand_size_prefix = 0;
				return (1);
			}
			else
			{
				/* 16-bit memory address operand specified */
				unsigned short seg;
				unsigned short inc;
				unsigned short source;
				unsigned long  cs_segment_limit, ds_segment_limit, es_segment_limit, fs_segment_limit, gs_segment_limit, ss_segment_limit;
				if (c->fs_segment == 1)
				{
					/* FS   segment   override   prefix   specified */
					seg = e386_get_fs(c);
				}
				else if (c->gs_segment == 1)
				{
					/* GS   segment   override   prefix   specified */
					seg = e386_get_gs(c);
				}
				else
				{
					/* CS, DS, ES, FS, GS, or SS segments specified */
					seg = e86_get_seg(c, E86_REG_DS);
				}
				if (c->e386_operand_size_prefix == 1)
				{
					/* 32-bit register operand specified */
					inc = e86_get_df(c) ? 0xfffe : 0x0004;
				}
				else
				{
					/* 16-bit register operand specified */
					inc = e86_get_df(c) ? 0xfffe : 0x0002;
				}
				source = e86_get_si(c);                                     /* Source Operand   */
				cs_segment_limit = c->cs_segment_register_descriptor_limit; /* CS Segment Limit */
				ds_segment_limit = c->ds_segment_register_descriptor_limit; /* DS Segment Limit */
				es_segment_limit = c->es_segment_register_descriptor_limit; /* ES Segment Limit */
				fs_segment_limit = c->fs_segment_register_descriptor_limit; /* FS Segment Limit */
				gs_segment_limit = c->gs_segment_register_descriptor_limit; /* GS Segment Limit */
				ss_segment_limit = c->ss_segment_register_descriptor_limit; /* SS Segment Limit */
				/* General Protection Fault if corresponding I/O permission bits in Task State Segment equals 1 */
				if (c->prefix & (E86_PREFIX_REP | E86_PREFIX_REPN)) {
					while (e86_get_cx(c) > 0) {
						e86_set_prt16(c, e86_get_dx(c), e86_get_mem16(c, seg, e86_get_si(c)));
						e86_set_si(c, e86_get_si(c) + inc);
						e86_set_cx(c, e86_get_cx(c) - 1);
						c->instructions += 1;
						if (c->memory_clock_cycle_disable == 1)
						{
							/* External memory cycle */
							unsigned char memclk;
							memclk = c->memory_clock_cycle_count;
							e86_set_clk(c, memclk);
						}
						else
						{
							/* CPU memory cycle */
							e86_set_clk(c, 28);
						}
					}
				}
				else {
					e86_set_prt16(c, e86_get_dx(c), e86_get_mem16(c, seg, e86_get_si(c)));
					e86_set_si(c, e86_get_si(c) + inc);
					if (c->memory_clock_cycle_disable == 1)
					{
						/* External memory cycle */
						unsigned char memclk;
						memclk = c->memory_clock_cycle_count;
						e86_set_clk(c, memclk);
					}
					else
					{
						/* CPU memory cycle */
						e86_set_clk(c, 28);
					}
				}
				c->fs_segment = 0;
				c->gs_segment = 0;
				c->e386_address_size_prefix = 0;
				c->e386_operand_size_prefix = 0;
				return (1);
			}
			/* e386_virtual_8086_mode_op_6f(c); */
			/* return (1); */
		}
		else
		{
			/* Protected Mode */
			if (c->e386_address_size_prefix == 1)
			{
				/* 32-bit memory address operand specified */
				unsigned long  seg;
				unsigned long  inc;
				unsigned long  source;
				unsigned long  cs_segment_limit, ds_segment_limit, es_segment_limit, fs_segment_limit, gs_segment_limit, ss_segment_limit;
				unsigned char  iopl, cpl;
				if (c->fs_segment == 1)
				{
					/* FS   segment   override   prefix   specified */
					seg = e386_get_fs(c);
				}
				else if (c->gs_segment == 1)
				{
					/* GS   segment   override   prefix   specified */
					seg = e386_get_gs(c);
				}
				else
				{
					/* CS, DS, ES, FS, GS, or SS segments specified */
					seg = e86_get_seg(c, E86_REG_DS);
				}
				if (c->e386_operand_size_prefix == 1)
				{
					/* 32-bit register operand specified */
					inc = e386_get_df(c) ? 0xfffffffe : 0x00000004;
				}
				else
				{
					/* 16-bit register operand specified */
					inc = e386_get_df(c) ? 0xfffffffe : 0x00000002;
				}
				source = e386_get_esi(c);                                   /* Source Operand   */
				cs_segment_limit = c->cs_segment_register_descriptor_limit; /* CS Segment Limit */
				ds_segment_limit = c->ds_segment_register_descriptor_limit; /* DS Segment Limit */
				es_segment_limit = c->es_segment_register_descriptor_limit; /* ES Segment Limit */
				fs_segment_limit = c->fs_segment_register_descriptor_limit; /* FS Segment Limit */
				gs_segment_limit = c->gs_segment_register_descriptor_limit; /* GS Segment Limit */
				ss_segment_limit = c->ss_segment_register_descriptor_limit; /* SS Segment Limit */
				iopl = e386_get_iopl(c);
				cpl = c->current_privilege_level;
				if (((seg == e386_get_cs(c)) && (source > cs_segment_limit)) || ((seg == e386_get_ds(c)) && (source > ds_segment_limit)) || ((seg == e386_get_es(c)) && (source > es_segment_limit)))
				{
					/* General Protection Fault if outside of CS, DS, or ES segment limits */
					e86_trap(c, 13);
					c->fs_segment = 0;
					c->gs_segment = 0;
					c->e386_address_size_prefix = 1;
					c->e386_operand_size_prefix = 1;
					return (0);
				}
				if (((seg == e386_get_cs(c)) && (c->cs_segment_register_descriptor_available == 0)) || ((seg == e386_get_ds(c)) && (c->ds_segment_register_descriptor_available == 0)) || ((seg == e386_get_es(c)) && (c->es_segment_register_descriptor_available == 0)))
				{
					/* General Protection Fault if illegal memory operand effective address in CS, DS, or ES segments */
					e86_trap(c, 13);
					c->fs_segment = 0;
					c->gs_segment = 0;
					c->e386_address_size_prefix = 1;
					c->e386_operand_size_prefix = 1;
					return (0);
				}
				if ((seg == e386_get_ss(c)) && (source > ss_segment_limit))
				{
					/* Stack              Fault if outside of            SS segment limit  */
					e86_trap(c, 12);
					c->fs_segment = 0;
					c->gs_segment = 0;
					c->e386_address_size_prefix = 1;
					c->e386_operand_size_prefix = 1;
					return (0);
				}
				if ((seg == e386_get_ss(c)) && (c->ss_segment_register_descriptor_available == 0))
				{
					/* Stack              Fault if illegal memory operand effective address in            SS segment  */
					e86_trap(c, 12);
					c->fs_segment = 0;
					c->gs_segment = 0;
					c->e386_address_size_prefix = 1;
					c->e386_operand_size_prefix = 1;
					return (0);
				}
				/* General Protection Fault if Current Privilege Level is greater than I/O Privilege leval and */
				/* corresponding I/O permission bits in Task State Segment equals 1                            */
				if (c->prefix & (E86_PREFIX_REP | E86_PREFIX_REPN)) {
					while (e386_get_ecx(c) > 0) {
						e86_set_prt32(c, e386_get_edx(c), e386_get_mem32(c, seg, e386_get_esi(c)));
						e386_set_esi(c, e386_get_esi(c) + inc);
						e386_set_ecx(c, e386_get_ecx(c) - 1);
						c->instructions += 1;
						if (c->memory_clock_cycle_disable == 1)
						{
							/* External memory cycle */
							unsigned char memclk;
							memclk = c->memory_clock_cycle_count;
							e86_set_clk(c, memclk);
						}
						else
						{
							/* CPU memory cycle */
							if (cpl > iopl)
							{
								/* Current Privilege Level greater than I/O Privilege Level */
								e86_set_clk(c, 28);
							}
							else
							{
								/* Current Privilege Level greater than I/O Privilege Level */
								e86_set_clk(c, 8);
							}
						}
					}
				}
				else {
					e86_set_prt32(c, e386_get_edx(c), e386_get_mem32(c, seg, e386_get_esi(c)));
					e386_set_esi(c, e386_get_esi(c) + inc);
					if (c->memory_clock_cycle_disable == 1)
					{
						/* External memory cycle */
						unsigned char memclk;
						memclk = c->memory_clock_cycle_count;
						e86_set_clk(c, memclk);
					}
					else
					{
						/* CPU memory cycle */
						if (cpl > iopl)
						{
							/* Current Privilege Level greater than I/O Privilege Level */
							e86_set_clk(c, 28);
						}
						else
						{
							/* Current Privilege Level greater than I/O Privilege Level */
							e86_set_clk(c, 8);
						}
					}
				}
				c->fs_segment = 0;
				c->gs_segment = 0;
				c->e386_address_size_prefix = 1;
				c->e386_operand_size_prefix = 1;
				return (1);
			}
			else
			{
				/* 16-bit memory address operand specified */
				unsigned short seg;
				unsigned short inc;
				unsigned short source;
				unsigned long  cs_segment_limit, ds_segment_limit, es_segment_limit, fs_segment_limit, gs_segment_limit, ss_segment_limit;
				unsigned char  iopl, cpl;
				if (c->fs_segment == 1)
				{
					/* FS   segment   override   prefix   specified */
					seg = e386_get_fs(c);
				}
				else if (c->gs_segment == 1)
				{
					/* GS   segment   override   prefix   specified */
					seg = e386_get_gs(c);
				}
				else
				{
					/* CS, DS, ES, FS, GS, or SS segments specified */
					seg = e86_get_seg(c, E86_REG_DS);
				}
				if (c->e386_operand_size_prefix == 1)
				{
					/* 32-bit register operand specified */
					inc = e86_get_df(c) ? 0xfffe : 0x0004;
				}
				else
				{
					/* 16-bit register operand specified */
					inc = e86_get_df(c) ? 0xfffe : 0x0002;
				}
				source = e86_get_si(c);                                     /* Source Operand   */
				cs_segment_limit = c->cs_segment_register_descriptor_limit; /* CS Segment Limit */
				ds_segment_limit = c->ds_segment_register_descriptor_limit; /* DS Segment Limit */
				es_segment_limit = c->es_segment_register_descriptor_limit; /* ES Segment Limit */
				fs_segment_limit = c->fs_segment_register_descriptor_limit; /* FS Segment Limit */
				gs_segment_limit = c->gs_segment_register_descriptor_limit; /* GS Segment Limit */
				ss_segment_limit = c->ss_segment_register_descriptor_limit; /* SS Segment Limit */
				iopl = e386_get_iopl(c);
				cpl = c->current_privilege_level;
				if (((seg == e86_get_cs(c)) && (source > cs_segment_limit)) || ((seg == e86_get_ds(c)) && (source > ds_segment_limit)) || ((seg == e86_get_es(c)) && (source > es_segment_limit)))
				{
					/* General Protection Fault if outside of CS, DS, or ES segment limits */
					e86_trap(c, 13);
					c->fs_segment = 0;
					c->gs_segment = 0;
					c->e386_address_size_prefix = 1;
					c->e386_operand_size_prefix = 1;
					return (0);
				}
				if (((seg == e86_get_cs(c)) && (c->cs_segment_register_descriptor_available == 0)) || ((seg == e86_get_ds(c)) && (c->ds_segment_register_descriptor_available == 0)) || ((seg == e86_get_es(c)) && (c->es_segment_register_descriptor_available == 0)))
				{
					/* General Protection Fault if illegal memory operand effective address in CS, DS, or ES segments */
					e86_trap(c, 13);
					c->fs_segment = 0;
					c->gs_segment = 0;
					c->e386_address_size_prefix = 1;
					c->e386_operand_size_prefix = 1;
					return (0);
				}
				if ((seg == e86_get_ss(c)) && (source > ss_segment_limit))
				{
					/* Stack              Fault if outside of            SS segment limit  */
					e86_trap(c, 12);
					c->fs_segment = 0;
					c->gs_segment = 0;
					c->e386_address_size_prefix = 1;
					c->e386_operand_size_prefix = 1;
					return (0);
				}
				if ((seg == e86_get_ss(c)) && (c->ss_segment_register_descriptor_available == 0))
				{
					/* Stack              Fault if illegal memory operand effective address in            SS segment  */
					e86_trap(c, 12);
					c->fs_segment = 0;
					c->gs_segment = 0;
					c->e386_address_size_prefix = 1;
					c->e386_operand_size_prefix = 1;
					return (0);
				}
				/* General Protection Fault if Current Privilege Level is greater than I/O Privilege leval and */
				/* corresponding I/O permission bits in Task State Segment equals 1                            */
				if (c->prefix & (E86_PREFIX_REP | E86_PREFIX_REPN)) {
					while (e86_get_cx(c) > 0) {
						e86_set_prt16(c, e86_get_dx(c), e86_get_mem16(c, seg, e86_get_si(c)));
						e86_set_si(c, e86_get_si(c) + inc);
						e86_set_cx(c, e86_get_cx(c) - 1);
						c->instructions += 1;
						if (c->memory_clock_cycle_disable == 1)
						{
							/* External memory cycle */
							unsigned char memclk;
							memclk = c->memory_clock_cycle_count;
							e86_set_clk(c, memclk);
						}
						else
						{
							/* CPU memory cycle */
							if (cpl > iopl)
							{
								/* Current Privilege Level greater than I/O Privilege Level */
								e86_set_clk(c, 28);
							}
							else
							{
								/* Current Privilege Level greater than I/O Privilege Level */
								e86_set_clk(c, 8);
							}
						}
					}
				}
				else {
					e86_set_prt16(c, e86_get_dx(c), e86_get_mem16(c, seg, e86_get_si(c)));
					e86_set_si(c, e86_get_si(c) + inc);
					if (c->memory_clock_cycle_disable == 1)
					{
						/* External memory cycle */
						unsigned char memclk;
						memclk = c->memory_clock_cycle_count;
						e86_set_clk(c, memclk);
					}
					else
					{
						/* CPU memory cycle */
						if (cpl > iopl)
						{
							/* Current Privilege Level greater than I/O Privilege Level */
							e86_set_clk(c, 28);
						}
						else
						{
							/* Current Privilege Level greater than I/O Privilege Level */
							e86_set_clk(c, 8);
						}
					}
				}
				c->fs_segment = 0;
				c->gs_segment = 0;
				c->e386_address_size_prefix = 1;
				c->e386_operand_size_prefix = 1;
				return (1);
			}
			/* e386_protected_mode_op_6f(c); */
			/* return (1); */
		}
	}
	else {
		/* Intel 80186 CPU or  lower */
		unsigned short seg;
		unsigned short inc;
		seg = e86_get_seg(c, E86_REG_DS);
		inc = e86_get_df(c) ? 0xfffe : 0x0002;
		if (c->prefix & (E86_PREFIX_REP | E86_PREFIX_REPN)) {
			while (e86_get_cx(c) > 0) {
				e86_set_prt16(c, e86_get_dx(c), e86_get_mem16(c, seg, e86_get_si(c)));
				e86_set_si(c, e86_get_si(c) + inc);
				e86_set_cx(c, e86_get_cx(c) - 1);
				c->instructions += 1;
				if (c->memory_clock_cycle_disable == 1)
				{
					/* External memory cycle */
					unsigned char memclk;
					memclk = c->memory_clock_cycle_count;
					e86_set_clk(c, memclk);
				}
				else
				{
					/* CPU memory cycle */
					e86_set_clk(c, 8);
				}
			}
		}
		else {
			e86_set_prt16(c, e86_get_dx(c), e86_get_mem16(c, seg, e86_get_si(c)));
			e86_set_si(c, e86_get_si(c) + inc);
			if (c->memory_clock_cycle_disable == 1)
			{
				/* External memory cycle */
				unsigned char memclk;
				memclk = c->memory_clock_cycle_count;
				e86_set_clk(c, memclk);
			}
			else
			{
				/* CPU memory cycle */
				e86_set_clk(c, 8);
			}
		}
		return (1);
	}
}

/* OP C0: ROL, ROR, RCL, RCR, SHL, SHR, SAR r/m8, imm8 (Real Mode) */
/* static
unsigned e386_real_mode_op_c0(e8086_t *c)
{
	unsigned       xop;
	unsigned       cnt;
	unsigned short d, s;
	xop = (c->pq[1] >> 3) & 7;
	e86_get_ea_ptr(c, c->pq + 1);
	s = e86_get_ea8(c);
	cnt = c->pq[c->ea.cnt + 1];
	if (c->cpu & E86_CPU_MASK_SHIFT) {
		cnt &= 0x1f;
	}
	if (cnt == 0) {
		c->e386_address_size_prefix = 0;
		c->e386_operand_size_prefix = 0;
		return (c->ea.cnt + 1);
	}
	switch (xop) {
	case 0: /* ROL r/m8, imm8 */
/* 		d = (s << (cnt & 7)) | (s >> (8 - (cnt & 7)));
		e86_set_cf(c, d & 1);
		e86_set_of(c, ((s << 1) ^ s) & 0x80);
		break;
	case 1: /* ROR r/m8, imm8 */
/* 		d = (s >> (cnt & 7)) | (s << (8 - (cnt & 7)));
		e86_set_cf(c, d & 0x80);
		e86_set_of(c, ((d << 1) ^ s) & 0x80);
		break;
	case 2: /* RCL r/m8, imm8 */
/* 		s |= e86_get_cf(c) << 8;
		d = (s << (cnt % 9)) | (s >> (9 - (cnt % 9)));
		e86_set_cf(c, d & 0x100);
		e86_set_of(c, ((s << 1) ^ s) & 0x80);
		break;
	case 3: /* RCR r/m8, imm8 */
/* 		s |= e86_get_cf(c) << 8;
		d = (s >> (cnt % 9)) | (s << (9 - (cnt % 9)));
		e86_set_cf(c, d & 0x100);
		e86_set_of(c, ((d << 1) ^ s) & 0x80);
		break;
	case 4: /* SHL r/m8, imm8 */
/* 		d = (cnt > 8) ? 0 : (s << cnt);
		e86_set_flg_szp_8(c, d);
		e86_set_cf(c, d & 0x100);
		e86_set_of(c, ((s << 1) ^ s) & 0x80);
		break;
	case 5: /* SHR r/m8, imm8 */
			/* c > 0 */
/* 		d = (cnt > 8) ? 0 : (s >> (cnt - 1));
		e86_set_cf(c, d & 1);
		d = d >> 1;
		e86_set_flg_szp_8(c, d);
		e86_set_of(c, s & 0x80);
		break;
	case 7: /* SAR r/m8, imm8 */
/* 		s |= (s & 0x80) ? 0xff00 : 0x0000;
		d = s >> ((cnt >= 8) ? 7 : (cnt - 1));
		e86_set_cf(c, d & 1);
		d = (d >> 1) & 0xff;
		e86_set_flg_szp_8(c, d);
		e86_set_of(c, 0);
		break;
	default:
		d = 0; /* To avoid compiler warning */
/* 		break;
	}
	e86_set_ea8(c, d & 0xff);
	if (c->memory_clock_cycle_disable == 1)
	{
		/* External memory cycle */
/* 		unsigned char memclk;
		memclk = c->memory_clock_cycle_count;
		e86_set_clk(c, memclk);
	}
	else
	{
		/* CPU memory cycle */
/* 		e86_set_clk_ea(c, 5 + cnt, 17 + cnt);
	}
	c->e386_address_size_prefix = 0;
	c->e386_operand_size_prefix = 0;
	return (c->ea.cnt + 2);
} */

/* OP C0: ROL, ROR, RCL, RCR, SHL, SHR, SAR r/m8, imm8 (Virtual 8086 Mode) */
/* static
unsigned e386_virtual_8086_mode_op_c0(e8086_t *c)
{
unsigned       xop;
unsigned       cnt;
unsigned short d, s;
xop = (c->pq[1] >> 3) & 7;
e86_get_ea_ptr(c, c->pq + 1);
s = e86_get_ea8(c);
cnt = c->pq[c->ea.cnt + 1];
if (c->cpu & E86_CPU_MASK_SHIFT) {
cnt &= 0x1f;
}
if (cnt == 0) {
c->e386_address_size_prefix = 0;
c->e386_operand_size_prefix = 0;
return (c->ea.cnt + 1);
}
switch (xop) {
case 0: /* ROL r/m8, imm8 */
/* 		d = (s << (cnt & 7)) | (s >> (8 - (cnt & 7)));
e86_set_cf(c, d & 1);
e86_set_of(c, ((s << 1) ^ s) & 0x80);
break;
case 1: /* ROR r/m8, imm8 */
/* 		d = (s >> (cnt & 7)) | (s << (8 - (cnt & 7)));
e86_set_cf(c, d & 0x80);
e86_set_of(c, ((d << 1) ^ s) & 0x80);
break;
case 2: /* RCL r/m8, imm8 */
/* 		s |= e86_get_cf(c) << 8;
d = (s << (cnt % 9)) | (s >> (9 - (cnt % 9)));
e86_set_cf(c, d & 0x100);
e86_set_of(c, ((s << 1) ^ s) & 0x80);
break;
case 3: /* RCR r/m8, imm8 */
/* 		s |= e86_get_cf(c) << 8;
d = (s >> (cnt % 9)) | (s << (9 - (cnt % 9)));
e86_set_cf(c, d & 0x100);
e86_set_of(c, ((d << 1) ^ s) & 0x80);
break;
case 4: /* SHL r/m8, imm8 */
/* 		d = (cnt > 8) ? 0 : (s << cnt);
e86_set_flg_szp_8(c, d);
e86_set_cf(c, d & 0x100);
e86_set_of(c, ((s << 1) ^ s) & 0x80);
break;
case 5: /* SHR r/m8, imm8 */
/* c > 0 */
/* 		d = (cnt > 8) ? 0 : (s >> (cnt - 1));
e86_set_cf(c, d & 1);
d = d >> 1;
e86_set_flg_szp_8(c, d);
e86_set_of(c, s & 0x80);
break;
case 7: /* SAR r/m8, imm8 */
/* 		s |= (s & 0x80) ? 0xff00 : 0x0000;
d = s >> ((cnt >= 8) ? 7 : (cnt - 1));
e86_set_cf(c, d & 1);
d = (d >> 1) & 0xff;
e86_set_flg_szp_8(c, d);
e86_set_of(c, 0);
break;
default:
d = 0; /* To avoid compiler warning */
/* 		break;
}
e86_set_ea8(c, d & 0xff);
if (c->memory_clock_cycle_disable == 1)
{
/* External memory cycle */
/* 		unsigned char memclk;
memclk = c->memory_clock_cycle_count;
e86_set_clk(c, memclk);
}
else
{
/* CPU memory cycle */
/* 		e86_set_clk_ea(c, 5 + cnt, 17 + cnt);
}
c->e386_address_size_prefix = 0;
c->e386_operand_size_prefix = 0;
return (c->ea.cnt + 2);
} */

/* OP C0: ROL, ROR, RCL, RCR, SHL, SHR, SAR r/m8, imm8 (Protected Mode) */
/* static
unsigned e386_protected_mode_op_c0(e8086_t *c)
{
unsigned       xop;
unsigned       cnt;
unsigned short d, s;
xop = (c->pq[1] >> 3) & 7;
e86_get_ea_ptr(c, c->pq + 1);
s = e86_get_ea8(c);
cnt = c->pq[c->ea.cnt + 1];
if (c->cpu & E86_CPU_MASK_SHIFT) {
cnt &= 0x1f;
}
if (cnt == 0) {
c->e386_address_size_prefix = 0;
c->e386_operand_size_prefix = 0;
return (c->ea.cnt + 1);
}
switch (xop) {
case 0: /* ROL r/m8, imm8 */
/* 		d = (s << (cnt & 7)) | (s >> (8 - (cnt & 7)));
e86_set_cf(c, d & 1);
e86_set_of(c, ((s << 1) ^ s) & 0x80);
break;
case 1: /* ROR r/m8, imm8 */
/* 		d = (s >> (cnt & 7)) | (s << (8 - (cnt & 7)));
e86_set_cf(c, d & 0x80);
e86_set_of(c, ((d << 1) ^ s) & 0x80);
break;
case 2: /* RCL r/m8, imm8 */
/* 		s |= e86_get_cf(c) << 8;
d = (s << (cnt % 9)) | (s >> (9 - (cnt % 9)));
e86_set_cf(c, d & 0x100);
e86_set_of(c, ((s << 1) ^ s) & 0x80);
break;
case 3: /* RCR r/m8, imm8 */
/* 		s |= e86_get_cf(c) << 8;
d = (s >> (cnt % 9)) | (s << (9 - (cnt % 9)));
e86_set_cf(c, d & 0x100);
e86_set_of(c, ((d << 1) ^ s) & 0x80);
break;
case 4: /* SHL r/m8, imm8 */
/* 		d = (cnt > 8) ? 0 : (s << cnt);
e86_set_flg_szp_8(c, d);
e86_set_cf(c, d & 0x100);
e86_set_of(c, ((s << 1) ^ s) & 0x80);
break;
case 5: /* SHR r/m8, imm8 */
/* c > 0 */
/* 		d = (cnt > 8) ? 0 : (s >> (cnt - 1));
e86_set_cf(c, d & 1);
d = d >> 1;
e86_set_flg_szp_8(c, d);
e86_set_of(c, s & 0x80);
break;
case 7: /* SAR r/m8, imm8 */
/* 		s |= (s & 0x80) ? 0xff00 : 0x0000;
d = s >> ((cnt >= 8) ? 7 : (cnt - 1));
e86_set_cf(c, d & 1);
d = (d >> 1) & 0xff;
e86_set_flg_szp_8(c, d);
e86_set_of(c, 0);
break;
default:
d = 0; /* To avoid compiler warning */
/* 		break;
}
e86_set_ea8(c, d & 0xff);
if (c->memory_clock_cycle_disable == 1)
{
/* External memory cycle */
/* 		unsigned char memclk;
memclk = c->memory_clock_cycle_count;
e86_set_clk(c, memclk);
}
else
{
/* CPU memory cycle */
/* 		e86_set_clk_ea(c, 5 + cnt, 17 + cnt);
}
c->e386_address_size_prefix = 0;
c->e386_operand_size_prefix = 0;
return (c->ea.cnt + 2);
} */

/* OP C0: ROL, ROR, RCL, RCR, SHL, SHR, SAR r/m8, imm8 */
static
unsigned op_c0 (e8086_t *c)
{
	if (c->cpumodel >= 0x80386) {
		/* Intel 80386 CPU or higher */
		e286_get_machine_status_word(c);
		e386_get_flags(c);
		if ((e286_get_pe(c) == 0) && (e386_get_vm(c) == 0))
		{
			/* Real Mode */
			unsigned       xop;
			unsigned       cnt;
			unsigned short d, s;
			xop = (c->pq[1] >> 3) & 7;
			g386_real_mode_get_ea_ptr(c, c->pq + 1);
			s = g386_real_mode_get_ea8(c);
			cnt = c->pq[c->ea.cnt + 1];
			if (c->cpu & E86_CPU_MASK_SHIFT) {
				cnt &= 0x1f;
			}
			if (cnt == 0) {
				c->e386_address_size_prefix = 1;
				c->e386_operand_size_prefix = 1;
				return (c->ea.cnt + 1);
			}
			switch (xop) {
			case 0: /* ROL r/m8, imm8 */
				d = (s << (cnt & 7)) | (s >> (8 - (cnt & 7)));
				e86_set_cf(c, d & 1);
				e86_set_of(c, ((s << 1) ^ s) & 0x80);
				break;
			case 1: /* ROR r/m8, imm8 */
				d = (s >> (cnt & 7)) | (s << (8 - (cnt & 7)));
				e86_set_cf(c, d & 0x80);
				e86_set_of(c, ((d << 1) ^ s) & 0x80);
				break;
			case 2: /* RCL r/m8, imm8 */
				s |= e86_get_cf(c) << 8;
				d = (s << (cnt % 9)) | (s >> (9 - (cnt % 9)));
				e86_set_cf(c, d & 0x100);
				e86_set_of(c, ((s << 1) ^ s) & 0x80);
				break;
			case 3: /* RCR r/m8, imm8 */
				s |= e86_get_cf(c) << 8;
				d = (s >> (cnt % 9)) | (s << (9 - (cnt % 9)));
				e86_set_cf(c, d & 0x100);
				e86_set_of(c, ((d << 1) ^ s) & 0x80);
				break;
			case 4: /* SHL r/m8, imm8 */
				d = (cnt > 8) ? 0 : (s << cnt);
				e86_set_flg_szp_8(c, d);
				e86_set_cf(c, d & 0x100);
				e86_set_of(c, ((s << 1) ^ s) & 0x80);
				break;
			case 5: /* SHR r/m8, imm8 */
					/* c > 0 */
				d = (cnt > 8) ? 0 : (s >> (cnt - 1));
				e86_set_cf(c, d & 1);
				d = d >> 1;
				e86_set_flg_szp_8(c, d);
				e86_set_of(c, s & 0x80);
				break;
			case 7: /* SAR r/m8, imm8 */
				s |= (s & 0x80) ? 0xff00 : 0x0000;
				d = s >> ((cnt >= 8) ? 7 : (cnt - 1));
				e86_set_cf(c, d & 1);
				d = (d >> 1) & 0xff;
				e86_set_flg_szp_8(c, d);
				e86_set_of(c, 0);
				break;
			default:
				d = 0; /* To avoid compiler warning */
				break;
			}
			g386_real_mode_set_ea8(c, d & 0xff);
			if (c->memory_clock_cycle_disable == 1)
			{
				/* External memory cycle */
				unsigned char memclk;
				memclk = c->memory_clock_cycle_count;
				e86_set_clk(c, memclk);
			}
			else
			{
				/* CPU memory cycle */
				e86_set_clk_ea(c, 3 + cnt, 7 + cnt);
			}
			c->e386_address_size_prefix = 1;
			c->e386_operand_size_prefix = 1;
			return (c->ea.cnt + 2);
			/* e386_real_mode_op_c0(c); */
			/* return (c->ea.cnt + 2); */
		}
		else if (e386_get_vm(c) == 1)
		{
			/* Virtual 8086 Mode */
			unsigned       xop;
			unsigned       cnt;
			unsigned short d, s;
			xop = (c->pq[1] >> 3) & 7;
			g386_virtual_8086_mode_get_ea_ptr(c, c->pq + 1);
			s = g386_virtual_8086_mode_get_ea8(c);
			cnt = c->pq[c->ea.cnt + 1];
			if (c->cpu & E86_CPU_MASK_SHIFT) {
				cnt &= 0x1f;
			}
			if (cnt == 0) {
				c->e386_address_size_prefix = 1;
				c->e386_operand_size_prefix = 1;
				return (c->ea.cnt + 1);
			}
			switch (xop) {
			case 0: /* ROL r/m8, imm8 */
				d = (s << (cnt & 7)) | (s >> (8 - (cnt & 7)));
				e86_set_cf(c, d & 1);
				e86_set_of(c, ((s << 1) ^ s) & 0x80);
				break;
			case 1: /* ROR r/m8, imm8 */
				d = (s >> (cnt & 7)) | (s << (8 - (cnt & 7)));
				e86_set_cf(c, d & 0x80);
				e86_set_of(c, ((d << 1) ^ s) & 0x80);
				break;
			case 2: /* RCL r/m8, imm8 */
				s |= e86_get_cf(c) << 8;
				d = (s << (cnt % 9)) | (s >> (9 - (cnt % 9)));
				e86_set_cf(c, d & 0x100);
				e86_set_of(c, ((s << 1) ^ s) & 0x80);
				break;
			case 3: /* RCR r/m8, imm8 */
				s |= e86_get_cf(c) << 8;
				d = (s >> (cnt % 9)) | (s << (9 - (cnt % 9)));
				e86_set_cf(c, d & 0x100);
				e86_set_of(c, ((d << 1) ^ s) & 0x80);
				break;
			case 4: /* SHL r/m8, imm8 */
				d = (cnt > 8) ? 0 : (s << cnt);
				e86_set_flg_szp_8(c, d);
				e86_set_cf(c, d & 0x100);
				e86_set_of(c, ((s << 1) ^ s) & 0x80);
				break;
			case 5: /* SHR r/m8, imm8 */
					/* c > 0 */
				d = (cnt > 8) ? 0 : (s >> (cnt - 1));
				e86_set_cf(c, d & 1);
				d = d >> 1;
				e86_set_flg_szp_8(c, d);
				e86_set_of(c, s & 0x80);
				break;
			case 7: /* SAR r/m8, imm8 */
				s |= (s & 0x80) ? 0xff00 : 0x0000;
				d = s >> ((cnt >= 8) ? 7 : (cnt - 1));
				e86_set_cf(c, d & 1);
				d = (d >> 1) & 0xff;
				e86_set_flg_szp_8(c, d);
				e86_set_of(c, 0);
				break;
			default:
				d = 0; /* To avoid compiler warning */
				break;
			}
			g386_virtual_8086_mode_set_ea8(c, d & 0xff);
			if (c->memory_clock_cycle_disable == 1)
			{
				/* External memory cycle */
				unsigned char memclk;
				memclk = c->memory_clock_cycle_count;
				e86_set_clk(c, memclk);
			}
			else
			{
				/* CPU memory cycle */
				e86_set_clk_ea(c, 3 + cnt, 7 + cnt);
			}
			c->e386_address_size_prefix = 1;
			c->e386_operand_size_prefix = 1;
			return (c->ea.cnt + 2);
			/* e386_virtual_8086_mode_op_c0(c); */
			/* return (c->ea.cnt + 2); */
		}
		else
		{
			/* Protected Mode */
			unsigned       xop;
			unsigned       cnt;
			unsigned short d, s;
			xop = (c->pq[1] >> 3) & 7;
			g386_protected_mode_get_ea_ptr(c, c->pq + 1);
			s = g386_protected_mode_get_ea8(c);
			cnt = c->pq[c->ea.cnt + 1];
			if (c->cpu & E86_CPU_MASK_SHIFT) {
				cnt &= 0x1f;
			}
			if (cnt == 0) {
				c->e386_address_size_prefix = 1;
				c->e386_operand_size_prefix = 1;
				return (c->ea.cnt + 1);
			}
			switch (xop) {
			case 0: /* ROL r/m8, imm8 */
				d = (s << (cnt & 7)) | (s >> (8 - (cnt & 7)));
				e86_set_cf(c, d & 1);
				e86_set_of(c, ((s << 1) ^ s) & 0x80);
				break;
			case 1: /* ROR r/m8, imm8 */
				d = (s >> (cnt & 7)) | (s << (8 - (cnt & 7)));
				e86_set_cf(c, d & 0x80);
				e86_set_of(c, ((d << 1) ^ s) & 0x80);
				break;
			case 2: /* RCL r/m8, imm8 */
				s |= e86_get_cf(c) << 8;
				d = (s << (cnt % 9)) | (s >> (9 - (cnt % 9)));
				e86_set_cf(c, d & 0x100);
				e86_set_of(c, ((s << 1) ^ s) & 0x80);
				break;
			case 3: /* RCR r/m8, imm8 */
				s |= e86_get_cf(c) << 8;
				d = (s >> (cnt % 9)) | (s << (9 - (cnt % 9)));
				e86_set_cf(c, d & 0x100);
				e86_set_of(c, ((d << 1) ^ s) & 0x80);
				break;
			case 4: /* SHL r/m8, imm8 */
				d = (cnt > 8) ? 0 : (s << cnt);
				e86_set_flg_szp_8(c, d);
				e86_set_cf(c, d & 0x100);
				e86_set_of(c, ((s << 1) ^ s) & 0x80);
				break;
			case 5: /* SHR r/m8, imm8 */
					/* c > 0 */
				d = (cnt > 8) ? 0 : (s >> (cnt - 1));
				e86_set_cf(c, d & 1);
				d = d >> 1;
				e86_set_flg_szp_8(c, d);
				e86_set_of(c, s & 0x80);
				break;
			case 7: /* SAR r/m8, imm8 */
				s |= (s & 0x80) ? 0xff00 : 0x0000;
				d = s >> ((cnt >= 8) ? 7 : (cnt - 1));
				e86_set_cf(c, d & 1);
				d = (d >> 1) & 0xff;
				e86_set_flg_szp_8(c, d);
				e86_set_of(c, 0);
				break;
			default:
				d = 0; /* To avoid compiler warning */
				break;
			}
			g386_protected_mode_set_ea8(c, d & 0xff);
			if (c->memory_clock_cycle_disable == 1)
			{
				/* External memory cycle */
				unsigned char memclk;
				memclk = c->memory_clock_cycle_count;
				e86_set_clk(c, memclk);
			}
			else
			{
				/* CPU memory cycle */
				e86_set_clk_ea(c, 3 + cnt, 7 + cnt);
			}
			c->e386_address_size_prefix = 1;
			c->e386_operand_size_prefix = 1;
			return (c->ea.cnt + 2);
			/* e386_protected_mode_op_c0(c); */
			/* return (c->ea.cnt + 2); */
		}
	}
	else {
		/* Intel 80186 CPU or  lower */
		unsigned       xop;
		unsigned       cnt;
		unsigned short d, s;
		xop = (c->pq[1] >> 3) & 7;
		e86_get_ea_ptr(c, c->pq + 1);
		s = e86_get_ea8(c);
		cnt = c->pq[c->ea.cnt + 1];
		if (c->cpu & E86_CPU_MASK_SHIFT) {
			cnt &= 0x1f;
		}
		if (cnt == 0) {
			return (c->ea.cnt + 1);
		}
		switch (xop) {
		case 0: /* ROL r/m8, imm8 */
			d = (s << (cnt & 7)) | (s >> (8 - (cnt & 7)));
			e86_set_cf(c, d & 1);
			e86_set_of(c, ((s << 1) ^ s) & 0x80);
			break;
		case 1: /* ROR r/m8, imm8 */
			d = (s >> (cnt & 7)) | (s << (8 - (cnt & 7)));
			e86_set_cf(c, d & 0x80);
			e86_set_of(c, ((d << 1) ^ s) & 0x80);
			break;
		case 2: /* RCL r/m8, imm8 */
			s |= e86_get_cf(c) << 8;
			d = (s << (cnt % 9)) | (s >> (9 - (cnt % 9)));
			e86_set_cf(c, d & 0x100);
			e86_set_of(c, ((s << 1) ^ s) & 0x80);
			break;
		case 3: /* RCR r/m8, imm8 */
			s |= e86_get_cf(c) << 8;
			d = (s >> (cnt % 9)) | (s << (9 - (cnt % 9)));
			e86_set_cf(c, d & 0x100);
			e86_set_of(c, ((d << 1) ^ s) & 0x80);
			break;
		case 4: /* SHL r/m8, imm8 */
			d = (cnt > 8) ? 0 : (s << cnt);
			e86_set_flg_szp_8(c, d);
			e86_set_cf(c, d & 0x100);
			e86_set_of(c, ((s << 1) ^ s) & 0x80);
			break;
		case 5: /* SHR r/m8, imm8 */
				/* c > 0 */
			d = (cnt > 8) ? 0 : (s >> (cnt - 1));
			e86_set_cf(c, d & 1);
			d = d >> 1;
			e86_set_flg_szp_8(c, d);
			e86_set_of(c, s & 0x80);
			break;
		case 7: /* SAR r/m8, imm8 */
			s |= (s & 0x80) ? 0xff00 : 0x0000;
			d = s >> ((cnt >= 8) ? 7 : (cnt - 1));
			e86_set_cf(c, d & 1);
			d = (d >> 1) & 0xff;
			e86_set_flg_szp_8(c, d);
			e86_set_of(c, 0);
			break;
		default:
			d = 0; /* To avoid compiler warning */
			break;
		}
		e86_set_ea8(c, d & 0xff);
		if (c->memory_clock_cycle_disable == 1)
		{
			/* External memory cycle */
			unsigned char memclk;
			memclk = c->memory_clock_cycle_count;
			e86_set_clk(c, memclk);
		}
		else
		{
			/* CPU memory cycle */
			e86_set_clk_ea(c, 5 + cnt, 17 + cnt);
		}
		return (c->ea.cnt + 2);
	}
}

/* OP C1: ROL, ROR, RCL, RCR, SHL, SHR, SAR r/m16, imm8 or ROL, ROR, RCL, RCR, SHL, SHR, SAR r/m32, imm8 (Real Mode) */
/* static
unsigned e386_real_mode_op_c1(e8086_t *c)
{
	unsigned      xop;
	unsigned      cnt;
	unsigned long d, s;
	xop = (c->pq[1] >> 3) & 7;
	e86_get_ea_ptr(c, c->pq + 1);
	s = e86_get_ea16(c);
	cnt = c->pq[c->ea.cnt + 1];
	if (c->cpu & E86_CPU_MASK_SHIFT) {
		cnt &= 0x1f;
	}
	if (cnt == 0) {
		c->e386_address_size_prefix = 0;
		c->e386_operand_size_prefix = 0;
		return (c->ea.cnt + 1);
	}
	switch (xop) {
	case 0: /* ROL r/m16, imm8 */
/* 		d = (s << (cnt & 15)) | (s >> (16 - (cnt & 15)));
		e86_set_cf(c, d & 1);
		e86_set_of(c, ((s << 1) ^ s) & 0x8000);
		break;
	case 1: /* ROR r/m16, imm8 */
/* 		d = (s >> (cnt & 15)) | (s << (16 - (cnt & 15)));
		e86_set_cf(c, d & 0x8000);
		e86_set_of(c, ((d << 1) ^ s) & 0x8000);
		break;
	case 2: /* RCL r/m16, imm8 */
/* 		s |= e86_get_cf(c) << 16;
		d = (s << (cnt % 17)) | (s >> (17 - (cnt % 17)));
		e86_set_cf(c, d & 0x10000);
		e86_set_of(c, ((s << 1) ^ s) & 0x8000);
		break;
	case 3: /* RCR r/m16, imm8 */
/* 		s |= e86_get_cf(c) << 16;
		d = (s >> (cnt % 17)) | (s << (17 - (cnt % 17)));
		e86_set_cf(c, d & 0x10000);
		e86_set_of(c, ((d << 1) ^ s) & 0x8000);
		break;
	case 4: /* SHL r/m16, imm8 */
/* 		d = (cnt > 16) ? 0 : (s << cnt);
		e86_set_flg_szp_16(c, d);
		e86_set_cf(c, d & 0x10000);
		e86_set_of(c, ((s << 1) ^ s) & 0x8000);
		break;
	case 5: /* SHR r/m16, imm8 */
			/* c > 0 */
/* 		d = (cnt > 16) ? 0 : (s >> (cnt - 1));
		e86_set_cf(c, d & 1);
		d = d >> 1;
		e86_set_flg_szp_16(c, d);
		e86_set_of(c, s & 0x8000);
		break;
	case 7: /* SAR r/m16, imm8 */
/* 		s |= (s & 0x8000) ? 0xffff0000 : 0x00000000;
		d = s >> ((cnt >= 16) ? 15 : (cnt - 1));
		e86_set_cf(c, d & 1);
		d = (d >> 1) & 0xffff;
		e86_set_flg_szp_16(c, d);
		e86_set_of(c, 0);
		break;
	default:
		d = 0; /* To avoid compiler warning */
/* 		break;
	}
	e86_set_ea16(c, d);
	if (c->memory_clock_cycle_disable == 1)
	{
		/* External memory cycle */
/* 		unsigned char memclk;
		memclk = c->memory_clock_cycle_count;
		e86_set_clk(c, memclk);
	}
	else
	{
		/* CPU memory cycle */
/* 		e86_set_clk_ea(c, 5 + cnt, 17 + cnt);
	}
	c->e386_address_size_prefix = 0;
	c->e386_operand_size_prefix = 0;
	return (c->ea.cnt + 2);
} */

/* OP C1: ROL, ROR, RCL, RCR, SHL, SHR, SAR r/m16, imm8 or ROL, ROR, RCL, RCR, SHL, SHR, SAR r/m32, imm8 (Virtual 8086 Mode) */
/* static
unsigned e386_virtual_8086_mode_op_c1(e8086_t *c)
{
unsigned      xop;
unsigned      cnt;
unsigned long d, s;
xop = (c->pq[1] >> 3) & 7;
e86_get_ea_ptr(c, c->pq + 1);
s = e86_get_ea16(c);
cnt = c->pq[c->ea.cnt + 1];
if (c->cpu & E86_CPU_MASK_SHIFT) {
cnt &= 0x1f;
}
if (cnt == 0) {
c->e386_address_size_prefix = 0;
c->e386_operand_size_prefix = 0;
return (c->ea.cnt + 1);
}
switch (xop) {
case 0: /* ROL r/m16, imm8 */
/* 		d = (s << (cnt & 15)) | (s >> (16 - (cnt & 15)));
e86_set_cf(c, d & 1);
e86_set_of(c, ((s << 1) ^ s) & 0x8000);
break;
case 1: /* ROR r/m16, imm8 */
/* 		d = (s >> (cnt & 15)) | (s << (16 - (cnt & 15)));
e86_set_cf(c, d & 0x8000);
e86_set_of(c, ((d << 1) ^ s) & 0x8000);
break;
case 2: /* RCL r/m16, imm8 */
/* 		s |= e86_get_cf(c) << 16;
d = (s << (cnt % 17)) | (s >> (17 - (cnt % 17)));
e86_set_cf(c, d & 0x10000);
e86_set_of(c, ((s << 1) ^ s) & 0x8000);
break;
case 3: /* RCR r/m16, imm8 */
/* 		s |= e86_get_cf(c) << 16;
d = (s >> (cnt % 17)) | (s << (17 - (cnt % 17)));
e86_set_cf(c, d & 0x10000);
e86_set_of(c, ((d << 1) ^ s) & 0x8000);
break;
case 4: /* SHL r/m16, imm8 */
/* 		d = (cnt > 16) ? 0 : (s << cnt);
e86_set_flg_szp_16(c, d);
e86_set_cf(c, d & 0x10000);
e86_set_of(c, ((s << 1) ^ s) & 0x8000);
break;
case 5: /* SHR r/m16, imm8 */
/* c > 0 */
/* 		d = (cnt > 16) ? 0 : (s >> (cnt - 1));
e86_set_cf(c, d & 1);
d = d >> 1;
e86_set_flg_szp_16(c, d);
e86_set_of(c, s & 0x8000);
break;
case 7: /* SAR r/m16, imm8 */
/* 		s |= (s & 0x8000) ? 0xffff0000 : 0x00000000;
d = s >> ((cnt >= 16) ? 15 : (cnt - 1));
e86_set_cf(c, d & 1);
d = (d >> 1) & 0xffff;
e86_set_flg_szp_16(c, d);
e86_set_of(c, 0);
break;
default:
d = 0; /* To avoid compiler warning */
/* 		break;
}
e86_set_ea16(c, d);
if (c->memory_clock_cycle_disable == 1)
{
/* External memory cycle */
/* 		unsigned char memclk;
memclk = c->memory_clock_cycle_count;
e86_set_clk(c, memclk);
}
else
{
/* CPU memory cycle */
/* 		e86_set_clk_ea(c, 5 + cnt, 17 + cnt);
}
c->e386_address_size_prefix = 0;
c->e386_operand_size_prefix = 0;
return (c->ea.cnt + 2);

/* OP C1: ROL, ROR, RCL, RCR, SHL, SHR, SAR r/m16, imm8 or ROL, ROR, RCL, RCR, SHL, SHR, SAR r/m32, imm8 (Protected Mode) */
/* static
unsigned e386_protected_mode_op_c1(e8086_t *c)
{
unsigned      xop;
unsigned      cnt;
unsigned long d, s;
xop = (c->pq[1] >> 3) & 7;
e86_get_ea_ptr(c, c->pq + 1);
s = e86_get_ea16(c);
cnt = c->pq[c->ea.cnt + 1];
if (c->cpu & E86_CPU_MASK_SHIFT) {
cnt &= 0x1f;
}
if (cnt == 0) {
c->e386_address_size_prefix = 0;
c->e386_operand_size_prefix = 0;
return (c->ea.cnt + 1);
}
switch (xop) {
case 0: /* ROL r/m16, imm8 */
/* 		d = (s << (cnt & 15)) | (s >> (16 - (cnt & 15)));
e86_set_cf(c, d & 1);
e86_set_of(c, ((s << 1) ^ s) & 0x8000);
break;
case 1: /* ROR r/m16, imm8 */
/* 		d = (s >> (cnt & 15)) | (s << (16 - (cnt & 15)));
e86_set_cf(c, d & 0x8000);
e86_set_of(c, ((d << 1) ^ s) & 0x8000);
break;
case 2: /* RCL r/m16, imm8 */
/* 		s |= e86_get_cf(c) << 16;
d = (s << (cnt % 17)) | (s >> (17 - (cnt % 17)));
e86_set_cf(c, d & 0x10000);
e86_set_of(c, ((s << 1) ^ s) & 0x8000);
break;
case 3: /* RCR r/m16, imm8 */
/* 		s |= e86_get_cf(c) << 16;
d = (s >> (cnt % 17)) | (s << (17 - (cnt % 17)));
e86_set_cf(c, d & 0x10000);
e86_set_of(c, ((d << 1) ^ s) & 0x8000);
break;
case 4: /* SHL r/m16, imm8 */
/* 		d = (cnt > 16) ? 0 : (s << cnt);
e86_set_flg_szp_16(c, d);
e86_set_cf(c, d & 0x10000);
e86_set_of(c, ((s << 1) ^ s) & 0x8000);
break;
case 5: /* SHR r/m16, imm8 */
/* c > 0 */
/* 		d = (cnt > 16) ? 0 : (s >> (cnt - 1));
e86_set_cf(c, d & 1);
d = d >> 1;
e86_set_flg_szp_16(c, d);
e86_set_of(c, s & 0x8000);
break;
case 7: /* SAR r/m16, imm8 */
/* 		s |= (s & 0x8000) ? 0xffff0000 : 0x00000000;
d = s >> ((cnt >= 16) ? 15 : (cnt - 1));
e86_set_cf(c, d & 1);
d = (d >> 1) & 0xffff;
e86_set_flg_szp_16(c, d);
e86_set_of(c, 0);
break;
default:
d = 0; /* To avoid compiler warning */
/* 		break;
}
e86_set_ea16(c, d);
if (c->memory_clock_cycle_disable == 1)
{
/* External memory cycle */
/* 		unsigned char memclk;
memclk = c->memory_clock_cycle_count;
e86_set_clk(c, memclk);
}
else
{
/* CPU memory cycle */
/* 		e86_set_clk_ea(c, 5 + cnt, 17 + cnt);
}
c->e386_address_size_prefix = 0;
c->e386_operand_size_prefix = 0;
return (c->ea.cnt + 2);

/* OP C1: ROL, ROR, RCL, RCR, SHL, SHR, SAR r/m16, imm8 */
static
unsigned op_c1 (e8086_t *c)
{
	if (c->cpumodel >= 0x80386) {
		/* Intel 80386 CPU or higher */
		e286_get_machine_status_word(c);
		e386_get_flags(c);
		if ((e286_get_pe(c) == 0) && (e386_get_vm(c) == 0))
		{
			/* Real Mode */
			unsigned      xop;
			unsigned      cnt;
			if (c->e386_address_size_prefix == 1)
			{
				/* 32-bit memory address operand specified */
				unsigned long long d, s;
				xop = (c->pq[1] >> 3) & 7;
				g386_real_mode_get_ea_ptr32(c, c->pq + 1);
				s = g386_real_mode_get_ea32(c);
				cnt = c->pq[c->ea.cnt + 1];
				if (c->cpu & E86_CPU_MASK_SHIFT) {
					cnt &= 0x1f;
				}
				if (cnt == 0) {
					return (c->ea.cnt + 1);
				}
				switch (xop) {
				case 0: /* ROL r/m32, imm8 */
					d = (s << (cnt & 31)) | (s >> (32 - (cnt & 31)));
					e86_set_cf(c, d & 1);
					e86_set_of(c, ((s << 1) ^ s) & 0x80000000);
					break;
				case 1: /* ROR r/m32, imm8 */
					d = (s >> (cnt & 31)) | (s << (32 - (cnt & 31)));
					e86_set_cf(c, d & 0x80000000);
					e86_set_of(c, ((d << 1) ^ s) & 0x80000000);
					break;
				case 2: /* RCL r/m32, imm8 */
					s |= e86_get_cf(c) << 31;
					d = (s << (cnt % 33)) | (s >> (33 - (cnt % 33)));
					e86_set_cf(c, d & 0x100000000);
					e86_set_of(c, ((s << 1) ^ s) & 0x80000000);
					break;
				case 3: /* RCR r/m32, imm8 */
					s |= e86_get_cf(c) << 31;
					d = (s >> (cnt % 33)) | (s << (33 - (cnt % 33)));
					e86_set_cf(c, d & 0x100000000);
					e86_set_of(c, ((d << 1) ^ s) & 0x80000000);
					break;
				case 4: /* SHL r/m32, imm8 */
					d = (cnt > 32) ? 0 : (s << cnt);
					e86_set_flg_szp_32(c, d);
					e86_set_cf(c, d & 0x100000000);
					e86_set_of(c, ((s << 1) ^ s) & 0x80000000);
					break;
				case 5: /* SHR r/m32, imm8 */
						/* c > 0 */
					d = (cnt > 32) ? 0 : (s >> (cnt - 1));
					if (cnt > 31)
					{
						/* Shift count higher than 31 */
						d = d & 0xfffff;
					}
					e86_set_cf(c, d & 1);
					d = d >> 1 & 0xffffffff;
					e86_set_flg_szp_32(c, d);
					e86_set_of(c, s & 0x80000000);
					break;
				case 7: /* SAR r/m32, imm8 */
					s |= (s & 0x80000000) ? 0xffffffff00000000 : 0x0000000000000000;
					d = s >> ((cnt >= 32) ? 31 : (cnt - 1));
					if (cnt > 31)
					{
						/* Shift count higher than 31 */
						d = d & 0xfffff;
					}
					e86_set_cf(c, d & 1);
					d = (d >> 1) & 0xffffffff;
					e86_set_flg_szp_32(c, d);
					e86_set_of(c, 0);
					break;
				default:
					d = 0; /* To avoid compiler warning */
					break;
				}
				g386_real_mode_set_ea32(c, d);
			}
			else
			{
				/* 16-bit memory address operand specified */
				unsigned long d, s;
				xop = (c->pq[1] >> 3) & 7;
				g386_real_mode_get_ea_ptr(c, c->pq + 1);
				s = g386_real_mode_get_ea16(c);
				cnt = c->pq[c->ea.cnt + 1];
				if (c->cpu & E86_CPU_MASK_SHIFT) {
					cnt &= 0x1f;
				}
				if (cnt == 0) {
					return (c->ea.cnt + 1);
				}
				switch (xop) {
				case 0: /* ROL r/m16, imm8 */
					d = (s << (cnt & 15)) | (s >> (16 - (cnt & 15)));
					e86_set_cf(c, d & 1);
					e86_set_of(c, ((s << 1) ^ s) & 0x8000);
					break;
				case 1: /* ROR r/m16, imm8 */
					d = (s >> (cnt & 15)) | (s << (16 - (cnt & 15)));
					e86_set_cf(c, d & 0x8000);
					e86_set_of(c, ((d << 1) ^ s) & 0x8000);
					break;
				case 2: /* RCL r/m16, imm8 */
					s |= e86_get_cf(c) << 16;
					d = (s << (cnt % 17)) | (s >> (17 - (cnt % 17)));
					e86_set_cf(c, d & 0x10000);
					e86_set_of(c, ((s << 1) ^ s) & 0x8000);
					break;
				case 3: /* RCR r/m16, imm8 */
					s |= e86_get_cf(c) << 16;
					d = (s >> (cnt % 17)) | (s << (17 - (cnt % 17)));
					e86_set_cf(c, d & 0x10000);
					e86_set_of(c, ((d << 1) ^ s) & 0x8000);
					break;
				case 4: /* SHL r/m16, imm8 */
					d = (cnt > 16) ? 0 : (s << cnt);
					e86_set_flg_szp_16(c, d);
					e86_set_cf(c, d & 0x10000);
					e86_set_of(c, ((s << 1) ^ s) & 0x8000);
					break;
				case 5: /* SHR r/m16, imm8 */
						/* c > 0 */
					d = (cnt > 16) ? 0 : (s >> (cnt - 1));
					e86_set_cf(c, d & 1);
					d = d >> 1;
					e86_set_flg_szp_16(c, d);
					e86_set_of(c, s & 0x8000);
					break;
				case 7: /* SAR r/m16, imm8 */
					s |= (s & 0x8000) ? 0xffff0000 : 0x00000000;
					d = s >> ((cnt >= 16) ? 15 : (cnt - 1));
					e86_set_cf(c, d & 1);
					d = (d >> 1) & 0xffff;
					e86_set_flg_szp_16(c, d);
					e86_set_of(c, 0);
					break;
				default:
					d = 0; /* To avoid compiler warning */
					break;
				}
				g386_real_mode_set_ea16(c, d);
			}
			if (c->memory_clock_cycle_disable == 1)
			{
				/* External memory cycle */
				unsigned char memclk;
				memclk = c->memory_clock_cycle_count;
				e86_set_clk(c, memclk);
			}
			else
			{
				/* CPU memory cycle */
				e86_set_clk_ea(c, 3 + cnt, 7 + cnt);
			}
			return (c->ea.cnt + 2);
			/* e386_real_mode_op_c1(c); */
			/* return (c->ea.cnt + 2); */
		}
		else if (e386_get_vm(c) == 1)
		{
			/* Virtual 8086 Mode */
			unsigned      xop;
			unsigned      cnt;
			if (c->e386_address_size_prefix == 1)
			{
				/* 32-bit memory address operand specified */
				unsigned long long d, s;
				xop = (c->pq[1] >> 3) & 7;
				g386_virtual_8086_mode_get_ea_ptr32(c, c->pq + 1);
				s = g386_virtual_8086_mode_get_ea32(c);
				cnt = c->pq[c->ea.cnt + 1];
				if (c->cpu & E86_CPU_MASK_SHIFT) {
					cnt &= 0x1f;
				}
				if (cnt == 0) {
					return (c->ea.cnt + 1);
				}
				switch (xop) {
				case 0: /* ROL r/m32, imm8 */
					d = (s << (cnt & 31)) | (s >> (32 - (cnt & 31)));
					e86_set_cf(c, d & 1);
					e86_set_of(c, ((s << 1) ^ s) & 0x80000000);
					break;
				case 1: /* ROR r/m32, imm8 */
					d = (s >> (cnt & 31)) | (s << (32 - (cnt & 31)));
					e86_set_cf(c, d & 0x80000000);
					e86_set_of(c, ((d << 1) ^ s) & 0x80000000);
					break;
				case 2: /* RCL r/m32, imm8 */
					s |= e86_get_cf(c) << 31;
					d = (s << (cnt % 33)) | (s >> (33 - (cnt % 33)));
					e86_set_cf(c, d & 0x100000000);
					e86_set_of(c, ((s << 1) ^ s) & 0x80000000);
					break;
				case 3: /* RCR r/m32, imm8 */
					s |= e86_get_cf(c) << 31;
					d = (s >> (cnt % 33)) | (s << (33 - (cnt % 33)));
					e86_set_cf(c, d & 0x100000000);
					e86_set_of(c, ((d << 1) ^ s) & 0x80000000);
					break;
				case 4: /* SHL r/m32, imm8 */
					d = (cnt > 32) ? 0 : (s << cnt);
					e86_set_flg_szp_32(c, d);
					e86_set_cf(c, d & 0x100000000);
					e86_set_of(c, ((s << 1) ^ s) & 0x80000000);
					break;
				case 5: /* SHR r/m32, imm8 */
						/* c > 0 */
					d = (cnt > 32) ? 0 : (s >> (cnt - 1));
					if (cnt > 31)
					{
						/* Shift count higher than 31 */
						d = d & 0xfffff;
					}
					e86_set_cf(c, d & 1);
					d = d >> 1 & 0xffffffff;
					e86_set_flg_szp_32(c, d);
					e86_set_of(c, s & 0x80000000);
					break;
				case 7: /* SAR r/m32, imm8 */
					s |= (s & 0x80000000) ? 0xffffffff00000000 : 0x0000000000000000;
					d = s >> ((cnt >= 32) ? 31 : (cnt - 1));
					if (cnt > 31)
					{
						/* Shift count higher than 31 */
						d = d & 0xfffff;
					}
					e86_set_cf(c, d & 1);
					d = (d >> 1) & 0xffffffff;
					e86_set_flg_szp_32(c, d);
					e86_set_of(c, 0);
					break;
				default:
					d = 0; /* To avoid compiler warning */
					break;
				}
				g386_virtual_8086_mode_set_ea32(c, d);
			}
			else
			{
				/* 16-bit memory address operand specified */
				unsigned long d, s;
				xop = (c->pq[1] >> 3) & 7;
				g386_virtual_8086_mode_get_ea_ptr(c, c->pq + 1);
				s = g386_virtual_8086_mode_get_ea16(c);
				cnt = c->pq[c->ea.cnt + 1];
				if (c->cpu & E86_CPU_MASK_SHIFT) {
					cnt &= 0x1f;
				}
				if (cnt == 0) {
					return (c->ea.cnt + 1);
				}
				switch (xop) {
				case 0: /* ROL r/m16, imm8 */
					d = (s << (cnt & 15)) | (s >> (16 - (cnt & 15)));
					e86_set_cf(c, d & 1);
					e86_set_of(c, ((s << 1) ^ s) & 0x8000);
					break;
				case 1: /* ROR r/m16, imm8 */
					d = (s >> (cnt & 15)) | (s << (16 - (cnt & 15)));
					e86_set_cf(c, d & 0x8000);
					e86_set_of(c, ((d << 1) ^ s) & 0x8000);
					break;
				case 2: /* RCL r/m16, imm8 */
					s |= e86_get_cf(c) << 16;
					d = (s << (cnt % 17)) | (s >> (17 - (cnt % 17)));
					e86_set_cf(c, d & 0x10000);
					e86_set_of(c, ((s << 1) ^ s) & 0x8000);
					break;
				case 3: /* RCR r/m16, imm8 */
					s |= e86_get_cf(c) << 16;
					d = (s >> (cnt % 17)) | (s << (17 - (cnt % 17)));
					e86_set_cf(c, d & 0x10000);
					e86_set_of(c, ((d << 1) ^ s) & 0x8000);
					break;
				case 4: /* SHL r/m16, imm8 */
					d = (cnt > 16) ? 0 : (s << cnt);
					e86_set_flg_szp_16(c, d);
					e86_set_cf(c, d & 0x10000);
					e86_set_of(c, ((s << 1) ^ s) & 0x8000);
					break;
				case 5: /* SHR r/m16, imm8 */
						/* c > 0 */
					d = (cnt > 16) ? 0 : (s >> (cnt - 1));
					e86_set_cf(c, d & 1);
					d = d >> 1;
					e86_set_flg_szp_16(c, d);
					e86_set_of(c, s & 0x8000);
					break;
				case 7: /* SAR r/m16, imm8 */
					s |= (s & 0x8000) ? 0xffff0000 : 0x00000000;
					d = s >> ((cnt >= 16) ? 15 : (cnt - 1));
					e86_set_cf(c, d & 1);
					d = (d >> 1) & 0xffff;
					e86_set_flg_szp_16(c, d);
					e86_set_of(c, 0);
					break;
				default:
					d = 0; /* To avoid compiler warning */
					break;
				}
				g386_virtual_8086_mode_set_ea16(c, d);
			}
			if (c->memory_clock_cycle_disable == 1)
			{
				/* External memory cycle */
				unsigned char memclk;
				memclk = c->memory_clock_cycle_count;
				e86_set_clk(c, memclk);
			}
			else
			{
				/* CPU memory cycle */
				e86_set_clk_ea(c, 3 + cnt, 7 + cnt);
			}
			return (c->ea.cnt + 2);
			/* e386_virtual_8086_mode_op_c1(c); */
			/* return (c->ea.cnt + 2); */
		}
		else
		{
			/* Protected Mode */
			unsigned      xop;
			unsigned      cnt;
			if (c->e386_address_size_prefix == 1)
			{
				/* 32-bit memory address operand specified */
				unsigned long long d, s;
				xop = (c->pq[1] >> 3) & 7;
				g386_protected_mode_get_ea_ptr32(c, c->pq + 1);
				s = g386_protected_mode_get_ea32(c);
				cnt = c->pq[c->ea.cnt + 1];
				if (c->cpu & E86_CPU_MASK_SHIFT) {
					cnt &= 0x1f;
				}
				if (cnt == 0) {
					return (c->ea.cnt + 1);
				}
				switch (xop) {
				case 0: /* ROL r/m32, imm8 */
					d = (s << (cnt & 31)) | (s >> (32 - (cnt & 31)));
					e86_set_cf(c, d & 1);
					e86_set_of(c, ((s << 1) ^ s) & 0x80000000);
					break;
				case 1: /* ROR r/m32, imm8 */
					d = (s >> (cnt & 31)) | (s << (32 - (cnt & 31)));
					e86_set_cf(c, d & 0x80000000);
					e86_set_of(c, ((d << 1) ^ s) & 0x80000000);
					break;
				case 2: /* RCL r/m32, imm8 */
					s |= e86_get_cf(c) << 31;
					d = (s << (cnt % 33)) | (s >> (33 - (cnt % 33)));
					e86_set_cf(c, d & 0x100000000);
					e86_set_of(c, ((s << 1) ^ s) & 0x80000000);
					break;
				case 3: /* RCR r/m32, imm8 */
					s |= e86_get_cf(c) << 31;
					d = (s >> (cnt % 33)) | (s << (33 - (cnt % 33)));
					e86_set_cf(c, d & 0x100000000);
					e86_set_of(c, ((d << 1) ^ s) & 0x80000000);
					break;
				case 4: /* SHL r/m32, imm8 */
					d = (cnt > 32) ? 0 : (s << cnt);
					e86_set_flg_szp_32(c, d);
					e86_set_cf(c, d & 0x100000000);
					e86_set_of(c, ((s << 1) ^ s) & 0x80000000);
					break;
				case 5: /* SHR r/m32, imm8 */
						/* c > 0 */
					d = (cnt > 32) ? 0 : (s >> (cnt - 1));
					if (cnt > 31)
					{
						/* Shift count higher than 31 */
						d = d & 0xfffff;
					}
					e86_set_cf(c, d & 1);
					d = d >> 1 & 0xffffffff;
					e86_set_flg_szp_32(c, d);
					e86_set_of(c, s & 0x80000000);
					break;
				case 7: /* SAR r/m32, imm8 */
					s |= (s & 0x80000000) ? 0xffffffff00000000 : 0x0000000000000000;
					d = s >> ((cnt >= 32) ? 31 : (cnt - 1));
					if (cnt > 31)
					{
						/* Shift count higher than 31 */
						d = d & 0xfffff;
					}
					e86_set_cf(c, d & 1);
					d = (d >> 1) & 0xffffffff;
					e86_set_flg_szp_32(c, d);
					e86_set_of(c, 0);
					break;
				}
				g386_protected_mode_set_ea32(c, d);
			}
			else
			{
				/* 16-bit memory address operand specified */
				unsigned long d, s;
				xop = (c->pq[1] >> 3) & 7;
				g386_protected_mode_get_ea_ptr(c, c->pq + 1);
				s = g386_protected_mode_get_ea16(c);
				cnt = c->pq[c->ea.cnt + 1];
				if (c->cpu & E86_CPU_MASK_SHIFT) {
					cnt &= 0x1f;
				}
				if (cnt == 0) {
					return (c->ea.cnt + 1);
				}
				switch (xop) {
				case 0: /* ROL r/m16, imm8 */
					d = (s << (cnt & 15)) | (s >> (16 - (cnt & 15)));
					e86_set_cf(c, d & 1);
					e86_set_of(c, ((s << 1) ^ s) & 0x8000);
					break;
				case 1: /* ROR r/m16, imm8 */
					d = (s >> (cnt & 15)) | (s << (16 - (cnt & 15)));
					e86_set_cf(c, d & 0x8000);
					e86_set_of(c, ((d << 1) ^ s) & 0x8000);
					break;
				case 2: /* RCL r/m16, imm8 */
					s |= e86_get_cf(c) << 16;
					d = (s << (cnt % 17)) | (s >> (17 - (cnt % 17)));
					e86_set_cf(c, d & 0x10000);
					e86_set_of(c, ((s << 1) ^ s) & 0x8000);
					break;
				case 3: /* RCR r/m16, imm8 */
					s |= e86_get_cf(c) << 16;
					d = (s >> (cnt % 17)) | (s << (17 - (cnt % 17)));
					e86_set_cf(c, d & 0x10000);
					e86_set_of(c, ((d << 1) ^ s) & 0x8000);
					break;
				case 4: /* SHL r/m16, imm8 */
					d = (cnt > 16) ? 0 : (s << cnt);
					e86_set_flg_szp_16(c, d);
					e86_set_cf(c, d & 0x10000);
					e86_set_of(c, ((s << 1) ^ s) & 0x8000);
					break;
				case 5: /* SHR r/m16, imm8 */
						/* c > 0 */
					d = (cnt > 16) ? 0 : (s >> (cnt - 1));
					e86_set_cf(c, d & 1);
					d = d >> 1;
					e86_set_flg_szp_16(c, d);
					e86_set_of(c, s & 0x8000);
					break;
				case 7: /* SAR r/m16, imm8 */
					s |= (s & 0x8000) ? 0xffff0000 : 0x00000000;
					d = s >> ((cnt >= 16) ? 15 : (cnt - 1));
					e86_set_cf(c, d & 1);
					d = (d >> 1) & 0xffff;
					e86_set_flg_szp_16(c, d);
					e86_set_of(c, 0);
					break;
				default:
					d = 0; /* To avoid compiler warning */
					break;
				}
				g386_protected_mode_set_ea16(c, d);
			}
			if (c->memory_clock_cycle_disable == 1)
			{
				/* External memory cycle */
				unsigned char memclk;
				memclk = c->memory_clock_cycle_count;
				e86_set_clk(c, memclk);
			}
			else
			{
				/* CPU memory cycle */
				e86_set_clk_ea(c, 3 + cnt, 7 + cnt);
			}
			return (c->ea.cnt + 2);
			/* e386_protected_mode_op_c1(c); */
			/* return (c->ea.cnt + 2); */
		}
	}
	else {
		/* Intel 80186 CPU or  lower */
		unsigned      xop;
		unsigned      cnt;
		unsigned long d, s;
		xop = (c->pq[1] >> 3) & 7;
		e86_get_ea_ptr(c, c->pq + 1);
		s = e86_get_ea16(c);
		cnt = c->pq[c->ea.cnt + 1];
		if (c->cpu & E86_CPU_MASK_SHIFT) {
			cnt &= 0x1f;
		}
		if (cnt == 0) {
			return (c->ea.cnt + 1);
		}
		switch (xop) {
		case 0: /* ROL r/m16, imm8 */
			d = (s << (cnt & 15)) | (s >> (16 - (cnt & 15)));
			e86_set_cf(c, d & 1);
			e86_set_of(c, ((s << 1) ^ s) & 0x8000);
			break;
		case 1: /* ROR r/m16, imm8 */
			d = (s >> (cnt & 15)) | (s << (16 - (cnt & 15)));
			e86_set_cf(c, d & 0x8000);
			e86_set_of(c, ((d << 1) ^ s) & 0x8000);
			break;
		case 2: /* RCL r/m16, imm8 */
			s |= e86_get_cf(c) << 16;
			d = (s << (cnt % 17)) | (s >> (17 - (cnt % 17)));
			e86_set_cf(c, d & 0x10000);
			e86_set_of(c, ((s << 1) ^ s) & 0x8000);
			break;
		case 3: /* RCR r/m16, imm8 */
			s |= e86_get_cf(c) << 16;
			d = (s >> (cnt % 17)) | (s << (17 - (cnt % 17)));
			e86_set_cf(c, d & 0x10000);
			e86_set_of(c, ((d << 1) ^ s) & 0x8000);
			break;
		case 4: /* SHL r/m16, imm8 */
			d = (cnt > 16) ? 0 : (s << cnt);
			e86_set_flg_szp_16(c, d);
			e86_set_cf(c, d & 0x10000);
			e86_set_of(c, ((s << 1) ^ s) & 0x8000);
			break;
		case 5: /* SHR r/m16, imm8 */
				/* c > 0 */
			d = (cnt > 16) ? 0 : (s >> (cnt - 1));
			e86_set_cf(c, d & 1);
			d = d >> 1;
			e86_set_flg_szp_16(c, d);
			e86_set_of(c, s & 0x8000);
			break;
		case 7: /* SAR r/m16, imm8 */
			s |= (s & 0x8000) ? 0xffff0000 : 0x00000000;
			d = s >> ((cnt >= 16) ? 15 : (cnt - 1));
			e86_set_cf(c, d & 1);
			d = (d >> 1) & 0xffff;
			e86_set_flg_szp_16(c, d);
			e86_set_of(c, 0);
			break;
		default:
			d = 0; /* To avoid compiler warning */
			break;
		}
		e86_set_ea16(c, d);
		if (c->memory_clock_cycle_disable == 1)
		{
			/* External memory cycle */
			unsigned char memclk;
			memclk = c->memory_clock_cycle_count;
			e86_set_clk(c, memclk);
		}
		else
		{
			/* CPU memory cycle */
			e86_set_clk_ea(c, 5 + cnt, 17 + cnt);
		}
		return (c->ea.cnt + 2);
	}
}

/* OP C8: ENTER imm16, imm8 (Real Mode) */
static
unsigned e386_real_mode_op_c8(e8086_t *c)
{
	unsigned char  level;
	unsigned short tmp;
	g386_real_mode_push(c, e86_get_bp(c));
	tmp = e86_get_sp(c);
	level = c->pq[3] & 0x1f;
	if (level > 0) {
		unsigned       i;
		unsigned short sp, bp;
		sp = e86_get_sp(c);
		bp = e86_get_bp(c);
		for (i = 1; i < level; i++) {
			bp -= 2;
			g386_real_mode_push(c, e86_get_mem16(c, sp, bp));
		}
		g386_real_mode_push(c, tmp);
	}
	e86_set_bp(c, tmp);
	e86_set_sp(c, tmp - e86_mk_uint16(c->pq[1], c->pq[2]));
	switch (level) {
	case 0:
		if (c->memory_clock_cycle_disable == 1)
		{
			/* External memory cycle */
			unsigned char memclk;
			memclk = c->memory_clock_cycle_count;
			e86_set_clk(c, memclk);
		}
		else
		{
			/* CPU memory cycle */
			e86_set_clk(c, 15);
		}
		break;
	case 1:
		if (c->memory_clock_cycle_disable == 1)
		{
			/* External memory cycle */
			unsigned char memclk;
			memclk = c->memory_clock_cycle_count;
			e86_set_clk(c, memclk);
		}
		else
		{
			/* CPU memory cycle */
			e86_set_clk(c, 25);
		}
		break;
	default:
		if (c->memory_clock_cycle_disable == 1)
		{
			/* External memory cycle */
			unsigned char memclk;
			memclk = c->memory_clock_cycle_count;
			e86_set_clk(c, memclk);
		}
		else
		{
			/* CPU memory cycle */
			e86_set_clk(c, 22 + 16 * (level - 1));
		}
		break;
	}
	c->e386_address_size_prefix = 0;
	c->e386_operand_size_prefix = 0;
}

/* OP C8: ENTER imm16, imm8 (Virtual 8086 Mode) */
static
unsigned e386_virtual_8086_mode_op_c8(e8086_t *c)
{
	unsigned char  level;
	unsigned short tmp;
	g386_virtual_8086_mode_push(c, e86_get_bp(c));
	tmp = e86_get_sp(c);
	level = c->pq[3] & 0x1f;
	if (level > 0) {
		unsigned       i;
		unsigned short sp, bp;
		sp = e86_get_sp(c);
		bp = e86_get_bp(c);
		for (i = 1; i < level; i++) {
			bp -= 2;
			g386_virtual_8086_mode_push(c, e86_get_mem16(c, sp, bp));
		}
		g386_virtual_8086_mode_push(c, tmp);
	}
	e86_set_bp(c, tmp);
	e86_set_sp(c, tmp - e86_mk_uint16(c->pq[1], c->pq[2]));
	switch (level) {
	case 0:
		if (c->memory_clock_cycle_disable == 1)
		{
			/* External memory cycle */
			unsigned char memclk;
			memclk = c->memory_clock_cycle_count;
			e86_set_clk(c, memclk);
		}
		else
		{
			/* CPU memory cycle */
			e86_set_clk(c, 15);
		}
		break;
	case 1:
		if (c->memory_clock_cycle_disable == 1)
		{
			/* External memory cycle */
			unsigned char memclk;
			memclk = c->memory_clock_cycle_count;
			e86_set_clk(c, memclk);
		}
		else
		{
			/* CPU memory cycle */
			e86_set_clk(c, 25);
		}
		break;
	default:
		if (c->memory_clock_cycle_disable == 1)
		{
			/* External memory cycle */
			unsigned char memclk;
			memclk = c->memory_clock_cycle_count;
			e86_set_clk(c, memclk);
		}
		else
		{
			/* CPU memory cycle */
			e86_set_clk(c, 22 + 16 * (level - 1));
		}
		break;
	}
	c->e386_address_size_prefix = 0;
	c->e386_operand_size_prefix = 0;
}

/* OP C8: ENTER imm16, imm8 (Protected Mode) */
static
unsigned e386_protected_mode_op_c8(e8086_t *c)
{
	unsigned char  level;
	unsigned short tmp;
	g386_protected_mode_push(c, e86_get_bp(c));
	tmp = e86_get_sp(c);
	level = c->pq[3] & 0x1f;
	if (level > 0) {
		unsigned       i;
		unsigned short sp, bp;
		sp = e86_get_sp(c);
		bp = e86_get_bp(c);
		for (i = 1; i < level; i++) {
			bp -= 2;
			g386_protected_mode_push(c, e86_get_mem16(c, sp, bp));
		}
		g386_protected_mode_push(c, tmp);
	}
	e86_set_bp(c, tmp);
	e86_set_sp(c, tmp - e86_mk_uint16(c->pq[1], c->pq[2]));
	switch (level) {
	case 0:
		if (c->memory_clock_cycle_disable == 1)
		{
			/* External memory cycle */
			unsigned char memclk;
			memclk = c->memory_clock_cycle_count;
			e86_set_clk(c, memclk);
		}
		else
		{
			/* CPU memory cycle */
			e86_set_clk(c, 15);
		}
		break;
	case 1:
		if (c->memory_clock_cycle_disable == 1)
		{
			/* External memory cycle */
			unsigned char memclk;
			memclk = c->memory_clock_cycle_count;
			e86_set_clk(c, memclk);
		}
		else
		{
			/* CPU memory cycle */
			e86_set_clk(c, 25);
		}
		break;
	default:
		if (c->memory_clock_cycle_disable == 1)
		{
			/* External memory cycle */
			unsigned char memclk;
			memclk = c->memory_clock_cycle_count;
			e86_set_clk(c, memclk);
		}
		else
		{
			/* CPU memory cycle */
			e86_set_clk(c, 22 + 16 * (level - 1));
		}
		break;
	}
	c->e386_address_size_prefix = 1;
	c->e386_operand_size_prefix = 1;
}

/* OP C8: ENTER imm16, imm8 */
static
unsigned op_c8 (e8086_t *c)
{
	if (c->cpumodel >= 0x80386) {
		/* Intel 80386 CPU or higher */
		e286_get_machine_status_word(c);
		e386_get_flags(c);
		if ((e286_get_pe(c) == 0) && (e386_get_vm(c) == 0))
		{
			/* Real Mode */
			e386_real_mode_op_c8(c);
			return (4);
		}
		else if (e386_get_vm(c) == 1)
		{
			/* Virtual 8086 Mode */
			e386_virtual_8086_mode_op_c8(c);
			return (4);
		}
		else
		{
			/* Protected Mode */
			e386_protected_mode_op_c8(c);
			return (4);
		}
	}
	else {
		/* Intel 80186 CPU or  lower */
		unsigned char  level;
		unsigned short tmp;
		e86_push(c, e86_get_bp(c));
		tmp = e86_get_sp(c);
		level = c->pq[3] & 0x1f;
		if (level > 0) {
			unsigned       i;
			unsigned short sp, bp;
			sp = e86_get_sp(c);
			bp = e86_get_bp(c);
			for (i = 1; i < level; i++) {
				bp -= 2;
				e86_push(c, e86_get_mem16(c, sp, bp));
			}
			e86_push(c, tmp);
		}
		e86_set_bp(c, tmp);
		e86_set_sp(c, tmp - e86_mk_uint16(c->pq[1], c->pq[2]));
		switch (level) {
		case 0:
			if (c->memory_clock_cycle_disable == 1)
			{
				/* External memory cycle */
				unsigned char memclk;
				memclk = c->memory_clock_cycle_count;
				e86_set_clk(c, memclk);
			}
			else
			{
				/* CPU memory cycle */
				e86_set_clk(c, 15);
			}
			break;
		case 1:
			if (c->memory_clock_cycle_disable == 1)
			{
				/* External memory cycle */
				unsigned char memclk;
				memclk = c->memory_clock_cycle_count;
				e86_set_clk(c, memclk);
			}
			else
			{
				/* CPU memory cycle */
				e86_set_clk(c, 25);
			}
			break;
		default:
			if (c->memory_clock_cycle_disable == 1)
			{
				/* External memory cycle */
				unsigned char memclk;
				memclk = c->memory_clock_cycle_count;
				e86_set_clk(c, memclk);
			}
			else
			{
				/* CPU memory cycle */
				e86_set_clk(c, 22 + 16 * (level - 1));
			}
			break;
		}
		return (4);
	}
}

/* OP C9: LEAVE (Real Mode) */
static
unsigned e386_real_mode_op_c9(e8086_t *c)
{
	e86_set_sp(c, e86_get_bp(c));
	e86_set_bp(c, g386_real_mode_pop(c));
	if (c->memory_clock_cycle_disable == 1)
	{
		/* External memory cycle */
		unsigned char memclk;
		memclk = c->memory_clock_cycle_count;
		e86_set_clk(c, memclk);
	}
	else
	{
		/* CPU memory cycle */
		e86_set_clk(c, 8);
	}
	c->e386_address_size_prefix = 0;
	c->e386_operand_size_prefix = 0;
}

/* OP C9: LEAVE (Virtual 8086 Mode) */
static
unsigned e386_virtual_8086_mode_op_c9(e8086_t *c)
{
	e86_set_sp(c, e86_get_bp(c));
	e86_set_bp(c, g386_virtual_8086_mode_pop(c));
	if (c->memory_clock_cycle_disable == 1)
	{
		/* External memory cycle */
		unsigned char memclk;
		memclk = c->memory_clock_cycle_count;
		e86_set_clk(c, memclk);
	}
	else
	{
		/* CPU memory cycle */
		e86_set_clk(c, 8);
	}
	c->e386_address_size_prefix = 0;
	c->e386_operand_size_prefix = 0;
}

/* OP C9: LEAVE (Protected Mode) */
static
unsigned e386_protected_mode_op_c9(e8086_t *c)
{
	e86_set_sp(c, e86_get_bp(c));
	e86_set_bp(c, g386_protected_mode_pop_word_or_dword_register(c));
	if (c->memory_clock_cycle_disable == 1)
	{
		/* External memory cycle */
		unsigned char memclk;
		memclk = c->memory_clock_cycle_count;
		e86_set_clk(c, memclk);
	}
	else
	{
		/* CPU memory cycle */
		e86_set_clk(c, 8);
	}
	c->e386_address_size_prefix = 1;
	c->e386_operand_size_prefix = 1;
}

/* OP C9: LEAVE */
static
unsigned op_c9 (e8086_t *c)
{
	if (c->cpumodel >= 0x80386) {
		/* Intel 80386 CPU or higher */
		e286_get_machine_status_word(c);
		e386_get_flags(c);
		if ((e286_get_pe(c) == 0) && (e386_get_vm(c) == 0))
		{
			/* Real Mode */
			e386_real_mode_op_c9(c);
			return (1);
		}
		else if (e386_get_vm(c) == 1)
		{
			/* Virtual 8086 Mode */
			e386_virtual_8086_mode_op_c9(c);
			return (1);
		}
		else
		{
			/* Protected Mode */
			e386_protected_mode_op_c9(c);
			return (1);
		}
	}
	else {
		/* Intel 80186 CPU or  lower */
		e86_set_sp(c, e86_get_bp(c));
		e86_set_bp(c, e86_pop(c));
		if (c->memory_clock_cycle_disable == 1)
		{
			/* External memory cycle */
			unsigned char memclk;
			memclk = c->memory_clock_cycle_count;
			e86_set_clk(c, memclk);
		}
		else
		{
			/* CPU memory cycle */
			e86_set_clk(c, 8);
		}
		return (1);
	}
}

void e86_set_80186 (e8086_t *c)
{
	e86_set_8086 (c);
	c->cpumodel = 0x80186; /* Set to default CPU model */
	c->cpu &= ~E86_CPU_REP_BUG;
	c->cpu |= E86_CPU_MASK_SHIFT;
	c->cpu |= E86_CPU_INT6;
	c->op[0x60] = &op_60;
	c->op[0x61] = &op_61;
	c->op[0x62] = &op_62;
	c->op[0x68] = &op_68;
	c->op[0x69] = &op_69;
	c->op[0x6a] = &op_6a;
	c->op[0x6b] = &op_6b;
	c->op[0x6c] = &op_6c;
	c->op[0x6d] = &op_6d;
	c->op[0x6e] = &op_6e;
	c->op[0x6f] = &op_6f;
	c->op[0xc0] = &op_c0;
	c->op[0xc1] = &op_c1;
	c->op[0xc8] = &op_c8;
	c->op[0xc9] = &op_c9;
}

void e86_set_80186_disassembler(e8086_t *c)
{
	e86_set_8086(c);
	c->cpumodel = 0x80186; /* Set to default CPU model */
	c->disassemble = 1;    /* Turn on CPU instruction disassembler */
	c->cpu &= ~E86_CPU_REP_BUG;
	c->cpu |= E86_CPU_MASK_SHIFT;
	c->cpu |= E86_CPU_INT6;
	c->op[0x60] = &op_60;
	c->op[0x61] = &op_61;
	c->op[0x62] = &op_62;
	c->op[0x68] = &op_68;
	c->op[0x69] = &op_69;
	c->op[0x6a] = &op_6a;
	c->op[0x6b] = &op_6b;
	c->op[0x6c] = &op_6c;
	c->op[0x6d] = &op_6d;
	c->op[0x6e] = &op_6e;
	c->op[0x6f] = &op_6f;
	c->op[0xc0] = &op_c0;
	c->op[0xc1] = &op_c1;
	c->op[0xc8] = &op_c8;
	c->op[0xc9] = &op_c9;
}

void e86_set_80188 (e8086_t *c)
{
	e86_set_80186 (c);
	c->cpumodel = 0x80188; /* Set to default CPU model */
	c->cpu |= E86_CPU_8BIT;
	e86_set_pq_size (c, 4);
}

void e86_set_80188_disassembler(e8086_t *c)
{
	e86_set_80186(c);
	c->cpumodel = 0x80188; /* Set to default CPU model */
	c->disassemble = 1;    /* Turn on CPU instruction disassembler */
	c->cpu |= E86_CPU_8BIT;
	e86_set_pq_size(c, 4);
}
