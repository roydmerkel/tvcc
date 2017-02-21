/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/e8086/flags.c                                        *
 * Created:     2003-04-18 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2009 Hampa Hug <hampa@hampa.ch>                     *
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


static
char parity[256] = {
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0
};


/*************************************************************************
 * Flags functions
 *************************************************************************/

void e86_set_flg_szp_8 (e8086_t *c, unsigned char val)
{
	unsigned short set;
	set = 0;
	val &= 0xff;
	if (val == 0) {
		set |= E86_FLG_ZF;
	}
	else if (val & 0x80) {
		set |= E86_FLG_SF;
	}
	if (parity[val] == 0) {
		set |= E86_FLG_PF;
	}
	c->flg &= ~(E86_FLG_SF | E86_FLG_ZF | E86_FLG_PF);
	c->flg |= set;
}

void e86_set_flg_szp_16 (e8086_t *c, unsigned short val)
{
	unsigned short set;
	set = 0;
	if ((val & 0xffff) == 0) {
		set |= E86_FLG_ZF;
	}
	else if (val & 0x8000) {
		set |= E86_FLG_SF;
	}
	if (parity[val & 0xff] == 0) {
		set |= E86_FLG_PF;
	}
	c->flg &= ~(E86_FLG_SF | E86_FLG_ZF | E86_FLG_PF);
	c->flg |= set;
}

void e86_set_flg_szp_32(e8086_t *c, unsigned long val)
{
	unsigned long set;
	set = 0;
	if ((val & 0xffffffff) == 0) {
		set |= E86_FLG_ZF;
	}
	else if (val & 0x80000000) {
		set |= E86_FLG_SF;
	}
	if (parity[val & 0xffff] == 0) {
		set |= E86_FLG_PF;
	}
	c->flg &= ~(E86_FLG_SF | E86_FLG_ZF | E86_FLG_PF);
	c->flg |= set;
}

void e86_set_flg_log_8 (e8086_t *c, unsigned char val)
{
	e86_set_flg_szp_8 (c, val);
	c->flg &= ~(E86_FLG_CF | E86_FLG_OF);
}

void e86_set_flg_log_16 (e8086_t *c, unsigned short val)
{
	e86_set_flg_szp_16 (c, val);
	c->flg &= ~(E86_FLG_CF | E86_FLG_OF);
}

void e86_set_flg_log_32(e8086_t *c, unsigned long val)
{
	e86_set_flg_szp_32(c, val);
	c->flg &= ~(E86_FLG_CF | E86_FLG_OF);
}

void e86_set_flg_add_8 (e8086_t *c, unsigned char s1, unsigned char s2)
{
	unsigned short set;
	unsigned short dst;
	e86_set_flg_szp_8 (c, s1 + s2);
	set = 0;
	dst = (unsigned short) s1 + (unsigned short) s2;
	if (dst & 0xff00) {
		set |= E86_FLG_CF;
	}
	if ((dst ^ s1) & (dst ^ s2) & 0x80) {
		set |= E86_FLG_OF;
	}
	if ((s1 ^ s2 ^ dst) & 0x10) {
		set |= E86_FLG_AF;
	}
	c->flg &= ~(E86_FLG_CF | E86_FLG_OF | E86_FLG_AF);
	c->flg |= set;
}

void e86_set_flg_add_16 (e8086_t *c, unsigned short s1, unsigned short s2)
{
	unsigned short set;
	unsigned long  dst;
	e86_set_flg_szp_16 (c, s1 + s2);
	set = 0;
	dst = (unsigned long) s1 + (unsigned long) s2;
	if (dst & 0xffff0000) {
		set |= E86_FLG_CF;
	}
	if ((dst ^ s1) & (dst ^ s2) & 0x8000) {
		set |= E86_FLG_OF;
	}
	if ((s1 ^ s2 ^ dst) & 0x10) {
		set |= E86_FLG_AF;
	}
	c->flg &= ~(E86_FLG_CF | E86_FLG_OF | E86_FLG_AF);
	c->flg |= set;
}

void e86_set_flg_add_32(e8086_t *c, unsigned long s1, unsigned long s2)
{
	unsigned long set;
	unsigned long long dst;
	e86_set_flg_szp_32(c, s1 + s2);
	set = 0;
	dst = (unsigned long long)s1 + (unsigned long long)s2;
	if (dst & 0xffffffff00000000) {
		set |= E86_FLG_CF;
	}
	if ((dst ^ s1) & (dst ^ s2) & 0x80000000) {
		set |= E86_FLG_OF;
	}
	if ((s1 ^ s2 ^ dst) & 0x1000) {
		set |= E86_FLG_AF;
	}
	c->flg &= ~(E86_FLG_CF | E86_FLG_OF | E86_FLG_AF);
	c->flg |= set;
}

void e86_set_flg_adc_8 (e8086_t *c, unsigned char s1, unsigned char s2, unsigned char s3)
{
	unsigned short set;
	unsigned short dst;
	e86_set_flg_szp_8 (c, s1 + s2 + s3);
	set = 0;
	dst = (unsigned short) s1 + (unsigned short) s2 + (unsigned short) s3;
	if (dst & 0xff00) {
		set |= E86_FLG_CF;
	}
	if ((dst ^ s1) & (dst ^ s2) & 0x80) {
		set |= E86_FLG_OF;
	}
	if ((s1 ^ s2 ^ dst) & 0x10) {
		set |= E86_FLG_AF;
	}
	c->flg &= ~(E86_FLG_CF | E86_FLG_OF | E86_FLG_AF);
	c->flg |= set;
}

void e86_set_flg_adc_16 (e8086_t *c, unsigned short s1, unsigned short s2, unsigned short s3)
{
	unsigned short set;
	unsigned long  dst;
	e86_set_flg_szp_16 (c, s1 + s2 + s3);
	set = 0;
	dst = (unsigned long) s1 + (unsigned long) s2 + (unsigned long) s3;
	if (dst & 0xffff0000) {
		set |= E86_FLG_CF;
	}
	if ((dst ^ s1) & (dst ^ s2) & 0x8000) {
		set |= E86_FLG_OF;
	}
	if ((s1 ^ s2 ^ dst) & 0x10) {
		set |= E86_FLG_AF;
	}
	c->flg &= ~(E86_FLG_CF | E86_FLG_OF | E86_FLG_AF);
	c->flg |= set;
}

void e86_set_flg_adc_32(e8086_t *c, unsigned long s1, unsigned long s2, unsigned long s3)
{
	unsigned long set;
	unsigned long long dst;
	e86_set_flg_szp_32(c, s1 + s2 + s3);
	set = 0;
	dst = (unsigned long long)s1 + (unsigned long long)s2 + (unsigned long long)s3;
	if (dst & 0xffffffff00000000) {
		set |= E86_FLG_CF;
	}
	if ((dst ^ s1) & (dst ^ s2) & 0x80000000) {
		set |= E86_FLG_OF;
	}
	if ((s1 ^ s2 ^ dst) & 0x1000) {
		set |= E86_FLG_AF;
	}
	c->flg &= ~(E86_FLG_CF | E86_FLG_OF | E86_FLG_AF);
	c->flg |= set;
}

void e86_set_flg_sbb_8 (e8086_t *c, unsigned char s1, unsigned char s2, unsigned char s3)
{
	unsigned short set;
	unsigned short dst;
	e86_set_flg_szp_8 (c, s1 - s2 - s3);
	set = 0;
	dst = s1 - s2 - s3;
	if (dst & 0xff00) {
		set |= E86_FLG_CF;
	}
	if ((s1 ^ dst) & (s1 ^ s2) & 0x80) {
		set |= E86_FLG_OF;
	}
	if ((s1 ^ s2 ^ dst) & 0x10) {
		set |= E86_FLG_AF;
	}
	c->flg &= ~(E86_FLG_CF | E86_FLG_OF | E86_FLG_AF);
	c->flg |= set;
}

void e86_set_flg_sbb_16 (e8086_t *c, unsigned short s1, unsigned short s2, unsigned short s3)
{
	unsigned short set;
	unsigned long  dst;
	e86_set_flg_szp_16 (c, s1 - s2 - s3);
	set = 0;
	dst = (unsigned long) s1 - (unsigned long) s2 - (unsigned long) s3;
	if (dst & 0xffff0000) {
		set |= E86_FLG_CF;
	}
	if ((s1 ^ dst) & (s1 ^ s2) & 0x8000) {
		set |= E86_FLG_OF;
	}
	if ((s1 ^ s2 ^ dst) & 0x10) {
		set |= E86_FLG_AF;
	}
	c->flg &= ~(E86_FLG_CF | E86_FLG_OF | E86_FLG_AF);
	c->flg |= set;
}

void e86_set_flg_sbb_32(e8086_t *c, unsigned long s1, unsigned long s2, unsigned long s3)
{
	unsigned long set;
	unsigned long long dst;
	e86_set_flg_szp_32(c, s1 - s2 - s3);
	set = 0;
	dst = (unsigned long long)s1 - (unsigned long long)s2 - (unsigned long long)s3;
	if (dst & 0xffffffff00000000) {
		set |= E86_FLG_CF;
	}
	if ((s1 ^ dst) & (s1 ^ s2) & 0x80000000) {
		set |= E86_FLG_OF;
	}
	if ((s1 ^ s2 ^ dst) & 0x1000) {
		set |= E86_FLG_AF;
	}
	c->flg &= ~(E86_FLG_CF | E86_FLG_OF | E86_FLG_AF);
	c->flg |= set;
}

void e86_set_flg_sub_8 (e8086_t *c, unsigned char s1, unsigned char s2)
{
	unsigned short set;
	unsigned short dst;
	e86_set_flg_szp_8 (c, s1 - s2);
	set = 0;
	dst = s1 - s2;
	if (dst & 0xff00) {
		set |= E86_FLG_CF;
	}
	if ((s1 ^ dst) & (s1 ^ s2) & 0x80) {
		set |= E86_FLG_OF;
	}
	if ((s1 ^ s2 ^ dst) & 0x10) {
		set |= E86_FLG_AF;
	}
	c->flg &= ~(E86_FLG_CF | E86_FLG_OF | E86_FLG_AF);
	c->flg |= set;
}

void e86_set_flg_sub_16 (e8086_t *c, unsigned short s1, unsigned short s2)
{
	unsigned short set;
	unsigned long  dst;
	e86_set_flg_szp_16 (c, s1 - s2);
	set = 0;
	dst = (unsigned long) s1 - (unsigned long) s2;
	if (dst & 0xffff0000) {
		set |= E86_FLG_CF;
	}
	if ((s1 ^ dst) & (s1 ^ s2) & 0x8000) {
		set |= E86_FLG_OF;
	}
	if ((s1 ^ s2 ^ dst) & 0x10) {
		set |= E86_FLG_AF;
	}
	c->flg &= ~(E86_FLG_CF | E86_FLG_OF | E86_FLG_AF);
	c->flg |= set;
}

void e86_set_flg_sub_32(e8086_t *c, unsigned long s1, unsigned long s2)
{
	unsigned long set;
	unsigned long long dst;
	e86_set_flg_szp_32(c, s1 - s2);
	set = 0;
	dst = (unsigned long long)s1 - (unsigned long long)s2;
	if (dst & 0xffffffff00000000) {
		set |= E86_FLG_CF;
	}
	if ((s1 ^ dst) & (s1 ^ s2) & 0x80000000) {
		set |= E86_FLG_OF;
	}
	if ((s1 ^ s2 ^ dst) & 0x1000) {
		set |= E86_FLG_AF;
	}
	c->flg &= ~(E86_FLG_CF | E86_FLG_OF | E86_FLG_AF);
	c->flg |= set;
}
