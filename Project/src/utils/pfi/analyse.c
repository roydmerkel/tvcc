/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pfi/analyse.c                                      *
 * Created:     2013-12-21 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2013-2014 Hampa Hug <hampa@hampa.ch>                     *
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

#include <drivers/pri/pri.h>
#include <drivers/pri/pri-img.h>


static
void pfi_smooth (unsigned long *val, unsigned cnt)
{
	unsigned      i;
	unsigned long tmp[3];

	tmp[0] = 0;
	tmp[1] = val[0];
	tmp[2] = val[1];

	for (i = 0; i < cnt; i++) {
		val[i] = (tmp[0] + 2 * tmp[1] + tmp[2]) / 4;

		tmp[0] = tmp[1];
		tmp[1] = tmp[2];

		if ((i + 2) < cnt) {
			tmp[2] = val[i + 2];
		}
		else {
			tmp[2] = 0;
		}
	}
}

#if 0
static
void pfi_print_pdf (const unsigned long *val, unsigned cnt)
{
	unsigned      i;
	unsigned long total;

	total = 0;

	for (i = 0; i < cnt; i++) {
		total += val[i];
	}

	for (i = 0; i < cnt; i++) {
		fprintf (stdout, "%u: %lu (%.2f)\n", i, val[i], (100.0 * val[i]) / total);
	}
}
#endif

static
void pfi_get_pdf (pfi_trk_t *trk, unsigned long *val, unsigned cnt, unsigned rev)
{
	unsigned      i, index;
	unsigned long pulse, idx;

	pfi_trk_rewind (trk);

	for (i = 0; i < cnt; i++) {
		val[i] = 0;
	}

	index = 0;

	while (pfi_trk_get_pulse (trk, &pulse, &idx) == 0) {
		if ((pulse == 0) || (idx < pulse)) {
			index += 1;

			if (index > rev) {
				break;
			}
		}

		if (pulse == 0) {
			continue;
		}

		if (index == rev) {
			if (pulse < cnt) {
				val[pulse] += 1;
			}
		}
	}
}

static
void pfi_get_peaks (pfi_trk_t *trk, double *peak, unsigned *cnt, unsigned max)
{
	unsigned      i, rev;
	double        v;
	unsigned long n;
	unsigned long pdf[256];

	rev = (par_revolution < 1) ? 1 : par_revolution;

	pfi_get_pdf (trk, pdf, 256, rev);

	*cnt = 0;

	pfi_smooth (pdf, 256);

	i = 0;
	while (i < 256) {
		if (pdf[i] < 16) {
			i += 1;
			continue;
		}

		v = (double) i * pdf[i];
		n = pdf[i];
		i += 1;

		while ((i < 256) && (pdf[i] >= 16)) {
			v += (double) i * pdf[i];
			n += pdf[i];
			i += 1;
		}

		if ((n > 64) && (*cnt < max)) {
			peak[*cnt] = v / n;
			*cnt += 1;
		}
	}

	//pfi_print_dist (pcnt, 256);
}

static
void pfi_analyse (pfi_trk_t *trk, unsigned *type, double *freq)
{
	unsigned pcnt;
	double   pval[16];

	*type = 0;
	*freq = 0.0;

	pfi_get_peaks (trk, pval, &pcnt, 16);

	if (pcnt == 0) {
		return;
	}

	*freq = trk->clock / pval[0];

	if (pcnt < 2) {
		return;
	}

	if ((pval[1] / pval[0]) < 1.75) {
		*type = 2;
		*freq *= 2.0;
	}
	else {
		*type = 1;
	}
}

static
int pfi_analyse_track_cb (pfi_img_t *img, pfi_trk_t *trk, unsigned long c, unsigned long h, void *opaque)
{
	unsigned      type, rate;
	double        freq;
	const char    *name;

	pfi_analyse (trk, &type, &freq);

	if (type == 2) {
		name = "MFM";
	}
	else if (type == 1) {
		name = "FM";
	}
	else {
		name = "UNK";
	}

	rate = (unsigned) (freq / 1000.0 + 0.5);

	fprintf (stdout, "Track %2lu/%lu: %3s %u KHz\n", c, h, name, rate);

	return (0);
}

int pfi_analyse_tracks (pfi_img_t *img)
{
	return (pfi_for_all_tracks (img, pfi_analyse_track_cb, NULL));
}
