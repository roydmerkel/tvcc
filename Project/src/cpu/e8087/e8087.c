/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/e8086/e8086.c                                        *
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


#include "e8087.h"
#include "intrnl87.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

/* WARNING: This file is used as a placeholder for the upcoming Intel 8087 FPU. Please do not use this file in any production emulator. */
static unsigned char e86_get_mem_uint8 (void *mem, unsigned long addr);
static unsigned short e86_get_mem_uint16 (void *mem, unsigned long addr);
static void e86_set_mem_uint8 (void *mem, unsigned long addr, unsigned char val);
static void e86_set_mem_uint16 (void *mem, unsigned long addr, unsigned short val);


static char *dreg16[8] = { "ax", "cx", "dx", "bx", "sp", "bp", "si", "di" };
static char *dreg8[8] = { "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh" };
static char *sreg[4] = { "es", "cs", "ss", "ds" };


void e87_init (e8087_t *c)
{
	unsigned i;

	c->cpu = E87_CPU_REP_BUG;

	c->mem = NULL;
	c->mem_get_uint8 = e86_get_mem_uint8;
	c->mem_get_uint16 = e86_get_mem_uint16;
	c->mem_set_uint8 = e86_set_mem_uint8;
	c->mem_set_uint16 = e86_set_mem_uint16;

	c->prt = NULL;
	c->prt_get_uint8 = e86_get_mem_uint8;
	c->prt_get_uint16 = e86_get_mem_uint16;
	c->prt_set_uint8 = e86_set_mem_uint8;
	c->prt_set_uint16 = e86_set_mem_uint16;

	c->ram = NULL;
	c->ram_cnt = 0;

	c->addr_mask = 0xfffff;

	c->inta_ext = NULL;
	c->inta = NULL;

	c->op_ext = NULL;
	c->op_hook = NULL;
	c->op_stat = NULL;
	c->op_undef = NULL;
	c->op_int = NULL;

	c->pq_size = 4;
	c->pq_fill = 6;

	c->irq = 0;

	c->halt = 0;

	c->int_cnt = 0;
	c->int_vec = 0;
	c->int_cs = 0;
	c->int_ip = 0;

	for (i = 0; i < 256; i++) {
		c->op[i] = e87_opcodes[i];
	}

	c->clocks = 0;
	c->instructions = 0;
	c->delay = 0;
}

void e87_free (e8087_t *c)
{
}

e8087_t *e87_new (void)
{
	e8087_t *c;

	c = malloc (sizeof (e8087_t));
	if (c == NULL) {
		return (NULL);
	}

	e87_init (c);

	return (c);
}

void e87_del (e8087_t *c)
{
	if (c != NULL) {
		e87_free (c);
		free (c);
	}
}

void e87_set_8087 (e8087_t *c)
{
	unsigned i;

	c->cpu = E87_CPU_REP_BUG;

	for (i = 0; i < 256; i++) {
		c->op[i] = e87_opcodes[i];
	}

	e87_set_pq_size (c, 6);
}

void e87_set_8088_87 (e8087_t *c)
{
	e87_set_8087 (c);

	c->cpu |= E87_CPU_8BIT;

	e87_set_pq_size (c, 4);
}

void e87_set_v30_87 (e8087_t *c)
{
	e87_set_8087 (c);

	c->cpu &= ~(E87_CPU_REP_BUG | E87_CPU_MASK_SHIFT);
}

void e87_set_v20_87 (e8087_t *c)
{
	e87_set_8088_87 (c);

	c->cpu &= ~(E87_CPU_REP_BUG | E87_CPU_MASK_SHIFT);
}

void e87_set_pq_size (e8087_t *c, unsigned size)
{
	if (size > E87_PQ_MAX) {
		size = E87_PQ_MAX;
	}

	c->pq_size = size;
	c->pq_fill = (size < 6) ? 6 : size;
	c->pq_cnt = 0;
}

void e87_set_options (e8087_t *c, unsigned opt, int set)
{
	if (set) {
		c->cpu |= opt;
	}
	else {
		c->cpu &= ~opt;
	}
}

void e87_set_addr_mask (e8087_t *c, unsigned long msk)
{
	c->addr_mask = msk;
}

unsigned long e87_get_addr_mask (e8087_t *c)
{
	return (c->addr_mask);
}

void e87_set_inta_fct (e8087_t *c, void *ext, void *fct)
{
	c->inta_ext = ext;
	c->inta = fct;
}

void e87_set_ram (e8087_t *c, unsigned char *ram, unsigned long cnt)
{
	c->ram = ram;
	c->ram_cnt = cnt;
}

void e87_set_mem (e8087_t *c, void *mem,
	e87_get_uint8_f get8, e87_set_uint8_f set8,
	e87_get_uint16_f get16, e87_set_uint16_f set16)
{
	c->mem = mem;
	c->mem_get_uint8 = get8;
	c->mem_set_uint8 = set8;
	c->mem_get_uint16 = get16;
	c->mem_set_uint16 = set16;
}

void e87_set_prt (e8087_t *c, void *prt,
	e87_get_uint8_f get8, e87_set_uint8_f set8,
	e87_get_uint16_f get16, e87_set_uint16_f set16)
{
	c->prt = prt;
	c->prt_get_uint8 = get8;
	c->prt_set_uint8 = set8;
	c->prt_get_uint16 = get16;
	c->prt_set_uint16 = set16;
}

static
unsigned char e87_get_mem_uint8 (void *mem, unsigned long addr)
{
	return (0xaa);
}

static
unsigned short e87_get_mem_uint16 (void *mem, unsigned long addr)
{
	return (0xaaaa);
}

static
void e87_set_mem_uint8 (void *mem, unsigned long addr, unsigned char val)
{
}

static
void e87_set_mem_uint16 (void *mem, unsigned long addr, unsigned short val)
{
}

int e87_get_reg (e8087_t *c, const char *reg, unsigned long *val)
{
	unsigned i;

	if (*reg == '%') {
		reg += 1;
	}

	for (i = 0; i < 8; i++) {
		if (strcmp (reg, dreg16[i]) == 0) {
			*val = e87_get_reg16 (c, i);
			return (0);
		}

		if (strcmp (reg, dreg8[i]) == 0) {
			*val = e87_get_reg8 (c, i);
			return (0);
		}
	}

	for (i = 0; i < 4; i++) {
		if (strcmp (reg, sreg[i]) == 0) {
			*val = e87_get_sreg (c, i);
			return (0);
		}
	}

	if (strcmp (reg, "ip") == 0) {
		*val = e87_get_ip (c);
		return (0);
	}

	if (strcmp (reg, "flags") == 0) {
		*val = e87_get_flags (c);
		return (0);
	}

	return (1);
}

int e87_set_reg (e8087_t *c, const char *reg, unsigned long val)
{
	unsigned i;

	if (*reg == '%') {
		reg += 1;
	}

	for (i = 0; i < 8; i++) {
		if (strcmp (reg, dreg16[i]) == 0) {
			e87_set_reg16 (c, i, val);
			return (0);
		}

		if (strcmp (reg, dreg8[i]) == 0) {
			e87_set_reg8 (c, i, val);
			return (0);
		}
	}

	for (i = 0; i < 4; i++) {
		if (strcmp (reg, sreg[i]) == 0) {
			e87_set_sreg (c, i, val);
			return (0);
		}
	}

	if (strcmp (reg, "ip") == 0) {
		e87_set_ip (c, val);
		return (0);
	}

	if (strcmp (reg, "flags") == 0) {
		e87_set_flags (c, val);
		return (0);
	}

	return (1);
}

void e87_trap (e8087_t *c, unsigned n)
{
	unsigned short ip;
	unsigned short ofs;

	if (c->op_int != NULL) {
		c->op_int (c->op_ext, n);
	}

	if (c->prefix & E87_PREFIX_KEEP) {
		ip = e87_get_cur_ip (c);

		if (c->cpu & E87_CPU_REP_BUG) {
			if (c->prefix & (E87_PREFIX_REP | E87_PREFIX_REPN)) {
				if (c->prefix & E87_PREFIX_SEG) {
					ip += 1;
				}
			}
		}

		c->prefix = 0;
	}
	else {
		ip = e87_get_ip (c);
	}

	c->int_cnt += 1;
	c->int_vec = n;
	c->int_cs = e87_get_cs (c);
	c->int_ip = ip;

	e87_push (c, e87_get_flags (c));
	e87_push (c, e87_get_cs (c));
	e87_push (c, ip);

	ofs = (unsigned short) (n & 0xff) << 2;

	e87_set_ip (c, e87_get_mem16 (c, 0, ofs));
	e87_set_cs (c, e87_get_mem16 (c, 0, ofs + 2));
	c->flg &= ~(E87_FLG_I | E87_FLG_T);

	e87_pq_init (c);
}

void e87_irq (e8087_t *cpu, unsigned char val)
{
	cpu->irq = (val != 0);
}

static
void e86_irq_ack (e8087_t *c)
{
	unsigned char irq;

	c->irq = 0;
	c->halt = 0;

	if (c->inta != NULL) {
		irq = c->inta (c->inta_ext);

		e86_set_clk (c, 61);

		e86_trap (c, irq);
	}
}

int e87_interrupt (e8087_t *cpu, unsigned n)
{
	if (e87_get_if (cpu)) {
		cpu->halt = 0;
		e87_trap (cpu, n);
		return (0);
	}

	return (1);
}

unsigned e87_undefined (e8087_t *c)
{
	if (c->op_undef != NULL) {
		c->op_undef (c->op_ext, c->pq[0], c->pq[1]);
	}

	if (c->cpu & E87_CPU_INT6) {
		e87_trap (c, 6);
		e87_set_clk (c, 50);
		return (0);
	}

	return (1);
}

unsigned long long e87_get_clock (e8087_t *c)
{
	return (c->clocks);
}

unsigned long long e87_get_opcnt (e8087_t *c)
{
	return (c->instructions);
}

void e87_reset (e8087_t *c)
{
	unsigned i;

	c->instructions = 0;

	c->addr_mask = 0xfffff;

	for (i = 0; i < 8; i++) {
		c->dreg[i] = 0;
	}

	for (i = 0; i < 4; i++) {
		c->sreg[i] = 0;
	}

	e86_set_cs (c, 0xf000);
	e86_set_ip (c, 0xfff0);
	e86_set_flags (c, 0x0000);

	c->pq_cnt = 0;

	c->irq = 0;

	c->halt = 0;

	c->prefix = 0;
}

void e87_execute (e8087_t *c)
{
	unsigned cnt;
	char     irq;

	if (c->halt) {
		e87_set_clk (c, 2);

		if (c->irq && e87_get_if (c)) {
			e87_irq_ack (c);
		}

		return;
	}

	if ((c->prefix & E87_PREFIX_KEEP) == 0) {
		c->prefix = 0;
		c->cur_ip = c->ip;
	}

	irq = c->irq;

	c->enable_int = 1;

	do {
		e87_pq_fill (c);

		c->prefix &= ~E87_PREFIX_NEW;

		if (c->op_stat != NULL) {
			c->op_stat (c->op_ext, c->pq[0], c->pq[1]);
		}

		cnt = c->op[c->pq[0]] (c);

		if (cnt > 0) {
			c->ip = (c->ip + cnt) & 0xffff;
			e86_pq_adjust (c, cnt);
		}
		else {
			c->delay += 10;
		}
	} while (c->prefix & E87_PREFIX_NEW);

	c->instructions += 1;

	if (c->enable_int) {
		if (c->flg & E87_FLG_T) {
			c->halt = 0;
			e87_trap (c, 1);
		}
		else if (irq && c->irq && e87_get_if (c)) {
			e87_irq_ack (c);
		}
	}
}

void e87_clock (e8087_t *c, unsigned n)
{
	while (n >= c->delay) {
		n -= c->delay;
		c->clocks += c->delay;
		c->delay = 0;
		e87_execute (c);
	}

	c->delay -= n;
	c->clocks += n;
}
