/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/video/cga.c                                      *
 * Created:     2003-04-18 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2014 Hampa Hug <hampa@hampa.ch>                     *
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
#include <math.h>

#include <lib/log.h>
#include <lib/msg.h>

#include <devices/video/compaq.h>


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
/* -------------------------------------------------------------------- */
/* Timing Generator:                                                    */
/* -------------------------------------------------------------------- */
/* This generator produces the timing signals used by the 6845 CRT      */
/* Controller and by the dynamic memory. It also solves the processor/  */
/* graphic controller contentions for access to the display buffer.     */
/* -------------------------------------------------------------------- */
/* ------------------------------------------------------------------------------------------- */
/* Quote from John Elliott, Vintage PC pages, Compaq Portable/Deskpro, Video Output:           */
/* ------------------------------------------------------------------------------------------- */
/* Video output is the major difference between a Compaq (either sort) and the original IBM.   */
/* The cleverness is partly in the video card, and partly in the ROM BIOS.                     */
/*                                                                                             */
/* At the time the Compaq Portable was released, IBM users had the choice of MDA or CGA video  */
/* video. MDA has a single high-quality 80x25 text mode with 9x14 character bitmaps, while CGA */
/* has grainier 80x25 and 40x25 text modes (with 8x8 character bitmaps) and 640x200 or 320x200 */
/* graphics. The matching IBM monitors are single-frequency, so an MDA display can't display a */
/* CGA signal or vice versa.                                                                   */
/*                                                                                             */
/* The trick that the Compaq computers play is that the Portable's built-in display, and the   */
/* Deskpro's mono monitor, can display either type of signal. Therefore the CGA's graphics     */
/* modes and its 40x25 text appear as usual (albeit in greenscale rather than colour) but its  */
/* 80x25 text mode is replaced by the higher-resolution MDA version.                           */
/*                                                                                             */
/* This is all very well, if you're using a Compaq monitor that can change scan rates like     */
/* this. If you're not, there are various provisions.                                          */
/*                                                                                             */
/* 1. If the video card in the Portable is in the high-resolution mode, it does not send any   */
/* signal to the external monitor socket. That way, an external monitor won't be damaged trying*/
/* to display signals it can't cope with.                                                      */
/* 2. By default the card in the Deskpro doesn't do this, because the Deskpro monitor connects */
/* to the external socket and needs the high- resolution signals. However, the original        */
/* behaviour can be reinstated by changing a couple of jumpers, so the same card can be used   */
/* in the Portable.                                                                            */
/* 3. The Deskpro can also be set to use a CGA monitor rather than a Compaq one. This is done  */
/* by turning motherboard DIP switch SW6 on: if so, the CGA 80x25 mode is used rather than the */
/* high-resolution one.                                                                        */
/* 4. It's possible to switch between the high-resolution 80x25 mode and the standard CGA one  */
/* by using key combinations. CTRL + ALT + < selects the CGA-compatible mode, while CTRL + ALT */
/* + > selects the high-resolution version. This is the only way to get the Portable to display*/
/* 80x25 output on an external CGA monitor.                                                    */
/*                                                                                             */
/* The method used by the card to detect what resolution to use is very simple: if the CRTC    */
/* character height register is 13 (ie, characters are 14 rows) then it goes into the high     */
/* resolution mode. If not, it behaves like a normal CGA.                                      */
/*                                                                                             */
/* The card in the Deskpro has an extra feature: It can be made to display text attributes like*/
/* an MDA does (ie, you only get bright and underlined) rather than in 15 shades of green. This*/
/* feature only applies in the high-resolution mode, and it's selected by setting bit 7 of the */
/* CGA control register (port 03D8h).                                                          */
/* ------------------------------------------------------------------------------------------- */
#define COMPAQ_IFREQ 1193182
#define COMPAQ_MDA_PFREQ 16257000
#define COMPAQ_CGA_PFREQ 14318184      /* CGA clock crystal for COMPAQ video display board                      */
#define COMPAQ_MDA_HFREQ 18430         /* MDA clock crystal for COMPAQ video display board                      */
#define COMPAQ_CGA_HFREQ 15750         /* CGA horizontal frequency clock crystal for COMPAQ video display board */
#define COMPAQ_MDA_VFREQ 50            /* MDA horizontal frequency clock crystal for COMPAQ video display board */
#define COMPAQ_CGA_VFREQ 60            /* CGA vertical   frequency clock crystal for COMPAQ video display board */

/* character width */
#define COMPAQ_CW    9
/* -------------------------------------------------------------------- */
/* Controller:                                                          */
/* -------------------------------------------------------------------- */
/* The controller is a Motorola 6845 Cathode Ray Tube (CRT) controller. */
/* It provides the necessary interface to drive the raster-scan CRT.    */
/* -------------------------------------------------------------------- */
/* Mode Set Register:                                                   */
/* -------------------------------------------------------------------- */
/* The mode set register is a general-purpose, programmable, I/O        */
/* register. It has I/O ports that may be individually programmed. Its  */
/* function in this adapter is to provide mode selection and color      */
/* selection in the medium-resolution color-graphics mode.              */
/* -------------------------------------------------------------------- */
/* Programming Considerations:                                          */
/* -------------------------------------------------------------------- */
/* Programming the Mode Control and Status Register:                    */
/* -------------------------------------------------------------------- */
/* The following I/O devices are defined on the Color/Graphics Monitor  */
/* Adapter.                                                             */
/*                                                                      */
/* Address:A9:A8:A7:A6:A5:A4:A3:A2:A1:A0:Function of Register:          */
/*  - 3D8: 1  1  1  1  0  1  1  0  0  0 : - Mode Control Register(D0)   */
/*  - 3D9: 1  1  1  1  0  1  1  0  0  1 : - Color Select Register(D0)   */
/*  - 3DA: 1  1  1  1  0  1  1  0  1  0 : - Status       Register(D1)   */
/*  - 3DB: 1  1  1  1  0  1  1  0  1  1 : - Clear  Light Pen Latch      */
/*  - 3DC: 1  1  1  1  0  1  1  1  0  0 : - Preset Light Pen Latch      */
/*  - 3D4: 1  1  1  1  0  1  0  Z  Z  0 : - 6845 Index   Register       */
/*  - 3D5: 1  1  1  1  0  1  0  Z  Z  1 : - 6845 Data    Register       */
/* Z = don't care condition                                             */
/* -------------------------------------------------------------------- */
#define COMPAQ_CRTC_INDEX0  0  /* 6845 Index   Register  */
#define COMPAQ_CRTC_DATA0   1  /* 6845 Data    Register  */
#define COMPAQ_CRTC_INDEX   4  /* 6845 Index   Register  */
#define COMPAQ_CRTC_DATA    5  /* 6845 Data    Register  */
/* #define COMPAQ_MODE         6  /* Mode Control Register  */ /* Disabled; replaced with Mode Control Register code below */
#define COMPAQ_MODE         8  /* Mode Control Register  */
#define COMPAQ_CSEL         9  /* Color Select Register  */
#define COMPAQ_STATUS       10 /* Status       Register  */
#define COMPAQ_PEN_RESET    11 /* Preset Light Pen Latch */
#define COMPAQ_PEN_SET      12 /* Clear  Light Pen Latch */
/* -------------------------------------------------------------------- */
/* Sequence of Events for Changing Modes:                               */
/* -------------------------------------------------------------------- */
/* - 1. Determine the mode of operation.                                */
/* - 2. Reset the video-enable bit in the mode-control register.        */
/* - 3. Program the 6845 CRT controller to select the mode.             */
/* - 4. Program the mode-control and color-select registers including   */
/* re-enabling the video.                                               */
/* -------------------------------------------------------------------- */
/* Programming the 6845 CRT Controller:                                 */
/* -------------------------------------------------------------------- */
/* The controller has 19 internal accessible register, which are used to*/
/* tdefine and control a raster-scan CRT display. One of these registers*/
/* , the index register, is used as a pointer to the other 18 registers.*/
/* It is a write-only register, which is loaded from the processor by   */
/* executing an 'out' instruction to I/O address hex 3D4. The five least*/
/* -significant bits of the I/O bus are loaded into the index register. */
/*                                                                      */
/* In order to load any of the other 18 registers, the index register is*/
/* first loaded with the necessary pointer, then the data register is   */
/* loaded with the information to be placed in the selected register.   */
/* The data register is loaded from the processor by executing an 'out' */
/* instruction to I/O address hex 3D5.                                  */
/*                                                                      */
/* The table on the next page defines the values that must be loaded    */
/* int the 6845 CRT Controller registers to control the different modes */
/* of operation supported by the attachment.                            */
/* -------------------------------------------------------------------- */
/* 6845 Register Description:                                           */
/* -------------------------------------------------------------------- */
/* Note: All register values are given in hexadecimal                   */
/*                                             40 by 25 80 by 25        */
/* Address  Register:Register:  Units:    I/O:  Alpha-   Alpha-  Graphic*/
/* Register:Number:  Type:      Units:    I/O:  numeric:numeric: Modes: */
/*  -  0     - R 0   Horizontal Character Write  - 38    - 71     - 38  */
/*                   Total                Only                          */
/*  -  1     - R 1   Horizontal Character Write  - 28    - 50     - 28  */
/*                   Displayed            Only                          */
/*  -  2     - R 2   Horizontal Character Write  - 2D    - 5A     - 2D  */
/*                   Sync Position        Only                          */
/*  -  3     - R 3   Horizontal Character Write  - 0A    - 0A     - 0A  */
/*                   Sync Width           Only                          */
/*  -  4     - R 4   Vertical   Character Write  - 1F    - 1F     - 1F  */
/*                   Total         Row    Only                          */
/*  -  5     - R 5   Vertical   Scan Line Write  - 06    - 06     - 06  */
/*                   Total Adjust         Only                          */
/*  -  6     - R 6   Vertical   Character Write  - 19    - 19     - 64  */
/*                   Displayed     Row    Only                          */
/*  -  7     - R 7   Vertical   Character Write  - 1C    - 1C     - 1C  */
/*                   Sync Position Row    Only                          */
/*  -  8     - R 8   Interlace      -     Write  - 02    - 02     - 02  */
/*                   Mode                 Only                          */
/*  -  9     - R 9   Maximum    Scan Line Write  - 07    - 07     - 07  */
/*                   Scan Line            Only                          */
/*                   Address                                            */
/*  -  A     - R10 Cursor Start Scan Line Write  - 06    - 06     - 06  */
/*                                        Only                          */
/*  -  B     - R11 Cursor End   Scan Line Write  - 07    - 07     - 07  */
/*                                        Only                          */
/*  -  C     - R12 Start            -     Write  - 00    - 00     - 00  */
/*                 Address (H)            Only                          */
/*  -  D     - R13 Start            -     Write  - 00    - 00     - 00  */
/*                 Address (L)            Only                          */
/*  -  E     - R14 Cursor           -     Read/  - XX    - XX     - XX  */
/*                 Address (H)            Write                         */
/*  -  F     - R15 Cursor           -     Read/  - XX    - XX     - XX  */
/*                 Address (L)            Write                         */
/*  - 10     - R16 Light Pen (H)    -     Read   - XX    - XX     - XX  */
/*                                        Only                          */
/*  - 11     - R17 Light Pen (L)    -     Read   - XX    - XX     - XX  */
/*                                        Only                          */
/* -------------------------------------------------------------------- */
#define COMPAQ_CRTC_HT      0  /*  0/R 0: Horizontal Total              : 61 */
#define COMPAQ_CRTC_HD      1  /*  1/R 1: Horizontal Displayed          : 52 */
#define COMPAQ_CRTC_HS      2  /*  2/R 2: Horizontal Sync Position      : 50 */
#define COMPAQ_CRTC_SYN     3  /*  3/R 3: Horizontal Sync Width         :  F*/
#define COMPAQ_CRTC_VT      4  /*  4/R 4: Vertical   Total         Row  : 19 */
#define COMPAQ_CRTC_VTA     5  /*  5/R 5: Vertical   Total Adjust       :  6 */
#define COMPAQ_CRTC_VD      6  /*  6/R 6: Vertical   Displayed     Row  : 19 */
#define COMPAQ_CRTC_VS      7  /*  7/R 7: Vertical   Sync Position Row  : 19 */
#define COMPAQ_CRTC_IL      8  /*  8/R 8: Interlace Mode                : 02 */
#define COMPAQ_CRTC_MS      9  /*  9/R 9: Maximum Scan Line Address     :  D */ /* See note below */
#define COMPAQ_CRTC_CS      10 /*  A/R10: Cursor Start                  :  B */
#define COMPAQ_CRTC_CE      11 /*  B/R11: Cursor End                    :  C */
#define COMPAQ_CRTC_SH      12 /*  C/R12: Start             Address (H) : 00 */
#define COMPAQ_CRTC_SL      13 /*  D/R13: Start             Address (L) : 00 */
#define COMPAQ_CRTC_PH      14 /*  E/R14: Cursor            Address (H) : 00 */
#define COMPAQ_CRTC_PL      15 /*  F/R15: Cursor            Address (L) : 00 */
#define COMPAQ_CRTC_LH      16 /* 10/R16: Light Pen                 (H) : -- */
#define COMPAQ_CRTC_LL      17 /* 11/R17: Light Pen                 (L) : -- */
/* ------------------------------------------------------------------------------------------- */
/* Quote from John Elliott, Vintage PC pages, Compaq Portable/Deskpro, Video Output:           */
/* ------------------------------------------------------------------------------------------- */
/* The method used by the card to detect what resolution to use is very simple: if the CRTC    */
/* character height register is 13 (ie, characters are 14 rows) then it goes into the high     */
/* resolution mode. If not, it behaves like a normal CGA.                                      */
/* ------------------------------------------------------------------------------------------- */
/* ------------------------------------------------------------------- */
/* 6845 CRT Control Port 1 (Hex 3B8):                                  */
/* ------------------------------------------------------------------- */
/* Bit Number: Function:                                               */
/* - 0  :       - +High Resolution Mode                                */
/* - 3  :       - +Video Enable                                        */
/* - 5  :       - +Enable Blink                                        */
/* ------------------------------------------------------------------- */
/* mode control register */
#define COMPAQ_MODE_CS             0x01		/* clock select */
#define COMPAQ_MODE_G320           0x02
#define COMPAQ_MODE_CBURST         0x04
#define COMPAQ_MODE_ENABLE         0x08
#define COMPAQ_MODE_G640           0x10
#define COMPAQ_MODE_BLINK          0x20
#define COMPAQ_MODE_UNDEFINED      0x40 /* Undefined Bit 6 */
#define COMPAQ_MODE_HIRESMONOCOLOR 0x80 /* Switch between MDA text colors and 15 shades of green */
/* ------------------------------------------------------------------------------------------- */
/* Quote from John Elliott, Vintage PC pages, Compaq Portable/Deskpro, Video Output:           */
/* ------------------------------------------------------------------------------------------- */
/* The card in the Deskpro has an extra feature: It can be made to display text attributes like*/
/* an MDA does (ie, you only get bright and underlined) rather than in 15 shades of green. This*/
/* feature only applies in the high-resolution mode, and it's selected by setting bit 7 of the */
/* CGA control register (port 03D8h).                                                          */
/* ------------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
/* Color-Select Register:                                               */
/* -------------------------------------------------------------------- */
/* The color-select register is a 6-bit output-only register. Its I/O   */
/* address is hex 3D9, and it can be written to using a processor 'out' */
/* command. The following are the bit definitions for this register.    */
/*  - Bit 0: Selects blue border color in 40 by 25 alphanumeric mode.   */
/*  Selects blue background color (C0-C1) in 320 by 200 graphics mode.  */
/*  Selects blue foreground color in 640 by 200 graphics mode.          */
/*  - Bit 1: Selects green border color in 40 by 25 alphanumeric mode.  */
/*  Selects green background color (C0-C1) in 320 by 200 graphics mode. */
/*  Selects green foreground color in 640 by 200 graphics mode.         */
/*  - Bit 2: Selects red border color in 40 by 25 alphanumeric mode.    */
/*  Selects red background color (C0-C1) in 320 by 200 graphics mode.   */
/*  Selects red foreground color in 640 by 200 graphics mode.           */
/*  - Bit 3: Selects intensified border color in 40 by 25 alphanumeric  */
/*  mode. Selects intensified background color (C0-C1) in 320 by 200    */
/*  graphics mode. Selects intensified foreground color in 640 by 200   */
/*  graphics mode.                                                      */
/*  - Bit 4:Selects alternate, intensified set of colors in the graphics*/
/*  mode. Selects background colors in the alphanumeric mode.           */
/*  - Bit 5: Selects active color set in 320 by 200 graphics mode.      */
/*  When bit 5 is set to 1, colors are determined as follows:           */
/*  C1:  C0:  Set Selected:                                             */
/*   - 0: - 0: - Background (Defined by bits 0-3 of port hex 3D9)       */
/*   - 0: - 1: - Cyan                                                   */
/*   - 1: - 0: - Magenta                                                */
/*   - 1: - 1: - White                                                  */
/*  When bit 5 is set to 0, colors are determined as follows:           */
/*  C1:  C0:  Set Selected:                                             */
/*   - 0: - 0: - Background (Defined by bits 0-3 of port hex 3D9)       */
/*   - 0: - 1: - Green                                                  */
/*   - 1: - 0: - Red                                                    */
/*   - 1: - 1: - Brown                                                  */
/*  - Bit 6: Not used                                                   */
/*  - Bit 7: Not used                                                   */
/* -------------------------------------------------------------------- */
#define COMPAQ_CSEL_COL     0x0f
#define COMPAQ_CSEL_INT     0x10
#define COMPAQ_CSEL_PAL     0x20
/* -------------------------------------------------------------------- */
/* Status Register:                                                     */
/* -------------------------------------------------------------------- */
/* The status register is a 4-bit read-only register. Its I/O address is*/
/* hex 3DA, and it can be read using the processor 'in' instruction. The*/
/* following are bit definitions for this register.                     */
/*  - Bit 0: A 1 indicates that a regen-buffer memory access can be made*/
/*  without interfering with the display.                               */
/*  - Bit 1: A 1 indicates that a positive-going edge from the light pen*/
/*  has set the light pen's trigger. This trigger is reset when power is*/
/*  turned on and may also be cleared by a processor 'out' command to   */
/*  hex address 3DB. No specific data setting is required; the action is*/
/*  address-activated.                                                  */
/*  - Bit 2: The light pen switch is reflected in this bit. The switch  */
/*  is not latched or debounced. A 0 indicates that the switch is on.   */
/*  - Bit 3: A 1 indicates that the raster is in a vertical retrace     */
/*  mode. Screen-buffer updating can be performed at this time.         */
/* -------------------------------------------------------------------- */
/* ------------------------------------------------------------------- */
/* 6845 CRT Status  Port   (Hex 3BA):                                  */
/* ------------------------------------------------------------------- */
/* - 0  :       - +Horizontal Drive                                    */
/* - 1  :       - Reserved                                             */
/* - 2  :       - Reserved                                             */
/* - 3  :       - +Black/White Video                                   */
/* ------------------------------------------------------------------- */
/* CRTC status register */
/* #define COMPAQ_STATUS_HSYNC 0x01		/* horizontal sync */ /* Disabled MDA registers */
/* #define COMPAQ_STATUS_VIDEO 0x08		/* video signal */    /* Disabled MDA registers */
#define COMPAQ_STATUS_SYNC  0x01		/* -display enable */
#define COMPAQ_STATUS_PEN   0x02
#define COMPAQ_STATUS_VSYNC 0x08		/* vertical sync */

#define COMPAQ_UPDATE_DIRTY   1
#define COMPAQ_UPDATE_RETRACE 2


#include "compaq_font.h"



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
/* -------------------------------------------------------------------- */
/* IBM Color/Graphics Monitor Adapter: Description:                     */
/* -------------------------------------------------------------------- */
/* The IBM Color/Graphics Monitor Adapter is designed to attach to the  */
/* IBM Color Display, to a variety of television-frequency monitors, or */
/* to home television sets (user-supplied RF modulator is required for  */
/* home television sets). This adapter is capable of operating in black-*/
/* and-white or color. It provides three video interfaces: a composite- */
/* video port, a direct-drive port, and a connection interface for      */
/* driving a user-supplied RF modulator. A light pen interface is also  */
/* provided.                                                            */
/*                                                                      */
/* The adapter has two basic modes of operation: alphanumeric (A/N) and */
/* all-points-addressable (APA) graphics. Additional modes are available*/
/* within the A/N or APA graphics modes. In the A/N mode, the display   */
/* be operated in either a 40-column by 25-row mode for a low-resolution*/
/* monitor or home television, or in an 80-column by 25-row mode for    */
/* high-resolution monitors. In both modes, characters are defined in an*/
/* 8-wide by 8-high character box and are 7-wide by 7-high, double      */
/* dotted characters with one descender. Both uppercase and lowercase   */
/* characters are supported in all modes.                               */
/*                                                                      */
/* The character attributes of reverse video, blinking, and highlighting*/
/* are available in the black-and-white mode. In the color mode, 16     */
/* foreground and 8 background colors are available for each character. */
/* In addition, blinking on a per-character basis is available.         */
/*                                                                      */
/* The monitor adapter contains 16K bytes of storage. As an example, a  */
/* 40-column by 25-row display screen uses 1000 bytes to store character*/
/* information and 1000 bytes to store attribute/color information. This*/
/* means that up to eight screens can be stored in the adapter memory.  */
/* Similarly, in an 80-wide by 25-row mode, four display screens can be */
/* stored in the adapter memory. The entire 16K bytes of storage in the */
/* display adapter are directly accessible by the processor, which      */
/* allows maximum program flexibility in managing the screen.           */
/*                                                                      */
/* In A/N color modes, it is also possible to select the color of the   */
/* screen's border. One of 16 colors can be selected.                   */
/*                                                                      */
/* In the APA graphics mode, there are two resolutions available: a     */
/* medium-resolution color graphics mode (320 PELs by 200 rows) and a   */
/* high-resolution black-and-white graphics mode (640 PELs by 200 rows).*/
/* In the medium-resolution mode, each picture element (PEL) may have   */
/* one of four colors. The background color (Color 0) may be any of the */
/* 16 possible colors. The remaining three colors come from one of the  */
/* two program-selectable palettes. One palette contains green/red/brown*/
/* ; the other contains cyan/magenta/white.                             */
/*                                                                      */
/* The high-resolution mode is available only in black-and-white because*/
/* the entire 16K bytes of storage in the adapter is used to define the */
/* on or off state of the PELs.                                         */
/*                                                                      */
/* The adapter operates in noninterlace mode at either 7 or 14 MHz,     */
/* depending on the mode of the operation selected.                     */
/*                                                                      */
/* In the A/N mode, characters are formed from a ROS character generator*/
/* . The character generator contains dot patterns for 256 different    */
/* characters. The character set contains the following major groupings */
/* of characters.                                                       */
/*  - 16 special characters for game support                            */
/*  - 15 characters for word-processing editing support                 */
/*  - 96 characters for the standard ASCII graphics set                 */
/*  - 48 characters for foreign-language support                        */
/*  - 48 characters for business block-graphics support (for the drawing*/
/*  of charts, boxes, and tables using single and double lines)         */
/*  - 16 selected Greek characters                                      */
/*  - 15 selected scientific-notation characters                        */
/* The color/graphics monitor function is on a single adapter. The      */
/* direct-drive and composite-video ports are right-angle mounted       */
/* connectors on the adapter, and extend through the rear panel of the  */
/* system unit. The direct-drive video port is a 9-pin, D-shell, female */
/* connector. The composite-video port is a standard female phono jack. */
/*                                                                      */
/* The display adapter uses a Motorola 6845 CRT Controller device. This */
/* adapter is highly programmable with respect to raster and character  */
/* parameters. Therefore, many additional modes are possible with       */
/* programming of the adapter.                                          */
/*                                                                      */
/* On the following page is a block diagram of the Color/Graphics       */
/* Monitor Adapter.                                                     */
/* -------------------------------------------------------------------- */
/* Color/Graphics Monitor Adapter Block Diagram:                        */
/* -------------------------------------------------------------------- */
/* ----FILE WILL BE INCLUDED IN NEXT RELEASE----                        */
/* -------------------------------------------------------------------- */
/* Specifications:                                                      */
/* -------------------------------------------------------------------- */
/* The following pages contain card and connector specifications for the*/
/* IBM Color/Graphics Monitor Adapter.                                  */
/* -------------------------------------------------------------------- */
/* Color Composite Signal Phono Jack         :       ( O )              */
/*                                                ___________           */
/* Color Direct Drive 9-Pin D-Shell Connector: 5: \O O O O O/ 1:        */
/*                                             9:  \O O O O/  6:        */
/*                                                  -------             */
/* -------------------------------------------------------------------- */
/* Connector Specifications (Part 1 of 2):                              */
/* -------------------------------------------------------------------- */
/* At Standard TTL Levels:                                              */
/*  - Color/Graphics <-: - 1: Ground          : -> IBM Color Display    */
/*  - Direct-Drive   <-: - 2: Ground          : -> or other Direct-Drive*/
/*  - Adapter        <-: - 3: Red             : -> Monitor              */
/*  - Color/Graphics <-: - 4: Green           : -> IBM Color Display    */
/*  - Direct-Drive   <-: - 5: Blue            : -> or other Direct-Drive*/
/*  - Adapter        <-: - 6: Intensity       : -> Monitor              */
/*  - Color/Graphics <-: - 7: Reserved        : -> IBM Color Display    */
/*  - Direct-Drive   <-: - 8: Horizontal Drive: -> or other Direct-Drive*/
/*  - Adapter        <-: - 9: Vertical   Drive: -> Monitor              */
/* Composite Phono Jack Hookup to Monitor:                              */
/*       /^\                                                            */
/*      /   \                                                           */
/*     / / \ \ Composite Video Signal of Approximately 1.5 Volts:       */
/*  - |   1   | - Video: <-: Peak to Peak Amplitude: 1:->Color/Graphics */
/*     \ \ / /  Monitor: <-: Chassis Ground        : 2:->Composite Jack */
/*      \ 2 /                                                           */
/*       \v/                                                            */
/* -------------------------------------------------------------------- */
/* Connector Specifications (Part 2 of 2):                              */
/* -------------------------------------------------------------------- */
/* Color/Graphics Monitor Adapter:                       | |   |        */
/*  - P1 (4-Pin Berg Strip) for RF Modulator       :     4 3 2 1        */
/*                                                   | | | |   |        */
/*  - P2 (6-Pin Berg Strip) for Light-Pen Connector: 6 5 4 3 2 1        */
/* RF Modulator Interface        :                                      */
/*  - Color/Graphics : <-: - 1: +12 Volts             : -> RF Modulator */
/*  - Monitor Adapter: <-: - 2: (key) Not Used        : -> RF Modulator */
/*  - Color/Graphics : <-: - 3: Composite Video Output: -> RF Modulator */
/*  - Monitor Adapter: <-: - 4: Logic Ground          : -> RF Modulator */
/* Light Pen    Interface        :                                      */
/*  - Color/Graphics : <-: - 1: -Light Pen Input      : -> Light Pen    */
/*  - Monitor Adapter: <-: - 2: (key) Not Used        : -> Light Pen    */
/*  - Color/Graphics : <-: - 3: -Light Pen Switch     : -> Light Pen    */
/*  - Monitor Adapter: <-: - 4: Chassis Ground        : -> Light Pen    */
/*  - Color/Graphics : <-: - 5: + 5 Volts             : -> Light Pen    */
/*  - Monitor Adapter: <-: - 6: +12 Volts             : -> Light Pen    */
/* -------------------------------------------------------------------- */
/* Logic Diagrams:                                                      */
/* -------------------------------------------------------------------- */
/* The following pages contain the logic diagrams for the IBM Color/    */
/* Graphics Monitor Adapter.                                            */
/*  - Color/Graphics Monitor Adapter (Sheet 1 of 6)                     */
/*  - Color/Graphics Monitor Adapter (Sheet 2 of 6)                     */
/*  - Color/Graphics Monitor Adapter (Sheet 3 of 6)                     */
/*  - Color/Graphics Monitor Adapter (Sheet 4 of 6)                     */
/*  - Color/Graphics Monitor Adapter (Sheet 5 of 6)                     */
/*  - Color/Graphics Monitor Adapter (Sheet 6 of 6)                     */
/* NOTE: Logic diagram scans will be included in an upcoming release.   */
/* -------------------------------------------------------------------- */
static void compaq_clock (compaq_t *compaq, unsigned long cnt);
/* -------------------------------------------------------------------- */
/* Timing Generator:                                                    */
/* -------------------------------------------------------------------- */
/* This generator produces the timing signals used by the 6845 CRT      */
/* Controller and by the dynamic memory. It also solves the processor/  */
/* graphic controller contentions for access to the display buffer.     */
/* -------------------------------------------------------------------- */


unsigned char compaq_rgb[16][3] = {
	/* -------------------------------------------------------------------- */
	/* Alphanumeric Mode:                                                   */
	/* -------------------------------------------------------------------- */
	/* Every display character position in the alphanumeric mode is defined */
	/* by two bytes in regen buffer (a part of the monitor adapter), not the*/
	/* system memory. Both the Color/Graphics Monitor Adapter and the       */
	/* the Monochrome Display and Printer Adapter use the following 2-byte  */
	/* character-attribute format.                                          */
	/*                                                                      */
	/* Display-Character Code Byte:     Attribute Byte:                     */
	/*  - 7 - 6 - 5 - 4 - 3 - 2 - 1 - 0  - 7 - 6 - 5 - 4 - 3 - 2 - 1 - 0    */
	/*                                                                      */
	/* The following table shows the functions of the attribute byte.       */
	/* -------------------------------------------------------------------- */
	/* Attribute Function/Attribute Byte:                                   */
	/* -------------------------------------------------------------------- */
	/* I = Highlighted Foreground (Character):                              */
	/* B = Blinking    Foreground (Character):                              */
	/* Normal             (Background): Normal             (Foreground):    */
	/*  - 7 : 6 5 4                      - 3 : 2 1 0                        */
	/*  - B : R G B                      - I : R G B                        */
	/*  - FG: Background                 - Foreground:                      */
	/*  - B : 0 0 0                      - I : 1 1 1                        */
	/* Reverse Video      (Background): Reverse Video      (Foreground):    */
	/*  - 7 : 6 5 4                      - 3 : 2 1 0                        */
	/*  - B : R G B                      - I : R G B                        */
	/*  - FG: Background                 - Foreground:                      */
	/*  - B : 1 1 1                      - I : 0 0 0                        */
	/* Nondisplay (Black) (Background): Nondisplay (Black) (Foreground):    */
	/*  - 7 : 6 5 4                      - 3 : 2 1 0                        */
	/*  - B : R G B                      - I : R G B                        */
	/*  - FG: Background                 - Foreground:                      */
	/*  - B : 0 0 0                      - I : 0 0 0                        */
	/* Nondisplay (White) (Background): Nondisplay (White) (Foreground):    */
	/*  - 7 : 6 5 4                      - 3 : 2 1 0                        */
	/*  - B : R G B                      - I : R G B                        */
	/*  - FG: Background                 - Foreground:                      */
	/*  - B : 1 1 1                      - I : 1 1 1                        */
	/*                                                                      */
	/* The definitions of the attribute byte are in the following table.    */
	/*  - 7: B: Blinking                                                    */
	/*  - 6: R:                                                             */
	/*  - 5: G: Background Color                                            */
	/*  - 4: B:                                                             */
	/*  - 3: I: Intensity                                                   */
	/*  - 2: R:                                                             */
	/*  - 1: G: Foreground                                                  */
	/*  - 0: B:                                                             */
	/* In the alphanumeric mode, the display can be operated in either a low*/
	/* -resolution mode or a high-resolution mode.                          */
	/*                                                                      */
	/* The low-resolution alphanumeric mode has the following features:     */
	/*  - Supports home color televisions or low-resolution monitors.       */
	/*  - Displays up to 25 rows of 40 characters each.                     */
	/*  - Has a ROS character generator that contains dot patterns for a    */
	/*  maximum of 256 different characters.                                */
	/*  - Requires 2,000 bytes of read/write memory (on the adapter).       */
	/*  - Has an 8-high by 8-wide character box.                            */
	/*  - Has two jumper-controlled character fonts available: a 7-high by 5*/
	/*  -wide single-dot character font with one descender, and a 7-high by */
	/*  7-wide double-dotted character font with one descender.             */
	/*  - Has one character attribute for each character.                   */
	/* The high-resolution alphanumeric mode has the following features:    */
	/*  - Supports the IBM Color Display or other color monitor with direct-*/
	/*  drive capability.                                                   */
	/*  - Supports a black-and-white composite-video monitor.               */
	/*  - Displays up to 25 rows of 80 characters each.                     */
	/*  - Has a ROS display generator that contains dot patterns for a      */
	/*  maximum of 256 characters.                                          */
	/*  - Requires 4,000 bytes of read/write memory (on the adapter).       */
	/*  - Has an 8-high by 8-wide character box.                            */
	/*  - Has two jumper-controlled character fonts available: a 7-high by 5*/
	/*  -wide single-dot character font with one descender, and a 7-high by */
	/*  7-wide double-dot character font with one descender.                */
	/*  - Has one character attribute for each character.                   */
	/* The Color/Graphics Monitor Adapter will change foreground and        */
	/* background colors according to the color value selected in the       */
	/* attribute byte. The following figure shows the color values for the  */
	/* various red, green, blue, and intensity bit settings.                */
	/*  - Black                 : Red: 0 Green: 0 Blue: 0 Intensity: 0      */
	/*  - Blue                  : Red: 0 Green: 0 Blue: 1 Intensity: 0      */
	/*  - Green                 : Red: 0 Green: 1 Blue: 0 Intensity: 0      */
	/*  - Cyan                  : Red: 0 Green: 1 Blue: 1 Intensity: 0      */
	/*  - Red                   : Red: 1 Green: 0 Blue: 0 Intensity: 0      */
	/*  - Magenta               : Red: 1 Green: 0 Blue: 1 Intensity: 0      */
	/*  - Brown                 : Red: 1 Green: 1 Blue: 0 Intensity: 0      */
	/*  - White                 : Red: 1 Green: 1 Blue: 1 Intensity: 0      */
	/*  - Gray                  : Red: 0 Green: 0 Blue: 0 Intensity: 1      */
	/*  - Light Blue            : Red: 0 Green: 0 Blue: 1 Intensity: 1      */
	/*  - Light Green           : Red: 0 Green: 1 Blue: 0 Intensity: 1      */
	/*  - Light Cyan            : Red: 0 Green: 1 Blue: 1 Intensity: 1      */
	/*  - Light Red             : Red: 1 Green: 0 Blue: 0 Intensity: 1      */
	/*  - Light Magenta         : Red: 1 Green: 0 Blue: 1 Intensity: 1      */
	/*  - Yellow                : Red: 1 Green: 1 Blue: 0 Intensity: 1      */
	/*  - White (High Intensity): Red: 1 Green: 1 Blue: 1 Intensity: 1      */
	/* Note: Not all monitors recognize the intensity (I) bit.              */
	/* -------------------------------------------------------------------- */
	{ 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0xaa },
	{ 0x00, 0xaa, 0x00 },
	{ 0x00, 0xaa, 0xaa },
	{ 0xaa, 0x00, 0x00 },
	{ 0xaa, 0x00, 0xaa },
	{ 0xaa, 0x55, 0x00 },
	{ 0xaa, 0xaa, 0xaa },
	{ 0x55, 0x55, 0x55 },
	{ 0x55, 0x55, 0xff },
	{ 0x55, 0xff, 0x55 },
	{ 0x55, 0xff, 0xff },
	{ 0xff, 0x55, 0x55 },
	{ 0xff, 0x55, 0xff },
	{ 0xff, 0xff, 0x55 },
	{ 0xff, 0xff, 0xff }
};


/*
 * Set the blink frequency
 */
void compaq_set_blink_rate (compaq_t *compaq, unsigned freq)
{
	compaq->blink_on = 1;
	compaq->blink_cnt = freq;
	compaq->blink_freq = freq;

	compaq->update_state |= COMPAQ_UPDATE_DIRTY;
}

static
void COMPAQ_set_color (COMPAQ_t *COMPAQ, unsigned i1, unsigned i2, unsigned r, unsigned g, unsigned b)
{
	unsigned i;

	if ((i1 > 16) || (i2 > 16)) {
		return;
	}
	r &= 0xff;
	g &= 0xff;
	b &= 0xff;
	for (i = i1; i <= i2; i++) {
		COMPAQ->rgb[i][0] = r;
		COMPAQ->rgb[i][1] = g;
		COMPAQ->rgb[i][2] = b;
	}
}

/*
 * Map a color name to background/normal/bright RGB values
 */
static
void COMPAQ_get_color (const char *name,
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
void COMPAQ_map_attribute (COMPAQ_t *COMPAQ, unsigned attr, const unsigned char **fg, const unsigned char **bg)
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
		*fg = COMPAQ->rgb[0];
		*bg = COMPAQ->rgb[7];

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
			if ((COMPAQ->reg[COMPAQ_MODE] & COMPAQ_MODE_BLINK) == 0) {
				*bg = COMPAQ->rgb[15];
			}
		}
	}
	else {
		*fg = COMPAQ->rgb[attr & 0x0f];
		*bg = COMPAQ->rgb[0];
	}
}

/*
 * Get CRTC start offset
 */
unsigned compaq_get_start (compaq_t *compaq)
{
	unsigned val;

	/* val = COMPAQ->reg_crt[0x0c]; */              /* Disabled MDA register code */
	/* val = (val << 8) | COMPAQ->reg_crt[0x0d]; */ /* Disabled MDA register code */
	val = compaq->reg_crt[COMPAQ_CRTC_SH];              /*  C/R12: Start             Address (H) */
	val = (val << 8) | compaq->reg_crt[COMPAQ_CRTC_SL]; /*  D/R13: Start             Address (L) */

	return (val);
}

/*
 * Get the absolute cursor position
 */
unsigned compaq_get_cursor (compaq_t *compaq)
{
	unsigned val;

	/* val = COMPAQMDAMODE5140->reg_crt[0x0e]; */              /* Disabled MDA register code */
	/* val = (val << 8) | COMPAQMDAMODE5140->reg_crt[0x0f]; */ /* Disabled MDA register code */
	val = compaq->reg_crt[COMPAQ_CRTC_PH];              /*  E/R14: Cursor            Address (H) */
	val = (val << 8) | compaq->reg_crt[COMPAQ_CRTC_PL]; /*  F/R15: Cursor            Address (L) */

	return (val);
}

/*
 * Get the on screen cursor position
 */
int compaq_get_position (compaq_t *compaq, unsigned *x, unsigned *y)
{
	unsigned pos, ofs;
    /* ------------------------------------------------------------------------------------------- */
    /* Quote from John Elliott, Vintage PC pages, Compaq Portable/Deskpro, Video Output:           */
    /* ------------------------------------------------------------------------------------------- */
    /* The method used by the card to detect what resolution is very simple: if the CRTC character */
    /* height register is 13 (ie, characters are 14 rows) then it goes into the high resolution    */
    /* mode. If not, it behaves like a normal CGA.                                                 */
    /* ------------------------------------------------------------------------------------------- */
	if (compaq->hires_mono_font = 13);
	{
		/* IBM MDA resolution */
		pos = (COMPAQ_get_cursor (COMPAQ) - COMPAQ_get_start (COMPAQ)) & 0x0fff;
		if (compaq->w == 0) {
			return (1);
		}
	}
	else
	{
		/* IBM CGA resolution */
		if ((COMPAQ->w == 0) || (COMPAQ->h == 0)) {
			return (1);
		}
	}
	pos = compaq_get_cursor (compaq);
	ofs = compaq_get_start (compaq);

	pos = (pos - ofs) & 0x3fff;
	if (pos >= (compaq->w * compaq->h)) {
		return (1);
	}
	*x = pos % compaq->w;
	*y = pos / compaq->w;
	return (0);
}

/*
 * Set the internal palette from the mode and color select registers
 */
static
void compaq_set_palette (compaq_t *compaq)
{
	unsigned char mode, csel;
	/* -------------------------------------------------------------------- */
	/* Alphanumeric Mode:                                                   */
	/* -------------------------------------------------------------------- */
	/* Every display character position in the alphanumeric mode is defined */
	/* by two bytes in regen buffer (a part of the monitor adapter), not the*/
	/* system memory. Both the Color/Graphics Monitor Adapter and the       */
	/* the Monochrome Display and Printer Adapter use the following 2-byte  */
	/* character-attribute format.                                          */
	/*                                                                      */
	/* Display-Character Code Byte:     Attribute Byte:                     */
	/*  - 7 - 6 - 5 - 4 - 3 - 2 - 1 - 0  - 7 - 6 - 5 - 4 - 3 - 2 - 1 - 0    */
	/*                                                                      */
	/* The following table shows the functions of the attribute byte.       */
	/* -------------------------------------------------------------------- */
	/* Attribute Function/Attribute Byte:                                   */
	/* -------------------------------------------------------------------- */
	/* I = Highlighted Foreground (Character):                              */
	/* B = Blinking    Foreground (Character):                              */
	/* Normal             (Background): Normal             (Foreground):    */
	/*  - 7 : 6 5 4                      - 3 : 2 1 0                        */
	/*  - B : R G B                      - I : R G B                        */
	/*  - FG: Background                 - Foreground:                      */
	/*  - B : 0 0 0                      - I : 1 1 1                        */
	/* Reverse Video      (Background): Reverse Video      (Foreground):    */
	/*  - 7 : 6 5 4                      - 3 : 2 1 0                        */
	/*  - B : R G B                      - I : R G B                        */
	/*  - FG: Background                 - Foreground:                      */
	/*  - B : 1 1 1                      - I : 0 0 0                        */
	/* Nondisplay (Black) (Background): Nondisplay (Black) (Foreground):    */
	/*  - 7 : 6 5 4                      - 3 : 2 1 0                        */
	/*  - B : R G B                      - I : R G B                        */
	/*  - FG: Background                 - Foreground:                      */
	/*  - B : 0 0 0                      - I : 0 0 0                        */
	/* Nondisplay (White) (Background): Nondisplay (White) (Foreground):    */
	/*  - 7 : 6 5 4                      - 3 : 2 1 0                        */
	/*  - B : R G B                      - I : R G B                        */
	/*  - FG: Background                 - Foreground:                      */
	/*  - B : 1 1 1                      - I : 1 1 1                        */
	/*                                                                      */
	/* The definitions of the attribute byte are in the following table.    */
	/*  - 7: B: Blinking                                                    */
	/*  - 6: R:                                                             */
	/*  - 5: G: Background Color                                            */
	/*  - 4: B:                                                             */
	/*  - 3: I: Intensity                                                   */
	/*  - 2: R:                                                             */
	/*  - 1: G: Foreground                                                  */
	/*  - 0: B:                                                             */
	/* In the alphanumeric mode, the display can be operated in either a low*/
	/* -resolution mode or a high-resolution mode.                          */
	/*                                                                      */
	/* The low-resolution alphanumeric mode has the following features:     */
	/*  - Supports home color televisions or low-resolution monitors.       */
	/*  - Displays up to 25 rows of 40 characters each.                     */
	/*  - Has a ROS character generator that contains dot patterns for a    */
	/*  maximum of 256 different characters.                                */
	/*  - Requires 2,000 bytes of read/write memory (on the adapter).       */
	/*  - Has an 8-high by 8-wide character box.                            */
	/*  - Has two jumper-controlled character fonts available: a 7-high by 5*/
	/*  -wide single-dot character font with one descender, and a 7-high by */
	/*  7-wide double-dotted character font with one descender.             */
	/*  - Has one character attribute for each character.                   */
	/* The high-resolution alphanumeric mode has the following features:    */
	/*  - Supports the IBM Color Display or other color monitor with direct-*/
	/*  drive capability.                                                   */
	/*  - Supports a black-and-white composite-video monitor.               */
	/*  - Displays up to 25 rows of 80 characters each.                     */
	/*  - Has a ROS display generator that contains dot patterns for a      */
	/*  maximum of 256 characters.                                          */
	/*  - Requires 4,000 bytes of read/write memory (on the adapter).       */
	/*  - Has an 8-high by 8-wide character box.                            */
	/*  - Has two jumper-controlled character fonts available: a 7-high by 5*/
	/*  -wide single-dot character font with one descender, and a 7-high by */
	/*  7-wide double-dot character font with one descender.                */
	/*  - Has one character attribute for each character.                   */
	/* The Color/Graphics Monitor Adapter will change foreground and        */
	/* background colors according to the color value selected in the       */
	/* attribute byte. The following figure shows the color values for the  */
	/* various red, green, blue, and intensity bit settings.                */
	/*  - Black                 : Red: 0 Green: 0 Blue: 0 Intensity: 0      */
	/*  - Blue                  : Red: 0 Green: 0 Blue: 1 Intensity: 0      */
	/*  - Green                 : Red: 0 Green: 1 Blue: 0 Intensity: 0      */
	/*  - Cyan                  : Red: 0 Green: 1 Blue: 1 Intensity: 0      */
	/*  - Red                   : Red: 1 Green: 0 Blue: 0 Intensity: 0      */
	/*  - Magenta               : Red: 1 Green: 0 Blue: 1 Intensity: 0      */
	/*  - Brown                 : Red: 1 Green: 1 Blue: 0 Intensity: 0      */
	/*  - White                 : Red: 1 Green: 1 Blue: 1 Intensity: 0      */
	/*  - Gray                  : Red: 0 Green: 0 Blue: 0 Intensity: 1      */
	/*  - Light Blue            : Red: 0 Green: 0 Blue: 1 Intensity: 1      */
	/*  - Light Green           : Red: 0 Green: 1 Blue: 0 Intensity: 1      */
	/*  - Light Cyan            : Red: 0 Green: 1 Blue: 1 Intensity: 1      */
	/*  - Light Red             : Red: 1 Green: 0 Blue: 0 Intensity: 1      */
	/*  - Light Magenta         : Red: 1 Green: 0 Blue: 1 Intensity: 1      */
	/*  - Yellow                : Red: 1 Green: 1 Blue: 0 Intensity: 1      */
	/*  - White (High Intensity): Red: 1 Green: 1 Blue: 1 Intensity: 1      */
	/* Note: Not all monitors recognize the intensity (I) bit.              */
	/* -------------------------------------------------------------------- */
	/* Mode-Control Register:                                               */
	/* -------------------------------------------------------------------- */
	/* The mode-control register is a 6-bit output-only register. Its I/O   */
	/* address is hex 3D8, and it can be written to using a processor 'out' */
	/* command. The following are bit definitions for this register.        */
	/*  - Bit 0: A 1 selects  80 by 25 alphanumeric mode.                   */
	/*           A 0 selects  40 by 25 alphanumeric mode.                   */
	/*  - Bit 1: A 1 selects 320 by 200 graphics    mode.                   */
	/*           A 0 selects           alphanumeric mode.                   */
	/*  - Bit 2: A 1 selects black-and-white        mode.                   */
	/*           A 0 selects color                  mode.                   */
	/*  - Bit 3: A 1 enables the video signal. The video signal is disabled */
	/*           when changing modes.                                       */
	/*  - Bit 4: A 1 selects the high-resolution (640 by 200) black-and-    */
	/*           white graphics mode. One of eight colors can be selected on*/
	/*           direct-drive monitors in this mode by using register hex   */
	/*           3D9.                                                       */
	/*  - Bit 5: A 1 will change the character background intensity to the  */
	/*           blinking attribute function for alphanumeric modes. When   */
	/*           the high-order attribute is not selected, 16 background    */
	/*           colors or intensified colors are available. This bit is set*/
	/*           to 1 to allow the blinking function.                       */
	/* -------------------------------------------------------------------- */
	/* Mode-Control Register Summary:                                       */
	/* -------------------------------------------------------------------- */
	/* Bits:                                                                */
	/* 0   :1   :2   :3   :4   :5   : 40 x 25 Alphanumeric Black-and-White  */
	/*  - 0: - 0: - 1: - 1: - 0: - 1: 40 x 25 Alphanumeric Color            */
	/*  - 0: - 0: - 0: - 1: - 0: - 1: 80 x 25 Alphanumeric Black-and-White  */
	/*  - 1: - 0: - 1: - 1: - 0: - 1: 80 x 25 Alphanumeric Color            */
	/*  - 1: - 0: - 0: - 1: - 0: - 1: 320 x 200 Black-and-White Graphics    */
	/*  - 0: - 1: - 1: - 1: - 0: - z: 320 x 200 Color Graphics              */
	/*  - 0: - 1: - 0: - 1: - 0: - z: 640 x 200 Black-and-White Graphics    */
	/* -------------------------------------------------------------------- */
	/*  - 5: Enable Blink Attribute                                         */
	/*  - 4: 640 x 200 Black-and-White                                      */
	/*  - 3: Enable Video Signal                                            */
	/*  - 2: Select Black-and-White Mode                                    */
	/*  - 1: Select 320 x 200 Graphics                                      */
	/*  - 0: 80 x 25 Alphanumeric Select                                    */
	/* z = don't care condition                                             */
	/* Note: The low-resolution (160 by 100) mode requires special          */
	/* programming and is set up as the 40 by 25 alphanumeric mode.         */
	/* -------------------------------------------------------------------- */
	mode = compaq->reg[COMPAQ_MODE];
	csel = compaq->reg[COMPAQ_CSEL];
	/* -------------------------------------------------------------------- */
	/* Color-Select Register:                                               */
	/* -------------------------------------------------------------------- */
	/* The color-select register is a 6-bit output-only register. Its I/O   */
	/* address is hex 3D9, and it can be written to using a processor 'out' */
	/* command. The following are the bit definitions for this register.    */
	/*  - Bit 0: Selects blue border color in 40 by 25 alphanumeric mode.   */
	/*  Selects blue background color (C0-C1) in 320 by 200 graphics mode.  */
	/*  Selects blue foreground color in 640 by 200 graphics mode.          */
	/*  - Bit 1: Selects green border color in 40 by 25 alphanumeric mode.  */
	/*  Selects green background color (C0-C1) in 320 by 200 graphics mode. */
	/*  Selects green foreground color in 640 by 200 graphics mode.         */
	/*  - Bit 2: Selects red border color in 40 by 25 alphanumeric mode.    */
	/*  Selects red background color (C0-C1) in 320 by 200 graphics mode.   */
	/*  Selects red foreground color in 640 by 200 graphics mode.           */
	/*  - Bit 3: Selects intensified border color in 40 by 25 alphanumeric  */
	/*  mode. Selects intensified background color (C0-C1) in 320 by 200    */
	/*  graphics mode. Selects intensified foreground color in 640 by 200   */
	/*  graphics mode.                                                      */
	/*  - Bit 4:Selects alternate, intensified set of colors in the graphics*/
	/*  mode. Selects background colors in the alphanumeric mode.           */
	/*  - Bit 5: Selects active color set in 320 by 200 graphics mode.      */
	/*  When bit 5 is set to 1, colors are determined as follows:           */
	/*  C1:  C0:  Set Selected:                                             */
	/*   - 0: - 0: - Background (Defined by bits 0-3 of port hex 3D9)       */
	/*   - 0: - 1: - Cyan                                                   */
	/*   - 1: - 0: - Magenta                                                */
	/*   - 1: - 1: - White                                                  */
	/*  When bit 5 is set to 0, colors are determined as follows:           */
	/*  C1:  C0:  Set Selected:                                             */
	/*   - 0: - 0: - Background (Defined by bits 0-3 of port hex 3D9)       */
	/*   - 0: - 1: - Green                                                  */
	/*   - 1: - 0: - Red                                                    */
	/*   - 1: - 1: - Brown                                                  */
	/*  - Bit 6: Not used                                                   */
	/*  - Bit 7: Not used                                                   */
	/* -------------------------------------------------------------------- */
	compaq->pal[0] = csel & 0x0f;

	if (mode & COMPAQ_MODE_CBURST) {
		compaq->hires_mono_font = 8;
		compaq->pal[1] = 3;
		compaq->pal[2] = 4;
		compaq->pal[3] = 7;
	}
	else if (csel & COMPAQ_CSEL_PAL) {
		compaq->hires_mono_font = 8;
		compaq->pal[1] = 3;
		compaq->pal[2] = 5;
		compaq->pal[3] = 7;
	}
	else {
		compaq->hires_mono_font = 8;
		compaq->pal[1] = 2;
		compaq->pal[2] = 4;
		compaq->pal[3] = 6;
	}
	if (csel & COMPAQ_CSEL_INT) {
		compaq->hires_mono_font = 8;
		compaq->pal[1] += 8;
		compaq->pal[2] += 8;
		compaq->pal[3] += 8;
	}
	/* -------------------------------------------------------------------- */
	/* Medium-Resolution Color/Graphics Mode:                               */
	/* -------------------------------------------------------------------- */
	/* The medium-resolution mode supports home televisions or color        */
	/* monitors. It has the following features:                             */
	/*  - Contains a maximum of 320 PELs by 200 rows, with each PEL being 1-*/
	/*  high by 1-wide.                                                     */
	/*  - Preselects 1 of 4 colors for each PEL.                            */
	/*  - Requires 16,000 bytes of read/write memory on the adapter.        */
	/*  - Uses memory-mapped graphics.                                      */
	/*  - Formats 4 PELs per byte in the following manner:                  */
	/*  - - 7 C1/6 C0: First  Display PEL                                   */
	/*  - - 5 C1/4 C0: Second Display PEL                                   */
	/*  - - 3 C1/2 C0: Third  Display PEL                                   */
	/*  - - 1 C1/0 C0: Fourth Display PEL                                   */
	/*  - Organizes graphics storage into two banks of 8,000 bytes, using   */
	/*  the following format:                                               */
	/*  Memory Address (in hex): Function:                                  */
	/*   - B8000                  - Even Scans (0,2,4,...198) 8,000 bytes   */
	/*   - B9F3F                  - Not Used                                */
	/*   - BA000                  - Odd  Scans (1,3,5,...199) 8,000 bytes   */
	/*   - B8FFF                  - Not Used                                */
	/*  Address hex B8000 contains the PEL instruction for the upper-left   */
	/*  corner of the display area.                                         */
	/* -------------------------------------------------------------------- */
	/* Color selection is determined by the following logic:                */
	/* -------------------------------------------------------------------- */
	/* C1: C0 : Function:                                                   */
	/*  - C1: 0/C0: 0: Dot takes on the color of 1 to 16 preselected        */
	/*  background colors.                                                  */
	/*  - C1: 0/C0: 1: Selects first  color of preselected Color Set 1 or   */
	/*  Color Set 2.                                                        */
	/*  - C1: 1/C0: 0: Selects second color of preselected Color Set 1 or   */
	/*  Color Set 2.                                                        */
	/*  - C1: 1/C0: 1: Selects third  color of preselected Color Set 1 or   */
	/*  Color Set 2.                                                        */
	/* C1 and C0 select 4 of 16 preselected colors. This color selection    */
	/* (palette) is preloaded in an I/O port.                               */
	/*                                                                      */
	/* The two color sets are: Color Set 1/Color Set 2:                     */
	/*  - Color 1 is Green/Color 1 is Cyan                                  */
	/*  - Color 2 is Red  /Color 2 is Magenta                               */
	/*  - Color 3 is Brown/Color 3 is White                                 */
	/* The background colors are the same basic 8 colors defined for low-   */
	/* resolution graphics, plus 8 alternate intensities defined by the     */
	/* intensity bit, for a total of 16 colors, including black and white.  */
	/* -------------------------------------------------------------------- */
	/* High-Resolution Black-and-White Graphics Mode:                       */
	/* -------------------------------------------------------------------- */
	/* The high-resolution mode supports color monitors. This mode has the  */
	/* following features:                                                  */
	/*  - Contains a maximum of 640 PELs by 200 rows, with each PEL being 1-*/
	/*  high by 1-wide.                                                     */
	/*  - Supports black-and-white only.                                    */
	/*  - Requires 16,000 bytes of read/write memory on the adapter.        */
	/*  - Addressing and mapping procedures are the same as medium-         */
	/*  resolution color/graphics, but the data format is different. In this*/
	/*  mode, each bit is mapped to a PEL on the screen.                    */
	/*  - Formats 8 PELs per byte in the following manner:                  */
	/*  - - 7: First   Display PEL                                          */
	/*  - - 6: Second  Display PEL                                          */
	/*  - - 5: Third   Display PEL                                          */
	/*  - - 4: Fourth  Display PEL                                          */
	/*  - - 3: Fifth   Display PEL                                          */
	/*  - - 2: Sixth   Display PEL                                          */
	/*  - - 1: Seventh Display PEL                                          */
	/*  - - 0: Eighth  Display PEL                                          */
	/* -------------------------------------------------------------------- */
}

/*
 * Set the timing values from the CRTC registers
 */
static
void compaq_set_timing (compaq_t *compaq)
{
	/* -------------------------------------------------------------------- */
	/* Timing Generator:                                                    */
	/* -------------------------------------------------------------------- */
	/* This generator produces the timing signals used by the 6845 CRT      */
	/* Controller and by the dynamic memory. It also solves the processor/  */
	/* graphic controller contentions for access to the display buffer.     */
	/* -------------------------------------------------------------------- */
	compaq->ch = (compaq->reg_crt[COMPAQ_CRTC_MS] & 0x1f) + 1; /* 9/R 9: Maximum Scan Line Address:  D */
	compaq->w = compaq->reg_crt[COMPAQ_CRTC_HD]; /*  1/R 1: Horizontal Displayed                  : 50 */
	compaq->h = compaq->reg_crt[COMPAQ_CRTC_VD]; /*  6/R 6: Vertical   Displayed     Row          : 19 */

	compaq->clk_ht = 8 * (compaq->reg_crt[COMPAQ_CRTC_HT] + 1); /*  0/R 0: Horizontal Total       : 61 */
	compaq->clk_hd = 8 * compaq->reg_crt[COMPAQ_CRTC_HD];       /*  1/R 1: Horizontal Displayed   : 50 */

	compaq->clk_vt = compaq->ch * (compaq->reg_crt[COMPAQ_CRTC_VT] + 7) * compaq->clk_ht; /*  4/R 4: Vertical   Total         Row: 19 */
	compaq->clk_vd = compaq->ch * compaq->reg_crt[COMPAQ_CRTC_VD] * compaq->clk_ht;       /*  6/R 6: Vertical   Displayed     Row  */
}

/*
 * Get the dot clock
 */
static
unsigned long compaq_get_dotclock (compaq_t *compaq)
{
	unsigned long long clk;
	/* -------------------------------------------------------------------- */
	/* Timing Generator:                                                    */
	/* -------------------------------------------------------------------- */
	/* This generator produces the timing signals used by the 6845 CRT      */
	/* Controller and by the dynamic memory. It also solves the processor/  */
	/* graphic controller contentions for access to the display buffer.     */
	/* -------------------------------------------------------------------- */
	clk = compaq->video.dotclk[0];
    /* ------------------------------------------------------------------------------------------- */
    /* Quote from John Elliott, Vintage PC pages, Compaq Portable/Deskpro, Video Output:           */
    /* ------------------------------------------------------------------------------------------- */
    /* The method used by the card to detect what resolution is very simple: if the CRTC character */
    /* height register is 13 (ie, characters are 14 rows) then it goes into the high resolution    */
    /* mode. If not, it behaves like a normal CGA.                                                 */
    /* ------------------------------------------------------------------------------------------- */
	if (compaq->hires_mono_font = 13);
	{
		/* IBM MDA resolution */
		clk = (COMPAQ_MDA_PFREQ * clk) / COMPAQ_IFREQ;
	}
	else
	{
		/* IBM CGA resolution */
		clk = (COMPAQ_CGA_PFREQ * clk) / COMPAQ_IFREQ;
	}
	if ((compaq->reg[COMPAQ_MODE] & COMPAQ_MODE_CS) == 0) {
		clk >>= 1;
	}
	return (clk);
}

/*
 * Set the internal screen buffer size
 */
int compaq_set_buf_size (compaq_t *compaq, unsigned w, unsigned h)
{
	unsigned long cnt;
	unsigned char *tmp;
	/* -------------------------------------------------------------------- */
	/* Display Buffer:                                                      */
	/* -------------------------------------------------------------------- */
	/* The display buffer resides in the processor-address space, starting  */
	/* at address hex B8000. It provides 16 bytes of dynamic read/write     */
	/* memory. A dual-ported implementation allows the processor and the    */
	/* graphics control unit access to this buffer. The processor and the   */
	/* control unit have equal access to this buffer during all modes of    */
	/* operation, except in the high-resolution alphanumeric mode. In this  */
	/* mode, only the processor should have access to this buffer during the*/
	/* horizontal-retrace intervals. While the processor may write to the   */
	/* required buffer at any time, a small amount of display interference  */
	/* will result if this does not occur during the horizontal-retrace     */
	/* intervals.                                                           */
	/* -------------------------------------------------------------------- */
	/* Basic Operations:                                                    */
	/* -------------------------------------------------------------------- */
	/* In the alphanumeric mode, the adapter fetches character and attribute*/
	/* information from its display buffer. The starting address of the     */
	/* display buffer is programmable through the CRT controller, but it    */
	/* must be an even address. The character codes and attributes are then */
	/* displayed according to their relative positions in the buffer. The   */
	/* The following addresses will produce an "AB" in the upper-left corner*/
	/* of a 40 by 25 screen and an "X" in the lower-right corner.           */
	/* -------------------------------------------------------------------- */
	/* (Even)    Memory   Display Buffer: (Example of a 40 by 25 Screen):   */
	/* Starting  Address                    - AB                            */
	/* Address: (in hex):                  Video Screen:                    */
	/*  - B8000:           - Character Code A                               */
	/*  - B8001:           - Attribute      A                               */
	/*  - B8002:           - Character Code B                               */
	/*  - B8003:           - Attribute      B                               */
	/* Last Address:                                                        */
	/*  - B87CE:           - Character Code X                               */
	/*  - B87CF:           - Attribute      X                               */
	/* -------------------------------------------------------------------- */
	/* The processor and the display control unit have equal access to the  */
	/* display buffer during all of the operating modes, except the high-   */
	/* resolution alphanumeric mode. During this mode, the processor gains  */
	/* access to the display buffer during the vertical retrace time. If it */
	/* does not, the display will be affected with random patterns as the   */
	/* microprocessor is using the display buffer. In the alphanumeric mode,*/
	/* the characters are displayed from a pre-stored ROM character         */
	/* generator that contains the dot patterns for all of the displayable  */
	/* characters.                                                          */
	/*                                                                      */
	/* In the graphics mode, the displayed dots and colors, up to 16K bytes,*/
	/* are fetched from the display buffer.                                 */
	/* -------------------------------------------------------------------- */
	cnt = 3UL * (unsigned long) w * (unsigned long) h;
	if (cnt > compaq->bufmax) {
		tmp = realloc (compaq->buf, cnt);
		if (tmp == NULL) {
			return (1);
		}

		compaq->buf = tmp;
		compaq->bufmax = cnt;
	}
	compaq->buf_w = w;
	compaq->buf_h = h;
	return (0);
}

/*
 * Draw the cursor in the internal buffer
 */
static
void compaq_mode0_update_cursor (compaq_t *compaq)
{
	unsigned            i, j;
	unsigned            x, y;
	unsigned            c1, c2;
	unsigned            addr; /* MDA address only */
	const unsigned char *src; /* MDA address only */
	const unsigned char *col;
	unsigned char       *dst;
	/* -------------------------------------------------------------------- */
	/* Basic Operations:                                                    */
	/* -------------------------------------------------------------------- */
	/* In the alphanumeric mode, the adapter fetches character and attribute*/
	/* information from its display buffer. The starting address of the     */
	/* display buffer is programmable through the CRT controller, but it    */
	/* must be an even address. The character codes and attributes are then */
	/* displayed according to their relative positions in the buffer. The   */
	/* The following addresses will produce an "AB" in the upper-left corner*/
	/* of a 40 by 25 screen and an "X" in the lower-right corner.           */
	/* -------------------------------------------------------------------- */
	/* (Even)    Memory   Display Buffer: (Example of a 40 by 25 Screen):   */
	/* Starting  Address                    - AB                            */
	/* Address: (in hex):                  Video Screen:                    */
	/*  - B8000:           - Character Code A                               */
	/*  - B8001:           - Attribute      A                               */
	/*  - B8002:           - Character Code B                               */
	/*  - B8003:           - Attribute      B                               */
	/* Last Address:                                                        */
	/*  - B87CE:           - Character Code X                               */
	/*  - B87CF:           - Attribute      X                               */
	/* -------------------------------------------------------------------- */
	/* The processor and the display control unit have equal access to the  */
	/* display buffer during all of the operating modes, except the high-   */
	/* resolution alphanumeric mode. During this mode, the processor gains  */
	/* access to the display buffer during the vertical retrace time. If it */
	/* does not, the display will be affected with random patterns as the   */
	/* microprocessor is using the display buffer. In the alphanumeric mode,*/
	/* the characters are displayed from a pre-stored ROM character         */
	/* generator that contains the dot patterns for all of the displayable  */
	/* characters.                                                          */
	/*                                                                      */
	/* In the graphics mode, the displayed dots and colors, up to 16K bytes,*/
	/* are fetched from the display buffer.                                 */
	/* -------------------------------------------------------------------- */
    if (compaq->blink_on == 0) {
		return;
	}
	if ((compaq->reg_crt[COMPAQ_CRTC_CS] & 0x60) == 0x20) { /*  A/R10: Cursor Start             :  B */
		/* cursor off */
		return;
	}
	src = COMPAQ->mem; /* MDA address only */
	if (compaq_get_position (compaq, &x, &y)) {
		return;
	}
	c1 = COMPAQ->reg_crt[COMPAQ_CRTC_CS] & 0x1f; /* A/R10: Cursor Start             :  B */
	c2 = COMPAQ->reg_crt[COMPAQ_CRTC_CE] & 0x1f; /* A/R11: Cursor End               :  C */
	if (c1 >= compaq->ch) {
		return;
	}
	if (c2 >= compaq->ch) {
		c2 = compaq->ch - 1;
	}
	addr = (COMPAQ_get_start (COMPAQ) + 2 * (COMPAQ->w * y + x) + 1) & 0x0fff; /* MDA address only */
    /* ------------------------------------------------------------------------------------------- */
    /* Quote from John Elliott, Vintage PC pages, Compaq Portable/Deskpro, Video Output:           */
    /* ------------------------------------------------------------------------------------------- */
    /* The method used by the card to detect what resolution is very simple: if the CRTC character */
    /* height register is 13 (ie, characters are 14 rows) then it goes into the high resolution    */
    /* mode. If not, it behaves like a normal CGA.                                                 */
    /* ------------------------------------------------------------------------------------------- */
	if (compaq->hires_mono_font = 13);
	{
		/* IBM MDA resolution */
		col = COMPAQ->rgb[src[addr] & 0x0f];
		dst = COMPAQ->buf + 3 * COMPAQ_CW * (COMPAQ->w * (COMPAQ->ch * y + c1) + x);
		for (j = c1; j <= c2; j++) {
			for (i = 0; i < COMPAQ_CW; i++) {
				dst[3 * i + 0] = col[0];
				dst[3 * i + 1] = col[1];
				dst[3 * i + 2] = col[2];
			}
			dst += 3 * COMPAQ_CW * compaq->w;
		}
	}
	else
	{
		/* IBM CGA resolution */
		col = compaq_rgb[compaq->mem[2 * (compaq->w * y + x) + 1] & 0x0f];
		dst = compaq->buf + 3 * 8 * (compaq->w * (compaq->ch * y + c1) + x);
		for (j = c1; j <= c2; j++) {
			for (i = 0; i < 8; i++) {
				dst[3 * i + 0] = col[0];
				dst[3 * i + 1] = col[1];
				dst[3 * i + 2] = col[2];
			}
			dst += 3 * 8 * compaq->w;
		}
	}
}

/*
 * Draw a character in the internal buffer
 */
static
void compaq_mode0_update_char (compaq_t *compaq,
	unsigned char *dst, unsigned char *buf, unsigned char c, unsigned char a)
{
	unsigned            i, j;
	int                 elg, blk; /* MDA address only */
	unsigned            ull; /* MDA address currently enabled */
	int                 blk;
	unsigned char       val;
	const unsigned char *fnt;
	const unsigned char *fg, *bg;
	/* -------------------------------------------------------------------- */
	/* Display Buffer:                                                      */
	/* -------------------------------------------------------------------- */
	/* The display buffer resides in the processor-address space, starting  */
	/* at address hex B8000. It provides 16 bytes of dynamic read/write     */
	/* memory. A dual-ported implementation allows the processor and the    */
	/* graphics control unit access to this buffer. The processor and the   */
	/* control unit have equal access to this buffer during all modes of    */
	/* operation, except in the high-resolution alphanumeric mode. In this  */
	/* mode, only the processor should have access to this buffer during the*/
	/* horizontal-retrace intervals. While the processor may write to the   */
	/* required buffer at any time, a small amount of display interference  */
	/* will result if this does not occur during the horizontal-retrace     */
	/* intervals.                                                           */
	/* -------------------------------------------------------------------- */
	blk = 0;
	/* -------------------------------------------------------------------- */
	/* Basic Operations:                                                    */
	/* -------------------------------------------------------------------- */
	/* In the alphanumeric mode, the adapter fetches character and attribute*/
	/* information from its display buffer. The starting address of the     */
	/* display buffer is programmable through the CRT controller, but it    */
	/* must be an even address. The character codes and attributes are then */
	/* displayed according to their relative positions in the buffer. The   */
	/* The following addresses will produce an "AB" in the upper-left corner*/
	/* of a 40 by 25 screen and an "X" in the lower-right corner.           */
	/* -------------------------------------------------------------------- */
	/* (Even)    Memory   Display Buffer: (Example of a 40 by 25 Screen):   */
	/* Starting  Address                    - AB                            */
	/* Address: (in hex):                  Video Screen:                    */
	/*  - B8000:           - Character Code A                               */
	/*  - B8001:           - Attribute      A                               */
	/*  - B8002:           - Character Code B                               */
	/*  - B8003:           - Attribute      B                               */
	/* Last Address:                                                        */
	/*  - B87CE:           - Character Code X                               */
	/*  - B87CF:           - Attribute      X                               */
	/* -------------------------------------------------------------------- */
	/* The processor and the display control unit have equal access to the  */
	/* display buffer during all of the operating modes, except the high-   */
	/* resolution alphanumeric mode. During this mode, the processor gains  */
	/* access to the display buffer during the vertical retrace time. If it */
	/* does not, the display will be affected with random patterns as the   */
	/* microprocessor is using the display buffer. In the alphanumeric mode,*/
	/* the characters are displayed from a pre-stored ROM character         */
	/* generator that contains the dot patterns for all of the displayable  */
	/* characters.                                                          */
	/*                                                                      */
	/* In the graphics mode, the displayed dots and colors, up to 16K bytes,*/
	/* are fetched from the display buffer.                                 */
	/* -------------------------------------------------------------------- */
	/* Mode-Control Register:                                               */
	/* -------------------------------------------------------------------- */
	/* The mode-control register is a 6-bit output-only register. Its I/O   */
	/* address is hex 3D8, and it can be written to using a processor 'out' */
	/* command. The following are bit definitions for this register.        */
	/*  - Bit 0: A 1 selects  80 by 25 alphanumeric mode.                   */
	/*           A 0 selects  40 by 25 alphanumeric mode.                   */
	/*  - Bit 1: A 1 selects 320 by 200 graphics    mode.                   */
	/*           A 0 selects           alphanumeric mode.                   */
	/*  - Bit 2: A 1 selects black-and-white        mode.                   */
	/*           A 0 selects color                  mode.                   */
	/*  - Bit 3: A 1 enables the video signal. The video signal is disabled */
	/*           when changing modes.                                       */
	/*  - Bit 4: A 1 selects the high-resolution (640 by 200) black-and-    */
	/*           white graphics mode. One of eight colors can be selected on*/
	/*           direct-drive monitors in this mode by using register hex   */
	/*           3D9.                                                       */
	/*  - Bit 5: A 1 will change the character background intensity to the  */
	/*           blinking attribute function for alphanumeric modes. When   */
	/*           the high-order attribute is not selected, 16 background    */
	/*           colors or intensified colors are available. This bit is set*/
	/*           to 1 to allow the blinking function.                       */
	/* -------------------------------------------------------------------- */
	/* Mode-Control Register Summary:                                       */
	/* -------------------------------------------------------------------- */
	/* Bits:                                                                */
	/* 0   :1   :2   :3   :4   :5   : 40 x 25 Alphanumeric Black-and-White  */
	/*  - 0: - 0: - 1: - 1: - 0: - 1: 40 x 25 Alphanumeric Color            */
	/*  - 0: - 0: - 0: - 1: - 0: - 1: 80 x 25 Alphanumeric Black-and-White  */
	/*  - 1: - 0: - 1: - 1: - 0: - 1: 80 x 25 Alphanumeric Color            */
	/*  - 1: - 0: - 0: - 1: - 0: - 1: 320 x 200 Black-and-White Graphics    */
	/*  - 0: - 1: - 1: - 1: - 0: - z: 320 x 200 Color Graphics              */
	/*  - 0: - 1: - 0: - 1: - 0: - z: 640 x 200 Black-and-White Graphics    */
	/* -------------------------------------------------------------------- */
	/*  - 5: Enable Blink Attribute                                         */
	/*  - 4: 640 x 200 Black-and-White                                      */
	/*  - 3: Enable Video Signal                                            */
	/*  - 2: Select Black-and-White Mode                                    */
	/*  - 1: Select 320 x 200 Graphics                                      */
	/*  - 0: 80 x 25 Alphanumeric Select                                    */
	/* z = don't care condition                                             */
	/* Note: The low-resolution (160 by 100) mode requires special          */
	/* programming and is set up as the 40 by 25 alphanumeric mode.         */
	/* -------------------------------------------------------------------- */
	/* Sequence of Events for Changing Modes:                               */
	/* -------------------------------------------------------------------- */
	/* - 1. Determine the mode of operation.                                */
	/* - 2. Reset the video-enable bit in the mode-control register.        */
	/* - 3. Program the 6845 CRT controller to select the mode.             */
	/* - 4. Program the mode-control and color-select registers including   */
	/* re-enabling the video.                                               */
	/* -------------------------------------------------------------------- */
    /* ------------------------------------------------------------------------------------------- */
    /* Quote from John Elliott, Vintage PC pages, Compaq Portable/Deskpro, Video Output:           */
    /* ------------------------------------------------------------------------------------------- */
    /* The method used by the card to detect what resolution is very simple: if the CRTC character */
    /* height register is 13 (ie, characters are 14 rows) then it goes into the high resolution    */
    /* mode. If not, it behaves like a normal CGA.                                                 */
    /* ------------------------------------------------------------------------------------------- */
	if (compaq->hires_mono_font = 13);
	{
		/* IBM MDA resolution */
		if (compaq->reg[COMPAQ_MODE] & COMPAQ_MODE_BLINK) {
			a &= 0x7f;
		}
	}
	else
	{
		/* IBM CGA resolution */
		if (compaq->reg[COMPAQ_MODE] & COMPAQ_MODE_BLINK) {
			if (a & 0x80) {
				blk = !compaq->blink_on;
				if ((compaq->reg[COMPAQ_CSEL] & COMPAQ_CSEL_INT) == 0) {
					a &= 0x7f;
				}
			}
		}
	}
	/* -------------------------------------------------------------------- */
	/* Color-Select Register:                                               */
	/* -------------------------------------------------------------------- */
	/* The color-select register is a 6-bit output-only register. Its I/O   */
	/* address is hex 3D9, and it can be written to using a processor 'out' */
	/* command. The following are the bit definitions for this register.    */
	/*  - Bit 0: Selects blue border color in 40 by 25 alphanumeric mode.   */
	/*  Selects blue background color (C0-C1) in 320 by 200 graphics mode.  */
	/*  Selects blue foreground color in 640 by 200 graphics mode.          */
	/*  - Bit 1: Selects green border color in 40 by 25 alphanumeric mode.  */
	/*  Selects green background color (C0-C1) in 320 by 200 graphics mode. */
	/*  Selects green foreground color in 640 by 200 graphics mode.         */
	/*  - Bit 2: Selects red border color in 40 by 25 alphanumeric mode.    */
	/*  Selects red background color (C0-C1) in 320 by 200 graphics mode.   */
	/*  Selects red foreground color in 640 by 200 graphics mode.           */
	/*  - Bit 3: Selects intensified border color in 40 by 25 alphanumeric  */
	/*  mode. Selects intensified background color (C0-C1) in 320 by 200    */
	/*  graphics mode. Selects intensified foreground color in 640 by 200   */
	/*  graphics mode.                                                      */
	/*  - Bit 4:Selects alternate, intensified set of colors in the graphics*/
	/*  mode. Selects background colors in the alphanumeric mode.           */
	/*  - Bit 5: Selects active color set in 320 by 200 graphics mode.      */
	/*  When bit 5 is set to 1, colors are determined as follows:           */
	/*  C1:  C0:  Set Selected:                                             */
	/*   - 0: - 0: - Background (Defined by bits 0-3 of port hex 3D9)       */
	/*   - 0: - 1: - Cyan                                                   */
	/*   - 1: - 0: - Magenta                                                */
	/*   - 1: - 1: - White                                                  */
	/*  When bit 5 is set to 0, colors are determined as follows:           */
	/*  C1:  C0:  Set Selected:                                             */
	/*   - 0: - 0: - Background (Defined by bits 0-3 of port hex 3D9)       */
	/*   - 0: - 1: - Green                                                  */
	/*   - 1: - 0: - Red                                                    */
	/*   - 1: - 1: - Brown                                                  */
	/*  - Bit 6: Not used                                                   */
	/*  - Bit 7: Not used                                                   */
	/* -------------------------------------------------------------------- */
	fg = compaq_rgb[a & 0x0f];
	bg = compaq_rgb[(a >> 4) & 0x0f];

	ull = ((a & 0x07) == 1) ? 13 : 0xffff;
	elg = ((c >= 0xc0) && (c <= 0xdf));

	COMPAQ_map_attribute (COMPAQ, a, &fg, &bg); /* MDA capabilities reused */
	/* ------------------------------------------------------------------------------------------- */
	/* Quote from John Elliott, Vintage PC pages, Compaq Portable/Deskpro, Video Output:           */
	/* ------------------------------------------------------------------------------------------- */
	/* The method used by the card to detect what resolution is very simple: if the CRTC character */
	/* height register is 13 (ie, characters are 14 rows) then it goes into the high resolution    */
	/* mode. If not, it behaves like a normal CGA.                                                 */
	/* ------------------------------------------------------------------------------------------- */
	if (compaq->hires_mono_font = 13);
	{
		/* MDA font height (720x350) */
		fnt = compaq->font + 14 * c;
		for (j = 0; j < compaq->ch; j++) {
			if (blk) {
				val = 0x000;      /* Enabled  MDA settings */
			}
			else if (j == ull) {  /* Enabled  MDA settings */
				val = 0x1ff;
			}
			else {
				val = fnt[j % 14] << 1;
				if (elg) {
					val |= (val >> 1) & 1;
				}
			}
			for (i = 0; i < COMPAQ_CW; i++) {
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
			dst += (3 * COMPAQ_CW * COMPAQ->w);
		}
	}
	else
	{
		/* CGA font height (640x200) */
		fnt = compaq->font + 8 * c;
		for (j = 0; j < compaq->ch; j++) {
			if (blk) {
				val = 0x00;
			}
			else {
				val = fnt[j & 7];
			}
			for (i = 0; i < 8; i++) {
				if (val & 0x80) {
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
			dst += (3 * 8 * compaq->w);
		}
	}
	/* -------------------------------------------------------------------- */
	/* Alphanumeric Mode:                                                   */
	/* -------------------------------------------------------------------- */
	/* Every display character position in the alphanumeric mode is defined */
	/* by two bytes in regen buffer (a part of the monitor adapter), not the*/
	/* system memory. Both the Color/Graphics Monitor Adapter and the       */
	/* the Monochrome Display and Printer Adapter use the following 2-byte  */
	/* character-attribute format.                                          */
	/*                                                                      */
	/* Display-Character Code Byte:     Attribute Byte:                     */
	/*  - 7 - 6 - 5 - 4 - 3 - 2 - 1 - 0  - 7 - 6 - 5 - 4 - 3 - 2 - 1 - 0    */
	/*                                                                      */
	/* The following table shows the functions of the attribute byte.       */
	/* -------------------------------------------------------------------- */
	/* Attribute Function/Attribute Byte:                                   */
	/* -------------------------------------------------------------------- */
	/* I = Highlighted Foreground (Character):                              */
	/* B = Blinking    Foreground (Character):                              */
	/* Normal             (Background): Normal             (Foreground):    */
	/*  - 7 : 6 5 4                      - 3 : 2 1 0                        */
	/*  - B : R G B                      - I : R G B                        */
	/*  - FG: Background                 - Foreground:                      */
	/*  - B : 0 0 0                      - I : 1 1 1                        */
	/* Reverse Video      (Background): Reverse Video      (Foreground):    */
	/*  - 7 : 6 5 4                      - 3 : 2 1 0                        */
	/*  - B : R G B                      - I : R G B                        */
	/*  - FG: Background                 - Foreground:                      */
	/*  - B : 1 1 1                      - I : 0 0 0                        */
	/* Nondisplay (Black) (Background): Nondisplay (Black) (Foreground):    */
	/*  - 7 : 6 5 4                      - 3 : 2 1 0                        */
	/*  - B : R G B                      - I : R G B                        */
	/*  - FG: Background                 - Foreground:                      */
	/*  - B : 0 0 0                      - I : 0 0 0                        */
	/* Nondisplay (White) (Background): Nondisplay (White) (Foreground):    */
	/*  - 7 : 6 5 4                      - 3 : 2 1 0                        */
	/*  - B : R G B                      - I : R G B                        */
	/*  - FG: Background                 - Foreground:                      */
	/*  - B : 1 1 1                      - I : 1 1 1                        */
	/*                                                                      */
	/* The definitions of the attribute byte are in the following table.    */
	/*  - 7: B: Blinking                                                    */
	/*  - 6: R:                                                             */
	/*  - 5: G: Background Color                                            */
	/*  - 4: B:                                                             */
	/*  - 3: I: Intensity                                                   */
	/*  - 2: R:                                                             */
	/*  - 1: G: Foreground                                                  */
	/*  - 0: B:                                                             */
	/* In the alphanumeric mode, the display can be operated in either a low*/
	/* -resolution mode or a high-resolution mode.                          */
	/*                                                                      */
	/* The low-resolution alphanumeric mode has the following features:     */
	/*  - Supports home color televisions or low-resolution monitors.       */
	/*  - Displays up to 25 rows of 40 characters each.                     */
	/*  - Has a ROS character generator that contains dot patterns for a    */
	/*  maximum of 256 different characters.                                */
	/*  - Requires 2,000 bytes of read/write memory (on the adapter).       */
	/*  - Has an 8-high by 8-wide character box.                            */
	/*  - Has two jumper-controlled character fonts available: a 7-high by 5*/
	/*  -wide single-dot character font with one descender, and a 7-high by */
	/*  7-wide double-dotted character font with one descender.             */
	/*  - Has one character attribute for each character.                   */
	/* The high-resolution alphanumeric mode has the following features:    */
	/*  - Supports the IBM Color Display or other color monitor with direct-*/
	/*  drive capability.                                                   */
	/*  - Supports a black-and-white composite-video monitor.               */
	/*  - Displays up to 25 rows of 80 characters each.                     */
	/*  - Has a ROS display generator that contains dot patterns for a      */
	/*  maximum of 256 characters.                                          */
	/*  - Requires 4,000 bytes of read/write memory (on the adapter).       */
	/*  - Has an 8-high by 8-wide character box.                            */
	/*  - Has two jumper-controlled character fonts available: a 7-high by 5*/
	/*  -wide single-dot character font with one descender, and a 7-high by */
	/*  7-wide double-dot character font with one descender.                */
	/*  - Has one character attribute for each character.                   */
	/* The Color/Graphics Monitor Adapter will change foreground and        */
	/* background colors according to the color value selected in the       */
	/* attribute byte. The following figure shows the color values for the  */
	/* various red, green, blue, and intensity bit settings.                */
	/*  - Black                 : Red: 0 Green: 0 Blue: 0 Intensity: 0      */
	/*  - Blue                  : Red: 0 Green: 0 Blue: 1 Intensity: 0      */
	/*  - Green                 : Red: 0 Green: 1 Blue: 0 Intensity: 0      */
	/*  - Cyan                  : Red: 0 Green: 1 Blue: 1 Intensity: 0      */
	/*  - Red                   : Red: 1 Green: 0 Blue: 0 Intensity: 0      */
	/*  - Magenta               : Red: 1 Green: 0 Blue: 1 Intensity: 0      */
	/*  - Brown                 : Red: 1 Green: 1 Blue: 0 Intensity: 0      */
	/*  - White                 : Red: 1 Green: 1 Blue: 1 Intensity: 0      */
	/*  - Gray                  : Red: 0 Green: 0 Blue: 0 Intensity: 1      */
	/*  - Light Blue            : Red: 0 Green: 0 Blue: 1 Intensity: 1      */
	/*  - Light Green           : Red: 0 Green: 1 Blue: 0 Intensity: 1      */
	/*  - Light Cyan            : Red: 0 Green: 1 Blue: 1 Intensity: 1      */
	/*  - Light Red             : Red: 1 Green: 0 Blue: 0 Intensity: 1      */
	/*  - Light Magenta         : Red: 1 Green: 0 Blue: 1 Intensity: 1      */
	/*  - Yellow                : Red: 1 Green: 1 Blue: 0 Intensity: 1      */
	/*  - White (High Intensity): Red: 1 Green: 1 Blue: 1 Intensity: 1      */
	/* Note: Not all monitors recognize the intensity (I) bit.              */
	/* -------------------------------------------------------------------- */
}

/*
 * Update mode 0 (text 80 * 25 * 16)
 */
static
void compaq_mode0_update (compaq_t *compaq)
{
	unsigned            x, y;
	unsigned            ofs;
	const unsigned char *src;
	unsigned char       *dst;
	compaq->hires_mono_font = 8;
	if (compaq_set_buf_size (compaq, 8 * compaq->w, compaq->ch * compaq->h)) {
		return;
	}
	src = compaq->mem;
	ofs = (2 * compaq_get_start (compaq)) & 0x3ffe;
	for (y = 0; y < compaq->h; y++) {
		dst = compaq->buf + 3 * (8 * compaq->w) * (compaq->ch * y);
		for (x = 0; x < compaq->w; x++) {
			compaq_mode0_update_char (compaq,
				dst + 3 * 8 * x, src[ofs], src[ofs + 1]
			);
			ofs = (ofs + 2) & 0x3ffe;
		}
	}
	compaq_mode0_update_cursor (compaq);
}

/* -------------------------------------------------------------------- */
/* Graphics Modes:                                                      */
/* -------------------------------------------------------------------- */
/* The Color/Graphics Monitor Adapter has three graphics modes: low-    */
/* resolution, medium-resolution, and high-resolution color graphics.   */
/* However, only medium- and high-resolution graphics are supported in  */
/* ROM. The following figure shows these modes.                         */
/* -------------------------------------------------------------------- */
/*                   Horizontal Vertical Number of Colors Available     */
/* Mode:               (PELs):   (Rows):(Includes Background Color )    */
/*  - Low    Resolution: 160       100   - 16 (Includes black-and-white)*/
/*  - Medium Resolution: 320       200   -  4 Colors Total              */
/*                                        1 of 16 for Background and 1  */
/*                                        of Green, Red, or Brown or 1  */
/*                                        of Cyan, Megenta, or White    */
/*  - High   Resolution: 640       200   - Black-and-white only         */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/* Low-Resolution Color/Graphics Mode:                                  */
/* -------------------------------------------------------------------- */
/* The low-resolution mode supports home televisions or color monitors. */
/* This mode, not supported in ROM, has the following features:         */
/*  - Contains a maximum of 160 PELs by 100 rows, with each PEL being 2-*/
/*  high by 2-wide.                                                     */
/*  - Specified 1 of 16 colors for each PEL by the I, R, G, and B bits. */
/*  - Requires 16,000 bytes of read/write memory on the adapter.        */
/*  - Uses memory-mapped graphics.                                      */
/* -------------------------------------------------------------------- */

/*
 * Update mode 1 (graphics 320 * 200 * 4)
 */
static
void compaq_mode1_update (compaq_t *compaq)
{
	unsigned            i, x, y, w, h;
	unsigned            ofs, val, idx;
	unsigned char       *dst;
	const unsigned char *col, *mem;
	/* -------------------------------------------------------------------- */
	/* Medium-Resolution Color/Graphics Mode:                               */
	/* -------------------------------------------------------------------- */
	/* The medium-resolution mode supports home televisions or color        */
	/* monitors. It has the following features:                             */
	/*  - Contains a maximum of 320 PELs by 200 rows, with each PEL being 1-*/
	/*  high by 1-wide.                                                     */
	/*  - Preselects 1 of 4 colors for each PEL.                            */
	/*  - Requires 16,000 bytes of read/write memory on the adapter.        */
	/*  - Uses memory-mapped graphics.                                      */
	/*  - Formats 4 PELs per byte in the following manner:                  */
	/*  - - 7 C1/6 C0: First  Display PEL                                   */
	/*  - - 5 C1/4 C0: Second Display PEL                                   */
	/*  - - 3 C1/2 C0: Third  Display PEL                                   */
	/*  - - 1 C1/0 C0: Fourth Display PEL                                   */
	/*  - Organizes graphics storage into two banks of 8,000 bytes, using   */
	/*  the following format:                                               */
	/*  Memory Address (in hex): Function:                                  */
	/*   - B8000                  - Even Scans (0,2,4,...198) 8,000 bytes   */
	/*   - B9F3F                  - Not Used                                */
	/*   - BA000                  - Odd  Scans (1,3,5,...199) 8,000 bytes   */
	/*   - B8FFF                  - Not Used                                */
	/*  Address hex B8000 contains the PEL instruction for the upper-left   */
	/*  corner of the display area.                                         */
	/* -------------------------------------------------------------------- */
	/* Color selection is determined by the following logic:                */
	/* -------------------------------------------------------------------- */
	/* C1: C0 : Function:                                                   */
	/*  - C1: 0/C0: 0: Dot takes on the color of 1 to 16 preselected        */
	/*  background colors.                                                  */
	/*  - C1: 0/C0: 1: Selects first  color of preselected Color Set 1 or   */
	/*  Color Set 2.                                                        */
	/*  - C1: 1/C0: 0: Selects second color of preselected Color Set 1 or   */
	/*  Color Set 2.                                                        */
	/*  - C1: 1/C0: 1: Selects third  color of preselected Color Set 1 or   */
	/*  Color Set 2.                                                        */
	/* C1 and C0 select 4 of 16 preselected colors. This color selection    */
	/* (palette) is preloaded in an I/O port.                               */
	/*                                                                      */
	/* The two color sets are: Color Set 1/Color Set 2:                     */
	/*  - Color 1 is Green/Color 1 is Cyan                                  */
	/*  - Color 2 is Red  /Color 2 is Magenta                               */
	/*  - Color 3 is Brown/Color 3 is White                                 */
	/* The background colors are the same basic 8 colors defined for low-   */
	/* resolution graphics, plus 8 alternate intensities defined by the     */
	/* intensity bit, for a total of 16 colors, including black and white.  */
	/* -------------------------------------------------------------------- */
	/* ------------------------------------------------------------------------------------------- */
	/* Quote from John Elliott, Vintage PC pages, Compaq Portable/Deskpro, Video Output:           */
	/* ------------------------------------------------------------------------------------------- */
	/* The method used by the card to detect what resolution is very simple: if the CRTC character */
	/* height register is 13 (ie, characters are 14 rows) then it goes into the high resolution    */
	/* mode. If not, it behaves like a normal CGA.                                                 */
	/* ------------------------------------------------------------------------------------------- */
	compaq->hires_mono_font = 8
	if (compaq_set_buf_size (compaq, 8 * compaq->w, 2 * compaq->h)) {
		return;
	}
	dst = compaq->buf;
	w = 2 * compaq->w;
	h = 2 * compaq->h;
	ofs = (2 * compaq_get_start (compaq)) & 0x1fff;
	for (y = 0; y < h; y++) {
		mem = compaq->mem + ((y & 1) << 13);
		for (x = 0; x < w; x++) {
			val = mem[(ofs + x) & 0x1fff];
			for (i = 0; i < 4; i++) {
				idx = (val >> 6) & 3;
				idx = compaq->pal[idx];
				col = compaq_rgb[idx];
				val = (val << 2) & 0xff;

				dst[0] = col[0];
				dst[1] = col[1];
				dst[2] = col[2];

				dst += 3;
			}
		}
		if (y & 1) {
			ofs = (ofs + w) & 0x1fff;
		}
	}
}

/*
 * Update mode 2 (graphics 640 * 200 * 2)
 */
static
void compaq_mode2_update (compaq_t *compaq)
{
	unsigned            i, x, y, w, h;
	unsigned            ofs, val, idx;
	unsigned char       *dst;
	const unsigned char *col, *mem;
	/* -------------------------------------------------------------------- */
	/* High-Resolution Black-and-White Graphics Mode:                       */
	/* -------------------------------------------------------------------- */
	/* The high-resolution mode supports color monitors. This mode has the  */
	/* following features:                                                  */
	/*  - Contains a maximum of 640 PELs by 200 rows, with each PEL being 1-*/
	/*  high by 1-wide.                                                     */
	/*  - Supports black-and-white only.                                    */
	/*  - Requires 16,000 bytes of read/write memory on the adapter.        */
	/*  - Addressing and mapping procedures are the same as medium-         */
	/*  resolution color/graphics, but the data format is different. In this*/
	/*  mode, each bit is mapped to a PEL on the screen.                    */
	/*  - Formats 8 PELs per byte in the following manner:                  */
	/*  - - 7: First   Display PEL                                          */
	/*  - - 6: Second  Display PEL                                          */
	/*  - - 5: Third   Display PEL                                          */
	/*  - - 4: Fourth  Display PEL                                          */
	/*  - - 3: Fifth   Display PEL                                          */
	/*  - - 2: Sixth   Display PEL                                          */
	/*  - - 1: Seventh Display PEL                                          */
	/*  - - 0: Eighth  Display PEL                                          */
	/* -------------------------------------------------------------------- */
	/* ------------------------------------------------------------------------------------------- */
	/* Quote from John Elliott, Vintage PC pages, Compaq Portable/Deskpro, Video Output:           */
	/* ------------------------------------------------------------------------------------------- */
	/* The method used by the card to detect what resolution is very simple: if the CRTC character */
	/* height register is 13 (ie, characters are 14 rows) then it goes into the high resolution    */
	/* mode. If not, it behaves like a normal CGA.                                                 */
	/* ------------------------------------------------------------------------------------------- */
	compaq->hires_mono_font = 8
	if (compaq_set_buf_size (compaq, 16 * compaq->w, 2 * compaq->h)) {
		return;
	}
	dst = compaq->buf;

	h = 2 * compaq->h;
	w = 2 * compaq->w;

	ofs = (2 * compaq_get_start (compaq)) & 0x1fff;
	for (y = 0; y < h; y++) {
		mem = compaq->mem + ((y & 1) << 13);
		for (x = 0; x < w; x++) {
			val = mem[(ofs + x) & 0x1fff];
			for (i = 0; i < 8; i++) {
				idx = (val & 0x80) ? compaq->pal[0] : 0;
				col = compaq_rgb[idx];
				val = (val << 1) & 0xff;

				dst[0] = col[0];
				dst[1] = col[1];
				dst[2] = col[2];

				dst += 3;
			}
		}
		if (y & 1) {
			ofs = (ofs + w) & 0x1fff;
		}
	}
}

/*
 * Create the composite mode color lookup table
 *
 * This is adapted from DOSBox
 */
static
void compaq_make_comp_tab (compaq_t *compaq)
{
	unsigned            i, j;
	double              Y, I, Q, t;
	double              r, g, b;
	double              hue, sat, lum;
	double              hv, sh, ch;
	const unsigned char *src;
	unsigned char       *dst;
	/* -------------------------------------------------------------------- */
	/* Composite Color Generator:                                           */
	/* -------------------------------------------------------------------- */
	/* This generator produces base-band-video color information.           */
	/* -------------------------------------------------------------------- */

	/* -------------------------------------------------------------------- */
	/* Alphanumeric Mode:                                                   */
	/* -------------------------------------------------------------------- */
	/* Every display character position in the alphanumeric mode is defined */
	/* by two bytes in regen buffer (a part of the monitor adapter), not the*/
	/* system memory. Both the Color/Graphics Monitor Adapter and the       */
	/* the Monochrome Display and Printer Adapter use the following 2-byte  */
	/* character-attribute format.                                          */
	/*                                                                      */
	/* Display-Character Code Byte:     Attribute Byte:                     */
	/*  - 7 - 6 - 5 - 4 - 3 - 2 - 1 - 0  - 7 - 6 - 5 - 4 - 3 - 2 - 1 - 0    */
	/*                                                                      */
	/* The following table shows the functions of the attribute byte.       */
	/* -------------------------------------------------------------------- */
	/* Attribute Function/Attribute Byte:                                   */
	/* -------------------------------------------------------------------- */
	/* I = Highlighted Foreground (Character):                              */
	/* B = Blinking    Foreground (Character):                              */
	/* Normal             (Background): Normal             (Foreground):    */
	/*  - 7 : 6 5 4                      - 3 : 2 1 0                        */
	/*  - B : R G B                      - I : R G B                        */
	/*  - FG: Background                 - Foreground:                      */
	/*  - B : 0 0 0                      - I : 1 1 1                        */
	/* Reverse Video      (Background): Reverse Video      (Foreground):    */
	/*  - 7 : 6 5 4                      - 3 : 2 1 0                        */
	/*  - B : R G B                      - I : R G B                        */
	/*  - FG: Background                 - Foreground:                      */
	/*  - B : 1 1 1                      - I : 0 0 0                        */
	/* Nondisplay (Black) (Background): Nondisplay (Black) (Foreground):    */
	/*  - 7 : 6 5 4                      - 3 : 2 1 0                        */
	/*  - B : R G B                      - I : R G B                        */
	/*  - FG: Background                 - Foreground:                      */
	/*  - B : 0 0 0                      - I : 0 0 0                        */
	/* Nondisplay (White) (Background): Nondisplay (White) (Foreground):    */
	/*  - 7 : 6 5 4                      - 3 : 2 1 0                        */
	/*  - B : R G B                      - I : R G B                        */
	/*  - FG: Background                 - Foreground:                      */
	/*  - B : 1 1 1                      - I : 1 1 1                        */
	/*                                                                      */
	/* The definitions of the attribute byte are in the following table.    */
	/*  - 7: B: Blinking                                                    */
	/*  - 6: R:                                                             */
	/*  - 5: G: Background Color                                            */
	/*  - 4: B:                                                             */
	/*  - 3: I: Intensity                                                   */
	/*  - 2: R:                                                             */
	/*  - 1: G: Foreground                                                  */
	/*  - 0: B:                                                             */
	/* In the alphanumeric mode, the display can be operated in either a low*/
	/* -resolution mode or a high-resolution mode.                          */
	/*                                                                      */
	/* The low-resolution alphanumeric mode has the following features:     */
	/*  - Supports home color televisions or low-resolution monitors.       */
	/*  - Displays up to 25 rows of 40 characters each.                     */
	/*  - Has a ROS character generator that contains dot patterns for a    */
	/*  maximum of 256 different characters.                                */
	/*  - Requires 2,000 bytes of read/write memory (on the adapter).       */
	/*  - Has an 8-high by 8-wide character box.                            */
	/*  - Has two jumper-controlled character fonts available: a 7-high by 5*/
	/*  -wide single-dot character font with one descender, and a 7-high by */
	/*  7-wide double-dotted character font with one descender.             */
	/*  - Has one character attribute for each character.                   */
	/* The high-resolution alphanumeric mode has the following features:    */
	/*  - Supports the IBM Color Display or other color monitor with direct-*/
	/*  drive capability.                                                   */
	/*  - Supports a black-and-white composite-video monitor.               */
	/*  - Displays up to 25 rows of 80 characters each.                     */
	/*  - Has a ROS display generator that contains dot patterns for a      */
	/*  maximum of 256 characters.                                          */
	/*  - Requires 4,000 bytes of read/write memory (on the adapter).       */
	/*  - Has an 8-high by 8-wide character box.                            */
	/*  - Has two jumper-controlled character fonts available: a 7-high by 5*/
	/*  -wide single-dot character font with one descender, and a 7-high by */
	/*  7-wide double-dot character font with one descender.                */
	/*  - Has one character attribute for each character.                   */
	/* The Color/Graphics Monitor Adapter will change foreground and        */
	/* background colors according to the color value selected in the       */
	/* attribute byte. The following figure shows the color values for the  */
	/* various red, green, blue, and intensity bit settings.                */
	/*  - Black                 : Red: 0 Green: 0 Blue: 0 Intensity: 0      */
	/*  - Blue                  : Red: 0 Green: 0 Blue: 1 Intensity: 0      */
	/*  - Green                 : Red: 0 Green: 1 Blue: 0 Intensity: 0      */
	/*  - Cyan                  : Red: 0 Green: 1 Blue: 1 Intensity: 0      */
	/*  - Red                   : Red: 1 Green: 0 Blue: 0 Intensity: 0      */
	/*  - Magenta               : Red: 1 Green: 0 Blue: 1 Intensity: 0      */
	/*  - Brown                 : Red: 1 Green: 1 Blue: 0 Intensity: 0      */
	/*  - White                 : Red: 1 Green: 1 Blue: 1 Intensity: 0      */
	/*  - Gray                  : Red: 0 Green: 0 Blue: 0 Intensity: 1      */
	/*  - Light Blue            : Red: 0 Green: 0 Blue: 1 Intensity: 1      */
	/*  - Light Green           : Red: 0 Green: 1 Blue: 0 Intensity: 1      */
	/*  - Light Cyan            : Red: 0 Green: 1 Blue: 1 Intensity: 1      */
	/*  - Light Red             : Red: 1 Green: 0 Blue: 0 Intensity: 1      */
	/*  - Light Magenta         : Red: 1 Green: 0 Blue: 1 Intensity: 1      */
	/*  - Yellow                : Red: 1 Green: 1 Blue: 0 Intensity: 1      */
	/*  - White (High Intensity): Red: 1 Green: 1 Blue: 1 Intensity: 1      */
	/* Note: Not all monitors recognize the intensity (I) bit.              */
	/* -------------------------------------------------------------------- */
	if (compaq->comp_tab == NULL) {
		compaq->comp_tab = malloc (5 * 16 * 3);
	}
	hue = 0.0;
	sat = 2.0 / 3.0;
	lum = 1.0;
	if (compaq->reg[COMPAQ_CSEL] & COMPAQ_CSEL_PAL) {
		hv = 35.0;
	}
	else {
		hv = 50.0;
	}
	hv = (hv + hue) * 0.017453292519943295474;
	sh = sin (hv);
	ch = cos (hv);
	/* -------------------------------------------------------------------- */
	/* Color-Select Register:                                               */
	/* -------------------------------------------------------------------- */
	/* The color-select register is a 6-bit output-only register. Its I/O   */
	/* address is hex 3D9, and it can be written to using a processor 'out' */
	/* command. The following are the bit definitions for this register.    */
	/*  - Bit 0: Selects blue border color in 40 by 25 alphanumeric mode.   */
	/*  Selects blue background color (C0-C1) in 320 by 200 graphics mode.  */
	/*  Selects blue foreground color in 640 by 200 graphics mode.          */
	/*  - Bit 1: Selects green border color in 40 by 25 alphanumeric mode.  */
	/*  Selects green background color (C0-C1) in 320 by 200 graphics mode. */
	/*  Selects green foreground color in 640 by 200 graphics mode.         */
	/*  - Bit 2: Selects red border color in 40 by 25 alphanumeric mode.    */
	/*  Selects red background color (C0-C1) in 320 by 200 graphics mode.   */
	/*  Selects red foreground color in 640 by 200 graphics mode.           */
	/*  - Bit 3: Selects intensified border color in 40 by 25 alphanumeric  */
	/*  mode. Selects intensified background color (C0-C1) in 320 by 200    */
	/*  graphics mode. Selects intensified foreground color in 640 by 200   */
	/*  graphics mode.                                                      */
	/*  - Bit 4:Selects alternate, intensified set of colors in the graphics*/
	/*  mode. Selects background colors in the alphanumeric mode.           */
	/*  - Bit 5: Selects active color set in 320 by 200 graphics mode.      */
	/*  When bit 5 is set to 1, colors are determined as follows:           */
	/*  C1:  C0:  Set Selected:                                             */
	/*   - 0: - 0: - Background (Defined by bits 0-3 of port hex 3D9)       */
	/*   - 0: - 1: - Cyan                                                   */
	/*   - 1: - 0: - Magenta                                                */
	/*   - 1: - 1: - White                                                  */
	/*  When bit 5 is set to 0, colors are determined as follows:           */
	/*  C1:  C0:  Set Selected:                                             */
	/*   - 0: - 0: - Background (Defined by bits 0-3 of port hex 3D9)       */
	/*   - 0: - 1: - Green                                                  */
	/*   - 1: - 0: - Red                                                    */
	/*   - 1: - 1: - Brown                                                  */
	/*  - Bit 6: Not used                                                   */
	/*  - Bit 7: Not used                                                   */
	/* -------------------------------------------------------------------- */
	src = compaq_rgb[compaq->reg[COMPAQ_CSEL] & 0x0f];
	for (i = 0; i < 16; i++) {
		I = ((i & 0x08) ? 1.0 : 0.0) - ((i & 0x02) ? 1.0 : 0.0);
		Q = ((i & 0x04) ? 1.0 : 0.0) - ((i & 0x01) ? 1.0 : 0.0);

		I = sat * (0.5 * I);
		Q = sat * (0.5 * Q);

		t = ch * I + sh * Q;
		Q = ch * Q - sh * I;
		I = t;
		for (j = 0; j < 5; j++) {
			Y = lum * (double) j / 4.0;

			r = Y + 0.956 * I + 0.621 * Q;
			g = Y - 0.272 * I - 0.647 * Q;
			b = Y - 1.105 * I + 1.702 * Q;

			r = (r < 0.0) ? 0.0 : ((r > 1.0) ? 1.0 : r);
			g = (g < 0.0) ? 0.0 : ((g > 1.0) ? 1.0 : g);
			b = (b < 0.0) ? 0.0 : ((b > 1.0) ? 1.0 : b);

			dst = compaq->comp_tab + 3 * (16 * j + i);

			dst[0] = (unsigned char) (src[0] * r);
			dst[1] = (unsigned char) (src[1] * g);
			dst[2] = (unsigned char) (src[2] * b);
		}
	}
	compaq->comp_tab_ok = 1;
}

/*
 * Update mode 2 (graphics 640 * 200 * 2) for composite
 */
static
void compaq_mode2c_update (compaq_t *compaq)
{
	unsigned            x, y, w, h, n, ofs;
	unsigned            avgmsk, avgval, pat;
	const unsigned char *src, *col;
	unsigned char       *dst;
	/* -------------------------------------------------------------------- */
	/* Composite Color Generator:                                           */
	/* -------------------------------------------------------------------- */
	/* This generator produces base-band-video color information.           */
	/* -------------------------------------------------------------------- */
	/* ------------------------------------------------------------------------------------------- */
	/* Quote from John Elliott, Vintage PC pages, Compaq Portable/Deskpro, Video Output:           */
	/* ------------------------------------------------------------------------------------------- */
	/* The method used by the card to detect what resolution is very simple: if the CRTC character */
	/* height register is 13 (ie, characters are 14 rows) then it goes into the high resolution    */
	/* mode. If not, it behaves like a normal CGA.                                                 */
	/* ------------------------------------------------------------------------------------------- */
	compaq->hires_mono_font = 8
	if (compaq->comp_tab_ok == 0) {
		compaq_make_comp_tab (compaq);
	}
	if (compaq_set_buf_size (compaq, 16 * compaq->w, 2 * compaq->h)) {
		return;
	}
	dst = compaq->buf;
	ofs = (2 * compaq_get_start (compaq)) & 0x1fff;
	pat = 0;

	h = 2 * compaq->h;
	w = 2 * compaq->w;
	for (y = 0; y < h; y++) {
		src = compaq->mem + ((y & 1) << 13);

		avgmsk = (src[ofs & 0x1fff] >> 7) & 1;
		avgval = avgmsk;

		for (x = 0; x < (8 * w); x++) {
			if ((x & 3) == 0) {
				if ((x & 7) == 0) {
					pat = src[(ofs + (x >> 3)) & 0x1fff];
					pat = (pat << 4) | ((pat >> 4) & 0x0f);
				}
				else {
					pat = pat >> 4;
				}
			}
			avgval = avgval - ((avgmsk >> 3) & 1);
			avgmsk = avgmsk << 1;

			n = x + 1;
			if (n < (8 * w)) {
				if (src[(ofs + (n >> 3)) & 0x1fff] & (0x80 >> (n & 7))) {
					avgmsk |= 1;
					avgval += 1;
				}
			}
			col = compaq->comp_tab + 3 * (16 * avgval + (pat & 0x0f));

			dst[0] = col[0];
			dst[1] = col[1];
			dst[2] = col[2];

			dst += 3;
		}

		if (y & 1) {
			ofs = (ofs + w) & 0x1fff;
		}
	}
}

/*
 * Update the internal screen buffer when the screen is blank
 */
static
void compaq_update_blank (compaq_t *compaq)
{
	unsigned long x, y;
	int           fx, fy;
	unsigned char *dst;
	/* -------------------------------------------------------------------- */
	/* Display Buffer:                                                      */
	/* -------------------------------------------------------------------- */
	/* The display buffer resides in the processor-address space, starting  */
	/* at address hex B8000. It provides 16 bytes of dynamic read/write     */
	/* memory. A dual-ported implementation allows the processor and the    */
	/* graphics control unit access to this buffer. The processor and the   */
	/* control unit have equal access to this buffer during all modes of    */
	/* operation, except in the high-resolution alphanumeric mode. In this  */
	/* mode, only the processor should have access to this buffer during the*/
	/* horizontal-retrace intervals. While the processor may write to the   */
	/* required buffer at any time, a small amount of display interference  */
	/* will result if this does not occur during the horizontal-retrace     */
	/* intervals.                                                           */
	/* -------------------------------------------------------------------- */
	/* ------------------------------------------------------------------------------------------- */
	/* Quote from John Elliott, Vintage PC pages, Compaq Portable/Deskpro, Video Output:           */
	/* ------------------------------------------------------------------------------------------- */
	/* The method used by the card to detect what resolution is very simple: if the CRTC character */
	/* height register is 13 (ie, characters are 14 rows) then it goes into the high resolution    */
	/* mode. If not, it behaves like a normal CGA.                                                 */
	/* ------------------------------------------------------------------------------------------- */
	if (compaq->hires_mono_font = 13);
	{
		/* IBM MDA resolution */
		compaq_set_buf_size (compaq, 720, 350);
	}
	else
    {
		/* IBM CGA resolution */
		compaq_set_buf_size (compaq, 640, 200);
    }
	dst = compaq->buf;
	for (y = 0; y < compaq->buf_h; y++) {
		fy = (y % 16) < 8;
		for (x = 0; x < compaq->buf_w; x++) {
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
void compaq_update (compaq_t *compaq)
{
	unsigned            x, y; /* Continued from IBM MDA */
	unsigned            ofs;  /* Continued from IBM MDA */
	const unsigned char *src; /* Continued from IBM MDA */
	unsigned char       *dst; /* Continued from IBM MDA */
	unsigned char mode;       /* Continued from IBM CGA */
	/* -------------------------------------------------------------------- */
	/* Display Buffer:                                                      */
	/* -------------------------------------------------------------------- */
	/* The display buffer resides in the processor-address space, starting  */
	/* at address hex B8000. It provides 16 bytes of dynamic read/write     */
	/* memory. A dual-ported implementation allows the processor and the    */
	/* graphics control unit access to this buffer. The processor and the   */
	/* control unit have equal access to this buffer during all modes of    */
	/* operation, except in the high-resolution alphanumeric mode. In this  */
	/* mode, only the processor should have access to this buffer during the*/
	/* horizontal-retrace intervals. While the processor may write to the   */
	/* required buffer at any time, a small amount of display interference  */
	/* will result if this does not occur during the horizontal-retrace     */
	/* intervals.                                                           */
	/* -------------------------------------------------------------------- */
	compaq->buf_w = 0;
	compaq->buf_h = 0;
	/* -------------------------------------------------------------------- */
	/* Mode-Control Register:                                               */
	/* -------------------------------------------------------------------- */
	/* The mode-control register is a 6-bit output-only register. Its I/O   */
	/* address is hex 3D8, and it can be written to using a processor 'out' */
	/* command. The following are bit definitions for this register.        */
	/*  - Bit 0: A 1 selects  80 by 25 alphanumeric mode.                   */
	/*           A 0 selects  40 by 25 alphanumeric mode.                   */
	/*  - Bit 1: A 1 selects 320 by 200 graphics    mode.                   */
	/*           A 0 selects           alphanumeric mode.                   */
	/*  - Bit 2: A 1 selects black-and-white        mode.                   */
	/*           A 0 selects color                  mode.                   */
	/*  - Bit 3: A 1 enables the video signal. The video signal is disabled */
	/*           when changing modes.                                       */
	/*  - Bit 4: A 1 selects the high-resolution (640 by 200) black-and-    */
	/*           white graphics mode. One of eight colors can be selected on*/
	/*           direct-drive monitors in this mode by using register hex   */
	/*           3D9.                                                       */
	/*  - Bit 5: A 1 will change the character background intensity to the  */
	/*           blinking attribute function for alphanumeric modes. When   */
	/*           the high-order attribute is not selected, 16 background    */
	/*           colors or intensified colors are available. This bit is set*/
	/*           to 1 to allow the blinking function.                       */
	/* -------------------------------------------------------------------- */
	/* Mode-Control Register Summary:                                       */
	/* -------------------------------------------------------------------- */
	/* Bits:                                                                */
	/* 0   :1   :2   :3   :4   :5   : 40 x 25 Alphanumeric Black-and-White  */
	/*  - 0: - 0: - 1: - 1: - 0: - 1: 40 x 25 Alphanumeric Color            */
	/*  - 0: - 0: - 0: - 1: - 0: - 1: 80 x 25 Alphanumeric Black-and-White  */
	/*  - 1: - 0: - 1: - 1: - 0: - 1: 80 x 25 Alphanumeric Color            */
	/*  - 1: - 0: - 0: - 1: - 0: - 1: 320 x 200 Black-and-White Graphics    */
	/*  - 0: - 1: - 1: - 1: - 0: - z: 320 x 200 Color Graphics              */
	/*  - 0: - 1: - 0: - 1: - 0: - z: 640 x 200 Black-and-White Graphics    */
	/* -------------------------------------------------------------------- */
	/*  - 5: Enable Blink Attribute                                         */
	/*  - 4: 640 x 200 Black-and-White                                      */
	/*  - 3: Enable Video Signal                                            */
	/*  - 2: Select Black-and-White Mode                                    */
	/*  - 1: Select 320 x 200 Graphics                                      */
	/*  - 0: 80 x 25 Alphanumeric Select                                    */
	/* z = don't care condition                                             */
	/* Note: The low-resolution (160 by 100) mode requires special          */
	/* programming and is set up as the 40 by 25 alphanumeric mode.         */
	/* -------------------------------------------------------------------- */
	mode = compaq->reg[COMPAQ_MODE];
	/* -------------------------------------------------------------------- */
	/* Sequence of Events for Changing Modes:                               */
	/* -------------------------------------------------------------------- */
	/* - 1. Determine the mode of operation.                                */
	/* - 2. Reset the video-enable bit in the mode-control register.        */
	/* - 3. Program the 6845 CRT controller to select the mode.             */
	/* - 4. Program the mode-control and color-select registers including   */
	/* re-enabling the video.                                               */
	/* -------------------------------------------------------------------- */
	if ((compaq->reg[COMPAQ_MODE] & COMPAQ_MODE_ENABLE) == 0) { /* MDA equivalent */
		compaq_update_blank (compaq);
		return;
	}
	else if ((compaq->w == 0) || (compaq->h == 0)) {
		compaq_update_blank (compaq);
		return;
	}
	else if ((mode & COMPAQ_MODE_ENABLE) == 0) { /* CGA equivalent */
		compaq_update_blank (compaq);
	}
	if (compaq_set_buf_size (compaq, COMPAQ_CW * compaq->w, compaq->ch * compaq->h)) {
		return;
	}
	else if (mode & COMPAQ_MODE_G320) {
		if (mode & COMPAQ_MODE_G640) {
			if ((mode & COMPAQ_MODE_CBURST) == 0) {
				compaq_mode2c_update (compaq);
			}
			else {
				compaq_mode2_update (compaq);
			}
		}
		else {
			compaq_mode1_update (compaq);
		}
	}
	else {
		compaq_mode0_update (compaq);
	}
	src = compaq->mem;
	ofs = (2 * compaq_get_start (compaq)) & 0x0ffe;
	dst = compaq->buf;
	for (y = 0; y < compaq->h; y++) {
		for (x = 0; x < compaq->w; x++) {
			compaq_draw_char (compaq, dst + 3 * COMPAQ_CW * x, src[ofs], src[ofs + 1]);
			ofs = (ofs + 2) & 0x0ffe;
		}
		dst += 3 * (COMPAQ_CW * compaq->w) * compaq->ch;
	}
	compaq_draw_cursor (compaq);
	/* -------------------------------------------------------------------- */
	/* Basic Operations:                                                    */
	/* -------------------------------------------------------------------- */
	/* In the alphanumeric mode, the adapter fetches character and attribute*/
	/* information from its display buffer. The starting address of the     */
	/* display buffer is programmable through the CRT controller, but it    */
	/* must be an even address. The character codes and attributes are then */
	/* displayed according to their relative positions in the buffer. The   */
	/* The following addresses will produce an "AB" in the upper-left corner*/
	/* of a 40 by 25 screen and an "X" in the lower-right corner.           */
	/* -------------------------------------------------------------------- */
	/* (Even)    Memory   Display Buffer: (Example of a 40 by 25 Screen):   */
	/* Starting  Address                    - AB                            */
	/* Address: (in hex):                  Video Screen:                    */
	/*  - B8000:           - Character Code A                               */
	/*  - B8001:           - Attribute      A                               */
	/*  - B8002:           - Character Code B                               */
	/*  - B8003:           - Attribute      B                               */
	/* Last Address:                                                        */
	/*  - B87CE:           - Character Code X                               */
	/*  - B87CF:           - Attribute      X                               */
	/* -------------------------------------------------------------------- */
	/* The processor and the display control unit have equal access to the  */
	/* display buffer during all of the operating modes, except the high-   */
	/* resolution alphanumeric mode. During this mode, the processor gains  */
	/* access to the display buffer during the vertical retrace time. If it */
	/* does not, the display will be affected with random patterns as the   */
	/* microprocessor is using the display buffer. In the alphanumeric mode,*/
	/* the characters are displayed from a pre-stored ROM character         */
	/* generator that contains the dot patterns for all of the displayable  */
	/* characters.                                                          */
	/*                                                                      */
	/* In the graphics mode, the displayed dots and colors, up to 16K bytes,*/
	/* are fetched from the display buffer.                                 */
	/* -------------------------------------------------------------------- */
}


/*
 * Get a CRTC register
 */
/* static /* Disabled MDA register code */
/* void compaq_crtc_set_reg (compaq_t *compaq, unsigned reg, unsigned char val) */
static
unsigned char compaq_crtc_get_reg (compaq_t *compaq, unsigned reg)
{
	/* ------------------------------------------------------------------------------------------- */
	/* Quote from John Elliott, Vintage PC pages, Compaq Portable/Deskpro, Video Output:           */
	/* ------------------------------------------------------------------------------------------- */
	/* The method used by the card to detect what resolution is very simple: if the CRTC character */
	/* height register is 13 (ie, characters are 14 rows) then it goes into the high resolution    */
	/* mode. If not, it behaves like a normal CGA.                                                 */
	/* ------------------------------------------------------------------------------------------- */
	if (compaq->hires_mono_font = 13);
	{
		/* IBM MDA resolution */
		if ((reg > 15)) {
			return;
		}
	}
	else
    {
		/* IBM CGA resolution */
		if ((reg < 12) || (reg > 17)) {
			return (0xff);
		}
	}
	return (compaq->reg_crt[reg]);
}

/*
 * Set a CRTC register
 */
static
void compaq_crtc_set_reg (compaq_t *compaq, unsigned reg, unsigned char val)
{
	if (reg > 15) {
		return;
	}
	if (compaq->reg_crt[reg] == val) {
		return;
	}
	compaq->reg_crt[reg] = val;
	compaq_set_timing (compaq);
	compaq->update_state |= COMPAQ_UPDATE_DIRTY;
}


/*
 * Get the CRTC index register
 */
static
unsigned char compaq_get_crtc_index (compaq_t *compaq)
{
	return (compaq->reg[COMPAQ_CRTC_INDEX]);
}

/*
 * Get the CRTC data register
 */
static
unsigned char compaq_get_crtc_data (compaq_t *compaq)
{
	return (compaq_crtc_get_reg (compaq, compaq->reg[COMPAQ_CRTC_INDEX]));
}

/*
 * Get the mode control register
 */
static
unsigned char compaq_get_mode (compaq_t *compaq)
{
	return (compaq->reg[COMPAQ_MODE]);
}

/*
 * Get the status register
 */
static
unsigned char compaq_get_status (compaq_t *compaq)
{
	unsigned char val, vid;
	unsigned long clk;

	compaq_clock (compaq, 0);

	clk = compaq_get_dotclock (compaq);

	/* simulate the video signal */
	/* ------------------------------------------------------------------- */
	/* 6845 CRT Status  Port   (Hex 3BA):                                  */
	/* ------------------------------------------------------------------- */
	/* - 0  :       - +Horizontal Drive                                    */
	/* - 1  :       - Reserved                                             */
	/* - 2  :       - Reserved                                             */
	/* - 3  :       - +Black/White Video                                   */
	/* ------------------------------------------------------------------- */
	compaq->reg[COMPAQ_STATUS] ^= COMPAQ_STATUS_VIDEO;

	val = compaq->reg[COMPAQ_STATUS];
	/* ------------------------------------------------------------------- */
	/* 6845 CRT Status  Port   (Hex 3BA):                                  */
	/* ------------------------------------------------------------------- */
	/* - 0  :       - +Horizontal Drive                                    */
	/* - 1  :       - Reserved                                             */
	/* - 2  :       - Reserved                                             */
	/* - 3  :       - +Black/White Video                                   */
	/* ------------------------------------------------------------------- */
	/* ------------------------------------------------------------------------------------------- */
	/* Quote from John Elliott, Vintage PC pages, Compaq Portable/Deskpro, Video Output:           */
	/* ------------------------------------------------------------------------------------------- */
	/* The method used by the card to detect what resolution is very simple: if the CRTC character */
	/* height register is 13 (ie, characters are 14 rows) then it goes into the high resolution    */
	/* mode. If not, it behaves like a normal CGA.                                                 */
	/* ------------------------------------------------------------------------------------------- */
	if (compaq->hires_mono_font = 13);
	{
		/* IBM MDA resolution */
		val = compaq->reg[COMPAQ_STATUS] & ~COMPAQ_STATUS_VIDEO;
		vid = compaq->reg[COMPAQ_STATUS] & COMPAQ_STATUS_VIDEO;
		val |= COMPAQ_STATUS_HSYNC;
		if (compaq->clk_ht > 0) {
			if ((clk % compaq->clk_ht) < compaq->clk_hd) {
				val &= ~COMPAQ_STATUS_HSYNC;
				if (clk < compaq->clk_vd) {
					val |= vid;
				}
			}
		}
	}
	else
    {
		/* IBM CGA resolution */
		val |= (COMPAQ_STATUS_VSYNC | COMPAQ_STATUS_SYNC);
		if (clk < compaq->clk_vd) {
			val &= ~COMPAQ_STATUS_VSYNC;
			if (compaq->clk_ht > 0) {
				if ((clk % compaq->clk_ht) < compaq->clk_hd) {
					val &= ~COMPAQ_STATUS_SYNC;
				}
			}
		}
	}
	return (val);
}

/*
 * Set the CRTC index register
 */
static
void compaq_set_crtc_index (compaq_t *compaq, unsigned char val)
{
	/* ------------------------------------------------------------------------------------------- */
	/* Quote from John Elliott, Vintage PC pages, Compaq Portable/Deskpro, Video Output:           */
	/* ------------------------------------------------------------------------------------------- */
	/* The method used by the card to detect what resolution is very simple: if the CRTC character */
	/* height register is 13 (ie, characters are 14 rows) then it goes into the high resolution    */
	/* mode. If not, it behaves like a normal CGA.                                                 */
	/* ------------------------------------------------------------------------------------------- */
	if (compaq->hires_mono_font = 13);
	{
		/* IBM MDA resolution */
		compaq->reg[COMPAQ_CRTC_INDEX] = val;
	}
	else
    {
		/* IBM CGA resolution */
		compaq->reg[COMPAQ_CRTC_INDEX] = val & 0x1f;
	}
}

/*
 * Set the CRTC data register
 */
static
void compaq_set_crtc_data (compaq_t *compaq, unsigned char val)
{
	compaq->reg[COMPAQ_CRTC_DATA] = val;

	compaq_crtc_set_reg (compaq, compaq->reg[COMPAQ_CRTC_INDEX], val);
}

/*
 * Set the mode register
 */
static
void compaq_set_mode (compaq_t *compaq, unsigned char val)
{
	/* -------------------------------------------------------------------- */
	/* Mode-Control Register:                                               */
	/* -------------------------------------------------------------------- */
	/* The mode-control register is a 6-bit output-only register. Its I/O   */
	/* address is hex 3D8, and it can be written to using a processor 'out' */
	/* command. The following are bit definitions for this register.        */
	/*  - Bit 0: A 1 selects  80 by 25 alphanumeric mode.                   */
	/*           A 0 selects  40 by 25 alphanumeric mode.                   */
	/*  - Bit 1: A 1 selects 320 by 200 graphics    mode.                   */
	/*           A 0 selects           alphanumeric mode.                   */
	/*  - Bit 2: A 1 selects black-and-white        mode.                   */
	/*           A 0 selects color                  mode.                   */
	/*  - Bit 3: A 1 enables the video signal. The video signal is disabled */
	/*           when changing modes.                                       */
	/*  - Bit 4: A 1 selects the high-resolution (640 by 200) black-and-    */
	/*           white graphics mode. One of eight colors can be selected on*/
	/*           direct-drive monitors in this mode by using register hex   */
	/*           3D9.                                                       */
	/*  - Bit 5: A 1 will change the character background intensity to the  */
	/*           blinking attribute function for alphanumeric modes. When   */
	/*           the high-order attribute is not selected, 16 background    */
	/*           colors or intensified colors are available. This bit is set*/
	/*           to 1 to allow the blinking function.                       */
	/* -------------------------------------------------------------------- */
	/* Mode-Control Register Summary:                                       */
	/* -------------------------------------------------------------------- */
	/* Bits:                                                                */
	/* 0   :1   :2   :3   :4   :5   : 40 x 25 Alphanumeric Black-and-White  */
	/*  - 0: - 0: - 1: - 1: - 0: - 1: 40 x 25 Alphanumeric Color            */
	/*  - 0: - 0: - 0: - 1: - 0: - 1: 80 x 25 Alphanumeric Black-and-White  */
	/*  - 1: - 0: - 1: - 1: - 0: - 1: 80 x 25 Alphanumeric Color            */
	/*  - 1: - 0: - 0: - 1: - 0: - 1: 320 x 200 Black-and-White Graphics    */
	/*  - 0: - 1: - 1: - 1: - 0: - z: 320 x 200 Color Graphics              */
	/*  - 0: - 1: - 0: - 1: - 0: - z: 640 x 200 Black-and-White Graphics    */
	/* -------------------------------------------------------------------- */
	/*  - 5: Enable Blink Attribute                                         */
	/*  - 4: 640 x 200 Black-and-White                                      */
	/*  - 3: Enable Video Signal                                            */
	/*  - 2: Select Black-and-White Mode                                    */
	/*  - 1: Select 320 x 200 Graphics                                      */
	/*  - 0: 80 x 25 Alphanumeric Select                                    */
	/* z = don't care condition                                             */
	/* Note: The low-resolution (160 by 100) mode requires special          */
	/* programming and is set up as the 40 by 25 alphanumeric mode.         */
	/* -------------------------------------------------------------------- */
	compaq->reg[COMPAQ_MODE] = val;

	compaq_set_palette (compaq);
	compaq_set_timing (compaq);

	compaq->update_state |= COMPAQ_UPDATE_DIRTY;
	/* -------------------------------------------------------------------- */
	/* Sequence of Events for Changing Modes:                               */
	/* -------------------------------------------------------------------- */
	/* - 1. Determine the mode of operation.                                */
	/* - 2. Reset the video-enable bit in the mode-control register.        */
	/* - 3. Program the 6845 CRT controller to select the mode.             */
	/* - 4. Program the mode-control and color-select registers including   */
	/* re-enabling the video.                                               */
	/* -------------------------------------------------------------------- */
}

/*
 * Set the color select register
 */
static
void compaq_set_color_select (compaq_t *compaq, unsigned char val)
{
	/* -------------------------------------------------------------------- */
	/* Color-Select Register:                                               */
	/* -------------------------------------------------------------------- */
	/* The color-select register is a 6-bit output-only register. Its I/O   */
	/* address is hex 3D9, and it can be written to using a processor 'out' */
	/* command. The following are the bit definitions for this register.    */
	/*  - Bit 0: Selects blue border color in 40 by 25 alphanumeric mode.   */
	/*  Selects blue background color (C0-C1) in 320 by 200 graphics mode.  */
	/*  Selects blue foreground color in 640 by 200 graphics mode.          */
	/*  - Bit 1: Selects green border color in 40 by 25 alphanumeric mode.  */
	/*  Selects green background color (C0-C1) in 320 by 200 graphics mode. */
	/*  Selects green foreground color in 640 by 200 graphics mode.         */
	/*  - Bit 2: Selects red border color in 40 by 25 alphanumeric mode.    */
	/*  Selects red background color (C0-C1) in 320 by 200 graphics mode.   */
	/*  Selects red foreground color in 640 by 200 graphics mode.           */
	/*  - Bit 3: Selects intensified border color in 40 by 25 alphanumeric  */
	/*  mode. Selects intensified background color (C0-C1) in 320 by 200    */
	/*  graphics mode. Selects intensified foreground color in 640 by 200   */
	/*  graphics mode.                                                      */
	/*  - Bit 4:Selects alternate, intensified set of colors in the graphics*/
	/*  mode. Selects background colors in the alphanumeric mode.           */
	/*  - Bit 5: Selects active color set in 320 by 200 graphics mode.      */
	/*  When bit 5 is set to 1, colors are determined as follows:           */
	/*  C1:  C0:  Set Selected:                                             */
	/*   - 0: - 0: - Background (Defined by bits 0-3 of port hex 3D9)       */
	/*   - 0: - 1: - Cyan                                                   */
	/*   - 1: - 0: - Magenta                                                */
	/*   - 1: - 1: - White                                                  */
	/*  When bit 5 is set to 0, colors are determined as follows:           */
	/*  C1:  C0:  Set Selected:                                             */
	/*   - 0: - 0: - Background (Defined by bits 0-3 of port hex 3D9)       */
	/*   - 0: - 1: - Green                                                  */
	/*   - 1: - 0: - Red                                                    */
	/*   - 1: - 1: - Brown                                                  */
	/*  - Bit 6: Not used                                                   */
	/*  - Bit 7: Not used                                                   */
	/* -------------------------------------------------------------------- */
	compaq->reg[COMPAQ_CSEL] = val;

	compaq_set_palette (compaq);

	compaq->update_state |= COMPAQ_UPDATE_DIRTY;
}

/*
 * Get a CGA register
 */
unsigned char compaq_reg_get_uint8 (compaq_t *compaq, unsigned long addr)
{
	switch (addr) {
	case COMPAQ_CRTC_INDEX0: /* Present on CGA only */
	case COMPAQ_CRTC_INDEX:  /* Present on both MDA and CGA */
		return (compaq_get_crtc_index (compaq));

	case COMPAQ_CRTC_DATA0:  /* Present on CGA only */
	case COMPAQ_CRTC_DATA:   /* Present on both MDA and CGA */
		return (compaq_get_crtc_data (compaq));

	case COMPAQ_STATUS:      /* Present on both MDA and CGA */
		return (compaq_get_status (compaq));
	}

	return (0xff);
}

unsigned short compaq_reg_get_uint16 (compaq_t *compaq, unsigned long addr)
{
	unsigned short ret;

	ret = compaq_reg_get_uint8 (compaq, addr);
	/* ------------------------------------------------------------------------------------------- */
	/* Quote from John Elliott, Vintage PC pages, Compaq Portable/Deskpro, Video Output:           */
	/* ------------------------------------------------------------------------------------------- */
	/* The method used by the card to detect what resolution is very simple: if the CRTC character */
	/* height register is 13 (ie, characters are 14 rows) then it goes into the high resolution    */
	/* mode. If not, it behaves like a normal CGA.                                                 */
	/* ------------------------------------------------------------------------------------------- */
	if (compaq->hires_mono_font = 13);
	{
		/* IBM MDA resolution */
		ret = compaq_reg_get_uint8 (compaq, addr);
		ret |= compaq_reg_get_uint8 (compaq, addr + 1) << 8;
	}
	else
    {
		/* IBM CGA resolution */
		if ((addr + 1) < compaq->regblk->size) {
			ret |= compaq_reg_get_uint8 (compaq, addr + 1) << 8;
		}
	}
	return (ret);
}

/*
 * Set a CGA register
 */
void compaq_reg_set_uint8 (compaq_t *compaq, unsigned long addr, unsigned char val)
{
	switch (addr) {
	case COMPAQ_CRTC_INDEX0: /* Present on CGA only */
	case COMPAQ_CRTC_INDEX:  /* Present on both MDA and CGA */
		compaq_set_crtc_index (compaq, val);
		break;

	case COMPAQ_CRTC_DATA0:  /* Present on CGA only */
	case COMPAQ_CRTC_DATA:   /* Present on both MDA and CGA */
		compaq_set_crtc_data (compaq, val);
		break;

	case COMPAQ_MODE:        /* Present on both MDA and CGA */
		compaq_set_mode (compaq, val);
		break;

	case COMPAQ_CSEL:        /* Present on CGA only */
		compaq_set_color_select (compaq, val);
		break;

	case COMPAQ_PEN_RESET:   /* Present on CGA only */
		compaq->reg[COMPAQ_STATUS] &= ~COMPAQ_STATUS_PEN;
		break;

	case COMPAQ_PEN_SET:     /* Present on CGA only */
		compaq->reg[COMPAQ_STATUS] |= COMPAQ_STATUS_PEN;
		break;

	default:
		compaq->reg[addr] = val; /* Copied from CGA code instead of MDA code */
		break;
	}
}

void compaq_reg_set_uint16 (compaq_t *compaq, unsigned long addr, unsigned short val)
{
	/* ------------------------------------------------------------------------------------------- */
	/* Quote from John Elliott, Vintage PC pages, Compaq Portable/Deskpro, Video Output:           */
	/* ------------------------------------------------------------------------------------------- */
	/* The method used by the card to detect what resolution is very simple: if the CRTC character */
	/* height register is 13 (ie, characters are 14 rows) then it goes into the high resolution    */
	/* mode. If not, it behaves like a normal CGA.                                                 */
	/* ------------------------------------------------------------------------------------------- */
	if (compaq->hires_mono_font = 13);
	{
		/* IBM MDA resolution */
		compaq_reg_set_uint8 (compaq, addr + 0, val & 0xff);
		compaq_reg_set_uint8 (compaq, addr + 1, (val >> 8) & 0xff);
	}
	else
    {
		/* IBM CGA resolution */
		compaq_reg_set_uint8 (compaq, addr, val & 0xff);
	}
	if ((addr + 1) < compaq->regblk->size) {
		compaq_reg_set_uint8 (compaq, addr + 1, val >> 8);
	}
}


static
void compaq_mem_set_uint8 (compaq_t *compaq, unsigned long addr, unsigned char val)
{
	/* ------------------------------------------------------------------------------------------- */
	/* Quote from John Elliott, Vintage PC pages, Compaq Portable/Deskpro, Video Output:           */
	/* ------------------------------------------------------------------------------------------- */
	/* The method used by the card to detect what resolution is very simple: if the CRTC character */
	/* height register is 13 (ie, characters are 14 rows) then it goes into the high resolution    */
	/* mode. If not, it behaves like a normal CGA.                                                 */
	/* ------------------------------------------------------------------------------------------- */
	if (compaq->hires_mono_font = 13);
	{
		/* IBM MDA resolution */
		addr &= 4095;
		if (compaq->mem[addr] == val) {
			return;
		}
	}
	else
    {
		/* IBM CGA resolution */
		if (compaq->mem[addr] == val) {
			return;
		}
	}
	compaq->mem[addr] = val;
	compaq->update_state |= COMPAQ_UPDATE_DIRTY;
}

static
void compaq_mem_set_uint16 (compaq_t *compaq, unsigned long addr, unsigned short val)
{
	/* ------------------------------------------------------------------------------------------- */
	/* Quote from John Elliott, Vintage PC pages, Compaq Portable/Deskpro, Video Output:           */
	/* ------------------------------------------------------------------------------------------- */
	/* The method used by the card to detect what resolution is very simple: if the CRTC character */
	/* height register is 13 (ie, characters are 14 rows) then it goes into the high resolution    */
	/* mode. If not, it behaves like a normal CGA.                                                 */
	/* ------------------------------------------------------------------------------------------- */
	if (compaq->hires_mono_font = 13);
	{
		/* IBM MDA resolution */
		compaq_mem_set_uint8 (compaq, addr + 0, val & 0xff);
	}
	else
    {
		/* IBM CGA resolution */
		compaq_mem_set_uint8 (compaq, addr, val & 0xff);
	}
	if ((addr + 1) < compaq->memblk->size) {
		compaq_mem_set_uint8 (compaq, addr + 1, (val >> 8) & 0xff);
	}
}


static
int compaq_set_msg (compaq_t *compaq, const char *msg, const char *val)
{
	if (msg_is_message ("emu.video.blink", msg)) {
		unsigned freq;
		if (msg_get_uint (val, &freq)) {
			return (1);
		}
		compaq_set_blink_rate (compaq, freq);
		return (0);
	}
	return (-1);
}

static
void compaq_set_terminal (compaq_t *compaq, terminal_t *trm)
{
	compaq->term = trm;
	/* ------------------------------------------------------------------------------------------- */
	/* Quote from John Elliott, Vintage PC pages, Compaq Portable/Deskpro, Video Output:           */
	/* ------------------------------------------------------------------------------------------- */
	/* The method used by the card to detect what resolution is very simple: if the CRTC character */
	/* height register is 13 (ie, characters are 14 rows) then it goes into the high resolution    */
	/* mode. If not, it behaves like a normal CGA.                                                 */
	/* ------------------------------------------------------------------------------------------- */
    if (compaq->term != NULL) {
		if (compaq->hires_mono_font = 13);
		{
			/* IBM MDA resolution */
			trm_open (compaq->term, 720, 350);
		}
		else
		{
			/* IBM CGA resolution */
			trm_open (compaq->term, 640, 200);
		}
	}
}

static
mem_blk_t *compaq_get_mem (compaq_t *compaq)
{
	return (compaq->memblk);
}

static
mem_blk_t *compaq_get_reg (compaq_t *compaq)
{
	return (compaq->regblk);
}

static
void compaq_print_info (compaq_t *compaq, FILE *fp)
{
	unsigned i;

	fprintf (fp, "DEV: COMPAQ Video Display Unit\n");

	/* fprintf (fp, "COMPAQMDAMODE5140: OFS=%04X  POS=%04X\n", */ /* Disabled MDA function */
	fprintf (fp, "COMPAQ: OFS=%04X  POS=%04X  BG=%02X  PAL=%u\n",
		compaq_get_start (compaq),  /* Applies to MDA and CGA */
		compaq_get_cursor (compaq), /* Applies to MDA and CGA */
		/* -------------------------------------------------------------------- */
		/* Color-Select Register:                                               */
		/* -------------------------------------------------------------------- */
		/* The color-select register is a 6-bit output-only register. Its I/O   */
		/* address is hex 3D9, and it can be written to using a processor 'out' */
		/* command. The following are the bit definitions for this register.    */
		/*  - Bit 0: Selects blue border color in 40 by 25 alphanumeric mode.   */
		/*  Selects blue background color (C0-C1) in 320 by 200 graphics mode.  */
		/*  Selects blue foreground color in 640 by 200 graphics mode.          */
		/*  - Bit 1: Selects green border color in 40 by 25 alphanumeric mode.  */
		/*  Selects green background color (C0-C1) in 320 by 200 graphics mode. */
		/*  Selects green foreground color in 640 by 200 graphics mode.         */
		/*  - Bit 2: Selects red border color in 40 by 25 alphanumeric mode.    */
		/*  Selects red background color (C0-C1) in 320 by 200 graphics mode.   */
		/*  Selects red foreground color in 640 by 200 graphics mode.           */
		/*  - Bit 3: Selects intensified border color in 40 by 25 alphanumeric  */
		/*  mode. Selects intensified background color (C0-C1) in 320 by 200    */
		/*  graphics mode. Selects intensified foreground color in 640 by 200   */
		/*  graphics mode.                                                      */
		/*  - Bit 4:Selects alternate, intensified set of colors in the graphics*/
		/*  mode. Selects background colors in the alphanumeric mode.           */
		/*  - Bit 5: Selects active color set in 320 by 200 graphics mode.      */
		/*  When bit 5 is set to 1, colors are determined as follows:           */
		/*  C1:  C0:  Set Selected:                                             */
		/*   - 0: - 0: - Background (Defined by bits 0-3 of port hex 3D9)       */
		/*   - 0: - 1: - Cyan                                                   */
		/*   - 1: - 0: - Magenta                                                */
		/*   - 1: - 1: - White                                                  */
		/*  When bit 5 is set to 0, colors are determined as follows:           */
		/*  C1:  C0:  Set Selected:                                             */
		/*   - 0: - 0: - Background (Defined by bits 0-3 of port hex 3D9)       */
		/*   - 0: - 1: - Green                                                  */
		/*   - 1: - 0: - Red                                                    */
		/*   - 1: - 1: - Brown                                                  */
		/*  - Bit 6: Not used                                                   */
		/*  - Bit 7: Not used                                                   */
		/* -------------------------------------------------------------------- */
		compaq->reg[COMPAQ_CSEL] & 0x0f,    /* Applies to CGA only */
		(compaq->reg[COMPAQ_CSEL] >> 5) & 1 /* Applies to CGA only */
	);
	/* -------------------------------------------------------------------- */
	/* Timing Generator:                                                    */
	/* -------------------------------------------------------------------- */
	/* This generator produces the timing signals used by the 6845 CRT      */
	/* Controller and by the dynamic memory. It also solves the processor/  */
	/* graphic controller contentions for access to the display buffer.     */
	/* -------------------------------------------------------------------- */
	fprintf (fp, "CLK: CLK=%lu  HT=%lu HD=%lu  VT=%lu VD=%lu\n",
		compaq_get_dotclock (compaq),
		compaq->clk_ht, compaq->clk_hd,
		compaq->clk_vt, compaq->clk_vd
	);

	/* fprintf (fp, "REG: CRTC=%02X  MODE=%02X  STATUS=%02X\n", */ /* Disabled MDA function */
	fprintf (fp,
		"REG: MODE=%02X  CSEL=%02X  STATUS=%02X"
		"  PAL=[%02X %02X %02X %02X]\n",
		/* -------------------------------------------------------------------- */
		/* Mode-Control Register:                                               */
		/* -------------------------------------------------------------------- */
		/* The mode-control register is a 6-bit output-only register. Its I/O   */
		/* address is hex 3D8, and it can be written to using a processor 'out' */
		/* command. The following are bit definitions for this register.        */
		/*  - Bit 0: A 1 selects  80 by 25 alphanumeric mode.                   */
		/*           A 0 selects  40 by 25 alphanumeric mode.                   */
		/*  - Bit 1: A 1 selects 320 by 200 graphics    mode.                   */
		/*           A 0 selects           alphanumeric mode.                   */
		/*  - Bit 2: A 1 selects black-and-white        mode.                   */
		/*           A 0 selects color                  mode.                   */
		/*  - Bit 3: A 1 enables the video signal. The video signal is disabled */
		/*           when changing modes.                                       */
		/*  - Bit 4: A 1 selects the high-resolution (640 by 200) black-and-    */
		/*           white graphics mode. One of eight colors can be selected on*/
		/*           direct-drive monitors in this mode by using register hex   */
		/*           3D9.                                                       */
		/*  - Bit 5: A 1 will change the character background intensity to the  */
		/*           blinking attribute function for alphanumeric modes. When   */
		/*           the high-order attribute is not selected, 16 background    */
		/*           colors or intensified colors are available. This bit is set*/
		/*           to 1 to allow the blinking function.                       */
		/* -------------------------------------------------------------------- */
		/* Mode-Control Register Summary:                                       */
		/* -------------------------------------------------------------------- */
		/* Bits:                                                                */
		/* 0   :1   :2   :3   :4   :5   : 40 x 25 Alphanumeric Black-and-White  */
		/*  - 0: - 0: - 1: - 1: - 0: - 1: 40 x 25 Alphanumeric Color            */
		/*  - 0: - 0: - 0: - 1: - 0: - 1: 80 x 25 Alphanumeric Black-and-White  */
		/*  - 1: - 0: - 1: - 1: - 0: - 1: 80 x 25 Alphanumeric Color            */
		/*  - 1: - 0: - 0: - 1: - 0: - 1: 320 x 200 Black-and-White Graphics    */
		/*  - 0: - 1: - 1: - 1: - 0: - z: 320 x 200 Color Graphics              */
		/*  - 0: - 1: - 0: - 1: - 0: - z: 640 x 200 Black-and-White Graphics    */
		/* -------------------------------------------------------------------- */
		/*  - 5: Enable Blink Attribute                                         */
		/*  - 4: 640 x 200 Black-and-White                                      */
		/*  - 3: Enable Video Signal                                            */
		/*  - 2: Select Black-and-White Mode                                    */
		/*  - 1: Select 320 x 200 Graphics                                      */
		/*  - 0: 80 x 25 Alphanumeric Select                                    */
		/* z = don't care condition                                             */
		/* Note: The low-resolution (160 by 100) mode requires special          */
		/* programming and is set up as the 40 by 25 alphanumeric mode.         */
		/* -------------------------------------------------------------------- */
		/* ------------------------------------------------------------------------------------------- */
		/* Quote from John Elliott, Vintage PC pages, Compaq Portable/Deskpro, Video Output:           */
		/* ------------------------------------------------------------------------------------------- */
		/* The method used by the card to detect what resolution is very simple: if the CRTC character */
		/* height register is 13 (ie, characters are 14 rows) then it goes into the high resolution    */
		/* mode. If not, it behaves like a normal CGA.                                                 */
		/* ------------------------------------------------------------------------------------------- */
		if (compaq->hires_mono_font = 13);
		{
			/* IBM MDA resolution */
			compaq->reg[COMPAQ_CRTC_INDEX], compaq_get_mode (compaq),
		}
		else
		{
			/* IBM CGA resolution */
			compaq->reg[COMPAQ_MODE],
		}
		/* -------------------------------------------------------------------- */
		/* Sequence of Events for Changing Modes:                               */
		/* -------------------------------------------------------------------- */
		/* - 1. Determine the mode of operation.                                */
		/* - 2. Reset the video-enable bit in the mode-control register.        */
		/* - 3. Program the 6845 CRT controller to select the mode.             */
		/* - 4. Program the mode-control and color-select registers including   */
		/* re-enabling the video.                                               */
		/* -------------------------------------------------------------------- */
		compaq->reg[COMPAQ_CSEL],
		/* -------------------------------------------------------------------- */
		/* Status Register:                                                     */
		/* -------------------------------------------------------------------- */
		/* The status register is a 4-bit read-only register. Its I/O address is*/
		/* hex 3DA, and it can be read using the processor 'in' instruction. The*/
		/* following are bit definitions for this register.                     */
		/*  - Bit 0: A 1 indicates that a regen-buffer memory access can be made*/
		/*  without interfering with the display.                               */
		/*  - Bit 1: A 1 indicates that a positive-going edge from the light pen*/
		/*  has set the light pen's trigger. This trigger is reset when power is*/
		/*  turned on and may also be cleared by a processor 'out' command to   */
		/*  hex address 3DB. No specific data setting is required; the action is*/
		/*  address-activated.                                                  */
		/*  - Bit 2: The light pen switch is reflected in this bit. The switch  */
		/*  is not latched or debounced. A 0 indicates that the switch is on.   */
		/*  - Bit 3: A 1 indicates that the raster is in a vertical retrace     */
		/*  mode. Screen-buffer updating can be performed at this time.         */
		/* -------------------------------------------------------------------- */
		/* ------------------------------------------------------------------------------------------- */
		/* Quote from John Elliott, Vintage PC pages, Compaq Portable/Deskpro, Video Output:           */
		/* ------------------------------------------------------------------------------------------- */
		/* The method used by the card to detect what resolution is very simple: if the CRTC character */
		/* height register is 13 (ie, characters are 14 rows) then it goes into the high resolution    */
		/* mode. If not, it behaves like a normal CGA.                                                 */
		/* ------------------------------------------------------------------------------------------- */
		if (compaq->hires_mono_font = 13);
		{
			/* IBM MDA resolution */
			compaq_get_status (compaq), /* Remove '','' if compiler breaks */
		}
		else
		{
			/* IBM CGA resolution */
			compaq->reg[COMPAQ_STATUS],
		}
		compaq->pal[0], compaq->pal[1], compaq->pal[2], compaq->pal[3]
	);

	fprintf (fp, "CRTC=[%02X", compaq->reg_crt[0]);
	for (i = 1; i < 18; i++) {
		/* ------------------------------------------------------------------------------------------- */
		/* Quote from John Elliott, Vintage PC pages, Compaq Portable/Deskpro, Video Output:           */
		/* ------------------------------------------------------------------------------------------- */
		/* The method used by the card to detect what resolution is very simple: if the CRTC character */
		/* height register is 13 (ie, characters are 14 rows) then it goes into the high resolution    */
		/* mode. If not, it behaves like a normal CGA.                                                 */
		/* ------------------------------------------------------------------------------------------- */
		if (compaq->hires_mono_font = 13);
		{
			/* IBM MDA resolution */
			if ((i & 7) == 0) {
				fputs ("-", fp);
			}
			else {
				fputs (" ", fp);
			}
		}
		else
		{
			/* IBM CGA resolution */
			fputs ((i & 7) ? " " : "-", fp);
		}
		fprintf (fp, "%02X", compaq->reg_crt[i]);
	}
	fputs ("]\n", fp);
	fflush (fp);
}

/*
 * Force a screen update
 */
static
void compaq_redraw (compaq_t *compaq, int now)
{
	if (now) {
		if (compaq->term != NULL) {
			compaq->update (compaq);

			trm_set_size (compaq->term, compaq->buf_w, compaq->buf_h);
			trm_set_lines (compaq->term, compaq->buf, 0, compaq->buf_h);
			trm_update (compaq->term);
		}
	}
	compaq->update_state |= COMPAQ_UPDATE_DIRTY;
}

static
void compaq_clock (compaq_t *compaq, unsigned long cnt)
{
	unsigned long clk;
	/* -------------------------------------------------------------------- */
	/* Timing Generator:                                                    */
	/* -------------------------------------------------------------------- */
	/* This generator produces the timing signals used by the 6845 CRT      */
	/* Controller and by the dynamic memory. It also solves the processor/  */
	/* graphic controller contentions for access to the display buffer.     */
	/* -------------------------------------------------------------------- */
	if (compaq->clk_vt < 50000) {
		return;
	}
	clk = compaq_get_dotclock (compaq);
	if (clk < compaq->clk_vd) {
		compaq->update_state &= ~COMPAQ_UPDATE_RETRACE;
		return;
	}
	if (clk >= compaq->clk_vt) {
		compaq->video.dotclk[0] = 0;
		compaq->video.dotclk[1] = 0;
		compaq->video.dotclk[2] = 0;
	}
	if (compaq->update_state & COMPAQ_UPDATE_RETRACE) {
		return;
	}
	if (compaq->blink_cnt > 0) {
		compaq->blink_cnt -= 1;

		if (compaq->blink_cnt == 0) {
			compaq->blink_cnt = compaq->blink_freq;
			compaq->blink_on = !compaq->blink_on;
			if ((compaq->reg[COMPAQ_MODE] & COMPAQ_MODE_G320) == 0) {
				compaq->update_state |= COMPAQ_UPDATE_DIRTY;
			}
		}
	}
	if (compaq->term != NULL) {
		if (compaq->update_state & COMPAQ_UPDATE_DIRTY) {
			compaq->update (compaq);
			trm_set_size (compaq->term, compaq->buf_w, compaq->buf_h);
			trm_set_lines (compaq->term, compaq->buf, 0, compaq->buf_h);
		}
		trm_update (compaq->term);
	}
	compaq->update_state = COMPAQ_UPDATE_RETRACE;
}

void compaq_free (compaq_t *compaq)
{
	mem_blk_del (compaq->memblk);
	mem_blk_del (compaq->regblk);
	free (compaq->comp_tab);
}

void compaq_del (compaq_t *compaq)
{
	if (compaq != NULL) {
		compaq_free (compaq);
		free (compaq);
	}
}

static
void compaq_set_font (compaq_t *compaq, unsigned font)
{
	/* -------------------------------------------------------------------- */
	/* Character Generator:                                                 */
	/* -------------------------------------------------------------------- */
	/* A ROS character generator is used with 8K bytes of storage that      */
	/* cannot be read from or written to under program control. This is a   */
	/* general-purpose ROS character generator with three character fonts.  */
	/* Two character fonts are used on the Color/Graphics Monitor Adapter: a*/
	/* 7-high by 7-wide double-dot font and a 7-high by 5-wide single-dot   */
	/* font. The font is selected by a jumper (P3). The single-dot font is  */
	/* selected by inserting the jumper; the double-dot font is selected by */
	/* removing the jumper.                                                 */
	/* -------------------------------------------------------------------- */
    /* ------------------------------------------------------------------------------------------- */
    /* Quote from John Elliott, Vintage PC pages, Compaq Portable/Deskpro, Video Output:           */
    /* ------------------------------------------------------------------------------------------- */
    /* The method used by the card to detect what resolution is very simple: if the CRTC character */
    /* height register is 13 (ie, characters are 14 rows) then it goes into the high resolution    */
    /* mode. If not, it behaves like a normal CGA.                                                 */
    /* ------------------------------------------------------------------------------------------- */
	if (compaq->hires_mono_font = 13);
	{
		/* IBM MDA resolution */
		fprintf ("ERROR: Unable to load CGA font while in high-resolution mode.");
	}
	else
	{
		/* IBM CGA resolution */
		if (font == 0) {
			memcpy (compaq->font, compaq_font_thick, 2048);
		}
		else {
			memcpy (compaq->font, compaq_font_thin, 2048);
		}
	}
}

static
int compaq_load_font (compaq_t *compaq, const char *fname)
{
	/* -------------------------------------------------------------------- */
	/* Character Generator:                                                 */
	/* -------------------------------------------------------------------- */
	/* A ROS character generator is used with 8K bytes of storage that      */
	/* cannot be read from or written to under program control. This is a   */
	/* general-purpose ROS character generator with three character fonts.  */
	/* Two character fonts are used on the Color/Graphics Monitor Adapter: a*/
	/* 7-high by 7-wide double-dot font and a 7-high by 5-wide single-dot   */
	/* font. The font is selected by a jumper (P3). The single-dot font is  */
	/* selected by inserting the jumper; the double-dot font is selected by */
	/* removing the jumper.                                                 */
	/* -------------------------------------------------------------------- */
    /* ------------------------------------------------------------------------------------------- */
    /* Quote from John Elliott, Vintage PC pages, Compaq Portable/Deskpro, Video Output:           */
    /* ------------------------------------------------------------------------------------------- */
    /* The method used by the card to detect what resolution is very simple: if the CRTC character */
    /* height register is 13 (ie, characters are 14 rows) then it goes into the high resolution    */
    /* mode. If not, it behaves like a normal CGA.                                                 */
    /* ------------------------------------------------------------------------------------------- */
	if (compaq->hires_mono_font = 13);
	{
		/* IBM MDA resolution */
		fprintf ("ERROR: Unable to load CGA font while in high-resolution mode.");
	}
	else
	{
		/* IBM CGA resolution */
		FILE *fp;
		fp = fopen (fname, "rb");
		if (fp == NULL) {
			return (1);
		}
		if (fread (compaq->font, 1, 2048, fp) != 2048) {
			fclose (fp);
			return (1);
		}
		fclose (fp);
		return (0);
	}
}

/* compaq_t *compaq_new (unsigned long io, unsigned long mem, unsigned long size) */ /* Disabled MDA functions */
void compaq_init (compaq_t *compaq, unsigned long io, unsigned long addr, unsigned long size)
{
	unsigned i;
/*	compaq_t    *compaq; /* Disabled MDA functions */
	/* compaq = malloc (sizeof (compaq_t));
	if (compaq == NULL) {
		return (NULL);
	} */
	pce_video_init (&compaq->video);

	compaq->video.ext = compaq;
	compaq->video.del = (void *) compaq_del;
	compaq->video.set_msg = (void *) compaq_set_msg;
	compaq->video.set_terminal = (void *) compaq_set_terminal;
	compaq->video.get_mem = (void *) compaq_get_mem;
	compaq->video.get_reg = (void *) compaq_get_reg;
	compaq->video.print_info = (void *) compaq_print_info;
	compaq->video.redraw = (void *) compaq_redraw;
	compaq->video.clock = (void *) compaq_clock;

	compaq->ext = NULL;
	compaq->update = compaq_update;

	/* if (size < 4096) { /* Disabled MDA functions */
		/* size = 4096;
	} */
	size = (size < 16384) ? 16384UL : size;

	/* compaq->memblk = mem_blk_new (mem, size, 0); */ /* Disabled MDA address */
	/* compaq->memblk->ext = compaq; */
	/* compaq->memblk->set_uint8 = (void *) compaq_mem_set_uint8; */
	/* compaq->memblk->set_uint16 = (void *) compaq_mem_set_uint16; */
	/* compaq->memblk->get_uint8 = (void *) compaq_mem_get_uint8; */
	/* compaq->memblk->get_uint16 = (void *) compaq_mem_get_uint16; */
	/* memset (compaq->mem, 0, sizeof (compaq->mem)); */
	compaq->memblk = mem_blk_new (addr, size, 1);
	compaq->memblk->ext = compaq;
	compaq->memblk->set_uint8 = (void *) compaq_mem_set_uint8;
	compaq->memblk->set_uint16 = (void *) &compaq_mem_set_uint16;
	compaq->mem = compaq->memblk->data;
	mem_blk_clear (compaq->memblk, 0x00);

	compaq->regblk = mem_blk_new (io, 16, 1);
	compaq->regblk->ext = compaq;
	compaq->regblk->set_uint8 = (void *) compaq_reg_set_uint8;
	compaq->regblk->set_uint16 = (void *) compaq_reg_set_uint16;
	compaq->regblk->get_uint8 = (void *) compaq_reg_get_uint8;
	compaq->regblk->get_uint16 = (void *) compaq_reg_get_uint16;
	compaq->reg = compaq->regblk->data;
	mem_blk_clear (compaq->regblk, 0x00);

	compaq->term = NULL;

	for (i = 0; i < 18; i++) {
		compaq->reg_crt[i] = 0;
	}
	compaq->pal[0] = 0;
	compaq->pal[1] = 11;
	compaq->pal[2] = 13;
	compaq->pal[3] = 15;
	/* -------------------------------------------------------------------- */
	/* Alphanumeric Mode:                                                   */
	/* -------------------------------------------------------------------- */
	/* Every display character position in the alphanumeric mode is defined */
	/* by two bytes in regen buffer (a part of the monitor adapter), not the*/
	/* system memory. Both the Color/Graphics Monitor Adapter and the       */
	/* the Monochrome Display and Printer Adapter use the following 2-byte  */
	/* character-attribute format.                                          */
	/*                                                                      */
	/* Display-Character Code Byte:     Attribute Byte:                     */
	/*  - 7 - 6 - 5 - 4 - 3 - 2 - 1 - 0  - 7 - 6 - 5 - 4 - 3 - 2 - 1 - 0    */
	/*                                                                      */
	/* The following table shows the functions of the attribute byte.       */
	/* -------------------------------------------------------------------- */
	/* Attribute Function/Attribute Byte:                                   */
	/* -------------------------------------------------------------------- */
	/* I = Highlighted Foreground (Character):                              */
	/* B = Blinking    Foreground (Character):                              */
	/* Normal             (Background): Normal             (Foreground):    */
	/*  - 7 : 6 5 4                      - 3 : 2 1 0                        */
	/*  - B : R G B                      - I : R G B                        */
	/*  - FG: Background                 - Foreground:                      */
	/*  - B : 0 0 0                      - I : 1 1 1                        */
	/* Reverse Video      (Background): Reverse Video      (Foreground):    */
	/*  - 7 : 6 5 4                      - 3 : 2 1 0                        */
	/*  - B : R G B                      - I : R G B                        */
	/*  - FG: Background                 - Foreground:                      */
	/*  - B : 1 1 1                      - I : 0 0 0                        */
	/* Nondisplay (Black) (Background): Nondisplay (Black) (Foreground):    */
	/*  - 7 : 6 5 4                      - 3 : 2 1 0                        */
	/*  - B : R G B                      - I : R G B                        */
	/*  - FG: Background                 - Foreground:                      */
	/*  - B : 0 0 0                      - I : 0 0 0                        */
	/* Nondisplay (White) (Background): Nondisplay (White) (Foreground):    */
	/*  - 7 : 6 5 4                      - 3 : 2 1 0                        */
	/*  - B : R G B                      - I : R G B                        */
	/*  - FG: Background                 - Foreground:                      */
	/*  - B : 1 1 1                      - I : 1 1 1                        */
	/*                                                                      */
	/* The definitions of the attribute byte are in the following table.    */
	/*  - 7: B: Blinking                                                    */
	/*  - 6: R:                                                             */
	/*  - 5: G: Background Color                                            */
	/*  - 4: B:                                                             */
	/*  - 3: I: Intensity                                                   */
	/*  - 2: R:                                                             */
	/*  - 1: G: Foreground                                                  */
	/*  - 0: B:                                                             */
	/* In the alphanumeric mode, the display can be operated in either a low*/
	/* -resolution mode or a high-resolution mode.                          */
	/*                                                                      */
	/* The low-resolution alphanumeric mode has the following features:     */
	/*  - Supports home color televisions or low-resolution monitors.       */
	/*  - Displays up to 25 rows of 40 characters each.                     */
	/*  - Has a ROS character generator that contains dot patterns for a    */
	/*  maximum of 256 different characters.                                */
	/*  - Requires 2,000 bytes of read/write memory (on the adapter).       */
	/*  - Has an 8-high by 8-wide character box.                            */
	/*  - Has two jumper-controlled character fonts available: a 7-high by 5*/
	/*  -wide single-dot character font with one descender, and a 7-high by */
	/*  7-wide double-dotted character font with one descender.             */
	/*  - Has one character attribute for each character.                   */
	/* The high-resolution alphanumeric mode has the following features:    */
	/*  - Supports the IBM Color Display or other color monitor with direct-*/
	/*  drive capability.                                                   */
	/*  - Supports a black-and-white composite-video monitor.               */
	/*  - Displays up to 25 rows of 80 characters each.                     */
	/*  - Has a ROS display generator that contains dot patterns for a      */
	/*  maximum of 256 characters.                                          */
	/*  - Requires 4,000 bytes of read/write memory (on the adapter).       */
	/*  - Has an 8-high by 8-wide character box.                            */
	/*  - Has two jumper-controlled character fonts available: a 7-high by 5*/
	/*  -wide single-dot character font with one descender, and a 7-high by */
	/*  7-wide double-dot character font with one descender.                */
	/*  - Has one character attribute for each character.                   */
	/* The Color/Graphics Monitor Adapter will change foreground and        */
	/* background colors according to the color value selected in the       */
	/* attribute byte. The following figure shows the color values for the  */
	/* various red, green, blue, and intensity bit settings.                */
	/*  - Black                 : Red: 0 Green: 0 Blue: 0 Intensity: 0      */
	/*  - Blue                  : Red: 0 Green: 0 Blue: 1 Intensity: 0      */
	/*  - Green                 : Red: 0 Green: 1 Blue: 0 Intensity: 0      */
	/*  - Cyan                  : Red: 0 Green: 1 Blue: 1 Intensity: 0      */
	/*  - Red                   : Red: 1 Green: 0 Blue: 0 Intensity: 0      */
	/*  - Magenta               : Red: 1 Green: 0 Blue: 1 Intensity: 0      */
	/*  - Brown                 : Red: 1 Green: 1 Blue: 0 Intensity: 0      */
	/*  - White                 : Red: 1 Green: 1 Blue: 1 Intensity: 0      */
	/*  - Gray                  : Red: 0 Green: 0 Blue: 0 Intensity: 1      */
	/*  - Light Blue            : Red: 0 Green: 0 Blue: 1 Intensity: 1      */
	/*  - Light Green           : Red: 0 Green: 1 Blue: 0 Intensity: 1      */
	/*  - Light Cyan            : Red: 0 Green: 1 Blue: 1 Intensity: 1      */
	/*  - Light Red             : Red: 1 Green: 0 Blue: 0 Intensity: 1      */
	/*  - Light Magenta         : Red: 1 Green: 0 Blue: 1 Intensity: 1      */
	/*  - Yellow                : Red: 1 Green: 1 Blue: 0 Intensity: 1      */
	/*  - White (High Intensity): Red: 1 Green: 1 Blue: 1 Intensity: 1      */
	/* Note: Not all monitors recognize the intensity (I) bit.              */
	/* -------------------------------------------------------------------- */
	compaqcgamode->comp_tab_ok = 0;
	compaqcgamode->comp_tab = NULL;

	compaq->font = compaq_font_8x14; /* Applies to MDA only */

	compaq_set_color (compaq, 0, 0, 0x00, 0x00, 0x00);  /* Applies to MDA only */
	compaq_set_color (compaq, 1, 7, 0x00, 0xaa, 0x00);  /* Applies to MDA only */
	compaq_set_color (compaq, 8, 15, 0xaa, 0xff, 0xaa); /* Applies to MDA only */

	compaq->blink_on = 1;    /* Continued from CGA code */
	compaq->blink_cnt = 0;   /* Continued from CGA code */
	compaq->blink_freq = 16; /* Continued from CGA code */

	compaq->w = 0;
	compaq->h = 0;
	compaq->ch = 0;
	/* -------------------------------------------------------------------- */
	/* Timing Generator:                                                    */
	/* -------------------------------------------------------------------- */
	/* This generator produces the timing signals used by the 6845 CRT      */
	/* Controller and by the dynamic memory. It also solves the processor/  */
	/* graphic controller contentions for access to the display buffer.     */
	/* -------------------------------------------------------------------- */
	compaq->clk_ht = 0;
	compaq->clk_vt = 0;
	compaq->clk_hd = 0;
	compaq->clk_vd = 0;
	/* -------------------------------------------------------------------- */
	/* Display Buffer:                                                      */
	/* -------------------------------------------------------------------- */
	/* The display buffer resides in the processor-address space, starting  */
	/* at address hex B8000. It provides 16 bytes of dynamic read/write     */
	/* memory. A dual-ported implementation allows the processor and the    */
	/* graphics control unit access to this buffer. The processor and the   */
	/* control unit have equal access to this buffer during all modes of    */
	/* operation, except in the high-resolution alphanumeric mode. In this  */
	/* mode, only the processor should have access to this buffer during the*/
	/* horizontal-retrace intervals. While the processor may write to the   */
	/* required buffer at any time, a small amount of display interference  */
	/* will result if this does not occur during the horizontal-retrace     */
	/* intervals.                                                           */
	/* -------------------------------------------------------------------- */
	compaq->bufmax = 0;
	compaq->buf = NULL;

	compaq->update_state = 0;

	compaq_set_font (compaq, 0); /* Applies to CGA only */
	/* return (compaq); */ /* Disabled MDA code */
}

compaq_t *compaq_new (unsigned long io, unsigned long addr, unsigned long size)
{
	compaq_t *compaq;
	compaq = malloc (sizeof (compaq_t));
	if (compaq == NULL) {
		return (NULL);
	}
	compaq_init (compaq, io, addr, size);
	return (compaq);
}

video_t *compaq_new_ini (ini_sct_t *sct)
{
	unsigned long io, addr, size;
	unsigned long col0, col1, col2; /* MDA functions */
	const char    *col; /* MDA functions */
	unsigned      font;
	unsigned      blink;
	const char    *fontfile;
	compaq_t         *compaq;

	/* ini_get_uint32 (sct, "io", &io, 0x3b4); */ /* Disabled MDA address */
	ini_get_uint32 (sct, "io", &io, 0x3d0);
	/* -------------------------------------------------------------------- */
	/* Display Buffer:                                                      */
	/* -------------------------------------------------------------------- */
	/* The display buffer resides in the processor-address space, starting  */
	/* at address hex B8000. It provides 16 bytes of dynamic read/write     */
	/* memory. A dual-ported implementation allows the processor and the    */
	/* graphics control unit access to this buffer. The processor and the   */
	/* control unit have equal access to this buffer during all modes of    */
	/* operation, except in the high-resolution alphanumeric mode. In this  */
	/* mode, only the processor should have access to this buffer during the*/
	/* horizontal-retrace intervals. While the processor may write to the   */
	/* required buffer at any time, a small amount of display interference  */
	/* will result if this does not occur during the horizontal-retrace     */
	/* intervals.                                                           */
	/* -------------------------------------------------------------------- */
	/* Memory Requirements:                                                 */
	/* -------------------------------------------------------------------- */
	/* The memory used by this adapter is self-contained. It consists of 16K*/
	/* bytes of memory without parity. This memory is used as both a display*/
	/* buffer for alphanumeric data and as a bit map for graphics data. The */
	/* regen buffer's address starts at hex B8000.                          */
	/*                                                                      */
	/* Read/Write Memory Address Space (in hex):                            */
	/*  - 01000: System Read/Write Memory                                   */
	/*  - A0000: System Read/Write Memory   (128K Reserved Regen Area)      */
	/*  - B8000: Display Buffer (16K Bytes) (128K Reserved Regen Area)      */
	/*  - BC000: Display Buffer (16K Bytes) (128K Reserved Regen Area)      */
	/*  - C0000:                            (128K Reserved Regen Area)      */
	/* -------------------------------------------------------------------- */
	/* ini_get_uint32 (sct, "address", &addr, 0xb0000); */ /* Disabled MDA address */
	ini_get_uint32 (sct, "address", &addr, 0xb8000);
	/* ini_get_uint32 (sct, "size", &size, 32768); */      /* Disabled MDA address */
	ini_get_uint32 (sct, "size", &size, 16384);
	/* -------------------------------------------------------------------- */
	/* Basic Operations:                                                    */
	/* -------------------------------------------------------------------- */
	/* In the alphanumeric mode, the adapter fetches character and attribute*/
	/* information from its display buffer. The starting address of the     */
	/* display buffer is programmable through the CRT controller, but it    */
	/* must be an even address. The character codes and attributes are then */
	/* displayed according to their relative positions in the buffer. The   */
	/* The following addresses will produce an "AB" in the upper-left corner*/
	/* of a 40 by 25 screen and an "X" in the lower-right corner.           */
	/* -------------------------------------------------------------------- */
	/* (Even)    Memory   Display Buffer: (Example of a 40 by 25 Screen):   */
	/* Starting  Address                    - AB                            */
	/* Address: (in hex):                  Video Screen:                    */
	/*  - B8000:           - Character Code A                               */
	/*  - B8001:           - Attribute      A                               */
	/*  - B8002:           - Character Code B                               */
	/*  - B8003:           - Attribute      B                               */
	/* Last Address:                                                        */
	/*  - B87CE:           - Character Code X                               */
	/*  - B87CF:           - Attribute      X                               */
	/* -------------------------------------------------------------------- */
	/* The processor and the display control unit have equal access to the  */
	/* display buffer during all of the operating modes, except the high-   */
	/* resolution alphanumeric mode. During this mode, the processor gains  */
	/* access to the display buffer during the vertical retrace time. If it */
	/* does not, the display will be affected with random patterns as the   */
	/* microprocessor is using the display buffer. In the alphanumeric mode,*/
	/* the characters are displayed from a pre-stored ROM character         */
	/* generator that contains the dot patterns for all of the displayable  */
	/* characters.                                                          */
	/*                                                                      */
	/* In the graphics mode, the displayed dots and colors, up to 16K bytes,*/
	/* are fetched from the display buffer.                                 */
	/* -------------------------------------------------------------------- */
	/* Character Generator:                                                 */
	/* -------------------------------------------------------------------- */
	/* A ROS character generator is used with 8K bytes of storage that      */
	/* cannot be read from or written to under program control. This is a   */
	/* general-purpose ROS character generator with three character fonts.  */
	/* Two character fonts are used on the Color/Graphics Monitor Adapter: a*/
	/* 7-high by 7-wide double-dot font and a 7-high by 5-wide single-dot   */
	/* font. The font is selected by a jumper (P3). The single-dot font is  */
	/* selected by inserting the jumper; the double-dot font is selected by */
	/* removing the jumper.                                                 */
	/* -------------------------------------------------------------------- */
	ini_get_uint16 (sct, "font", &font, 0);        /* Applies to CGA only */
	ini_get_uint16 (sct, "blink", &blink, 0);      /* Applies to MDA and CGA */
	ini_get_string (sct, "file", &fontfile, NULL); /* Applies to CGA only */

	pce_log_tag (MSG_INF,
		/* "VIDEO:", "COMPAQ io=0x%04lx addr=0x%05lx size=0x%05lx\n", */ /* Disabled MDA code */
		"VIDEO:",
		"COMPAQ io=0x%04lx addr=0x%05lx size=0x%05lx font=%u file=%s\n",
		/* io, addr, size */ /* Disabled MDA code */
		io, addr, size, font,
		(fontfile != NULL) ? fontfile : "<none>" /* Applies to CGA only */
	);
	ini_get_string (sct, "color", &col, "green"); /* Continued from MDA code */

	compaq_get_color (col, &col0, &col1, &col2);  /* Continued from MDA code */

	ini_get_uint32 (sct, "color_background", &col0, col0); /* Continued from MDA code */
	ini_get_uint32 (sct, "color_normal", &col1, col1);     /* Continued from MDA code */
	ini_get_uint32 (sct, "color_bright", &col2, col2);     /* Continued from MDA code */

	compaq = compaq_new (io, addr, size);

	if (compaq == NULL) {
		return (NULL);
	}
	compaq_set_color (compaq, 0, 0, col0 >> 16, col0 >> 8, col0);  /* Continued from MDA code */
	compaq_set_color (compaq, 1, 7, col1 >> 16, col1 >> 8, col1);  /* Continued from MDA code */
	compaq_set_color (compaq, 8, 15, col2 >> 16, col2 >> 8, col2); /* Continued from MDA code */

	compaq_set_font (compaq, font);

	if (fontfile != NULL) {
		if (compaq_load_font (compaq, fontfile)) {
			pce_log (MSG_ERR, "ERROR: Unable to load COMPAQ CGA font (%s)\n",
				fontfile
			);
		}
	}
	compaq_set_blink_rate (compaq, blink);
	return (&compaq->video);
}
