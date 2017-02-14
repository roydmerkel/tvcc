/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/video/vga.h                                      *
 * Created:     2004-03-25 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2011 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_VIDEO_VGA_H
#define PCE_VIDEO_VGA_H 1


#include <libini/libini.h>
#include <drivers/video/terminal.h>
#include <devices/video/video.h>
/* ---------------------------------------------------------------------------------------------------------------- */
/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; Video Subsystem:                            */
/* -----------------------------------------------------------------------------------------------------------------*/
/* The video subsystem is resident on the system board and consists of:                                             */
/*  - Video memory controller gate array                                                                            */
/*  - Video formatt gate array                                                                                      */
/*  - 64K bytes of multiport dynamic memory                                                                         */
/*  - 8K bytes static RAM character generator                                                                       */
/*  - 256-by-18-bit color palette with three 6-bit digital-to-analog con-verters (DAC).                             */
/*At the BIOS level (interrupt level 10), the Type 8525 maintains compat-ibility with the IBM Color Graphics Adapter*/
/* (CGA).                                                                                                           */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Block Diagram:                                                                                                   */
/* ---------------------------------------------------------------------------------------------------------------- */
/*  - D0-D7                                                                                                         */
/*  - I/O R/W                                                                                                       */
/*  - 3DX Decode                                                                                                    */
/*  - A0-A15                                                                                                        */
/*  - 25 MHz Oscillator                                                                                             */
/*  - Video Memory Control Gate Array                                                                               */
/*  - Video Buffer (64K)                                                                                            */
/*  - 8K RAM Lodable Character Generator                                                                            */
/*  - Address MUX                                                                                                   */
/*  - Video Formatter Gate Array                                                                                    */
/*  - 256x18 Color Palette with DACs (RGB)                                                                          */
/*  - Monitor Sense 0 and                                                                                           */
/*  - Syncs                                                                                                         */
/*  - Analog Monitor (RGB)                                                                                          */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Figure 1-21. Video Subsystem Block Diagram:                                                                      */
/* ---------------------------------------------------------------------------------------------------------------- */

typedef struct vga_s {
	video_t       video;
	terminal_t    *term;
	mem_blk_t     *memblk;
	unsigned char *mem;
	mem_blk_t     *regblk;
	unsigned char *reg;
	unsigned char reg_seq[5];
	unsigned char reg_grc[9];
	unsigned char reg_atc[21];
	unsigned char reg_crt[25];
	unsigned char reg_dac[768];
	unsigned      latch_addr;
	unsigned      latch_hpp;
	char          atc_flipflop;
	unsigned      dac_addr_read;
	unsigned      dac_addr_write;
	unsigned char dac_state;
	unsigned char latch[4];
	char          blink_on;
	unsigned      blink_cnt;
	unsigned      blink_freq;
	/* these are derived from the crtc registers */
	unsigned long clk_ht;
	unsigned long clk_hd;
	unsigned long clk_vt;
	unsigned long clk_vd;
	unsigned      buf_w;
	unsigned      buf_h;
	unsigned long bufmax;
	unsigned char *buf;
	unsigned char update_state;
	void          *set_irq_ext;
	void          (*set_irq) (void *ext, unsigned char val);
	unsigned char set_irq_val;
} vga_t;

void vga_set_irq_fct (vga_t *ega, void *ext, void *fct);
void vga_init (vga_t *vga, unsigned long io, unsigned long addr);
void vga_free (vga_t *vga);
vga_t *vga_new (unsigned long io, unsigned long addr);
void vga_del (vga_t *vga);
video_t *vga_new_ini (ini_sct_t *sct);

#endif
