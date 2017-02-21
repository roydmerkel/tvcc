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


#include "e8086.h"
#include <libini/libini.h>


/* typedef struct current_privilege_level_t {
	unsigned       cpl;            /* Current privilege level (CPL): "0", "1", "2", or "3" */
/* } current_privilege_level_t; */

/* typedef struct cs_segment_register_descriptor_t {
	unsigned long  cs_segment_register_descriptor_base_address_offset;/* : 16-bit (80286)/32-bit (80386 and higher) */
/* 	unsigned long  cs_segment_register_descriptor_limit; /* Segment limit: 16-bit (80286)/32-bit (80386 and higher) */
	/* NOTE: Change "segment_limit" back to "size" if necessary and required by source code */
/* 	unsigned       cs_segment_register_descriptor_granularity_bit; /* Segment granularity bit: */
/* 	unsigned       cs_segment_register_descriptor_big;             /* Big: "0" or "1" */
/* 	unsigned       cs_segment_register_descriptor_default;     /* Default: "0" or "1" */
/* 	unsigned       cs_segment_register_descriptor_x_;                /* X: "0" or "1" */
/* 	unsigned       cs_segment_register_descriptor_0_;                /* 0: "0" or "1" */
/* 	unsigned       cs_segment_register_descriptor_available;/*AVL(available for use by systems programmers):"0"or"1"*/
/* 	unsigned       cs_segment_register_descriptor_is_present;      /* present: "1" if present and "0" if not present*/
/* 	unsigned       cs_segment_register_descriptor_privilege_level; /* Descriptor privilege level: "1", "2", or "3" */
/* 	unsigned       cs_segment_register_type_of_segment_or_gate;    /* Type of Segment or Gate: */
/* 	unsigned       cs_segment_register_descriptor_type_1;          /* Segment type 1: "0" or "1": */
/* 	unsigned       cs_segment_register_descriptor_type_0;          /* Segment type 0: "0" or "1": */
/* 	unsigned       cs_segment_register_descriptor_type_c;/* conforming: "0" for conforming or "1" for non-conforming: */
/* 	unsigned       cs_segment_register_descriptor_type_e;          /* Segment type expand-down: "0" or "1" */
/* 	unsigned       cs_segment_register_descriptor_type_r;          /* Segment type readable: "0" or "1" */
/* 	unsigned       cs_segment_register_descriptor_type_w;          /* Segment type writable: "0" or "1" */
/* 	unsigned       cs_segment_register_descriptor_accessed;        /* Segment A (accessed): "0" or "1" */
/* 	unsigned       cs_segment_register_selector_table_indicator;   /* Segment table indicator */
/* 	unsigned long  cs_segment_register_selector_index;             /* Segment table indicator */
/* 	unsigned       cs_segment_register_selector_requester_privilege_level;
/* } cs_segment_register_descriptor_t;                                /* Segment requester privilege level */

/* typedef struct ds_segment_register_descriptor_t {
/* 	unsigned long  ds_segment_register_descriptor_base_address_offset;            /* Segment base:  16-bit (80286)/32-bit (80386 and higher) */
/* 	unsigned long  ds_segment_register_descriptor_limit;           /* Segment limit: 16-bit (80286)/32-bit (80386 and higher) */
	/* NOTE: Change "segment_limit" back to "size" if necessary and required by source code */
/* 	unsigned       ds_segment_register_descriptor_granularity_bit; /* Segment granularity bit: */
/* 	unsigned       ds_segment_register_descriptor_big;             /* Big: "0" or "1" */
/* 	unsigned       ds_segment_register_descriptor_default;           /* Default: "0" or "1" */
/* 	unsigned       ds_segment_register_descriptor_x_;              /* X: "0" or "1" */
/* 	unsigned       ds_segment_register_descriptor_0_;              /* 0: "0" or "1" */
/* 	unsigned       ds_segment_register_descriptor_available;       /* Segment AVL (available for use by systems programmers): "0" or "1" */
/* 	unsigned       ds_segment_register_descriptor_is_present;      /* Segment present: "1" if present and "0" if not present */
/* 	unsigned       ds_segment_register_descriptor_privilege_level; /* Descriptor privilege level: "1", "2", or "3" */
/* 	unsigned       ds_segment_register_type_of_segment_or_gate;    /* Type of Segment or Gate: */
/* 	unsigned       ds_segment_register_descriptor_type_1;          /* Segment type 1: "0" or "1": */
/* 	unsigned       ds_segment_register_descriptor_type_0;          /* Segment type 0: "0" or "1": */
/* 	unsigned       ds_segment_register_descriptor_type_c;          /* Segment type conforming: "0" for conforming or "1" for non-conforming: */
/* 	unsigned       ds_segment_register_descriptor_type_e;          /* Segment type expand-down: "0" or "1" */
/* 	unsigned       ds_segment_register_descriptor_type_r;          /* Segment type readable: "0" or "1" */
/* 	unsigned       ds_segment_register_descriptor_type_w;          /* Segment type writable: "0" or "1" */
/* 	unsigned       ds_segment_register_descriptor_accessed;        /* Segment A (accessed): "0" or "1" */
/* 	unsigned       ds_segment_register_selector_table_indicator;   /* Segment table indicator */
/* 	unsigned long  ds_segment_register_selector_index;             /* Segment table indicator */
/* 	unsigned       ds_segment_register_selector_requester_privilege_level;
} ds_segment_register_descriptor_t;                                /* Segment requester privilege level */

/* typedef struct es_segment_register_descriptor_t {
	unsigned long  es_segment_register_descriptor_base_address_offset;            /* Segment base:  16-bit (80286)/32-bit (80386 and higher) */
/* 	unsigned long  es_segment_register_descriptor_limit;           /* Segment limit: 16-bit (80286)/32-bit (80386 and higher) */
	/* NOTE: Change "segment_limit" back to "size" if necessary and required by source code */
/* 	unsigned       es_segment_register_descriptor_granularity_bit; /* Segment granularity bit: */
/* 	unsigned       es_segment_register_descriptor_big;             /* Big: "0" or "1" */
/* 	unsigned       es_segment_register_descriptor_default;           /* Default: "0" or "1" */
/* 	unsigned       es_segment_register_descriptor_x_;              /* X: "0" or "1" */
/* 	unsigned       es_segment_register_descriptor_0_;              /* 0: "0" or "1" */
/* 	unsigned       es_segment_register_descriptor_available;       /* Segment AVL (available for use by systems programmers): "0" or "1" */
/* 	unsigned       es_segment_register_descriptor_is_present;      /* Segment present: "1" if present and "0" if not present */
/* 	unsigned       es_segment_register_descriptor_privilege_level; /* Descriptor privilege level: "1", "2", or "3" */
/* 	unsigned       es_segment_register_type_of_segment_or_gate;    /* Type of Segment or Gate: */
/* 	unsigned       es_segment_register_descriptor_type_1;          /* Segment type 1: "0" or "1": */
/* 	unsigned       es_segment_register_descriptor_type_0;          /* Segment type 0: "0" or "1": */
/* 	unsigned       es_segment_register_descriptor_type_c;          /* Segment type conforming: "0" for conforming or "1" for non-conforming: */
/* 	unsigned       es_segment_register_descriptor_type_e;          /* Segment type expand-down: "0" or "1" */
/* 	unsigned       es_segment_register_descriptor_type_r;          /* Segment type readable: "0" or "1" */
/* 	unsigned       es_segment_register_descriptor_type_w;          /* Segment type writable: "0" or "1" */
/* 	unsigned       es_segment_register_descriptor_accessed;        /* Segment A (accessed): "0" or "1" */
/* 	unsigned       es_segment_register_selector_table_indicator;   /* Segment table indicator */
/* 	unsigned long  es_segment_register_selector_index;             /* Segment table indicator */
/* 	unsigned       es_segment_register_selector_requester_privilege_level;
} es_segment_register_descriptor_t;                                /* Segment requester privilege level */

/* typedef struct fs_segment_register_descriptor_t {
	unsigned long  fs_segment_register_descriptor_base_address_offset;            /* Segment base:  16-bit (80286)/32-bit (80386 and higher) */
/* 	unsigned long  fs_segment_register_descriptor_limit;           /* Segment limit: 16-bit (80286)/32-bit (80386 and higher) */
	/* NOTE: Change "segment_limit" back to "size" if necessary and required by source code */
/* 	unsigned       fs_segment_register_descriptor_granularity_bit; /* Segment granularity bit: */
/* 	unsigned       fs_segment_register_descriptor_big;             /* Big: "0" or "1" */
/* 	unsigned       fs_segment_register_descriptor_default;           /* Default: "0" or "1" */
/* 	unsigned       fs_segment_register_descriptor_x_;              /* X: "0" or "1" */
/* 	unsigned       fs_segment_register_descriptor_0_;              /* 0: "0" or "1" */
/* 	unsigned       fs_segment_register_descriptor_available;       /* Segment AVL (available for use by systems programmers): "0" or "1" */
/* 	unsigned       fs_segment_register_descriptor_is_present;      /* Segment present: "1" if present and "0" if not present */
/* 	unsigned       fs_segment_register_descriptor_privilege_level; /* Descriptor privilege level: "1", "2", or "3" */
/* 	unsigned       fs_segment_register_type_of_segment_or_gate;    /* Type of Segment or Gate: */
/* 	unsigned       fs_segment_register_descriptor_type_1;          /* Segment type 1: "0" or "1": */
/* 	unsigned       fs_segment_register_descriptor_type_0;          /* Segment type 0: "0" or "1": */
/* 	unsigned       fs_segment_register_descriptor_type_c;          /* Segment type conforming: "0" for conforming or "1" for non-conforming: */
/* 	unsigned       fs_segment_register_descriptor_type_e;          /* Segment type expand-down: "0" or "1" */
/* 	unsigned       fs_segment_register_descriptor_type_r;          /* Segment type readable: "0" or "1" */
/* 	unsigned       fs_segment_register_descriptor_type_w;          /* Segment type writable: "0" or "1" */
/* 	unsigned       fs_segment_register_descriptor_accessed;        /* Segment A (accessed): "0" or "1" */
/* 	unsigned       fs_segment_register_selector_table_indicator;   /* Segment table indicator */
/* 	unsigned long  fs_segment_register_selector_index;             /* Segment table indicator */
/* 	unsigned       fs_segment_register_selector_requester_privilege_level;
} fs_segment_register_descriptor_t;                                /* Segment requester privilege level */

/* typedef struct gs_segment_register_descriptor_t {
	unsigned long  gs_segment_register_descriptor_base_address_offset;            /* Segment base:  16-bit (80286)/32-bit (80386 and higher) */
/* 	unsigned long  gs_segment_register_descriptor_limit;           /* Segment limit: 16-bit (80286)/32-bit (80386 and higher) */
	/* NOTE: Change "segment_limit" back to "size" if necessary and required by source code */
/* 	unsigned       gs_segment_register_descriptor_granularity_bit; /* Segment granularity bit: */
/* 	unsigned       gs_segment_register_descriptor_big;             /* Big: "0" or "1" */
/* 	unsigned       gs_segment_register_descriptor_default;           /* Default: "0" or "1" */
/* 	unsigned       gs_segment_register_descriptor_x_;              /* X: "0" or "1" */
/* 	unsigned       gs_segment_register_descriptor_0_;              /* 0: "0" or "1" */
/* 	unsigned       gs_segment_register_descriptor_available;       /* Segment AVL (available for use by systems programmers): "0" or "1" */
/* 	unsigned       gs_segment_register_descriptor_is_present;      /* Segment present: "1" if present and "0" if not present */
/* 	unsigned       gs_segment_register_descriptor_privilege_level; /* Descriptor privilege level: "1", "2", or "3" */
/* 	unsigned       gs_segment_register_type_of_segment_or_gate;    /* Type of Segment or Gate: */
/* 	unsigned       gs_segment_register_descriptor_type_1;          /* Segment type 1: "0" or "1": */
/* 	unsigned       gs_segment_register_descriptor_type_0;          /* Segment type 0: "0" or "1": */
/* 	unsigned       gs_segment_register_descriptor_type_c;          /* Segment type conforming: "0" for conforming or "1" for non-conforming: */
/* 	unsigned       gs_segment_register_descriptor_type_e;          /* Segment type expand-down: "0" or "1" */
/* 	unsigned       gs_segment_register_descriptor_type_r;          /* Segment type readable: "0" or "1" */
/* 	unsigned       gs_segment_register_descriptor_type_w;          /* Segment type writable: "0" or "1" */
/* 	unsigned       gs_segment_register_descriptor_accessed;        /* Segment A (accessed): "0" or "1" */
/* 	unsigned       gs_segment_register_selector_table_indicator;   /* Segment table indicator */
/* 	unsigned long  gs_segment_register_selector_index;             /* Segment table indicator */
/* 	unsigned       gs_segment_register_selector_requester_privilege_level;
} gs_segment_register_descriptor_t;                                /* Segment requester privilege level */

/* typedef struct ss_segment_register_descriptor_t {
/* Current Privilege Level: */
/* 	unsigned long  ss_segment_register_descriptor_base_address_offset;            /* Segment base:  16-bit (80286)/32-bit (80386 and higher) */
/* 	unsigned long  ss_segment_register_descriptor_limit;           /* Segment limit: 16-bit (80286)/32-bit (80386 and higher) */
	/* NOTE: Change "segment_limit" back to "size" if necessary and required by source code */
/* 	unsigned       ss_segment_register_descriptor_granularity_bit; /* Segment granularity bit: */
/* 	unsigned       ss_segment_register_descriptor_big;             /* Big: "0" or "1" */
/* 	unsigned       ss_segment_register_descriptor_default;           /* Default: "0" or "1" */
/* 	unsigned       ss_segment_register_descriptor_x_;              /* X: "0" or "1" */
/* 	unsigned       ss_segment_register_descriptor_0_;              /* 0: "0" or "1" */
/* 	unsigned       ss_segment_register_descriptor_available;       /* Segment AVL (available for use by systems programmers): "0" or "1" */
/* 	unsigned       ss_segment_register_descriptor_is_present;      /* Segment present: "1" if present and "0" if not present */
/* 	unsigned       ss_segment_register_descriptor_privilege_level; /* Descriptor privilege level: "1", "2", or "3" */
/* 	unsigned       ss_segment_register_type_of_segment_or_gate;    /* Type of Segment or Gate: */
/* 	unsigned       ss_segment_register_descriptor_type_1;          /* Segment type 1: "0" or "1": */
/* 	unsigned       ss_segment_register_descriptor_type_0;          /* Segment type 0: "0" or "1": */
/* 	unsigned       ss_segment_register_descriptor_type_c;          /* Segment type conforming: "0" for conforming or "1" for non-conforming: */
/* 	unsigned       ss_segment_register_descriptor_type_e;          /* Segment type expand-down: "0" or "1" */
/* 	unsigned       ss_segment_register_descriptor_type_r;          /* Segment type readable: "0" or "1" */
/* 	unsigned       ss_segment_register_descriptor_type_w;          /* Segment type writable: "0" or "1" */
/* 	unsigned       ss_segment_register_descriptor_accessed;        /* Segment A (accessed): "0" or "1" */
/* 	unsigned       ss_segment_register_selector_table_indicator;   /* Segment table indicator */
/* 	unsigned long  ss_segment_register_selector_index;             /* Segment table indicator */
/* 	unsigned       ss_segment_register_selector_requester_privilege_level;
/* Privilege Level 2: */
/* 	unsigned long  ss_segment_register_descriptor_base_address_offset_for_privilege_level_2;            /* Segment base:  16-bit (80286)/32-bit (80386 and higher) */
/* 	unsigned long  ss_segment_register_descriptor_limit_for_privilege_level_2;           /* Segment limit: 16-bit (80286)/32-bit (80386 and higher) */
	/* NOTE: Change "segment_limit" back to "size" if necessary and required by source code */
/* 	unsigned       ss_segment_register_descriptor_granularity_bit_for_privilege_level_2; /* Segment granularity bit: */
/* 	unsigned       ss_segment_register_descriptor_big_for_privilege_level_2;             /* Big: "0" or "1" */
/* 	unsigned       ss_segment_register_descriptor_default_for_privilege_level_2;           /* Default: "0" or "1" */
/* 	unsigned       ss_segment_register_descriptor_x__for_privilege_level_2;              /* X: "0" or "1" */
/* 	unsigned       ss_segment_register_descriptor_0__for_privilege_level_2;              /* 0: "0" or "1" */
/* 	unsigned       ss_segment_register_descriptor_available_for_privilege_level_2;       /* Segment AVL (available for use by systems programmers): "0" or "1" */
/* 	unsigned       ss_segment_register_descriptor_is_present_for_privilege_level_2;      /* Segment present: "1" if present and "0" if not present */
/* 	unsigned       ss_segment_register_descriptor_privilege_level_for_privilege_level_2; /* Descriptor privilege level: "1", "2", or "3" */
/* 	unsigned       ss_segment_register_type_of_segment_or_gate_for_privilege_level_2;    /* Type of Segment or Gate: */
/* 	unsigned       ss_segment_register_descriptor_type_1_for_privilege_level_2;          /* Segment type 1: "0" or "1": */
/* 	unsigned       ss_segment_register_descriptor_type_0_for_privilege_level_2;          /* Segment type 0: "0" or "1": */
/* 	unsigned       ss_segment_register_descriptor_type_c_for_privilege_level_2;          /* Segment type conforming: "0" for conforming or "1" for non-conforming: */
/* 	unsigned       ss_segment_register_descriptor_type_e_for_privilege_level_2;          /* Segment type expand-down: "0" or "1" */
/* 	unsigned       ss_segment_register_descriptor_type_r_for_privilege_level_2;          /* Segment type readable: "0" or "1" */
/* 	unsigned       ss_segment_register_descriptor_type_w_for_privilege_level_2;          /* Segment type writable: "0" or "1" */
/* 	unsigned       ss_segment_register_descriptor_accessed_for_privilege_level_2;        /* Segment A (accessed): "0" or "1" */
/* 	unsigned       ss_segment_register_selector_table_indicator_for_privilege_level_2;   /* Segment table indicator */
/* 	unsigned long  ss_segment_register_selector_index_for_privilege_level_2;             /* Segment table indicator */
/* 	unsigned       ss_segment_register_selector_requester_privilege_level_for_privilege_level_2;
/* Privilege Level 1: */
/* 	unsigned long  ss_segment_register_descriptor_base_address_offset_for_privilege_level_1;            /* Segment base:  16-bit (80286)/32-bit (80386 and higher) */
/* 	unsigned long  ss_segment_register_descriptor_limit_for_privilege_level_1;           /* Segment limit: 16-bit (80286)/32-bit (80386 and higher) */
	/* NOTE: Change "segment_limit" back to "size" if necessary and required by source code */
/* 	unsigned       ss_segment_register_descriptor_granularity_bit_for_privilege_level_1; /* Segment granularity bit: */
/* 	unsigned       ss_segment_register_descriptor_big_for_privilege_level_1;             /* Big: "0" or "1" */
/* 	unsigned       ss_segment_register_descriptor_default_for_privilege_level_1;           /* Default: "0" or "1" */
/* 	unsigned       ss_segment_register_descriptor_x__for_privilege_level_1;              /* X: "0" or "1" */
/* 	unsigned       ss_segment_register_descriptor_0__for_privilege_level_1;              /* 0: "0" or "1" */
/* 	unsigned       ss_segment_register_descriptor_available_for_privilege_level_1;       /* Segment AVL (available for use by systems programmers): "0" or "1" */
/* 	unsigned       ss_segment_register_descriptor_is_present_for_privilege_level_1;      /* Segment present: "1" if present and "0" if not present */
/* 	unsigned       ss_segment_register_descriptor_privilege_level_for_privilege_level_1; /* Descriptor privilege level: "1", "2", or "3" */
/* 	unsigned       ss_segment_register_type_of_segment_or_gate_for_privilege_level_1;    /* Type of Segment or Gate: */
/* 	unsigned       ss_segment_register_descriptor_type_1_for_privilege_level_1;          /* Segment type 1: "0" or "1": */
/* 	unsigned       ss_segment_register_descriptor_type_0_for_privilege_level_1;          /* Segment type 0: "0" or "1": */
/* 	unsigned       ss_segment_register_descriptor_type_c_for_privilege_level_1;          /* Segment type conforming: "0" for conforming or "1" for non-conforming: */
/* 	unsigned       ss_segment_register_descriptor_type_e_for_privilege_level_1;          /* Segment type expand-down: "0" or "1" */
/* 	unsigned       ss_segment_register_descriptor_type_r_for_privilege_level_1;          /* Segment type readable: "0" or "1" */
/* 	unsigned       ss_segment_register_descriptor_type_w_for_privilege_level_1;          /* Segment type writable: "0" or "1" */
/* 	unsigned       ss_segment_register_descriptor_accessed_for_privilege_level_1;        /* Segment A (accessed): "0" or "1" */
/* 	unsigned       ss_segment_register_selector_table_indicator_for_privilege_level_1;   /* Segment table indicator */
/* 	unsigned long  ss_segment_register_selector_index_for_privilege_level_1;             /* Segment table indicator */
/* 	unsigned       ss_segment_register_selector_requester_privilege_level_for_privilege_level_1;
/* Privilege Level 0: */
/* 	unsigned long  ss_segment_register_descriptor_base_address_offset_for_privilege_level_0;            /* Segment base:  16-bit (80286)/32-bit (80386 and higher) */
/* 	unsigned long  ss_segment_register_descriptor_limit_for_privilege_level_0;           /* Segment limit: 16-bit (80286)/32-bit (80386 and higher) */
	/* NOTE: Change "segment_limit" back to "size" if necessary and required by source code */
/* 	unsigned       ss_segment_register_descriptor_granularity_bit_for_privilege_level_0; /* Segment granularity bit: */
/* 	unsigned       ss_segment_register_descriptor_big_for_privilege_level_0;             /* Big: "0" or "1" */
/* 	unsigned       ss_segment_register_descriptor_default_for_privilege_level_0;           /* Default: "0" or "1" */
/* 	unsigned       ss_segment_register_descriptor_x__for_privilege_level_0;              /* X: "0" or "1" */
/* 	unsigned       ss_segment_register_descriptor_0__for_privilege_level_0;              /* 0: "0" or "1" */
/* 	unsigned       ss_segment_register_descriptor_available_for_privilege_level_0;       /* Segment AVL (available for use by systems programmers): "0" or "1" */
/* 	unsigned       ss_segment_register_descriptor_is_present_for_privilege_level_0;      /* Segment present: "1" if present and "0" if not present */
/* 	unsigned       ss_segment_register_descriptor_privilege_level_for_privilege_level_0; /* Descriptor privilege level: "1", "2", or "3" */
/* 	unsigned       ss_segment_register_type_of_segment_or_gate_for_privilege_level_0;    /* Type of Segment or Gate: */
/* 	unsigned       ss_segment_register_descriptor_type_1_for_privilege_level_0;          /* Segment type 1: "0" or "1": */
/* 	unsigned       ss_segment_register_descriptor_type_0_for_privilege_level_0;          /* Segment type 0: "0" or "1": */
/* 	unsigned       ss_segment_register_descriptor_type_c_for_privilege_level_0;          /* Segment type conforming: "0" for conforming or "1" for non-conforming: */
/* 	unsigned       ss_segment_register_descriptor_type_e_for_privilege_level_0;          /* Segment type expand-down: "0" or "1" */
/* 	unsigned       ss_segment_register_descriptor_type_r_for_privilege_level_0;          /* Segment type readable: "0" or "1" */
/* 	unsigned       ss_segment_register_descriptor_type_w_for_privilege_level_0;          /* Segment type writable: "0" or "1" */
/* 	unsigned       ss_segment_register_descriptor_accessed_for_privilege_level_0;        /* Segment A (accessed): "0" or "1" */
/* 	unsigned       ss_segment_register_selector_table_indicator_for_privilege_level_0;   /* Segment table indicator */
/* 	unsigned long  ss_segment_register_selector_index_for_privilege_level_0;             /* Segment table indicator */
/* 	unsigned       ss_segment_register_selector_requester_privilege_level_for_privilege_level_0;
} ss_segment_register_descriptor_t;                                /* Segment requester privilege level */

/* typedef struct cr3_pdbr_register_for_current_page_table_t {
	unsigned long  pdbr_page_frame_address_specified;          /* Page frame address:  32-bit (80386 and higher) */
/* 	unsigned long  pdbr_page_frame_address_being_loaded;       /* Page frame address:  32-bit (80386 and higher) */
/* 	unsigned long  pdbr_page_frame_address;                    /* Page frame address:  32-bit (80386 and higher) */
/* 	unsigned       pdbr_page_table_is_present;                 /* Page table present: "1" if present and "0" if not present */
/* 	unsigned       pdbr_page_table_read_write;                 /* Page table read/write: "0" if read-only and "1" if read/write */
/* 	unsigned       pdbr_page_table_user_supervisor;            /* Page table user/supervisor: "0" if user and "1" if supervisor: */
/* 	unsigned       pdbr_page_table_dirty;                      /* Page table Default: "0" or "1" */
/* 	unsigned       pdbr_page_table_available;                  /* Page table available for systems programmer use: "0" or "1" */
/* } cr3_pdbr_register_for_current_page_table_t; */

typedef struct directory_page_table_t {
	unsigned long  directory_page_frame_address_specified;          /* Page frame address:  32-bit (80386 and higher) */
	unsigned long  directory_page_frame_address_being_loaded;       /* Page frame address:  32-bit (80386 and higher) */
	unsigned long  directory_page_frame_address;                    /* Page frame address:  32-bit (80386 and higher) */
	unsigned       directory_page_table_read_write;                 /* Page table read/write: "0" if read-only and "1" if read/write */
	unsigned       directory_page_table_user_supervisor;            /* Page table user/supervisor: "0" if user and "1" if supervisor: */
	unsigned       directory_page_table_dirty;                      /* Page table Default: "0" or "1" */
	unsigned       directory_page_table_available;                  /* Page table available for systems programmer use: "0" or "1" */
} directory_page_table_t;

typedef struct {
	directory_page_table_t      **directory_page;
} directory_page_t;

typedef struct entry_page_table_t {
	unsigned long  entry_page_frame_address_specified;          /* Page frame address:  32-bit (80386 and higher) */
	unsigned long  entry_page_frame_address_being_loaded;       /* Page frame address:  32-bit (80386 and higher) */
	unsigned long  entry_page_frame_address;                    /* Page frame address:  32-bit (80386 and higher) */
	unsigned       entry_page_table_is_present;                 /* Page table present: "1" if present and "0" if not present */
	unsigned       entry_page_table_read_write;                 /* Page table read/write: "0" if read-only and "1" if read/write */
	unsigned       entry_page_table_user_supervisor;            /* Page table user/supervisor: "0" if user and "1" if supervisor: */
	unsigned       entry_page_table_dirty;                      /* Page table Default: "0" or "1" */
	unsigned       entry_page_table_available;                  /* Page table available for systems programmer use: "0" or "1" */
} entry_page_table_t;

typedef struct {
	entry_page_table_t      **entry_page;
} entry_page_t;

/* typedef struct gdtr_register_for_global_descriptor_t {
	unsigned long  gdtr_register_for_global_descriptor_base_address_offset;            /* Segment base:  16-bit (80286)/32-bit (80386 and higher) */
/* 	unsigned long  gdtr_register_for_global_descriptor_limit;           /* Segment limit: 16-bit (80286)/32-bit (80386 and higher) */
	/* NOTE: Change "segment_limit" back to "size" if necessary and required by source code */
/* 	unsigned       gdtr_register_for_global_descriptor_granularity_bit; /* Segment granularity bit: */
/* 	unsigned       gdtr_register_for_global_descriptor_big;             /* Big: "0" or "1" */
/* 	unsigned       gdtr_register_for_global_descriptor_default;           /* Default: "0" or "1" */
/* 	unsigned       gdtr_register_for_global_descriptor_x_;              /* X: "0" or "1" */
/* 	unsigned       gdtr_register_for_global_descriptor_0_;              /* 0: "0" or "1" */
/* 	unsigned       gdtr_register_for_global_descriptor_available;       /* Segment AVL (available for use by systems programmers): "0" or "1" */
/* 	unsigned       gdtr_register_for_global_descriptor_is_present;      /* Segment present: "1" if present and "0" if not present */
/* 	unsigned       gdtr_register_for_global_descriptor_privilege_level; /* Descriptor privilege level: "1", "2", or "3" */
/* 	unsigned       gdtr_register_for_global_descriptor_type_of_segment_or_gate;          /* Type of Segment or Gate: */
/* 	unsigned       gdtr_register_for_global_descriptor_type_1;      /* Segment type 1: "0" or "1": */
/* 	unsigned       gdtr_register_for_global_descriptor_type_0;      /* Segment type 0: "0" or "1": */
/* 	unsigned       gdtr_register_for_global_descriptor_type_c;      /* Segment type conforming: "0" for conforming or "1" for non-conforming: */
/* 	unsigned       gdtr_register_for_global_descriptor_type_e;          /* Segment type expand-down: "0" or "1" */
/* 	unsigned       gdtr_register_for_global_descriptor_type_r;          /* Segment type readable: "0" or "1" */
/* 	unsigned       gdtr_register_for_global_descriptor_type_w;          /* Segment type writable: "0" or "1" */
/* 	unsigned       gdtr_register_for_global_descriptor_accessed;        /* Segment A (accessed): "0" or "1" */
/* 	unsigned       gdtr_register_for_global_descriptor_selector_table_indicator;        /* Segment table indicator */
/* 	unsigned long  gdtr_register_for_global_descriptor_selector_index;  /* Segment table indicator */
/* 	unsigned       gdtr_register_for_global_descriptor_selector_requester_privilege_level;        /* Segment requester privilege level */
/* 	unsigned long  gdtr_register_for_global_descriptor_callgate_offset;    /* Segment 80386 call gate offset */
/* 	unsigned       gdtr_register_for_global_descriptor_callgate_is_present;/* Segment call gate present: "1" if present and "0" if not present */
/* 	unsigned       gdtr_register_for_global_descriptor_callgate_type;      /* See "0110" for 80386 call gate */
/* 	unsigned       gdtr_register_for_global_descriptor_callgate_000;       /* */
/* 	unsigned       gdtr_register_for_global_descriptor_callgate_dword_count; /* Segment call gate DWORD count (32-bit) */
/* } gdtr_register_for_global_descriptor_t;

typedef struct ldtr_register_for_local_descriptor_t {
	unsigned long  ldtr_register_for_local_descriptor_base_address_offset;            /* Segment base:  16-bit (80286)/32-bit (80386 and higher) */
/* 	unsigned long  ldtr_register_for_local_descriptor_limit;           /* Segment limit: 16-bit (80286)/32-bit (80386 and higher) */
	/* NOTE: Change "segment_limit" back to "size" if necessary and required by source code */
/* 	unsigned       ldtr_register_for_local_descriptor_granularity_bit; /* Segment granularity bit: */
/* 	unsigned       ldtr_register_for_local_descriptor_big;             /* Big: "0" or "1" */
/* 	unsigned       ldtr_register_for_local_descriptor_default;           /* Default: "0" or "1" */
/* 	unsigned       ldtr_register_for_local_descriptor_x_;              /* X: "0" or "1" */
/* 	unsigned       ldtr_register_for_local_descriptor_0_;              /* 0: "0" or "1" */
/* 	unsigned       ldtr_register_for_local_descriptor_available;       /* Segment AVL (available for use by systems programmers): "0" or "1" */
/* 	unsigned       ldtr_register_for_local_descriptor_is_present;      /* Segment present: "1" if present and "0" if not present */
/* 	unsigned       ldtr_register_for_local_descriptor_privilege_level; /* Descriptor privilege level: "1", "2", or "3" */
/* 	unsigned       ldtr_register_for_local_descriptor_type_of_segment_or_gate;          /* Type of Segment or Gate: */
/* 	unsigned       ldtr_register_for_local_descriptor_type_1;      /* Segment type 1: "0" or "1": */
/* 	unsigned       ldtr_register_for_local_descriptor_type_0;      /* Segment type 0: "0" or "1": */
/* 	unsigned       ldtr_register_for_local_descriptor_type_c;      /* Segment type conforming: "0" for conforming or "1" for non-conforming: */
/* 	unsigned       ldtr_register_for_local_descriptor_type_e;          /* Segment type expand-down: "0" or "1" */
/* 	unsigned       ldtr_register_for_local_descriptor_type_r;          /* Segment type readable: "0" or "1" */
/* 	unsigned       ldtr_register_for_local_descriptor_type_w;          /* Segment type writable: "0" or "1" */
/* 	unsigned       ldtr_register_for_local_descriptor_accessed;        /* Segment A (accessed): "0" or "1" */
/* 	unsigned       ldtr_register_for_local_descriptor_selector_table_indicator;        /* Segment table indicator */
/* 	unsigned long  ldtr_register_for_local_descriptor_selector_index;  /* Segment table indicator */
/* 	unsigned       ldtr_register_for_local_descriptor_selector_requester_privilege_level;        /* Segment requester privilege level */
/* 	unsigned long  ldtr_register_for_local_descriptor_callgate_offset;    /* Segment 80386 call gate offset */
/* 	unsigned       ldtr_register_for_local_descriptor_callgate_is_present;/* Segment call gate present: "1" if present and "0" if not present */
/* 	unsigned       ldtr_register_for_local_descriptor_callgate_type;      /* See "0110" for 80386 call gate */
/* 	unsigned       ldtr_register_for_local_descriptor_callgate_000;       /* */
/* 	unsigned       ldtr_register_for_local_descriptor_callgate_dword_count; /* Segment call gate DWORD count (32-bit) */
/* } ldtr_register_for_local_descriptor_t; */


typedef struct {
	unsigned long  gdt_segment_base_address_offset;            /* Segment base:  16-bit (80286)/32-bit (80386 and higher) */
	unsigned long  gdt_segment_limit;           /* Segment limit: 16-bit (80286)/32-bit (80386 and higher) */
	/* NOTE: Change "segment_limit" back to "size" if necessary and required by source code */
	/* unsigned char  alloc; */
	unsigned       gdt_granularity_bit; /* Segment granularity bit: */
	unsigned       gdt_big;             /* Big: "0" or "1" */
	unsigned       gdt_default;           /* Default: "0" or "1" */
	unsigned       gdt_x_;              /* X: "0" or "1" */
	unsigned       gdt_0_;              /* 0: "0" or "1" */
	unsigned       gdt_segment_available;       /* Segment AVL (available for use by systems programmers): "0" or "1" */
	unsigned       gdt_segment_is_present;      /* Segment present: "1" if present and "0" if not present */
	unsigned       gdt_descriptor_privilege_level; /* Descriptor privilege level: "1", "2", or "3" */
	unsigned       gdt_type_of_segment_or_gate;          /* Type of Segment or Gate: */
	unsigned       gdt_type_1;          /* Segment type 1: "0" or "1": */
	unsigned       gdt_type_0;          /* Segment type 0: "0" or "1": */
	unsigned       gdt_type_c;          /* Segment type conforming: "0" for conforming or "1" for non-conforming: */
	unsigned       gdt_type_e;          /* Segment type expand-down: "0" or "1" */
	unsigned       gdt_type_r;          /* Segment type readable: "0" or "1" */
	unsigned       gdt_type_w;          /* Segment type writable: "0" or "1" */
	unsigned       gdt_segment_accessed;        /* Segment A (accessed): "0" or "1" */
	unsigned       gdt_selector_table_indicator;        /* Segment table indicator */
	unsigned long  *gdt_selector_index;        /* Segment table indicator */
	unsigned       gdt_selector_requester_privilege_level;        /* Segment requester privilege level */
	unsigned long  gdt_callgate_offset;        /* Segment 80386 call gate offset */
	unsigned       gdt_callgate_is_present;        /* Segment call gate present: "1" if present and "0" if not present */
	unsigned       gdt_callgate_type;        /* See "0110" for 80386 call gate */
	unsigned       gdt_callgate_000;        /* */
	unsigned       gdt_callgate_dword_count;        /* Segment call gate DWORD count (32-bit) */
} gdt_global_descriptor_t;

/* typedef struct gdt_count_t {
	unsigned short gdt_number_of_global_segment_descriptors_in_use; /* Number of segment descriptors in use for Global Descriptor Table */
/* } gdt_count_t; */

typedef struct {
	gdt_global_descriptor_t      **gdt;
} gdt_global_t;

typedef struct ldt_local_descriptor_t {
	unsigned long  ldt_segment_base_address_offset;            /* Segment base:  16-bit (80286)/32-bit (80386 and higher) */
	unsigned long  ldt_segment_limit;           /* Segment limit: 16-bit (80286)/32-bit (80386 and higher) */
	/* NOTE: Change "segment_limit" back to "size" if necessary and required by source code */
	/* unsigned char  alloc; */
	unsigned       ldt_granularity_bit; /* Segment granularity bit: */
	unsigned       ldt_big;             /* Big: "0" or "1" */
	unsigned       ldt_default;           /* Default: "0" or "1" */
	unsigned       ldt_x_;              /* X: "0" or "1" */
	unsigned       ldt_0_;              /* 0: "0" or "1" */
	unsigned       ldt_segment_available;       /* Segment AVL (available for use by systems programmers): "0" or "1" */
	unsigned       ldt_segment_is_present;      /* Segment present: "1" if present and "0" if not present */
	unsigned       ldt_descriptor_privilege_level; /* Descriptor privilege level: "1", "2", or "3" */
	unsigned       ldt_type_of_segment_or_gate;          /* Type of Segment or Gate: */
	unsigned       ldt_type_1;          /* Segment type 1: "0" or "1": */
	unsigned       ldt_type_0;          /* Segment type 0: "0" or "1": */
	unsigned       ldt_type_c;          /* Segment type conforming: "0" for conforming or "1" for non-conforming: */
	unsigned       ldt_type_e;          /* Segment type expand-down: "0" or "1" */
	unsigned       ldt_type_r;          /* Segment type readable: "0" or "1" */
	unsigned       ldt_type_w;          /* Segment type writable: "0" or "1" */
	unsigned       ldt_segment_accessed;        /* Segment A (accessed): "0" or "1" */
	unsigned       ldt_selector_table_indicator;        /* Segment table indicator */
	unsigned long  ldt_selector_index;        /* Segment table indicator */
	unsigned       ldt_selector_requester_privilege_level;        /* Segment requester privilege level */
	unsigned long  ldt_callgate_offset;        /* Segment 80386 call gate offset */
	unsigned       ldt_callgate_is_present;        /* Segment call gate present: "1" if present and "0" if not present */
	unsigned       ldt_callgate_type;        /* See "0110" for 80386 call gate */
	unsigned       ldt_callgate_000;        /* */
	unsigned       ldt_callgate_dword_count;        /* Segment call gate DWORD count (32-bit) */
} ldt_local_descriptor_t;

typedef struct ldt_count_t {
	unsigned short ldt_number_of_local_segment_descriptors_in_use; /* Number of segment descriptors in use for Local Descriptor Table */
} ldt_count_t;

typedef struct {
	ldt_local_descriptor_t      **local_descriptor;
} ldt_local_t;
/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual; 9.4 Interrupt Descriptor Table:                            */
/* ---------------------------------------------------------------------------------------------------------------- */
/* The interrupt descriptor table (IDT) associates each interrupt or exception identifier with a descriptor for the */
/* instructions that service associated event. Like the GDT and LDTs, the IDT is an array of 8-byte descriptors.    */
/* Unlike the GDT and LDTs, the first entry of the IDT may contain a descriptor. To form an index into the IDT, the */
/* processor multiplies the interrupt or exception identifier by eight. Because there are only 256 identifiers, the */
/* IDT need not contain more than 256 descriptors. It can contain fewer than 256 entries; entries are required only */
/* for interrupt identifiers that are actually used.                                                                */
/*                                                                                                                  */
/*The IDT may reside anywhere in physical memory. As Figure 9-1 shows, the processor locates the IDT by means of the*/
/* IDT register (IDTR). The instructions LIDT and SIDT operate on the IDTR. Both instructions have one explicit     */
/* operand: the address in memory of a 6-byte area. Figure 9-2 shows the format of this area.                       */
/*                                                                                                                  */
/* LIDT (Load IDT register) loads the IDT register with the linear base address and limit values contained in the   */
/* memory operand. This instruction can be executed only when the CPL is zero. It is normally used by the           */
/* initialization logic of an operating system when creating an IDT. An operating system may also use it to change  */
/* from one IDT to another.                                                                                         */
/*                                                                                                                  */
/* SIDT (Store IDT register) copies the base and limit value stored in IDTR to a memory location. This instruction  */
/* can be executed at any privilege level.                                                                          */
/*                                                                                                                  */
/* Table 9-2. Priority Among Simultaneous Interrupts and Exceptions:                                                */
/* Priority:  Class of Interrupt or Exception:                                                                      */
/*  - HIGHEST: - Faults except debug faults                                                                         */
/*             - Trap instructions INTO, INT n, INT 3                                                               */
/*             - Debug traps for this instruction                                                                   */
/*             - NMI  interrupt                                                                                     */
/*  - LOWEST : - INTR interrupt                                                                                     */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual; 9.5 IDT Descriptors:                                       */
/* ---------------------------------------------------------------------------------------------------------------- */
/* The IDT may contain any of three kinds of descriptor:                                                            */
/*  - Task gates                                                                                                    */
/*  - Interrupt gates                                                                                               */
/*  - Trap gates                                                                                                    */
/*Figure 9-3 illustrates the format of task gates and 80386 interrupt gates and trap gates. (The task gate in an IDT*/
/* is the same as the task gate already discussed in Chapter 7.)                                                    */
/* ---------------------------------------------------------------------------------------------------------------- */
typedef struct idt_interrupt_descriptor_t {
	unsigned long  idt_gate_for_interrupt_selector;            /* Interrupt descriptor base:  16-bit (80286)/32-bit (80386 and higher) */
	unsigned long  idt_gate_for_interrupt_offset;           /* Interrupt descriptor limit: 16-bit (80286)/32-bit (80386 and higher) */
	unsigned long  idt_gate_for_interrupt_limit;
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
	unsigned       idt_gate_for_interrupt_present;          /* Present: */
	unsigned       idt_gate_for_interrupt_descriptor_privilege_level;          /* Descriptor Privilege Level: */
	unsigned       idt_gate_for_interrupt_selector_table_indicator;          /* Present: */
	unsigned       idt_gate_for_interrupt_selector_requester_privilege_level;          /* Descriptor Privilege Level: */
	unsigned       idt_gate_for_interrupt_granularity_bit;
	unsigned       idt_gate_for_interrupt_available;          /* Present: */
	unsigned       idt_gate_for_interrupt_accessed;          /* Present: */
	unsigned       idt_gate_for_interrupt_number_n_;          /* Gate for Interrupt #N: */
	unsigned       idt_gate_for_interrupt_number_2_;          /* Gate for Interrupt #2: */
	unsigned       idt_gate_for_interrupt_number_1_;          /* Gate for Interrupt #1: */
	unsigned       idt_gate_for_interrupt_number_0_;          /* Gate for Interrupt #0: */
} idt_interrupt_descriptor_t;

typedef struct {
	idt_interrupt_descriptor_t      **idt_interrupt;
} idt_interrupt_t;

typedef struct idt_trap_descriptor_t {
	unsigned long  idt_gate_for_trap_selector;            /* Interrupt descriptor base:  16-bit (80286)/32-bit (80386 and higher) */
	unsigned long  idt_gate_for_trap_offset;           /* Interrupt descriptor limit: 16-bit (80286)/32-bit (80386 and higher) */
	unsigned long  idt_gate_for_trap_limit;
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
	unsigned       idt_gate_for_trap_present;          /* Present: */
	unsigned       idt_gate_for_trap_descriptor_privilege_level;          /* Descriptor Privilege Level: */
	unsigned       idt_gate_for_trap_selector_table_indicator;          /* Present: */
	unsigned       idt_gate_for_trap_selector_requester_privilege_level;          /* Descriptor Privilege Level: */
	unsigned       idt_gate_for_trap_granularity_bit;
	unsigned       idt_gate_for_trap_available;          /* Present: */
	unsigned       idt_gate_for_trap_accessed;          /* Present: */
	unsigned       idt_gate_for_trap_number_n_;          /* Gate for Interrupt #N: */
	unsigned       idt_gate_for_trap_number_2_;          /* Gate for Interrupt #2: */
	unsigned       idt_gate_for_trap_number_1_;          /* Gate for Interrupt #1: */
	unsigned       idt_gate_for_trap_number_0_;          /* Gate for Interrupt #0: */
} idt_trap_descriptor_t;

typedef struct {
	idt_trap_descriptor_t      **idt_trap;
} idt_trap_t;

typedef struct idt_task_descriptor_t {
	unsigned long  idt_gate_for_task_selector;            /* Interrupt descriptor base:  16-bit (80286)/32-bit (80386 and higher) */
	unsigned long  idt_gate_for_task_offset;           /* Interrupt descriptor limit: 16-bit (80286)/32-bit (80386 and higher) */
	unsigned long  idt_gate_for_task_limit;
	/* NOTE: Change "idt_limit" back to "size" if necessary and required by source code */
	/* unsigned char  alloc; */
	unsigned       idt_gate_for_task_present;          /* Present: */
	unsigned       idt_gate_for_task_descriptor_privilege_level;          /* Descriptor Privilege Level: */
	unsigned       idt_gate_for_task_number_n_;          /* Gate for Interrupt #N: */
	unsigned       idt_gate_for_task_number_2_;          /* Gate for Interrupt #2: */
	unsigned       idt_gate_for_task_number_1_;          /* Gate for Interrupt #1: */
	unsigned       idt_gate_for_task_number_0_;          /* Gate for Interrupt #0: */
} idt_task_descriptor_t;

typedef struct {
	idt_task_descriptor_t      **idt_task;
} idt_task_t;
/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual; 9.6 Interrupt Tasks and Interrupt Procedures:              */
/* ---------------------------------------------------------------------------------------------------------------- */
/*Just as a CALL instruction can call either a procedure or a task, so an interrupt or exception can "call" an      */
/* interrupt handler that is either a procedure or a task. When responding to an interrupt or exception, the        */
/*processor uses the interrupt or exception identifier to index a descriptor in the IDT. If the processor indexes to*/
/* an interrupt gate or trap gate, it invokes the handler in a manner similar to a CALL to a call gate. If the      */
/* processor finds a task gate, it causes a task switch in a manner similar to a CALL to a task gate.               */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual; 9.6.1 Interrupt Procedures:                                */
/* ---------------------------------------------------------------------------------------------------------------- */
/*An interrupt gate or trap gate points indirectly to a procedure which will execute in the context of the currently*/
/* executing task as illustrated by Figure 9-4. The selector of the gate points to an executable-segment descriptor */
/* in either the GDT or the current LDT. The offset field of the gate points to the beginning of the interrupt or   */
/* exception handling procedure.                                                                                    */
/*                                                                                                                  */
/* The 80386 invokes an interrupt or exception handling procedure in much the same manner as it CALLs a procedure;  */
/* the differences are explained in the following sections.                                                         */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual; 9.6.1.1 Stack of Interrupt Procedure:                      */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Just as with a control transfer due to a CALL instruction, a control transfer to an interrupt or exception       */
/* handling procedure uses the stack to store the information needed for returning to the original procedure. As    */
/* Figure 9-5 shows, an interrupt pushes the EFLAGS register onto the stack before the pointer to the interrupted   */
/* instruction.                                                                                                     */
/*                                                                                                                  */
/* Certain types of exceptions also cause an error code to be pushed on the stack. An exception handler can use the */
/* error code to help diagnose the exception.                                                                       */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual; 9.6.1.2 Returning from an Interrupt Procedure:             */
/* ---------------------------------------------------------------------------------------------------------------- */
/*An interrupt procedure also differs from a normal procedure in the method of leaving the procedure. The IRET      */
/*instruction is used to exit from an interrupt procedure. IRET is similar to RET except that IRET increments ESP by*/
/* an extra four bytes (because of the flags on the stack) and moves the saved flags into the EFLAGS register. The  */
/* IOPL field of EFLAGS is changed only if the CPL is zero. The IF flag is changed only if CPL <= IOPL.             */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual; 9.6.1.3 Flags Usage by Interrupt Procedure:                */
/* ---------------------------------------------------------------------------------------------------------------- */
/*Interrupts that vector through either interrupt gates or trap gates cause TF (the trap flag) to be reset after the*/
/* current value of TF is saved on the stack as part of EFLAGS. By this action the processor prevents debugging     */
/*activity that uses single-stepping from affecting interrupt response. A subsequent IRET instruction restores TF to*/
/* the value in the EFLAGS image on the stack.                                                                      */
/*                                                                                                                  */
/* The difference between an interrupt gate and a trap gate is in the effect on IF (the interrupt-enable flag). An  */
/* interrupt that vectors through an interrupt gate resets IF, thereby preventing other interrupts from interfering */
/* with the current interrupt handler. A subsequent IRET instruction restores IF to the value in the EFLAGS image on*/
/* the stack. AN interrupt through a trap gate does not change IF.                                                  */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual; 9.6.1.4 Protection in Interrupt Procedures:                */
/* ---------------------------------------------------------------------------------------------------------------- */
/* The privilege rule that governs interrupt procedures is similar to that for procedure calls: the CPU does not    */
/* permit an interrupt to transfer control to a procedure in a segment of lesser privilege (numerically greater     */
/*privilege level) than the current privilege level. An attempt to violate this rule results in a general protection*/
/* exception.                                                                                                       */
/*                                                                                                                  */
/* Because occurrence of interrupts is not generally predictable, this privilege rule effectively imposes           */
/* restrictions on the privilege levels at which interrupt and exception handling procedures can execute. Either of */
/* the following strategies can be employed to ensure that the privilege rule is never violated.                    */
/*-Place the handler in a conforming segment. This strategy suits the handlers for certain exceptions (divide error,*/
/* for example). Such a handler must use only the data available to it from the stack. If it needed data from a data*/
/*  segment, the data segment would have to have privilege level three, thereby making it unprotected.              */
/*  - Place the handler procedure in a privilege level zero segment.                                                */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual; 9.6.2 Interrupt Tasks:                                     */
/* ---------------------------------------------------------------------------------------------------------------- */
/* A task gate in the IDT points indirectly to a task, as Figure 9-6 illustrates. The selector of the gate points to*/
/* a TSS descriptor in the GDT.                                                                                     */
/*                                                                                                                  */
/* When an interrupt or exception vectors to a task gate in the IDT, a task switch results. Handling an interrupt   */
/* with a separate task offers two advantages:                                                                      */
/*  - The entire context is saved automatically.                                                                    */
/* -The interrupt handler can be isolated from other tasks by giving it a separate address space, either via its LDT*/
/* or via its page directory. The actions that the processor takes to perform a task switch are discussed in Chapter*/
/* 7. THe interrupt task returns to the interrupted task by executing an IRET instruction.                          */
/*                                                                                                                  */
/* If the task switch is caused by an exception that has an error code, the processor automatically pushes the error*/
/* code onto the stack that corresponds to the privilege level of the first instruction to be executed in the       */
/* interrupt task.                                                                                                  */
/*                                                                                                                  */
/* When interrupt tasks are used in an operating system for the 80386, there are actually two schedulers: the       */
/* software scheduler (part of the operating system) and the hardware scheduler (part of the processor's interrupt  */
/* mechanism). The design of the software scheduler should account for the fact that the hardware scheduler may     */
/* dispatch an interrupt task whenever task whenever interrupts are enabled.                                        */
/* ---------------------------------------------------------------------------------------------------------------- */
/* typedef struct idtr_interrupt_register_t {
	unsigned long  idtr_gate_for_interrupt_register_selector;            /* Interrupt descriptor base:  16-bit (80286)/32-bit (80386 and higher) */
/* 	unsigned long  idtr_gate_for_interrupt_register_offset;           /* Interrupt descriptor limit: 16-bit (80286)/32-bit (80386 and higher) */
/* 	unsigned long  idtr_gate_for_interrupt_register_limit;
	unsigned       idtr_gate_for_interrupt_register_x_;            /* Segment type 1: "0" or "1": */
/* 	unsigned       idtr_gate_for_interrupt_register_0_;            /* Segment type 1: "0" or "1": */
/* 	unsigned       idtr_gate_for_interrupt_register_type;            /* Segment type 1: "0" or "1": */
/* 	unsigned       idtr_gate_for_interrupt_register_type_b;            /* Segment type 1: "0" or "1": */
/* 	unsigned       idtr_gate_for_interrupt_register_default;            /* Segment type 1: "0" or "1": */
/* 	unsigned       idtr_gate_for_interrupt_register_type_1;          /* Segment type 1: "0" or "1": */
/* 	unsigned       idtr_gate_for_interrupt_register_type_0;          /* Segment type 0: "0" or "1": */
/* 	unsigned       idtr_gate_for_interrupt_register_type_c;          /* Segment type conforming: "0" for conforming or "1" for non-conforming: */
/* 	unsigned       idtr_gate_for_interrupt_register_type_e;          /* Segment type expand-down: "0" or "1" */
/* 	unsigned       idtr_gate_for_interrupt_register_type_r;          /* Segment type readable: "0" or "1" */
/* 	unsigned       idtr_gate_for_interrupt_register_type_w;          /* Segment type writable: "0" or "1" */
/* 	unsigned       idtr_gate_for_interrupt_register_present;          /* Present: */
/* 	unsigned       idtr_gate_for_interrupt_register_descriptor_privilege_level;          /* Descriptor Privilege Level: */
/* 	unsigned       idtr_gate_for_interrupt_register_selector_table_indicator;          /* Present: */
/* 	unsigned       idtr_gate_for_interrupt_register_selector_requester_privilege_level;          /* Descriptor Privilege Level: */
/* 	unsigned       idtr_gate_for_interrupt_register_granularity_bit;
	unsigned       idtr_gate_for_interrupt_register_available;          /* Present: */
/* 	unsigned       idtr_gate_for_interrupt_register_accessed;          /* Present: */
/* 	unsigned       idtr_gate_for_interrupt_register_interrupt_n_;          /* Gate for Interrupt #N: */
/* 	unsigned       idtr_gate_for_interrupt_register_interrupt_2_;          /* Gate for Interrupt #2: */
/* 	unsigned       idtr_gate_for_interrupt_register_interrupt_1_;          /* Gate for Interrupt #1: */
/* 	unsigned       idtr_gate_for_interrupt_register_interrupt_0_;          /* Gate for Interrupt #0: */
/* } idtr_interrupt_register_t;

typedef struct idtr_trap_register_t {
	unsigned long  idtr_gate_for_trap_register_selector;            /* Interrupt descriptor base:  16-bit (80286)/32-bit (80386 and higher) */
/* 	unsigned long  idtr_gate_for_trap_register_offset;           /* Interrupt descriptor limit: 16-bit (80286)/32-bit (80386 and higher) */
/* 	unsigned long  idtr_gate_for_trap_register_limit;
	unsigned       idtr_gate_for_trap_register_x_;            /* Segment type 1: "0" or "1": */
/* 	unsigned       idtr_gate_for_trap_register_0_;            /* Segment type 1: "0" or "1": */
/* 	unsigned       idtr_gate_for_trap_register_type;            /* Segment type 1: "0" or "1": */
/* 	unsigned       idtr_gate_for_trap_register_type_b;            /* Segment type 1: "0" or "1": */
/* 	unsigned       idtr_gate_for_trap_register_default;            /* Segment type 1: "0" or "1": */
/* 	unsigned       idtr_gate_for_trap_register_type_1;          /* Segment type 1: "0" or "1": */
/* 	unsigned       idtr_gate_for_trap_register_type_0;          /* Segment type 0: "0" or "1": */
/* 	unsigned       idtr_gate_for_trap_register_type_c;          /* Segment type conforming: "0" for conforming or "1" for non-conforming: */
/* 	unsigned       idtr_gate_for_trap_register_type_e;          /* Segment type expand-down: "0" or "1" */
/* 	unsigned       idtr_gate_for_trap_register_type_r;          /* Segment type readable: "0" or "1" */
/* 	unsigned       idtr_gate_for_trap_register_type_w;          /* Segment type writable: "0" or "1" */
/* 	unsigned       idtr_gate_for_trap_register_present;          /* Present: */
/* 	unsigned       idtr_gate_for_trap_register_descriptor_privilege_level;          /* Descriptor Privilege Level: */
/* 	unsigned       idtr_gate_for_trap_register_selector_table_indicator;          /* Present: */
/* 	unsigned       idtr_gate_for_trap_register_selector_requester_privilege_level;          /* Descriptor Privilege Level: */
/* 	unsigned       idtr_gate_for_trap_register_granularity_bit;
	unsigned       idtr_gate_for_trap_register_available;          /* Present: */
/* 	unsigned       idtr_gate_for_trap_register_accessed;          /* Present: */
/* 	unsigned       idtr_gate_for_trap_register_interrupt_n_;          /* Gate for Interrupt #N: */
/* 	unsigned       idtr_gate_for_trap_register_interrupt_2_;          /* Gate for Interrupt #2: */
/* 	unsigned       idtr_gate_for_trap_register_interrupt_1_;          /* Gate for Interrupt #1: */
/* 	unsigned       idtr_gate_for_trap_register_interrupt_0_;          /* Gate for Interrupt #0: */
/* } idtr_trap_register_t;
/* ------------------------------------------------------ --------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual; 9.7 Error Code:                                            */
/* ---------------------------------------------------------------------------------------------------------------- */
/* With exceptions that relate to a specific segment, the processor pushes an error code onto the stack of the      */
/* exception handler (whether procedure or task). The error code has the format shown in Figure 9-7. The format of  */
/*the error code resembles that of a selector; however, instead of an RPL field, the error code contains two one-bit*/
/* items:                                                                                                           */
/*  - 1. The processor sets the EXT bit if an event external to the program caused the exception.                   */
/*  - 2. The processor sets the I-bit (IDT-bit) if the index portion of the error code refers to a gate descriptor  */
/*  in the IDT.                                                                                                     */
/* If the I-bit is not set, the TI bit indicates whether the error code refers to the GDT (value 0) or to the LDT   */
/* (value 1). The remaining 14 bits are the upper 14 bits of the segment selector involved. In some cases the error */
/* code on the stack is null, i.e., all bits in the low-order word are zero.                                        */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual; 9.8 Exception Conditions:                                  */
/* ---------------------------------------------------------------------------------------------------------------- */
/* The following sections describe each of the possible exception conditions in detail. Each description classifies */
/* the exception as a fault, trap, or abort. This classification provides information needed by systems programmers */
/* for restarting the procedure in which the exception occurred:                                                    */
/*  - Faults: The CS and EIP values saved when a fault is reported point to the instruction causing the fault.      */
/*  - Traps : The CS and EIP values stored when the trap is reported point to the instruction dynamically after the */
/*  instruction causing the trap. If a trap is detected during an instruction that alters program flow, the reported*/
/*  values of CS and EIP reflect the alteration of program flow. For example, if a trap is detected in a JMP        */
/*  instruction, the CS and EIP values pushed onto the stack point to the target of the JMP, not to the instruction */
/*  after the JMP.                                                                                                  */
/*  - Aborts:An abort is an exception that permits neither precise location of the instruction causing the exception*/
/*  nor restart of the program that caused the exception. Aborts are used to report severe errors, such as hardware */
/*  errors and inconsistent or illegal values in system tables.                                                     */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual; 9.8.1  Interrupt  0 -- Divide Error:                       */
/* ---------------------------------------------------------------------------------------------------------------- */
/* The divide-error fault occurs during a DIV or an IDIV instruction when the divisor is zero.                      */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual; 9.8.2  Interrupt  1 -- Debug Exceptions:                   */
/* ---------------------------------------------------------------------------------------------------------------- */
/* The processor triggers this interrupt for any of a number of conditions; whether the exception is a fault or a   */
/* trap depends on the condition:                                                                                   */
/*  - Instruction address breakpoint fault.                                                                         */
/*  - Data address breakpoint trap.                                                                                 */
/*  - General detect fault.                                                                                         */
/*  - Single-step trap.                                                                                             */
/*  - Task-switch breakpoint trap.                                                                                  */
/*The processor does not push an error code for this exception. An exception handler can examine the debug registers*/
/* to determine which condition caused the exception. Refer to Chapter 12 for more detailed information about       */
/* debugging and the debug registers.                                                                               */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual; 9.8.3  Interrupt  3 -- Breakpoint:                         */
/* ---------------------------------------------------------------------------------------------------------------- */
/* The INT 3 instruction causes this trap. The INT 3 instruction is one byte long, which makes it easy to replace an*/
/* opcode in an executable segment with the breakpoint opcode. The operating system or a debugging subsystem can use*/
/* a data-segment alias for an executable segment to place an INT 3 anywhere it is convenient to arrest normal      */
/* execution so that some sort of special processing can be performed. Debuggers typically use breakpoints as a way */
/* of displaying registers, variables, etc., at crucial points in a task.                                           */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual; 9.8.4  Interrupt  4 -- Overflow:                           */
/* ---------------------------------------------------------------------------------------------------------------- */
/*This trap occurs when the processor encounters an INTO instruction (overflow) flag is set. Since signed arithmetic*/
/* and unsigned arithmetic both use the same arithmetic instructions, the processor cannot determine which is       */
/* intended and therefore does not overflow exceptions automatically. Instead it merely sets OF when the results, if*/
/* interpreted as signed numbers, would be out of range. When doing arithmetic on signed operands, careful          */
/* programmers and compilers either test OF directly or use the INTO instruction.                                   */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual; 9.8.6  Interrupt  6 -- Invalid Opcode:                     */
/* ---------------------------------------------------------------------------------------------------------------- */
/* This fault occurs when an invalid opcode is detected by the execution unit. (The exception is not detected until */
/* an attempt is made to execute the invalid opcode; i.e., prefetching an invalid opcode does not cause this        */
/* exception.) No error code is pushed on the stack. The exception can be handled within the same task.             */
/*                                                                                                                  */
/* This exception also occurs when the type of operand is invalid for the given opcode. Examples include an         */
/* intersegment JMP referencing a register operand, or an LES instruction with a register source operand.           */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual; 9.8.7  Interrupt  7 -- Coprocessor Not Available:          */
/* ---------------------------------------------------------------------------------------------------------------- */
/* This exception occurs in either of two conditions:                                                               */
/* -The processor encounters an ESC (escape) instruction, and the EM (emulate) bit of CR0 (control register zero) is*/
/*  set.                                                                                                            */
/* -The processor encounters either the WAIT instruction or an ESC instruction, and both the MP(monitor coprocessor)*/
/*  and TS (task switched) bits of CR0 are set.                                                                     */
/* Refer to Chapter 11 for information about the coprocessor interface.                                             */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual; 9.8.8  Interrupt  8 -- Double Fault:                       */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Normally, when the processor detects an exception while trying to invoke the handler for a prior exception, the  */
/* two exceptions can be handled serially. If, however, the processor cannot handle them serially, it signals the   */
/* double-fault exception instead. To determine when two faults are to be signaled as a double fault, the 80386     */
/* divides the exceptions into three classes: benign exceptions, contributory exceptions, and page faults. Table 9-3*/
/* shows this classification.                                                                                       */
/*                                                                                                                  */
/* Table 9-4 shows which combinations of exceptions cause a double fault and which do not.                          */
/*                                                                                                                  */
/* The processor always pushes an error code onto the stack of the double-fault handler; however, the error code is */
/* always zero. The faulting instruction may not be restarted. If any other exception occurs while attempting to    */
/* invoke the double-fault handler, the processor shuts down.                                                       */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Table 9-3. Double-Fault Detection Classes:                                                                       */
/* Class:                     ID:   Description:                                                                    */
/*  - Benign       Exceptions: -  1: - Debug exceptions                                                             */
/*  - Benign       Exceptions: -  2: - NMI                                                                          */
/*  - Benign       Exceptions: -  3: - Breakpoint                                                                   */
/*  - Benign       Exceptions: -  4: - Overflow                                                                     */
/*  - Benign       Exceptions: -  5: - Bounds check                                                                 */
/*  - Benign       Exceptions: -  6: - Invalid opcode                                                               */
/*  - Benign       Exceptions: -  7: - Coprocessor not available                                                    */
/*  - Benign       Exceptions: - 16: - Coprocessor error                                                            */
/*  - Contributory Exceptions: -  0: - Divide error                                                                 */
/*  - Contributory Exceptions: -  9: - Coprocessor Segment Overrun                                                  */
/*  - Contributory Exceptions: - 10: - Invalid TSS                                                                  */
/*  - Contributory Exceptions: - 11: - Segment not present                                                          */
/*  - Contributory Exceptions: - 12: - Stack exception                                                              */
/*  - Contributory Exceptions: - 13: - General protection                                                           */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Table 9-4. Double-Fault Definition:                                                                              */
/* FIRST EXCEPTION:                SECOND EXCEPTION:                                                                */
/*  - Benign       Exception: - OK  - Benign       Exception: OK                                                    */
/*  - Benign       Exception: - OK  - Contributory Exception: OK                                                    */
/*  - Benign       Exception: - OK  - Page Fault            : OK                                                    */
/*  - Contributory Exception: - OK  - Benign       Exception: OK                                                    */
/*  - Contributory Exception: - OK  - Contributory Exception: DOUBLE                                                */
/*  - Contributory Exception: - OK  - Page Fault            : OK                                                    */
/*  - Page Fault            : - OK  - Benign       Exception: OK                                                    */
/*  - Page Fault            : - OK  - Contributory Exception: DOUBLE                                                */
/*  - Page Fault            : - OK  - Page Fault            : DOUBLE                                                */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual; 9.8.9  Interrupt  9 -- Coprocessor Segment Overrun:        */
/* ---------------------------------------------------------------------------------------------------------------- */
/*This exception is raised in protected mode if the 80386 detects a page or segment violation while transferring the*/
/* middle portion of a coprocessor operand to the NPX. This exception is avoidable. Refer to Chapter 11 for more    */
/* information about the coprocessor interface.                                                                     */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual; 9.8.10 Interrupt 10 -- Invalid TSS:                        */
/* ---------------------------------------------------------------------------------------------------------------- */
/*Interrupt 10 occurs if during a task switch the new TSS is invalid. A TSS is considered invalid in the cases shown*/
/* in Table 9-5. An error code is pushed onto the stack to help identify the cause of the fault. The EXT bit        */
/* indicates whether the exception was caused by a condition outside the control of the program; e.g., an external  */
/* interrupt via a task gate triggered a switch to an invalid TSS.                                                  */
/*                                                                                                                  */
/* This fault can occur either in the context of the original task or in the context of the new task. Until the     */
/*processor has completely verified the presence of the new TSS, the exception occurs in the context of the original*/
/* task. Once the existence of the new TSS is verified, the task switch is considered complete; i.e., TR is updated */
/* and, if the switch is due to a CALL or interrupt, the backlink of the new TSS is set to the old TSS. Any errors  */
/* discovered by the processor after this point are handled in the context of the new task.                         */
/*                                                                                                                  */
/* To insure a proper TSS to process it, the handler for exception 10 must be a task invoked via a task gate.       */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Table 9-5. Conditions That Invalidate the TSS:                                                                   */
/* Error Code:             Condition:                                                                               */
/*  - TSS         id + EXT: - The limit in the TSS descriptor is less than 103                                      */
/*  - LDT         id + EXT: - Invalid LDT selector or LDT not present                                               */
/*  - SS          id + EXT: - Stack segment selector is outside table limit                                         */
/*  - SS          id + EXT: - Stack segment is not a writable segment                                               */
/*  - SS          id + EXT: - Stack segment DPL does not match new CPL                                              */
/*  - SS          id + EXT: - Stack segment selector RPL < > CPL                                                    */
/*  - CS          id + EXT: - Code segment selector is outside table limit                                          */
/*  - CS          id + EXT: - Code segment selector does not refer to code segment                                  */
/*  - CS          id + EXT: - DPL of non-conforming code segment < > new CPL                                        */
/*  - CS          id + EXT: - DPL of conforming code segment > new CPL                                              */
/*  - DS/ES/FS/GS id + EXT: - DS, ES, FS, or GS segment selector is outside table limits                            */
/*  - DS/ES/FS/GS id + EXT: - DS, ES, FS, or GS is not readable segment                                             */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual; 9.8.11 Interrupt 11 -- Segment Not Present:                */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Exception 11 occurs when the processor detects that the present bit of a descriptor is zero. The processor can   */
/* trigger this fault in any of these cases:                                                                        */
/*  - While attempting to load the CS, DS, ES, FS, or GS registers; loading the SS register, however, causes a stack*/
/*  fault.                                                                                                          */
/*  - While attempting loading the LDT register with an LLDT instruction; loading the LDT register during a task    */
/*  switch operation, however, causes the "invalid TSS" exception.                                                  */
/*This fault is restartable. If the exception handler makes the segment present and returns, the interrupted program*/
/* will resume execution.                                                                                           */
/*                                                                                                                  */
/* If a not-present exception occurs during a task switch, not all the steps of the task switch are complete. During*/
/* a task switch, the processor first loads all segment registers, then checks their contents for validity. If a not*/
/* -present exception is discovered, the remaining segment registers have not been checked and therefore may not be */
/* usable for referencing memory. The not-present handler should not rely on being able to use the values found in  */
/* CS, SS, DS, ES, FS, and GS without causing another exception. The exception handler should check all segment     */
/* registers before trying to resume the new task; otherwise, general protection faults may result later under      */
/* conditions that make diagnosis more difficult. There are three ways to handle this case:                         */
/*  - 1. Handle the not-present fault with a task. The task switch back to the interrupted task will cause the      */
/*  processor to check the registers as it loads them from the TSS.                                                 */
/*  - 2. PUSH and POP all segment registers. Each POP causes the processor to check the new contents of the segment */
/*  register.                                                                                                       */
/*  - 3. Scrutinize the contents of each segment register image in the TSS, simulating the test that the processor  */
/*  makes when it loads a segment register.                                                                         */
/* This exception pushes an error code onto the stack. The EXT bit of the error code is set if an event external to */
/* the program caused an interrupt that subsequently referenced a not-present segment. The I-bit is set if the error*/
/* code refers to an IDT entry, e.g., an INT instruction referencing a not-present gate.                            */
/*                                                                                                                  */
/* An operating system typically uses the "segment not present" exception to implement virtual memory at the segment*/
/* level. A not-present indication in a gate descriptor, however, usually does not indicate that a segment is not   */
/* present (because gates do not necessarily correspond to segments). Not-present gates may be used by an operating */
/* system to trigger exceptions of special significance to the operating system.                                    */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual; 9.8.12 Interrupt 12 -- Stack Exception:                    */
/* ---------------------------------------------------------------------------------------------------------------- */
/* A stack fault occurs in either of two general conditions:                                                        */
/*  - As a result of a limit violation in any operation that refers to the SS register. This includes stack-oriented*/
/*  instructions such as POP, PUSH, ENTER, and LEAVE, as well as other memory references that implicitly use SS (for*/
/*  example, MOV AX, [BP+61]). ENTER causes this exception when the stack is too small for the indicated local-     */
/*  variable space.                                                                                                 */
/*  - When attempting to load the SS register with a descriptor that is marked not-present but is otherwise valid.  */
/*  This can occur in a task switch, an interlevel CALL, an interlevel return, an LSS instruction, or a MOV or POP  */
/*  instruction to SS.                                                                                              */
/* When the processor detects a stack exception, it pushes an error code onto the stack of the exception handler. If*/
/*the exception is due to a not-present stack segment or to overflow of the new stack during an interlevel CALL, the*/
/* error code contains a selector to the segment in question (the exception handler can test the present bit in the */
/* descriptor to determine which exception occurred); otherwise the error code is zero.                             */
/*                                                                                                                  */
/* An instruction that causes this fault is restartable in all cases. The return pointer pushed onto the exception  */
/* handler's stack points to the instruction that needs to be restarted. This instruction is usually the one that   */
/* caused the exception; however, in the case of a stack exception due to loading of a not-present stack-segment    */
/* descriptor during a task switch, the indicated instruction is the first instruction of the new task.             */
/*                                                                                                                  */
/* When a stack fault occurs during a task switch, the segment registers may not be usable for referencing memory.  */
/* During a task switch, the selector values are loaded before the descriptors are checked. If a stack fault is     */
/* discovered, the remaining segment registers have not been checked and therefore may not be usable for referencing*/
/* memory. The stack fault handler should not rely on being able to use the values found in CS, SS, DS, ES, FS, and */
/* GS without causing another exception. The exception handler should check all segments registers before trying to */
/* resume the new task; otherwise, general protection faults may result later under conditions that make diagnosis  */
/* more difficult.                                                                                                  */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual; 9.8.13 Interrupt 13 -- General Protection Exception:       */
/* ---------------------------------------------------------------------------------------------------------------- */
/* All protection violations that do not cause another exception cause a general protection exception. This includes*/
/* (but is not limited to):                                                                                         */
/*  -  1. Exceeding segment limit when using CS, DS, ES, FS, or GS                                                  */
/*  -  2. Exceeding segment limit when referencing a descriptor table                                               */
/*  -  3. Transferring control to a segment that is not executable                                                  */
/*  -  4. Writing into a read-only data segment or into a code segment                                              */
/*  -  5. Reading from an execute-only segment                                                                      */
/*  -  6. Loading the SS register with a read-only descriptor (unless the selector comes from the TSS during a task */
/*  switch, in which case a TSS exception occurs)                                                                   */
/*  -  7. Loading SS, DS, ES, FS, or GS with the descriptor of a system segment                                     */
/*  -  8. Loading DS, ES, FS, or GS with the descriptor of an executable segment that is not also readable          */
/*  -  9. Loading SS with the descriptor of an executable segment                                                   */
/*  - 10. Accessing memory via DS, ES, FS, or GS when the segment register contains a null selector                 */
/*  - 11. Switching to a busy task                                                                                  */
/*  - 12. Violating privilege rules                                                                                 */
/*  - 13. Loading CR0 with PG=1 and PE=0.                                                                           */
/*  - 14. Interrupt or exception via trap or interrupt gate from V86 mode to privilege level other than zero.       */
/*  - 15. Exceeding the instruction length limit of 15 bytes (this can occur only if redundant prefixes are placed  */
/*  before an instruction)                                                                                          */
/* The general protection exception is a fault. In response to a general protection exception, the processor pushes */
/* an error code onto the exception handler's stack. If loading a descriptor causes the exception, the error code   */
/* contains a selector to the descriptor; otherwise, the error code is null. The source of the selector in an error */
/* code may be any of the following:                                                                                */
/*  - 1. An operand of the instruction.                                                                             */
/*  - 2. A selector from a gate that is the operand of the instruction.                                             */
/*  - 3. A selector from a TSS involved in a task switch.                                                           */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual; 9.8.14 Interrupt 14 -- Page Fault:                         */
/* ---------------------------------------------------------------------------------------------------------------- */
/* This exception occurs when paging is enabled (PG=1) and the processor detects one of the following conditions    */
/* while translating a linear address to a physical address:                                                        */
/*  - The page-directory or page-table entry needed for the address translation has zero in its present bit.        */
/*  - The current procedure does not have sufficient privilege to access the indicated page.                        */
/* The processor makes available to the page fault handler two items of information that aid in diagnosing the      */
/* exception and recovering from it:                                                                                */
/*-An error code on the stack. The error code for a page fault has a format different from that for other exceptions*/
/*  (see Figure 9-8). The error code tells the exception handler three things:                                      */
/*  - - 1. Whether the exception was due to a not present page or to an access rights violation.                    */
/*  - - 2. Whether the processor was executing at user or supervisor level at the time of the exception.            */
/*  - - 3. Whether the memory access that caused the exception was a read or write.                                 */
/*  - CR2 (control register two). The processor stores in CR2 the linear address used in the access that caused the */
/* exception (see Figure 9-9). The exception handler can use this address to locate the corresponding page directory*/
/*  and page table entries. If another page fault can occur during execution of the page fault handler, the handler */
/*  should push CR2 onto the stack.                                                                                 */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Figure 9-8.  Page-Fault Error Code Format:                                                                       */
/* Field: Value: Description:                                                                                       */
/*  - U/S: - 0:   - The access causing the fault originated when the processor was executing in supervisor mode.    */
/*  - U/S: - 1:   - The access causing the fault originated when the processor was executing in user mode.          */
/*  - W/R: - 0:   - The access causing the fault was a read.                                                        */
/*  - W/R: - 1:   - The access causing the fault was a write.                                                       */
/*  - P  : - 0:   - The fault was caused by a not-present page.                                                     */
/*  - P  : - 1:   - The fault was caused by a page-level protection violation.                                      */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual; 9.8.14.1 Page Fault During Task Switch:                    */
/* ---------------------------------------------------------------------------------------------------------------- */
/* The processor may access any of four segments during a task switch:                                              */
/*  - 1. Writes the state of the original task in the TSS of that task.                                             */
/*  - 2. Reads the GDT to locate the TSS descriptor of the new task.                                                */
/*  - 3. Reads the TSS of the new task to check the types of segment descriptors from the TSS.                      */
/*  - 4. May read the LDT of the new task in order to verify the segment registers stored in the new TSS.           */
/* A page fault can result from accessing any of these segments. In the latter two cases the exception occurs in the*/
/* context of the new task. The instruction pointer refers to the next instruction of the new task, not to the      */
/*instruction that caused the task switch. If the design of the operating system permits page faults to occur during*/
/* task-switches, the page-fault handler should be invoked via a task gate.                                         */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Figure 9-9.  CR2 Format:                                                                                         */
/* 31     23     15     7     0                                                                                     */
/*  - PAGE FAULT LINEAR ADDRESS                                                                                     */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual; 9.8.14.2 Page Fault with Inconsistent Stack Pointer:       */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Special care should be taken to ensure that a page fault does not cause the processor to use an invalid stack    */
/* pointer (SS:ESP). Software written for earlier processors in the 8086 family often uses a pair of instructions to*/
/* change to a new stack; for example:                                                                              */
/*  - MOV SS, AX                                                                                                    */
/*  - MOV SP, StackTop                                                                                              */
/* With the 80386, because the second instruction accesses memory, it is possible to get a page fault after SS has  */
/* been changed but before SP has received the corresponding change. At this point, the two parts of the stack      */
/* pointer SS:SP (or, for 32-bit programs, SS:ESP) are inconsistent.                                                */
/*                                                                                                                  */
/* The processor does not use the inconsistent stack pointer if the handling of the page fault causes a stack switch*/
/* to a well defined stack (i.e., the handler is a task or a more privileged procedure). However, if the page fault */
/* handler is invoked by a trap or interrupt gate and the page fault occurs at the same privilege level as the page */
/* fault handler, the processor will attempt to use the stack indicated by the current (invalid) stack pointer.     */
/*                                                                                                                  */
/* In systems that implement paging and that handle page faults within the faulting task (with trap or interrupt    */
/*gates), software that executes at the same privilege level as the page fault handler should initialize a new stack*/
/*by using the new LSS instruction rather than an instruction pair shown above. When the page fault handler executes*/
/* at privilege level zero (the normal case), the scope of the problem is limited to privilege-level zero code,     */
/* typically the kernel of the operating system.                                                                    */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual; 9.8.15 Interrupt 16 -- Coprocessor Error:                  */
/* ---------------------------------------------------------------------------------------------------------------- */
/*The 80386 reports this exception when it detects a signal from the 80287 or 80387 on the 80386's ERROR# input pin.*/
/* The 80386 tests this pin only at the beginning of certain ESC instructions and when it encounters a WAIT         */
/* instruction while the EM bit of the MSW is zero (no emulation). Refer to Chapter 11 for more information on the  */
/* coprocessor interface.                                                                                           */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual; 9.9  Exception  Summary:                                   */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Table 9-6 summarizes the exceptions recognized by the 386.                                                       */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual; 9.10 Error Code Summary:                                   */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Table 9-7 summarizes the error information that is available with each exception.                                */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Table 9-6. Exception Summary:                    Return Address Points                                           */
/* Description:                   Interrupt Number: to Faulting Instruction: Exception Type: Function That Can      */
/*                                                                                           Generate the Exception:*/
/*  - Divide error:                -  0:             - YES:                   - FAULT:        - DIV, IDIV           */
/*  - Debug exceptions:            -  1:             - *1:                    - *1:           - Any instruction     */
/*  - Breakpoint:                  -  3:             - NO:                    - TRAP:         - One-byte INT 3      */
/*  - Overflow:                    -  4:             - NO:                    - TRAP:         - INTO                */
/*  - Bounds check:                -  5:             - YES:                   - FAULT:        - BOUND               */
/*  - Invalid opcode:              -  6:             - YES:                   - FAULT:        - Any illegal         */
/*                                                                                            instruction           */
/*  - Coprocessor not available:   -  7:             - YES:                   - FAULT:        - ESC, WAIT           */
/*  - Double fault:                -  8:             - YES:                   - ABORT:        - Any instruction that*/
/*                                                                                        can generate an exception */
/*  - Coprocessor Segment Overrun: -  9:             - NO:                    - ABORT:        - Any operand of an   */
/*                                                           ESC instruction that wraps around the end of a segment.*/
/*  - Invalid TSS:                 - 10:             - YES:                   - FAULT2:       - JMP, CALL, IRET, any*/
/*                                                                                            interrupt             */
/*  - Segment not present:         - 11:             - YES:                   - FAULT:        - Any segment-register*/
/*                                                                                            modifier              */
/*  - Stack exception:             - 12:             - YES:                   - FAULT:        - Any memory reference*/
/*                                                                                            thru SS               */
/*  - General Protection:          - 13:             - YES:                   - FAULT/ABORT3: - Any memory reference*/
/*                                                                                            or code fetch         */
/*  - Page fault:                  - 14:             - YES:                   - FAULT:        - Any memory reference*/
/*                                                                                            or code fetch         */
/*  - Coprocessor error:           - 16:             - YES:                   - FAULT4:       - ESC, WAIT           */
/*  - Two-byte SW Interrupt        -  0-255:         - NO:                    - TRAP:         - INT n               */
/* 1. Some debug exceptions are traps and some are faults. The exception handler can determine which has occurred by*/
/* examining DR6. (Refer to Chapter 12.)                                                                            */
/* 2. An invalid-TSS fault is not restartable if it occurs during the processing of an external interrupt.          */
/* 3. All GP faults are restartable. If the fault occurs while attempting to vector to the handler for an external  */
/* interrupt, the interrupted program is restartable, but the interrupt may be lost.                                */
/* 4. Coprocessor errors are reported as a fault on the first ESC or WAIT instruction executed after the ESC        */
/* instruction that caused the error.                                                                               */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Table 9-7. Error-Code Summary:                                                                                   */
/* Description:                   Interrupt Number: Error Code:                                                     */
/*  - Debug error:                 -  0:             - No                                                           */
/*  - Debug exceptions:            -  1:             - No                                                           */
/*  - Breakpoint:                  -  3:             - No                                                           */
/*  - Overflow:                    -  4:             - No                                                           */
/*  - Bounds check:                -  5:             - No                                                           */
/*  - Invalid opcode:              -  6:             - No                                                           */
/*  - Coprocessor not available:   -  7:             - No                                                           */
/*  - System error:                -  8:             - Yes (always 0)                                               */
/*  - Coprocessor segment overrun: -  9:             - No                                                           */
/*  - Invalid TSS:                 - 10:             - Yes                                                          */
/*  - Segment not present:         - 11:             - Yes                                                          */
/*  - Stack exception:             - 12:             - Yes                                                          */
/*  - General protection fault:    - 13:             - Yes                                                          */
/*  - Page fault:                  - 14:             - Yes                                                          */
/*  - Coprocessor error:           - 16:             - No                                                           */
/*  - Two-byte SW interrupt:       - 0-255:          - No                                                           */
/* ---------------------------------------------------------------------------------------------------------------- */
/* typedef struct exception_handler_t {
	unsigned long  exception_handler_selector_index;            /* Selector index for faulting instruction: */
	/* NOTE: Change "idt_limit" back to "size" if necessary and required by source code */
	/* unsigned char  alloc; */
/* 	unsigned       exception_handler_table_indicator;           /* Table indicator for faulting instruction: */
/* 	unsigned       exception_handler_interrupt_descriptor_table;/* Set if index portion of error code refers to a trap gate in the IDT: */
/* 	unsigned       exception_handler_external;                  /* Set if events external to the running program caused the faulting instruction */
/* 	unsigned       exception_handler_page_fault_user_supervisor;/* 0: Page fault in supervisor mode - 1: Page fault in user mode */
/* 	unsigned       exception_handler_page_fault_write_read;     /* 0: Page fault during read - 1: Page fault during write */
/* 	unsigned       exception_handler_page_fault_present;        /* 0: Page fault caused by non-present page - 1: Page fault caused by page-level
	protection violation */
/* 	unsigned long  exception_handler_cr2_page_fault_linear_address; /* Value of control register 2 during page switch that occurred during task switch */
/* } exception_handler_t;

typedef struct idt_count_t {
	unsigned short idt_number_of_interrupt_segment_descriptors_in_use; /* Number of segment descriptors in use for Interrupt Descriptor Table */
/* } idt_count_t;

/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual: 10.6.1 Structure of the TLB:                               */
/* ---------------------------------------------------------------------------------------------------------------- */
/* The TLB is a four-way set-associative memory. Figure 10-3 illustrates the structure of the TLB. There are four   */
/* sets of eight entries each. Each entry consists of a tag and data. Tags are 24-bits wide. They contain the       */
/* high-order 20 bits of the linear address, the valid bit, and three attribute bits. The data portion of each entry*/
/* contains the high-order 20 bits of the physical address.                                                         */
/* ---------------------------------------------------------------------------------------------------------------- */
/* NOTE: Use "& 0xfffff" to simulate 20-bit integers */
/* typedef struct translation_lookaside_buffer_t {
	/* Set 11: */
/* 	unsigned long  tlb_set_11_entry_8_tag_7;
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
	/* Set 10: */
/* 	unsigned long  tlb_set_10_entry_8_tag_7;
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
	/* Set 01: */
/* 	unsigned long  tlb_set_01_entry_8_tag_7;
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
	/* Set 00: */
/* 	unsigned long  tlb_set_00_entry_8_tag_7;
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
} translation_lookaside_buffer_t;

/* ------------------------------------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual: 10.6.2 Test Registers:                                                                */
/* ------------------------------------------------------------------------------------------------------------------------------------------- */
/* Two test registers, shown in Figure 10-4, are provided for the purpose of testing. TR6 is the test command register, and TR7 is the test    */
/* data register. These registers are accessed by variants of the MOV instruction. A test register may be either the source operand or         */
/* destination operand. The MOV instructions are defined in both real-address mode and protected mode. The test registers are privileged       */
/* resources; in protected mode, the MOV instructions that access them can only be executed at privilege level 0. An attempt to read or write  */
/* the test registers when executing at any other privilege level causes a general protection exception.                                       */
/*                                                                                                                                             */
/* The test command register (TR6) contains a command and an address tag to use in performing the command:                                     */
/*  - C: This is the command bit. There are two TLB testing commands: write entries into the TLB, and perform TLB lookups. To cause an         */
/*  immediate write into the TLB entry, move a doubleword into TR6 that contains a 0 in this bit. To cause an immediate TLB lookup, move       */
/*  doubleword into TR6 that contains a 1 in this bit.                                                                                         */
/*  - Linear Address: On a TLB write, a TLB entry is allocated to this linear address; the rest of that TLB entry is set per the value of TR7  */
/*  and the value just written into TR6. On a TLB lookup, the TLB is interrogated per this value; if one and only one TLB entry matches, the   */
/*  rest of the fields of TR6 and TR7 are set from the matching TLB entry.                                                                     */
/*  - V: The valid bit for this TLB entry. The TLB uses the valid bit to identify entries that contain valid data. Entries of the TLB that have*/
/*  not been assigned values have zero in the valid bit. All valid bits can be cleared by writing to CR3.                                      */
/*  - D, D#: The dirty bit (and its complement) for/from the TLB entry.                                                                        */
/*  - U, U#: The U/S bit (and its complement) for/from the TLB entry.                                                                          */
/*  - W, W#: The R/W bit (and its complement) for/from the TLB entry.                                                                          */
/* The meaning of these pairs of bits is given by Table 10-1, where X represents D, U, or W.                                                   */
/*                                                                                                                                             */
/* The test data register (TR7) holds data read from or data to be written to the TLB.                                                         */
/*  - Physical Address: This is the data filed of the TLB. On a write to the TLB, the TLB entry allocated to the linear address in TR6 is set  */
/*  to this value. On a TLB lookup, if HT is set, the data field (physical address) from the TLB is read out to this field. If HT is not set,  */
/*  this field is undefined.                                                                                                                   */
/*  - HT: For a TLB lookup, the HT bit indicates whether the lookup was a hit (HT := 1) or a miss (HT :=0). For a TLB write, HT must be set to */
/*  1.                                                                                                                                         */
/*  - REP: For a TLB write, selects which of four associative blocks of the TLB is to be written. For a TLB read, if HT is set, REP reports in */
/* which of the four associative blocks the tag was found; if HT is not set, REP is undefined.                                                 */
/* ------------------------------------------------------------------------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual; Chapter 12 Debugging:                                                                 */
/* ------------------------------------------------------------------------------------------------------------------------------------------- */
/* The 80386 brings to Intel's line of microprocessors significant advances in debugging power. The single-step exception and breakpoint       */
/* exception of previous processors are still available in the 80386, but the principal debugging support takes the form of debug registers.   */
/* The debug registers support both instruction breakpoints and data breakpoints. Data breakpoints are an important innovation that can save   */
/* hours of debugging time by pinpointing, for example, exactly when a data structure is being overwritten. The breakpoint registers also      */
/* eliminate the complexities associated with writing a breakpoint instruction into a code segment (requires a data-segment alias for the code */
/* segment) or a code segment shared by multiple tasks (the breakpoint exception can occur in the context of any of the tasks). Breakpoints can*/
/* even be set in code contained in ROM.                                                                                                       */
/* ------------------------------------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual; 12.1 Debugging Features of the Architecture:                                          */
/* ------------------------------------------------------------------------------------------------------------------------------------------- */
/* The features of the 80386 architecture that support debugging include:                                                                      */
/*  - Reserved debug interrupt vector: Permits processor to automatically invoke a debugger task or procedure when an event occurs that is of  */
/*                                     interest to the debugger.                                                                               */
/*  - Four debug address registers: Permit programmers to specify up to four addresses that the CPU will automatically monitor.                */
/*  - Debug control register: Allows programmers to selectively enable various debug conditions associated with the four debug addresses.      */
/*  - Debug status register: Helps debugger identify condition that caused debug exception.                                                    */
/*  - Trap bit of TSS (T-bit): Permits monitoring of task switches.                                                                            */
/*  - Resume flag (RF) of flags register: Allows an instruction to be restarted after a debug exception without immediately causing another    */
/*                                        debug exception due to the same condition.                                                           */
/*  - Single-step flag (TF): Allows complete monitoring of program flow by specifying whether the CPU should cause a debug exception with the  */
/*                           execution of every instruction.                                                                                   */
/*  - Breakpoint instruction: Permits debugger intervention at any point in program execution and aids debugging of debugger programs.         */
/*  -Reserved interrupt vector for breakpoint exception:Permits processor to automatically invoke a handler task or procedure upon encountering*/
/*                                                      a breakpoint instruction.                                                              */
/* These features make it possible to invoke a debugger that is either a separate task or a procedure in the context of the current task. The  */
/* debugger can be invoked under any of the following kinds of conditions:                                                                     */
/*  - Task switch to a specific task.                                                                                                          */
/*  - Execution of the breakpoint instruction.                                                                                                 */
/*  - Execution of every instruction.                                                                                                          */
/*  - Execution of any instruction at a given address.                                                                                         */
/*  - Read or write of a byte, word, or doubleword at any specified address.                                                                   */
/*  - Write to a byte, word, or doubleword at any specified address.                                                                           */
/*  - Attempt to change a debug register.                                                                                                      */
/* ------------------------------------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual; 12.2 Debug Registers:                                                                 */
/* ------------------------------------------------------------------------------------------------------------------------------------------- */
/* Six 80386 registers are used to control debug features. These registers are accessed by variants of the MOV instruction. A debug register   */
/* may be either the source operand or destination operand. The debug registers are privileged resources; the MOV instructions that access them*/
/* can only be executed at privilege level zero. An attempt to read or write the debug registers when executing at any other privilege level   */
/* causes a general protection exception. Figure 12-1 shows the format of the debug registers.                                                 */
/* ------------------------------------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual; 12.2.1 Debug Address Registers (DR0-DR3):                                             */
/* ------------------------------------------------------------------------------------------------------------------------------------------- */
/* Each of these registers contains the linear address associated with one of four breakpoint conditions. Each breakpoint condition is further */
/* defined by bits in DR7.                                                                                                                     */
/*                                                                                                                                             */
/* The debug address registers are effective whether or not paging is enabled. The addresses in these registers linear addresses. If paging is */
/* enabled, the linear addresses are translated into physical addresses by the processor's paging mechanism (as explained in Chapter 5). If    */
/* paging is not enabled, these linear addresses are the same as physical addresses.                                                           */
/*                                                                                                                                                     */
/* Note that when paging is enabled, different tasks may have different linear-to-physical address mappings. When this is the case, an address */
/* in a debug address register may be relevant to one task but not to another. For this reason the 80386 has both global and local enable bits */
/* in DR. These bits indicate whether a given debug address has a global (all tasks) or local (current tasks only) relevance.                  */
/* ------------------------------------------------------------------------------------------------------------------------------------------- */
/* typedef struct debug_address_registers_t {
	unsigned long  dr0_breakpoint_0_linear_address;
	unsigned long  dr1_breakpoint_1_linear_address;
	unsigned long  dr2_breakpoint_2_linear_address;
	unsigned long  dr3_breakpoint_3_linear_address;
} debug_address_registers_t;

typedef struct debug_reserved_registers_t {
	unsigned       dr4_reserved; /* Reserved by Intel */
/* 	unsigned       dr5_reserved; /* Reserved by Intel */
/* } debug_reserved_registers_t;
/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual; 12.2.2 Debug Control Register (DR7):                       */
/* ---------------------------------------------------------------------------------------------------------------- */
/* The debug control register shown in Figure 12-1 both helps to define the debug conditions and selectively enables*/
/* and disables those conditions.                                                                                   */
/*                                                                                                                  */
/* For each address in registers DR0-DR3, the corresponding fields R/W0 through R/W3 specify the type of action that*/
/* should cause a breakpoint. The processor interprets these bits as follows:                                       */
/*  - 00 -- Break on instruction execution only                                                                     */
/*  - 01 -- Break on data writes only                                                                               */
/*  - 10 -- undefined                                                                                               */
/*  - 11 -- Break on data reads or writes but not instruction fetches                                               */
/* Fields LEN0 through LEN3 specify the length of data item to be monitored. A length of 1, 2, or 4 bytes may be    */
/* specified. The values of the length fields are interpreted as follows:                                           */
/*  - 00 -- one-byte length                                                                                         */
/*  - 01 -- two-byte length                                                                                         */
/*  - 10 -- undefined                                                                                               */
/*  - 11 -- four-byte length                                                                                        */
/* If RWn is 00 (instruction execution), then LENn should also be 00. Any other length is undefined.                */
/*                                                                                                                  */
/* The low-order eight bits of DR7 (L0 through L3 and G0 through G3) selectively enable the four address breakpoint */
/* conditions. There are two levels of enabling: the local (L0 through L3) and global (G0 through G3) levels. The   */
/* local enable bits are automatically reset by the processor at every task switch to avoid unwanted breakpoint     */
/*conditions in the new task. The global enable bits are not reset by a task switch; therefore, they can be used for*/
/*conditions that are global to all tasks.                                                                          */
/*                                                                                                                  */
/* The LE and GE bits control the "exact data breakpoint match" feature of the processor. If either LE or GE is set,*/
/* the processor slows execution so that data breakpoints are reported on the instruction that causes them. It is   */
/* recommended that one of these bits be set whenever data breakpoints are armed. The processor clears LE at a task */
/* switch but does not clear GE.                                                                                    */
/* ---------------------------------------------------------------------------------------------------------------- */
/* typedef struct debug_control_registers_t {
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
} debug_control_registers_t;
/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual; 12.2.3 Debug Status Register (DR6):                        */
/* ---------------------------------------------------------------------------------------------------------------- */
/* The debug status register shown in Figure 12-1 permits the debugger to determine which debug conditions have     */
/* occurred.                                                                                                        */
/*                                                                                                                  */
/* When the processor detects an enabled debug exception, it sets the low-order bits of this register (B0 thru B3)  */
/* before entering the debug exception handler. Bn is set if the condition described by DRn, LENn, and R/Wn occurs. */
/* (Note that the processor sets Bn regardless of whether Gn or Ln is set. If more than one breakpoint condition    */
/*occurs at one time and if the breakpoint trap occurs due to an enabled condition other than n, Bn may be set, even*/
/* though neither Gn nor Ln is set.)                                                                                */
/*                                                                                                                  */
/*The BT bit is associated with the T-bit (debug trap bit) of the TSS (refer to 7 for the location of the T-bit).   */
/*The processor sets the BT bit before entering the debug handler if a task switch has occurred and the T-bit of the*/
/* new TSS is set. There is no corresponding bit in DR7 that enables and disables this trap; the T-bit of the TSS is*/
/* the sole enabling bit.                                                                                           */
/*                                                                                                                  */
/* The BS bit is associated with the TF (trap flag) bit of the EFLAGS register. The BS bit is set if the debug      */
/* handler is entered due to the occurrence of a single-step exception. The single-step trap is the highest-priority*/
/* debug exception; therefore, when BS is set, any of the other debug status bits may also be set.                  */
/*                                                                                                                  */
/* The BD bit is set if the next instruction will read or write one of the eight debug registers and ICE-386 is also*/
/* using the debug registers at the same time.                                                                      */
/*                                                                                                                  */
/*Note that the bits of DR6 are never cleared by the processor. To avoid any confusion in identifying the next debug*/
/* exception, the debug handler should move zeros to DR6 immediately before returning.                              */
/* ---------------------------------------------------------------------------------------------------------------- */
/* typedef struct debug_status_registers_t {
	unsigned       dr6_enabled_debug_exception_b0;
	unsigned       dr6_enabled_debug_exception_b1;
	unsigned       dr6_enabled_debug_exception_b2;
	unsigned       dr6_enabled_debug_exception_b3;
	unsigned       dr6_task_switch_bt;
	unsigned       dr6_single_step_exception_bs;
	unsigned       dr6_ice386_bd;
} debug_status_registers_t;
/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual; 12.2.4 Breakpoint Field Recognition:                       */
/* ---------------------------------------------------------------------------------------------------------------- */
/* The linear address and LEN field for each of the four breakpoint conditions define a range of sequential byte    */
/*addresses for a data breakpoint. The LEN field permits specification of a one-, two-, or four-byte field. Two-byte*/
/* fields must be aligned on word boundaries (addresses that are multiples of four). These requirements are enforced*/
/*by the processor; it uses the LEN bits to mask the low-order bits of the addresses in the debug address registers.*/
/* Improperly aligned code or data breakpoint addresses will not yield the expected results.                        */
/*                                                                                                                  */
/* A data read or write breakpoint is triggered if any of the bytes participating in a memory access is within the  */
/* field defined by a breakpoint address register and the corresponding LEN field. Table 12-1 gives some examples of*/
/* breakpoint fields with memory references that both do and do not cause traps.                                    */
/*                                                                                                                  */
/* To set a data breakpoint for a misaligned field longer than one byte; it may be desirable to put two sets of     */
/* entries in the breakpoint register such that each entry is properly aligned and the two entries together span the*/
/* length of the field.                                                                                             */
/*                                                                                                                  */
/* Instruction breakpoint addresses must have a length specification of one byte (LEN = 00); other values are       */
/* undefined. The processor recognizes an instruction breakpoint address only when it points to the first byte of an*/
/* instruction. If the instruction has any prefixes, the breakpoint address must point to the first prefix.         */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Table 12-1. Breakpoint Field Recognition Examples:                                                               */
/* Register Contents: Address (hex): Length:                                                                        */
/*  - DR0:             - 0A0001:      - 1 (LEN0 = 00)                                                               */
/*  - DR1:             - 0A0002:      - 1 (LEN1 = 00)                                                               */
/*  - DR2:             - 0B0002:      - 2 (LEN2 = 01)                                                               */
/*  - DR3:             - 0C0000:      - 4 (LEN3 = 11)                                                               */
/* Some Examples of Memory References That Cause Traps:                                                             */
/*                     - 0A0001:      - 2                                                                           */
/*                     - 0A0002:      - 2                                                                           */
/*                     - 0B0002:      - 2                                                                           */
/*                     - 0B0001:      - 4                                                                           */
/*                     - 0C0000:      - 4                                                                           */
/*                     - 0C0001:      - 2                                                                           */
/*                     - 0C0003:      - 1                                                                           */
/* Some Examples of Memory References That Don't Cause Traps:                                                       */
/*                     - 0A0000:      - 1                                                                           */
/*                     - 0A0003:      - 4                                                                           */
/*                     - 0B0000:      - 2                                                                           */
/*                     - 0C0004:      - 4                                                                           */
/* ---------------------------------------------------------------------------------------------------------------- */
typedef struct tss_task_state_segment_descriptor_t {
	unsigned long  tss_gdt_segment_base_address_offset;            /* Segment base:  16-bit (80286)/32-bit (80386 and higher) */
	unsigned long  tss_gdt_segment_limit;           /* Segment limit: 16-bit (80286)/32-bit (80386 and higher) */
	/* NOTE: Change "segment_limit" back to "size" if necessary and required by source code */
	/* unsigned char  alloc; */
	unsigned       tss_gdt_granularity_bit; /* Segment granularity bit: */
	unsigned       tss_gdt_big;             /* Big: "0" or "1" */
	unsigned       tss_gdt_default;           /* Default: "0" or "1" */
	unsigned       tss_gdt_x_;              /* X: "0" or "1" */
	unsigned       tss_gdt_0_;              /* 0: "0" or "1" */
	unsigned       tss_gdt_segment_available;       /* Segment AVL (available for use by systems programmers): "0" or "1" */
	unsigned       tss_gdt_segment_is_present;      /* Segment present: "1" if present and "0" if not present */
	unsigned       tss_gdt_descriptor_privilege_level; /* Descriptor privilege level: "1", "2", or "3" */
	unsigned       tss_gdt_type_of_segment_or_gate;          /* Type of Segment or Gate: */
	unsigned       tss_gdt_type_1;          /* Segment type 1: "0" or "1": */
	unsigned       tss_gdt_type_0;          /* Segment type 0: "0" or "1": */
	unsigned       tss_gdt_type_c;          /* Segment type conforming: "0" for conforming or "1" for non-conforming: */
	unsigned       tss_gdt_type_e;          /* Segment type expand-down: "0" or "1" */
	unsigned       tss_gdt_type_r;          /* Segment type readable: "0" or "1" */
	unsigned       tss_gdt_type_w;          /* Segment type writable: "0" or "1" */
	unsigned       tss_gdt_segment_accessed;        /* Segment A (accessed): "0" or "1" */
	unsigned       tss_gdt_selector_table_indicator;        /* Segment table indicator */
	unsigned long  tss_gdt_selector_index;        /* Segment table indicator */
	unsigned       tss_gdt_selector_requester_privilege_level;        /* Segment requester privilege level */
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
} tss_task_state_segment_descriptor_t;

typedef struct {
	tss_task_state_segment_descriptor_t      **tss;
} tss_t;
/* ---------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual; 15.1.2 Linear Address Formation:                           */
/* ---------------------------------------------------------------------------------------------------------------- */
/* In V86 mode, the 80386 processor does not interpret 8086 selectors by referring to descriptors; instead, it forms*/
/* linear addresses as an 8086 would. It shifts the selector left by four bits to form a 20-bit base address. The   */
/* effective address is extended with four high-order zeros and added to the base address to create a linear address*/
/* as Figure 15-1 illustrates.                                                                                      */
/*                                                                                                                  */
/*Because of the possibility of a carry, the resulting linear address may contain up to 21 significant bits. An 8086*/
/* program may generate linear addresses anywhere in the range 0 to 10FFEFH (one megabyte plus approximately 64     */
/* Kbytes) of the task's linear address space.                                                                      */
/*                                                                                                                  */
/* V86 tasks generate 32-bit linear addresses. While an 8086 program can only utilize the low-order 21 bits of a    */
/* linear address, the linear address can be mapped via page tables to any 32-bit physical address.                 */
/*                                                                                                                  */
/*Unlike the 8086 and 80286, 32-bit effective addresses can be generated (via the address-size prefix); however, the*/
/* value of a 32-bit address may not exceed 65,535 without causing an exception. For full compatibility with 80286  */
/* real-address mode, pseudo-protection faults (interrupt 12 or 13 with no error code) occur if an address is       */
/* generated outside the range 0 through 65,535.                                                                    */
/* ---------------------------------------------------------------------------------------------------------------- */
/* typedef struct virtual_8086_mode_address_t {
	unsigned long  v86_base_segment_selector;
	unsigned long  v86_offset_effective_address;
	/* See above for result of linear address */
/* } virtual_8086_mode_address_t; */

/* typedef struct {
	virtual_8086_mode_address_t      **v86;
} v86_t; */


/* typedef struct tr_register_for_task_state_segment_descriptor_t {
	unsigned long  tr_tss_gdt_segment_base_address_offset;            /* Segment base:  16-bit (80286)/32-bit (80386 and higher) */
/* 	unsigned long  tr_tss_gdt_segment_limit;           /* Segment limit: 16-bit (80286)/32-bit (80386 and higher) */
	/* NOTE: Change "segment_limit" back to "size" if necessary and required by source code */
	/* unsigned char  alloc; */
/* 	unsigned       tr_tss_gdt_granularity_bit; /* Segment granularity bit: */
/* 	unsigned       tr_tss_gdt_big;             /* Big: "0" or "1" */
/* 	unsigned       tr_tss_gdt_default;           /* Default: "0" or "1" */
/* 	unsigned       tr_tss_gdt_x_;              /* X: "0" or "1" */
/* 	unsigned       tr_tss_gdt_0_;              /* 0: "0" or "1" */
/* 	unsigned       tr_tss_gdt_segment_available;       /* Segment AVL (available for use by systems programmers): "0" or "1" */
/* 	unsigned       tr_tss_gdt_segment_is_present;      /* Segment present: "1" if present and "0" if not present */
/* 	unsigned       tr_tss_gdt_descriptor_privilege_level; /* Descriptor privilege level: "1", "2", or "3" */
/* 	unsigned       tr_tss_gdt_type_of_segment_or_gate;          /* Type of Segment or Gate: */
/* 	unsigned       tr_tss_gdt_type_1;          /* Segment type 1: "0" or "1": */
/* 	unsigned       tr_tss_gdt_type_0;          /* Segment type 0: "0" or "1": */
/* 	unsigned       tr_tss_gdt_type_c;          /* Segment type conforming: "0" for conforming or "1" for non-conforming: */
/* 	unsigned       tr_tss_gdt_type_e;          /* Segment type expand-down: "0" or "1" */
/* 	unsigned       tr_tss_gdt_type_r;          /* Segment type readable: "0" or "1" */
/* 	unsigned       tr_tss_gdt_type_w;          /* Segment type writable: "0" or "1" */
/* 	unsigned       tr_tss_gdt_selector_table_indicator;        /* Segment table indicator */
/* 	unsigned long  tr_tss_gdt_selector_index;        /* Segment table indicator */
/* 	unsigned       tr_tss_gdt_selector_requester_privilege_level;        /* Segment requester privilege level */
/* 	unsigned       tr_tss_gdt_segment_accessed;        /* Segment A (accessed): "0" or "1" */
/* 	unsigned long  tr_tss_ldt_segment_base_address_offset;            /* Segment base:  16-bit (80286)/32-bit (80386 and higher) */
/* 	unsigned long  tr_tss_ldt_segment_limit;           /* Segment limit: 16-bit (80286)/32-bit (80386 and higher) */
/* 	unsigned       tr_tss_ldt_granularity_bit; /* Segment granularity bit: */
/* 	unsigned       tr_tss_ldt_big;             /* Big: "0" or "1" */
/* 	unsigned       tr_tss_ldt_default;           /* Default: "0" or "1" */
/* 	unsigned       tr_tss_ldt_x_;              /* X: "0" or "1" */
/* 	unsigned       tr_tss_ldt_0_;              /* 0: "0" or "1" */
/* 	unsigned       tr_tss_ldt_segment_available;       /* Segment AVL (available for use by systems programmers): "0" or "1" */
/* 	unsigned       tr_tss_ldt_segment_is_present;      /* Segment present: "1" if present and "0" if not present */
/* 	unsigned       tr_tss_ldt_descriptor_privilege_level; /* Descriptor privilege level: "1", "2", or "3" */
/* 	unsigned       tr_tss_ldt_type_of_segment_or_gate;          /* Type of Segment or Gate: */
/* 	unsigned       tr_tss_ldt_type_1;          /* Segment type 1: "0" or "1": */
/* 	unsigned       tr_tss_ldt_type_0;          /* Segment type 0: "0" or "1": */
/* 	unsigned       tr_tss_ldt_type_c;          /* Segment type conforming: "0" for conforming or "1" for non-conforming: */
/* 	unsigned       tr_tss_ldt_type_e;          /* Segment type expand-down: "0" or "1" */
/* 	unsigned       tr_tss_ldt_type_r;          /* Segment type readable: "0" or "1" */
/* 	unsigned       tr_tss_ldt_type_w;          /* Segment type writable: "0" or "1" */
/* 	unsigned       tr_tss_ldt_segment_accessed;        /* Segment A (accessed): "0" or "1" */
/* 	unsigned       tr_tss_ldt_selector_table_indicator;        /* Segment table indicator */
/* 	unsigned long  tr_tss_ldt_selector_index;        /* Segment table indicator */
/* 	unsigned       tr_tss_ldt_selector_requester_privilege_level;        /* Segment requester privilege level */
	/* I/O (Input/Output) Map Base: */
/* 	unsigned long  input_output_map_base;                        /* I/O (Input/Output) Map Base */
	/* Debug trap bit (T): */
/* 	unsigned       debug_trap_bit;                               /* Debug trap bit (T) */
	/*  CPU Registers (32-bit) for tr_tss: */
/* 	unsigned long  tr_tss_register_eax;                                /* AX (16-bit)/eAX (32-bit) CPU register */
/* 	unsigned long  tr_tss_register_ebx;                                /* BX (16-bit)/eBX (32-bit) CPU register */
/* 	unsigned long  tr_tss_register_ecx;                                /* CX (16-bit)/eCX (32-bit) CPU register */
/* 	unsigned long  tr_tss_register_edx;                                /* DX (16-bit)/eDX (32-bit) CPU register */
/* 	unsigned long  tr_tss_register_esp;                                /* SP (16-bit)/eSP (32-bit) CPU register */
/* 	unsigned long  tr_tss_register_ebp;                                /* BP (16-bit)/eBP (32-bit) CPU register */
/* 	unsigned long  tr_tss_register_esi;                                /* SI (16-bit)/eSI (32-bit) CPU register */
/* 	unsigned long  tr_tss_register_edi;                                /* DI (16-bit)/eDI (32-bit) CPU register */
/* 	unsigned long  tr_tss_register_esp_for_privilege_level_2;          /* SP (16-bit)/eSP (32-bit) CPU register */
/* 	unsigned long  tr_tss_register_esp_for_privilege_level_1;          /* SP (16-bit)/eSP (32-bit) CPU register */
/* 	unsigned long  tr_tss_register_esp_for_privilege_level_0;          /* SP (16-bit)/eSP (32-bit) CPU register */
	/*  FLAGS (16-bit) register for tr_tss: */
/* 	unsigned       tr_tss_flags_cf;                                 /* Carry           Flag */
/* 	unsigned       tr_tss_flags_pf;                                 /* Parity          Flag */
/* 	unsigned       tr_tss_flags_ac;                                 /* Auxiliary Carry Flag */
/* 	unsigned       tr_tss_flags_zf;                                 /* Zero            Flag */
/* 	unsigned       tr_tss_flags_sf;                                 /* Sign            Flag */
/* 	unsigned       tr_tss_flags_tf;                                 /* Trap            Flag */
/* 	unsigned       tr_tss_flags_if;                                 /* Interrupt       Flag */
/* 	unsigned       tr_tss_flags_df;                                 /* Difference      Flag */
/* 	unsigned       tr_tss_flags_of;                                 /* Overflow        Flag */
	/* EFLAGS (32-bit) register for tr_tss: */
/* 	unsigned       tr_tss_eflags_iopl;                              /* Input/Output Privilege Level Flag */
/* 	unsigned       tr_tss_eflags_nt;                                /* Nested Task                  Flag */
/* 	unsigned       tr_tss_eflags_rf;                                /* Return                       Flag */
/* 	unsigned       tr_tss_eflags_vm;                                /* Virtual Machine              Flag */
	/* Instruction Pointer (EIP) for tr_tss: */
/* 	unsigned long  tr_tss_instruction_pointer_eip;                  /* Instruction Pointer (EIP) */
	/* Control Register 3 (Page Directory Base Register): */
/* 	unsigned long  tr_tss_pdbr_page_frame_address_specified;          /* Page frame address:  32-bit (80386 and higher) */
/* 	unsigned long  tr_tss_pdbr_page_frame_address_being_loaded;       /* Page frame address:  32-bit (80386 and higher) */
/* 	unsigned long  tr_tss_pdbr_page_frame_address;                    /* Page frame address:  32-bit (80386 and higher) */
/* 	unsigned       tr_tss_pdbr_page_table_is_present;                 /* Page table present: "1" if present and "0" if not present */
/* 	unsigned       tr_tss_pdbr_page_table_read_write;                 /* Page table read/write: "0" if read-only and "1" if read/write */
/* 	unsigned       tr_tss_pdbr_page_table_user_supervisor;            /* Page table user/supervisor: "0" if user and "1" if supervisor: */
/* 	unsigned       tr_tss_pdbr_page_table_dirty;                      /* Page table Default: "0" or "1" */
/* 	unsigned       tr_tss_pdbr_page_table_available;                  /* Page table available for systems programmer use: "0" or "1" */
	/* Page Directory: */
/* 	unsigned long  tr_tss_directory_page_frame_address_specified;          /* Page frame address:  32-bit (80386 and higher) */
/* 	unsigned long  tr_tss_directory_page_frame_address_being_loaded;       /* Page frame address:  32-bit (80386 and higher) */
/* 	unsigned long  tr_tss_directory_page_frame_address;                    /* Page frame address:  32-bit (80386 and higher) */
/* 	unsigned       tr_tss_directory_page_table_read_write;                 /* Page table read/write: "0" if read-only and "1" if read/write */
/* 	unsigned       tr_tss_directory_page_table_user_supervisor;            /* Page table user/supervisor: "0" if user and "1" if supervisor: */
/* 	unsigned       tr_tss_directory_page_table_dirty;                      /* Page table Default: "0" or "1" */
/* 	unsigned       tr_tss_directory_page_table_available;                  /* Page table available for systems programmer use: "0" or "1" */
	/* ESP (stack pointer register) for tr_tss: */
/* 	unsigned long  tr_tss_stack_pointer_esp_for_privilege_level_2;  /* ESP (stack pointer register) for Privilege Level 2 */
/* 	unsigned long  tr_tss_stack_pointer_esp_for_privilege_level_1;  /* ESP (stack pointer register) for Privilege Level 1 */
/* 	unsigned long  tr_tss_stack_pointer_esp_for_privilege_level_0;  /* ESP (stack pointer register) for Privilege Level 0 */
	/* CS segment register for tr_tss: */
/* 	unsigned long  tr_tss_cs_segment_register_descriptor_base_address_offset;                       /* Segment base:  16-bit (80286)/32-bit (80386 and higher) */
/* 	unsigned long  tr_tss_cs_segment_register_descriptor_limit;                      /* Segment limit: 16-bit (80286)/32-bit (80386 and higher) */
	/* NOTE: Change "segment_limit" back to "size" if necessary and required by source code */
/* 	unsigned       tr_tss_cs_segment_register_descriptor_granularity_bit;            /* Segment granularity bit: */
/* 	unsigned       tr_tss_cs_segment_register_descriptor_big;                        /* Big: "0" or "1" */
/* 	unsigned       tr_tss_cs_segment_register_descriptor_default;                      /* Default: "0" or "1" */
/* 	unsigned       tr_tss_cs_segment_register_descriptor_x_;                         /* X: "0" or "1" */
/* 	unsigned       tr_tss_cs_segment_register_descriptor_0_;                         /* 0: "0" or "1" */
/* 	unsigned       tr_tss_cs_segment_register_descriptor_available;                  /* Segment AVL (available for use by systems programmers): "0" or "1" */
/* 	unsigned       tr_tss_cs_segment_register_descriptor_is_present;                 /* Segment present: "1" if present and "0" if not present */
/* 	unsigned       tr_tss_cs_segment_register_descriptor_privilege_level;            /* Descriptor privilege level: "1", "2", or "3" */
/* 	unsigned       tr_tss_cs_segment_register_type_of_segment_or_gate;               /* Type of Segment or Gate: */
/* 	unsigned       tr_tss_cs_segment_register_descriptor_type_1;                     /* Segment type 1: "0" or "1": */
/* 	unsigned       tr_tss_cs_segment_register_descriptor_type_0;                     /* Segment type 0: "0" or "1": */
/* 	unsigned       tr_tss_cs_segment_register_descriptor_type_c;                     /* Segment type conforming: "0" for conforming or "1" for non-conforming: */
/* 	unsigned       tr_tss_cs_segment_register_descriptor_type_e;                     /* Segment type expand-down: "0" or "1" */
/* 	unsigned       tr_tss_cs_segment_register_descriptor_type_r;                     /* Segment type readable: "0" or "1" */
/* 	unsigned       tr_tss_cs_segment_register_descriptor_type_w;                     /* Segment type writable: "0" or "1" */
/* 	unsigned       tr_tss_cs_segment_register_descriptor_accessed;                   /* Segment A (accessed): "0" or "1" */
/* 	unsigned       tr_tss_cs_segment_register_selector_table_indicator;        /* Segment table indicator */
/* 	unsigned long  tr_tss_cs_segment_register_selector_index;        /* Segment table indicator */
/* 	unsigned       tr_tss_cs_segment_register_selector_requester_privilege_level;        /* Segment requester privilege level */
	/* DS segment register for tr_tss: */
/* 	unsigned long  tr_tss_ds_segment_register_descriptor_base_address_offset;                       /* Segment base:  16-bit (80286)/32-bit (80386 and higher) */
/* 	unsigned long  tr_tss_ds_segment_register_descriptor_limit;                      /* Segment limit: 16-bit (80286)/32-bit (80386 and higher) */
	/* NOTE: Change "segment_limit" back to "size" if necessary and required by source code */
/* 	unsigned       tr_tss_ds_segment_register_descriptor_granularity_bit;            /* Segment granularity bit: */
/* 	unsigned       tr_tss_ds_segment_register_descriptor_big;                        /* Big: "0" or "1" */
/* 	unsigned       tr_tss_ds_segment_register_descriptor_default;                      /* Default: "0" or "1" */
/* 	unsigned       tr_tss_ds_segment_register_descriptor_x_;                         /* X: "0" or "1" */
/* 	unsigned       tr_tss_ds_segment_register_descriptor_0_;                         /* 0: "0" or "1" */
/* 	unsigned       tr_tss_ds_segment_register_descriptor_available;                  /* Segment AVL (available for use by systems programmers): "0" or "1" */
/* 	unsigned       tr_tss_ds_segment_register_descriptor_is_present;                 /* Segment present: "1" if present and "0" if not present */
/* 	unsigned       tr_tss_ds_segment_register_descriptor_privilege_level;            /* Descriptor privilege level: "1", "2", or "3" */
/* 	unsigned       tr_tss_ds_segment_register_type_of_segment_or_gate;               /* Type of Segment or Gate: */
/* 	unsigned       tr_tss_ds_segment_register_descriptor_type_1;                     /* Segment type 1: "0" or "1": */
/* 	unsigned       tr_tss_ds_segment_register_descriptor_type_0;                     /* Segment type 0: "0" or "1": */
/* 	unsigned       tr_tss_ds_segment_register_descriptor_type_c;                     /* Segment type conforming: "0" for conforming or "1" for non-conforming: */
/* 	unsigned       tr_tss_ds_segment_register_descriptor_type_e;                     /* Segment type expand-down: "0" or "1" */
/* 	unsigned       tr_tss_ds_segment_register_descriptor_type_r;                     /* Segment type readable: "0" or "1" */
/* 	unsigned       tr_tss_ds_segment_register_descriptor_type_w;                     /* Segment type writable: "0" or "1" */
/* 	unsigned       tr_tss_ds_segment_register_descriptor_accessed;                   /* Segment A (accessed): "0" or "1" */
/* 	unsigned       tr_tss_ds_segment_register_selector_table_indicator;        /* Segment table indicator */
/* 	unsigned long  tr_tss_ds_segment_register_selector_index;        /* Segment table indicator */
/* 	unsigned       tr_tss_ds_segment_register_selector_requester_privilege_level;        /* Segment requester privilege level */
	/* ES segment register for tr_tss: */
/* 	unsigned long  tr_tss_es_segment_register_descriptor_base_address_offset;                       /* Segment base:  16-bit (80286)/32-bit (80386 and higher) */
/* 	unsigned long  tr_tss_es_segment_register_descriptor_limit;                      /* Segment limit: 16-bit (80286)/32-bit (80386 and higher) */
	/* NOTE: Change "segment_limit" back to "size" if necessary and required by source code */
/* 	unsigned       tr_tss_es_segment_register_descriptor_granularity_bit;            /* Segment granularity bit: */
/* 	unsigned       tr_tss_es_segment_register_descriptor_big;                        /* Big: "0" or "1" */
/* 	unsigned       tr_tss_es_segment_register_descriptor_default;                      /* Default: "0" or "1" */
/* 	unsigned       tr_tss_es_segment_register_descriptor_x_;                         /* X: "0" or "1" */
/* 	unsigned       tr_tss_es_segment_register_descriptor_0_;                         /* 0: "0" or "1" */
/* 	unsigned       tr_tss_es_segment_register_descriptor_available;                  /* Segment AVL (available for use by systems programmers): "0" or "1" */
/* 	unsigned       tr_tss_es_segment_register_descriptor_is_present;                 /* Segment present: "1" if present and "0" if not present */
/* 	unsigned       tr_tss_es_segment_register_descriptor_privilege_level;            /* Descriptor privilege level: "1", "2", or "3" */
/* 	unsigned       tr_tss_es_segment_register_type_of_segment_or_gate;               /* Type of Segment or Gate: */
/* 	unsigned       tr_tss_es_segment_register_descriptor_type_1;                     /* Segment type 1: "0" or "1": */
/* 	unsigned       tr_tss_es_segment_register_descriptor_type_0;                     /* Segment type 0: "0" or "1": */
/* 	unsigned       tr_tss_es_segment_register_descriptor_type_c;                     /* Segment type conforming: "0" for conforming or "1" for non-conforming: */
/* 	unsigned       tr_tss_es_segment_register_descriptor_type_e;                     /* Segment type expand-down: "0" or "1" */
/* 	unsigned       tr_tss_es_segment_register_descriptor_type_r;                     /* Segment type readable: "0" or "1" */
/* 	unsigned       tr_tss_es_segment_register_descriptor_type_w;                     /* Segment type writable: "0" or "1" */
/* 	unsigned       tr_tss_es_segment_register_descriptor_accessed;                   /* Segment A (accessed): "0" or "1" */
/* 	unsigned       tr_tss_es_segment_register_selector_table_indicator;        /* Segment table indicator */
/* 	unsigned long  tr_tss_es_segment_register_selector_index;        /* Segment table indicator */
/* 	unsigned       tr_tss_es_segment_register_selector_requester_privilege_level;        /* Segment requester privilege level */
	/* DS segment register for tr_tss: */
/* 	unsigned long  tr_tss_fs_segment_register_descriptor_base_address_offset;                       /* Segment base:  16-bit (80286)/32-bit (80386 and higher) */
/* 	unsigned long  tr_tss_fs_segment_register_descriptor_limit;                      /* Segment limit: 16-bit (80286)/32-bit (80386 and higher) */
	/* NOTE: Change "segment_limit" back to "size" if necessary and required by source code */
/* 	unsigned       tr_tss_fs_segment_register_descriptor_granularity_bit;            /* Segment granularity bit: */
/* 	unsigned       tr_tss_fs_segment_register_descriptor_big;                        /* Big: "0" or "1" */
/* 	unsigned       tr_tss_fs_segment_register_descriptor_default;                      /* Default: "0" or "1" */
/* 	unsigned       tr_tss_fs_segment_register_descriptor_x_;                         /* X: "0" or "1" */
/* 	unsigned       tr_tss_fs_segment_register_descriptor_0_;                         /* 0: "0" or "1" */
/* 	unsigned       tr_tss_fs_segment_register_descriptor_available;                  /* Segment AVL (available for use by systems programmers): "0" or "1" */
/* 	unsigned       tr_tss_fs_segment_register_descriptor_is_present;                 /* Segment present: "1" if present and "0" if not present */
/* 	unsigned       tr_tss_fs_segment_register_descriptor_privilege_level;            /* Descriptor privilege level: "1", "2", or "3" */
/* 	unsigned       tr_tss_fs_segment_register_type_of_segment_or_gate;               /* Type of Segment or Gate: */
/* 	unsigned       tr_tss_fs_segment_register_descriptor_type_1;                     /* Segment type 1: "0" or "1": */
/* 	unsigned       tr_tss_fs_segment_register_descriptor_type_0;                     /* Segment type 0: "0" or "1": */
/* 	unsigned       tr_tss_fs_segment_register_descriptor_type_c;                     /* Segment type conforming: "0" for conforming or "1" for non-conforming: */
/* 	unsigned       tr_tss_fs_segment_register_descriptor_type_e;                     /* Segment type expand-down: "0" or "1" */
/* 	unsigned       tr_tss_fs_segment_register_descriptor_type_r;                     /* Segment type readable: "0" or "1" */
/* 	unsigned       tr_tss_fs_segment_register_descriptor_type_w;                     /* Segment type writable: "0" or "1" */
/* 	unsigned       tr_tss_fs_segment_register_descriptor_accessed;                   /* Segment A (accessed): "0" or "1" */
/* 	unsigned       tr_tss_fs_segment_register_selector_table_indicator;        /* Segment table indicator */
/* 	unsigned long  tr_tss_fs_segment_register_selector_index;        /* Segment table indicator */
/* 	unsigned       tr_tss_fs_segment_register_selector_requester_privilege_level;        /* Segment requester privilege level */
	/* DS segment register for tr_tss: */
/* 	unsigned long  tr_tss_gs_segment_register_descriptor_base_address_offset;                       /* Segment base:  16-bit (80286)/32-bit (80386 and higher) */
/* 	unsigned long  tr_tss_gs_segment_register_descriptor_limit;                      /* Segment limit: 16-bit (80286)/32-bit (80386 and higher) */
	/* NOTE: Change "segment_limit" back to "size" if necessary and required by source code */
/* 	unsigned       tr_tss_gs_segment_register_descriptor_granularity_bit;            /* Segment granularity bit: */
/* 	unsigned       tr_tss_gs_segment_register_descriptor_big;                        /* Big: "0" or "1" */
/* 	unsigned       tr_tss_gs_segment_register_descriptor_default;                      /* Default: "0" or "1" */
/* 	unsigned       tr_tss_gs_segment_register_descriptor_x_;                         /* X: "0" or "1" */
/* 	unsigned       tr_tss_gs_segment_register_descriptor_0_;                         /* 0: "0" or "1" */
/* 	unsigned       tr_tss_gs_segment_register_descriptor_available;                  /* Segment AVL (available for use by systems programmers): "0" or "1" */
/* 	unsigned       tr_tss_gs_segment_register_descriptor_is_present;                 /* Segment present: "1" if present and "0" if not present */
/* 	unsigned       tr_tss_gs_segment_register_descriptor_privilege_level;            /* Descriptor privilege level: "1", "2", or "3" */
/* 	unsigned       tr_tss_gs_segment_register_type_of_segment_or_gate;               /* Type of Segment or Gate: */
/* 	unsigned       tr_tss_gs_segment_register_descriptor_type_1;                     /* Segment type 1: "0" or "1": */
/* 	unsigned       tr_tss_gs_segment_register_descriptor_type_0;                     /* Segment type 0: "0" or "1": */
/* 	unsigned       tr_tss_gs_segment_register_descriptor_type_c;                     /* Segment type conforming: "0" for conforming or "1" for non-conforming: */
/* 	unsigned       tr_tss_gs_segment_register_descriptor_type_e;                     /* Segment type expand-down: "0" or "1" */
/* 	unsigned       tr_tss_gs_segment_register_descriptor_type_r;                     /* Segment type readable: "0" or "1" */
/* 	unsigned       tr_tss_gs_segment_register_descriptor_type_w;                     /* Segment type writable: "0" or "1" */
/* 	unsigned       tr_tss_gs_segment_register_descriptor_accessed;                   /* Segment A (accessed): "0" or "1" */
/* 	unsigned       tr_tss_gs_segment_register_selector_table_indicator;        /* Segment table indicator */
/* 	unsigned long  tr_tss_gs_segment_register_selector_index;        /* Segment table indicator */
/* 	unsigned       tr_tss_gs_segment_register_selector_requester_privilege_level;        /* Segment requester privilege level */
	/* SS segment register (stack segment) for tr_tss (privilege level 2): */
/* 	unsigned long  tr_tss_ss_segment_register_descriptor_base_address_offset_for_privilege_level_2;            /* Segment base:  16-bit (80286)/32-bit (80386 and higher) */
/* 	unsigned long  tr_tss_ss_segment_register_descriptor_limit_for_privilege_level_2;           /* Segment limit: 16-bit (80286)/32-bit (80386 and higher) */
	/* NOTE: Change "segment_limit" back to "size" if necessary and required by source code */
/* 	unsigned       tr_tss_ss_segment_register_descriptor_granularity_bit_for_privilege_level_2; /* Segment granularity bit: */
/* 	unsigned       tr_tss_ss_segment_register_descriptor_big_for_privilege_level_2;             /* Big: "0" or "1" */
/* 	unsigned       tr_tss_ss_segment_register_descriptor_default_for_privilege_level_2;           /* Default: "0" or "1" */
/* 	unsigned       tr_tss_ss_segment_register_descriptor_x_for_privilege_level_2;               /* X: "0" or "1" */
/* 	unsigned       tr_tss_ss_segment_register_descriptor_0_for_privilege_level_2;               /* 0: "0" or "1" */
/* 	unsigned       tr_tss_ss_segment_register_descriptor_available_for_privilege_level_2;/* Segment AVL(available for use by systems programmers):"0"or"1" */
/* 	unsigned       tr_tss_ss_segment_register_descriptor_is_present_for_privilege_level_2;      /* Segment present: "1" if present and "0" if not present */
/* 	unsigned       tr_tss_ss_segment_register_descriptor_privilege_level_for_privilege_level_2; /* Descriptor privilege level: "1", "2", or "3" */
/* 	unsigned       tr_tss_ss_segment_register_type_of_segment_or_gate_for_privilege_level_2;    /* Type of Segment or Gate: */
/* 	unsigned       tr_tss_ss_segment_register_descriptor_type_1_for_privilege_level_2;          /* Segment type 1: "0" or "1": */
/* 	unsigned       tr_tss_ss_segment_register_descriptor_type_0_for_privilege_level_2;          /* Segment type 0: "0" or "1": */
/* 	unsigned       tr_tss_ss_segment_register_descriptor_type_c_for_privilege_level_2;/*Segment type conforming:"0"for conforming or "1"for non-conforming:*/
/* 	unsigned       tr_tss_ss_segment_register_descriptor_type_e_for_privilege_level_2;          /* Segment type expand-down: "0" or "1" */
/* 	unsigned       tr_tss_ss_segment_register_descriptor_type_r_for_privilege_level_2;          /* Segment type readable: "0" or "1" */
/* 	unsigned       tr_tss_ss_segment_register_descriptor_type_w_for_privilege_level_2;          /* Segment type writable: "0" or "1" */
/* 	unsigned       tr_tss_ss_segment_register_descriptor_accessed_for_privilege_level_2;        /* Segment A (accessed): "0" or "1" */
/* 	unsigned       tr_tss_ss_segment_register_selector_table_indicator_for_privilege_level_2;   /* Segment table indicator */
/* 	unsigned long  tr_tss_ss_segment_register_selector_index_for_privilege_level_2;             /* Segment table indicator */
/* 	unsigned       tr_tss_ss_segment_register_selector_requester_privilege_level_for_privilege_level_2; /* Segment requester privilege level */
	/* Stack segment for tr_tss (privilege level 1): */
/* 	unsigned long  tr_tss_ss_segment_register_descriptor_base_address_offset_for_privilege_level_1;            /* Segment base:  16-bit (80286)/32-bit (80386 and higher) */
/* 	unsigned long  tr_tss_ss_segment_register_descriptor_limit_for_privilege_level_1;           /* Segment limit: 16-bit (80286)/32-bit (80386 and higher) */
	/* NOTE: Change "segment_limit" back to "size" if necessary and required by source code */
/* 	unsigned       tr_tss_ss_segment_register_descriptor_granularity_bit_for_privilege_level_1; /* Segment granularity bit: */
/* 	unsigned       tr_tss_ss_segment_register_descriptor_big_for_privilege_level_1;             /* Big: "0" or "1" */
/* 	unsigned       tr_tss_ss_segment_register_descriptor_default_for_privilege_level_1;           /* Default: "0" or "1" */
/* 	unsigned       tr_tss_ss_segment_register_descriptor_x_for_privilege_level_1;               /* X: "0" or "1" */
/* 	unsigned       tr_tss_ss_segment_register_descriptor_0_for_privilege_level_1;               /* 0: "0" or "1" */
/* 	unsigned       tr_tss_ss_segment_register_descriptor_available_for_privilege_level_1;/* Segment AVL(available for use by systems programmers):"0"or"1" */
/* 	unsigned       tr_tss_ss_segment_register_descriptor_is_present_for_privilege_level_1;      /* Segment present: "1" if present and "0" if not present */
/* 	unsigned       tr_tss_ss_segment_register_descriptor_privilege_level_for_privilege_level_1; /* Descriptor privilege level: "1", "2", or "3" */
/* 	unsigned       tr_tss_ss_segment_register_type_of_segment_or_gate_for_privilege_level_1;    /* Type of Segment or Gate: */
/* 	unsigned       tr_tss_ss_segment_register_descriptor_type_1_for_privilege_level_1;          /* Segment type 1: "0" or "1": */
/* 	unsigned       tr_tss_ss_segment_register_descriptor_type_0_for_privilege_level_1;          /* Segment type 0: "0" or "1": */
/* 	unsigned       tr_tss_ss_segment_register_descriptor_type_c_for_privilege_level_1;/*Segment type conforming:"0"for conforming or "1"for non-conforming:*/
/* 	unsigned       tr_tss_ss_segment_register_descriptor_type_e_for_privilege_level_1;          /* Segment type expand-down: "0" or "1" */
/* 	unsigned       tr_tss_ss_segment_register_descriptor_type_r_for_privilege_level_1;          /* Segment type readable: "0" or "1" */
/* 	unsigned       tr_tss_ss_segment_register_descriptor_type_w_for_privilege_level_1;          /* Segment type writable: "0" or "1" */
/* 	unsigned       tr_tss_ss_segment_register_descriptor_accessed_for_privilege_level_1;        /* Segment A (accessed): "0" or "1" */
/* 	unsigned       tr_tss_ss_segment_register_selector_table_indicator_for_privilege_level_1;   /* Segment table indicator */
/* 	unsigned long  tr_tss_ss_segment_register_selector_index_for_privilege_level_1;             /* Segment table indicator */
/* 	unsigned       tr_tss_ss_segment_register_selector_requester_privilege_level_for_privilege_level_1; /* Segment requester privilege level */
	/* Stack segment for tr_tss (privilege level 0): */
/* 	unsigned long  tr_tss_ss_segment_register_descriptor_base_address_offset_for_privilege_level_0;            /* Segment base:  16-bit (80286)/32-bit (80386 and higher) */
/* 	unsigned long  tr_tss_ss_segment_register_descriptor_limit_for_privilege_level_0;           /* Segment limit: 16-bit (80286)/32-bit (80386 and higher) */
	/* NOTE: Change "segment_limit" back to "size" if necessary and required by source code */
/* 	unsigned       tr_tss_ss_segment_register_descriptor_granularity_bit_for_privilege_level_0; /* Segment granularity bit: */
/* 	unsigned       tr_tss_ss_segment_register_descriptor_big_for_privilege_level_0;             /* Big: "0" or "1" */
/* 	unsigned       tr_tss_ss_segment_register_descriptor_default_for_privilege_level_0;           /* Default: "0" or "1" */
/* 	unsigned       tr_tss_ss_segment_register_descriptor_x_for_privilege_level_0;               /* X: "0" or "1" */
/* 	unsigned       tr_tss_ss_segment_register_descriptor_0_for_privilege_level_0;               /* 0: "0" or "1" */
/* 	unsigned       tr_tss_ss_segment_register_descriptor_available_for_privilege_level_0;/* Segment AVL(available for use by systems programmers):"0"or"1" */
/* 	unsigned       tr_tss_ss_segment_register_descriptor_is_present_for_privilege_level_0;      /* Segment present: "1" if present and "0" if not present */
/* 	unsigned       tr_tss_ss_segment_register_descriptor_privilege_level_for_privilege_level_0; /* Descriptor privilege level: "1", "2", or "3" */
/* 	unsigned       tr_tss_ss_segment_register_type_of_segment_or_gate_for_privilege_level_0;    /* Type of Segment or Gate: */
/* 	unsigned       tr_tss_ss_segment_register_descriptor_type_1_for_privilege_level_0;          /* Segment type 1: "0" or "1": */
/* 	unsigned       tr_tss_ss_segment_register_descriptor_type_0_for_privilege_level_0;          /* Segment type 0: "0" or "1": */
/* 	unsigned       tr_tss_ss_segment_register_descriptor_type_c_for_privilege_level_0;/*Segment type conforming:"0"for conforming or "1"for non-conforming:*/
/* 	unsigned       tr_tss_ss_segment_register_descriptor_type_e_for_privilege_level_0;          /* Segment type expand-down: "0" or "1" */
/* 	unsigned       tr_tss_ss_segment_register_descriptor_type_r_for_privilege_level_0;          /* Segment type readable: "0" or "1" */
/* 	unsigned       tr_tss_ss_segment_register_descriptor_type_w_for_privilege_level_0;          /* Segment type writable: "0" or "1" */
/* 	unsigned       tr_tss_ss_segment_register_descriptor_accessed_for_privilege_level_0;        /* Segment A (accessed): "0" or "1" */
/* 	unsigned       tr_tss_ss_segment_register_selector_table_indicator_for_privilege_level_0;   /* Segment table indicator */
/* 	unsigned long  tr_tss_ss_segment_register_selector_index_for_privilege_level_0;             /* Segment table indicator */
/* 	unsigned       tr_tss_ss_segment_register_selector_requester_privilege_level_for_privilege_level_0; /* Segment requester privilege level */
	/* Task State Segment (tr_tss) Back Link: */
/* 	unsigned long  tr_tss_back_link;                                /* tr_tss (Task State Segment) back link */
	/* Task Gate Descriptor: */
/* 	unsigned       task_gate_segment_limit;   /* Used for referring to Task State Segment descriptor only */
/* 	unsigned       task_gate_segment_present;      /* Segment present: "1" if present and "0" if not present */
/* 	unsigned       task_gate_is_present;      /* Segment present: "1" if present and "0" if not present */
/* 	unsigned       task_gate_privilege_level; /* Descriptor privilege level: "1", "2", or "3" */
/* 	unsigned       task_gate_type_of_segment_or_gate;          /* Type of Segment or Gate: */
/* 	unsigned       task_gate_selector_table_indicator;        /* Segment table indicator */
/* 	unsigned long  task_gate_selector_index;        /* Segment table indicator */
/* 	unsigned       task_gate_selector_requester_privilege_level;        /* Segment requester privilege level */
	/* Used for task gates residing in Interrupt Descriptor Table (Interrupt Gates): */
/* 	unsigned long  task_gate_idt_gate_for_interrupt_selector;            /* Interrupt descriptor base:  16-bit (80286)/32-bit (80386 and higher) */
/* 	unsigned long  task_gate_idt_gate_for_interrupt_offset;           /* Interrupt descriptor limit: 16-bit (80286)/32-bit (80386 and higher) */
/* 	unsigned       task_gate_idt_gate_for_interrupt_present;          /* Present: */
/* 	unsigned       task_gate_idt_gate_for_interrupt_descriptor_privilege_level;          /* Descriptor Privilege Level: */
/* 	unsigned       task_gate_idt_gate_for_interrupt_number_n_;          /* Gate for Interrupt #N: */
/* 	unsigned       task_gate_idt_gate_for_interrupt_number_2_;          /* Gate for Interrupt #2: */
/* 	unsigned       task_gate_idt_gate_for_interrupt_number_1_;          /* Gate for Interrupt #1: */
/* 	unsigned       task_gate_idt_gate_for_interrupt_number_0_;          /* Gate for Interrupt #0: */
	/* Used for task gates residing in Interrupt Descriptor Table (Trap Gates): */
/* 	unsigned long  task_gate_idt_gate_for_trap_selector;            /* Interrupt descriptor base:  16-bit (80286)/32-bit (80386 and higher) */
/* 	unsigned long  task_gate_idt_gate_for_trap_offset;           /* Interrupt descriptor limit: 16-bit (80286)/32-bit (80386 and higher) */
/* 	unsigned       task_gate_idt_gate_for_trap_present;          /* Present: */
/* 	unsigned       task_gate_idt_gate_for_trap_descriptor_privilege_level;          /* Descriptor Privilege Level: */
/* 	unsigned       task_gate_idt_gate_for_trap_number_n_;          /* Gate for Interrupt #N: */
/* 	unsigned       task_gate_idt_gate_for_trap_number_2_;          /* Gate for Interrupt #2: */
/* 	unsigned       task_gate_idt_gate_for_trap_number_1_;          /* Gate for Interrupt #1: */
/* 	unsigned       task_gate_idt_gate_for_trap_number_0_;          /* Gate for Interrupt #0: */
/* } tr_register_for_task_state_segment_descriptor_t;


/* Type of segment descriptors or gates: */
/*  - 0: Invalid */
/*  - 1: Available 80286 TSS */
/*  - 2: LDT */
/*  - 3: Busy 80286 TSS */
/*  - 4: 80286 call gate */
/*  - 5: 80286/80386 task gate */
/*  - 6: 80286 trap gate */
/*  - 7: 80286 interrupt gate */
/*  - 8: Invalid */
/*  - 9: Available 80386 TSS */
/*  - A: Invalid */
/*  - B: Busy 80386 TSS */
/*  - C: 80386 call gate */
/*  - D: Invalid */
/*  - E: 80386 trap gate: */
/*  - F: 0386 interrupt gate */
/* ldt_t *ldt_new (ini_sct_t *sct); /* Bring in code from MAIN.H and MAIN.C for this one */
/* void ldt_del (ldt_t *ldt); */
/* mmu386_blk_t *ldt_get_seg_ldt_mem (ldt_t *ldt); */
/* void ldt_reset (ldt_t *ldt); */
/* void ldt_handler (ldt_t *ldt, e8086_t *cpu); */


#endif
