/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pfi/decode.c                                       *
 * Created:     2012-01-20 by Hampa Hug <hampa@hampa.ch>                     *
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


#include "main.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <drivers/pfi/pfi.h>
#include <drivers/pfi/decode-bits.h>

#include <drivers/pri/pri.h>
#include <drivers/pri/pri-img.h>


struct decode_bits_s {
	const char    *type;
	FILE          *fp;
	unsigned long rate;
};


struct decode_pri_s {
	pri_img_t     *img;
	unsigned long rate;
	unsigned      revolution;
};


static
int pfi_decode_bits_raw (FILE *fp, const unsigned char *buf, unsigned long cnt)
{
	if (fwrite (buf, (cnt + 7) / 8, 1, fp) != 1) {
		return (1);
	}

	return (0);
}

static
int pfi_decode_bits_mfm (FILE *fp, const unsigned char *buf, unsigned long cnt)
{
	unsigned msk;
	unsigned outbuf, outcnt;
	unsigned val, clk;

	msk = 0x80;

	val = 0;
	clk = 0xffff;

	outbuf = 0;
	outcnt = 0;

	while (cnt > 0) {
		val = (val << 1) | ((*buf & msk) != 0);
		clk = (clk << 1) | ((clk ^ 1) & 1);

		if ((val & 0xffff) == 0x4489) {
			if (outcnt > 0) {
				fputc (outbuf << (8 - outcnt), fp);
				outbuf = 0;
				outcnt = 0;
			}

			clk = 0xaaaa;
		}

		if ((clk & 0x8000) == 0) {
			outbuf = (outbuf << 1) | ((val & 0x8000) != 0);
			outcnt += 1;

			if (outcnt >= 8) {
				fputc (outbuf, fp);
				outbuf = 0;
				outcnt = 0;
			}
		}

		msk >>= 1;

		if (msk == 0) {
			msk = 0x80;
			buf += 1;
		}

		cnt -= 1;
	}

	return (0);
}

static
int pfi_decode_bits_gcr (FILE *fp, const unsigned char *buf, unsigned long cnt)
{
	unsigned val, msk;

	msk = 0x80;
	val = 0;

	while (cnt > 0) {
		val = (val << 1) | ((*buf & msk) != 0);

		if (val & 0x80) {
			fputc (val, fp);
			val = 0;
		}

		msk >>= 1;

		if (msk == 0) {
			msk = 0x80;
			buf += 1;
		}

		cnt -= 1;
	}

	return (0);
}

static
int pfi_decode_bits_cb (pfi_img_t *img, pfi_trk_t *trk, unsigned long c, unsigned long h, void *opaque)
{
	int                  r;
	struct decode_bits_s *par;
	pfi_bitbuf_t         bit;

	par = opaque;

	pfi_bitbuf_alloc (&bit, 0, 0);

	if (pfi_trk_decode_bits (trk, &bit, NULL, par->rate, par_revolution)) {
		pfi_bitbuf_free (&bit);
		return (1);
	}

	if (strcmp (par->type, "raw") == 0) {
		r = pfi_decode_bits_raw (par->fp, bit.buf, bit.cnt);
	}
	else if (strcmp (par->type, "gcr-raw") == 0) {
		r = pfi_decode_bits_gcr (par->fp, bit.buf, bit.cnt);
	}
	else if (strcmp (par->type, "mfm-raw") == 0) {
		r = pfi_decode_bits_mfm (par->fp, bit.buf, bit.cnt);
	}
	else {
		r = 1;
	}

	pfi_bitbuf_free (&bit);

	return (r);
}

int pfi_decode_bits (pfi_img_t *img, const char *type, unsigned long rate, const char *fname)
{
	int                  r;
	struct decode_bits_s par;

	par.type = type;
	par.rate = rate;
	par.fp = fopen (fname, "wb");

	if (par.fp == NULL) {
		return (1);
	}

	r = pfi_for_all_tracks (img, pfi_decode_bits_cb, &par);

	fclose (par.fp);

	return (r);
}

static
unsigned long pfi_bit_diff (const unsigned char *p1, unsigned long i1, const unsigned char *p2, unsigned long i2, unsigned long cnt)
{
	unsigned long i;
	unsigned      j1, j2;
	unsigned long ret;

	j1 = ~i1 & 7;
	j2 = ~i2 & 7;

	p1 += i1 / 8;
	p2 += i2 / 8;

	ret = 0;

	for (i = 0; i < cnt; i++) {
		ret += ((*p1 >> j1) ^ (*p2 >> j2)) & 1;

		if (j1 == 0) {
			p1 += 1;
			j1 = 7;
		}
		else {
			j1 -= 1;
		}

		if (j2 == 0) {
			p2 += 1;
			j2 = 7;
		}
		else {
			j2 -= 1;
		}
	}

	return (ret);
}

static
int pfi_bitbuf_fold (pfi_bitbuf_t *bit, pfi_bitbuf_t *fuz, unsigned c, unsigned h)
{
	unsigned      i;
	unsigned long n, d;
	unsigned long pos;
	unsigned long mind, minp, dist;

	mind = -1;
	minp = 0;

	for (i = 1; i < 256; i++) {
		pos = (i & 1) ? (bit->cnt - i / 2) : (bit->cnt + i / 2);

		if (pos >= bit->max) {
			continue;
		}

		n = bit->max - pos;

		if (n > pos) {
			n = pos;
		}

		n = (7 * n) / 8;

		d = pfi_bit_diff (bit->buf, 0, bit->buf, pos, n);

		if ((d < mind) || (i == 1)) {
			mind = d;
			minp = pos;

			if (mind <= (n / 1024)) {
				break;
			}
		}
	}

	dist = (minp < bit->cnt) ? (bit->cnt - minp) : (minp - bit->cnt);

	fprintf (stderr, "%u/%u: %6lu at %6lu %c %lu\n",
		c, h, mind, minp,
		(minp < bit->cnt) ? '-' : '+',
		dist
	);

	bit->cnt = minp;

	if (fuz != NULL) {
		fuz->cnt = minp;
	}

	return (0);
}

static
void pfi_decode_fuzzy (pri_trk_t *trk, pfi_bitbuf_t *fuz)
{
	unsigned long i;
	unsigned long val;

	val = 0;

	for (i = 0; i < fuz->cnt; i++) {
		val <<= 1;

		if (fuz->buf[i >> 3] & (0x80 >> (i & 7))) {
			val |= 1;
		}

		if (val & 0x80000000) {
			pri_trk_evt_add (trk, PRI_EVENT_FUZZY, i - 31, val);
			val = 0;
		}
	}
}

static
int pfi_decode_pri_trk_cb (pfi_img_t *img, pfi_trk_t *strk, unsigned long c, unsigned long h, void *opaque)
{
	pri_trk_t           *dtrk;
	struct decode_pri_s *par;
	pfi_bitbuf_t        bit, fuz, *pfuz;

	par = opaque;

	if ((dtrk = pri_img_get_track (par->img, c, h, 1)) == NULL) {
		return (1);
	}

	if (pfi_bitbuf_alloc (&bit, 0, 0)) {
		return (1);
	}

	if (par_weak_bits) {
		if (pfi_bitbuf_alloc (&fuz, 0, 0)) {
			return (1);
		}

		pfuz = &fuz;
	}
	else {
		pfuz = NULL;
	}

	pfi_trk_rewind (strk);

	if (pfi_trk_decode_bits (strk, &bit, pfuz, par->rate, par->revolution)) {
		pfi_bitbuf_free (&fuz);
		pfi_bitbuf_free (&bit);
		return (1);
	}

	pfi_bitbuf_fold (&bit, pfuz, c, h);

	pri_trk_set_clock (dtrk, par->rate);
	pri_trk_set_size (dtrk, bit.cnt);

	memcpy (dtrk->data, bit.buf, (bit.cnt + 7) / 8);

	if (pfuz != NULL) {
		pfi_decode_fuzzy (dtrk, pfuz);
	}

	if (pfuz != NULL) {
		pfi_bitbuf_free (pfuz);
	}

	pfi_bitbuf_free (&bit);

	pri_trk_clear_slack (dtrk);

	return (0);
}

static
pri_img_t *pfi_decode_pri (pfi_img_t *img, unsigned long rate)
{
	struct decode_pri_s par;

	if ((par.img = pri_img_new()) == NULL) {
		return (NULL);
	}

	par.rate = rate;
	par.revolution = par_revolution;

	if (pfi_for_all_tracks (img, pfi_decode_pri_trk_cb, &par)) {
		pri_img_del (par.img);
		return (NULL);
	}

	if (img->comment_size > 0) {
		pri_img_set_comment (par.img, img->comment, img->comment_size);
	}

	return (par.img);
}

int pfi_decode_bits_pri (pfi_img_t *img, unsigned long rate, const char *fname)
{
	int       r;
	FILE      *fp;
	pri_img_t *dimg;

	if ((dimg = pfi_decode_pri (img, rate)) == NULL) {
		return (1);
	}

	if ((fp = fopen (fname, "wb")) == NULL) {
		pri_img_del (dimg);
		return (1);
	}

	r = pri_img_save_fp (fp, dimg, PRI_FORMAT_PRI);

	fclose (fp);

	pri_img_del (dimg);

	return (r);
}


int pfi_decode (pfi_img_t *img, const char *type, unsigned long rate, const char *fname)
{
	if (rate == 0) {
		rate = 500000;
	}
	else if (rate <= 1000) {
		rate *= 1000;
	}

	if (strcmp (type, "pri") == 0) {
		return (pfi_decode_bits_pri (img, rate, fname));
	}
	else if (strcmp (type, "mfm-raw") == 0) {
		return (pfi_decode_bits (img, type, rate, fname));
	}
	else if (strcmp (type, "gcr-raw") == 0) {
		return (pfi_decode_bits (img, type, rate, fname));
	}
	else if (strcmp (type, "raw") == 0) {
		return (pfi_decode_bits (img, type, rate, fname));
	}

	return (1);
}
