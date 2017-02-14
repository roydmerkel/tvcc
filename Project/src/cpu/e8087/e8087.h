/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/e8086/e8086.h                                        *
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


#ifndef PCE_E8087_H
#define PCE_E8087_H 1


#include <stdio.h>

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
/* CPU options */
#define E87_CPU_REP_BUG    0x01         /* enable rep/seg bug */
#define E87_CPU_MASK_SHIFT 0x02         /* mask shift count */
#define E87_CPU_PUSH_FIRST 0x04         /* push sp before decrementing it */
#define E87_CPU_INT6       0x08         /* throw illegal opcode exception */
#define E87_CPU_INT7       0x10		/* throw escape opcode exception */
#define E87_CPU_FLAGS286   0x20         /* Allow clearing flags 12-15 */
#define E87_CPU_8BIT       0x40		/* 16 bit accesses take more time */

/* CPU flags */
#define E87_FLG_C 0x0001
#define E87_FLG_P 0x0004
#define E87_FLG_A 0x0010
#define E87_FLG_Z 0x0040
#define E87_FLG_S 0x0080
#define E87_FLG_T 0x0100
#define E87_FLG_I 0x0200
#define E87_FLG_D 0x0400
#define E87_FLG_O 0x0800
/* -------------------------------------------------------------------------------------------------------- -- */
/* IBM PC (Model 5150/5160): Quote from Technical Reference; Section 2. Coprocessor: Programming Interface:    */
/* ----------------------------------------------------------------------------------------------------------- */
/* The coprocessor extends the data types, registers, and instructions to the microprocessor.                  */
/*                                                                                                             */
/* The coprocessor has eight 80-bit registers, which provide the equivalent capacity of the 40 16-bit registers*/
/* found in the microprocessor. This register space allows constants and temporary results to be held in       */
/* registers during calculations, thus reducing memory access and improving speed as well as bus availability. */
/* The register space can be used as a stack or as a fixed register set. When used as a stack, only the top two*/
/* stack elements are operated on. When used as a stack, only the top two stack elements are operated on. The  */
/* figure below shows representations of large and small numbers in each data type.                            */
/*  ---------------------------------------------------------------------------------------------------------  */
/*  Data Types:                                                                                                */
/*  ---------------------------------------------------------------------------------------------------------  */
/*  Data Type:         Bits: Significant Digits (Decimal): Approximate Range (decimal):                        */
/*   - Word  Integer  : - 16: -  4:                         - -32,768  <=X<= + 32,767                          */
/*   - Short Integer  : - 32: -  9:                         - -2x10(9) <=X<= + 2x10(0)                         */
/*   - Long  Integer  : - 64: - 18:                         - -9x10(18)<=X<= + 9x10(18)                        */
/*   - Packed Decimal : - 80: - 18:                         - -99...99 <=X<= + 99...99(18 digits)              */
/*   - Short     Real*: - 32: - 6-7:                        - 8.43x10-(37) <=|X|<= 3.37x10(38)                 */
/*   - Long      Real*: - 64: - 15-16:                      - 4.19x10-(307)<=|X|<= 1.67x10(308)                */
/*   - Temporary Real : - 80: - 19:                         - 3.4x10-(4932)<=|X|<= 1.2x10(4932)                */
/*   *The short and long real data types correspond to the single and double precision data types.             */
/* ----------------------------------------------------------------------------------------------------------- */
/* 16 bit register values */
#define E87_REG_AX 0
#define E87_REG_CX 1
#define E87_REG_DX 2
#define E87_REG_BX 3
#define E87_REG_SP 4
#define E87_REG_BP 5
#define E87_REG_SI 6
#define E87_REG_07 7

/* Segment register values */
#define E87_REG_ES 0
#define E87_REG_CS 1
#define E87_REG_SS 2
#define E87_REG_DS 3
/* ------------------------------------------------------------------------------------------------------------- */
/* IBM PC (Model 5150/5160): Quote from Technical Reference; Section 2. Coprocessor: Hardware Interface:         */
/* ------------------------------------------------------------------------------------------------------------- */
/* The coprocessor uses the same clock generator and system bus interface components as the microprocessor. The  */
/* coprocessor is wired directly into the microprocessor. The microprocessor's queue status lines (QS0 and QS1)  */
/* enable the coprocessor to obtain and decode instructions simultaneously with the microprocessor. The          */
/* coprocessor's 'busy' signal informs the microprocessor that it is executing; the microprocessor's WAIT        */
/* instruction forces the microprocessor to wait until the coprocessor is finished executing (WAIT FOR NOT BUSY).*/
/*                                                                                                               */
/* When an incorrect instruction is sent to the coprocessor (for example, divide by 0 or load a full register),  */
/* the coprocessor can signal the microprocessor with an interrupt. There are three conditions that will disable */
/* the coprocessor interrupt to the microprocessor:                                                              */
/*  - 1. Exception and interrupt-enable bits of the control word are set to 1's.                                 */
/*  - 2. System-board switch-block 1, switch 2, set in the On position.                                          */
/*  - 3. Non-maskable interrupt (NMI) register (REG) is set to zero.                                             */
/* At power-on time, the NMI REG is cleared to disable the NMI. Any program using the coprocessor's interrupt    */
/* capability must ensure that conditions 2 and 3 are never met during the operation of the software or an       */
/* "Endless WAIT" will occus. An "Endless WAIT" will have the microprocessor waiting for the 'not busy' signal   */
/* from the coprocessor while the coprocessor is waiting for the microprocessor to interrupt.                    */
/*                                                                                                               */
/* Because a memory parity error may also cause an interrupt to the microprocessor NMI line, the program should  */
/* check the coprocessor status for an exception condition. If a coprocessor exception condition is not found,   */
/* control should be passed to the normal NMI handler. If an 8087 exception condition is found, the program may  */
/* clear the exception by executing the FNSAVE or the FNCLEX instruction, and the exception can be identified and*/
/* acted upon.                                                                                                   */
/*                                                                                                               */
/* The NMI REG and the coprocessor's interrupt are tied to the NMI line through the NMI interrupt logic. Minor   */
/* modifications to programs designed for use with a coprocessor must be made before the programs will be        */
/* compatible with the IBM Personal Computer Math Coprocessor.                                                   */
/*                                                                                                               */
/* Coprocessor Interconnection:                                                                                  */
/*  - Memory and System Board: --> NMI INT Logic: NMI:  -->  NMI (8088 CPU):                                     */
/*                                 NMI INT Logic: INT: |-->  CLK (8088 CPU): <->|                                */
/*                                                 ^   | |-> RQ/GT1:          |    8088 Family:   --> Multimaster*/
/*                                                 |   | |   OS0: OS1: TEST:  |--> Bus Interface: --> System Bus */
/*                                                 |   | |    |    |    ^     |    Components:                   */
/*                                                 |   | |    V    V    |     |                                  */
/*                                                 |   | |   OS0: OS1: BUSY:  |                                  */
/*                                                 |   | |-> RQ/GTO:   Math  <->                                 */
/*  - 8284 Clock Generator CLK: -------------------|---|---> CLK:  Coprocessor:                                  */
/*                                                 |-------> INT:                                                */
/*                                                                RQ/GT1:                                        */
/* Detailed information for the internal functions of the Intel 8087 Coprocessor can be found in the books listed*/
/* in the Bibliography.                                                                                          */
/* ------------------------------------------------------------------------------------------------------------- */
#define E87_PREFIX_NEW  0x0001
#define E87_PREFIX_SEG  0x0002
#define E87_PREFIX_REP  0x0004
#define E87_PREFIX_REPN 0x0008
#define E87_PREFIX_LOCK 0x0010
#define E87_PREFIX_KEEP 0x0020

#define E87_PQ_MAX 16


struct e8087_t;


typedef unsigned char (*e87_get_uint8_f) (void *ext, unsigned long addr);
typedef unsigned short (*e87_get_uint16_f) (void *ext, unsigned long addr);
typedef void (*e87_set_uint8_f) (void *ext, unsigned long addr, unsigned char val);
typedef void (*e87_set_uint16_f) (void *ext, unsigned long addr, unsigned short val);

typedef unsigned (*e87_opcode_f) (struct e8087_t *c);


typedef struct e8087_t {
	unsigned         cpu;

	unsigned short   dreg[8];
	unsigned short   sreg[4];
	unsigned short   ip;
	unsigned short   flg;

	void             *mem;
	e86_get_uint8_f  mem_get_uint8;
	e86_set_uint8_f  mem_set_uint8;
	e86_get_uint16_f mem_get_uint16;
	e86_set_uint16_f mem_set_uint16;

	void             *prt;
	e86_get_uint8_f  prt_get_uint8;
	e86_set_uint8_f  prt_set_uint8;
	e86_get_uint16_f prt_get_uint16;
	e86_set_uint16_f prt_set_uint16;

	unsigned char    *ram;
	unsigned long    ram_cnt;

	unsigned long    addr_mask;

	void             *inta_ext;
	unsigned char    (*inta) (void *ext);

	void             *op_ext;
	void             (*op_hook) (void *ext, unsigned char op1, unsigned char op2);
	void             (*op_stat) (void *ext, unsigned char op1, unsigned char op2);
	void             (*op_undef) (void *ext, unsigned char op1, unsigned char op2);
	void             (*op_int) (void *ext, unsigned char n);

	unsigned short   cur_ip;

	unsigned         pq_size;
	unsigned         pq_fill;
	unsigned         pq_cnt;
	unsigned char    pq[E87_PQ_MAX];

	unsigned         prefix;

	unsigned short   seg_override;

	int              halt;

	char             irq;
	char             enable_int;

	unsigned         int_cnt;
	unsigned char    int_vec;
	unsigned short   int_cs;
	unsigned short   int_ip;

	e87_opcode_f     op[256];

	struct {
		int            is_mem;
		unsigned char  *data;
		unsigned short seg;
		unsigned short ofs;
		unsigned short cnt;
	} ea;

	unsigned long    delay;

	unsigned long long clocks;
	unsigned long long instructions;
} e8087_t;


#define e87_get_reg80(cpu, reg) \
	((((reg) & 4) ? ((cpu)->dreg[(reg) & 3] >> 16) : (cpu)->dreg[(reg) & 3]) & 0xffffffffffffffffff) /* Extended to 80-bits from 8-bit 8086 CPU registers */

#define e87_get_reg16(cpu, reg) ((cpu)->dreg[(reg) & 7])
#define e87_get_ax(cpu) ((cpu)->dreg[E87_REG_AX])
#define e87_get_bx(cpu) ((cpu)->dreg[E87_REG_BX])
#define e87_get_cx(cpu) ((cpu)->dreg[E87_REG_CX])
#define e87_get_dx(cpu) ((cpu)->dreg[E87_REG_DX])
#define e87_get_sp(cpu) ((cpu)->dreg[E87_REG_SP])
#define e87_get_bp(cpu) ((cpu)->dreg[E87_REG_BP])
#define e87_get_si(cpu) ((cpu)->dreg[E87_REG_SI])
#define e87_get_di(cpu) ((cpu)->dreg[E87_REG_DI])
#define e87_get_07(cpu) ((cpu)->dreg[E87_REG_07])


#define e87_get_sreg(cpu, reg) ((cpu)->sreg[(reg) & 3])
#define e87_get_cs(cpu) ((cpu)->sreg[E87_REG_CS])
#define e87_get_ds(cpu) ((cpu)->sreg[E87_REG_DS])
#define e87_get_es(cpu) ((cpu)->sreg[E87_REG_ES])
#define e87_get_ss(cpu) ((cpu)->sreg[E87_REG_SS])


#define e87_set_reg80(cpu, reg, val) \
	do { \
		unsigned char v = (val) & 0xffffffffffffffffff; \
		unsigned      r = (reg); \
		if (r & 4) { \
			(cpu)->dreg[r & 3] &= 0x000000000000000000ffffffffffffffffff; \
			(cpu)->dreg[r & 3] |= v << 8; \
		} \
		else { \
			(cpu)->dreg[r & 3] &= 0xffffffffffffffffff000000000000000000; \
			(cpu)->dreg[r & 3] |= v; \
		} \
	} while (0)

#define e87_set_ax(cpu, val) do { (cpu)->dreg[E87_REG_AX] = (val) & 0xffffffffffffffffff; } while (0)
#define e87_set_bx(cpu, val) do { (cpu)->dreg[E87_REG_BX] = (val) & 0xffffffffffffffffff; } while (0)
#define e87_set_cx(cpu, val) do { (cpu)->dreg[E87_REG_CX] = (val) & 0xffffffffffffffffff; } while (0)
#define e87_set_dx(cpu, val) do { (cpu)->dreg[E87_REG_DX] = (val) & 0xffffffffffffffffff; } while (0)
#define e87_set_sp(cpu, val) do { (cpu)->dreg[E87_REG_SP] = (val) & 0xffffffffffffffffff; } while (0)
#define e87_set_bp(cpu, val) do { (cpu)->dreg[E87_REG_BP] = (val) & 0xffffffffffffffffff; } while (0)
#define e87_set_si(cpu, val) do { (cpu)->dreg[E87_REG_SI] = (val) & 0xffffffffffffffffff; } while (0)
#define e87_set_di(cpu, val) do { (cpu)->dreg[E87_REG_DI] = (val) & 0xffffffffffffffffff; } while (0)
#define


#define e87_set_sreg(cpu, reg, val) \
	do { (cpu)->sreg[(reg) & 3] = (val) & 0xffff; } while (0)

#define e87_set_cs(cpu, val) do { (cpu)->sreg[E87_REG_CS] = (val) & 0xffff; } while (0)
#define e87_set_ds(cpu, val) do { (cpu)->sreg[E87_REG_DS] = (val) & 0xffff; } while (0)
#define e87_set_es(cpu, val) do { (cpu)->sreg[E87_REG_ES] = (val) & 0xffff; } while (0)
#define e87_set_ss(cpu, val) do { (cpu)->sreg[E87_REG_SS] = (val) & 0xffff; } while (0)


#define e87_get_ip(cpu) ((cpu)->ip)
#define e87_get_cur_ip(cpu) ((cpu)->cur_ip)
#define e87_set_ip(cpu, val) do { (cpu)->ip = (val) & 0xffff; } while (0)


#define e87_get_flags(cpu) ((cpu)->flg)
#define e87_get_f(cpu, f) (((cpu)->flg & (f)) != 0)
#define e87_get_cf(cpu) (((cpu)->flg & E87_FLG_C) != 0)
#define e87_get_pf(cpu) (((cpu)->flg & E87_FLG_P) != 0)
#define e87_get_af(cpu) (((cpu)->flg & E87_FLG_A) != 0)
#define e87_get_zf(cpu) (((cpu)->flg & E87_FLG_Z) != 0)
#define e87_get_of(cpu) (((cpu)->flg & E87_FLG_O) != 0)
#define e87_get_sf(cpu) (((cpu)->flg & E87_FLG_S) != 0)
#define e87_get_df(cpu) (((cpu)->flg & E87_FLG_D) != 0)
#define e87_get_if(cpu) (((cpu)->flg & E87_FLG_I) != 0)
#define e87_get_tf(cpu) (((cpu)->flg & E87_FLG_T) != 0)


#define e87_set_flags(c, v) do { (c)->flg = (v) & 0xffffU; } while (0)

#define e87_set_f(c, f, v) \
	do { if (v) (c)->flg |= (f); else (c)->flg &= ~(f); } while (0)

#define e87_set_cf(c, v) e87_set_f (c, E87_FLG_C, v)
#define e87_set_pf(c, v) e87_set_f (c, E87_FLG_P, v)
#define e87_set_af(c, v) e87_set_f (c, E87_FLG_A, v)
#define e87_set_zf(c, v) e87_set_f (c, E87_FLG_Z, v)
#define e87_set_of(c, v) e87_set_f (c, E87_FLG_O, v)
#define e87_set_sf(c, v) e87_set_f (c, E87_FLG_S, v)
#define e87_set_df(c, v) e87_set_f (c, E87_FLG_D, v)
#define e87_set_if(c, v) e87_set_f (c, E87_FLG_I, v)
#define e87_set_tf(c, v) e87_set_f (c, E87_FLG_T, v)


#define e87_get_linear(seg, ofs) \
	((((seg) & 0xffffUL) << 4) + ((ofs) & 0xffff))


static inline
unsigned char e87_get_mem8 (e8087_t *c, unsigned short seg, unsigned short ofs)
{
	unsigned long addr = e87_get_linear (seg, ofs) & c->addr_mask;
	return ((addr < c->ram_cnt) ? c->ram[addr] : c->mem_get_uint8 (c->mem, addr));
}

static inline
void e87_set_mem8 (e8087_t *c, unsigned short seg, unsigned short ofs, unsigned char val)
{
	unsigned long addr = e87_get_linear (seg, ofs) & c->addr_mask;

	if (addr < c->ram_cnt) {
		c->ram[addr] = val;
	}
	else {
		c->mem_set_uint8 (c->mem, addr, val);
	}
}

static inline
unsigned short e87_get_mem16 (e8087_t *c, unsigned short seg, unsigned short ofs)
{
	unsigned long addr = e87_get_linear (seg, ofs) & c->addr_mask;

	if ((addr + 1) < c->ram_cnt) {
		return (c->ram[addr] + (c->ram[addr + 1] << 8));
	}
	else {
		return (c->mem_get_uint16 (c->mem, addr));
	}
}

static inline
void e87_set_mem16 (e8087_t *c, unsigned short seg, unsigned short ofs, unsigned short val)
{
	unsigned long addr = e87_get_linear (seg, ofs) & c->addr_mask;

	if ((addr + 1) < c->ram_cnt) {
		c->ram[addr] = val & 0xff;
		c->ram[addr + 1] = (val >> 8) & 0xff;
	}
	else {
		c->mem_set_uint16 (c->mem, addr, val);
	}
}

#define e87_get_prt8(cpu, ofs) \
	(cpu)->prt_get_uint8 ((cpu)->prt, ofs)

#define e87_get_prt16(cpu, ofs) \
	(cpu)->prt_get_uint16 ((cpu)->prt, ofs)

#define e87_set_prt8(cpu, ofs, val) \
	do { (cpu)->prt_set_uint8 ((cpu)->prt, ofs, val); } while (0)

#define e87_set_prt16(cpu, ofs, val) \
	do { (cpu)->prt_set_uint16 ((cpu)->prt, ofs, val); } while (0)

#define e87_get_delay(c) ((c)->delay)


void e87_init (e8087_t *c);
void e87_free (e8087_t *c);

e8087_t *e87_new (void);
void e87_del (e8087_t *c);

void e87_set_8087 (e8087_t *c);
void e87_set_8088_87 (e8087_t *c);
void e87_set_80187 (e8087_t *c);
void e87_set_80188_87 (e8087_t *c);
void e87_set_80287 (e8087_t *c);
void e87_set_v30_87 (e8087_t *c);
void e87_set_v20_87 (e8087_t *c);

/*!***************************************************************************
 * @short Set the prefetch queue size
 * @param size The emulated prefetch queue size
 *****************************************************************************/
void e87_set_pq_size (e8087_t *c, unsigned size);

/*!***************************************************************************
 * @short Set CPU options
 * @param opt A bit mask indicating the desired options
 * @param set If true, the options in opt are set otherwise they are reset
 *****************************************************************************/
void e87_set_options (e8087_t *c, unsigned opt, int set);

void e87_set_addr_mask (e8087_t *c, unsigned long msk);
unsigned long e87_get_addr_mask (e8087_t *c);

void e87_set_inta_fct (e8087_t *c, void *ext, void *fct);

void e87_set_ram (e8087_t *c, unsigned char *ram, unsigned long cnt);

void e87_set_mem (e8087_t *c, void *mem,
	e87_get_uint8_f get8, e87_set_uint8_f set8,
	e87_get_uint16_f get16, e87_set_uint16_f set16
);

void e87_set_prt (e8087_t *c, void *prt,
	e87_get_uint8_f get8, e87_set_uint8_f set8,
	e87_get_uint16_f get16, e87_set_uint16_f set16
);

int e87_get_reg (e8087_t *c, const char *reg, unsigned long *val);
int e87_set_reg (e8087_t *c, const char *reg, unsigned long val);

void e87_irq (e8087_t *cpu, unsigned char val);

int e87_interrupt (e8087_t *cpu, unsigned n);

unsigned e87_undefined (e8087_t *c);

unsigned long long e87_get_clock (e8087_t *c);

unsigned long long e87_get_opcnt (e8087_t *c);

void e87_reset (e8087_t *c);

void e87_execute (e8087_t *c);

void e87_clock (e8087_t *c, unsigned n);


void e87_push (e8087_t *c, unsigned short val);
unsigned short e87_pop (e8087_t *c);
void e87_trap (e8087_t *c, unsigned n);

void e87_pq_init (e8087_t *c);
void e87_pq_fill (e8087_t *c);


#define E87_DFLAGS_187  0x0001
#define E87_DFLAGS_CALL 0x0100
#define E87_DFLAGS_LOOP 0x0200

typedef struct {
	unsigned       flags;

	unsigned       seg;

	unsigned short ip;
	unsigned       dat_n;
	unsigned char  dat[16];

	char           op[64];

	unsigned       arg_n;
	char           arg1[64];
	char           arg2[64];
} e87_disasm_t;


void e87_disasm (e87_disasm_t *op, unsigned char *src, unsigned short ip);
void e87_disasm_mem (e8087_t *c, e86_disasm_t *op, unsigned short, unsigned short ip);
void e87_disasm_cur (e8087_t *c, e86_disasm_t *op);


#endif
