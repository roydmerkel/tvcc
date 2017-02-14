/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/ibmpc/xms.c                                         *
 * Created:     2003-09-01 by Hampa Hug <hampa@hampa.ch>                     *
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


#include <cpu/e8086/e8086.h>
/* #include <cpu/e8086/mmu386.h> */

#include <stdlib.h>
#include <string.h>

#include <lib/console.h>
#include <lib/log.h>

#include <libini/libini.h>
/* Source code example for determining granularity bit while loading descriptor tables: */
/*void gdt_granularity_bit (gdt_global_descriptor_t *global_descriptor)
{
	unsigned long val;
	unsigned long address_20_bit;
	if (global_descriptor->gdt_granularity_bit == 1) {
 		global_descriptor->gdt_segment_limit = val;
	}
	else if (global_descriptor->gdt_granularity_bit == 1) && ((val) < 0xFFF)  {
 		global_descriptor->gdt_segment_limit = 0xFFF;
	}
	else {
		address_20_bit = (val) & 0xfffff;
		global_descriptor->gdt_segment_limit = address_20_bit;
	}
} */

/* Source code example for loading segment selectors into GDTR or LDTR registers: */
/* void gdt_get_selector (gdt_global_descriptor_t *global_descriptor)
{
	unsigned long selector_index;
	unsigned long index;
	selector_index = global_descriptor->gdt_selector_index;
	index = (selector_index) * 8;
	global_descriptor->gdtr_selector_index = index;
} */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual (5.1):                                                      */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Because the first entry of the GDT is not used by the processor, a selector that has an index of zero and a table*/
/* indicator of zero (i.e., a selector that points to the first entry of the GDT), can be used as a null selector.  */
/* The processor does not cause an exception when a segment register (other than CS or SS) is loaded with a null    */
/*selector. It will, however, cause an exception when the segment register is used to access memory. This feature is*/
/* useful for initializing unused segment registers so as to trap accidental references.                            */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Source code example for triggering a general protection fault: */
/*void gdt_general_protection_fault (gdt_global_descriptor_t *global_descriptor)
{
	unsigned long val;
	if ((global_descriptor->gdt_segment_type_e == 0) && (val) > (global_descriptor->gdt_segment_limit)) {
		return (NULL); /* Replace with actual exception CPU interrupt */
/*	}
	else if ((global_descriptor->gdt_segment_type_e == 1) && (val) < (global_descriptor->gdt_segment_limit)) {
		return (NULL); /* Replace with actual exception CPU interrupt */
/*	}
} */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual (6.3.1.2 Limit Checking):                                   */
/* ---------------------------------------------------------------------------------------------------------------- */
/* The limit field of a segment descriptor is used by the processor to prevent programs from addressing outside the */
/* segment. The processor's interpretation of the limit depends on the setting of the G (granularity) bit. For data */
/* segments, the processor's interpretation of the limit depends also on the E-bit (expansion-direction bit) and the*/
/* B-bit (big bit) (refer to Table 6-2).                                                                            */
/*                                                                                                                  */
/* When G=0, the actual limit is the value of the 20-bit limit field as it appears in the descriptor. In this case, */
/* the limit may range from 0 to 0FFFFH (2^(20) - 1 or 1 megabyte). When G=1, the processor appends 12 low-order one*/
/* -bits to the value in the limit field. In this case the actual limit may range from 0FFFH (2^(12) - 1 ir 4       */
/* kilobytes) to 0FFFFFFFFH(2^(32) - 1 or 4 gigabytes).                                                             */
/*                                                                                                                  */
/* For all types of segments except expand-down data segments, the value of the limit is one less than the size     */
/* (expressed in bytes) of the segment. The processor causes a general protection exception in any of these cases:  */
/*  - Attempt to access a memory byte at an address > limit.                                                        */
/*  - Attempt to access a memory word at an address >= limit.                                                       */
/*  - Attempt to access a memory doubleword at an address >= (limit-2).                                             */
/* For expand-down data segments, the limit has the same function but is interpreted differently. In these cases the*/
/* range of valid addresses is from limit + 1 to either 64K or 2^(32) - 1 (4 Gbytes) depending on the B-bit. An     */
/*expand-down segment has maximum size when the limit is zero. (Turning on the expand-down bit swaps which bytes are*/
/* accessible and which are not.)                                                                                   */
/*                                                                                                                  */
/* The expand-down feature makes it possible to expand the size of a stack by copying it to a larger segment without*/
/* needing also to update intrastack pointers.                                                                      */
/*                                                                                                                  */
/* The limit field of descriptors for descriptor tables is used by the processor to prevent programs from selecting */
/* a table entry outside the descriptor descriptor table. The limit of a descriptor table identifies the last valid */
/* byte of the last descriptor in the table. Since each descriptor is eight bytes long, the limit value is N * 8 - 1*/
/* for a table that can contain up to N descriptors.                                                                */
/*                                                                                                                  */
/*Limit checking catches programming errors such as runaway subscripts and invalid pointer calculations. Such errors*/
/* are detected when they occur, so that identification of the cause is easier. Without limit checking, such errors */
/* could corrupt other modules; the existence of such errors would not be discovered until later, when the corrupted*/
/* module behaves incorrectly, and when the identification of the cause is difficult.                               */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Source code example for loading global or local descriptor table into CS segment register: */
/*void gdt_load_gdt_into_cs_register (gdt_global_descriptor_t *global_descriptor)
{
	unsigned long val;
	if ((c->cs_segment_register_descriptor->cs_segment_register_descriptor_privilege_level) < (current_privilege_level->cpl) && (global_descriptor->gdt_selector_requester_privilege_level)) {
		return (NULL); /* Replace with actual exception CPU interrupt */
/*	}
	else {
		c->cs_segment_register_descriptor->cs_segment_register_descriptor_base_address_offset = global_descriptor->gdt_segment_base_address_offset = NULL;
		c->cs_segment_register_descriptor->cs_segment_register_descriptor_limit = global_descriptor->gdt_segment_limit;
		c->cs_segment_register_descriptor->cs_segment_register_descriptor_is_present = global_descriptor->gdt_segment_is_present;
		c->cs_segment_register_descriptor->cs_segment_register_descriptor_granularity_bit = global_descriptor->gdt_segment_granularity_bit;
		c->cs_segment_register_descriptor->cs_segment_register_descriptor_big = global_descriptor->gdt_segment_big;
		c->cs_segment_register_descriptor->cs_segment_register_descriptor_x = global_descriptor->gdt_segment_x;
		c->cs_segment_register_descriptor->cs_segment_register_descriptor_o_ = global_descriptor->gdt_segment_o_;
		c->cs_segment_register_descriptor->cs_segment_register_descriptor_available = global_descriptor->gdt_segment_available;
		c->cs_segment_register_descriptor->cs_segment_register_descriptor_is_present = global_descriptor->gdt_segment_is_present;
		c->cs_segment_register_descriptor->cs_segment_register_descriptor_privilege_level = global_descriptor->gdt_segment_privilege_level;
		c->cs_segment_register_descriptor->cs_segment_register_descriptor_type_of_segment_or_gate = global_descriptor->gdt_segment_type_of_segment_or_gate;
		c->cs_segment_register_descriptor->cs_segment_register_descriptor_type_1 = global_descriptor->gdt_segment_type_1;
		c->cs_segment_register_descriptor->cs_segment_register_descriptor_type_0 = global_descriptor->gdt_segment_type_0;
		c->cs_segment_register_descriptor->cs_segment_register_descriptor_type_c = global_descriptor->gdt_segment_type_c;
		c->cs_segment_register_descriptor->cs_segment_register_descriptor_type_e = global_descriptor->gdt_segment_type_e;
		c->cs_segment_register_descriptor->cs_segment_register_descriptor_type_r = global_descriptor->gdt_segment_type_r;
		c->cs_segment_register_descriptor->cs_segment_register_descriptor_type_w = global_descriptor->gdt_segment_type_w;
		c->cs_segment_register_descriptor->cs_segment_register_descriptor_segment_accessed = global_descriptor->gdt_segment_accessed;
		c->cs_segment_register_descriptor->cs_segment_register_descriptor_selector_table_indicator = global_descriptor->gdt_segment_selector_table_indicator;
		c->cs_segment_register_descriptor->cs_segment_register_descriptor_selector_index = global_descriptor->gdt_segment_selector_index;
		c->cs_segment_register_descriptor->cs_segment_register_descriptor_selector_requester_privilege_level = global_descriptor->gdt_segment_selector_requester_privilege_level;
/*	}
} */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual (6.3.2 Restricting Access to Data):                         */
/* ---------------------------------------------------------------------------------------------------------------- */
/* To address operands in memory, an 80386 program must load the selector of a data segment into a data-segment     */
/* register (DS, ES, FS, GS, SS). The processor automatically evaluates access to a data segment by comparing       */
/* privilege levels. The evaluation is performed at the time a selector for the descriptor of the target segment is */
/* loaded into the data-segment register. As Figure 6-3 shows, three different privilege levels enter into this type*/
/* of privilege check:                                                                                              */
/*  - 1. The CPL (current privilege level).                                                                         */
/*  - 2. The RPL (requester's privilege level) of the selector used to specify the target segment.                  */
/*  - 3. The DPL of the descriptor of the target segment.                                                           */
/* instructions may load a data-segment register (and subsequently use the target segment) only if the DPL of the   */
/* target segment is numerically greater than or equal to the maximum of the CPL and the selector's RPL. In other   */
/* words, a procedure can only access data that is at the same or less privileged level.                            */
/*                                                                                                                  */
/* The addressable domain of a task varies as CPL changes. When CPL is zero, data segments at all privilege levels  */
/*are accessible; when CPL is one, only data segments at privilege levels one through three are accessible; when CPL*/
/* is three, only data segments at privilege level three are accessible. This property of the 80386 can be used, for*/
/* example, to prevent application procedures from reading or changing tables of the operating system.              */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual (6.3.3 Restricting Control Transfers):                      */
/* ---------------------------------------------------------------------------------------------------------------- */
/* With the 80386, control transfers are accomplished b y the instructions JMP, CALL, RET, INT, and IRET, as well as*/
/*by the exception and interrupt mechanisms. Exceptions and interrupts are special cases that Chapter 9 covers. This*/
/* chapter discusses only JMP, CALL, and RET instructions.                                                          */
/*                                                                                                                  */
/*The "near" forms of JMP, CALL, and RET transfer within the current code segment, and therefore are subject only to*/
/* limit checking. The processor ensures that the destination of the JMP, CALL, or RET instruction does not exceed  */
/*the limit of the current executable segment. This limit is cached in the CS register; therefore, protection checks*/
/* for near transfers require no extra clock cycles.                                                                */
/*                                                                                                                  */
/* The operands of the "far" forms of JMP and CALL refer to other segments; therefore, the processor performs       */
/* privilege checking. There are two ways a JMP or CALL can refer to another segment:                               */
/*  - 1. The operand selects the descriptor of another executable segment.                                          */
/*  - 2. The operand selects a call gate descriptor. This gated form of transfer is discussed in a later section on */
/* call gates.                                                                                                      */
/* As Figure 6-4 shows, two different privilege levels enter into a privilege check for a control transfer that does*/
/* not use a call gate:                                                                                             */
/*  - 1. The CPL (current privilege level).                                                                         */
/*  - 2. The DPL of the descriptor of the target segment.                                                           */
/* Normally the CPL is equal to the DPL of the segment that the processor is currently executing. CPL may, however, */
/* be greater than DPL of the conforming bit is set in the descriptor of the current executable segment. The        */
/* processor keeps a record of the CPL cached in the CS register; this value can be different from the DPL in the   */
/* descriptor of the code segment.                                                                                  */
/*                                                                                                                  */
/* The processor permits a JMP or CALL directly to another segment only if one of the following privilege rules is  */
/* satisfied:                                                                                                       */
/*  - DPL of the target is equal to CPL.                                                                            */
/*  - The conforming bit of the target code-segment descriptor is set, and the DPL of the target is less than or    */
/* equal to CPL.                                                                                                    */
/* An executable segment whose descriptor has the conforming bit set is called a conforming segment. The conforming-*/
/* segment mechanism permits sharing of procedures that may be called from various privilege levels but should      */
/* execute at the privilege level of the calling procedure. Examples of such procedures include math libraries and  */
/*some exception handlers. When control is transferred to a conforming segment, the CPL does not change. This is the*/
/* only case when CPL may be unequal to the DPL of the current executable segment.                                  */
/*                                                                                                                  */
/* Most code segments are not conforming. The basic rules of privilege above mean that, for nonconforming segments, */
/* control can be transferred without a gate only to executable segments at the same level of privilege. There is a */
/* need, however, to transfer control to (numerically) smaller privilege levels; this need is met by the CALL       */
/*instruction when used with call-gate descriptors, which are explained in the next section. The JMP instruction may*/
/* never transfer control to a nonconforming segment whose DPL does not equal CPL.                                  */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual (6.3.4 Gate Descriptors Guard Procedure Entry Points):      */
/* ---------------------------------------------------------------------------------------------------------------- */
/* For a JMP instruction to a nonconforming segment, both of the following privilege rules must be satisfied;       */
/* otherwise, a general protection exception results.                                                               */
/*  - MAX (CPL,RPL) <= gate DPL                                                                                     */
/*  - target segment DPL = CPL                                                                                      */
/* For a CALL instruction (or for a JMP instruction to a conforming segment), both of the following privilege rules */
/* must be satisfied; otherwise, a general protection exception results.                                            */
/*  - MAX (CPL,RPL) <= gate DPL                                                                                     */
/*  - target segment DPL <= CPL                                                                                     */
/* ---------------------------------------------------------------------------------------------------------------- */


gdt_global_descriptor_t *global_descriptor_new (unsigned long gdt_segment_limit)
{
	unsigned long gdt_selector_index;
	gdt_global_descriptor_t *gdt;
	gdt = malloc (sizeof (gdt_global_descriptor_t));
	if (gdt == NULL) {
		return (NULL);
	}
	if (gdt_segment_limit > 0) {
		gdt->gdt_selector_index = malloc (gdt_segment_limit);
		if (gdt->gdt_selector_index == NULL) {
			free (gdt);
			return (NULL);
		}
	}
	else {
		gdt->gdt_selector_index = NULL;
	}
	gdt->gdt_segment_limit = gdt_segment_limit;
	gdt->gdt_segment_is_present = 0;
	gdt->gdt_granularity_bit = 0;
	gdt->gdt_big = 0;
	gdt->gdt_default = 0;
	gdt->gdt_x_ = 0;
	gdt->gdt_0_ = 0;
	gdt->gdt_segment_available = 0;
	gdt->gdt_descriptor_privilege_level = 0;
	gdt->gdt_type_of_segment_or_gate = 0;
	gdt->gdt_type_1 = 0;
	gdt->gdt_type_0 = 0;
	gdt->gdt_type_c = 0;
	gdt->gdt_type_e = 0;
	gdt->gdt_type_r = 0;
	gdt->gdt_type_w = 0;
	gdt->gdt_segment_accessed = 0;
	gdt->gdt_selector_table_indicator = 1;
	gdt->gdt_segment_base_address_offset = 0;
	gdt->gdt_selector_requester_privilege_level = 0;
	return (gdt);
}

void gdt_del (gdt_global_descriptor_t *gdt)
{
	if (gdt != NULL) {
		free (gdt->gdt_selector_index);
		free (gdt);
	}
}

gdt_global_descriptor_t *gdt_get_global_segment_descriptor (gdt_global_descriptor_t *gdt, unsigned long segment)
{
	if (segment > 0) {
		return (gdt->gdt_selector_index[segment]); /* Removed " - 1" from "[segment]" */
	}
	return (NULL);
}

int gdt_set_global_descriptor (gdt_global_descriptor_t *gdt, unsigned long segment)
{
	if (segment == 0) {
		return (1);
	}
	if (segment > 0) {
		unsigned value;
		value = segment;
		gdt->gdt_selector_index = (gdt_global_descriptor_t **) realloc (gdt->gdt_selector_index, value * sizeof (gdt_global_descriptor_t));
	}
	return (0);
}

int gdt_allocate_new_global_segment_descriptor (gdt_global_descriptor_t *gdt, unsigned long gdt_segment_limit, unsigned *segment)
{
	unsigned value;
	gdt_global_descriptor_t *gdt;
	segment = value
	gdt = gdt_global_descriptor_new (gdt_segment_limit);
	gdt_set_global_descriptor (gdt, gdt_selector_index, value);
	return (0);
}

static
int gdt_remove_global_segment_descriptor (gdt_global_descriptor_t *gdt, unsigned segment)
{
	if (segment == 0) {
		return (1);
	}
	gdt_del (gdt->gdt_selector_index[segment]); /* Removed " - 1" from "[segment]" */
	gdt->gdt_selector_index[segment] = NULL;    /* Removed " - 1" from "[segment]" */
	return (0);
}

ldt_local_descriptor_t *local_descriptor_new (unsigned long ldt_segment_limit)
{
	unsigned long ldt_selector_index;
	ldt_local_descriptor_t *ldt;
	ldt = malloc (sizeof (local_descriptor_t));
	if (ldt == NULL) {
		return (NULL);
	}
	if (ldt_segment_limit > 0) {
		ldt->ldt_selector_index = malloc (ldt_segment_limit);
		if (ldt->ldt_selector_index == NULL) {
			free (ldt);
			return (NULL);
		}
	}
	else {
		ldt->ldt_selector_index = NULL;
	}
	ldt->ldt_segment_base_address_offset = 0;
	ldt->ldt_segment_limit = ldt_segment_limit;
	ldt->ldt_segment_is_present = 0;
	ldt->ldt_granularity_bit = 0;
	ldt->ldt_big = 0;
	ldt->ldt_default = 0;
	ldt->ldt_x = 0;
	ldt->ldt_0_ = 0;
	ldt->ldt_segment_available = 0;
	ldt->ldt_descriptor_privilege_level = 0;
	ldt->ldt_type_of_segment_or_gate = 0; /* Set to "2" when Local Descriptor Table is being identified */
	ldt->ldt_type_1 = 0;
	ldt->ldt_type_0 = 0;
	ldt->ldt_type_c = 0;
	ldt->ldt_type_e = 0;
	ldt->ldt_type_r = 0;
	ldt->ldt_type_w = 0;
	ldt->ldt_segment_accessed = 0;
	ldt->ldt_selector_table_indicator = 1;
	ldt->ldt_selector_requester_privilege_level = 0;
	return (ldt);
}

void ldt_del (ldt_local_descriptor_t *ldt)
{
	if (ldt != NULL) {
		free (ldt->ldt_selector_index);
		free (ldt);
	}
}

ldt_local_descriptor_t *ldt_get_local_segment_descriptor (ldt_local_descriptor_t *ldt, unsigned segment)
{
	if (segment > 0) {
		return (ldt->ldt_selector_index[segment]); /* Removed " - 1" from "[segment]" */
	}
	return (NULL);
}

int ldt_set_local_descriptor (ldt_local_descriptor_t *ldt, unsigned segment)
{
	if (segment == 0) {
		return (1);
	}
	if (segment > 0) {
		unsigned value;
		value = segment;
		ldt->ldt_selector_index = (ldt_local_descriptor_t **) realloc (ldt->ldt_selector_index, value * sizeof (ldt_local_descriptor_t));
	}
	return (0);
}

int ldt_allocate_new_local_segment_descriptor (ldt_local_descriptor_t *ldt, unsigned long ldt_segment_limit, unsigned *segment)
{
	unsigned value;
	ldt_local_descriptor_t *ldt;
	segment = value
	ldt = ldt_local_descriptor_new (ldt_segment_limit);
	ldt_set_local_descriptor (ldt, ldt_selector_index, value);
	return (0);
}

static
int ldt_remove_local_segment_descriptor (ldt_local_descriptor_t *ldt, unsigned segment)
{
	if (segment == 0) {
		return (1);
	}
	ldt_del (ldt->ldt_selector_index[segment]); /* Removed " - 1" from "[segment]" */
	ldt->ldt_selector_index[segment] = NULL;    /* Removed " - 1" from "[segment]" */
	return (0);
}

idt_interrupt_descriptor_t *interrupt_descriptor_new (unsigned long idt_gate_for_interrupt_offset)
{
	unsigned long idt_gate_for_interrupt_selector;
	idt_interrupt_descriptor_t *idt;
	idt = malloc (sizeof (interrupt_descriptor_t));
	if (idt == NULL) {
		return (NULL);
	}
	if (idt_gate_for_interrupt_offset > 0) {
		idt->idt_gate_for_interrupt_selector = malloc (idt_gate_for_interrupt_offset);
		if (idt->idt_gate_for_interrupt_selector == NULL) {
			free (idt);
			return (NULL);
		}
	}
	else {
		idt->idt_gate_for_interrupt_selector = NULL;
	}
	idt->idt_gate_for_interrupt_limit = idt_gate_for_interrupt_limit;
	idt->idt_gate_for_interrupt_offset = 0;
	idt->idt_gate_for_interrupt_present = 0;
	idt->idt_gate_for_interrupt_x_;          /* Segment type 1: "0" or "1": */
	idt->idt_gate_for_interrupt_0_;         /* Segment type 0: "0" or "1": */
	idt->idt_gate_for_interrupt_type = 7;
	idt->idt_gate_for_interrupt_type_b;          /* Segment type 1: "0" or "1": */
	idt->idt_gate_for_interrupt_default;         /* Segment type 0: "0" or "1": */
	idt->idt_gate_for_interrupt_type_1;          /* Segment type 1: "0" or "1": */
	idt->idt_gate_for_interrupt_type_0;          /* Segment type 0: "0" or "1": */
	idt->idt_gate_for_interrupt_type_c;          /* Segment type conforming: "0" for conforming or "1" for non-conforming: */
	idt->idt_gate_for_interrupt_type_e;          /* Segment type expand-down: "0" or "1" */
	idt->idt_gate_for_interrupt_type_r;          /* Segment type readable: "0" or "1" */
	idt->idt_gate_for_interrupt_type_w;          /* Segment type writable: "0" or "1" */
	idt->idt_gate_for_interrupt_descriptor_privilege_level = 0;
	idt->idt_gate_for_interrupt_selector_table_indicator = 0;
	idt->idt_gate_for_interrupt_selector_requester_privilege_level = 0;
	idt->idt_gate_for_interrupt_granularity_bit = 0;
	idt->idt_gate_for_interrupt_available = 0;
	idt->idt_gate_for_interrupt_accessed = 0;
	idt->idt_gate_for_interrupt_#n_ = 0;
	idt->idt_gate_for_interrupt_#2_ = 0;
	idt->idt_gate_for_interrupt_#1_ = 0;
	idt->idt_gate_for_interrupt_#0_ = 0;
	return (idt);
}

void idt_del (idt_interrupt_descriptor_t *idt)
{
	if (idt != NULL) {
		free (idt->idt_gate_for_interrupt_selector);
		free (idt);
	}
}

idt_interrupt_descriptor_t *idt_get_interrupt_gate_descriptor (idt_interrupt_descriptor_t *idt, unsigned gate)
{
	if (gate > 0) {
		return (idt->idt_gate_for_interrupt_selector[gate]); /* Removed " - 1" from "[gate]" */
	}
	return (NULL);
}

int idt_set_interrupt_descriptor (idt_interrupt_descriptor_t *idt, unsigned gate)
{
	if (gate == 0) {
		return (1);
	}
	if (gate > 0) {
		unsigned value;
		value = gate;
		idt->idt_gate_for_interrupt_selector = (idt_interrupt_descriptor_t **) realloc (idt->idt_gate_for_interrupt_selector, value * sizeof (idt_interrupt_descriptor_t));
	}
	return (0);
}

int idt_allocate_new_interrupt_gate_descriptor (idt_interrupt_descriptor_t *ldt, unsigned long idt_gate_for_interrupt_offset, unsigned *gate)
{
	unsigned value;
	idt_interrupt_descriptor_t *idt;
	gate = value
	idt = idt_interrupt_descriptor_new (idt_gate_for_interrupt_offset);
	idt_set_interrupt_descriptor (idt, idt_gate_for_interrupt_selector, value);
	return (0);
}

static
int idt_remove_interrupt_gate_descriptor (idt_interrupt_descriptor_t *idt, unsigned gate)
{
	if (gate == 0) {
		return (1);
	}
	idt_del (idt->idt_gate_for_interrupt_selector[gate]); /* Removed " - 1" from "[segment]" */
	idt->idt_gate_for_interrupt_selector[gate] = NULL;    /* Removed " - 1" from "[segment]" */
	return (0);
}

idt_task_descriptor_t *task_descriptor_new (unsigned long idt_gate_for_task_offset)
{
	unsigned long idt_gate_for_task_selector;
	idt_task_descriptor_t *idt_task;
	idt_task = malloc (sizeof (task_descriptor_t));
	if (idt_task == NULL) {
		return (NULL);
	}
	if (idt_gate_for_task_offset > 0) {
		idt_task->idt_gate_for_task_selector = malloc (idt_gate_for_task_offset);
		if (idt_task->idt_gate_for_task_selector == NULL) {
			free (idt_task);
			return (NULL);
		}
	}
	else {
		idt_task->idt_gate_for_task_selector = NULL;
	}
	idt_task->idt_gate_for_task_offset = idt_gate_for_task_offset;
	idt_task->idt_gate_for_task_present = 0;
	idt_task->idt_gate_for_task_type = 7;
	idt_task->idt_gate_for_task_limit = 0;
	idt_task->idt_gate_for_task_descriptor_privilege_level = 0;
	idt_task->idt_gate_for_task_number_n_ = 0;
	idt_task->idt_gate_for_task_number_2_ = 0;
	idt_task->idt_gate_for_task_number_1_ = 0;
	idt_task->idt_gate_for_task_number_0_ = 0;
	return (idt_task);
}

void idt_del (idt_task_descriptor_t *idt_task)
{
	if (idt_task != NULL) {
		free (idt_task->idt_gate_for_task_selector);
		free (idt_task);
	}
}

idt_task_descriptor_t *idt_get_task_gate_descriptor (idt_task_descriptor_t *idt_task, unsigned gate)
{
	if (gate > 0) {
		return (idt_task->idt_gate_for_task_selector[gate]); /* Removed " - 1" from "[gate]" */
	}
	return (NULL);
}

int idt_set_task_descriptor (idt_task_descriptor_t *idt_task, unsigned gate)
{
	if (gate == 0) {
		return (1);
	}
	if (gate > 0) {
		unsigned value;
		value = gate;
		idt_task->idt_gate_for_task_selector = (idt_task_descriptor_t **) realloc (idt_task->idt_gate_for_task_selector, value * sizeof (idt_task_descriptor_t));
	}
	return (0);
}

int idt_allocate_new_task_gate_descriptor (idt_task_descriptor_t *idt_task, unsigned long idt_gate_for_task_offset, unsigned *gate)
{
	unsigned value;
	idt_task_descriptor_t *idt_task;
	gate = value
	idt_task = idt_task_descriptor_new (idt_gate_for_task_offset);
	idt_set_task_descriptor (idt_task, idt_gate_for_task_selector, value);
	return (0);
}

static
int idt_remove_task_gate_descriptor (idt_task_descriptor_t *idt_task, unsigned gate)
{
	if (gate == 0) {
		return (1);
	}
	idt_del (idt_task->idt_gate_for_task_selector[gate]); /* Removed " - 1" from "[segment]" */
	idt_task->idt_gate_for_task_selector[gate] = NULL;    /* Removed " - 1" from "[segment]" */
	return (0);
}

idt_trap_descriptor_t *trap_descriptor_new (unsigned long idt_gate_for_trap_offset)
{
	idt_trap_descriptor_t *idt_trap;
	idt_trap = malloc (sizeof (trap_descriptor_t));
	if (trap_descriptor == NULL) {
		return (NULL);
	}
	if (idt_gate_for_trap_offset > 0) {
		idt_trap->idt_gate_for_trap_selector = malloc (idt_gate_for_trap_offset);
		if (idt_trap->idt_gate_for_trap_selector == NULL) {
			free (idt_trap);
			return (NULL);
		}
	}
	else {
		idt_trap->idt_gate_for_trap_selector = NULL;
	}
	idt_trap->idt_gate_for_trap_limit = idt_gate_for_trap_limit;
	idt_trap->idt_gate_for_trap_offset = 0;
	idt_trap->idt_gate_for_trap_present = 0;
	idt_trap->idt_gate_for_trap_x_;          /* Segment type 1: "0" or "1": */
	idt_trap->idt_gate_for_trap_0_;         /* Segment type 0: "0" or "1": */
	idt_trap->idt_gate_for_trap_type = 7;
	idt_trap->idt_gate_for_trap_type_b;          /* Segment type 1: "0" or "1": */
	idt_trap->idt_gate_for_trap_default;         /* Segment type 0: "0" or "1": */
	idt_trap->idt_gate_for_trap_type_1;          /* Segment type 1: "0" or "1": */
	idt_trap->idt_gate_for_trap_type_0;          /* Segment type 0: "0" or "1": */
	idt_trap->idt_gate_for_trap_type_c;          /* Segment type conforming: "0" for conforming or "1" for non-conforming: */
	idt_trap->idt_gate_for_trap_type_e;          /* Segment type expand-down: "0" or "1" */
	idt_trap->idt_gate_for_trap_type_r;          /* Segment type readable: "0" or "1" */
	idt_trap->idt_gate_for_trap_type_w;          /* Segment type writable: "0" or "1" */
	idt_trap->idt_gate_for_trap_descriptor_privilege_level = 0;
	idt_trap->idt_gate_for_trap_selector_table_indicator = 0;
	idt_trap->idt_gate_for_trap_selector_requester_privilege_level = 0;
	idt_trap->idt_gate_for_trap_granularity_bit = 0;
	idt_trap->idt_gate_for_trap_available = 0;
	idt_trap->idt_gate_for_trap_accessed = 0;
	idt_trap->idt_gate_for_trap_number_n_ = 0;
	idt_trap->idt_gate_for_trap_number_2_ = 0;
	idt_trap->idt_gate_for_trap_number_1_ = 0;
	idt_trap->idt_gate_for_trap_number_0_ = 0;
	return (idt_trap);
}

void idt_del (idt_trap_descriptor_t *idt_trap)
{
	if (idt_trap != NULL) {
		free (idt_trap->idt_gate_for_trap_selector);
		free (idt_trap);
	}
}

idt_trap_descriptor_t *idt_get_trap_gate_descriptor (idt_trap_descriptor_t *idt_trap, unsigned gate)
{
	if (gate > 0) {
		return (idt_trap->idt_gate_for_trap_selector[gate]); /* Removed " - 1" from "[gate]" */
	}
	return (NULL);
}

int idt_set_trap_descriptor (idt_trap_descriptor_t *idt_trap, unsigned gate)
{
	if (gate == 0) {
		return (1);
	}
	if (gate > 0) {
		unsigned value;
		value = gate;
		idt_trap->idt_gate_for_trap_selector = (idt_trap_descriptor_t **) realloc (idt_trap->idt_gate_for_trap_selector, value * sizeof (idt_trap_descriptor_t));
	}
	return (0);
}

int idt_allocate_new_trap_gate_descriptor (idt_trap_descriptor_t *idt_trap, unsigned long idt_gate_for_trap_offset, unsigned *gate)
{
	unsigned value;
	idt_trap_descriptor_t *idt_trap;
	gate = value
	idt_trap = idt_trap_descriptor_new (idt_gate_for_trap_offset);
	idt_set_trap_descriptor (idt_trap, idt_gate_for_trap_selector, value);
	return (0);
}

static
int idt_remove_trap_gate_descriptor (idt_trap_descriptor_t *idt_trap, unsigned gate)
{
	if (gate == 0) {
		return (1);
	}
	idt_del (idt_trap->idt_gate_for_trap_selector[gate]); /* Removed " - 1" from "[segment]" */
	idt_trap->idt_gate_for_trap_selector[gate] = NULL;    /* Removed " - 1" from "[segment]" */
	return (0);
}

/* virtual_8086_mode_address_t *virtual_8086_mode_address_new (unsigned long v86_base_segment_selector);
{
	unsigned long v86_offset_effective_address;
	virtual_8086_mode_address_t *virtual_8086_mode_address;
	virtual_8086_mode_address = malloc (sizeof (v86_offset_effective_address_t));
	if (virtual_8086_mode_address == NULL) {
		return (NULL);
	}
	if (v86_offset_effective_address > 0) {
		if (v86_offset_effective_address <= 65535) {
			virtual_8086_mode_address->v86_base_segment_selector = malloc (v86_offset_effective_address);
			if (virtual_8086_mode_address->v86_base_segment_selector == NULL) {
				free (virtual_8086_mode_address);
				return (NULL);
			}
		}
        else {
			trap_descriptor->idt_gate_for_trap_selector = NULL; /* Should be interrupt 12 or 13 */
/*		}
	}
	else {
		trap_descriptor->idt_gate_for_trap_selector = NULL;
	}
	virtual_8086_mode_address->v86_offset_effective_address = v86_offset_effective_address;
	/* See above for result of linear address */
/*	return (virtual_8086_mode_address);
}

void v86_del (virtual_8086_mode_address_t *virtual_8086_mode_address)
{
	if (virtual_8086_mode_address != NULL) {
		free (virtual_8086_mode_address->v86_base_segment_selector);
		free (virtual_8086_mode_address);
	}
}

virtual_8086_mode_address_t *v86_get_virtual_8086_mode_linear_address (virtual_8086_mode_address_t *v86, unsigned segment)
{
	if (segment > 0) {
		return (virtual_8086_mode_address->v86_base_segment_selector[segment]); /* Removed " - 1" from "[segment]" */
/*	}
	return (NULL);
}

int v86_set_virtual_8086_mode_linear_address (virtual_8086_mode_address_t *v86, unsigned segment)
{
	if (segment == 0) {
		return (1);
	}
	if (segment > 0) {
		unsigned value;
		value = segment;
		virtual_8086_mode_address->v86_base_segment_selector = (virtual_8086_mode_address_t **) realloc (virtual_8086_mode_address->v86_base_segment_selector, value * sizeof (virtual_8086_mode_address));
	}
	return (0);
}

int v86_allocate_new_virtual_8086_mode_linear_address (virtual_8086_mode_address_t *v86, unsigned long v86_offset_effective_address, unsigned *segment)
{
	unsigned value;
	virtual_8086_mode_address_t *v86;
	segment = value
	v86 = virtual_8086_mode_address_new (v86_offset_effective_address);
	v86_set_virtual_8086_mode_linear_address (v86, v86_base_segment_selector, value);
	return (0);
}

static
int v86_remove_virtual_8086_mode_linear_address (virtual_8086_mode_address_t *v86, unsigned segment)
{
	if (segment == 0) {
		return (1);
	}
	v86_del (virtual_8086_mode_address->v86_base_segment_selector[segment]); /* Removed " - 1" from "[segment]" */
/*	virtual_8086_mode_address->idt_base_segment_selector[segment] = NULL;    /* Removed " - 1" from "[segment]" */
/*	return (0);
} */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual; 6.4.1.1 Restricting Addressable Domain:                    */
/* ---------------------------------------------------------------------------------------------------------------- */
/* The concept of privilege for pages is implemented by assigning each page to one of two levels:                   */
/*  - 1. Supervisor level (U/S=0) -- for the operating system and other systems software and related data.          */
/*  - 2. User level (U/S=1) -- for applications procedures and data.                                                */
/* The current level (U or S) is related to CPL. If CPL is 0, 1, or 2, the processor is executing at supervisor     */
/* level. If CPL is 3, the processor is executing at user level.                                                    */
/*                                                                                                                  */
/* When the processor is executing at supervisor level, all pages are addressable, but, when the processor is       */
/* executing at user level, only pages that belong to the user level are addressable.                               */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual; 6.4.1.2 Type Checking:                                     */
/* ---------------------------------------------------------------------------------------------------------------- */
/* At the level of page addressing, two types are defined:                                                          */
/*  - 1. Read-only access (R/W=0)                                                                                   */
/*  - 2. Read/write access (R/W=1)                                                                                  */
/* When the processor is executing at supervisor level, all pages are both readable and writable. When the processor*/
/* is executing at user level, only pages that belong to the user level and are marked for read/write access are    */
/* writable; pages that belong to supervisor level are neither readable nor writable from user level.               */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual; 6.4.2 Combining Protection of Both Levels of Page Tables:  */
/* ---------------------------------------------------------------------------------------------------------------- */
/* For any one page, the protection attributes of its page directory entry may differ from those of its page table  */
/*entry. The 80386 computes the effective protection attributes for a page by examining the protection attributes in*/
/* both the directory and the page table. Table 6-5 shows the effective protection provided by the possible         */
/* combinations of protection attributes.                                                                           */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual; 6.4.3 Overrides to Page Protection:                        */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Certain accesses are checked as if they are privilege-level 0 references, even if CPL = 3:                       */
/*  - LDT, GDT, TSS, IDT references.                                                                                */
/*  - Access to inner stack during ring-crossing CALL/INT.                                                          */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual; 6.5 Combining Page and Segment Protection:                 */
/* ---------------------------------------------------------------------------------------------------------------- */
/* When paging is enabled, the 80386 first evaluates segment protection, then evaluates page protection. If the     */
/* processor detects a protection violation at either the segment or the page level, the requested operation cannot */
/* proceed; a protection exception occurs instead.                                                                  */
/*                                                                                                                  */
/*For example, it is possible to define a large data segment which has some subunits that are read-only and other   */
/* subunits that are read-write. In this case, the page directory (or page table) entries for the read-only subunits*/
/* would have the U/S and R/W bits set to x0, indicating no write rights for all the pages described by that        */
/* directory entry (or for individual pages). This technique might be used, for example, in a UNIX-like system to   */
/* define a large data segment, part of which is read only (for shared data or ROMmed constants). This enables UNIX-*/
/*like systems to define a "flat" data space as one large segment, use "flat" pointers to address within this "flat"*/
/* space, yet be able to protect shared data, shared files mapped into the virtual space, and supervisor areas.     */
/* ---------------------------------------------------------------------------------------------------------------- */
directory_page_table_t *directory_page_table_new (unsigned long directory_page_frame_specified)
{
	unsigned long directory_page_frame_address_being_loaded;
	directory_page_table_t *directory_page;
	directory_page = malloc (sizeof (directory_page_table_t));
	if (directory_page == NULL) {
		return (NULL);
	}
	if (directory_page_frame_address_specified > 0) {
		directory_page->directory_page_frame_address_being_loaded = (directory_page_frame_address_specified) & 0xfffff;
		directory_page->directory_page_frame_address = malloc (directory_page_frame_address_being_loaded);
		if (directory_page->directory_page_frame_address == NULL) {
			free (directory_page);
			return (NULL);
		}
	}
	else {
		directory_page->directory_page_frame_address = NULL;
	}
	directory_page->directory_page_frame_address_being_loaded = directory_page_frame_address_being_loaded;
	directory_page->directory_page_table_available = 0;
	directory_page->directory_page_table_dirty = 0;
	directory_page->directory_page_table_read_write = 0;
	directory_page->directory_page_table_user_supervisor = 0;
	return (directory_page);
}

void directory_page_table_del (directory_page_table_t *directory_page)
{
	if (directory_page != NULL) {
		free (directory_page->directory_page_frame_address);
		free (directory_page);
	}
}

directory_page_table_t *directory_get_page_table_descriptor (directory_page_table_t *directory_page, unsigned address)
{
	if (address > 0) {
		return (directory_page->directory_page_frame_address[address]); /* Removed " - 1" from "[address]" */
	}
	return (NULL);
}

int directory_set_page_table_descriptor (directory_page_table_t *directory_page, unsigned address)
{
	if (address == 0) {
		return (1);
	}
	if (address > 0) {
		unsigned value;
		value = address;
		directory_page->directory_page_frame_address = (directory_page_table_t **) realloc (directory_page->directory_page_frame_address, value * sizeof (directory_page_table_t));
	}
	return (0);
}

int directory_allocate_new_page_table_descriptor (directory_page_table_t *directory_page, unsigned long directory_page_frame_address, unsigned *address)
{
	unsigned value;
	directory_page_table_t *directory_page;
	address = value
	directory_page = directory_page_table_new (directory_page_frame_address_being_loaded);
	directory_set_page_table_descriptor (directory_page, directory_page_frame_address, value);
	return (0);
}

static
int directory_remove_page_table_descriptor (directory_page_table_t *directory_page, unsigned address)
{
	if (address == 0) {
		return (1);
	}
	directory_page_table_del (directory_page->directory_page_frame_address[address]); /* Removed " - 1" from "[segment]" */
	directory_page->directory_page_frame_address[address] = NULL;    /* Removed " - 1" from "[segment]" */
	return (0);
}

entry_page_table_t *entry_page_table_new (unsigned long entry_page_frame_specified)
{
	unsigned long entry_page_frame_address_being_loaded;
	entry_page_table_t *entry_page;
	entry_page = malloc (sizeof (entry_page_table_t));
	if (entry_page == NULL) {
		return (NULL);
	}
	if (entry_page_frame_address_specified > 0) {
		entry_page->entry_page_frame_address_being_loaded = (entry_page_frame_address_specified) & 0xfffff;
		entry_page->entry_page_frame_address = malloc (entry_page_frame_address_being_loaded);
		if (entry_page->entry_page_frame_address == NULL) {
			free (entry_page);
			return (NULL);
		}
	}
	else {
		entry_page->entry_page_frame_address = NULL;
	}
	entry_page->entry_page_frame_address_being_loaded = entry_page_frame_address_being_loaded;
	entry_page->entry_page_table_is_present = 0;
	entry_page->entry_page_table_available = 0;
	entry_page->entry_page_table_dirty = 0;
	entry_page->entry_page_table_read_write = 0;
	entry_page->entry_page_table_user_supervisor = 0;
	return (entry_page);
}

void entry_page_table_del (entry_page_table_t *entry_page)
{
	if (entry_page != NULL) {
		free (entry_page->entry_page_frame_address);
		free (entry_page);
	}
}

entry_page_table_t *entry_get_page_table_descriptor (entry_page_table_t *entry_page, unsigned address)
{
	if (address > 0) {
		return (entry_page->entry_page_frame_address[address]); /* Removed " - 1" from "[address]" */
	}
	return (NULL);
}

int entry_set_page_table_descriptor (entry_page_table_t *entry_page, unsigned address)
{
	if (address == 0) {
		return (1);
	}
	if (address > 0) {
		unsigned value;
		value = address;
		entry_page->entry_page_frame_address = (entry_page_table_t **) realloc (entry_page->entry_page_frame_address, value * sizeof (entry_page_table_t));
	}
	return (0);
}

int entry_allocate_new_page_table_descriptor (entry_page_table_t *entry_page, unsigned long entry_page_frame_address, unsigned *address)
{
	unsigned value;
	entry_page_table_t *entry_page;
	address = value
	entry_page = entry_page_table_new (entry_page_frame_address_being_loaded);
	entry_set_page_table_descriptor (entry_page, entry_page_frame_address, value);
	return (0);
}

static
int entry_remove_page_table_descriptor (entry_page_table_t *entry_page, unsigned address)
{
	if (address == 0) {
		return (1);
	}
	entry_page_table_del (entry_page->entry_page_frame_address[address]); /* Removed " - 1" from "[segment]" */
	entry_page->entry_page_frame_address[address] = NULL;    /* Removed " - 1" from "[segment]" */
	return (0);
}

/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual (5.2.4.2 Present Bit):                                      */
/* ---------------------------------------------------------------------------------------------------------------- */
/* The Present bit indicates whether a page table entry can be used in address translation. P=1 indicates that the  */
/* entry can be used.                                                                                               */
/*                                                                                                                  */
/*When P=0 in either level of page tables, the entry is not valid for address translation, and the rest of the entry*/
/* is available for software use; none of the other bits in the entry is tested by the hardware. Figure 5-11        */
/* illustrates the format of a page-table entry when P=0.                                                           */
/*                                                                                                                  */
/* If P=0 in either level of page tables when an attempt is made to use a page-table entry for address translation, */
/* the processor signals a page exception. In software systems that support paged virtual memory, the page-not-     */
/* present exception handler can bring the required page into physical memory. The instruction that caused the      */
/* exception can then be reexecuted. Refer to Chapter 9 for more information on exception handlers.                 */
/*                                                                                                                  */
/* Note that there is no present bit for the page directory itself. The page directory may be not-present while the */
/* associated task is suspended, but the operating system must ensure that the page directory indicated by the CR3  */
/* image in the TSS is present in physical memory before the task is dispatched. Refer to Chapter 7 for an          */
/* explanation of the TSS and task dispatching.                                                                     */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual (5.2.4.3 Accessed and Dirty Bits):                          */
/* ---------------------------------------------------------------------------------------------------------------- */
/* These bits provide data about page usage in both levels of the page tables. With the exception of the dirty bit  */
/* in a page directory entry, these bits are set by the hardware; however, the processor does not clear any of these*/
/* bits.                                                                                                            */
/*                                                                                                                  */
/* The processor sets the corresponding accessed bits in both levels of page tables to one before a read or write   */
/* operation to a page.                                                                                             */
/*                                                                                                                  */
/* The processor sets the dirty bit in the second-level page table to one before a write to an address covered by   */
/* that page table entry. The dirty bit in directory entries is undefined.                                          */
/*                                                                                                                  */
/* An operating system that supports paged virtual memory can use these bits to determine what pages to eliminate   */
/* from physical memory when the demand for memory exceeds the physical memory available. The operating system is   */
/* responsible for testing and clearing these bits.                                                                 */
/*                                                                                                                  */
/* Refer to Chapter 11 for how the 80386 coordinates updates to the accessed and dirty bits in multiprocessor       */
/* systems.                                                                                                         */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual (5.2.4.4 Read/Write and User/Supervisor Bits):              */
/* ---------------------------------------------------------------------------------------------------------------- */
/* These bits are not used for address translation, but are used for page-level protection, which the processor     */
/* performs at the same time as address translation. Refer to Chapter 6 where protection is discussed in detail.    */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual (5.2.5 Page Translation Cache):                             */
/* ---------------------------------------------------------------------------------------------------------------- */
/* For greatest efficiency in address translation, the processor stores the most recently used page-table data in an*/
/* on-chip cache. Only if the necessary paging information is not in the cache must both levels of page tables be   */
/* referenced.                                                                                                      */
/*                                                                                                                  */
/* The existence of the page-translation cache is invisible to applications programmers but not to systems          */
/* programmers; operating-system programmers must flush the cache whenever the page tables are changed. The page-   */
/* translation cache can be flushed by either of two methods:                                                       */
/* The processor sets the dirty bit in the second-level page table to one before a write to an address covered by   */
/* that page table entry. The dirty                                                                                 */
/*  - 1. By reloading CR3 with a MOV instruction, for example:                                                      */
/*       MOV CR3, EAX                                                                                               */
/*  - 2. By performing a task switch to a TSS that has a different CR3 image than the current TSS. (Refer to Chapter*/
/* 7 for more information on task switching).                                                                       */
/* ---------------------------------------------------------------------------------------------------------------- */

tss_task_state_segment_descriptor_t *task_state_segment_descriptor_new (unsigned long tss_gdt_selector_index);
{
	unsigned long tss_gdt_segment_limit; /* Task State Segment being created */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual (7.2 TSS Descriptor):                                       */
/* ---------------------------------------------------------------------------------------------------------------- */
/* The task state segment, like all other segments, is defined by a descriptor. Figure 7-2 shows the format of a TSS*/
/* descriptor. The B-bit in the type field indicates whether the task is busy. A type code of 9 indicates a non-busy*/
/* task; a type code of 11 indicates a busy task. Tasks are not reentrant. The B-bit allows the processor to detect */
/* an attempt to switch to a task that is already busy.                                                             */
/*                                                                                                                  */
/* The BASE, LIMIT, and DPL fields and the G-bit and P-bit have functions similar to their counterparts in data-    */
/* segment descriptors. The LIMIT field, however, must have a value equal to or greater than 103. An attempt to     */
/* switch to a task whose TSS descriptor has a limit less that 103 causes an exception. A larger limit is           */
/* permissible, and a larger limit is required if an I/O permission map is present. A larger limit may also be      */
/* convenient for systems software if additional data is stored in the same segment as the TSS.                     */
/*                                                                                                                  */
/* A procedure that has access to a TSS descriptor can cause a task switch. In most systems the DPL fields of TSS   */
/* descriptors should be set to zero, so that only trusted software has the right to perform task switching.        */
/*                                                                                                                  */
/* Having access to a TSS-descriptor does not give a procedure the right to read or modify a TSS. Reading and       */
/*modification can be accomplished only with another descriptor that redefines the TSS as a data segment. An attempt*/
/* to load a TSS descriptor into any of the segment registers (CS, SS, DS, ES, FS, GS) causes an exception.         */
/*                                                                                                                  */
/*TSS descriptors may reside only in the GDT. An attempt to identify a TSS with a selector that has TI=1 (indicating*/
/* the current LDT) results in an exception.                                                                        */
/* ---------------------------------------------------------------------------------------------------------------- */
	tss_task_state_segment_descriptor_t *tss;
	tss = malloc (sizeof (tss_task_state_segment_descriptor_t));
	if (tss == NULL) {
		return (NULL);
	}
	if ((tss_gdt_segment_limit > 102) && (tss_ldt_segment_limit > 102)) {
		tss->tss_gdt_selector_index = malloc (tss_gdt_segment_limit);
		tss->tss_ldt_selector_index = malloc (tss_ldt_segment_limit);
		if ((tss->tss_gdt_selector_index == NULL) && (tss->tss_ldt_selector_index == NULL)) {
			free (tss);
			return (NULL);
		}
	}
	else {
		tss->tss_gdt_selector_index = NULL;
		tss->tss_ldt_selector_index = NULL;
	}
	tss->tss_gdt_segment_base_address_offset = 0;
	tss->tss_gdt_segment_limit = tss_gdt_segment_limit;
	tss->tss_gdt_segment_is_present = 0;
	tss->tss_gdt_granularity_bit = 0;
	tss->tss_gdt_big = 0;
	tss->tss_gdt_default = 0;
	tss->tss_gdt_x = 0;
	tss->tss_gdt_o_ = 0;
	tss->tss_gdt_segment_available = 0;
	tss->tss_gdt_segment_is_present = 0;
	tss->tss_gdt_descriptor_privilege_level = 0;
	tss->tss_gdt_type_of_segment_or_gate = 9; /* 1/9 for Available 286/386 TSS; 3/B for Busy 286/386 TSS */
	tss->tss_gdt_type_1 = 0;
	tss->tss_gdt_type_0 = 0;
	tss->tss_gdt_type_c = 0;
	tss->tss_gdt_type_e = 0;
	tss->tss_gdt_type_r = 0;
	tss->tss_gdt_type_w = 0;
	tss->tss_gdt_segment_accessed = 0;
	tss->tss_gdt_selector_table_indicator = 1;
	tss->tss_gdt_selector_requester_privilege_level = 0;
	tss->tss_ldt_segment_base_address_offset = 0;
	tss->tss_ldt_segment_limit = 0;
	tss->tss_ldt_segment_is_present = 0;
	tss->tss_ldt_granularity_bit = 0;
	tss->tss_ldt_big = 0;
	tss->tss_ldt_default = 0;
	tss->tss_ldt_x = 0;
	tss->tss_ldt_o_ = 0;
	tss->tss_ldt_segment_available = 0;
	tss->tss_ldt_segment_is_present = 0;
	tss->tss_ldt_descriptor_privilege_level = 0;
	tss->tss_ldt_type_of_segment_or_gate = 0;
	tss->tss_ldt_type_1 = 0;
	tss->tss_ldt_type_0 = 0;
	tss->tss_ldt_type_c = 0;
	tss->tss_ldt_type_e = 0;
	tss->tss_ldt_type_r = 0;
	tss->tss_ldt_type_w = 0;
	tss->tss_ldt_segment_accessed = 0;
	tss->tss_ldt_selector_table_indicator = 1;
	tss->tss_ldt_selector_requester_privilege_level = 0;
	tss->input_output_map_base = 0;
	tss->debug_trap_bit = 0;
	tss->tss_register_eax = 0;
	tss->tss_register_ebx = 0;
	tss->tss_register_ecx = 0;
	tss->tss_register_edx = 0;
	tss->tss_register_esp = 0;
	tss->tss_register_ebp = 0;
	tss->tss_register_esi = 0;
	tss->tss_register_edi = 0;
	tss->tss_register_esp_for_privilege_level_2 = 0;
	tss->tss_register_esp_for_privilege_level_1 = 0;
	tss->tss_register_esp_for_privilege_level_0 = 0;
	tss->tss_flags_cf = 0;
	tss->tss_flags_pf = 0;
	tss->tss_flags_ac = 0;
	tss->tss_flags_zf = 0;
	tss->tss_flags_sf = 0;
	tss->tss_flags_tf = 0;
	tss->tss_flags_if = 0;
	tss->tss_flags_df = 0;
	tss->tss_flags_of = 0;
	tss->tss_eflags_iopl = 0;
	tss->tss_eflags_nt = 0;
	tss->tss_eflags_rf = 0;
	tss->tss_eflags_vm = 0;
	tss->tss_instruction_pointer_eip = 0;
	tss->tss_pdbr_page_frame_address_specified = 0;
	tss->tss_pdbr_page_frame_address_being_loaded = 0;
	tss->tss_pdbr_page_frame_address = 0;
	tss->tss_pdbr_page_frame_is_present = 0;
	tss->tss_pdbr_page_frame_read_write = 0;
	tss->tss_pdbr_page_frame_user_supervisor = 0;
	tss->tss_pdbr_page_frame_dirty = 0;
	tss->tss_pdbr_page_frame_available = 0;
	tss->tss_directory_page_frame_address_specified = 0;
	tss->tss_directory_page_frame_address_being_loaded = 0;
	tss->tss_directory_page_frame_address = 0;
	tss->tss_directory_page_frame_read_write = 0;
	tss->tss_directory_page_frame_user_supervisor = 0;
	tss->tss_directory_page_frame_dirty = 0;
	tss->tss_directory_page_frame_available = 0;
	tss->tss_stack_pointer_esp_for_privilege_level_2 = 0;
	tss->tss_stack_pointer_esp_for_privilege_level_1 = 0;
	tss->tss_stack_pointer_esp_for_privilege_level_0 = 0;
	tss->tss_cs_segment_register_descriptor_base_address_offset = 0;
	tss->tss_cs_segment_register_descriptor_limit = 0;
	tss->tss_cs_segment_register_descriptor_granularity_bit = 0;
	tss->tss_cs_segment_register_descriptor_big = 0;
	tss->tss_cs_segment_register_descriptor_default = 0;
	tss->tss_cs_segment_register_descriptor_x_ = 0;
	tss->tss_cs_segment_register_descriptor_0_ = 0;
	tss->tss_cs_segment_register_descriptor_available = 0;
	tss->tss_cs_segment_register_descriptor_is_present = 0;
	tss->tss_cs_segment_register_type_of_segment_or_gate = 0;
	tss->tss_cs_segment_register_descriptor_type_1 = 0;
	tss->tss_cs_segment_register_descriptor_type_0 = 0;
	tss->tss_cs_segment_register_descriptor_type_c = 0;
	tss->tss_cs_segment_register_descriptor_type_e = 0;
	tss->tss_cs_segment_register_descriptor_type_r = 0;
	tss->tss_cs_segment_register_descriptor_type_w = 0;
	tss->tss_cs_segment_register_selector_table_indicator = 0;
	tss->tss_cs_segment_register_selector_index = 0;
	tss->tss_cs_segment_register_selector_requester = 0;
	tss->tss_ds_segment_register_descriptor_base_address_offset = 0;
	tss->tss_ds_segment_register_descriptor_limit = 0;
	tss->tss_ds_segment_register_descriptor_granularity_bit = 0;
	tss->tss_ds_segment_register_descriptor_big = 0;
	tss->tss_ds_segment_register_descriptor_default = 0;
	tss->tss_ds_segment_register_descriptor_x_ = 0;
	tss->tss_ds_segment_register_descriptor_0_ = 0;
	tss->tss_ds_segment_register_descriptor_available = 0;
	tss->tss_ds_segment_register_descriptor_is_present = 0;
	tss->tss_ds_segment_register_type_of_segment_or_gate = 0;
	tss->tss_ds_segment_register_descriptor_type_1 = 0;
	tss->tss_ds_segment_register_descriptor_type_0 = 0;
	tss->tss_ds_segment_register_descriptor_type_c = 0;
	tss->tss_ds_segment_register_descriptor_type_e = 0;
	tss->tss_ds_segment_register_descriptor_type_r = 0;
	tss->tss_ds_segment_register_descriptor_type_w = 0;
	tss->tss_ds_segment_register_selector_table_indicator = 0;
	tss->tss_ds_segment_register_selector_index = 0;
	tss->tss_ds_segment_register_selector_requester_privilege_level = 0;
	tss->tss_es_segment_register_descriptor_base_address_offset = 0;
	tss->tss_es_segment_register_descriptor_limit = 0;
	tss->tss_es_segment_register_descriptor_granularity_bit = 0;
	tss->tss_es_segment_register_descriptor_big = 0;
	tss->tss_es_segment_register_descriptor_default = 0;
	tss->tss_es_segment_register_descriptor_x_ = 0;
	tss->tss_es_segment_register_descriptor_0_ = 0;
	tss->tss_es_segment_register_descriptor_available = 0;
	tss->tss_es_segment_register_descriptor_is_present = 0;
	tss->tss_es_segment_register_type_of_segment_or_gate = 0;
	tss->tss_es_segment_register_descriptor_type_1 = 0;
	tss->tss_es_segment_register_descriptor_type_0 = 0;
	tss->tss_es_segment_register_descriptor_type_c = 0;
	tss->tss_es_segment_register_descriptor_type_e = 0;
	tss->tss_es_segment_register_descriptor_type_r = 0;
	tss->tss_es_segment_register_descriptor_type_w = 0;
	tss->tss_es_segment_register_selector_table_indicator = 0;
	tss->tss_es_segment_register_selector_index = 0;
	tss->tss_es_segment_register_selector_requester_privilege_level = 0;
	tss->tss_fs_segment_register_descriptor_base_address_offset = 0;
	tss->tss_fs_segment_register_descriptor_limit = 0;
	tss->tss_fs_segment_register_descriptor_granularity_bit = 0;
	tss->tss_fs_segment_register_descriptor_big = 0;
	tss->tss_fs_segment_register_descriptor_default = 0;
	tss->tss_fs_segment_register_descriptor_x_ = 0;
	tss->tss_fs_segment_register_descriptor_0_ = 0;
	tss->tss_fs_segment_register_descriptor_available = 0;
	tss->tss_fs_segment_register_descriptor_is_present = 0;
	tss->tss_fs_segment_register_type_of_segment_or_gate = 0;
	tss->tss_fs_segment_register_descriptor_type_1 = 0;
	tss->tss_fs_segment_register_descriptor_type_0 = 0;
	tss->tss_fs_segment_register_descriptor_type_c = 0;
	tss->tss_fs_segment_register_descriptor_type_e = 0;
	tss->tss_fs_segment_register_descriptor_type_r = 0;
	tss->tss_fs_segment_register_descriptor_type_w = 0;
	tss->tss_fs_segment_register_selector_table_indicator = 0;
	tss->tss_fs_segment_register_selector_index = 0;
	tss->tss_fs_segment_register_selector_requester_privilege_level = 0;
	tss->tss_gs_segment_register_descriptor_base_address_offset = 0;
	tss->tss_gs_segment_register_descriptor_limit = 0;
	tss->tss_gs_segment_register_descriptor_granularity_bit = 0;
	tss->tss_gs_segment_register_descriptor_big = 0;
	tss->tss_gs_segment_register_descriptor_default = 0;
	tss->tss_gs_segment_register_descriptor_x_ = 0;
	tss->tss_gs_segment_register_descriptor_0_ = 0;
	tss->tss_gs_segment_register_descriptor_available = 0;
	tss->tss_gs_segment_register_descriptor_is_present = 0;
	tss->tss_gs_segment_register_type_of_segment_or_gate = 0;
	tss->tss_gs_segment_register_descriptor_type_1 = 0;
	tss->tss_gs_segment_register_descriptor_type_0 = 0;
	tss->tss_gs_segment_register_descriptor_type_c = 0;
	tss->tss_gs_segment_register_descriptor_type_e = 0;
	tss->tss_gs_segment_register_descriptor_type_r = 0;
	tss->tss_gs_segment_register_descriptor_type_w = 0;
	tss->tss_gs_segment_register_selector_table_indicator = 0;
	tss->tss_gs_segment_register_selector_index = 0;
	tss->tss_gs_segment_register_selector_requester_privilege_level = 0;
	tss->tss_ss_segment_register_descriptor_base_address_offset_for_privilege_level_2 = 0;
	tss->tss_ss_segment_register_descriptor_limit_for_privilege_level_2 = 0;
	tss->tss_ss_segment_register_descriptor_granularity_bit_for_privilege_level_2 = 0;
	tss->tss_ss_segment_register_descriptor_big_for_privilege_level_2 = 0;
	tss->tss_ss_segment_register_descriptor_default_for_privilege_level_2 = 0;
	tss->tss_ss_segment_register_descriptor_x_for_privilege_level_2 = 0;
	tss->tss_ss_segment_register_descriptor_0_for_privilege_level_2 = 0;
	tss->tss_ss_segment_register_descriptor_available_for_privilege_level_2 = 0;
	tss->tss_ss_segment_register_descriptor_is_present_for_privilege_level_2 = 0;
	tss->tss_ss_segment_register_type_of_segment_or_gate_for_privilege_level_2 = 0;
	tss->tss_ss_segment_register_descriptor_type_1_for_privilege_level_2 = 0;
	tss->tss_ss_segment_register_descriptor_type_0_for_privilege_level_2 = 0;
	tss->tss_ss_segment_register_descriptor_type_c_for_privilege_level_2 = 0;
	tss->tss_ss_segment_register_descriptor_type_e_for_privilege_level_2 = 0;
	tss->tss_ss_segment_register_descriptor_type_r_for_privilege_level_2 = 0;
	tss->tss_ss_segment_register_descriptor_type_w_for_privilege_level_2 = 0;
	tss->tss_ss_segment_register_selector_table_indicator_for_privilege_level_2 = 0;
	tss->tss_ss_segment_register_selector_index_for_privilege_level_2 = 0;
	tss->tss_ss_segment_register_selector_requester_privilege_level_for_privilege_level_2 = 0;
	tss->tss_ss_segment_register_descriptor_base_address_offset_for_privilege_level_1 = 0;
	tss->tss_ss_segment_register_descriptor_limit_for_privilege_level_1 = 0;
	tss->tss_ss_segment_register_descriptor_granularity_bit_for_privilege_level_1 = 0;
	tss->tss_ss_segment_register_descriptor_big_for_privilege_level_1 = 0;
	tss->tss_ss_segment_register_descriptor_default_for_privilege_level_1 = 0;
	tss->tss_ss_segment_register_descriptor_x_for_privilege_level_1 = 0;
	tss->tss_ss_segment_register_descriptor_0_for_privilege_level_1 = 0;
	tss->tss_ss_segment_register_descriptor_available_for_privilege_level_1 = 0;
	tss->tss_ss_segment_register_descriptor_is_present_for_privilege_level_1 = 0;
	tss->tss_ss_segment_register_type_of_segment_or_gate_for_privilege_level_1 = 0;
	tss->tss_ss_segment_register_descriptor_type_1_for_privilege_level_1 = 0;
	tss->tss_ss_segment_register_descriptor_type_0_for_privilege_level_1 = 0;
	tss->tss_ss_segment_register_descriptor_type_c_for_privilege_level_1 = 0;
	tss->tss_ss_segment_register_descriptor_type_e_for_privilege_level_1 = 0;
	tss->tss_ss_segment_register_descriptor_type_r_for_privilege_level_1 = 0;
	tss->tss_ss_segment_register_descriptor_type_w_for_privilege_level_1 = 0;
	tss->tss_ss_segment_register_selector_table_indicator_for_privilege_level_1 = 0;
	tss->tss_ss_segment_register_selector_index_for_privilege_level_1 = 0;
	tss->tss_ss_segment_register_selector_requester_privilege_level_for_privilege_level_1 = 0;
	tss->tss_ss_segment_register_descriptor_base_address_offset_for_privilege_level_0 = 0;
	tss->tss_ss_segment_register_descriptor_limit_for_privilege_level_0 = 0;
	tss->tss_ss_segment_register_descriptor_limit_for_privilege_level_0 = 0;
	tss->tss_ss_segment_register_descriptor_limit_for_privilege_level_0 = 0;
	tss->tss_ss_segment_register_descriptor_granularity_bit_for_privilege_level_0 = 0;
	tss->tss_ss_segment_register_descriptor_big_for_privilege_level_0 = 0;
	tss->tss_ss_segment_register_descriptor_default_for_privilege_level_0 = 0;
	tss->tss_ss_segment_register_descriptor_x_for_privilege_level_0 = 0;
	tss->tss_ss_segment_register_descriptor_0_for_privilege_level_0 = 0;
	tss->tss_ss_segment_register_descriptor_available_for_privilege_level_0 = 0;
	tss->tss_ss_segment_register_descriptor_is_present_for_privilege_level_0 = 0;
	tss->tss_ss_segment_register_type_of_segment_or_gate_for_privilege_level_0 = 0;
	tss->tss_ss_segment_register_descriptor_type_1_for_privilege_level_0 = 0;
	tss->tss_ss_segment_register_descriptor_type_0_for_privilege_level_0 = 0;
	tss->tss_ss_segment_register_descriptor_type_c_for_privilege_level_0 = 0;
	tss->tss_ss_segment_register_descriptor_type_e_for_privilege_level_0 = 0;
	tss->tss_ss_segment_register_descriptor_type_r_for_privilege_level_0 = 0;
	tss->tss_ss_segment_register_descriptor_type_w_for_privilege_level_0 = 0;
	tss->tss_ss_segment_register_selector_table_indicator_for_privilege_level_0 = 0;
	tss->tss_ss_segment_register_selector_index_for_privilege_level_0 = 0;
	tss->tss_ss_segment_register_selector_requester_privilege_level_for_privilege_level_0 = 0;
	tss->tss_back_link = 0;
	return (tss);
}

void tss_del (tss_task_state_segment_descriptor_t *tss)
{
	if (tss != NULL) {
		free (tss->tss_gdt_selector_index);
		free (tss);
	}
}

tss_task_state_segment_descriptor_t *tss_get_task_state_segment_descriptor (tss_task_state_segment_descriptor_t *tss)
{
	unsigned tss_gdt_segment;
	unsigned tss_ldt_segment;
	unsigned tss_directory_page_address;
	unsigned tss_backlink_value;
	if ((tss_gdt_segment > 0) && (tss_ldt_segment > 0) && (tss_directory_page_address > 0) && (tss_backlink > 0)) {
		return (tss->tss_gdt_selector_index[tss_gdt_segment]); /* Removed " - 1" from "[address]" */
		return (tss->tss_ldt_selector_index[tss_ldt_segment]); /* Removed " - 1" from "[address]" */
		return (tss->tss_directory_page_frame_address[tss_directory_page_address]); /* Removed " - 1" from "[address]" */
		return (tss->tss_back_link[tss_backlink_value]); /* Removed " - 1" from "[address]" */
	}
	return (NULL);
}

/* task_state_segment_descriptor_t *tss_get_task_state_segment_selector_for_task_gate (tss_task_state_segment_descriptor_t *tss)
{
	/* NOTE: Used only for identifying and associating task gate selectors with Task State Segment descriptors */
/*	unsigned selector;
	if ((tss_gdt_segment > 0) && (tss_ldt_segment > 0) && (tss_directory_page_address > 0) && (tss_backlink > 0)) {
		return (task_state_segment_descriptor->tss_gdt_selector_index[selector]); /* Removed " - 1" from "[address]" */
/*	}
	return (NULL);
}

task_state_segment_descriptor_t *tss_get_task_state_segment_limit_for_task_gate (tss_task_state_segment_descriptor_t *tss)
{
	/* NOTE: Used only for identifying and associating task gate selectors with Task State Segment descriptors */
/*	unsigned task_gate_limit;
	if ((tss_gdt_segment > 0) && (tss_ldt_segment > 0) && (tss_directory_page_address > 0) && (tss_backlink > 0)) {
		return (task_state_segment_descriptor->tss_gdt_selector_index[task_gate_limit]); /* Removed " - 1" from "[address]" */
/*	}
	return (NULL);
} */

int tss_set_task_state_segment_descriptor (tss_task_state_segment_descriptor_t *tss)
{
	unsigned tss_gdt_segment;
	unsigned tss_ldt_segment;
	unsigned tss_directory_page_address;
	unsigned tss_backlink_value;
	if ((tss_gdt_segment == 0) || (tss_ldt_segment == 0) || (tss_directory_page_address == 0) || (tss_backlink_value == 0)) {
		return (1);
	}
	if ((tss_gdt_segment > 0) && (tss_ldt_segment > 0) && (tss_directory_page_address > 0) && (tss_backlink_value > 0)) {
		unsigned value;
		value = tss_gdt_segment;
		tss->tss_gdt_selector_index = (tss_task_state_segment_descriptor_t **) realloc (tss->tss_gdt_selector_index, value * sizeof (tss_task_state_segment_descriptor_t));
		value = tss_ldt_segment;
		tss->tss_ldt_selector_index = (tss_task_state_segment_descriptor_t **) realloc (tss->tss_ldt_selector_index, value * sizeof (tss_task_state_segment_descriptor_t));
		value = tss_directory_page_address;
		tss->tss_directory_page_frame_address = (tss_task_state_segment_descriptor_t **) realloc (tss->tss_directory_page_frame_address, value * sizeof (tss_task_state_segment_descriptor_t));
		value = tss_backlink_value;
		tss->tss_backlink = (tss_task_state_segment_descriptor_t **) realloc (tss->tss_backlink, value * sizeof (tss_task_state_segment_descriptor_t));
	}
	return (0);
}

int tss_allocate_new_task_state_segment_descriptor (tss_task_state_segment_descriptor_t *tss)
{
	/* unsigned *address; */
	unsigned tss_gdt_segment;
	unsigned tss_ldt_segment;
	unsigned tss_directory_page_address;
	unsigned tss_backlink_value;
	tss_task_state_segment_descriptor_t *tss;
	/* address = tss_gdt_segment */
	tss = task_state_segment_descriptor_new (tss_gdt_segment_limit) (tss_ldt_segment_limit) (tss_directory_page_frame_address_being_loaded);
	tss_set_task_state_segment_descriptor (tss, tss_gdt_selector_index, tss_gdt_segment);
	/* address = tss_ldt_segment */
	tss_set_task_state_segment_descriptor (tss, tss_ldt_selector_index, tss_ldt_segment);
	/* address = tss_directory_page_address; */
	tss_set_task_state_segment_descriptor (tss, tss_directory_page_frame_address, tss_directory_page_address);
	/* address = tss_backlink_value; */
	tss_set_task_state_segment_descriptor (tss, tss_back_link, tss_backlink_value);
	return (0);
}

static
int tss_remove_task_state_segment_descriptor (tss_task_state_segment_descriptor_t *tss)
{
	unsigned tss_gdt_segment;
	unsigned tss_ldt_segment;
	unsigned tss_directory_page_address;
	unsigned tss_backlink_value
	if ((tss_gdt_segment == 0) && (tss_ldt_segment == 0) && (tss_directory_page_address == 0) && (tss_backlink_value == 0)) {
		return (1);
	}
	tss_del (tss->tss_gdt_selector_index[tss_gdt_segment]); /* Removed " - 1" from "[segment]" */
	tss_del (tss->tss_ldt_selector_index[tss_ldt_segment]); /* Removed " - 1" from "[segment]" */
	tss_del (tss->tss_directory_page_frame_address[tss_directory_page_address]); /* Removed " - 1" from "[segment]" */
	tss_del (tss->tss_back_link[tss_backlink_value]); /* Removed " - 1" from "[segment]" */
	tss->tss_gdt_selector_index[tss_gdt_segment] = NULL;    /* Removed " - 1" from "[segment]" */
	tss->tss_ldt_selector_index[tss_ldt_segment] = NULL;    /* Removed " - 1" from "[segment]" */
	tss->tss_directory_page_frame_address[tss_directory_page_address] = NULL;    /* Removed " - 1" from "[segment]" */
	tss->tss_back_link[tss_backlink_value] = NULL;    /* Removed " - 1" from "[segment]" */
	return (0);
}

tss_task_state_segment_descriptor_t *tss_task_gate_descriptor_for_global_descriptor_table_new (unsigned long task_gate_selector_index);
{
	unsigned long tss_gdt_segment_limit; /* Task State Segment being created */
	tss_task_state_segment_descriptor_t *tss;
	tss = malloc (sizeof (tss_task_state_segment_descriptor_t));
	if (tss == NULL) {
		return (NULL);
	}
	if (tss_gdt_segment_limit > 0) {
		tss->task_gate_selector_index = malloc (tss_gdt_segment_limit);
		if (tss->tss_gdt_segment_limit == NULL) {
			free (tss);
			return (NULL);
		}
	}
	else {
		tss->task_gate_selector_index = NULL;
	}
	tss->task_gate_is_present = 0;
	tss->task_gate_type_of_segment_or_gate = 5;
	tss->task_gate_segment_accessed = 0;
	tss->task_gate_selector_table_indicator = 0;
	tss->task_gate_selector_index = task_gate_selector_index;
	tss->task_gate_selector_requester_privilege_level = 0;
	return (tss);
}

void tss_task_gate_for_global_descriptor_table_del (tss_task_state_segment_descriptor_t *tss)
{
	if (tss != NULL) {
		free (tss->task_gate_selector_index);
		free (tss);
	}
}

tss_task_state_segment_descriptor_t *tss_task_gate_descriptor_for_local_descriptor_table_new (unsigned long task_gate_selector_index);
{
	unsigned long tss_gdt_segment_limit; /* Task State Segment being created */
	tss_task_state_segment_descriptor_t *tss;
	tss = malloc (sizeof (tss_task_state_segment_descriptor_t));
	if (tss == NULL) {
		return (NULL);
	}
	if (tss_gdt_segment_limit > 0) {
		tss->task_gate_selector_index = malloc (tss_gdt_segment_limit);
		if (tss->tss_gdt_segment_limit == NULL) {
			free (tss);
			return (NULL);
		}
	}
	else {
		tss->task_gate_selector_index = NULL;
	}
	tss->task_gate_is_present = 0;
	tss->task_gate_type_of_segment_or_gate = 5;
	tss->task_gate_segment_accessed = 0;
	tss->task_gate_selector_table_indicator = 1; /* Local Descriptor Table */
	tss->task_gate_selector_index = task_gate_selector_index;
	tss->task_gate_selector_requester_privilege_level = 0;
	return (tss);
}

void tss_task_gate_for_local_descriptor_table_del (tss_task_state_segment_descriptor_t *tss)
{
	if (tss != NULL) {
		free (tss->task_gate_selector_index);
		free (tss);
	}
}

tss_task_state_segment_descriptor_t *tss_task_gate_descriptor_for_interrupt_descriptor_table_new (unsigned long task_gate_selector_index);
{
	unsigned long tss_gdt_segment_limit; /* Task State Segment being created */
	tss_task_state_segment_descriptor_t *tss;
	tss = malloc (sizeof (tss_task_state_segment_descriptor_t));
	if (tss == NULL) {
		return (NULL);
	}
	if (tss_gdt_segment_limit > 0) {
		tss->task_gate_selector_index = malloc (tss_gdt_segment_limit);
		if (tss->tss_gdt_segment_limit == NULL) {
			free (tss);
			return (NULL);
		}
	}
	else {
		tss->task_gate_selector_index = NULL;
	}
	tss->task_gate_is_present = 0;
	tss->task_gate_type_of_segment_or_gate = 5;
	tss->task_gate_segment_accessed = 0;
	tss->task_gate_selector_table_indicator = 1; /* Local Descriptor Table */
	tss->task_gate_selector_index = task_gate_selector_index;
	tss->task_gate_selector_requester_privilege_level = 0;
	return (tss);
}

void tss_task_gate_for_interrupt_descriptor_table_del (tss_task_state_segment_descriptor_t *tss)
{
	if (tss != NULL) {
		free (tss->task_gate_selector_index);
		free (tss);
	}
}
/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual; 7.4 Task Gate Descriptor:                                  */
/* ---------------------------------------------------------------------------------------------------------------- */
/* A task gate descriptor provides an indirect, protected reference to a TSS. Figure 7-4 illustrates the format of a*/
/* task gate.                                                                                                       */
/*                                                                                                                  */
/* The SELECTOR field of a task gate must refer to a TSS descriptor. The value of the RPL in this selector is not   */
/* used by processor.                                                                                               */
/*                                                                                                                  */
/* The DPL field of a task gate controls the right to use the descriptor to cause a task switch. A procedure may not*/
/*select a task gate descriptor unless the maximum of the selector's RPL and the CPL of the procedure is numerically*/
/*less than or equal to the DPL of the descriptor. This constraint prevents untrusted procedures from causing a task*/
/* switch. (Note that when a task gate is used, the DPL of the target TSS descriptor is not used for privilege      */
/* checking.)                                                                                                       */
/*                                                                                                                  */
/* A procedure that has access to a task gate has the power to cause a task switch, just as a procedure that has    */
/* access to a TSS descriptor.                                                                                      */
/* The 80386 has task gates in addition to TSS descriptors to satisfy three needs:                                  */
/*  - 1.The need for a task to have a single busy bit. Because the busy-bit is stored in the TSS descriptor, each   */
/*  task should have only one such descriptor. There may, however, be several task gates that select the single TSS */
/*  descriptor.                                                                                                     */
/*  - 2.The need to provide selective access to tasks. Task gates fulfill this need, because they can reside in LDTs*/
/*  and can have a DPL that is different from the TSS descriptor's DPL. A procedure that does not have sufficient   */
/*  privilege to use the TSS descriptor in the GDT (which usually has a DPL of 0) can still switch to another task  */
/*  if it has access to a task gate for that task in its LDT. With task gates, systems software can limit the right */
/*  to cause task switches to specific tasks.                                                                       */
/*  - 3.The need for an interrupt or exception to cause a task switch. Task gates may also reside in the IDT, making*/
/*  it possible for interrupts and exceptions to cause task switching. When interrupt or exception vectors to an IDT*/
/*  entry that contains a task gate, the 80386 switches to the indicated task. Thus, all tasks in the system can    */
/*  benefit from the protection afforded by isolation from interrupt tasks.                                         */
/* Figure 7-5 illustrates how both a task gate in an LDT and a task gate in the IDT can identify the same task.     */
/* ---------------------------------------------------------------------------------------------------------------- */
task_state_segment_descriptor_t *tss_get_task_gate_descriptor (tss_task_state_segment_descriptor_t *tss)
{
	unsigned task_gate_selector;
	if (task_gate_selector > 0) {
		return (tss->tss_gdt_segment_base[tss_gdt_segment_base]); /* Removed " - 1" from "[address]" */
		return (tss->task_gate_selector_index[task_gate_selector_index]); /* Removed " - 1" from "[address]" */
	}
	return (NULL);
}

int tss_set_task_gate_selector_descriptor (tss_task_state_segment_descriptor_t *tss)
{
	unsigned task_gate_selector;
	if (task_gate_selector == 0) {
		return (1);
	}
	if (task_gate_selector > 0) {
		unsigned value;
		value = tas_gate_selector;
		tss->tss_gdt_segment_base = (tss_task_state_segment_descriptor_t **) realloc (tss->tss_gdt_segment_base, value * sizeof (tss_task_state_segment_descriptor_t));
		tss->task_gate_segment_base = (tss_task_state_segment_descriptor_t **) realloc (tss->task_gate_selector_index, value * sizeof (tss_task_state_segment_descriptor_t));
	}
	return (0);
}
/* int tss_allocate_new_task_gate_selector_descriptor (tss_task_state_segment_descriptor_t *tss)
{
	/* unsigned *address; */
/*	unsigned task_gate_selector;
	tss_task_state_segment_descriptor_t *tss;
	/* address = tss_gdt_segment */
/*	tss = task_state_segment_descriptor_new (tss_gdt_segment_limit) (tss_ldt_segment_limit) (tss_directory_page_frame_address_being_loaded);
	tss_set_task_state_segment_descriptor (tss, tss_gdt_segment_base, tss_gdt_segment);
	/* address = tss_ldt_segment */
/*	tss_set_task_state_segment_descriptor (tss, tss_ldt_segment_base, tss_ldt_segment);
	/* address = tss_directory_page_address; */
/*	tss_set_task_state_segment_descriptor (tss, tss_directory_page_frame_address, tss_directory_page_address);
	/* address = tss_backlink_value; */
/*	tss_set_task_state_segment_descriptor (tss, tss_back_link, tss_backlink_value);
	return (0);
} */

static
int tss_remove_task_gate_selector_descriptor (tss_task_state_segment_descriptor_t *tss)
{
	unsigned task_gate_selector;
	if (task_gate_selector == 0) {
		return (1);
	}
	tss_del (tss->task_gate_selector_index[task_gate_selector]); /* Removed " - 1" from "[segment]" */
	tss->task_gate_selector_index[task_gate_selector] = NULL;    /* Removed " - 1" from "[segment]" */
	return (0);
}
/* NOTE: The quotes below refer strictly to subroutines that are planned to be added in the future. */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual: 7.5 Task Switching:                                        */
/* ---------------------------------------------------------------------------------------------------------------- */
/* The 80386 switches execution to another task in any of four cases:                                               */
/*  - 1. The current task executes a JMP or CALL that refers to a TSS descriptor.                                   */
/*  - 2. The current task executes a JMP or CALL that refers to a task gate.                                        */
/*  - 3. An interrupt or exception vectors to a task gate in the IDT.                                               */
/*  - 4. The current task executes an IRET when the NT flag is set.                                                 */
/* JMP, CALL, IRET, interrupts, and exceptions are all ordinary mechanisms of the 80386 that can be used in         */
/* circumstances that do not require a task switch. Either the type of descriptor referenced or the NT (nested task)*/
/* bit in the flag word distinguishes between the standard mechanism and the variant that causes a task switch.     */
/*                                                                                                                  */
/* To cause a task switch, a JMP or CALL instruction can refer either to a TSS descriptor or to a task gate. The    */
/* effect is the same in either case: the 80386 switches to the indicated task.                                     */
/*                                                                                                                  */
/* An exception or interrupt causes a task switch when it vectors to a task gate in the IDT. If it vectors to an    */
/* interrupt or trap gate in the IDT, a task switch does not occur. Refer to Chapter 9 for more information on the  */
/* interrupt mechanism.                                                                                             */
/*                                                                                                                  */
/* Whether invoked as a task or as a procedure of the interrupted task, an interrupt handler always returns control */
/* to the interrupted procedure in the interrupted task. If the NT flag is set, however, the handler is an interrupt*/
/* task, and the IRET switches back to the interrupted task.                                                        */
/*                                                                                                                  */
/* A task switching operation involves these steps:                                                                 */
/*  - 1. Checking that the current task is allowed to switch to the designated task. Data-access privilege rules    */
/*  apply in the case of JMP or CALL instructions. The DPL of the TSS descriptor or task gate must be numerically   */
/*  greater (e.g., lower privilege level) than or equal to the maximum of CPL and the RPL of the gate selector.     */
/*  Exceptions, interrupts, and IRET are permitted to switch tasks regardless of the DPL of the target task gate or */
/*  TSS descriptor.                                                                                                 */
/*  - 2. Checking that the TSS descriptor of the new task is marked present and has a valid limit. Any errors up to */
/*  this point occur in the context of the outgoing task. Errors are restartable and can be handled in a way that is*/
/*  transparent to applications procedures.                                                                         */
/*  - 3. Saving the state of the current task. The processor finds the base address of the current TSS cached in the*/
/*  task register. It copies the registers into the current TSS (EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI, ES, CS, SS,*/
/*  DS, FS, GS, and the flag register). The EIP field of the TSS points to the instruction after the one that caused*/
/*  the task switch.                                                                                                */
/*  - 4. Loading the task register with the selector of the incoming task's TSS descriptor, marking the incoming    */
/*  task's TSS descriptor as busy, and setting the TS (task switched) bit of the MSW. The selector is either the    */
/*  operand of a control transfer instruction or is taken from a task gate.                                         */
/*  - 5. Loading the incoming task's state from its TSS and resuming execution. The registers loaded are the LDT    */
/*  register; the flag register; the general registers EIP, EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI; the segment     */
/*  registers ES, CS, SS, DS, FS, and GS; and PDBR. Any errors detected in this step occur in the context of the    */
/*  incoming task. To an exception handler, it appears that the first instruction of the new task has not yet       */
/*  executed.                                                                                                       */
/* Note that the state of the outgoing task is always saved when a task switch occurs. If execution of that task is */
/* resumed, it starts after the instruction that caused the task switch. The registers are restored to the values   */
/* they held when the task stopped executing.                                                                       */
/*                                                                                                                  */
/* Every task switch sets TS (task switched) bit in the MSW (machine status word). The TS flag is useful to systems */
/* software when a coprocessor (such as a numerics coprocessor) is present. The TS bit signals that the context of  */
/* the coprocessor may not correspond to the current 80386 task. Chapter 11 discusses the TS bit and coprocessors in*/
/* more detail.                                                                                                     */
/*                                                                                                                  */
/* Exception handlers that field task-switch exception in the incoming task (exceptions due to tests 4 thru 16 of   */
/*Table 7-1) should be cautious about taking any action that might load the selector that caused the exception. Such*/
/* an action will probably cause another exception, unless the exception handler first examines the selector and    */
/* fixes any potential problem.                                                                                     */
/*                                                                                                                  */
/* The privilege level at which execution resumes in the incoming task is neither restricted nor affected by the    */
/*privilege level at which the outgoing task was executing. Because the tasks are isolated by their separate address*/
/* spaces and TSSs and because privilege rules can be used to prevent improper access to a TSS, no privilege rules  */
/*are needed to constrain the relation between the CPLs of the tasks. The new task begins executing at the privilege*/
/* level indicated by the RPL of the CS selector values that is loaded from the TSS.                                */
/*                                                                                                                  */
/* Table 7-1. Checks Made during a Task Switch:                                                                     */
/*  - NP = Segment-not-present exception                                                                            */
/*  - GP = General protection fault                                                                                 */
/*  - TS = Invalid TSS                                                                                              */
/*  - SF = Stack fault                                                                                              */
/* Validity tests of a selector check that the selector is in the proper table (e.g., the LDT selector refers to the*/
/* GDT), lies within the bounds of the table, and refers to the proper type of descriptor (e.g., the LDT selector   */
/* refers to an LDT descriptor).                                                                                    */
/*                                                                                                                  */
/* Test: Test Descripton:                                                             Exception: Error Code Selects:*/
/*  -  1:  - Incoming TSS descriptor is present:                                       - NP:      - Incoming TSS    */
/*  -  2:  - Incoming TSS descriptor is marked not-busy:                               - GP:      - Incoming TSS    */
/*  -  3:  - Limit of incoming TSS is greater than or equal to 103:                    - TS:      - Incoming TSS    */
/* -- All register and selector values are loaded --                                                                */
/*  -  4:  - LDT selector of incoming task is valid:                                   - TS:      - Incoming TSS    */
/*  -  5:  - LDT of incoming task is present:                                          - TS:      - Incoming TSS    */
/*  -  6:  - CS selector is valid:                                                     - TS:      - Code  segment   */
/*  -  7:  - Code segment is present:                                                  - NP:      - Code  segment   */
/*  -  8:  - Code segment DPL matches CS RPL                                           - TS:      - Code  segment   */
/*  -  9:  - Stack segment is valid:                                                   - GP:      - Stack segment   */
/*  - 10:  - Stack segment is present:                                                 - SF:      - Stack segment   */
/*  - 11:  - Stack segment  DPL = CPL:                                                 - SF:      - Stack segment   */
/*  - 12:  - Stack-selector RPL = CPL:                                                 - GP:      - Stack segment   */
/*  - 13:  - DS, ES, FS, GS selectors are valid:                                       - GP:      -       Segment   */
/*  - 14:  - DS, ES, FS, GS segments are readable:                                     - GP:      -       Segment   */
/*  - 15:  - DS, ES, FS, GS segments are present:                                      - NP:      -       Segment   */
/*  - 16:  - DS, ES, FS, GS segment DPL >= CPL (unless these are conforming segments): - GP:      -       Segment   */
/* ---------------------------------------------------------------------------------------------------------------- */
/* -------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual: 6.3.1.2 Limit Checking:                                    */
/* ---------------------------------------------------------------------------------------------------------------- */
/* The limit field of a segment descriptor is used by the processor to prevent programs from addressing outside the */
/* segment. The processor's interpretation of the limit depends on the setting of the G (granularity) bit. For data */
/* segments, the processor's interpretation of the limit depends also on the E-bit (expansion-direction bit) and the*/
/* B-bit (big bit) (refer to Table 6-2).                                                                            */
/*                                                                                                                  */
/* When G=0, the actual limit is the value of the 20-bit limit field as it appears in the descriptor. In this case, */
/*the limit may range from 0 to 0FFFFFH (2^(20) - 1 or 1 megabyte). When ^=1, the processor appends 12 low-order one*/
/* -bits to the value in the limit field. In this case the actual limit may range from 0FFFH (2^(12) - 1 or 4       */
/* kilobytes) to 0FFFFFFFFH(2^(32) - 1 or 4 gigabytes).                                                             */
/*                                                                                                                  */
/* For all types of segments except expand-down data segments, the value of the limit is one less than the size     */
/* (expressed in bytes) of the* segment. The processor causes a general-protection exception in any of these cases: */
/*  - Attempt to access a memory byte at an address > limit.                                                        */
/*  - Attempt to access a memory word at an address >= limit.                                                       */
/*  - Attempt to access a memory doubleword at an address >= (limit-2).                                             */
/* For expand-down data segments, the limit has the same function but is interpreted differently. In these cases the*/
/* range of valid addresses is from limit + 1 to either 64K or 2^(32) - 1 (4 Gbytes) depending on the B-bit. An     */
/*expand-down segment has maximum size when the limit is zero. (Turning on the expand-down bit swaps which bytes are*/
/* accessible and which are not.)                                                                                   */
/*                                                                                                                  */
/* The expand-down feature makes it possible to expand the size of a stack by copying it to a larger segment without*/
/* needing also to update intrastack pointers.                                                                      */
/*                                                                                                                  */
/* The limit field of descriptors for descriptor tables is used by the processor to prevent programs from selecting */
/* a table entry outside the descriptor table. The limit of a descriptor table identifies the last valid byte of the*/
/* last descriptor in the table. Since each descriptor is eight bytes long, the limit value is N * 8 - 1 for a table*/
/* that can contain up to N descriptors.                                                                            */
/*                                                                                                                  */
/*Limit checking catches programming errors such as runaway subscripts and invalid pointer calculations. Such errors*/
/* are detected when they occur, so that identification of the cause is easier. Without limit checking, such errors */
/* could corrupt other modules; the existence of such errors would not be discovered until later, when the corrupted*/
/* module behaves incorrectly, and when identification of the cause is difficult.                                   */
/* ---------------------------------------------------------------------------------------------------------------- */

/* NOTE: The following instructions below are kept as references for when 80386 CPU instructions start being added  */
/*in for Protected Mode and Virtual 8086 Mode. These instructions will be used for modifying settings for descriptor*/
/* table entries.                                                                                                   */
/* 03: global enable a20 */
/* void xms_03 (xms_t *xms, e8086_t *cpu)
{
	e86_set_addr_mask (cpu, ~0UL);
	e86_set_ax (cpu, 0x0001);
	e86_set_bl (cpu, 0x00);
}

/* 04: global disable a20 */
/* void xms_04 (xms_t *xms, e8086_t *cpu)
{
	e86_set_addr_mask (cpu, 0xfffff);
	e86_set_ax (cpu, 0x0001);
	e86_set_bl (cpu, 0x00);
}

/* 05: local enable a20 */
/* void xms_05 (xms_t *xms, e8086_t *cpu)
{
	e86_set_addr_mask (cpu, ~0UL);
	e86_set_ax (cpu, 0x0001);
	e86_set_bl (cpu, 0x00);
}

/* 06: local disable a20 */
/* void xms_06 (xms_t *xms, e8086_t *cpu)
{
	e86_set_addr_mask (cpu, 0xfffff);
	e86_set_ax (cpu, 0x0001);
	e86_set_bl (cpu, 0x00);
}

/* 07: query a20 */
/* void xms_07 (xms_t *xms, e8086_t *cpu)
{
	e86_set_ax (cpu, (e86_get_addr_mask (cpu) & 0x100000) != 0);
	e86_set_bl (cpu, 0x00);
}

static
void inc_addr (unsigned short *seg, unsigned short *ofs, unsigned short val)
{
	*ofs += val;
	*seg += *ofs >> 4;
	*ofs &= 0x000f;
}

/* 0C: lock extended memory block */
/* void xms_0c (xms_t *xms, e8086_t *cpu)
{
	unsigned  handle;
	xms_emb_t *blk;

	handle = e86_get_dx (cpu);
	blk = xms_get_emb (xms, handle);
	if (blk == NULL) {
		e86_set_ax (cpu, 0x0000);
		e86_set_bl (cpu, 0xa2);
		return;
	}
	blk->lock += 1;
	e86_set_ax (cpu, 0x0001);
	e86_set_bx (cpu, 0x0000);
	e86_set_dx (cpu, 0x0001);
}

/* 0D: unlock extended memory block */
/* void xms_0d (xms_t *xms, e8086_t *cpu)
{
	unsigned  handle;
	xms_emb_t *blk;

	handle = e86_get_dx (cpu);
	blk = xms_get_emb (xms, handle);
	if (blk == NULL) {
		e86_set_ax (cpu, 0x0000);
		e86_set_bl (cpu, 0xa2);
		return;
	}
	if (blk->lock == 0) {
		e86_set_ax (cpu, 0x0000);
		e86_set_bl (cpu, 0xaa);
		return;
	}
	blk->lock -= 1;
	e86_set_ax (cpu, 0x0001);
}

/* 0E: get emb handle information */
/* void xms_0e (xms_t *xms, e8086_t *cpu)
{
	unsigned  handle;
	xms_emb_t *blk;

	handle = e86_get_dx (cpu);
	blk = xms_get_emb (xms, handle);
	if (blk == NULL) {
		e86_set_ax (cpu, 0x0000);
		e86_set_bl (cpu, 0xa2);
		return;
	}
	e86_set_ax (cpu, 0x0001);
	e86_set_bh (cpu, blk->lock);
	e86_set_bl (cpu, 255);
	e86_set_dx (cpu, blk->size / 1024);
}

/* 0F: reallocate emb */
/* void xms_0f (xms_t *xms, e8086_t *cpu)
{
	unsigned      handle;
	unsigned long size;
	unsigned char *tmp;
	xms_emb_t     *blk;

	size = 1024UL * e86_get_bx (cpu);
	if (size > (xms->emb_max - xms->emb_used)) {
		e86_set_ax (cpu, 0x0000);
		e86_set_bl (cpu, 0xa0);
		return;
	}
	handle = e86_get_dx (cpu);
	blk = xms_get_emb (xms, handle);
	if (blk == NULL) {
		e86_set_ax (cpu, 0x0000);
		e86_set_bl (cpu, 0xa2);
		return;
	}
	tmp = (unsigned char *) realloc (blk->data, size);
	if (tmp == NULL) {
		e86_set_ax (cpu, 0x0000);
		e86_set_bl (cpu, 0xa0);
		return;
	}
	xms->emb_used += size;
	xms->emb_used -= blk->size;
	blk->data = tmp;
	blk->size = size;
	e86_set_ax (cpu, 0x0001);
}
