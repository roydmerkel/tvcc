/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/e8086/ea.c                                           *
 * Created:     1996-04-28 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 1996-2012 Hampa Hug <hampa@hampa.ch>                     *
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
#include "internal.h"

/* ------------------------------------------------------------------------------------------------------------------------------------------- */
/* Quote from Intel 80386 Programmer's Reference Manual; 14.1 Physical Address Formation:                                                      */
/* ------------------------------------------------------------------------------------------------------------------------------------------- */
/* The 80386 provides a one Mbyte + 64 Kbyte memory space for an 8086 program. Segment relocation is performed as in the 8086: the 16-bit value*/
/* in a segment selector is shifted left by four bits to form the base address of a segment. The effective address is extended with four high  */
/* order zeros and added to the base to form a linear address as Figure 14-1 illustrates. (The linear address is equivalent to the physical    */
/* address, because paging is not used in real-address mode.) Unlike the 8086, the resulting linear address may have up to 21 significant bits.*/
/*There is a possibility of a carry when the base address is added to the effective address. On the 8086, the carried bit is truncated, whereas*/
/* on the 80386 the carried bit is stored in bit position 20 of the linear address.                                                            */
/*                                                                                                                                             */
/* Unlike the 8086 and 80286, 32-bit effective addresses can be generated (via the address-size prefix); however, the value of a 32-bit address*/
/* may not exceed 65535 without causing an exception. For full compatibility with 80286 real-address mode, pseudo-protection faults (interrupt */
/* 12 or 13 with no error code) occur if an effective address is generated outside the range 0 through 65535.                                  */
/* ------------------------------------------------------------------------------------------------------------------------------------------- */
unsigned long real_mode_effective_address_limit;
real_mode_effective_address_limit = 65535;
/* Use below for when granularity bit occurs: */
/* segment_limit = insert_here;
granularity = 0xffffffff;
new_limit = granularity + segment_limit; */
/* EA 00: [BX + SI] or [eBX + eSI] */ /* Real Mode */
static
void ea386_real_mode_get00 (e8086_t *c)
{
	c->ea.is_mem = 1;                                /* Effective address is memory operand */
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.seg = e386_get_ds (c);                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_ebx (c) + e386_get_esi (c); /* Segment offset  : eBX and eSI       */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.seg = e386_get_ds (c) & 0xfffff;                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_ebx (c) + e386_get_esi (c) & 0xfffff; /* Segment offset  : eBX and eSI       */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 1;
	c->delay += 7;
}

/* EA 00: [BX + SI] or [eBX + eSI] */ /* Virtual 8086 Mode */
static
void ea386_virtual_8086_mode_get00 (e8086_t *c)
{
	c->ea.is_mem = 1;                                /* Effective address is memory operand */
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.seg = e386_get_ds (c);                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_ebx (c) + e386_get_esi (c); /* Segment offset  : eBX and eSI       */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.seg = e386_get_ds (c) & 0xfffff;                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_ebx (c) + e386_get_esi (c) & 0xfffff; /* Segment offset  : eBX and eSI       */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 1;
	c->delay += 7;
}

/* EA 00: [BX + SI] or [eBX + eSI] */ /* Protected Mode */
static
void ea386_protected_mode_get00 (e8086_t *c, ds_segment_register_descriptor_t *ds, ldt_local_descriptor_t *ldt, gdt_global_descriptor_t *gdt)
{
	/* New protected mode code: */
	c->ea.is_mem = 1;                                /* Effective address is memory operand */
	segment = e386_get_ds (c);                       /* Segment register: DS register       */
	check_ldt = segment;
	local_descriptor = ldt_get_local_segment_descriptor (ldt, check_ldt);
	check_gdt = segment;
	global_descriptor = gdt_get_global_segment_descriptor (gdt, check_gdt);
	if ((check_ldt != NULL) || (check_gdt != NULL))
	{
		/* ERROR: Segment already exists and assigned */ /* Possibly use actual CPU exception if required */
	}
	else
	{
		if (ds->ds_segment_register_selector_table_indicator (c) == 1) /* Allocate new local descriptor table: */
		{
			ldt_allocate_new_local_segment_descriptor (ldt, limit, segment);
			ldt_descriptor = ldt_get_local_segment_descriptor (ldt, segment);
			offset = e386_get_ebx (c) + e386_get_esi (c); /* Segment offset  : eBX and eSI       */
			ldt_descriptor->ldt_segment_base_address_offset = offset;
			ldt_descriptor->ldt_segment_is_present = ds->ds_segment_register_descriptor_is_present;
			ldt_descriptor->ldt_granularity_bit = ds->ds_segment_register_descriptor_granularity_bit;
			ldt_descriptor->ldt_big = ds->ds_segment_register_descriptor_big;
			ldt_descriptor->ldt_x = ds->ds_segment_register_descriptor_x_;
			ldt_descriptor->ldt_0_ = ds->ds_segment_register_descriptor_0_;
			ldt_descriptor->ldt_segment_available = ds->ds_segment_register_descriptor_available;
			ldt_descriptor->ldt_descriptor_privilege_level = ds->ds_segment_register_descriptor_privilege_level;
			ldt_descriptor->ldt_type_1 = ds->ds_segment_register_descriptor_type_1;
			ldt_descriptor->ldt_type_0 = ds->ds_segment_register_descriptor_type_0;
			ldt_descriptor->ldt_type_c = ds->ds_segment_register_descriptor_type_c;
			ldt_descriptor->ldt_type_e = ds->ds_segment_register_descriptor_type_e;
			ldt_descriptor->ldt_type_r = ds->ds_segment_register_descriptor_type_r;
			ldt_descriptor->ldt_type_w = ds->ds_segment_register_descriptor_type_w;
			ldt_descriptor->ldt_segment_accessed = ds->ds_segment_register_descriptor_accessed;
		}
		else /* Allocate new global descriptor table: */
		{
			gdt_allocate_new_global_segment_descriptor (gdt, limit, segment);
			gdt_descriptor = gdt_get_global_segment_descriptor (gdt, segment);
			offset = e386_get_ebx (c) + e386_get_esi (c); /* Segment offset  : eBX and eSI       */
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_is_present = ds->ds_segment_register_descriptor_is_present;
			gdt_descriptor->gdt_granularity_bit = ds->ds_segment_register_descriptor_granularity_bit;
			gdt_descriptor->gdt_big = ds->ds_segment_register_descriptor_big;
			gdt_descriptor->gdt_x = ds->ds_segment_register_descriptor_x_;
			gdt_descriptor->gdt_0_ = ds->ds_segment_register_descriptor_0_;
			gdt_descriptor->gdt_segment_available = ds->ds_segment_register_descriptor_available;
			gdt_descriptor->gdt_descriptor_privilege_level = ds->ds_segment_register_descriptor_privilege_level;
			gdt_descriptor->gdt_type_1 = ds->ds_segment_register_descriptor_type_1;
			gdt_descriptor->gdt_type_0 = ds->ds_segment_register_descriptor_type_0;
			gdt_descriptor->gdt_type_c = ds->ds_segment_register_descriptor_type_c;
			gdt_descriptor->gdt_type_e = ds->ds_segment_register_descriptor_type_e;
			gdt_descriptor->gdt_type_r = ds->ds_segment_register_descriptor_type_r;
			gdt_descriptor->gdt_type_w = ds->ds_segment_register_descriptor_type_w;
			gdt_descriptor->gdt_segment_accessed = ds->ds_segment_register_descriptor_accessed;
		}
	}
}

/* EA 01: [BX + DI] or [eBX + eDI] */ /* Real Mode */
static
void ea386_real_mode_get01 (e8086_t *c)
{
	c->ea.is_mem = 1;                                /* Effective address is memory operand */
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.seg = e386_get_ds (c);                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_ebx (c) + e386_get_edi (c); /* Segment offset  : eBX and eSI       */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.seg = e386_get_ds (c) & 0xfffff;                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_ebx (c) + e386_get_edi (c) & 0xfffff; /* Segment offset  : eBX and eSI       */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 1;
	c->delay += 8;
}

/* EA 01: [BX + DI] or [eBX + eDI] */ /* Virtual 8086 Mode */
static
void ea386_virtual_8086_mode_get01 (e8086_t *c)
{
	c->ea.is_mem = 1;                                /* Effective address is memory operand */
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.seg = e386_get_ds (c);                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_ebx (c) + e386_get_edi (c); /* Segment offset  : eBX and eSI       */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.seg = e386_get_ds (c) & 0xfffff;                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_ebx (c) + e386_get_edi (c) & 0xfffff; /* Segment offset  : eBX and eSI       */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 1;
	c->delay += 8;
}

/* EA 01: [BX + DI] or [eBX + eDI] */ /* Protected Mode */
static
void ea386_protected_mode_get01 (e8086_t *c, ds_segment_register_descriptor_t *ds, ldt_local_descriptor_t *ldt, gdt_global_descriptor_t *gdt)
{
	/* New protected mode code: */
	c->ea.is_mem = 1;                                /* Effective address is memory operand */
	segment = e386_get_ds (c);                       /* Segment register: DS register       */
	check_ldt = segment;
	local_descriptor = ldt_get_local_segment_descriptor (ldt, check_ldt);
	check_gdt = segment;
	global_descriptor = gdt_get_global_segment_descriptor (gdt, check_gdt);
	if ((check_ldt != NULL) || (check_gdt != NULL))
	{
		/* ERROR: Segment already exists and assigned */ /* Possibly use actual CPU exception if required */
	}
	else
	{
		if (ds->ds_segment_register_selector_table_indicator (c) == 1) /* Allocate new local descriptor table: */
		{
			ldt_allocate_new_local_segment_descriptor (ldt, limit, segment);
			ldt_descriptor = ldt_get_local_segment_descriptor (ldt, segment);
			offset = e386_get_ebx (c) + e386_get_edi (c); /* Segment offset  : eBX and eSI       */
			ldt_descriptor->ldt_segment_base_address_offset = offset;
			ldt_descriptor->ldt_segment_is_present = ds->ds_segment_register_descriptor_is_present;
			ldt_descriptor->ldt_granularity_bit = ds->ds_segment_register_descriptor_granularity_bit;
			ldt_descriptor->ldt_big = ds->ds_segment_register_descriptor_big;
			ldt_descriptor->ldt_x = ds->ds_segment_register_descriptor_x_;
			ldt_descriptor->ldt_0_ = ds->ds_segment_register_descriptor_0_;
			ldt_descriptor->ldt_segment_available = ds->ds_segment_register_descriptor_available;
			ldt_descriptor->ldt_descriptor_privilege_level = ds->ds_segment_register_descriptor_privilege_level;
			ldt_descriptor->ldt_type_1 = ds->ds_segment_register_descriptor_type_1;
			ldt_descriptor->ldt_type_0 = ds->ds_segment_register_descriptor_type_0;
			ldt_descriptor->ldt_type_c = ds->ds_segment_register_descriptor_type_c;
			ldt_descriptor->ldt_type_e = ds->ds_segment_register_descriptor_type_e;
			ldt_descriptor->ldt_type_r = ds->ds_segment_register_descriptor_type_r;
			ldt_descriptor->ldt_type_w = ds->ds_segment_register_descriptor_type_w;
			ldt_descriptor->ldt_segment_accessed = ds->ds_segment_register_descriptor_accessed;
		}
		else /* Allocate new global descriptor table: */
		{
			gdt_allocate_new_global_segment_descriptor (gdt, limit, segment);
			gdt_descriptor = gdt_get_global_segment_descriptor (gdt, segment);
			offset = e386_get_ebx (c) + e386_get_edi (c); /* Segment offset  : eBX and eSI       */
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_is_present = ds->ds_segment_register_descriptor_is_present;
			gdt_descriptor->gdt_granularity_bit = ds->ds_segment_register_descriptor_granularity_bit;
			gdt_descriptor->gdt_big = ds->ds_segment_register_descriptor_big;
			gdt_descriptor->gdt_x = ds->ds_segment_register_descriptor_x_;
			gdt_descriptor->gdt_0_ = ds->ds_segment_register_descriptor_0_;
			gdt_descriptor->gdt_segment_available = ds->ds_segment_register_descriptor_available;
			gdt_descriptor->gdt_descriptor_privilege_level = ds->ds_segment_register_descriptor_privilege_level;
			gdt_descriptor->gdt_type_1 = ds->ds_segment_register_descriptor_type_1;
			gdt_descriptor->gdt_type_0 = ds->ds_segment_register_descriptor_type_0;
			gdt_descriptor->gdt_type_c = ds->ds_segment_register_descriptor_type_c;
			gdt_descriptor->gdt_type_e = ds->ds_segment_register_descriptor_type_e;
			gdt_descriptor->gdt_type_r = ds->ds_segment_register_descriptor_type_r;
			gdt_descriptor->gdt_type_w = ds->ds_segment_register_descriptor_type_w;
			gdt_descriptor->gdt_segment_accessed = ds->ds_segment_register_descriptor_accessed;
		}
	}
}

/* EA 02: [BP + SI] or [eBP + eSI] */ /* Real Mode */
static
void ea386_real_mode_get02 (e8086_t *c)
{
	c->ea.is_mem = 1;                                /* Effective address is memory operand */
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.seg = e386_get_ss (c);                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_ebp (c) + e386_get_esi (c); /* Segment offset  : eBX and eSI       */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.seg = e386_get_ss (c) & 0xfffff;                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_ebp (c) + e386_get_esi (c) & 0xfffff; /* Segment offset  : eBX and eSI       */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 1;
	c->delay +=8;
}

/* EA 02: [BP + SI] or [eBP + eSI] */ /* Virtual 8086 Mode */
static
void ea386_virtual_8086_mode_get02 (e8086_t *c)
{
	c->ea.is_mem = 1;                                /* Effective address is memory operand */
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.seg = e386_get_ss (c);                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_ebp (c) + e386_get_esi (c); /* Segment offset  : eBX and eSI       */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.seg = e386_get_ss (c) & 0xfffff;                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_ebp (c) + e386_get_esi (c) & 0xfffff; /* Segment offset  : eBX and eSI       */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 1;
	c->delay +=8;
}

/* EA 02: [BP + SI] or [eBP + eSI] */ /* Protected Mode */
static
void ea386_protected_mode_get02 (e8086_t *c, ss_segment_register_descriptor_t *ss, ldt_local_descriptor_t *ldt, gdt_global_descriptor_t *gdt)
{
	/* New protected mode code: */
	c->ea.is_mem = 1;                                /* Effective address is memory operand */
	segment = e386_get_ss (c);                       /* Segment register: DS register       */
	check_ldt = segment;
	local_descriptor = ldt_get_local_segment_descriptor (ldt, check_ldt);
	check_gdt = segment;
	global_descriptor = gdt_get_global_segment_descriptor (gdt, check_gdt);
	if ((check_ldt != NULL) || (check_gdt != NULL))
	{
		/* ERROR: Segment already exists and assigned */ /* Possibly use actual CPU exception if required */
	}
	else
	{
		if (ss->ss_segment_register_selector_table_indicator (c) == 1) /* Allocate new local descriptor table: */
		{
			ldt_allocate_new_local_segment_descriptor (ldt, limit, segment);
			ldt_descriptor = ldt_get_local_segment_descriptor (ldt, segment);
			offset = e386_get_ebp (c) + e386_get_esi (c); /* Segment offset  : eBX and eSI       */
			ldt_descriptor->ldt_segment_base_address_offset = offset;
			ldt_descriptor->ldt_segment_is_present = ss->ss_segment_register_descriptor_is_present;
			ldt_descriptor->ldt_granularity_bit = ss->ss_segment_register_descriptor_granularity_bit;
			ldt_descriptor->ldt_big = ss->ss_segment_register_descriptor_big;
			ldt_descriptor->ldt_x = ss->ss_segment_register_descriptor_x_;
			ldt_descriptor->ldt_0_ = ss->ss_segment_register_descriptor_0_;
			ldt_descriptor->ldt_segment_available = ss->ss_segment_register_descriptor_available;
			ldt_descriptor->ldt_descriptor_privilege_level = ss->ss_segment_register_descriptor_privilege_level;
			ldt_descriptor->ldt_type_1 = ss->ss_segment_register_descriptor_type_1;
			ldt_descriptor->ldt_type_0 = ss->ss_segment_register_descriptor_type_0;
			ldt_descriptor->ldt_type_c = ss->ss_segment_register_descriptor_type_c;
			ldt_descriptor->ldt_type_e = ss->ss_segment_register_descriptor_type_e;
			ldt_descriptor->ldt_type_r = ss->ss_segment_register_descriptor_type_r;
			ldt_descriptor->ldt_type_w = ss->ss_segment_register_descriptor_type_w;
			ldt_descriptor->ldt_segment_accessed = ss->ss_segment_register_descriptor_accessed;
		}
		else /* Allocate new global descriptor table: */
		{
			gdt_allocate_new_global_segment_descriptor (gdt, limit, segment);
			gdt_descriptor = gdt_get_global_segment_descriptor (gdt, segment);
			offset = e386_get_ebp (c) + e386_get_esi (c); /* Segment offset  : eBX and eSI       */
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_is_present = ss->ss_segment_register_descriptor_is_present;
			gdt_descriptor->gdt_granularity_bit = ss->ss_segment_register_descriptor_granularity_bit;
			gdt_descriptor->gdt_big = ss->ss_segment_register_descriptor_big;
			gdt_descriptor->gdt_x = ss->ss_segment_register_descriptor_x_;
			gdt_descriptor->gdt_0_ = ss->ss_segment_register_descriptor_0_;
			gdt_descriptor->gdt_segment_available = ss->ss_segment_register_descriptor_available;
			gdt_descriptor->gdt_descriptor_privilege_level = ss->ss_segment_register_descriptor_privilege_level;
			gdt_descriptor->gdt_type_1 = ss->ss_segment_register_descriptor_type_1;
			gdt_descriptor->gdt_type_0 = ss->ss_segment_register_descriptor_type_0;
			gdt_descriptor->gdt_type_c = ss->ss_segment_register_descriptor_type_c;
			gdt_descriptor->gdt_type_e = ss->ss_segment_register_descriptor_type_e;
			gdt_descriptor->gdt_type_r = ss->ss_segment_register_descriptor_type_r;
			gdt_descriptor->gdt_type_w = ss->ss_segment_register_descriptor_type_w;
			gdt_descriptor->gdt_segment_accessed = ss->ss_segment_register_descriptor_accessed;
		}
	}
}

/* EA 03: [BP + DI] or [eBP + eDI] */ /* Real Mode */
static
void ea386_real_mode_get03 (e8086_t *c)
{
	c->ea.is_mem = 1;                                /* Effective address is memory operand */
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.seg = e386_get_ss (c);                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_ebp (c) + e386_get_edi (c); /* Segment offset  : eBX and eSI       */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.seg = e386_get_ss (c) & 0xfffff;                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_ebp (c) + e386_get_edi (c) & 0xfffff; /* Segment offset  : eBX and eSI       */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 1;
	c->delay += 7;
}

/* EA 03: [BP + DI] or [eBP + eDI] */ /* Virtual 8086 Mode */
static
void ea386_virtual_8086_mode_get03 (e8086_t *c)
{
	c->ea.is_mem = 1;                                /* Effective address is memory operand */
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.seg = e386_get_ss (c);                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_ebp (c) + e386_get_edi (c); /* Segment offset  : eBX and eSI       */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.seg = e386_get_ss (c) & 0xfffff;                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_ebp (c) + e386_get_edi (c) & 0xfffff; /* Segment offset  : eBX and eSI       */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 1;
	c->delay += 7;
}

/* EA 03: [BP + DI] or [eBP + eDI] */ /* Protected Mode */
static
void ea386_protected_mode_get03 (e8086_t *c, ss_segment_register_descriptor_t *ss, ldt_local_descriptor_t *ldt, gdt_global_descriptor_t *gdt)
{
	/* New protected mode code: */
	c->ea.is_mem = 1;                                /* Effective address is memory operand */
	segment = e386_get_ss (c);                       /* Segment register: DS register       */
	check_ldt = segment;
	local_descriptor = ldt_get_local_segment_descriptor (ldt, check_ldt);
	check_gdt = segment;
	global_descriptor = gdt_get_global_segment_descriptor (gdt, check_gdt);
	if ((check_ldt != NULL) || (check_gdt != NULL))
	{
		/* ERROR: Segment already exists and assigned */ /* Possibly use actual CPU exception if required */
	}
	else
	{
		if (ss->ss_segment_register_selector_table_indicator (c) == 1) /* Allocate new local descriptor table: */
		{
			ldt_allocate_new_local_segment_descriptor (ldt, limit, segment);
			ldt_descriptor = ldt_get_local_segment_descriptor (ldt, segment);
			offset = e386_get_ebp (c) + e386_get_edi (c); /* Segment offset  : eBX and eSI       */
			ldt_descriptor->ldt_segment_base_address_offset = offset;
			ldt_descriptor->ldt_segment_is_present = ss->ss_segment_register_descriptor_is_present;
			ldt_descriptor->ldt_granularity_bit = ss->ss_segment_register_descriptor_granularity_bit;
			ldt_descriptor->ldt_big = ss->ss_segment_register_descriptor_big;
			ldt_descriptor->ldt_x = ss->ss_segment_register_descriptor_x_;
			ldt_descriptor->ldt_0_ = ss->ss_segment_register_descriptor_0_;
			ldt_descriptor->ldt_segment_available = ss->ss_segment_register_descriptor_available;
			ldt_descriptor->ldt_descriptor_privilege_level = ss->ss_segment_register_descriptor_privilege_level;
			ldt_descriptor->ldt_type_1 = ss->ss_segment_register_descriptor_type_1;
			ldt_descriptor->ldt_type_0 = ss->ss_segment_register_descriptor_type_0;
			ldt_descriptor->ldt_type_c = ss->ss_segment_register_descriptor_type_c;
			ldt_descriptor->ldt_type_e = ss->ss_segment_register_descriptor_type_e;
			ldt_descriptor->ldt_type_r = ss->ss_segment_register_descriptor_type_r;
			ldt_descriptor->ldt_type_w = ss->ss_segment_register_descriptor_type_w;
			ldt_descriptor->ldt_segment_accessed = ss->ss_segment_register_descriptor_accessed;
		}
		else /* Allocate new global descriptor table: */
		{
			gdt_allocate_new_global_segment_descriptor (gdt, limit, segment);
			gdt_descriptor = gdt_get_global_segment_descriptor (gdt, segment);
			offset = e386_get_ebp (c) + e386_get_edi (c); /* Segment offset  : eBX and eSI       */
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_is_present = ss->ss_segment_register_descriptor_is_present;
			gdt_descriptor->gdt_granularity_bit = ss->ss_segment_register_descriptor_granularity_bit;
			gdt_descriptor->gdt_big = ss->ss_segment_register_descriptor_big;
			gdt_descriptor->gdt_x = ss->ss_segment_register_descriptor_x_;
			gdt_descriptor->gdt_0_ = ss->ss_segment_register_descriptor_0_;
			gdt_descriptor->gdt_segment_available = ss->ss_segment_register_descriptor_available;
			gdt_descriptor->gdt_descriptor_privilege_level = ss->ss_segment_register_descriptor_privilege_level;
			gdt_descriptor->gdt_type_1 = ss->ss_segment_register_descriptor_type_1;
			gdt_descriptor->gdt_type_0 = ss->ss_segment_register_descriptor_type_0;
			gdt_descriptor->gdt_type_c = ss->ss_segment_register_descriptor_type_c;
			gdt_descriptor->gdt_type_e = ss->ss_segment_register_descriptor_type_e;
			gdt_descriptor->gdt_type_r = ss->ss_segment_register_descriptor_type_r;
			gdt_descriptor->gdt_type_w = ss->ss_segment_register_descriptor_type_w;
			gdt_descriptor->gdt_segment_accessed = ss->ss_segment_register_descriptor_accessed;
		}
	}
}

/* EA 04: [SI] or [eSI] */ /* Real Mode */
static
void ea386_real_mode_get04 (e8086_t *c)
{
	c->ea.is_mem = 1;             /* Effective address is memory operand */
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.seg = e386_get_ds (c);                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_esi (c); /* Segment offset  : eBX and eSI       */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.seg = e386_get_ds (c) & 0xfffff;                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_esi (c) & 0xfffff; /* Segment offset  : eBX and eSI       */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 1;
	c->delay += 5;
}

/* EA 04: [SI] or [eSI] */ /* Virtual 8086 Mode */
static
void ea386_virtual_8086_mode_get04 (e8086_t *c)
{
	c->ea.is_mem = 1;             /* Effective address is memory operand */
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.seg = e386_get_ds (c);                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_esi (c); /* Segment offset  : eBX and eSI       */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.seg = e386_get_ds (c) & 0xfffff;                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_esi (c) & 0xfffff; /* Segment offset  : eBX and eSI       */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 1;
	c->delay += 5;
}

/* EA 04: [SI] or [eSI] */ /* Protected Mode */
static
void ea386_protected_mode_get04 (e8086_t *c, ds_segment_register_descriptor_t *ds, ldt_local_descriptor_t *ldt, gdt_global_descriptor_t *gdt)
{
	/* New protected mode code: */
	c->ea.is_mem = 1;                                /* Effective address is memory operand */
	segment = e386_get_ss (c);                       /* Segment register: DS register       */
	check_ldt = segment;
	local_descriptor = ldt_get_local_segment_descriptor (ldt, check_ldt);
	check_gdt = segment;
	global_descriptor = gdt_get_global_segment_descriptor (gdt, check_gdt);
	if ((check_ldt != NULL) || (check_gdt != NULL))
	{
		/* ERROR: Segment already exists and assigned */ /* Possibly use actual CPU exception if required */
	}
	else
	{
		if (ss->ss_segment_register_selector_table_indicator (c) == 1) /* Allocate new local descriptor table: */
		{
			ldt_allocate_new_local_segment_descriptor (ldt, limit, segment);
			ldt_descriptor = ldt_get_local_segment_descriptor (ldt, segment);
			offset = e386_get_esi (c); /* Segment offset  : eBX and eSI       */
			ldt_descriptor->ldt_segment_base_address_offset = offset;
			ldt_descriptor->ldt_segment_is_present = ds->ds_segment_register_descriptor_is_present;
			ldt_descriptor->ldt_granularity_bit = ds->ds_segment_register_descriptor_granularity_bit;
			ldt_descriptor->ldt_big = ds->ds_segment_register_descriptor_big;
			ldt_descriptor->ldt_x = ds->ds_segment_register_descriptor_x_;
			ldt_descriptor->ldt_0_ = ds->ds_segment_register_descriptor_0_;
			ldt_descriptor->ldt_segment_available = ds->ds_segment_register_descriptor_available;
			ldt_descriptor->ldt_descriptor_privilege_level = ds->ds_segment_register_descriptor_privilege_level;
			ldt_descriptor->ldt_type_1 = ds->ds_segment_register_descriptor_type_1;
			ldt_descriptor->ldt_type_0 = ds->ds_segment_register_descriptor_type_0;
			ldt_descriptor->ldt_type_c = ds->ds_segment_register_descriptor_type_c;
			ldt_descriptor->ldt_type_e = ds->ds_segment_register_descriptor_type_e;
			ldt_descriptor->ldt_type_r = ds->ds_segment_register_descriptor_type_r;
			ldt_descriptor->ldt_type_w = ds->ds_segment_register_descriptor_type_w;
			ldt_descriptor->ldt_segment_accessed = ds->ds_segment_register_descriptor_accessed;
		}
		else /* Allocate new global descriptor table: */
		{
			gdt_allocate_new_global_segment_descriptor (gdt, limit, segment);
			gdt_descriptor = gdt_get_global_segment_descriptor (gdt, segment);
			offset = e386_get_esi (c); /* Segment offset  : eBX and eSI       */
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_is_present = ds->ds_segment_register_descriptor_is_present;
			gdt_descriptor->gdt_granularity_bit = ds->ds_segment_register_descriptor_granularity_bit;
			gdt_descriptor->gdt_big = ds->ds_segment_register_descriptor_big;
			gdt_descriptor->gdt_x = ds->ds_segment_register_descriptor_x_;
			gdt_descriptor->gdt_0_ = ds->ds_segment_register_descriptor_0_;
			gdt_descriptor->gdt_segment_available = ds->ds_segment_register_descriptor_available;
			gdt_descriptor->gdt_descriptor_privilege_level = ds->ds_segment_register_descriptor_privilege_level;
			gdt_descriptor->gdt_type_1 = ds->ds_segment_register_descriptor_type_1;
			gdt_descriptor->gdt_type_0 = ds->ds_segment_register_descriptor_type_0;
			gdt_descriptor->gdt_type_c = ds->ds_segment_register_descriptor_type_c;
			gdt_descriptor->gdt_type_e = ds->ds_segment_register_descriptor_type_e;
			gdt_descriptor->gdt_type_r = ds->ds_segment_register_descriptor_type_r;
			gdt_descriptor->gdt_type_w = ds->ds_segment_register_descriptor_type_w;
			gdt_descriptor->gdt_segment_accessed = ds->ds_segment_register_descriptor_accessed;
		}
	}
}

/* EA 05: [DI] or [eDI] */ /* Real Mode */
static
void ea386_real_mode_get05 (e8086_t *c)
{
	c->ea.is_mem = 1;             /* Effective address is memory operand */
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.seg = e386_get_ds (c);                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_edi (c); /* Segment offset  : eBX and eSI       */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.seg = e386_get_ds (c) & 0xfffff;                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_edi (c) & 0xfffff; /* Segment offset  : eBX and eSI       */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 1;
	c->delay += 5;
}

/* EA 05: [DI] or [eDI] */ /* Virtual 8086 Mode */
static
void ea386_virtual_8086_mode_get05 (e8086_t *c)
{
	c->ea.is_mem = 1;             /* Effective address is memory operand */
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.seg = e386_get_ds (c);                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_edi (c); /* Segment offset  : eBX and eSI       */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.seg = e386_get_ds (c) & 0xfffff;                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_edi (c) & 0xfffff; /* Segment offset  : eBX and eSI       */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 1;
	c->delay += 5;
}

/* EA 05: [DI] or [eDI] */ /* Protected Mode */
static
void ea386_protected_mode_get05 (e8086_t *c, ds_segment_register_descriptor_t *ds, ldt_local_descriptor_t *ldt, gdt_global_descriptor_t *gdt)
{
	/* New protected mode code: */
	c->ea.is_mem = 1;                                /* Effective address is memory operand */
	segment = e386_get_ss (c);                       /* Segment register: DS register       */
	check_ldt = segment;
	local_descriptor = ldt_get_local_segment_descriptor (ldt, check_ldt);
	check_gdt = segment;
	global_descriptor = gdt_get_global_segment_descriptor (gdt, check_gdt);
	if ((check_ldt != NULL) || (check_gdt != NULL))
	{
		/* ERROR: Segment already exists and assigned */ /* Possibly use actual CPU exception if required */
	}
	else
	{
		if (ds->ds_segment_register_selector_table_indicator (c) == 1) /* Allocate new local descriptor table: */
		{
			ldt_allocate_new_local_segment_descriptor (ldt, limit, segment);
			ldt_descriptor = ldt_get_local_segment_descriptor (ldt, segment);
			offset = e386_get_edi (c); /* Segment offset  : eBX and eSI       */
			ldt_descriptor->ldt_segment_base_address_offset = offset;
			ldt_descriptor->ldt_segment_is_present = ds->ds_segment_register_descriptor_is_present;
			ldt_descriptor->ldt_granularity_bit = ds->ds_segment_register_descriptor_granularity_bit;
			ldt_descriptor->ldt_big = ds->ds_segment_register_descriptor_big;
			ldt_descriptor->ldt_x = ds->ds_segment_register_descriptor_x_;
			ldt_descriptor->ldt_0_ = ds->ds_segment_register_descriptor_0_;
			ldt_descriptor->ldt_segment_available = ds->ds_segment_register_descriptor_available;
			ldt_descriptor->ldt_descriptor_privilege_level = ds->ds_segment_register_descriptor_privilege_level;
			ldt_descriptor->ldt_type_1 = ds->ds_segment_register_descriptor_type_1;
			ldt_descriptor->ldt_type_0 = ds->ds_segment_register_descriptor_type_0;
			ldt_descriptor->ldt_type_c = ds->ds_segment_register_descriptor_type_c;
			ldt_descriptor->ldt_type_e = ds->ds_segment_register_descriptor_type_e;
			ldt_descriptor->ldt_type_r = ds->ds_segment_register_descriptor_type_r;
			ldt_descriptor->ldt_type_w = ds->ds_segment_register_descriptor_type_w;
			ldt_descriptor->ldt_segment_accessed = ds->ds_segment_register_descriptor_accessed;
		}
		else /* Allocate new global descriptor table: */
		{
			gdt_allocate_new_global_segment_descriptor (gdt, limit, segment);
			gdt_descriptor = gdt_get_global_segment_descriptor (gdt, segment);
			offset = e386_get_edi (c); /* Segment offset  : eBX and eSI       */
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_is_present = ds->ds_segment_register_descriptor_is_present;
			gdt_descriptor->gdt_granularity_bit = ds->ds_segment_register_descriptor_granularity_bit;
			gdt_descriptor->gdt_big = ds->ds_segment_register_descriptor_big;
			gdt_descriptor->gdt_x = ds->ds_segment_register_descriptor_x_;
			gdt_descriptor->gdt_0_ = ds->ds_segment_register_descriptor_0_;
			gdt_descriptor->gdt_segment_available = ds->ds_segment_register_descriptor_available;
			gdt_descriptor->gdt_descriptor_privilege_level = ds->ds_segment_register_descriptor_privilege_level;
			gdt_descriptor->gdt_type_1 = ds->ds_segment_register_descriptor_type_1;
			gdt_descriptor->gdt_type_0 = ds->ds_segment_register_descriptor_type_0;
			gdt_descriptor->gdt_type_c = ds->ds_segment_register_descriptor_type_c;
			gdt_descriptor->gdt_type_e = ds->ds_segment_register_descriptor_type_e;
			gdt_descriptor->gdt_type_r = ds->ds_segment_register_descriptor_type_r;
			gdt_descriptor->gdt_type_w = ds->ds_segment_register_descriptor_type_w;
			gdt_descriptor->gdt_segment_accessed = ds->ds_segment_register_descriptor_accessed;
		}
	}
}

/* EA 06: [XXXX] */ /* Real Mode */
static
void ea386_real_mode_get06 (e8086_t *c)
{
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.seg = e386_get_ds (c);                     /* Segment register: DS register       */
		c->ea.ofs = e386_mk_uint32 (c->ea.data[1], c->ea.data[2]); /* Segment offset  : 32-bit unsigned integer */
		c->ea.seg = e386_get_ds (c);                               /* Segment register: DS register             */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.seg = e386_get_ds (c) & 0xfffff;                     /* Segment register: DS register       */
		c->ea.ofs = e386_mk_uint32 (c->ea.data[1], c->ea.data[2]) & 0xfffff; /* Segment offset  : eBX and eSI       */
		c->ea.seg = e386_get_ds (c) & 0xfffff;                     /* Segment register: DS register       */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 3;
	c->delay += 6;
}

/* EA 06: [XXXX] */ /* Virtual 8086 Mode */
static
void ea386_virtual_8086_mode_get06 (e8086_t *c)
{
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.seg = e386_get_ds (c);                     /* Segment register: DS register       */
		c->ea.ofs = e386_mk_uint32 (c->ea.data[1], c->ea.data[2]); /* Segment offset  : 32-bit unsigned integer */
		c->ea.seg = e386_get_ds (c);                               /* Segment register: DS register             */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.seg = e386_get_ds (c) & 0xfffff;                     /* Segment register: DS register       */
		c->ea.ofs = e386_mk_uint32 (c->ea.data[1], c->ea.data[2]) & 0xfffff; /* Segment offset  : eBX and eSI       */
		c->ea.seg = e386_get_ds (c) & 0xfffff;                     /* Segment register: DS register       */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 3;
	c->delay += 6;
}

/* EA 06: [XXXX] */ /* Protected Mode */
static
void ea386_protected_mode_get06 (e8086_t *c, ds_segment_register_descriptor_t *ds, ldt_local_descriptor_t *ldt, gdt_global_descriptor_t *gdt)
{
	/* New protected mode code: */
	c->ea.is_mem = 1;                                /* Effective address is memory operand */
	segment = e386_get_ds (c);                       /* Segment register: DS register       */
	check_ldt = segment;
	local_descriptor = ldt_get_local_segment_descriptor (ldt, check_ldt);
	check_gdt = segment;
	global_descriptor = gdt_get_global_segment_descriptor (gdt, check_gdt);
	if ((check_ldt != NULL) || (check_gdt != NULL))
	{
		/* ERROR: Segment already exists and assigned */ /* Possibly use actual CPU exception if required */
	}
	else
	{
		if (ds->ds_segment_register_selector_table_indicator (c) == 1) /* Allocate new local descriptor table: */
		{
			ldt_allocate_new_local_segment_descriptor (ldt, limit, segment);
			ldt_descriptor = ldt_get_local_segment_descriptor (ldt, segment);
			offset = e386_mk_uint32 (c->ea.data[1], c->ea.data[2]); /* Segment offset  : eBX and eSI       */
			ldt_descriptor->ldt_segment_base_address_offset = offset;
			ldt_descriptor->ldt_segment_is_present = ds->ds_segment_register_descriptor_is_present;
			ldt_descriptor->ldt_granularity_bit = ds->ds_segment_register_descriptor_granularity_bit;
			ldt_descriptor->ldt_big = ds->ds_segment_register_descriptor_big;
			ldt_descriptor->ldt_x = ds->ds_segment_register_descriptor_x_;
			ldt_descriptor->ldt_0_ = ds->ds_segment_register_descriptor_0_;
			ldt_descriptor->ldt_segment_available = ds->ds_segment_register_descriptor_available;
			ldt_descriptor->ldt_descriptor_privilege_level = ds->ds_segment_register_descriptor_privilege_level;
			ldt_descriptor->ldt_type_1 = ds->ds_segment_register_descriptor_type_1;
			ldt_descriptor->ldt_type_0 = ds->ds_segment_register_descriptor_type_0;
			ldt_descriptor->ldt_type_c = ds->ds_segment_register_descriptor_type_c;
			ldt_descriptor->ldt_type_e = ds->ds_segment_register_descriptor_type_e;
			ldt_descriptor->ldt_type_r = ds->ds_segment_register_descriptor_type_r;
			ldt_descriptor->ldt_type_w = ds->ds_segment_register_descriptor_type_w;
			ldt_descriptor->ldt_segment_accessed = ds->ds_segment_register_descriptor_accessed;
		}
		else /* Allocate new global descriptor table: */
		{
			gdt_allocate_new_global_segment_descriptor (gdt, limit, segment);
			gdt_descriptor = gdt_get_global_segment_descriptor (gdt, segment);
			offset = e386_mk_uint32 (c->ea.data[1], c->ea.data[2]); /* Segment offset  : eBX and eSI       */
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_is_present = ds->ds_segment_register_descriptor_is_present;
			gdt_descriptor->gdt_granularity_bit = ds->ds_segment_register_descriptor_granularity_bit;
			gdt_descriptor->gdt_big = ds->ds_segment_register_descriptor_big;
			gdt_descriptor->gdt_x = ds->ds_segment_register_descriptor_x_;
			gdt_descriptor->gdt_0_ = ds->ds_segment_register_descriptor_0_;
			gdt_descriptor->gdt_segment_available = ds->ds_segment_register_descriptor_available;
			gdt_descriptor->gdt_descriptor_privilege_level = ds->ds_segment_register_descriptor_privilege_level;
			gdt_descriptor->gdt_type_1 = ds->ds_segment_register_descriptor_type_1;
			gdt_descriptor->gdt_type_0 = ds->ds_segment_register_descriptor_type_0;
			gdt_descriptor->gdt_type_c = ds->ds_segment_register_descriptor_type_c;
			gdt_descriptor->gdt_type_e = ds->ds_segment_register_descriptor_type_e;
			gdt_descriptor->gdt_type_r = ds->ds_segment_register_descriptor_type_r;
			gdt_descriptor->gdt_type_w = ds->ds_segment_register_descriptor_type_w;
			gdt_descriptor->gdt_segment_accessed = ds->ds_segment_register_descriptor_accessed;
		}
	}
}

/* EA 07: [BX] or [eBX] */ /* Real Mode */
static
void ea386_real_mode_get07 (e8086_t *c)
{
	c->ea.is_mem = 1;             /* Effective address is memory operand */
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.seg = e386_get_ds (c);                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_ebx (c) & 0xfffff; /* Segment offset  : eBX and eSI       */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.seg = e386_get_ds (c) & 0xfffff;                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_ebx (c) & 0xfffff; /* Segment offset  : eBX and eSI       */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 1;
	c->delay += 5;
}

/* EA 07: [BX] or [eBX] */ /* Virtual 8086 Mode */
static
void ea386_virtual_8086_mode_get07 (e8086_t *c)
{
	c->ea.is_mem = 1;             /* Effective address is memory operand */
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.seg = e386_get_ds (c);                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_ebx (c) & 0xfffff; /* Segment offset  : eBX and eSI       */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.seg = e386_get_ds (c) & 0xfffff;                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_ebx (c) & 0xfffff; /* Segment offset  : eBX and eSI       */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 1;
	c->delay += 5;
}

/* EA 07: [BX] or [eBX] */ /* Protected Mode */
static
void ea386_protected_mode_get07 (e8086_t *c, ds_segment_register_descriptor_t *ds, ldt_local_descriptor_t *ldt, gdt_global_descriptor_t *gdt)
{
	/* New protected mode code: */
	c->ea.is_mem = 1;                                /* Effective address is memory operand */
	segment = e386_get_ds (c);                       /* Segment register: DS register       */
	check_ldt = segment;
	local_descriptor = ldt_get_local_segment_descriptor (ldt, check_ldt);
	check_gdt = segment;
	global_descriptor = gdt_get_global_segment_descriptor (gdt, check_gdt);
	if ((check_ldt != NULL) || (check_gdt != NULL))
	{
		/* ERROR: Segment already exists and assigned */ /* Possibly use actual CPU exception if required */
	}
	else
	{
		if (ds->ds_segment_register_selector_table_indicator (c) == 1) /* Allocate new local descriptor table: */
		{
			ldt_allocate_new_local_segment_descriptor (ldt, limit, segment);
			ldt_descriptor = ldt_get_local_segment_descriptor (ldt, segment);
			offset = e386_get_ebx (c); /* Segment offset  : eBX and eSI       */
			ldt_descriptor->ldt_segment_base_address_offset = offset;
			ldt_descriptor->ldt_segment_is_present = ds->ds_segment_register_descriptor_is_present;
			ldt_descriptor->ldt_granularity_bit = ds->ds_segment_register_descriptor_granularity_bit;
			ldt_descriptor->ldt_big = ds->ds_segment_register_descriptor_big;
			ldt_descriptor->ldt_x = ds->ds_segment_register_descriptor_x_;
			ldt_descriptor->ldt_0_ = ds->ds_segment_register_descriptor_0_;
			ldt_descriptor->ldt_segment_available = ds->ds_segment_register_descriptor_available;
			ldt_descriptor->ldt_descriptor_privilege_level = ds->ds_segment_register_descriptor_privilege_level;
			ldt_descriptor->ldt_type_1 = ds->ds_segment_register_descriptor_type_1;
			ldt_descriptor->ldt_type_0 = ds->ds_segment_register_descriptor_type_0;
			ldt_descriptor->ldt_type_c = ds->ds_segment_register_descriptor_type_c;
			ldt_descriptor->ldt_type_e = ds->ds_segment_register_descriptor_type_e;
			ldt_descriptor->ldt_type_r = ds->ds_segment_register_descriptor_type_r;
			ldt_descriptor->ldt_type_w = ds->ds_segment_register_descriptor_type_w;
			ldt_descriptor->ldt_segment_accessed = ds->ds_segment_register_descriptor_accessed;
		}
		else /* Allocate new global descriptor table: */
		{
			gdt_allocate_new_global_segment_descriptor (gdt, limit, segment);
			gdt_descriptor = gdt_get_global_segment_descriptor (gdt, segment);
			offset = e386_get_ebx (c); /* Segment offset  : eBX and eSI       */
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_is_present = ds->ds_segment_register_descriptor_is_present;
			gdt_descriptor->gdt_granularity_bit = ds->ds_segment_register_descriptor_granularity_bit;
			gdt_descriptor->gdt_big = ds->ds_segment_register_descriptor_big;
			gdt_descriptor->gdt_x = ds->ds_segment_register_descriptor_x_;
			gdt_descriptor->gdt_0_ = ds->ds_segment_register_descriptor_0_;
			gdt_descriptor->gdt_segment_available = ds->ds_segment_register_descriptor_available;
			gdt_descriptor->gdt_descriptor_privilege_level = ds->ds_segment_register_descriptor_privilege_level;
			gdt_descriptor->gdt_type_1 = ds->ds_segment_register_descriptor_type_1;
			gdt_descriptor->gdt_type_0 = ds->ds_segment_register_descriptor_type_0;
			gdt_descriptor->gdt_type_c = ds->ds_segment_register_descriptor_type_c;
			gdt_descriptor->gdt_type_e = ds->ds_segment_register_descriptor_type_e;
			gdt_descriptor->gdt_type_r = ds->ds_segment_register_descriptor_type_r;
			gdt_descriptor->gdt_type_w = ds->ds_segment_register_descriptor_type_w;
			gdt_descriptor->gdt_segment_accessed = ds->ds_segment_register_descriptor_accessed;
		}
	}
}

/* EA 08: [BX + SI + XX] or [eBX + eSI + XX] */ /* Real Mode */
static
void ea386_real_mode_get08 (e8086_t *c)
{
	c->ea.is_mem = 1;                                                                 /* Effective address is memory operand                */
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.seg = e386_get_ds (c);                                                      /* Segment register: DS register                      */
		c->ea.ofs = e386_get_ebx (c) + e386_get_esi (c) + e386_mk_sint32 (c->ea.data[1]); /* Segment offset  : eBX and eSI combined with 32-bit */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.seg = e386_get_ds (c) & 0xfffff;                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_ebx (c) + e386_get_esi (c) + e386_mk_sint32 (c->ea.data[1]) & 0xfffff; /* Segment offset  : eBX and eSI       */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 2;                                                                    /* unsigned integer                                   */
	c->delay += 11;
}

/* EA 08: [BX + SI + XX] or [eBX + eSI + XX] */ /* Virtual 8086 Mode */
static
void ea386_virtual_8086_mode_get08 (e8086_t *c)
{
	c->ea.is_mem = 1;                                                                 /* Effective address is memory operand                */
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.seg = e386_get_ds (c);                                                      /* Segment register: DS register                      */
		c->ea.ofs = e386_get_ebx (c) + e386_get_esi (c) + e386_mk_sint32 (c->ea.data[1]); /* Segment offset  : eBX and eSI combined with 32-bit */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.seg = e386_get_ds (c) & 0xfffff;                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_ebx (c) + e386_get_esi (c) + e386_mk_sint32 (c->ea.data[1]) & 0xfffff; /* Segment offset  : eBX and eSI       */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 2;                                                                    /* unsigned integer                                   */
	c->delay += 11;
}

/* EA 08: [BX + SI + XX] or [eBX + eSI + XX] */ /* Protected Mode */
static
void ea386_protected_mode_get08 (e8086_t *c, ds_segment_register_descriptor_t *ds, ldt_local_descriptor_t *ldt, gdt_global_descriptor_t *gdt)
{
	/* New protected mode code: */
	c->ea.is_mem = 1;                                /* Effective address is memory operand */
	segment = e386_get_ds (c);                       /* Segment register: DS register       */
	check_ldt = segment;
	local_descriptor = ldt_get_local_segment_descriptor (ldt, check_ldt);
	check_gdt = segment;
	global_descriptor = gdt_get_global_segment_descriptor (gdt, check_gdt);
	if ((check_ldt != NULL) || (check_gdt != NULL))
	{
		/* ERROR: Segment already exists and assigned */ /* Possibly use actual CPU exception if required */
	}
	else
	{
		if (ds->ds_segment_register_selector_table_indicator (c) == 1) /* Allocate new local descriptor table: */
		{
			ldt_allocate_new_local_segment_descriptor (ldt, limit, segment);
			ldt_descriptor = ldt_get_local_segment_descriptor (ldt, segment);
			offset = e386_get_ebx (c) + e386_get_esi (c) + e386_mk_sint32 (c->ea.data[1]); /* Segment offset  : eBX and eSI       */
			ldt_descriptor->ldt_segment_base_address_offset = offset;
			ldt_descriptor->ldt_segment_is_present = ds->ds_segment_register_descriptor_is_present;
			ldt_descriptor->ldt_granularity_bit = ds->ds_segment_register_descriptor_granularity_bit;
			ldt_descriptor->ldt_big = ds->ds_segment_register_descriptor_big;
			ldt_descriptor->ldt_x = ds->ds_segment_register_descriptor_x_;
			ldt_descriptor->ldt_0_ = ds->ds_segment_register_descriptor_0_;
			ldt_descriptor->ldt_segment_available = ds->ds_segment_register_descriptor_available;
			ldt_descriptor->ldt_descriptor_privilege_level = ds->ds_segment_register_descriptor_privilege_level;
			ldt_descriptor->ldt_type_1 = ds->ds_segment_register_descriptor_type_1;
			ldt_descriptor->ldt_type_0 = ds->ds_segment_register_descriptor_type_0;
			ldt_descriptor->ldt_type_c = ds->ds_segment_register_descriptor_type_c;
			ldt_descriptor->ldt_type_e = ds->ds_segment_register_descriptor_type_e;
			ldt_descriptor->ldt_type_r = ds->ds_segment_register_descriptor_type_r;
			ldt_descriptor->ldt_type_w = ds->ds_segment_register_descriptor_type_w;
			ldt_descriptor->ldt_segment_accessed = ds->ds_segment_register_descriptor_accessed;
		}
		else /* Allocate new global descriptor table: */
		{
			gdt_allocate_new_global_segment_descriptor (gdt, limit, segment);
			gdt_descriptor = gdt_get_global_segment_descriptor (gdt, segment);
			offset = e386_get_ebx (c) + e386_get_esi (c) + e386_mk_sint32 (c->ea.data[1]); /* Segment offset  : eBX and eSI       */
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_is_present = ds->ds_segment_register_descriptor_is_present;
			gdt_descriptor->gdt_granularity_bit = ds->ds_segment_register_descriptor_granularity_bit;
			gdt_descriptor->gdt_big = ds->ds_segment_register_descriptor_big;
			gdt_descriptor->gdt_x = ds->ds_segment_register_descriptor_x_;
			gdt_descriptor->gdt_0_ = ds->ds_segment_register_descriptor_0_;
			gdt_descriptor->gdt_segment_available = ds->ds_segment_register_descriptor_available;
			gdt_descriptor->gdt_descriptor_privilege_level = ds->ds_segment_register_descriptor_privilege_level;
			gdt_descriptor->gdt_type_1 = ds->ds_segment_register_descriptor_type_1;
			gdt_descriptor->gdt_type_0 = ds->ds_segment_register_descriptor_type_0;
			gdt_descriptor->gdt_type_c = ds->ds_segment_register_descriptor_type_c;
			gdt_descriptor->gdt_type_e = ds->ds_segment_register_descriptor_type_e;
			gdt_descriptor->gdt_type_r = ds->ds_segment_register_descriptor_type_r;
			gdt_descriptor->gdt_type_w = ds->ds_segment_register_descriptor_type_w;
			gdt_descriptor->gdt_segment_accessed = ds->ds_segment_register_descriptor_accessed;
		}
	}
}

/* EA 09: [BX + DI + XX] or [eBX + eDI + XX] */ /* Real Mode */
static
void ea386_real_mode_get09 (e8086_t *c)
{
	c->ea.is_mem = 1;                                                                 /* Effective address is memory operand                */
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.seg = e386_get_ds (c);                                                      /* Segment register: DS register                      */
		c->ea.ofs = e386_get_ebx (c) + e386_get_edi (c) + e386_mk_sint32 (c->ea.data[1]); /* Segment offset  : eBX and eSI combined with 32-bit */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.seg = e386_get_ds (c) & 0xfffff;                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_ebx (c) + e386_get_edi (c) + e386_mk_sint32 (c->ea.data[1]) & 0xfffff; /* Segment offset  : eBX and eSI       */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 2;                                                                    /* unsigned integer                                   */
	c->delay += 12;
}

/* EA 09: [BX + DI + XX] or [eBX + eDI + XX] */ /* Virtual 8086 Mode */
static
void ea386_virtual_8086_mode_get09 (e8086_t *c)
{
	c->ea.is_mem = 1;                                                                 /* Effective address is memory operand                */
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.seg = e386_get_ds (c);                                                      /* Segment register: DS register                      */
		c->ea.ofs = e386_get_ebx (c) + e386_get_edi (c) + e386_mk_sint32 (c->ea.data[1]); /* Segment offset  : eBX and eSI combined with 32-bit */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.seg = e386_get_ds (c) & 0xfffff;                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_ebx (c) + e386_get_edi (c) + e386_mk_sint32 (c->ea.data[1]) & 0xfffff; /* Segment offset  : eBX and eSI       */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 2;                                                                    /* unsigned integer                                   */
	c->delay += 12;
}

/* EA 09: [BX + DI + XX] or [eBX + eDI + XX] */ /* Protected Mode */
static
void ea386_protected_mode_get09 (e8086_t *c, ds_segment_register_descriptor_t *ds, ldt_local_descriptor_t *ldt, gdt_global_descriptor_t *gdt)
{
	/* New protected mode code: */
	c->ea.is_mem = 1;                                /* Effective address is memory operand */
	segment = e386_get_ds (c);                       /* Segment register: DS register       */
	check_ldt = segment;
	local_descriptor = ldt_get_local_segment_descriptor (ldt, check_ldt);
	check_gdt = segment;
	global_descriptor = gdt_get_global_segment_descriptor (gdt, check_gdt);
	if ((check_ldt != NULL) || (check_gdt != NULL))
	{
		/* ERROR: Segment already exists and assigned */ /* Possibly use actual CPU exception if required */
	}
	else
	{
		if (ds->ds_segment_register_selector_table_indicator (c) == 1) /* Allocate new local descriptor table: */
		{
			ldt_allocate_new_local_segment_descriptor (ldt, limit, segment);
			ldt_descriptor = ldt_get_local_segment_descriptor (ldt, segment);
			offset = e386_get_ebx (c) + e386_get_edi (c) + e386_mk_sint32 (c->ea.data[1]); /* Segment offset  : eBX and eSI       */
			ldt_descriptor->ldt_segment_base_address_offset = offset;
			ldt_descriptor->ldt_segment_is_present = ds->ds_segment_register_descriptor_is_present;
			ldt_descriptor->ldt_granularity_bit = ds->ds_segment_register_descriptor_granularity_bit;
			ldt_descriptor->ldt_big = ds->ds_segment_register_descriptor_big;
			ldt_descriptor->ldt_x = ds->ds_segment_register_descriptor_x_;
			ldt_descriptor->ldt_0_ = ds->ds_segment_register_descriptor_0_;
			ldt_descriptor->ldt_segment_available = ds->ds_segment_register_descriptor_available;
			ldt_descriptor->ldt_descriptor_privilege_level = ds->ds_segment_register_descriptor_privilege_level;
			ldt_descriptor->ldt_type_1 = ds->ds_segment_register_descriptor_type_1;
			ldt_descriptor->ldt_type_0 = ds->ds_segment_register_descriptor_type_0;
			ldt_descriptor->ldt_type_c = ds->ds_segment_register_descriptor_type_c;
			ldt_descriptor->ldt_type_e = ds->ds_segment_register_descriptor_type_e;
			ldt_descriptor->ldt_type_r = ds->ds_segment_register_descriptor_type_r;
			ldt_descriptor->ldt_type_w = ds->ds_segment_register_descriptor_type_w;
			ldt_descriptor->ldt_segment_accessed = ds->ds_segment_register_descriptor_accessed;
		}
		else /* Allocate new global descriptor table: */
		{
			gdt_allocate_new_global_segment_descriptor (gdt, limit, segment);
			gdt_descriptor = gdt_get_global_segment_descriptor (gdt, segment);
			offset = e386_get_ebx (c) + e386_get_edi (c) + e386_mk_sint32 (c->ea.data[1]); /* Segment offset  : eBX and eSI       */
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_is_present = ds->ds_segment_register_descriptor_is_present;
			gdt_descriptor->gdt_granularity_bit = ds->ds_segment_register_descriptor_granularity_bit;
			gdt_descriptor->gdt_big = ds->ds_segment_register_descriptor_big;
			gdt_descriptor->gdt_x = ds->ds_segment_register_descriptor_x_;
			gdt_descriptor->gdt_0_ = ds->ds_segment_register_descriptor_0_;
			gdt_descriptor->gdt_segment_available = ds->ds_segment_register_descriptor_available;
			gdt_descriptor->gdt_descriptor_privilege_level = ds->ds_segment_register_descriptor_privilege_level;
			gdt_descriptor->gdt_type_1 = ds->ds_segment_register_descriptor_type_1;
			gdt_descriptor->gdt_type_0 = ds->ds_segment_register_descriptor_type_0;
			gdt_descriptor->gdt_type_c = ds->ds_segment_register_descriptor_type_c;
			gdt_descriptor->gdt_type_e = ds->ds_segment_register_descriptor_type_e;
			gdt_descriptor->gdt_type_r = ds->ds_segment_register_descriptor_type_r;
			gdt_descriptor->gdt_type_w = ds->ds_segment_register_descriptor_type_w;
			gdt_descriptor->gdt_segment_accessed = ds->ds_segment_register_descriptor_accessed;
		}
	}
}

/* EA 0A: [BP + SI + XX] or [eBP + eSI + XX] */ /* Real Mode */
static
void ea386_real_mode_get0a (e8086_t *c)
{
	c->ea.is_mem = 1;                                                                 /* Effective address is memory operand                */
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.seg = e386_get_ss (c);                                                      /* Segment register: DS register                      */
		c->ea.ofs = e386_get_ebp (c) + e386_get_esi (c) + e386_mk_sint32 (c->ea.data[1]); /* Segment offset  : eBX and eSI combined with 32-bit */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.seg = e386_get_ss (c) & 0xfffff;                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_ebp (c) + e386_get_esi (c) + e386_mk_sint32 (c->ea.data[1]) & 0xfffff; /* Segment offset  : eBX and eSI       */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 2;                                                                    /* unsigned integer                                   */
	c->delay += 12;
}

/* EA 0A: [BP + SI + XX] or [eBP + eSI + XX] */ /* Virtual 8086 Mode */
static
void ea386_virtual_8086_mode_get0a (e8086_t *c)
{
	c->ea.is_mem = 1;                                                                 /* Effective address is memory operand                */
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.seg = e386_get_ss (c);                                                      /* Segment register: DS register                      */
		c->ea.ofs = e386_get_ebp (c) + e386_get_esi (c) + e386_mk_sint32 (c->ea.data[1]); /* Segment offset  : eBX and eSI combined with 32-bit */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.seg = e386_get_ss (c) & 0xfffff;                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_ebp (c) + e386_get_esi (c) + e386_mk_sint32 (c->ea.data[1]) & 0xfffff; /* Segment offset  : eBX and eSI       */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 2;                                                                    /* unsigned integer                                   */
	c->delay += 12;
}

/* EA 0A: [BP + SI + XX] or [eBP + eSI + XX] */ /* Protected Mode */
static
void ea386_protected_mode_get0a (e8086_t *c, ss_segment_register_descriptor_t *ss, ldt_local_descriptor_t *ldt, gdt_global_descriptor_t *gdt)
{
	/* New protected mode code: */
	c->ea.is_mem = 1;                                /* Effective address is memory operand */
	segment = e386_get_ss (c);                       /* Segment register: DS register       */
	check_ldt = segment;
	local_descriptor = ldt_get_local_segment_descriptor (ldt, check_ldt);
	check_gdt = segment;
	global_descriptor = gdt_get_global_segment_descriptor (gdt, check_gdt);
	if ((check_ldt != NULL) || (check_gdt != NULL))
	{
		/* ERROR: Segment already exists and assigned */ /* Possibly use actual CPU exception if required */
	}
	else
	{
		if (ds->ds_segment_register_selector_table_indicator (c) == 1) /* Allocate new local descriptor table: */
		{
			ldt_allocate_new_local_segment_descriptor (ldt, limit, segment);
			ldt_descriptor = ldt_get_local_segment_descriptor (ldt, segment);
			offset = e386_get_ebp (c) + e386_get_esi (c) + e386_mk_sint32 (c->ea.data[1]); /* Segment offset  : eBX and eSI       */
			ldt_descriptor->ldt_segment_base_address_offset = offset;
			ldt_descriptor->ldt_segment_is_present = ss->ss_segment_register_descriptor_is_present;
			ldt_descriptor->ldt_granularity_bit = ss->ss_segment_register_descriptor_granularity_bit;
			ldt_descriptor->ldt_big = ss->ss_segment_register_descriptor_big;
			ldt_descriptor->ldt_x = ss->ss_segment_register_descriptor_x_;
			ldt_descriptor->ldt_0_ = ss->ss_segment_register_descriptor_0_;
			ldt_descriptor->ldt_segment_available = ss->ss_segment_register_descriptor_available;
			ldt_descriptor->ldt_descriptor_privilege_level = ss->ss_segment_register_descriptor_privilege_level;
			ldt_descriptor->ldt_type_1 = ss->ss_segment_register_descriptor_type_1;
			ldt_descriptor->ldt_type_0 = ss->ss_segment_register_descriptor_type_0;
			ldt_descriptor->ldt_type_c = ss->ss_segment_register_descriptor_type_c;
			ldt_descriptor->ldt_type_e = ss->ss_segment_register_descriptor_type_e;
			ldt_descriptor->ldt_type_r = ss->ss_segment_register_descriptor_type_r;
			ldt_descriptor->ldt_type_w = ss->ss_segment_register_descriptor_type_w;
			ldt_descriptor->ldt_segment_accessed = ss->ss_segment_register_descriptor_accessed;
		}
		else /* Allocate new global descriptor table: */
		{
			gdt_allocate_new_global_segment_descriptor (gdt, limit, segment);
			gdt_descriptor = gdt_get_global_segment_descriptor (gdt, segment);
			offset = e386_get_ebp (c) + e386_get_esi (c) + e386_mk_sint32 (c->ea.data[1]); /* Segment offset  : eBX and eSI       */
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_is_present = ss->ss_segment_register_descriptor_is_present;
			gdt_descriptor->gdt_granularity_bit = ss->ss_segment_register_descriptor_granularity_bit;
			gdt_descriptor->gdt_big = ss->ss_segment_register_descriptor_big;
			gdt_descriptor->gdt_x = ss->ss_segment_register_descriptor_x_;
			gdt_descriptor->gdt_0_ = ss->ss_segment_register_descriptor_0_;
			gdt_descriptor->gdt_segment_available = ss->ss_segment_register_descriptor_available;
			gdt_descriptor->gdt_descriptor_privilege_level = ss->ss_segment_register_descriptor_privilege_level;
			gdt_descriptor->gdt_type_1 = ss->ss_segment_register_descriptor_type_1;
			gdt_descriptor->gdt_type_0 = ss->ss_segment_register_descriptor_type_0;
			gdt_descriptor->gdt_type_c = ss->ss_segment_register_descriptor_type_c;
			gdt_descriptor->gdt_type_e = ss->ss_segment_register_descriptor_type_e;
			gdt_descriptor->gdt_type_r = ss->ss_segment_register_descriptor_type_r;
			gdt_descriptor->gdt_type_w = ss->ss_segment_register_descriptor_type_w;
			gdt_descriptor->gdt_segment_accessed = ss->ss_segment_register_descriptor_accessed;
		}
	}
}

/* EA 0B: [BP + DI + XX] or [eBP + eDI + XX] */ /* Real Mode */
static
void ea386_real_mode_get0b (e8086_t *c)
{
	c->ea.is_mem = 1;                                                                 /* Effective address is memory operand                */
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.seg = e386_get_ss (c);                                                      /* Segment register: DS register                      */
		c->ea.ofs = e386_get_ebp (c) + e386_get_edi (c) + e386_mk_sint32 (c->ea.data[1]); /* Segment offset  : eBX and eSI combined with 32-bit */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.seg = e386_get_ss (c) & 0xfffff;                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_ebp (c) + e386_get_edi (c) + e386_mk_sint32 (c->ea.data[1]) & 0xfffff; /* Segment offset  : eBX and eSI       */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 2;                                                                    /* unsigned integer                                   */
	c->delay += 11;
}

/* EA 0B: [BP + DI + XX] or [eBP + eDI + XX] */ /* Virtual 8086 Mode */
static
void ea386_virtual_8086_mode_get0b (e8086_t *c)
{
	c->ea.is_mem = 1;                                                                 /* Effective address is memory operand                */
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.seg = e386_get_ss (c);                                                      /* Segment register: DS register                      */
		c->ea.ofs = e386_get_ebp (c) + e386_get_edi (c) + e386_mk_sint32 (c->ea.data[1]); /* Segment offset  : eBX and eSI combined with 32-bit */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.seg = e386_get_ss (c) & 0xfffff;                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_ebp (c) + e386_get_edi (c) + e386_mk_sint32 (c->ea.data[1]) & 0xfffff; /* Segment offset  : eBX and eSI       */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 2;                                                                    /* unsigned integer                                   */
	c->delay += 11;
}

/* EA 0B: [BP + DI + XX] or [eBP + eDI + XX] */ /* Protected Mode */
static
void ea386_protected_mode_get0b (e8086_t *c, ss_segment_register_descriptor_t *ss, ldt_local_descriptor_t *ldt, gdt_global_descriptor_t *gdt)
{
	/* New protected mode code: */
	c->ea.is_mem = 1;                                /* Effective address is memory operand */
	segment = e386_get_ss (c);                       /* Segment register: DS register       */
	check_ldt = segment;
	local_descriptor = ldt_get_local_segment_descriptor (ldt, check_ldt);
	check_gdt = segment;
	global_descriptor = gdt_get_global_segment_descriptor (gdt, check_gdt);
	if ((check_ldt != NULL) || (check_gdt != NULL))
	{
		/* ERROR: Segment already exists and assigned */ /* Possibly use actual CPU exception if required */
	}
	else
	{
		if (ds->ds_segment_register_selector_table_indicator (c) == 1) /* Allocate new local descriptor table: */
		{
			ldt_allocate_new_local_segment_descriptor (ldt, limit, segment);
			ldt_descriptor = ldt_get_local_segment_descriptor (ldt, segment);
			offset = e386_get_ebp (c) + e386_get_edi (c) + e386_mk_sint32 (c->ea.data[1]); /* Segment offset  : eBX and eSI       */
			ldt_descriptor->ldt_segment_base_address_offset = offset;
			ldt_descriptor->ldt_segment_is_present = ss->ss_segment_register_descriptor_is_present;
			ldt_descriptor->ldt_granularity_bit = ss->ss_segment_register_descriptor_granularity_bit;
			ldt_descriptor->ldt_big = ss->ss_segment_register_descriptor_big;
			ldt_descriptor->ldt_x = ss->ss_segment_register_descriptor_x_;
			ldt_descriptor->ldt_0_ = ss->ss_segment_register_descriptor_0_;
			ldt_descriptor->ldt_segment_available = ss->ss_segment_register_descriptor_available;
			ldt_descriptor->ldt_descriptor_privilege_level = ss->ss_segment_register_descriptor_privilege_level;
			ldt_descriptor->ldt_type_1 = ss->ss_segment_register_descriptor_type_1;
			ldt_descriptor->ldt_type_0 = ss->ss_segment_register_descriptor_type_0;
			ldt_descriptor->ldt_type_c = ss->ss_segment_register_descriptor_type_c;
			ldt_descriptor->ldt_type_e = ss->ss_segment_register_descriptor_type_e;
			ldt_descriptor->ldt_type_r = ss->ss_segment_register_descriptor_type_r;
			ldt_descriptor->ldt_type_w = ss->ss_segment_register_descriptor_type_w;
			ldt_descriptor->ldt_segment_accessed = ss->ss_segment_register_descriptor_accessed;
		}
		else /* Allocate new global descriptor table: */
		{
			gdt_allocate_new_global_segment_descriptor (gdt, limit, segment);
			gdt_descriptor = gdt_get_global_segment_descriptor (gdt, segment);
			offset = e386_get_ebp (c) + e386_get_edi (c) + e386_mk_sint32 (c->ea.data[1]); /* Segment offset  : eBX and eSI       */
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_is_present = ss->ss_segment_register_descriptor_is_present;
			gdt_descriptor->gdt_granularity_bit = ss->ss_segment_register_descriptor_granularity_bit;
			gdt_descriptor->gdt_big = ss->ss_segment_register_descriptor_big;
			gdt_descriptor->gdt_x = ss->ss_segment_register_descriptor_x_;
			gdt_descriptor->gdt_0_ = ss->ss_segment_register_descriptor_0_;
			gdt_descriptor->gdt_segment_available = ss->ss_segment_register_descriptor_available;
			gdt_descriptor->gdt_descriptor_privilege_level = ss->ss_segment_register_descriptor_privilege_level;
			gdt_descriptor->gdt_type_1 = ss->ss_segment_register_descriptor_type_1;
			gdt_descriptor->gdt_type_0 = ss->ss_segment_register_descriptor_type_0;
			gdt_descriptor->gdt_type_c = ss->ss_segment_register_descriptor_type_c;
			gdt_descriptor->gdt_type_e = ss->ss_segment_register_descriptor_type_e;
			gdt_descriptor->gdt_type_r = ss->ss_segment_register_descriptor_type_r;
			gdt_descriptor->gdt_type_w = ss->ss_segment_register_descriptor_type_w;
			gdt_descriptor->gdt_segment_accessed = ss->ss_segment_register_descriptor_accessed;
		}
	}
}

/* EA 0C: [SI + XX] or [eSI + XX] */ /* Real Mode */
static
void ea386_real_mode_get0c (e8086_t *c)
{
	c->ea.is_mem = 1;                                              /* Effective address is memory operand                       */
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.seg = e386_get_ds (c);                                                      /* Segment register: DS register                      */
		c->ea.ofs = e386_get_esi (c) + e386_mk_sint32 (c->ea.data[1]); /* Segment offset  : eBX and eSI combined with 32-bit */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.seg = e386_get_ds (c) & 0xfffff;                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_esi (c) + e386_mk_sint32 (c->ea.data[1]) & 0xfffff; /* Segment offset  : eBX and eSI       */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 2;
	c->delay += 9;
}

/* EA 0C: [SI + XX] or [eSI + XX] */ /* Virtual 8086 Mode */
static
void ea386_virtual_8086_mode_get0c (e8086_t *c)
{
	c->ea.is_mem = 1;                                              /* Effective address is memory operand                       */
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.seg = e386_get_ds (c);                                                      /* Segment register: DS register                      */
		c->ea.ofs = e386_get_esi (c) + e386_mk_sint32 (c->ea.data[1]); /* Segment offset  : eBX and eSI combined with 32-bit */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.seg = e386_get_ds (c) & 0xfffff;                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_esi (c) + e386_mk_sint32 (c->ea.data[1]) & 0xfffff; /* Segment offset  : eBX and eSI       */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 2;
	c->delay += 9;
}

/* EA 0C: [SI + XX] or [eSI + XX] */ /* Protected Mode */
static
void ea386_protected_mode_get0c (e8086_t *c, ds_segment_register_descriptor_t *ds, ldt_local_descriptor_t *ldt, gdt_global_descriptor_t *gdt)
{
	/* New protected mode code: */
	c->ea.is_mem = 1;                                /* Effective address is memory operand */
	segment = e386_get_ds (c);                       /* Segment register: DS register       */
	check_ldt = segment;
	local_descriptor = ldt_get_local_segment_descriptor (ldt, check_ldt);
	check_gdt = segment;
	global_descriptor = gdt_get_global_segment_descriptor (gdt, check_gdt);
	if ((check_ldt != NULL) || (check_gdt != NULL))
	{
		/* ERROR: Segment already exists and assigned */ /* Possibly use actual CPU exception if required */
	}
	else
	{
		if (ds->ds_segment_register_selector_table_indicator (c) == 1) /* Allocate new local descriptor table: */
		{
			ldt_allocate_new_local_segment_descriptor (ldt, limit, segment);
			ldt_descriptor = ldt_get_local_segment_descriptor (ldt, segment);
			offset = e386_get_esi (c) + e386_mk_sint32 (c->ea.data[1]); /* Segment offset  : eBX and eSI       */
			ldt_descriptor->ldt_segment_base_address_offset = offset;
			ldt_descriptor->ldt_segment_is_present = ds->ds_segment_register_descriptor_is_present;
			ldt_descriptor->ldt_granularity_bit = ds->ds_segment_register_descriptor_granularity_bit;
			ldt_descriptor->ldt_big = ds->ds_segment_register_descriptor_big;
			ldt_descriptor->ldt_x = ds->ds_segment_register_descriptor_x_;
			ldt_descriptor->ldt_0_ = ds->ds_segment_register_descriptor_0_;
			ldt_descriptor->ldt_segment_available = ds->ds_segment_register_descriptor_available;
			ldt_descriptor->ldt_descriptor_privilege_level = ds->ds_segment_register_descriptor_privilege_level;
			ldt_descriptor->ldt_type_1 = ds->ds_segment_register_descriptor_type_1;
			ldt_descriptor->ldt_type_0 = ds->ds_segment_register_descriptor_type_0;
			ldt_descriptor->ldt_type_c = ds->ds_segment_register_descriptor_type_c;
			ldt_descriptor->ldt_type_e = ds->ds_segment_register_descriptor_type_e;
			ldt_descriptor->ldt_type_r = ds->ds_segment_register_descriptor_type_r;
			ldt_descriptor->ldt_type_w = ds->ds_segment_register_descriptor_type_w;
			ldt_descriptor->ldt_segment_accessed = ds->ds_segment_register_descriptor_accessed;
		}
		else /* Allocate new global descriptor table: */
		{
			gdt_allocate_new_global_segment_descriptor (gdt, limit, segment);
			gdt_descriptor = gdt_get_global_segment_descriptor (gdt, segment);
			offset = e386_get_esi (c) + e386_mk_sint32 (c->ea.data[1]); /* Segment offset  : eBX and eSI       */
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_is_present = ds->ds_segment_register_descriptor_is_present;
			gdt_descriptor->gdt_granularity_bit = ds->ds_segment_register_descriptor_granularity_bit;
			gdt_descriptor->gdt_big = ds->ds_segment_register_descriptor_big;
			gdt_descriptor->gdt_x = ds->ds_segment_register_descriptor_x_;
			gdt_descriptor->gdt_0_ = ds->ds_segment_register_descriptor_0_;
			gdt_descriptor->gdt_segment_available = ds->ds_segment_register_descriptor_available;
			gdt_descriptor->gdt_descriptor_privilege_level = ds->ds_segment_register_descriptor_privilege_level;
			gdt_descriptor->gdt_type_1 = ds->ds_segment_register_descriptor_type_1;
			gdt_descriptor->gdt_type_0 = ds->ds_segment_register_descriptor_type_0;
			gdt_descriptor->gdt_type_c = ds->ds_segment_register_descriptor_type_c;
			gdt_descriptor->gdt_type_e = ds->ds_segment_register_descriptor_type_e;
			gdt_descriptor->gdt_type_r = ds->ds_segment_register_descriptor_type_r;
			gdt_descriptor->gdt_type_w = ds->ds_segment_register_descriptor_type_w;
			gdt_descriptor->gdt_segment_accessed = ds->ds_segment_register_descriptor_accessed;
		}
	}
}

/* EA 0D: [DI + XX] or [eDI + XX] */ /* Real Mode */
static
void ea386_real_mode_get0d (e8086_t *c)
{
	c->ea.is_mem = 1;                                              /* Effective address is memory operand                       */
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.seg = e386_get_ds (c);                                                      /* Segment register: DS register                      */
		c->ea.ofs = e386_get_edi (c) + e386_mk_sint32 (c->ea.data[1]); /* Segment offset  : eBX and eSI combined with 32-bit */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.seg = e386_get_ds (c) & 0xfffff;                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_edi (c) + e386_mk_sint32 (c->ea.data[1]) & 0xfffff; /* Segment offset  : eBX and eSI       */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 2;
	c->delay += 9;
}

/* EA 0D: [DI + XX] or [eDI + XX] */ /* Virtual 8086 Mode */
static
void ea386_virtual_8086_mode_get0d (e8086_t *c)
{
	c->ea.is_mem = 1;                                              /* Effective address is memory operand                       */
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.seg = e386_get_ds (c);                                                      /* Segment register: DS register                      */
		c->ea.ofs = e386_get_edi (c) + e386_mk_sint32 (c->ea.data[1]); /* Segment offset  : eBX and eSI combined with 32-bit */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.seg = e386_get_ds (c) & 0xfffff;                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_edi (c) + e386_mk_sint32 (c->ea.data[1]) & 0xfffff; /* Segment offset  : eBX and eSI       */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 2;
	c->delay += 9;
}

/* EA 0D: [DI + XX] or [eDI + XX] */ /* Protected Mode */
static
void ea386_protected_mode_get0d (e8086_t *c, ds_segment_register_descriptor_t *ds, ldt_local_descriptor_t *ldt, gdt_global_descriptor_t *gdt)
{
	/* New protected mode code: */
	c->ea.is_mem = 1;                                /* Effective address is memory operand */
	segment = e386_get_ds (c);                       /* Segment register: DS register       */
	check_ldt = segment;
	local_descriptor = ldt_get_local_segment_descriptor (ldt, check_ldt);
	check_gdt = segment;
	global_descriptor = gdt_get_global_segment_descriptor (gdt, check_gdt);
	if ((check_ldt != NULL) || (check_gdt != NULL))
	{
		/* ERROR: Segment already exists and assigned */ /* Possibly use actual CPU exception if required */
	}
	else
	{
		if (ds->ds_segment_register_selector_table_indicator (c) == 1) /* Allocate new local descriptor table: */
		{
			ldt_allocate_new_local_segment_descriptor (ldt, limit, segment);
			ldt_descriptor = ldt_get_local_segment_descriptor (ldt, segment);
			offset = e386_get_edi (c) + e386_mk_sint32 (c->ea.data[1]); /* Segment offset  : eBX and eSI       */
			ldt_descriptor->ldt_segment_base_address_offset = offset;
			ldt_descriptor->ldt_segment_is_present = ds->ds_segment_register_descriptor_is_present;
			ldt_descriptor->ldt_granularity_bit = ds->ds_segment_register_descriptor_granularity_bit;
			ldt_descriptor->ldt_big = ds->ds_segment_register_descriptor_big;
			ldt_descriptor->ldt_x = ds->ds_segment_register_descriptor_x_;
			ldt_descriptor->ldt_0_ = ds->ds_segment_register_descriptor_0_;
			ldt_descriptor->ldt_segment_available = ds->ds_segment_register_descriptor_available;
			ldt_descriptor->ldt_descriptor_privilege_level = ds->ds_segment_register_descriptor_privilege_level;
			ldt_descriptor->ldt_type_1 = ds->ds_segment_register_descriptor_type_1;
			ldt_descriptor->ldt_type_0 = ds->ds_segment_register_descriptor_type_0;
			ldt_descriptor->ldt_type_c = ds->ds_segment_register_descriptor_type_c;
			ldt_descriptor->ldt_type_e = ds->ds_segment_register_descriptor_type_e;
			ldt_descriptor->ldt_type_r = ds->ds_segment_register_descriptor_type_r;
			ldt_descriptor->ldt_type_w = ds->ds_segment_register_descriptor_type_w;
			ldt_descriptor->ldt_segment_accessed = ds->ds_segment_register_descriptor_accessed;
		}
		else /* Allocate new global descriptor table: */
		{
			gdt_allocate_new_global_segment_descriptor (gdt, limit, segment);
			gdt_descriptor = gdt_get_global_segment_descriptor (gdt, segment);
			offset = e386_get_edi (c) + e386_mk_sint32 (c->ea.data[1]); /* Segment offset  : eBX and eSI       */
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_is_present = ds->ds_segment_register_descriptor_is_present;
			gdt_descriptor->gdt_granularity_bit = ds->ds_segment_register_descriptor_granularity_bit;
			gdt_descriptor->gdt_big = ds->ds_segment_register_descriptor_big;
			gdt_descriptor->gdt_x = ds->ds_segment_register_descriptor_x_;
			gdt_descriptor->gdt_0_ = ds->ds_segment_register_descriptor_0_;
			gdt_descriptor->gdt_segment_available = ds->ds_segment_register_descriptor_available;
			gdt_descriptor->gdt_descriptor_privilege_level = ds->ds_segment_register_descriptor_privilege_level;
			gdt_descriptor->gdt_type_1 = ds->ds_segment_register_descriptor_type_1;
			gdt_descriptor->gdt_type_0 = ds->ds_segment_register_descriptor_type_0;
			gdt_descriptor->gdt_type_c = ds->ds_segment_register_descriptor_type_c;
			gdt_descriptor->gdt_type_e = ds->ds_segment_register_descriptor_type_e;
			gdt_descriptor->gdt_type_r = ds->ds_segment_register_descriptor_type_r;
			gdt_descriptor->gdt_type_w = ds->ds_segment_register_descriptor_type_w;
			gdt_descriptor->gdt_segment_accessed = ds->ds_segment_register_descriptor_accessed;
		}
	}
}

/* EA 0E: [BP + XX] or [eBP + XX] */ /* Real Mode */
static
void ea386_real_mode_get0e (e8086_t *c)
{
	c->ea.is_mem = 1;                                              /* Effective address is memory operand                       */
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.seg = e386_get_ss (c);                                                      /* Segment register: DS register                      */
		c->ea.ofs = e386_get_ebp (c) + e386_mk_sint32 (c->ea.data[1]); /* Segment offset  : eBX and eSI combined with 32-bit */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.seg = e386_get_ss (c) & 0xfffff;                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_ebp (c) + e386_mk_sint32 (c->ea.data[1]) & 0xfffff; /* Segment offset  : eBX and eSI       */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 2;
	c->delay += 9;
}

/* EA 0E: [BP + XX] or [eBP + XX] */ /* Virtual 8086 Mode */
static
void ea386_virtual_8086_mode_get0e (e8086_t *c)
{
	c->ea.is_mem = 1;                                              /* Effective address is memory operand                       */
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.seg = e386_get_ss (c);                                                      /* Segment register: DS register                      */
		c->ea.ofs = e386_get_ebp (c) + e386_mk_sint32 (c->ea.data[1]); /* Segment offset  : eBX and eSI combined with 32-bit */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.seg = e386_get_ss (c) & 0xfffff;                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_ebp (c) + e386_mk_sint32 (c->ea.data[1]) & 0xfffff; /* Segment offset  : eBX and eSI       */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 2;
	c->delay += 9;
}

/* EA 0E: [BP + XX] or [eBP + XX] */ /* Protected Mode */
static
void ea386_protected_mode_get0e (e8086_t *c, ss_segment_register_descriptor_t *ss, ldt_local_descriptor_t *ldt, gdt_global_descriptor_t *gdt)
{
	/* New protected mode code: */
	c->ea.is_mem = 1;                                /* Effective address is memory operand */
	segment = e386_get_ss (c);                       /* Segment register: DS register       */
	check_ldt = segment;
	local_descriptor = ldt_get_local_segment_descriptor (ldt, check_ldt);
	check_gdt = segment;
	global_descriptor = gdt_get_global_segment_descriptor (gdt, check_gdt);
	if ((check_ldt != NULL) || (check_gdt != NULL))
	{
		/* ERROR: Segment already exists and assigned */ /* Possibly use actual CPU exception if required */
	}
	else
	{
		if (ds->ds_segment_register_selector_table_indicator (c) == 1) /* Allocate new local descriptor table: */
		{
			ldt_allocate_new_local_segment_descriptor (ldt, limit, segment);
			ldt_descriptor = ldt_get_local_segment_descriptor (ldt, segment);
			offset = e386_get_ebp (c) + e386_mk_sint32 (c->ea.data[1]); /* Segment offset  : eBX and eSI       */
			ldt_descriptor->ldt_segment_base_address_offset = offset;
			ldt_descriptor->ldt_segment_is_present = ss->ss_segment_register_descriptor_is_present;
			ldt_descriptor->ldt_granularity_bit = ss->ss_segment_register_descriptor_granularity_bit;
			ldt_descriptor->ldt_big = ss->ss_segment_register_descriptor_big;
			ldt_descriptor->ldt_x = ss->ss_segment_register_descriptor_x_;
			ldt_descriptor->ldt_0_ = ss->ss_segment_register_descriptor_0_;
			ldt_descriptor->ldt_segment_available = ss->ss_segment_register_descriptor_available;
			ldt_descriptor->ldt_descriptor_privilege_level = ss->ss_segment_register_descriptor_privilege_level;
			ldt_descriptor->ldt_type_1 = ss->ss_segment_register_descriptor_type_1;
			ldt_descriptor->ldt_type_0 = ss->ss_segment_register_descriptor_type_0;
			ldt_descriptor->ldt_type_c = ss->ss_segment_register_descriptor_type_c;
			ldt_descriptor->ldt_type_e = ss->ss_segment_register_descriptor_type_e;
			ldt_descriptor->ldt_type_r = ss->ss_segment_register_descriptor_type_r;
			ldt_descriptor->ldt_type_w = ss->ss_segment_register_descriptor_type_w;
			ldt_descriptor->ldt_segment_accessed = ss->ss_segment_register_descriptor_accessed;
		}
		else /* Allocate new global descriptor table: */
		{
			gdt_allocate_new_global_segment_descriptor (gdt, limit, segment);
			gdt_descriptor = gdt_get_global_segment_descriptor (gdt, segment);
			offset = e386_get_ebp (c) + e386_mk_sint32 (c->ea.data[1]); /* Segment offset  : eBX and eSI       */
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_is_present = ss->ss_segment_register_descriptor_is_present;
			gdt_descriptor->gdt_granularity_bit = ss->ss_segment_register_descriptor_granularity_bit;
			gdt_descriptor->gdt_big = ss->ss_segment_register_descriptor_big;
			gdt_descriptor->gdt_x = ss->ss_segment_register_descriptor_x_;
			gdt_descriptor->gdt_0_ = ss->ss_segment_register_descriptor_0_;
			gdt_descriptor->gdt_segment_available = ss->ss_segment_register_descriptor_available;
			gdt_descriptor->gdt_descriptor_privilege_level = ss->ss_segment_register_descriptor_privilege_level;
			gdt_descriptor->gdt_type_1 = ss->ss_segment_register_descriptor_type_1;
			gdt_descriptor->gdt_type_0 = ss->ss_segment_register_descriptor_type_0;
			gdt_descriptor->gdt_type_c = ss->ss_segment_register_descriptor_type_c;
			gdt_descriptor->gdt_type_e = ss->ss_segment_register_descriptor_type_e;
			gdt_descriptor->gdt_type_r = ss->ss_segment_register_descriptor_type_r;
			gdt_descriptor->gdt_type_w = ss->ss_segment_register_descriptor_type_w;
			gdt_descriptor->gdt_segment_accessed = ss->ss_segment_register_descriptor_accessed;
		}
	}
}

/* EA 0F: [BX + XX] or [eBX + XX] */ /* Real Mode */
static
void ea386_real_mode_get0f (e8086_t *c)
{
	c->ea.is_mem = 1;                                              /* Effective address is memory operand                       */
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.seg = e386_get_ds (c);                                                      /* Segment register: DS register                      */
		c->ea.ofs = e386_get_ebx (c) + e386_mk_sint32 (c->ea.data[1]); /* Segment offset  : eBX and eSI combined with 32-bit */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.seg = e386_get_ds (c) & 0xfffff;                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_ebx (c) + e386_mk_sint32 (c->ea.data[1]) & 0xfffff; /* Segment offset  : eBX and eSI       */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 2;
	c->delay += 9;
}

/* EA 0F: [BX + XX] or [eBX + XX] */ /* Virtual 8086 Mode */
static
void ea386_virtual_8086_mode_get0f (e8086_t *c)
{
	c->ea.is_mem = 1;                                              /* Effective address is memory operand                       */
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.seg = e386_get_ds (c);                                                      /* Segment register: DS register                      */
		c->ea.ofs = e386_get_ebx (c) + e386_mk_sint32 (c->ea.data[1]); /* Segment offset  : eBX and eSI combined with 32-bit */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.seg = e386_get_ds (c) & 0xfffff;                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_ebx (c) + e386_mk_sint32 (c->ea.data[1]) & 0xfffff; /* Segment offset  : eBX and eSI       */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 2;
	c->delay += 9;
}

/* EA 0F: [BX + XX] or [eBX + XX] */ /* Protected Mode */
static
void ea386_protected_mode_get0f (e8086_t *c, ds_segment_register_descriptor_t *ds, ldt_local_descriptor_t *ldt, gdt_global_descriptor_t *gdt)
{
	/* New protected mode code: */
	c->ea.is_mem = 1;                                /* Effective address is memory operand */
	segment = e386_get_ds (c);                       /* Segment register: DS register       */
	check_ldt = segment;
	local_descriptor = ldt_get_local_segment_descriptor (ldt, check_ldt);
	check_gdt = segment;
	global_descriptor = gdt_get_global_segment_descriptor (gdt, check_gdt);
	if ((check_ldt != NULL) || (check_gdt != NULL))
	{
		/* ERROR: Segment already exists and assigned */ /* Possibly use actual CPU exception if required */
	}
	else
	{
		if (ds->ds_segment_register_selector_table_indicator (c) == 1) /* Allocate new local descriptor table: */
		{
			ldt_allocate_new_local_segment_descriptor (ldt, limit, segment);
			ldt_descriptor = ldt_get_local_segment_descriptor (ldt, segment);
			offset = e386_get_ebx (c) + e386_mk_sint32 (c->ea.data[1]); /* Segment offset  : eBX and eSI       */
			ldt_descriptor->ldt_segment_base_address_offset = offset;
			ldt_descriptor->ldt_segment_is_present = ds->ds_segment_register_descriptor_is_present;
			ldt_descriptor->ldt_granularity_bit = ds->ds_segment_register_descriptor_granularity_bit;
			ldt_descriptor->ldt_big = ds->ds_segment_register_descriptor_big;
			ldt_descriptor->ldt_x = ds->ds_segment_register_descriptor_x_;
			ldt_descriptor->ldt_0_ = ds->ds_segment_register_descriptor_0_;
			ldt_descriptor->ldt_segment_available = ds->ds_segment_register_descriptor_available;
			ldt_descriptor->ldt_descriptor_privilege_level = ds->ds_segment_register_descriptor_privilege_level;
			ldt_descriptor->ldt_type_1 = ds->ds_segment_register_descriptor_type_1;
			ldt_descriptor->ldt_type_0 = ds->ds_segment_register_descriptor_type_0;
			ldt_descriptor->ldt_type_c = ds->ds_segment_register_descriptor_type_c;
			ldt_descriptor->ldt_type_e = ds->ds_segment_register_descriptor_type_e;
			ldt_descriptor->ldt_type_r = ds->ds_segment_register_descriptor_type_r;
			ldt_descriptor->ldt_type_w = ds->ds_segment_register_descriptor_type_w;
			ldt_descriptor->ldt_segment_accessed = ds->ds_segment_register_descriptor_accessed;
		}
		else /* Allocate new global descriptor table: */
		{
			gdt_allocate_new_global_segment_descriptor (gdt, limit, segment);
			gdt_descriptor = gdt_get_global_segment_descriptor (gdt, segment);
			offset = e386_get_ebx (c) + e386_mk_sint32 (c->ea.data[1]); /* Segment offset  : eBX and eSI       */
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_is_present = ds->ds_segment_register_descriptor_is_present;
			gdt_descriptor->gdt_granularity_bit = ds->ds_segment_register_descriptor_granularity_bit;
			gdt_descriptor->gdt_big = ds->ds_segment_register_descriptor_big;
			gdt_descriptor->gdt_x = ds->ds_segment_register_descriptor_x_;
			gdt_descriptor->gdt_0_ = ds->ds_segment_register_descriptor_0_;
			gdt_descriptor->gdt_segment_available = ds->ds_segment_register_descriptor_available;
			gdt_descriptor->gdt_descriptor_privilege_level = ds->ds_segment_register_descriptor_privilege_level;
			gdt_descriptor->gdt_type_1 = ds->ds_segment_register_descriptor_type_1;
			gdt_descriptor->gdt_type_0 = ds->ds_segment_register_descriptor_type_0;
			gdt_descriptor->gdt_type_c = ds->ds_segment_register_descriptor_type_c;
			gdt_descriptor->gdt_type_e = ds->ds_segment_register_descriptor_type_e;
			gdt_descriptor->gdt_type_r = ds->ds_segment_register_descriptor_type_r;
			gdt_descriptor->gdt_type_w = ds->ds_segment_register_descriptor_type_w;
			gdt_descriptor->gdt_segment_accessed = ds->ds_segment_register_descriptor_accessed;
		}
	}
}

/* EA 10: [BX + SI + XXXX] or [eBX + eSI + XXXX] */ /* Real Mode */
static
void ea386_real_mode_get10 (e8086_t *c)
{
	c->ea.is_mem = 1;                                           /* Effective address is memory operand                                      */
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.seg = e386_get_ds (c);                                /* Segment register: DS register                                            */
		c->ea.ofs = e386_get_ebx (c) + e386_get_esi (c);            /* Segment offset  : eBX and eSI                                            */
		c->ea.ofs += e386_mk_uint32 (c->ea.data[1], c->ea.data[2]); /* Additional segment offset: 32-bit signed integer added to previous value */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.seg = e386_get_ds (c) & 0xfffff;                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_ebx (c) + e386_get_esi (c) & 0xfffff;
		c->ea.ofs += e386_mk_uint32 (c->ea.data[1], c->ea.data[2]) & 0xfffff; /* Segment offset  : eBX and eSI       */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 3;
	c->delay += 11;
}

/* EA 10: [BX + SI + XXXX] or [eBX + eSI + XXXX] */ /* Virtual 8086 Mode */
static
void ea386_virtual_8086_mode_get10 (e8086_t *c)
{
	c->ea.is_mem = 1;                                           /* Effective address is memory operand                                      */
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.seg = e386_get_ds (c);                                /* Segment register: DS register                                            */
		c->ea.ofs = e386_get_ebx (c) + e386_get_esi (c);            /* Segment offset  : eBX and eSI                                            */
		c->ea.ofs += e386_mk_uint32 (c->ea.data[1], c->ea.data[2]); /* Additional segment offset: 32-bit signed integer added to previous value */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.seg = e386_get_ds (c) & 0xfffff;                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_ebx (c) + e386_get_esi (c) & 0xfffff;
		c->ea.ofs += e386_mk_uint32 (c->ea.data[1], c->ea.data[2]) & 0xfffff; /* Segment offset  : eBX and eSI       */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 3;
	c->delay += 11;
}

/* EA 10: [BX + SI + XXXX] or [eBX + eSI + XXXX] */ /* Protected Mode */
static
void ea386_protected_mode_get10 (e8086_t *c, ds_segment_register_descriptor_t *ds, ldt_local_descriptor_t *ldt, gdt_global_descriptor_t *gdt)
{
	/* New protected mode code: */
	c->ea.is_mem = 1;                                /* Effective address is memory operand */
	segment = e386_get_ds (c);                       /* Segment register: DS register       */
	check_ldt = segment;
	local_descriptor = ldt_get_local_segment_descriptor (ldt, check_ldt);
	check_gdt = segment;
	global_descriptor = gdt_get_global_segment_descriptor (gdt, check_gdt);
	if ((check_ldt != NULL) || (check_gdt != NULL))
	{
		/* ERROR: Segment already exists and assigned */ /* Possibly use actual CPU exception if required */
	}
	else
	{
		if (ds->ds_segment_register_selector_table_indicator (c) == 1) /* Allocate new local descriptor table: */
		{
			ldt_allocate_new_local_segment_descriptor (ldt, limit, segment);
			ldt_descriptor = ldt_get_local_segment_descriptor (ldt, segment);
			offset = e386_get_ebx (c) + e386_get_esi (c); /* Segment offset  : eBX and eSI       */
			ldt_descriptor->ldt_segment_base_address_offset = offset;
			offset += e386_mk_uint32 (c->ea.data[1], c->ea.data[2]) & 0xfffff; /* Segment offset  : eBX and eSI       */
			ldt_descriptor->ldt_segment_base_address_offset = offset;
			ldt_descriptor->ldt_segment_is_present = ds->ds_segment_register_descriptor_is_present;
			ldt_descriptor->ldt_granularity_bit = ds->ds_segment_register_descriptor_granularity_bit;
			ldt_descriptor->ldt_big = ds->ds_segment_register_descriptor_big;
			ldt_descriptor->ldt_x = ds->ds_segment_register_descriptor_x_;
			ldt_descriptor->ldt_0_ = ds->ds_segment_register_descriptor_0_;
			ldt_descriptor->ldt_segment_available = ds->ds_segment_register_descriptor_available;
			ldt_descriptor->ldt_descriptor_privilege_level = ds->ds_segment_register_descriptor_privilege_level;
			ldt_descriptor->ldt_type_1 = ds->ds_segment_register_descriptor_type_1;
			ldt_descriptor->ldt_type_0 = ds->ds_segment_register_descriptor_type_0;
			ldt_descriptor->ldt_type_c = ds->ds_segment_register_descriptor_type_c;
			ldt_descriptor->ldt_type_e = ds->ds_segment_register_descriptor_type_e;
			ldt_descriptor->ldt_type_r = ds->ds_segment_register_descriptor_type_r;
			ldt_descriptor->ldt_type_w = ds->ds_segment_register_descriptor_type_w;
			ldt_descriptor->ldt_segment_accessed = ds->ds_segment_register_descriptor_accessed;
		}
		else /* Allocate new global descriptor table: */
		{
			gdt_allocate_new_global_segment_descriptor (gdt, limit, segment);
			gdt_descriptor = gdt_get_global_segment_descriptor (gdt, segment);
			offset = e386_get_ebx (c) + e386_get_esi (c); /* Segment offset  : eBX and eSI       */
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			offset += e386_mk_uint32 (c->ea.data[1], c->ea.data[2]) & 0xfffff; /* Segment offset  : eBX and eSI       */
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_is_present = ds->ds_segment_register_descriptor_is_present;
			gdt_descriptor->gdt_granularity_bit = ds->ds_segment_register_descriptor_granularity_bit;
			gdt_descriptor->gdt_big = ds->ds_segment_register_descriptor_big;
			gdt_descriptor->gdt_x = ds->ds_segment_register_descriptor_x_;
			gdt_descriptor->gdt_0_ = ds->ds_segment_register_descriptor_0_;
			gdt_descriptor->gdt_segment_available = ds->ds_segment_register_descriptor_available;
			gdt_descriptor->gdt_descriptor_privilege_level = ds->ds_segment_register_descriptor_privilege_level;
			gdt_descriptor->gdt_type_1 = ds->ds_segment_register_descriptor_type_1;
			gdt_descriptor->gdt_type_0 = ds->ds_segment_register_descriptor_type_0;
			gdt_descriptor->gdt_type_c = ds->ds_segment_register_descriptor_type_c;
			gdt_descriptor->gdt_type_e = ds->ds_segment_register_descriptor_type_e;
			gdt_descriptor->gdt_type_r = ds->ds_segment_register_descriptor_type_r;
			gdt_descriptor->gdt_type_w = ds->ds_segment_register_descriptor_type_w;
			gdt_descriptor->gdt_segment_accessed = ds->ds_segment_register_descriptor_accessed;
		}
	}
}

/* EA 11: [BX + DI + XXXX] or [eBX + eDI + XXXX] */ /* Real Mode */
static
void ea386_real_mode_get11 (e8086_t *c)
{
	c->ea.is_mem = 1;                                           /* Effective address is memory operand                                      */
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.seg = e386_get_ds (c);                                /* Segment register: DS register                                            */
		c->ea.ofs = e386_get_ebx (c) + e386_get_edi (c);            /* Segment offset  : eBX and eSI                                            */
		c->ea.ofs += e386_mk_uint32 (c->ea.data[1], c->ea.data[2]); /* Additional segment offset: 32-bit signed integer added to previous value */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.seg = e386_get_ds (c) & 0xfffff;                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_ebx (c) + e386_get_edi (c) & 0xfffff;
		c->ea.ofs += e386_mk_uint32 (c->ea.data[1], c->ea.data[2]) & 0xfffff; /* Segment offset  : eBX and eSI       */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 3;
	c->delay += 12;
}

/* EA 11: [BX + DI + XXXX] or [eBX + eDI + XXXX] */ /* Virtual 8086 Mode */
static
void ea386_virtual_8086_mode_get11 (e8086_t *c)
{
	c->ea.is_mem = 1;                                           /* Effective address is memory operand                                      */
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.seg = e386_get_ds (c);                                /* Segment register: DS register                                            */
		c->ea.ofs = e386_get_ebx (c) + e386_get_edi (c);            /* Segment offset  : eBX and eSI                                            */
		c->ea.ofs += e386_mk_uint32 (c->ea.data[1], c->ea.data[2]); /* Additional segment offset: 32-bit signed integer added to previous value */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.seg = e386_get_ds (c) & 0xfffff;                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_ebx (c) + e386_get_edi (c) & 0xfffff;
		c->ea.ofs += e386_mk_uint32 (c->ea.data[1], c->ea.data[2]) & 0xfffff; /* Segment offset  : eBX and eSI       */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 3;
	c->delay += 12;
}

/* EA 11: [BX + DI + XXXX] or [eBX + eDI + XXXX] */ /* Protected Mode */
static
void ea386_protected_mode_get11 (e8086_t *c, ds_segment_register_descriptor_t *ds, ldt_local_descriptor_t *ldt, gdt_global_descriptor_t *gdt)
{
	/* New protected mode code: */
	c->ea.is_mem = 1;                                /* Effective address is memory operand */
	segment = e386_get_ds (c);                       /* Segment register: DS register       */
	check_ldt = segment;
	local_descriptor = ldt_get_local_segment_descriptor (ldt, check_ldt);
	check_gdt = segment;
	global_descriptor = gdt_get_global_segment_descriptor (gdt, check_gdt);
	if ((check_ldt != NULL) || (check_gdt != NULL))
	{
		/* ERROR: Segment already exists and assigned */ /* Possibly use actual CPU exception if required */
	}
	else
	{
		if (ds->ds_segment_register_selector_table_indicator (c) == 1) /* Allocate new local descriptor table: */
		{
			ldt_allocate_new_local_segment_descriptor (ldt, limit, segment);
			ldt_descriptor = ldt_get_local_segment_descriptor (ldt, segment);
			offset = e386_get_ebx (c) + e386_get_edi (c); /* Segment offset  : eBX and eSI       */
			ldt_descriptor->ldt_segment_base_address_offset = offset;
			offset += e386_mk_uint32 (c->ea.data[1], c->ea.data[2]); /* Segment offset  : eBX and eSI       */
			ldt_descriptor->ldt_segment_base_address_offset = offset;
			ldt_descriptor->ldt_segment_is_present = ds->ds_segment_register_descriptor_is_present;
			ldt_descriptor->ldt_granularity_bit = ds->ds_segment_register_descriptor_granularity_bit;
			ldt_descriptor->ldt_big = ds->ds_segment_register_descriptor_big;
			ldt_descriptor->ldt_x = ds->ds_segment_register_descriptor_x_;
			ldt_descriptor->ldt_0_ = ds->ds_segment_register_descriptor_0_;
			ldt_descriptor->ldt_segment_available = ds->ds_segment_register_descriptor_available;
			ldt_descriptor->ldt_descriptor_privilege_level = ds->ds_segment_register_descriptor_privilege_level;
			ldt_descriptor->ldt_type_1 = ds->ds_segment_register_descriptor_type_1;
			ldt_descriptor->ldt_type_0 = ds->ds_segment_register_descriptor_type_0;
			ldt_descriptor->ldt_type_c = ds->ds_segment_register_descriptor_type_c;
			ldt_descriptor->ldt_type_e = ds->ds_segment_register_descriptor_type_e;
			ldt_descriptor->ldt_type_r = ds->ds_segment_register_descriptor_type_r;
			ldt_descriptor->ldt_type_w = ds->ds_segment_register_descriptor_type_w;
			ldt_descriptor->ldt_segment_accessed = ds->ds_segment_register_descriptor_accessed;
		}
		else /* Allocate new global descriptor table: */
		{
			gdt_allocate_new_global_segment_descriptor (gdt, limit, segment);
			gdt_descriptor = gdt_get_global_segment_descriptor (gdt, segment);
			offset = e386_get_ebx (c) + e386_get_edi (c); /* Segment offset  : eBX and eSI       */
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			offset += e386_mk_uint32 (c->ea.data[1], c->ea.data[2]); /* Segment offset  : eBX and eSI       */
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_is_present = ds->ds_segment_register_descriptor_is_present;
			gdt_descriptor->gdt_granularity_bit = ds->ds_segment_register_descriptor_granularity_bit;
			gdt_descriptor->gdt_big = ds->ds_segment_register_descriptor_big;
			gdt_descriptor->gdt_x = ds->ds_segment_register_descriptor_x_;
			gdt_descriptor->gdt_0_ = ds->ds_segment_register_descriptor_0_;
			gdt_descriptor->gdt_segment_available = ds->ds_segment_register_descriptor_available;
			gdt_descriptor->gdt_descriptor_privilege_level = ds->ds_segment_register_descriptor_privilege_level;
			gdt_descriptor->gdt_type_1 = ds->ds_segment_register_descriptor_type_1;
			gdt_descriptor->gdt_type_0 = ds->ds_segment_register_descriptor_type_0;
			gdt_descriptor->gdt_type_c = ds->ds_segment_register_descriptor_type_c;
			gdt_descriptor->gdt_type_e = ds->ds_segment_register_descriptor_type_e;
			gdt_descriptor->gdt_type_r = ds->ds_segment_register_descriptor_type_r;
			gdt_descriptor->gdt_type_w = ds->ds_segment_register_descriptor_type_w;
			gdt_descriptor->gdt_segment_accessed = ds->ds_segment_register_descriptor_accessed;
		}
	}
}

/* EA 12: [BP + SI + XXXX] or [eBP + eSI + XXXX] */ /* Real Mode */
static
void ea386_real_mode_get12 (e8086_t *c)
{
	c->ea.is_mem = 1;                                           /* Effective address is memory operand                                      */
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.seg = e386_get_ss (c);                                /* Segment register: DS register                                            */
		c->ea.ofs = e386_get_ebp (c) + e386_get_esi (c);            /* Segment offset  : eBX and eSI                                            */
		c->ea.ofs += e386_mk_uint32 (c->ea.data[1], c->ea.data[2]); /* Additional segment offset: 32-bit signed integer added to previous value */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.seg = e386_get_ss (c) & 0xfffff;                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_ebp (c) + e386_get_esi (c) & 0xfffff;
		c->ea.ofs += e386_mk_uint32 (c->ea.data[1], c->ea.data[2]) & 0xfffff; /* Segment offset  : eBX and eSI       */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 3;
	c->delay += 12;
}

/* EA 12: [BP + SI + XXXX] or [eBP + eSI + XXXX] */ /* Virtual 8086 Mode */
static
void ea386_virtual_8086_mode_get12 (e8086_t *c)
{
	c->ea.is_mem = 1;                                           /* Effective address is memory operand                                      */
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.seg = e386_get_ss (c);                                /* Segment register: DS register                                            */
		c->ea.ofs = e386_get_ebp (c) + e386_get_esi (c);            /* Segment offset  : eBX and eSI                                            */
		c->ea.ofs += e386_mk_uint32 (c->ea.data[1], c->ea.data[2]); /* Additional segment offset: 32-bit signed integer added to previous value */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.seg = e386_get_ss (c) & 0xfffff;                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_ebp (c) + e386_get_esi (c) & 0xfffff;
		c->ea.ofs += e386_mk_uint32 (c->ea.data[1], c->ea.data[2]) & 0xfffff; /* Segment offset  : eBX and eSI       */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 3;
	c->delay += 12;
}

/* EA 12: [BP + SI + XXXX] or [eBP + eSI + XXXX] */ /* Protected Mode */
static
void ea386_protected_mode_get12 (e8086_t *c, ss_segment_register_descriptor_t *ss, ldt_local_descriptor_t *ldt, gdt_global_descriptor_t *gdt)
{
	/* New protected mode code: */
	c->ea.is_mem = 1;                                /* Effective address is memory operand */
	segment = e386_get_ss (c);                       /* Segment register: DS register       */
	check_ldt = segment;
	local_descriptor = ldt_get_local_segment_descriptor (ldt, check_ldt);
	check_gdt = segment;
	global_descriptor = gdt_get_global_segment_descriptor (gdt, check_gdt);
	if ((check_ldt != NULL) || (check_gdt != NULL))
	{
		/* ERROR: Segment already exists and assigned */ /* Possibly use actual CPU exception if required */
	}
	else
	{
		if (ss->ss_segment_register_selector_table_indicator (c) == 1) /* Allocate new local descriptor table: */
		{
			ldt_allocate_new_local_segment_descriptor (ldt, limit, segment);
			ldt_descriptor = ldt_get_local_segment_descriptor (ldt, segment);
			offset = e386_get_ebp (c) + e386_get_esi (c); /* Segment offset  : eBX and eSI       */
			ldt_descriptor->ldt_segment_base_address_offset = offset;
			offset += e386_mk_uint32 (c->ea.data[1], c->ea.data[2]); /* Segment offset  : eBX and eSI       */
			ldt_descriptor->ldt_segment_base_address_offset = offset;
			ldt_descriptor->ldt_segment_is_present = ss->ss_segment_register_descriptor_is_present;
			ldt_descriptor->ldt_granularity_bit = ss->ss_segment_register_descriptor_granularity_bit;
			ldt_descriptor->ldt_big = ss->ss_segment_register_descriptor_big;
			ldt_descriptor->ldt_x = ss->ss_segment_register_descriptor_x_;
			ldt_descriptor->ldt_0_ = ss->ss_segment_register_descriptor_0_;
			ldt_descriptor->ldt_segment_available = ss->ss_segment_register_descriptor_available;
			ldt_descriptor->ldt_descriptor_privilege_level = ss->ss_segment_register_descriptor_privilege_level;
			ldt_descriptor->ldt_type_1 = ss->ss_segment_register_descriptor_type_1;
			ldt_descriptor->ldt_type_0 = ss->ss_segment_register_descriptor_type_0;
			ldt_descriptor->ldt_type_c = ss->ss_segment_register_descriptor_type_c;
			ldt_descriptor->ldt_type_e = ss->ss_segment_register_descriptor_type_e;
			ldt_descriptor->ldt_type_r = ss->ss_segment_register_descriptor_type_r;
			ldt_descriptor->ldt_type_w = ss->ss_segment_register_descriptor_type_w;
			ldt_descriptor->ldt_segment_accessed = ss->ss_segment_register_descriptor_accessed;
		}
		else /* Allocate new global descriptor table: */
		{
			gdt_allocate_new_global_segment_descriptor (gdt, limit, segment);
			gdt_descriptor = gdt_get_global_segment_descriptor (gdt, segment);
			offset = e386_get_ebp (c) + e386_get_esi (c); /* Segment offset  : eBX and eSI       */
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			offset += e386_mk_uint32 (c->ea.data[1], c->ea.data[2]); /* Segment offset  : eBX and eSI       */
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_is_present = ss->ss_segment_register_descriptor_is_present;
			gdt_descriptor->gdt_granularity_bit = ss->ss_segment_register_descriptor_granularity_bit;
			gdt_descriptor->gdt_big = ss->ss_segment_register_descriptor_big;
			gdt_descriptor->gdt_x = ss->ss_segment_register_descriptor_x_;
			gdt_descriptor->gdt_0_ = ss->ss_segment_register_descriptor_0_;
			gdt_descriptor->gdt_segment_available = ss->ss_segment_register_descriptor_available;
			gdt_descriptor->gdt_descriptor_privilege_level = ss->ss_segment_register_descriptor_privilege_level;
			gdt_descriptor->gdt_type_1 = ss->ss_segment_register_descriptor_type_1;
			gdt_descriptor->gdt_type_0 = ss->ss_segment_register_descriptor_type_0;
			gdt_descriptor->gdt_type_c = ss->ss_segment_register_descriptor_type_c;
			gdt_descriptor->gdt_type_e = ss->ss_segment_register_descriptor_type_e;
			gdt_descriptor->gdt_type_r = ss->ss_segment_register_descriptor_type_r;
			gdt_descriptor->gdt_type_w = ss->ss_segment_register_descriptor_type_w;
			gdt_descriptor->gdt_segment_accessed = ss->ss_segment_register_descriptor_accessed;
		}
	}
}

/* EA 13: [BP + DI + XXXX] or [eBP + eDI + XXXX] */ /* Real Mode */
static
void ea386_real_mode_get13 (e8086_t *c)
{
	c->ea.is_mem = 1;                                           /* Effective address is memory operand                                      */
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.seg = e386_get_ss (c);                                /* Segment register: DS register                                            */
		c->ea.ofs = e386_get_ebp (c) + e386_get_edi (c);            /* Segment offset  : eBX and eSI                                            */
		c->ea.ofs += e386_mk_uint32 (c->ea.data[1], c->ea.data[2]); /* Additional segment offset: 32-bit signed integer added to previous value */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.seg = e386_get_ss (c) & 0xfffff;                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_ebp (c) + e386_get_edi (c) & 0xfffff;
		c->ea.ofs += e386_mk_uint32 (c->ea.data[1], c->ea.data[2]) & 0xfffff; /* Segment offset  : eBX and eSI       */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 3;
	c->delay += 11;
}

/* EA 13: [BP + DI + XXXX] or [eBP + eDI + XXXX] */ /* Virtual 8086 Mode */
static
void ea386_virtual_8086_mode_get13 (e8086_t *c)
{
	c->ea.is_mem = 1;                                           /* Effective address is memory operand                                      */
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.seg = e386_get_ss (c);                                /* Segment register: DS register                                            */
		c->ea.ofs = e386_get_ebp (c) + e386_get_edi (c);            /* Segment offset  : eBX and eSI                                            */
		c->ea.ofs += e386_mk_uint32 (c->ea.data[1], c->ea.data[2]); /* Additional segment offset: 32-bit signed integer added to previous value */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.seg = e386_get_ss (c) & 0xfffff;                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_ebp (c) + e386_get_edi (c) & 0xfffff;
		c->ea.ofs += e386_mk_uint32 (c->ea.data[1], c->ea.data[2]) & 0xfffff; /* Segment offset  : eBX and eSI       */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 3;
	c->delay += 11;
}

/* EA 13: [BP + DI + XXXX] or [eBP + eDI + XXXX] */ /* Protected Mode */
static
void ea386_protected_mode_get13 (e8086_t *c, ss_segment_register_descriptor_t *ss, ldt_local_descriptor_t *ldt, gdt_global_descriptor_t *gdt)
{
	/* New protected mode code: */
	c->ea.is_mem = 1;                                /* Effective address is memory operand */
	segment = e386_get_ss (c);                       /* Segment register: DS register       */
	check_ldt = segment;
	local_descriptor = ldt_get_local_segment_descriptor (ldt, check_ldt);
	check_gdt = segment;
	global_descriptor = gdt_get_global_segment_descriptor (gdt, check_gdt);
	if ((check_ldt != NULL) || (check_gdt != NULL))
	{
		/* ERROR: Segment already exists and assigned */ /* Possibly use actual CPU exception if required */
	}
	else
	{
		if (ss->ss_segment_register_selector_table_indicator (c) == 1) /* Allocate new local descriptor table: */
		{
			ldt_allocate_new_local_segment_descriptor (ldt, limit, segment);
			ldt_descriptor = ldt_get_local_segment_descriptor (ldt, segment);
			offset = e386_get_ebp (c) + e386_get_edi (c); /* Segment offset  : eBX and eSI       */
			ldt_descriptor->ldt_segment_base_address_offset = offset;
			offset += e386_mk_uint32 (c->ea.data[1], c->ea.data[2]); /* Segment offset  : eBX and eSI       */
			ldt_descriptor->ldt_segment_base_address_offset = offset;
			ldt_descriptor->ldt_segment_is_present = ss->ss_segment_register_descriptor_is_present;
			ldt_descriptor->ldt_granularity_bit = ss->ss_segment_register_descriptor_granularity_bit;
			ldt_descriptor->ldt_big = ss->ss_segment_register_descriptor_big;
			ldt_descriptor->ldt_x = ss->ss_segment_register_descriptor_x_;
			ldt_descriptor->ldt_0_ = ss->ss_segment_register_descriptor_0_;
			ldt_descriptor->ldt_segment_available = ss->ss_segment_register_descriptor_available;
			ldt_descriptor->ldt_descriptor_privilege_level = ss->ss_segment_register_descriptor_privilege_level;
			ldt_descriptor->ldt_type_1 = ss->ss_segment_register_descriptor_type_1;
			ldt_descriptor->ldt_type_0 = ss->ss_segment_register_descriptor_type_0;
			ldt_descriptor->ldt_type_c = ss->ss_segment_register_descriptor_type_c;
			ldt_descriptor->ldt_type_e = ss->ss_segment_register_descriptor_type_e;
			ldt_descriptor->ldt_type_r = ss->ss_segment_register_descriptor_type_r;
			ldt_descriptor->ldt_type_w = ss->ss_segment_register_descriptor_type_w;
			ldt_descriptor->ldt_segment_accessed = ss->ss_segment_register_descriptor_accessed;
		}
		else /* Allocate new global descriptor table: */
		{
			gdt_allocate_new_global_segment_descriptor (gdt, limit, segment);
			gdt_descriptor = gdt_get_global_segment_descriptor (gdt, segment);
			offset = e386_get_ebp (c) + e386_get_edi (c); /* Segment offset  : eBX and eSI       */
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			offset += e386_mk_uint32 (c->ea.data[1], c->ea.data[2]); /* Segment offset  : eBX and eSI       */
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_is_present = ss->ss_segment_register_descriptor_is_present;
			gdt_descriptor->gdt_granularity_bit = ss->ss_segment_register_descriptor_granularity_bit;
			gdt_descriptor->gdt_big = ss->ss_segment_register_descriptor_big;
			gdt_descriptor->gdt_x = ss->ss_segment_register_descriptor_x_;
			gdt_descriptor->gdt_0_ = ss->ss_segment_register_descriptor_0_;
			gdt_descriptor->gdt_segment_available = ss->ss_segment_register_descriptor_available;
			gdt_descriptor->gdt_descriptor_privilege_level = ss->ss_segment_register_descriptor_privilege_level;
			gdt_descriptor->gdt_type_1 = ss->ss_segment_register_descriptor_type_1;
			gdt_descriptor->gdt_type_0 = ss->ss_segment_register_descriptor_type_0;
			gdt_descriptor->gdt_type_c = ss->ss_segment_register_descriptor_type_c;
			gdt_descriptor->gdt_type_e = ss->ss_segment_register_descriptor_type_e;
			gdt_descriptor->gdt_type_r = ss->ss_segment_register_descriptor_type_r;
			gdt_descriptor->gdt_type_w = ss->ss_segment_register_descriptor_type_w;
			gdt_descriptor->gdt_segment_accessed = ss->ss_segment_register_descriptor_accessed;
		}
	}
}

/* EA 14: [SI + XXXX] or [eSI + XXXX] */ /* Real Mode */
static
void ea386_real_mode_get14 (e8086_t *c)
{
	c->ea.is_mem = 1;                                           /* Effective address is memory operand                                      */
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.seg = e386_get_ss (c);                                /* Segment register: DS register                                            */
		c->ea.ofs = e386_get_esi (c);            /* Segment offset  : eBX and eSI                                            */
		c->ea.ofs += e386_mk_uint32 (c->ea.data[1], c->ea.data[2]); /* Additional segment offset: 32-bit signed integer added to previous value */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.seg = e386_get_ss (c) & 0xfffff;                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_esi (c) & 0xfffff;
		c->ea.ofs += e386_mk_uint32 (c->ea.data[1], c->ea.data[2]) & 0xfffff; /* Segment offset  : eBX and eSI       */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 3;
	c->delay += 9;
}

/* EA 14: [SI + XXXX] or [eSI + XXXX] */ /* Virtual 8086 Mode */
static
void ea386_virtual_8086_mode_get14 (e8086_t *c)
{
	c->ea.is_mem = 1;                                           /* Effective address is memory operand                                      */
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.seg = e386_get_ss (c);                                /* Segment register: DS register                                            */
		c->ea.ofs = e386_get_esi (c);            /* Segment offset  : eBX and eSI                                            */
		c->ea.ofs += e386_mk_uint32 (c->ea.data[1], c->ea.data[2]); /* Additional segment offset: 32-bit signed integer added to previous value */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.seg = e386_get_ss (c) & 0xfffff;                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_esi (c) & 0xfffff;
		c->ea.ofs += e386_mk_uint32 (c->ea.data[1], c->ea.data[2]) & 0xfffff; /* Segment offset  : eBX and eSI       */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 3;
	c->delay += 9;
}

/* EA 14: [SI + XXXX] or [eSI + XXXX] */ /* Protected Mode */
static
void ea386_protected_mode_get14 (e8086_t *c, ss_segment_register_descriptor_t *ss, ldt_local_descriptor_t *ldt, gdt_global_descriptor_t *gdt)
{
	/* New protected mode code: */
	c->ea.is_mem = 1;                                /* Effective address is memory operand */
	segment = e386_get_ss (c);                       /* Segment register: DS register       */
	check_ldt = segment;
	local_descriptor = ldt_get_local_segment_descriptor (ldt, check_ldt);
	check_gdt = segment;
	global_descriptor = gdt_get_global_segment_descriptor (gdt, check_gdt);
	if ((check_ldt != NULL) || (check_gdt != NULL))
	{
		/* ERROR: Segment already exists and assigned */ /* Possibly use actual CPU exception if required */
	}
	else
	{
		if (ss->ss_segment_register_selector_table_indicator (c) == 1) /* Allocate new local descriptor table: */
		{
			ldt_allocate_new_local_segment_descriptor (ldt, limit, segment);
			ldt_descriptor = ldt_get_local_segment_descriptor (ldt, segment);
			offset = e386_get_esi (c); /* Segment offset  : eBX and eSI       */
			ldt_descriptor->ldt_segment_base_address_offset = offset;
			offset += e386_mk_uint32 (c->ea.data[1], c->ea.data[2]); /* Segment offset  : eBX and eSI       */
			ldt_descriptor->ldt_segment_base_address_offset = offset;
			ldt_descriptor->ldt_segment_is_present = ss->ss_segment_register_descriptor_is_present;
			ldt_descriptor->ldt_granularity_bit = ss->ss_segment_register_descriptor_granularity_bit;
			ldt_descriptor->ldt_big = ss->ss_segment_register_descriptor_big;
			ldt_descriptor->ldt_x = ss->ss_segment_register_descriptor_x_;
			ldt_descriptor->ldt_0_ = ss->ss_segment_register_descriptor_0_;
			ldt_descriptor->ldt_segment_available = ss->ss_segment_register_descriptor_available;
			ldt_descriptor->ldt_descriptor_privilege_level = ss->ss_segment_register_descriptor_privilege_level;
			ldt_descriptor->ldt_type_1 = ss->ss_segment_register_descriptor_type_1;
			ldt_descriptor->ldt_type_0 = ss->ss_segment_register_descriptor_type_0;
			ldt_descriptor->ldt_type_c = ss->ss_segment_register_descriptor_type_c;
			ldt_descriptor->ldt_type_e = ss->ss_segment_register_descriptor_type_e;
			ldt_descriptor->ldt_type_r = ss->ss_segment_register_descriptor_type_r;
			ldt_descriptor->ldt_type_w = ss->ss_segment_register_descriptor_type_w;
			ldt_descriptor->ldt_segment_accessed = ss->ss_segment_register_descriptor_accessed;
		}
		else /* Allocate new global descriptor table: */
		{
			gdt_allocate_new_global_segment_descriptor (gdt, limit, segment);
			gdt_descriptor = gdt_get_global_segment_descriptor (gdt, segment);
			offset = e386_get_esi (c); /* Segment offset  : eBX and eSI       */
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			offset += e386_mk_uint32 (c->ea.data[1], c->ea.data[2]); /* Segment offset  : eBX and eSI       */
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_is_present = ss->ss_segment_register_descriptor_is_present;
			gdt_descriptor->gdt_granularity_bit = ss->ss_segment_register_descriptor_granularity_bit;
			gdt_descriptor->gdt_big = ss->ss_segment_register_descriptor_big;
			gdt_descriptor->gdt_x = ss->ss_segment_register_descriptor_x_;
			gdt_descriptor->gdt_0_ = ss->ss_segment_register_descriptor_0_;
			gdt_descriptor->gdt_segment_available = ss->ss_segment_register_descriptor_available;
			gdt_descriptor->gdt_descriptor_privilege_level = ss->ss_segment_register_descriptor_privilege_level;
			gdt_descriptor->gdt_type_1 = ss->ss_segment_register_descriptor_type_1;
			gdt_descriptor->gdt_type_0 = ss->ss_segment_register_descriptor_type_0;
			gdt_descriptor->gdt_type_c = ss->ss_segment_register_descriptor_type_c;
			gdt_descriptor->gdt_type_e = ss->ss_segment_register_descriptor_type_e;
			gdt_descriptor->gdt_type_r = ss->ss_segment_register_descriptor_type_r;
			gdt_descriptor->gdt_type_w = ss->ss_segment_register_descriptor_type_w;
			gdt_descriptor->gdt_segment_accessed = ss->ss_segment_register_descriptor_accessed;
		}
	}
}

/* EA 15: [DI + XXXX] or [eDI + XXXX] */ /* Real Mode */
static
void ea386_real_mode_get15 (e8086_t *c)
{
	c->ea.is_mem = 1;                                           /* Effective address is memory operand                                      */
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.seg = e386_get_ds (c);                                /* Segment register: DS register                                            */
		c->ea.ofs = e386_get_edi (c);            /* Segment offset  : eBX and eSI                                            */
		c->ea.ofs += e386_mk_uint32 (c->ea.data[1], c->ea.data[2]); /* Additional segment offset: 32-bit signed integer added to previous value */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.seg = e386_get_ds (c) & 0xfffff;                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_edi (c) & 0xfffff;
		c->ea.ofs += e386_mk_uint32 (c->ea.data[1], c->ea.data[2]) & 0xfffff; /* Segment offset  : eBX and eSI       */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 3;
	c->delay += 9;
}

/* EA 15: [DI + XXXX] or [eDI + XXXX] */ /* Virtual 8086 Mode */
static
void ea386_virtual_8086_mode_get15 (e8086_t *c)
{
	c->ea.is_mem = 1;                                           /* Effective address is memory operand                                      */
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.seg = e386_get_ds (c);                                /* Segment register: DS register                                            */
		c->ea.ofs = e386_get_edi (c);            /* Segment offset  : eBX and eSI                                            */
		c->ea.ofs += e386_mk_uint32 (c->ea.data[1], c->ea.data[2]); /* Additional segment offset: 32-bit signed integer added to previous value */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.seg = e386_get_ds (c) & 0xfffff;                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_edi (c) & 0xfffff;
		c->ea.ofs += e386_mk_uint32 (c->ea.data[1], c->ea.data[2]) & 0xfffff; /* Segment offset  : eBX and eSI       */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 3;
	c->delay += 9;
}

/* EA 15: [DI + XXXX] or [eDI + XXXX] */ /* Protected Mode */
static
void ea386_protected_mode_get15 (e8086_t *c, ss_segment_register_descriptor_t *ss, ldt_local_descriptor_t *ldt, gdt_global_descriptor_t *gdt)
{
	/* New protected mode code: */
	c->ea.is_mem = 1;                                /* Effective address is memory operand */
	segment = e386_get_ss (c);                       /* Segment register: DS register       */
	check_ldt = segment;
	local_descriptor = ldt_get_local_segment_descriptor (ldt, check_ldt);
	check_gdt = segment;
	global_descriptor = gdt_get_global_segment_descriptor (gdt, check_gdt);
	if ((check_ldt != NULL) || (check_gdt != NULL))
	{
		/* ERROR: Segment already exists and assigned */ /* Possibly use actual CPU exception if required */
	}
	else
	{
		if (ss->ss_segment_register_selector_table_indicator (c) == 1) /* Allocate new local descriptor table: */
		{
			ldt_allocate_new_local_segment_descriptor (ldt, limit, segment);
			ldt_descriptor = ldt_get_local_segment_descriptor (ldt, segment);
			offset = e386_get_edi (c); /* Segment offset  : eBX and eSI       */
			ldt_descriptor->ldt_segment_base_address_offset = offset;
			offset += e386_mk_uint32 (c->ea.data[1], c->ea.data[2]); /* Segment offset  : eBX and eSI       */
			ldt_descriptor->ldt_segment_base_address_offset = offset;
			ldt_descriptor->ldt_segment_is_present = ss->ss_segment_register_descriptor_is_present;
			ldt_descriptor->ldt_granularity_bit = ss->ss_segment_register_descriptor_granularity_bit;
			ldt_descriptor->ldt_big = ss->ss_segment_register_descriptor_big;
			ldt_descriptor->ldt_x = ss->ss_segment_register_descriptor_x_;
			ldt_descriptor->ldt_0_ = ss->ss_segment_register_descriptor_0_;
			ldt_descriptor->ldt_segment_available = ss->ss_segment_register_descriptor_available;
			ldt_descriptor->ldt_descriptor_privilege_level = ss->ss_segment_register_descriptor_privilege_level;
			ldt_descriptor->ldt_type_1 = ss->ss_segment_register_descriptor_type_1;
			ldt_descriptor->ldt_type_0 = ss->ss_segment_register_descriptor_type_0;
			ldt_descriptor->ldt_type_c = ss->ss_segment_register_descriptor_type_c;
			ldt_descriptor->ldt_type_e = ss->ss_segment_register_descriptor_type_e;
			ldt_descriptor->ldt_type_r = ss->ss_segment_register_descriptor_type_r;
			ldt_descriptor->ldt_type_w = ss->ss_segment_register_descriptor_type_w;
			ldt_descriptor->ldt_segment_accessed = ss->ss_segment_register_descriptor_accessed;
		}
		else /* Allocate new global descriptor table: */
		{
			gdt_allocate_new_global_segment_descriptor (gdt, limit, segment);
			gdt_descriptor = gdt_get_global_segment_descriptor (gdt, segment);
			offset = e386_get_edi (c); /* Segment offset  : eBX and eSI       */
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			offset += e386_mk_uint32 (c->ea.data[1], c->ea.data[2]); /* Segment offset  : eBX and eSI       */
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_is_present = ss->ss_segment_register_descriptor_is_present;
			gdt_descriptor->gdt_granularity_bit = ss->ss_segment_register_descriptor_granularity_bit;
			gdt_descriptor->gdt_big = ss->ss_segment_register_descriptor_big;
			gdt_descriptor->gdt_x = ss->ss_segment_register_descriptor_x_;
			gdt_descriptor->gdt_0_ = ss->ss_segment_register_descriptor_0_;
			gdt_descriptor->gdt_segment_available = ss->ss_segment_register_descriptor_available;
			gdt_descriptor->gdt_descriptor_privilege_level = ss->ss_segment_register_descriptor_privilege_level;
			gdt_descriptor->gdt_type_1 = ss->ss_segment_register_descriptor_type_1;
			gdt_descriptor->gdt_type_0 = ss->ss_segment_register_descriptor_type_0;
			gdt_descriptor->gdt_type_c = ss->ss_segment_register_descriptor_type_c;
			gdt_descriptor->gdt_type_e = ss->ss_segment_register_descriptor_type_e;
			gdt_descriptor->gdt_type_r = ss->ss_segment_register_descriptor_type_r;
			gdt_descriptor->gdt_type_w = ss->ss_segment_register_descriptor_type_w;
			gdt_descriptor->gdt_segment_accessed = ss->ss_segment_register_descriptor_accessed;
		}
	}
}

/* EA 16: [BP + XXXX] or [eBP + XXXX] */ /* Real Mode */
static
void ea386_real_mode_get16 (e8086_t *c)
{
	c->ea.is_mem = 1;                                           /* Effective address is memory operand                                      */
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.seg = e386_get_ss (c);                                /* Segment register: DS register                                            */
		c->ea.ofs = e386_get_ebp (c);            /* Segment offset  : eBX and eSI                                            */
		c->ea.ofs += e386_mk_uint32 (c->ea.data[1], c->ea.data[2]); /* Additional segment offset: 32-bit signed integer added to previous value */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.seg = e386_get_ss (c) & 0xfffff;                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_ebp (c) & 0xfffff;
		c->ea.ofs += e386_mk_uint32 (c->ea.data[1], c->ea.data[2]) & 0xfffff; /* Segment offset  : eBX and eSI       */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 3;
	c->delay += 9;
}

/* EA 16: [BP + XXXX] or [eBP + XXXX] */ /* Virtual 8086 Mode */
static
void ea386_virtual_8086_mode_get16 (e8086_t *c)
{
	c->ea.is_mem = 1;                                           /* Effective address is memory operand                                      */
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.seg = e386_get_ss (c);                                /* Segment register: DS register                                            */
		c->ea.ofs = e386_get_ebp (c);            /* Segment offset  : eBX and eSI                                            */
		c->ea.ofs += e386_mk_uint32 (c->ea.data[1], c->ea.data[2]); /* Additional segment offset: 32-bit signed integer added to previous value */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.seg = e386_get_ss (c) & 0xfffff;                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_ebp (c) & 0xfffff;
		c->ea.ofs += e386_mk_uint32 (c->ea.data[1], c->ea.data[2]) & 0xfffff; /* Segment offset  : eBX and eSI       */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 3;
	c->delay += 9;
}

/* EA 16: [BP + XXXX] or [eBP + XXXX] */ /* Protected Mode */
static
void ea386_protected_mode_get16 (e8086_t *c, ss_segment_register_descriptor_t *ss, ldt_local_descriptor_t *ldt, gdt_global_descriptor_t *gdt)
{
	/* New protected mode code: */
	c->ea.is_mem = 1;                                /* Effective address is memory operand */
	segment = e386_get_ss (c);                       /* Segment register: DS register       */
	check_ldt = segment;
	local_descriptor = ldt_get_local_segment_descriptor (ldt, check_ldt);
	check_gdt = segment;
	global_descriptor = gdt_get_global_segment_descriptor (gdt, check_gdt);
	if ((check_ldt != NULL) || (check_gdt != NULL))
	{
		/* ERROR: Segment already exists and assigned */ /* Possibly use actual CPU exception if required */
	}
	else
	{
		if (ss->ss_segment_register_selector_table_indicator (c) == 1) /* Allocate new local descriptor table: */
		{
			ldt_allocate_new_local_segment_descriptor (ldt, limit, segment);
			ldt_descriptor = ldt_get_local_segment_descriptor (ldt, segment);
			offset = e386_get_ebp (c); /* Segment offset  : eBX and eSI       */
			ldt_descriptor->ldt_segment_base_address_offset = offset;
			offset += e386_mk_uint32 (c->ea.data[1], c->ea.data[2]); /* Segment offset  : eBX and eSI       */
			ldt_descriptor->ldt_segment_base_address_offset = offset;
			ldt_descriptor->ldt_segment_is_present = ss->ss_segment_register_descriptor_is_present;
			ldt_descriptor->ldt_granularity_bit = ss->ss_segment_register_descriptor_granularity_bit;
			ldt_descriptor->ldt_big = ss->ss_segment_register_descriptor_big;
			ldt_descriptor->ldt_x = ss->ss_segment_register_descriptor_x_;
			ldt_descriptor->ldt_0_ = ss->ss_segment_register_descriptor_0_;
			ldt_descriptor->ldt_segment_available = ss->ss_segment_register_descriptor_available;
			ldt_descriptor->ldt_descriptor_privilege_level = ss->ss_segment_register_descriptor_privilege_level;
			ldt_descriptor->ldt_type_1 = ss->ss_segment_register_descriptor_type_1;
			ldt_descriptor->ldt_type_0 = ss->ss_segment_register_descriptor_type_0;
			ldt_descriptor->ldt_type_c = ss->ss_segment_register_descriptor_type_c;
			ldt_descriptor->ldt_type_e = ss->ss_segment_register_descriptor_type_e;
			ldt_descriptor->ldt_type_r = ss->ss_segment_register_descriptor_type_r;
			ldt_descriptor->ldt_type_w = ss->ss_segment_register_descriptor_type_w;
			ldt_descriptor->ldt_segment_accessed = ss->ss_segment_register_descriptor_accessed;
		}
		else /* Allocate new global descriptor table: */
		{
			gdt_allocate_new_global_segment_descriptor (gdt, limit, segment);
			gdt_descriptor = gdt_get_global_segment_descriptor (gdt, segment);
			offset = e386_get_ebp (c); /* Segment offset  : eBX and eSI       */
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			offset += e386_mk_uint32 (c->ea.data[1], c->ea.data[2]); /* Segment offset  : eBX and eSI       */
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_is_present = ss->ss_segment_register_descriptor_is_present;
			gdt_descriptor->gdt_granularity_bit = ss->ss_segment_register_descriptor_granularity_bit;
			gdt_descriptor->gdt_big = ss->ss_segment_register_descriptor_big;
			gdt_descriptor->gdt_x = ss->ss_segment_register_descriptor_x_;
			gdt_descriptor->gdt_0_ = ss->ss_segment_register_descriptor_0_;
			gdt_descriptor->gdt_segment_available = ss->ss_segment_register_descriptor_available;
			gdt_descriptor->gdt_descriptor_privilege_level = ss->ss_segment_register_descriptor_privilege_level;
			gdt_descriptor->gdt_type_1 = ss->ss_segment_register_descriptor_type_1;
			gdt_descriptor->gdt_type_0 = ss->ss_segment_register_descriptor_type_0;
			gdt_descriptor->gdt_type_c = ss->ss_segment_register_descriptor_type_c;
			gdt_descriptor->gdt_type_e = ss->ss_segment_register_descriptor_type_e;
			gdt_descriptor->gdt_type_r = ss->ss_segment_register_descriptor_type_r;
			gdt_descriptor->gdt_type_w = ss->ss_segment_register_descriptor_type_w;
			gdt_descriptor->gdt_segment_accessed = ss->ss_segment_register_descriptor_accessed;
		}
	}
}

/* EA 17: [BX + XXXX] or [eBX + XXXX] */ /* Real Mode */
static
void ea386_real_mode_get17 (e8086_t *c)
{
	c->ea.is_mem = 1;                                           /* Effective address is memory operand                                      */
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.seg = e386_get_ds (c);                                /* Segment register: DS register                                            */
		c->ea.ofs = e386_get_ebx (c);            /* Segment offset  : eBX and eSI                                            */
		c->ea.ofs += e386_mk_uint32 (c->ea.data[1], c->ea.data[2]); /* Additional segment offset: 32-bit signed integer added to previous value */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.seg = e386_get_ds (c) & 0xfffff;                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_ebx (c) & 0xfffff;
		c->ea.ofs += e386_mk_uint32 (c->ea.data[1], c->ea.data[2]) & 0xfffff; /* Segment offset  : eBX and eSI       */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 3;
	c->delay += 9;
}

/* EA 17: [BX + XXXX] or [eBX + XXXX] */ /* Virtual 8086 Mode */
static
void ea386_virtual_8086_mode_get17 (e8086_t *c)
{
	c->ea.is_mem = 1;                                           /* Effective address is memory operand                                      */
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.seg = e386_get_ds (c);                                /* Segment register: DS register                                            */
		c->ea.ofs = e386_get_ebx (c);            /* Segment offset  : eBX and eSI                                            */
		c->ea.ofs += e386_mk_uint32 (c->ea.data[1], c->ea.data[2]); /* Additional segment offset: 32-bit signed integer added to previous value */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.seg = e386_get_ds (c) & 0xfffff;                     /* Segment register: DS register       */
		c->ea.ofs = e386_get_ebx (c) & 0xfffff;
		c->ea.ofs += e386_mk_uint32 (c->ea.data[1], c->ea.data[2]) & 0xfffff; /* Segment offset  : eBX and eSI       */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 3;
	c->delay += 9;
}

/* EA 17: [BX + XXXX] or [eBX + XXXX] */ /* Protected Mode */
static
void ea386_protected_mode_get17 (e8086_t *c, ds_segment_register_descriptor_t *ds, ldt_local_descriptor_t *ldt, gdt_global_descriptor_t *gdt)
{
	/* New protected mode code: */
	c->ea.is_mem = 1;                                /* Effective address is memory operand */
	segment = e386_get_ds (c);                       /* Segment register: DS register       */
	check_ldt = segment;
	local_descriptor = ldt_get_local_segment_descriptor (ldt, check_ldt);
	check_gdt = segment;
	global_descriptor = gdt_get_global_segment_descriptor (gdt, check_gdt);
	if ((check_ldt != NULL) || (check_gdt != NULL))
	{
		/* ERROR: Segment already exists and assigned */ /* Possibly use actual CPU exception if required */
	}
	else
	{
		if (ss->ss_segment_register_selector_table_indicator (c) == 1) /* Allocate new local descriptor table: */
		{
			ldt_allocate_new_local_segment_descriptor (ldt, limit, segment);
			ldt_descriptor = ldt_get_local_segment_descriptor (ldt, segment);
			offset = e386_get_ebx (c); /* Segment offset  : eBX and eSI       */
			ldt_descriptor->ldt_segment_base_address_offset = offset;
			offset += e386_mk_uint32 (c->ea.data[1], c->ea.data[2]); /* Segment offset  : eBX and eSI       */
			ldt_descriptor->ldt_segment_base_address_offset = offset;
			ldt_descriptor->ldt_segment_is_present = ds->ds_segment_register_descriptor_is_present;
			ldt_descriptor->ldt_granularity_bit = ds->ds_segment_register_descriptor_granularity_bit;
			ldt_descriptor->ldt_big = ds->ds_segment_register_descriptor_big;
			ldt_descriptor->ldt_x = ds->ds_segment_register_descriptor_x_;
			ldt_descriptor->ldt_0_ = ds->ds_segment_register_descriptor_0_;
			ldt_descriptor->ldt_segment_available = ds->ds_segment_register_descriptor_available;
			ldt_descriptor->ldt_descriptor_privilege_level = ds->ds_segment_register_descriptor_privilege_level;
			ldt_descriptor->ldt_type_1 = ds->ds_segment_register_descriptor_type_1;
			ldt_descriptor->ldt_type_0 = ds->ds_segment_register_descriptor_type_0;
			ldt_descriptor->ldt_type_c = ds->ds_segment_register_descriptor_type_c;
			ldt_descriptor->ldt_type_e = ds->ds_segment_register_descriptor_type_e;
			ldt_descriptor->ldt_type_r = ds->ds_segment_register_descriptor_type_r;
			ldt_descriptor->ldt_type_w = ds->ds_segment_register_descriptor_type_w;
			ldt_descriptor->ldt_segment_accessed = ds->ds_segment_register_descriptor_accessed;
		}
		else /* Allocate new global descriptor table: */
		{
			gdt_allocate_new_global_segment_descriptor (gdt, limit, segment);
			gdt_descriptor = gdt_get_global_segment_descriptor (gdt, segment);
			offset = e386_get_ebx (c); /* Segment offset  : eBX and eSI       */
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			offset += e386_mk_uint32 (c->ea.data[1], c->ea.data[2]); /* Segment offset  : eBX and eSI       */
			gdt_descriptor->gdt_segment_base_address_offset = offset;
			gdt_descriptor->gdt_segment_is_present = ds->ds_segment_register_descriptor_is_present;
			gdt_descriptor->gdt_granularity_bit = ds->ds_segment_register_descriptor_granularity_bit;
			gdt_descriptor->gdt_big = ds->ds_segment_register_descriptor_big;
			gdt_descriptor->gdt_x = ds->ds_segment_register_descriptor_x_;
			gdt_descriptor->gdt_0_ = ds->ds_segment_register_descriptor_0_;
			gdt_descriptor->gdt_segment_available = ds->ds_segment_register_descriptor_available;
			gdt_descriptor->gdt_descriptor_privilege_level = ds->ds_segment_register_descriptor_privilege_level;
			gdt_descriptor->gdt_type_1 = ds->ds_segment_register_descriptor_type_1;
			gdt_descriptor->gdt_type_0 = ds->ds_segment_register_descriptor_type_0;
			gdt_descriptor->gdt_type_c = ds->ds_segment_register_descriptor_type_c;
			gdt_descriptor->gdt_type_e = ds->ds_segment_register_descriptor_type_e;
			gdt_descriptor->gdt_type_r = ds->ds_segment_register_descriptor_type_r;
			gdt_descriptor->gdt_type_w = ds->ds_segment_register_descriptor_type_w;
			gdt_descriptor->gdt_segment_accessed = ds->ds_segment_register_descriptor_accessed;
		}
	}
}

/* EA 18-1F: REG */ /* Real Mode */
static
void ea386_real_mode_get18 (e8086_t *c)
{
	c->ea.is_mem = 0;              /* Effective address is not memory operand */
	c->ea.seg = 0;                 /* Segment register: None                  */
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.ofs = c->ea.data[0] & 15; /* Segment offset  : Data                  */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.ofs = c->ea.data[0] & 7; /* Segment offset  : Data                  */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 1;
	c->delay = 0;
}

/* EA 18-1F: REG */ /* Virtual 8086 Mode */
static
void ea386_virtual_8086_mode_get18 (e8086_t *c)
{
	c->ea.is_mem = 0;              /* Effective address is not memory operand */
	segment = 0;                 /* Segment register: None                  */
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.ofs = c->ea.data[0] & 15; /* Segment offset  : Data                  */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.ofs = c->ea.data[0] & 7; /* Segment offset  : Data                  */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 1;
	c->delay = 0;
}

/* EA 18-1F: REG */ /* Protected Mode */
static
void ea386_protected_mode_get18 (e8086_t *c, ldt_local_descriptor_t *ldt, gdt_global_descriptor_t *gdt)
{
	c->ea.is_mem = 0;              /* Effective address is not memory operand */
	segment = 0;                 /* Segment register: None                  */
	if (c->e386_address_size_prefix_67h (c) == 1) /* 32-bit memory operand specified */
	{
		c->ea.ofs = c->ea.data[0] & 15; /* Segment offset  : Data                  */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		c->ea.ofs = c->ea.data[0] & 7; /* Segment offset  : Data                  */
	}
	c->e386_address_size_prefix_67h = 0;
	c->ea.cnt = 1;
	c->delay = 0;
}

e386_ea_f e386_ea[96] = {
	&ea386_real_mode_get00, &ea386_real_mode_get01, &ea386_real_mode_get02, &ea386_real_mode_get03,
	&ea386_virtual_8086_mode_get00, &ea386_virtual_8086_mode_get01, &ea386_virtual_8086_mode_get02, &ea386_virtual_8086_mode_get03,
	&ea386_protected_mode_get00, &ea386_protected_mode_get01, &ea386_protected_mode_get02, &ea386_protected_mode_get03,
	&ea386_real_mode_get04, &ea386_real_mode_get05, &ea386_real_mode_get06, &ea386_real_mode_get07,
	&ea386_virtual_8086_mode_get04, &ea386_virtual_8086_mode_get05, &ea386_virtual_8086_mode_get06, &ea386_virtual_8086_mode_get07,
	&ea386_protected_mode_get04, &ea386_protected_mode_get05, &ea386_protected_mode_get06, &ea386_protected_mode_get07,
	&ea386_real_mode_get08, &ea386_real_mode_get09, &ea386_real_mode_get0a, &ea386_real_mode_get0b,
	&ea386_virtual_8086_mode_get08, &ea386_virtual_8086_mode_get09, &ea386_virtual_8086_mode_get0a, &ea386_virtual_8086_mode_get0b,
	&ea386_protected_mode_get08, &ea386_protected_mode_get09, &ea386_protected_mode_get0a, &ea386_protected_mode_get0b,
	&ea386_real_mode_get0c, &ea386_real_mode_get0d, &ea386_real_mode_get0e, &ea386_real_mode_get0f,
	&ea386_virtual_8086_mode_get0c, &ea386_virtual_8086_mode_get0d, &ea386_virtual_8086_mode_get0e, &ea386_virtual_8086_mode_get0f,
	&ea386_protected_mode_get0c, &ea386_protected_mode_get0d, &ea386_protected_mode_get0e, &ea386_protected_mode_get0f,
	&ea386_real_mode_get10, &ea386_real_mode_get11, &ea386_real_mode_get12, &ea386_real_mode_get13,
	&ea386_virtual_8086_mode_get10, &ea386_virtual_8086_mode_get11, &ea386_virtual_8086_mode_get12, &ea386_virtual_8086_mode_get13,
	&ea386_protected_mode_get10, &ea386_protected_mode_get11, &ea386_protected_mode_get12, &ea386_protected_mode_get13,
	&ea386_real_mode_get14, &ea386_real_mode_get15, &ea386_real_mode_get16, &ea386_real_mode_get17,
	&ea386_virtual_8086_mode_get14, &ea386_virtual_8086_mode_get15, &ea386_virtual_8086_mode_get16, &ea386_virtual_8086_mode_get17,
	&ea386_protected_mode_get14, &ea386_protected_mode_get15, &ea386_protected_mode_get16, &ea386_protected_mode_get17,
	&ea386_real_mode_get18, &ea386_real_mode_get18, &ea386_real_mode_get18, &ea386_real_mode_get18,
	&ea386_virtual_8086_mode_get18, &ea386_virtual_8086_mode_get18, &ea386_virtual_8086_mode_get18, &ea386_virtual_8086_mode_get18,
	&ea386_protected_mode_get18, &ea386_protected_mode_get18, &ea386_protected_mode_get18, &ea386_protected_mode_get18,
	&ea386_real_mode_get18, &ea386_real_mode_get18, &ea386_real_mode_get18, &ea386_real_mode_get18
	&ea386_virtual_8086_mode_get18, &ea386_virtual_8086_mode_get18, &ea386_virtual_8086_mode_get18, &ea386_virtual_8086_mode_get18
	&ea386_protected_mode_get18, &ea386_protected_mode_get18, &ea386_protected_mode_get18, &ea386_protected_mode_get18
};

void e386_real_mode_get_ea_ptr (e8086_t *c, unsigned short *ea) /* Real Mode */
{
	/* 16-bit memory address specified */
	unsigned fea;
	c->ea.data = ea;                           /* Set effective address data to "ea"          */
	fea = (ea[0] & 7) | ((ea[0] & 0xc0) >> 3); /* Add up "ea" selector and shift right by 3   */
	e386_ea[fea] (c);                          /* Set effective address to above value        */
	c->ea.ofs &= 0xffffU;                      /* Effective address specified is 16-bit value */
	if (c->prefix & E86_PREFIX_SEG) {          /* If segment register override prefix is set: */
		c->ea.seg = c->seg_override;           /* Set "ea.seg" to another segment register    */
		c->delay += 2;                         /* Additional delay for time spent with above  */
	}
}

void e386_virtual_8086_mode_get_ea_ptr (e8086_t *c, unsigned short *ea) /* Virtual 8086 Mode */
{
	/* 16-bit memory address specified */
	unsigned fea;
	c->ea.data = ea;                           /* Set effective address data to "ea"          */
	fea = (ea[0] & 7) | ((ea[0] & 0xc0) >> 3); /* Add up "ea" selector and shift right by 3   */
	e386_ea[fea] (c);                          /* Set effective address to above value        */
	c->ea.ofs &= 0xffffU;                      /* Effective address specified is 16-bit value */
	if (c->prefix & E86_PREFIX_SEG) {          /* If segment register override prefix is set: */
		c->ea.seg = c->seg_override;           /* Set "ea.seg" to another segment register    */
		c->delay += 2;                         /* Additional delay for time spent with above  */
	}
}

void e386_protected_mode_get_ea_ptr (e8086_t *c, ldt_local_descriptor_t *ldt, gdt_global_descriptor_t *gdt, unsigned short *ea) /* Protected Mode */
{
	/* 16-bit memory address specified */
	unsigned fea;
	c->ea.data = ea;
	fea = (ea[0] & 7) | ((ea[0] & 0xc0) >> 3);
	e386_ea[fea] (c);
	c->ea.ofs &= 0xffffU;
	if (c->prefix & E86_PREFIX_SEG) {
		c->ea.seg = c->seg_override;
		c->delay += 2;
	}
}

void e386_real_mode_get_ea_ptr32 (e8086_t *c, unsigned long *ea) /* Real Mode */
{
	/* 32-bit memory address specified */
	unsigned fea;
	c->ea.data = ea;                             /* Set effective address data to "ea"          */
	fea = (ea[0] & 7) | ((ea[0] & 0xc000) >> 7);/* Add up "ea" selector and shift right by 3   *//*NOTE: Replace "7" and "3" with "15" and "7"*/
	e386_ea[fea] (c);                            /* Set effective address to above value        *//*respectively if required                   */
	c->ea.ofs &= 0xffffffffU;                    /* Effective address specified is 32-bit value */
	if (c->prefix & E86_PREFIX_SEG) {            /* If segment register override prefix is set: */
		c->ea.seg = c->seg_override;             /* Set "ea.seg" to another segment register    */
		c->delay += 2;                           /* Additional delay for time spent with above  */
	}
}

void e386_virtual_8086_mode_get_ea_ptr32 (e8086_t *c, unsigned long *ea) /* Virtual 8086 Mode */
{
	/* 32-bit memory address specified */
	unsigned fea;
	c->ea.data = ea;                             /* Set effective address data to "ea"          */
	fea = (ea[0] & 7) | ((ea[0] & 0xc000) >> 7);/* Add up "ea" selector and shift right by 3   *//*NOTE: Replace "7" and "3" with "15" and "7"*/
	e386_ea[fea] (c);                            /* Set effective address to above value        *//*respectively if required                   */
	c->ea.ofs &= 0xffffffffU;                    /* Effective address specified is 32-bit value */
	if (c->prefix & E86_PREFIX_SEG) {            /* If segment register override prefix is set: */
		c->ea.seg = c->seg_override;               /* Set "ea.seg" to another segment register    */
		c->delay += 2;                           /* Additional delay for time spent with above  */
	}
}

void e386_protected_mode_get_ea_ptr32 (e8086_t *c, ldt_local_descriptor_t *ldt, gdt_global_descriptor_t *gdt, unsigned long *ea) /* Protected Mode */
{
	/* 32-bit memory address specified */
	unsigned fea;
	c->ea.data = ea;                             /* Set effective address data to "ea"          */
	fea = (ea[0] & 7) | ((ea[0] & 0xc000) >> 7); /* Add up "ea" selector and shift right by 3   *//*NOTE: Replace "7" and "3" with "15" and "7"*/
	e386_ea[fea] (c);                            /* Set effective address to above value        *//*respectively if required                   */
	c->ea.ofs &= 0xfffffffU;                     /* Effective address specified is 32-bit value */
	if (c->prefix & E86_PREFIX_SEG) {            /* If segment register override prefix is set: */
		c->ea.seg = c->seg_override;             /* Set "ea.seg" to another segment register    */
		c->delay += 2;                           /* Additional delay for time spent with above  */
	}
}

unsigned char e386_real_mode_get_ea8 (e8086_t *c) /* Real Mode */
{
	/* 8-bit memory address specified */
	if (c->ea.is_mem) {
		return (e86_get_mem8 (c, c->ea.seg, c->ea.ofs));  /* Retrieve segment register and offset  if effective address is memory operand     */
	}
	return (e86_get_reg8 (c, c->ea.ofs));                 /* Retrieve  8-bit registers and offsets if effective address is not memory operand */
}

unsigned char e386_virtual_8086_mode_get_ea8 (e8086_t *c) /* Virtual 8086 Mode */
{
	/* 8-bit memory address specified */
	if (c->ea.is_mem) {
		return (e86_get_mem8 (c, c->ea.seg, c->ea.ofs));  /* Retrieve segment register and offset  if effective address is memory operand     */
	}
	return (e86_get_reg8 (c, c->ea.ofs));                 /* Retrieve  8-bit registers and offsets if effective address is not memory operand */
}

unsigned char e386_protected_mode_get_ea8 (e8086_t *c, ldt_local_descriptor_t *ldt, gdt_global_descriptor_t *gdt) /* Protected Mode */
{
	/* 8-bit memory address specified */
	if (c->ea.is_mem) {
		return (ldt_get_local_segment_descriptor (ldt, segment, offset & 0xff));   /* Retrieve segment register and offset  if effective address is memory operand     */
		return (gdt_get_global_segment_descriptor (gdt, segment, offset & 0xff));   /* Retrieve segment register and offset  if effective address is memory operand     */
	}
	return (e86_get_reg8 (c, offset & 0xff));                  /* Retrieve  8-bit registers and offsets if effective address is not memory operand */
}

unsigned short e386_real_mode_get_ea16 (e8086_t *c) /* Real Mode */
{
	/* 16-bit memory address specified */
	if (c->ea.is_mem) {
		if ((c->cpu & E86_CPU_8BIT) || (c->ea.ofs & 1)) {
			c->delay += 4;
		}
		return (e86_get_mem16 (c, c->ea.seg, c->ea.ofs)); /* Retrieve segment register and offset  if effective address is memory operand     */
	}
	if (c->e386_operand_size_prefix_66h (c) == 1) /* 32-bit memory operand specified */
	{
		return (e386_get_reg32 (c, c->ea.ofs));                /* Retrieve  8-bit registers and offsets if effective address is not memory operand */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		return (e286_get_reg16 (c, c->ea.ofs));                /* Retrieve  8-bit registers and offsets if effective address is not memory operand */
	}
}

unsigned short e386_virtual_8086_mode_get_ea16 (e8086_t *c) /* Virtual 8086 Mode */
{
	/* 16-bit memory address specified */
	if (c->ea.is_mem) {
		if ((c->cpu & E86_CPU_8BIT) || (c->ea.ofs & 1)) {
			c->delay += 4;
		}
		return (e86_get_mem16 (c, c->ea.seg, c->ea.ofs)); /* Retrieve segment register and offset  if effective address is memory operand     */
	}
	if (c->e386_operand_size_prefix_66h (c) == 1) /* 32-bit memory operand specified */
	{
		return (e386_get_reg32 (c, c->ea.ofs));                /* Retrieve  8-bit registers and offsets if effective address is not memory operand */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		return (e286_get_reg16 (c, c->ea.ofs));                /* Retrieve  8-bit registers and offsets if effective address is not memory operand */
	}
}

unsigned short e386_protected_mode_get_ea16_or_32 (e8086_t *c, ldt_local_descriptor_t *ldt, gdt_global_descriptor_t *gdt, tss_task_state_segment_descriptor_t *tss, cs_segment_register_descriptor_t *cs, ds_segment_register_descriptor_t *ds, es_segment_register_descriptor_t *es, fs_segment_register_descriptor_t *fs, gs_segment_register_descriptor_t *gs, ss_segment_register_descriptor_t *ss, unsigned long segment, unsigned long offset) /* Protected Mode */
{
	/* 16-bit memory address specified */
	if (c->ea.is_mem) {
		if ((c->cpu & E86_CPU_8BIT) || (c->ea.ofs & 1)) {
			c->delay += 4;
		}
		else if ((segment) == (cs->cs_segment_register_selector_index) {
			cs_segment_limit = cs->cs_segment_register_descriptor_limit;
			if (cs->cs_segment_register_descriptor_granularity_bit == 1) {
				/* Use below for when granularity bit occurs: */
				segment_limit = cs_segment_limit;
				granularity = 0xffffffff;
				cs_segment_limit = granularity + segment_limit;
			}
			else if (((cs->cs_segment_register_descriptor_type_e == 0) && (segment > cs_segment_limit)) || ((cs->cs_segment_register_descriptor_type_e == 1) && (segment < cs_segment_limit))) {
				/* Outside CS, DS, ES, FS, or GS segment limits; General Protection Fault goes here: */
			}
			else if (cs->cs_segment_register_descriptor_is_present == 0) {
				/* Segment is not valid; General Protection Fault goes here: */
			}
			else if (segment == 0x0000) {
				/* Null selector; General Protection Fault goes here: */
			}
			else {
				c->ea.seg = segment;
				c->ea.ofs = offset;
				if ((cs->cs_segment_register_descriptor_big == 1) || (cs->cs_segment_register_descriptor_default == 1) || (c->e386_address_size_prefix_67h == 1)) {
					/* 32-bit memory address operand specified */
					return (e386_get_mem32 (c, c->ea.seg, c->ea.ofs))
				}
				else {
					/* 16-bit memory address operand specified */
					return (e86_get_mem16 (c, c->ea.seg, c->ea.ofs))
				}
			}
		}
		else if ((segment) == (ds->ds_segment_register_selector_index) {
			ds_segment_limit = ds->ds_segment_register_descriptor_limit;
			if (ds->ds_segment_register_descriptor_granularity_bit == 1) {
				/* Use below for when granularity bit occurs: */
				segment_limit = ds_segment_limit;
				granularity = 0xffffffff;
				ds_segment_limit = granularity + segment_limit;
			}
			else if (((ds->ds_segment_register_descriptor_type_e == 0) && (segment > ds_segment_limit)) || ((ds->ds_segment_register_descriptor_type_e == 1) && (segment < ds_segment_limit))) {
				/* Outside CS, DS, ES, FS, or GS segment limits; General Protection Fault goes here: */
			}
			else if (ds->ds_segment_register_descriptor_is_present == 0) {
				/* Segment is not valid; General Protection Fault goes here: */
			}
			else if (segment == 0x0000) {
				/* Null selector; General Protection Fault goes here: */
			}
			else {
				c->ea.seg = segment;
				c->ea.ofs = offset;
				if ((ds->ds_segment_register_descriptor_big == 1) || (ds->ds_segment_register_descriptor_default == 1) || (c->e386_address_size_prefix_67h == 1)) {
					/* 32-bit memory address operand specified */
					return (e386_get_mem32 (c, c->ea.seg, c->ea.ofs))
				}
				else {
					/* 16-bit memory address operand specified */
					return (e86_get_mem16 (c, c->ea.seg, c->ea.ofs))
				}
			}
		}
		else if ((segment) == (es->es_segment_register_selector_index) {
			es_segment_limit = es->es_segment_register_descriptor_limit;
			if (es->es_segment_register_descriptor_granularity_bit == 1) {
				/* Use below for when granularity bit occurs: */
				segment_limit = es_segment_limit;
				granularity = 0xffffffff;
				es_segment_limit = granularity + segment_limit;
			}
			else if (((es->es_segment_register_descriptor_type_e == 0) && (segment > es_segment_limit)) || ((es->es_segment_register_descriptor_type_e == 1) && (segment < es_segment_limit))) {
				/* Outside CS, DS, ES, FS, or GS segment limits; General Protection Fault goes here: */
			}
			else if (es->es_segment_register_descriptor_is_present == 0) {
				/* Segment is not valid; General Protection Fault goes here: */
			}
			else if (segment == 0x0000) {
				/* Null selector; General Protection Fault goes here: */
			}
			else {
				c->ea.seg = segment;
				c->ea.ofs = offset;
				if ((es->es_segment_register_descriptor_big == 1) || (es->es_segment_register_descriptor_default == 1) || (c->e386_address_size_prefix_67h == 1)) {
					/* 32-bit memory address operand specified */
					return (e386_get_mem32 (c, c->ea.seg, c->ea.ofs))
				}
				else {
					/* 16-bit memory address operand specified */
					return (e86_get_mem16 (c, c->ea.seg, c->ea.ofs))
				}
			}
		}
		else if ((segment) == (fs->fs_segment_register_selector_index) {
			fs_segment_limit = fs->fs_segment_register_descriptor_limit;
			if (fs->fs_segment_register_descriptor_granularity_bit == 1) {
				/* Use below for when granularity bit occurs: */
				segment_limit = fs_segment_limit;
				granularity = 0xffffffff;
				fs_segment_limit = granularity + segment_limit;
			}
			else if (((fs->fs_segment_register_descriptor_type_e == 0) && (segment > fs_segment_limit)) || ((fs->fs_segment_register_descriptor_type_e == 1) && (segment < fs_segment_limit))) {
				/* Outside CS, DS, ES, FS, or GS segment limits; General Protection Fault goes here: */
			}
			else if (fs->fs_segment_register_descriptor_is_present == 0) {
				/* Segment is not valid; General Protection Fault goes here: */
			}
			else if (segment == 0x0000) {
				/* Null selector; General Protection Fault goes here: */
			}
			else {
				c->ea.seg = segment;
				c->ea.ofs = offset;
				if ((fs->fs_segment_register_descriptor_big == 1) || (fs->fs_segment_register_descriptor_default == 1) || (c->e386_address_size_prefix_67h == 1)) {
					/* 32-bit memory address operand specified */
					return (e386_get_mem32 (c, c->ea.seg, c->ea.ofs))
				}
				else {
					/* 16-bit memory address operand specified */
					return (e86_get_mem16 (c, c->ea.seg, c->ea.ofs))
				}
			}
		}
		else if ((segment) == (gs->gs_segment_register_selector_index) {
			gs_segment_limit = gs->gs_segment_register_descriptor_limit;
			if (gs->gs_segment_register_descriptor_granularity_bit == 1) {
				/* Use below for when granularity bit occurs: */
				segment_limit = gs_segment_limit;
				granularity = 0xffffffff;
				gs_segment_limit = granularity + segment_limit;
			}
			else if (((gs->gs_segment_register_descriptor_type_e == 0) && (segment > gs_segment_limit)) || ((gs->gs_segment_register_descriptor_type_e == 1) && (segment < gs_segment_limit))) {
				/* Outside CS, DS, ES, FS, or GS segment limits; General Protection Fault goes here: */
			}
			else if (gs->gs_segment_register_descriptor_is_present == 0) {
				/* Segment is not valid; General Protection Fault goes here: */
			}
			else if (segment == 0x0000) {
				/* Null selector; General Protection Fault goes here: */
			}
			else {
				c->ea.seg = segment;
				c->ea.ofs = offset;
				if ((gs->gs_segment_register_descriptor_big == 1) || (gs->gs_segment_register_descriptor_default == 1) || (c->e386_address_size_prefix_67h == 1)) {
					/* 32-bit memory address operand specified */
					return (e386_get_mem32 (c, c->ea.seg, c->ea.ofs))
				}
				else {
					/* 16-bit memory address operand specified */
					return (e86_get_mem16 (c, c->ea.seg, c->ea.ofs))
				}
			}
		}
		else if ((segment) == (ss->ss_segment_register_selector_index) {
			ss_segment_limit = ss->ss_segment_register_descriptor_limit;
			if (ss->ss_segment_register_descriptor_granularity_bit == 1) {
				/* Use below for when granularity bit occurs: */
				segment_limit = ss_segment_limit;
				granularity = 0xffffffff;
				ss_segment_limit = granularity + segment_limit;
			}
			else if (((ss->ss_segment_register_descriptor_type_e == 0) && (segment > ss_segment_limit)) || ((ss->ss_segment_register_descriptor_type_e == 1) && (segment < ss_segment_limit))) {
				/* Outside SS segment limit; Stack Fault goes here: */
			}
			else if (ss->ss_segment_register_descriptor_is_present == 0) {
				/* Segment is not valid; Stack Fault goes here: */
			}
			else if (segment == 0x0000) {
				/* Null selector; General Protection Fault goes here: */
			}
			else {
				c->ea.seg = segment;
				c->ea.ofs = offset;
				if ((ss->ss_segment_register_descriptor_big == 1) || (ss->ss_segment_register_descriptor_default == 1) || (c->e386_address_size_prefix_67h == 1)) {
					/* 32-bit memory address operand specified */
					return (e386_get_mem32 (c, c->ea.seg, c->ea.ofs))
				}
				else {
					/* 16-bit memory address operand specified */
					return (e86_get_mem16 (c, c->ea.seg, c->ea.ofs))
				}
			}
		}
		else {
			return (tss_get_task_state_segment_descriptor (tss, segment, offset & 0xffffffff));   /* Retrieve segment register and offset  if effective address is memory operand     */
			return (tss_get_task_gate_descriptor (tss, segment, offset & 0xffffffff));   /* Retrieve segment register and offset  if effective address is memory operand     */
			return (ldt_get_local_segment_descriptor (ldt, segment, offset & 0xffffffff));   /* Retrieve segment register and offset  if effective address is memory operand     */
			return (gdt_get_global_segment_descriptor (gdt, segment, offset & 0xffffffff));   /* Retrieve segment register and offset  if effective address is memory operand     */
		}
	}
	if (c->e386_operand_size_prefix_66h (c) == 1) /* 32-bit memory operand specified */
	{
		return (e386_get_reg32 (c, offset & 0xffffffff));                /* Retrieve  8-bit registers and offsets if effective address is not memory operand */
	}
	else /* 16-bit memory operand specified (shifted left by four bits to 20-bits wide) */
	{
		return (e386_get_reg16 (c, offset & 0xffff));                /* Retrieve  8-bit registers and offsets if effective address is not memory operand */
	}
}

unsigned long e386_real_mode_get_ea32 (e8086_t *c) /* Real Mode */
{
	/* 32-bit memory address specified */
	if (c->ea.is_mem) {
		return (e386_get_mem32 (c, c->ea.seg, c->ea.ofs)); /* Retrieve segment register and offset  if effective address is memory operand     */
	}
	return (e386_get_reg32 (c, c->ea.ofs));                /* Retrieve  8-bit registers and offsets if effective address is not memory operand */
}

unsigned long e386_virtual_8086_mode_get_ea32 (e8086_t *c) /* Virtual 8086 Mode */
{
	/* 32-bit memory address specified */
	if (c->ea.is_mem) {
		return (e386_get_mem32 (c, c->ea.seg, c->ea.ofs)); /* Retrieve segment register and offset  if effective address is memory operand     */
	}
	return (e386_get_reg32 (c, c->ea.ofs));                /* Retrieve  8-bit registers and offsets if effective address is not memory operand */
}

/* unsigned long e386_protected_mode_get_ea32 (e8086_t *c, ldt_local_descriptor_t *ldt, gdt_global_descriptor_t *gdt) /* Protected Mode */
{
	/* 32-bit memory address specified */
/*	if (c->ea.is_mem) {
		return (ldt_get_local_segment_descriptor (ldt, segment, offset & 0xffffffff));   /* Retrieve segment register and offset  if effective address is memory operand     */
/*		return (gdt_get_global_segment_descriptor (gdt, segment, offset & 0xffffffff));   /* Retrieve segment register and offset  if effective address is memory operand     */
/*	}
	return (e386_get_reg32 (c, offset));                /* Retrieve  8-bit registers and offsets if effective address is not memory operand */
/* } */

void e386_real_mode_set_ea8 (e8086_t *c, cs_segment_register_descriptor_t *cs, ds_segment_register_descriptor_t *ds, es_segment_register_descriptor_t *es, fs_segment_register_descriptor_t *fs, gs_segment_register_descriptor_t *gs, ss_segment_register_descriptor_t *ss, unsigned char val) /* Real Mode */
{
	/* 8-bit memory address specified */
	e386_get_cs (c);
	e386_get_ds (c);
	e386_get_es (c);
	e386_get_fs (c);
	e386_get_gs (c);
	if (c->ea.is_mem) {
		if ((segment) == (e86_reg_cs (c)) {
			cs_segment_limit = cs->cs_segment_register_descriptor_limit;
			if (cs->cs_segment_register_descriptor_granularity_bit == 1) {
				/* Use below for when granularity bit occurs: */
				segment_limit = cs_segment_limit;
				granularity = 0xffffffff;
				cs_segment_limit = granularity + segment_limit;
			}
			else if (((cs->cs_segment_register_descriptor_type_e == 0) && (segment > cs_segment_limit)) || ((cs->cs_segment_register_descriptor_type_e == 1) && (segment < cs_segment_limit))) {
				/* Outside CS, DS, ES, FS, or GS segment limits (normally 0FFFFH); Interrupt 13 goes here: */
			}
			else {
				c->ea.seg = segment;
				c->ea.ofs = offset;
				e86_set_mem8 (c, c->seg.ofs, c->ea.ofs, val)
			}
		}
		else if ((segment) == (e86_reg_ds (c)) {
			ds_segment_limit = ds->ds_segment_register_descriptor_limit;
			if (ds->ds_segment_register_descriptor_granularity_bit == 1) {
				/* Use below for when granularity bit occurs: */
				segment_limit = ds_segment_limit;
				granularity = 0xffffffff;
				ds_segment_limit = granularity + segment_limit;
			}
			else if (((ds->ds_segment_register_descriptor_type_e == 0) && (segment > ds_segment_limit)) || ((ds->ds_segment_register_descriptor_type_e == 1) && (segment < ds_segment_limit))) {
				/* Outside CS, DS, ES, FS, or GS segment limits (normally 0FFFFH); Interrupt 13 goes here: */
			}
			else {
				c->ea.seg = segment;
				c->ea.ofs = offset;
				e86_set_mem8 (c, c->seg.ofs, c->ea.ofs, val)
			}
		}
		else if ((segment) == (e86_reg_es (c)) {
			es_segment_limit = es->es_segment_register_descriptor_limit;
			if (es->es_segment_register_descriptor_granularity_bit == 1) {
				/* Use below for when granularity bit occurs: */
				segment_limit = es_segment_limit;
				granularity = 0xffffffff;
				es_segment_limit = granularity + segment_limit;
			}
			else if (((es->es_segment_register_descriptor_type_e == 0) && (segment > es_segment_limit)) || ((es->es_segment_register_descriptor_type_e == 1) && (segment < es_segment_limit))) {
				/* Outside CS, DS, ES, FS, or GS segment limits (normally 0FFFFH); Interrupt 13 goes here: */
			}
			else {
				c->ea.seg = segment;
				c->ea.ofs = offset;
				e86_set_mem8 (c, c->seg.ofs, c->ea.ofs, val)
			}
		}
		else if ((segment) == (e86_reg_fs (c)) {
			fs_segment_limit = fs->fs_segment_register_descriptor_limit;
			if (fs->fs_segment_register_descriptor_granularity_bit == 1) {
				/* Use below for when granularity bit occurs: */
				segment_limit = fs_segment_limit;
				granularity = 0xffffffff;
				fs_segment_limit = granularity + segment_limit;
			}
			else if (((fs->fs_segment_register_descriptor_type_e == 0) && (segment > fs_segment_limit)) || ((fs->fs_segment_register_descriptor_type_e == 1) && (segment < fs_segment_limit))) {
				/* Outside CS, DS, ES, FS, or GS segment limits (normally 0FFFFH); Interrupt 13 goes here: */
			}
			else {
				c->ea.seg = segment;
				c->ea.ofs = offset;
				e86_set_mem8 (c, c->seg.ofs, c->ea.ofs, val)
			}
		}
		else if ((segment) == (e86_reg_gs (c)) {
			gs_segment_limit = gs->gs_segment_register_descriptor_limit;
			if (gs->gs_segment_register_descriptor_granularity_bit == 1) {
				/* Use below for when granularity bit occurs: */
				segment_limit = gs_segment_limit;
				granularity = 0xffffffff;
				gs_segment_limit = granularity + segment_limit;
			}
			else if (((gs->gs_segment_register_descriptor_type_e == 0) && (segment > gs_segment_limit)) || ((gs->gs_segment_register_descriptor_type_e == 1) && (segment < gs_segment_limit))) {
				/* Outside CS, DS, ES, FS, or GS segment limits (normally 0FFFFH); Interrupt 13 goes here: */
			}
			else {
				c->ea.seg = segment;
				c->ea.ofs = offset;
				e86_set_mem8 (c, c->seg.ofs, c->ea.ofs, val)
			}
		}
		else if ((segment) == (e86_reg_ss (c)) {
			ss_segment_limit = ss->ss_segment_register_descriptor_limit;
			if (ss->ss_segment_register_descriptor_granularity_bit == 1) {
				/* Use below for when granularity bit occurs: */
				segment_limit = ss_segment_limit;
				granularity = 0xffffffff;
				ss_segment_limit = granularity + segment_limit;
			}
			else if (((ss->ss_segment_register_descriptor_type_e == 0) && (segment > ss_segment_limit)) || ((ss->ss_segment_register_descriptor_type_e == 1) && (segment < ss_segment_limit))) {
				/* Outside CS, DS, ES, FS, or GS segment limits (normally 0FFFFH); Interrupt 13 goes here: */
			}
			else {
				c->ea.seg = segment;
				c->ea.ofs = offset;
				e86_set_mem8 (c, c->seg.ofs, c->ea.ofs, val)
			}
		}
		else {
			/* Normal memory address */
			if (c->ea.is_mem) {
				e86_set_mem8 (c, c->ea.seg, c->ea.ofs, val);  /* Set  8-bit memory address to segment register and offset if address is memory operand */
			}
		}
	}
	else {
		e86_set_reg8 (c, c->ea.ofs, val);             /* Set  8-bit CPU register to offset if effective address is not memory operand          */
	}
}

void e386_virtual_8086_mode_set_ea8 (e8086_t *c, cs_segment_register_descriptor_t *cs, ds_segment_register_descriptor_t *ds, es_segment_register_descriptor_t *es, fs_segment_register_descriptor_t *fs, gs_segment_register_descriptor_t *gs, ss_segment_register_descriptor_t *ss, unsigned char val) /* Virtual 8086 Mode */
{
	/* 8-bit memory address specified */
	e386_get_cs (c);
	e386_get_ds (c);
	e386_get_es (c);
	e386_get_fs (c);
	e386_get_gs (c);
	if (c->ea.is_mem) {
		if ((segment) == (e86_reg_cs (c)) {
			cs_segment_limit = cs->cs_segment_register_descriptor_limit;
			if (cs->cs_segment_register_descriptor_granularity_bit == 1) {
				/* Use below for when granularity bit occurs: */
				segment_limit = cs_segment_limit;
				granularity = 0xffffffff;
				cs_segment_limit = granularity + segment_limit;
			}
			else if (((cs->cs_segment_register_descriptor_type_e == 0) && (segment > cs_segment_limit)) || ((cs->cs_segment_register_descriptor_type_e == 1) && (segment < cs_segment_limit))) {
				/* Outside CS, DS, ES, FS, or GS segment limits (normally 0FFFFH); Interrupt 13 goes here: */
			}
			else {
				c->ea.seg = segment;
				c->ea.ofs = offset;
				e86_set_mem8 (c, c->seg.ofs, c->ea.ofs, val)
			}
		}
		else if ((segment) == (e86_reg_ds (c)) {
			ds_segment_limit = ds->ds_segment_register_descriptor_limit;
			if (ds->ds_segment_register_descriptor_granularity_bit == 1) {
				/* Use below for when granularity bit occurs: */
				segment_limit = ds_segment_limit;
				granularity = 0xffffffff;
				ds_segment_limit = granularity + segment_limit;
			}
			else if (((ds->ds_segment_register_descriptor_type_e == 0) && (segment > ds_segment_limit)) || ((ds->ds_segment_register_descriptor_type_e == 1) && (segment < ds_segment_limit))) {
				/* Outside CS, DS, ES, FS, or GS segment limits (normally 0FFFFH); Interrupt 13 goes here: */
			}
			else {
				c->ea.seg = segment;
				c->ea.ofs = offset;
				e86_set_mem8 (c, c->seg.ofs, c->ea.ofs, val)
			}
		}
		else if ((segment) == (e86_reg_es (c)) {
			es_segment_limit = es->es_segment_register_descriptor_limit;
			if (es->es_segment_register_descriptor_granularity_bit == 1) {
				/* Use below for when granularity bit occurs: */
				segment_limit = es_segment_limit;
				granularity = 0xffffffff;
				es_segment_limit = granularity + segment_limit;
			}
			else if (((es->es_segment_register_descriptor_type_e == 0) && (segment > es_segment_limit)) || ((es->es_segment_register_descriptor_type_e == 1) && (segment < es_segment_limit))) {
				/* Outside CS, DS, ES, FS, or GS segment limits (normally 0FFFFH); Interrupt 13 goes here: */
			}
			else {
				c->ea.seg = segment;
				c->ea.ofs = offset;
				e86_set_mem8 (c, c->seg.ofs, c->ea.ofs, val)
			}
		}
		else if ((segment) == (e86_reg_fs (c)) {
			fs_segment_limit = fs->fs_segment_register_descriptor_limit;
			if (fs->fs_segment_register_descriptor_granularity_bit == 1) {
				/* Use below for when granularity bit occurs: */
				segment_limit = fs_segment_limit;
				granularity = 0xffffffff;
				fs_segment_limit = granularity + segment_limit;
			}
			else if (((fs->fs_segment_register_descriptor_type_e == 0) && (segment > fs_segment_limit)) || ((fs->fs_segment_register_descriptor_type_e == 1) && (segment < fs_segment_limit))) {
				/* Outside CS, DS, ES, FS, or GS segment limits (normally 0FFFFH); Interrupt 13 goes here: */
			}
			else {
				c->ea.seg = segment;
				c->ea.ofs = offset;
				e86_set_mem8 (c, c->seg.ofs, c->ea.ofs, val)
			}
		}
		else if ((segment) == (e86_reg_gs (c)) {
			gs_segment_limit = gs->gs_segment_register_descriptor_limit;
			if (gs->gs_segment_register_descriptor_granularity_bit == 1) {
				/* Use below for when granularity bit occurs: */
				segment_limit = gs_segment_limit;
				granularity = 0xffffffff;
				gs_segment_limit = granularity + segment_limit;
			}
			else if (((gs->gs_segment_register_descriptor_type_e == 0) && (segment > gs_segment_limit)) || ((gs->gs_segment_register_descriptor_type_e == 1) && (segment < gs_segment_limit))) {
				/* Outside CS, DS, ES, FS, or GS segment limits (normally 0FFFFH); Interrupt 13 goes here: */
			}
			else {
				c->ea.seg = segment;
				c->ea.ofs = offset;
				e86_set_mem8 (c, c->seg.ofs, c->ea.ofs, val)
			}
		}
		else if ((segment) == (e86_reg_ss (c)) {
			ss_segment_limit = ss->ss_segment_register_descriptor_limit;
			if (ss->ss_segment_register_descriptor_granularity_bit == 1) {
				/* Use below for when granularity bit occurs: */
				segment_limit = ss_segment_limit;
				granularity = 0xffffffff;
				ss_segment_limit = granularity + segment_limit;
			}
			else if (((ss->ss_segment_register_descriptor_type_e == 0) && (segment > ss_segment_limit)) || ((ss->ss_segment_register_descriptor_type_e == 1) && (segment < ss_segment_limit))) {
				/* Outside CS, DS, ES, FS, or GS segment limits (normally 0FFFFH); Interrupt 13 goes here: */
			}
			else {
				c->ea.seg = segment;
				c->ea.ofs = offset;
				e86_set_mem8 (c, c->seg.ofs, c->ea.ofs, val)
			}
		}
		else {
			/* Normal memory address */
			if (c->ea.is_mem) {
				e86_set_mem8 (c, c->ea.seg, c->ea.ofs, val);  /* Set  8-bit memory address to segment register and offset if address is memory operand */
			}
		}
	}
	else {
		e86_set_reg8 (c, c->ea.ofs, val);             /* Set  8-bit CPU register to offset if effective address is not memory operand          */
	}
}
/* ------------------------------------------------------------------------------------------------------------------------------------------ */
/* Protected Mode Exceptions:                                                                                                                 */
/* ------------------------------------------------------------------------------------------------------------------------------------------ */
/*  - #GP(0): If the destination is located in a non-writable segment (W=0). If a memory operand effective address is outside the CS, DS, ES, */
/*            FS, or GS segment limit. If the DS, ES, FS, or GS register is used to access memory and it contains a null segment selector.    */
/*  - #SS(0): If a memory operand effective address is outside the SS segment limit.                                                          */
/*  - #PF(fault-code): If a page fault occurs.                                                                                                */
/* ------------------------------------------------------------------------------------------------------------------------------------------ */
void e386_protected_mode_set_ea8 (e8086_t *c, cs_segment_register_descriptor_t *cs, ds_segment_register_descriptor_t *ds, es_segment_register_descriptor_t *es, fs_segment_register_descriptor_t *fs, gs_segment_register_descriptor_t *gs, ss_segment_register_descriptor_t *ss, ldt_local_descriptor_t *ldt, gdt_global_descriptor_t *gdt, tss_task_state_segment_descriptor_t *tss, unsigned char val) /* Protected Mode */
{
	/* 8-bit memory address specified */
	if (c->ea.is_mem) {
		/* Start with Global Descriptor Table: */
		gdt = gdt_get_global_segment_descriptor (gdt, segment, offset & 0xff);
		if (segment == NULL) {
			/* Search inside of Local Descriptor Table instead */
			ldt = ldt_get_local_segment_descriptor (ldt, segment, offset & 0xff);
			if (segment == NULL) {
				/* Search inside of Task State Segment Descriptor Table instead */
				tss = tss_get_task_state_segment_descriptor (tss, segment, offset & 0xff);
				tss_segment_limit = tss->tss_gdt_segment_limit;
				if (segment == NULL) {
					if ((segment) == (cs->cs_segment_register_selector_index) {
						cs_segment_limit = cs->cs_segment_register_descriptor_limit;
						if (cs->cs_segment_register_descriptor_granularity_bit == 1) {
							/* Use below for when granularity bit occurs: */
							segment_limit = cs_segment_limit;
							granularity = 0xffffffff;
							cs_segment_limit = granularity + segment_limit;
						}
						else if (((cs->cs_segment_register_descriptor_type_e == 0) && (segment > cs_segment_limit)) || ((cs->cs_segment_register_descriptor_type_e == 1) && (segment < cs_segment_limit))) {
							/* Outside CS, DS, ES, FS, or GS segment limits; General Protection Fault goes here: */
						}
						else if (cs->cs_segment_register_descriptor_is_present == 0) {
							/* Segment is not valid; General Protection Fault goes here: */
						}
						else if (segment == 0x0000) {
							/* Null selector; General Protection Fault goes here: */
						}
						else {
							c->ea.seg = segment;
							c->ea.ofs = offset;
							e86_set_mem8 (c, c->seg.ofs, c->ea.ofs, val)
						}
					}
					else if ((segment) == (ds->ds_segment_register_selector_index) {
						ds_segment_limit = ds->ds_segment_register_descriptor_limit;
						if (ds->ds_segment_register_descriptor_granularity_bit == 1) {
							/* Use below for when granularity bit occurs: */
							segment_limit = ds_segment_limit;
							granularity = 0xffffffff;
							ds_segment_limit = granularity + segment_limit;
						}
						else if (((ds->ds_segment_register_descriptor_type_e == 0) && (segment > ds_segment_limit)) || ((ds->ds_segment_register_descriptor_type_e == 1) && (segment < ds_segment_limit))) {
							/* Outside CS, DS, ES, FS, or GS segment limits; General Protection Fault goes here: */
						}
						else if (ds->ds_segment_register_descriptor_is_present == 0) {
							/* Segment is not valid; General Protection Fault goes here: */
						}
						else if (segment == 0x0000) {
							/* Null selector; General Protection Fault goes here: */
						}
						else {
							c->ea.seg = segment;
							c->ea.ofs = offset;
							e86_set_mem8 (c, c->seg.ofs, c->ea.ofs, val)
						}
					}
					else if ((segment) == (es->es_segment_register_selector_index) {
						es_segment_limit = es->es_segment_register_descriptor_limit;
						if (es->es_segment_register_descriptor_granularity_bit == 1) {
							/* Use below for when granularity bit occurs: */
							segment_limit = es_segment_limit;
							granularity = 0xffffffff;
							es_segment_limit = granularity + segment_limit;
						}
						else if (((es->es_segment_register_descriptor_type_e == 0) && (segment > es_segment_limit)) || ((es->es_segment_register_descriptor_type_e == 1) && (segment < es_segment_limit))) {
							/* Outside CS, DS, ES, FS, or GS segment limits; General Protection Fault goes here: */
						}
						else if (es->es_segment_register_descriptor_is_present == 0) {
							/* Segment is not valid; General Protection Fault goes here: */
						}
						else if (segment == 0x0000) {
							/* Null selector; General Protection Fault goes here: */
						}
						else {
							c->ea.seg = segment;
							c->ea.ofs = offset;
							e86_set_mem8 (c, c->seg.ofs, c->ea.ofs, val)
						}
					}
					else if ((segment) == (fs->fs_segment_register_selector_index) {
						fs_segment_limit = fs->fs_segment_register_descriptor_limit;
						if (fs->fs_segment_register_descriptor_granularity_bit == 1) {
							/* Use below for when granularity bit occurs: */
							segment_limit = fs_segment_limit;
							granularity = 0xffffffff;
							fs_segment_limit = granularity + segment_limit;
						}
						else if (((fs->fs_segment_register_descriptor_type_e == 0) && (segment > fs_segment_limit)) || ((fs->fs_segment_register_descriptor_type_e == 1) && (segment < fs_segment_limit))) {
							/* Outside CS, DS, ES, FS, or GS segment limits; General Protection Fault goes here: */
						}
						else if (fs->fs_segment_register_descriptor_is_present == 0) {
							/* Segment is not valid; General Protection Fault goes here: */
						}
						else if (segment == 0x0000) {
							/* Null selector; General Protection Fault goes here: */
						}
						else {
							c->ea.seg = segment;
							c->ea.ofs = offset;
							e86_set_mem8 (c, c->seg.ofs, c->ea.ofs, val)
						}
					}
					else if ((segment) == (gs->gs_segment_register_selector_index) {
						gs_segment_limit = gs->gs_segment_register_descriptor_limit;
						if (gs->gs_segment_register_descriptor_granularity_bit == 1) {
							/* Use below for when granularity bit occurs: */
							segment_limit = gs_segment_limit;
							granularity = 0xffffffff;
							gs_segment_limit = granularity + segment_limit;
						}
						else if (((gs->gs_segment_register_descriptor_type_e == 0) && (segment > gs_segment_limit)) || ((gs->gs_segment_register_descriptor_type_e == 1) && (segment < gs_segment_limit))) {
							/* Outside CS, DS, ES, FS, or GS segment limits; General Protection Fault goes here: */
						}
						else if (gs->gs_segment_register_descriptor_is_present == 0) {
							/* Segment is not valid; General Protection Fault goes here: */
						}
						else if (segment == 0x0000) {
							/* Null selector; General Protection Fault goes here: */
						}
						else {
							c->ea.seg = segment;
							c->ea.ofs = offset;
							e86_set_mem8 (c, c->seg.ofs, c->ea.ofs, val)
						}
					}
					else if ((segment) == (ss->ss_segment_register_selector_index) {
						ss_segment_limit = ss->ss_segment_register_descriptor_limit;
						if (ss->ss_segment_register_descriptor_granularity_bit == 1) {
							/* Use below for when granularity bit occurs: */
							segment_limit = ss_segment_limit;
							granularity = 0xffffffff;
							ss_segment_limit = granularity + segment_limit;
						}
						else if (((ss->ss_segment_register_descriptor_type_e == 0) && (segment > ss_segment_limit)) || ((ss->ss_segment_register_descriptor_type_e == 1) && (segment < ss_segment_limit))) {
							/* Outside SS segment limit; Stack Fault goes here: */
						}
						else if (ss->ss_segment_register_descriptor_is_present == 0) {
							/* Segment is not valid; Stack Fault goes here: */
						}
						else if (segment == 0x0000) {
							/* Null selector; General Protection Fault goes here: */
						}
						else {
							c->ea.seg = segment;
							c->ea.ofs = offset;
							e86_set_mem8 (c, c->seg.ofs, c->ea.ofs, val)
						}
					}
					else {
						/* Segment obviously does not exist; possibly use actual CPU exception if required */
					}
				}
				else if (tss->tss_gdt_granularity_bit == 1) {
					/* Use below for when granularity bit occurs: */
					segment_limit = tss_segment_limit;
					granularity = 0xffffffff;
					tss_segment_limit = granularity + segment_limit;
				}
				else if (tss->tss_gdt_segment_is_present == 0) {
					/* Segment is not valid; General Protection Fault goes here: */
				}
				else if (((tss->tss_gdt_type_e == 0) && ((segment) || (offset) > (tss_segment_limit))) || ((tss->tss_gdt_type_e == 1) && ((segment) || (offset) < (tss->tss_gdt_segment_limit)))) {
					/* Outside segment limits; General Protection Fault goes here: */
				}
				else if (tss->tss_gdt_type_w == 0) {
					/* Non-writable segment; General Protection Fault goes here: */
				}
				else {
					c->ea.seg = segment & 0xff;  /* Set  8-bit memory address to segment register and offset if address is memory operand */
					c->ea.ofs = offset & 0xff;  /* Set  8-bit memory address to segment register and offset if address is memory operand */
					e86_set_mem8 (c, c->seg.ofs, c->ea.ofs, val);  /* Set  8-bit memory address to segment register and offset if address is memory operand */
				}
			}
			ldt_segment_limit = ldt->ldt_segment_limit;
			else if (ldt->ldt_granularity_bit == 1) {
				/* Use below for when granularity bit occurs: */
				segment_limit = ldt_segment_limit;
				granularity = 0xffffffff;
				ldt_segment_limit = granularity + segment_limit;
			}
			else if (ldt->ldt_segment_is_present == 0) {
				/* Segment is not valid; General Protection Fault goes here: */
			}
			else if (((ldt->ldt_type_e == 0) && ((segment) || (offset) > (ldt_segment_limit))) || ((ldt->ldt_type_e == 1) && ((segment) || (offset) < (ldt->ldt_segment_limit)))) {
				/* Outside segment limits; General Protection Fault goes here: */
			}
			else if (ldt->ldt_type_w == 0) {
				/* Non-writable segment; General Protection Fault goes here: */
			}
			else {
				c->ea.seg = segment & 0xff;  /* Set  8-bit memory address to segment register and offset if address is memory operand */
				c->ea.ofs = offset & 0xff;  /* Set  8-bit memory address to segment register and offset if address is memory operand */
				e86_set_mem8 (c, c->seg.ofs, c->ea.ofs, val);  /* Set  8-bit memory address to segment register and offset if address is memory operand */
			}
		}
		gdt_segment_limit = gdt->gdt_segment_limit;
		else if (gdt->gdt_granularity_bit == 1) {
			/* Use below for when granularity bit occurs: */
			segment_limit = gdt_segment_limit;
			granularity = 0xffffffff;
			gdt_segment_limit = granularity + segment_limit;
		}
		else if (gdt->gdt_segment_is_present == 0) {
			/* Segment is not valid; General Protection Fault goes here: */
		}
		else if (((gdt->gdt_type_e == 0) && ((segment) || (offset) > (gdt_segment_limit))) || ((gdt->gdt_type_e == 1) && ((segment) || (offset) < (gdt->gdt_segment_limit)))) {
			/* Outside segment limits; General Protection Fault goes here: */
		}
		else if (gdt->gdt_type_w == 0) {
			/* Non-writable segment; General Protection Fault goes here: */
		}
		else {
			c->ea.seg = segment & 0xff;  /* Set  8-bit memory address to segment register and offset if address is memory operand */
			c->ea.ofs = offset & 0xff;  /* Set  8-bit memory address to segment register and offset if address is memory operand */
			e86_set_mem8 (c, c->seg.ofs, c->ea.ofs, val);  /* Set  8-bit memory address to segment register and offset if address is memory operand */
		}
	}
	else {
		c->ea.ofs = offset & 0xff;
		e86_set_reg8 (c, c->ea.ofs, val);             /* Set  8-bit CPU register to offset if effective address is not memory operand          */
	}
}

void e386_real_mode_set_ea16 (e8086_t *c, unsigned short val) /* Real Mode */
{
	/* 16-bit memory address specified */
	if (c->ea.is_mem) {
		if ((c->cpu & E86_CPU_8BIT) || (c->ea.ofs & 1)) {
			c->delay += 4;
		}
		if ((segment) == (cs->cs_segment_register_selector_index) {
			cs_segment_limit = cs->cs_segment_register_descriptor_limit;
			if (cs->cs_segment_register_descriptor_granularity_bit == 1) {
				/* Use below for when granularity bit occurs: */
				segment_limit = cs_segment_limit;
				granularity = 0xffffffff;
				cs_segment_limit = granularity + segment_limit;
			}
			else if (((cs->cs_segment_register_descriptor_type_e == 0) && (segment > cs_segment_limit)) || ((cs->cs_segment_register_descriptor_type_e == 1) && (segment < cs_segment_limit))) {
				/* Outside CS, DS, ES, FS, or GS segment limits (normally 0FFFFH); Interrupt 13 goes here: */
			}
			else {
				c->ea.seg = segment;
				c->ea.ofs = offset;
				e86_set_mem16 (c, c->seg.ofs, c->ea.ofs, val)
			}
		}
		else if ((segment) == (ds->ds_segment_register_selector_index) {
			ds_segment_limit = ds->ds_segment_register_descriptor_limit;
			if (ds->ds_segment_register_descriptor_granularity_bit == 1) {
				/* Use below for when granularity bit occurs: */
				segment_limit = ds_segment_limit;
				granularity = 0xffffffff;
				ds_segment_limit = granularity + segment_limit;
			}
			else if (((ds->ds_segment_register_descriptor_type_e == 0) && (segment > ds_segment_limit)) || ((ds->ds_segment_register_descriptor_type_e == 1) && (segment < ds_segment_limit))) {
				/* Outside CS, DS, ES, FS, or GS segment limits (normally 0FFFFH); Interrupt 13 goes here: */
			}
			else {
				c->ea.seg = segment;
				c->ea.ofs = offset;
				e86_set_mem16 (c, c->seg.ofs, c->ea.ofs, val)
			}
		}
		else if ((segment) == (es->es_segment_register_selector_index) {
			es_segment_limit = es->es_segment_register_descriptor_limit;
			if (es->es_segment_register_descriptor_granularity_bit == 1) {
				/* Use below for when granularity bit occurs: */
				segment_limit = es_segment_limit;
				granularity = 0xffffffff;
				es_segment_limit = granularity + segment_limit;
			}
			else if (((es->es_segment_register_descriptor_type_e == 0) && (segment > es_segment_limit)) || ((es->es_segment_register_descriptor_type_e == 1) && (segment < es_segment_limit))) {
				/* Outside CS, DS, ES, FS, or GS segment limits (normally 0FFFFH); Interrupt 13 goes here: */
			}
			else {
				c->ea.seg = segment;
				c->ea.ofs = offset;
				e86_set_mem16 (c, c->seg.ofs, c->ea.ofs, val)
			}
		}
		else if ((segment) == (fs->fs_segment_register_selector_index) {
			fs_segment_limit = fs->fs_segment_register_descriptor_limit;
			if (fs->fs_segment_register_descriptor_granularity_bit == 1) {
				/* Use below for when granularity bit occurs: */
				segment_limit = fs_segment_limit;
				granularity = 0xffffffff;
				fs_segment_limit = granularity + segment_limit;
			}
			else if (((fs->fs_segment_register_descriptor_type_e == 0) && (segment > fs_segment_limit)) || ((fs->fs_segment_register_descriptor_type_e == 1) && (segment < fs_segment_limit))) {
				/* Outside CS, DS, ES, FS, or GS segment limits (normally 0FFFFH); Interrupt 13 goes here: */
			}
			else {
				c->ea.seg = segment;
				c->ea.ofs = offset;
				e86_set_mem16 (c, c->seg.ofs, c->ea.ofs, val)
			}
		}
		else if ((segment) == (gs->gs_segment_register_selector_index) {
			gs_segment_limit = gs->gs_segment_register_descriptor_limit;
			if (gs->gs_segment_register_descriptor_granularity_bit == 1) {
				/* Use below for when granularity bit occurs: */
				segment_limit = gs_segment_limit;
				granularity = 0xffffffff;
				gs_segment_limit = granularity + segment_limit;
			}
			else if (((gs->gs_segment_register_descriptor_type_e == 0) && (segment > gs_segment_limit)) || ((gs->gs_segment_register_descriptor_type_e == 1) && (segment < gs_segment_limit))) {
				/* Outside CS, DS, ES, FS, or GS segment limits (normally 0FFFFH); Interrupt 13 goes here: */
			}
			else {
				c->ea.seg = segment;
				c->ea.ofs = offset;
				e86_set_mem16 (c, c->seg.ofs, c->ea.ofs, val)
			}
		}
		else if ((segment) == (ss->ss_segment_register_selector_index) {
			ss_segment_limit = ss->ss_segment_register_descriptor_limit;
			if (ss->ss_segment_register_descriptor_granularity_bit == 1) {
				/* Use below for when granularity bit occurs: */
				segment_limit = ss_segment_limit;
				granularity = 0xffffffff;
				ss_segment_limit = granularity + segment_limit;
			}
			else if (((ss->ss_segment_register_descriptor_type_e == 0) && (segment > ss_segment_limit)) || ((ss->ss_segment_register_descriptor_type_e == 1) && (segment < ss_segment_limit))) {
				/* Outside CS, DS, ES, FS, or GS segment limits (normally 0FFFFH); Interrupt 13 goes here: */
			}
			else {
				c->ea.seg = segment;
				c->ea.ofs = offset;
				e86_set_mem16 (c, c->seg.ofs, c->ea.ofs, val)
			}
		}
		else {
			/* Normal memory address specified */
			c->ea.seg = segment;
			c->ea.ofs = offset;
			e86_set_mem16 (c, c->seg.ofs, c->ea.ofs, val)
		}
	}
	else {
		e386_set_reg16 (c, c->ea.ofs, val);           /* Set 16-bit CPU register to offset if effective address is not memory operand          */
	}
}

void e386_virtual_8086_mode_set_ea16 (e8086_t *c, unsigned short val) /* Virtual 8086 Mode */
{
	/* 16-bit memory address specified */
	if (c->ea.is_mem) {
		if ((c->cpu & E86_CPU_8BIT) || (c->ea.ofs & 1)) {
			c->delay += 4;
		}
		if ((segment) == (cs->cs_segment_register_selector_index) {
			cs_segment_limit = cs->cs_segment_register_descriptor_limit;
			if (cs->cs_segment_register_descriptor_granularity_bit == 1) {
				/* Use below for when granularity bit occurs: */
				segment_limit = cs_segment_limit;
				granularity = 0xffffffff;
				cs_segment_limit = granularity + segment_limit;
			}
			else if (((cs->cs_segment_register_descriptor_type_e == 0) && (segment > cs_segment_limit)) || ((cs->cs_segment_register_descriptor_type_e == 1) && (segment < cs_segment_limit))) {
				/* Outside CS, DS, ES, FS, or GS segment limits (normally 0FFFFH); Interrupt 13 goes here: */
			}
			else {
				c->ea.seg = segment;
				c->ea.ofs = offset;
				e86_set_mem16 (c, c->seg.ofs, c->ea.ofs, val)
			}
		}
		else if ((segment) == (ds->ds_segment_register_selector_index) {
			ds_segment_limit = ds->ds_segment_register_descriptor_limit;
			if (ds->ds_segment_register_descriptor_granularity_bit == 1) {
				/* Use below for when granularity bit occurs: */
				segment_limit = ds_segment_limit;
				granularity = 0xffffffff;
				ds_segment_limit = granularity + segment_limit;
			}
			else if (((ds->ds_segment_register_descriptor_type_e == 0) && (segment > ds_segment_limit)) || ((ds->ds_segment_register_descriptor_type_e == 1) && (segment < ds_segment_limit))) {
				/* Outside CS, DS, ES, FS, or GS segment limits (normally 0FFFFH); Interrupt 13 goes here: */
			}
			else {
				c->ea.seg = segment;
				c->ea.ofs = offset;
				e86_set_mem16 (c, c->seg.ofs, c->ea.ofs, val)
			}
		}
		else if ((segment) == (es->es_segment_register_selector_index) {
			es_segment_limit = es->es_segment_register_descriptor_limit;
			if (es->es_segment_register_descriptor_granularity_bit == 1) {
				/* Use below for when granularity bit occurs: */
				segment_limit = es_segment_limit;
				granularity = 0xffffffff;
				es_segment_limit = granularity + segment_limit;
			}
			else if (((es->es_segment_register_descriptor_type_e == 0) && (segment > es_segment_limit)) || ((es->es_segment_register_descriptor_type_e == 1) && (segment < es_segment_limit))) {
				/* Outside CS, DS, ES, FS, or GS segment limits (normally 0FFFFH); Interrupt 13 goes here: */
			}
			else {
				c->ea.seg = segment;
				c->ea.ofs = offset;
				e86_set_mem16 (c, c->seg.ofs, c->ea.ofs, val)
			}
		}
		else if ((segment) == (fs->fs_segment_register_selector_index) {
			fs_segment_limit = fs->fs_segment_register_descriptor_limit;
			if (fs->fs_segment_register_descriptor_granularity_bit == 1) {
				/* Use below for when granularity bit occurs: */
				segment_limit = fs_segment_limit;
				granularity = 0xffffffff;
				fs_segment_limit = granularity + segment_limit;
			}
			else if (((fs->fs_segment_register_descriptor_type_e == 0) && (segment > fs_segment_limit)) || ((fs->fs_segment_register_descriptor_type_e == 1) && (segment < fs_segment_limit))) {
				/* Outside CS, DS, ES, FS, or GS segment limits (normally 0FFFFH); Interrupt 13 goes here: */
			}
			else {
				c->ea.seg = segment;
				c->ea.ofs = offset;
				e86_set_mem16 (c, c->seg.ofs, c->ea.ofs, val)
			}
		}
		else if ((segment) == (gs->gs_segment_register_selector_index) {
			gs_segment_limit = gs->gs_segment_register_descriptor_limit;
			if (gs->gs_segment_register_descriptor_granularity_bit == 1) {
				/* Use below for when granularity bit occurs: */
				segment_limit = gs_segment_limit;
				granularity = 0xffffffff;
				gs_segment_limit = granularity + segment_limit;
			}
			else if (((gs->gs_segment_register_descriptor_type_e == 0) && (segment > gs_segment_limit)) || ((gs->gs_segment_register_descriptor_type_e == 1) && (segment < gs_segment_limit))) {
				/* Outside CS, DS, ES, FS, or GS segment limits (normally 0FFFFH); Interrupt 13 goes here: */
			}
			else {
				c->ea.seg = segment;
				c->ea.ofs = offset;
				e86_set_mem16 (c, c->seg.ofs, c->ea.ofs, val)
			}
		}
		else if ((segment) == (ss->ss_segment_register_selector_index) {
			ss_segment_limit = ss->ss_segment_register_descriptor_limit;
			if (ss->ss_segment_register_descriptor_granularity_bit == 1) {
				/* Use below for when granularity bit occurs: */
				segment_limit = ss_segment_limit;
				granularity = 0xffffffff;
				ss_segment_limit = granularity + segment_limit;
			}
			else if (((ss->ss_segment_register_descriptor_type_e == 0) && (segment > ss_segment_limit)) || ((ss->ss_segment_register_descriptor_type_e == 1) && (segment < ss_segment_limit))) {
				/* Outside CS, DS, ES, FS, or GS segment limits (normally 0FFFFH); Interrupt 13 goes here: */
			}
			else {
				c->ea.seg = segment;
				c->ea.ofs = offset;
				e86_set_mem16 (c, c->seg.ofs, c->ea.ofs, val)
			}
		}
		else {
			/* Normal memory address specified */
			c->ea.seg = segment;
			c->ea.ofs = offset;
			e86_set_mem16 (c, c->seg.ofs, c->ea.ofs, val)
		}
	}
	else {
		e386_set_reg16 (c, c->ea.ofs, val);           /* Set 16-bit CPU register to offset if effective address is not memory operand          */
	}
}

void e386_protected_mode_set_ea16_or_32 (e8086_t *c, cs_segment_register_descriptor_t *cs, ds_segment_register_descriptor_t *ds, es_segment_register_descriptor_t *es, fs_segment_register_descriptor_t *fs, gs_segment_register_descriptor_t *gs, ss_segment_register_descriptor_t *ss, ldt_local_descriptor_t *ldt, gdt_global_descriptor_t *gdt, tss_task_state_segment_descriptor_t *tss, unsigned long val) /* Protected Mode */
{
	/* 16-bit or 32-bit memory address specified */
	if (c->ea.is_mem) {
		/* Start with Global Descriptor Table: */
		gdt = gdt_get_global_segment_descriptor (gdt, segment, offset & 0xffffffff);
		if (segment == NULL) {
			/* Search inside of Local Descriptor Table instead */
			ldt = ldt_get_local_segment_descriptor (ldt, segment, offset & 0xffffffff);
			if (segment == NULL) {
				/* Search inside of Task State Segment Descriptor Table instead */
				tss = tss_get_task_state_segment_descriptor (tss, segment, offset & 0xffffffff);
				if (segment == NULL) {
					if ((segment) == (cs->cs_segment_register_selector_index) {
						cs_segment_limit = cs->cs_segment_register_descriptor_limit;
						if (cs->cs_segment_register_descriptor_granularity_bit == 1) {
							/* Use below for when granularity bit occurs: */
							segment_limit = cs_segment_limit;
							granularity = 0xffffffff;
							cs_segment_limit = granularity + segment_limit;
						}
						else if (((cs->cs_segment_register_descriptor_type_e == 0) && (segment > cs_segment_limit)) || ((cs->cs_segment_register_descriptor_type_e == 1) && (segment < cs_segment_limit))) {
							/* Outside CS, DS, ES, FS, or GS segment limits; General Protection Fault goes here: */
						}
						else if (cs->cs_segment_register_descriptor_is_present == 0) {
							/* Segment is not valid; General Protection Fault goes here: */
						}
						else if (segment == 0x0000) {
							/* Null selector; General Protection Fault goes here: */
						}
						else {
							c->ea.seg = segment;
							c->ea.ofs = offset;
							if ((cs->cs_segment_register_descriptor_big == 1) || (cs->cs_segment_register_descriptor_default == 1) || (c->e386_address_size_prefix_67h == 1)) {
								/* 32-bit memory address operand specified */
								e386_set_mem32 (c, c->seg.ofs, c->ea.ofs, val)
							}
							else {
								/* 16-bit memory address operand specified */
								e86_set_mem16 (c, c->seg.ofs, c->ea.ofs, val)
							}
						}
					}
					else if ((segment) == (ds->ds_segment_register_selector_index) {
						ds_segment_limit = ds->ds_segment_register_descriptor_limit;
						if (ds->ds_segment_register_descriptor_granularity_bit == 1) {
							/* Use below for when granularity bit occurs: */
							segment_limit = ds_segment_limit;
							granularity = 0xffffffff;
							ds_segment_limit = granularity + segment_limit;
						}
						else if (((ds->ds_segment_register_descriptor_type_e == 0) && (segment > ds_segment_limit)) || ((ds->ds_segment_register_descriptor_type_e == 1) && (segment < ds_segment_limit))) {
							/* Outside CS, DS, ES, FS, or GS segment limits; General Protection Fault goes here: */
						}
						else if (ds->ds_segment_register_descriptor_is_present == 0) {
							/* Segment is not valid; General Protection Fault goes here: */
						}
						else if (segment == 0x0000) {
							/* Null selector; General Protection Fault goes here: */
						}
						else {
							c->ea.seg = segment;
							c->ea.ofs = offset;
							if ((ds->ds_segment_register_descriptor_big == 1) || (ds->ds_segment_register_descriptor_default == 1) || (c->e386_address_size_prefix_67h == 1)) {
								/* 32-bit memory address operand specified */
								e386_set_mem32 (c, c->seg.ofs, c->ea.ofs, val)
							}
							else {
								/* 16-bit memory address operand specified */
								e86_set_mem16 (c, c->seg.ofs, c->ea.ofs, val)
							}
						}
					}
					else if ((segment) == (es->es_segment_register_selector_index) {
						es_segment_limit = es->es_segment_register_descriptor_limit;
						if (es->es_segment_register_descriptor_granularity_bit == 1) {
							/* Use below for when granularity bit occurs: */
							segment_limit = es_segment_limit;
							granularity = 0xffffffff;
							es_segment_limit = granularity + segment_limit;
						}
						else if (((es->es_segment_register_descriptor_type_e == 0) && (segment > es_segment_limit)) || ((es->es_segment_register_descriptor_type_e == 1) && (segment < es_segment_limit))) {
							/* Outside CS, DS, ES, FS, or GS segment limits; General Protection Fault goes here: */
						}
						else if (es->es_segment_register_descriptor_is_present == 0) {
							/* Segment is not valid; General Protection Fault goes here: */
						}
						else if (segment == 0x0000) {
							/* Null selector; General Protection Fault goes here: */
						}
						else {
							c->ea.seg = segment;
							c->ea.ofs = offset;
							if ((es->es_segment_register_descriptor_big == 1) || (es->es_segment_register_descriptor_default == 1) || (c->e386_address_size_prefix_67h == 1)) {
								/* 32-bit memory address operand specified */
								e386_set_mem32 (c, c->seg.ofs, c->ea.ofs, val)
							}
							else {
								/* 16-bit memory address operand specified */
								e86_set_mem16 (c, c->seg.ofs, c->ea.ofs, val)
							}
						}
					}
					else if ((segment) == (fs->fs_segment_register_selector_index) {
						fs_segment_limit = fs->fs_segment_register_descriptor_limit;
						if (fs->fs_segment_register_descriptor_granularity_bit == 1) {
							/* Use below for when granularity bit occurs: */
							segment_limit = fs_segment_limit;
							granularity = 0xffffffff;
							fs_segment_limit = granularity + segment_limit;
						}
						else if (((fs->fs_segment_register_descriptor_type_e == 0) && (segment > fs_segment_limit)) || ((fs->fs_segment_register_descriptor_type_e == 1) && (segment < fs_segment_limit))) {
							/* Outside CS, DS, ES, FS, or GS segment limits; General Protection Fault goes here: */
						}
						else if (fs->fs_segment_register_descriptor_is_present == 0) {
							/* Segment is not valid; General Protection Fault goes here: */
						}
						else if (segment == 0x0000) {
							/* Null selector; General Protection Fault goes here: */
						}
						else {
							c->ea.seg = segment;
							c->ea.ofs = offset;
							if ((fs->fs_segment_register_descriptor_big == 1) || (fs->fs_segment_register_descriptor_default == 1) || (c->e386_address_size_prefix_67h == 1)) {
								/* 32-bit memory address operand specified */
								e386_set_mem32 (c, c->seg.ofs, c->ea.ofs, val)
							}
							else {
								/* 16-bit memory address operand specified */
								e86_set_mem16 (c, c->seg.ofs, c->ea.ofs, val)
							}
						}
					}
					else if ((segment) == (gs->gs_segment_register_selector_index) {
						gs_segment_limit = gs->gs_segment_register_descriptor_limit;
						if (gs->gs_segment_register_descriptor_granularity_bit == 1) {
							/* Use below for when granularity bit occurs: */
							segment_limit = gs_segment_limit;
							granularity = 0xffffffff;
							gs_segment_limit = granularity + segment_limit;
						}
						else if (((gs->gs_segment_register_descriptor_type_e == 0) && (segment > gs_segment_limit)) || ((gs->gs_segment_register_descriptor_type_e == 1) && (segment < gs_segment_limit))) {
							/* Outside CS, DS, ES, FS, or GS segment limits; General Protection Fault goes here: */
						}
						else if (gs->gs_segment_register_descriptor_is_present == 0) {
							/* Segment is not valid; General Protection Fault goes here: */
						}
						else if (segment == 0x0000) {
							/* Null selector; General Protection Fault goes here: */
						}
						else {
							c->ea.seg = segment;
							c->ea.ofs = offset;
							if ((gs->gs_segment_register_descriptor_big == 1) || (gs->gs_segment_register_descriptor_default == 1) || (c->e386_address_size_prefix_67h == 1)) {
								/* 32-bit memory address operand specified */
								e386_set_mem32 (c, c->seg.ofs, c->ea.ofs, val)
							}
							else {
								/* 16-bit memory address operand specified */
								e86_set_mem16 (c, c->seg.ofs, c->ea.ofs, val)
							}
						}
					}
					else if ((segment) == (ss->ss_segment_register_selector_index) {
						ss_segment_limit = ss->ss_segment_register_descriptor_limit;
						if (ss->ss_segment_register_descriptor_granularity_bit == 1) {
							/* Use below for when granularity bit occurs: */
							segment_limit = ss_segment_limit;
							granularity = 0xffffffff;
							ss_segment_limit = granularity + segment_limit;
						}
						else if (((ss->ss_segment_register_descriptor_type_e == 0) && (segment > ss_segment_limit)) || ((ss->ss_segment_register_descriptor_type_e == 1) && (segment < ss_segment_limit))) {
							/* Outside SS segment limit; Stack Fault goes here: */
						}
						else if (ss->ss_segment_register_descriptor_is_present == 0) {
							/* Segment is not valid; Stack Fault goes here: */
						}
						else if (segment == 0x0000) {
							/* Null selector; General Protection Fault goes here: */
						}
						else {
							c->ea.seg = segment;
							c->ea.ofs = offset;
							if ((ss->ss_segment_register_descriptor_big == 1) || (ss->ss_segment_register_descriptor_default == 1) || (c->e386_address_size_prefix_67h == 1)) {
								/* 32-bit memory address operand specified */
								e386_set_mem32 (c, c->seg.ofs, c->ea.ofs, val)
							}
							else {
								/* 16-bit memory address operand specified */
								e86_set_mem16 (c, c->seg.ofs, c->ea.ofs, val)
							}
						}
					}
					else {
						/* Segment obviously does not exist; possibly use actual CPU exception if required */
					}
				}
				tss_segment_limit = tss->tss_gdt_segment_limit;
				else if (tss->tss_gdt_segment_is_present == 0) {
					/* Segment is not valid; General Protection Fault goes here: */
				}
				else if (((tss->tss_gdt_type_e == 0) && ((segment) || (offset) > (tss_segment_limit))) || ((tss->tss_gdt_type_e == 1) && ((segment) || (offset) < (tss->tss_gdt_segment_limit)))) {
					/* Outside segment limits; General Protection Fault goes here: */
				}
				else if (tss->tss_gdt_type_w == 0) {
					/* Non-writable segment; General Protection Fault goes here: */
				}
				else {
					if ((tss->tss_gdt_big == 1) || (tss->tss_gdt_default == 1) || ((tss->tss_gdt_big == 0) && (c->e386_address_size_prefix_67h == 1)) || ((tss->tss_gdt_default == 0) && (c->e386_address_size_prefix_67h == 1))) {
						/* 32-bit memory address specified */
						c->ea.seg = segment & 0xffffffff;  /* Set 32-bit memory address to segment register and offset if address is memory operand */
						c->ea.ofs = offset & 0xffffffff;  /* Set 32-bit memory address to segment register and offset if address is memory operand */
						e86_set_mem32 (c, c->seg.ofs, c->ea.ofs, val);  /* Set 32-bit memory address to segment register and offset if address is memory operand */
					}
					else {
						/* 16-bit memory address specified */
						c->ea.seg = segment & 0xffff;  /* Set 16-bit memory address to segment register and offset if address is memory operand */
						c->ea.ofs = offset & 0xffff;  /* Set 16-bit memory address to segment register and offset if address is memory operand */
						e86_set_mem16 (c, c->seg.ofs, c->ea.ofs, val);  /* Set 16-bit memory address to segment register and offset if address is memory operand */
					}
				}
			}
			ldt_segment_limit = ldt->ldt_segment_limit;
			else if (ldt->ldt_segment_is_present == 0) {
				/* Segment is not valid; General Protection Fault goes here: */
			}
			else if (((ldt->ldt_type_e == 0) && ((segment) || (offset) > (ldt_segment_limit))) || ((ldt->ldt_type_e == 1) && ((segment) || (offset) < (ldt->ldt_segment_limit)))) {
				/* Outside segment limits; General Protection Fault goes here: */
			}
			else if (ldt->ldt_type_w == 0) {
				/* Non-writable segment; General Protection Fault goes here: */
			}
			else {
				if ((ldt->ldt_big == 1) || (ldt->ldt_default == 1) || ((ldt->gdt_big == 0) && (c->e386_address_size_prefix_67h == 1)) || ((ldt->gdt_default == 0) && (c->e386_address_size_prefix_67h == 1))) {
					/* 32-bit memory address specified */
					c->ea.seg = segment & 0xffffffff;  /* Set 32-bit memory address to segment register and offset if address is memory operand */
					c->ea.ofs = offset & 0xffffffff;  /* Set 32-bit memory address to segment register and offset if address is memory operand */
					e86_set_mem32 (c, c->seg.ofs, c->ea.ofs, val);  /* Set 32-bit memory address to segment register and offset if address is memory operand */
				}
				else {
					/* 16-bit memory address specified */
					c->ea.seg = segment & 0xffff;  /* Set 16-bit memory address to segment register and offset if address is memory operand */
					c->ea.ofs = offset & 0xffff;  /* Set 16-bit memory address to segment register and offset if address is memory operand */
					e86_set_mem16 (c, c->seg.ofs, c->ea.ofs, val);  /* Set 16-bit memory address to segment register and offset if address is memory operand */
				}
			}
		}
		gdt_segment_limit = gdt->gdt_segment_limit;
		else if (gdt->gdt_segment_is_present == 0) {
			/* Segment is not valid; General Protection Fault goes here: */
		}
		else if (((gdt->gdt_type_e == 0) && ((segment) || (offset) > (gdt_segment_limit))) || ((gdt->gdt_type_e == 1) && ((segment) || (offset) < (gdt->gdt_segment_limit)))) {
			/* Outside segment limits; General Protection Fault goes here: */
		}
		else if (gdt->gdt_type_w == 0) {
			/* Non-writable segment; General Protection Fault goes here: */
		}
		else {
			if ((gdt->gdt_big == 1) || (gdt->gdt_default == 1) || ((gdt->gdt_big == 0) && (c->e386_address_size_prefix_67h == 1)) || ((gdt->gdt_default == 0) && (c->e386_address_size_prefix_67h == 1))) {
				/* 32-bit memory address specified */
				c->ea.seg = segment & 0xffffffff;  /* Set 32-bit memory address to segment register and offset if address is memory operand */
				c->ea.ofs = offset & 0xffffffff;  /* Set 32-bit memory address to segment register and offset if address is memory operand */
				e86_set_mem32 (c, c->seg.ofs, c->ea.ofs, val);  /* Set 32-bit memory address to segment register and offset if address is memory operand */
			}
			else {
				/* 16-bit memory address specified */
				c->ea.seg = segment & 0xffff;  /* Set 16-bit memory address to segment register and offset if address is memory operand */
				c->ea.ofs = offset & 0xffff;  /* Set 16-bit memory address to segment register and offset if address is memory operand */
				e86_set_mem16 (c, c->seg.ofs, c->ea.ofs, val);  /* Set 16-bit memory address to segment register and offset if address is memory operand */
			}
		}
	}
	else {
		if (c->e386_operand_size_prefix_66h (c) == 1) /* 32-bit register operand specified */
		{
			c->ea.ofs = offset & 0xffffffff;
			e86_set_reg32 (c, c->ea.ofs, val);             /* Set 32-bit CPU register to offset if effective address is not memory operand          */
		}
		else /* 16-bit register operand specified (shifted left by four bits to 20-bits wide) */
		{
			c->ea.ofs = offset & 0xffff;
			e86_set_reg16 (c, c->ea.ofs, val);             /* Set 16-bit CPU register to offset if effective address is not memory operand          */
		}
		c->e386_operand_size_prefix_66h == 0;
	}
	c->e386_address_size_prefix_67h == 1; /* Disable address size prefix for each instruction once used */
}

void e386_real_mode_set_ea32 (e8086_t *c, unsigned long val) /* Real Mode */
{
	/* 32-bit memory address specified */
	if (c->ea.is_mem) {
		if ((segment) == (cs->cs_segment_register_selector_index) {
			cs_segment_limit = cs->cs_segment_register_descriptor_limit;
			if (cs->cs_segment_register_descriptor_granularity_bit == 1) {
				/* Use below for when granularity bit occurs: */
				segment_limit = cs_segment_limit;
				granularity = 0xffffffff;
				cs_segment_limit = granularity + segment_limit;
			}
			else if (((cs->cs_segment_register_descriptor_type_e == 0) && (segment > cs_segment_limit)) || ((cs->cs_segment_register_descriptor_type_e == 1) && (segment < cs_segment_limit))) {
				/* Outside CS, DS, ES, FS, or GS segment limits (normally 0FFFFH); Interrupt 13 goes here: */
			}
			else {
				c->ea.seg = segment;
				c->ea.ofs = offset;
				e386_set_mem32 (c, c->seg.ofs, c->ea.ofs, val)
			}
		}
		else if ((segment) == (ds->ds_segment_register_selector_index) {
			ds_segment_limit = ds->ds_segment_register_descriptor_limit;
			if (ds->ds_segment_register_descriptor_granularity_bit == 1) {
				/* Use below for when granularity bit occurs: */
				segment_limit = ds_segment_limit;
				granularity = 0xffffffff;
				ds_segment_limit = granularity + segment_limit;
			}
			else if (((ds->ds_segment_register_descriptor_type_e == 0) && (segment > ds_segment_limit)) || ((ds->ds_segment_register_descriptor_type_e == 1) && (segment < ds_segment_limit))) {
				/* Outside CS, DS, ES, FS, or GS segment limits (normally 0FFFFH); Interrupt 13 goes here: */
			}
			else {
				c->ea.seg = segment;
				c->ea.ofs = offset;
				e386_set_mem32 (c, c->seg.ofs, c->ea.ofs, val)
			}
		}
		else if ((segment) == (es->es_segment_register_selector_index) {
			es_segment_limit = es->es_segment_register_descriptor_limit;
			if (es->es_segment_register_descriptor_granularity_bit == 1) {
				/* Use below for when granularity bit occurs: */
				segment_limit = es_segment_limit;
				granularity = 0xffffffff;
				es_segment_limit = granularity + segment_limit;
			}
			else if (((es->es_segment_register_descriptor_type_e == 0) && (segment > es_segment_limit)) || ((es->es_segment_register_descriptor_type_e == 1) && (segment < es_segment_limit))) {
				/* Outside CS, DS, ES, FS, or GS segment limits (normally 0FFFFH); Interrupt 13 goes here: */
			}
			else {
				c->ea.seg = segment;
				c->ea.ofs = offset;
				e386_set_mem32 (c, c->seg.ofs, c->ea.ofs, val)
			}
		}
		else if ((segment) == (fs->fs_segment_register_selector_index) {
			fs_segment_limit = fs->fs_segment_register_descriptor_limit;
			if (fs->fs_segment_register_descriptor_granularity_bit == 1) {
				/* Use below for when granularity bit occurs: */
				segment_limit = fs_segment_limit;
				granularity = 0xffffffff;
				fs_segment_limit = granularity + segment_limit;
			}
			else if (((fs->fs_segment_register_descriptor_type_e == 0) && (segment > fs_segment_limit)) || ((fs->fs_segment_register_descriptor_type_e == 1) && (segment < fs_segment_limit))) {
				/* Outside CS, DS, ES, FS, or GS segment limits (normally 0FFFFH); Interrupt 13 goes here: */
			}
			else {
				c->ea.seg = segment;
				c->ea.ofs = offset;
				e386_set_mem32 (c, c->seg.ofs, c->ea.ofs, val)
			}
		}
		else if ((segment) == (gs->gs_segment_register_selector_index) {
			gs_segment_limit = gs->gs_segment_register_descriptor_limit;
			if (gs->gs_segment_register_descriptor_granularity_bit == 1) {
				/* Use below for when granularity bit occurs: */
				segment_limit = gs_segment_limit;
				granularity = 0xffffffff;
				gs_segment_limit = granularity + segment_limit;
			}
			else if (((gs->gs_segment_register_descriptor_type_e == 0) && (segment > gs_segment_limit)) || ((gs->gs_segment_register_descriptor_type_e == 1) && (segment < gs_segment_limit))) {
				/* Outside CS, DS, ES, FS, or GS segment limits (normally 0FFFFH); Interrupt 13 goes here: */
			}
			else {
				c->ea.seg = segment;
				c->ea.ofs = offset;
				e386_set_mem32 (c, c->seg.ofs, c->ea.ofs, val)
			}
		}
		else if ((segment) == (ss->ss_segment_register_selector_index) {
			ss_segment_limit = ss->ss_segment_register_descriptor_limit;
			if (ss->ss_segment_register_descriptor_granularity_bit == 1) {
				/* Use below for when granularity bit occurs: */
				segment_limit = ss_segment_limit;
				granularity = 0xffffffff;
				ss_segment_limit = granularity + segment_limit;
			}
			else if (((ss->ss_segment_register_descriptor_type_e == 0) && (segment > ss_segment_limit)) || ((ss->ss_segment_register_descriptor_type_e == 1) && (segment < ss_segment_limit))) {
				/* Outside SS segment limit (normally 0FFFFH); Interrupt 13 goes here: */
			}
			else {
				c->ea.seg = segment;
				c->ea.ofs = offset;
				e386_set_mem32 (c, c->seg.ofs, c->ea.ofs, val)
			}
		}
		else {
			/* Normal memory address specified */
			e386_set_mem32 (c, c->ea.seg, c->ea.ofs, val);
		}
	}
	else {
		e386_set_reg32 (c, c->ea.ofs, val);           /* Set 16-bit CPU register to offset if effective address is not memory operand          */
	}
}

void e386_virtual_8086_mode_set_ea32 (e8086_t *c, unsigned long val) /* Virtual 8086 Mode */
{
	/* 32-bit memory address specified */
	if (c->ea.is_mem) {
		if ((segment) == (cs->cs_segment_register_selector_index) {
			cs_segment_limit = cs->cs_segment_register_descriptor_limit;
			if (cs->cs_segment_register_descriptor_granularity_bit == 1) {
				/* Use below for when granularity bit occurs: */
				segment_limit = cs_segment_limit;
				granularity = 0xffffffff;
				cs_segment_limit = granularity + segment_limit;
			}
			else if (((cs->cs_segment_register_descriptor_type_e == 0) && (segment > cs_segment_limit)) || ((cs->cs_segment_register_descriptor_type_e == 1) && (segment < cs_segment_limit))) {
				/* Outside CS, DS, ES, FS, or GS segment limits (normally 0FFFFH); Interrupt 13 goes here: */
			}
			else {
				c->ea.seg = segment;
				c->ea.ofs = offset;
				e386_set_mem32 (c, c->seg.ofs, c->ea.ofs, val)
			}
		}
		else if ((segment) == (ds->ds_segment_register_selector_index) {
			ds_segment_limit = ds->ds_segment_register_descriptor_limit;
			if (ds->ds_segment_register_descriptor_granularity_bit == 1) {
				/* Use below for when granularity bit occurs: */
				segment_limit = ds_segment_limit;
				granularity = 0xffffffff;
				ds_segment_limit = granularity + segment_limit;
			}
			else if (((ds->ds_segment_register_descriptor_type_e == 0) && (segment > ds_segment_limit)) || ((ds->ds_segment_register_descriptor_type_e == 1) && (segment < ds_segment_limit))) {
				/* Outside CS, DS, ES, FS, or GS segment limits (normally 0FFFFH); Interrupt 13 goes here: */
			}
			else {
				c->ea.seg = segment;
				c->ea.ofs = offset;
				e386_set_mem32 (c, c->seg.ofs, c->ea.ofs, val)
			}
		}
		else if ((segment) == (es->es_segment_register_selector_index) {
			es_segment_limit = es->es_segment_register_descriptor_limit;
			if (es->es_segment_register_descriptor_granularity_bit == 1) {
				/* Use below for when granularity bit occurs: */
				segment_limit = es_segment_limit;
				granularity = 0xffffffff;
				es_segment_limit = granularity + segment_limit;
			}
			else if (((es->es_segment_register_descriptor_type_e == 0) && (segment > es_segment_limit)) || ((es->es_segment_register_descriptor_type_e == 1) && (segment < es_segment_limit))) {
				/* Outside CS, DS, ES, FS, or GS segment limits (normally 0FFFFH); Interrupt 13 goes here: */
			}
			else {
				c->ea.seg = segment;
				c->ea.ofs = offset;
				e386_set_mem32 (c, c->seg.ofs, c->ea.ofs, val)
			}
		}
		else if ((segment) == (fs->fs_segment_register_selector_index) {
			fs_segment_limit = fs->fs_segment_register_descriptor_limit;
			if (fs->fs_segment_register_descriptor_granularity_bit == 1) {
				/* Use below for when granularity bit occurs: */
				segment_limit = fs_segment_limit;
				granularity = 0xffffffff;
				fs_segment_limit = granularity + segment_limit;
			}
			else if (((fs->fs_segment_register_descriptor_type_e == 0) && (segment > fs_segment_limit)) || ((fs->fs_segment_register_descriptor_type_e == 1) && (segment < fs_segment_limit))) {
				/* Outside CS, DS, ES, FS, or GS segment limits (normally 0FFFFH); Interrupt 13 goes here: */
			}
			else {
				c->ea.seg = segment;
				c->ea.ofs = offset;
				e386_set_mem32 (c, c->seg.ofs, c->ea.ofs, val)
			}
		}
		else if ((segment) == (gs->gs_segment_register_selector_index) {
			gs_segment_limit = gs->gs_segment_register_descriptor_limit;
			if (gs->gs_segment_register_descriptor_granularity_bit == 1) {
				/* Use below for when granularity bit occurs: */
				segment_limit = gs_segment_limit;
				granularity = 0xffffffff;
				gs_segment_limit = granularity + segment_limit;
			}
			else if (((gs->gs_segment_register_descriptor_type_e == 0) && (segment > gs_segment_limit)) || ((gs->gs_segment_register_descriptor_type_e == 1) && (segment < gs_segment_limit))) {
				/* Outside CS, DS, ES, FS, or GS segment limits (normally 0FFFFH); Interrupt 13 goes here: */
			}
			else {
				c->ea.seg = segment;
				c->ea.ofs = offset;
				e386_set_mem32 (c, c->seg.ofs, c->ea.ofs, val)
			}
		}
		else if ((segment) == (ss->ss_segment_register_selector_index) {
			ss_segment_limit = ss->ss_segment_register_descriptor_limit;
			if (ss->ss_segment_register_descriptor_granularity_bit == 1) {
				/* Use below for when granularity bit occurs: */
				segment_limit = ss_segment_limit;
				granularity = 0xffffffff;
				ss_segment_limit = granularity + segment_limit;
			}
			else if (((ss->ss_segment_register_descriptor_type_e == 0) && (segment > ss_segment_limit)) || ((ss->ss_segment_register_descriptor_type_e == 1) && (segment < ss_segment_limit))) {
				/* Outside SS segment limit (normally 0FFFFH); Interrupt 13 goes here: */
			}
			else {
				c->ea.seg = segment;
				c->ea.ofs = offset;
				e386_set_mem32 (c, c->seg.ofs, c->ea.ofs, val)
			}
		}
		else {
			/* Normal memory address specified */
			e386_set_mem32 (c, c->ea.seg, c->ea.ofs, val);
		}
	}
	else {
		e386_set_reg32 (c, c->ea.ofs, val);           /* Set 16-bit CPU register to offset if effective address is not memory operand          */
	}
}
