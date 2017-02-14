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


#ifndef PCE_VIDEO_COMPAQ_H
#define PCE_VIDEO_COMPAQ_H 1


#include <libini/libini.h>
#include <drivers/video/terminal.h>
#include <devices/video/video.h>


typedef struct compaq_s {
	video_t       video;

	void          *ext;

	/* update the internal screen buffer */
	void          (*update) (struct compaq_s *ext);

	mem_blk_t     *memblk;
	/* unsigned char mem[4096]; */ /* Disabled MDA address */
	unsigned char *mem; /* Re-enable MDA behavior above if incompatible with COMPAQ display */

	mem_blk_t     *regblk;
	unsigned char *reg;

	terminal_t    *term;

	unsigned char reg_crt[18];

	unsigned char pal[4];

	unsigned char *font; /* Re-enable old CGA behavior below if incompatible with COMPAQ display */
/*	unsigned char font[2048]; */

	unsigned char rgb[16][3]; /* Disable MDA behavior if incompatible with COMPAQ display */
	/* composite mode color lookup table */
	char          comp_tab_ok;
	unsigned char *comp_tab;

	char          blink_on;
	unsigned      blink_cnt;
	unsigned      blink_freq;

	unsigned      external_display; /* External Display Enable/Disable */
	unsigned      external_cga;     /* Use to determine whether external CGA display is connected */
	unsigned      hires_mono_font;  /* Use to determine whether high-resolution font is loaded */

	/* these are derived from the crtc registers */
	unsigned      w;
	unsigned      h;
	unsigned      ch;

	unsigned long clk_ht; /* R 0: Horizontal Total         : 61 */
	unsigned long clk_vt; /* R 4: Vertical Total           : 19 */
	unsigned long clk_hd; /* R 1: Horizontal Displayed     : 50 */
	unsigned long clk_vd; /* R 6: Vertical Displayed       : 19 */

	unsigned      buf_w;
	unsigned      buf_h;
	unsigned long bufmax;
	unsigned char *buf;

	unsigned char update_state;
} compaq_t;


extern unsigned char compaq_rgb[16][3]; /* Disable and replace with above if required by COMPAQ display */


void compaq_set_blink_rate (compaq_t *compaq, unsigned freq);
unsigned compaq_get_start (compaq_t *compaq);
unsigned compaq_get_cursor (compaq_t *compaq);
int compaq_get_position (compaq_t *compaq, unsigned *x, unsigned *y);
int compaq_set_buf_size (compaq_t *compaq, unsigned w, unsigned h);
void compaq_update (compaq_t *compaq);
unsigned char compaq_reg_get_uint8 (compaq_t *compaq, unsigned long addr);
unsigned short compaq_reg_get_uint16 (compaq_t *compaq, unsigned long addr);
void compaq_reg_set_uint8 (compaq_t *compaq, unsigned long addr, unsigned char val);
void compaq_reg_set_uint16 (compaq_t *compaq, unsigned long addr, unsigned short val);


void compaq_init (compaq_t *compaq, unsigned long io, unsigned long addr, unsigned long size);

void compaq_free (compaq_t *compaq);

/* compaq_t *compaq_new (unsigned long io, unsigned long addr, unsigned long mem, unsigned long size); */ /* Disabled MDA address */
compaq_t *compaq_new (unsigned long io, unsigned long addr, unsigned long size);

void compaq_del (compaq_t *compaq);

video_t *compaq_new_ini (ini_sct_t *sct);


#endif
