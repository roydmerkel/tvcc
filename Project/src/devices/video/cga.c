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

#include <devices/video/cga.h>
/* -------------------------------------------------------------------- */
/* Timing Generator:                                                    */
/* -------------------------------------------------------------------- */
/* This generator produces the timing signals used by the 6845 CRT      */
/* Controller and by the dynamic memory. It also solves the processor/  */
/* graphic controller contentions for access to the display buffer.     */
/* -------------------------------------------------------------------- */
#define CGA_IFREQ 1193182
#define CGA_PFREQ 14318184
#define CGA_HFREQ 15750
#define CGA_VFREQ 60

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
#define CGA_CRTC_INDEX0  0  /* 6845 Index   Register  */
#define CGA_CRTC_DATA0   1  /* 6845 Data    Register  */
#define CGA_CRTC_INDEX   4  /* 6845 Index   Register  */
#define CGA_CRTC_DATA    5  /* 6845 Data    Register  */
#define CGA_MODE         8  /* Mode Control Register  */
#define CGA_CSEL         9  /* Color Select Register  */
#define CGA_STATUS       10 /* Status       Register  */
#define CGA_PEN_RESET    11 /* Preset Light Pen Latch */
#define CGA_PEN_SET      12 /* Clear  Light Pen Latch */
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
#define CGA_CRTC_HT      0  /*  0/R 0: Horizontal Total              */
#define CGA_CRTC_HD      1  /*  1/R 1: Horizontal Displayed          */
#define CGA_CRTC_HS      2  /*  2/R 2: Horizontal Sync Position      */
#define CGA_CRTC_SYN     3  /*  3/R 3: Horizontal Sync Width         */
#define CGA_CRTC_VT      4  /*  4/R 4: Vertical   Total         Row  */
#define CGA_CRTC_VTA     5  /*  5/R 5: Vertical   Total Adjust       */
#define CGA_CRTC_VD      6  /*  6/R 6: Vertical   Displayed     Row  */
#define CGA_CRTC_VS      7  /*  7/R 7: Vertical   Sync Position Row  */
#define CGA_CRTC_IL      8  /*  8/R 8: Interlace Mode                */
#define CGA_CRTC_MS      9  /*  9/R 9: Maximum Scan Line Address     */
#define CGA_CRTC_CS      10 /*  A/R10: Cursor Start                  */
#define CGA_CRTC_CE      11 /*  B/R11: Cursor End                    */
#define CGA_CRTC_SH      12 /*  C/R12: Start             Address (H) */
#define CGA_CRTC_SL      13 /*  D/R13: Start             Address (L) */
#define CGA_CRTC_PH      14 /*  E/R14: Cursor            Address (H) */
#define CGA_CRTC_PL      15 /*  F/R15: Cursor            Address (L) */
#define CGA_CRTC_LH      16 /* 10/R16: Light Pen                 (H) */
#define CGA_CRTC_LL      17 /* 11/R17: Light Pen                 (L) */

#define CGA_MODE_CS      0x01		/* clock select */
#define CGA_MODE_G320    0x02
#define CGA_MODE_CBURST  0x04
#define CGA_MODE_ENABLE  0x08
#define CGA_MODE_G640    0x10
#define CGA_MODE_BLINK   0x20
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
#define CGA_CSEL_COL     0x0f
#define CGA_CSEL_INT     0x10
#define CGA_CSEL_PAL     0x20
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
#define CGA_STATUS_SYNC  0x01		/* -display enable */
#define CGA_STATUS_PEN   0x02
#define CGA_STATUS_VSYNC 0x08		/* vertical sync */

#define CGA_UPDATE_DIRTY   1
#define CGA_UPDATE_RETRACE 2


#include "cga_font.h"


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
static void cga_clock (cga_t *cga, unsigned long cnt);
/* -------------------------------------------------------------------- */
/* Timing Generator:                                                    */
/* -------------------------------------------------------------------- */
/* This generator produces the timing signals used by the 6845 CRT      */
/* Controller and by the dynamic memory. It also solves the processor/  */
/* graphic controller contentions for access to the display buffer.     */
/* -------------------------------------------------------------------- */


unsigned char cga_rgb[16][3] = {
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
void cga_set_blink_rate (cga_t *cga, unsigned freq)
{
	cga->blink_on = 1;
	cga->blink_cnt = freq;
	cga->blink_freq = freq;

	cga->update_state |= CGA_UPDATE_DIRTY;
}

/*
 * Get CRTC start offset
 */
unsigned cga_get_start (cga_t *cga)
{
	unsigned val;

	val = cga->reg_crt[CGA_CRTC_SH];              /*  C/R12: Start             Address (H) */
	val = (val << 8) | cga->reg_crt[CGA_CRTC_SL]; /*  D/R13: Start             Address (L) */

	return (val);
}

/*
 * Get the absolute cursor position
 */
unsigned cga_get_cursor (cga_t *cga)
{
	unsigned val;

	val = cga->reg_crt[CGA_CRTC_PH];              /*  E/R14: Cursor            Address (H) */
	val = (val << 8) | cga->reg_crt[CGA_CRTC_PL]; /*  F/R15: Cursor            Address (L) */

	return (val);
}

/*
 * Get the on screen cursor position
 */
int cga_get_position (cga_t *cga, unsigned *x, unsigned *y)
{
	unsigned pos, ofs;

	if (cga->w == 0) {
		return (1);
	}

	pos = cga_get_cursor (cga);
	ofs = cga_get_start (cga);

	pos = (pos - ofs) & 0x3fff;

	if (pos >= (cga->w * cga->h)) {
		return (1);
	}

	*x = pos % cga->w;
	*y = pos / cga->w;

	return (0);
}

/*
 * Set the internal palette from the mode and color select registers
 */
static
void cga_set_palette (cga_t *cga)
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
	mode = cga->reg[CGA_MODE];
	csel = cga->reg[CGA_CSEL];
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
	cga->pal[0] = csel & 0x0f;

	if (mode & CGA_MODE_CBURST) {
		cga->pal[1] = 3;
		cga->pal[2] = 4;
		cga->pal[3] = 7;
	}
	else if (csel & CGA_CSEL_PAL) {
		cga->pal[1] = 3;
		cga->pal[2] = 5;
		cga->pal[3] = 7;
	}
	else {
		cga->pal[1] = 2;
		cga->pal[2] = 4;
		cga->pal[3] = 6;
	}

	if (csel & CGA_CSEL_INT) {
		cga->pal[1] += 8;
		cga->pal[2] += 8;
		cga->pal[3] += 8;
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
void cga_set_timing (cga_t *cga)
{
	/* -------------------------------------------------------------------- */
	/* Timing Generator:                                                    */
	/* -------------------------------------------------------------------- */
	/* This generator produces the timing signals used by the 6845 CRT      */
	/* Controller and by the dynamic memory. It also solves the processor/  */
	/* graphic controller contentions for access to the display buffer.     */
	/* -------------------------------------------------------------------- */
	cga->ch = (cga->reg_crt[CGA_CRTC_MS] & 0x1f) + 1; /*  9/R 9: Maximum Scan Line Address     */
	cga->w = cga->reg_crt[CGA_CRTC_HD]; /*  1/R 1: Horizontal Displayed          */
	cga->h = cga->reg_crt[CGA_CRTC_VD]; /*  6/R 6: Vertical   Displayed     Row  */

	cga->clk_ht = 8 * (cga->reg_crt[CGA_CRTC_HT] + 1); /*  0/R 0: Horizontal Total              */
	cga->clk_hd = 8 * cga->reg_crt[CGA_CRTC_HD];       /*  1/R 1: Horizontal Displayed          */

	cga->clk_vt = cga->ch * (cga->reg_crt[CGA_CRTC_VT] + 7) * cga->clk_ht; /*  4/R 4: Vertical   Total         Row  */
	cga->clk_vd = cga->ch * cga->reg_crt[CGA_CRTC_VD] * cga->clk_ht;       /*  6/R 6: Vertical   Displayed     Row  */
}

/*
 * Get the dot clock
 */
static
unsigned long cga_get_dotclock (cga_t *cga)
{
	unsigned long long clk;
	/* -------------------------------------------------------------------- */
	/* Timing Generator:                                                    */
	/* -------------------------------------------------------------------- */
	/* This generator produces the timing signals used by the 6845 CRT      */
	/* Controller and by the dynamic memory. It also solves the processor/  */
	/* graphic controller contentions for access to the display buffer.     */
	/* -------------------------------------------------------------------- */
	clk = cga->video.dotclk[0];
	clk = (CGA_PFREQ * clk) / CGA_IFREQ;

	if ((cga->reg[CGA_MODE] & CGA_MODE_CS) == 0) {
		clk >>= 1;
	}

	return (clk);
}

/*
 * Set the internal screen buffer size
 */
int cga_set_buf_size (cga_t *cga, unsigned w, unsigned h)
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

	if (cnt > cga->bufmax) {
		tmp = realloc (cga->buf, cnt);
		if (tmp == NULL) {
			return (1);
		}

		cga->buf = tmp;
		cga->bufmax = cnt;
	}

	cga->buf_w = w;
	cga->buf_h = h;

	return (0);
}

/*
 * Draw the cursor in the internal buffer
 */
static
void cga_mode0_update_cursor (cga_t *cga)
{
	unsigned            i, j;
	unsigned            x, y;
	unsigned            c1, c2;
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
    if (cga->blink_on == 0) {
		return;
	}

	if ((cga->reg_crt[CGA_CRTC_CS] & 0x60) == 0x20) { /*  A/R10: Cursor Start                  */
		/* cursor off */
		return;
	}

	if (cga_get_position (cga, &x, &y)) {
		return;
	}

	c1 = cga->reg_crt[CGA_CRTC_CS] & 0x1f; /*  A/R10: Cursor Start                  */
	c2 = cga->reg_crt[CGA_CRTC_CE] & 0x1f; /*  B/R11: Cursor End                    */

	if (c1 >= cga->ch) {
		return;
	}

	if (c2 >= cga->ch) {
		c2 = cga->ch - 1;
	}

	col = cga_rgb[cga->mem[2 * (cga->w * y + x) + 1] & 0x0f];
	dst = cga->buf + 3 * 8 * (cga->w * (cga->ch * y + c1) + x);

	for (j = c1; j <= c2; j++) {
		for (i = 0; i < 8; i++) {
			dst[3 * i + 0] = col[0];
			dst[3 * i + 1] = col[1];
			dst[3 * i + 2] = col[2];
		}
		dst += 3 * 8 * cga->w;
	}
}

/*
 * Draw a character in the internal buffer
 */
static
void cga_mode0_update_char (cga_t *cga,
	unsigned char *dst, unsigned char c, unsigned char a)
{
	unsigned            i, j;
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
	if (cga->reg[CGA_MODE] & CGA_MODE_BLINK) {
		if (a & 0x80) {
			blk = !cga->blink_on;

			if ((cga->reg[CGA_CSEL] & CGA_CSEL_INT) == 0) {
				a &= 0x7f;
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
	fg = cga_rgb[a & 0x0f];
	bg = cga_rgb[(a >> 4) & 0x0f];

	fnt = cga->font + 8 * c;

	for (j = 0; j < cga->ch; j++) {
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

		dst += (3 * 8 * cga->w);
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
void cga_mode0_update (cga_t *cga)
{
	unsigned            x, y;
	unsigned            ofs;
	const unsigned char *src;
	unsigned char       *dst;
	if (cga_set_buf_size (cga, 8 * cga->w, cga->ch * cga->h)) {
		return;
	}

	src = cga->mem;
	ofs = (2 * cga_get_start (cga)) & 0x3ffe;

	for (y = 0; y < cga->h; y++) {
		dst = cga->buf + 3 * (8 * cga->w) * (cga->ch * y);

		for (x = 0; x < cga->w; x++) {
			cga_mode0_update_char (cga,
				dst + 3 * 8 * x, src[ofs], src[ofs + 1]
			);

			ofs = (ofs + 2) & 0x3ffe;
		}
	}

	cga_mode0_update_cursor (cga);
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
void cga_mode1_update (cga_t *cga)
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
	if (cga_set_buf_size (cga, 8 * cga->w, 2 * cga->h)) {
		return;
	}

	dst = cga->buf;

	w = 2 * cga->w;
	h = 2 * cga->h;

	ofs = (2 * cga_get_start (cga)) & 0x1fff;

	for (y = 0; y < h; y++) {
		mem = cga->mem + ((y & 1) << 13);

		for (x = 0; x < w; x++) {
			val = mem[(ofs + x) & 0x1fff];

			for (i = 0; i < 4; i++) {
				idx = (val >> 6) & 3;
				idx = cga->pal[idx];
				col = cga_rgb[idx];
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
void cga_mode2_update (cga_t *cga)
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
	if (cga_set_buf_size (cga, 16 * cga->w, 2 * cga->h)) {
		return;
	}

	dst = cga->buf;

	h = 2 * cga->h;
	w = 2 * cga->w;

	ofs = (2 * cga_get_start (cga)) & 0x1fff;

	for (y = 0; y < h; y++) {
		mem = cga->mem + ((y & 1) << 13);

		for (x = 0; x < w; x++) {
			val = mem[(ofs + x) & 0x1fff];

			for (i = 0; i < 8; i++) {
				idx = (val & 0x80) ? cga->pal[0] : 0;
				col = cga_rgb[idx];
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
void cga_make_comp_tab (cga_t *cga)
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
	if (cga->comp_tab == NULL) {
		cga->comp_tab = malloc (5 * 16 * 3);
	}

	hue = 0.0;
	sat = 2.0 / 3.0;
	lum = 1.0;

	if (cga->reg[CGA_CSEL] & CGA_CSEL_PAL) {
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
	src = cga_rgb[cga->reg[CGA_CSEL] & 0x0f];

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

			dst = cga->comp_tab + 3 * (16 * j + i);

			dst[0] = (unsigned char) (src[0] * r);
			dst[1] = (unsigned char) (src[1] * g);
			dst[2] = (unsigned char) (src[2] * b);
		}
	}

	cga->comp_tab_ok = 1;
}

/*
 * Update mode 2 (graphics 640 * 200 * 2) for composite
 */
static
void cga_mode2c_update (cga_t *cga)
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
	if (cga->comp_tab_ok == 0) {
		cga_make_comp_tab (cga);
	}

	if (cga_set_buf_size (cga, 16 * cga->w, 2 * cga->h)) {
		return;
	}

	dst = cga->buf;
	ofs = (2 * cga_get_start (cga)) & 0x1fff;
	pat = 0;

	h = 2 * cga->h;
	w = 2 * cga->w;

	for (y = 0; y < h; y++) {
		src = cga->mem + ((y & 1) << 13);

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

			col = cga->comp_tab + 3 * (16 * avgval + (pat & 0x0f));

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
void cga_update_blank (cga_t *cga)
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
	cga_set_buf_size (cga, 640, 200);

	dst = cga->buf;

	for (y = 0; y < cga->buf_h; y++) {
		fy = (y % 16) < 8;

		for (x = 0; x < cga->buf_w; x++) {
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
void cga_update (cga_t *cga)
{
	unsigned char mode;
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
	cga->buf_w = 0;
	cga->buf_h = 0;
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
	mode = cga->reg[CGA_MODE];
	/* -------------------------------------------------------------------- */
	/* Sequence of Events for Changing Modes:                               */
	/* -------------------------------------------------------------------- */
	/* - 1. Determine the mode of operation.                                */
	/* - 2. Reset the video-enable bit in the mode-control register.        */
	/* - 3. Program the 6845 CRT controller to select the mode.             */
	/* - 4. Program the mode-control and color-select registers including   */
	/* re-enabling the video.                                               */
	/* -------------------------------------------------------------------- */
	if ((mode & CGA_MODE_ENABLE) == 0) {
		cga_update_blank (cga);
	}
	else if (mode & CGA_MODE_G320) {
		if (mode & CGA_MODE_G640) {
			if ((mode & CGA_MODE_CBURST) == 0) {
				cga_mode2c_update (cga);
			}
			else {
				cga_mode2_update (cga);
			}
		}
		else {
			cga_mode1_update (cga);
		}
	}
	else {
		cga_mode0_update (cga);
	}
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
static
unsigned char cga_crtc_get_reg (cga_t *cga, unsigned reg)
{
	if ((reg < 12) || (reg > 17)) {
		return (0xff);
	}

	return (cga->reg_crt[reg]);
}

/*
 * Set a CRTC register
 */
static
void cga_crtc_set_reg (cga_t *cga, unsigned reg, unsigned char val)
{
	if (reg > 15) {
		return;
	}

	if (cga->reg_crt[reg] == val) {
		return;
	}

	cga->reg_crt[reg] = val;

	cga_set_timing (cga);

	cga->update_state |= CGA_UPDATE_DIRTY;
}


/*
 * Get the CRTC index register
 */
static
unsigned char cga_get_crtc_index (cga_t *cga)
{
	return (cga->reg[CGA_CRTC_INDEX]);
}

/*
 * Get the CRTC data register
 */
static
unsigned char cga_get_crtc_data (cga_t *cga)
{
	return (cga_crtc_get_reg (cga, cga->reg[CGA_CRTC_INDEX]));
}

/*
 * Get the status register
 */
static
unsigned char cga_get_status (cga_t *cga)
{
	unsigned char val;
	unsigned long clk;

	cga_clock (cga, 0);

	clk = cga_get_dotclock (cga);

	val = cga->reg[CGA_STATUS];

	val |= (CGA_STATUS_VSYNC | CGA_STATUS_SYNC);

	if (clk < cga->clk_vd) {
		val &= ~CGA_STATUS_VSYNC;

		if (cga->clk_ht > 0) {
			if ((clk % cga->clk_ht) < cga->clk_hd) {
				val &= ~CGA_STATUS_SYNC;
			}
		}
	}

	return (val);
}

/*
 * Set the CRTC index register
 */
static
void cga_set_crtc_index (cga_t *cga, unsigned char val)
{
	cga->reg[CGA_CRTC_INDEX] = val & 0x1f;
}

/*
 * Set the CRTC data register
 */
static
void cga_set_crtc_data (cga_t *cga, unsigned char val)
{
	cga->reg[CGA_CRTC_DATA] = val;

	cga_crtc_set_reg (cga, cga->reg[CGA_CRTC_INDEX], val);
}

/*
 * Set the mode register
 */
static
void cga_set_mode (cga_t *cga, unsigned char val)
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
	cga->reg[CGA_MODE] = val;

	cga_set_palette (cga);
	cga_set_timing (cga);

	cga->update_state |= CGA_UPDATE_DIRTY;
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
void cga_set_color_select (cga_t *cga, unsigned char val)
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
	cga->reg[CGA_CSEL] = val;

	cga_set_palette (cga);

	cga->update_state |= CGA_UPDATE_DIRTY;
}

/*
 * Get a CGA register
 */
unsigned char cga_reg_get_uint8 (cga_t *cga, unsigned long addr)
{
	switch (addr) {
	case CGA_CRTC_INDEX0:
	case CGA_CRTC_INDEX:
		return (cga_get_crtc_index (cga));

	case CGA_CRTC_DATA0:
	case CGA_CRTC_DATA:
		return (cga_get_crtc_data (cga));

	case CGA_STATUS:
		return (cga_get_status (cga));
	}

	return (0xff);
}

unsigned short cga_reg_get_uint16 (cga_t *cga, unsigned long addr)
{
	unsigned short ret;

	ret = cga_reg_get_uint8 (cga, addr);

	if ((addr + 1) < cga->regblk->size) {
		ret |= cga_reg_get_uint8 (cga, addr + 1) << 8;
	}

	return (ret);
}

/*
 * Set a CGA register
 */
void cga_reg_set_uint8 (cga_t *cga, unsigned long addr, unsigned char val)
{
	switch (addr) {
	case CGA_CRTC_INDEX0:
	case CGA_CRTC_INDEX:
		cga_set_crtc_index (cga, val);
		break;

	case CGA_CRTC_DATA0:
	case CGA_CRTC_DATA:
		cga_set_crtc_data (cga, val);
		break;

	case CGA_MODE:
		cga_set_mode (cga, val);
		break;

	case CGA_CSEL:
		cga_set_color_select (cga, val);
		break;

	case CGA_PEN_RESET:
		cga->reg[CGA_STATUS] &= ~CGA_STATUS_PEN;
		break;

	case CGA_PEN_SET:
		cga->reg[CGA_STATUS] |= CGA_STATUS_PEN;
		break;

	default:
		cga->reg[addr] = val;
		break;
	}
}

void cga_reg_set_uint16 (cga_t *cga, unsigned long addr, unsigned short val)
{
	cga_reg_set_uint8 (cga, addr, val & 0xff);

	if ((addr + 1) < cga->regblk->size) {
		cga_reg_set_uint8 (cga, addr + 1, val >> 8);
	}
}


static
void cga_mem_set_uint8 (cga_t *cga, unsigned long addr, unsigned char val)
{
	if (cga->mem[addr] == val) {
		return;
	}

	cga->mem[addr] = val;

	cga->update_state |= CGA_UPDATE_DIRTY;
}

static
void cga_mem_set_uint16 (cga_t *cga, unsigned long addr, unsigned short val)
{
	cga_mem_set_uint8 (cga, addr, val & 0xff);

	if ((addr + 1) < cga->memblk->size) {
		cga_mem_set_uint8 (cga, addr + 1, (val >> 8) & 0xff);
	}
}


static
int cga_set_msg (cga_t *cga, const char *msg, const char *val)
{
	if (msg_is_message ("emu.video.blink", msg)) {
		unsigned freq;

		if (msg_get_uint (val, &freq)) {
			return (1);
		}

		cga_set_blink_rate (cga, freq);

		return (0);
	}

	return (-1);
}

static
void cga_set_terminal (cga_t *cga, terminal_t *trm)
{
	cga->term = trm;

	if (cga->term != NULL) {
		trm_open (cga->term, 640, 200);
	}
}

static
mem_blk_t *cga_get_mem (cga_t *cga)
{
	return (cga->memblk);
}

static
mem_blk_t *cga_get_reg (cga_t *cga)
{
	return (cga->regblk);
}

static
void cga_print_info (cga_t *cga, FILE *fp)
{
	unsigned i;

	fprintf (fp, "DEV: CGA\n");

	fprintf (fp, "CGA: OFS=%04X  POS=%04X  BG=%02X  PAL=%u\n",
		cga_get_start (cga),
		cga_get_cursor (cga),
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
		cga->reg[CGA_CSEL] & 0x0f,
		(cga->reg[CGA_CSEL] >> 5) & 1
	);
	/* -------------------------------------------------------------------- */
	/* Timing Generator:                                                    */
	/* -------------------------------------------------------------------- */
	/* This generator produces the timing signals used by the 6845 CRT      */
	/* Controller and by the dynamic memory. It also solves the processor/  */
	/* graphic controller contentions for access to the display buffer.     */
	/* -------------------------------------------------------------------- */
	fprintf (fp, "CLK: CLK=%lu  HT=%lu HD=%lu  VT=%lu VD=%lu\n",
		cga_get_dotclock (cga),
		cga->clk_ht, cga->clk_hd,
		cga->clk_vt, cga->clk_vd
	);

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
		cga->reg[CGA_MODE],
		/* -------------------------------------------------------------------- */
		/* Sequence of Events for Changing Modes:                               */
		/* -------------------------------------------------------------------- */
		/* - 1. Determine the mode of operation.                                */
		/* - 2. Reset the video-enable bit in the mode-control register.        */
		/* - 3. Program the 6845 CRT controller to select the mode.             */
		/* - 4. Program the mode-control and color-select registers including   */
		/* re-enabling the video.                                               */
		/* -------------------------------------------------------------------- */
		cga->reg[CGA_CSEL],
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
		cga->reg[CGA_STATUS],
		cga->pal[0], cga->pal[1], cga->pal[2], cga->pal[3]
	);

	fprintf (fp, "CRTC=[%02X", cga->reg_crt[0]);
	for (i = 1; i < 18; i++) {
		fputs ((i & 7) ? " " : "-", fp);
		fprintf (fp, "%02X", cga->reg_crt[i]);
	}
	fputs ("]\n", fp);

	fflush (fp);
}

/*
 * Force a screen update
 */
static
void cga_redraw (cga_t *cga, int now)
{
	if (now) {
		if (cga->term != NULL) {
			cga->update (cga);

			trm_set_size (cga->term, cga->buf_w, cga->buf_h);
			trm_set_lines (cga->term, cga->buf, 0, cga->buf_h);
			trm_update (cga->term);
		}
	}

	cga->update_state |= CGA_UPDATE_DIRTY;
}

static
void cga_clock (cga_t *cga, unsigned long cnt)
{
	unsigned long clk;
	/* -------------------------------------------------------------------- */
	/* Timing Generator:                                                    */
	/* -------------------------------------------------------------------- */
	/* This generator produces the timing signals used by the 6845 CRT      */
	/* Controller and by the dynamic memory. It also solves the processor/  */
	/* graphic controller contentions for access to the display buffer.     */
	/* -------------------------------------------------------------------- */
	if (cga->clk_vt < 50000) {
		return;
	}

	clk = cga_get_dotclock (cga);

	if (clk < cga->clk_vd) {
		cga->update_state &= ~CGA_UPDATE_RETRACE;
		return;
	}

	if (clk >= cga->clk_vt) {
		cga->video.dotclk[0] = 0;
		cga->video.dotclk[1] = 0;
		cga->video.dotclk[2] = 0;
	}

	if (cga->update_state & CGA_UPDATE_RETRACE) {
		return;
	}

	if (cga->blink_cnt > 0) {
		cga->blink_cnt -= 1;

		if (cga->blink_cnt == 0) {
			cga->blink_cnt = cga->blink_freq;
			cga->blink_on = !cga->blink_on;

			if ((cga->reg[CGA_MODE] & CGA_MODE_G320) == 0) {
				cga->update_state |= CGA_UPDATE_DIRTY;
			}
		}
	}

	if (cga->term != NULL) {
		if (cga->update_state & CGA_UPDATE_DIRTY) {
			cga->update (cga);
			trm_set_size (cga->term, cga->buf_w, cga->buf_h);
			trm_set_lines (cga->term, cga->buf, 0, cga->buf_h);
		}

		trm_update (cga->term);
	}

	cga->update_state = CGA_UPDATE_RETRACE;
}

void cga_free (cga_t *cga)
{
	mem_blk_del (cga->memblk);
	mem_blk_del (cga->regblk);
	free (cga->comp_tab);
}

void cga_del (cga_t *cga)
{
	if (cga != NULL) {
		cga_free (cga);
		free (cga);
	}
}

static
void cga_set_font (cga_t *cga, unsigned font)
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
	if (font == 0) {
		memcpy (cga->font, cga_font_thick, 2048);
	}
	else {
		memcpy (cga->font, cga_font_thin, 2048);
	}
}

static
int cga_load_font (cga_t *cga, const char *fname)
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
	FILE *fp;

	fp = fopen (fname, "rb");

	if (fp == NULL) {
		return (1);
	}

	if (fread (cga->font, 1, 2048, fp) != 2048) {
		fclose (fp);
		return (1);
	}

	fclose (fp);

	return (0);
}

void cga_init (cga_t *cga, unsigned long io, unsigned long addr, unsigned long size)
{
	unsigned i;

	pce_video_init (&cga->video);

	cga->video.ext = cga;
	cga->video.del = (void *) cga_del;
	cga->video.set_msg = (void *) cga_set_msg;
	cga->video.set_terminal = (void *) cga_set_terminal;
	cga->video.get_mem = (void *) cga_get_mem;
	cga->video.get_reg = (void *) cga_get_reg;
	cga->video.print_info = (void *) cga_print_info;
	cga->video.redraw = (void *) cga_redraw;
	cga->video.clock = (void *) cga_clock;

	cga->ext = NULL;
	cga->update = cga_update;

	size = (size < 16384) ? 16384UL : size;

	cga->memblk = mem_blk_new (addr, size, 1);
	cga->memblk->ext = cga;
	cga->memblk->set_uint8 = (void *) cga_mem_set_uint8;
	cga->memblk->set_uint16 = (void *) &cga_mem_set_uint16;
	cga->mem = cga->memblk->data;
	mem_blk_clear (cga->memblk, 0x00);

	cga->regblk = mem_blk_new (io, 16, 1);
	cga->regblk->ext = cga;
	cga->regblk->set_uint8 = (void *) cga_reg_set_uint8;
	cga->regblk->set_uint16 = (void *) cga_reg_set_uint16;
	cga->regblk->get_uint8 = (void *) cga_reg_get_uint8;
	cga->regblk->get_uint16 = (void *) cga_reg_get_uint16;
	cga->reg = cga->regblk->data;
	mem_blk_clear (cga->regblk, 0x00);

	cga->term = NULL;

	for (i = 0; i < 18; i++) {
		cga->reg_crt[i] = 0;
	}

	cga->pal[0] = 0;
	cga->pal[1] = 11;
	cga->pal[2] = 13;
	cga->pal[3] = 15;
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
	cga->comp_tab_ok = 0;
	cga->comp_tab = NULL;

	cga->blink_on = 1;
	cga->blink_cnt = 0;
	cga->blink_freq = 16;

	cga->w = 0;
	cga->h = 0;
	cga->ch = 0;
	/* -------------------------------------------------------------------- */
	/* Timing Generator:                                                    */
	/* -------------------------------------------------------------------- */
	/* This generator produces the timing signals used by the 6845 CRT      */
	/* Controller and by the dynamic memory. It also solves the processor/  */
	/* graphic controller contentions for access to the display buffer.     */
	/* -------------------------------------------------------------------- */
	cga->clk_ht = 0;
	cga->clk_vt = 0;
	cga->clk_hd = 0;
	cga->clk_vd = 0;
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
	cga->bufmax = 0;
	cga->buf = NULL;

	cga->update_state = 0;

	cga_set_font (cga, 0);
}

cga_t *cga_new (unsigned long io, unsigned long addr, unsigned long size)
{
	cga_t *cga;

	cga = malloc (sizeof (cga_t));
	if (cga == NULL) {
		return (NULL);
	}

	cga_init (cga, io, addr, size);

	return (cga);
}

video_t *cga_new_ini (ini_sct_t *sct)
{
	unsigned long io, addr, size;
	unsigned      font;
	unsigned      blink;
	const char    *fontfile;
	cga_t         *cga;

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
	ini_get_uint32 (sct, "address", &addr, 0xb8000);
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
	ini_get_uint16 (sct, "font", &font, 0);
	ini_get_uint16 (sct, "blink", &blink, 0);
	ini_get_string (sct, "file", &fontfile, NULL);

	pce_log_tag (MSG_INF,
		"VIDEO:",
		"CGA io=0x%04lx addr=0x%05lx size=0x%05lx font=%u file=%s\n",
		io, addr, size, font,
		(fontfile != NULL) ? fontfile : "<none>"
	);

	cga = cga_new (io, addr, size);

	if (cga == NULL) {
		return (NULL);
	}

	cga_set_font (cga, font);

	if (fontfile != NULL) {
		if (cga_load_font (cga, fontfile)) {
			pce_log (MSG_ERR, "ERROR: Unable to load IBM CGA font (%s)\n",
				fontfile
			);
		}
	}

	cga_set_blink_rate (cga, blink);

	return (&cga->video);
}
