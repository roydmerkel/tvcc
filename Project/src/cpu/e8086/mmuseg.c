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


#include "e8086.h"
#include "mmuseg.h"

#include <stdlib.h>
#include <string.h>


/* ----------------------------------- */
/*           Global   Descriptor Table */
/*           Global   Descriptor Table */
/*           Global   Descriptor Table */
/*           Global   Descriptor Table */
/*           Global   Descriptor Table */
/*           Global   Descriptor Table */
/*           Global   Descriptor Table */
/*           Global   Descriptor Table */
/*           Global   Descriptor Table */
/*           Global   Descriptor Table */
/* ----------------------------------- */
mmuseg_global_descriptor_table_gdt_t *gdt_new (unsigned long selector_table_index)
{
	mmuseg_global_descriptor_table_gdt_t *gdt;
	gdt = malloc (sizeof (mmuseg_global_descriptor_table_gdt_t));
	if (gdt == NULL) {
		return (NULL);
	}
	if (selector_table_index > 0) {
		gdt->selector = malloc (selector_table_index);
		if (gdt->selector == NULL) {
			free (gdt);
			return (NULL);
		}
	}
	else {
		gdt->selector = NULL;
	}
	gdt->selector_table_index = selector_table_index;
	gdt->present = 0;
	return (gdt);
}

void gdt_del (mmuseg_global_descriptor_table_gdt_t *gdt)
{
	if (gdt != NULL) {
		free (gdt->selector);
		free (gdt);
	}
}

mmuseg_global_descriptor_table_t *mmuseg_gdt_new (gdt_segment_t *seg)
{
	mmuseg_global_descriptor_table_t         *mmu_gdt;
	/* init->mmu = mmuseg_new(segment); */
	mmu_gdt = malloc (sizeof (mmuseg_global_descriptor_table_t));
	if (mmu_gdt == NULL) {
		return (NULL);
	}
	mmu_gdt->largest_gdt_in_use = 0;
	/* mmu->gdt = NULL; */
	return (mmu_gdt);
}

void mmuseg_gdt_del (mmuseg_global_descriptor_table_t *mmu_gdt)
{
	unsigned i;
	if (mmu_gdt != NULL) {
		for (i = 0; i <= mmu_gdt->largest_gdt_in_use; i++) {
			gdt_del (mmu_gdt->gdt[i]);
		}
	}
}

mmuseg_global_descriptor_table_gdt_t *mmuseg_get_gdt (mmuseg_global_descriptor_table_t *mmu_gdt, unsigned selector)
{
	if ((selector > 0) && (selector <= mmu_gdt->largest_gdt_in_use)) {
		return (mmu_gdt->gdt[selector - 1]);
	}
	return (NULL);
}

int mmuseg_set_gdt (mmuseg_global_descriptor_table_t *mmu_gdt, mmuseg_global_descriptor_table_gdt_t *gdt, unsigned selector)
{
	if (selector == 0) {
		return (1);
	}
	if (selector > mmu_gdt->largest_gdt_in_use) {
		unsigned i;
		i = mmu_gdt->largest_gdt_in_use;
		mmu_gdt->largest_gdt_in_use = selector;
		mmu_gdt->gdt = (mmuseg_global_descriptor_table_gdt_t **)realloc(mmu_gdt->gdt, mmu_gdt->largest_gdt_in_use * sizeof(mmuseg_global_descriptor_table_gdt_t));
		while (i < mmu_gdt->largest_gdt_in_use) {
			mmu_gdt->gdt[i] = NULL;
			i += 1;
		}
	}
	mmu_gdt->gdt[selector - 1] = gdt;
	return (0);
}

int mmuseg_alloc_gdt (mmuseg_global_descriptor_table_t *mmu_gdt, unsigned long selector_table_index, unsigned *selector)
{
	unsigned  i;
	mmuseg_global_descriptor_table_gdt_t *gdt;
	*selector = selector_table_index;
	gdt = gdt_new (selector_table_index);
	mmuseg_set_gdt (mmu_gdt, gdt, i);
	return (0);
}

static
int mmuseg_free_gdt (mmuseg_global_descriptor_table_t *mmu_gdt, unsigned selector)
{
	if ((selector == 0) || (selector > mmu_gdt->largest_gdt_in_use)) {
		return (1);
	}
	gdt_del (mmu_gdt->gdt[selector - 1]);
	mmu_gdt->gdt[selector - 1] = NULL;
	return (0);
}
/* ----------------------------------- */
/*            Local   Descriptor Table */
/*            Local   Descriptor Table */
/*            Local   Descriptor Table */
/*            Local   Descriptor Table */
/*            Local   Descriptor Table */
/*            Local   Descriptor Table */
/*            Local   Descriptor Table */
/*            Local   Descriptor Table */
/*            Local   Descriptor Table */
/*            Local   Descriptor Table */
/* ----------------------------------- */
mmuseg_local_descriptor_table_ldt_t *ldt_new(unsigned long selector_table_index)
{
	mmuseg_local_descriptor_table_ldt_t *ldt;
	ldt = malloc(sizeof(mmuseg_local_descriptor_table_ldt_t));
	if (ldt == NULL) {
		return (NULL);
	}
	if (selector_table_index > 0) {
		ldt->selector = malloc(selector_table_index);
		if (ldt->selector == NULL) {
			free(ldt);
			return (NULL);
		}
	}
	else {
		ldt->selector = NULL;
	}
	ldt->selector_table_index = selector_table_index;
	ldt->present = 0;
	return (ldt);
}

void ldt_del(mmuseg_local_descriptor_table_ldt_t *ldt)
{
	if (ldt != NULL) {
		free(ldt->selector);
		free(ldt);
	}
}

mmuseg_local_descriptor_table_t *mmuseg_ldt_new(ldt_segment_t *seg)
{
	mmuseg_local_descriptor_table_t         *mmu_ldt;
	/* init->mmu = mmuseg_new(segment); */
	mmu_ldt = malloc(sizeof(mmuseg_local_descriptor_table_t));
	if (mmu_ldt == NULL) {
		return (NULL);
	}
	mmu_ldt->largest_ldt_in_use = 0;
	/* mmu->ldt = NULL; */
	return (mmu_ldt);
}

void mmuseg_ldt_del(mmuseg_local_descriptor_table_t *mmu_ldt)
{
	unsigned i;
	if (mmu_ldt != NULL) {
		for (i = 0; i <= mmu_ldt->largest_ldt_in_use; i++) {
			ldt_del(mmu_ldt->ldt[i]);
		}
	}
}

mmuseg_local_descriptor_table_ldt_t *mmuseg_get_ldt(mmuseg_local_descriptor_table_t *mmu_ldt, unsigned selector)
{
	if ((selector > 0) && (selector <= mmu_ldt->largest_ldt_in_use)) {
		return (mmu_ldt->ldt[selector - 1]);
	}
	return (NULL);
}

int mmuseg_set_ldt(mmuseg_local_descriptor_table_t *mmu_ldt, mmuseg_local_descriptor_table_ldt_t *ldt, unsigned selector)
{
	if (selector == 0) {
		return (1);
	}
	if (selector > mmu_ldt->largest_ldt_in_use) {
		unsigned i;
		i = mmu_ldt->largest_ldt_in_use;
		mmu_ldt->largest_ldt_in_use = selector;
		mmu_ldt->ldt = (mmuseg_local_descriptor_table_ldt_t **)realloc(mmu_ldt->ldt, mmu_ldt->largest_ldt_in_use * sizeof(mmuseg_local_descriptor_table_ldt_t));
		while (i < mmu_ldt->largest_ldt_in_use) {
			mmu_ldt->ldt[i] = NULL;
			i += 1;
		}
	}
	mmu_ldt->ldt[selector - 1] = ldt;
	return (0);
}

int mmuseg_alloc_ldt(mmuseg_local_descriptor_table_t *mmu_ldt, unsigned long selector_table_index, unsigned *selector)
{
	unsigned  i;
	mmuseg_local_descriptor_table_ldt_t *ldt;
	*selector = selector_table_index;
	ldt = ldt_new(selector_table_index);
	mmuseg_set_ldt(mmu_ldt, ldt, i);
	return (0);
}

static
int mmuseg_free_ldt(mmuseg_local_descriptor_table_t *mmu_ldt, unsigned selector)
{
	if ((selector == 0) || (selector > mmu_ldt->largest_ldt_in_use)) {
		return (1);
	}
	ldt_del(mmu_ldt->ldt[selector - 1]);
	mmu_ldt->ldt[selector - 1] = NULL;
	return (0);
}
/* ----------------------------------- */
/*          Interrupt Descriptor Table */
/*          Interrupt Descriptor Table */
/*          Interrupt Descriptor Table */
/*          Interrupt Descriptor Table */
/*          Interrupt Descriptor Table */
/*          Interrupt Descriptor Table */
/*          Interrupt Descriptor Table */
/*          Interrupt Descriptor Table */
/*          Interrupt Descriptor Table */
/*          Interrupt Descriptor Table */
/* ----------------------------------- */

mmuseg_interrupt_descriptor_table_idt_t *idt_new(unsigned long selector_table_index)
{
	mmuseg_interrupt_descriptor_table_idt_t *idt;
	idt = malloc(sizeof(mmuseg_interrupt_descriptor_table_idt_t));
	if (idt == NULL) {
		return (NULL);
	}
	if (selector_table_index > 0) {
		idt->selector = malloc(selector_table_index);
		if (idt->selector == NULL) {
			free(idt);
			return (NULL);
		}
	}
	else {
		idt->selector = NULL;
	}
	idt->selector_table_index = selector_table_index;
	idt->present = 0;
	return (idt);
}

void idt_del(mmuseg_interrupt_descriptor_table_idt_t *idt)
{
	if (idt != NULL) {
		free(idt->selector);
		free(idt);
	}
}

mmuseg_interrupt_descriptor_table_t *mmuseg_idt_new(idt_segment_t *seg)
{
	mmuseg_interrupt_descriptor_table_t         *mmu_idt;
	/* init->mmu = mmuseg_new(segment); */
	mmu_idt = malloc(sizeof(mmuseg_interrupt_descriptor_table_t));
	if (mmu_idt == NULL) {
		return (NULL);
	}
	mmu_idt->largest_idt_in_use = 0;
	/* mmu->idt = NULL; */
	return (mmu_idt);
}

void mmuseg_idt_del(mmuseg_interrupt_descriptor_table_t *mmu_idt)
{
	unsigned i;
	if (mmu_idt != NULL) {
		for (i = 0; i <= mmu_idt->largest_idt_in_use; i++) {
			idt_del(mmu_idt->idt[i]);
		}
	}
}

mmuseg_interrupt_descriptor_table_idt_t *mmuseg_get_idt(mmuseg_interrupt_descriptor_table_t *mmu_idt, unsigned selector)
{
	if ((selector > 0) && (selector <= mmu_idt->largest_idt_in_use)) {
		return (mmu_idt->idt[selector - 1]);
	}
	return (NULL);
}

int mmuseg_set_idt(mmuseg_interrupt_descriptor_table_t *mmu_idt, mmuseg_interrupt_descriptor_table_idt_t *idt, unsigned selector)
{
	if (selector == 0) {
		return (1);
	}
	if (selector > mmu_idt->largest_idt_in_use) {
		unsigned i;
		i = mmu_idt->largest_idt_in_use;
		mmu_idt->largest_idt_in_use = selector;
		mmu_idt->idt = (mmuseg_interrupt_descriptor_table_idt_t **)realloc(mmu_idt->idt, mmu_idt->largest_idt_in_use * sizeof(mmuseg_interrupt_descriptor_table_idt_t));
		while (i < mmu_idt->largest_idt_in_use) {
			mmu_idt->idt[i] = NULL;
			i += 1;
		}
	}
	mmu_idt->idt[selector - 1] = idt;
	return (0);
}

int mmuseg_alloc_idt(mmuseg_interrupt_descriptor_table_t *mmu_idt, unsigned long selector_table_index, unsigned *selector)
{
	unsigned  i;
	mmuseg_interrupt_descriptor_table_idt_t *idt;
	*selector = selector_table_index;
	idt = idt_new(selector_table_index);
	mmuseg_set_idt(mmu_idt, idt, i);
	return (0);
}

static
int mmuseg_free_idt(mmuseg_interrupt_descriptor_table_t *mmu_idt, unsigned selector)
{
	if ((selector == 0) || (selector > mmu_idt->largest_idt_in_use)) {
		return (1);
	}
	idt_del(mmu_idt->idt[selector - 1]);
	mmu_idt->idt[selector - 1] = NULL;
	return (0);
}

mmuseg_trap_descriptor_table_trap_t *trap_new(unsigned long selector_table_index)
{
	mmuseg_trap_descriptor_table_trap_t *trap;
	trap = malloc(sizeof(mmuseg_trap_descriptor_table_trap_t));
	if (trap == NULL) {
		return (NULL);
	}
	if (selector_table_index > 0) {
		trap->selector = malloc(selector_table_index);
		if (trap->selector == NULL) {
			free(trap);
			return (NULL);
		}
	}
	else {
		trap->selector = NULL;
	}
	trap->selector_table_index = selector_table_index;
	trap->present = 0;
	return (trap);
}

void trap_del(mmuseg_trap_descriptor_table_trap_t *trap)
{
	if (trap != NULL) {
		free(trap->selector);
		free(trap);
	}
}

mmuseg_trap_descriptor_table_t *mmuseg_trap_new(trap_segment_t *seg)
{
	mmuseg_trap_descriptor_table_t         *mmu_trap;
	/* init->mmu = mmuseg_new(segment); */
	mmu_trap = malloc(sizeof(mmuseg_trap_descriptor_table_t));
	if (mmu_trap == NULL) {
		return (NULL);
	}
	mmu_trap->largest_trap_in_use = 0;
	/* mmu->trap = NULL; */
	return (mmu_trap);
}

void mmuseg_trap_del(mmuseg_trap_descriptor_table_t *mmu_trap)
{
	unsigned i;
	if (mmu_trap != NULL) {
		for (i = 0; i <= mmu_trap->largest_trap_in_use; i++) {
			trap_del(mmu_trap->trap[i]);
		}
	}
}

mmuseg_trap_descriptor_table_trap_t *mmuseg_get_trap(mmuseg_trap_descriptor_table_t *mmu_trap, unsigned selector)
{
	if ((selector > 0) && (selector <= mmu_trap->largest_trap_in_use)) {
		return (mmu_trap->trap[selector - 1]);
	}
	return (NULL);
}

int mmuseg_set_trap(mmuseg_trap_descriptor_table_t *mmu_trap, mmuseg_trap_descriptor_table_trap_t *trap, unsigned selector)
{
	if (selector == 0) {
		return (1);
	}
	if (selector > mmu_trap->largest_trap_in_use) {
		unsigned i;
		i = mmu_trap->largest_trap_in_use;
		mmu_trap->largest_trap_in_use = selector;
		mmu_trap->trap = (mmuseg_trap_descriptor_table_trap_t **)realloc(mmu_trap->trap, mmu_trap->largest_trap_in_use * sizeof(mmuseg_trap_descriptor_table_trap_t));
		while (i < mmu_trap->largest_trap_in_use) {
			mmu_trap->trap[i] = NULL;
			i += 1;
		}
	}
	mmu_trap->trap[selector - 1] = trap;
	return (0);
}

int mmuseg_alloc_trap(mmuseg_trap_descriptor_table_t *mmu_trap, unsigned long selector_table_index, unsigned *selector)
{
	unsigned  i;
	mmuseg_trap_descriptor_table_trap_t *trap;
	*selector = selector_table_index;
	trap = trap_new(selector_table_index);
	mmuseg_set_trap(mmu_trap, trap, i);
	return (0);
}

static
int mmuseg_free_trap(mmuseg_trap_descriptor_table_t *mmu_trap, unsigned selector)
{
	if ((selector == 0) || (selector > mmu_trap->largest_trap_in_use)) {
		return (1);
	}
	trap_del(mmu_trap->trap[selector - 1]);
	mmu_trap->trap[selector - 1] = NULL;
	return (0);
}

mmuseg_task_descriptor_table_task_t *task_new(unsigned long selector_table_index)
{
	mmuseg_task_descriptor_table_task_t *task;
	task = malloc(sizeof(mmuseg_task_descriptor_table_task_t));
	if (task == NULL) {
		return (NULL);
	}
	if (selector_table_index > 0) {
		task->selector = malloc(selector_table_index);
		if (task->selector == NULL) {
			free(task);
			return (NULL);
		}
	}
	else {
		task->selector = NULL;
	}
	task->selector_table_index = selector_table_index;
	task->present = 0;
	return (task);
}

void task_del(mmuseg_task_descriptor_table_task_t *task)
{
	if (task != NULL) {
		free(task->selector);
		free(task);
	}
}

mmuseg_task_descriptor_table_t *mmuseg_task_new(task_segment_t *seg)
{
	mmuseg_task_descriptor_table_t         *mmu_task;
	/* init->mmu = mmuseg_new(segment); */
	mmu_task = malloc(sizeof(mmuseg_task_descriptor_table_t));
	if (mmu_task == NULL) {
		return (NULL);
	}
	mmu_task->largest_task_in_use = 0;
	/* mmu->task = NULL; */
	return (mmu_task);
}

void mmuseg_task_del(mmuseg_task_descriptor_table_t *mmu_task)
{
	unsigned i;
	if (mmu_task != NULL) {
		for (i = 0; i <= mmu_task->largest_task_in_use; i++) {
			task_del(mmu_task->task[i]);
		}
	}
}

mmuseg_task_descriptor_table_task_t *mmuseg_get_task(mmuseg_task_descriptor_table_t *mmu_task, unsigned selector)
{
	if ((selector > 0) && (selector <= mmu_task->largest_task_in_use)) {
		return (mmu_task->task[selector - 1]);
	}
	return (NULL);
}

int mmuseg_set_task(mmuseg_task_descriptor_table_t *mmu_task, mmuseg_task_descriptor_table_task_t *task, unsigned selector)
{
	if (selector == 0) {
		return (1);
	}
	if (selector > mmu_task->largest_task_in_use) {
		unsigned i;
		i = mmu_task->largest_task_in_use;
		mmu_task->largest_task_in_use = selector;
		mmu_task->task = (mmuseg_task_descriptor_table_task_t **)realloc(mmu_task->task, mmu_task->largest_task_in_use * sizeof(mmuseg_task_descriptor_table_task_t));
		while (i < mmu_task->largest_task_in_use) {
			mmu_task->task[i] = NULL;
			i += 1;
		}
	}
	mmu_task->task[selector - 1] = task;
	return (0);
}

int mmuseg_alloc_task(mmuseg_task_descriptor_table_t *mmu_task, unsigned long selector_table_index, unsigned *selector)
{
	unsigned  i;
	mmuseg_task_descriptor_table_task_t *task;
	*selector = selector_table_index;
	task = task_new(selector_table_index);
	mmuseg_set_task(mmu_task, task, i);
	return (0);
}

static
int mmuseg_free_task(mmuseg_task_descriptor_table_t *mmu_task, unsigned selector)
{
	if ((selector == 0) || (selector > mmu_task->largest_task_in_use)) {
		return (1);
	}
	task_del(mmu_task->task[selector - 1]);
	mmu_task->task[selector - 1] = NULL;
	return (0);
}

/* ----------------------------------- */
/* Task State Segment Descriptor Table */
/* Task State Segment Descriptor Table */
/* Task State Segment Descriptor Table */
/* Task State Segment Descriptor Table */
/* Task State Segment Descriptor Table */
/* Task State Segment Descriptor Table */
/* Task State Segment Descriptor Table */
/* Task State Segment Descriptor Table */
/* Task State Segment Descriptor Table */
/* Task State Segment Descriptor Table */
/* ----------------------------------- */
mmuseg_task_state_segment_descriptor_table_tss_t *tss_new(unsigned long selector_table_index)
{
	mmuseg_task_state_segment_descriptor_table_tss_t *tss;
	tss = malloc(sizeof(mmuseg_task_state_segment_descriptor_table_tss_t));
	if (tss == NULL) {
		return (NULL);
	}
	if (selector_table_index > 0) {
		tss->selector = malloc(selector_table_index);
		if (tss->selector == NULL) {
			free(tss);
			return (NULL);
		}
	}
	else {
		tss->selector = NULL;
	}
	tss->selector_table_index = selector_table_index;
	tss->present = 0;
	return (tss);
}

void tss_del(mmuseg_task_state_segment_descriptor_table_tss_t *tss)
{
	if (tss != NULL) {
		free(tss->selector);
		free(tss);
	}
}

mmuseg_task_state_segment_descriptor_table_t *mmuseg_tss_new(tss_segment_t *seg)
{
	mmuseg_task_state_segment_descriptor_table_t         *mmu_tss;
	/* init->mmu = mmuseg_new(segment); */
	mmu_tss = malloc(sizeof(mmuseg_task_state_segment_descriptor_table_t));
	if (mmu_tss == NULL) {
		return (NULL);
	}
	mmu_tss->largest_tss_in_use = 0;
	/* mmu->idt = NULL; */
	return (mmu_tss);
}

void mmuseg_tss_del(mmuseg_task_state_segment_descriptor_table_t *mmu_tss)
{
	unsigned i;
	if (mmu_tss != NULL) {
		for (i = 0; i <= mmu_tss->largest_tss_in_use; i++) {
			tss_del(mmu_tss->tss[i]);
		}
	}
}

mmuseg_task_state_segment_descriptor_table_tss_t *mmuseg_get_tss(mmuseg_task_state_segment_descriptor_table_t *mmu_tss, unsigned selector)
{
	if ((selector > 0) && (selector <= mmu_tss->largest_tss_in_use)) {
		return (mmu_tss->tss[selector - 1]);
	}
	return (NULL);
}

int mmuseg_set_tss(mmuseg_task_state_segment_descriptor_table_t *mmu_tss, mmuseg_task_state_segment_descriptor_table_tss_t *tss, unsigned selector)
{
	if (selector == 0) {
		return (1);
	}
	if (selector > mmu_tss->largest_tss_in_use) {
		unsigned i;
		i = mmu_tss->largest_tss_in_use;
		mmu_tss->largest_tss_in_use = selector;
		mmu_tss->tss = (mmuseg_task_state_segment_descriptor_table_tss_t **)realloc(mmu_tss->tss, mmu_tss->largest_tss_in_use * sizeof(mmuseg_task_state_segment_descriptor_table_tss_t));
		while (i < mmu_tss->largest_tss_in_use) {
			mmu_tss->tss[i] = NULL;
			i += 1;
		}
	}
	mmu_tss->tss[selector - 1] = tss;
	return (0);
}

int mmuseg_alloc_tss(mmuseg_task_state_segment_descriptor_table_t *mmu_tss, unsigned long selector_table_index, unsigned *selector)
{
	unsigned  i;
	mmuseg_task_state_segment_descriptor_table_tss_t *tss;
	*selector = selector_table_index;
	tss = tss_new(selector_table_index);
	mmuseg_set_tss(mmu_tss, tss, i);
	return (0);
}

static
int mmuseg_free_tss(mmuseg_task_state_segment_descriptor_table_t *mmu_tss, unsigned selector)
{
	if ((selector == 0) || (selector > mmu_tss->largest_tss_in_use)) {
		return (1);
	}
	tss_del(mmu_tss->tss[selector - 1]);
	mmu_tss->tss[selector - 1] = NULL;
	return (0);
}
/* ----------------------------------- */
/* Directory Page Table Entries        */
/* Directory Page Table Entries        */
/* Directory Page Table Entries        */
/* Directory Page Table Entries        */
/* Directory Page Table Entries        */
/* Directory Page Table Entries        */
/* Directory Page Table Entries        */
/* Directory Page Table Entries        */
/* Directory Page Table Entries        */
/* Directory Page Table Entries        */
/* ----------------------------------- */
mmuseg_directory_page_table_page_t *directory_page_new(unsigned long directory_page_table_address)
{
	mmuseg_directory_page_table_page_t *directory_page;
	directory_page = malloc(sizeof(mmuseg_directory_page_table_page_t));
	if (directory_page == NULL) {
		return (NULL);
	}
	if (directory_page_table_address > 0) {
		directory_page->directory_page_table = malloc(directory_page_table_address);
		if (directory_page->directory_page_table == NULL) {
			free(directory_page);
			return (NULL);
		}
	}
	else {
		directory_page->directory_page_table = NULL;
	}
	directory_page->directory_page_table_address = directory_page_table_address;
	directory_page->present = 0;
	return (directory_page);
}

void directory_page_del(mmuseg_directory_page_table_page_t *directory_page)
{
	if (directory_page != NULL) {
		free(directory_page->directory_page_table);
		free(directory_page);
	}
}

mmuseg_directory_page_table_page_t *mmuseg_get_directory_page(mmuseg_directory_page_table_t *mmu_directory_page, unsigned selector)
{
	if ((selector > 0) && (selector <= mmu_directory_page->largest_directory_page_in_use)) {
		return (mmu_directory_page->directory_page[selector - 1]);
	}
	return (NULL);
}

int mmuseg_set_directory_page(mmuseg_directory_page_table_t *mmu_directory_page, mmuseg_directory_page_table_page_t *directory_page, unsigned selector)
{
	if (selector == 0) {
		return (1);
	}
	if (selector > mmu_directory_page->largest_directory_page_in_use) {
		unsigned i;
		i = mmu_directory_page->largest_directory_page_in_use;
		mmu_directory_page->largest_directory_page_in_use = selector;
		mmu_directory_page->directory_page = (mmuseg_directory_page_table_page_t **)realloc(mmu_directory_page->directory_page, mmu_directory_page->largest_directory_page_in_use * sizeof(mmuseg_directory_page_table_page_t));
		while (i < mmu_directory_page->largest_directory_page_in_use) {
			mmu_directory_page->directory_page[i] = NULL;
			i += 1;
		}
	}
	mmu_directory_page->directory_page[selector - 1] = directory_page;
	return (0);
}

int mmuseg_alloc_directory_page(mmuseg_directory_page_table_t *mmu_directory_page, unsigned long directory_page_table_address, unsigned *selector)
{
	unsigned  i;
	mmuseg_directory_page_table_page_t *directory_page;
	*selector = directory_page_table_address;
	directory_page = directory_page_new(directory_page_table_address);
	mmuseg_set_directory_page(mmu_directory_page, directory_page, i);
	return (0);
}

static
int mmuseg_free_directory_page(mmuseg_directory_page_table_t *mmu_directory_page, unsigned selector)
{
	if ((selector == 0) || (selector > mmu_directory_page->largest_directory_page_in_use)) {
		return (1);
	}
	directory_page_del(mmu_directory_page->directory_page[selector - 1]);
	mmu_directory_page->directory_page[selector - 1] = NULL;
	return (0);
}
/* ----------------------------------- */
/* Directory Page Table Entries        */
/* Directory Page Table Entries        */
/* Directory Page Table Entries        */
/* Directory Page Table Entries        */
/* Directory Page Table Entries        */
/* Directory Page Table Entries        */
/* Directory Page Table Entries        */
/* Directory Page Table Entries        */
/* Directory Page Table Entries        */
/* Directory Page Table Entries        */
/* ----------------------------------- */
mmuseg_entry_page_table_page_t *entry_page_new(unsigned long entry_page_table_address)
{
	mmuseg_entry_page_table_page_t *entry_page;
	entry_page = malloc(sizeof(mmuseg_entry_page_table_page_t));
	if (entry_page == NULL) {
		return (NULL);
	}
	if (entry_page_table_address > 0) {
		entry_page->entry_page_table = malloc(entry_page_table_address);
		if (entry_page->entry_page_table == NULL) {
			free(entry_page);
			return (NULL);
		}
	}
	else {
		entry_page->entry_page_table = NULL;
	}
	entry_page->entry_page_table_address = entry_page_table_address;
	entry_page->present = 0;
	return (entry_page);
}

void entry_page_del(mmuseg_entry_page_table_page_t *entry_page)
{
	if (entry_page != NULL) {
		free(entry_page->entry_page_table);
		free(entry_page);
	}
}

mmuseg_entry_page_table_page_t *mmuseg_get_entry_page(mmuseg_entry_page_table_t *mmu_entry_page, unsigned selector)
{
	if ((selector > 0) && (selector <= mmu_entry_page->largest_entry_page_in_use)) {
		return (mmu_entry_page->entry_page[selector - 1]);
	}
	return (NULL);
}

int mmuseg_set_entry_page(mmuseg_entry_page_table_t *mmu_entry_page, mmuseg_entry_page_table_page_t *entry_page, unsigned selector)
{
	if (selector == 0) {
		return (1);
	}
	if (selector > mmu_entry_page->largest_entry_page_in_use) {
		unsigned i;
		i = mmu_entry_page->largest_entry_page_in_use;
		mmu_entry_page->largest_entry_page_in_use = selector;
		mmu_entry_page->entry_page = (mmuseg_entry_page_table_page_t **)realloc(mmu_entry_page->entry_page, mmu_entry_page->largest_entry_page_in_use * sizeof(mmuseg_entry_page_table_page_t));
		while (i < mmu_entry_page->largest_entry_page_in_use) {
			mmu_entry_page->entry_page[i] = NULL;
			i += 1;
		}
	}
	mmu_entry_page->entry_page[selector - 1] = entry_page;
	return (0);
}

int mmuseg_alloc_entry_page(mmuseg_entry_page_table_t *mmu_entry_page, unsigned long entry_page_table_address, unsigned *selector)
{
	unsigned  i;
	mmuseg_entry_page_table_page_t *entry_page;
	*selector = entry_page_table_address;
	entry_page = entry_page_new(entry_page_table_address);
	mmuseg_set_entry_page(mmu_entry_page, entry_page, i);
	return (0);
}

static
int mmuseg_free_entry_page(mmuseg_entry_page_table_t *mmu_entry_page, unsigned selector)
{
	if ((selector == 0) || (selector > mmu_entry_page->largest_entry_page_in_use)) {
		return (1);
	}
	entry_page_del(mmu_entry_page->entry_page[selector - 1]);
	mmu_entry_page->entry_page[selector - 1] = NULL;
	return (0);
}


/* 00: get driver version */
/* void xms_00 (xms_t *xms, e8086_t *cpu)
{
	e86_set_ax (cpu, 0x0300);
	e86_set_bx (cpu, 0x0300);
	e86_set_dx (cpu, (xms->hma != NULL));
}

/* 01: request high memory area */
/* void xms_01 (xms_t *xms, e8086_t *cpu)
{
	if (xms->hma == NULL) {
		e86_set_ax (cpu, 0x0000);
		e86_set_bl (cpu, 0x90);
		return;
	}

	if (xms->hma_alloc) {
		e86_set_ax (cpu, 0x0000);
		e86_set_bl (cpu, 0x91);
		return;
	}

	xms->hma_alloc = 1;

	e86_set_ax (cpu, 0x0001);
	e86_set_bl (cpu, 0x00);
}

/* 02: release high memory area */
/* void xms_02 (xms_t *xms, e8086_t *cpu)
{
	if (xms->hma == NULL) {
		e86_set_ax (cpu, 0x0000);
		e86_set_bl (cpu, 0x90);
		return;
	}

	if (xms->hma_alloc == 0) {
		e86_set_ax (cpu, 0x0000);
		e86_set_bl (cpu, 0x93);
		return;
	}

	xms->hma_alloc = 0;

	e86_set_ax (cpu, 0x0001);
	e86_set_bl (cpu, 0x00);
}

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

/* 08: query free extended memory */
/* void xms_08 (xms_t *xms, e8086_t *cpu)
{
	e86_set_ax (cpu, (xms->emb_max - xms->emb_used) / 1024);
	e86_set_dx (cpu, (xms->emb_max - xms->emb_used) / 1024);
	e86_set_bl (cpu, 0x00);
}

/* 09: allocate extended memory block */
/* void xms_09 (xms_t *xms, e8086_t *cpu)
{
	unsigned      handle;
	unsigned long size;

	size = 1024UL * e86_get_dx (cpu);

	if (size > (xms->emb_max - xms->emb_used)) {
		e86_set_ax (cpu, 0x0000);
		e86_set_bl (cpu, 0xa0);
		return;
	}

	if (xms_alloc_emb (xms, size, &handle)) {
		e86_set_ax (cpu, 0x0000);
		e86_set_bl (cpu, 0xa1);
		return;
	}

	e86_set_ax (cpu, 0x0001);
	e86_set_dx (cpu, handle);
	e86_set_bl (cpu, 0x00);
}

/* 0A: free extended memory block */
/* void xms_0a (xms_t *xms, e8086_t *cpu)
{
	unsigned handle;

	handle = e86_get_dx (cpu);

	if (xms_free_emb (xms, handle)) {
		e86_set_ax (cpu, 0x0000);
		e86_set_bl (cpu, 0xa2);
		return;
	}

	e86_set_ax (cpu, 0x0001);
	e86_set_bl (cpu, 0x00);
}

static
void inc_addr (unsigned short *seg, unsigned short *ofs, unsigned short val)
{
	*ofs += val;

	*seg += *ofs >> 4;
	*ofs &= 0x000f;
}

/* 0B: move extended memory */
/* void xms_0b (xms_t *xms, e8086_t *cpu)
{
	unsigned long  i;
	unsigned short seg1, ofs1, seg2, ofs2;
	xms_emb_t      *src, *dst;
	unsigned long  cnt;
	unsigned       srch, dsth;
	unsigned long  srca, dsta;

	seg1 = e86_get_ds (cpu);
	ofs1 = e86_get_si (cpu);

	cnt = e86_get_mem16 (cpu, seg1, ofs1);
	cnt += e86_get_mem16 (cpu, seg1, ofs1 + 2) << 16;
	srch = e86_get_mem16 (cpu, seg1, ofs1 + 4);
	srca = e86_get_mem16 (cpu, seg1, ofs1 + 6);
	srca += e86_get_mem16 (cpu, seg1, ofs1 + 8) << 16;
	dsth = e86_get_mem16 (cpu, seg1, ofs1 + 10);
	dsta = e86_get_mem16 (cpu, seg1, ofs1 + 12);
	dsta += e86_get_mem16 (cpu, seg1, ofs1 + 14) << 16;

	if ((srch == 0) && (dsth == 0)) {
		seg1 = (srca >> 16) & 0xffff;
		ofs1 = srca & 0xffff;
		seg2 = (dsta >> 16) & 0xffff;
		ofs2 = dsta & 0xffff;

		for (i = 0; i < cnt; i++) {
			e86_set_mem8 (cpu, seg2, ofs2, e86_get_mem8 (cpu, seg1, ofs1));
			inc_addr (&seg1, &ofs1, 1);
			inc_addr (&seg2, &ofs2, 1);
		}
	}
	else if ((srch != 0) && (dsth == 0)) {
		src = xms_get_emb (xms, srch);

		if (src == NULL) {
			e86_set_ax (cpu, 0x0000);
			e86_set_bl (cpu, 0xa3);
			return;
		}

		if (srca >= src->size) {
			e86_set_ax (cpu, 0x0000);
			e86_set_bl (cpu, 0xa4);
			return;
		}

		if ((srca + cnt) > src->size) {
			e86_set_ax (cpu, 0x0000);
			e86_set_bl (cpu, 0xa7);
		}

		seg2 = (dsta >> 16) & 0xffff;
		ofs2 = dsta & 0xffff;

		for (i = 0; i < cnt; i++) {
			e86_set_mem8 (cpu, seg2, ofs2, src->data[srca + i]);
			inc_addr (&seg2, &ofs2, 1);
		}
	}
	else if ((srch == 0) && (dsth != 0)) {
		dst = xms_get_emb (xms, dsth);

		if (dst == NULL) {
			e86_set_ax (cpu, 0x0000);
			e86_set_bl (cpu, 0xa5);
			return;
		}

		if (dsta >= dst->size) {
			e86_set_ax (cpu, 0x0000);
			e86_set_bl (cpu, 0xa6);
			return;
		}

		if ((dsta + cnt) > dst->size) {
			e86_set_ax (cpu, 0x0000);
			e86_set_bl (cpu, 0xa7);
			return;
		}

		seg1 = (srca >> 16) & 0xffff;
		ofs1 = srca & 0xffff;

		for (i = 0; i < cnt; i++) {
			dst->data[dsta + i] = e86_get_mem8 (cpu, seg1, ofs1);
			inc_addr (&seg1, &ofs1, 1);
		}
	}
	else {
		src = xms_get_emb (xms, srch);

		if (src == NULL) {
			e86_set_ax (cpu, 0x0000);
			e86_set_bl (cpu, 0xa3);
			return;
		}

		if (srca >= src->size) {
			e86_set_ax (cpu, 0x0000);
			e86_set_bl (cpu, 0xa4);
			return;
		}

		if ((srca + cnt) > src->size) {
			e86_set_ax (cpu, 0x0000);
			e86_set_bl (cpu, 0xa7);
		}

		dst = xms_get_emb (xms, dsth);

		if (dst == NULL) {
			e86_set_ax (cpu, 0x0000);
			e86_set_bl (cpu, 0xa5);
			return;
		}

		if (dsta >= dst->size) {
			e86_set_ax (cpu, 0x0000);
			e86_set_bl (cpu, 0xa6);
			return;
		}

		if ((dsta + cnt) > dst->size) {
			e86_set_ax (cpu, 0x0000);
			e86_set_bl (cpu, 0xa7);
		}

		memcpy (dst->data, src->data, cnt);
	}

	e86_set_ax (cpu, 0x001);
	e86_set_bl (cpu, 0x00);
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

/* 10: request umb */
/* void xms_10 (xms_t *xms, e8086_t *cpu)
{
	unsigned       idx;
	unsigned short size;

	size = e86_get_dx (cpu);

	if (xms_alloc_umb (xms, size, &idx)) {
		unsigned short max;

		e86_set_ax (cpu, 0x0000);
		max = umb_get_max (xms);
		e86_set_dx (cpu, max);
		e86_set_bl (cpu, (max == 0) ? 0xb1 : 0xb0);

		return;
	}

	e86_set_ax (cpu, 0x0001);
	e86_set_dx (cpu, xms->umb[idx].size);
	e86_set_bx (cpu, xms->umb[idx].segm);
}

/* 11: release umb */
/* void xms_11 (xms_t *xms, e8086_t *cpu)
{
	unsigned short segm;

	segm = e86_get_dx (cpu);

	if (xms_free_umb (xms, segm)) {
		e86_set_ax (cpu, 0x0001);
		e86_set_bl (cpu, 0xb2);
	}

	e86_set_ax (cpu, 0x0001);
}

void xms_info (xms_t *xms, e8086_t *cpu)
{
	unsigned short flags;

	flags = 0;

	if (xms != NULL) {
		flags = 0x0001;

		if (xms->hma != NULL) {
			flags |= 0x0002;
		}

		e86_set_cx (cpu, xms->umb_size);
		e86_set_dx (cpu, xms->emb_max / 1024UL);
	}

	e86_set_ax (cpu, flags);
}

void xms_handler (xms_t *xms, e8086_t *cpu)
{
	if (xms == NULL) {
		e86_set_ax (cpu, 0x0000);
		e86_set_bl (cpu, 0x80);
	}

	switch (e86_get_ah (cpu)) {
	case 0x00:
		xms_00 (xms, cpu);
		break;

	case 0x01:
		xms_01 (xms, cpu);
		break;

	case 0x02:
		xms_02 (xms, cpu);
		break;

	case 0x03:
		xms_03 (xms, cpu);
		break;

	case 0x04:
		xms_04 (xms, cpu);
		break;

	case 0x05:
		xms_05 (xms, cpu);
		break;

	case 0x06:
		xms_06 (xms, cpu);
		break;

	case 0x07:
		xms_07 (xms, cpu);
		break;

	case 0x08:
		xms_08 (xms, cpu);
		break;

	case 0x09:
		xms_09 (xms, cpu);
		break;

	case 0x0a:
		xms_0a (xms, cpu);
		break;

	case 0x0b:
		xms_0b (xms, cpu);
		break;

	case 0x0c:
		xms_0c (xms, cpu);
		break;

	case 0x0d:
		xms_0d (xms, cpu);
		break;

	case 0x0e:
		xms_0e (xms, cpu);
		break;

	case 0x0f:
		xms_0f (xms, cpu);
		break;

	case 0x10:
		xms_10 (xms, cpu);
		break;

	case 0x11:
		xms_11 (xms, cpu);
		break;

	default:
		pce_log (MSG_DEB, "xms: unknown function (%x)\n",
			e86_get_ah (cpu)
		);
		e86_set_ax (cpu, 0x0000);
		e86_set_bl (cpu, 0x80);
		break;
	}
} */
