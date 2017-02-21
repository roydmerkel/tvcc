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


#ifndef PCE_E8086_H
#define PCE_E8086_H 1


#include <stdio.h>
/* #include "mmuseg.h" /* Memory Managment Unit (Intel 80386 CPU) */


/* CPU options */
#define E86_CPU_REP_BUG    0x01         /* enable rep/seg bug */
#define E86_CPU_MASK_SHIFT 0x02         /* mask shift count */
#define E86_CPU_PUSH_FIRST 0x04         /* push sp before decrementing it */
#define E86_CPU_INT6       0x08         /* throw illegal opcode exception */
#define E86_CPU_INT7       0x10		/* throw escape opcode exception */
#define E86_CPU_FLAGS286   0x20         /* Allow clearing flags 12-15 */
#define E86_CPU_8BIT       0x40		/* 16 bit accesses take more time */

/* CPU flags */
#define E86_FLG_CF      0x00000001 /* 00: Carry       Flag */
#define E86_FLG_BIT_1   0x00000002 /* 01: Reserved (1)     */
#define E86_FLG_PF      0x00000004 /* 02: Parity      Flag */
#define E86_FLG_BIT_3   0x00000008 /* 03: Reserved (0)     */
#define E86_FLG_AF      0x00000010 /* 04: Accumulator Flag */
#define E86_FLG_BIT_5   0x00000020 /* 05: Reserved (0)     */
#define E86_FLG_ZF      0x00000040 /* 06: Zero        Flag */
#define E86_FLG_SF      0x00000080 /* 07: Sign        Flag */
#define E86_FLG_TF      0x00000100 /* 08: Trap        Flag */
#define E86_FLG_IF      0x00000200 /* 09: Interrupt   Flag */
#define E86_FLG_DF      0x00000400 /* 10: Direction   Flag */
#define E86_FLG_OF      0x00000800 /* 11: Overflow    Flag */
#define E286_FLG_IOPL   0x00001000 /* 12: Input/Output Privilege Level Flag (Intel 80286 CPU and higher only) */
#define E286_FLG_BIT_13 0x00002000 /* 13: Reserved (0)                                                        */
#define E286_FLG_NT     0x00004000 /* 14: Nested Task                  Flag (Intel 80286 CPU and higher only) */
#define E286_FLG_BIT_15 0x00008000 /* 15: Reserved (0)                                                        */
#define E386_FLG_RF     0x00010000 /* 16: Return                       Flag (Intel 80386 CPU and higher only) */
#define E386_FLG_VM     0x00020000 /* 17: Virtual Machine              Flag (Intel 80386 CPU and higher only) */
#define E386_FLG_BIT_18 0x00040000 /* 18: Reserved (0)                                                        */
#define E386_FLG_BIT_19 0x00080000 /* 19: Reserved (0)                                                        */
#define E386_FLG_BIT_20 0x00100000 /* 20: Reserved (0)                                                        */
#define E386_FLG_BIT_21 0x00200000 /* 21: Reserved (0)                                                        */
#define E386_FLG_BIT_22 0x00400000 /* 22: Reserved (0)                                                        */
#define E386_FLG_BIT_23 0x00800000 /* 23: Reserved (0)                                                        */
#define E386_FLG_BIT_24 0x01000000 /* 24: Reserved (0)                                                        */
#define E386_FLG_BIT_25 0x02000000 /* 25: Reserved (0)                                                        */
#define E386_FLG_BIT_26 0x04000000 /* 26: Reserved (0)                                                        */
#define E386_FLG_BIT_27 0x08000000 /* 27: Reserved (0)                                                        */
#define E386_FLG_BIT_28 0x10000000 /* 28: Reserved (0)                                                        */
#define E386_FLG_BIT_29 0x20000000 /* 29: Reserved (0)                                                        */
#define E386_FLG_BIT_30 0x40000000 /* 30: Reserved (0)                                                        */
#define E386_FLG_BIT_31 0x80000000 /* 31: Reserved (0)                                                        */

/* -------------------------------------------------------------------------------------- */
/* IBM PC (Model 5150/5160): Quote from IBM PC Technical Reference (8088 Register Model): */
/* -------------------------------------------------------------------------------------- */
/* General Register File:                                                                 */
/*  - AX: AH: AL: Accumulator                                                             */
/*  - BX: BH: BL: Base                                                                    */
/*  - CX: CH: CL: Count                                                                   */
/*  - DX: DH: DL: Data                                                                    */
/* -------------------------------------------------------------------------------------- */
/*  - SP: Stack Pointer                                                                   */
/*  - BP: Base Pointer                                                                    */
/*  - SI: Source Index                                                                    */
/*  - DI: Destination Index                                                               */
/* -------------------------------------------------------------------------------------- */
/*  - IP            : Instruction Pointer                                                 */
/*  - FLAGSH: FLAGSL: Status Flags                                                        */
/* -------------------------------------------------------------------------------------- */
/* Segment Register File:                                                                 */
/*  - CS: Code Segment                                                                    */
/*  - DS: Data Segment                                                                    */
/*  - SS: Stack Segment                                                                   */
/*  - ES: Extra Segment                                                                   */
/* -------------------------------------------------------------------------------------- */
/* Instructions which reference the flag register file as a 16-bit object use the symbol  */
/* FLAGS to represent the file:                                                           */
/* -------------------------------------------------------------------------------------- */
/*         15                                  7                                0         */
/*         X   X   X   X   OF   DF   IF   TF   SF   ZF   X   AF   X   PF   X   CF         */
/*         x = Don't Care                                                                 */
/* -------------------------------------------------------------------------------------- */
/* 8080 Flags:                                                                            */
/*  - AF: Auxiliary Carry - BCD                                                           */
/*  - CF: Carry Flag                                                                      */
/*  - PF: Parity Flag                                                                     */
/*  - SF: Sign Flag                                                                       */
/*  - ZF: Zero Flag                                                                       */
/* -------------------------------------------------------------------------------------- */
/* 8088 Flags:                                                                            */
/*  - DF: Direction Flag (Strings)                                                        */
/*  - IF: Interrupt Enable Flag                                                           */
/*  - OF: Overflow Flag (CF + SF)                                                         */
/*  - TF: Trap - Single Step Flag                                                         */
/* -------------------------------------------------------------------------------------- */
/* 32-bit register values */
/*See 16-bit values*//*eAX CPU register (Intel 80386 CPU and higher) */
/*See 16-bit values*//*eCX CPU register (Intel 80386 CPU and higher) */
/*See 16-bit values*//*eDX CPU register (Intel 80386 CPU and higher) */
/*See 16-bit values*//*eBX CPU register (Intel 80386 CPU and higher) */
/*See 16-bit values*//*eSP CPU register (Intel 80386 CPU and higher) */
/*See 16-bit values*//*eBP CPU register (Intel 80386 CPU and higher) */
/*See 16-bit values*//*eSI CPU register (Intel 80386 CPU and higher) */
/*See 16-bit values*//*eDI CPU register (Intel 80386 CPU and higher) */
/* 16-bit register values */
#define E86_REG_AX 0 /* AX CPU register (Intel  8086 CPU and higher) */
#define E86_REG_CX 1 /* CX CPU register (Intel  8086 CPU and higher) */
#define E86_REG_DX 2 /* DX CPU register (Intel  8086 CPU and higher) */
#define E86_REG_BX 3 /* BX CPU register (Intel  8086 CPU and higher) */
#define E86_REG_SP 4 /* SP CPU register (Intel  8086 CPU and higher) */
#define E86_REG_BP 5 /* BP CPU register (Intel  8086 CPU and higher) */
#define E86_REG_SI 6 /* SI CPU register (Intel  8086 CPU and higher) */
#define E86_REG_DI 7 /* DI CPU register (Intel  8086 CPU and higher) */
/*  8-bit register values */
#define E86_REG_AL 0 /* AL CPU register (Intel  8080 CPU and higher) */
#define E86_REG_CL 1 /* CL CPU register (Intel  8080 CPU and higher) */
#define E86_REG_DL 2 /* DL CPU register (Intel  8080 CPU and higher) */
#define E86_REG_BL 3 /* BL CPU register (Intel  8080 CPU and higher) */
#define E86_REG_AH 4 /* AH CPU register (Intel  8080 CPU and higher) */
#define E86_REG_CH 5 /* CH CPU register (Intel  8080 CPU and higher) */
#define E86_REG_DH 6 /* DH CPU register (Intel  8080 CPU and higher) */
#define E86_REG_BH 7 /* BH CPU register (Intel  8080 CPU and higher) */

/* Segment register values */
#define E86_REG_ES 0  /* Extra Segment (Intel 8086/8088 CPU and higher ) */
#define E86_REG_CS 1  /* Code  Segment (Intel 8086/8088 CPU and higher ) */
#define E86_REG_SS 2  /* Stack Segment (Intel 8086/8088 CPU and higher ) */
#define E86_REG_DS 3  /* Data  Segment (Intel 8086/8088 CPU and higher ) */
/* Intel 80386 CPU and higher only */
#define E386_REG_ES 0 /* Required for MOV sreg, r/m16 (Opcode 8E) instruction */
#define E386_REG_CS 1 /* Required for MOV sreg, r/m16 (Opcode 8E) instruction */
#define E386_REG_SS 2 /* Required for MOV sreg, r/m16 (Opcode 8E) instruction */
#define E386_REG_DS 3 /* Required for MOV sreg, r/m16 (Opcode 8E) instruction */
#define E386_REG_FS 4 /* Extra Segment (Intel 80386 CPU and higher only) */
#define E386_REG_GS 5 /* Extra Segment (Intel 80386 CPU and higher only) */

#define E86_PREFIX_NEW  0x0001
#define E86_PREFIX_SEG  0x0002
#define E86_PREFIX_REP  0x0004
#define E86_PREFIX_REPN 0x0008
#define E86_PREFIX_LOCK 0x0010
#define E86_PREFIX_KEEP 0x0020

#define E86_PQ_MAX 16


struct e8086_t;


typedef unsigned char(*e86_get_uint8_f) (void *ext, unsigned long addr);
typedef unsigned short(*e86_get_uint16_f) (void *ext, unsigned long addr);
typedef unsigned long(*e86_get_uint32_f) (void *ext, unsigned long long addr);
typedef void(*e86_set_uint8_f) (void *ext, unsigned long addr, unsigned char val);
typedef void(*e86_set_uint16_f) (void *ext, unsigned long addr, unsigned short val);
typedef void(*e86_set_uint32_f) (void *ext, unsigned long long addr, unsigned long val);

typedef unsigned(*e86_opcode_f) (struct e8086_t *c);


typedef struct e8086_t {
	unsigned         cpu;
	unsigned         cpumodel;
	unsigned         e386_address_size_prefix;
	unsigned         e386_operand_size_prefix;
	unsigned         disassemble;
	unsigned         memory_clock_cycle_disable;/* Used by machines which use custom clock cycles */
	unsigned         memory_clock_cycle_count;  /* Used by machines which use custom clock cycles */
	unsigned         fs_segment;
	unsigned         gs_segment;
	unsigned long    dreg[8];
	unsigned long    sreg[4];
	unsigned long    sreg386[6];
	unsigned long    segment; /* Used for allocating segment descriptors */
	unsigned long    machine_status_word; /* Intel 80286 CPU and higher only */
	unsigned long    ip;  /* Instruction Pointer (IP) */
	unsigned long    flg; /* FLAGS register */
	void             *mem;
	e86_get_uint8_f  mem_get_uint8;
	e86_set_uint8_f  mem_set_uint8;
	e86_get_uint16_f mem_get_uint16;
	e86_set_uint16_f mem_set_uint16;
	e86_get_uint32_f mem_get_uint32;
	e86_set_uint32_f mem_set_uint32;
	void             *prt;
	e86_get_uint8_f  prt_get_uint8;
	e86_set_uint8_f  prt_set_uint8;
	e86_get_uint16_f prt_get_uint16;
	e86_set_uint16_f prt_set_uint16;
	e86_get_uint32_f prt_get_uint32;
	e86_set_uint32_f prt_set_uint32;
	unsigned char    *ram;
	unsigned long    ram_cnt;
	unsigned long    addr_mask;
	void             *inta_ext;
	unsigned char(*inta) (void *ext);
	void             *op_ext;
	void(*op_hook) (void *ext, unsigned char op1, unsigned char op2);
	void(*op_stat) (void *ext, unsigned char op1, unsigned char op2);
	void(*op_undef) (void *ext, unsigned char op1, unsigned char op2);
	void(*op_int) (void *ext, unsigned char n);
	unsigned short   cur_ip;
	unsigned         pq_size;
	unsigned         pq_fill;
	unsigned         pq_cnt;
	unsigned char    pq[E86_PQ_MAX];
	unsigned         prefix;
	unsigned long    seg_override; /* Formerly "unsigned short" */
	int              halt;
	char             irq;
	char             enable_int;
	unsigned         int_cnt;
	unsigned char    int_vec;
	unsigned short   int_cs;
	unsigned short   int_ip;
	unsigned         memory_management_unit_is_enabled_for_gdt;
	/* Used to enable and disable Memory Management Unit (MMU) for    Global Descriptor Table */
	unsigned         memory_management_unit_is_enabled_for_ldt;
	/* Used to enable and disable Memory Management Unit (MMU) for     Local Descriptor Table */
	unsigned         memory_management_unit_is_enabled_for_idt;
	/* Used to enable and disable Memory Management Unit (MMU) for Interrupt Descriptor Table */
	unsigned         memory_management_unit_is_enabled_for_tss;
	/* Used to enable and disable Memory Management Unit (MMU) for Task State Segment descriptors */
	unsigned         paging_unit_is_enabled;
	/* Used to enable and disable Paging Unit */
	e86_opcode_f     op[256];
	struct {
		int            is_mem;
		unsigned char  *data;
		unsigned long  seg;/* Memory address segment being specified *//*Formerly "unsigned short"*/
		unsigned long  ofs;/* Memory address offset  being specified *//*Formerly "unsigned short"*/
		unsigned short cnt;
	} ea;
	unsigned long    delay;
	unsigned long long clocks;
	unsigned long long instructions;
	/* Intel 80386 CPU and higher only */
	unsigned         current_privilege_level;/*Current privilege level (CPL):"0", "1", "2", or "3"*/
	/* CS segment register descriptor: */
	unsigned long  cs_segment_register_descriptor_base_address_offset;/* : 16-bit (80286)/32-bit  */
	                                                                  /*     (80386 and higher)   */
	unsigned long  cs_segment_register_descriptor_limit; /* Segment limit: 16-bit (80286)/32-bit  */
	                                                     /*                  (80386 and higher)   */
	/* NOTE: Change "segment_limit" back to "size" if necessary and required by source code       */
	unsigned       cs_segment_register_descriptor_granularity_bit; /* Segment granularity bit:    */
	unsigned       cs_segment_register_descriptor_big;             /* Big: "0" or "1"             */
	unsigned       cs_segment_register_descriptor_default;     /* Default: "0" or "1"             */
	unsigned       cs_segment_register_descriptor_x_;                /* X: "0" or "1"             */
	unsigned       cs_segment_register_descriptor_0_;                /* 0: "0" or "1"             */
	unsigned       cs_segment_register_descriptor_available;/*AVL(available for use by systems    */
	                                                        /*programmers):"0" or "1"             */
	unsigned       cs_segment_register_descriptor_is_present;  /* present: "1" if present and "0" */
	                                                           /* if not present                  */
	unsigned       cs_segment_register_descriptor_privilege_level; /* Descriptor privilege level: */
	                                                               /*      "1", "2", or "3"       */
	unsigned       cs_segment_register_type_of_segment_or_gate;    /* Type of Segment or Gate:    */
	unsigned       cs_segment_register_descriptor_type_1;          /* Segment type 1: "0" or "1"  */
	unsigned       cs_segment_register_descriptor_type_0;          /* Segment type 0: "0" or "1"  */
	unsigned       cs_segment_register_descriptor_type_c;/*Conforming: "0" for conforming or "1"  */
	                                                     /*for non-conforming:                    */
	unsigned       cs_segment_register_descriptor_type_e;          /* Segment type expand-down:   */
	                                                               /*                 "0" or "1"  */
	unsigned       cs_segment_register_descriptor_type_r;          /* Segment type readable:      */
	                                                               /*                 "0" or "1"  */
	unsigned       cs_segment_register_descriptor_type_w;          /* Segment type writable:      */
	                                                               /*                 "0" or "1"  */
	unsigned       cs_segment_register_descriptor_accessed;        /* Segment A (accessed):       */
	                                                               /*                 "0" or "1"  */
	unsigned       cs_segment_register_selector_table_indicator;   /* Segment table indicator     */
	unsigned long  cs_segment_register_selector_index;             /* Segment table indicator     */
	unsigned       cs_segment_register_selector_requester_privilege_level;
	/* DS segment register descriptor: */
	unsigned long  ds_segment_register_descriptor_base_address_offset;/* : 16-bit (80286)/32-bit  */
																	  /*     (80386 and higher)   */
	unsigned long  ds_segment_register_descriptor_limit; /* Segment limit: 16-bit (80286)/32-bit  */
														 /*                  (80386 and higher)   */
														 /* NOTE: Change "segment_limit" back to "size" if necessary and required by source code       */
	unsigned       ds_segment_register_descriptor_granularity_bit; /* Segment granularity bit:    */
	unsigned       ds_segment_register_descriptor_big;             /* Big: "0" or "1"             */
	unsigned       ds_segment_register_descriptor_default;     /* Default: "0" or "1"             */
	unsigned       ds_segment_register_descriptor_x_;                /* X: "0" or "1"             */
	unsigned       ds_segment_register_descriptor_0_;                /* 0: "0" or "1"             */
	unsigned       ds_segment_register_descriptor_available;/*AVL(available for use by systems    */
															/*programmers):"0" or "1"             */
	unsigned       ds_segment_register_descriptor_is_present;  /* present: "1" if present and "0" */
															   /* if not present                  */
	unsigned       ds_segment_register_descriptor_privilege_level; /* Descriptor privilege level: */
																   /*      "1", "2", or "3"       */
	unsigned       ds_segment_register_type_of_segment_or_gate;    /* Type of Segment or Gate:    */
	unsigned       ds_segment_register_descriptor_type_1;          /* Segment type 1: "0" or "1"  */
	unsigned       ds_segment_register_descriptor_type_0;          /* Segment type 0: "0" or "1"  */
	unsigned       ds_segment_register_descriptor_type_c;/*Conforming: "0" for conforming or "1"  */
														 /*for non-conforming:                    */
	unsigned       ds_segment_register_descriptor_type_e;          /* Segment type expand-down:   */
																   /*                 "0" or "1"  */
	unsigned       ds_segment_register_descriptor_type_r;          /* Segment type readable:      */
																   /*                 "0" or "1"  */
	unsigned       ds_segment_register_descriptor_type_w;          /* Segment type writable:      */
																   /*                 "0" or "1"  */
	unsigned       ds_segment_register_descriptor_accessed;        /* Segment A (accessed):       */
																   /*                 "0" or "1"  */
	unsigned       ds_segment_register_selector_table_indicator;   /* Segment table indicator     */
	unsigned long  ds_segment_register_selector_index;             /* Segment table indicator     */
	unsigned       ds_segment_register_selector_requester_privilege_level;
	/* ES segment register descriptor: */
	unsigned long  es_segment_register_descriptor_base_address_offset;/* : 16-bit (80286)/32-bit  */
																	  /*     (80386 and higher)   */
	unsigned long  es_segment_register_descriptor_limit; /* Segment limit: 16-bit (80286)/32-bit  */
														 /*                  (80386 and higher)   */
														 /* NOTE: Change "segment_limit" back to "size" if necessary and required by source code       */
	unsigned       es_segment_register_descriptor_granularity_bit; /* Segment granularity bit:    */
	unsigned       es_segment_register_descriptor_big;             /* Big: "0" or "1"             */
	unsigned       es_segment_register_descriptor_default;     /* Default: "0" or "1"             */
	unsigned       es_segment_register_descriptor_x_;                /* X: "0" or "1"             */
	unsigned       es_segment_register_descriptor_0_;                /* 0: "0" or "1"             */
	unsigned       es_segment_register_descriptor_available;/*AVL(available for use by systems    */
															/*programmers):"0" or "1"             */
	unsigned       es_segment_register_descriptor_is_present;  /* present: "1" if present and "0" */
															   /* if not present                  */
	unsigned       es_segment_register_descriptor_privilege_level; /* Descriptor privilege level: */
																   /*      "1", "2", or "3"       */
	unsigned       es_segment_register_type_of_segment_or_gate;    /* Type of Segment or Gate:    */
	unsigned       es_segment_register_descriptor_type_1;          /* Segment type 1: "0" or "1"  */
	unsigned       es_segment_register_descriptor_type_0;          /* Segment type 0: "0" or "1"  */
	unsigned       es_segment_register_descriptor_type_c;/*Conforming: "0" for conforming or "1"  */
														 /*for non-conforming:                    */
	unsigned       es_segment_register_descriptor_type_e;          /* Segment type expand-down:   */
																   /*                 "0" or "1"  */
	unsigned       es_segment_register_descriptor_type_r;          /* Segment type readable:      */
																   /*                 "0" or "1"  */
	unsigned       es_segment_register_descriptor_type_w;          /* Segment type writable:      */
																   /*                 "0" or "1"  */
	unsigned       es_segment_register_descriptor_accessed;        /* Segment A (accessed):       */
																   /*                 "0" or "1"  */
	unsigned       es_segment_register_selector_table_indicator;   /* Segment table indicator     */
	unsigned long  es_segment_register_selector_index;             /* Segment table indicator     */
	unsigned       es_segment_register_selector_requester_privilege_level;
	/* FS segment register descriptor: */
	unsigned long  fs_segment_register_descriptor_base_address_offset;/* : 16-bit (80286)/32-bit  */
																	  /*     (80386 and higher)   */
	unsigned long  fs_segment_register_descriptor_limit; /* Segment limit: 16-bit (80286)/32-bit  */
														 /*                  (80386 and higher)   */
														 /* NOTE: Change "segment_limit" back to "size" if necessary and required by source code       */
	unsigned       fs_segment_register_descriptor_granularity_bit; /* Segment granularity bit:    */
	unsigned       fs_segment_register_descriptor_big;             /* Big: "0" or "1"             */
	unsigned       fs_segment_register_descriptor_default;     /* Default: "0" or "1"             */
	unsigned       fs_segment_register_descriptor_x_;                /* X: "0" or "1"             */
	unsigned       fs_segment_register_descriptor_0_;                /* 0: "0" or "1"             */
	unsigned       fs_segment_register_descriptor_available;/*AVL(available for use by systems    */
															/*programmers):"0" or "1"             */
	unsigned       fs_segment_register_descriptor_is_present;  /* present: "1" if present and "0" */
															   /* if not present                  */
	unsigned       fs_segment_register_descriptor_privilege_level; /* Descriptor privilege level: */
																   /*      "1", "2", or "3"       */
	unsigned       fs_segment_register_type_of_segment_or_gate;    /* Type of Segment or Gate:    */
	unsigned       fs_segment_register_descriptor_type_1;          /* Segment type 1: "0" or "1"  */
	unsigned       fs_segment_register_descriptor_type_0;          /* Segment type 0: "0" or "1"  */
	unsigned       fs_segment_register_descriptor_type_c;/*Conforming: "0" for conforming or "1"  */
														 /*for non-conforming:                    */
	unsigned       fs_segment_register_descriptor_type_e;          /* Segment type expand-down:   */
																   /*                 "0" or "1"  */
	unsigned       fs_segment_register_descriptor_type_r;          /* Segment type readable:      */
																   /*                 "0" or "1"  */
	unsigned       fs_segment_register_descriptor_type_w;          /* Segment type writable:      */
																   /*                 "0" or "1"  */
	unsigned       fs_segment_register_descriptor_accessed;        /* Segment A (accessed):       */
																   /*                 "0" or "1"  */
	unsigned       fs_segment_register_selector_table_indicator;   /* Segment table indicator     */
	unsigned long  fs_segment_register_selector_index;             /* Segment table indicator     */
	unsigned       fs_segment_register_selector_requester_privilege_level;
	/* GS segment register descriptor: */
	unsigned long  gs_segment_register_descriptor_base_address_offset;/* : 16-bit (80286)/32-bit  */
																	  /*     (80386 and higher)   */
	unsigned long  gs_segment_register_descriptor_limit; /* Segment limit: 16-bit (80286)/32-bit  */
														 /*                  (80386 and higher)   */
														 /* NOTE: Change "segment_limit" back to "size" if necessary and required by source code       */
	unsigned       gs_segment_register_descriptor_granularity_bit; /* Segment granularity bit:    */
	unsigned       gs_segment_register_descriptor_big;             /* Big: "0" or "1"             */
	unsigned       gs_segment_register_descriptor_default;     /* Default: "0" or "1"             */
	unsigned       gs_segment_register_descriptor_x_;                /* X: "0" or "1"             */
	unsigned       gs_segment_register_descriptor_0_;                /* 0: "0" or "1"             */
	unsigned       gs_segment_register_descriptor_available;/*AVL(available for use by systems    */
															/*programmers):"0" or "1"             */
	unsigned       gs_segment_register_descriptor_is_present;  /* present: "1" if present and "0" */
															   /* if not present                  */
	unsigned       gs_segment_register_descriptor_privilege_level; /* Descriptor privilege level: */
																   /*      "1", "2", or "3"       */
	unsigned       gs_segment_register_type_of_segment_or_gate;    /* Type of Segment or Gate:    */
	unsigned       gs_segment_register_descriptor_type_1;          /* Segment type 1: "0" or "1"  */
	unsigned       gs_segment_register_descriptor_type_0;          /* Segment type 0: "0" or "1"  */
	unsigned       gs_segment_register_descriptor_type_c;/*Conforming: "0" for conforming or "1"  */
														 /*for non-conforming:                    */
	unsigned       gs_segment_register_descriptor_type_e;          /* Segment type expand-down:   */
																   /*                 "0" or "1"  */
	unsigned       gs_segment_register_descriptor_type_r;          /* Segment type readable:      */
																   /*                 "0" or "1"  */
	unsigned       gs_segment_register_descriptor_type_w;          /* Segment type writable:      */
																   /*                 "0" or "1"  */
	unsigned       gs_segment_register_descriptor_accessed;        /* Segment A (accessed):       */
																   /*                 "0" or "1"  */
	unsigned       gs_segment_register_selector_table_indicator;   /* Segment table indicator     */
	unsigned long  gs_segment_register_selector_index;             /* Segment table indicator     */
	unsigned       gs_segment_register_selector_requester_privilege_level;
	/* SS segment register descriptor: */
	unsigned long  ss_segment_register_descriptor_base_address_offset;/* : 16-bit (80286)/32-bit  */
																	  /*     (80386 and higher)   */
	unsigned long  ss_segment_register_descriptor_limit; /* Segment limit: 16-bit (80286)/32-bit  */
														 /*                  (80386 and higher)   */
														 /* NOTE: Change "segment_limit" back to "size" if necessary and required by source code       */
	unsigned       ss_segment_register_descriptor_granularity_bit; /* Segment granularity bit:    */
	unsigned       ss_segment_register_descriptor_big;             /* Big: "0" or "1"             */
	unsigned       ss_segment_register_descriptor_default;     /* Default: "0" or "1"             */
	unsigned       ss_segment_register_descriptor_x_;                /* X: "0" or "1"             */
	unsigned       ss_segment_register_descriptor_0_;                /* 0: "0" or "1"             */
	unsigned       ss_segment_register_descriptor_available;/*AVL(available for use by systems    */
															/*programmers):"0" or "1"             */
	unsigned       ss_segment_register_descriptor_is_present;  /* present: "1" if present and "0" */
															   /* if not present                  */
	unsigned       ss_segment_register_descriptor_privilege_level; /* Descriptor privilege level: */
																   /*      "1", "2", or "3"       */
	unsigned       ss_segment_register_type_of_segment_or_gate;    /* Type of Segment or Gate:    */
	unsigned       ss_segment_register_descriptor_type_1;          /* Segment type 1: "0" or "1"  */
	unsigned       ss_segment_register_descriptor_type_0;          /* Segment type 0: "0" or "1"  */
	unsigned       ss_segment_register_descriptor_type_c;/*Conforming: "0" for conforming or "1"  */
														 /*for non-conforming:                    */
	unsigned       ss_segment_register_descriptor_type_e;          /* Segment type expand-down:   */
																   /*                 "0" or "1"  */
	unsigned       ss_segment_register_descriptor_type_r;          /* Segment type readable:      */
																   /*                 "0" or "1"  */
	unsigned       ss_segment_register_descriptor_type_w;          /* Segment type writable:      */
																   /*                 "0" or "1"  */
	unsigned       ss_segment_register_descriptor_accessed;        /* Segment A (accessed):       */
																   /*                 "0" or "1"  */
	unsigned       ss_segment_register_selector_table_indicator;   /* Segment table indicator     */
	unsigned long  ss_segment_register_selector_index;             /* Segment table indicator     */
	unsigned       ss_segment_register_selector_requester_privilege_level;
	/* Privilege Level 2: */
	unsigned long  ss_segment_register_descriptor_base_address_offset_for_privilege_level_2;
	unsigned long  ss_segment_register_descriptor_limit_for_privilege_level_2;
	unsigned       ss_segment_register_descriptor_granularity_bit_for_privilege_level_2;
	unsigned       ss_segment_register_descriptor_big_for_privilege_level_2;
	unsigned       ss_segment_register_descriptor_default_for_privilege_level_2;
	unsigned       ss_segment_register_descriptor_x__for_privilege_level_2;
	unsigned       ss_segment_register_descriptor_0__for_privilege_level_2;
	unsigned       ss_segment_register_descriptor_available_for_privilege_level_2;
	unsigned       ss_segment_register_descriptor_is_present_for_privilege_level_2;
	unsigned       ss_segment_register_descriptor_privilege_level_for_privilege_level_2;
	unsigned       ss_segment_register_type_of_segment_or_gate_for_privilege_level_2;
	unsigned       ss_segment_register_descriptor_type_1_for_privilege_level_2;
	unsigned       ss_segment_register_descriptor_type_0_for_privilege_level_2;
	unsigned       ss_segment_register_descriptor_type_c_for_privilege_level_2;
	unsigned       ss_segment_register_descriptor_type_e_for_privilege_level_2;
	unsigned       ss_segment_register_descriptor_type_r_for_privilege_level_2;
	unsigned       ss_segment_register_descriptor_type_w_for_privilege_level_2;
	unsigned       ss_segment_register_descriptor_accessed_for_privilege_level_2;
	unsigned       ss_segment_register_selector_table_indicator_for_privilege_level_2;
	unsigned long  ss_segment_register_selector_index_for_privilege_level_2;
	unsigned       ss_segment_register_selector_requester_privilege_level_for_privilege_level_2;
	/* Privilege Level 1: */
	unsigned long  ss_segment_register_descriptor_base_address_offset_for_privilege_level_1;
	unsigned long  ss_segment_register_descriptor_limit_for_privilege_level_1;
	unsigned       ss_segment_register_descriptor_granularity_bit_for_privilege_level_1;
	unsigned       ss_segment_register_descriptor_big_for_privilege_level_1;
	unsigned       ss_segment_register_descriptor_default_for_privilege_level_1;
	unsigned       ss_segment_register_descriptor_x__for_privilege_level_1;
	unsigned       ss_segment_register_descriptor_0__for_privilege_level_1;
	unsigned       ss_segment_register_descriptor_available_for_privilege_level_1;
	unsigned       ss_segment_register_descriptor_is_present_for_privilege_level_1;
	unsigned       ss_segment_register_descriptor_privilege_level_for_privilege_level_1;
	unsigned       ss_segment_register_type_of_segment_or_gate_for_privilege_level_1;
	unsigned       ss_segment_register_descriptor_type_1_for_privilege_level_1;
	unsigned       ss_segment_register_descriptor_type_0_for_privilege_level_1;
	unsigned       ss_segment_register_descriptor_type_c_for_privilege_level_1;
	unsigned       ss_segment_register_descriptor_type_e_for_privilege_level_1;
	unsigned       ss_segment_register_descriptor_type_r_for_privilege_level_1;
	unsigned       ss_segment_register_descriptor_type_w_for_privilege_level_1;
	unsigned       ss_segment_register_descriptor_accessed_for_privilege_level_1;
	unsigned       ss_segment_register_selector_table_indicator_for_privilege_level_1;
	unsigned long  ss_segment_register_selector_index_for_privilege_level_1;
	unsigned       ss_segment_register_selector_requester_privilege_level_for_privilege_level_1;
	/* Privilege Level 0: */
	unsigned long  ss_segment_register_descriptor_base_address_offset_for_privilege_level_0;
	unsigned long  ss_segment_register_descriptor_limit_for_privilege_level_0;
	unsigned       ss_segment_register_descriptor_granularity_bit_for_privilege_level_0;
	unsigned       ss_segment_register_descriptor_big_for_privilege_level_0;
	unsigned       ss_segment_register_descriptor_default_for_privilege_level_0;
	unsigned       ss_segment_register_descriptor_x__for_privilege_level_0;
	unsigned       ss_segment_register_descriptor_0__for_privilege_level_0;
	unsigned       ss_segment_register_descriptor_available_for_privilege_level_0;
	unsigned       ss_segment_register_descriptor_is_present_for_privilege_level_0;
	unsigned       ss_segment_register_descriptor_privilege_level_for_privilege_level_0;
	unsigned       ss_segment_register_type_of_segment_or_gate_for_privilege_level_0;
	unsigned       ss_segment_register_descriptor_type_1_for_privilege_level_0;
	unsigned       ss_segment_register_descriptor_type_0_for_privilege_level_0;
	unsigned       ss_segment_register_descriptor_type_c_for_privilege_level_0;
	unsigned       ss_segment_register_descriptor_type_e_for_privilege_level_0;
	unsigned       ss_segment_register_descriptor_type_r_for_privilege_level_0;
	unsigned       ss_segment_register_descriptor_type_w_for_privilege_level_0;
	unsigned       ss_segment_register_descriptor_accessed_for_privilege_level_0;
	unsigned       ss_segment_register_selector_table_indicator_for_privilege_level_0;
	unsigned long  ss_segment_register_selector_index_for_privilege_level_0;
	unsigned       ss_segment_register_selector_requester_privilege_level_for_privilege_level_0;
	/* CR3 Register (Current Page Table) */
	unsigned long  pdbr_page_frame_address_specified;    /* Page frame address:  32-bit (80386)   */
	unsigned long  pdbr_page_frame_address_being_loaded; /* Page frame address:  32-bit (80386)   */
	unsigned long  pdbr_page_frame_address;              /* Page frame address:  32-bit (80386)   */
	unsigned       pdbr_page_table_is_present;     /* Page table present: "1" if present and "0"  */
	                                               /*                         if not present      */
	unsigned       pdbr_page_table_read_write;     /* Page table read/write: "0" if read-only and */
	                                               /*                        "1" if read/write    */
	unsigned       pdbr_page_table_user_supervisor;/* Page table user/supervisor: "0" if user and */
	                                               /*                        "1" if supervisor:   */
	unsigned       pdbr_page_table_dirty;          /* Page table Default:    "0" or "1"           */
	unsigned       pdbr_page_table_available;      /* Page table available for systems programmer */
                                                   /*                   use: "0" or "1"           */
	/* GDTR Register For Global Descriptor Table */
	unsigned long  gdtr_register_for_global_descriptor_base_address_offset;            /* Segment base:  16-bit (80286)/32-bit (80386 and higher) */
	unsigned long  gdtr_register_for_global_descriptor_limit;           /* Segment limit: 16-bit (80286)/32-bit (80386 and higher) */
																		/* NOTE: Change "segment_limit" back to "size" if necessary and required by source code */
	unsigned       gdtr_register_for_global_descriptor_granularity_bit; /* Segment granularity bit: */
	unsigned       gdtr_register_for_global_descriptor_big;             /* Big: "0" or "1" */
	unsigned       gdtr_register_for_global_descriptor_default;           /* Default: "0" or "1" */
	unsigned       gdtr_register_for_global_descriptor_x_;              /* X: "0" or "1" */
	unsigned       gdtr_register_for_global_descriptor_0_;              /* 0: "0" or "1" */
	unsigned       gdtr_register_for_global_descriptor_available;       /* Segment AVL (available for use by systems programmers): "0" or "1" */
	unsigned       gdtr_register_for_global_descriptor_is_present;      /* Segment present: "1" if present and "0" if not present */
	unsigned       gdtr_register_for_global_descriptor_privilege_level; /* Descriptor privilege level: "1", "2", or "3" */
	unsigned       gdtr_register_for_global_descriptor_type_of_segment_or_gate;          /* Type of Segment or Gate: */
	unsigned       gdtr_register_for_global_descriptor_type_1;      /* Segment type 1: "0" or "1": */
	unsigned       gdtr_register_for_global_descriptor_type_0;      /* Segment type 0: "0" or "1": */
	unsigned       gdtr_register_for_global_descriptor_type_c;      /* Segment type conforming: "0" for conforming or "1" for non-conforming: */
	unsigned       gdtr_register_for_global_descriptor_type_e;          /* Segment type expand-down: "0" or "1" */
	unsigned       gdtr_register_for_global_descriptor_type_r;          /* Segment type readable: "0" or "1" */
	unsigned       gdtr_register_for_global_descriptor_type_w;          /* Segment type writable: "0" or "1" */
	unsigned       gdtr_register_for_global_descriptor_accessed;        /* Segment A (accessed): "0" or "1" */
	unsigned       gdtr_register_for_global_descriptor_selector_table_indicator;        /* Segment table indicator */
	unsigned long  gdtr_register_for_global_descriptor_selector_index;  /* Segment table indicator */
	unsigned       gdtr_register_for_global_descriptor_selector_requester_privilege_level;        /* Segment requester privilege level */
	unsigned long  gdtr_register_for_global_descriptor_callgate_offset;    /* Segment 80386 call gate offset */
	unsigned       gdtr_register_for_global_descriptor_callgate_is_present;/* Segment call gate present: "1" if present and "0" if not present */
	unsigned       gdtr_register_for_global_descriptor_callgate_type;      /* See "0110" for 80386 call gate */
	unsigned       gdtr_register_for_global_descriptor_callgate_000;       /* */
	unsigned       gdtr_register_for_global_descriptor_callgate_dword_count; /* Segment call gate DWORD count (32-bit) */
	/* LDTR Register For Local Descriptor Table */
	unsigned long  ldtr_register_for_local_descriptor_base_address_offset;            /* Segment base:  16-bit (80286)/32-bit (80386 and higher) */
	unsigned long  ldtr_register_for_local_descriptor_limit;           /* Segment limit: 16-bit (80286)/32-bit (80386 and higher) */
																	   /* NOTE: Change "segment_limit" back to "size" if necessary and required by source code */
	unsigned       ldtr_register_for_local_descriptor_granularity_bit; /* Segment granularity bit: */
	unsigned       ldtr_register_for_local_descriptor_big;             /* Big: "0" or "1" */
	unsigned       ldtr_register_for_local_descriptor_default;           /* Default: "0" or "1" */
	unsigned       ldtr_register_for_local_descriptor_x_;              /* X: "0" or "1" */
	unsigned       ldtr_register_for_local_descriptor_0_;              /* 0: "0" or "1" */
	unsigned       ldtr_register_for_local_descriptor_available;       /* Segment AVL (available for use by systems programmers): "0" or "1" */
	unsigned       ldtr_register_for_local_descriptor_is_present;      /* Segment present: "1" if present and "0" if not present */
	unsigned       ldtr_register_for_local_descriptor_privilege_level; /* Descriptor privilege level: "1", "2", or "3" */
	unsigned       ldtr_register_for_local_descriptor_type_of_segment_or_gate;          /* Type of Segment or Gate: */
	unsigned       ldtr_register_for_local_descriptor_type_1;      /* Segment type 1: "0" or "1": */
	unsigned       ldtr_register_for_local_descriptor_type_0;      /* Segment type 0: "0" or "1": */
	unsigned       ldtr_register_for_local_descriptor_type_c;      /* Segment type conforming: "0" for conforming or "1" for non-conforming: */
	unsigned       ldtr_register_for_local_descriptor_type_e;          /* Segment type expand-down: "0" or "1" */
	unsigned       ldtr_register_for_local_descriptor_type_r;          /* Segment type readable: "0" or "1" */
	unsigned       ldtr_register_for_local_descriptor_type_w;          /* Segment type writable: "0" or "1" */
	unsigned       ldtr_register_for_local_descriptor_accessed;        /* Segment A (accessed): "0" or "1" */
	unsigned       ldtr_register_for_local_descriptor_selector_table_indicator;        /* Segment table indicator */
	unsigned long  ldtr_register_for_local_descriptor_selector_index;  /* Segment table indicator */
	unsigned       ldtr_register_for_local_descriptor_selector_requester_privilege_level;        /* Segment requester privilege level */
	unsigned long  ldtr_register_for_local_descriptor_callgate_offset;    /* Segment 80386 call gate offset */
	unsigned       ldtr_register_for_local_descriptor_callgate_is_present;/* Segment call gate present: "1" if present and "0" if not present */
	unsigned       ldtr_register_for_local_descriptor_callgate_type;      /* See "0110" for 80386 call gate */
	unsigned       ldtr_register_for_local_descriptor_callgate_000;       /* */
	unsigned       ldtr_register_for_local_descriptor_callgate_dword_count; /* Segment call gate DWORD count (32-bit) */
	/* IDTR Interrupt Register */
	unsigned long  idtr_gate_for_interrupt_register_selector;            /* Interrupt descriptor base:  16-bit (80286)/32-bit (80386 and higher) */
	unsigned long  idtr_gate_for_interrupt_register_offset;           /* Interrupt descriptor limit: 16-bit (80286)/32-bit (80386 and higher) */
	unsigned long  idtr_gate_for_interrupt_register_limit;
	unsigned       idtr_gate_for_interrupt_register_x_;            /* Segment type 1: "0" or "1": */
	unsigned       idtr_gate_for_interrupt_register_0_;            /* Segment type 1: "0" or "1": */
	unsigned       idtr_gate_for_interrupt_register_type;            /* Segment type 1: "0" or "1": */
	unsigned       idtr_gate_for_interrupt_register_type_b;            /* Segment type 1: "0" or "1": */
	unsigned       idtr_gate_for_interrupt_register_default;            /* Segment type 1: "0" or "1": */
	unsigned       idtr_gate_for_interrupt_register_type_1;          /* Segment type 1: "0" or "1": */
	unsigned       idtr_gate_for_interrupt_register_type_0;          /* Segment type 0: "0" or "1": */
	unsigned       idtr_gate_for_interrupt_register_type_c;          /* Segment type conforming: "0" for conforming or "1" for non-conforming: */
	unsigned       idtr_gate_for_interrupt_register_type_e;          /* Segment type expand-down: "0" or "1" */
	unsigned       idtr_gate_for_interrupt_register_type_r;          /* Segment type readable: "0" or "1" */
	unsigned       idtr_gate_for_interrupt_register_type_w;          /* Segment type writable: "0" or "1" */
	unsigned       idtr_gate_for_interrupt_register_present;          /* Present: */
	unsigned       idtr_gate_for_interrupt_register_descriptor_privilege_level;          /* Descriptor Privilege Level: */
	unsigned       idtr_gate_for_interrupt_register_selector_table_indicator;          /* Present: */
	unsigned       idtr_gate_for_interrupt_register_selector_requester_privilege_level;          /* Descriptor Privilege Level: */
	unsigned       idtr_gate_for_interrupt_register_granularity_bit;
	unsigned       idtr_gate_for_interrupt_register_available;          /* Present: */
	unsigned       idtr_gate_for_interrupt_register_accessed;          /* Present: */
	unsigned       idtr_gate_for_interrupt_register_interrupt_n_;          /* Gate for Interrupt #N: */
	unsigned       idtr_gate_for_interrupt_register_interrupt_2_;          /* Gate for Interrupt #2: */
	unsigned       idtr_gate_for_interrupt_register_interrupt_1_;          /* Gate for Interrupt #1: */
	unsigned       idtr_gate_for_interrupt_register_interrupt_0_;          /* Gate for Interrupt #0: */
	/* IDTR Trap Register */
	unsigned long  idtr_gate_for_trap_register_selector;            /* Interrupt descriptor base:  16-bit (80286)/32-bit (80386 and higher) */
	unsigned long  idtr_gate_for_trap_register_offset;           /* Interrupt descriptor limit: 16-bit (80286)/32-bit (80386 and higher) */
	unsigned long  idtr_gate_for_trap_register_limit;
	unsigned       idtr_gate_for_trap_register_x_;            /* Segment type 1: "0" or "1": */
	unsigned       idtr_gate_for_trap_register_0_;            /* Segment type 1: "0" or "1": */
	unsigned       idtr_gate_for_trap_register_type;            /* Segment type 1: "0" or "1": */
	unsigned       idtr_gate_for_trap_register_type_b;            /* Segment type 1: "0" or "1": */
	unsigned       idtr_gate_for_trap_register_default;            /* Segment type 1: "0" or "1": */
	unsigned       idtr_gate_for_trap_register_type_1;          /* Segment type 1: "0" or "1": */
	unsigned       idtr_gate_for_trap_register_type_0;          /* Segment type 0: "0" or "1": */
	unsigned       idtr_gate_for_trap_register_type_c;          /* Segment type conforming: "0" for conforming or "1" for non-conforming: */
	unsigned       idtr_gate_for_trap_register_type_e;          /* Segment type expand-down: "0" or "1" */
	unsigned       idtr_gate_for_trap_register_type_r;          /* Segment type readable: "0" or "1" */
	unsigned       idtr_gate_for_trap_register_type_w;          /* Segment type writable: "0" or "1" */
	unsigned       idtr_gate_for_trap_register_present;          /* Present: */
	unsigned       idtr_gate_for_trap_register_descriptor_privilege_level;          /* Descriptor Privilege Level: */
	unsigned       idtr_gate_for_trap_register_selector_table_indicator;          /* Present: */
	unsigned       idtr_gate_for_trap_register_selector_requester_privilege_level;          /* Descriptor Privilege Level: */
	unsigned       idtr_gate_for_trap_register_granularity_bit;
	unsigned       idtr_gate_for_trap_register_available;          /* Present: */
	unsigned       idtr_gate_for_trap_register_accessed;          /* Present: */
	unsigned       idtr_gate_for_trap_register_interrupt_n_;          /* Gate for Interrupt #N: */
	unsigned       idtr_gate_for_trap_register_interrupt_2_;          /* Gate for Interrupt #2: */
	unsigned       idtr_gate_for_trap_register_interrupt_1_;          /* Gate for Interrupt #1: */
	unsigned       idtr_gate_for_trap_register_interrupt_0_;          /* Gate for Interrupt #0: */
	/* Exception Handler */
	unsigned long  exception_handler_selector_index;            /* Selector index for faulting instruction: */
																/* NOTE: Change "idt_limit" back to "size" if necessary and required by source code */
																/* unsigned char  alloc; */
	unsigned       exception_handler_table_indicator;           /* Table indicator for faulting instruction: */
	unsigned       exception_handler_interrupt_descriptor_table;/* Set if index portion of error code refers to a trap gate in the IDT: */
	unsigned       exception_handler_external;                  /* Set if events external to the running program caused the faulting instruction */
	unsigned       exception_handler_page_fault_user_supervisor;/* 0: Page fault in supervisor mode - 1: Page fault in user mode */
	unsigned       exception_handler_page_fault_write_read;     /* 0: Page fault during read - 1: Page fault during write */
	unsigned       exception_handler_page_fault_present;        /* 0: Page fault caused by non-present page - 1: Page fault caused by page-level
																protection violation */
	unsigned long  exception_handler_cr2_page_fault_linear_address; /* Value of control register 2 during page switch that occurred during task switch */
	/* Number Of Descriptor Tables In Use */
	unsigned short idt_number_of_interrupt_segment_descriptors_in_use; /* Number of segment descriptors in use for Interrupt Descriptor Table */
	/* Translation Lookaside Buffer Set 11: */
	unsigned long  tlb_set_11_entry_8_tag_7;
	unsigned long  tlb_set_11_entry_8_data_7;
	unsigned       tlb_set_11_entry_8_tr6_test_command_register_command_bit;
	unsigned long  tlb_set_11_entry_8_tr6_test_command_register_physical_address;
	unsigned long  tlb_set_11_entry_8_tr6_test_command_register_linear_address;
	unsigned       tlb_set_11_entry_8_tr6_test_command_register_valid_bit;
	unsigned       tlb_set_11_entry_8_tr6_test_command_register_dirty_bit_d;
	unsigned       tlb_set_11_entry_8_tr6_test_command_register_dirty_bit_d_number;
	unsigned       tlb_set_11_entry_8_tr6_test_command_register_user_supervisor_bit_u;
	unsigned       tlb_set_11_entry_8_tr6_test_command_register_user_supervisor_bit_u_number;
	unsigned       tlb_set_11_entry_8_tr6_test_command_register_read_write_bit_w;
	unsigned       tlb_set_11_entry_8_tr6_test_command_register_read_write_bit_w_number;
	unsigned long  tlb_set_11_entry_8_tr7_test_data_register_physical_address;
	unsigned       tlb_set_11_entry_8_tr7_test_data_register_ht_bit;
	unsigned       tlb_set_11_entry_8_tr7_test_data_register_rep_bit_d;
	unsigned long  tlb_set_11_entry_7_tag_6;
	unsigned long  tlb_set_11_entry_7_data_6;
	unsigned       tlb_set_11_entry_7_tr6_test_command_register_command_bit;
	unsigned long  tlb_set_11_entry_7_tr6_test_command_register_physical_address;
	unsigned long  tlb_set_11_entry_7_tr6_test_command_register_linear_address;
	unsigned       tlb_set_11_entry_7_tr6_test_command_register_valid_bit;
	unsigned       tlb_set_11_entry_7_tr6_test_command_register_dirty_bit_d;
	unsigned       tlb_set_11_entry_7_tr6_test_command_register_dirty_bit_d_number;
	unsigned       tlb_set_11_entry_7_tr6_test_command_register_user_supervisor_bit_u;
	unsigned       tlb_set_11_entry_7_tr6_test_command_register_user_supervisor_bit_u_number;
	unsigned       tlb_set_11_entry_7_tr6_test_command_register_read_write_bit_w;
	unsigned       tlb_set_11_entry_7_tr6_test_command_register_read_write_bit_w_number;
	unsigned long  tlb_set_11_entry_7_tr7_test_data_register_physical_address;
	unsigned       tlb_set_11_entry_7_tr7_test_data_register_ht_bit;
	unsigned       tlb_set_11_entry_7_tr7_test_data_register_rep_bit_d;
	unsigned long  tlb_set_11_entry_6_tag_5;
	unsigned long  tlb_set_11_entry_6_data_5;
	unsigned       tlb_set_11_entry_6_tr6_test_command_register_command_bit;
	unsigned long  tlb_set_11_entry_6_tr6_test_command_register_physical_address;
	unsigned long  tlb_set_11_entry_6_tr6_test_command_register_linear_address;
	unsigned       tlb_set_11_entry_6_tr6_test_command_register_valid_bit;
	unsigned       tlb_set_11_entry_6_tr6_test_command_register_dirty_bit_d;
	unsigned       tlb_set_11_entry_6_tr6_test_command_register_dirty_bit_d_number;
	unsigned       tlb_set_11_entry_6_tr6_test_command_register_user_supervisor_bit_u;
	unsigned       tlb_set_11_entry_6_tr6_test_command_register_user_supervisor_bit_u_number;
	unsigned       tlb_set_11_entry_6_tr6_test_command_register_read_write_bit_w;
	unsigned       tlb_set_11_entry_6_tr6_test_command_register_read_write_bit_w_number;
	unsigned long  tlb_set_11_entry_6_tr7_test_data_register_physical_address;
	unsigned       tlb_set_11_entry_6_tr7_test_data_register_ht_bit;
	unsigned       tlb_set_11_entry_6_tr7_test_data_register_rep_bit_d;
	unsigned long  tlb_set_11_entry_5_tag_4;
	unsigned long  tlb_set_11_entry_5_data_4;
	unsigned       tlb_set_11_entry_5_tr6_test_command_register_command_bit;
	unsigned long  tlb_set_11_entry_5_tr6_test_command_register_physical_address;
	unsigned long  tlb_set_11_entry_5_tr6_test_command_register_linear_address;
	unsigned       tlb_set_11_entry_5_tr6_test_command_register_valid_bit;
	unsigned       tlb_set_11_entry_5_tr6_test_command_register_dirty_bit_d;
	unsigned       tlb_set_11_entry_5_tr6_test_command_register_dirty_bit_d_number;
	unsigned       tlb_set_11_entry_5_tr6_test_command_register_user_supervisor_bit_u;
	unsigned       tlb_set_11_entry_5_tr6_test_command_register_user_supervisor_bit_u_number;
	unsigned       tlb_set_11_entry_5_tr6_test_command_register_read_write_bit_w;
	unsigned       tlb_set_11_entry_5_tr6_test_command_register_read_write_bit_w_number;
	unsigned long  tlb_set_11_entry_5_tr7_test_data_register_physical_address;
	unsigned       tlb_set_11_entry_5_tr7_test_data_register_ht_bit;
	unsigned       tlb_set_11_entry_5_tr7_test_data_register_rep_bit_d;
	unsigned long  tlb_set_11_entry_4_tag_3;
	unsigned long  tlb_set_11_entry_4_data_3;
	unsigned       tlb_set_11_entry_4_tr6_test_command_register_command_bit;
	unsigned long  tlb_set_11_entry_4_tr6_test_command_register_physical_address;
	unsigned long  tlb_set_11_entry_4_tr6_test_command_register_linear_address;
	unsigned       tlb_set_11_entry_4_tr6_test_command_register_valid_bit;
	unsigned       tlb_set_11_entry_4_tr6_test_command_register_dirty_bit_d;
	unsigned       tlb_set_11_entry_4_tr6_test_command_register_dirty_bit_d_number;
	unsigned       tlb_set_11_entry_4_tr6_test_command_register_user_supervisor_bit_u;
	unsigned       tlb_set_11_entry_4_tr6_test_command_register_user_supervisor_bit_u_number;
	unsigned       tlb_set_11_entry_4_tr6_test_command_register_read_write_bit_w;
	unsigned       tlb_set_11_entry_4_tr6_test_command_register_read_write_bit_w_number;
	unsigned long  tlb_set_11_entry_4_tr7_test_data_register_physical_address;
	unsigned       tlb_set_11_entry_4_tr7_test_data_register_ht_bit;
	unsigned       tlb_set_11_entry_4_tr7_test_data_register_rep_bit_d;
	unsigned long  tlb_set_11_entry_3_tag_2;
	unsigned long  tlb_set_11_entry_3_data_2;
	unsigned       tlb_set_11_entry_3_tr6_test_command_register_command_bit;
	unsigned long  tlb_set_11_entry_3_tr6_test_command_register_physical_address;
	unsigned long  tlb_set_11_entry_3_tr6_test_command_register_linear_address;
	unsigned       tlb_set_11_entry_3_tr6_test_command_register_valid_bit;
	unsigned       tlb_set_11_entry_3_tr6_test_command_register_dirty_bit_d;
	unsigned       tlb_set_11_entry_3_tr6_test_command_register_dirty_bit_d_number;
	unsigned       tlb_set_11_entry_3_tr6_test_command_register_user_supervisor_bit_u;
	unsigned       tlb_set_11_entry_3_tr6_test_command_register_user_supervisor_bit_u_number;
	unsigned       tlb_set_11_entry_3_tr6_test_command_register_read_write_bit_w;
	unsigned       tlb_set_11_entry_3_tr6_test_command_register_read_write_bit_w_number;
	unsigned long  tlb_set_11_entry_3_tr7_test_data_register_physical_address;
	unsigned       tlb_set_11_entry_3_tr7_test_data_register_ht_bit;
	unsigned       tlb_set_11_entry_3_tr7_test_data_register_rep_bit_d;
	unsigned long  tlb_set_11_entry_2_tag_1;
	unsigned long  tlb_set_11_entry_2_data_1;
	unsigned       tlb_set_11_entry_2_tr6_test_command_register_command_bit;
	unsigned long  tlb_set_11_entry_2_tr6_test_command_register_physical_address;
	unsigned long  tlb_set_11_entry_2_tr6_test_command_register_linear_address;
	unsigned       tlb_set_11_entry_2_tr6_test_command_register_valid_bit;
	unsigned       tlb_set_11_entry_2_tr6_test_command_register_dirty_bit_d;
	unsigned       tlb_set_11_entry_2_tr6_test_command_register_dirty_bit_d_number;
	unsigned       tlb_set_11_entry_2_tr6_test_command_register_user_supervisor_bit_u;
	unsigned       tlb_set_11_entry_2_tr6_test_command_register_user_supervisor_bit_u_number;
	unsigned       tlb_set_11_entry_2_tr6_test_command_register_read_write_bit_w;
	unsigned       tlb_set_11_entry_2_tr6_test_command_register_read_write_bit_w_number;
	unsigned long  tlb_set_11_entry_2_tr7_test_data_register_physical_address;
	unsigned       tlb_set_11_entry_2_tr7_test_data_register_ht_bit;
	unsigned       tlb_set_11_entry_2_tr7_test_data_register_rep_bit_d;
	unsigned long  tlb_set_11_entry_1_tag_0;
	unsigned long  tlb_set_11_entry_1_data_0;
	unsigned       tlb_set_11_entry_1_tr6_test_command_register_command_bit;
	unsigned long  tlb_set_11_entry_1_tr6_test_command_register_physical_address;
	unsigned long  tlb_set_11_entry_1_tr6_test_command_register_linear_address;
	unsigned       tlb_set_11_entry_1_tr6_test_command_register_valid_bit;
	unsigned       tlb_set_11_entry_1_tr6_test_command_register_dirty_bit_d;
	unsigned       tlb_set_11_entry_1_tr6_test_command_register_dirty_bit_d_number;
	unsigned       tlb_set_11_entry_1_tr6_test_command_register_user_supervisor_bit_u;
	unsigned       tlb_set_11_entry_1_tr6_test_command_register_user_supervisor_bit_u_number;
	unsigned       tlb_set_11_entry_1_tr6_test_command_register_read_write_bit_w;
	unsigned       tlb_set_11_entry_1_tr6_test_command_register_read_write_bit_w_number;
	unsigned long  tlb_set_11_entry_1_tr7_test_data_register_physical_address;
	unsigned       tlb_set_11_entry_1_tr7_test_data_register_ht_bit;
	unsigned       tlb_set_11_entry_1_tr7_test_data_register_rep_bit_d;
	/* Translation Lookaside Buffer Set 10: */
	unsigned long  tlb_set_10_entry_8_tag_7;
	unsigned long  tlb_set_10_entry_8_data_7;
	unsigned       tlb_set_10_entry_8_tr6_test_command_register_command_bit;
	unsigned long  tlb_set_10_entry_8_tr6_test_command_register_physical_address;
	unsigned long  tlb_set_10_entry_8_tr6_test_command_register_linear_address;
	unsigned       tlb_set_10_entry_8_tr6_test_command_register_valid_bit;
	unsigned       tlb_set_10_entry_8_tr6_test_command_register_dirty_bit_d;
	unsigned       tlb_set_10_entry_8_tr6_test_command_register_dirty_bit_d_number;
	unsigned       tlb_set_10_entry_8_tr6_test_command_register_user_supervisor_bit_u;
	unsigned       tlb_set_10_entry_8_tr6_test_command_register_user_supervisor_bit_u_number;
	unsigned       tlb_set_10_entry_8_tr6_test_command_register_read_write_bit_w;
	unsigned       tlb_set_10_entry_8_tr6_test_command_register_read_write_bit_w_number;
	unsigned long  tlb_set_10_entry_8_tr7_test_data_register_physical_address;
	unsigned       tlb_set_10_entry_8_tr7_test_data_register_ht_bit;
	unsigned       tlb_set_10_entry_8_tr7_test_data_register_rep_bit_d;
	unsigned long  tlb_set_10_entry_7_tag_6;
	unsigned long  tlb_set_10_entry_7_data_6;
	unsigned       tlb_set_10_entry_7_tr6_test_command_register_command_bit;
	unsigned long  tlb_set_10_entry_7_tr6_test_command_register_physical_address;
	unsigned long  tlb_set_10_entry_7_tr6_test_command_register_linear_address;
	unsigned       tlb_set_10_entry_7_tr6_test_command_register_valid_bit;
	unsigned       tlb_set_10_entry_7_tr6_test_command_register_dirty_bit_d;
	unsigned       tlb_set_10_entry_7_tr6_test_command_register_dirty_bit_d_number;
	unsigned       tlb_set_10_entry_7_tr6_test_command_register_user_supervisor_bit_u;
	unsigned       tlb_set_10_entry_7_tr6_test_command_register_user_supervisor_bit_u_number;
	unsigned       tlb_set_10_entry_7_tr6_test_command_register_read_write_bit_w;
	unsigned       tlb_set_10_entry_7_tr6_test_command_register_read_write_bit_w_number;
	unsigned long  tlb_set_10_entry_7_tr7_test_data_register_physical_address;
	unsigned       tlb_set_10_entry_7_tr7_test_data_register_ht_bit;
	unsigned       tlb_set_10_entry_7_tr7_test_data_register_rep_bit_d;
	unsigned long  tlb_set_10_entry_6_tag_5;
	unsigned long  tlb_set_10_entry_6_data_5;
	unsigned       tlb_set_10_entry_6_tr6_test_command_register_command_bit;
	unsigned long  tlb_set_10_entry_6_tr6_test_command_register_physical_address;
	unsigned long  tlb_set_10_entry_6_tr6_test_command_register_linear_address;
	unsigned       tlb_set_10_entry_6_tr6_test_command_register_valid_bit;
	unsigned       tlb_set_10_entry_6_tr6_test_command_register_dirty_bit_d;
	unsigned       tlb_set_10_entry_6_tr6_test_command_register_dirty_bit_d_number;
	unsigned       tlb_set_10_entry_6_tr6_test_command_register_user_supervisor_bit_u;
	unsigned       tlb_set_10_entry_6_tr6_test_command_register_user_supervisor_bit_u_number;
	unsigned       tlb_set_10_entry_6_tr6_test_command_register_read_write_bit_w;
	unsigned       tlb_set_10_entry_6_tr6_test_command_register_read_write_bit_w_number;
	unsigned long  tlb_set_10_entry_6_tr7_test_data_register_physical_address;
	unsigned       tlb_set_10_entry_6_tr7_test_data_register_ht_bit;
	unsigned       tlb_set_10_entry_6_tr7_test_data_register_rep_bit_d;
	unsigned long  tlb_set_10_entry_5_tag_4;
	unsigned long  tlb_set_10_entry_5_data_4;
	unsigned       tlb_set_10_entry_5_tr6_test_command_register_command_bit;
	unsigned long  tlb_set_10_entry_5_tr6_test_command_register_physical_address;
	unsigned long  tlb_set_10_entry_5_tr6_test_command_register_linear_address;
	unsigned       tlb_set_10_entry_5_tr6_test_command_register_valid_bit;
	unsigned       tlb_set_10_entry_5_tr6_test_command_register_dirty_bit_d;
	unsigned       tlb_set_10_entry_5_tr6_test_command_register_dirty_bit_d_number;
	unsigned       tlb_set_10_entry_5_tr6_test_command_register_user_supervisor_bit_u;
	unsigned       tlb_set_10_entry_5_tr6_test_command_register_user_supervisor_bit_u_number;
	unsigned       tlb_set_10_entry_5_tr6_test_command_register_read_write_bit_w;
	unsigned       tlb_set_10_entry_5_tr6_test_command_register_read_write_bit_w_number;
	unsigned long  tlb_set_10_entry_5_tr7_test_data_register_physical_address;
	unsigned       tlb_set_10_entry_5_tr7_test_data_register_ht_bit;
	unsigned       tlb_set_10_entry_5_tr7_test_data_register_rep_bit_d;
	unsigned long  tlb_set_10_entry_4_tag_3;
	unsigned long  tlb_set_10_entry_4_data_3;
	unsigned       tlb_set_10_entry_4_tr6_test_command_register_command_bit;
	unsigned long  tlb_set_10_entry_4_tr6_test_command_register_physical_address;
	unsigned long  tlb_set_10_entry_4_tr6_test_command_register_linear_address;
	unsigned       tlb_set_10_entry_4_tr6_test_command_register_valid_bit;
	unsigned       tlb_set_10_entry_4_tr6_test_command_register_dirty_bit_d;
	unsigned       tlb_set_10_entry_4_tr6_test_command_register_dirty_bit_d_number;
	unsigned       tlb_set_10_entry_4_tr6_test_command_register_user_supervisor_bit_u;
	unsigned       tlb_set_10_entry_4_tr6_test_command_register_user_supervisor_bit_u_number;
	unsigned       tlb_set_10_entry_4_tr6_test_command_register_read_write_bit_w;
	unsigned       tlb_set_10_entry_4_tr6_test_command_register_read_write_bit_w_number;
	unsigned long  tlb_set_10_entry_4_tr7_test_data_register_physical_address;
	unsigned       tlb_set_10_entry_4_tr7_test_data_register_ht_bit;
	unsigned       tlb_set_10_entry_4_tr7_test_data_register_rep_bit_d;
	unsigned long  tlb_set_10_entry_3_tag_2;
	unsigned long  tlb_set_10_entry_3_data_2;
	unsigned       tlb_set_10_entry_3_tr6_test_command_register_command_bit;
	unsigned long  tlb_set_10_entry_3_tr6_test_command_register_physical_address;
	unsigned long  tlb_set_10_entry_3_tr6_test_command_register_linear_address;
	unsigned       tlb_set_10_entry_3_tr6_test_command_register_valid_bit;
	unsigned       tlb_set_10_entry_3_tr6_test_command_register_dirty_bit_d;
	unsigned       tlb_set_10_entry_3_tr6_test_command_register_dirty_bit_d_number;
	unsigned       tlb_set_10_entry_3_tr6_test_command_register_user_supervisor_bit_u;
	unsigned       tlb_set_10_entry_3_tr6_test_command_register_user_supervisor_bit_u_number;
	unsigned       tlb_set_10_entry_3_tr6_test_command_register_read_write_bit_w;
	unsigned       tlb_set_10_entry_3_tr6_test_command_register_read_write_bit_w_number;
	unsigned long  tlb_set_10_entry_3_tr7_test_data_register_physical_address;
	unsigned       tlb_set_10_entry_3_tr7_test_data_register_ht_bit;
	unsigned       tlb_set_10_entry_3_tr7_test_data_register_rep_bit_d;
	unsigned long  tlb_set_10_entry_2_tag_1;
	unsigned long  tlb_set_10_entry_2_data_1;
	unsigned       tlb_set_10_entry_2_tr6_test_command_register_command_bit;
	unsigned long  tlb_set_10_entry_2_tr6_test_command_register_physical_address;
	unsigned long  tlb_set_10_entry_2_tr6_test_command_register_linear_address;
	unsigned       tlb_set_10_entry_2_tr6_test_command_register_valid_bit;
	unsigned       tlb_set_10_entry_2_tr6_test_command_register_dirty_bit_d;
	unsigned       tlb_set_10_entry_2_tr6_test_command_register_dirty_bit_d_number;
	unsigned       tlb_set_10_entry_2_tr6_test_command_register_user_supervisor_bit_u;
	unsigned       tlb_set_10_entry_2_tr6_test_command_register_user_supervisor_bit_u_number;
	unsigned       tlb_set_10_entry_2_tr6_test_command_register_read_write_bit_w;
	unsigned       tlb_set_10_entry_2_tr6_test_command_register_read_write_bit_w_number;
	unsigned long  tlb_set_10_entry_2_tr7_test_data_register_physical_address;
	unsigned       tlb_set_10_entry_2_tr7_test_data_register_ht_bit;
	unsigned       tlb_set_10_entry_2_tr7_test_data_register_rep_bit_d;
	unsigned long  tlb_set_10_entry_1_tag_0;
	unsigned long  tlb_set_10_entry_1_data_0;
	unsigned       tlb_set_10_entry_1_tr6_test_command_register_command_bit;
	unsigned long  tlb_set_10_entry_1_tr6_test_command_register_physical_address;
	unsigned long  tlb_set_10_entry_1_tr6_test_command_register_linear_address;
	unsigned       tlb_set_10_entry_1_tr6_test_command_register_valid_bit;
	unsigned       tlb_set_10_entry_1_tr6_test_command_register_dirty_bit_d;
	unsigned       tlb_set_10_entry_1_tr6_test_command_register_dirty_bit_d_number;
	unsigned       tlb_set_10_entry_1_tr6_test_command_register_user_supervisor_bit_u;
	unsigned       tlb_set_10_entry_1_tr6_test_command_register_user_supervisor_bit_u_number;
	unsigned       tlb_set_10_entry_1_tr6_test_command_register_read_write_bit_w;
	unsigned       tlb_set_10_entry_1_tr6_test_command_register_read_write_bit_w_number;
	unsigned long  tlb_set_10_entry_1_tr7_test_data_register_physical_address;
	unsigned       tlb_set_10_entry_1_tr7_test_data_register_ht_bit;
	unsigned       tlb_set_10_entry_1_tr7_test_data_register_rep_bit_d;
	/* Translation Lookaside Buffer Set 01: */
	unsigned long  tlb_set_01_entry_8_tag_7;
	unsigned long  tlb_set_01_entry_8_data_7;
	unsigned       tlb_set_01_entry_8_tr6_test_command_register_command_bit;
	unsigned long  tlb_set_01_entry_8_tr6_test_command_register_physical_address;
	unsigned long  tlb_set_01_entry_8_tr6_test_command_register_linear_address;
	unsigned       tlb_set_01_entry_8_tr6_test_command_register_valid_bit;
	unsigned       tlb_set_01_entry_8_tr6_test_command_register_dirty_bit_d;
	unsigned       tlb_set_01_entry_8_tr6_test_command_register_dirty_bit_d_number;
	unsigned       tlb_set_01_entry_8_tr6_test_command_register_user_supervisor_bit_u;
	unsigned       tlb_set_01_entry_8_tr6_test_command_register_user_supervisor_bit_u_number;
	unsigned       tlb_set_01_entry_8_tr6_test_command_register_read_write_bit_w;
	unsigned       tlb_set_01_entry_8_tr6_test_command_register_read_write_bit_w_number;
	unsigned long  tlb_set_01_entry_8_tr7_test_data_register_physical_address;
	unsigned       tlb_set_01_entry_8_tr7_test_data_register_ht_bit;
	unsigned       tlb_set_01_entry_8_tr7_test_data_register_rep_bit_d;
	unsigned long  tlb_set_01_entry_7_tag_6;
	unsigned long  tlb_set_01_entry_7_data_6;
	unsigned       tlb_set_01_entry_7_tr6_test_command_register_command_bit;
	unsigned long  tlb_set_01_entry_7_tr6_test_command_register_physical_address;
	unsigned long  tlb_set_01_entry_7_tr6_test_command_register_linear_address;
	unsigned       tlb_set_01_entry_7_tr6_test_command_register_valid_bit;
	unsigned       tlb_set_01_entry_7_tr6_test_command_register_dirty_bit_d;
	unsigned       tlb_set_01_entry_7_tr6_test_command_register_dirty_bit_d_number;
	unsigned       tlb_set_01_entry_7_tr6_test_command_register_user_supervisor_bit_u;
	unsigned       tlb_set_01_entry_7_tr6_test_command_register_user_supervisor_bit_u_number;
	unsigned       tlb_set_01_entry_7_tr6_test_command_register_read_write_bit_w;
	unsigned       tlb_set_01_entry_7_tr6_test_command_register_read_write_bit_w_number;
	unsigned long  tlb_set_01_entry_7_tr7_test_data_register_physical_address;
	unsigned       tlb_set_01_entry_7_tr7_test_data_register_ht_bit;
	unsigned       tlb_set_01_entry_7_tr7_test_data_register_rep_bit_d;
	unsigned long  tlb_set_01_entry_6_tag_5;
	unsigned long  tlb_set_01_entry_6_data_5;
	unsigned       tlb_set_01_entry_6_tr6_test_command_register_command_bit;
	unsigned long  tlb_set_01_entry_6_tr6_test_command_register_physical_address;
	unsigned long  tlb_set_01_entry_6_tr6_test_command_register_linear_address;
	unsigned       tlb_set_01_entry_6_tr6_test_command_register_valid_bit;
	unsigned       tlb_set_01_entry_6_tr6_test_command_register_dirty_bit_d;
	unsigned       tlb_set_01_entry_6_tr6_test_command_register_dirty_bit_d_number;
	unsigned       tlb_set_01_entry_6_tr6_test_command_register_user_supervisor_bit_u;
	unsigned       tlb_set_01_entry_6_tr6_test_command_register_user_supervisor_bit_u_number;
	unsigned       tlb_set_01_entry_6_tr6_test_command_register_read_write_bit_w;
	unsigned       tlb_set_01_entry_6_tr6_test_command_register_read_write_bit_w_number;
	unsigned long  tlb_set_01_entry_6_tr7_test_data_register_physical_address;
	unsigned       tlb_set_01_entry_6_tr7_test_data_register_ht_bit;
	unsigned       tlb_set_01_entry_6_tr7_test_data_register_rep_bit_d;
	unsigned long  tlb_set_01_entry_5_tag_4;
	unsigned long  tlb_set_01_entry_5_data_4;
	unsigned       tlb_set_01_entry_5_tr6_test_command_register_command_bit;
	unsigned long  tlb_set_01_entry_5_tr6_test_command_register_physical_address;
	unsigned long  tlb_set_01_entry_5_tr6_test_command_register_linear_address;
	unsigned       tlb_set_01_entry_5_tr6_test_command_register_valid_bit;
	unsigned       tlb_set_01_entry_5_tr6_test_command_register_dirty_bit_d;
	unsigned       tlb_set_01_entry_5_tr6_test_command_register_dirty_bit_d_number;
	unsigned       tlb_set_01_entry_5_tr6_test_command_register_user_supervisor_bit_u;
	unsigned       tlb_set_01_entry_5_tr6_test_command_register_user_supervisor_bit_u_number;
	unsigned       tlb_set_01_entry_5_tr6_test_command_register_read_write_bit_w;
	unsigned       tlb_set_01_entry_5_tr6_test_command_register_read_write_bit_w_number;
	unsigned long  tlb_set_01_entry_5_tr7_test_data_register_physical_address;
	unsigned       tlb_set_01_entry_5_tr7_test_data_register_ht_bit;
	unsigned       tlb_set_01_entry_5_tr7_test_data_register_rep_bit_d;
	unsigned long  tlb_set_01_entry_4_tag_3;
	unsigned long  tlb_set_01_entry_4_data_3;
	unsigned       tlb_set_01_entry_4_tr6_test_command_register_command_bit;
	unsigned long  tlb_set_01_entry_4_tr6_test_command_register_physical_address;
	unsigned long  tlb_set_01_entry_4_tr6_test_command_register_linear_address;
	unsigned       tlb_set_01_entry_4_tr6_test_command_register_valid_bit;
	unsigned       tlb_set_01_entry_4_tr6_test_command_register_dirty_bit_d;
	unsigned       tlb_set_01_entry_4_tr6_test_command_register_dirty_bit_d_number;
	unsigned       tlb_set_01_entry_4_tr6_test_command_register_user_supervisor_bit_u;
	unsigned       tlb_set_01_entry_4_tr6_test_command_register_user_supervisor_bit_u_number;
	unsigned       tlb_set_01_entry_4_tr6_test_command_register_read_write_bit_w;
	unsigned       tlb_set_01_entry_4_tr6_test_command_register_read_write_bit_w_number;
	unsigned long  tlb_set_01_entry_4_tr7_test_data_register_physical_address;
	unsigned       tlb_set_01_entry_4_tr7_test_data_register_ht_bit;
	unsigned       tlb_set_01_entry_4_tr7_test_data_register_rep_bit_d;
	unsigned long  tlb_set_01_entry_3_tag_2;
	unsigned long  tlb_set_01_entry_3_data_2;
	unsigned       tlb_set_01_entry_3_tr6_test_command_register_command_bit;
	unsigned long  tlb_set_01_entry_3_tr6_test_command_register_physical_address;
	unsigned long  tlb_set_01_entry_3_tr6_test_command_register_linear_address;
	unsigned       tlb_set_01_entry_3_tr6_test_command_register_valid_bit;
	unsigned       tlb_set_01_entry_3_tr6_test_command_register_dirty_bit_d;
	unsigned       tlb_set_01_entry_3_tr6_test_command_register_dirty_bit_d_number;
	unsigned       tlb_set_01_entry_3_tr6_test_command_register_user_supervisor_bit_u;
	unsigned       tlb_set_01_entry_3_tr6_test_command_register_user_supervisor_bit_u_number;
	unsigned       tlb_set_01_entry_3_tr6_test_command_register_read_write_bit_w;
	unsigned       tlb_set_01_entry_3_tr6_test_command_register_read_write_bit_w_number;
	unsigned long  tlb_set_01_entry_3_tr7_test_data_register_physical_address;
	unsigned       tlb_set_01_entry_3_tr7_test_data_register_ht_bit;
	unsigned       tlb_set_01_entry_3_tr7_test_data_register_rep_bit_d;
	unsigned long  tlb_set_01_entry_2_tag_1;
	unsigned long  tlb_set_01_entry_2_data_1;
	unsigned       tlb_set_01_entry_2_tr6_test_command_register_command_bit;
	unsigned long  tlb_set_01_entry_2_tr6_test_command_register_physical_address;
	unsigned long  tlb_set_01_entry_2_tr6_test_command_register_linear_address;
	unsigned       tlb_set_01_entry_2_tr6_test_command_register_valid_bit;
	unsigned       tlb_set_01_entry_2_tr6_test_command_register_dirty_bit_d;
	unsigned       tlb_set_01_entry_2_tr6_test_command_register_dirty_bit_d_number;
	unsigned       tlb_set_01_entry_2_tr6_test_command_register_user_supervisor_bit_u;
	unsigned       tlb_set_01_entry_2_tr6_test_command_register_user_supervisor_bit_u_number;
	unsigned       tlb_set_01_entry_2_tr6_test_command_register_read_write_bit_w;
	unsigned       tlb_set_01_entry_2_tr6_test_command_register_read_write_bit_w_number;
	unsigned long  tlb_set_01_entry_2_tr7_test_data_register_physical_address;
	unsigned       tlb_set_01_entry_2_tr7_test_data_register_ht_bit;
	unsigned       tlb_set_01_entry_2_tr7_test_data_register_rep_bit_d;
	unsigned long  tlb_set_01_entry_1_tag_0;
	unsigned long  tlb_set_01_entry_1_data_0;
	unsigned       tlb_set_01_entry_1_tr6_test_command_register_command_bit;
	unsigned long  tlb_set_01_entry_1_tr6_test_command_register_physical_address;
	unsigned long  tlb_set_01_entry_1_tr6_test_command_register_linear_address;
	unsigned       tlb_set_01_entry_1_tr6_test_command_register_valid_bit;
	unsigned       tlb_set_01_entry_1_tr6_test_command_register_dirty_bit_d;
	unsigned       tlb_set_01_entry_1_tr6_test_command_register_dirty_bit_d_number;
	unsigned       tlb_set_01_entry_1_tr6_test_command_register_user_supervisor_bit_u;
	unsigned       tlb_set_01_entry_1_tr6_test_command_register_user_supervisor_bit_u_number;
	unsigned       tlb_set_01_entry_1_tr6_test_command_register_read_write_bit_w;
	unsigned       tlb_set_01_entry_1_tr6_test_command_register_read_write_bit_w_number;
	unsigned long  tlb_set_01_entry_1_tr7_test_data_register_physical_address;
	unsigned       tlb_set_01_entry_1_tr7_test_data_register_ht_bit;
	unsigned       tlb_set_01_entry_1_tr7_test_data_register_rep_bit_d;
	/* Translation Lookaside Buffer Set 00: */
	unsigned long  tlb_set_00_entry_8_tag_7;
	unsigned long  tlb_set_00_entry_8_data_7;
	unsigned       tlb_set_00_entry_8_tr6_test_command_register_command_bit;
	unsigned long  tlb_set_00_entry_8_tr6_test_command_register_physical_address;
	unsigned long  tlb_set_00_entry_8_tr6_test_command_register_linear_address;
	unsigned       tlb_set_00_entry_8_tr6_test_command_register_valid_bit;
	unsigned       tlb_set_00_entry_8_tr6_test_command_register_dirty_bit_d;
	unsigned       tlb_set_00_entry_8_tr6_test_command_register_dirty_bit_d_number;
	unsigned       tlb_set_00_entry_8_tr6_test_command_register_user_supervisor_bit_u;
	unsigned       tlb_set_00_entry_8_tr6_test_command_register_user_supervisor_bit_u_number;
	unsigned       tlb_set_00_entry_8_tr6_test_command_register_read_write_bit_w;
	unsigned       tlb_set_00_entry_8_tr6_test_command_register_read_write_bit_w_number;
	unsigned long  tlb_set_00_entry_8_tr7_test_data_register_physical_address;
	unsigned       tlb_set_00_entry_8_tr7_test_data_register_ht_bit;
	unsigned       tlb_set_00_entry_8_tr7_test_data_register_rep_bit_d;
	unsigned long  tlb_set_00_entry_7_tag_6;
	unsigned long  tlb_set_00_entry_7_data_6;
	unsigned       tlb_set_00_entry_7_tr6_test_command_register_command_bit;
	unsigned long  tlb_set_00_entry_7_tr6_test_command_register_physical_address;
	unsigned long  tlb_set_00_entry_7_tr6_test_command_register_linear_address;
	unsigned       tlb_set_00_entry_7_tr6_test_command_register_valid_bit;
	unsigned       tlb_set_00_entry_7_tr6_test_command_register_dirty_bit_d;
	unsigned       tlb_set_00_entry_7_tr6_test_command_register_dirty_bit_d_number;
	unsigned       tlb_set_00_entry_7_tr6_test_command_register_user_supervisor_bit_u;
	unsigned       tlb_set_00_entry_7_tr6_test_command_register_user_supervisor_bit_u_number;
	unsigned       tlb_set_00_entry_7_tr6_test_command_register_read_write_bit_w;
	unsigned       tlb_set_00_entry_7_tr6_test_command_register_read_write_bit_w_number;
	unsigned long  tlb_set_00_entry_7_tr7_test_data_register_physical_address;
	unsigned       tlb_set_00_entry_7_tr7_test_data_register_ht_bit;
	unsigned       tlb_set_00_entry_7_tr7_test_data_register_rep_bit_d;
	unsigned long  tlb_set_00_entry_6_tag_5;
	unsigned long  tlb_set_00_entry_6_data_5;
	unsigned       tlb_set_00_entry_6_tr6_test_command_register_command_bit;
	unsigned long  tlb_set_00_entry_6_tr6_test_command_register_physical_address;
	unsigned long  tlb_set_00_entry_6_tr6_test_command_register_linear_address;
	unsigned       tlb_set_00_entry_6_tr6_test_command_register_valid_bit;
	unsigned       tlb_set_00_entry_6_tr6_test_command_register_dirty_bit_d;
	unsigned       tlb_set_00_entry_6_tr6_test_command_register_dirty_bit_d_number;
	unsigned       tlb_set_00_entry_6_tr6_test_command_register_user_supervisor_bit_u;
	unsigned       tlb_set_00_entry_6_tr6_test_command_register_user_supervisor_bit_u_number;
	unsigned       tlb_set_00_entry_6_tr6_test_command_register_read_write_bit_w;
	unsigned       tlb_set_00_entry_6_tr6_test_command_register_read_write_bit_w_number;
	unsigned long  tlb_set_00_entry_6_tr7_test_data_register_physical_address;
	unsigned       tlb_set_00_entry_6_tr7_test_data_register_ht_bit;
	unsigned       tlb_set_00_entry_6_tr7_test_data_register_rep_bit_d;
	unsigned long  tlb_set_00_entry_5_tag_4;
	unsigned long  tlb_set_00_entry_5_data_4;
	unsigned       tlb_set_00_entry_5_tr6_test_command_register_command_bit;
	unsigned long  tlb_set_00_entry_5_tr6_test_command_register_physical_address;
	unsigned long  tlb_set_00_entry_5_tr6_test_command_register_linear_address;
	unsigned       tlb_set_00_entry_5_tr6_test_command_register_valid_bit;
	unsigned       tlb_set_00_entry_5_tr6_test_command_register_dirty_bit_d;
	unsigned       tlb_set_00_entry_5_tr6_test_command_register_dirty_bit_d_number;
	unsigned       tlb_set_00_entry_5_tr6_test_command_register_user_supervisor_bit_u;
	unsigned       tlb_set_00_entry_5_tr6_test_command_register_user_supervisor_bit_u_number;
	unsigned       tlb_set_00_entry_5_tr6_test_command_register_read_write_bit_w;
	unsigned       tlb_set_00_entry_5_tr6_test_command_register_read_write_bit_w_number;
	unsigned long  tlb_set_00_entry_5_tr7_test_data_register_physical_address;
	unsigned       tlb_set_00_entry_5_tr7_test_data_register_ht_bit;
	unsigned       tlb_set_00_entry_5_tr7_test_data_register_rep_bit_d;
	unsigned long  tlb_set_00_entry_4_tag_3;
	unsigned long  tlb_set_00_entry_4_data_3;
	unsigned       tlb_set_00_entry_4_tr6_test_command_register_command_bit;
	unsigned long  tlb_set_00_entry_4_tr6_test_command_register_physical_address;
	unsigned long  tlb_set_00_entry_4_tr6_test_command_register_linear_address;
	unsigned       tlb_set_00_entry_4_tr6_test_command_register_valid_bit;
	unsigned       tlb_set_00_entry_4_tr6_test_command_register_dirty_bit_d;
	unsigned       tlb_set_00_entry_4_tr6_test_command_register_dirty_bit_d_number;
	unsigned       tlb_set_00_entry_4_tr6_test_command_register_user_supervisor_bit_u;
	unsigned       tlb_set_00_entry_4_tr6_test_command_register_user_supervisor_bit_u_number;
	unsigned       tlb_set_00_entry_4_tr6_test_command_register_read_write_bit_w;
	unsigned       tlb_set_00_entry_4_tr6_test_command_register_read_write_bit_w_number;
	unsigned long  tlb_set_00_entry_4_tr7_test_data_register_physical_address;
	unsigned       tlb_set_00_entry_4_tr7_test_data_register_ht_bit;
	unsigned       tlb_set_00_entry_4_tr7_test_data_register_rep_bit_d;
	unsigned long  tlb_set_00_entry_3_tag_2;
	unsigned long  tlb_set_00_entry_3_data_2;
	unsigned       tlb_set_00_entry_3_tr6_test_command_register_command_bit;
	unsigned long  tlb_set_00_entry_3_tr6_test_command_register_physical_address;
	unsigned long  tlb_set_00_entry_3_tr6_test_command_register_linear_address;
	unsigned       tlb_set_00_entry_3_tr6_test_command_register_valid_bit;
	unsigned       tlb_set_00_entry_3_tr6_test_command_register_dirty_bit_d;
	unsigned       tlb_set_00_entry_3_tr6_test_command_register_dirty_bit_d_number;
	unsigned       tlb_set_00_entry_3_tr6_test_command_register_user_supervisor_bit_u;
	unsigned       tlb_set_00_entry_3_tr6_test_command_register_user_supervisor_bit_u_number;
	unsigned       tlb_set_00_entry_3_tr6_test_command_register_read_write_bit_w;
	unsigned       tlb_set_00_entry_3_tr6_test_command_register_read_write_bit_w_number;
	unsigned long  tlb_set_00_entry_3_tr7_test_data_register_physical_address;
	unsigned       tlb_set_00_entry_3_tr7_test_data_register_ht_bit;
	unsigned       tlb_set_00_entry_3_tr7_test_data_register_rep_bit_d;
	unsigned long  tlb_set_00_entry_2_tag_1;
	unsigned long  tlb_set_00_entry_2_data_1;
	unsigned       tlb_set_00_entry_2_tr6_test_command_register_command_bit;
	unsigned long  tlb_set_00_entry_2_tr6_test_command_register_physical_address;
	unsigned long  tlb_set_00_entry_2_tr6_test_command_register_linear_address;
	unsigned       tlb_set_00_entry_2_tr6_test_command_register_valid_bit;
	unsigned       tlb_set_00_entry_2_tr6_test_command_register_dirty_bit_d;
	unsigned       tlb_set_00_entry_2_tr6_test_command_register_dirty_bit_d_number;
	unsigned       tlb_set_00_entry_2_tr6_test_command_register_user_supervisor_bit_u;
	unsigned       tlb_set_00_entry_2_tr6_test_command_register_user_supervisor_bit_u_number;
	unsigned       tlb_set_00_entry_2_tr6_test_command_register_read_write_bit_w;
	unsigned       tlb_set_00_entry_2_tr6_test_command_register_read_write_bit_w_number;
	unsigned long  tlb_set_00_entry_2_tr7_test_data_register_physical_address;
	unsigned       tlb_set_00_entry_2_tr7_test_data_register_ht_bit;
	unsigned       tlb_set_00_entry_2_tr7_test_data_register_rep_bit_d;
	unsigned long  tlb_set_00_entry_1_tag_0;
	unsigned long  tlb_set_00_entry_1_data_0;
	unsigned       tlb_set_00_entry_1_tr6_test_command_register_command_bit;
	unsigned long  tlb_set_00_entry_1_tr6_test_command_register_physical_address;
	unsigned long  tlb_set_00_entry_1_tr6_test_command_register_linear_address;
	unsigned       tlb_set_00_entry_1_tr6_test_command_register_valid_bit;
	unsigned       tlb_set_00_entry_1_tr6_test_command_register_dirty_bit_d;
	unsigned       tlb_set_00_entry_1_tr6_test_command_register_dirty_bit_d_number;
	unsigned       tlb_set_00_entry_1_tr6_test_command_register_user_supervisor_bit_u;
	unsigned       tlb_set_00_entry_1_tr6_test_command_register_user_supervisor_bit_u_number;
	unsigned       tlb_set_00_entry_1_tr6_test_command_register_read_write_bit_w;
	unsigned       tlb_set_00_entry_1_tr6_test_command_register_read_write_bit_w_number;
	unsigned long  tlb_set_00_entry_1_tr7_test_data_register_physical_address;
	unsigned       tlb_set_00_entry_1_tr7_test_data_register_ht_bit;
	unsigned       tlb_set_00_entry_1_tr7_test_data_register_rep_bit_d;
	/* Debug Address  Registers */
	unsigned long  dr0_breakpoint_0_linear_address;
	unsigned long  dr1_breakpoint_1_linear_address;
	unsigned long  dr2_breakpoint_2_linear_address;
	unsigned long  dr3_breakpoint_3_linear_address;
	/* Debug Reserved Registers */
	unsigned       dr4_reserved; /* Reserved by Intel */
	unsigned       dr5_reserved; /* Reserved by Intel */
	/* Debug Control  Registers */
	unsigned       dr7_read_write_0;
	unsigned       dr7_read_write_1;
	unsigned       dr7_read_write_2;
	unsigned       dr7_read_write_3;
	unsigned       dr7_length_0;
	unsigned       dr7_length_1;
	unsigned       dr7_length_2;
	unsigned       dr7_length_3;
	unsigned       dr7_breakpoint_condition_local_0;
	unsigned       dr7_breakpoint_condition_local_1;
	unsigned       dr7_breakpoint_condition_local_2;
	unsigned       dr7_breakpoint_condition_local_3;
	unsigned       dr7_breakpoint_condition_global_0;
	unsigned       dr7_breakpoint_condition_global_1;
	unsigned       dr7_breakpoint_condition_global_2;
	unsigned       dr7_breakpoint_condition_global_3;
	unsigned       dr7_exact_data_breakpoit_match_le;
	unsigned       dr7_exact_databreakpoint_match_le;
	/* Debug Status   Registers */
	unsigned       dr6_enabled_debug_exception_b0;
	unsigned       dr6_enabled_debug_exception_b1;
	unsigned       dr6_enabled_debug_exception_b2;
	unsigned       dr6_enabled_debug_exception_b3;
	unsigned       dr6_task_switch_bt;
	unsigned       dr6_single_step_exception_bs;
	unsigned       dr6_ice386_bd;
	/* TR Register For Task State Segment Descriptor */
	unsigned long  tr_tss_gdt_segment_base_address_offset;            /* Segment base:  16-bit (80286)/32-bit (80386 and higher) */
	unsigned long  tr_tss_gdt_segment_limit;           /* Segment limit: 16-bit (80286)/32-bit (80386 and higher) */
													   /* NOTE: Change "segment_limit" back to "size" if necessary and required by source code */
													   /* unsigned char  alloc; */
	unsigned       tr_tss_gdt_granularity_bit; /* Segment granularity bit: */
	unsigned       tr_tss_gdt_big;             /* Big: "0" or "1" */
	unsigned       tr_tss_gdt_default;           /* Default: "0" or "1" */
	unsigned       tr_tss_gdt_x_;              /* X: "0" or "1" */
	unsigned       tr_tss_gdt_0_;              /* 0: "0" or "1" */
	unsigned       tr_tss_gdt_segment_available;       /* Segment AVL (available for use by systems programmers): "0" or "1" */
	unsigned       tr_tss_gdt_segment_is_present;      /* Segment present: "1" if present and "0" if not present */
	unsigned       tr_tss_gdt_descriptor_privilege_level; /* Descriptor privilege level: "1", "2", or "3" */
	unsigned       tr_tss_gdt_type_of_segment_or_gate;          /* Type of Segment or Gate: */
	unsigned       tr_tss_gdt_type_1;          /* Segment type 1: "0" or "1": */
	unsigned       tr_tss_gdt_type_0;          /* Segment type 0: "0" or "1": */
	unsigned       tr_tss_gdt_type_c;          /* Segment type conforming: "0" for conforming or "1" for non-conforming: */
	unsigned       tr_tss_gdt_type_e;          /* Segment type expand-down: "0" or "1" */
	unsigned       tr_tss_gdt_type_r;          /* Segment type readable: "0" or "1" */
	unsigned       tr_tss_gdt_type_w;          /* Segment type writable: "0" or "1" */
	unsigned       tr_tss_gdt_selector_table_indicator;        /* Segment table indicator */
	unsigned long  tr_tss_gdt_selector_index;        /* Segment table indicator */
	unsigned       tr_tss_gdt_selector_requester_privilege_level;        /* Segment requester privilege level */
	unsigned       tr_tss_gdt_segment_accessed;        /* Segment A (accessed): "0" or "1" */
	unsigned long  tr_tss_ldt_segment_base_address_offset;            /* Segment base:  16-bit (80286)/32-bit (80386 and higher) */
	unsigned long  tr_tss_ldt_segment_limit;           /* Segment limit: 16-bit (80286)/32-bit (80386 and higher) */
	unsigned       tr_tss_ldt_granularity_bit; /* Segment granularity bit: */
	unsigned       tr_tss_ldt_big;             /* Big: "0" or "1" */
	unsigned       tr_tss_ldt_default;           /* Default: "0" or "1" */
	unsigned       tr_tss_ldt_x_;              /* X: "0" or "1" */
	unsigned       tr_tss_ldt_0_;              /* 0: "0" or "1" */
	unsigned       tr_tss_ldt_segment_available;       /* Segment AVL (available for use by systems programmers): "0" or "1" */
	unsigned       tr_tss_ldt_segment_is_present;      /* Segment present: "1" if present and "0" if not present */
	unsigned       tr_tss_ldt_descriptor_privilege_level; /* Descriptor privilege level: "1", "2", or "3" */
	unsigned       tr_tss_ldt_type_of_segment_or_gate;          /* Type of Segment or Gate: */
	unsigned       tr_tss_ldt_type_1;          /* Segment type 1: "0" or "1": */
	unsigned       tr_tss_ldt_type_0;          /* Segment type 0: "0" or "1": */
	unsigned       tr_tss_ldt_type_c;          /* Segment type conforming: "0" for conforming or "1" for non-conforming: */
	unsigned       tr_tss_ldt_type_e;          /* Segment type expand-down: "0" or "1" */
	unsigned       tr_tss_ldt_type_r;          /* Segment type readable: "0" or "1" */
	unsigned       tr_tss_ldt_type_w;          /* Segment type writable: "0" or "1" */
	unsigned       tr_tss_ldt_segment_accessed;        /* Segment A (accessed): "0" or "1" */
	unsigned       tr_tss_ldt_selector_table_indicator;        /* Segment table indicator */
	unsigned long  tr_tss_ldt_selector_index;        /* Segment table indicator */
	unsigned       tr_tss_ldt_selector_requester_privilege_level;        /* Segment requester privilege level */
																		 /* I/O (Input/Output) Map Base: */
	unsigned long  input_output_map_base;                        /* I/O (Input/Output) Map Base */
																 /* Debug trap bit (T): */
	unsigned       debug_trap_bit;                               /* Debug trap bit (T) */
																 /*  CPU Registers (32-bit) for tr_tss: */
	unsigned long  tr_tss_register_eax;                                /* AX (16-bit)/eAX (32-bit) CPU register */
	unsigned long  tr_tss_register_ebx;                                /* BX (16-bit)/eBX (32-bit) CPU register */
	unsigned long  tr_tss_register_ecx;                                /* CX (16-bit)/eCX (32-bit) CPU register */
	unsigned long  tr_tss_register_edx;                                /* DX (16-bit)/eDX (32-bit) CPU register */
	unsigned long  tr_tss_register_esp;                                /* SP (16-bit)/eSP (32-bit) CPU register */
	unsigned long  tr_tss_register_ebp;                                /* BP (16-bit)/eBP (32-bit) CPU register */
	unsigned long  tr_tss_register_esi;                                /* SI (16-bit)/eSI (32-bit) CPU register */
	unsigned long  tr_tss_register_edi;                                /* DI (16-bit)/eDI (32-bit) CPU register */
	unsigned long  tr_tss_register_esp_for_privilege_level_2;          /* SP (16-bit)/eSP (32-bit) CPU register */
	unsigned long  tr_tss_register_esp_for_privilege_level_1;          /* SP (16-bit)/eSP (32-bit) CPU register */
	unsigned long  tr_tss_register_esp_for_privilege_level_0;          /* SP (16-bit)/eSP (32-bit) CPU register */
																	   /*  FLAGS (16-bit) register for tr_tss: */
	unsigned       tr_tss_flags_cf;                                 /* Carry           Flag */
	unsigned       tr_tss_flags_pf;                                 /* Parity          Flag */
	unsigned       tr_tss_flags_ac;                                 /* Auxiliary Carry Flag */
	unsigned       tr_tss_flags_zf;                                 /* Zero            Flag */
	unsigned       tr_tss_flags_sf;                                 /* Sign            Flag */
	unsigned       tr_tss_flags_tf;                                 /* Trap            Flag */
	unsigned       tr_tss_flags_if;                                 /* Interrupt       Flag */
	unsigned       tr_tss_flags_df;                                 /* Difference      Flag */
	unsigned       tr_tss_flags_of;                                 /* Overflow        Flag */
																	/* EFLAGS (32-bit) register for tr_tss: */
	unsigned       tr_tss_eflags_iopl;                              /* Input/Output Privilege Level Flag */
	unsigned       tr_tss_eflags_nt;                                /* Nested Task                  Flag */
	unsigned       tr_tss_eflags_rf;                                /* Return                       Flag */
	unsigned       tr_tss_eflags_vm;                                /* Virtual Machine              Flag */
																	/* Instruction Pointer (EIP) for tr_tss: */
	unsigned long  tr_tss_instruction_pointer_eip;                  /* Instruction Pointer (EIP) */
																	/* Control Register 3 (Page Directory Base Register): */
	unsigned long  tr_tss_pdbr_page_frame_address_specified;          /* Page frame address:  32-bit (80386 and higher) */
	unsigned long  tr_tss_pdbr_page_frame_address_being_loaded;       /* Page frame address:  32-bit (80386 and higher) */
	unsigned long  tr_tss_pdbr_page_frame_address;                    /* Page frame address:  32-bit (80386 and higher) */
	unsigned       tr_tss_pdbr_page_table_is_present;                 /* Page table present: "1" if present and "0" if not present */
	unsigned       tr_tss_pdbr_page_table_read_write;                 /* Page table read/write: "0" if read-only and "1" if read/write */
	unsigned       tr_tss_pdbr_page_table_user_supervisor;            /* Page table user/supervisor: "0" if user and "1" if supervisor: */
	unsigned       tr_tss_pdbr_page_table_dirty;                      /* Page table Default: "0" or "1" */
	unsigned       tr_tss_pdbr_page_table_available;                  /* Page table available for systems programmer use: "0" or "1" */
																	  /* Page Directory: */
	unsigned long  tr_tss_directory_page_frame_address_specified;          /* Page frame address:  32-bit (80386 and higher) */
	unsigned long  tr_tss_directory_page_frame_address_being_loaded;       /* Page frame address:  32-bit (80386 and higher) */
	unsigned long  tr_tss_directory_page_frame_address;                    /* Page frame address:  32-bit (80386 and higher) */
	unsigned       tr_tss_directory_page_table_read_write;                 /* Page table read/write: "0" if read-only and "1" if read/write */
	unsigned       tr_tss_directory_page_table_user_supervisor;            /* Page table user/supervisor: "0" if user and "1" if supervisor: */
	unsigned       tr_tss_directory_page_table_dirty;                      /* Page table Default: "0" or "1" */
	unsigned       tr_tss_directory_page_table_available;                  /* Page table available for systems programmer use: "0" or "1" */
																		   /* ESP (stack pointer register) for tr_tss: */
	unsigned long  tr_tss_stack_pointer_esp_for_privilege_level_2;  /* ESP (stack pointer register) for Privilege Level 2 */
	unsigned long  tr_tss_stack_pointer_esp_for_privilege_level_1;  /* ESP (stack pointer register) for Privilege Level 1 */
	unsigned long  tr_tss_stack_pointer_esp_for_privilege_level_0;  /* ESP (stack pointer register) for Privilege Level 0 */
																	/* CS segment register for tr_tss: */
	unsigned long  tr_tss_cs_segment_register_descriptor_base_address_offset;                       /* Segment base:  16-bit (80286)/32-bit (80386 and higher) */
	unsigned long  tr_tss_cs_segment_register_descriptor_limit;                      /* Segment limit: 16-bit (80286)/32-bit (80386 and higher) */
																					 /* NOTE: Change "segment_limit" back to "size" if necessary and required by source code */
	unsigned       tr_tss_cs_segment_register_descriptor_granularity_bit;            /* Segment granularity bit: */
	unsigned       tr_tss_cs_segment_register_descriptor_big;                        /* Big: "0" or "1" */
	unsigned       tr_tss_cs_segment_register_descriptor_default;                      /* Default: "0" or "1" */
	unsigned       tr_tss_cs_segment_register_descriptor_x_;                         /* X: "0" or "1" */
	unsigned       tr_tss_cs_segment_register_descriptor_0_;                         /* 0: "0" or "1" */
	unsigned       tr_tss_cs_segment_register_descriptor_available;                  /* Segment AVL (available for use by systems programmers): "0" or "1" */
	unsigned       tr_tss_cs_segment_register_descriptor_is_present;                 /* Segment present: "1" if present and "0" if not present */
	unsigned       tr_tss_cs_segment_register_descriptor_privilege_level;            /* Descriptor privilege level: "1", "2", or "3" */
	unsigned       tr_tss_cs_segment_register_type_of_segment_or_gate;               /* Type of Segment or Gate: */
	unsigned       tr_tss_cs_segment_register_descriptor_type_1;                     /* Segment type 1: "0" or "1": */
	unsigned       tr_tss_cs_segment_register_descriptor_type_0;                     /* Segment type 0: "0" or "1": */
	unsigned       tr_tss_cs_segment_register_descriptor_type_c;                     /* Segment type conforming: "0" for conforming or "1" for non-conforming: */
	unsigned       tr_tss_cs_segment_register_descriptor_type_e;                     /* Segment type expand-down: "0" or "1" */
	unsigned       tr_tss_cs_segment_register_descriptor_type_r;                     /* Segment type readable: "0" or "1" */
	unsigned       tr_tss_cs_segment_register_descriptor_type_w;                     /* Segment type writable: "0" or "1" */
	unsigned       tr_tss_cs_segment_register_descriptor_accessed;                   /* Segment A (accessed): "0" or "1" */
	unsigned       tr_tss_cs_segment_register_selector_table_indicator;        /* Segment table indicator */
	unsigned long  tr_tss_cs_segment_register_selector_index;        /* Segment table indicator */
	unsigned       tr_tss_cs_segment_register_selector_requester_privilege_level;        /* Segment requester privilege level */
																						 /* DS segment register for tr_tss: */
	unsigned long  tr_tss_ds_segment_register_descriptor_base_address_offset;                       /* Segment base:  16-bit (80286)/32-bit (80386 and higher) */
	unsigned long  tr_tss_ds_segment_register_descriptor_limit;                      /* Segment limit: 16-bit (80286)/32-bit (80386 and higher) */
																					 /* NOTE: Change "segment_limit" back to "size" if necessary and required by source code */
	unsigned       tr_tss_ds_segment_register_descriptor_granularity_bit;            /* Segment granularity bit: */
	unsigned       tr_tss_ds_segment_register_descriptor_big;                        /* Big: "0" or "1" */
	unsigned       tr_tss_ds_segment_register_descriptor_default;                      /* Default: "0" or "1" */
	unsigned       tr_tss_ds_segment_register_descriptor_x_;                         /* X: "0" or "1" */
	unsigned       tr_tss_ds_segment_register_descriptor_0_;                         /* 0: "0" or "1" */
	unsigned       tr_tss_ds_segment_register_descriptor_available;                  /* Segment AVL (available for use by systems programmers): "0" or "1" */
	unsigned       tr_tss_ds_segment_register_descriptor_is_present;                 /* Segment present: "1" if present and "0" if not present */
	unsigned       tr_tss_ds_segment_register_descriptor_privilege_level;            /* Descriptor privilege level: "1", "2", or "3" */
	unsigned       tr_tss_ds_segment_register_type_of_segment_or_gate;               /* Type of Segment or Gate: */
	unsigned       tr_tss_ds_segment_register_descriptor_type_1;                     /* Segment type 1: "0" or "1": */
	unsigned       tr_tss_ds_segment_register_descriptor_type_0;                     /* Segment type 0: "0" or "1": */
	unsigned       tr_tss_ds_segment_register_descriptor_type_c;                     /* Segment type conforming: "0" for conforming or "1" for non-conforming: */
	unsigned       tr_tss_ds_segment_register_descriptor_type_e;                     /* Segment type expand-down: "0" or "1" */
	unsigned       tr_tss_ds_segment_register_descriptor_type_r;                     /* Segment type readable: "0" or "1" */
	unsigned       tr_tss_ds_segment_register_descriptor_type_w;                     /* Segment type writable: "0" or "1" */
	unsigned       tr_tss_ds_segment_register_descriptor_accessed;                   /* Segment A (accessed): "0" or "1" */
	unsigned       tr_tss_ds_segment_register_selector_table_indicator;        /* Segment table indicator */
	unsigned long  tr_tss_ds_segment_register_selector_index;        /* Segment table indicator */
	unsigned       tr_tss_ds_segment_register_selector_requester_privilege_level;        /* Segment requester privilege level */
																						 /* ES segment register for tr_tss: */
	unsigned long  tr_tss_es_segment_register_descriptor_base_address_offset;                       /* Segment base:  16-bit (80286)/32-bit (80386 and higher) */
	unsigned long  tr_tss_es_segment_register_descriptor_limit;                      /* Segment limit: 16-bit (80286)/32-bit (80386 and higher) */
																					 /* NOTE: Change "segment_limit" back to "size" if necessary and required by source code */
	unsigned       tr_tss_es_segment_register_descriptor_granularity_bit;            /* Segment granularity bit: */
	unsigned       tr_tss_es_segment_register_descriptor_big;                        /* Big: "0" or "1" */
	unsigned       tr_tss_es_segment_register_descriptor_default;                      /* Default: "0" or "1" */
	unsigned       tr_tss_es_segment_register_descriptor_x_;                         /* X: "0" or "1" */
	unsigned       tr_tss_es_segment_register_descriptor_0_;                         /* 0: "0" or "1" */
	unsigned       tr_tss_es_segment_register_descriptor_available;                  /* Segment AVL (available for use by systems programmers): "0" or "1" */
	unsigned       tr_tss_es_segment_register_descriptor_is_present;                 /* Segment present: "1" if present and "0" if not present */
	unsigned       tr_tss_es_segment_register_descriptor_privilege_level;            /* Descriptor privilege level: "1", "2", or "3" */
	unsigned       tr_tss_es_segment_register_type_of_segment_or_gate;               /* Type of Segment or Gate: */
	unsigned       tr_tss_es_segment_register_descriptor_type_1;                     /* Segment type 1: "0" or "1": */
	unsigned       tr_tss_es_segment_register_descriptor_type_0;                     /* Segment type 0: "0" or "1": */
	unsigned       tr_tss_es_segment_register_descriptor_type_c;                     /* Segment type conforming: "0" for conforming or "1" for non-conforming: */
	unsigned       tr_tss_es_segment_register_descriptor_type_e;                     /* Segment type expand-down: "0" or "1" */
	unsigned       tr_tss_es_segment_register_descriptor_type_r;                     /* Segment type readable: "0" or "1" */
	unsigned       tr_tss_es_segment_register_descriptor_type_w;                     /* Segment type writable: "0" or "1" */
	unsigned       tr_tss_es_segment_register_descriptor_accessed;                   /* Segment A (accessed): "0" or "1" */
	unsigned       tr_tss_es_segment_register_selector_table_indicator;        /* Segment table indicator */
	unsigned long  tr_tss_es_segment_register_selector_index;        /* Segment table indicator */
	unsigned       tr_tss_es_segment_register_selector_requester_privilege_level;        /* Segment requester privilege level */
																						 /* DS segment register for tr_tss: */
	unsigned long  tr_tss_fs_segment_register_descriptor_base_address_offset;                       /* Segment base:  16-bit (80286)/32-bit (80386 and higher) */
	unsigned long  tr_tss_fs_segment_register_descriptor_limit;                      /* Segment limit: 16-bit (80286)/32-bit (80386 and higher) */
																					 /* NOTE: Change "segment_limit" back to "size" if necessary and required by source code */
	unsigned       tr_tss_fs_segment_register_descriptor_granularity_bit;            /* Segment granularity bit: */
	unsigned       tr_tss_fs_segment_register_descriptor_big;                        /* Big: "0" or "1" */
	unsigned       tr_tss_fs_segment_register_descriptor_default;                      /* Default: "0" or "1" */
	unsigned       tr_tss_fs_segment_register_descriptor_x_;                         /* X: "0" or "1" */
	unsigned       tr_tss_fs_segment_register_descriptor_0_;                         /* 0: "0" or "1" */
	unsigned       tr_tss_fs_segment_register_descriptor_available;                  /* Segment AVL (available for use by systems programmers): "0" or "1" */
	unsigned       tr_tss_fs_segment_register_descriptor_is_present;                 /* Segment present: "1" if present and "0" if not present */
	unsigned       tr_tss_fs_segment_register_descriptor_privilege_level;            /* Descriptor privilege level: "1", "2", or "3" */
	unsigned       tr_tss_fs_segment_register_type_of_segment_or_gate;               /* Type of Segment or Gate: */
	unsigned       tr_tss_fs_segment_register_descriptor_type_1;                     /* Segment type 1: "0" or "1": */
	unsigned       tr_tss_fs_segment_register_descriptor_type_0;                     /* Segment type 0: "0" or "1": */
	unsigned       tr_tss_fs_segment_register_descriptor_type_c;                     /* Segment type conforming: "0" for conforming or "1" for non-conforming: */
	unsigned       tr_tss_fs_segment_register_descriptor_type_e;                     /* Segment type expand-down: "0" or "1" */
	unsigned       tr_tss_fs_segment_register_descriptor_type_r;                     /* Segment type readable: "0" or "1" */
	unsigned       tr_tss_fs_segment_register_descriptor_type_w;                     /* Segment type writable: "0" or "1" */
	unsigned       tr_tss_fs_segment_register_descriptor_accessed;                   /* Segment A (accessed): "0" or "1" */
	unsigned       tr_tss_fs_segment_register_selector_table_indicator;        /* Segment table indicator */
	unsigned long  tr_tss_fs_segment_register_selector_index;        /* Segment table indicator */
	unsigned       tr_tss_fs_segment_register_selector_requester_privilege_level;        /* Segment requester privilege level */
																						 /* DS segment register for tr_tss: */
	unsigned long  tr_tss_gs_segment_register_descriptor_base_address_offset;                       /* Segment base:  16-bit (80286)/32-bit (80386 and higher) */
	unsigned long  tr_tss_gs_segment_register_descriptor_limit;                      /* Segment limit: 16-bit (80286)/32-bit (80386 and higher) */
																					 /* NOTE: Change "segment_limit" back to "size" if necessary and required by source code */
	unsigned       tr_tss_gs_segment_register_descriptor_granularity_bit;            /* Segment granularity bit: */
	unsigned       tr_tss_gs_segment_register_descriptor_big;                        /* Big: "0" or "1" */
	unsigned       tr_tss_gs_segment_register_descriptor_default;                      /* Default: "0" or "1" */
	unsigned       tr_tss_gs_segment_register_descriptor_x_;                         /* X: "0" or "1" */
	unsigned       tr_tss_gs_segment_register_descriptor_0_;                         /* 0: "0" or "1" */
	unsigned       tr_tss_gs_segment_register_descriptor_available;                  /* Segment AVL (available for use by systems programmers): "0" or "1" */
	unsigned       tr_tss_gs_segment_register_descriptor_is_present;                 /* Segment present: "1" if present and "0" if not present */
	unsigned       tr_tss_gs_segment_register_descriptor_privilege_level;            /* Descriptor privilege level: "1", "2", or "3" */
	unsigned       tr_tss_gs_segment_register_type_of_segment_or_gate;               /* Type of Segment or Gate: */
	unsigned       tr_tss_gs_segment_register_descriptor_type_1;                     /* Segment type 1: "0" or "1": */
	unsigned       tr_tss_gs_segment_register_descriptor_type_0;                     /* Segment type 0: "0" or "1": */
	unsigned       tr_tss_gs_segment_register_descriptor_type_c;                     /* Segment type conforming: "0" for conforming or "1" for non-conforming: */
	unsigned       tr_tss_gs_segment_register_descriptor_type_e;                     /* Segment type expand-down: "0" or "1" */
	unsigned       tr_tss_gs_segment_register_descriptor_type_r;                     /* Segment type readable: "0" or "1" */
	unsigned       tr_tss_gs_segment_register_descriptor_type_w;                     /* Segment type writable: "0" or "1" */
	unsigned       tr_tss_gs_segment_register_descriptor_accessed;                   /* Segment A (accessed): "0" or "1" */
	unsigned       tr_tss_gs_segment_register_selector_table_indicator;        /* Segment table indicator */
	unsigned long  tr_tss_gs_segment_register_selector_index;        /* Segment table indicator */
	unsigned       tr_tss_gs_segment_register_selector_requester_privilege_level;        /* Segment requester privilege level */
																						 /* SS segment register (stack segment) for tr_tss (privilege level 2): */
	unsigned long  tr_tss_ss_segment_register_descriptor_base_address_offset_for_privilege_level_2;            /* Segment base:  16-bit (80286)/32-bit (80386 and higher) */
	unsigned long  tr_tss_ss_segment_register_descriptor_limit_for_privilege_level_2;           /* Segment limit: 16-bit (80286)/32-bit (80386 and higher) */
																								/* NOTE: Change "segment_limit" back to "size" if necessary and required by source code */
	unsigned       tr_tss_ss_segment_register_descriptor_granularity_bit_for_privilege_level_2; /* Segment granularity bit: */
	unsigned       tr_tss_ss_segment_register_descriptor_big_for_privilege_level_2;             /* Big: "0" or "1" */
	unsigned       tr_tss_ss_segment_register_descriptor_default_for_privilege_level_2;           /* Default: "0" or "1" */
	unsigned       tr_tss_ss_segment_register_descriptor_x_for_privilege_level_2;               /* X: "0" or "1" */
	unsigned       tr_tss_ss_segment_register_descriptor_0_for_privilege_level_2;               /* 0: "0" or "1" */
	unsigned       tr_tss_ss_segment_register_descriptor_available_for_privilege_level_2;/* Segment AVL(available for use by systems programmers):"0"or"1" */
	unsigned       tr_tss_ss_segment_register_descriptor_is_present_for_privilege_level_2;      /* Segment present: "1" if present and "0" if not present */
	unsigned       tr_tss_ss_segment_register_descriptor_privilege_level_for_privilege_level_2; /* Descriptor privilege level: "1", "2", or "3" */
	unsigned       tr_tss_ss_segment_register_type_of_segment_or_gate_for_privilege_level_2;    /* Type of Segment or Gate: */
	unsigned       tr_tss_ss_segment_register_descriptor_type_1_for_privilege_level_2;          /* Segment type 1: "0" or "1": */
	unsigned       tr_tss_ss_segment_register_descriptor_type_0_for_privilege_level_2;          /* Segment type 0: "0" or "1": */
	unsigned       tr_tss_ss_segment_register_descriptor_type_c_for_privilege_level_2;/*Segment type conforming:"0"for conforming or "1"for non-conforming:*/
	unsigned       tr_tss_ss_segment_register_descriptor_type_e_for_privilege_level_2;          /* Segment type expand-down: "0" or "1" */
	unsigned       tr_tss_ss_segment_register_descriptor_type_r_for_privilege_level_2;          /* Segment type readable: "0" or "1" */
	unsigned       tr_tss_ss_segment_register_descriptor_type_w_for_privilege_level_2;          /* Segment type writable: "0" or "1" */
	unsigned       tr_tss_ss_segment_register_descriptor_accessed_for_privilege_level_2;        /* Segment A (accessed): "0" or "1" */
	unsigned       tr_tss_ss_segment_register_selector_table_indicator_for_privilege_level_2;   /* Segment table indicator */
	unsigned long  tr_tss_ss_segment_register_selector_index_for_privilege_level_2;             /* Segment table indicator */
	unsigned       tr_tss_ss_segment_register_selector_requester_privilege_level_for_privilege_level_2; /* Segment requester privilege level */
																										/* Stack segment for tr_tss (privilege level 1): */
	unsigned long  tr_tss_ss_segment_register_descriptor_base_address_offset_for_privilege_level_1;            /* Segment base:  16-bit (80286)/32-bit (80386 and higher) */
	unsigned long  tr_tss_ss_segment_register_descriptor_limit_for_privilege_level_1;           /* Segment limit: 16-bit (80286)/32-bit (80386 and higher) */
																								/* NOTE: Change "segment_limit" back to "size" if necessary and required by source code */
	unsigned       tr_tss_ss_segment_register_descriptor_granularity_bit_for_privilege_level_1; /* Segment granularity bit: */
	unsigned       tr_tss_ss_segment_register_descriptor_big_for_privilege_level_1;             /* Big: "0" or "1" */
	unsigned       tr_tss_ss_segment_register_descriptor_default_for_privilege_level_1;           /* Default: "0" or "1" */
	unsigned       tr_tss_ss_segment_register_descriptor_x_for_privilege_level_1;               /* X: "0" or "1" */
	unsigned       tr_tss_ss_segment_register_descriptor_0_for_privilege_level_1;               /* 0: "0" or "1" */
	unsigned       tr_tss_ss_segment_register_descriptor_available_for_privilege_level_1;/* Segment AVL(available for use by systems programmers):"0"or"1" */
	unsigned       tr_tss_ss_segment_register_descriptor_is_present_for_privilege_level_1;      /* Segment present: "1" if present and "0" if not present */
	unsigned       tr_tss_ss_segment_register_descriptor_privilege_level_for_privilege_level_1; /* Descriptor privilege level: "1", "2", or "3" */
	unsigned       tr_tss_ss_segment_register_type_of_segment_or_gate_for_privilege_level_1;    /* Type of Segment or Gate: */
	unsigned       tr_tss_ss_segment_register_descriptor_type_1_for_privilege_level_1;          /* Segment type 1: "0" or "1": */
	unsigned       tr_tss_ss_segment_register_descriptor_type_0_for_privilege_level_1;          /* Segment type 0: "0" or "1": */
	unsigned       tr_tss_ss_segment_register_descriptor_type_c_for_privilege_level_1;/*Segment type conforming:"0"for conforming or "1"for non-conforming:*/
	unsigned       tr_tss_ss_segment_register_descriptor_type_e_for_privilege_level_1;          /* Segment type expand-down: "0" or "1" */
	unsigned       tr_tss_ss_segment_register_descriptor_type_r_for_privilege_level_1;          /* Segment type readable: "0" or "1" */
	unsigned       tr_tss_ss_segment_register_descriptor_type_w_for_privilege_level_1;          /* Segment type writable: "0" or "1" */
	unsigned       tr_tss_ss_segment_register_descriptor_accessed_for_privilege_level_1;        /* Segment A (accessed): "0" or "1" */
	unsigned       tr_tss_ss_segment_register_selector_table_indicator_for_privilege_level_1;   /* Segment table indicator */
	unsigned long  tr_tss_ss_segment_register_selector_index_for_privilege_level_1;             /* Segment table indicator */
	unsigned       tr_tss_ss_segment_register_selector_requester_privilege_level_for_privilege_level_1; /* Segment requester privilege level */
																										/* Stack segment for tr_tss (privilege level 0): */
	unsigned long  tr_tss_ss_segment_register_descriptor_base_address_offset_for_privilege_level_0;            /* Segment base:  16-bit (80286)/32-bit (80386 and higher) */
	unsigned long  tr_tss_ss_segment_register_descriptor_limit_for_privilege_level_0;           /* Segment limit: 16-bit (80286)/32-bit (80386 and higher) */
																								/* NOTE: Change "segment_limit" back to "size" if necessary and required by source code */
	unsigned       tr_tss_ss_segment_register_descriptor_granularity_bit_for_privilege_level_0; /* Segment granularity bit: */
	unsigned       tr_tss_ss_segment_register_descriptor_big_for_privilege_level_0;             /* Big: "0" or "1" */
	unsigned       tr_tss_ss_segment_register_descriptor_default_for_privilege_level_0;           /* Default: "0" or "1" */
	unsigned       tr_tss_ss_segment_register_descriptor_x_for_privilege_level_0;               /* X: "0" or "1" */
	unsigned       tr_tss_ss_segment_register_descriptor_0_for_privilege_level_0;               /* 0: "0" or "1" */
	unsigned       tr_tss_ss_segment_register_descriptor_available_for_privilege_level_0;/* Segment AVL(available for use by systems programmers):"0"or"1" */
	unsigned       tr_tss_ss_segment_register_descriptor_is_present_for_privilege_level_0;      /* Segment present: "1" if present and "0" if not present */
	unsigned       tr_tss_ss_segment_register_descriptor_privilege_level_for_privilege_level_0; /* Descriptor privilege level: "1", "2", or "3" */
	unsigned       tr_tss_ss_segment_register_type_of_segment_or_gate_for_privilege_level_0;    /* Type of Segment or Gate: */
	unsigned       tr_tss_ss_segment_register_descriptor_type_1_for_privilege_level_0;          /* Segment type 1: "0" or "1": */
	unsigned       tr_tss_ss_segment_register_descriptor_type_0_for_privilege_level_0;          /* Segment type 0: "0" or "1": */
	unsigned       tr_tss_ss_segment_register_descriptor_type_c_for_privilege_level_0;/*Segment type conforming:"0"for conforming or "1"for non-conforming:*/
	unsigned       tr_tss_ss_segment_register_descriptor_type_e_for_privilege_level_0;          /* Segment type expand-down: "0" or "1" */
	unsigned       tr_tss_ss_segment_register_descriptor_type_r_for_privilege_level_0;          /* Segment type readable: "0" or "1" */
	unsigned       tr_tss_ss_segment_register_descriptor_type_w_for_privilege_level_0;          /* Segment type writable: "0" or "1" */
	unsigned       tr_tss_ss_segment_register_descriptor_accessed_for_privilege_level_0;        /* Segment A (accessed): "0" or "1" */
	unsigned       tr_tss_ss_segment_register_selector_table_indicator_for_privilege_level_0;   /* Segment table indicator */
	unsigned long  tr_tss_ss_segment_register_selector_index_for_privilege_level_0;             /* Segment table indicator */
	unsigned       tr_tss_ss_segment_register_selector_requester_privilege_level_for_privilege_level_0; /* Segment requester privilege level */
																										/* Task State Segment (tr_tss) Back Link: */
	unsigned long  tr_tss_back_link;                                /* tr_tss (Task State Segment) back link */
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
	unsigned long  task_gate_idt_gate_for_interrupt_selector;            /* Interrupt descriptor base:  16-bit (80286)/32-bit (80386 and higher) */
	unsigned long  task_gate_idt_gate_for_interrupt_offset;           /* Interrupt descriptor limit: 16-bit (80286)/32-bit (80386 and higher) */
	unsigned       task_gate_idt_gate_for_interrupt_present;          /* Present: */
	unsigned       task_gate_idt_gate_for_interrupt_descriptor_privilege_level;          /* Descriptor Privilege Level: */
	unsigned       task_gate_idt_gate_for_interrupt_number_n_;          /* Gate for Interrupt #N: */
	unsigned       task_gate_idt_gate_for_interrupt_number_2_;          /* Gate for Interrupt #2: */
	unsigned       task_gate_idt_gate_for_interrupt_number_1_;          /* Gate for Interrupt #1: */
	unsigned       task_gate_idt_gate_for_interrupt_number_0_;          /* Gate for Interrupt #0: */
																		/* Used for task gates residing in Interrupt Descriptor Table (Trap Gates): */
	unsigned long  task_gate_idt_gate_for_trap_selector;            /* Interrupt descriptor base:  16-bit (80286)/32-bit (80386 and higher) */
	unsigned long  task_gate_idt_gate_for_trap_offset;           /* Interrupt descriptor limit: 16-bit (80286)/32-bit (80386 and higher) */
	unsigned       task_gate_idt_gate_for_trap_present;          /* Present: */
	unsigned       task_gate_idt_gate_for_trap_descriptor_privilege_level;          /* Descriptor Privilege Level: */
	unsigned       task_gate_idt_gate_for_trap_number_n_;          /* Gate for Interrupt #N: */
	unsigned       task_gate_idt_gate_for_trap_number_2_;          /* Gate for Interrupt #2: */
	unsigned       task_gate_idt_gate_for_trap_number_1_;          /* Gate for Interrupt #1: */
	unsigned       task_gate_idt_gate_for_trap_number_0_;          /* Gate for Interrupt #0: */
} e8086_t;


#define e86_get_reg8(cpu, reg) \
	((((reg) & 4) ? ((cpu)->dreg[(reg) & 3] >> 8) : (cpu)->dreg[(reg) & 3]) & 0xff)
                                                          /* Retrieve  8-bit  registers (8080) */
#define e86_get_al(cpu) ((cpu)->dreg[E86_REG_AX] & 0xff)  /* Retrieve  AL CPU register  (8080) */
#define e86_get_bl(cpu) ((cpu)->dreg[E86_REG_BX] & 0xff)  /* Retrieve  BL CPU register  (8080) */
#define e86_get_cl(cpu) ((cpu)->dreg[E86_REG_CX] & 0xff)  /* Retrieve  CL CPU register  (8080) */
#define e86_get_dl(cpu) ((cpu)->dreg[E86_REG_DX] & 0xff)  /* Retrieve  DL CPU register  (8080) */
#define e86_get_ah(cpu) (((cpu)->dreg[E86_REG_AX] >> 8) & 0xff)     /* AH CPU register  (8080) */
#define e86_get_bh(cpu) (((cpu)->dreg[E86_REG_BX] >> 8) & 0xff)     /* BH CPU register  (8080) */
#define e86_get_ch(cpu) (((cpu)->dreg[E86_REG_CX] >> 8) & 0xff)     /* CH CPU register  (8080) */
#define e86_get_dh(cpu) (((cpu)->dreg[E86_REG_DX] >> 8) & 0xff)     /* DH CPU register  (8080) */


#define e86_get_reg16(cpu, reg) ((cpu)->dreg[(reg) & 7])  /* Retrieve 16-bit  registers (8086) */
#define e86_get_ax(cpu) ((cpu)->dreg[E86_REG_AX] & 0xffff)/* Retrieve  AX CPU register  (8086) */
#define e86_get_bx(cpu) ((cpu)->dreg[E86_REG_BX] & 0xffff)/* Retrieve  BX CPU register  (8086) */
#define e86_get_cx(cpu) ((cpu)->dreg[E86_REG_CX] & 0xffff)/* Retrieve  CX CPU register  (8086) */
#define e86_get_dx(cpu) ((cpu)->dreg[E86_REG_DX] & 0xffff)/* Retrieve  DX CPU register  (8086) */
#define e86_get_sp(cpu) ((cpu)->dreg[E86_REG_SP] & 0xffff)/* Retrieve  SP CPU register  (8086) */
#define e86_get_bp(cpu) ((cpu)->dreg[E86_REG_BP] & 0xffff)/* Retrieve  BP CPU register  (8086) */
#define e86_get_si(cpu) ((cpu)->dreg[E86_REG_SI] & 0xffff)/* Retrieve  SI CPU register  (8086) */
#define e86_get_di(cpu) ((cpu)->dreg[E86_REG_DI] & 0xffff)/* Retrieve  DI CPU register  (8086) */

#define e386_get_reg32(cpu, reg) ((cpu)->dreg[(reg) & 7]) /* Retrieve 32-bit  registers(80386) */
#define e386_get_eax(cpu) ((cpu)->dreg[E86_REG_AX])       /* Retrieve eAX CPU register (80386) */
#define e386_get_ebx(cpu) ((cpu)->dreg[E86_REG_BX])       /* Retrieve eBX CPU register (80386) */
#define e386_get_ecx(cpu) ((cpu)->dreg[E86_REG_CX])       /* Retrieve eCX CPU register (80386) */
#define e386_get_edx(cpu) ((cpu)->dreg[E86_REG_DX])       /* Retrieve eDX CPU register (80386) */
#define e386_get_esp(cpu) ((cpu)->dreg[E86_REG_SP])       /* Retrieve eSP CPU register (80386) */
#define e386_get_ebp(cpu) ((cpu)->dreg[E86_REG_BP])       /* Retrieve eBP CPU register (80386) */
#define e386_get_esi(cpu) ((cpu)->dreg[E86_REG_SI])       /* Retrieve eSI CPU register (80386) */
#define e386_get_edi(cpu) ((cpu)->dreg[E86_REG_DI])       /* Retrieve eDI CPU register (80386) */

#define e86_get_sreg(cpu, reg) ((cpu)->sreg[(reg) & 3]) /*Retreive 16-bit segment registers (8086)*/
#define e86_get_cs(cpu) ((cpu)->sreg[E86_REG_CS] & 0xffff) /*   16-bit CS segment register  (8086)*/
#define e86_get_ds(cpu) ((cpu)->sreg[E86_REG_DS] & 0xffff) /*   16-bit DS segment register  (8086)*/
#define e86_get_es(cpu) ((cpu)->sreg[E86_REG_ES] & 0xffff) /*   16-bit ES segment register  (8086)*/
#define e86_get_ss(cpu) ((cpu)->sreg[E86_REG_SS] & 0xffff) /*   16-bit SS segment register  (8086)*/

#define e386_get_sreg(cpu, reg) ((cpu)->sreg[(reg) & 3])/*Retrieve 32-bit segment registers(80386)*/
#define e386_get_cs(cpu) ((cpu)->sreg[E86_REG_CS])  /* Retrieve 32-bit CS segment register (80386)*/
#define e386_get_ds(cpu) ((cpu)->sreg[E86_REG_DS])  /* Retrieve 32-bit DS segment register (80386)*/
#define e386_get_es(cpu) ((cpu)->sreg[E86_REG_ES])  /* Retrieve 32-bit ES segment register (80386)*/
#define e386_get_ss(cpu) ((cpu)->sreg[E86_REG_SS])  /* Retrieve 32-bit SS segment register (80386)*/

#define e386_get_sreg_fs_gs_16(cpu, reg) ((cpu)->sreg386[(reg) & 2])
											 /* Retrieve 16-bit FS and GS segment registers(80386)*/
#define e386_get_fs_16(cpu) ((cpu)->sreg386[E386_REG_FS] & 0xffff) /*  FS segment register (80386)*/
#define e386_get_gs_16(cpu) ((cpu)->sreg386[E386_REG_GS] & 0xffff) /*  GS segment register (80386)*/

#define e386_get_sreg_fs_gs(cpu, reg) ((cpu)->sreg386[(reg) & 2])
											 /* Retrieve 32-bit FS and GS segment registers(80386)*/
#define e386_get_fs(cpu) ((cpu)->sreg386[E386_REG_FS])     /* Retrieve FS segment register (80386)*/
#define e386_get_gs(cpu) ((cpu)->sreg386[E386_REG_GS])     /* Retrieve GS segment register (80386)*/

#define e86_set_reg8(cpu, reg, val) \
	do { \
		unsigned char v = (val) & 0xff; \
		unsigned      r = (reg); \
		if (r & 4) { \
			(cpu)->dreg[r & 3] &= 0x00ff; \
			(cpu)->dreg[r & 3] |= v << 8; \
		} \
		else { \
			(cpu)->dreg[r & 3] &= 0xff00; \
			(cpu)->dreg[r & 3] |= v; \
		} \
	} while (0)                                   /* Set  8-bit general purpose registers  (8080) */

#define e86_set_al(cpu, val) e86_set_reg8 (cpu, E86_REG_AL, val) /* Set  AL CPU register */
#define e86_set_bl(cpu, val) e86_set_reg8 (cpu, E86_REG_BL, val) /* Set  BL CPU register */
#define e86_set_cl(cpu, val) e86_set_reg8 (cpu, E86_REG_CL, val) /* Set  CL CPU register */
#define e86_set_dl(cpu, val) e86_set_reg8 (cpu, E86_REG_DL, val) /* Set  DL CPU register */
#define e86_set_ah(cpu, val) e86_set_reg8 (cpu, E86_REG_AH, val) /* Set  AH CPU register */
#define e86_set_bh(cpu, val) e86_set_reg8 (cpu, E86_REG_BH, val) /* Set  BH CPU register */
#define e86_set_ch(cpu, val) e86_set_reg8 (cpu, E86_REG_CH, val) /* Set  CH CPU register */
#define e86_set_dh(cpu, val) e86_set_reg8 (cpu, E86_REG_DH, val) /* Set  DH CPU register */


#define e86_set_reg16(cpu, reg, val) \
	do { (cpu)->dreg[(reg) & 7] = (val) & 0xffffffff; } while (0)
							    				  /* Set 16-bit general purpose registers  (8086) */

#define e86_set_ax(cpu, val) do { (cpu)->dreg[E86_REG_AX] = (val) & 0xffffffff; } while (0)
												  /* Set  AX CPU register */
#define e86_set_bx(cpu, val) do { (cpu)->dreg[E86_REG_BX] = (val) & 0xffffffff; } while (0)
												  /* Set  BX CPU register */
#define e86_set_cx(cpu, val) do { (cpu)->dreg[E86_REG_CX] = (val) & 0xffffffff; } while (0)
												  /* Set  CX CPU register */
#define e86_set_dx(cpu, val) do { (cpu)->dreg[E86_REG_DX] = (val) & 0xffffffff; } while (0)
												  /* Set  DX CPU register */
#define e86_set_sp(cpu, val) do { (cpu)->dreg[E86_REG_SP] = (val) & 0xffffffff; } while (0)
												  /* Set  SP CPU register */
#define e86_set_bp(cpu, val) do { (cpu)->dreg[E86_REG_BP] = (val) & 0xffffffff; } while (0)
												  /* Set  BP CPU register */
#define e86_set_si(cpu, val) do { (cpu)->dreg[E86_REG_SI] = (val) & 0xffffffff; } while (0)
												  /* Set  SI CPU register */
#define e86_set_di(cpu, val) do { (cpu)->dreg[E86_REG_DI] = (val) & 0xffffffff; } while (0)
												  /* Set  DI CPU register */

#define e386_set_reg32(cpu, reg, val) \
	do { (cpu)->dreg[(reg) & 7] = (val) & 0xffffffff; } while (0)
												  /* Set 32-bit general purpose registers (80386) */

#define e386_set_eax(cpu, val) do { (cpu)->dreg[E86_REG_AX] = (val) & 0xffffffff; } while (0)
												  /* Set eAX CPU register */
#define e386_set_ebx(cpu, val) do { (cpu)->dreg[E86_REG_BX] = (val) & 0xffffffff; } while (0)
												  /* Set eBX CPU register */
#define e386_set_ecx(cpu, val) do { (cpu)->dreg[E86_REG_CX] = (val) & 0xffffffff; } while (0)
												  /* Set eCX CPU register */
#define e386_set_edx(cpu, val) do { (cpu)->dreg[E86_REG_DX] = (val) & 0xffffffff; } while (0)
												  /* Set eDX CPU register */
#define e386_set_esp(cpu, val) do { (cpu)->dreg[E86_REG_SP] = (val) & 0xffffffff; } while (0)
												  /* Set eSP CPU register */
#define e386_set_ebp(cpu, val) do { (cpu)->dreg[E86_REG_BP] = (val) & 0xffffffff; } while (0)
												  /* Set eBP CPU register */
#define e386_set_esi(cpu, val) do { (cpu)->dreg[E86_REG_SI] = (val) & 0xffffffff; } while (0)
												  /* Set eSI CPU register */
#define e386_set_edi(cpu, val) do { (cpu)->dreg[E86_REG_DI] = (val) & 0xffffffff; } while (0)
												  /* Set eDI CPU register */

#define e86_set_sreg(cpu, reg, val) \
	do { (cpu)->sreg[(reg) & 3] = (val) & 0xffff; } while (0)
												  /* Set 16-bit     segment     registers  (8086) */
#define e86_set_cs(cpu, val) do { (cpu)->sreg[E86_REG_CS] = (val) & 0xffff; } while (0)
												  /* Set 16-bit CS */
#define e86_set_ds(cpu, val) do { (cpu)->sreg[E86_REG_DS] = (val) & 0xffff; } while (0)
												  /* Set 16-bit DS */
#define e86_set_es(cpu, val) do { (cpu)->sreg[E86_REG_ES] = (val) & 0xffff; } while (0)
												  /* Set 16-bit ES */
#define e86_set_ss(cpu, val) do { (cpu)->sreg[E86_REG_SS] = (val) & 0xffff; } while (0)
												  /* Set 16-bit SS */

#define e386_set_sreg(cpu, reg, val) \
	do { (cpu)->sreg[(reg) & 3] = (val) & 0xffffffff; } while (0)
												  /* Set 32-bit     segment     registers (80386) */
#define e386_set_cs(cpu, val) do { (cpu)->sreg[E86_REG_CS] = (val) & 0xffffffff; } while (0)
												  /* Set 32-bit CS */
#define e386_set_ds(cpu, val) do { (cpu)->sreg[E86_REG_DS] = (val) & 0xffffffff; } while (0)
												  /* Set 32-bit DS */
#define e386_set_es(cpu, val) do { (cpu)->sreg[E86_REG_ES] = (val) & 0xffffffff; } while (0)
												  /* Set 32-bit ES */
#define e386_set_ss(cpu, val) do { (cpu)->sreg[E86_REG_SS] = (val) & 0xffffffff; } while (0)
												  /* Set 32-bit SS */

#define e386_set_sreg_fs_gs_16(cpu, reg, val) \
	do { (cpu)->sreg386[(reg) & 6] = (val) & 0xffff; } while (0)
												  /*Set 16-bit FS and GS segment registers (80386)*/
#define e386_setxcs_16(cpu, val) do { (cpu)->sreg386[E386_REG_CS] = (val) & 0xffff; } while (0)
												  /* Set 16-bit FS */
#define e386_setxds_16(cpu, val) do { (cpu)->sreg386[E386_REG_DS] = (val) & 0xffff; } while (0)
												  /* Set 16-bit GS */
#define e386_setxes_16(cpu, val) do { (cpu)->sreg386[E386_REG_ES] = (val) & 0xffff; } while (0)
												  /* Set 16-bit GS */
#define e386_set_fs_16(cpu, val) do { (cpu)->sreg386[E386_REG_FS] = (val) & 0xffff; } while (0)
												  /* Set 16-bit FS */
#define e386_set_gs_16(cpu, val) do { (cpu)->sreg386[E386_REG_GS] = (val) & 0xffff; } while (0)
												  /* Set 16-bit GS */
#define e386_setxss_16(cpu, val) do { (cpu)->sreg386[E386_REG_SS] = (val) & 0xffff; } while (0)
												  /* Set 16-bit GS */

#define e386_set_sreg_fs_gs(cpu, reg, val) \
	do { (cpu)->sreg386[(reg) & 6] = (val) & 0xffffffff; } while (0)
												  /*Set 32-bit FS and GS segment registers (80386)*/
#define e386_setxcs(cpu, val) do { (cpu)->sreg386[E386_REG_CS] = (val) & 0xffffffff; } while (0)
												  /* Set 32-bit FS */
#define e386_setxds(cpu, val) do { (cpu)->sreg386[E386_REG_DS] = (val) & 0xffffffff; } while (0)
												  /* Set 32-bit GS */
#define e386_setxes(cpu, val) do { (cpu)->sreg386[E386_REG_ES] = (val) & 0xffffffff; } while (0)
												  /* Set 32-bit GS */
#define e386_set_fs(cpu, val) do { (cpu)->sreg386[E386_REG_FS] = (val) & 0xffffffff; } while (0)
												  /* Set 32-bit FS */
#define e386_set_gs(cpu, val) do { (cpu)->sreg386[E386_REG_GS] = (val) & 0xffffffff; } while (0)
												  /* Set 32-bit GS */
#define e386_setxss(cpu, val) do { (cpu)->sreg386[E386_REG_SS] = (val) & 0xffffffff; } while (0)
												  /* Set 32-bit GS */

#define e86_get_ip(cpu) ((cpu)->ip)
#define e86_get_cur_ip(cpu) ((cpu)->cur_ip)
#define e86_set_ip(cpu, val) do { (cpu)->ip = (val) & 0xffff; } while (0)
												  /* Set 16-bit Instruction Pointer (IP)*/
#define e386_get_eip(cpu) ((cpu)->ip)
#define e386_get_cur_eip(cpu) ((cpu)->cur_ip)
#define e386_set_eip(cpu, val) do { (cpu)->ip = (val) & 0xffffffff; } while (0)
												  /* Set 32-bit Instruction Pointer(eIP)*/
/*NOTE: 32-bit instruction pointer (eIP) register is only available on Intel 80386 CPU and higher */

#define e86_get_flags(cpu) ((cpu)->flg)  /* Retieve  FLAGS register for Intel 80186 CPU and lower */
#define e86_get_f(cpu, f) (((cpu)->flg & (f)) != 0)
#define e86_get_cf(cpu) (((cpu)->flg & E86_FLG_CF) != 0)          /* 00: Carry       Flag (Intel  8086) */
#define e86_get_bit_1(cpu) (((cpu)->flg & E86_FLG_BIT_1) != 0)    /* 01: Reserved (0)     (Intel  8086) */
#define e86_get_pf(cpu) (((cpu)->flg & E86_FLG_PF) != 0)          /* 02: Parity      Flag (Intel  8086) */
#define e86_get_bit_3(cpu) (((cpu)->flg & E86_FLG_BIT_3) != 0)    /* 03: Reserved (0)     (Intel  8086) */
#define e86_get_af(cpu) (((cpu)->flg & E86_FLG_AF) != 0)          /* 04: Accumulator Flag (Intel  8086) */
#define e86_get_bit_5(cpu) (((cpu)->flg & E86_FLG_BIT_5) != 0)    /* 05: Reserved (0)     (Intel  8086) */
#define e86_get_zf(cpu) (((cpu)->flg & E86_FLG_ZF) != 0)          /* 06: Zero        Flag (Intel  8086) */
#define e86_get_of(cpu) (((cpu)->flg & E86_FLG_OF) != 0)          /* 07: Overflow    Flag (Intel  8086) */
#define e86_get_sf(cpu) (((cpu)->flg & E86_FLG_SF) != 0)          /* 08: Sign        Flag (Intel  8086) */
#define e86_get_df(cpu) (((cpu)->flg & E86_FLG_DF) != 0)          /* 09: Direction   Flag (Intel  8086) */
#define e86_get_if(cpu) (((cpu)->flg & E86_FLG_IF) != 0)          /* 10: Interrupt   Flag (Intel  8086) */
#define e86_get_tf(cpu) (((cpu)->flg & E86_FLG_TF) != 0)          /* 11: Trap        Flag (Intel  8086) */
#define e86_get_bit_12(cpu) (((cpu)->flg & E286_FLG_IOPL) != 0)   /* 12: Reserved (0)     (Intel  8086) */
#define e86_get_bit_13(cpu) (((cpu)->flg & E286_FLG_BIT_13) != 0) /* 13: Reserved (0)     (Intel  8086) */
#define e86_get_bit_14(cpu) (((cpu)->flg & E286_FLG_NT) != 0)     /* 14: Reserved (0)     (Intel  8086) */
#define e86_get_bit_15(cpu) (((cpu)->flg & E286_FLG_BIT_15) != 0) /* 15: Reserved (0)     (Intel  8086) */

#define e286_get_flags(cpu) ((cpu)->flg) /* Retrieve  FLAGS register for Intel 80286 CPU */
#define e286_get_f(cpu, f) (((cpu)->flg & (f)) != 0)
#define e286_get_cf(cpu) (((cpu)->flg & E86_FLG_CF) != 0)         /* 00: Carry       Flag (Intel  8086) */
#define e286_get_bit_1(cpu) (((cpu)->flg & E86_FLG_BIT_1) != 0)   /* 01: Reserved (0)     (Intel  8086) */
#define e286_get_pf(cpu) (((cpu)->flg & E86_FLG_PF) != 0)         /* 02: Parity      Flag (Intel  8086) */
#define e286_get_bit_3(cpu) (((cpu)->flg & E86_FLG_BIT_3) != 0)   /* 03: Reserved (0)     (Intel  8086) */
#define e286_get_af(cpu) (((cpu)->flg & E86_FLG_AF) != 0)         /* 04: Accumulator Flag (Intel  8086) */
#define e286_get_bit_5(cpu) (((cpu)->flg & E86_FLG_BIT_5) != 0)   /* 05: Reserved (0)     (Intel  8086) */
#define e286_get_zf(cpu) (((cpu)->flg & E86_FLG_ZF) != 0)         /* 06: Zero        Flag (Intel  8086) */
#define e286_get_of(cpu) (((cpu)->flg & E86_FLG_OF) != 0)         /* 07: Overflow    Flag (Intel  8086) */
#define e286_get_sf(cpu) (((cpu)->flg & E86_FLG_SF) != 0)         /* 08: Sign        Flag (Intel  8086) */
#define e286_get_df(cpu) (((cpu)->flg & E86_FLG_DF) != 0)         /* 09: Direction   Flag (Intel  8086) */
#define e286_get_if(cpu) (((cpu)->flg & E86_FLG_IF) != 0)         /* 10: Interrupt   Flag (Intel  8086) */
#define e286_get_tf(cpu) (((cpu)->flg & E86_FLG_TF) != 0)         /* 11: Trap        Flag (Intel  8086) */
#define e286_get_iopl(cpu) (((cpu)->flg & E286_FLG_IOPL) != 0)    /* 12:                                */
								        /* Input/Output Privilege Level Flag (Intel 80286) */
#define e286_get_bit_13(cpu) (((cpu)->flg & E286_FLG_BIT_13) != 0)/* 13: Reserved (0)     (Intel  8086) */
#define e286_get_nt(cpu) (((cpu)->flg & E286_FLG_NT) != 0)        /* 14: Nested Task Flag (Intel 80286) */
#define e286_get_bit_15(cpu) (((cpu)->flg & E286_FLG_BIT_15) != 0)/* 15: Reserved (0)     (Intel  8086) */

#define e386_get_flags(cpu) ((cpu)->flg)/* Retrieve EFLAGS register for Intel 80386 CPU and higher*/
#define e386_get_f(cpu, f) (((cpu)->flg & (f)) != 0)
#define e386_get_cf(cpu) (((cpu)->flg & E86_FLG_CF) != 0)         /* 00: Carry           Flag (Intel  8086) */
#define e386_get_bit_1(cpu) (((cpu)->flg & E86_FLG_BIT_1) != 0)   /* 01: Reserved (0)         (Intel  8086) */
#define e386_get_pf(cpu) (((cpu)->flg & E86_FLG_PF) != 0)         /* 02: Parity          Flag (Intel  8086) */
#define e386_get_bit_3(cpu) (((cpu)->flg & E86_FLG_BIT_3) != 0)   /* 03: Reserved (0)         (Intel  8086) */
#define e386_get_af(cpu) (((cpu)->flg & E86_FLG_AF) != 0)         /* 04: Accumulator     Flag (Intel  8086) */
#define e386_get_bit_5(cpu) (((cpu)->flg & E86_FLG_BIT_5) != 0)   /* 05: Reserved (0)         (Intel  8086) */
#define e386_get_zf(cpu) (((cpu)->flg & E86_FLG_ZF) != 0)         /* 06: Zero            Flag (Intel  8086) */
#define e386_get_of(cpu) (((cpu)->flg & E86_FLG_OF) != 0)         /* 07: Overflow        Flag (Intel  8086) */
#define e386_get_sf(cpu) (((cpu)->flg & E86_FLG_SF) != 0)         /* 08: Sign            Flag (Intel  8086) */
#define e386_get_df(cpu) (((cpu)->flg & E86_FLG_DF) != 0)         /* 09: Direction       Flag (Intel  8086) */
#define e386_get_if(cpu) (((cpu)->flg & E86_FLG_IF) != 0)         /* 10: Interrupt       Flag (Intel  8086) */
#define e386_get_tf(cpu) (((cpu)->flg & E86_FLG_TF) != 0)         /* 11: Trap            Flag (Intel  8086) */
#define e386_get_iopl(cpu) (((cpu)->flg & E286_FLG_IOPL) != 0)    /* 12:                                    */
										                 /* Input/Output Privilege Level Flag (Intel 80286) */
#define e386_get_bit_13(cpu) (((cpu)->flg & E286_FLG_BIT_13) != 0)/* 13: Reserved (0)         (Intel 80286) */
#define e386_get_nt(cpu) (((cpu)->flg & E286_FLG_NT) != 0)        /* 14: Nested Task Flag     (Intel 80286) */
#define e386_get_bit_15(cpu) (((cpu)->flg & E286_FLG_BIT_15) != 0)/* 15: Reserved (0)         (Intel 80286) */
#define e386_get_rf(cpu) (((cpu)->flg & E386_FLG_RF) != 0)        /* 16: Return          Flag (Intel 80386) */
#define e386_get_vm(cpu) (((cpu)->flg & E386_FLG_VM) != 0)        /* 17: Virtual Machine Flag (Intel 80386) */
#define e386_get_bit_18(cpu) (((cpu)->flg & E386_FLG_BIT_18) != 0)/* 18: Reserved (0)         (Intel 80386) */
#define e386_get_bit_19(cpu) (((cpu)->flg & E386_FLG_BIT_19) != 0)/* 19: Reserved (0)         (Intel 80386) */
#define e386_get_bit_20(cpu) (((cpu)->flg & E386_FLG_BIT_20) != 0)/* 20: Reserved (0)         (Intel 80386) */
#define e386_get_bit_21(cpu) (((cpu)->flg & E386_FLG_BIT_21) != 0)/* 21: Reserved (0)         (Intel 80386) */
#define e386_get_bit_22(cpu) (((cpu)->flg & E386_FLG_BIT_22) != 0)/* 22: Reserved (0)         (Intel 80386) */
#define e386_get_bit_23(cpu) (((cpu)->flg & E386_FLG_BIT_23) != 0)/* 23: Reserved (0)         (Intel 80386) */
#define e386_get_bit_24(cpu) (((cpu)->flg & E386_FLG_BIT_24) != 0)/* 24: Reserved (0)         (Intel 80386) */
#define e386_get_bit_25(cpu) (((cpu)->flg & E386_FLG_BIT_25) != 0)/* 25: Reserved (0)         (Intel 80386) */
#define e386_get_bit_26(cpu) (((cpu)->flg & E386_FLG_BIT_26) != 0)/* 26: Reserved (0)         (Intel 80386) */
#define e386_get_bit_27(cpu) (((cpu)->flg & E386_FLG_BIT_27) != 0)/* 27: Reserved (0)         (Intel 80386) */
#define e386_get_bit_28(cpu) (((cpu)->flg & E386_FLG_BIT_28) != 0)/* 28: Reserved (0)         (Intel 80386) */
#define e386_get_bit_29(cpu) (((cpu)->flg & E386_FLG_BIT_29) != 0)/* 29: Reserved (0)         (Intel 80386) */
#define e386_get_bit_30(cpu) (((cpu)->flg & E386_FLG_BIT_30) != 0)/* 30: Reserved (0)         (Intel 80386) */
#define e386_get_bit_31(cpu) (((cpu)->flg & E386_FLG_BIT_31) != 0)/* 31: Reserved (0)         (Intel 80386) */

#define e86_set_flags(c, v) do { (c)->flg = (v) & 0xffffU; } while (0)      /* Set  FLAGS register for Intel  8086 */

#define e86_set_f(c, f, v) \
	do { if (v) (c)->flg |= (f); else (c)->flg &= ~(f); } while (0)

#define e86_set_cf(c, v) e86_set_f (c, E86_FLG_CF, v)           /* 00: Carry       Flag (Intel 8086/8088 CPU and higher) */
#define e86_set_bit_1(c, v) e86_set_f (c, E86_FLG_BIT_1, v)     /* 01: Reserved (1)     (Intel 8086/8088 CPU and higher) */
#define e86_set_pf(c, v) e86_set_f (c, E86_FLG_PF, v)           /* 02: Parity      Flag (Intel 8086/8088 CPU and higher) */
#define e86_set_bit_3(c, v) e86_set_f (c, E86_FLG_BIT_3, v)     /* 03: Reserved (0)     (Intel 8086/8088 CPU and higher) */
#define e86_set_af(c, v) e86_set_f (c, E86_FLG_AF, v)           /* 04: Accumulator Flag (Intel 8086/8088 CPU and higher) */
#define e86_set_bit_5(c, v) e86_set_f (c, E86_FLG_BIT_5, v)     /* 05: Reserved (0)     (Intel 8086/8088 CPU and higher) */
#define e86_set_zf(c, v) e86_set_f (c, E86_FLG_ZF, v)           /* 06: Zero        Flag (Intel 8086/8088 CPU and higher) */
#define e86_set_of(c, v) e86_set_f (c, E86_FLG_OF, v)           /* 07: Overflow    Flag (Intel 8086/8088 CPU and higher) */
#define e86_set_sf(c, v) e86_set_f (c, E86_FLG_SF, v)           /* 08: Sign        Flag (Intel 8086/8088 CPU and higher) */
#define e86_set_df(c, v) e86_set_f (c, E86_FLG_DF, v)           /* 09: Direction   Flag (Intel 8086/8088 CPU and higher) */
#define e86_set_if(c, v) e86_set_f (c, E86_FLG_IF, v)           /* 10: Interrupt   Flag (Intel 8086/8088 CPU and higher) */
#define e86_set_tf(c, v) e86_set_f (c, E86_FLG_TF, v)           /* 11: Trap        Flag (Intel 8086/8088 CPU and higher) */
#define e86_set_bit_12(c, v) e86_set_f (c, E286_FLG_IOPL, v)    /* 12: Reserved (0)     (Intel 8086/8088 CPU and higher) */
#define e86_set_bit_13(c, v) e86_set_f (c, E286_FLG_BIT_13, v)  /* 13: Reserved (0)     (Intel 8086/8088 CPU and higher) */
#define e86_set_bit_14(c, v) e86_set_f (c, E286_FLG_NT, v)      /* 14: Reserved (0)     (Intel 8086/8088 CPU and higher) */
#define e86_set_bit_15(c, v) e86_set_f (c, E286_FLG_BIT_15, v)  /* 15: Reserved (0)     (Intel 8086/8088 CPU and higher) */

#define e286_set_flags(c, v) do { (c)->flg = (v) & 0xffffffU; } while (0)   /* Set  FLAGS register for Intel 80286 */

#define e286_set_f(c, f, v) \
	do { if (v) (c)->flg |= (f); else (c)->flg &= ~(f); } while (0)

#define e286_set_cf(c, v) e286_set_f (c, E86_FLG_CF, v)         /* 00: Carry       Flag (Intel 8086/8088 CPU and higher) */
#define e286_set_bit_1(c, v) e286_set_f (c, E86_FLG_BIT_1, v)   /* 01: Reserved (1)     (Intel 8086/8088 CPU and higher) */
#define e286_set_pf(c, v) e286_set_f (c, E86_FLG_PF, v)         /* 02: Parity      Flag (Intel 8086/8088 CPU and higher) */
#define e286_set_bit_3(c, v) e286_set_f (c, E86_FLG_BIT_3, v)   /* 03: Reserved (0)     (Intel 8086/8088 CPU and higher) */
#define e286_set_af(c, v) e286_set_f (c, E86_FLG_AF, v)         /* 04: Accumulator Flag (Intel 8086/8088 CPU and higher) */
#define e286_set_bit_5(c, v) e286_set_f (c, E86_FLG_BIT_5, v)   /* 05: Reserved (0)     (Intel 8086/8088 CPU and higher) */
#define e286_set_zf(c, v) e286_set_f (c, E86_FLG_ZF, v)         /* 06: Zero        Flag (Intel 8086/8088 CPU and higher) */
#define e286_set_of(c, v) e286_set_f (c, E86_FLG_OF, v)         /* 07: Overflow    Flag (Intel 8086/8088 CPU and higher) */
#define e286_set_sf(c, v) e286_set_f (c, E86_FLG_SF, v)         /* 08: Sign        Flag (Intel 8086/8088 CPU and higher) */
#define e286_set_df(c, v) e286_set_f (c, E86_FLG_DF, v)         /* 09: Direction   Flag (Intel 8086/8088 CPU and higher) */
#define e286_set_if(c, v) e286_set_f (c, E86_FLG_IF, v)         /* 10: Interrupt   Flag (Intel 8086/8088 CPU and higher) */
#define e286_set_tf(c, v) e286_set_f (c, E86_FLG_TF, v)         /* 11: Trap        Flag (Intel 8086/8088 CPU and higher) */
#define e286_set_iopl(c, v) e286_set_f (c, E286_FLG_IOPL, v)    /* 12: Input/Output Privilege Level Flag (Intel 80286)   */
#define e286_set_bit_13(c, v) e286_set_f (c, E286_FLG_BIT_13, v)/* 13: Reserved (0)     (Intel 8086/8088 CPU and higher) */
#define e286_set_nt(c, v) e286_set_f (c, E286_FLG_NT, v)        /* 14: Nested Task                  Flag (Intel 80286)   */
#define e286_set_bit_15(c, v) e86_set_f (c, E286_FLG_BIT_15, v) /* 15: Reserved (0)     (Intel 8086/8088 CPU and higher) */

#define e386_set_flags(c, v) do { (c)->flg = (v) & 0xffffffffU; } while (0) /* Set EFLAGS register for Intel 80386 */

#define e386_set_f(c, f, v) \
	do { if (v) (c)->flg |= (f); else (c)->flg &= ~(f); } while (0)

#define e386_set_cf(c, v) e386_set_f (c, E86_FLG_CF, v)         /* 00: Carry       Flag (Intel 8086/8088 CPU and higher) */
#define e386_set_bit_1(c, v) e386_set_f (c, E86_FLG_BIT_1, v)   /* 01: Reserved (1)     (Intel 8086/8088 CPU and higher) */
#define e386_set_pf(c, v) e386_set_f (c, E86_FLG_PF, v)         /* 02: Parity      Flag (Intel 8086/8088 CPU and higher) */
#define e386_set_bit_3(c, v) e386_set_f (c, E86_FLG_BIT_3, v)   /* 03: Reserved (0)     (Intel 8086/8088 CPU and higher) */
#define e386_set_af(c, v) e386_set_f (c, E86_FLG_AF, v)         /* 04: Accumulator Flag (Intel 8086/8088 CPU and higher) */
#define e386_set_bit_5(c, v) e386_set_f (c, E86_FLG_BIT_5, v)   /* 05: Reserved (0)     (Intel 8086/8088 CPU and higher) */
#define e386_set_zf(c, v) e386_set_f (c, E86_FLG_ZF, v)         /* 06: Zero        Flag (Intel 8086/8088 CPU and higher) */
#define e386_set_of(c, v) e386_set_f (c, E86_FLG_OF, v)         /* 07: Overflow    Flag (Intel 8086/8088 CPU and higher) */
#define e386_set_sf(c, v) e386_set_f (c, E86_FLG_SF, v)         /* 08: Sign        Flag (Intel 8086/8088 CPU and higher) */
#define e386_set_df(c, v) e386_set_f (c, E86_FLG_DF, v)         /* 09: Direction   Flag (Intel 8086/8088 CPU and higher) */
#define e386_set_if(c, v) e386_set_f (c, E86_FLG_IF, v)         /* 10: Interrupt   Flag (Intel 8086/8088 CPU and higher) */
#define e386_set_tf(c, v) e386_set_f (c, E86_FLG_TF, v)         /* 11: Trap        Flag (Intel 8086/8088 CPU and higher) */
#define e386_set_iopl(c, v) e386_set_f (c, E286_FLG_IOPL, v)    /* 12: Input/Output Privilege Level Flag (Intel 80286)   */
#define e386_set_bit_13(c, v) e386_set_f (c, E286_FLG_BIT_13, v)/* 13: Reserved (0)     (Intel 8086/8088 CPU and higher) */
#define e386_set_nt(c, v) e386_set_f (c, E286_FLG_NT, v)        /* 14: Nested Task                  Flag (Intel 80286)   */
#define e386_set_bit_15(c, v) e386_set_f (c, E286_FLG_BIT_15, v)/* 15: Reserved (0)     (Intel 8086/8088 CPU and higher) */
#define e386_set_rf(c, v) e386_set_f (c, E386_FLG_RF, v)        /* 16: Return                       Flag (Intel 80386)   */
#define e386_set_vm(c, v) e386_set_f (c, E386_FLG_VM, v)        /* 17: Virtual Machine              Flag (Intel 80386)   */
#define e386_set_bit_18(c, v) e386_set_f (c, E386_FLG_BIT_18, v)/* 18: Reserved (0)     (Intel 8086/8088 CPU and higher) */
#define e386_set_bit_19(c, v) e386_set_f (c, E386_FLG_BIT_19, v)/* 19: Reserved (0)     (Intel 8086/8088 CPU and higher) */
#define e386_set_bit_20(c, v) e386_set_f (c, E386_FLG_BIT_20, v)/* 20: Reserved (0)     (Intel 8086/8088 CPU and higher) */
#define e386_set_bit_21(c, v) e386_set_f (c, E386_FLG_BIT_21, v)/* 21: Reserved (0)     (Intel 8086/8088 CPU and higher) */
#define e386_set_bit_22(c, v) e386_set_f (c, E386_FLG_BIT_22, v)/* 22: Reserved (0)     (Intel 8086/8088 CPU and higher) */
#define e386_set_bit_23(c, v) e386_set_f (c, E386_FLG_BIT_23, v)/* 23: Reserved (0)     (Intel 8086/8088 CPU and higher) */
#define e386_set_bit_24(c, v) e386_set_f (c, E386_FLG_BIT_24, v)/* 24: Reserved (0)     (Intel 8086/8088 CPU and higher) */
#define e386_set_bit_25(c, v) e386_set_f (c, E386_FLG_BIT_25, v)/* 25: Reserved (0)     (Intel 8086/8088 CPU and higher) */
#define e386_set_bit_26(c, v) e386_set_f (c, E386_FLG_BIT_26, v)/* 26: Reserved (0)     (Intel 8086/8088 CPU and higher) */
#define e386_set_bit_27(c, v) e386_set_f (c, E386_FLG_BIT_27, v)/* 27: Reserved (0)     (Intel 8086/8088 CPU and higher) */
#define e386_set_bit_28(c, v) e386_set_f (c, E386_FLG_BIT_28, v)/* 28: Reserved (0)     (Intel 8086/8088 CPU and higher) */
#define e386_set_bit_29(c, v) e386_set_f (c, E386_FLG_BIT_29, v)/* 29: Reserved (0)     (Intel 8086/8088 CPU and higher) */
#define e386_set_bit_30(c, v) e386_set_f (c, E386_FLG_BIT_30, v)/* 30: Reserved (0)     (Intel 8086/8088 CPU and higher) */
#define e386_set_bit_31(c, v) e386_set_f (c, E386_FLG_BIT_31, v)/* 31: Reserved (0)     (Intel 8086/8088 CPU and higher) */

#define e86_get_linear(seg, ofs) \
	((((seg) & 0xffffUL) << 4) + ((ofs) & 0xffff))         /* Retrieve 16-bit linear memory address */

#define e386_get_linear(seg, ofs) \
	((((seg) & 0xffffffffUL) << 4) + ((ofs) & 0xffffffff)) /* Retrieve 32-bit linear memory address (Intel 80386) */
/* Machine Status Word (Intel 80286 CPU and higher only) */
#define E286_MSW_PE 0x0001 /* Protected Mode                 */
#define E286_MSW_MP 0x0002 /* Math Coprocessor               */
#define E286_MSW_EM 0x0004 /* Emulate or Monitor Coprocessor */
#define E286_MSW_TS 0x0008 /* Task Switch                    */
#define E386_MSW_ET 0x0010 /* Intel 80387 CPU is present     */
#define E386_MSW_PG 0x0020 /* Paging                         */

/* Machine Status Word (Intel 80286 CPU and higher only) */
#define e286_get_machine_status_word(cpu) ((cpu)->machine_status_word)     /* Retieve Machine Status Word */
#define e286_get_msw(cpu, msw) (((cpu)->machine_status_word & (msw)) != 0)
#define e286_get_pe(cpu) (((cpu)->machine_status_word & E286_MSW_PE) != 0) /* Protected Mode           Flag */
#define e286_get_mp(cpu) (((cpu)->machine_status_word & E286_MSW_MP) != 0) /* Multiprocessor           Flag */
#define e286_get_em(cpu) (((cpu)->machine_status_word & E286_MSW_EM) != 0) /* Emulate Math Coprocessor Flag */
#define e286_get_ts(cpu) (((cpu)->machine_status_word & E286_MSW_TS) != 0) /* Task Switched            Flag */
#define e386_get_et(cpu) (((cpu)->machine_status_word & E386_MSW_ET) != 0) /* Intel 80387 is Present   Flag */
#define e386_get_pg(cpu) (((cpu)->machine_status_word & E386_MSW_PG) != 0) /* Paging is Enabled        Flag */

#define e286_set_machine_status_word(c, v) do { (c)->machine_status_word = (v) & 0xffffffffU; } while (0)
/* Set Machine Status Word */
#define e286_set_msw(c, msw, v) \
	do { if (v) (c)->machine_status_word |= (msw); else (c)->machine_status_word &= ~(msw); } while (0)

#define e286_set_pe(c, v) e286_set_msw (c, E286_MSW_EM, v) /* Protected Mode           Flag */
#define e286_set_mp(c, v) e286_set_msw (c, E286_MSW_MP, v) /* Multiprocessor           Flag */
#define e286_set_em(c, v) e286_set_msw (c, E286_MSW_EM, v) /* Emulate Math Coprocessor Flag */
#define e286_set_ts(c, v) e286_set_msw (c, E286_MSW_TS, v) /* Task Switched            Flag */
#define e386_set_et(c, v) e286_set_msw (c, E386_MSW_ET, v) /* Intel 80387 is Present   Flag */
#define e386_set_pg(c, v) e286_set_msw (c, E386_MSW_PG, v) /* Paging is Enabled        Flag */

#define e86_set_clk(cpu, clk) do { (cpu)->delay += (clk); } while (0)
#define e86_set_clk_ea(cpu, clkreg, clkmem) \
	do { \
		(c)->delay += ((c)->ea.is_mem) ? (clkmem) : (clkreg); \
	} while (0)


static inline
unsigned char e86_get_mem8(e8086_t *c, unsigned short seg, unsigned short ofs)
{
	/* Retrieve  8-bit memory address */
	unsigned long addr = e86_get_linear(seg, ofs) & c->addr_mask;
	return ((addr < c->ram_cnt) ? c->ram[addr] : c->mem_get_uint8(c->mem, addr));
}

static inline
void e86_set_mem8(e8086_t *c, unsigned short seg, unsigned short ofs, unsigned char val)
{
	/* Set       8-bit memory address */
	unsigned long addr = e86_get_linear(seg, ofs) & c->addr_mask;
	if (addr < c->ram_cnt) {
		c->ram[addr] = val;
	}
	else {
		c->mem_set_uint8(c->mem, addr, val);
	}
}

static inline
unsigned short e86_get_mem16(e8086_t *c, unsigned short seg, unsigned short ofs)
{
	/* Retrieve 16-bit memory address */
	unsigned long addr = e86_get_linear(seg, ofs) & c->addr_mask;
	if ((addr + 1) < c->ram_cnt) {
		return (c->ram[addr] + (c->ram[addr + 1] << 8));
	}
	else {
		return (c->mem_get_uint16(c->mem, addr));
	}
}

static inline
void e86_set_mem16(e8086_t *c, unsigned short seg, unsigned short ofs, unsigned short val)
{
	/* Set      16-bit memory address */
	unsigned long addr = e86_get_linear(seg, ofs) & c->addr_mask;
	if ((addr + 1) < c->ram_cnt) {
		c->ram[addr] = val & 0xff;
		c->ram[addr + 1] = (val >> 8) & 0xff;
	}
	else {
		c->mem_set_uint16(c->mem, addr, val);
	}
}

static inline
unsigned short e386_get_mem32(e8086_t *c, unsigned long seg, unsigned long ofs)
{
	/* Retrieve 32-bit memory address (Intel 80386 CPU and higher only) */
	unsigned long addr = e386_get_linear(seg, ofs) & c->addr_mask;
	if ((addr + 1) < c->ram_cnt) {
		return (c->ram[addr] + (c->ram[addr + 1] << 8));
	}
	else {
		return (c->mem_get_uint32(c->mem, addr));
	}
}

static inline
void e386_set_mem32(e8086_t *c, unsigned long seg, unsigned long ofs, unsigned long val)
{
	/* Set      32-bit memory address (Intel 80386 CPU and higher only) */
	unsigned long addr = e386_get_linear(seg, ofs) & c->addr_mask;
	if ((addr + 1) < c->ram_cnt) {
		c->ram[addr] = val & 0xffff;
		c->ram[addr + 1] = (val >> 8) & 0xffff;
	}
	else {
		c->mem_set_uint32(c->mem, addr, val);
	}
}

#define e86_get_prt8(cpu, ofs) \
	(cpu)->prt_get_uint8 ((cpu)->prt, ofs)                         /* Retrieve  8-bit CPU port */
#define e86_get_prt16(cpu, ofs) \
	(cpu)->prt_get_uint16 ((cpu)->prt, ofs)                        /* Retrieve 16-bit CPU port */
#define e86_get_prt32(cpu, ofs) \
	(cpu)->prt_get_uint32 ((cpu)->prt, ofs)                        /* Retrieve 32-bit CPU port */
#define e86_set_prt8(cpu, ofs, val) \
	do { (cpu)->prt_set_uint8 ((cpu)->prt, ofs, val); } while (0)  /* Set       8-bit CPU port */
#define e86_set_prt16(cpu, ofs, val) \
	do { (cpu)->prt_set_uint16 ((cpu)->prt, ofs, val); } while (0) /* Set      16-bit CPU port */
#define e86_set_prt32(cpu, ofs, val) \
	do { (cpu)->prt_set_uint32 ((cpu)->prt, ofs, val); } while (0) /* Set      32-bit CPU port (Intel 80386 CPU) */
#define e86_get_delay(c) ((c)->delay)                              /* Retrieve delay count     */

void e86_init(e8086_t *c); /* Initialize     Intel 8088/8086 CPU or higher */
void e86_free(e8086_t *c); /* Power off      Intel 8088/8086 CPU or higher */
e8086_t *e86_new(void);    /* Add new        Intel 8088/8086 CPU or higher */
void e86_del(e8086_t *c);  /* Remove current Intel 8088/8086 CPU or higher */

void e86_set_8086(e8086_t *c);  /* Intel  8086 CPU */
void e86_set_8086_disassembler(e8086_t *c);  /* Intel  8086 CPU */
void e86_set_8088(e8086_t *c);  /* Intel  8088 CPU */
void e86_set_8088_disassembler(e8086_t *c);  /* Intel  8088 CPU */
void e86_set_80186(e8086_t *c); /* Intel 80186 CPU */
void e86_set_80186_disassembler(e8086_t *c); /* Intel 80186 CPU */
void e86_set_80188(e8086_t *c); /* Intel 80188 CPU */
void e86_set_80188_disassembler(e8086_t *c); /* Intel 80188 CPU */
void e86_set_80286(e8086_t *c); /* Intel 80286 CPU */
void e86_set_80386(e8086_t *c); /* Intel 80386 CPU */
void e86_set_80386_disassembler(e8086_t *c); /* Intel 80386 CPU */
void e86_set_v30(e8086_t *c);   /* NEC    V30  CPU */
void e86_set_v30_disassembler(e8086_t *c);   /* NEC    V30  CPU */
void e86_set_v20(e8086_t *c);   /* NEC    V20  CPU */
void e86_set_v20(e8086_t *c);   /* NEC    V20  CPU */
void e86_set_v20_disassembler(e8086_t *c);   /* NEC    V20  CPU */

/*!***************************************************************************
* @short Set the prefetch queue size
* @param size The emulated prefetch queue size
*****************************************************************************/
void e86_set_pq_size(e8086_t *c, unsigned size);
/*!***************************************************************************
* @short Set CPU options
* @param opt A bit mask indicating the desired options
* @param set If true, the options in opt are set otherwise they are reset
*****************************************************************************/
void e86_set_options(e8086_t *c, unsigned opt, int set);
void e86_set_addr_mask(e8086_t *c, unsigned long msk);
unsigned long e86_get_addr_mask(e8086_t *c);
void e86_set_inta_fct(e8086_t *c, void *ext, void *fct);
void e86_set_ram(e8086_t *c, unsigned char *ram, unsigned long cnt);
void e86_set_mem(e8086_t *c, void *mem,
	e86_get_uint8_f get8, e86_set_uint8_f set8,
	e86_get_uint16_f get16, e86_set_uint16_f set16,
	e86_get_uint32_f get32, e86_set_uint32_f set32
);
void e86_set_prt(e8086_t *c, void *prt,
	e86_get_uint8_f get8, e86_set_uint8_f set8,
	e86_get_uint16_f get16, e86_set_uint16_f set16,
	e86_get_uint32_f get32, e86_set_uint32_f set32
);
int e86_get_reg(e8086_t *c, const char *reg, unsigned long *val);
int e86_set_reg(e8086_t *c, const char *reg, unsigned long val);
void e86_irq(e8086_t *cpu, unsigned char val);
int e86_interrupt(e8086_t *cpu, unsigned n);

unsigned e86_undefined(e8086_t *c);
unsigned long long e86_get_clock(e8086_t *c);
unsigned long long e86_get_opcnt(e8086_t *c);
void e86_reset(e8086_t *c);
void e86_execute(e8086_t *c);
void e86_clock(e8086_t *c, unsigned n);

void e86_push(e8086_t *c, unsigned short val);
unsigned short e86_pop(e8086_t *c);
void e86_trap(e8086_t *c, unsigned n);

void e86_pq_init(e8086_t *c);
void e86_pq_fill(e8086_t *c);


#define E86_DFLAGS_186  0x0001
#define E86_DFLAGS_CALL 0x0100
#define E86_DFLAGS_LOOP 0x0200

typedef struct {
	unsigned       flags;
	unsigned       seg;
	unsigned long  ip; /* Formerly "unsigned short" */
	unsigned       dat_n;
	unsigned char  dat[16];
	char           op[64];
	unsigned       arg_n;
	char           arg1[64];
	char           arg2[64];
} e86_disasm_t;

/* 16-bit disassembler for Intel 80286 CPU and  lower */
void e86_disasm(e86_disasm_t *op, unsigned char *src, unsigned short ip);             /* Instruction Pointer (IP) */
void e86_disasm_mem(e8086_t *c, e86_disasm_t *op, unsigned short, unsigned short ip); /* Instruction Pointer (IP) */
void e86_disasm_cur(e8086_t *c, e86_disasm_t *op);

/* 32-bit disassembler for Intel 80386 CPU and higher */
void e386_disasm(e86_disasm_t *op, unsigned char *src, unsigned long ip);             /* Instruction Pointer(eIP) */
void e386_disasm_mem(e8086_t *c, e86_disasm_t *op, unsigned long, unsigned long ip);  /* Instruction Pointer(eIP) */
void e386_disasm_cur(e8086_t *c, e86_disasm_t *op);

#endif
