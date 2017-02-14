/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pfi/pfi-scp.c                                    *
 * Created:     2014-01-30 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2014-2015 Hampa Hug <hampa@hampa.ch>                     *
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
#include "pfi-io.h"
#include "pfi-scp.h"


#define SCP_CLOCK 40000000


static
int scp_calc_checksum (FILE *fp, unsigned long *cksum)
{
	unsigned      i, n;
	unsigned long val;
	unsigned char buf[256];

	if (pfi_set_pos (fp, 0x10)) {
		return (1);
	}

	val = 0;

	while ((n = fread (buf, 1, 256, fp)) > 0) {
		for (i = 0; i < n; i++) {
			val += buf[i];
		}
	}

	*cksum = val & 0xffffffff;

	return (0);
}

static
int scp_load_track (FILE *fp, pfi_img_t *img, unsigned c, unsigned h, unsigned revs)
{
	unsigned      i, t;
	unsigned      val;
	unsigned long j;
	unsigned long ofs;
	unsigned long dcnt, dofs, idx, oflow;
	unsigned char buf[16];
	pfi_trk_t     *trk;

	t = (c << 1) | (h & 1);

	if (pfi_read_ofs (fp, 0x10 + 4 * t, buf, 4)) {
		return (1);
	}

	ofs = pfi_get_uint32_le (buf, 0);

	if (ofs == 0) {
		return (0);
	}

	if (pfi_read_ofs (fp, ofs, buf, 4)) {
		return (1);
	}

	if ((buf[0] != 'T') || (buf[1] != 'R') || (buf[2] != 'K')) {
		return (1);
	}

	if (buf[3] != t) {
		fprintf (stderr, "scp: bad track in track header (%u/%u %u)\n",
			c, h, buf[3]
		);
		//return (1);
	}

	if ((trk = pfi_img_get_track (img, c, h, 1)) == NULL) {
		return (1);
	}

	pfi_trk_set_clock (trk, SCP_CLOCK);

	idx = 0;

	if (pfi_trk_add_index (trk, idx)) {
		return (1);
	}

	oflow = 0;

	for (i = 0; i < revs; i++) {
		if (pfi_read_ofs (fp, ofs + 4 + 12 * i, buf, 12)) {
			return (1);
		}

		idx += pfi_get_uint32_le (buf, 0);
		dcnt = pfi_get_uint32_le (buf, 4);
		dofs = pfi_get_uint32_le (buf, 8);

		if (pfi_trk_add_index (trk, idx)) {
			return (1);
		}

		if (pfi_set_pos (fp, ofs + dofs)) {
			return (1);
		}

		for (j = 0; j < dcnt; j++) {
			if (pfi_read (fp, buf, 2)) {
				return (1);
			}

			val = pfi_get_uint16_be (buf, 0);

			if (val == 0) {
				oflow += 65536;
				continue;
			}

			if (pfi_trk_add_pulse (trk, oflow + val)) {
				return (1);
			}

			oflow = 0;
		}
	}

	return (0);
}

static
int scp_load_image (FILE *fp, pfi_img_t *img)
{
	unsigned      t;
	unsigned long ck1, ck2;
	unsigned char buf[16];

	if (pfi_read_ofs (fp, 0, buf, 16)) {
		return (1);
	}

	if ((buf[0] != 'S') || (buf[1] != 'C') || (buf[2] != 'P')) {
		return (1);
	}

	if ((buf[9] != 0) && (buf[9] != 16)) {
		return (1);
	}

	if (scp_calc_checksum (fp, &ck1)) {
		return (1);
	}

	ck2 = pfi_get_uint32_le (buf, 12);

	if (ck1 != ck2) {
		fprintf (stderr, "scp: bad checksum\n");
		return (1);
	}

	for (t = 0; t < 166; t++) {
		if (scp_load_track (fp, img, t >> 1, t & 1, buf[5])) {
			return (1);
		}
	}

	return (0);
}

pfi_img_t *pfi_load_scp (FILE *fp)
{
	pfi_img_t *img;

	if ((img = pfi_img_new()) == NULL) {
		return (NULL);
	}

	if (scp_load_image (fp, img)) {
		pfi_img_del (img);
		return (NULL);
	}

	return (img);
}


int pfi_save_scp (FILE *fp, const pfi_img_t *img)
{
	return (1);
}


int pfi_probe_scp_fp (FILE *fp)
{
	unsigned char buf[4];

	if (pfi_read_ofs (fp, 0, buf, 4)) {
		return (0);
	}

	if ((buf[0] != 'S') || (buf[1] != 'C') || (buf[2] != 'P')) {
		return (0);
	}

	return (1);
}

int pfi_probe_scp (const char *fname)
{
	int  r;
	FILE *fp;

	if ((fp = fopen (fname, "rb")) == NULL) {
		return (0);
	}

	r = pfi_probe_scp_fp (fp);

	fclose (fp);

	return (r);
}
