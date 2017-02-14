/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/video/cga.h                                      *
 * Created:     2003-04-18 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2013 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_VIDEO_CGA5140_H
#define PCE_VIDEO_CGA5140_H 1


#include <libini/libini.h>
#include <drivers/video/terminal.h>
#include <devices/video/video.h>


typedef struct cga5140_s {
	video_t       video;

	void          *ext;

	/* update the internal screen buffer */
	void          (*update) (struct cga5140_s *ext);

	mem_blk_t     *memblk;
	unsigned char *mem;

	mem_blk_t     *regblk;
	unsigned char *reg;

	terminal_t    *term;

	unsigned char reg_crt[18];

	unsigned char pal[4];

	unsigned char font[2048];

	/* composite mode color lookup table */
	char          comp_tab_ok;
	unsigned char *comp_tab;

	char          blink_on;
	unsigned      blink_cnt;
	unsigned      blink_freq;

	/* these are derived from the crtc registers */
	unsigned      w;
	unsigned      h;
	unsigned      ch;

	unsigned long clk_ht;
	unsigned long clk_vt;
	unsigned long clk_hd;
	unsigned long clk_vd;

	unsigned      buf_w;
	unsigned      buf_h;
	unsigned long bufmax;
	unsigned char *buf;

	unsigned char update_state;
} cga5140_t;


extern unsigned char cga5140_rgb[16][3];


void cga5140_set_blink_rate (cga5140_t *cga5140, unsigned freq);
unsigned cga5140_get_start (cga5140_t *cga5140);
unsigned cga5140_get_cursor (cga5140_t *cga5140);
int cga5140_get_position (cga5140_t *cga5140, unsigned *x, unsigned *y);
int cga5140_set_buf_size (cga5140_t *cga5140, unsigned w, unsigned h);
void cga5140_update (cga5140_t *cga5140);
unsigned char cga5140_reg_get_uint8 (cga5140_t *cga5140, unsigned long addr);
unsigned short cga5140_reg_get_uint16 (cga5140_t *cga5140, unsigned long addr);
void cga5140_reg_set_uint8 (cga5140_t *cga5140, unsigned long addr, unsigned char val);
void cga5140_reg_set_uint16 (cga5140_t *cga5140, unsigned long addr, unsigned short val);


void cga5140_init (cga5140_t *cga5140, unsigned long io, unsigned long addr, unsigned long size);

void cga5140_free (cga5140_t *cga5140);

cga5140_t *cga5140_new (unsigned long io, unsigned long addr, unsigned long size);

void cga5140_del (cga5140_t *cga5140);

video_t *cga5140_new_ini (ini_sct_t *sct);


#endif
