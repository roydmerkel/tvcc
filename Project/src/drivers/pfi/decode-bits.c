/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pfi/decode-bits.c                                *
 * Created:     2012-01-21 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2015 Hampa Hug <hampa@hampa.ch>                     *
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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pfi.h"
#include "decode-bits.h"


void pfi_bitbuf_init (pfi_bitbuf_t *buf, void *src, unsigned long cnt, unsigned long max)
{
	buf->buf = src;
	buf->idx = 0;
	buf->cnt = cnt;
	buf->max = max;

	buf->free = 0;
	buf->wrap = 0;
}

int pfi_bitbuf_alloc (pfi_bitbuf_t *buf, unsigned long cnt, unsigned long max)
{
	if (max == 0) {
		buf->buf = NULL;
	}
	else {
		buf->buf = malloc (max);

		if (buf->buf == NULL) {
			return (1);
		}
	}

	pfi_bitbuf_init (buf, buf->buf, cnt, max);

	buf->free = 1;

	return (0);
}

void pfi_bitbuf_free (pfi_bitbuf_t *buf)
{
	if (buf->free) {
		free (buf->buf);
	}
}

static inline
void pfi_bitbuf_next_bit (pfi_bitbuf_t *buf)
{
	buf->idx += 1;

	if (buf->idx >= buf->max) {
		buf->wrap = 1;
		buf->idx = 0;
	}
}

int pfi_bitbuf_done (const pfi_bitbuf_t *buf)
{
	if ((buf->idx >= buf->cnt) || buf->wrap) {
		return (1);
	}

	return (0);
}

int pfi_bitbuf_get_bit (pfi_bitbuf_t *buf)
{
	int           ret;
	unsigned char m;

	m = 0x80 >> (buf->idx & 7);

	ret = ((buf->buf[buf->idx / 8] & m) != 0);

	pfi_bitbuf_next_bit (buf);

	return (ret);
}

void pfi_bitbuf_set_bit (pfi_bitbuf_t *buf, int val)
{
	unsigned char m;

	m = 0x80 >> (buf->idx & 7);

	if (val) {
		buf->buf[buf->idx & 7] |= m;
	}
	else {
		buf->buf[buf->idx & 7] &= ~m;
	}

	pfi_bitbuf_next_bit (buf);
}

void pfi_bitbuf_set_bits (pfi_bitbuf_t *buf, unsigned long val, unsigned cnt)
{
	unsigned      i;
	unsigned char m;

	i = buf->idx / 8;
	m = 0x80 >> (buf->idx & 7);

	while (cnt > 0) {
		cnt -= 1;

		if ((val >> cnt) & 1) {
			buf->buf[i] |= m;
		}
		else {
			buf->buf[i] &= ~m;
		}

		m >>= 1;

		if (m == 0) {
			i += 1;
			m = 0x80;
		}

		pfi_bitbuf_next_bit (buf);
	}
}

int pfi_bitbuf_add_bits (pfi_bitbuf_t *buf, unsigned long val, unsigned cnt)
{
	unsigned long i;
	unsigned char m;
	unsigned long max;
	unsigned char *tmp;

	if ((buf->cnt + cnt) > buf->max) {
		max = (buf->max < 4096) ? 4096 : buf->max;

		while ((buf->cnt + cnt) > max) {
			max *= 2;
		}

		tmp = realloc (buf->buf, (max + 7) / 8);

		if (tmp == NULL) {
			return (1);
		}

		buf->buf = tmp;
		buf->max = max;
	}

	i = buf->cnt / 8;
	m = 0x80 >> (buf->cnt & 7);

	while (cnt > 0) {
		cnt -= 1;

		if ((val >> cnt) & 1) {
			buf->buf[i] |= m;
		}
		else {
			buf->buf[i] &= ~m;
		}

		m >>= 1;

		if (m == 0) {
			i += 1;
			m = 0x80;
		}

		buf->cnt += 1;
	}

	return (0);
}

int pfi_trk_decode_bits (pfi_trk_t *trk, pfi_bitbuf_t *bit, pfi_bitbuf_t *fuz, unsigned long rate, unsigned rev)
{
	unsigned      idx;
	unsigned long cnt, val, ofs;
	unsigned char weak;
	double        cell, cell0, pulse;
	double        adjust1, adjust2, limit1, limit2, phase;

	pfi_trk_rewind (trk);

	if (rev < 1) {
		rev = 1;
	}

	cell0 = (double) trk->clock / rate;

	cnt = 0;
	ofs = 0;
	idx = 0;

	cell = cell0;
	pulse = 0.0;

	adjust1 = 0.9995;
	adjust2 = 1.0005;

	limit1 = 0.9 * cell0;
	limit2 = 1.1 * cell0;

	phase = 0.5;

#if 0
	phase = -0.25;
	adjust1 *= adjust1;
	adjust2 *= adjust2;
	limit1 = 0.9 * cell0;
	limit2 = 1.1 * cell0;
#endif

	weak = 0;

	while (pfi_trk_get_pulse (trk, &val, &ofs) == 0) {
		if ((val == 0) || (ofs < val)) {
			idx += 1;

			if (idx == rev) {
				bit->idx = 0;
				bit->cnt = 0;
			}
			else if (idx == (rev + 1)) {
				cnt = bit->cnt;
			}
			else if (idx > (rev + 1)) {
				break;
			}
		}

		if (val == 0) {
			continue;
		}

		pulse += (double) val / cell;

		while (pulse > 1.5) {
			pfi_bitbuf_add_bits (bit, 0, 1);

			if (fuz != NULL) {
				if (pulse < 1.6) {
					weak = 3;
				}

				pfi_bitbuf_add_bits (fuz, weak & 1, 1);

				weak >>= 1;
			}

			pulse -= 1.0;
		}

		pfi_bitbuf_add_bits (bit, 1, 1);
		pulse -= 1.0;

		if (fuz != NULL) {
			if (pulse > 0.4) {
				weak = 3;
			}

			pfi_bitbuf_add_bits (fuz, weak & 1, 1);

			weak >>= 1;
		}

		if (pulse < 0.0) {
			cell *= adjust1;

			if (cell < limit1) {
				cell = limit1;
			}
		}
		else if (pulse > 0.0) {
			cell *= adjust2;

			if (cell > limit2) {
				cell = limit2;
			}
		}

		pulse *= phase;
	}

	if (cnt > bit->cnt) {
		cnt = bit->cnt;
	}

	bit->max = bit->cnt;
	bit->cnt = cnt;

	return (0);
}
