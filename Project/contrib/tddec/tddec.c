/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   tddec/main.c                                                 *
 * Created:     2014-01-05 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2014 Hampa Hug <hampa@hampa.ch>                          *
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

#include <stdio.h>
#include <string.h>

#include "tddec.h"


#define TD_CRC_POLY 0xa097

#define LH1_MASK (LH1_N - 1)


static const unsigned char lh1_tab_code[256] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
	0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a,
	0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
	0x0c, 0x0c, 0x0c, 0x0c, 0x0d, 0x0d, 0x0d, 0x0d,
	0x0e, 0x0e, 0x0e, 0x0e, 0x0f, 0x0f, 0x0f, 0x0f,
	0x10, 0x10, 0x10, 0x10, 0x11, 0x11, 0x11, 0x11,
	0x12, 0x12, 0x12, 0x12, 0x13, 0x13, 0x13, 0x13,
	0x14, 0x14, 0x14, 0x14, 0x15, 0x15, 0x15, 0x15,
	0x16, 0x16, 0x16, 0x16, 0x17, 0x17, 0x17, 0x17,
	0x18, 0x18, 0x19, 0x19, 0x1a, 0x1a, 0x1b, 0x1b,
	0x1c, 0x1c, 0x1d, 0x1d, 0x1e, 0x1e, 0x1f, 0x1f,
	0x20, 0x20, 0x21, 0x21, 0x22, 0x22, 0x23, 0x23,
	0x24, 0x24, 0x25, 0x25, 0x26, 0x26, 0x27, 0x27,
	0x28, 0x28, 0x29, 0x29, 0x2a, 0x2a, 0x2b, 0x2b,
	0x2c, 0x2c, 0x2d, 0x2d, 0x2e, 0x2e, 0x2f, 0x2f,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
	0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
};

static const unsigned char lh1_tab_len[256] = {
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08
};


static
void print_help (void)
{
	fputs (
		"tddec: decompress teledisk td0 files\n"
		"usage: tddec input output\n",
		stdout
	);

	fflush (stdout);
}

static
void print_version (void)
{
	fputs (
		"tddec (pce) version 2014-12-08\n\n"
		"Copyright (C) 2014 Hampa Hug <hampa@hampa.ch>\n",
		stdout
	);

	fflush (stdout);
}

static
void lh1_tree_init (lh1_tree_t *tr)
{
	unsigned i, j;

	for (i = 0; i < LH1_SYM; i++) {
		tr->freq[i] = 1;
		tr->down[i] = ~i;
		tr->leaf[i] = i;
	}

	j = 0;

	for (i = LH1_SYM; i < LH1_NODES; i++) {
		tr->up[j + 0] = i;
		tr->up[j + 1] = i;

		tr->freq[i] = tr->freq[j] + tr->freq[j + 1];
		tr->down[i] = j;

		j += 2;
	}

	tr->up[LH1_ROOT] = 0;
}

static
void lh1_tree_scale (lh1_tree_t *tr)
{
	unsigned i, j, k, v;

	j = 0;
	for (i = 0; i < LH1_NODES; i++) {
		if (tr->down[i] & 0x8000) {
			tr->freq[j] = (tr->freq[i] + 1) / 2;
			tr->down[j] = tr->down[i];
			j += 1;
		}
	}

	j = 0;
	for (i = LH1_SYM; i < LH1_NODES; i++) {
		v = tr->freq[j] + tr->freq[j + 1];
		k = i;

		while (v < tr->freq[k - 1]) {
			tr->freq[k] = tr->freq[k - 1];
			tr->down[k] = tr->down[k - 1];
			k -= 1;
		}

		tr->freq[k] = v;
		tr->down[k] = j;

		j += 2;
	}

	for (i = 0; i < LH1_NODES; i++) {
		j = tr->down[i];

		if (j & 0x8000) {
			tr->leaf[~j & 0xffff] = i;
		}
		else {
			tr->up[j + 0] = i;
			tr->up[j + 1] = i;
		}
	}
}

static
void lh1_tree_update (lh1_tree_t *tr, unsigned sym)
{
	unsigned n1, n2, d1, d2;
	unsigned v;

	if (tr->freq[LH1_ROOT] & 0x8000) {
		lh1_tree_scale (tr);
	}

	n1 = tr->leaf[sym];

	while (n1 != LH1_ROOT) {
		tr->freq[n1] += 1;

		v = tr->freq[n1];
		n2 = n1 + 1;

		if (v <= tr->freq[n2]) {
			n1 = tr->up[n1];
			continue;
		}

		while (v > tr->freq[n2 + 1]) {
			n2 += 1;
		}

		tr->freq[n1] = tr->freq[n2];
		tr->freq[n2] = v;

		d1 = tr->down[n1];
		d2 = tr->down[n2];

		tr->down[n1] = d2;
		tr->down[n2] = d1;

		if (d1 & 0x8000) {
			tr->leaf[~d1 & 0xffff] = n2;
		}
		else {
			tr->up[d1 + 0] = n2;
			tr->up[d1 + 1] = n2;
		}

		if (d2 & 0x8000) {
			tr->leaf[~d2 & 0xffff] = n1;
		}
		else {
			tr->up[d2 + 0] = n1;
			tr->up[d2 + 1] = n1;
		}

		n1 = tr->up[n2];
	}

	tr->freq[LH1_ROOT] += 1;
}

static
void lh1_read_init (lh1_dec_t *lh, FILE *fp)
{
	lh->fp = fp;
	lh->eof = 0;

	lh->crc = 0;

	lh->bit_cnt = 0;
	lh->bit_buf = 0;

	lh->inp_idx = 0;
	lh->inp_cnt = 0;

	lh->buf_head = 0;
	lh->buf_tail = 0;
	lh->buf_cnt = 0;

	memset (lh->buf, ' ', LH1_N);

	lh1_tree_init (&lh->tr);
}

static
void lh1_clear_buffer (lh1_dec_t *lh)
{
	unsigned i;

	for (i = 0; i < 256; i++) {
		lh->inp_buf[i] = 0;
	}

	lh->inp_cnt = 256;

	lh->eof = 1;
}

static
void lh1_fill_buffer (lh1_dec_t *lh)
{
	if (lh->inp_idx >= lh->inp_cnt) {
		lh->inp_idx = 0;
		lh->inp_cnt = fread (lh->inp_buf, 1, 256, lh->fp);

		if (lh->inp_cnt == 0) {
			lh1_clear_buffer (lh);
		}
	}

	while (lh->bit_cnt <= 8) {
		if (lh->inp_idx >= lh->inp_cnt) {
			lh1_clear_buffer (lh);
		}

		lh->bit_buf |= (unsigned) lh->inp_buf[lh->inp_idx++] << (8 - lh->bit_cnt);
		lh->bit_cnt += 8;
	}
}

static
unsigned char lh1_get_byte (lh1_dec_t *lh)
{
	unsigned v;

	lh1_fill_buffer (lh);

	v = (lh->bit_buf >> 8) & 0xff;

	lh->bit_buf <<= 8;
	lh->bit_cnt -= 8;

	return (v);
}

static
int lh1_get_bit (lh1_dec_t *lh)
{
	int v;

	lh1_fill_buffer (lh);

	v = (lh->bit_buf & 0x8000) != 0;

	lh->bit_buf <<= 1;
	lh->bit_cnt -= 1;

	return (v);
}

static
unsigned lh1_get_sym (lh1_dec_t *lh)
{
	unsigned n;

	n = lh->tr.down[LH1_ROOT];

	while ((n & 0x8000) == 0) {
		n = lh->tr.down[n + lh1_get_bit (lh)];
	}

	return (~n & 0xffff);
}

static
unsigned lh1_get_dist (lh1_dec_t *lh)
{
	unsigned char i, n;
	unsigned      v;

	i = lh1_get_byte (lh);
	v = (unsigned) lh1_tab_code[i] << 6;
	n = lh1_tab_len[i];

	while (n > 2) {
		i = (i << 1) + lh1_get_bit (lh);
		n -= 1;
	}

	v |= (i & 0x3f);

	return (v);
}

unsigned lh1_read (lh1_dec_t *lh, void *buf, unsigned cnt)
{
	unsigned      idx, sym, dist;
	unsigned char len;
	unsigned char c;
	unsigned char *ptr;

	if (lh->eof) {
		return (0);
	}

	ptr = buf;
	idx = 0;
	sym = 0;

	while (idx < cnt) {
		while ((lh->buf_cnt > 0) && (idx < cnt)) {
			c = lh->buf[lh->buf_tail];
			lh->buf[lh->buf_head] = c;

			lh->buf_head = (lh->buf_head + 1) & LH1_MASK;
			lh->buf_tail = (lh->buf_tail + 1) & LH1_MASK;
			lh->buf_cnt -= 1;

			ptr[idx] = c;
			idx += 1;
		}

		while (idx < cnt) {
			sym = lh1_get_sym (lh);

			if (lh->eof) {
				return (idx);
			}

			lh1_tree_update (&lh->tr, sym);

			if (sym > 255) {
				break;
			}

			lh->buf[lh->buf_head] = sym;
			lh->buf_head = (lh->buf_head + 1) & LH1_MASK;

			ptr[idx] = sym;
			idx += 1;
		}

		if (idx < cnt) {
			dist = lh1_get_dist (lh);

			if (lh->eof) {
				return (idx);
			}

			len = (sym - 255) + 2;

			lh->buf_tail = (lh->buf_head - dist - 1) & LH1_MASK;
			lh->buf_cnt = len;
		}
	}

	return (idx);
}

static
unsigned td_crc (unsigned crc, const void *buf, unsigned cnt)
{
	unsigned            i;
	const unsigned char *tmp;

	tmp = buf;

	while (cnt > 0) {
		crc ^= (*tmp & 0xff) << 8;

		for (i = 0; i < 8; i++) {
			if (crc & 0x8000) {
				crc = (crc << 1) ^ TD_CRC_POLY;
			}
			else {
				crc = crc << 1;
			}
		}

		tmp += 1;
		cnt -= 1;
	}

	return (crc & 0xffff);
}

static
int td_copy (FILE *inp, FILE *out)
{
	unsigned      n;
	unsigned char buf[256];

	while ((n = fread (buf, 1, 256, inp)) > 0) {
		if (fwrite (buf, 1, n, out) != n) {
			return (1);
		}
	}

	return (0);
}

static
int td_decode (FILE *inp, FILE *out)
{
	unsigned      n;
	unsigned      crc;
	unsigned char buf[256];
	lh1_dec_t     lh1;

	if (fread (buf, 1, 12, inp) != 12) {
		return (1);
	}

	if ((buf[0] == 'T') && (buf[1] == 'D')) {
		if (fwrite (buf, 1, 12, out) != 12) {
			return (1);
		}

		return (td_copy (inp, out));
	}

	if ((buf[0] != 't') || (buf[1] != 'd')) {
		return (1);
	}

	buf[0] = 'T';
	buf[1] = 'D';

	crc = td_crc (0, buf, 10);

	buf[10] = crc & 0xff;
	buf[11] = (crc >> 8) & 0xff;

	if (fwrite (buf, 1, 12, out) != 12) {
		return (1);
	}

	lh1_read_init (&lh1, inp);

	while ((n = lh1_read (&lh1, buf, 256)) > 0) {
		if (fwrite (buf, 1, n, out) != n) {
			return (1);
		}
	}

	return (0);
}

static
int td_process (const char *ninp, const char *nout)
{
	int  r;
	FILE *inp, *out;

	if (strcmp (ninp, "-") == 0) {
		inp = stdin;
	}
	else if ((inp = fopen (ninp, "rb")) == NULL) {
		return (1);
	}

	if (strcmp (nout, "-") == 0) {
		out = stdout;
	}
	else if ((out = fopen (nout, "wb")) == NULL) {
		return (1);
	}

	r = td_decode (inp, out);

	if (out != stdout) {
		fclose (out);
	}

	if (inp != stdin) {
		fclose (inp);
	}

	return (r);
}

int main (int argc, char **argv)
{
	int        i;
	const char *inp, *out;

	if (argc == 2) {
		if (strcmp (argv[1], "--help") == 0) {
			print_help();
			return (0);
		}
		else if (strcmp (argv[1], "--version") == 0) {
			print_version();
			return (0);
		}
	}

	i = 1;
	inp = NULL;
	out = NULL;

	while (i < argc) {
		if (inp == NULL) {
			inp = argv[i];
		}
		else if (out == NULL) {
			out = argv[i];
		}
		else {
			fprintf (stderr, "%s: too many arguments (%s)\n",
				argv[0], argv[i]
			);
			return (1);
		}

		i += 1;
	}

	if ((inp == NULL) || (out == NULL)) {
		fprintf (stderr, "%s: missing file name\n\n", argv[0]);
		print_help();
		return (1);
	}

	if (td_process (inp, out)) {
		return (1);
	}

	return (0);
}
