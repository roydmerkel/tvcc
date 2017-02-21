/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/video/mda.c                                      *
 * Created:     2003-04-13 by Hampa Hug <hampa@hampa.ch>                     *
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lib/log.h>
#include <lib/msg.h>

#include "mda5140.h"


/* ------------------------------------------------------------------------------------ */
/* Quote from IBM Monochrome Display and Printer Adapter: Specifications:               */
/* ------------------------------------------------------------------------------------ */
/* Connector Specifications:                                                            */
/* ------------------------------------------------------------------------------------ */
/* 9-Pin Monochrome Display connector:   _____________                                  */
/*                                     5:\ o o o o o /1:                                */
/*                                     9: \ o o o o / 6:                                */
/*                                         ---------                                    */
/* At Standard TTL Levels:                                                              */
/* - IBM Monochrome Display and Printer Adapter: 1: Ground     : IBM Monochrome Display */
/* - IBM Monochrome Display and Printer Adapter: 2: Ground     : IBM Monochrome Display */
/* - IBM Monochrome Display and Printer Adapter: 3: Not Used   : IBM Monochrome Display */
/* - IBM Monochrome Display and Printer Adapter: 4: Not Used   : IBM Monochrome Display */
/* - IBM Monochrome Display and Printer Adapter: 5: Not Used   : IBM Monochrome Display */
/* - IBM Monochrome Display and Printer Adapter: 6: +Intensity : IBM Monochrome Display */
/* - IBM Monochrome Display and Printer Adapter: 7: +Video     : IBM Monochrome Display */
/* - IBM Monochrome Display and Printer Adapter: 8: +Horizontal: IBM Monochrome Display */
/* - IBM Monochrome Display and Printer Adapter: 9: -Vertical  : IBM Monochrome Display */
/* Note: Signal voltages are 0.0 to 0.6 Vdc at down level and +2.4 to 3.5 Vdc at high   */
/* level.                                                                               */
/* ------------------------------------------------------------------------------------ */
/* Printer Adapter Function:                                                            */
/* ------------------------------------------------------------------------------------ */
/* NOTE: See PARPORT.H and PARPORT.C for information on the Printer Adapter Function in */
/* this emulator.                                                                       */
/* ------------------------------------------------------------------------------------ */
/* Logic Diagrams:                                                                      */
/* ------------------------------------------------------------------------------------ */
/* Monochrome Display Adapter (Sheet  1 of 10):                                         */
/* Monochrome Display Adapter (Sheet  2 of 10):                                         */
/* Monochrome Display Adapter (Sheet  3 of 10):                                         */
/* Monochrome Display Adapter (Sheet  4 of 10):                                         */
/* Monochrome Display Adapter (Sheet  5 of 10):                                         */
/* Monochrome Display Adapter (Sheet  6 of 10):                                         */
/* Monochrome Display Adapter (Sheet  7 of 10):                                         */
/* Monochrome Display Adapter (Sheet  8 of 10):                                         */
/* Monochrome Display Adapter (Sheet  9 of 10):                                         */
/* Monochrome Display Adapter (Sheet 10 of 10):                                         */
/*  - Please see included files for details (NOTE: Will be included in upcoming release)*/
/* ------------------------------------------------------------------------------------ */
#define MDA5140_IFREQ 1193182
#define MDA5140_PFREQ 16257000
#define MDA5140_HFREQ 18430
#define MDA5140_VFREQ 50

/* character width */
#define MDA5140_CW    9

/* mda registers */
#define MDA5140_CRTC_INDEX   0
#define MDA5140_CRTC_DATA    1
#define MDA5140_MODE         4
#define MDA5140_STATUS       6

#define MDA5140_CRTC_HT      0  /* R 0: Horizontal Total         : 61 */
#define MDA5140_CRTC_HD      1  /* R 1: Horizontal Displayed     : 50 */
#define MDA5140_CRTC_HS      2  /* R 2: Horizontal Sync Position : 52 */
#define MDA5140_CRTC_SYN     3  /* R 3: Horizontal Sync Width    :  F */
#define MDA5140_CRTC_VT      4  /* R 4: Vertical Total           : 19 */
#define MDA5140_CRTC_VTA     5  /* R 5: Vertical Total Adjust    :  6 */
#define MDA5140_CRTC_VD      6  /* R 6: Vertical Displayed       : 19 */
#define MDA5140_CRTC_VS      7  /* R 7: Vertical Sync Position   : 19 */
#define MDA5140_CRTC_IL      8  /* R 8: Interlace Mode           : 02 */
#define MDA5140_CRTC_MS      9  /* R 9: Maximum Scan Line Address:  D */
#define MDA5140_CRTC_CS      10 /* R10: Cursor Start             :  B */
#define MDA5140_CRTC_CE      11 /* R11: Cursor End               :  C */
#define MDA5140_CRTC_SH      12 /* R12: Start Address (H)        : 00 */
#define MDA5140_CRTC_SL      13 /* R13: Start Address (L)        : 00 */
#define MDA5140_CRTC_PH      14 /* R14: Cursor (H)               : 00 */
#define MDA5140_CRTC_PL      15 /* R15: Cursor (L)               : 00 */
#define MDA5140_CRTC_LH      16 /* R16: Reserved                 : -- */
#define MDA5140_CRTC_LL      17 /* R17: Reserved                 : -- */

/* ------------------------------------------------------------------- */
/* 6845 CRT Control Port 1 (Hex 3B8):                                  */
/* ------------------------------------------------------------------- */
/* Bit Number: Function:                                               */
/* - 0  :       - +High Resolution Mode                                */
/* - 1  :       - Not Used                                             */
/* - 2  :       - Not Used                                             */
/* - 3  :       - +Video Enable                                        */
/* - 4  :       - Not Used                                             */
/* - 5  :       - +Enable Blink                                        */
/* - 6,7:       - Not Used                                             */
/* ------------------------------------------------------------------- */
/* mode control register */
#define MDA5140_MODE_ENABLE  0x08
#define MDA5140_MODE_BLINK   0x20

/* ------------------------------------------------------------------- */
/* 6845 CRT Status  Port   (Hex 3BA):                                  */
/* ------------------------------------------------------------------- */
/* - 0  :       - +Horizontal Drive                                    */
/* - 1  :       - Reserved                                             */
/* - 2  :       - Reserved                                             */
/* - 3  :       - +Black/White Video                                   */
/* ------------------------------------------------------------------- */
/* CRTC status register */
#define MDA5140_STATUS_HSYNC 0x01		/* horizontal sync */
#define MDA5140_STATUS_VIDEO 0x08		/* video signal */

#define MDA5140_UPDATE_DIRTY   1
#define MDA5140_UPDATE_RETRACE 2


#include "mda5140_font.h"


/* ------------------------------------------------------------------- */
/* Quote from IBM Monochrome Display and Printer Adapter: Description: */
/* ------------------------------------------------------------------- */
/* The IBM Monochrome Display and Printer Adapter is designed around   */
/* the Motorola 6845 CRT Controller module. There are 4K bytes of RAM  */
/* on the adapter that are used for the display buffer. This buffer has*/
/* two ports to which the system unit's microprocessor has direct      */
/* access. No parity is provided on the display buffer.                */
/*                                                                     */
/* Two bytes are fetched from the display buffer in 553 ns, providing a*/
/* data rate of 1.8M bytes/second.                                     */
/*                                                                     */
/* The adapter supports 256 different character codes. An 8K-byte      */
/* character generator contains the fonts for the character codes. The */
/* characters, values, and screen characteristics are given in "Of     */
/* Characters, Keystrokes, and Colors" in your Technical Reference     */
/* system manual.                                                      */
/*                                                                     */
/* This adapter, when used with a display containing P39 phospher, does*/
/* not support a light pen.                                            */
/*                                                                     */
/* Where possible, only one low-power Schottky (LS) load is present on */
/* any I/O slot. Some of the address bus lines have two LS loads. No   */
/* signal has more than two LS loads.                                  */
/*                                                                     */
/* Characteristics of the adapter are:                                 */
/*  - Supports 80-character by 25-line screen                          */
/*  - Has direct-drive output                                          */
/*  - Supports 9-PEL by 14-PEL character box                           */
/*  - Supports 7-PEL by 9-PEL character                                */
/*  - Has 18-kHz monitor                                               */
/*  - Has character attributes                                         */
/* ------------------------------------------------------------------- */
/* The following is a block diagram of the monochrome display adapter  */
/* portion of the IBM Monochrome Display and Printer Adapter.          */
/* ----INSERT NAME OF FILE FOR DIAGRAM HERE----                        */
/* ------------------------------------------------------------------- */
/* Programing Considerations:                                          */
/* ------------------------------------------------------------------- */
/* The following table summarizes the 6845 controller module's internal*/
/* data registers, their functions, and their parameters. For the IBM  */
/* Monochrome Display, the values must be programmed into the 6845 to  */
/* ensure proper initialization of the display.                        */
/* ------------------------------------------------------------------- */
/* Register Register File:          Program Unit:IBM Monochrome Display*/
/* Number:                                        (Address in hex)     */
/* -  R0: - Horizontal Total:         - Characters:     - 61           */
/* -  R1: - Horizontal Displayed:     - Characters:     - 50           */
/* -  R2: - Horizontal Sync Position: - Characters:     - 52           */
/* -  R3: - Horizontal Sync Width:    - Characters:     - F            */
/* -  R4: - Vertical Total:           - Character Rows: - 19           */
/* -  R5: - Vertical Total Adjust:    - Scan Line:      - 6            */
/* -  R6: - Vertical Displayed:       - Character Row:  - 19           */
/* -  R7: - Vertical Sync Position:   - Character Row:  - 19           */
/* -  R8: - Interlace Mode:           - ---------:      - 02           */
/* -  R9: - Maximum Scan Line Address:- Scan Line:      - D            */
/* - R10: - Cursor Start:             - Scan Line:      - B            */
/* - R11: - Cursor End:               - Scan Line:      - C            */
/* - R12: - Start Address (H):        - ---------:      - 00           */
/* - R13: - Start Address (L):        - ---------:      - 00           */
/* - R14: - Cursor (H):               - ---------:      - 00           */
/* - R15: - Cursor (L):               - ---------:      - 00           */
/* - R16: - Reserved:                 - ---------:      - --           */
/* - R17: - Reserved:                 - ---------:      - --           */
/* ------------------------------------------------------------------- */
/* To ensure proper initialization, the first command issued to the IBM*/
/* Monochrome Display and Printer Adapter must be sent to the CRT      */
/* control port 1 (hex 3B8), and must be a hex 01, to set the high-    */
/* resolution mode. If this bit is not set, the system unit's          */
/* microprocessor's access to the adapter must never occur. If the high*/
/* -resolution bit is not set, the system unit's microprocessor will   */
/* stop running.                                                       */
/*                                                                     */
/* System configurations that have both an IBM Monochrome Display and  */
/* Printer Adapter, and an IBM Color/Graphics Monitor Adapter, must    */
/* ensure that both adapters are properly initialized after a power-on */
/* reset. Damage to either display may occur if not properly           */
/* initialized.                                                        */
/* ------------------------------------------------------------------- */
/* The IBM Monochrome Display and Printer Adapter supports 256         */
/* different character codes. In the character set are alphanumerics   */
/* and block graphics. Each character in the display buffer has a      */
/* corresponding character attribute. The character code must be an    */
/* even address, and the attribute code must be an odd address in the  */
/* display buffer.                                                     */
/*                                                                     */
/* Character Code Even Address (M):                                    */
/* - 7:                                                                */
/* - 6:                                                                */
/* - 5:                                                                */
/* - 4:                                                                */
/* - 3:                                                                */
/* - 2:                                                                */
/* - 1:                                                                */
/* - 0:                                                                */
/* Attribute Code Odd  Address (M+1):                                  */
/* - 7: BL: Blink                                                      */
/* - 6: R : Background                                                 */
/* - 5: G : Background                                                 */
/* - 4: B : Background                                                 */
/* - 3: I : Intensity                                                  */
/* - 2: R : Foreground                                                 */
/* - 1: G : Foreground                                                 */
/* - 0: B : Foreground                                                 */
/* The adapter decodes the character attribute byte as defined above.  */
/* The blink and intensity bits may be combined with the foreground and*/
/* background bits to further enhance the character attribute functions*/
/* listed below:                                                       */
/* Background Foreground Function:                                     */
/*  R  G  B:   R  G  B:  Function:                                     */
/*  0  0  0:   0  0  0:   - Non-Display                                */
/*  0  0  0:   0  0  1:   - Underline                                  */
/*  0  0  0:   1  1  1:   - White Character/Black Background           */
/*  1  1  1:   0  0  0:   - Reverse Video                              */
/* The 4K display buffer supports one screen of the 25 rows of 80      */
/* characters, plus a character attribute for each display character.  */
/* The starting address of the buffer is hex B0000. The display buffer */
/* can be read using direct memory access (DMA); however, at least one */
/* wait state will be inserted by the system unit's microprocessor. The*/
/* duration of the wait state will vary, because the microprocessor/   */
/* monitor is synchronized with the character clock on this adapter.   */
/* ------------------------------------------------------------------- */
/* Interrupt level 7 is used on the parallel interface. Interrupts can */
/* be enabled or disabled through the printer control port. The        */
/* interrupt is a high-level active signal.                            */
/*                                                                     */
/* The following table breaks down the functions of the I/O address    */
/* decode for the adapter. The I/O address decode is from hex 3B0      */
/* through hex 3BF. The bit assignment for each I/O address follows:   */
/* ------------------------------------------------------------------- */
/* I/O Address and Bit Map:                                            */
/* ------------------------------------------------------------------- */
/* I/O Register Address: Function:                                     */
/* - 3B0:                 - Not Used                                   */
/* - 3B1:                 - Not Used                                   */
/* - 3B2:                 - Not Used                                   */
/* - 3B3:                 - Not Used                                   */
/* - 3B4:                 - 6845 Index Register                        */
/* - 3B5:                 - 6845 Data  Register                        */
/* - 3B6:                 - Not Used                                   */
/* - 3B7:                 - Not Used                                   */
/* - 3B8:                 - CRT Control Port 1                         */
/* - 3B9:                 - Reserved                                   */
/* - 3BA:                 - CRT Status  Port                           */
/* - 3BB:                 - Reserved                                   */
/* - 3BC:                 - Parallel Data Port                         */
/* - 3BD:                 - Printer Status  Port                       */
/* - 3BE:                 - Printer Control Port                       */
/* - 3BF:                 - Not Used                                   */
/* ------------------------------------------------------------------- */
/* 6845 CRT Control Port 1 (Hex 3B8):                                  */
/* ------------------------------------------------------------------- */
/* Bit Number: Function:                                               */
/* - 0  :       - +High Resolution Mode                                */
/* - 1  :       - Not Used                                             */
/* - 2  :       - Not Used                                             */
/* - 3  :       - +Video Enable                                        */
/* - 4  :       - Not Used                                             */
/* - 5  :       - +Enable Blink                                        */
/* - 6,7:       - Not Used                                             */
/* ------------------------------------------------------------------- */
/* 6845 CRT Status  Port   (Hex 3BA):                                  */
/* ------------------------------------------------------------------- */
/* - 0  :       - +Horizontal Drive                                    */
/* - 1  :       - Reserved                                             */
/* - 2  :       - Reserved                                             */
/* - 3  :       - +Black/White Video                                   */
/* ------------------------------------------------------------------- */
static void mda_clock (mda5140_t *mda5140, unsigned long cnt);


/*
 * Set the blink frequency
 */
static
void mda5140_set_blink_rate (mda5140_t *mda, unsigned freq)
{
	mda5140->blink_on = 1;
	mda5140->blink_cnt = freq;
	mda5140->blink_freq = freq;

	mda5140->update_state |= MDA5140_UPDATE_DIRTY;
}

static
void mda5140_set_color (mda5140_t *mda5140, unsigned i1, unsigned i2, unsigned r, unsigned g, unsigned b)
{
	unsigned i;

	if ((i1 > 16) || (i2 > 16)) {
		return;
	}

	r &= 0xff;
	g &= 0xff;
	b &= 0xff;

	for (i = i1; i <= i2; i++) {
		mda5140->rgb[i][0] = r;
		mda5140->rgb[i][1] = g;
		mda5140->rgb[i][2] = b;
	}
}

/*
 * Map a color name to background/normal/bright RGB values
 */
static
void mda5140_get_color (const char *name,
	unsigned long *back, unsigned long *normal, unsigned long *bright)
{
	*back = 0x000000;

	if (strcmp (name, "amber") == 0) {
		*normal = 0xe89050;
		*bright = 0xfff0c8;
	}
	else if (strcmp (name, "green") == 0) {
		*normal = 0x55aa55;
		*bright = 0xaaffaa;
	}
	else if (strcmp (name, "gray") == 0) {
		*normal = 0xaaaaaa;
		*bright = 0xffffff;
	}
	else {
		*normal = 0xe89050;
		*bright = 0xfff0c8;
	}
}

/*
 * Map an attribute to foreground and background colors
 */
static
void mda5140_map_attribute (mda5140_t *mda5140, unsigned attr, const unsigned char **fg, const unsigned char **bg)
{
	/* ------------------------------------------------------------------- */
	/* The IBM Monochrome Display and Printer Adapter supports 256         */
	/* different character codes. In the character set are alphanumerics   */
	/* and block graphics. Each character in the display buffer has a      */
	/* corresponding character attribute. The character code must be an    */
	/* even address, and the attribute code must be an odd address in the  */
	/* display buffer.                                                     */
	/*                                                                     */
	/* Character Code Even Address (M):                                    */
	/* - 7:                                                                */
	/* - 6:                                                                */
	/* - 5:                                                                */
	/* - 4:                                                                */
	/* - 3:                                                                */
	/* - 2:                                                                */
	/* - 1:                                                                */
	/* - 0:                                                                */
	/* Attribute Code Odd  Address (M+1):                                  */
	/* - 7: BL: Blink                                                      */
	/* - 6: R : Background                                                 */
	/* - 5: G : Background                                                 */
	/* - 4: B : Background                                                 */
	/* - 3: I : Intensity                                                  */
	/* - 2: R : Foreground                                                 */
	/* - 1: G : Foreground                                                 */
	/* - 0: B : Foreground                                                 */
	/* The adapter decodes the character attribute byte as defined above.  */
	/* The blink and intensity bits may be combined with the foreground and*/
	/* background bits to further enhance the character attribute functions*/
	/* listed below:                                                       */
	/* Background Foreground Function:                                     */
	/*  R  G  B:   R  G  B:  Function:                                     */
	/*  0  0  0:   0  0  0:   - Non-Display                                */
	/*  0  0  0:   0  0  1:   - Underline                                  */
	/*  0  0  0:   1  1  1:   - White Character/Black Background           */
	/*  1  1  1:   0  0  0:   - Reverse Video                              */
	/* The 4K display buffer supports one screen of the 25 rows of 80      */
	/* characters, plus a character attribute for each display character.  */
	/* The starting address of the buffer is hex B0000. The display buffer */
	/* can be read using direct memory access (DMA); however, at least one */
	/* wait state will be inserted by the system unit's microprocessor. The*/
	/* duration of the wait state will vary, because the microprocessor/   */
	/* monitor is synchronized with the character clock on this adapter.   */
	/* ------------------------------------------------------------------- */
	if ((attr & 0x77) == 0x70) {
		*fg = mda5140->rgb[0];
		*bg = mda5140->rgb[7];

	/* ------------------------------------------------------------------- */
	/* 6845 CRT Control Port 1 (Hex 3B8):                                  */
	/* ------------------------------------------------------------------- */
	/* Bit Number: Function:                                               */
	/* - 0  :       - +High Resolution Mode                                */
	/* - 1  :       - Not Used                                             */
	/* - 2  :       - Not Used                                             */
	/* - 3  :       - +Video Enable                                        */
	/* - 4  :       - Not Used                                             */
	/* - 5  :       - +Enable Blink                                        */
	/* - 6,7:       - Not Used                                             */
	/* ------------------------------------------------------------------- */
		if (attr & 0x80) {
			if ((mda5140->reg[MDA5140_MODE] & MDA5140_MODE_BLINK) == 0) {
				*bg = mda5140->rgb[15];
			}
		}
	}
	else {
		*fg = mda5140->rgb[attr & 0x0f];
		*bg = mda5140->rgb[0];
	}
}

/*
 * Get CRTC start offset
 */
static
unsigned mda5140_get_start (mda5140_t *mda5140)
{
	unsigned val;

	val = mda5140->reg_crt[0x0c];
	val = (val << 8) | mda5140->reg_crt[0x0d];

	return (val);
}

/*
 * Get the absolute cursor position
 */
static
unsigned mda5140_get_cursor (mda5140_t *mda5140)
{
	unsigned val;

	val = mda5140->reg_crt[0x0e];
	val = (val << 8) | mda5140->reg_crt[0x0f];

	return (val);
}

/*
 * Get the on screen cursor position
 */
static
int mda5140_get_position (mda5140_t *mda5140, unsigned *x, unsigned *y)
{
	unsigned pos;

	if ((mda5140->w == 0) || (mda5140->h == 0)) {
		return (1);
	}

	pos = (mda5140_get_cursor (mda5140) - mda5140_get_start (mda5140)) & 0x0fff;

	*x = pos % mda5140->w;
	*y = pos / mda5140->w;

	return (0);
}

/*
 * Set the timing values from the CRTC registers
 */
static
void mda5140_set_timing (mda5140_t *mda5140)
{
	mda5140->ch = (mda5140->reg_crt[MDA5140_CRTC_MS] & 0x1f) + 1; /* R 9: Maximum Scan Line Address:  D */
	mda5140->w = mda5140->reg_crt[MDA5140_CRTC_HD]; /* R 1: Horizontal Displayed     : 50 */
	mda5140->h = mda5140->reg_crt[MDA5140_CRTC_VD]; /* R 6: Vertical Displayed       : 19 */

	mda5140->clk_ht = MDA5140_CW * (mda5140->reg_crt[MDA5140_CRTC_HT] + 1); /* R 0: Horizontal Total         : 61 */
	mda5140->clk_hd = MDA5140_CW * mda5140->reg_crt[MDA5140_CRTC_HD]; /* R 1: Horizontal Displayed     : 50 */

	mda5140->clk_vt = mda5140->ch * (mda5140->reg_crt[MDA5140_CRTC_VT] + 1) * mda->clk_ht; /* R 4: Vertical Total           : 19 */
	mda5140->clk_vd = mda5140->ch * mda5140->reg_crt[MDA5140_CRTC_VD] * mda->clk_ht;
}

/*
 * Get the dot clock
 */
static
unsigned long mda5140_get_dotclock (mda5140_t *mda5140)
{
	unsigned long long clk;

	clk = mda5140->video.dotclk[0];
	clk = (MDA5140_PFREQ * clk) / MDA5140_IFREQ;

	return (clk);
}

/*
 * Set the internal screen buffer size
 */
static
int mda5140_set_buf_size (mda5140_t *mda5140, unsigned w, unsigned h)
{
	unsigned long cnt;
	unsigned char *tmp;

	cnt = 3UL * (unsigned long) w * (unsigned long) h;

	if (cnt > mda5140->bufmax) {
		tmp = realloc (mda5140->buf, cnt);
		if (tmp == NULL) {
			return (1);
		}

		mda5140->buf = tmp;
		mda5140->bufmax = cnt;
	}

	mda5140->buf_w = w;
	mda5140->buf_h = h;

	return (0);
}

/*
 * Draw the cursor in the internal buffer
 */
static
void mda5140_draw_cursor (mda5140_t *mda5140)
{
	unsigned            i, j;
	unsigned            x, y;
	unsigned            c1, c2;
	unsigned            addr;
	const unsigned char *src;
	const unsigned char *col;
	unsigned char       *dst;

	if (mda5140->blink_on == 0) {
		return;
	}

	if ((mda5140->reg_crt[MDA5140_CRTC_CS] & 0x60) == 0x20) { /* R10: Cursor Start             :  B */
		/* cursor off */
		return;
	}

	src = mda5140->mem;

	if (mda5140_get_position (mda5140, &x, &y)) {
		return;
	}

	c1 = mda5140->reg_crt[MDA5140_CRTC_CS] & 0x1f; /* R10: Cursor Start             :  B */
	c2 = mda5140->reg_crt[MDA5140_CRTC_CE] & 0x1f; /* R11: Cursor End               :  C */

	if (c1 >= mda5140->ch) {
		return;
	}

	if (c2 >= mda5140->ch) {
		c2 = mda5140->ch - 1;
	}

	addr = (mda5140_get_start (mda5140) + 2 * (mda5140->w * y + x) + 1) & 0x0fff;

	col = mda5140->rgb[src[addr] & 0x0f];
	dst = mda5140->buf + 3 * MDA5140_CW * (mda5140->w * (mda5140->ch * y + c1) + x);

	for (j = c1; j <= c2; j++) {
		for (i = 0; i < MDA5140_CW; i++) {
			dst[3 * i + 0] = col[0];
			dst[3 * i + 1] = col[1];
			dst[3 * i + 2] = col[2];
		}
		dst += 3 * MDA5140_CW * mda5140->w;
	}
}

/*
 * Draw a character in the internal buffer
 */
static
void mda5140_draw_char (mda5140_t *mda5140, unsigned char *buf, unsigned char c, unsigned char a)
{
	unsigned            i, j;
	int                 elg, blk;
	unsigned            ull;
	unsigned            val;
	unsigned char       *dst;
	const unsigned char *fnt;
	const unsigned char *fg, *bg;

	blk = 0;

	/* ------------------------------------------------------------------- */
	/* 6845 CRT Control Port 1 (Hex 3B8):                                  */
	/* ------------------------------------------------------------------- */
	/* Bit Number: Function:                                               */
	/* - 0  :       - +High Resolution Mode                                */
	/* - 1  :       - Not Used                                             */
	/* - 2  :       - Not Used                                             */
	/* - 3  :       - +Video Enable                                        */
	/* - 4  :       - Not Used                                             */
	/* - 5  :       - +Enable Blink                                        */
	/* - 6,7:       - Not Used                                             */
	/* ------------------------------------------------------------------- */
	if (mda5140->reg[MDA5140_MODE] & MDA5140_MODE_BLINK) {
		if (a & 0x80) {
			blk = !mda5140->blink_on;
		}

		a &= 0x7f;
	}

	ull = ((a & 0x07) == 1) ? 13 : 0xffff;
	elg = ((c >= 0xc0) && (c <= 0xdf));

	mda5140_map_attribute (mda5140, a, &fg, &bg);

	fnt = mda5140->font + 14 * c;

	dst = buf;

	for (j = 0; j < mda5140->ch; j++) {
		if (blk) {
			val = 0x000;
		}
		else if (j == ull) {
			val = 0x1ff;
		}
		else {
			val = fnt[j % 14] << 1;

			if (elg) {
				val |= (val >> 1) & 1;
			}
		}

		for (i = 0; i < MDA5140_CW; i++) {
			if (val & 0x100) {
				dst[3 * i + 0] = fg[0];
				dst[3 * i + 1] = fg[1];
				dst[3 * i + 2] = fg[2];
			}
			else {
				dst[3 * i + 0] = bg[0];
				dst[3 * i + 1] = bg[1];
				dst[3 * i + 2] = bg[2];
			}

			val <<= 1;
		}

		dst += (3 * MDA5140_CW * mda5140->w);
	}
}

/*
 * Update the internal screen buffer when the screen is blank
 */
static
void mda5140_update_blank (mda5140_t *mda5140)
{
	unsigned long x, y;
	int           fx, fy;
	unsigned char *dst;

	mda5140_set_buf_size (mda5140, 720, 350);

	dst = mda5140->buf;

	for (y = 0; y < mda5140->buf_h; y++) {
		fy = (y % 16) < 8;

		for (x = 0; x < mda5140->buf_w; x++) {
			fx = (x % 16) < 8;

			dst[0] = (fx != fy) ? 0x20 : 0x00;
			dst[1] = dst[0];
			dst[2] = dst[0];

			dst += 3;
		}
	}
}

/*
 * Update the internal screen buffer
 */
static
void mda5140_update (mda5140_t *mda5140)
{
	unsigned            x, y;
	unsigned            ofs;
	const unsigned char *src;
	unsigned char       *dst;

	/* ------------------------------------------------------------------- */
	/* 6845 CRT Control Port 1 (Hex 3B8):                                  */
	/* ------------------------------------------------------------------- */
	/* Bit Number: Function:                                               */
	/* - 0  :       - +High Resolution Mode                                */
	/* - 1  :       - Not Used                                             */
	/* - 2  :       - Not Used                                             */
	/* - 3  :       - +Video Enable                                        */
	/* - 4  :       - Not Used                                             */
	/* - 5  :       - +Enable Blink                                        */
	/* - 6,7:       - Not Used                                             */
	/* ------------------------------------------------------------------- */
	if ((mda5140->reg[MDA5140_MODE] & MDA5140_MODE_ENABLE) == 0) {
		mda5140_update_blank (mda5140);
		return;
	}

	if ((mda5140->w == 0) || (mda5140->h == 0)) {
		mda5140_update_blank (mda5140);
		return;
	}

	if (mda5140_set_buf_size (mda5140, MDA5140_CW * mda5140->w, mda5140->ch * mda5140->h)) {
		return;
	}

	src = mda5140->mem;
	ofs = (2 * mda5140_get_start (mda5140)) & 0x0ffe;

	dst = mda5140->buf;

	for (y = 0; y < mda5140->h; y++) {
		for (x = 0; x < mda5140->w; x++) {
			mda5140_draw_char (mda5140, dst + 3 * MDA5140_CW * x, src[ofs], src[ofs + 1]);

			ofs = (ofs + 2) & 0x0ffe;
		}

		dst += 3 * (MDA5140_CW * mda5140->w) * mda5140->ch;
	}

	mda5140_draw_cursor (mda5140);
}


/*
 * Get a CRTC register
 */
static
unsigned char mda5140_crtc_get_reg (mda5140_t *mda5140, unsigned reg)
{
	if (reg > 15) {
		return (0xff);
	}

	return (mda5140->reg_crt[reg]);
}

/*
 * Set a CRTC register
 */
static
void mda5140_crtc_set_reg (mda5140_t *mda5140, unsigned reg, unsigned char val)
{
	if (reg > 15) {
		return;
	}

	if (mda5140->reg_crt[reg] == val) {
		return;
	}

	mda5140->reg_crt[reg] = val;

	mda5140_set_timing (mda5140);

	mda5140->update_state |= MDA5140_UPDATE_DIRTY;
}


/*
 * Get the CRTC index register
 */
static
unsigned char mda5140_get_crtc_index (mda5140_t *mda5140)
{
	return (mda5140->reg[MDA5140_CRTC_INDEX]);
}

/*
 * Get the CRTC data register
 */
static
unsigned char mda5140_get_crtc_data (mda5140_t *mda5140)
{
	return (mda5140_crtc_get_reg (mda5140, mda5140->reg[MDA5140_CRTC_INDEX]));
}

/*
 * Get the mode control register
 */
static
unsigned char mda5140_get_mode (mda5140_t *mda5140)
{
	return (mda5140->reg[MDA5140_MODE]);
}

/*
 * Get the status register
 */
static
unsigned char mda5140_get_status (mda5140_t *mda5140)
{
	unsigned char val, vid;
	unsigned long clk;

	mda5140_clock (mda5140, 0);

	clk = mda5140_get_dotclock (mda5140);

	/* simulate the video signal */
	/* ------------------------------------------------------------------- */
	/* 6845 CRT Status  Port   (Hex 3BA):                                  */
	/* ------------------------------------------------------------------- */
	/* - 0  :       - +Horizontal Drive                                    */
	/* - 1  :       - Reserved                                             */
	/* - 2  :       - Reserved                                             */
	/* - 3  :       - +Black/White Video                                   */
	/* ------------------------------------------------------------------- */
	mda5140->reg[MDA5140_STATUS] ^= MDA5140_STATUS_VIDEO;

	val = mda5140->reg[MDA5140_STATUS] & ~MDA5140_STATUS_VIDEO;
	vid = mda5140->reg[MDA5140_STATUS] & MDA5140_STATUS_VIDEO;

	/* ------------------------------------------------------------------- */
	/* 6845 CRT Status  Port   (Hex 3BA):                                  */
	/* ------------------------------------------------------------------- */
	/* - 0  :       - +Horizontal Drive                                    */
	/* - 1  :       - Reserved                                             */
	/* - 2  :       - Reserved                                             */
	/* - 3  :       - +Black/White Video                                   */
	/* ------------------------------------------------------------------- */
	val |= MDA5140_STATUS_HSYNC;

	if (mda5140->clk_ht > 0) {
		if ((clk % mda5140->clk_ht) < mda5140->clk_hd) {
			val &= ~MDA5140_STATUS_HSYNC;

			if (clk < mda5140->clk_vd) {
				val |= vid;
			}
		}
	}

	return (val);
}

/*
 * Set the CRTC index register
 */
static
void mda5140_set_crtc_index (mda5140_t *mda5140, unsigned char val)
{
	mda5140->reg[MDA5140_CRTC_INDEX] = val;
}

/*
 * Set the CRTC data register
 */
static
void mda5140_set_crtc_data (mda5140_t *mda5140, unsigned char val)
{
	mda5140->reg[MDA5140_CRTC_DATA] = val;

	mda5140_crtc_set_reg (mda5140, mda5140->reg[MDA5140_CRTC_INDEX], val);
}

/*
 * Set the mode register
 */
static
void mda5140_set_mode (mda5140_t *mda5140, unsigned char val)
{
	mda5140->reg[MDA5140_MODE] = val;

	mda5140->update_state |= MDA5140_UPDATE_DIRTY;
}

/*
 * Get an MDA5140 register
 */
static
unsigned char mda5140_reg_get_uint8 (mda5140_t *mda5140, unsigned long addr)
{
	switch (addr) {
	case MDA5140_CRTC_INDEX:
		return (mda5140_get_crtc_index (mda5140));

	case MDA5140_CRTC_DATA:
		return (mda5140_get_crtc_data (mda5140));

	case MDA5140_STATUS:
		return (mda5140_get_status (mda5140));

	default:
		return (0xff);
	}
}

static
unsigned short mda5140_reg_get_uint16 (mda5140_t *mda5140, unsigned long addr)
{
	unsigned short ret;

	ret = mda5140_reg_get_uint8 (mda5140, addr);
	ret |= mda5140_reg_get_uint8 (mda5140, addr + 1) << 8;

	return (ret);
}

/*
 * Set an MDA5140 register
 */
static
void mda5140_reg_set_uint8 (mda5140_t *mda5140, unsigned long addr, unsigned char val)
{
	switch (addr) {
	case MDA5140_CRTC_INDEX:
		mda5140_set_crtc_index (mda5140, val);
		break;

	case MDA5140_CRTC_DATA:
		mda5140_set_crtc_data (mda5140, val);
		break;

	case MDA5140_MODE:
		mda5140_set_mode (mda5140, val);
		break;
	}
}

static
void mda5140_reg_set_uint16 (mda5140_t *mda5140, unsigned long addr, unsigned short val)
{
	mda5140_reg_set_uint8 (mda5140, addr + 0, val & 0xff);
	mda5140_reg_set_uint8 (mda5140, addr + 1, (val >> 8) & 0xff);
}


static
unsigned char mda5140_mem_get_uint8 (mda5140_t *mda5140, unsigned long addr)
{
	return (mda5140->mem[addr & 4095]);
}

static
unsigned short mda5140_mem_get_uint16 (mda5140_t *mda5140, unsigned long addr)
{
	unsigned short val;

	val = mda5140_mem_get_uint8 (mda5140, addr);
	val |= (unsigned) mda5140_mem_get_uint8 (mda5140, addr + 1) << 8;

	return (val);
}

static
void mda5140_mem_set_uint8 (mda5140_t *mda5140, unsigned long addr, unsigned char val)
{
	addr &= 4095;

	if (mda5140->mem[addr] == val) {
		return;
	}

	mda5140->mem[addr] = val;

	mda5140->update_state |= MDA5140_UPDATE_DIRTY;
}

static
void mda5140_mem_set_uint16 (mda5140_t *mda5140, unsigned long addr, unsigned short val)
{
	mda5140_mem_set_uint8 (mda5140, addr + 0, val & 0xff);

	if ((addr + 1) < mda5140->memblk->size) {
		mda5140_mem_set_uint8 (mda5140, addr + 1, (val >> 8) & 0xff);
	}
}


static
void mda5140_del (mda5140_t *mda5140)
{
	if (mda5140 != NULL) {
		mem_blk_del (mda5140->memblk);
		mem_blk_del (mda5140->regblk);
		free (mda5140);
	}
}

static
int mda5140_set_msg (mda5140_t *mda5140, const char *msg, const char *val)
{
	if (msg_is_message ("emu.video.blink", msg)) {
		unsigned freq;

		if (msg_get_uint (val, &freq)) {
			return (1);
		}

		mda5140_set_blink_rate (mda5140, freq);

		return (0);
	}

	return (-1);
}

static
void mda5140_set_terminal (mda5140_t *mda5140, terminal_t *trm)
{
	mda5140->term = trm;

	if (mda5140->term != NULL) {
		trm_open (mda5140->term, 720, 350);
	}
}

static
mem_blk_t *mda5140_get_mem (mda5140_t *mda5140)
{
	return (mda5140->memblk);
}

static
mem_blk_t *mda5140_get_reg (mda5140_t *mda5140)
{
	return (mda5140->regblk);
}

static
void mda5140_print_info (mda5140_t *mda5140, FILE *fp)
{
	unsigned i;
	unsigned pos, ofs;

	fprintf (fp, "Video adapter: Monochrome Display Adapter (MDA5140)\n");

	pos = mda5140_get_cursor (mda5140);
	ofs = mda5140_get_start (mda5140);

	fprintf (fp, "MDA5140: OFS=%04X  POS=%04X\n",
		 ofs, pos
	);

	fprintf (fp, "CLK: CLK=%lu  HT=%lu HD=%lu  VT=%lu VD=%lu\n",
		mda5140_get_dotclock (mda5140),
		mda5140->clk_ht, mda5140->clk_hd,
		mda5140->clk_vt, mda5140->clk_vd
	);

	fprintf (fp, "REG: CRTC=%02X  MODE=%02X  STATUS=%02X\n",
		mda5140->reg[MDA5140_CRTC_INDEX], mda5140_get_mode (mda5140),
		mda5140_get_status (mda5140)
	);

	fprintf (fp, "CRTC=[%02X", mda5140->reg_crt[0]);

	for (i = 1; i < 18; i++) {
		if ((i & 7) == 0) {
			fputs ("-", fp);
		}
		else {
			fputs (" ", fp);
		}
		fprintf (fp, "%02X", mda5140->reg_crt[i]);
	}
	fputs ("]\n", fp);

	fflush (fp);
}

/*
 * Force a screen update
 */
static
void mda5140_redraw (mda5140_t *mda5140, int now)
{
	if (now) {
		if (mda5140->term != NULL) {
			mda5140_update (mda5140);
			trm_set_size (mda5140->term, mda5140->buf_w, mda5140->buf_h);
			trm_set_lines (mda5140->term, mda5140->buf, 0, mda5140->buf_h);
			trm_update (mda5140->term);
		}
	}

	mda5140->update_state |= MDA5140_UPDATE_DIRTY;
}

static
void mda5140_clock (mda5140_t *mda5140, unsigned long cnt)
{
	unsigned long clk;

	if (mda5140->clk_vt < 50000) {
		return;
	}

	clk = mda5140_get_dotclock (mda5140);

	if (clk < mda5140->clk_vd) {
		mda5140->update_state &= ~MDA5140_UPDATE_RETRACE;
		return;
	}

	if (clk >= mda5140->clk_vt) {
		mda5140->video.dotclk[0] = 0;
		mda5140->video.dotclk[1] = 0;
		mda5140->video.dotclk[2] = 0;
	}

	if (mda5140->update_state & MDA5140_UPDATE_RETRACE) {
		return;
	}

	if (mda5140->blink_cnt > 0) {
		mda5140->blink_cnt -= 1;

		if (mda5140->blink_cnt == 0) {
			mda5140->blink_cnt = mda5140->blink_freq;
			mda5140->blink_on = !mda5140->blink_on;
			mda5140->update_state |= MDA5140_UPDATE_DIRTY;
		}
	}

	if (mda5140->term != NULL) {
		if (mda5140->update_state & MDA5140_UPDATE_DIRTY) {
			mda5140_update (mda5140);
			trm_set_size (mda5140->term, mda5140->buf_w, mda5140->buf_h);
			trm_set_lines (mda5140->term, mda5140->buf, 0, mda5140->buf_h);
		}

		trm_update (mda5140->term);
	}

	mda5140->update_state = MDA5140_UPDATE_RETRACE;
}

mda5140_t *mda5140_new (unsigned long io, unsigned long mem, unsigned long size)
{
	unsigned i;
	mda5140_t    *mda5140;

	mda5140 = malloc (sizeof (mda5140_t));
	if (mda5140 == NULL) {
		return (NULL);
	}

	pce_video_init (&mda5140->video);

	mda5140->video.ext = mda5140;
	mda5140->video.del = (void *) mda5140_del;
	mda5140->video.set_msg = (void *) mda5140_set_msg;
	mda5140->video.set_terminal = (void *) mda5140_set_terminal;
	mda5140->video.get_mem = (void *) mda5140_get_mem;
	mda5140->video.get_reg = (void *) mda5140_get_reg;
	mda5140->video.print_info = (void *) mda5140_print_info;
	mda5140->video.redraw = (void *) mda5140_redraw;
	mda5140->video.clock = (void *) mda5140_clock;

	if (size < 4096) {
		size = 4096;
	}

	mda5140->memblk = mem_blk_new (mem, size, 0);
	mda5140->memblk->ext = mda5140;
	mda5140->memblk->set_uint8 = (void *) mda5140_mem_set_uint8;
	mda5140->memblk->set_uint16 = (void *) mda5140_mem_set_uint16;
	mda5140->memblk->get_uint8 = (void *) mda5140_mem_get_uint8;
	mda5140->memblk->get_uint16 = (void *) mda5140_mem_get_uint16;
	memset (mda5140->mem, 0, sizeof (mda5140->mem));

	mda5140->regblk = mem_blk_new (io, 16, 1);
	mda5140->regblk->ext = mda5140;
	mda5140->regblk->set_uint8 = (void *) mda5140_reg_set_uint8;
	mda5140->regblk->set_uint16 = (void *) mda5140_reg_set_uint16;
	mda5140->regblk->get_uint8 = (void *) mda5140_reg_get_uint8;
	mda5140->regblk->get_uint16 = (void *) mda5140_reg_get_uint16;
	mda5140->reg = mda5140->regblk->data;
	mem_blk_clear (mda5140->regblk, 0x00);

	mda5140->term = NULL;

	for (i = 0; i < 18; i++) {
		mda5140->reg_crt[i] = 0;
	}

	mda5140->font = mda5140_font_8x14;

	mda5140_set_color (mda5140, 0, 0, 0x00, 0x00, 0x00);
	mda5140_set_color (mda5140, 1, 7, 0x00, 0xaa, 0x00);
	mda5140_set_color (mda5140, 8, 15, 0xaa, 0xff, 0xaa);

	mda5140->w = 0;
	mda5140->h = 0;
	mda5140->ch = 0;

	mda5140->clk_ht = 0;
	mda5140->clk_vt = 0;
	mda5140->clk_hd = 0;
	mda5140->clk_vd = 0;

	mda5140->bufmax = 0;
	mda5140->buf = NULL;

	mda5140->update_state = 0;

	return (mda5140);
}

video_t *mda5140_new_ini (ini_sct_t *sct)
{
	unsigned long io, addr, size;
	unsigned long col0, col1, col2;
	const char    *col;
	unsigned      blink;
	mda5140_t         *mda5140;

	ini_get_uint32 (sct, "io", &io, 0x3b4);
	ini_get_uint32 (sct, "address", &addr, 0xb0000);
	ini_get_uint32 (sct, "size", &size, 32768);

	ini_get_uint16 (sct, "blink", &blink, 0);

	pce_log_tag (MSG_INF,
		"VIDEO:", "MDA5140 io=0x%04lx addr=0x%05lx size=0x%05lx\n",
		io, addr, size
	);

	ini_get_string (sct, "color", &col, "green");

	mda5140_get_color (col, &col0, &col1, &col2);

	ini_get_uint32 (sct, "color_background", &col0, col0);
	ini_get_uint32 (sct, "color_normal", &col1, col1);
	ini_get_uint32 (sct, "color_bright", &col2, col2);

	mda5140 = mda5140_new (io, addr, size);
	if (mda5140 == NULL) {
		return (NULL);
	}

	mda5140_set_color (mda5140, 0, 0, col0 >> 16, col0 >> 8, col0);
	mda5140_set_color (mda5140, 1, 7, col1 >> 16, col1 >> 8, col1);
	mda5140_set_color (mda5140, 8, 15, col2 >> 16, col2 >> 8, col2);

	mda5140_set_blink_rate (mda5140, blink);

	return (&mda5140->video);
}