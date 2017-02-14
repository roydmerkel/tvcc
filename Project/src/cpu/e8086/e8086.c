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


#include "e8086.h"
#include "internal.h"
#include "mmuseg.h" /* Memory Managment Unit (Intel 80386 CPU) */
#include <lib/log.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>


static unsigned char e86_get_mem_uint8 (void *mem, unsigned long addr);
static unsigned short e86_get_mem_uint16 (void *mem, unsigned long addr);
static unsigned long e86_get_mem_uint32 (void *mem, unsigned long addr);
static void e86_set_mem_uint8 (void *mem, unsigned long addr, unsigned char val);
static void e86_set_mem_uint16 (void *mem, unsigned long addr, unsigned short val);
static void e86_set_mem_uint32 (void *mem, unsigned long addr, unsigned long val);


static char *dreg32[8] = { "eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi" };
static char *dreg16[8] = { "ax", "cx", "dx", "bx", "sp", "bp", "si", "di" };
static char *dreg8[8] = { "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh" };
static char *sreg[4] = { "es", "cs", "ss", "ds" };
static char *sreg386[2] = { "fs", "gs" };


void e86_init(e8086_t *c)
{
	unsigned i;
	if (c->cpumodel >= 0x80386) {
		/* Intel 80386 CPU or higher */
		c->cpu = E86_CPU_REP_BUG;
		c->mem = NULL;
		/* ---------- */
		c->mem_get_uint8 = e86_get_mem_uint8;
		c->mem_get_uint16 = e86_get_mem_uint16;
		/* c->mem_get_uint32 = e86_get_mem_uint32; */
		c->mem_set_uint8 = e86_set_mem_uint8;
		c->mem_set_uint16 = e86_set_mem_uint16;
		/* c->mem_set_uint32 = e86_set_mem_uint32; */
		/* ---------- */
		c->prt = NULL;
		c->prt_get_uint8 = e86_get_mem_uint8;
		c->prt_get_uint16 = e86_get_mem_uint16;
		/* c->prt_get_uint32 = e86_get_mem_uint32; */
		c->prt_set_uint8 = e86_set_mem_uint8;
		c->prt_set_uint16 = e86_set_mem_uint16;
		/* c->prt_set_uint32 = e86_set_mem_uint32; */
		/* ----------- */
		c->ram = NULL;
		c->ram_cnt = 0;
		/* ----------- */
		c->addr_mask = 0xffffffff;
		c->inta_ext = NULL;
		/* ----------- */
		c->inta = NULL;
		/* ----------- */
		c->op_ext = NULL;
		/* ----------- */
		c->op_hook = NULL;
		c->op_stat = NULL;
		c->op_undef = NULL;
		c->op_int = NULL;
		/* ----------- */
		c->pq_size = 4;
		/* ----------- */
		c->pq_fill = 6;
		/* ----------- */
		c->irq = 0;
		c->halt = 0;
		c->int_cnt = 0;
		/* ----------- */
		c->int_vec = 0;
		c->int_cs = 0;
		c->int_ip = 0;
		for (i = 0; i < 256; i++) {
			c->op[i] = e86_opcodes[i];
		}
		c->clocks = 0;
		c->instructions = 0;
		c->delay = 0;
		c->memory_clock_cycle_disable = 0;
		c->memory_clock_cycle_count = 0;
	}
	else {
		/* Intel 80186 CPU or  lower */
		/* ---------- */
		c->cpu = E86_CPU_REP_BUG;
		c->mem = NULL;
		/* ---------- */
		c->mem_get_uint8 = e86_get_mem_uint8;
		c->mem_get_uint16 = e86_get_mem_uint16;
		c->mem_set_uint8 = e86_set_mem_uint8;
		c->mem_set_uint16 = e86_set_mem_uint16;
		/* ---------- */
		c->prt = NULL;
		c->prt_get_uint8 = e86_get_mem_uint8;
		c->prt_get_uint16 = e86_get_mem_uint16;
		c->prt_set_uint8 = e86_set_mem_uint8;
		c->prt_set_uint16 = e86_set_mem_uint16;
		/* ----------- */
		c->ram = NULL;
		c->ram_cnt = 0;
		/* ----------- */
		c->addr_mask = 0xfffff;
		c->inta_ext = NULL;
		/* ----------- */
		c->inta = NULL;
		/* ----------- */
		c->op_ext = NULL;
		/* ----------- */
		c->op_hook = NULL;
		c->op_stat = NULL;
		c->op_undef = NULL;
		c->op_int = NULL;
		/* ----------- */
		c->pq_size = 4;
		/* ----------- */
		c->pq_fill = 6;
		/* ----------- */
		c->irq = 0;
		c->halt = 0;
		c->int_cnt = 0;
		/* ----------- */
		c->int_vec = 0;
		c->int_cs = 0;
		c->int_ip = 0;
		for (i = 0; i < 256; i++) {
			c->op[i] = e86_opcodes[i];
		}
		c->clocks = 0;
		c->instructions = 0;
		c->delay = 0;
		c->memory_clock_cycle_disable = 0;
		c->memory_clock_cycle_count = 0;
	}
}

void e86_free(e8086_t *c)
{
}

e8086_t *e86_new(void)
{
	e8086_t *c;
	c = malloc(sizeof(e8086_t));
	if (c == NULL) {
		return (NULL);
	}
	e86_init(c);
	return (c);
}

void e86_del(e8086_t *c)
{
	if (c != NULL) {
		e86_free(c);
		free(c);
	}
}

void e86_set_8086(e8086_t *c)
{
	unsigned i;
	c->cpumodel = 0x8086; /* Set to default CPU model */
	c->cpu = E86_CPU_REP_BUG;
	for (i = 0; i < 256; i++) {
		c->op[i] = e86_opcodes[i];
	}
	e86_set_pq_size(c, 6);
}

void e86_set_8086_disassembler(e8086_t *c)
{
	unsigned i;
	c->cpumodel = 0x8086; /* Set to default CPU model */
	c->disassemble = 1;   /* Turn on CPU instruction disassembler */
	c->cpu = E86_CPU_REP_BUG;
	for (i = 0; i < 256; i++) {
		c->op[i] = e86_opcodes[i];
	}
	e86_set_pq_size(c, 6);
}

void e86_set_8088(e8086_t *c)
{
	e86_set_8086(c);
	c->cpumodel = 0x8088; /* Set to default CPU model */
	c->cpu |= E86_CPU_8BIT;
	e86_set_pq_size(c, 4);
}

void e86_set_8088_disassembler(e8086_t *c)
{
	e86_set_8086(c);
	c->cpumodel = 0x8088; /* Set to default CPU model */
	c->disassemble = 1;   /* Turn on CPU instruction disassembler */
	c->cpu |= E86_CPU_8BIT;
	e86_set_pq_size(c, 4);
}

void e86_set_v30(e8086_t *c)
{
	e86_set_8086(c);
	c->cpumodel = 0x8086; /* Set to default CPU model */
	c->cpu &= ~(E86_CPU_REP_BUG | E86_CPU_MASK_SHIFT);
}

void e86_set_v30_disassembler(e8086_t *c)
{
	e86_set_8086(c);
	c->cpumodel = 0x8086; /* Set to default CPU model */
	c->disassemble = 1;   /* Turn on CPU instruction disassembler */
	c->cpu &= ~(E86_CPU_REP_BUG | E86_CPU_MASK_SHIFT);
}

void e86_set_v20(e8086_t *c)
{
	e86_set_8088(c);
	c->cpumodel = 0x8088; /* Set to default CPU model */
	c->cpu &= ~(E86_CPU_REP_BUG | E86_CPU_MASK_SHIFT);
}

void e86_set_v20_disassembler(e8086_t *c)
{
	e86_set_8088(c);
	c->cpumodel = 0x8088; /* Set to default CPU model */
	c->disassemble = 1;   /* Turn on CPU instruction disassembler */
	c->cpu &= ~(E86_CPU_REP_BUG | E86_CPU_MASK_SHIFT);
}

void e86_set_pq_size(e8086_t *c, unsigned size)
{
	if (size > E86_PQ_MAX) {
		size = E86_PQ_MAX;
	}
	c->pq_size = size;
	c->pq_fill = (size < 6) ? 6 : size;
	c->pq_cnt = 0;
}

void e86_set_options(e8086_t *c, unsigned opt, int set)
{
	if (set) {
		c->cpu |= opt;
	}
	else {
		c->cpu &= ~opt;
	}
}

void e86_set_addr_mask(e8086_t *c, unsigned long msk)
{
	c->addr_mask = msk;
}

unsigned long e86_get_addr_mask(e8086_t *c)
{
	return (c->addr_mask);
}

void e86_set_inta_fct(e8086_t *c, void *ext, void *fct)
{
	c->inta_ext = ext;
	c->inta = fct;
}

void e86_set_ram(e8086_t *c, unsigned char *ram, unsigned long cnt)
{
	c->ram = ram;
	c->ram_cnt = cnt;
}

void e86_set_mem(e8086_t *c, void *mem, e86_get_uint8_f get8, e86_set_uint8_f set8, e86_get_uint16_f get16, e86_set_uint16_f set16, e86_get_uint32_f get32, e86_set_uint32_f set32)
{
	c->mem = mem;
	c->mem_get_uint8 = get8;
	c->mem_set_uint8 = set8;
	c->mem_get_uint16 = get16;
	c->mem_set_uint16 = set16;
	c->mem_get_uint32 = get32;
	c->mem_set_uint32 = set32;
}

void e86_set_prt(e8086_t *c, void *prt, e86_get_uint8_f get8, e86_set_uint8_f set8, e86_get_uint16_f get16, e86_set_uint16_f set16, e86_get_uint32_f get32, e86_set_uint32_f set32)
{
	c->prt = prt;
	c->prt_get_uint8 = get8;
	c->prt_set_uint8 = set8;
	c->prt_get_uint16 = get16;
	c->prt_set_uint16 = set16;
	c->prt_get_uint32 = get32;
	c->prt_set_uint32 = set32;
}

static
unsigned char e86_get_mem_uint8(void *mem, unsigned long addr)
{
	return (0xaa);
}

static
unsigned short e86_get_mem_uint16(void *mem, unsigned long addr)
{
	return (0xaaaa);
}

static
unsigned long e86_get_mem_uint32(void *mem, unsigned long addr)
{
	return (0xaaaaaaaa);
}

static
void e86_set_mem_uint8(void *mem, unsigned long addr, unsigned char val)
{
}

static
void e86_set_mem_uint16(void *mem, unsigned long addr, unsigned short val)
{
}

static
void e86_set_mem_uint32(void *mem, unsigned long addr, unsigned long val)
{
}

int e86_get_reg(e8086_t *c, const char *reg, unsigned long *val)
{
	unsigned i;
	if (c->cpumodel >= 0x80386) {
		/* Intel 80386 CPU or higher */
		if (*reg == '%') {
			reg += 1;
		}
		for (i = 0; i < 8; i++) {
			if (strcmp(reg, dreg32[i]) == 0) {
				*val = e386_get_reg32(c, i);
				return (0);
			}
			if (strcmp(reg, dreg16[i]) == 0) {
				*val = e86_get_reg16(c, i);
				return (0);
			}
			if (strcmp(reg, dreg8[i]) == 0) {
				*val = e86_get_reg8(c, i);
				return (0);
			}
		}
		for (i = 0; i < 4; i++) {
			if (strcmp(reg, sreg[i]) == 0) {
				*val = e86_get_sreg(c, i);
				return (0);
			}
		}
		for (i = 0; i < 2; i++) {
			if (strcmp(reg, sreg386[i]) == 0) {
				*val = e386_get_sreg_fs_gs(c, i);
				return (0);
			}
		}
		if (strcmp(reg, "eip") == 0) {
			*val = e86_get_ip(c);
			return (0);
		}
		if (strcmp(reg, "ip") == 0) {
			*val = e86_get_ip(c);
			return (0);
		}
		if (strcmp(reg, "eflags") == 0) {
			*val = e386_get_flags(c);
			return (0);
		}
		if (strcmp(reg, "flags") == 0) {
			*val = e286_get_flags(c);
			return (0);
		}
		return (1);
	}
	else {
		/* Intel 80286 CPU or  lower */
		if (*reg == '%') {
			reg += 1;
		}
		for (i = 0; i < 8; i++) {
			if (strcmp(reg, dreg16[i]) == 0) {
				*val = e86_get_reg16(c, i);
				return (0);
			}
			if (strcmp(reg, dreg8[i]) == 0) {
				*val = e86_get_reg8(c, i);
				return (0);
			}
		}
		for (i = 0; i < 4; i++) {
			if (strcmp(reg, sreg[i]) == 0) {
				*val = e86_get_sreg(c, i);
				return (0);
			}
		}
		if (strcmp(reg, "ip") == 0) {
			*val = e86_get_ip(c);
			return (0);
		}
		if (strcmp(reg, "flags") == 0) {
			*val = e86_get_flags(c);
			return (0);
		}
		return (1);
	}
}

int e86_set_reg(e8086_t *c, const char *reg, unsigned long val)
{
	unsigned i;
	if (c->cpumodel >= 0x80386) {
		/* Intel 80386 CPU or higher */
		if (*reg == '%') {
			reg += 1;
		}
		for (i = 0; i < 8; i++) {
			if (strcmp(reg, dreg32[i]) == 0) {
				e386_set_reg32(c, i, val);
				return (0);
			}
			if (strcmp(reg, dreg16[i]) == 0) {
				e86_set_reg16(c, i, val);
				return (0);
			}
			if (strcmp(reg, dreg8[i]) == 0) {
				e86_set_reg8(c, i, val);
				return (0);
			}
		}
		for (i = 0; i < 4; i++) {
			if (strcmp(reg, sreg[i]) == 0) {
				e86_set_sreg(c, i, val);
				return (0);
			}
		}
		for (i = 0; i < 2; i++) {
			if (strcmp(reg, sreg386[i]) == 0) {
				e386_set_sreg_fs_gs(c, i, val);
				return (0);
			}
		}
		if (strcmp(reg, "eip") == 0) {
			e386_set_eip(c, val);
			return (0);
		}
		if (strcmp(reg, "ip") == 0) {
			e86_set_ip(c, val);
			return (0);
		}
		if (strcmp(reg, "eflags") == 0) {
			e386_set_flags(c, val);
			return (0);
		}
		if (strcmp(reg, "flags") == 0) {
			e286_set_flags(c, val);
			return (0);
		}
		return (1);
	}
	else {
		/* Intel 80186 CPU or  lower */
		if (*reg == '%') {
			reg += 1;
		}
		for (i = 0; i < 8; i++) {
			if (strcmp(reg, dreg16[i]) == 0) {
				e86_set_reg16(c, i, val);
				return (0);
			}
			if (strcmp(reg, dreg8[i]) == 0) {
				e86_set_reg8(c, i, val);
				return (0);
			}
		}
		for (i = 0; i < 4; i++) {
			if (strcmp(reg, sreg[i]) == 0) {
				e86_set_sreg(c, i, val);
				return (0);
			}
		}
		if (strcmp(reg, "ip") == 0) {
			e86_set_ip(c, val);
			return (0);
		}
		if (strcmp(reg, "flags") == 0) {
			e86_set_flags(c, val);
			return (0);
		}
		return (1);
	}
}

void e86_trap(e8086_t *c, unsigned n)
{
	if (c->cpumodel >= 0x80386) {
		/* Intel 80386 CPU or higher */
		unsigned short ip;
		unsigned short ofs;
		if (c->op_int != NULL) {
			c->op_int(c->op_ext, n);
		}
		if (c->prefix & E86_PREFIX_KEEP) {
			ip = e86_get_cur_ip(c);
			if (c->cpu & E86_CPU_REP_BUG) {
				if (c->prefix & (E86_PREFIX_REP | E86_PREFIX_REPN)) {
					if (c->prefix & E86_PREFIX_SEG) {
						ip += 1;
					}
				}
			}
			c->prefix = 0;
		}
		else {
			ip = e86_get_ip(c);
		}
		c->int_cnt += 1;
		c->int_vec = n;
		c->int_cs = e86_get_cs(c);
		c->int_ip = ip;
		e86_push(c, e86_get_flags(c));
		e86_push(c, e86_get_cs(c));
		e86_push(c, ip);
		ofs = (unsigned short)(n & 0xff) << 2;
		e86_set_ip(c, e86_get_mem16(c, 0, ofs));
		e86_set_cs(c, e86_get_mem16(c, 0, ofs + 2));
		c->flg &= ~(E86_FLG_IF | E86_FLG_TF);
		e86_pq_init(c);
	}
	else {
		/* Intel 80286 CPU or  lower */
		unsigned short ip;
		unsigned short ofs;
		if (c->op_int != NULL) {
			c->op_int(c->op_ext, n);
		}
		if (c->prefix & E86_PREFIX_KEEP) {
			ip = e86_get_cur_ip(c);
			if (c->cpu & E86_CPU_REP_BUG) {
				if (c->prefix & (E86_PREFIX_REP | E86_PREFIX_REPN)) {
					if (c->prefix & E86_PREFIX_SEG) {
						ip += 1;
					}
				}
			}
			c->prefix = 0;
		}
		else {
			ip = e86_get_ip(c);
		}
		c->int_cnt += 1;
		c->int_vec = n;
		c->int_cs = e86_get_cs(c);
		c->int_ip = ip;
		e86_push(c, e86_get_flags(c));
		e86_push(c, e86_get_cs(c));
		e86_push(c, ip);
		ofs = (unsigned short)(n & 0xff) << 2;
		e86_set_ip(c, e86_get_mem16(c, 0, ofs));
		e86_set_cs(c, e86_get_mem16(c, 0, ofs + 2));
		c->flg &= ~(E86_FLG_IF | E86_FLG_TF);
		e86_pq_init(c);
	}
}

void e86_irq(e8086_t *cpu, unsigned char val)
{
	cpu->irq = (val != 0);
}

static
void e86_irq_ack(e8086_t *c)
{
	unsigned char irq;
	c->irq = 0;
	c->halt = 0;
	if (c->inta != NULL) {
		irq = c->inta(c->inta_ext);
		e86_set_clk(c, 61);
		e86_trap(c, irq);
	}
}

int e86_interrupt(e8086_t *cpu, unsigned n)
{
	if (cpu->cpumodel >= 0x80386) {
		/* Intel 80386 CPU or higher */
		if (e86_get_if(cpu)) {
			cpu->halt = 0;
			e86_trap(cpu, n);
			return (0);
		}
		return (1);
	}
	else {
		/* Intel 80286 CPU or  lower */
		if (e86_get_if(cpu)) {
			cpu->halt = 0;
			e86_trap(cpu, n);
			return (0);
		}
		return (1);
	}
}

unsigned e86_undefined(e8086_t *c)
{
	if (c->cpumodel >= 0x80386) {
		/* Intel 80386 CPU or higher */
		if (c->op_undef != NULL) {
			c->op_undef(c->op_ext, c->pq[0], c->pq[1]);
		}
		if (c->cpu & E86_CPU_INT6) {
			e86_trap(c, 6);
			e86_set_clk(c, 50);
			/* return (0); */
		}
		/* return (1); */
	}
	else {
		/* Intel 80286 CPU or  lower */
		if (c->op_undef != NULL) {
			c->op_undef(c->op_ext, c->pq[0], c->pq[1]);
		}
		if (c->cpu & E86_CPU_INT6) {
			e86_trap(c, 6);
			e86_set_clk(c, 50);
			/* return (0); */
		}
		/* return (1); */
	}
}

unsigned long long e86_get_clock(e8086_t *c)
{
	return (c->clocks);
}

unsigned long long e86_get_opcnt(e8086_t *c)
{
	return (c->instructions);
}

void e86_reset(e8086_t *c)
{
	if (c->cpumodel >= 0x80386) {
		/* Intel 80386 CPU or higher */
		unsigned i;
		c->instructions = 0;
		c->addr_mask = 0xffffffff;
		for (i = 0; i < 8; i++) {
			c->dreg[i] = 0;
		}
		for (i = 0; i < 4; i++) {
			c->sreg[i] = 0;
		}
		for (i = 0; i < 2; i++) {
			c->sreg386[i] = 0;
		}
		e386_get_flags(c);
		e286_get_machine_status_word(c);
		e286_set_pe(c, 0);
		e386_set_vm(c, 0);
		e86_set_cs(c, 0xf000);
		e86_set_ds(c, 0x0000);
		e86_set_es(c, 0x0000);
		e86_set_ss(c, 0x0000);
		e386_set_fs(c, 0x0000); /* Set FS segment register to 0000H */
		e386_set_gs(c, 0x0000); /* Set GS segment register to 0000H */
		c->cs_segment_register_descriptor_limit = 0x0ffff; /* Load default segment limits for Real Mode */
		c->ds_segment_register_descriptor_limit = 0x0ffff; /* Load default segment limits for Real Mode */
		c->es_segment_register_descriptor_limit = 0x0ffff; /* Load default segment limits for Real Mode */
		c->fs_segment_register_descriptor_limit = 0x0ffff; /* Load default segment limits for Real Mode */
		c->gs_segment_register_descriptor_limit = 0x0ffff; /* Load default segment limits for Real Mode */
		c->ss_segment_register_descriptor_limit = 0x0ffff; /* Load default segment limits for Real Mode */
		e386_set_eip(c, 0x0000fff0);
		e386_set_flags(c, 0x00000002);
		e286_set_machine_status_word(c, 0x00000000); /* Initial Contents of CR0 (Control Register Zero) */
		e86_set_ax(c, 0x0000); /* CPU has passed self-test feature */
		e86_set_dx(c, 0x0308); /* D2 stepping */
		c->e386_address_size_prefix = 0;
		c->e386_operand_size_prefix = 0;
		c->pq_cnt = 0;
		c->irq = 0;
		c->halt = 0;
		c->prefix = 0;
	}
	else {
		/* Intel 80286 CPU or  lower */
		unsigned i;
		c->instructions = 0;
		c->addr_mask = 0xfffff;
		for (i = 0; i < 8; i++) {
			c->dreg[i] = 0;
		}
		for (i = 0; i < 4; i++) {
			c->sreg[i] = 0;
		}
		e86_set_cs(c, 0xf000);
		e86_set_ip(c, 0xfff0);
		e86_set_flags(c, 0x0000);
		c->pq_cnt = 0;
		c->irq = 0;
		c->halt = 0;
		c->prefix = 0;
	}
}

void e86_execute(e8086_t *c)
{
	unsigned       cnt;
	unsigned short flg;
	char           irq;
	if (c->cpumodel >= 0x80386) {
		/* Intel 80386 CPU or higher */
		if (c->halt) {
			e86_set_clk(c, 2);
			if (c->irq && e86_get_if(c)) {
				e86_irq_ack(c);
			}
			return;
		}
		if ((c->prefix & E86_PREFIX_KEEP) == 0) {
			c->prefix = 0;
			c->cur_ip = c->ip;
		}
		flg = c->flg;
		irq = c->irq;
		c->enable_int = 1;
		do {
			e86_pq_fill(c);
			c->prefix &= ~E86_PREFIX_NEW;
			if (c->op_stat != NULL) {
				c->op_stat(c->op_ext, c->pq[0], c->pq[1]);
			}
			cnt = c->op[c->pq[0]](c);
			if (cnt > 0) {
				c->ip = (c->ip + cnt) & 0xffff;
				e86_pq_adjust(c, cnt);
			}
			else {
				c->delay += 10;
			}
		} while (c->prefix & E86_PREFIX_NEW);
		c->instructions += 1;
		if (c->enable_int) {
			if (flg & c->flg & E86_FLG_TF) {
				c->halt = 0;
				e86_trap(c, 1);
			}
			else if (irq && c->irq && e86_get_if(c)) {
				e86_irq_ack(c);
			}
		}
	}
	else {
		/* Intel 80286 CPU or  lower */
		if (c->halt) {
			e86_set_clk(c, 2);
			if (c->irq && e86_get_if(c)) {
				e86_irq_ack(c);
			}
			return;
		}
		if ((c->prefix & E86_PREFIX_KEEP) == 0) {
			c->prefix = 0;
			c->cur_ip = c->ip;
		}
		irq = c->irq;
		c->enable_int = 1;
		do {
			e86_pq_fill(c);
			c->prefix &= ~E86_PREFIX_NEW;
			if (c->op_stat != NULL) {
				c->op_stat(c->op_ext, c->pq[0], c->pq[1]);
			}
			cnt = c->op[c->pq[0]](c);
			if (cnt > 0) {
				c->ip = (c->ip + cnt) & 0xffff;
				e86_pq_adjust(c, cnt);
			}
			else {
				c->delay += 10;
			}
		} while (c->prefix & E86_PREFIX_NEW);
		c->instructions += 1;
		if (c->enable_int) {
			if (c->flg & E86_FLG_TF) {
				c->halt = 0;
				e86_trap(c, 1);
			}
			else if (irq && c->irq && e86_get_if(c)) {
				e86_irq_ack(c);
			}
		}
	}
}

void e86_clock(e8086_t *c, unsigned n)
{
	while (n >= c->delay) {
		n -= c->delay;
		c->clocks += c->delay;
		c->delay = 0;
		e86_execute(c);
	}
	c->delay -= n;
	c->clocks += n;
}
