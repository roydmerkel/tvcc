/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pfi/decode-bits.h                                *
 * Created:     2013-12-25 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2013-2015 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PFI_DECODE_BITS_H
#define PFI_DECODE_BITS_H 1


#include <drivers/pfi/track.h>


typedef struct {
	unsigned char *buf;
	unsigned long idx;
	unsigned long cnt;
	unsigned long max;

	char          free;
	char          wrap;
} pfi_bitbuf_t;


void pfi_bitbuf_init (pfi_bitbuf_t *buf, void *src, unsigned long cnt, unsigned long max);
int pfi_bitbuf_alloc (pfi_bitbuf_t *buf, unsigned long cnt, unsigned long max);
void pfi_bitbuf_free (pfi_bitbuf_t *buf);
int pfi_bitbuf_done (const pfi_bitbuf_t *buf);
int pfi_bitbuf_get_bit (pfi_bitbuf_t *buf);
void pfi_bitbuf_set_bit (pfi_bitbuf_t *buf, int val);
void pfi_bitbuf_set_bits (pfi_bitbuf_t *buf, unsigned long val, unsigned cnt);
int pfi_bitbuf_add_bits (pfi_bitbuf_t *buf, unsigned long val, unsigned cnt);

int pfi_trk_decode_bits (pfi_trk_t *trk, pfi_bitbuf_t *bit, pfi_bitbuf_t *fuz, unsigned long rate, unsigned rev);


#endif
