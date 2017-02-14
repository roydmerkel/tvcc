/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/psi/psi-img-td0.c                                *
 * Created:     2010-09-04 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2010-2013 Hampa Hug <hampa@hampa.ch>                     *
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
#include <time.h>

#include "psi.h"
#include "psi-io.h"
#include "psi-img-td0.h"


#define TD_CRC_POLY 0xa097


static
unsigned td0_crc (unsigned crc, const void *buf, unsigned cnt)
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
void td0_set_encoding (psi_sct_t *sct, unsigned dr)
{
	unsigned enc;

	enc = (dr & 0x80) ? PSI_ENC_FM : PSI_ENC_MFM;

	if ((dr & 0x7f) == 2) {
		enc |= 0x8000;
	}

	psi_sct_set_encoding (sct, enc);
}

static
int td0_load_data_0 (FILE *fp, psi_sct_t *sct, unsigned cnt)
{
	if (cnt != sct->n) {
		return (1);
	}

	if (psi_read (fp, sct->data, cnt)) {
		fprintf (stderr, "td0: read error\n");
		return (1);
	}

	return (0);
}

static
int td0_load_data_1 (FILE *fp, psi_sct_t *sct, unsigned cnt)
{
	unsigned      i, n;
	unsigned char buf[4];

	if (cnt != 4) {
		return (1);
	}

	if (psi_read (fp, buf, 4)) {
		return (1);
	}

	n = psi_get_uint16_le (buf, 0);

	if ((2 * n) != sct->n) {
		return (1);
	}

	for (i = 0; i < n; i++) {
		sct->data[2 * i + 0] = buf[2];
		sct->data[2 * i + 1] = buf[3];
	}

	return (0);
}

static
int td0_load_data_2 (FILE *fp, psi_sct_t *sct, unsigned cnt)
{
	unsigned      idx, rep;
	unsigned char buf[4];

	idx = 0;

	while (cnt > 0) {
		if (cnt < 2) {
			return (1);
		}

		if (psi_read (fp, buf, 2)) {
			return (1);
		}

		cnt -= 2;

		if (buf[0] == 0) {
			rep = buf[1];

			if (rep > cnt) {
				return (1);
			}

			if ((idx + rep) > sct->n) {
				return (1);
			}

			if (psi_read (fp, sct->data + idx, rep)) {
				return (1);
			}

			idx += rep;
			cnt -= rep;
		}
		else if (buf[0] == 1) {
			rep = buf[1];

			if (cnt < 2) {
				return (1);
			}

			if ((idx + (2 * rep)) > sct->n) {
				return (1);
			}

			if (psi_read (fp, buf + 2, 2)) {
				return (1);
			}

			cnt -= 2;

			while (rep > 0) {
				sct->data[idx + 0] = buf[2];
				sct->data[idx + 1] = buf[3];

				idx += 2;
				rep -= 1;
			}
		}
		else {
			return (1);
		}
	}

	if (idx < sct->n) {
		return (1);
	}

	return (0);
}

static
int td0_load_sector (FILE *fp, psi_img_t *img, unsigned c, unsigned h, unsigned dr)
{
	unsigned      crc1, crc2, crc3;
	unsigned      s, n;
	unsigned      cnt;
	unsigned      flg;
	unsigned char buf[8];
	psi_sct_t     *sct;

	if (psi_read (fp, buf, 6)) {
		return (1);
	}

	s = buf[2];
	n = buf[3];

	flg = buf[4];

	sct = psi_sct_new (buf[0], buf[1], buf[2], (n <= 6) ? (128U << n) : 0);

	if (sct == NULL) {
		return (1);
	}

	psi_sct_set_mfm_size (sct, buf[3]);

	if (psi_img_add_sector (img, sct, c, h)) {
		psi_sct_del (sct);
		return (1);
	}

	td0_set_encoding (sct, dr);

	if (flg & 0x02) {
		psi_sct_set_flags (sct, PSI_FLAG_CRC_DATA, 1);
	}

	if (flg & 0x04) {
		psi_sct_set_flags (sct, PSI_FLAG_DEL_DAM, 1);
	}

	crc1 = buf[5];
	crc2 = td0_crc (0, buf, 5);

	if (flg & 0x30) {
		if (crc1 != (crc2 & 0xff)) {
			fprintf (stderr,
				"td0: crc error at sector %u/%u/%u (no data)\n",
				c, h, sct->s
			);
			return (1);
		}

		psi_sct_fill (sct, (flg & 0x10) ? 0xf6 : 0x00);

		return (0);
	}

	if (n <= 6) {
		if (psi_read (fp, buf, 3)) {
			return (1);
		}

		cnt = psi_get_uint16_le (buf, 0);

		if (cnt == 0) {
			fprintf (stderr, "td0: zero data length (%u/%u/%u)\n",
				c, h, s
			);
			return (1);
		}

		cnt -= 1;

		if (buf[2] == 0) {
			if (td0_load_data_0 (fp, sct, cnt)) {
				return (1);
			}
		}
		else if (buf[2] == 1) {
			if (td0_load_data_1 (fp, sct, cnt)) {
				return (1);
			}
		}
		else if (buf[2] == 2) {
			if (td0_load_data_2 (fp, sct, cnt)) {
				return (1);
			}
		}
		else {
			fprintf (stderr, "td0: unknown compression (%u/%u/%u %u)\n",
				c, h, s, buf[2]
			);
			return (1);
		}
	}

	crc3 = td0_crc (crc2, sct->data, sct->n);
	crc2 = td0_crc (0, sct->data, sct->n);

	if (crc1 != (crc2 & 0xff)) {
		fprintf (stderr, "td0: sector crc over header+data\n");

		if (crc1 != (crc3 & 0xff)) {
			fprintf (stderr,
				"td0: crc error at sector %u/%u/%u (%02X %04X %04X)\n",
				c, h, sct->s, crc1, crc2, crc3
			);
			return (1);
		}
	}

	if (flg & 0x40) {
		fprintf (stderr, "td0: dropping phantom sector %u/%u/%u\n",
			c, h, sct->s
		);

		psi_img_remove_sector (img, sct);
		psi_sct_del (sct);
	}

	return (0);
}

static
int td0_load_tracks (FILE *fp, psi_img_t *img, unsigned dr)
{
	unsigned      i, n;
	unsigned      c, h;
	unsigned      dr2;
	unsigned      crc;
	unsigned char buf[16];

	while (1) {
		if (psi_read (fp, buf, 1)) {
			return (1);
		}

		if (buf[0] == 0xff) {
			return (0);
		}

		if (psi_read (fp, buf + 1, 3)) {
			return (1);
		}

		crc = td0_crc (0, buf, 3);

		if (buf[3] != (crc & 0xff)) {
			fprintf (stderr, "td0: track crc (%02X %04X)\n",
				buf[3], crc
			);
			return (1);
		}

		n = buf[0];
		c = buf[1];
		h = buf[2] & 0x7f;

		dr2 = dr;

		if (buf[2] & 0x80) {
			dr2 |= 0x80;
		}

		for (i = 0; i < n; i++) {
			if (td0_load_sector (fp, img, c, h, dr2)) {
				return (1);
			}
		}
	}

	return (1);
}

static
int td0_load_comment (FILE *fp, psi_img_t *img)
{
	unsigned      i, j, k;
	unsigned      cnt;
	unsigned      crc1, crc2;
	unsigned char buf[16];
	unsigned char *cmt;

	if (psi_read (fp, buf, 10)) {
		return (1);
	}

	crc1 = psi_get_uint16_le (buf, 0);
	crc2 = td0_crc (0, buf + 2, 8);

	cnt = psi_get_uint16_le (buf, 2);

	if (cnt > 0) {
		cmt = malloc (cnt);

		if (cmt == NULL) {
			return (1);
		}

		if (psi_read (fp, cmt, cnt)) {
			free (cmt);
			return (1);
		}

		crc2 = td0_crc (crc2, cmt, cnt);

		i = 0;
		j = cnt;

		while ((i < j) && (cmt[i] == 0)) {
			i += 1;
		}

		while ((i < j) && (cmt[j - 1] == 0)) {
			j -= 1;
		}

		for (k = i; k < j; k++) {
			if (cmt[k] == 0) {
				cmt[k] = 0x0a;
			}
		}

		psi_img_set_comment (img, cmt + i, j - i);

		free (cmt);

		psi_img_clean_comment (img);
	}

	if (crc1 != crc2) {
		fprintf (stderr, "td0: comment crc (%04X %04X)\n", crc1, crc2);
		return (1);
	}

	return (0);
}

static
int td0_load_fp (FILE *fp, psi_img_t *img)
{
	unsigned      vers;
	int           comment;
	unsigned      datarate;
	unsigned      crc1, crc2;
	unsigned char buf[16];

	if (psi_read (fp, buf, 12)) {
		return (1);
	}

	if ((buf[0] == 't') && (buf[1] == 'd')) {
		fprintf (stderr, "td0: advanced compression not supported\n");
		return (1);
	}

	if ((buf[0] != 'T') || (buf[1] != 'D')) {
		return (1);
	}

	if (buf[2] != 0) {
		return (1);
	}

	vers = buf[4];

	if (vers > 21) {
		return (1);
	}

	datarate = buf[5];

	comment = (buf[7] & 0x80) != 0;

	crc1 = psi_get_uint16_le (buf, 10);
	crc2 = td0_crc (0, buf, 10);

	if (crc1 != crc2) {
		fprintf (stderr, "td0: header crc (%04X %04X)\n", crc1, crc2);
		return (1);
	}

	if (comment) {
		if (td0_load_comment (fp, img)) {
			return (1);
		}
	}

	if (td0_load_tracks (fp, img, datarate)) {
		return (1);
	}

	return (0);
}

psi_img_t *psi_load_td0 (FILE *fp)
{
	psi_img_t *img;

	img = psi_img_new();

	if (img == NULL) {
		return (NULL);
	}

	if (td0_load_fp (fp, img)) {
		psi_img_del (img);
		return (NULL);
	}

	return (img);
}


static
int td0_save_header (FILE *fp, const psi_img_t *img)
{
	unsigned        c, h, s;
	unsigned        cn, hn, sn;
	unsigned        enc;
	unsigned        dt;
	unsigned        crc;
	const psi_cyl_t *cyl;
	const psi_trk_t *trk;
	const psi_sct_t *sct;
	unsigned char   buf[16];

	cn = img->cyl_cnt;
	hn = 0;
	sn = 0;
	enc = 0;

	for (c = 0; c < img->cyl_cnt; c++) {
		cyl = img->cyl[c];

		if (cyl->trk_cnt > hn) {
			hn = cyl->trk_cnt;
		}

		for (h = 0; h < cyl->trk_cnt; h++) {
			trk = cyl->trk[h];

			if (trk->sct_cnt > sn) {
				sn = trk->sct_cnt;
			}

			for (s = 0; s < trk->sct_cnt; s++) {
				sct = trk->sct[s];

				if (enc == 0) {
					enc = sct->encoding;
				}
			}
		}
	}

	switch (enc) {
	case PSI_ENC_FM_DD:
		enc = 0x80;
		break;

	case PSI_ENC_FM_HD:
		enc = 0x82;
		break;

	case PSI_ENC_MFM_DD:
		enc = 0x00;
		break;

	case PSI_ENC_MFM_HD:
		enc = 0x02;
		break;

	default:
		enc = 0;
		break;
	}

	if (cn < 45) {
		dt = 1;
	}
	else if (sn < 12) {
		dt = 3;
	}
	else if (sn < 17) {
		dt = 2;
	}
	else {
		dt = 4;
	}

	buf[0] = 'T';
	buf[1] = 'D';
	buf[2] = 0;
	buf[3] = 0;
	buf[4] = 21;
	buf[5] = enc;
	buf[6] = dt;
	buf[7] = 0;
	buf[8] = 0;
	buf[9] = hn;

	if (img->comment_size > 0) {
		buf[7] |= 0x80;
	}

	crc = td0_crc (0, buf, 10);

	psi_set_uint16_le (buf, 10, crc);

	if (psi_write (fp, buf, 12)) {
		return (1);
	}

	return (0);
}

static
int td0_save_comment (FILE *fp, const psi_img_t *img)
{
	unsigned      i, n;
	unsigned      crc;
	time_t        t;
	struct tm     *tm;
	unsigned char buf[16];
	unsigned char *cmt;

	n = img->comment_size;

	if (n > 65535) {
		n = 65535;
	}

	psi_set_uint16_le (buf, 2, n);

	t = time (NULL);
	tm = gmtime (&t);

	buf[4] = tm->tm_year;
	buf[5] = tm->tm_mon;
	buf[6] = tm->tm_mday;
	buf[7] = tm->tm_hour;
	buf[8] = tm->tm_min;
	buf[9] = tm->tm_sec;

	cmt = malloc (n);

	if (cmt == NULL) {
		return (1);
	}

	for (i = 0; i < n; i++) {
		if (img->comment[i] == 0x0a) {
			cmt[i] = 0;
		}
		else {
			cmt[i] = img->comment[i];
		}
	}

	crc = td0_crc (0, buf + 2, 8);
	crc = td0_crc (crc, cmt, n);

	psi_set_uint16_le (buf, 0, crc);

	if (psi_write (fp, buf, 10)) {
		free (cmt);
		return (1);
	}

	if (psi_write (fp, cmt, n)) {
		free (cmt);
		return (1);
	}

	free (cmt);

	return (0);
}

static
int td0_save_sector (FILE *fp, const psi_sct_t *sct, unsigned c, unsigned h)
{
	unsigned      i;
	int           cmpr;
	unsigned char buf[16];

	if (sct->n & (sct->n - 1)) {
		return (1);
	}

	buf[0] = sct->c;
	buf[1] = sct->h;
	buf[2] = sct->s;
	buf[3] = psi_sct_get_mfm_size (sct);
	buf[4] = 0;

	if (sct->flags & PSI_FLAG_CRC_DATA) {
		buf[4] |= 0x02;
	}

	if (sct->flags & PSI_FLAG_DEL_DAM) {
		buf[4] |= 0x04;
	}

	buf[5] = td0_crc (0, sct->data, sct->n) & 0xff;

	if (psi_write (fp, buf, 6)) {
		return (1);
	}

	if (buf[3] > 6) {
		return (0);
	}

	cmpr = 1;

	if ((sct->n & 1) || (sct->n < 2)) {
		cmpr = 0;
	}
	else {
		for (i = 2; i < sct->n; i++) {
			if (sct->data[i] != sct->data[i & 1]) {
				cmpr = 0;
				break;
			}
		}
	}

	if (cmpr) {
		psi_set_uint16_le (buf, 0, 5);
		buf[2] = 1;
		psi_set_uint16_le (buf, 3, sct->n / 2);
		buf[5] = sct->data[0];
		buf[6] = sct->data[1];

		if (psi_write (fp, buf, 7)) {
			return (1);
		}
	}
	else {
		psi_set_uint16_le (buf, 0, sct->n + 1);
		buf[2] = 0;

		if (psi_write (fp, buf, 3)) {
			return (1);
		}

		if (psi_write (fp, sct->data, sct->n)) {
			return (1);
		}
	}

	return (0);
}

static
int td0_save_track (FILE *fp, const psi_trk_t *trk, unsigned c, unsigned h)
{
	unsigned      s;
	unsigned char buf[4];

	buf[0] = trk->sct_cnt;
	buf[1] = c;
	buf[2] = h;
	buf[3] = td0_crc (0, buf, 3) & 0xff;

	if (psi_write (fp, buf, 4)) {
		return (1);
	}

	for (s = 0; s < trk->sct_cnt; s++) {
		if (td0_save_sector (fp, trk->sct[s], c, h)) {
			return (1);
		}
	}

	return (0);
}

int psi_save_td0 (FILE *fp, const psi_img_t *img)
{
	unsigned        c, h;
	const psi_cyl_t *cyl;
	const psi_trk_t *trk;
	unsigned char   buf[4];

	if (td0_save_header (fp, img)) {
		return (1);
	}

	if (img->comment_size > 0) {
		if (td0_save_comment (fp, img)) {
			return (1);
		}
	}

	for (c = 0; c < img->cyl_cnt; c++) {
		cyl = img->cyl[c];

		for (h = 0; h < cyl->trk_cnt; h++) {
			trk = cyl->trk[h];

			if (td0_save_track (fp, trk, c, h)) {
				return (1);
			}
		}
	}

	buf[0] = 0xff;
	buf[1] = 0;
	buf[2] = 0;
	buf[3] = td0_crc (0, buf, 3) & 0xff;

	if (psi_write (fp, buf, 4)) {
		return (1);
	}

	fflush (fp);

	return (0);
}

int psi_probe_td0_fp (FILE *fp)
{
	unsigned char buf[16];

	if (fseek (fp, 0, SEEK_SET)) {
		return (0);
	}

	if (psi_read (fp, buf, 16)) {
		return (0);
	}

	if ((buf[0] == 't') && (buf[1] == 'd')) {
		;
	}
	else if ((buf[0] == 'T') && (buf[1] == 'D')) {
		;
	}
	else {
		return (0);
	}

	if (buf[2] != 0) {
		return (0);
	}

	return (1);
}

int psi_probe_td0 (const char *fname)
{
	int  r;
	FILE *fp;

	fp = fopen (fname, "rb");

	if (fp == NULL) {
		return (0);
	}

	r = psi_probe_td0_fp (fp);

	fclose (fp);

	return (r);
}
