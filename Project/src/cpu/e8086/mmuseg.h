/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/ibmpc/xms.h                                         *
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


#ifndef PCE_MMUSEG_H
#define PCE_MMUSEG_H 1


#include <cpu/e8086/e8086.h>


#define PCE_GDT_MAX  8192
#define PCE_LDT_MAX  8192
#define PCE_PAGE_MAX  256 


typedef struct {
	unsigned      selector_table_indicator;        /* Segment table indicator */
	unsigned long selector_table_index;
	unsigned      selector_requester_privilege_level;        /* Segment requester privilege level */
	unsigned      present;
	unsigned char *selector;
} mmuseg_global_descriptor_table_gdt_t;


typedef struct {
	unsigned       largest_gdt_in_use;
	mmuseg_global_descriptor_table_gdt_t      **gdt;
	/* unsigned long  gdt_used; */
	/* unsigned long  gdt_max; */
} mmuseg_global_descriptor_table_t;

typedef struct gdt_segment_s {
	struct gdt_segment_s *x_gdt;
	unsigned long  gdt_segment_base_address_offset;            /* Segment base:  16-bit (80286)/32-bit (80386 and higher) */
	unsigned long  gdt_segment_limit;
	unsigned       gdt_granularity_bit; /* Segment granularity bit: */
	unsigned       gdt_big;             /* Big: "0" or "1" */
	unsigned       gdt_default;           /* Default: "0" or "1" */
	unsigned       gdt_x_;              /* X: "0" or "1" */
	unsigned       gdt_0_;              /* 0: "0" or "1" */
	unsigned       gdt_segment_available;       /* Segment AVL (available for use by systems programmers): "0" or "1" */
	unsigned       gdt_descriptor_privilege_level; /* Descriptor privilege level: "1", "2", or "3" */
	unsigned       gdt_type_of_segment_or_gate;          /* Type of Segment or Gate: */
	unsigned       gdt_type_1;          /* Segment type 1: "0" or "1": */
	unsigned       gdt_type_0;          /* Segment type 0: "0" or "1": */
	unsigned       gdt_type_c;          /* Segment type conforming: "0" for conforming or "1" for non-conforming: */
	unsigned       gdt_type_e;          /* Segment type expand-down: "0" or "1" */
	unsigned       gdt_type_r;          /* Segment type readable: "0" or "1" */
	unsigned       gdt_type_w;          /* Segment type writable: "0" or "1" */
	unsigned       gdt_segment_accessed;        /* Segment A (accessed): "0" or "1" */
	unsigned long  gdt_callgate_offset;        /* Segment 80386 call gate offset */
	unsigned       gdt_callgate_is_present;        /* Segment call gate present: "1" if present and "0" if not present */
	unsigned       gdt_callgate_type;        /* See "0110" for 80386 call gate */
	unsigned       gdt_callgate_000;        /* */
	unsigned       gdt_callgate_dword_count;        /* Segment call gate DWORD count (32-bit) */
} gdt_segment_t;

typedef struct {
	unsigned      selector_table_indicator;        /* Segment table indicator */
	unsigned long selector_table_index;
	unsigned      selector_requester_privilege_level;        /* Segment requester privilege level */
	unsigned      present;
	unsigned char *selector;
} mmuseg_local_descriptor_table_ldt_t;


typedef struct {
	unsigned       largest_ldt_in_use;
	mmuseg_local_descriptor_table_ldt_t      **ldt;
	/* unsigned long  gdt_used; */
	/* unsigned long  gdt_max; */
} mmuseg_local_descriptor_table_t;

typedef struct ldt_segment_s {
	struct ldt_segment_s *x_ldt;
	unsigned long  ldt_segment_base_address_offset;            /* Segment base:  16-bit (80286)/32-bit (80386 and higher) */
	unsigned long  ldt_segment_limit;
	unsigned       ldt_granularity_bit; /* Segment granularity bit: */
	unsigned       ldt_big;             /* Big: "0" or "1" */
	unsigned       ldt_default;           /* Default: "0" or "1" */
	unsigned       ldt_x_;              /* X: "0" or "1" */
	unsigned       ldt_0_;              /* 0: "0" or "1" */
	unsigned       ldt_segment_available;       /* Segment AVL (available for use by systems programmers): "0" or "1" */
	unsigned       ldt_descriptor_privilege_level; /* Descriptor privilege level: "1", "2", or "3" */
	unsigned       ldt_type_of_segment_or_gate;          /* Type of Segment or Gate: */
	unsigned       ldt_type_1;          /* Segment type 1: "0" or "1": */
	unsigned       ldt_type_0;          /* Segment type 0: "0" or "1": */
	unsigned       ldt_type_c;          /* Segment type conforming: "0" for conforming or "1" for non-conforming: */
	unsigned       ldt_type_e;          /* Segment type expand-down: "0" or "1" */
	unsigned       ldt_type_r;          /* Segment type readable: "0" or "1" */
	unsigned       ldt_type_w;          /* Segment type writable: "0" or "1" */
	unsigned       ldt_segment_accessed;        /* Segment A (accessed): "0" or "1" */
	unsigned long  ldt_callgate_offset;        /* Segment 80386 call gate offset */
	unsigned       ldt_callgate_is_present;        /* Segment call gate present: "1" if present and "0" if not present */
	unsigned       ldt_callgate_type;        /* See "0110" for 80386 call gate */
	unsigned       ldt_callgate_000;        /* */
	unsigned       ldt_callgate_dword_count;        /* Segment call gate DWORD count (32-bit) */
} ldt_segment_t;

typedef struct {
	unsigned      idt_gate_for_interrupt_selector_table_indicator;          /* Present: */
	unsigned long selector_table_index;
	unsigned      idt_gate_for_interrupt_selector_requester_privilege_level;          /* Descriptor Privilege Level: */
	unsigned      present;
	unsigned char *selector;
} mmuseg_interrupt_descriptor_table_idt_t;


typedef struct {
	unsigned       largest_idt_in_use;
	mmuseg_interrupt_descriptor_table_idt_t      **idt;
	/* unsigned long  gdt_used; */
	/* unsigned long  gdt_max; */
} mmuseg_interrupt_descriptor_table_t;

typedef struct idt_segment_s {
	struct idt_segment_s *x_idt;
	unsigned long  idt_gate_for_interrupt_selector;            /* Interrupt descriptor base:  16-bit (80286)/32-bit (80386 and higher) */
	unsigned long  idt_gate_for_interrupt_offset;           /* Interrupt descriptor limit: 16-bit (80286)/32-bit (80386 and higher) */
	unsigned long  idt_segment_limit;
	unsigned       idt_gate_for_interrupt_x_;            /* Segment type 1: "0" or "1": */
	unsigned       idt_gate_for_interrupt_0_;            /* Segment type 1: "0" or "1": */
	unsigned       idt_gate_for_interrupt_type;            /* Segment type 1: "0" or "1": */
	unsigned       idt_gate_for_interrupt_type_b;            /* Segment type 1: "0" or "1": */
	unsigned       idt_gate_for_interrupt_default;            /* Segment type 1: "0" or "1": */
	unsigned       idt_gate_for_interrupt_type_1;          /* Segment type 1: "0" or "1": */
	unsigned       idt_gate_for_interrupt_type_0;          /* Segment type 0: "0" or "1": */
	unsigned       idt_gate_for_interrupt_type_c;          /* Segment type conforming: "0" for conforming or "1" for non-conforming: */
	unsigned       idt_gate_for_interrupt_type_e;          /* Segment type expand-down: "0" or "1" */
	unsigned       idt_gate_for_interrupt_type_r;          /* Segment type readable: "0" or "1" */
	unsigned       idt_gate_for_interrupt_type_w;          /* Segment type writable: "0" or "1" */
														   /* NOTE: Change "idt_limit" back to "size" if necessary and required by source code */
														   /* unsigned char  alloc; */
	unsigned       idt_gate_for_interrupt_descriptor_privilege_level;          /* Descriptor Privilege Level: */
	unsigned       idt_gate_for_interrupt_granularity_bit;
	unsigned       idt_gate_for_interrupt_available;          /* Present: */
	unsigned       idt_gate_for_interrupt_accessed;          /* Present: */
	unsigned       idt_gate_for_interrupt_number_n_;          /* Gate for Interrupt #N: */
	unsigned       idt_gate_for_interrupt_number_2_;          /* Gate for Interrupt #2: */
	unsigned       idt_gate_for_interrupt_number_1_;          /* Gate for Interrupt #1: */
	unsigned       idt_gate_for_interrupt_number_0_;          /* Gate for Interrupt #0: */
} idt_segment_t;

typedef struct {
	unsigned      idt_gate_for_trap_selector_table_indicator;          /* Present: */
	unsigned long selector_table_index;
	unsigned      idt_gate_for_trap_selector_requester_privilege_level;          /* Descriptor Privilege Level: */
	unsigned      present;
	unsigned char *selector;
} mmuseg_trap_descriptor_table_trap_t;


typedef struct {
	unsigned       largest_trap_in_use;
	mmuseg_trap_descriptor_table_trap_t      **trap;
	/* unsigned long  gdt_used; */
	/* unsigned long  gdt_max; */
} mmuseg_trap_descriptor_table_t;

typedef struct trap_segment_s {
	struct trap_segment_s *x_trap;
	unsigned long  idt_gate_for_trap_selector;            /* Interrupt descriptor base:  16-bit (80286)/32-bit (80386 and higher) */
	unsigned long  idt_gate_for_trap_offset;           /* Interrupt descriptor limit: 16-bit (80286)/32-bit (80386 and higher) */
	unsigned long  idt_trap_segment_limit;
	unsigned       idt_gate_for_trap_x_;            /* Segment type 1: "0" or "1": */
	unsigned       idt_gate_for_trap_0_;            /* Segment type 1: "0" or "1": */
	unsigned       idt_gate_for_trap_type;            /* Segment type 1: "0" or "1": */
	unsigned       idt_gate_for_trap_type_b;            /* Segment type 1: "0" or "1": */
	unsigned       idt_gate_for_trap_default;            /* Segment type 1: "0" or "1": */
	unsigned       idt_gate_for_trap_type_1;          /* Segment type 1: "0" or "1": */
	unsigned       idt_gate_for_trap_type_0;          /* Segment type 0: "0" or "1": */
	unsigned       idt_gate_for_trap_type_c;          /* Segment type conforming: "0" for conforming or "1" for non-conforming: */
	unsigned       idt_gate_for_trap_type_e;          /* Segment type expand-down: "0" or "1" */
	unsigned       idt_gate_for_trap_type_r;          /* Segment type readable: "0" or "1" */
	unsigned       idt_gate_for_trap_type_w;          /* Segment type writable: "0" or "1" */
														   /* NOTE: Change "idt_limit" back to "size" if necessary and required by source code */
														   /* unsigned char  alloc; */
	unsigned       idt_gate_for_trap_descriptor_privilege_level;          /* Descriptor Privilege Level: */
	unsigned       idt_gate_for_trap_granularity_bit;
	unsigned       idt_gate_for_trap_available;          /* Present: */
	unsigned       idt_gate_for_trap_accessed;          /* Present: */
	unsigned       idt_gate_for_trap_number_n_;          /* Gate for Interrupt #N: */
	unsigned       idt_gate_for_trap_number_2_;          /* Gate for Interrupt #2: */
	unsigned       idt_gate_for_trap_number_1_;          /* Gate for Interrupt #1: */
	unsigned       idt_gate_for_trap_number_0_;          /* Gate for Interrupt #0: */
} trap_segment_t;

typedef struct {
	unsigned      idt_gate_for_task_selector_table_indicator;          /* Present: */
	unsigned long selector_table_index;
	unsigned      idt_gate_for_task_selector_requester_privilege_level;          /* Descriptor Privilege Level: */
	unsigned      present;
	unsigned char *selector;
} mmuseg_task_descriptor_table_task_t;


typedef struct {
	unsigned       largest_task_in_use;
	mmuseg_task_descriptor_table_task_t      **task;
	/* unsigned long  gdt_used; */
	/* unsigned long  gdt_max; */
} mmuseg_task_descriptor_table_t;

typedef struct task_segment_s {
	struct task_segment_s *x_task;
	unsigned long  idt_gate_for_task_selector;            /* Interrupt descriptor base:  16-bit (80286)/32-bit (80386 and higher) */
	unsigned long  idt_gate_for_task_offset;           /* Interrupt descriptor limit: 16-bit (80286)/32-bit (80386 and higher) */
	unsigned long  idt_task_segment_limit;
	unsigned       idt_gate_for_task_x_;            /* Segment type 1: "0" or "1": */
	unsigned       idt_gate_for_task_0_;            /* Segment type 1: "0" or "1": */
	unsigned       idt_gate_for_task_type;            /* Segment type 1: "0" or "1": */
	unsigned       idt_gate_for_task_type_b;            /* Segment type 1: "0" or "1": */
	unsigned       idt_gate_for_task_default;            /* Segment type 1: "0" or "1": */
	unsigned       idt_gate_for_task_type_1;          /* Segment type 1: "0" or "1": */
	unsigned       idt_gate_for_task_type_0;          /* Segment type 0: "0" or "1": */
	unsigned       idt_gate_for_task_type_c;          /* Segment type conforming: "0" for conforming or "1" for non-conforming: */
	unsigned       idt_gate_for_task_type_e;          /* Segment type expand-down: "0" or "1" */
	unsigned       idt_gate_for_task_type_r;          /* Segment type readable: "0" or "1" */
	unsigned       idt_gate_for_task_type_w;          /* Segment type writable: "0" or "1" */
													  /* NOTE: Change "idt_limit" back to "size" if necessary and required by source code */
													  /* unsigned char  alloc; */
	unsigned       idt_gate_for_task_descriptor_privilege_level;          /* Descriptor Privilege Level: */
	unsigned       idt_gate_for_task_granularity_bit;
	unsigned       idt_gate_for_task_available;          /* Present: */
	unsigned       idt_gate_for_task_accessed;          /* Present: */
	unsigned       idt_gate_for_task_number_n_;          /* Gate for Interrupt #N: */
	unsigned       idt_gate_for_task_number_2_;          /* Gate for Interrupt #2: */
	unsigned       idt_gate_for_task_number_1_;          /* Gate for Interrupt #1: */
	unsigned       idt_gate_for_task_number_0_;          /* Gate for Interrupt #0: */
} task_segment_t;

typedef struct {
	unsigned      selector_table_indicator;        /* Segment table indicator */
	unsigned long selector_table_index;        /* Segment table indicator */
	unsigned      selector_requester_privilege_level;        /* Segment requester privilege level */
	unsigned      present;
	unsigned char *selector;
} mmuseg_task_state_segment_descriptor_table_tss_t;


typedef struct {
	unsigned       largest_tss_in_use;
	mmuseg_task_state_segment_descriptor_table_tss_t      **tss;
	/* unsigned long  gdt_used; */
	/* unsigned long  gdt_max; */
} mmuseg_task_state_segment_descriptor_table_t;

typedef struct tss_segment_s {
	struct tss_segment_s *x_tss;
	unsigned long  tss_segment_base_address_offset;            /* Segment base:  16-bit (80286)/32-bit (80386 and higher) */
	unsigned long  tss_segment_limit;           /* Segment limit: 16-bit (80286)/32-bit (80386 and higher) */
													/* NOTE: Change "segment_limit" back to "size" if necessary and required by source code */
													/* unsigned char  alloc; */
	unsigned       tss_granularity_bit; /* Segment granularity bit: */
	unsigned       tss_big;             /* Big: "0" or "1" */
	unsigned       tss_default;           /* Default: "0" or "1" */
	unsigned       tss_x_;              /* X: "0" or "1" */
	unsigned       tss_0_;              /* 0: "0" or "1" */
	unsigned       tss_segment_available;       /* Segment AVL (available for use by systems programmers): "0" or "1" */
	unsigned       tss_descriptor_privilege_level; /* Descriptor privilege level: "1", "2", or "3" */
	unsigned       tss_type_of_segment_or_gate;          /* Type of Segment or Gate: */
	unsigned       tss_type_1;          /* Segment type 1: "0" or "1": */
	unsigned       tss_type_0;          /* Segment type 0: "0" or "1": */
	unsigned       tss_type_c;          /* Segment type conforming: "0" for conforming or "1" for non-conforming: */
	unsigned       tss_type_e;          /* Segment type expand-down: "0" or "1" */
	unsigned       tss_type_r;          /* Segment type readable: "0" or "1" */
	unsigned       tss_type_w;          /* Segment type writable: "0" or "1" */
	unsigned       tss_segment_accessed;        /* Segment A (accessed): "0" or "1" */
	unsigned long  tss_ldt_segment_base_address_offset;            /* Segment base:  16-bit (80286)/32-bit (80386 and higher) */
	unsigned long  tss_ldt_segment_limit;           /* Segment limit: 16-bit (80286)/32-bit (80386 and higher) */
	unsigned       tss_ldt_granularity_bit; /* Segment granularity bit: */
	unsigned       tss_ldt_big;             /* Big: "0" or "1" */
	unsigned       tss_ldt_default;           /* Default: "0" or "1" */
	unsigned       tss_ldt_x_;              /* X: "0" or "1" */
	unsigned       tss_ldt_0_;              /* 0: "0" or "1" */
	unsigned       tss_ldt_segment_available;       /* Segment AVL (available for use by systems programmers): "0" or "1" */
	unsigned       tss_ldt_segment_is_present;      /* Segment present: "1" if present and "0" if not present */
	unsigned       tss_ldt_descriptor_privilege_level; /* Descriptor privilege level: "1", "2", or "3" */
	unsigned       tss_ldt_type_of_segment_or_gate;          /* Type of Segment or Gate: */
	unsigned       tss_ldt_type_1;          /* Segment type 1: "0" or "1": */
	unsigned       tss_ldt_type_0;          /* Segment type 0: "0" or "1": */
	unsigned       tss_ldt_type_c;          /* Segment type conforming: "0" for conforming or "1" for non-conforming: */
	unsigned       tss_ldt_type_e;          /* Segment type expand-down: "0" or "1" */
	unsigned       tss_ldt_type_r;          /* Segment type readable: "0" or "1" */
	unsigned       tss_ldt_type_w;          /* Segment type writable: "0" or "1" */
	unsigned       tss_ldt_segment_accessed;        /* Segment A (accessed): "0" or "1" */
	unsigned       tss_ldt_selector_table_indicator;        /* Segment table indicator */
	unsigned long  tss_ldt_selector_index;        /* Segment table indicator */
	unsigned       tss_ldt_selector_requester_privilege_level;        /* Segment requester privilege level */
	unsigned long  specified_value_of_tss;                       /* Value of Task State Segment being created */
																 /* I/O (Input/Output) Map Base: */
	unsigned long  input_output_map_base;                        /* I/O (Input/Output) Map Base */
																 /* Debug trap bit (T): */
	unsigned       debug_trap_bit;                               /* Debug trap bit (T) */
																 /*  CPU Registers (32-bit) for TSS: */
	unsigned long  tss_register_eax;                                /* AX (16-bit)/eAX (32-bit) CPU register */
	unsigned long  tss_register_ebx;                                /* BX (16-bit)/eBX (32-bit) CPU register */
	unsigned long  tss_register_ecx;                                /* CX (16-bit)/eCX (32-bit) CPU register */
	unsigned long  tss_register_edx;                                /* DX (16-bit)/eDX (32-bit) CPU register */
	unsigned long  tss_register_esp;                                /* SP (16-bit)/eSP (32-bit) CPU register */
	unsigned long  tss_register_ebp;                                /* BP (16-bit)/eBP (32-bit) CPU register */
	unsigned long  tss_register_esi;                                /* SI (16-bit)/eSI (32-bit) CPU register */
	unsigned long  tss_register_edi;                                /* DI (16-bit)/eDI (32-bit) CPU register */
	unsigned long  tss_register_esp_for_privilege_level_2;          /* SP (16-bit)/eSP (32-bit) CPU register */
	unsigned long  tss_register_esp_for_privilege_level_1;          /* SP (16-bit)/eSP (32-bit) CPU register */
	unsigned long  tss_register_esp_for_privilege_level_0;          /* SP (16-bit)/eSP (32-bit) CPU register */
																	/*  FLAGS (16-bit) register for TSS: */
	unsigned       tss_flags_cf;                                 /* Carry           Flag */
	unsigned       tss_flags_pf;                                 /* Parity          Flag */
	unsigned       tss_flags_af;                                 /* Auxiliary Carry Flag */
	unsigned       tss_flags_zf;                                 /* Zero            Flag */
	unsigned       tss_flags_sf;                                 /* Sign            Flag */
	unsigned       tss_flags_tf;                                 /* Trap            Flag */
	unsigned       tss_flags_if;                                 /* Interrupt       Flag */
	unsigned       tss_flags_df;                                 /* Difference      Flag */
	unsigned       tss_flags_of;                                 /* Overflow        Flag */
																 /* EFLAGS (32-bit) register for TSS: */
	unsigned       tss_eflags_iopl;                              /* Input/Output Privilege Level Flag */
	unsigned       tss_eflags_nt;                                /* Nested Task                  Flag */
	unsigned       tss_eflags_rf;                                /* Return                       Flag */
	unsigned       tss_eflags_vm;                                /* Virtual Machine              Flag */
																 /* Instruction Pointer (EIP) for TSS: */
	unsigned long  tss_instruction_pointer_eip;                  /* Instruction Pointer (EIP) */
																 /* Control Register 3 (Page Directory Base Register): */
	unsigned long  tss_pdbr_page_frame_address_specified;          /* Page frame address:  32-bit (80386 and higher) */
	unsigned long  tss_pdbr_page_frame_address_being_loaded;       /* Page frame address:  32-bit (80386 and higher) */
	unsigned long  tss_pdbr_page_frame_address;                    /* Page frame address:  32-bit (80386 and higher) */
	unsigned       tss_pdbr_page_table_is_present;                 /* Page table present: "1" if present and "0" if not present */
	unsigned       tss_pdbr_page_table_read_write;                 /* Page table read/write: "0" if read-only and "1" if read/write */
	unsigned       tss_pdbr_page_table_user_supervisor;            /* Page table user/supervisor: "0" if user and "1" if supervisor: */
	unsigned       tss_pdbr_page_table_dirty;                      /* Page table Default: "0" or "1" */
	unsigned       tss_pdbr_page_table_available;                  /* Page table available for systems programmer use: "0" or "1" */
																   /* Page Directory: */
	unsigned long  tss_directory_page_frame_address_specified;          /* Page frame address:  32-bit (80386 and higher) */
	unsigned long  tss_directory_page_frame_address_being_loaded;       /* Page frame address:  32-bit (80386 and higher) */
	unsigned long  tss_directory_page_frame_address;                    /* Page frame address:  32-bit (80386 and higher) */
	unsigned       tss_directory_page_table_read_write;                 /* Page table read/write: "0" if read-only and "1" if read/write */
	unsigned       tss_directory_page_table_user_supervisor;            /* Page table user/supervisor: "0" if user and "1" if supervisor: */
	unsigned       tss_directory_page_table_dirty;                      /* Page table Default: "0" or "1" */
	unsigned       tss_directory_page_table_available;                  /* Page table available for systems programmer use: "0" or "1" */
																		/* ESP (stack pointer register) for TSS: */
	unsigned long  tss_stack_pointer_esp_for_privilege_level_2;  /* ESP (stack pointer register) for Privilege Level 2 */
	unsigned long  tss_stack_pointer_esp_for_privilege_level_1;  /* ESP (stack pointer register) for Privilege Level 1 */
	unsigned long  tss_stack_pointer_esp_for_privilege_level_0;  /* ESP (stack pointer register) for Privilege Level 0 */
																 /* CS segment register for TSS: */
	unsigned long  tss_cs_segment_register_descriptor_base_address_offset;                       /* Segment base:  16-bit (80286)/32-bit (80386 and higher) */
	unsigned long  tss_cs_segment_register_descriptor_limit;                      /* Segment limit: 16-bit (80286)/32-bit (80386 and higher) */
																				  /* NOTE: Change "segment_limit" back to "size" if necessary and required by source code */
	unsigned       tss_cs_segment_register_descriptor_granularity_bit;            /* Segment granularity bit: */
	unsigned       tss_cs_segment_register_descriptor_big;                        /* Big: "0" or "1" */
	unsigned       tss_cs_segment_register_descriptor_default;                      /* Default: "0" or "1" */
	unsigned       tss_cs_segment_register_descriptor_x_;                         /* X: "0" or "1" */
	unsigned       tss_cs_segment_register_descriptor_0_;                         /* 0: "0" or "1" */
	unsigned       tss_cs_segment_register_descriptor_available;                  /* Segment AVL (available for use by systems programmers): "0" or "1" */
	unsigned       tss_cs_segment_register_descriptor_is_present;                 /* Segment present: "1" if present and "0" if not present */
	unsigned       tss_cs_segment_register_descriptor_privilege_level;            /* Descriptor privilege level: "1", "2", or "3" */
	unsigned       tss_cs_segment_register_type_of_segment_or_gate;               /* Type of Segment or Gate: */
	unsigned       tss_cs_segment_register_descriptor_type_1;                     /* Segment type 1: "0" or "1": */
	unsigned       tss_cs_segment_register_descriptor_type_0;                     /* Segment type 0: "0" or "1": */
	unsigned       tss_cs_segment_register_descriptor_type_c;                     /* Segment type conforming: "0" for conforming or "1" for non-conforming: */
	unsigned       tss_cs_segment_register_descriptor_type_e;                     /* Segment type expand-down: "0" or "1" */
	unsigned       tss_cs_segment_register_descriptor_type_r;                     /* Segment type readable: "0" or "1" */
	unsigned       tss_cs_segment_register_descriptor_type_w;                     /* Segment type writable: "0" or "1" */
	unsigned       tss_cs_segment_register_descriptor_accessed;                   /* Segment A (accessed): "0" or "1" */
	unsigned       tss_cs_segment_register_selector_table_indicator;        /* Segment table indicator */
	unsigned long  tss_cs_segment_register_selector_index;        /* Segment table indicator */
	unsigned       tss_cs_segment_register_selector_requester_privilege_level;        /* Segment requester privilege level */
																					  /* DS segment register for TSS: */
	unsigned long  tss_ds_segment_register_descriptor_base_address_offset;                       /* Segment base:  16-bit (80286)/32-bit (80386 and higher) */
	unsigned long  tss_ds_segment_register_descriptor_limit;                      /* Segment limit: 16-bit (80286)/32-bit (80386 and higher) */
																				  /* NOTE: Change "segment_limit" back to "size" if necessary and required by source code */
	unsigned       tss_ds_segment_register_descriptor_granularity_bit;            /* Segment granularity bit: */
	unsigned       tss_ds_segment_register_descriptor_big;                        /* Big: "0" or "1" */
	unsigned       tss_ds_segment_register_descriptor_default;                      /* Default: "0" or "1" */
	unsigned       tss_ds_segment_register_descriptor_x_;                         /* X: "0" or "1" */
	unsigned       tss_ds_segment_register_descriptor_0_;                         /* 0: "0" or "1" */
	unsigned       tss_ds_segment_register_descriptor_available;                  /* Segment AVL (available for use by systems programmers): "0" or "1" */
	unsigned       tss_ds_segment_register_descriptor_is_present;                 /* Segment present: "1" if present and "0" if not present */
	unsigned       tss_ds_segment_register_descriptor_privilege_level;            /* Descriptor privilege level: "1", "2", or "3" */
	unsigned       tss_ds_segment_register_type_of_segment_or_gate;               /* Type of Segment or Gate: */
	unsigned       tss_ds_segment_register_descriptor_type_1;                     /* Segment type 1: "0" or "1": */
	unsigned       tss_ds_segment_register_descriptor_type_0;                     /* Segment type 0: "0" or "1": */
	unsigned       tss_ds_segment_register_descriptor_type_c;                     /* Segment type conforming: "0" for conforming or "1" for non-conforming: */
	unsigned       tss_ds_segment_register_descriptor_type_e;                     /* Segment type expand-down: "0" or "1" */
	unsigned       tss_ds_segment_register_descriptor_type_r;                     /* Segment type readable: "0" or "1" */
	unsigned       tss_ds_segment_register_descriptor_type_w;                     /* Segment type writable: "0" or "1" */
	unsigned       tss_ds_segment_register_descriptor_accessed;                   /* Segment A (accessed): "0" or "1" */
	unsigned       tss_ds_segment_register_selector_table_indicator;        /* Segment table indicator */
	unsigned long  tss_ds_segment_register_selector_index;        /* Segment table indicator */
	unsigned       tss_ds_segment_register_selector_requester_privilege_level;        /* Segment requester privilege level */
																					  /* ES segment register for TSS: */
	unsigned long  tss_es_segment_register_descriptor_base_address_offset;                       /* Segment base:  16-bit (80286)/32-bit (80386 and higher) */
	unsigned long  tss_es_segment_register_descriptor_limit;                      /* Segment limit: 16-bit (80286)/32-bit (80386 and higher) */
																				  /* NOTE: Change "segment_limit" back to "size" if necessary and required by source code */
	unsigned       tss_es_segment_register_descriptor_granularity_bit;            /* Segment granularity bit: */
	unsigned       tss_es_segment_register_descriptor_big;                        /* Big: "0" or "1" */
	unsigned       tss_es_segment_register_descriptor_default;                      /* Default: "0" or "1" */
	unsigned       tss_es_segment_register_descriptor_x_;                         /* X: "0" or "1" */
	unsigned       tss_es_segment_register_descriptor_0_;                         /* 0: "0" or "1" */
	unsigned       tss_es_segment_register_descriptor_available;                  /* Segment AVL (available for use by systems programmers): "0" or "1" */
	unsigned       tss_es_segment_register_descriptor_is_present;                 /* Segment present: "1" if present and "0" if not present */
	unsigned       tss_es_segment_register_descriptor_privilege_level;            /* Descriptor privilege level: "1", "2", or "3" */
	unsigned       tss_es_segment_register_type_of_segment_or_gate;               /* Type of Segment or Gate: */
	unsigned       tss_es_segment_register_descriptor_type_1;                     /* Segment type 1: "0" or "1": */
	unsigned       tss_es_segment_register_descriptor_type_0;                     /* Segment type 0: "0" or "1": */
	unsigned       tss_es_segment_register_descriptor_type_c;                     /* Segment type conforming: "0" for conforming or "1" for non-conforming: */
	unsigned       tss_es_segment_register_descriptor_type_e;                     /* Segment type expand-down: "0" or "1" */
	unsigned       tss_es_segment_register_descriptor_type_r;                     /* Segment type readable: "0" or "1" */
	unsigned       tss_es_segment_register_descriptor_type_w;                     /* Segment type writable: "0" or "1" */
	unsigned       tss_es_segment_register_descriptor_accessed;                   /* Segment A (accessed): "0" or "1" */
	unsigned       tss_es_segment_register_selector_table_indicator;        /* Segment table indicator */
	unsigned long  tss_es_segment_register_selector_index;        /* Segment table indicator */
	unsigned       tss_es_segment_register_selector_requester_privilege_level;        /* Segment requester privilege level */
																					  /* DS segment register for TSS: */
	unsigned long  tss_fs_segment_register_descriptor_base_address_offset;                       /* Segment base:  16-bit (80286)/32-bit (80386 and higher) */
	unsigned long  tss_fs_segment_register_descriptor_limit;                      /* Segment limit: 16-bit (80286)/32-bit (80386 and higher) */
																				  /* NOTE: Change "segment_limit" back to "size" if necessary and required by source code */
	unsigned       tss_fs_segment_register_descriptor_granularity_bit;            /* Segment granularity bit: */
	unsigned       tss_fs_segment_register_descriptor_big;                        /* Big: "0" or "1" */
	unsigned       tss_fs_segment_register_descriptor_default;                      /* Default: "0" or "1" */
	unsigned       tss_fs_segment_register_descriptor_x_;                         /* X: "0" or "1" */
	unsigned       tss_fs_segment_register_descriptor_0_;                         /* 0: "0" or "1" */
	unsigned       tss_fs_segment_register_descriptor_available;                  /* Segment AVL (available for use by systems programmers): "0" or "1" */
	unsigned       tss_fs_segment_register_descriptor_is_present;                 /* Segment present: "1" if present and "0" if not present */
	unsigned       tss_fs_segment_register_descriptor_privilege_level;            /* Descriptor privilege level: "1", "2", or "3" */
	unsigned       tss_fs_segment_register_type_of_segment_or_gate;               /* Type of Segment or Gate: */
	unsigned       tss_fs_segment_register_descriptor_type_1;                     /* Segment type 1: "0" or "1": */
	unsigned       tss_fs_segment_register_descriptor_type_0;                     /* Segment type 0: "0" or "1": */
	unsigned       tss_fs_segment_register_descriptor_type_c;                     /* Segment type conforming: "0" for conforming or "1" for non-conforming: */
	unsigned       tss_fs_segment_register_descriptor_type_e;                     /* Segment type expand-down: "0" or "1" */
	unsigned       tss_fs_segment_register_descriptor_type_r;                     /* Segment type readable: "0" or "1" */
	unsigned       tss_fs_segment_register_descriptor_type_w;                     /* Segment type writable: "0" or "1" */
	unsigned       tss_fs_segment_register_descriptor_accessed;                   /* Segment A (accessed): "0" or "1" */
	unsigned       tss_fs_segment_register_selector_table_indicator;        /* Segment table indicator */
	unsigned long  tss_fs_segment_register_selector_index;        /* Segment table indicator */
	unsigned       tss_fs_segment_register_selector_requester_privilege_level;        /* Segment requester privilege level */
																					  /* DS segment register for TSS: */
	unsigned long  tss_gs_segment_register_descriptor_base_address_offset;                       /* Segment base:  16-bit (80286)/32-bit (80386 and higher) */
	unsigned long  tss_gs_segment_register_descriptor_limit;                      /* Segment limit: 16-bit (80286)/32-bit (80386 and higher) */
																				  /* NOTE: Change "segment_limit" back to "size" if necessary and required by source code */
	unsigned       tss_gs_segment_register_descriptor_granularity_bit;            /* Segment granularity bit: */
	unsigned       tss_gs_segment_register_descriptor_big;                        /* Big: "0" or "1" */
	unsigned       tss_gs_segment_register_descriptor_default;                      /* Default: "0" or "1" */
	unsigned       tss_gs_segment_register_descriptor_x_;                         /* X: "0" or "1" */
	unsigned       tss_gs_segment_register_descriptor_0_;                         /* 0: "0" or "1" */
	unsigned       tss_gs_segment_register_descriptor_available;                  /* Segment AVL (available for use by systems programmers): "0" or "1" */
	unsigned       tss_gs_segment_register_descriptor_is_present;                 /* Segment present: "1" if present and "0" if not present */
	unsigned       tss_gs_segment_register_descriptor_privilege_level;            /* Descriptor privilege level: "1", "2", or "3" */
	unsigned       tss_gs_segment_register_type_of_segment_or_gate;               /* Type of Segment or Gate: */
	unsigned       tss_gs_segment_register_descriptor_type_1;                     /* Segment type 1: "0" or "1": */
	unsigned       tss_gs_segment_register_descriptor_type_0;                     /* Segment type 0: "0" or "1": */
	unsigned       tss_gs_segment_register_descriptor_type_c;                     /* Segment type conforming: "0" for conforming or "1" for non-conforming: */
	unsigned       tss_gs_segment_register_descriptor_type_e;                     /* Segment type expand-down: "0" or "1" */
	unsigned       tss_gs_segment_register_descriptor_type_r;                     /* Segment type readable: "0" or "1" */
	unsigned       tss_gs_segment_register_descriptor_type_w;                     /* Segment type writable: "0" or "1" */
	unsigned       tss_gs_segment_register_descriptor_accessed;                   /* Segment A (accessed): "0" or "1" */
	unsigned       tss_gs_segment_register_selector_table_indicator;        /* Segment table indicator */
	unsigned long  tss_gs_segment_register_selector_index;        /* Segment table indicator */
	unsigned       tss_gs_segment_register_selector_requester_privilege_level;        /* Segment requester privilege level */
																					  /* SS segment register (stack segment) for TSS (privilege level 2): */
	unsigned long  tss_ss_segment_register_descriptor_base_address_offset_for_privilege_level_2;            /* Segment base:  16-bit (80286)/32-bit (80386 and higher) */
	unsigned long  tss_ss_segment_register_descriptor_limit_for_privilege_level_2;           /* Segment limit: 16-bit (80286)/32-bit (80386 and higher) */
																							 /* NOTE: Change "segment_limit" back to "size" if necessary and required by source code */
	unsigned       tss_ss_segment_register_descriptor_granularity_bit_for_privilege_level_2; /* Segment granularity bit: */
	unsigned       tss_ss_segment_register_descriptor_big_for_privilege_level_2;             /* Big: "0" or "1" */
	unsigned       tss_ss_segment_register_descriptor_default_for_privilege_level_2;           /* Default: "0" or "1" */
	unsigned       tss_ss_segment_register_descriptor_x_for_privilege_level_2;               /* X: "0" or "1" */
	unsigned       tss_ss_segment_register_descriptor_0_for_privilege_level_2;               /* 0: "0" or "1" */
	unsigned       tss_ss_segment_register_descriptor_available_for_privilege_level_2;/* Segment AVL(available for use by systems programmers):"0"or"1" */
	unsigned       tss_ss_segment_register_descriptor_is_present_for_privilege_level_2;      /* Segment present: "1" if present and "0" if not present */
	unsigned       tss_ss_segment_register_descriptor_privilege_level_for_privilege_level_2; /* Descriptor privilege level: "1", "2", or "3" */
	unsigned       tss_ss_segment_register_type_of_segment_or_gate_for_privilege_level_2;    /* Type of Segment or Gate: */
	unsigned       tss_ss_segment_register_descriptor_type_1_for_privilege_level_2;          /* Segment type 1: "0" or "1": */
	unsigned       tss_ss_segment_register_descriptor_type_0_for_privilege_level_2;          /* Segment type 0: "0" or "1": */
	unsigned       tss_ss_segment_register_descriptor_type_c_for_privilege_level_2;/*Segment type conforming:"0"for conforming or "1"for non-conforming:*/
	unsigned       tss_ss_segment_register_descriptor_type_e_for_privilege_level_2;          /* Segment type expand-down: "0" or "1" */
	unsigned       tss_ss_segment_register_descriptor_type_r_for_privilege_level_2;          /* Segment type readable: "0" or "1" */
	unsigned       tss_ss_segment_register_descriptor_type_w_for_privilege_level_2;          /* Segment type writable: "0" or "1" */
	unsigned       tss_ss_segment_register_descriptor_accessed_for_privilege_level_2;        /* Segment A (accessed): "0" or "1" */
	unsigned       tss_ss_segment_register_selector_table_indicator_for_privilege_level_2;   /* Segment table indicator */
	unsigned long  tss_ss_segment_register_selector_index_for_privilege_level_2;             /* Segment table indicator */
	unsigned       tss_ss_segment_register_selector_requester_privilege_level_for_privilege_level_2; /* Segment requester privilege level */
																									 /* Stack segment for TSS (privilege level 1): */
	unsigned long  tss_ss_segment_register_descriptor_base_address_offset_for_privilege_level_1;            /* Segment base:  16-bit (80286)/32-bit (80386 and higher) */
	unsigned long  tss_ss_segment_register_descriptor_limit_for_privilege_level_1;           /* Segment limit: 16-bit (80286)/32-bit (80386 and higher) */
																							 /* NOTE: Change "segment_limit" back to "size" if necessary and required by source code */
	unsigned       tss_ss_segment_register_descriptor_granularity_bit_for_privilege_level_1; /* Segment granularity bit: */
	unsigned       tss_ss_segment_register_descriptor_big_for_privilege_level_1;             /* Big: "0" or "1" */
	unsigned       tss_ss_segment_register_descriptor_default_for_privilege_level_1;           /* Default: "0" or "1" */
	unsigned       tss_ss_segment_register_descriptor_x_for_privilege_level_1;               /* X: "0" or "1" */
	unsigned       tss_ss_segment_register_descriptor_0_for_privilege_level_1;               /* 0: "0" or "1" */
	unsigned       tss_ss_segment_register_descriptor_available_for_privilege_level_1;/* Segment AVL(available for use by systems programmers):"0"or"1" */
	unsigned       tss_ss_segment_register_descriptor_is_present_for_privilege_level_1;      /* Segment present: "1" if present and "0" if not present */
	unsigned       tss_ss_segment_register_descriptor_privilege_level_for_privilege_level_1; /* Descriptor privilege level: "1", "2", or "3" */
	unsigned       tss_ss_segment_register_type_of_segment_or_gate_for_privilege_level_1;    /* Type of Segment or Gate: */
	unsigned       tss_ss_segment_register_descriptor_type_1_for_privilege_level_1;          /* Segment type 1: "0" or "1": */
	unsigned       tss_ss_segment_register_descriptor_type_0_for_privilege_level_1;          /* Segment type 0: "0" or "1": */
	unsigned       tss_ss_segment_register_descriptor_type_c_for_privilege_level_1;/*Segment type conforming:"0"for conforming or "1"for non-conforming:*/
	unsigned       tss_ss_segment_register_descriptor_type_e_for_privilege_level_1;          /* Segment type expand-down: "0" or "1" */
	unsigned       tss_ss_segment_register_descriptor_type_r_for_privilege_level_1;          /* Segment type readable: "0" or "1" */
	unsigned       tss_ss_segment_register_descriptor_type_w_for_privilege_level_1;          /* Segment type writable: "0" or "1" */
	unsigned       tss_ss_segment_register_descriptor_accessed_for_privilege_level_1;        /* Segment A (accessed): "0" or "1" */
	unsigned       tss_ss_segment_register_selector_table_indicator_for_privilege_level_1;   /* Segment table indicator */
	unsigned long  tss_ss_segment_register_selector_index_for_privilege_level_1;             /* Segment table indicator */
	unsigned       tss_ss_segment_register_selector_requester_privilege_level_for_privilege_level_1; /* Segment requester privilege level */
																									 /* Stack segment for TSS (privilege level 0): */
	unsigned long  tss_ss_segment_register_descriptor_base_address_offset_for_privilege_level_0;            /* Segment base:  16-bit (80286)/32-bit (80386 and higher) */
	unsigned long  tss_ss_segment_register_descriptor_limit_for_privilege_level_0;           /* Segment limit: 16-bit (80286)/32-bit (80386 and higher) */
																							 /* NOTE: Change "segment_limit" back to "size" if necessary and required by source code */
	unsigned       tss_ss_segment_register_descriptor_granularity_bit_for_privilege_level_0; /* Segment granularity bit: */
	unsigned       tss_ss_segment_register_descriptor_big_for_privilege_level_0;             /* Big: "0" or "1" */
	unsigned       tss_ss_segment_register_descriptor_default_for_privilege_level_0;           /* Default: "0" or "1" */
	unsigned       tss_ss_segment_register_descriptor_x_for_privilege_level_0;               /* X: "0" or "1" */
	unsigned       tss_ss_segment_register_descriptor_0_for_privilege_level_0;               /* 0: "0" or "1" */
	unsigned       tss_ss_segment_register_descriptor_available_for_privilege_level_0;/* Segment AVL(available for use by systems programmers):"0"or"1" */
	unsigned       tss_ss_segment_register_descriptor_is_present_for_privilege_level_0;      /* Segment present: "1" if present and "0" if not present */
	unsigned       tss_ss_segment_register_descriptor_privilege_level_for_privilege_level_0; /* Descriptor privilege level: "1", "2", or "3" */
	unsigned       tss_ss_segment_register_type_of_segment_or_gate_for_privilege_level_0;    /* Type of Segment or Gate: */
	unsigned       tss_ss_segment_register_descriptor_type_1_for_privilege_level_0;          /* Segment type 1: "0" or "1": */
	unsigned       tss_ss_segment_register_descriptor_type_0_for_privilege_level_0;          /* Segment type 0: "0" or "1": */
	unsigned       tss_ss_segment_register_descriptor_type_c_for_privilege_level_0;/*Segment type conforming:"0"for conforming or "1"for non-conforming:*/
	unsigned       tss_ss_segment_register_descriptor_type_e_for_privilege_level_0;          /* Segment type expand-down: "0" or "1" */
	unsigned       tss_ss_segment_register_descriptor_type_r_for_privilege_level_0;          /* Segment type readable: "0" or "1" */
	unsigned       tss_ss_segment_register_descriptor_type_w_for_privilege_level_0;          /* Segment type writable: "0" or "1" */
	unsigned       tss_ss_segment_register_descriptor_accessed_for_privilege_level_0;        /* Segment A (accessed): "0" or "1" */
	unsigned       tss_ss_segment_register_selector_table_indicator_for_privilege_level_0;   /* Segment table indicator */
	unsigned long  tss_ss_segment_register_selector_index_for_privilege_level_0;             /* Segment table indicator */
	unsigned       tss_ss_segment_register_selector_requester_privilege_level_for_privilege_level_0; /* Segment requester privilege level */
																									 /* Task State Segment (TSS) Back Link: */
	unsigned long  tss_back_link;                                /* TSS (Task State Segment) back link */
																 /* Optional Task Gate Descriptor tables: */
																 /* Task Gate Descriptor: */
	unsigned       task_gate_segment_limit;   /* Used for referring to Task State Segment descriptor only */
	unsigned       task_gate_segment_present;      /* Segment present: "1" if present and "0" if not present */
	unsigned       task_gate_is_present;      /* Segment present: "1" if present and "0" if not present */
	unsigned       task_gate_privilege_level; /* Descriptor privilege level: "1", "2", or "3" */
	unsigned       task_gate_type_of_segment_or_gate;          /* Type of Segment or Gate: */
	unsigned       task_gate_selector_table_indicator;        /* Segment table indicator */
	unsigned long  task_gate_selector_index;        /* Segment table indicator */
	unsigned       task_gate_selector_requester_privilege_level;        /* Segment requester privilege level */
																		/* Used for task gates residing in Interrupt Descriptor Table (Interrupt Gates): */
																		/* unsigned long  task_gate_idt_gate_for_interrupt_selector;            /* Interrupt descriptor base:  16-bit (80286)/32-bit (80386 and higher) */
																		/* unsigned long  task_gate_idt_gate_for_interrupt_offset;           /* Interrupt descriptor limit: 16-bit (80286)/32-bit (80386 and higher) */
																		/* unsigned       task_gate_idt_gate_for_interrupt_present;          /* Present: */
																		/* unsigned       task_gate_idt_gate_for_interrupt_descriptor_privilege_level;          /* Descriptor Privilege Level: */
																		/* unsigned       task_gate_idt_gate_for_interrupt_#n_;          /* Gate for Interrupt #N: */
																		/* unsigned       task_gate_idt_gate_for_interrupt_#2_;          /* Gate for Interrupt #2: */
																		/* unsigned       task_gate_idt_gate_for_interrupt_#1_;          /* Gate for Interrupt #1: */
																		/* unsigned       task_gate_idt_gate_for_interrupt_#0_;          /* Gate for Interrupt #0: */
																		/* Used for task gates residing in Interrupt Descriptor Table (Trap Gates): */
																		/* unsigned long  task_gate_idt_gate_for_trap_selector;            /* Interrupt descriptor base:  16-bit (80286)/32-bit (80386 and higher) */
																		/* unsigned long  task_gate_idt_gate_for_trap_offset;           /* Interrupt descriptor limit: 16-bit (80286)/32-bit (80386 and higher) */
																		/* unsigned       task_gate_idt_gate_for_trap_present;          /* Present: */
																		/* unsigned       task_gate_idt_gate_for_trap_descriptor_privilege_level;          /* Descriptor Privilege Level: */
	unsigned       task_gate_idt_gate_for_trap_number_n_;          /* Gate for Interrupt #N: */
	unsigned       task_gate_idt_gate_for_trap_number_2_;          /* Gate for Interrupt #2: */
	unsigned       task_gate_idt_gate_for_trap_number_1_;          /* Gate for Interrupt #1: */
	unsigned       task_gate_idt_gate_for_trap_number_0_;          /* Gate for Interrupt #0: */
} tss_segment_t;

typedef struct {
	unsigned long directory_page_table_address;
	unsigned      directory_page_table_read_write;                 /* Page table read/write: "0" if read-only and "1" if read/write */
	unsigned      directory_page_table_user_supervisor;            /* Page table user/supervisor: "0" if user and "1" if supervisor: */
	unsigned      directory_page_table_dirty;                      /* Page table Default: "0" or "1" */
	unsigned      present;
	unsigned char *directory_page_table;
} mmuseg_directory_page_table_page_t;


typedef struct {
	unsigned       largest_directory_page_in_use;
	mmuseg_directory_page_table_page_t      **directory_page;
	/* unsigned long  gdt_used; */
	/* unsigned long  gdt_max; */
} mmuseg_directory_page_table_t;

typedef struct directory_page_table_s {
	struct directory_page_table_s *x_directory_page;
	unsigned long directory_page_table_address;
} directory_page_table_t;

typedef struct {
	unsigned long entry_page_table_address;
	unsigned      present;
	unsigned      entry_page_table_read_write;                 /* Page table read/write: "0" if read-only and "1" if read/write */
	unsigned      entry_page_table_user_supervisor;            /* Page table user/supervisor: "0" if user and "1" if supervisor: */
	unsigned      entry_page_table_dirty;                      /* Page table Default: "0" or "1" */
	unsigned char *entry_page_table;
} mmuseg_entry_page_table_page_t;


typedef struct {
	unsigned       largest_entry_page_in_use;
	mmuseg_entry_page_table_page_t      **entry_page;
	/* unsigned long  gdt_used; */
	/* unsigned long  gdt_max; */
} mmuseg_entry_page_table_t;

typedef struct entry_page_table_s {
	struct entry_page_table_s *x_entry_page;
	unsigned long entry_page_table_address;
} entry_page_table_t;

typedef struct init_t {
	mmuseg_global_descriptor_table_t             *mmu_gdt;
	mmuseg_local_descriptor_table_t              *mmu_ldt;
	mmuseg_interrupt_descriptor_table_t          *mmu_idt;
	mmuseg_task_state_segment_descriptor_table_t *mmu_tss;
	mmuseg_directory_page_table_t                *mmu_directory_page;
	mmuseg_entry_page_table_t                    *mmu_entry_page;
} init_t;


mmuseg_global_descriptor_table_t *mmuseg_gdt_new (gdt_segment_t *gdt_seg);
void mmuseg_gdt_del (mmuseg_global_descriptor_table_t *mmu_gdt);
mmuseg_local_descriptor_table_t *mmuseg_ldt_new(ldt_segment_t *ldt_seg);
void mmuseg_ldt_del(mmuseg_local_descriptor_table_t *mmu_ldt);
mmuseg_interrupt_descriptor_table_t *mmuseg_idt_new(idt_segment_t *idt_seg);
void mmuseg_idt_del(mmuseg_interrupt_descriptor_table_t *mmu_idt);
mmuseg_task_state_segment_descriptor_table_t *mmuseg_tss_new(tss_segment_t *tss_seg);
void mmuseg_tss_del(mmuseg_task_state_segment_descriptor_table_t *mmu_tss);
mmuseg_directory_page_table_t *mmuseg_directory_page_new(directory_page_table_t *directory_page);
void mmuseg_directory_page_del(mmuseg_directory_page_table_t *mmu_directory_page);
mmuseg_entry_page_table_t *mmuseg_entry_page_new(entry_page_table_t *entry_page);
void mmuseg_entry_page_del(mmuseg_entry_page_table_t *mmu_entry_page);
#endif
