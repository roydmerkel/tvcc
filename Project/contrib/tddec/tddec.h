/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   tddec/tddec.h
 * Created:     2012-12-24 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2014 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_TDDEC_H
#define PCE_TDDEC_H 1


#define LH1_N     4096
#define LH1_L     60
#define LH1_SYM   (256 + LH1_L - 2)
#define LH1_NODES (2 * LH1_SYM - 1)
#define LH1_ROOT  (LH1_NODES - 1)

#define LH1_RES_IO 1


typedef struct {
	unsigned short up[LH1_NODES];
	unsigned short down[LH1_NODES];
	unsigned short freq[LH1_NODES];
	unsigned short leaf[LH1_SYM];
} lh1_tree_t;


typedef struct {
	FILE           *fp;
	char           eof;

	unsigned short crc;

	unsigned       bit_cnt;
	unsigned short bit_buf;

	unsigned       inp_idx;
	unsigned       inp_cnt;
	unsigned char  inp_buf[256];

	unsigned short buf_head;
	unsigned short buf_tail;
	unsigned char  buf_cnt;
	unsigned char  buf[LH1_N];

	lh1_tree_t     tr;
} lh1_dec_t;


#endif
