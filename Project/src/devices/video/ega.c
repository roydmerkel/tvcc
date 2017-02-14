/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/video/ega.c                                      *
 * Created:     2003-09-06 by Hampa Hug <hampa@hampa.ch>                     *
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

#include <lib/log.h>
#include <lib/msg.h>

#include <devices/video/ega.h>


#define EGA_IFREQ  1193182

#define EGA_PFREQ0 14318180
#define EGA_HFREQ0 15750
#define EGA_VFREQ0 60

#define EGA_PFREQ1 16257000
#define EGA_HFREQ1 21850
#define EGA_VFREQ1 60

#define EGA_PFREQ2 16257000
#define EGA_HFREQ2 18430
#define EGA_VFREQ2 50


#define EGA_MONITOR_ECD 0
#define EGA_MONITOR_CGA 1
#define EGA_MONITOR_MDA 2


#define EGA_ATC_INDEX     0x10		/* attribute controller index/data register */
#define EGA_STATUS0       0x12		/* Input Status Register 0 */
#define EGA_MOUT_W        0x12		/* Miscellaneous Output Register */
/* ------------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Display, August 2nd, 1984: Sequencer Registers:     */
/* ------------------------------------------------------------------------------------ */
/* Name:                   Port:  Index:                                                */
/*  - Address:              - 3C4: -  -:                                                */
/*  - Reset:                - 3C5: - 00:                                                */
/*  - Clocking Mode:        - 3C5: - 01:                                                */
/*  - Map Mask:             - 3C5: - 02:                                                */
/*  - Character Map Select: - 3C5: - 03:                                                */
/*  - Memory Mode:          - 3C5: - 04:                                                */
/* Sequencer Address Register:                                                          */
/* The Address Register is a pointer register located at address hex 3C4. This register */
/* is loaded with a binary value that points to the sequencer data register where data  */
/* is to be written. This value is referred to as "Index" in the table above.           */
/* Sequencer Address Register Format:                                                   */
/*  - Bit 0: Sequencer Address                                                          */
/*  - Bit 1: Sequencer Address                                                          */
/*  - Bit 2: Sequencer Address                                                          */
/*  - Bit 3: Sequencer Address                                                          */
/*  - Bit 4: Sequencer Address                                                          */
/*  - Bit 5: Not Used                                                                   */
/*  - Bit 6: Not Used                                                                   */
/*  - Bit 7: Not Used                                                                   */
/* Bit 0-Bit 3: Sequencer Address Bits--A binary value pointing to the register where   */
/* data is to be written.                                                               */
/* ------------------------------------------------------------------------------------ */
#define EGA_SEQ_INDEX     0x14		/* sequencer index register */
#define EGA_SEQ_DATA      0x15		/* sequencer data register */
#define EGA_GENOA         0x18		/* GENOA EGA extension register */
#define EGA_MOUT          0x1c		/* miscellaneous output register */
#define EGA_GRC_INDEX     0x1e		/* graphics controller index register */
#define EGA_GRC_DATA      0x1f		/* graphics controller data register */
#define EGA_CRT_INDEX0    0x20		/* crtc index register */
#define EGA_CRT_DATA0     0x21		/* crtc data register */
#define EGA_CRT_INDEX     0x24		/* crtc index register */
#define EGA_CRT_DATA      0x25		/* crtc data register */
#define EGA_STATUS1       0x2a		/* input status register 1 */ /* Feature Control Register */
/* ------------------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Attribute Controller Registers:*/
/* ------------------------------------------------------------------------------------------ */
/* Name:                             Port:  Index:                                            */
/*  - Mode Control           Register: - 3C0: - 10:                                           */
/*  - Color Plane Enable     Register: - 3C0: - 12:                                           */
/*  - Horizontal Pel Panning Register: - 3C0: - 13:                                           */
/* Mode Control Register:                                                                     */
/* This is a write-only register pointed to by the value in the Attribute address register.   */
/* This value must be hex 10 before writing can take place. The processor output port address */
/* for this register is hex 3C0.                                                              */
/* Mode Control Register Format:                                                              */
/*  - Bit 0: Graphics/Alphanumeric Mode                                                       */
/*  - Bit 1: Display Type                                                                     */
/*  - Bit 2: Enable Line Graphics Character Codes                                             */
/*  - Bit 3: Select Background Intensity or Enable Blink                                      */
/*  - Bit 4: Not Used                                                                         */
/*  - Bit 5: Not Used                                                                         */
/*  - Bit 6: Not Used                                                                         */
/*  - Bit 7: Not Used                                                                         */
/* Bit 0: Graphics/Alphanumeric Mode--A logical 0 selects selects alphanumeric mode. A logical*/
/* 1 selects graphics mode.                                                                   */
/*                                                                                            */
/* Bit 1: Monochrome Display/Color Display--A logical 0 selects color display attributes. A   */
/* logical 1 selects IBM Monochrome Display attributes.                                       */
/*                                                                                            */
/* Bit 2: Enable Line Graphics Character Codes--When this bit is set to 0, the ninth dot will */
/* be the same as the background. A logical 1 enables the special line graphics character     */
/* codes for the IBM Monochrome Display adapter. This bit when enabled forces the ninth dot of*/
/* a line graphic character to be identical to the eighth dot of the character. The line      */
/* graphics character codes for the Monochrome Display Adapter are Hex C0 through Hex DF.     */
/*                                                                                            */
/* For character fonts that do not utilize the line graphics character codes in the range of  */
/* Hex C0 through Hex DF, bit 2 of this register should be a logical 0. Otherwise unwanted    */
/* video information will be displayed on the CRT screen.                                     */
/*                                                                                            */
/* Bit 3: Enable Blink/Select Background Intensity--A logical 0 selects the background        */
/* intensity of the attribute input. This mode was available on the Monochrome and Color      */
/* Graphics adapters. A logical 1 enables the blink attribute in alphanumeric modes. This bit */
/* must also be set to 1 for blinking graphics modes.                                         */
/*                                                                                            */
/* Color Plane Enable Register:                                                               */
/* This is a write-only register pointed to by the value in the Attribute address register.   */
/* This value must be hex 12 before writing can take place. The processor output port address */
/* for this register is 3C0.                                                                  */
/* Color Plane Enable Register Format:                                                        */
/*  - Bit 0: Enable Color Plane                                                               */
/*  - Bit 1: Enable Color Plane                                                               */
/*  - Bit 2: Enable Color Plane                                                               */
/*  - Bit 3: Enable Color Plane                                                               */
/*  - Bit 4: Video Status MUX                                                                 */
/*  - Bit 5: Video Status MUX                                                                 */
/*  - Bit 6: Not Used                                                                         */
/*  - Bit 7: Not Used                                                                         */
/* Bit 0-Bit 3: Enable Color Plane--Writing a logical 1 in any of bits 0 through 3 enables the*/
/* respective display memory color plane.                                                     */
/*                                                                                            */
/* Bit 4-Bit 5: Video Status MUX--Selects two of the six color outputs to be available on the */
/* status port. The following table illustrates the combinations available and the color      */
/* output wiring.                                                                             */
/* Color Plane Enable Register: Input Status Register One:                                    */
/* Bit 5:                Bit 4: Bit 5:              Bit 4:                                    */
/*  - 0:                  - 0:   - Red:              - Blue:                                  */
/*  - 0:                  - 1:   - Secondary Blue:   - Green:                                 */
/*  - 1:                  - 0:   - Secondary Red:    - Secondary Green:                       */
/*  - 1:                  - 1:   - Not Used:         - Not Used                               */
/* Horizontal Pel Panning Register:                                                           */
/* This is a write-only register pointed to by the value in the Attribute address register.   */
/* This value must be hex 12 before writing can take place. The processor output port address */
/* for this register is hex 3C0.                                                              */
/* Horizontal Pel Panning Register Format:                                                    */
/*  - Bit 0: Horizontal Pel Panning                                                           */
/*  - Bit 1: Horizontal Pel Panning                                                           */
/*  - Bit 2: Horizontal Pel Panning                                                           */
/*  - Bit 3: Horizontal Pel Panning                                                           */
/*  - Bit 4: Not Used                                                                         */
/*  - Bit 5: Not Used                                                                         */
/*  - Bit 6: Not Used                                                                         */
/*  - Bit 7: Not Used                                                                         */
/* Bit 0-Bit 3: Horizontal Pel Panning--This 4 bit register selects the number of picture     */
/* elements (pels) to shift the video data horizontally to the left. Pel panning is available */
/* in both A/N and APA modes. In Monochrome A/N mode, the image can be shifted a maximum of 9 */
/* pels. In all other A/N and APA modes, the image can be shifted a maximum of 8 pels. The    */
/* sequence for shifting the image is given below:                                            */
/*  - 9 pels/character : 8,0,1,2,3,4,5,6,7 (Monochrome A/N mode only)                         */
/*  - 8 pels/character : 0,1,2,3,4,5,6,7 (All other Modes)                                    */
/* ------------------------------------------------------------------------------------------ */
#define EGA_ATC_MODE      16		/* Attribute Controller Registers: Mode Control           Register */
#define EGA_ATC_CPE       18		/* Attribute Controller Registers: Color Plane Enable     Register */
#define EGA_ATC_HPP       19        /* Attribute Controller Registers: Horizontal Pel Panning Register */
/* ------------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Display, August 2nd, 1984: Sequencer Registers:     */
/* ------------------------------------------------------------------------------------ */
/* Name:                   Port:  Index:                                                */
/*  - Address:              - 3C4: -  -:                                                */
/*  - Reset:                - 3C5: - 00:                                                */
/*  - Clocking Mode:        - 3C5: - 01:                                                */
/*  - Map Mask:             - 3C5: - 02:                                                */
/*  - Character Map Select: - 3C5: - 03:                                                */
/*  - Memory Mode:          - 3C5: - 04:                                                */
/* Reset Register:                                                                      */
/* This is a write-only register pointed to when the value in the address register is   */
/* hex 00. The output port address for this register is hex 3C5.                        */
/* Reset Register Format:                                                               */
/*  - Bit 0:Asynchronous Reset                                                          */
/*  - Bit 1: Synchronous Reset                                                          */
/*  - Bit 2: Not Used                                                                   */
/*  - Bit 3: Not Used                                                                   */
/*  - Bit 4: Not Used                                                                   */
/*  - Bit 5: Not Used                                                                   */
/*  - Bit 6: Not Used                                                                   */
/*  - Bit 7: Not Used                                                                   */
/* Bit 0:Asynchronous Reset--A logical 0 commands the sequencer to asynchronous clear   */
/* and halt. All outputs are placed in the high impedance state when this bit is a 0. A */
/* logical 1 commands the sequencer to run unless bit 1 is set to zero. Resetting the   */
/* sequencer with this bit can cause data loss in the dynamic RAMs.                     */
/*                                                                                      */
/* Bit 1: Synchronous Reset--A logical 0 commands the sequencer to synchronous clear and*/
/* halt. Bits 1 and 0 must both be ones to allow the sequencer to operate. Reset the    */
/* sequencer with this bit before changing the Clocking Mode Register, if memory        */
/* contents are to be preserved.                                                        */
/*                                                                                      */
/* Clocking Mode Register:                                                              */
/* This is write-only register pointed to when the value in the address register is hex */
/* 01. The output port address for this register is hex 3C5.                            */
/* Clocking Mode Register Format:                                                       */
/*  - Bit 0: 8/9 Dot Clocks                                                             */
/*  - Bit 1: Bandwidth                                                                  */
/*  - Bit 2: Shift Load                                                                 */
/*  - Bit 3: Dot Clock                                                                  */
/*  - Bit 4: Not Used                                                                   */
/*  - Bit 5: Not Used                                                                   */
/*  - Bit 6: Not Used                                                                   */
/*  - Bit 7: Not Used                                                                   */
/* Bit 0: 8/9 Dot Clocks--A logical 0 directs the sequencer to generate character clocks*/
/* 9 dots wide; a logical 1 directs the sequencer to generate character clocks 8 dots   */
/* wide. Monochrome alphanumeric mode (07H) is the only mode that uses character clocks */
/* 9 dots wide. All other modes must use 8 dots per character clock.                    */
/*                                                                                      */
/* Bit 1: Bandwidth--A logical 0 makes CRT memory cycles occur on 4 out of 5 available  */
/* memory cycles; a logical 1 makes CRT memory cycles occur on 2 out of 5 available     */
/* memory cycles. Medium resolution modes require less data to be fetched from the      */
/* display buffer during the horizontal scan time. This allows the CPU greater access   */
/* time to the display buffer. All high resolution modes must provide the CRTC with 4   */
/* out of 5 memory cycles in order to refresh the display image.                        */
/*                                                                                      */
/* Bit 2: Shift Load--When set to 0, the video serializers are reloaded every character */
/* clock; when set to 1, the video serializers are loaded every other character clock.  */
/* This mode is useful when 16 bits are fetched per cycle and chained together in the   */
/* shift registers.                                                                     */
/*                                                                                      */
/* Bit 3: Dot Clock--A logical 0 selects normal dot clocks derived from the sequencer   */
/* master clock input. When this bit is set to 1, the master clock will be divided by 2 */
/* to generate the dot clock. All the other timings will be stretched since they are    */
/* derived from the dot clock. Dot clock divided by two is used for 320x200 modes (0, 1,*/
/* 4, 5) to provide a pixel rate of 7 MHz, (9 MHz for mode D).                          */
/*                                                                                      */
/* Map Mask Register:                                                                   */
/* This is a write-only register pointed to when the value in the address register is   */
/* hex 02. The output port address for this register is hex 3C5.                        */
/*  - Bit 0: 1 Enables Map 0                                                            */
/*  - Bit 1: 1 Enables Map 1                                                            */
/*  - Bit 2: 1 Enables Map 2                                                            */
/*  - Bit 3: 1 Enables Map 3                                                            */
/*  - Bit 4: Not Used                                                                   */
/*  - Bit 5: Not Used                                                                   */
/*  - Bit 6: Not Used                                                                   */
/*  - Bit 7: Not Used                                                                   */
/* Bit 0-Bit 3: Map Mask--A logical 1 in bits 3 through 0 enables the processor to write*/
/* to the corresponding maps 3 through 0. If this register is programmed with a value of*/
/* 0FH, the CPU can perform a 32-bit write operation with only one memory cycle. This   */
/* substantially reduces the overhead on the CPU during display update cycles in        */
/* graphics modes. Data scrolling operations are also enhanced by setting this register */
/* to a value of 0FH and writing the display buffer address with the data stored in the */
/* CPU data latches. This is a read-modify-write operation. When odd/even modes are     */
/* selected, maps 0 and 1 and 2 and 3 should have the same map mask value.              */
/*                                                                                      */
/* Character Map Select Register:                                                       */
/* This is a write-only register pointed to when the value in the address register is   */
/* hex 03. The output port address for this register is 3C5.                            */
/*  - Bit 0: Character Map Select B                                                     */
/*  - Bit 1: Character Map Select B                                                     */
/*  - Bit 2: Character Map Select A                                                     */
/*  - Bit 3: Character Map Select A                                                     */
/*  - Bit 4: Not Used                                                                   */
/*  - Bit 5: Not Used                                                                   */
/*  - Bit 6: Not Used                                                                   */
/*  - Bit 7: Not Used                                                                   */
/* Bit 0-Bit 1: Character Map Select B--Selects the map used to generate alpha          */
/* characters when attribute bit 3 is a 0, according to the following table:            */
/* Bits     1 0:Map Selected:Table Location:                                            */
/*  - Value 0 0: - 0:         - 1st 8K of Plane 2 Bank 0                                */
/*  - Value 0 1: - 1:         - 2nd 8K of Plane 2 Bank 1                                */
/*  - Value 1 0: - 2:         - 3rd 8K of Plane 2 Bank 2                                */
/*  - Value 1 1: - 3:         - 4th 8K of Plane 2 Bank 3                                */
/* Bit 2-Bit 3: Character Map Select A--Selects the map used to generate alpha          */
/* characters when attribute bit 3 is a 1, according to the following table:            */
/* Bits     3 2:Map Selected:Table Location:                                            */
/*  - Value 0 0: - 0:         - 1st 8K of Plane 2 Bank 0                                */
/*  - Value 0 1: - 1:         - 2nd 8K of Plane 2 Bank 1                                */
/*  - Value 1 0: - 2:         - 3rd 8K of Plane 2 Bank 2                                */
/*  - Value 1 1: - 3:         - 4th 8K of Plane 2 Bank 3                                */
/* In alphanumeric mode, bit 3 of the attribute byte normally has the function of       */
/* turning the foreground intensity on or off. This bit however may be redefined as a   */
/* switch between character sets. This function is enabled when there is a difference   */
/* between the value in Character Map Select A and the value in Character Map Select B. */
/* Whenever these two values are the same, the character select function is disabled.   */
/* The memory mode register bit 1 must be a 1 (indicates the memory extension card is   */
/* installed in the unit) to enable this function; otherwise, bank 0 is always selected.*/
/*                                                                                      */
/* 128K of graphics memory is required to support two character sets. 256K supports four*/
/* character sets. Asynchronous reset clears this register to 0. This should be done    */
/* only when the sequencer is reset.                                                    */
/*                                                                                      */
/* Memory Mode Register:                                                                */
/* This is a write-only register pointed to when the value in the address register is   */
/* hex 04. The processor output port address for this register is 3C5.                  */
/* Memory Mode Register Format:                                                         */
/*  - 0: Alpha                                                                          */
/*  - 1: Extended Memory                                                                */
/*  - 2: Odd/Even                                                                       */
/*  - 3: Not Used                                                                       */
/*  - 4: Not Used                                                                       */
/*  - 5: Not Used                                                                       */
/*  - 6: Not Used                                                                       */
/*  - 7: Not Used                                                                       */
/* Bit 0: Alpha--A logical 0 indicates that a non-alpha mode is active. A logical 1     */
/* indicates that alpha mode is active and enables the character generator map select   */
/* function.                                                                            */
/*                                                                                      */
/* Bit 1: Extended Memory--A logical 0 indicates that the memory expansion card is not  */
/* installed. A logical 1 indicates that the memory expansion card is installed and     */
/* enables access to the extended memory through address bits 14 and 15.                */
/*                                                                                      */
/* Bit 2: Odd/Even--A logical 0 directs even processor addresses to access maps 0 and 2,*/
/* while odd processor addresses access maps 1 and 3. A logical 1 causes processor      */
/* addresses to sequentially access data within a bit map. The maps are accessed        */
/* according to the value in the map mask register.                                     */
/* ------------------------------------------------------------------------------------ */
#define EGA_SEQ_RESET     0		/* Sequencer Registers: Reset                Register */
#define EGA_SEQ_CLOCK     1		/* Sequencer Registers: Clocking Mode        Register */
#define EGA_SEQ_MAPMASK   2		/* Sequencer Registers: Map Mask             Register */
#define EGA_SEQ_CMAPSEL   3		/* Sequencer Registers: Character Map Select Register */
#define EGA_SEQ_MODE      4		/* Sequencer Registers: Memory Mode          Register */
/* ------------------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Graphics Controller Registers: */
/* ------------------------------------------------------------------------------------------ */
/* Name:                    Port:  Index:                                                     */
/*  - Graphics 1 Position   : - 3CC: -   :                                                    */
/*  - Graphics 2 Position   : - 3CA: -   :                                                    */
/*  - Graphics 1 & 2 Address: - 3CE: -   :                                                    */
/*  -        Set/Reset      : - 3CF: - 00:                                                    */
/*  - Enable Set/Reset      : - 3CF: - 01:                                                    */
/*  - Color Compare         : - 3CF: - 02:                                                    */
/*  - Data Rotate           : - 3CF: - 03:                                                    */
/*  - Read Map Select       : - 3CF: - 04:                                                    */
/*  - Mode Register         : - 3CF: - 05:                                                    */
/*  - Miscellaneous         : - 3CF: - 06:                                                    */
/*  - Color Don't Care      : - 3CF: - 07:                                                    */
/*  - Bit Mask              : - 3CF: - 08:                                                    */
/* Graphics 1 Position Register:                                                              */
/* This is a write-only register. The processor output port address for this register is hex  */
/* 3CC.                                                                                       */
/* Graphics I Position Register Format:                                                       */
/*  - Bit 0: Position 0                                                                       */
/*  - Bit 1: Position 1                                                                       */
/*  - Bit 2: Not Used                                                                         */
/*  - Bit 3: Not Used                                                                         */
/*  - Bit 4: Not Used                                                                         */
/*  - Bit 5: Not Used                                                                         */
/*  - Bit 6: Not Used                                                                         */
/*  - Bit 7: Not Used                                                                         */
/* Bit 0-Bit 1: Position--These 2 bits are binary encoded hierarchy bits for the graphics     */
/* chips. The position register controls which 2 bits of the processor data bus each chip     */
/* responds to. Graphics 1 must be programmed with a position register value of 0 for this    */
/* card.                                                                                      */
/*                                                                                            */
/* Graphics 2 Position Register:                                                              */
/* This is a write-only register. The processor output port address for this register is hex  */
/* 3CC.                                                                                       */
/* Graphics II Position Register Format:                                                      */
/*  - Bit 0: Position 0                                                                       */
/*  - Bit 1: Position 1                                                                       */
/*  - Bit 2: Not Used                                                                         */
/*  - Bit 3: Not Used                                                                         */
/*  - Bit 4: Not Used                                                                         */
/*  - Bit 5: Not Used                                                                         */
/*  - Bit 6: Not Used                                                                         */
/*  - Bit 7: Not Used                                                                         */
/* Bit 0-Bit 1: Position--These 2 bits are binary encoded hierarchy bits for the graphics     */
/* chips. The position register controls which 2 bits of the processor data bus each chip     */
/* responds to. Graphics 1 must be programmed with a position register value of 1 for this    */
/* card.                                                                                      */
/*                                                                                            */
/* Graphics 1 and 2 Address Register:                                                         */
/* This is a write-only register and the processor output port address for this register is   */
/* hex 3CE.                                                                                   */
/* Graphics 1 and 2 Address Register Formats:                                                 */
/*  - Bit 0: Graphics Address                                                                 */
/*  - Bit 1: Graphics Address                                                                 */
/*  - Bit 2: Graphics Address                                                                 */
/*  - Bit 3: Graphics Address                                                                 */
/*  - Bit 4: Not Used                                                                         */
/*  - Bit 5: Not Used                                                                         */
/*  - Bit 6: Not Used                                                                         */
/*  - Bit 7: Not Used                                                                         */
/* Bit 0-Bit 3: Graphics 1 and 2 Address Bits--This output loads the address register in both */
/* graphics chips simultaneously. This register points to the data register of the graphics   */
/* chips.                                                                                     */
/*                                                                                            */
/* Set/Reset Register:                                                                        */
/* This is a write-only register pointed to by the value in the Graphics 1 and 2 address      */
/* register. This value must be hex 00 before writing can take place. The processor output    */
/* port address for this register is hex 3CF.                                                 */
/* Set/Reset Register Format:                                                                 */
/*  - Bit 0: Set/Reset Bit 0                                                                  */
/*  - Bit 1: Set/Reset Bit 1                                                                  */
/*  - Bit 2: Set/Reset Bit 2                                                                  */
/*  - Bit 3: Set/Reset Bit 3                                                                  */
/*  - Bit 4: Not Used                                                                         */
/*  - Bit 5: Not Used                                                                         */
/*  - Bit 6: Not Used                                                                         */
/*  - Bit 7: Not Used                                                                         */
/* Bit 0-Bit 3: Set/Reset--These bits represent the value written to the respective memory    */
/* planes when the processor does a memory write with write mode 0 selected and set/reset mode*/
/* is enabled. Set/Reset can be enabled on a plane by plane basis with separate OUT commands  */
/* to the Set/Reset register.                                                                 */
/*                                                                                            */
/* Enable Set/Reset Register:                                                                 */
/* This is a write-only register and is pointed to by the value in the Graphics 1 and 2       */
/* address register. This value must be hex 01 before writing can take place. The processor   */
/* output port for this register is hex 3CF.                                                  */
/* Color Compare    Register Format:                                                          */
/*  - Bit 0: Color Compare 0                                                                  */
/*  - Bit 1: Color Compare 1                                                                  */
/*  - Bit 2: Color Compare 2                                                                  */
/*  - Bit 3: Color Compare 3                                                                  */
/* Bit 0-Bit 3: Enable Set/Reset--These bits enable the set/reset function. The respective    */
/* memory plane is written with the value of the Set/Reset register provided the write value  */
/* is 0. When write mode is 0 and Set/Reset is not enabled on a plane, that plane is written  */
/* with the value of the processor data.                                                      */
/*                                                                                            */
/* Color Compare    Register:                                                                 */
/* This is a write-only register pointed to by the value in the Graphics 1 and 2 address      */
/* register. This value must be hex 02 before writing can take place. The processor output    */
/* port address for this register is hex 3CF.                                                 */
/* Enable Set/Reset Register Format:                                                          */
/*  - Bit 0: Enable Set/Reset Bit 0                                                           */
/*  - Bit 1: Enable Set/Reset Bit 1                                                           */
/*  - Bit 2: Enable Set/Reset Bit 2                                                           */
/*  - Bit 3: Enable Set/Reset Bit 3                                                           */
/*  - Bit 4: Not Used                                                                         */
/*  - Bit 5: Not Used                                                                         */
/*  - Bit 6: Not Used                                                                         */
/*  - Bit 7: Not Used                                                                         */
/* Bit 0-Bit 3: Color Compare--These bits represent a 4 bit color value to be compared. If the*/
/* processor sets read mode 1 on the graphics chips, and does a memory read, the data returned*/
/* from the memory cycle will be a 1 in each bit position where the 4 bit planes equal the    */
/* color compare register.                                                                    */
/*                                                                                            */
/* Data Rotate Register:                                                                      */
/* This is a write-only register pointed to by the value in the Graphics 1 and 2 address      */
/* register. This value must be hex 03 before writing can take place. The processor output    */
/* port address for this register is hex 3CF.                                                 */
/* Data Rotate Register Format:                                                               */
/*  - Bit 0: Rotate Count                                                                     */
/*  - Bit 1: Rotate Count 1                                                                   */
/*  - Bit 2: Rotate Count 2                                                                   */
/*  - Bit 3: Function Select                                                                  */
/*  - Bit 4: Function Select                                                                  */
/*  - Bit 5: Not Used                                                                         */
/*  - Bit 6: Not Used                                                                         */
/*  - Bit 7: Not Used                                                                         */
/* Bit 0-Bit 2: Rotate Count--These bits represent a binary encoded value of the number of    */
/* positions to rotate the processor data bus during processor memory writes. This operation  */
/* is done when the write mode is 0. To write unrotated data the processor must select a count*/
/* of 0.                                                                                      */
/*                                                                                            */
/* Bit 3-Bit 4: Function Select--Data written to memory can operate logically with data       */
/* already in the processor latches. The bit functions are defined in the following table.    */
/* Bits 4 3:                                                                                  */
/*  -   0 0: Data unmodified                                                                  */
/*  -   0 1: Data AND'ed with latched data.                                                   */
/*  -   1 0: Data  OR'ed with latched data.                                                   */
/*  -   1 1: Data XOR'ed with latched data.                                                   */
/* Data may be any of the choices selected by the Write Mode Register except processor        */
/* latches. If rotated data is selected, the rotate applies before the logical function.      */
/*                                                                                            */
/* Read Map Select Register:                                                                  */
/* This is a write-only register pointed to by the value in the Graphics 1 and 2 address      */
/* register. This value must be hex 04 before writing can take place. The processor output    */
/* port address for this register is hex 3CF.                                                 */
/* Read Map Select Register Format:                                                           */
/*  - Bit 0: Map Select 0                                                                     */
/*  - Bit 1: Map Select 1                                                                     */
/*  - Bit 2: Map Select 2                                                                     */
/*  - Bit 3: Not Used                                                                         */
/*  - Bit 4: Not Used                                                                         */
/*  - Bit 5: Not Used                                                                         */
/*  - Bit 6: Not Used                                                                         */
/*  - Bit 7: Not Used                                                                         */
/* Bit 0-Bit 2: Map Select--These bits represent a binary encoded value of the memory plane   */
/* number from which the processor reads data. This register has no effect on the color       */
/* compare read mode described elsewhere in this section.                                     */
/*                                                                                            */
/* Mode Register:                                                                             */
/* This is a write-only register pointed to by the value in the Graphics 1 and 2 address      */
/* register. This value must be hex 05 before writing can take place. The processor output    */
/* port address for this register is 3CF.                                                     */
/* Mode Register Format:                                                                      */
/*  - Bit 0: Write Mode                                                                       */
/*  - Bit 1: Write Mode                                                                       */
/*  - Bit 2: Test Condition                                                                   */
/*  - Bit 3: Read  Mode                                                                       */
/*  - Bit 4: Odd/Even                                                                         */
/*  - Bit 5: Test Register Mode                                                               */
/*  - Bit 6: Not Used                                                                         */
/*  - Bit 7: Not Used                                                                         */
/* Bit-Bit 1: Write Mode                                                                      */
/* Bits 1 0:                                                                                  */
/*  -   0 0: Each memory plane is written with the processor data rotated by the number of    */
/*  counts in the rotate register, unless Set/Reset is enabled for the plane. Planes for which*/
/*  Set/Reset is enabled are written with 8 bits of the value contained in the Set/Reset      */
/*  register for that plane.                                                                  */
/*  -   0 1: Each memory plane is written with the contents of the processor latches. These   */
/*  latches are loaded by a processor read operation.                                         */
/*  -   1 0: Memory plane n (0 through 3) is filled with 8 bits of the value of data bit n.   */
/*  -   1 1: Not Valid                                                                        */
/* The logic function specified by the function select register also applies.                 */
/*                                                                                            */
/* Bit 2: Test Condition--A logical 1 directs graphics controller outputs to be placed in high*/
/* impedance state for testing.                                                               */
/*                                                                                            */
/* Bit 3: Read Mode--When this bit is a logical 0, the processor reads data from the memory   */
/* plane selected by the read map select register. When this bit is a logical 1, the processor*/
/* reads the results of the comparison of the 4 memory planes and the color compare register. */
/*                                                                                            */
/* Bit 4: Odd/Even--A logical 1 selects the odd/even addressing mode, which is useful for     */
/* emulation of the Color Graphics Monitor Adapter compatible modes. Normally the value here  */
/* follows the value of the Memory Mode Register bit 3 of the Sequencer.                      */
/*                                                                                            */
/* Bit 5: Shift Register--A logical 1 directs the shift registers on each graphics chip to    */
/* format the serial data stream with even numbered bits on the even numbered maps and odd    */
/* numbered bits on the odd maps.                                                             */
/*                                                                                            */
/* Miscellaneous Register:                                                                    */
/* This is a write-only register pointed to by the value in the Graphics 1 and 2 address      */
/* register. This value must be hex 06 before writing can take place. The processor output    */
/* port for this register is hex 3CF.                                                         */
/* Miscellaneous Register Format:                                                             */
/*  - Bit 0: Graphics Mode                                                                    */
/*  - Bit 1: Chain Odd Maps to Even                                                           */
/*  - Bit 2: Memory Map 0                                                                     */
/*  - Bit 3: Memory Map 1                                                                     */
/*  - Bit 4: Not Used                                                                         */
/*  - Bit 5: Not Used                                                                         */
/*  - Bit 6: Not Used                                                                         */
/*  - Bit 7: Not Used                                                                         */
/* Bit 0: Graphics Mode--This bit controls alpha-mode addressing. A logical 1 selects graphics*/
/* mode. When set to graphics mode, the character generator address latches are disabled.     */
/*                                                                                            */
/* Bit 1: Chain Odd Maps To Even Maps--When set to 1, this bit directs the processor address  */
/* bit 0 to be replaced by a higher order bit and odd/even maps to be selected with odd/even  */
/* values of the processor A0 bit, respectively.                                              */
/*                                                                                            */
/* Bit 2-Bit 3: Memory Map--These bits control the mapping of the regenerative buffer into the*/
/* processor address space.                                                                   */
/* Bits 3 2:                                                                                  */
/*  -   0 0: Hex A000 for 128K bytes.                                                         */
/*  -   0 1: Hex A000 for  64K bytes.                                                         */
/*  -   1 0: Hex B000 for  32K bytes.                                                         */
/*  -   1 1: Hex B800 for  32K bytes.                                                         */
/* If the display adapter is mapped at address hex A000 for 128K bytes, no other adapter can  */
/* be installed in the system.                                                                */
/*                                                                                            */
/* Color Don't Care Register:                                                                 */
/* This is a write-only register and is pointed to by the value in the Graphics 1 and 2       */
/* address register. This value must be hex 07 before writing can take place. The processor   */
/* output port for this register is hex 3CF.                                                  */
/* Color Don't Care Register Format:                                                          */
/*  - Bit 0: Color Plane 0=Don't Care                                                         */
/*  - Bit 1: Color Plane 1=Don't Care                                                         */
/*  - Bit 2: Color Plane 2=Don't Care                                                         */
/*  - Bit 3: Color Plane 3=Don't Care                                                         */
/*  - Bit 4: Not Used                                                                         */
/*  - Bit 5: Not Used                                                                         */
/*  - Bit 6: Not Used                                                                         */
/*  - Bit 7: Not Used                                                                         */
/* Bit 0: Color Don't Care--Compare plane 0=don't care when reading color compare when this   */
/* bit is set to 1.                                                                           */
/*                                                                                            */
/* Bit 1: Color Don't Care--Compare plane 1=don't care when reading color compare when this   */
/* bit is set to 1.                                                                           */
/*                                                                                            */
/* Bit 2: Color Don't Care--Compare plane 2=don't care when reading color compare when this   */
/* bit is set to 1.                                                                           */
/*                                                                                            */
/* Bit 3: Color Don't Care--Compare plane 3=don't care when reading color compare when this   */
/* bit is set to 1.                                                                           */
/*                                                                                            */
/* Bit Mask  Register:                                                                        */
/* This is a write-only register and is pointed to by the value in the Graphics 1 and 2       */
/* address register. This value must be hex 08 before writing can take place. The processor   */
/* output port for this register is hex 3CF.                                                  */
/* Bit Mask Register Format:                                                                  */
/*  - Bit 0: 0-immune to change    1-Unimpeded Writes                                         */
/*  - Bit 1: 0-immune to change    1-Unimpeded Writes                                         */
/*  - Bit 2: 0-immune to change    1-Unimpeded Writes                                         */
/*  - Bit 3: 0-immune to change    1-Unimpeded Writes                                         */
/*  - Bit 4: 0-immune to change    1-Unimpeded Writes                                         */
/*  - Bit 5: 0-immune to change    1-Unimpeded Writes                                         */
/*  - Bit 6: 0-immune to change    1-Unimpeded Writes                                         */
/*  - Bit 7: 0-immune to change    1-Unimpeded Writes                                         */
/* Bit 0-Bit 7: Bit Mask--Any bit programmed to n causes the corresponding bit nin each bit   */
/* plane to be immune to change provided that the location being written was the last location*/
/* read by the processor. Bits programmed to a 1 allow unimpeded writes to the corresponding  */
/* bits in the bit planes.                                                                    */
/*                                                                                            */
/* The bit mask applies to any data written by the processor (rotate, AND'ed, OR'ed, XOR'ed,  */
/* DX, and S/R). To preserve bits using the bit mask, data must be latched internally by      */
/* reading the location. When data is written to preserve the bits, the most current data in  */
/* latches is written in those positions. The bit mask applies to all bit planes              */
/* simultaneously.                                                                            */
/* ------------------------------------------------------------------------------------------ */
#define EGA_GRC_SETRESET  0		/* Graphics Controller Registers: Set/Reset        Register */
#define EGA_GRC_ENABLESR  1		/* Graphics Controller Registers: Enable Set/Reset Register */
#define EGA_GRC_COLCMP    2		/* Graphics Controller Registers: Color Compare    Register */
#define EGA_GRC_ROTATE    3		/* Graphics Controller Registers: Data Rotate      Register */
#define EGA_GRC_READMAP   4		/* Graphics Controller Registers: Read Map Select  Register */
#define EGA_GRC_MODE      5		/* Graphics Controller Registers: Graphics Mode    Register */
#define EGA_GRC_MISC      6		/* Graphics Controller Registers: Miscellaneous    Register */
#define EGA_GRC_CDC       7		/* Graphics Controller Registers: Color Don't Care Register */
#define EGA_GRC_BITMASK   8		/* Graphics Controller Registers: Bit Mask         Register */
/* ------------------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: CRT Control Registers:         */
/* ------------------------------------------------------------------------------------------ */
/* Name:                       Port:  Index:                                                  */
/*  - Address Register:         - 3?4: - -                                                    */
/*  - Horizontal Total:         - 3?5: - 00                                                   */
/*  - Horizontal Display End:   - 3?5: - 01                                                   */
/*  - Start Horizontal Blank:   - 3?5: - 02                                                   */
/*  - End   Horizontal Blank:   - 3?5: - 03                                                   */
/*  - Start Horizontal Retrace: - 3?5: - 04                                                   */
/*  - End   Horizontal Retrace: - 3?5: - 05                                                   */
/*  - Vertical   Total:         - 3?5: - 06                                                   */
/*  - Overflow:                 - 3?5: - 07                                                   */
/*  - Preset Row Scan:          - 3?5: - 08                                                   */
/*  - Max Scan Line:            - 3?5: - 09                                                   */
/*  - Cursor Start:             - 3?5: - 0A                                                   */
/*  - Cursor End:               - 3?5: - 0B                                                   */
/*  - Start Address High:       - 3?5: - 0C                                                   */
/*  - Start Address Low:        - 3?5: - 0D                                                   */
/*  - Cursor Location High:     - 3?5: - 0E                                                   */
/*  - Cursor Location Low:      - 3?5: - 0F                                                   */
/*  - Vertical Retrace Start:   - 3?5: - 10                                                   */
/*  - Light Pen High:           - 3?5: - 10                                                   */
/*  - Vertical Retrace End:     - 3?5: - 11                                                   */
/*  - Light Pen Low:            - 3?5: - 11                                                   */
/*  - Vertical Display End:     - 3?5: - 12                                                   */
/*  - Offset:                   - 3?5: - 13                                                   */
/*  - Underline Location:       - 3?5: - 14                                                   */
/*  - Start Vertical   Blank:   - 3?5: - 15                                                   */
/*  - End   Vertical   Blank:   - 3?5: - 16                                                   */
/*  - Mode Control:             - 3?5: - 17                                                   */
/*  - Line Compare:             - 3?5: - 18                                                   */
/* ? = B in Monochrome Modes and D in Color Modes                                             */
/*                                                                                            */
/* CRT Controller Address Register:                                                           */
/* The Address register is pointer register located at hex 3B4 or hex 3D4. If an IBM          */
/* Monochrome Display is attached to the adapter, address 3B4 is used. If a color display is  */
/* attached to the adapter, address 3D4 is used. This register is loaded with a binary value  */
/* that points to the CRT Controller data register where data is to be written. This value is */
/* referred to as "Index" in the table above.                                                 */
/* CRT Controller Address Register Format:                                                    */
/*  - Bit 0: CRTC Address                                                                     */
/*  - Bit 1: CRTC Address                                                                     */
/*  - Bit 2: CRTC Address                                                                     */
/*  - Bit 3: CRTC Address                                                                     */
/*  - Bit 4: CRTC Address                                                                     */
/*  - Bit 5: Not Used                                                                         */
/*  - Bit 6: Not Used                                                                         */
/*  - Bit 7: Not Used                                                                         */
/* Bit 0-Bit 4: CRT Controller Address Bits--A binary value pointing to the CRT Controller    */
/* register where data is to be written.                                                      */
/*                                                                                            */
/* Horizontal Total Register:                                                                 */
/* This is a write-only register pointed to when the value in the CRT Controller address is   */
/* hex 00. The processor output port address for this register is hex 3B5 or hex 3D5.         */
/* Horizontal Total Register Format:                                                          */
/*  - Bit 0: Horizontal Total                                                                 */
/*  - Bit 1: Horizontal Total                                                                 */
/*  - Bit 2: Horizontal Total                                                                 */
/*  - Bit 3: Horizontal Total                                                                 */
/*  - Bit 4: Horizontal Total                                                                 */
/*  - Bit 5: Horizontal Total                                                                 */
/*  - Bit 6: Horizontal Total                                                                 */
/*  - Bit 7: Horizontal Total                                                                 */
/* This register defines the total number of characters in the horizontal scan interval       */
/* including the retrace time. The value directly controls the period of the horizontal       */
/* retrace output signal. An internal horizontal character counter counts character clock     */
/* inputs to CRT Controller, and all horizontal and vertical timings are based upon the       */
/* horizontal register. Comparators are used to compare register values with horizontal       */
/* character values to provide horizontal timings.                                            */
/*                                                                                            */
/* Bit 0-Bit 7: Horizontal Total--The total number of characters less 2.                      */
/*                                                                                            */
/* Horizontal Display Enable End Register:                                                    */
/* This is a write-only register pointed to when the value in the CRT Controller address      */
/* register is hex 01. The processor output port address for this register is hex 3B5 or hex  */
/* 3D5.                                                                                       */
/* Horizontal Display Enable End Register Format:                                             */
/*  - Bit 0: Horizontal Display Enable End                                                    */
/*  - Bit 1: Horizontal Display Enable End                                                    */
/*  - Bit 2: Horizontal Display Enable End                                                    */
/*  - Bit 3: Horizontal Display Enable End                                                    */
/*  - Bit 4: Horizontal Display Enable End                                                    */
/*  - Bit 5: Horizontal Display Enable End                                                    */
/*  - Bit 6: Horizontal Display Enable End                                                    */
/*  - Bit 7: Horizontal Display Enable End                                                    */
/* This register defines the length of the horizontal display enable signal. It determines the*/
/* number of displayed character positions per horizontal line.                               */
/*  - Bit 0-Bit 7: Horizontal display enable end--A value one less than the total number of   */
/* displayed characters.                                                                      */
/*                                                                                            */
/* Start Horizontal Blinking Register:                                                        */
/* This is a write-only register pointed to when the value in the CRT Controller address      */
/* register is hex 02. The processor output port address for this register is hex 3B5 or hex  */
/* 3D5.                                                                                       */
/* Start Horizontal Blanking Register Format:                                                 */
/*  - Bit 0: Start Vertical Blinking                                                          */
/*  - Bit 1: Start Vertical Blinking                                                          */
/*  - Bit 2: Start Vertical Blinking                                                          */
/*  - Bit 3: Start Vertical Blinking                                                          */
/*  - Bit 4: Start Vertical Blinking                                                          */
/*  - Bit 5: Start Vertical Blinking                                                          */
/*  - Bit 6: Start Vertical Blinking                                                          */
/*  - Bit 7: Start Vertical Blinking                                                          */
/* This register determines when the horizontal blanking output signal becomes active. The row*/
/* scan address and underline scan line decode outputs are multiplexed on the memory address  */
/* outputs and cursor outputs respectively during the blanking interval. These outputs are    */
/* latched external to the CRT Controller with the falling edge of the BLANK output signal.   */
/* The row scan address and underline signals remain on the output signals for one character  */
/* count beyond the end of the blanking signal.                                               */
/*  - Bit 0-Bit 7: Start Horizontal Blanking--The horizontal blanking signal becomes active   */
/* when the horizontal character counter reaches this value.                                  */
/*                                                                                            */
/* End Horizontal Blanking Register:                                                          */
/* This is a write-only register pointed to when the value in the CRT Controller address      */
/* register is hex 03. The processor output port address for this register is hex 3B5 or hex  */
/* 3D5.                                                                                       */
/* End Horizontal Blanking Register Format:                                                   */
/*  - Bit 0: End Blanking                                                                     */
/*  - Bit 1: End Blanking                                                                     */
/*  - Bit 2: End Blanking                                                                     */
/*  - Bit 3: End Blanking                                                                     */
/*  - Bit 4: End Blanking                                                                     */
/*  - Bit 5: Display Enable Skew Control                                                      */
/*  - Bit 6: Display Enable Skew Control                                                      */
/*  - Bit 7: Not Used                                                                         */
/* This register determines when the horizontal blanking output signal becomes inactive. The  */
/* row scan address and underline scan line decode outputs are multiplexed on the memory      */
/* address outputs and the cursor outputs respectively during the blanking interval. These    */
/* outputs are latched external to the CRT Controller with the falling edge of the BLANK      */
/* output signal. The row scan address and underline signals remain on the output signals for */
/* one character count beyond the end of the blanking signal.                                 */
/*                                                                                            */
/* Bit 0-Bit 4: End Horizontal Blanking--A value equal to the five least significant bits of  */
/* the horizontal character counter value at which time the horizontal blanking signal becomes*/
/* inactive (logical 0). To obtain a blanking signal of width W, the following algorithm is   */
/* used: Value of Start Blanking Register + Width of Blanking signal in character clock units */
/* = 5-bit result to be programmed into the End Horizontal Blanking Register.                 */
/*                                                                                            */
/* Bit 5-Bit 6: Display Enable Skew Control--These two bits determine the amount of display   */
/* enable skew. Display enable skew control is required to provide sufficient time for the CRT*/
/* Controller to access the display buffer to obtain a character and attribute code, access   */
/* the character generator font, and then go through the Horizontal Pel Panning Register in   */
/* the Attribute Controller. Each access requires the display enable signal to be skewed one  */
/* character clock unit so that the video output is in synchronization with the horizontal and*/
/* vertical retrace signals. The bit values and amount of skew are shown in the following     */
/* table:                                                                                     */
/* Bits 6 5:                                                                                  */
/*  -   0 0: Zero  character clock skew                                                       */
/*  -   0 1: One   character clock skew                                                       */
/*  -   1 0: Two   character clock skew                                                       */
/*  -   1 1: Three character clock skew                                                       */
/*                                                                                            */
/* Start Horizontal Retrace Pulse Register:                                                   */
/* This is a write-only register pointed to when the value in the CRT Controller address      */
/* register is hex 04. The processor output port address for this register is hex 3B5 or hex  */
/* 3D5.                                                                                       */
/* Start Horizontal Retrace Pulse Register Format:                                            */
/*  - Bit 0: Start Horizontal Retrace Pulse                                                   */
/*  - Bit 1: Start Horizontal Retrace Pulse                                                   */
/*  - Bit 2: Start Horizontal Retrace Pulse                                                   */
/*  - Bit 3: Start Horizontal Retrace Pulse                                                   */
/*  - Bit 4: Start Horizontal Retrace Pulse                                                   */
/*  - Bit 5: Start Horizontal Retrace Pulse                                                   */
/*  - Bit 6: Start Horizontal Retrace Pulse                                                   */
/*  - Bit 7: Start Horizontal Retrace Pulse                                                   */
/* This register is used to center the screen horizontally, and to specify the character      */
/* position at which the Horizontal Retrace Pulse becomes active.                             */
/*                                                                                            */
/* Bit 0-Bit 7: Start Horizontal Retrace Pulse--The value programmed is a binary count of the */
/* character position number at which the signal becomes active.                              */
/*                                                                                            */
/* End Horizontal Retrace Register:                                                           */
/* This is a write-only register pointed to when the value in the CRT Controller address      */
/* register is hex 05. The processor output port address for this register is hex 3B5 or hex  */
/* 3D5.                                                                                       */
/* End Horizontal Retrace Register Format:                                                    */
/*  - Bit 0: End Horizontal Retrace                                                           */
/*  - Bit 1: End Horizontal Retrace                                                           */
/*  - Bit 2: End Horizontal Retrace                                                           */
/*  - Bit 3: End Horizontal Retrace                                                           */
/*  - Bit 4: End Horizontal Retrace                                                           */
/*  - Bit 5:     Horizontal Retrace Delay                                                     */
/*  - Bit 6:     Horizontal Retrace Delay                                                     */
/*  - Bit 7: Start Odd Memory Address                                                         */
/* This register specified the character position at which the Horizontal Retrace Pulse       */
/* becomes inactive (logical 0).                                                              */
/*                                                                                            */
/* Bit 0-Bit 4: End Horizontal Retrace--A value equal to the five least significant bits of   */
/* the horizontal character counter value at which time the horizontal retrace signal becomes */
/* inactive (logical 0). To obtain a retrace signal of width W, the following algorithm is    */
/* used: Value of Start Retrace Register + width of horizontal retrace signal in character    */
/* clock units = 5-bit result to be programmed into the End Horizontal Retrace Register.      */
/*                                                                                            */
/* Bit 5-Bit 6: Horizontal Retrace Delay--These bits control the skew of the horizontal       */
/* retrace signal. Binary 00 equals no Horizontal Retrace Delay. For some modes, it is        */
/* necessary to provide a horizontal retrace signal that takes up the entire blanking         */
/* interval. Some internal timings are generated by the falling edge of the horizontal retrace*/
/* signal. To guarantee the signals are latched properly, the retrace signal is started before*/
/* the end of the display enable signal, and then skewed several character clock times to     */
/* provide the proper screen centering.                                                       */
/*                                                                                            */
/* Bit 7: Start Odd/Even Memory Address--This bit controls whether the first CRT memory       */
/* address output after a horizontal retrace begins with an even or an odd address. A logical */
/* 0 selects even addresses, a logical 1 selects odd addresses. This bit is used for          */
/* horizontal pel panning applications. Generally, this bit should be set to a logical 0.     */
/*                                                                                            */
/* Vertical Total Register:                                                                   */
/* This is a write-only register pointed to when the value in the CRT Controller address      */
/* register is hex 06. The processor output port address for this register is hex 3B5 or 3D5. */
/* Vertical Total Register Format:                                                            */
/*  - Bit 0: Vertical Total                                                                   */
/*  - Bit 1: Vertical Total                                                                   */
/*  - Bit 2: Vertical Total                                                                   */
/*  - Bit 3: Vertical Total                                                                   */
/*  - Bit 4: Vertical Total                                                                   */
/*  - Bit 5: Vertical Total                                                                   */
/*  - Bit 6: Vertical Total                                                                   */
/*  - Bit 7: Vertical Total                                                                   */
/* Bit 0-Bit 7: Vertical Total--This is the low-order eight bits of a nine-bit register. The  */
/* binary value represents the number of horizontal raster scans on the CRT screen, including */
/* vertical retrace. The value in this register determines the period of the vertical retrace */
/* signal. Bit 8 of this register is contained in the CRT Controller Overflow Register hex 07 */
/* bit 0.                                                                                     */
/*                                                                                            */
/* CRT Controller Overflow Register:                                                          */
/* This is a write-only register pointed to when the value in the CRT Controller Address      */
/* Register is hex 07. The processor output port address for this register is hex 3B5 or hex  */
/* 3D5.                                                                                       */
/*  - Bit 0: Vertical Total              Bit 8                                                */
/*  - Bit 1: Vertical Display Enable End Bit 8                                                */
/*  - Bit 2: Vertical Retrace Start      Bit 8                                                */
/*  - Bit 3: Start Vertical Blank        Bit 8                                                */
/*  - Bit 4: Line Compare                Bit 8                                                */
/*  - Bit 5: Cursor Location             Bit 8                                                */
/* Bit 0: Vertical Total--Bit 8 of the Vertical Total register (index hex 06).                */
/*                                                                                            */
/* Bit 1: Vertical Display Enable End--Bit 8 of the Vertical Display Enable End register      */
/* (index hex 12).                                                                            */
/*                                                                                            */
/* Bit 2: Vertical Retrace Start--Bit 8 of the Vertical Retrace Start register (index hex 10).*/
/*                                                                                            */
/* Bit 3: Start Vertical Blank--Bit 8 of the Start Vertical Blank register (index hex 15).    */
/*                                                                                            */
/* Bit 4: Line Compare--Bit 8 of the Line Compare register (index hex 18).                    */
/*                                                                                            */
/* Bit 5: Cursor Location--Bit 8 of the Cursor Location register (index hex 0A).              */
/*                                                                                            */
/* Preset Row Scan Register:                                                                  */
/* This is a write-only register pointed to when the value in the CRT Controller address      */
/* register is hex 08. The processor output port address for this register is hex 3B5 or hex  */
/* 3D5.                                                                                       */
/* Preset Row Scan Register Format:                                                           */
/*  - Bit 0: Starting Row Scan Count after a Vertical Retrace                                 */
/*  - Bit 1: Starting Row Scan Count after a Vertical Retrace                                 */
/*  - Bit 2: Starting Row Scan Count after a Vertical Retrace                                 */
/*  - Bit 3: Starting Row Scan Count after a Vertical Retrace                                 */
/*  - Bit 4: Starting Row Scan Count after a Vertical Retrace                                 */
/*  - Bit 5: Not Used                                                                         */
/*  - Bit 6: Not Used                                                                         */
/*  - Bit 7: Not Used                                                                         */
/* This register is used for pel scrolling.                                                   */
/*                                                                                            */
/* Bit 0-Bit 4: Preset Row Scan (Pel Scrolling)--This register specified the starting row scan*/
/* count after a vertical retrace. The row scan counter increments each horizontal retrace    */
/* time until a maximum row scan occurs. At maximum row scan compare time the row scan is     */
/* cleared (not preset).                                                                      */
/*                                                                                            */
/* Maximum Scan Line Register:                                                                */
/* This is a write-only register pointed to when the value in the CRT Controller address      */
/* register is hex 09. The processor output port address for this register is hex 3B5 or hex  */
/* 3D5.                                                                                       */
/* Maximum Scan Line Register Format:                                                         */
/*  - Bit 0: Maximum Scan Line                                                                */
/*  - Bit 1: Maximum Scan Line                                                                */
/*  - Bit 2: Maximum Scan Line                                                                */
/*  - Bit 3: Maximum Scan Line                                                                */
/*  - Bit 4: Maximum Scan Line                                                                */
/*  - Bit 5: Not Used                                                                         */
/*  - Bit 6: Not Used                                                                         */
/*  - Bit 7: Not Used                                                                         */
/* Bit 0-Bit 4: Maximum Scan Line--This register specified the number of scan lines per       */
/* character row. The number to be programmed is the maximum row scan number minus one.       */
/*                                                                                            */
/* Cursor Start Register:                                                                     */
/* This is a write-only register pointed to when the value in the CRT Controller register is  */
/* hex 0A. The processor output port address for this register is hex 3B5 or hex 3D5.         */
/* Cursor Start Register Format:                                                              */
/*  - Bit 0: Row Scan Cursor Begins                                                           */
/*  - Bit 1: Row Scan Cursor Begins                                                           */
/*  - Bit 2: Row Scan Cursor Begins                                                           */
/*  - Bit 3: Row Scan Cursor Begins                                                           */
/*  - Bit 4: Row Scan Cursor Begins                                                           */
/*  - Bit 5: Not Used                                                                         */
/*  - Bit 6: Not Used                                                                         */
/*  - Bit 7: Not Used                                                                         */
/* Bit 0-Bit 4: Cursor Start--This register specifies the row scan of a character line where  */
/* the cursor is to begin. The number programmed should be one less than the starting cursor  */
/* row scan.                                                                                  */
/*                                                                                            */
/* Cursor End Register:                                                                       */
/* This is a write-only register pointed to when the value in the CRT Controller address      */
/* register is hex 0B. The processor output port address for this register is hex 3B5 or hex  */
/* 3D5.                                                                                       */
/* Cursor End Register Format:                                                                */
/*  - Bit 0: Row Scan Cursor Ends                                                             */
/*  - Bit 1: Row Scan Cursor Ends                                                             */
/*  - Bit 2: Row Scan Cursor Ends                                                             */
/*  - Bit 3: Row Scan Cursor Ends                                                             */
/*  - Bit 4: Row Scan Cursor Ends                                                             */
/*  - Bit 5: Cursor Skew Control                                                              */
/*  - Bit 6: Cursor Skew Control                                                              */
/*  - Bit 7: Not Used                                                                         */
/* Bit 0-Bit 4: Cursor End--These bits specify the row scan where the cursor is to end.       */
/*                                                                                            */
/* Bit 5-Bit 6: Cursor Skew--These bits control the skew of the cursor signal.                */
/* Bits 6 5:                                                                                  */
/*  -   0 0: Zero  character clock skew                                                       */
/*  -   0 1: One   character clock skew                                                       */
/*  -   1 0: Two   character clock skew                                                       */
/*  -   1 1: Three character clock skew                                                       */
/*                                                                                            */
/* Start Address High Register:                                                               */
/* This is a read/write register pointed to when the value in the CRT Controller address      */
/* register is hex 0C. The processor input/output port address for this register is hex 3B5 or*/
/* hex 3D5.                                                                                   */
/* Start Address High Register Format:                                                        */
/*  - Bit 0: High Order Start Address                                                         */
/*  - Bit 1: High Order Start Address                                                         */
/*  - Bit 2: High Order Start Address                                                         */
/*  - Bit 3: High Order Start Address                                                         */
/*  - Bit 4: High Order Start Address                                                         */
/*  - Bit 5: High Order Start Address                                                         */
/*  - Bit 6: High Order Start Address                                                         */
/*  - Bit 7: High Order Start Address                                                         */
/* Bit 0-Bit 7: Start Address High--These are the high-order eight bits of the start address. */
/* The 16-bit value, from high-order and low-order start address register, is the first       */
/* address after the vertical retrace on each screen refresh.                                 */
/*                                                                                            */
/* Start Address Low Register:                                                                */
/* This is a read/write register pointed to when the value in the CRT Controller address      */
/* register is hex 0D. The processor input/output port address for this register is hex 3B5 or*/
/* hex 3D5.                                                                                   */
/* Start Address Low Register Format:                                                         */
/*  - Bit 0: Low Order Start Address                                                          */
/*  - Bit 1: Low Order Start Address                                                          */
/*  - Bit 2: Low Order Start Address                                                          */
/*  - Bit 3: Low Order Start Address                                                          */
/*  - Bit 4: Low Order Start Address                                                          */
/*  - Bit 5: Low Order Start Address                                                          */
/*  - Bit 6: Low Order Start Address                                                          */
/*  - Bit 7: Low Order Start Address                                                          */
/* Bit 0-Bit 7: Start Address Low--These are the low-order 8 bits of the start address.       */
/*                                                                                            */
/* Cursor Location High Register:                                                             */
/* This is a read/write register pointed to when the value in the CRT Controller address      */
/* register is hex 0E. The processor input/output port address for this register is hex 3B5 or*/
/* hex 3D5.                                                                                   */
/* Cursor Location High Register Format:                                                      */
/*  - Bit 0: High Order Cursor Location                                                       */
/*  - Bit 1: High Order Cursor Location                                                       */
/*  - Bit 2: High Order Cursor Location                                                       */
/*  - Bit 3: High Order Cursor Location                                                       */
/*  - Bit 4: High Order Cursor Location                                                       */
/*  - Bit 5: High Order Cursor Location                                                       */
/*  - Bit 6: High Order Cursor Location                                                       */
/*  - Bit 7: High Order Cursor Location                                                       */
/* Bit 0-Bit 7: Cursor Location High--These are the high-order 8 bits of the cursor location. */
/*                                                                                            */
/* Cursor Location Low Register:                                                              */
/* This is a read/write register pointed to when the value in the CRT Controller address      */
/* register is hex 0F. The processor input/output port address for this register is hex 3B5 or*/
/* hex 3D5.                                                                                   */
/* Cursor Location Low Register Format:                                                       */
/*  - Bit 0: Low Order Cursor Location                                                        */
/*  - Bit 1: Low Order Cursor Location                                                        */
/*  - Bit 2: Low Order Cursor Location                                                        */
/*  - Bit 3: Low Order Cursor Location                                                        */
/*  - Bit 4: Low Order Cursor Location                                                        */
/*  - Bit 5: Low Order Cursor Location                                                        */
/*  - Bit 6: Low Order Cursor Location                                                        */
/*  - Bit 7: Low Order Cursor Location                                                        */
/* Bit 0-Bit 7: Cursor Location Low--These are the high-order 8 bits of the cursor location.  */
/*                                                                                            */
/* Vertical Retrace Start Register:                                                           */
/* This is a write-only register pointed to when the value in the CRT Controller address      */
/* register is hex 10. The processor output port address for this register is hex 3B5 or hex  */
/* 3D5.                                                                                       */
/* Vertical Retrace Start Register Format                                                     */
/*  - Bit 0: Low Order Vertical Retrace Pulse                                                 */
/*  - Bit 1: Low Order Vertical Retrace Pulse                                                 */
/*  - Bit 2: Low Order Vertical Retrace Pulse                                                 */
/*  - Bit 3: Low Order Vertical Retrace Pulse                                                 */
/*  - Bit 4: Low Order Vertical Retrace Pulse                                                 */
/*  - Bit 5: Low Order Vertical Retrace Pulse                                                 */
/*  - Bit 6: Low Order Vertical Retrace Pulse                                                 */
/*  - Bit 7: Low Order Vertical Retrace Pulse                                                 */
/* Bit 0-Bit 7: Vertical Retrace Start--This is the low-order 8 bits of the vertical retrace  */
/* pulse start position programmed in horizontal scan lines. Bit 8 is in the overflow register*/
/* location hex 07.                                                                           */
/*                                                                                            */
/* Light Pen High Register:                                                                   */
/* This is a read-only register pointed to when the value in the CRT Controller address is hex*/
/* 10. The processor input port address for this register is hex 3B5 or hex 3D5.              */
/* Light Pen High Register Format:                                                            */
/*  - Bit 0: High Order Memory Address Counter                                                */
/*  - Bit 1: High Order Memory Address Counter                                                */
/*  - Bit 2: High Order Memory Address Counter                                                */
/*  - Bit 3: High Order Memory Address Counter                                                */
/*  - Bit 4: High Order Memory Address Counter                                                */
/*  - Bit 5: High Order Memory Address Counter                                                */
/*  - Bit 6: High Order Memory Address Counter                                                */
/*  - Bit 7: High Order Memory Address Counter                                                */
/* Bit 0-Bit 7: Light Pen High--This is the high order 8 bits of the memory address counter at*/
/* the time the light pen was triggered.                                                      */
/*                                                                                            */
/* Vertical Retrace End Register:                                                             */
/* This is a write-only register pointed to when the value in the CRT Controller address      */
/* register is hex 11. The processor output port address for this register is hex 3B5 or hex  */
/* 3D5.                                                                                       */
/* Vertical Retrace End Register Format:                                                      */
/*  - Bit 0: Vertical Retrace End                                                             */
/*  - Bit 1: Vertical Retrace End                                                             */
/*  - Bit 2: Vertical Retrace End                                                             */
/*  - Bit 3: Vertical Retrace End                                                             */
/*  - Bit 4: 0=Clear  Vertical Interrupt                                                      */
/*  - Bit 5: 0=Enable Vertical Interrupt                                                      */
/*  - Bit 6: Not Used                                                                         */
/*  - Bit 7: Not Used                                                                         */
/* Bit 0-Bit 3: Vertical Retrace End--These bits determine the horizontal scan count value    */
/* when the vertical retrace output signal becomes inactive. The register is programmed in    */
/* units of horizontal scan lines. To obtain a vertical retrace signal of width W, the        */
/* following algorithm is used: Value of Start Vertical Retrace Register + width of vertical  */
/* retrace signal in horizontal scan units = 4-bit result to be programmed into the End       */
/* Horizontal Retrace Register.                                                               */
/*                                                                                            */
/* Bit 4: Clear  Vertical Interrupt--A logical 0 will clear a vertical interrupt.             */
/*                                                                                            */
/* Bit 5: Enable Vertical Interrupt--A logical 0 will enable  vertical interrupt.             */
/*                                                                                            */
/* Light Pen Low Register:                                                                    */
/* This is a read-only register pointed to when the value in the CRT Controller address       */
/* register is hex 11. The processor input port address for this register is hex 3B5 or 3D5.  */
/*  - Bit 0: Low Order Memory Address Counter                                                 */
/*  - Bit 1: Low Order Memory Address Counter                                                 */
/*  - Bit 2: Low Order Memory Address Counter                                                 */
/*  - Bit 3: Low Order Memory Address Counter                                                 */
/*  - Bit 4: Low Order Memory Address Counter                                                 */
/*  - Bit 5: Low Order Memory Address Counter                                                 */
/*  - Bit 6: Low Order Memory Address Counter                                                 */
/*  - Bit 7: Low Order Memory Address Counter                                                 */
/* Bit 0-Bit 7: Light Pen Low--This is the low-order 8 bits of the memory address counter at  */
/* the time the light pen was triggered.                                                      */
/*                                                                                            */
/* Vertical Display Enable End Register:                                                      */
/* This is a write-only register pointed to when the value in the CRT Controller address      */
/* register is hex 12. The processor output port address for this register is hex 3B5 or hex  */
/* 3D5.                                                                                       */
/* Vertical Display Enable End Register Format:                                               */
/*  - Bit 0: Low Order Vertical Display Enable End                                            */
/*  - Bit 1: Low Order Vertical Display Enable End                                            */
/*  - Bit 2: Low Order Vertical Display Enable End                                            */
/*  - Bit 3: Low Order Vertical Display Enable End                                            */
/*  - Bit 4: Low Order Vertical Display Enable End                                            */
/*  - Bit 5: Low Order Vertical Display Enable End                                            */
/*  - Bit 6: Low Order Vertical Display Enable End                                            */
/*  - Bit 7: Low Order Vertical Display Enable End                                            */
/* Bit 0-Bit 7: Vertical Display Enable End--These are the low-order 8 bits of the vertical   */
/* display enable end position. This address specifies which scan line ends the active video  */
/* area of the screen. Bit 8 is in the overflow register location hex 07.                     */
/*                                                                                            */
/* Offset Register:                                                                           */
/* This is a write-only register pointed to when the value in the CRT Controller address      */
/* register is hex 13. The processor output port address for this register is hex 3B5 or hex  */
/* 3D5.                                                                                       */
/* Offset Register Format:                                                                    */
/*  - Bit 0: Logical line width of the screen                                                 */
/*  - Bit 1: Logical line width of the screen                                                 */
/*  - Bit 2: Logical line width of the screen                                                 */
/*  - Bit 3: Logical line width of the screen                                                 */
/*  - Bit 4: Logical line width of the screen                                                 */
/*  - Bit 5: Logical line width of the screen                                                 */
/*  - Bit 6: Logical line width of the screen                                                 */
/*  - Bit 7: Logical line width of the screen                                                 */
/* Bit 0-Bit 7: Offset--This register specifies the logical line width of the screen. The     */
/* starting memory address for the next character row is larger than the current character row*/
/* by this amount. The Offset Register is programmed with a word address. Depending upon the  */
/* method of clocking the CRT Controller, this word address is either a word or double word   */
/* address.                                                                                   */
/*                                                                                            */
/* Underline Location Register:                                                               */
/* This is a write-only register pointed to when the value in the CRT Controller address is   */
/* hex 14. The processor output port address for this register is hex 3B5 or hex 3D5.         */
/* Underline Location Register Format:                                                        */
/*  - Bit 0: Horizontal row scan where underline will occur                                   */
/*  - Bit 1: Horizontal row scan where underline will occur                                   */
/*  - Bit 2: Horizontal row scan where underline will occur                                   */
/*  - Bit 3: Horizontal row scan where underline will occur                                   */
/*  - Bit 4: Horizontal row scan where underline will occur                                   */
/*  - Bit 5: Not Used                                                                         */
/*  - Bit 6: Not Used                                                                         */
/*  - Bit 7: Not Used                                                                         */
/* Bit 0-Bit 4: Underline Location--This register specifies the horizontal row scan on which  */
/* underline will occur. The value programmed is one less than the scan line number desired.  */
/*                                                                                            */
/* Start Vertical Blanking Register:                                                          */
/* This is a write-only pointed to when the value in the CRT Controller address register is   */
/* hex 15. The processor output port address for this register is hex 3B5 or hex 3D5.         */
/* Start Vertical Blanking Register Format:                                                   */
/*  - Bit 0: Start Vertical Blanking                                                          */
/*  - Bit 1: Start Vertical Blanking                                                          */
/*  - Bit 2: Start Vertical Blanking                                                          */
/*  - Bit 3: Start Vertical Blanking                                                          */
/*  - Bit 4: Start Vertical Blanking                                                          */
/*  - Bit 5: Start Vertical Blanking                                                          */
/*  - Bit 6: Start Vertical Blanking                                                          */
/*  - Bit 7: Start Vertical Blanking                                                          */
/* Bit 0-Bit 7: Start Vertical Blank--These are the low 8 bits of the horizontal scan line    */
/* count, at which the vertical blanking signal becomes active. Bit 8 bit is in the overflow  */
/* register hex 07.                                                                           */
/*                                                                                            */
/* End Vertical Blanking Register:                                                            */
/* This is a write-only register pointed to when the value in the CRT Controller address      */
/* register is hex 16. The processor output port address for this register is hex 3B5 or hex  */
/* 3D5.                                                                                       */
/* End Vertical Blanking Register Format:                                                     */
/*  - Bit 0: End Vertical Blanking                                                            */
/*  - Bit 1: End Vertical Blanking                                                            */
/*  - Bit 2: End Vertical Blanking                                                            */
/*  - Bit 3: End Vertical Blanking                                                            */
/*  - Bit 4: End Vertical Blanking                                                            */
/*  - Bit 5: Not Used                                                                         */
/*  - Bit 6: Not Used                                                                         */
/*  - Bit 7: Not Used                                                                         */
/* Bit 0-Bit 4: End Vertical Blank--This register specifies the horizontal scan count value   */
/* when the vertical blank output signal becomes inactive. The register is programmed in units*/
/* of horizontal scan lines. To obtain a vertical blank signal of width W, the following      */
/* algorithm is used: Value of Start Vertical Blank Register + width of vertical blank signal */
/* in horizontal scan units = 5-bit result to be programmed into the End Vertical Blank       */
/* Register.                                                                                  */
/*                                                                                            */
/* Mode Control Register:                                                                     */
/* This is a write-only register pointed to when the value in the CRT Controller address      */
/* register is hex 17. The processor output port address for this register is hex 3B5 or hex  */
/* 3D5.                                                                                       */
/* Mode Control Register Format:                                                              */
/*  - Bit 0: CMS 0                                                                            */
/*  - Bit 1: Select Row Scan Counter                                                          */
/*  - Bit 2: Horizontal Retrace Select                                                        */
/*  - Bit 3: Count by Two                                                                     */
/*  - Bit 4: Output Control                                                                   */
/*  - Bit 5: Address Wrap                                                                     */
/*  - Bit 6: Word/Byte Mode                                                                   */
/*  - Bit 7: Hardware Reset                                                                   */
/* Bit 0: Compatibility Mode Support-- When this bit is a logical 0, the row scan address bit */
/* 0 is substituted for memory address bit 13 during active display time. A logical 1 enables */
/* memory address bit 13 to appear on the memory address output bit 13 signal of the CRT      */
/* Controller. The CRT Controller used on the IBM Color/Graphics Monitor Adapter is the 6456. */
/* The 6845 has 128 horizontal scan line address capability. To obtain 640 by 200 graphics    */
/* resolution, the CRTC was programmed for 100 horizontal scan lines with 2 row scan addresses*/
/* per character row. Row scan address bit 0 became the most significant address bit to the   */
/* display buffer. Successive scan lines of the display image were displaced in memory by 8K  */
/* bytes. This bit allows compatibility with the 6845 and Color Graphics APA modes of         */
/* operation.                                                                                 */
/*                                                                                            */
/* Bit 1: Select Row Scan Counter--A logical 0 selects row scan counter bit 1 on MA 14 output */
/* pin. A logical 1 selects MA 14 counter bit on MA 14 output pin.                            */
/*                                                                                            */
/* Bit 2: Horizontal Retrace Select--This bit selects Horizontal Retrace or Horizontal Retrace*/
/* divided by 2 as the clock that controls the vertical timing counter. This bit can be used  */
/* to effectively double the vertical resolution capability of the CRT Controller. The        */
/* vertical counter has a maximum resolution of 512 scan lines due to the 9-bit wide Vertical */
/* Total Register. If the vertical counter is clocked with the horizontal retrace divided by 2*/
/* clock, then the vertical resolution is doubled to 1024 horizontal scan lines. A logical 0  */
/* selects HRTC and a logical 1 selects HRTC divided by 2.                                    */
/*                                                                                            */
/* Bit 3: Count By Two-- When this bit is set to 0, the memory address counter is clocked with*/
/* the character clock input. A logical 1 clocks the memory address counter with the character*/
/* clock input divided by 2. This bit is used to create either a byte or word refresh address */
/* for the display buffer.                                                                    */
/*                                                                                            */
/* Bit 4: Output Control--A logical 0 enables the module output drivers. A logical 1 forces   */
/* all outputs into high impedance state.                                                     */
/*                                                                                            */
/* Bit 5: Address Wrap--This bit selects Memory Address counter bit MA 13 or bit MA 15, and it*/
/* appears on the MA 0 output pin in the word address mode. If you are not in the word address*/
/* mode, MA 0 counter output appears on the MA 0 output pin. A logical 1 selects MA 15. In odd*/
/* /even mode, bit MA 13 should be selected when the 64K memory is installed on the board. Bit*/
/* MA 15 should be selected when greater than 64K memory is installed. This function is used  */
/* implement Color Graphics Monitor Adapter compatibility.                                    */
/*                                                                                            */
/* Bit 6: Word Mode or Byte Mode--When this bit is a logical 0, the Word Mode shifts all      */
/* memory address counter bits down one bit, and the most significant bit of the counter      */
/* appears on the least significant bit of the memory address outputs. See table below for    */
/* address output details. A logical 1 selects the Byte Address mode.                         */
/*               Internal Memory Address Counter                                              */
/*               Wiring to the Output Multiplexer:                                            */
/* CRTC Out Pin: Byte Address Mode: Word Address Mode:                                        */
/*  - MA 0/RFA 0: - MA  0:           - MA 15 or MA 13:                                        */
/*  - MA 1/RFA 1: - MA  1:           - MA  0:                                                 */
/*  - MA 2/RFA 2: - MA  2:           - MA  1:                                                 */
/*  - MA 3/RFA 3: - MA  3:           - MA  2:                                                 */
/*  -             -                  -                                                        */
/*  -             -                  -                                                        */
/*  -             -                  -                                                        */
/*  - MA 14/RS 3: - MA 14:           - MA 13:                                                 */
/*  - MA 15/RS 4: - MA 15:           - MA 14:                                                 */
/* Bit 7: Hardware Reset--A logical 0 forces horizontal and vertical retrace to clear. A      */
/* logical 1 forces horizontal and vertical retrace to be enabled.                            */
/*                                                                                            */
/* Line Compare Register:                                                                     */
/* This is a write-only register pointed to when the value in the CRT Controller address      */
/* register is hex 18. The processor output port address for this register is hex 3B5 or hex  */
/* 3D5.                                                                                       */
/* Line Compare Register Format:                                                              */
/*  - Bit 0: Line Compare Target                                                              */
/*  - Bit 1: Line Compare Target                                                              */
/*  - Bit 2: Line Compare Target                                                              */
/*  - Bit 3: Line Compare Target                                                              */
/*  - Bit 4: Line Compare Target                                                              */
/*  - Bit 5: Line Compare Target                                                              */
/*  - Bit 6: Line Compare Target                                                              */
/*  - Bit 7: Line Compare Target                                                              */
/* Bit 0-Bit 7: Line Compare--This register is the low-order 8 bits of the compare target.    */
/* When the vertical counter reaches this value, the internal start of the line counter is    */
/* cleared. This allows an area of the screen to be immune to scrolling. Bit 8 of this        */
/* register is in the overflow register hex 07.                                               */
/* ------------------------------------------------------------------------------------------ */
#define EGA_CRT_HT        0		/* Address Register: Horizontal Total */
#define EGA_CRT_HD        1		/* Address Register: Horizontal Display Enable End */
#define EGA_CRT_HBS       2		/* Address Register: Start Horizontal Blank */
#define EGA_CRT_HBE       3		/* Address Register: End   Horizontal Blank */
#define EGA_CRT_HRS       4		/* Address Register: Start Horizontal Retrace */
#define EGA_CRT_HRE       5		/* Address Register: End   Horizontal Retrace */
#define EGA_CRT_VT        6		/* Address Register: Vertical Total */
#define EGA_CRT_OF        7		/* Address Register: Overflow */
#define EGA_CRT_MS        9		/* Address Register: Maximum Scan Line */
#define EGA_CRT_CS        10	/* Address Register: Cursor Start */
#define EGA_CRT_CE        11    /* Address Register: Cursor End */
#define EGA_CRT_SAH       12	/* Address Register: Start Address High */
#define EGA_CRT_SAL       13	/* Address Register: Start Address Low */
#define EGA_CRT_CLH       14	/* Address Register: Cursor Location High */
#define EGA_CRT_CLL       15	/* Address Register: Cursor Location low */
#define EGA_CRT_VRS       16	/* Address Register: Vertical Retrace Start */
#define EGA_CRT_VRE       17	/* Address Register: Vertical Retrace End */
#define EGA_CRT_VD        18	/* Address Register: Vertical Display Enable End */
#define EGA_CRT_OFS       19	/* Address Register: Offset */
#define EGA_CRT_ULL       20	/* Address Register: Underline Location */
#define EGA_CRT_MODE      23	/* Address Register: Mode Control */
#define EGA_CRT_LC        24    /* Address Register: Line Compare */
/* ----------------------------------------------------------------------------------- */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Registers:              */
/* ----------------------------------------------------------------------------------- */
/* External Registers:                                                                 */
/* This section contains descriptions of the registers of the Enhanced Graphics Adapter*/
/* that are not contained in an LSI device.                                            */
/* Name:                              Port:  Index:                                    */
/*  - Miscellaneous Output Register  : - 3C2: - -:                                     */
/*  - Feature Control      Register  : - 3?A: - -:                                     */
/*  - Input Status         Register 0: - 3C2: - -:                                     */
/*  - Input Status         Register 1: - 3?2: - -:                                     */
/* ? = B in Monochrome Modes    ? = D in Color Modes                                   */
/* Input Status Register Zero:                                                         */
/* This is a read-only register. The processor input port address is hex 3C2.          */
/* Input Status Register Zero Format:                                                  */
/*  - Bit 0: Not Used                                                                  */
/*  - Bit 1: Not Used                                                                  */
/*  - Bit 2: Not Used                                                                  */
/*  - Bit 3: Not Used                                                                  */
/*  - Bit 4: Switch Sense                                                              */
/*  - Bit 5: Reserved                                                                  */
/*  - Bit 6: Reserved                                                                  */
/*  - Bit 7: CRT Interrupt                                                             */
/* Bit 4: Switch Sense--When set to 1, this bit allows the processor to read the four  */
/* configuration switches on the board. The setting of the CLKSEL field determines     */
/* which switch is being read. The switch configuration can be determined by reading   */
/* byte 40:88H in RAM.                                                                 */
/*  - Bit 3: Switch 4 ; Logical 0 = switch closed                                      */
/*  - Bit 2: Switch 3 ; Logical 0 = switch closed                                      */
/*  - Bit 1: Switch 2 ; Logical 0 = switch closed                                      */
/*  - Bit 0: Switch 1 ; Logical 0 = switch closed                                      */
/*                                                                                     */
/* Bits 5 and 6: Feature Code--These bits are input from the Feat (0) and Feat (1) pins*/
/* on the feature connector.                                                           */
/*                                                                                     */
/* Bit 7: CRT Interrupt--A logical 1 indicates video is being displayed on the CRT     */
/* screen; a logical 0 indicates that vertical retrace is occurring.                   */
/* ----------------------------------------------------------------------------------- */
#define EGA_STATUS0_SS    0x10		/* Switch Sense  */
#define EGA_STATUS0_CI    0x80		/* CRT Interrupt */
/* ----------------------------------------------------------------------------------- */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Registers:              */
/* ----------------------------------------------------------------------------------- */
/* External Registers:                                                                 */
/* This section contains descriptions of the registers of the Enhanced Graphics Adapter*/
/* that are not contained in an LSI device.                                            */
/* Name:                              Port:  Index:                                    */
/*  - Miscellaneous Output Register  : - 3C2: - -:                                     */
/*  - Feature Control      Register  : - 3?A: - -:                                     */
/*  - Input Status         Register 0: - 3C2: - -:                                     */
/*  - Input Status         Register 1: - 3?2: - -:                                     */
/* ? = B in Monochrome Modes    ? = D in Color Modes                                   */
/* Miscellaneous Output Register:                                                      */
/* This is a write-only register. The processor output port address is hex 3C2. A      */
/* hardware reset causes all bits to reset to zero.                                    */
/* Miscellaneous Output Register Format:                                               */
/*  - Bit 0: I/O Address Select                                                        */
/*  - Bit 1: Enable Ram                                                                */
/*  - Bit 2: Clock Select 0                                                            */
/*  - Bit 3: Clock Select 1                                                            */
/*  - Bit 4: Disable Internal Video Drivers                                            */
/*  - Bit 5: Page Bit For Odd/Even                                                     */
/*  - Bit 6: Horizontal Retrace Polarity                                               */
/*  - Bit 7: Vertical   Retrace Polarity                                               */
/* Bit 0: 3BX/3DX CRTC I/O Address--This bit maps the CRTC I/O addresses for IBM       */
/* Monochrome or Color/Graphics Monitor Adapter emulation. A logical 0 sets CRTC       */
/* addresses to 3BX and Input Status Register 1 's address to 3BA for Monochrome       */
/* emulation. A logical 1 sets CRTC addresses to 3DX and Input Status Register 1's     */
/* address to 3DA for Color/Graphics Monitor Adapter emulation.                        */
/*                                                                                     */
/* Bit 1: Enable RAM--A logical 0 disables RAM from the processor; a logical 1 enables */
/* RAM to respond at addresses designated by the Control Data Select value programmed  */
/* into the Graphics Controllers.                                                      */
/*                                                                                     */
/* Bit 2-Bit 3: Clock Select--These two bits select the clock source according to the  */
/* following table:                                                                    */
/* Bits:                                                                               */
/*  - 3 2:                                                                             */
/* --------                                                                            */
/*  - 0 0-: Selects 14 MHz clock from the processor I/O channel                        */
/*  - 0 1-: Selects 16 MHz clock on-board oscillator                                   */
/*  - 1 0-: Selects external clock source from the feature connector.                  */
/*  - 1 1-: Not used                                                                   */
/*                                                                                     */
/* Bit 4: Disable Internal Video Drivers--A logical 0 activates internal video drivers;*/
/* a logical 1 disables internal video drivers. When the internal video drivers are    */
/* disabled, the source of the direct drive color output becomes the feature connector */
/* direct drive outputs.                                                               */
/*                                                                                     */
/* Bit 5: Page Bit For Odd/Even--Selects between two 64K pages of memory when in the   */
/* Odd/Even modes (0,1,2,3,7). A logical 0 selects the low page of memory; a logical 1 */
/* selects the high page of memory.                                                    */
/*                                                                                     */
/* Bit 6: Horizontal Retrace Polarity--A logical 0 selects positive horizontal retrace;*/
/* a logical 1 selects negative horizontal retrace.                                    */
/*                                                                                     */
/* Bit 7: Vertical   Retrace Polarity--A logical 0 selects positive vertical   retrace;*/
/* a logical 1 selects negative vertical   retrace.                                    */
/* ----------------------------------------------------------------------------------- */
#define EGA_MOUT_IOS      0x01		/* I/O Address Select */
#define EGA_MOUT_ERAM     0x02		/* Enable RAM */
#define EGA_MOUT_CS       0x04		/* Clock Select */
#define EGA_MOUT_HSP      0x40		/* Horizontal Retrace Polarity */
#define EGA_MOUT_VSP      0x80		/* Vertical   Retrace Polarity */
/* ----------------------------------------------------------------------------------- */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Registers:              */
/* ----------------------------------------------------------------------------------- */
/* External Registers:                                                                 */
/* This section contains descriptions of the registers of the Enhanced Graphics Adapter*/
/* that are not contained in an LSI device.                                            */
/* Name:                              Port:  Index:                                    */
/*  - Miscellaneous Output Register  : - 3C2: - -:                                     */
/*  - Feature Control      Register  : - 3?A: - -:                                     */
/*  - Input Status         Register 0: - 3C2: - -:                                     */
/*  - Input Status         Register 1: - 3?2: - -:                                     */
/* ? = B in Monochrome Modes    ? = D in Color Modes                                   */
/* Input Status Register One:                                                          */
/* This is a read-only register. The processor port address is hex 3BA or hex 3DA.     */
/* Input Status Register One Format:                                                   */
/*  - Bit 0: Display Enable                                                            */
/*  - Bit 1: Light Pen Strobe                                                          */
/*  - Bit 2: Light Pen Switch                                                          */
/*  - Bit 3: Vertical Retrace                                                          */
/*  - Bit 4: Diagnostic 1                                                              */
/*  - Bit 5: Diagnostic 0                                                              */
/*  - Bit 6: Not Used                                                                  */
/* Bit 0: Display Enable--Logical 0 indicates the CRT raster is in a horizontal or     */
/* vertical retrace interval. This bit is the real time status of the display enable   */
/* signal. Some programs use this status bit to restrict screen updates to inactive    */
/* display intervals. The Enhanced Graphics Adapter does not require the CPU to update */
/* the screen buffer during inactive display intervals to avoid glitches in the display*/
/* image.                                                                              */
/*                                                                                     */
/* Bit 1: Light Pen Strobe--A logical 0 indicates that the light pen trigger has not   */
/* been set; a logical 1 indicates that the light pen trigger has been set.            */
/*                                                                                     */
/* Bit 2: Light Pen Switch--A logical 0 indicates that the light pen switch is closed; */
/* a logical 1 indicates that the light pen switch is open.                            */
/*                                                                                     */
/* Bit 3: Vertical Retrace--A logical 0 indicates that video information is being      */
/* displayed on the CRT screen; a logical 1 indicates the CRT is in a vertical retrace */
/* interval. This bit can be programmed to interrupt the processor on interrupt level 2*/
/* at the start of the vertical retrace. This is done through bits 4 and 5 of the      */
/* Vertical Retrace End Register of the CRTC.                                          */
/*                                                                                     */
/* Bits 4 and 5: Diagnostic Usage--These bits are selectively connected to two of the  */
/* six color outputs of the Attribute Controller. The Color Plane Enable register      */
/* controls the multiplexer for the video wiring. The following table illustrates the  */
/* combinations available and the color output wiring.                                 */
/* Color Plane Register: Input Status Register One:                                    */
/* Bit 5: Bit 4:         Bit 5:            Bit 4:                                      */
/*  - 0:   - 0:           - Red:            - Blue:                                    */
/*  - 0:   - 1:           - Secondary Blue: - Green:                                   */
/*  - 1:   - 0:           - Secondary Red:  - Secondary Green                          */
/*  - 1:   - 1:           - Not Used:       - Not Used:                                */
/* ----------------------------------------------------------------------------------- */
#define EGA_STATUS1_DE    0x01		/* Display Enable   */
#define EGA_STATUS1_LPS_1 0x02      /* Light Pen Strobe */
#define EGA_STATUS1_LPS_2 0x04      /* Light Pen Switch */
#define EGA_STATUS1_VR    0x08      /* Vertical Retrace */
#define EGA_STATUS1_DIAG  0x10      /* Diagnostic Usage */
/* ------------------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Attribute Controller Registers:*/
/* ------------------------------------------------------------------------------------------ */
/* Name:                             Port:  Index:                                            */
/*  - Mode Control           Register: - 3C0: - 10:                                           */
/* Mode Control Register:                                                                     */
/* This is a write-only register pointed to by the value in the Attribute address register.   */
/* This value must be hex 10 before writing can take place. The processor output port address */
/* for this register is hex 3C0.                                                              */
/* Mode Control Register Format:                                                              */
/*  - Bit 0: Graphics/Alphanumeric Mode                                                       */
/*  - Bit 1: Display Type                                                                     */
/*  - Bit 2: Enable Line Graphics Character Codes                                             */
/*  - Bit 3: Select Background Intensity or Enable Blink                                      */
/*  - Bit 4: Not Used                                                                         */
/*  - Bit 5: Not Used                                                                         */
/*  - Bit 6: Not Used                                                                         */
/*  - Bit 7: Not Used                                                                         */
/* Bit 0: Graphics/Alphanumeric Mode--A logical 0 selects selects alphanumeric mode. A logical*/
/* 1 selects graphics mode.                                                                   */
/*                                                                                            */
/* Bit 1: Monochrome Display/Color Display--A logical 0 selects color display attributes. A   */
/* logical 1 selects IBM Monochrome Display attributes.                                       */
/*                                                                                            */
/* Bit 2: Enable Line Graphics Character Codes--When this bit is set to 0, the ninth dot will */
/* be the same as the background. A logical 1 enables the special line graphics character     */
/* codes for the IBM Monochrome Display adapter. This bit when enabled forces the ninth dot of*/
/* a line graphic character to be identical to the eighth dot of the character. The line      */
/* graphics character codes for the Monochrome Display Adapter are Hex C0 through Hex DF.     */
/*                                                                                            */
/* For character fonts that do not utilize the line graphics character codes in the range of  */
/* Hex C0 through Hex DF, bit 2 of this register should be a logical 0. Otherwise unwanted    */
/* video information will be displayed on the CRT screen.                                     */
/*                                                                                            */
/* Bit 3: Enable Blink/Select Background Intensity--A logical 0 selects the background        */
/* intensity of the attribute input. This mode was available on the Monochrome and Color      */
/* Graphics adapters. A logical 1 enables the blink attribute in alphanumeric modes. This bit */
/* must also be set to 1 for blinking graphics modes.                                         */
/* ------------------------------------------------------------------------------------------ */
#define EGA_ATC_MODE_G    0x01		/* Mode Control Register: Graphics/Alphanumeric Mode */
#define EGA_ATC_MODE_ME   0x02		/* Mode Control Register: Monochrome Display /Color Display */
#define EGA_ATC_MODE_ELG  0x04		/* Mode Control Register: Enable Line Graphics Character Codes */
#define EGA_ATC_MODE_EB   0x08		/* Mode Control Rergister: Enable Blink/Select Background Intensity */
/* ------------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Display, August 2nd, 1984: Sequencer Registers:     */
/* ------------------------------------------------------------------------------------ */
/* Reset Register:                                                                      */
/* This is a write-only register pointed to when the value in the address register is   */
/* hex 00. The output port address for this register is hex 3C5.                        */
/* Reset Register Format:                                                               */
/*  - Bit 0:Asynchronous Reset                                                          */
/*  - Bit 1: Synchronous Reset                                                          */
/*  - Bit 2: Not Used                                                                   */
/*  - Bit 3: Not Used                                                                   */
/*  - Bit 4: Not Used                                                                   */
/*  - Bit 5: Not Used                                                                   */
/*  - Bit 6: Not Used                                                                   */
/*  - Bit 7: Not Used                                                                   */
/* Bit 0:Asynchronous Reset--A logical 0 commands the sequencer to asynchronous clear   */
/* and halt. All outputs are placed in the high impedance state when this bit is a 0. A */
/* logical 1 commands the sequencer to run unless bit 1 is set to zero. Resetting the   */
/* sequencer with this bit can cause data loss in the dynamic RAMs.                     */
/*                                                                                      */
/* Bit 1: Synchronous Reset--A logical 0 commands the sequencer to synchronous clear and*/
/* halt. Bits 1 and 0 must both be ones to allow the sequencer to operate. Reset the    */
/* sequencer with this bit before changing the Clocking Mode Register, if memory        */
/* contents are to be preserved.                                                        */
/* ------------------------------------------------------------------------------------ */
#define EGA_SEQ_RESET_ASR 0x01		/* asynchronous reset */
#define EGA_SEQ_RESET_SR  0x02		/* synchronous reset */
/* ------------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Display, August 2nd, 1984: Sequencer Registers:     */
/* ------------------------------------------------------------------------------------ */
/* Clocking Mode Register:                                                              */
/* This is write-only register pointed to when the value in the address register is hex */
/* 01. The output port address for this register is hex 3C5.                            */
/* Clocking Mode Register Format:                                                       */
/*  - Bit 0: 8/9 Dot Clocks                                                             */
/*  - Bit 1: Bandwidth                                                                  */
/*  - Bit 2: Shift Load                                                                 */
/*  - Bit 3: Dot Clock                                                                  */
/*  - Bit 4: Not Used                                                                   */
/*  - Bit 5: Not Used                                                                   */
/*  - Bit 6: Not Used                                                                   */
/*  - Bit 7: Not Used                                                                   */
/* Bit 0: 8/9 Dot Clocks--A logical 0 directs the sequencer to generate character clocks*/
/* 9 dots wide; a logical 1 directs the sequencer to generate character clocks 8 dots   */
/* wide. Monochrome alphanumeric mode (07H) is the only mode that uses character clocks */
/* 9 dots wide. All other modes must use 8 dots per character clock.                    */
/*                                                                                      */
/* Bit 1: Bandwidth--A logical 0 makes CRT memory cycles occur on 4 out of 5 available  */
/* memory cycles; a logical 1 makes CRT memory cycles occur on 2 out of 5 available     */
/* memory cycles. Medium resolution modes require less data to be fetched from the      */
/* display buffer during the horizontal scan time. This allows the CPU greater access   */
/* time to the display buffer. All high resolution modes must provide the CRTC with 4   */
/* out of 5 memory cycles in order to refresh the display image.                        */
/*                                                                                      */
/* Bit 2: Shift Load--When set to 0, the video serializers are reloaded every character */
/* clock; when set to 1, the video serializers are loaded every other character clock.  */
/* This mode is useful when 16 bits are fetched per cycle and chained together in the   */
/* shift registers.                                                                     */
/*                                                                                      */
/* Bit 3: Dot Clock--A logical 0 selects normal dot clocks derived from the sequencer   */
/* master clock input. When this bit is set to 1, the master clock will be divided by 2 */
/* to generate the dot clock. All the other timings will be stretched since they are    */
/* derived from the dot clock. Dot clock divided by two is used for 320x200 modes (0, 1,*/
/* 4, 5) to provide a pixel rate of 7 MHz, (9 MHz for mode D).                          */
/* ------------------------------------------------------------------------------------ */
#define EGA_SEQ_CLOCK_D89 0x01		/* Sequencer Registers: 8/9 Dot Clocks */
#define EGA_SEQ_CLOCK_SL  0x04		/* Sequencer Registers: Shift Load */
#define EGA_SEQ_CLOCK_DC  0x08		/* Sequencer Registers: Dot Clock */
#define EGA_SEQ_CLOCK_SH4 0x10		/* shift 4 */
#define EGA_SEQ_CLOCK_SO  0x20		/* screen off */
/* ------------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Display, August 2nd, 1984: Sequencer Registers:     */
/* ------------------------------------------------------------------------------------ */
/* Memory Mode Register:                                                                */
/* This is a write-only register pointed to when the value in the address register is   */
/* hex 04. The processor output port address for this register is 3C5.                  */
/* Memory Mode Register Format:                                                         */
/*  - 0: Alpha                                                                          */
/*  - 1: Extended Memory                                                                */
/*  - 2: Odd/Even                                                                       */
/*  - 3: Not Used                                                                       */
/*  - 4: Not Used                                                                       */
/*  - 5: Not Used                                                                       */
/*  - 6: Not Used                                                                       */
/*  - 7: Not Used                                                                       */
/* Bit 0: Alpha--A logical 0 indicates that a non-alpha mode is active. A logical 1     */
/* indicates that alpha mode is active and enables the character generator map select   */
/* function.                                                                            */
/*                                                                                      */
/* Bit 1: Extended Memory--A logical 0 indicates that the memory expansion card is not  */
/* installed. A logical 1 indicates that the memory expansion card is installed and     */
/* enables access to the extended memory through address bits 14 and 15.                */
/*                                                                                      */
/* Bit 2: Odd/Even--A logical 0 directs even processor addresses to access maps 0 and 2,*/
/* while odd processor addresses access maps 1 and 3. A logical 1 causes processor      */
/* addresses to sequentially access data within a bit map. The maps are accessed        */
/* according to the value in the map mask register.                                     */
/* ------------------------------------------------------------------------------------ */
#define EGA_SEQ_MODE_EM   0x02		/* Sequencer Registers: Extended Memory */
#define EGA_SEQ_MODE_OE   0x04		/* Sequencer Registsrs: Odd/Even */
#define EGA_SEQ_MODE_CH4  0x08		/* chain 4 */

#define EGA_GRC_MODE_WM   0x03		/* write mode */
#define EGA_GRC_MODE_RM   0x08		/* read mode */
#define EGA_GRC_MODE_OE   0x10		/* odd/even mode */
#define EGA_GRC_MODE_SR   0x20

#define EGA_GRC_MISC_GM   0x01		/* graphics mode */
#define EGA_GRC_MISC_OE   0x02		/* odd/even mode */
#define EGA_GRC_MISC_MM   0x0c		/* memory map */
/* ------------------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: CRT Control Registers:         */
/* ------------------------------------------------------------------------------------------ */
/* Vertical Retrace Start Register:                                                           */
/* This is a write-only register pointed to when the value in the CRT Controller address      */
/* register is hex 10. The processor output port address for this register is hex 3B5 or hex  */
/* 3D5.                                                                                       */
/* Vertical Retrace Start Register Format                                                     */
/*  - Bit 0: Low Order Vertical Retrace Pulse                                                 */
/*  - Bit 1: Low Order Vertical Retrace Pulse                                                 */
/*  - Bit 2: Low Order Vertical Retrace Pulse                                                 */
/*  - Bit 3: Low Order Vertical Retrace Pulse                                                 */
/*  - Bit 4: Low Order Vertical Retrace Pulse                                                 */
/*  - Bit 5: Low Order Vertical Retrace Pulse                                                 */
/*  - Bit 6: Low Order Vertical Retrace Pulse                                                 */
/*  - Bit 7: Low Order Vertical Retrace Pulse                                                 */
/* Bit 0-Bit 7: Vertical Retrace Start--This is the low-order 8 bits of the vertical retrace  */
/* pulse start position programmed in horizontal scan lines. Bit 8 is in the overflow register*/
/* location hex 07.                                                                           */
/*                                                                                            */
/* Vertical Retrace End Register:                                                             */
/* This is a write-only register pointed to when the value in the CRT Controller address      */
/* register is hex 11. The processor output port address for this register is hex 3B5 or hex  */
/* 3D5.                                                                                       */
/* Vertical Retrace End Register Format:                                                      */
/*  - Bit 0: Vertical Retrace End                                                             */
/*  - Bit 1: Vertical Retrace End                                                             */
/*  - Bit 2: Vertical Retrace End                                                             */
/*  - Bit 3: Vertical Retrace End                                                             */
/*  - Bit 4: 0=Clear  Vertical Interrupt                                                      */
/*  - Bit 5: 0=Enable Vertical Interrupt                                                      */
/*  - Bit 6: Not Used                                                                         */
/*  - Bit 7: Not Used                                                                         */
/* Bit 0-Bit 3: Vertical Retrace End--These bits determine the horizontal scan count value    */
/* when the vertical retrace output signal becomes inactive. The register is programmed in    */
/* units of horizontal scan lines. To obtain a vertical retrace signal of width W, the        */
/* following algorithm is used: Value of Start Vertical Retrace Register + width of vertical  */
/* retrace signal in horizontal scan units = 4-bit result to be programmed into the End       */
/* Horizontal Retrace Register.                                                               */
/*                                                                                            */
/* Bit 4: Clear  Vertical Interrupt--A logical 0 will clear a vertical interrupt.             */
/*                                                                                            */
/* Bit 5: Enable Vertical Interrupt--A logical 0 will enable  vertical interrupt.             */
/* ------------------------------------------------------------------------------------------ */
#define EGA_CRT_VRE_CVI   0x10		/* CRT Control Registers: Vertical Retrace End Register: Clear  Vertical Interrupt */
#define EGA_CRT_VRE_EVI   0x20		/* CRT Control Registers: Vertical Retrace End Register: Enable Vertical Interrupt */

#define EGA_CRT_MODE_CMS0 0x01
#define EGA_CRT_MODE_WB   0x40		/* byte/word mode */
/* ----------------------------------------------------------------------------------- */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Registers:              */
/* ----------------------------------------------------------------------------------- */
/* External Registers:                                                                 */
/* This section contains descriptions of the registers of the Enhanced Graphics Adapter*/
/* that are not contained in an LSI device.                                            */
/* Name:                              Port:  Index:                                    */
/*  - Miscellaneous Output Register  : - 3C2: - -:                                     */
/*  - Feature Control      Register  : - 3?A: - -:                                     */
/*  - Input Status         Register 0: - 3C2: - -:                                     */
/*  - Input Status         Register 1: - 3?2: - -:                                     */
/* ? = B in Monochrome Modes    ? = D in Color Modes                                   */
/* Feature Control Register:                                                           */
/* This is a write-only register. The processor output register is hex 3Ba or 3DA.     */
/* Feature Control Register Format:                                                    */
/*  - Bit 0: Feature Control Bit 0                                                     */
/*  - Bit 1: Feature Control Bit 1                                                     */
/*  - Bit 2: Reserved                                                                  */
/*  - Bit 3: Reserved                                                                  */
/*  - Bit 4: Not Used                                                                  */
/*  - Bit 5: Not Used                                                                  */
/*  - Bit 6: Not Used                                                                  */
/*  - Bit 7: Not Used                                                                  */
/* Bits 0 and 1: Feature Control Bits--These bits are used to convey information to the*/
/* feature connector. The output of these bits goes to the FEAT 0 (pin 19) and FEAT 1  */
/* (pin 17) of the feature connector.                                                  */
/* ----------------------------------------------------------------------------------- */
#define EGA_FEAT_0      0x01 /* Feature Control Registers (FEAT 0) */
#define EGA_FEAT_1      0x02 /* Feature Control Registers (FEAT 1) */

#define EGA_UPDATE_DIRTY   1
#define EGA_UPDATE_RETRACE 2

/* --------------------------------------------------------------------------- */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Description:    */
/* --------------------------------------------------------------------------- */
/* The IBM Enhanced Graphics Adapter (EGA) is a graphics controller that       */
/* supports both color and monochrome direct drive displays in a variety of    */
/* modes. In addition to the direct drive port, a light pen interface is       */
/* provided. Advanced features on the adapter include bit-mapped graphics in   */
/* four planes and a RAM (Random Access Memory) loadable character generator.  */
/* Design features in the hardware substantially reduce the software overhead  */
/* for many graphics functions.                                                */
/*                                                                             */
/* The Enhanced Graphics Adapter provides Basic Input Output System (BIOS)     */
/* support for both alphanumeric (A/N) modes and all-points-addressable (APA)  */
/* graphics modes, including all modes supported by the Monochrome Display     */
/* Adapter and the Color/Graphics Monitor Adapter. Other modes provide APA     */
/* 640x350 pel graphics support for the IBM Monochrome Display, full 16 color  */
/* support in both 320x200 pel and 640x200 pel resolutions for the IBM Color   */
/* Display, and both A/N and APA support with resolution of 640x350 for the IBM*/
/* Enhanced Color Display. In alphanumeric modes, characters are formed from   */
/* one of two ROM (Read Only Memory) character generators on the adapter. One  */
/* character generator defines 7x9 characters in a 9x14 character box. For     */
/* Enhanced Color Display support, the 9x14 character set is modified to       */
/* provide an 8x14 character set. The second character generator defines 7x7   */
/* characters in an 8x8 character box. These generators contain dot patterns   */
/* for 256 different characters. The character sets are identical to those     */
/* provided by the IBM Monochrome Display Adapter and the IBM Color/Graphics   */
/* Monitor Adapter.                                                            */
/*                                                                             */
/* The adapter contains 64K bytes of storage configured as four 16K byte bit   */
/* planes. Memory expansion options are available to expand the adapter memory */
/* to 128K bytes or 256K bytes.                                                */
/*                                                                             */
/* The adapter is packaged on a single 13-1/8 inch (333.50 mm) card. The direct*/
/* drive port is a right-angle mounted connector at the rear of the adapter and*/
/* extends through the rear panel of the system unit. Also on the card are five*/
/* large scale integration (LS) modules custom designed for this controller.   */
/*                                                                             */
/* Located on the adapter is a feature connector that provides access to       */
/* internal functions through a 32-pin berg connector. A separate 64-pin       */
/* connector provides an interface for graphics memory expansion.              */
/*                                                                             */
/* The following is a block diagram of the Enhanced Graphics Adapter:          */
/*  - Enhanced Graphics Adapter Block Diagram (FILE INCLUDED IN NEXT RELEASE)  */
/* --------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Major Components:  */
/* ------------------------------------------------------------------------------ */
/* BIOS:                                                                          */
/* A read-only memory (ROM) Basic Input Output System (BIOS) module on the adapter*/
/* is linked to the system BIOS. This ROM BIOS contains character generators and  */
/* control code and is mapped into the processor address at hex C0000 for a length*/
/* of 16K bytes.                                                                  */
/* ------------------------------------------------------------------------------ */
/* ----------------------------------------------------------------------------------- */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Basic Operations:       */
/* ----------------------------------------------------------------------------------- */
/* Alphanumeric Modes:                                                                 */
/* The data format for alphanumeric modes on the Enhanced Graphics Adapter is the same */
/* as the data format on the IBM Color/Graphics Monitor Adapter and the IBM Monochrome */
/* Display Adapter. As an added function, bit three of the attribute byte may be       */
/* redefined by the Character Map Select register to act as a switch between character */
/* sets. This gives the programmer access to 512 characters at one time. This function */
/* is valid only when memory has been expanded to 128K bytes or more.                  */
/*                                                                                     */
/* When an alphanumeric mode is selected, the BIOS transfers character patterns from   */
/* the ROM to bit plane 2. The processor stores the character data in bit plane 0, and */
/* the attribute data in bit plane 1. The programmer can view bit planes 0 and 1 as a  */
/* single buffer in alphanumeric modes. The CRTC generates sequential addresses, and   */
/* fetches one character code byte and one attribute byte at a time. The character code*/
/* and row scan count address bit plane 2, which contains the character generates. The */
/* appropriate dot patterns are then sent to the palette in the attribute chip, where  */
/* color is assigned according to the attribute data.                                  */
/*                                                                                     */
/* Graphics Modes:                                                                     */
/* 320x200 Two and Four Color Graphics (Modes 4 and 5):                                */
/* Addressing, mapping and data format are the same as the 320x200 pel mode of the     */
/* Color/Graphics Monitor Adapter. The display buffer is configured at hex B8000. Bit  */
/* image data is stored in planes 0 and 1.                                             */
/*                                                                                     */
/* 640x200 Two Color Graphics (Mode 6):                                                */
/* Addressing, mapping and data format are the same as the 640x200 pel black and white */
/* mode of the Color/Graphics Monitor Adapter. The display buffer is configured at hex */
/* B8000. Bit image data is stored in bit plane 0.                                     */
/*                                                                                     */
/* 640x350 Monochrome Graphics (Mode F):                                               */
/* This mode supports graphics on the IBM Monochrome Display with the following        */
/* attributes: black, video, blinking video, and intensified video. Resolution of      */
/* 640x350 requires 56K bytes to support four attributes. By chaining maps 0 and 1,    */
/* then maps 2 and 3 together, two 32K bit planes can be formed. This chaining is done */
/* only when necessary (less than 128K of graphics memory). The first map is the video */
/* bit plane, and the second map is the intensity bit plane. Both planes reside at hex */
/* address A0000.                                                                      */
/*                                                                                     */
/* Two bits, one from each bit plane, define one picture element (pel) on the screen.  */
/* The bit definitions for the pels are given in the following table. The video bit    */
/* plane is denoted by C0 and the Intensity Bit Plane is denoted by C2.                */
/*                 C2:  C0:  Pixel Color:         Valid Attributes:                    */
/*                  - 0: - 0: -             Black: - 0                                 */
/*                  - 0: - 1: -             Video: - 3                                 */
/*                  - 1: - 0: - Blinking    Video: - C                                 */
/*                  - 1: - 1: - Intensified Video: - F                                 */
/* The byte organization in memory is sequential. The first eight pels on the screen   */
/* are defined by the contents of memory in location A000:0H, the second eight pels by */
/* location A000:1H, and so on. The first pel within any one byte is defined by bit 7  */
/* in the byte. The last pel within the byte is defined by bit 0 in the byte.          */
/*                                                                                     */
/* Monochrome graphics works in odd/even mode, which means that even CPU addresses go  */
/* into even bit planes and odd CPU addresses go into odd bit planes. Since both bit   */
/* planes reside at address A0000, the user must select which plane or planes he       */
/* desires to update. This is accomplished by the map mask register of the sequencer.  */
/* (See the table above for valid attributes).                                         */
/*                                                                                     */
/* 16/64 Color Graphics Modes (Mode 10):                                               */
/* These modes support graphics in 16 colors on either a medium or high resolution     */
/* monitor. The memory in these modes consists of using all four bit planes. Each bit  */
/* plane represents a color as shown below. The bit planes are denoted as C0,C1,C2     */
/* and C3 respectively.                                                                */
/*  - C0 = Blue        Pels                                                            */
/*  - C1 = Green       Pels                                                            */
/*  - C2 = Red         Pels                                                            */
/*  - C3 = Intensified Pels                                                            */
/* Four bits (one from each plane) define one pel on the screen. The color combinations*/
/* combinations are illustrated in the following table:                                */
/* I:   R:   G:   B:   Color:                                                          */
/*  - 0: - 0: - 0: - 0: - Black                                                        */
/*  - 0: - 0: - 0: - 1: - Blue                                                         */
/*  - 0: - 0: - 1: - 0: - Green                                                        */
/*  - 0: - 0: - 1: - 1: - Cyan                                                         */
/*  - 0: - 1: - 0: - 0: - Red                                                          */
/*  - 0: - 1: - 0: - 1: - Magenta                                                      */
/*  - 0: - 1: - 1: - 0: - Brown                                                        */
/*  - 0: - 1: - 1: - 1: - White                                                        */
/*  - 1: - 0: - 0: - 0: - Dark Grey                                                    */
/*  - 1: - 0: - 0: - 1: - Light Blue                                                   */
/*  - 1: - 0: - 1: - 0: - Light Green                                                  */
/*  - 1: - 0: - 1: - 1: - Light Cyan                                                   */
/*  - 1: - 1: - 0: - 0: - Light Red                                                    */
/*  - 1: - 1: - 0: - 1: - Light Magenta                                                */
/*  - 1: - 1: - 1: - 0: - Yellow                                                       */
/*  - 1: - 1: - 1: - 1: - Intensified White                                            */
/* The display buffer resides at address A0000. The map mask register of the sequencer */
/* is used to select any or all of the bit planes to be updated when a memory write to */
/* the display buffer is executed by the CPU.                                          */
/*                                                                                     */
/* Color Mapping:                                                                      */
/* The Enhanced Graphics Adapter supports 640x350 Graphics for both the IBM Monochrome */
/* and the IBM Enhanced Color Displays. Four color capability is supported on the EGA  */
/* without the Graphics Memory Expansion Card (base 64 KB), and sixteen colors are     */
/* supported when the Graphics Memory Expansion Card is installed on the adapter (128  */
/* KB or above). This section describes the differences in the colors displayed        */
/* depending upon the graphics memory available. Note that colors 0H, 1H, 4H, and 7H   */
/* map directly regardless of the graphics memory available.                           */
/* Character Attribute: Monochrome:    Mode 10H 64KB: Mode 10H >64KB:                  */
/*  - 00H:*              - Black:       - Black:       - Black:                        */
/*  - 01H:*              - Video:       - Blue:        - Blue:                         */
/*  - 02H:               - Black:       - Black:       - Green:                        */
/*  - 03H:               - Video:       - Blue:        - Cyan:                         */
/*  - 04H:*              - Blinking:    - Red:         - Red:                          */
/*  - 05H:               - Intensified: - White:       - Magenta:                      */
/*  - 06H:               - Blinking:    - Red:         - Brown:                        */
/*  - 07H:*              - Intensified: - White:       - White:                        */
/*  - 08H:               - Black:       - Black:       - Dark Gray:                    */
/*  - 09H:               - Video:       - Blue:        - Light Blue:                   */
/*  - 0AH:               - Black:       - Black:       - Light Green:                  */
/*  - 0BH:               - Video:       - Blue:        - Light Cyan:                   */
/*  - 0CH:               - Blinking:    - Red:         - Light Red:                    */
/*  - 0DH:               - Intensified: - White:       - Light Magenta:                */
/*  - 0EH:               - Blinking:    - Red:         - Yellow:                       */
/*  - 0FH:               - Intensified: - White:       - Intensified White:            */
/* *Graphics character attributes which map directly regardless of the graphics memory */
/* available.                                                                          */
/* ----------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------------- */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Programming Considerations:      */
/* -------------------------------------------------------------------------------------------- */
/* Programming the Registers:                                                                  */
/* Each of the LSI devices has an address register and a number of data registers. The address  */
/* register serves as a pointer to the other registers on the LSI device. It is a write-only    */
/* register that is loaded by the processor by executing an 'OUT' instruction to its I/O address*/
/* with the index of the selected data register.                                                */
/*                                                                                             */
/* The data registers on each LSI device are accessed through a common I/O address. They are    */
/* distinguished by the pointer (index) in the address register. To write to a data register,   */
/* the address register is loaded with the index of the appropriate data register, then the     */
/* selected data register is loaded by executing an 'OUT' instruction to the common I/O address.*/
/*                                                                                              */
/* The external registers that are not part of an LSI device and the Graphics I and II registers*/
/* are not accessed through an address register; they are written to directly.                  */
/*                                                                                              */
/* The following tables define the values that are loaded into the registers by BIOS to support */
/* the different modes of operation supported by this adapter.                                  */
/*                                                                                              */
/* External Registers:                                                                          */
/* Register:                   Mode of Operation:                                               */
/* Name:           Port:  Index: 0: 1: 2: 3: 4: 5: 6: 7: D: E: F: 10: F**: 10*: 0*: 1*: 2*: 3*: */
/*  - Miscellaneous: - 3C2: -    23 23 23 23 23 23 23 A6 23 23 A2 A7    A2  A7  A7  A7  A7  A7  */
/*  - Feature Cntrl: - 3?A: -    00 00 00 00 00 00 00 00 00 00 00 00    00  00  00  00  00  00  */
/*  - Input Stat 0 : - 3C2: -     -  -  -  -  -  -  -  -  -  -  -  -    -   -   -   -   -       */
/*  - Input Stat 1 : - 3?2  -     -  -  -  -  -  -  -  -  -  -  -  -    -   -   -   -   -       */
/*  ? = B  in monochrome modes    ? = D in color modes                                          */
/*  * Values for these modes when the IBM Enhanced Color Display is attached                    */
/*  **Values for these modes when greater than 64K Graphics Memory is installed                 */
/* Sequencer Registers:                                                                         */
/* Register:                   Mode of Operation:                                               */
/* Name:           Port:  Index: 0: 1: 2: 3: 4: 5: 6: 7: D: E: F: 10: F**: 10*: 0*: 1*: 2*: 3*: */
/*  - Seq Address  : - 3C4: -     -  -  -  -  -  -  -  -  -  -  -  -    -   -   -   -   -   -   */
/*  - Reset        : - 3C5: - 00:03 03 03 03 03 03 03 03 03 03 03 03   03  03  03  03  03  03   */
/*  - Clock Mode   : - 3C5: - 01:0B 0B 01 01 0B 0B 01 00 0B 01 05 05   01  01  0B  0B  01  01   */
/*                                                                                              */
/* CRT Controller Registers (1 of 2):                                                           */
/* Register:                   Mode of Operation:                                               */
/* Name:           Port:  Index: 0: 1: 2: 3: 4: 5: 6: 7: D: E: F: 10: F**: 10*: 0*: 1*: 2*: 3*: */
/*  - Address Reg  : - 3?4: -     -  -  -  -  -  -  -  -  -  -  -  -    -   -   -   -   -   -   */
/*  - Horiz Total  : - 3?5: - 00:37 37 70 70 37 37 70 60 37 70 60 5B   60  5B  2D  2D  5B  5B   */
/*  - Hrz Disp End : - 3?5: - 01:27 27 4F 4F 27 27 4F 4F 27 4F 4F 4F   4F  4F  27  27  4F  4F   */
/*  - Strt Hrz Blk : - 3?5: - 02:2D 2D 5C 5C 2D 2D 59 56 2D 56 56 53   56  53  2B  2B  53  53   */
/*  - End  Hrz Blk : - 3?5: - 03:37 37 2F 2F 37 37 2D 3A 37 2D 1A 17   3A  37  2D  2D  37  37   */
/*  - Strt Hrz Retr: - 3?5: - 04:31 31 5F 5F 30 30 5E 51 30 5E 50 50   50  52  28  28  51  51   */
/*  - End  Hrz Retr: - 3?5: - 05:15 15 07 07 14 14 06 60 14 06 E0 BA   60  00  6D  6D  5B  5B   */
/*  - Vert Total   : - 3?5: - 06:04 04 04 04 04 04 04 70 04 04 70 6C   70  6C  6C  6C  6C  6C   */
/*  - Overflow     : - 3?5: - 07:11 11 11 11 11 11 11 1F 11 11 1F 1F   1F  1F  1F  1F  1F  1F   */
/*  - Preset Row SC: - 3?5: - 08:00 00 00 00 00 00 00 00 00 00 00 00   00  00  00  00  00  00   */
/*  - Max Scan Line: - 3?5: - 09:07 07 07 07 01 01 01 0D 00 00 00 00   00  00  0D  0D  0D  0D   */
/*  - Cursor Start : - 3?5: - 0A:06 06 06 06 00 00 00 0B 00 00 00 00   00  00  0B  0B  0B  0B   */
/*  - Cursor End   : - 3?5: - 0B:07 07 07 07 00 00 00 0C 00 00 00 00   00  00  0C  0C  0C  0C   */
/*  - Strt Addr Hi : - 3?5: - 0C: -  -  -  -  -  -  -  -  -  -  -  -    -   -   -   -   -   -   */
/*  - Strt Addr Lo : - 3?5: - 0D: -  -  -  -  -  -  -  -  -  -  -  -    -   -   -   -   -   -   */
/*  ? = B in monochrome modes    ? = D in color modes                                           */
/*  * Values for these modes when the IBM Enhanced Color Display is attached                    */
/*  **Values for these modes when greater than 64K Graphics Memory is installed                 */
/*                                                                                              */
/* CRT Controller Registers (2 of 2):                                                           */
/* Register:                   Mode of Operation:                                               */
/* Name:           Port:  Index: 0: 1: 2: 3: 4: 5: 6: 7: D: E: F: 10: F**: 10*: 0*: 1*: 2*: 3*: */
/*  - Cursor LC Hi : - 3?5: - 0E: -  -  -  -  -  -  -  -  -  -  -  -    -   -   -   -   -   -   */
/*  - Cursor LC Low: - 3?5: - 0F: -  -  -  -  -  -  -  -  -  -  -  -    -   -   -   -   -   -   */
/*  - Vrt Retr Strt: - 3?5: - 10:E1 E1 E1 E1 E1 E1 E0 5E E1 E0 5E 5E   5E  5E  5E  5E  5E  5E   */
/*  - Light Pen Hi : - 3?5: - 10  -  -  -  -  -  -  -  -  -  -  -  -    -   -   -   -   -   -   */
/*  - Vert Retr End: - 3?5: - 11:24 24 24 24 24 24 23 2E 24 23 2E 2B   2E  2B  2B  2B  2B  2B   */
/*  - Light Pen Low: - 3?5: - 11: -  -  -  -  -  -  -  -  -  -  -  -    -   -   -   -   -   -   */
/*  - Vrt  Disp End: - 3?5: - 12:C7 C7 C7 C7 C7 C7 C7 5D C7 C7 5D 5D   5D  5D  5D  5D  5D  5D   */
/*  - Offset       : - 3?5: - 13:14 14 28 28 14 14 28 28 14 28 14 14   28  28  14  14  28  28   */
/*  - Underline Loc: - 3?5: - 14:08 08 08 08 00 00 00 0D 00 00 0D 0F   0D  0F  0F  0F  0F  0F   */
/*  - Strt Vert Blk: - 3?5: - 15:E0 E0 E0 E0 E0 E0 DF 5E E0 DF 5E 5F   5E  5F  5E  5E  5E  5E   */
/*  - End  Vert Blk: - 3?5: - 16:F0 F0 F0 F0 F0 F0 EF 6E F0 EF 6E 0A   6E  0A  0A  0A  0A  0A   */
/*  - Mode Control : - 3?5: - 17:A3 A3 A3 A3 A2 A2 C2 A3 E3 E3 8B 8B   E3  E3  A3  A3  A3  A3   */
/*  - Line Compare : - 3?5: - 18:FF FF FF FF FF FF FF FF FF FF FF FF   FF  FF  FF  FF  FF  FF   */
/*  ? = B in monochrome modes    ? = D in color modes                                           */
/*  * Values for these modes when the IBM Enhanced Color Display is attached                    */
/*  **Values for these modes when greater than 64K Graphics Memory is installed                 */
/* Graphics SI Registers:                                                                       */
/* Register:                   Mode of Operation:                                               */
/* Name:           Port:  Index: 0: 1: 2: 3: 4: 5: 6: 7: D: E: F: 10: F**: 10*: 0*: 1*: 2*: 3*: */
/*  - Grphx I  Pos : - 3CC: -    00 00 00 00 00 00 00 00 00 00 00 00   00  00  00  00  00  00   */
/*  - Grphx II Pos : - 3CA: -    01 01 01 01 01 01 01 01 01 01 01 01   01  01  01  01  01  01   */
/*  - Grphx I II AD: - 3CE: -     -  -  -  -  -  -  -  -  -  -  -  -    -   -   -   -   -   -   */
/*  - Set Reset    : - 3CF: - 00:00 00 00 00 00 00 00 00 00 00 00 00   00  00  00  00  00  00   */
/*  - Enable S/R   : - 3CF: - 01:00 00 00 00 00 00 00 00 00 00 00 00   00  00  00  00  00  00   */
/*  - Color Compare: - 3CF: - 02:00 00 00 00 00 00 00 00 00 00 00 00   00  00  00  00  00  00   */
/*  - Data Rotate  : - 3CF: - 03:00 00 00 00 00 00 00 00 00 00 00 00   00  00  00  00  00  00   */
/*  - Read Map Sel : - 3CF: - 04:00 00 00 00 00 00 00 00 00 00 00 00   00  00  00  00  00  00   */
/*  - Mode Register: - 3CF: - 05:10 10 10 10 30 30 0D 10 0D 00 10 10   0D  0D  10  10  10  10   */
/*  - Miscellaneous: - 3CF: - 06:0E 0E 0E 0E 0F 0F 0D 0A 05 05 07 07   05  05  0E  0E  0E  0E   */
/*  - Color No Care: - 3CF: - 07:00 00 00 00 00 00 00 00 0F 0F 0F 0F   0F  00  00  00  00  00   */
/*  - Bit Mask     : - 3CF: - 08:FF FF FF FF FF FF FF FF FF FF FF FF   FF  FF  FF  FF  FF  FF   */
/*  * Values for these modes when the IBM Enhanced Color Display is attached                    */
/*  **Values for these modes when greater than 64K Graphics Memory is installed                 */
/* Attribute Registers (1 of 2):                                                                */
/* Register:                   Mode of Operation:                                               */
/* Name:           Port:  Index: 0: 1: 2: 3: 4: 5: 6: 7: D: E: F: 10: F**: 10*: 0*: 1*: 2*: 3*: */
/*  - Address      : - 3?A: -     -  -  -  -  -  -  -  -  -  -  -  -    -   -   -   -   -   -   */
/*  - Palette      : - 3C0: - 00:00 00 00 00 00 00 00 00 00 00 00 00   00  00  00  00  00  00   */
/*  - Palette      : - 3C0: - 01:01 01 01 01 13 13 17 08 01 01 08 01   08  01  01  01  01  01   */
/*  - Palette      : - 3C0: - 02:02 02 02 02 15 15 17 08 02 02 00 00   00  02  02  02  02  02   */
/*  - Palette      : - 3C0: - 03:03 03 03 03 17 17 17 08 03 03 00 00   00  03  03  03  03  03   */
/*  - Palette      : - 3C0: - 04:04 04 04 04 02 02 17 08 04 04 18 04   18  04  04  04  04  04   */
/*  - Palette      : - 3C0: - 05:05 05 05 05 04 04 17 08 05 05 18 07   18  05  05  05  05  05   */
/*  - Palette      : - 3C0: - 06:06 06 06 06 06 06 17 08 06 06 00 00   00  06  14  14  14  14   */
/*  - Palette      : - 3C0: - 07:07 07 07 07 07 07 17 08 07 07 00 00   00  07  07  07  07  07   */
/*  - Palette      : - 3C0: - 08:10 10 10 10 10 10 17 10 10 10 00 00   00  38  38  38  38  38   */
/*  - Palette      : - 3C0: - 09:11 11 11 11 11 11 17 18 11 11 08 01   08  39  39  39  39  39   */
/*  - Palette      : - 3C0: - 0A:12 12 12 12 12 12 17 18 12 12 08 01   08  3A  3A  3A  3A  3A   */
/*  - Palette      : - 3C0: - 0B:13 13 13 13 13 13 17 18 13 13 08 01   08  3B  3B  3B  3B  3B   */
/*  ? = B in monochrome modes    ? = D in color modes                                           */
/*  * Values for these modes when the IBM Enhanced Color Display is attached                    */
/*  **Values for these modes when greater than 64K Graphics Memory is installed                 */
/* Attribute Registers (2 of 2):                                                                */
/* Register:                   Mode of Operation:                                               */
/* Name:           Port:  Index: 0: 1: 2: 3: 4: 5: 6: 7: D: E: F: 10: F**: 10*: 0*: 1*: 2*: 3*: */
/*  - Palette      : - 3C0: - 0C:14 14 14 14 14 14 17 18 14 14 00 04   00  3C  3C  3C  3C  3C   */
/*  - Palette      : - 3C0: - 0D:15 15 15 15 15 15 17 18 15 15 18 07   18  3D  3D  3D  3D  3D   */
/*  - Palette      : - 3C0: - 0E:16 16 16 16 16 16 17 18 16 16 00 00   00  3E  3E  3E  3E  3E   */
/*  - Palette      : - 3C0: - 0F:17 17 17 17 17 17 17 18 17 00 00 00   3F  3F  3F  3F  3F  3F   */
/*  - Mode Control : - 3C0: - 10:08 08 08 08 01 01 01 OE 01 01 0B 0B   0B  01  08  08  08  08   */
/*  - Overscan     : - 3C0: - 11:00 00 00 00 00 00 00 O0 00 00 00 00   00  00  00  00  00  00   */
/*  - Color Plane  : - 3C0: - 12:0F 0F 0F 0F 03 03 01 OF 0F 0F 05 05   05  0F  0F  0F  0F  0F   */
/*  - Hrz Panning  : - 3C0: - 13:00 00 00 00 00 00 00 O0 00 00 00 00   00  00  00  00  00  00   */
/*  * Values for these modes when the IBM Enhanced Color Display is attached                    */
/*  **Values for these modes when greater than 64K Graphics Memory is installed                 */
/* -------------------------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------------------------------------- */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: RAM Loadable Character Generator:           */
/* ------------------------------------------------------------------------------------------------------- */
/* The character generator on the adapter is RAM loadable and can support characters up to 32 scan lines   */
/* high. Two character generators are stored within the BIOS when an alphanumeric mode is selected. The    */
/* Character Map Select Register can be programmed to define the function of bit 3 of the attribute byte   */
/* to be a character generator switch. This allows the user to select between any two character sets       */
/* residing in bit plane 2. This effectively gives the user access to 512 characters instead of 256.       */
/* character tables may be loaded off line.The adapter must have 128K bytes of storage to support this     */
/* function. Up to four tables can be loaded can be loaded with 256K of graphics memory installed.         */
/*                                                                                                         */
/* The structure of the character tables is described in the following figure. The character generator is  */
/* in bit plane 2 and must be protected using the map mask function.                                       */
/* Bit Plane 2:                                                                                            */
/*  - + 0K: Character Generator 0:                                                                         */
/*  - + 8K:                                                                                                */
/*  - +16K: Character Generator 1:                                                                         */
/*  - +32K: Character Generator 2:                                                                         */
/*  - +48K: Character Generator 3:                                                                         */
/*  - +64K:                                                                                                */
/* The following figure illustrates the structure of each character pattern. If the CRT controller is      */
/* programmed to generate n row scans, then n bytes must be filled in for each character in the character  */
/* character generator. The example assumes eight row scans per character.                                 */
/* Address:          Byte Image:  Data:                                                                    */
/*  - CC * 32 + 0:| | | |X|X| | | | - 18H:                                                                 */
/*  -           1:| | |X|X|X|X| | | - 3EH:                                                                 */
/*  -           2:| |X|X| | |X|X| | - 66H:                                                                 */
/*              3:| |X|X| | |X|X| | - 66H:                                                                 */
/*              4:| |X|X|X|X|X|X| | - 7EH:                                                                 */
/*              5:| |X|X| | |X|X| | - 66H:                                                                 */
/*              6:| |X|X| | |X|X| | - 66H:                                                                 */
/*              7:| |X|X| | |X|X| | - 66H:                                                                 */
/*  CC = Value of the character code. For example, 41H in the case of an ASCII ''A''.                      */
/* Creating a 512 Character Set:                                                                           */
/* This section describes how to create a 512 character set on the IBM Color Display. Note that only 256   */
/* characters can be printed on the printer. This is a special application which the Enhanced Graphics     */
/* Graphics Adapter will support. The 9 by 14 characters will be displayed when attribute bit 3 is a       */
/* logical 0, and the IBM Color/Graphics Monitor Adapter 8 by 8 characters will be displayed when the      */
/* attribute bit 3 is a logical 1. This example is for demonstrative purposes only. The assembly language  */
/* routine for creating 512 characters is given below. Debug 2.0 was used for this example. The starting   */
/* assembly address is 100 and the character string is stored in location 200. This function requires 128K */
/* or more of graphics memory.                                                                             */
/*                                                                                                         */
/* a100                                                                                                    */
/* mov ax,1102      ;load 8x8 character font in character                                                  */
/* mov bl,02        ;generator number 2                                                                    */
/* int 10                                                                                                  */
/*                                                                                                         */
/* mov ax,1103      ;select 512 character operation                                                        */
/* mov bl,08        ;if attribute bit 3=1 use 8x8 font                                                     */
/* int 10           ;if attribute bit 3=0 use 9x14 font                                                    */
/*                                                                                                         */
/* mov ax,1000      ;set color plane enable to 7H to disable                                               */
/* mov bx,0712      ;attribute bit 3 in color palette                                                      */
/* int 10           ;lookup table                                                                          */
/*                                                                                                         */
/* mov ax,1301                                                                                             */
/* mov bx,000F      ;write char. string with attribute bit 3=1                                             */
/* mov cx,003A      ;cx = character string length                                                          */
/* mov dx,1600      ;write character on line 22 of display                                                 */
/* mov bp,0200      ;pointer to character string location                                                  */
/* push cs                                                                                                 */
/* pop es                                                                                                  */
/* int 10                                                                                                  */
/*                                                                                                         */
/* mov ax,1301                                                                                             */
/* mov bx,0007      ;write char. string with attribute bit 3=0                                             */
/* mov cx,003A      ;cx = character string length                                                          */
/* mov dx,1700      ;write character on line 23 of display                                                 */
/* mov bp,0200      ;pointer to character string location                                                  */
/* push cs                                                                                                 */
/* pop es                                                                                                  */
/* int 10                                                                                                  */
/* int 3                                                                                                   */
/*                                                                                                         */
/* a200 db          "This character string is used to show 512                                             */
/*                  characters"                                                                            */
/*                                                                                                         */
/* Creating an 80 by 43 Alphanumeric Mode:                                                                 */
/* The following examples show how to create 80 column by 43 row, both alphanumeric and graphics, images on*/
/* the IBM Monochrome Display. The BIOS Interface supports an 80 column by n row display by using the      */
/* character generator load routine call. The print screen routine must be revectored to handle the        */
/* additional character rows on the screen. The assembly language required for both an alphanumeric and a  */
/* graphics screen is shown below.                                                                         */
/*                                                                                                         */
/* mov al, 7        ;Monochrome alphanumeric mode                                                          */
/* int 10           ;video interrupt call                                                                  */
/* mov ax,1112      ;character generator BIOS routine                                                      */
/* mov bl,0         ;load 8 by 8 double dot character font                                                 */
/* int 10           ;video interrupt call                                                                  */
/* mov ax,1200      ;alternate screen routine                                                              */
/* move bl,20       ;select alternate print screen routine                                                 */
/* int 10           ;video interrupt call                                                                  */
/* int 3                                                                                                   */
/*                                                                                                         */
/* mov ax,f         ;Monochrome graphic mode                                                               */
/* int 10           ;video interrupt call                                                                  */
/* mov ax,1123      ;character generator BIOS routine                                                      */
/* mov bl,0         ;load 8 by 8 double dot character font                                                 */
/* mov dl,2B        ;43 character rows                                                                     */
/* int 10           ;video interrupt call                                                                  */
/* mov ax,1200      ;alternate screen routine                                                              */
/* mov bl,20        ;alternate print screen routine                                                        */
/* int 10           ;video interrupt call                                                                  */
/* int 3                                                                                                   */
/*                                                                                                         */
/* Vertical Interrupt Feature:                                                                             */
/* The Enhanced Graphics Adapter can be programmed to create an interrupt each time the vertical display   */
/* refresh time has ended. An interrupt handler routine must be written by the application to take         */
/* advantage of this feature. The CRT Vertical Interrupt is on IRQ2. The CPU can poll the Enhanced Graphics*/
/* Adapter Input Status Register 0 (bit 7) to determine whether the CRTC caused the interrupt to occur.    */
/*                                                                                                         */
/* The Vertical Retrace End Register (11H) in the CRT controller contains two bits which are used to       */
/* control the interrupt circuitry. The remaining bits must be output as per the value in the mode table.  */
/*  - Bit 5: Enable Vertical Interrupt--A logical 0 will enable  vertical interrupt.                       */
/*  - Bit 4: Clear  Vertical Interrupt--A logical 0 will clear a vertical interrupt.                       */
/* The sequence of events which occur in an interrupt handler are outlined below.                          */
/*  - 1. Clear IRQ latch and enable driver                                                                 */
/*  - 2. Enable IRQ latch                                                                                  */
/*  - 3. Wait for vertical interrupt                                                                       */
/*  - 4. Poll Interrupt Status Register 0 to determine if CRTC has caused the interrupt                    */
/*  - 5. If CRTC interrupt, then clear IRQ latch; if not, then branch to next interrupt handler.           */
/*  - 6. Enable IRQ latch                                                                                  */
/*  - 7. Update Enhanced Graphics Adapter during vertical blanking interval                                */
/*  - 8. Wait for next vertical interrupt                                                                  */
/* Creating a Split Screen:                                                                                */
/* The Enhanced Graphics Adapter hardware supports an alphanumeric mode dual screen display. The top       */
/* portion of the screen is designated as screen A, and the bottom portion of the screen is designated as  */
/* screen B as per the following figure.                                                                   */
/*                                                                                                         */
/* Dual Screen Definition:                                                                                 */
/*  - Screen A                                                                                             */
/*  - Screen B                                                                                             */
/* The following figure shows the screen mapping for a system containing a 32K byte alphanumeric storage   */
/* buffer. Note that the Enhanced Graphics Adapter has a 32K byte storage buffer in alphanumeric mode.     */
/* Information displayed on screen A is defined by the start address high and low registers (0CH and 0DH)  */
/* of CRTC. Information displayed on screen B always begins at address 0000H.                              */
/*                                                                                                         */
/* Screen Mapping Within the Display Buffer Address Space:                                                 */
/*  - 0000H: Screen B Buffer Storage Area                                                                  */
/*  - 0FFFH: Screen B Buffer Storage Area                                                                  */
/*  - 1000H: Screen A Buffer Storage Area                                                                  */
/*  - 7FFFH: Screen A Buffer Storage Area                                                                  */
/* The Line Compare Register (18H) of the CRT Controller is utilized to perform the split screen function. */
/* function. The CRTC has an internal horizontal scan counter, and logic which compares the horizontal scan*/
/* counter value to the Line Compare Register value and clears the memory address generator then           */
/* sequentially addresses the display buffer starting at location zero, and each subsequent row address is */
/* is determined by the 16 bit addition of the start of line latch and the offset register.                */
/*                                                                                                         */
/* Screen B can be smoothly scrolled onto the CRT screen by updating the Line compare in synchronization   */
/* with the vertical retrace signal. The information on screen B is immune from scrolling operations which */
/* utilize the Start Address High and Low registers to scroll through the screen A address map.            */
/*                                                                                                         */
/* Compatibility Issues:                                                                                   */
/* The CRT Controller on the IBM Enhanced Graphics Adapter is a custom design, and is different than the   */
/* 6845 controller used on the IBM Monochrome Monitor Adapter and the IBM Color/Graphics Monitor Adapter.  */
/* It should be noted that several CRTC register addresses differ between the adapters. The following      */
/* figure illustrates the registers which do not map directly across the two controllers.                  */
/* Register: 6845 Function:            EGA CRTC Function:                                                  */
/*  - 02H:    - Start Horiz. Retrace   : - Start Horiz. Blanking                                           */
/*  - 03H:    - End   Horiz. Retrace   : - End   Horiz. Blanking                                           */
/*  - 04H:    - Vertical Total         : - Start Horiz. Retrace                                            */
/*  - 05H:    - Vertical Total Adjust  : - End   Horiz. Retrace                                            */
/*  - 06H:    - Vertical Displayed     : - Vertical Total                                                  */
/*  - 07H:    - Vertical Sync Position : - Overflow                                                        */
/*  - 08H:    - Interlace Mode and Skew: - Preset Row Scan                                                 */
/* Existing applications which utilize the BIOS interface will generally be compatible with the Enhanced   */
/* Graphics Adapter.                                                                                       */
/*                                                                                                         */
/* Horizontal screen centering was required on the IBM Color/Graphics Monitor Adapter in order to center   */
/* the screen when generating composite video. This was done through the Horizontal Sync Position Register.*/
/* Position Register. Since the Enhanced Graphics Adapter does not support a composite video monitor,      */
/* programs which do screen centering may cause loss of the screen image if centering is attempted.        */
/*                                                                                                         */
/* The Enhanced Graphics Adapter offers a wider variety of displayable monochrome character attributes than*/
/* the IBM Monochrome Display Adapter. Some attribute values may display differently between the two       */
/* Adapters. The values listed in the table below, in any combinations with the blink and intensity and    */
/* intensity attributes, will display identically.                                                         */
/*                                                                                                         */
/* Background R G B: Foreground R G B: Position:                                                           */
/*  -         0 0 0:  -         0 0 0:  - Non-Display                                                      */
/*  -         0 0 0:  -         0 0 1:  - Underline                                                        */
/*  -         0 0 0:  -         1 1 1:  - White Character/Black Background                                 */
/*  -         1 1 1:  -         0 0 0:  - Reverse Video                                                    */
/* Software which explicitly addresses 3D8 (Mode Select Register) or 3D9 (Color Select Register) on the    */
/* Color Graphics Monitor Adapter may produce different results on the Enhanced Graphics Adapter. For      */
/* example, blinking which is disabled by writing to 3D8 on the Color Graphics Adapter will not be disabled*/
/* on the Enhanced Graphics Adapter.                                                                       */
/* ------------------------------------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------------------------------------- */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Interface:                                  */
/* ------------------------------------------------------------------------------------------------------- */
/* Feature Connector:                                                                                      */
/* The following is a description of the Enhanced Graphics Adapter feature connector. Note that signals    */
/* coming from the Enhanced Graphics Adapter are labeled "inputs" and the signals coming to the Enhanced   */
/* Graphics Adapter through the feature connector are labeled "outputs".                                   */
/*                                                                                                         */
/* Signal:     Description:                                                                                */
/*  - J2      : - This pin is connected to auxiliary jack 2 on the rear panel of the adapter.              */
/*  - R'OUT   : - Secondary red output                                                                     */
/*  - ATRS/L  : - Attribute shift load. This signal controls the serialization of the video information.   */
/*  The shift register parallel loadsat the dot clock leading edge when this signal is low.                */
/*  - G OUT   : - Primary green output                                                                     */
/*  - R'      : - Secondary red input                                                                      */
/*  - R       : - Primary   red input                                                                      */
/*  - FC1     : - This signal is input from bit 1 (Feature Control Bit 1) of the Feature Control Register. */
/*  - FC0     : - This signal is input from bit 0 (Feature Control Bit 0) of the Feature Control Register. */
/*  - FEAT 0  : - This signal is output to bit 5 (Feature Code 0) of Input Status Register 0.              */
/*  - B'/V    : - Secondary blue input/Monochrome video                                                    */
/*  - VIN     : - Vertical retrace input                                                                   */
/*  - Internal: - This signal is output to bit 4 (Disable Internal Video) drivers of the Miscellaneous     */
/*  Output Register.                                                                                       */
/*  - V OUT   : - Vertical retrace output                                                                  */
/*  - J1      : - This pin is connected to auxiliary jack 1 on the rear panel of the adapter.              */
/*  - G'OUT   : - Secondary green output                                                                   */
/*  - B'OUT   : - Secondary blue  output                                                                   */
/*  - B OUT   : - Blue output                                                                              */
/*  - G       : - Green input                                                                              */
/*  - B       : - Blue  input                                                                              */
/*  - R OUT   : - Red  output                                                                              */
/*  - BLANK   : - This is a composite horizontal and vertical blanking signal from the CRTC.               */
/*  - FEAT 1  : - This signal is output to bit 6 (Feature Code 1) of Input Status Register 0.              */
/*  - G'/I    : - Secondary green/Intensity input                                                          */
/*  - HIN     : - Horizontal retrace input from the CRTC                                                   */
/*  - 14MHZ   : - 14 MHz signal from the system board                                                      */
/*  - EXT OSC : - External dot clock output                                                                */
/*  - HOUT    : - Horizontal retrace output                                                                */
/* The following figure shows the layout and pin numbering of the feature connector.                       */
/*                                                                                                         */
/* Feature Connector Diagram:                                                                              */
/* Signal Name:     Signal Name:                                                                           */
/*  - Gnd     :  : 1 2 : - -12V    :                                                                       */
/*  - +12V    :  : 3 4 : - J1      :                                                                       */
/*  - J2      :  : 5 6 : - G'OUT   :                                                                       */
/*  - R'OUT   :  : 7 8 : - B'OUT   :                                                                       */
/*  - ATRS/L  :  : 9 10: - B OUT   :                                                                       */
/*  - G OUT   :  :11 12: - G       :                                                                       */
/*  - R'      :  :13 14: - B       :                                                                       */
/*  - R       :  :15 16: - R OUT   :                                                                       */
/*  - FEAT 1  :  :17 18: - BLANK   :                                                                       */
/*  - FEAT 0  :  :19 20: - FC1     :                                                                       */
/*  - FC0     :  :21 22: - G'/I    :                                                                       */
/*  - B'/V    :  :23 24: - HIN     :                                                                       */
/*  - VIN     :  :25 26: - 14MHz   :                                                                       */
/*  - Internal:  :27 28: - EXT OSC :                                                                       */
/*  - V OUT   :  :29 30: - HOUT    :                                                                       */
/*  - GND     :  :31 32: - +5V     :                                                                       */
/* ------------------------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------------------------- */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Specifications:                              */
/* -------------------------------------------------------------------------------------------------------- */
/* System Board Switches:                                                                                   */
/* The following figure shows the proper system board DIP switch settings for the IBM Enhanced Graphics     */
/* Adapter when used with the Personal Computer and the Personal Computer XT. The switch block locations    */
/* are illustrated in the Technical Reference Manual "System Board Component Diagram". The Personal Computer*/
/* has two DIP switch blocks; the switch settings shown pertain to DIP Switch Block 1. The Personal Computer*/
/* XT has one DIP switch block.                                                                             */
/*                                                                                                          */
/* Switch Block (1):                                                                                        */
/* O: 1: 2: 3: 4: 5: 6: 7: 8:                                                                               */
/* N: |  |  |  |  X  X  |  |                                                                                */
/* /\ |  |  |  |  |  |  |  |                                                                                */
/* || |  |  |  |  |  |  |  |                                                                                */
/* Note: The DIP switches must be set as shown whenever the IBM Enhanced Graphics Adapter is installed,     */
/* regardless of display type. This is true even when a second display adapter is installed in the system.  */
/*                                                                                                          */
/* Configuration Switches:                                                                                  */
/* The following diagram shows the location and orientation of the configuration switches on the Enhanced   */
/* Graphics Adapter.                                                                                        */
/*  - Option Retaining Bracket:                                                                             */
/*  - Optional Graphics Memory Expansion Card:                                                              */
/*  - Off On: 1 2 3 4                                                                                       */
/* Configuration Switch Settings:                                                                           */
/* The configuration switches on the Enhanced Graphics Adapter determine the type of display support the    */
/* adapter provides, as follows:                                                                            */
/* Switch Settings for Enhanced Graphics Adapter as Primary Display Adapter:                                */
/*                                                               Configuration:                             */
/* SW1: SW2:  SW3:  SW4:    Enhanced Adapter:                  Monochrome Adapter: Color/Graphics Adapter:  */
/*  - On  - Off - Off - On : - Color Display 40x25            : - Secondary:        -                       */
/*  - Off - Off - Off - On : - Color Display 80x25            : - Secondary:        -                       */
/*  - On  - On  - On  - Off: - Enhanced Display Emulation Mode: - Secondary:        -                       */
/*  - Off - On  - On  - Off: - Enhanced Display Hi Res    Mode: - Secondary:        -                       */
/*  - On  - Off - On  - Off: - Monochrome                     : -                   - Secondary 40x25:      */
/*  - Off - Off - On  - Off: - Monochrome                     : -                   - Secondary 80x25:      */
/* Switch Settings for Enhanced Graphics Adapter as Primary Display Adapter:                                */
/*                                                               Configuration:                             */
/* SW1: SW2:  SW3:  SW4:    Enhanced Adapter:                  Monochrome Adapter: Color/Graphics Adapter:  */
/*  - On  - On  - On  - On : - Color Display 40x25            : - Primary  :         -                      */
/*  - Off - On  - On  - On : - Color Display 80x25            : - Primary  :         -                      */
/*  - On  - Off - On  - On : - Enhanced Display Emulation Mode: - Primary  :         -                      */
/*  - Off - Off - On  - On : - Enhanced Display Hi Res    Mode: - Primary  :         -                      */
/*  - On  - ON  - Off - On : - Monochrome                     : -                    - Primary  40x25:      */
/*  - Off - On  - Off - On : - Monochrome                     : -                    - Primary  80x25:      */
/* Direct Drive Connector:       ___________                                                                */
/* 9-Pin Direct Drive Signal: 5: \o o o o o/ :1                                                             */
/*                            9:  \o o o o/  :6                                                             */
/* Signal Name - Description:      ------- Pin:                                                             */
/*  - Direct : - Ground                   : - 1: - Enhanced Graphics Adapter:                               */
/*  - Drive  : - Secondary Red            : - 2: - Enhanced Graphics Adapter:                               */
/*  - Display: - Primary   Red            : - 3: - Enhanced Graphics Adapter:                               */
/*  - Direct : - Primary   Green          : - 4: - Enhanced Graphics Adapter:                               */
/*  - Drive  : - Primary   Blue           : - 5: - Enhanced Graphics Adapter:                               */
/*  - Display: - Secondary Green/Intensity: - 6: - Enhanced Graphics Adapter:                               */
/*  - Direct : - Secondary Blue/Mono Video: - 7: - Enhanced Graphics Adapter:                               */
/*  - Drive  : - Horizontal Retrace       : - 8: - Enhanced Graphics Adapter:                               */
/*  - Display: - Vertical   Retrace       : - 9: - Enhanced Graphics Adapter:                               */
/* Light Pen Interface:                                                                                     */
/*  - P-2 Connector: |||| | P-2:                                                                            */
/*                   654321 P-2 Connector:       Pin:                                                       */
/*     Light Pen Attachment: - +Light Pen Input : - 1: Enhanced Graphics Adapter:                           */
/*     Light Pen Attachment: - Not used         : - 2: Enhanced Graphics Adapter:                           */
/*     Light Pen Attachment: - +Light Pen Switch: - 3: Enhanced Graphics Adapter:                           */
/*     Light Pen Attachment: - Ground           : - 4: Enhanced Graphics Adapter:                           */
/*     Light Pen Attachment: - +5 Volts         : - 5: Enhanced Graphics Adapter:                           */
/*     Light Pen Attachment: - 12 Volts         : - 6: Enhanced Graphics Adapter:                           */
/* Jumper Descriptions:                                                                                     */
/* Located on the adapter are two jumpers designated P1 and P3. Jumper P1 changes the function on pin 2 on  */
/* the direct drive interface. When placed on pins 2 and 3, jumper P1 selects ground as the function of     */
/* direct drive interface, pin 2. This selection is for displays that support five color outputs, such as   */
/* the IBM Color Display. When P1 is placed on pins 1 and 2, red prime output is placed on pin 2 of the     */
/* direct drive interface connector. This supports the IBM Enhanced Color Display, which utilizes six color */
/* outputs on the direct drive interface.                                                                   */
/*                                                                                                          */
/* Jumper P3 changes the I/O address port of the Enhanced Graphics Adapter within the system. In its normal */
/* position, (pins 1 and 2), all Enhanced Graphics Adapter addresses are in the range 3XX. Moving jumper P3 */
/* to pins 2 and 3 changes the addresses to 2XX. Operation of the adapter in the 2XX mode is not supported  */
/* in BIOS.                                                                                                 */
/*                                                                                                          */
/* The following figure shows the location of the jumpers and numbering of the connectors.                  */
/*  - P3: |XX - P1: XX|                                                                                     */
/*        321       321                                                                                     */
/* -------------------------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------------------------- */
/* Logic Diagrams:                                                                                          */
/* -------------------------------------------------------------------------------------------------------- */
/* Enhanced Graphics Adapter:                                                                               */
/*  - Enhanced Graphics Adapter Sheet  1 of 11                                                              */
/*  - Enhanced Graphics Adapter Sheet  2 of 11                                                              */
/*  - Enhanced Graphics Adapter Sheet  3 of 11                                                              */
/*  - Enhanced Graphics Adapter Sheet  4 of 11                                                              */
/*  - Enhanced Graphics Adapter Sheet  5 of 11                                                              */
/*  - Enhanced Graphics Adapter Sheet  6 of 11                                                              */
/*  - Enhanced Graphics Adapter Sheet  7 of 11                                                              */
/*  - Enhanced Graphics Adapter Sheet  8 of 11                                                              */
/*  - Enhanced Graphics Adapter Sheet  9 of 11                                                              */
/*  - Enhanced Graphics Adapter Sheet 10 of 11                                                              */
/*  - Enhanced Graphics Adapter Sheet 11 of 11                                                              */
/*  - Graphics Memory Expansion Card Sheet 1 of 5                                                           */
/*  - Graphics Memory Expansion Card Sheet 2 of 5                                                           */
/*  - Graphics Memory Expansion Card Sheet 3 of 5                                                           */
/*  - Graphics Memory Expansion Card Sheet 4 of 5                                                           */
/*  - Graphics Memory Expansion Card Sheet 5 of 5                                                           */
/* -------------------------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------------------------- */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: BIOS Listing:                                */
/* -------------------------------------------------------------------------------------------------------- */
/* Vectors with Special Meanings:                                                                           */
/* Interrupt Hex 42 - Reserved:                                                                             */
/* When an IBM Enhanced Graphics Adapter is installed, the BIOS routines use interrupt 42 to revector the   */
/* video pointer.                                                                                           */
/*                                                                                                          */
/* Interrupt Hex 43 - IBM Enhanced Graphics Video Parameters:                                               */
/* When an IBM Enhanced Graphics Adapter is installed, the BIOS routines use this vector to point to a data */
/* region containing the parameters required for initializing of the IBM Enhanced Graphics Adapter. Note    */
/* that the format of the table must adhere to the BIOS conventions established in the listing. The power-on*/
/* routines initialize this vector to point to the parameters contained in the IBM Enhanced Graphics Adapter*/
/* ROM.                                                                                                     */
/*                                                                                                          */
/* Interrupt Hex 44 - Graphics Character Table:                                                             */
/* When an IBM Enhanced Graphics Adapter is installed the BIOS routines use this vector to point to a table */
/* of dot patterns that will be used when graphics characters are to be displayed. This table will be used  */
/* for the first 128 code points in video modes 4, 5, and 6. This table will be used for 256 characters in  */
/* all additional graphics modes. See the appropriate BIOS interface for additional information on setting  */
/* and using the graphics character table pointer.                                                          */
/* ---------------------------------------------------------------------------------------------------------*/
static void ega_clock (ega_t *ega, unsigned long cnt);


static
void ega_set_irq (ega_t *ega, unsigned char val)
{
	val = (val != 0);
	if (ega->set_irq_val == val) {
		return;
	}
/* ----------------------------------------------------------------------------------- */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Registers:              */
/* ----------------------------------------------------------------------------------- */
/* External Registers:                                                                 */
/* This section contains descriptions of the registers of the Enhanced Graphics Adapter*/
/* that are not contained in an LSI device.                                            */
/* Name:                              Port:  Index:                                    */
/*  - Miscellaneous Output Register  : - 3C2: - -:                                     */
/*  - Feature Control      Register  : - 3?A: - -:                                     */
/*  - Input Status         Register 0: - 3C2: - -:                                     */
/*  - Input Status         Register 1: - 3?2: - -:                                     */
/* ? = B in Monochrome Modes    ? = D in Color Modes                                   */
/* Input Status Register Zero:                                                         */
/* This is a read-only register. The processor input port address is hex 3C2.          */
/* Input Status Register Zero Format:                                                  */
/*  - Bit 0: Not Used                                                                  */
/*  - Bit 1: Not Used                                                                  */
/*  - Bit 2: Not Used                                                                  */
/*  - Bit 3: Not Used                                                                  */
/*  - Bit 4: Switch Sense                                                              */
/*  - Bit 5: Reserved                                                                  */
/*  - Bit 6: Reserved                                                                  */
/*  - Bit 7: CRT Interrupt                                                             */
/* Bit 7: CRT Interrupt--A logical 1 indicates video is being displayed on the CRT     */
/* screen; a logical 0 indicates that vertical retrace is occurring.                   */
/* ----------------------------------------------------------------------------------- */
	if (val) {
		ega->reg[EGA_STATUS0] |= EGA_STATUS0_CI;  /* Input Status Register Zero: CRT Interrupt */
	}
	else {
		ega->reg[EGA_STATUS0] &= ~EGA_STATUS0_CI; /* Input Status Register Zero: CRT Interrupt */
	}
	ega->set_irq_val = val;
	if (ega->set_irq != NULL) {
		ega->set_irq (ega->set_irq_ext, val);
	}
}

/*
 * Set the configuration switches
 */
static
void ega_set_switches (ega_t *ega, unsigned val)
{
	static unsigned char mon[16] = {
		EGA_MONITOR_CGA, EGA_MONITOR_CGA,
		EGA_MONITOR_CGA, EGA_MONITOR_ECD,
		EGA_MONITOR_MDA, EGA_MONITOR_MDA,
		EGA_MONITOR_CGA, EGA_MONITOR_CGA,
		EGA_MONITOR_CGA, EGA_MONITOR_ECD,
		EGA_MONITOR_MDA, EGA_MONITOR_MDA,
		EGA_MONITOR_ECD, EGA_MONITOR_ECD,
		EGA_MONITOR_ECD, EGA_MONITOR_ECD
	};
	ega->switches = val;
	ega->monitor = mon[val & 0x0f];
}

/*
 * Set the blink frequency
 */
static
void ega_set_blink_rate (ega_t *ega, unsigned freq)
{
	ega->blink_on = 1;
	ega->blink_cnt = freq;
	ega->blink_freq = freq;

	ega->update_state |= EGA_UPDATE_DIRTY;
}

/*
 * Get the displayed width in pixels
 */
static
unsigned ega_get_w (ega_t *ega)
{
	unsigned val;
/* ------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Major Components:  */
/* ------------------------------------------------------------------------------ */
/* CRT Controller:                                                                */
/* The CRT (Cathode Ray Tube) Controller (CRTC) generates horizontal and vertical */
/* synchronous timings, addressing for the regenerative buffer, cursor and        */
/* underline timings, and refresh addressing for the dynamic RAMs.                */
/* ------------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: CRT Control Registers:         */
/* ------------------------------------------------------------------------------------------ */
/* Horizontal Display Enable End Register:                                                    */
/* This is a write-only register pointed to when the value in the CRT Controller address      */
/* register is hex 01. The processor output port address for this register is hex 3B5 or hex  */
/* 3D5.                                                                                       */
/* Horizontal Display Enable End Register Format:                                             */
/*  - Bit 0: Horizontal Display Enable End                                                    */
/*  - Bit 1: Horizontal Display Enable End                                                    */
/*  - Bit 2: Horizontal Display Enable End                                                    */
/*  - Bit 3: Horizontal Display Enable End                                                    */
/*  - Bit 4: Horizontal Display Enable End                                                    */
/*  - Bit 5: Horizontal Display Enable End                                                    */
/*  - Bit 6: Horizontal Display Enable End                                                    */
/*  - Bit 7: Horizontal Display Enable End                                                    */
/* This register defines the length of the horizontal display enable signal. It determines the*/
/* number of displayed character positions per horizontal line.                               */
/*  - Bit 0-Bit 7: Horizontal display enable end--A value one less than the total number of   */
/* displayed characters.                                                                      */
/* ------------------------------------------------------------------------------------------ */
	val = ega->reg_crt[EGA_CRT_HD] + 1; /* Horizontal Display Enable End Register */
/* ------------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Display, August 2nd, 1984: Sequencer Registers:     */
/* ------------------------------------------------------------------------------------ */
/* Clocking Mode Register:                                                              */
/* This is write-only register pointed to when the value in the address register is hex */
/* 01. The output port address for this register is hex 3C5.                            */
/* Clocking Mode Register Format:                                                       */
/*  - Bit 0: 8/9 Dot Clocks                                                             */
/*  - Bit 1: Bandwidth                                                                  */
/*  - Bit 2: Shift Load                                                                 */
/*  - Bit 3: Dot Clock                                                                  */
/*  - Bit 4: Not Used                                                                   */
/*  - Bit 5: Not Used                                                                   */
/*  - Bit 6: Not Used                                                                   */
/*  - Bit 7: Not Used                                                                   */
/* Bit 0: 8/9 Dot Clocks--A logical 0 directs the sequencer to generate character clocks*/
/* 9 dots wide; a logical 1 directs the sequencer to generate character clocks 8 dots   */
/* wide. Monochrome alphanumeric mode (07H) is the only mode that uses character clocks */
/* 9 dots wide. All other modes must use 8 dots per character clock.                    */
/*                                                                                      */
/* Bit 3: Dot Clock--A logical 0 selects normal dot clocks derived from the sequencer   */
/* master clock input. When this bit is set to 1, the master clock will be divided by 2 */
/* to generate the dot clock. All the other timings will be stretched since they are    */
/* derived from the dot clock. Dot clock divided by two is used for 320x200 modes (0, 1,*/
/* 4, 5) to provide a pixel rate of 7 MHz, (9 MHz for mode D).                          */
/* ------------------------------------------------------------------------------------ */
	val *= (ega->reg_seq[EGA_SEQ_CLOCK] & EGA_SEQ_CLOCK_D89) ? 8 : 9; /* Sequencer Registers: Dot Clock and 8/9 Dot Clocks */
	return (val);
}

/*
 * Get the displayed height in pixels
 */
static
unsigned ega_get_h (ega_t *ega)
{
	unsigned h;
/* ------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Major Components:  */
/* ------------------------------------------------------------------------------ */
/* CRT Controller:                                                                */
/* The CRT (Cathode Ray Tube) Controller (CRTC) generates horizontal and vertical */
/* synchronous timings, addressing for the regenerative buffer, cursor and        */
/* underline timings, and refresh addressing for the dynamic RAMs.                */
/* ------------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: CRT Control Registers:         */
/* ------------------------------------------------------------------------------------------ */
/* Vertical Display Enable End Register:                                                      */
/* This is a write-only register pointed to when the value in the CRT Controller address      */
/* register is hex 12. The processor output port address for this register is hex 3B5 or hex  */
/* 3D5.                                                                                       */
/* Vertical Display Enable End Register Format:                                               */
/*  - Bit 0: Low Order Vertical Display Enable End                                            */
/*  - Bit 1: Low Order Vertical Display Enable End                                            */
/*  - Bit 2: Low Order Vertical Display Enable End                                            */
/*  - Bit 3: Low Order Vertical Display Enable End                                            */
/*  - Bit 4: Low Order Vertical Display Enable End                                            */
/*  - Bit 5: Low Order Vertical Display Enable End                                            */
/*  - Bit 6: Low Order Vertical Display Enable End                                            */
/*  - Bit 7: Low Order Vertical Display Enable End                                            */
/* Bit 0-Bit 7: Vertical Display Enable End--These are the low-order 8 bits of the vertical   */
/* display enable end position. This address specifies which scan line ends the active video  */
/* area of the screen. Bit 8 is in the overflow register location hex 07.                     */
/*                                                                                            */
/* Offset Register:                                                                           */
/* This is a write-only register pointed to when the value in the CRT Controller address      */
/* register is hex 13. The processor output port address for this register is hex 3B5 or hex  */
/* 3D5.                                                                                       */
/* Offset Register Format:                                                                    */
/*  - Bit 0: Logical line width of the screen                                                 */
/*  - Bit 1: Logical line width of the screen                                                 */
/*  - Bit 2: Logical line width of the screen                                                 */
/*  - Bit 3: Logical line width of the screen                                                 */
/*  - Bit 4: Logical line width of the screen                                                 */
/*  - Bit 5: Logical line width of the screen                                                 */
/*  - Bit 6: Logical line width of the screen                                                 */
/*  - Bit 7: Logical line width of the screen                                                 */
/* Bit 0-Bit 7: Offset--This register specifies the logical line width of the screen. The     */
/* starting memory address for the next character row is larger than the current character row*/
/* by this amount. The Offset Register is programmed with a word address. Depending upon the  */
/* method of clocking the CRT Controller, this word address is either a word or double word   */
/* address.                                                                                   */
/* ------------------------------------------------------------------------------------------ */
	h = ega->reg_crt[EGA_CRT_VD]; /* Vertical Display Enable End Register */
	h |= (ega->reg_crt[EGA_CRT_OF] << 7) & 0x100; /* Offset Register */
	h += 1;
	return (h);
}

/*
 * Get the character width in pixels
 */
static
unsigned ega_get_cw (ega_t *ega)
{
/* ------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Major Components:  */
/* ------------------------------------------------------------------------------ */
/* Sequencer:                                                                     */
/* The Sequencer generates basic memory timings for the dynamic RAMs and the      */
/* character clock for controlling regenerative memory fetches. It allows the     */
/* processor to access memory during active display intervals by inserting        */
/* dedicated processor memory cycles periodically between the display memory      */
/* cycles. Map mask registers are available to protect entire memory maps from    */
/* being changed.                                                                 */
/* ------------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Display, August 2nd, 1984: Sequencer Registers:     */
/* ------------------------------------------------------------------------------------ */
/* Clocking Mode Register:                                                              */
/* This is write-only register pointed to when the value in the address register is hex */
/* 01. The output port address for this register is hex 3C5.                            */
/* Clocking Mode Register Format:                                                       */
/*  - Bit 0: 8/9 Dot Clocks                                                             */
/*  - Bit 1: Bandwidth                                                                  */
/*  - Bit 2: Shift Load                                                                 */
/*  - Bit 3: Dot Clock                                                                  */
/*  - Bit 4: Not Used                                                                   */
/*  - Bit 5: Not Used                                                                   */
/*  - Bit 6: Not Used                                                                   */
/*  - Bit 7: Not Used                                                                   */
/* Bit 0: 8/9 Dot Clocks--A logical 0 directs the sequencer to generate character clocks*/
/* 9 dots wide; a logical 1 directs the sequencer to generate character clocks 8 dots   */
/* wide. Monochrome alphanumeric mode (07H) is the only mode that uses character clocks */
/* 9 dots wide. All other modes must use 8 dots per character clock.                    */
/*                                                                                      */
/* Bit 3: Dot Clock--A logical 0 selects normal dot clocks derived from the sequencer   */
/* master clock input. When this bit is set to 1, the master clock will be divided by 2 */
/* to generate the dot clock. All the other timings will be stretched since they are    */
/* derived from the dot clock. Dot clock divided by two is used for 320x200 modes (0, 1,*/
/* 4, 5) to provide a pixel rate of 7 MHz, (9 MHz for mode D).                          */
/* ------------------------------------------------------------------------------------ */
	if ((ega->reg_seq[EGA_SEQ_CLOCK] & EGA_SEQ_CLOCK_D89) == 0) { /* Sequencer Registers: Dot Clock and 8/9 Dot Clocks */
		return (9);
	}
	return (8);
}

/*
 * Get the character height in pixels
 */
static
unsigned ega_get_ch (ega_t *ega)
{
/* ------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Major Components:  */
/* ------------------------------------------------------------------------------ */
/* CRT Controller:                                                                */
/* The CRT (Cathode Ray Tube) Controller (CRTC) generates horizontal and vertical */
/* synchronous timings, addressing for the regenerative buffer, cursor and        */
/* underline timings, and refresh addressing for the dynamic RAMs.                */
/* ------------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: CRT Control Registers:         */
/* ------------------------------------------------------------------------------------------ */
/* Maximum Scan Line Register:                                                                */
/* This is a write-only register pointed to when the value in the CRT Controller address      */
/* register is hex 09. The processor output port address for this register is hex 3B5 or hex  */
/* 3D5.                                                                                       */
/* Maximum Scan Line Register Format:                                                         */
/*  - Bit 0: Maximum Scan Line                                                                */
/*  - Bit 1: Maximum Scan Line                                                                */
/*  - Bit 2: Maximum Scan Line                                                                */
/*  - Bit 3: Maximum Scan Line                                                                */
/*  - Bit 4: Maximum Scan Line                                                                */
/*  - Bit 5: Not Used                                                                         */
/*  - Bit 6: Not Used                                                                         */
/*  - Bit 7: Not Used                                                                         */
/* Bit 0-Bit 4: Maximum Scan Line--This register specified the number of scan lines per       */
/* character row. The number to be programmed is the maximum row scan number minus one.       */
/* ------------------------------------------------------------------------------------------ */
	return ((ega->reg_crt[EGA_CRT_MS] & 0x1f) + 1);
}

/*
 * Get CRT start offset
 */
static
unsigned ega_get_start (ega_t *ega)
{
	unsigned val;
/* ------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Major Components:  */
/* ------------------------------------------------------------------------------ */
/* CRT Controller:                                                                */
/* The CRT (Cathode Ray Tube) Controller (CRTC) generates horizontal and vertical */
/* synchronous timings, addressing for the regenerative buffer, cursor and        */
/* underline timings, and refresh addressing for the dynamic RAMs.                */
/* ------------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: CRT Control Registers:         */
/* ------------------------------------------------------------------------------------------ */
/* Start Address High Register:                                                               */
/* This is a read/write register pointed to when the value in the CRT Controller address      */
/* register is hex 0C. The processor input/output port address for this register is hex 3B5 or*/
/* hex 3D5.                                                                                   */
/* Start Address High Register Format:                                                        */
/*  - Bit 0: High Order Start Address                                                         */
/*  - Bit 1: High Order Start Address                                                         */
/*  - Bit 2: High Order Start Address                                                         */
/*  - Bit 3: High Order Start Address                                                         */
/*  - Bit 4: High Order Start Address                                                         */
/*  - Bit 5: High Order Start Address                                                         */
/*  - Bit 6: High Order Start Address                                                         */
/*  - Bit 7: High Order Start Address                                                         */
/* Bit 0-Bit 7: Start Address High--These are the high-order eight bits of the start address. */
/* The 16-bit value, from high-order and low-order start address register, is the first       */
/* address after the vertical retrace on each screen refresh.                                 */
/*                                                                                            */
/* Start Address Low Register:                                                                */
/* This is a read/write register pointed to when the value in the CRT Controller address      */
/* register is hex 0D. The processor input/output port address for this register is hex 3B5 or*/
/* hex 3D5.                                                                                   */
/* Start Address Low Register Format:                                                         */
/*  - Bit 0: Low Order Start Address                                                          */
/*  - Bit 1: Low Order Start Address                                                          */
/*  - Bit 2: Low Order Start Address                                                          */
/*  - Bit 3: Low Order Start Address                                                          */
/*  - Bit 4: Low Order Start Address                                                          */
/*  - Bit 5: Low Order Start Address                                                          */
/*  - Bit 6: Low Order Start Address                                                          */
/*  - Bit 7: Low Order Start Address                                                          */
/* Bit 0-Bit 7: Start Address Low--These are the low-order 8 bits of the start address.       */
/* ------------------------------------------------------------------------------------------ */
	val = ega->reg_crt[EGA_CRT_SAH];              /* Start Address High Register */
	val = (val << 8) | ega->reg_crt[EGA_CRT_SAL]; /* Start Address Low  Register */
	return (val);
}

/*
 * Get the absolute cursor position
 */
static
unsigned ega_get_cursor (ega_t *ega)
{
	unsigned val;
/* ------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Major Components:  */
/* ------------------------------------------------------------------------------ */
/* CRT Controller:                                                                */
/* The CRT (Cathode Ray Tube) Controller (CRTC) generates horizontal and vertical */
/* synchronous timings, addressing for the regenerative buffer, cursor and        */
/* underline timings, and refresh addressing for the dynamic RAMs.                */
/* ------------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: CRT Control Registers:         */
/* ------------------------------------------------------------------------------------------ */
/* Cursor Location High Register:                                                             */
/* This is a read/write register pointed to when the value in the CRT Controller address      */
/* register is hex 0E. The processor input/output port address for this register is hex 3B5 or*/
/* hex 3D5.                                                                                   */
/* Cursor Location High Register Format:                                                      */
/*  - Bit 0: High Order Cursor Location                                                       */
/*  - Bit 1: High Order Cursor Location                                                       */
/*  - Bit 2: High Order Cursor Location                                                       */
/*  - Bit 3: High Order Cursor Location                                                       */
/*  - Bit 4: High Order Cursor Location                                                       */
/*  - Bit 5: High Order Cursor Location                                                       */
/*  - Bit 6: High Order Cursor Location                                                       */
/*  - Bit 7: High Order Cursor Location                                                       */
/* Bit 0-Bit 7: Cursor Location High--These are the high-order 8 bits of the cursor location. */
/*                                                                                            */
/* Cursor Location Low Register:                                                              */
/* This is a read/write register pointed to when the value in the CRT Controller address      */
/* register is hex 0F. The processor input/output port address for this register is hex 3B5 or*/
/* hex 3D5.                                                                                   */
/* Cursor Location Low Register Format:                                                       */
/*  - Bit 0: Low Order Cursor Location                                                        */
/*  - Bit 1: Low Order Cursor Location                                                        */
/*  - Bit 2: Low Order Cursor Location                                                        */
/*  - Bit 3: Low Order Cursor Location                                                        */
/*  - Bit 4: Low Order Cursor Location                                                        */
/*  - Bit 5: Low Order Cursor Location                                                        */
/*  - Bit 6: Low Order Cursor Location                                                        */
/*  - Bit 7: Low Order Cursor Location                                                        */
/* Bit 0-Bit 7: Cursor Location Low--These are the high-order 8 bits of the cursor location.  */
/* ------------------------------------------------------------------------------------------ */
	val = ega->reg_crt[EGA_CRT_CLH];
	val = (val << 8) | ega->reg_crt[EGA_CRT_CLL];
	return (val);
}

/*
 * Get the line compare register
 */
static
unsigned ega_get_line_compare (const ega_t *ega)
{
	unsigned val;
/* ------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Major Components:  */
/* ------------------------------------------------------------------------------ */
/* CRT Controller:                                                                */
/* The CRT (Cathode Ray Tube) Controller (CRTC) generates horizontal and vertical */
/* synchronous timings, addressing for the regenerative buffer, cursor and        */
/* underline timings, and refresh addressing for the dynamic RAMs.                */
/* ------------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: CRT Control Registers:         */
/* ------------------------------------------------------------------------------------------ */
/* CRT Controller Overflow Register:                                                          */
/* This is a write-only register pointed to when the value in the CRT Controller Address      */
/* Register is hex 07. The processor output port address for this register is hex 3B5 or hex  */
/* 3D5.                                                                                       */
/*  - Bit 0: Vertical Total              Bit 8                                                */
/*  - Bit 1: Vertical Display Enable End Bit 8                                                */
/*  - Bit 2: Vertical Retrace Start      Bit 8                                                */
/*  - Bit 3: Start Vertical Blank        Bit 8                                                */
/*  - Bit 4: Line Compare                Bit 8                                                */
/*  - Bit 5: Cursor Location             Bit 8                                                */
/* Bit 0: Vertical Total--Bit 8 of the Vertical Total register (index hex 06).                */
/*                                                                                            */
/* Bit 1: Vertical Display Enable End--Bit 8 of the Vertical Display Enable End register      */
/* (index hex 12).                                                                            */
/*                                                                                            */
/* Bit 2: Vertical Retrace Start--Bit 8 of the Vertical Retrace Start register (index hex 10).*/
/*                                                                                            */
/* Bit 3: Start Vertical Blank--Bit 8 of the Start Vertical Blank register (index hex 15).    */
/*                                                                                            */
/* Bit 4: Line Compare--Bit 8 of the Line Compare register (index hex 18).                    */
/*                                                                                            */
/* Bit 5: Cursor Location--Bit 8 of the Cursor Location register (index hex 0A).              */
/*                                                                                            */
/* Line Compare Register:                                                                     */
/* This is a write-only register pointed to when the value in the CRT Controller address      */
/* register is hex 18. The processor output port address for this register is hex 3B5 or hex  */
/* 3D5.                                                                                       */
/* Line Compare Register Format:                                                              */
/*  - Bit 0: Line Compare Target                                                              */
/*  - Bit 1: Line Compare Target                                                              */
/*  - Bit 2: Line Compare Target                                                              */
/*  - Bit 3: Line Compare Target                                                              */
/*  - Bit 4: Line Compare Target                                                              */
/*  - Bit 5: Line Compare Target                                                              */
/*  - Bit 6: Line Compare Target                                                              */
/*  - Bit 7: Line Compare Target                                                              */
/* Bit 0-Bit 7: Line Compare--This register is the low-order 8 bits of the compare target.    */
/* When the vertical counter reaches this value, the internal start of the line counter is    */
/* cleared. This allows an area of the screen to be immune to scrolling. Bit 8 of this        */
/* register is in the overflow register hex 07.                                               */
/* ------------------------------------------------------------------------------------------ */
	val = ega->reg_crt[EGA_CRT_LC];        /* Line Compare Register */
	if (ega->reg_crt[EGA_CRT_OF] & 0x10) { /* Overflow     Register */
		val += 256;
	}
	return (val);
}

/*
 * Get a palette entry
 */
static
void ega_get_palette (ega_t *ega, unsigned idx,
	unsigned char *r, unsigned char *g, unsigned char *b)
{
	unsigned v;
	unsigned mon;
/* ------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Major Components:  */
/* ------------------------------------------------------------------------------ */
/* Attribute Controller:                                                          */
/* The Attribute Controller provides a color palette of 16 colors, each of which  */
/* may be specified separately. Six color outputs are available for driving a     */
/* display. Blinking and underlining are controlled by this chip. This chip takes */
/* data from the display memory and formats it for display on the CRT screen.     */
/* ------------------------------------------------------------------------------ */
/* ----------------------------------------------------------------------------------- */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Registers:              */
/* ----------------------------------------------------------------------------------- */
/* External Registers:                                                                 */
/* This section contains descriptions of the registers of the Enhanced Graphics Adapter*/
/* that are not contained in an LSI device.                                            */
/* Name:                              Port:  Index:                                    */
/*  - Miscellaneous Output Register  : - 3C2: - -:                                     */
/*  - Feature Control      Register  : - 3?A: - -:                                     */
/*  - Input Status         Register 0: - 3C2: - -:                                     */
/*  - Input Status         Register 1: - 3?2: - -:                                     */
/* ? = B in Monochrome Modes    ? = D in Color Modes                                   */
/* Miscellaneous Output Register:                                                      */
/* This is a write-only register. The processor output port address is hex 3C2. A      */
/* hardware reset causes all bits to reset to zero.                                    */
/* Miscellaneous Output Register Format:                                               */
/*  - Bit 0: I/O Address Select                                                        */
/*  - Bit 1: Enable Ram                                                                */
/*  - Bit 2: Clock Select 0                                                            */
/*  - Bit 3: Clock Select 1                                                            */
/*  - Bit 4: Disable Internal Video Drivers                                            */
/*  - Bit 5: Page Bit For Odd/Even                                                     */
/*  - Bit 6: Horizontal Retrace Polarity                                               */
/*  - Bit 7: Vertical   Retrace Polarity                                               */
/* Bit 0: 3BX/3DX CRTC I/O Address--This bit maps the CRTC I/O addresses for IBM       */
/* Monochrome or Color/Graphics Monitor Adapter emulation. A logical 0 sets CRTC       */
/* addresses to 3BX and Input Status Register 1 's address to 3BA for Monochrome       */
/* emulation. A logical 1 sets CRTC addresses to 3DX and Input Status Register 1's     */
/* address to 3DA for Color/Graphics Monitor Adapter emulation.                        */
/*                                                                                     */
/* Bit 1: Enable RAM--A logical 0 disables RAM from the processor; a logical 1 enables */
/* RAM to respond at addresses designated by the Control Data Select value programmed  */
/* into the Graphics Controllers.                                                      */
/*                                                                                     */
/* Bit 2-Bit 3: Clock Select--These two bits select the clock source according to the  */
/* following table:                                                                    */
/* Bits:                                                                               */
/*  - 3 2:                                                                             */
/* --------                                                                            */
/*  - 0 0-: Selects 14 MHz clock from the processor I/O channel                        */
/*  - 0 1-: Selects 16 MHz clock on-board oscillator                                   */
/*  - 1 0-: Selects external clock source from the feature connector.                  */
/*  - 1 1-: Not used                                                                   */
/*                                                                                     */
/* Bit 4: Disable Internal Video Drivers--A logical 0 activates internal video drivers;*/
/* a logical 1 disables internal video drivers. When the internal video drivers are    */
/* disabled, the source of the direct drive color output becomes the feature connector */
/* direct drive outputs.                                                               */
/*                                                                                     */
/* Bit 5: Page Bit For Odd/Even--Selects between two 64K pages of memory when in the   */
/* Odd/Even modes (0,1,2,3,7). A logical 0 selects the low page of memory; a logical 1 */
/* selects the high page of memory.                                                    */
/*                                                                                     */
/* Bit 6: Horizontal Retrace Polarity--A logical 0 selects positive horizontal retrace;*/
/* a logical 1 selects negative horizontal retrace.                                    */
/*                                                                                     */
/* Bit 7: Vertical   Retrace Polarity--A logical 0 selects positive vertical   retrace;*/
/* a logical 1 selects negative vertical   retrace.                                    */
/* ----------------------------------------------------------------------------------- */
	if ((ega->reg[EGA_MOUT] & EGA_MOUT_VSP) == 0) {
		mon = EGA_MONITOR_CGA;
	}
	else {
		mon = ega->monitor;
	}
	idx &= ega->reg_atc[EGA_ATC_CPE];
	v = ega->reg_atc[idx & 0x0f];
	if (mon == EGA_MONITOR_MDA) {
		*r = 0;
		*g = 0;
		*b = 0;

		if (v & 0x08) {
			*r += 0xe8;
			*g += 0x90;
			*b += 0x50;
		}

		if (v & 0x10) {
			*r += 0x17;
			*g += 0x60;
			*b += 0x78;
		}
	}
	else if (mon == EGA_MONITOR_CGA) {
		*r = (v & 0x04) ? 0xaa : 0x00;
		*r += (v & 0x10) ? 0x55 : 0x00;

		*g = (v & 0x02) ? 0xaa : 0x00;
		*g += (v & 0x10) ? 0x55 : 0x00;

		*b = (v & 0x01) ? 0xaa : 0x00;
		*b += (v & 0x10) ? 0x55 : 0x00;

		if (v == 0x06) {
			*g = 0x55;
		}
	}
	else {
		*b = ((v << 1) & 0x02) | ((v >> 3) & 0x01);
		*b |= *b << 2;
		*b |= *b << 4;

		*g = ((v << 0) & 0x02) | ((v >> 4) & 0x01);
		*g |= *g << 2;
		*g |= *g << 4;

		*r = ((v >> 1) & 0x02) | ((v >> 5) & 0x01);
		*r |= *r << 2;
		*r |= *r << 4;
	}
}

/*
 * Get a transformed CRTC address
 */
static
unsigned ega_get_crtc_addr (ega_t *ega, unsigned addr, unsigned row)
{
/* ------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Major Components:  */
/* ------------------------------------------------------------------------------ */
/* CRT Controller:                                                                */
/* The CRT (Cathode Ray Tube) Controller (CRTC) generates horizontal and vertical */
/* synchronous timings, addressing for the regenerative buffer, cursor and        */
/* underline timings, and refresh addressing for the dynamic RAMs.                */
/*                                                                                */
/* Display Buffer:                                                                */
/* The display buffer on the adapter consists of 64K bytes of dynamic read/write  */
/* memory configured as four 16K byte video bit planes. Two options are available */
/* for expanding the graphics memory. The Graphics Memory Expansion Card plugs    */
/* into the memory expansion connector on the adapter, and adds one bank of 16K   */
/* to each of the four bit planes, increasing the graphics memory to 128K bytes.  */
/* The expansion card also provides DIP sockets for further memory expansion.     */
/* Populating the DIP sockets with the Graphics Memory Module Kit adds two        */
/* additional 16K banks to each bit plane, bringing the graphics memory to its    */
/* maximum of 256K bytes.                                                         */
/* ------------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: CRT Control Registers:         */
/* ------------------------------------------------------------------------------------------ */
/* Mode Control Register:                                                                     */
/* This is a write-only register pointed to when the value in the CRT Controller address      */
/* register is hex 17. The processor output port address for this register is hex 3B5 or hex  */
/* 3D5.                                                                                       */
/* Mode Control Register Format:                                                              */
/*  - Bit 0: CMS 0                                                                            */
/*  - Bit 6: Word/Byte Mode                                                                   */
/* Bit 0: Compatibility Mode Support-- When this bit is a logical 0, the row scan address bit */
/* 0 is substituted for memory address bit 13 during active display time. A logical 1 enables */
/* memory address bit 13 to appear on the memory address output bit 13 signal of the CRT      */
/* Controller. The CRT Controller used on the IBM Color/Graphics Monitor Adapter is the 6456. */
/* The 6845 has 128 horizontal scan line address capability. To obtain 640 by 200 graphics    */
/* resolution, the CRTC was programmed for 100 horizontal scan lines with 2 row scan addresses*/
/* per character row. Row scan address bit 0 became the most significant address bit to the   */
/* display buffer. Successive scan lines of the display image were displaced in memory by 8K  */
/* bytes. This bit allows compatibility with the 6845 and Color Graphics APA modes of         */
/* operation.                                                                                 */
/*                                                                                            */
/* Bit 6: Word Mode or Byte Mode--When this bit is a logical 0, the Word Mode shifts all      */
/* memory address counter bits down one bit, and the most significant bit of the counter      */
/* appears on the least significant bit of the memory address outputs. See table below for    */
/* address output details. A logical 1 selects the Byte Address mode.                         */
/*               Internal Memory Address Counter                                              */
/*               Wiring to the Output Multiplexer:                                            */
/* CRTC Out Pin: Byte Address Mode: Word Address Mode:                                        */
/*  - MA 0/RFA 0: - MA  0:           - MA 15 or MA 13:                                        */
/*  - MA 1/RFA 1: - MA  1:           - MA  0:                                                 */
/*  - MA 2/RFA 2: - MA  2:           - MA  1:                                                 */
/*  - MA 3/RFA 3: - MA  3:           - MA  2:                                                 */
/*  -             -                  -                                                        */
/*  -             -                  -                                                        */
/*  -             -                  -                                                        */
/*  - MA 14/RS 3: - MA 14:           - MA 13:                                                 */
/*  - MA 15/RS 4: - MA 15:           - MA 14:                                                 */
/* Bit 7: Hardware Reset--A logical 0 forces horizontal and vertical retrace to clear. A      */
/* logical 1 forces horizontal and vertical retrace to be enabled.                            */
/* ------------------------------------------------------------------------------------------ */
	if ((ega->reg_crt[EGA_CRT_MODE] & EGA_CRT_MODE_WB) == 0) {   /* Mode Control Register: Word Mode or Byte Mode */
		/* word mode */
		addr = ((addr << 1) | (addr >> 15)) & 0xffff;
	}
	if ((ega->reg_crt[EGA_CRT_MODE] & EGA_CRT_MODE_CMS0) == 0) { /* Mode Control Register: Compatibility Mode Support */
		/* CGA 8K bank simulation */
		addr = (addr & ~0x2000) | ((row & 1) << 13);
	}
	return (addr);
}

/*
 * Set the timing values from the CRT registers
 */
static
void ega_set_timing (ega_t *ega)
{
	int           d;
	unsigned      cw;
	unsigned long v;
/* ------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Major Components:  */
/* ------------------------------------------------------------------------------ */
/* CRT Controller:                                                                */
/* The CRT (Cathode Ray Tube) Controller (CRTC) generates horizontal and vertical */
/* synchronous timings, addressing for the regenerative buffer, cursor and        */
/* underline timings, and refresh addressing for the dynamic RAMs.                */
/* ------------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: CRT Control Registers:         */
/* ------------------------------------------------------------------------------------------ */
/* Name:                       Port:  Index:                                                  */
/*  - Address Register:         - 3?4: - -                                                    */
/*  - Horizontal Total:         - 3?5: - 00                                                   */
/*  - Horizontal Display End:   - 3?5: - 01                                                   */
/*  - Vertical   Total:         - 3?5: - 06                                                   */
/*  - Overflow:                 - 3?5: - 07                                                   */
/*  - Vertical Display End:     - 3?5: - 12                                                   */
/* ? = B in Monochrome Modes and D in Color Modes                                             */
/*                                                                                            */
/* Horizontal Total Register:                                                                 */
/* This is a write-only register pointed to when the value in the CRT Controller address is   */
/* hex 00. The processor output port address for this register is hex 3B5 or hex 3D5.         */
/* Horizontal Total Register Format:                                                          */
/*  - Bit 0: Horizontal Total                                                                 */
/*  - Bit 1: Horizontal Total                                                                 */
/*  - Bit 2: Horizontal Total                                                                 */
/*  - Bit 3: Horizontal Total                                                                 */
/*  - Bit 4: Horizontal Total                                                                 */
/*  - Bit 5: Horizontal Total                                                                 */
/*  - Bit 6: Horizontal Total                                                                 */
/*  - Bit 7: Horizontal Total                                                                 */
/* This register defines the total number of characters in the horizontal scan interval       */
/* including the retrace time. The value directly controls the period of the horizontal       */
/* retrace output signal. An internal horizontal character counter counts character clock     */
/* inputs to CRT Controller, and all horizontal and vertical timings are based upon the       */
/* horizontal register. Comparators are used to compare register values with horizontal       */
/* character values to provide horizontal timings.                                            */
/*                                                                                            */
/* Bit 0-Bit 7: Horizontal Total--The total number of characters less 2.                      */
/*                                                                                            */
/* Horizontal Display Enable End Register:                                                    */
/* This is a write-only register pointed to when the value in the CRT Controller address      */
/* register is hex 01. The processor output port address for this register is hex 3B5 or hex  */
/* 3D5.                                                                                       */
/* Horizontal Display Enable End Register Format:                                             */
/*  - Bit 0: Horizontal Display Enable End                                                    */
/*  - Bit 1: Horizontal Display Enable End                                                    */
/*  - Bit 2: Horizontal Display Enable End                                                    */
/*  - Bit 3: Horizontal Display Enable End                                                    */
/*  - Bit 4: Horizontal Display Enable End                                                    */
/*  - Bit 5: Horizontal Display Enable End                                                    */
/*  - Bit 6: Horizontal Display Enable End                                                    */
/*  - Bit 7: Horizontal Display Enable End                                                    */
/* This register defines the length of the horizontal display enable signal. It determines the*/
/* number of displayed character positions per horizontal line.                               */
/*  - Bit 0-Bit 7: Horizontal display enable end--A value one less than the total number of   */
/* displayed characters.                                                                      */
/*                                                                                            */
/* Vertical Total Register:                                                                   */
/* This is a write-only register pointed to when the value in the CRT Controller address      */
/* register is hex 06. The processor output port address for this register is hex 3B5 or 3D5. */
/* Vertical Total Register Format:                                                            */
/*  - Bit 0: Vertical Total                                                                   */
/*  - Bit 1: Vertical Total                                                                   */
/*  - Bit 2: Vertical Total                                                                   */
/*  - Bit 3: Vertical Total                                                                   */
/*  - Bit 4: Vertical Total                                                                   */
/*  - Bit 5: Vertical Total                                                                   */
/*  - Bit 6: Vertical Total                                                                   */
/*  - Bit 7: Vertical Total                                                                   */
/* Bit 0-Bit 7: Vertical Total--This is the low-order eight bits of a nine-bit register. The  */
/* binary value represents the number of horizontal raster scans on the CRT screen, including */
/* vertical retrace. The value in this register determines the period of the vertical retrace */
/* signal. Bit 8 of this register is contained in the CRT Controller Overflow Register hex 07 */
/* bit 0.                                                                                     */
/*                                                                                            */
/* CRT Controller Overflow Register:                                                          */
/* This is a write-only register pointed to when the value in the CRT Controller Address      */
/* Register is hex 07. The processor output port address for this register is hex 3B5 or hex  */
/* 3D5.                                                                                       */
/*  - Bit 0: Vertical Total              Bit 8                                                */
/*  - Bit 1: Vertical Display Enable End Bit 8                                                */
/*  - Bit 2: Vertical Retrace Start      Bit 8                                                */
/*  - Bit 3: Start Vertical Blank        Bit 8                                                */
/*  - Bit 4: Line Compare                Bit 8                                                */
/*  - Bit 5: Cursor Location             Bit 8                                                */
/* Bit 0: Vertical Total--Bit 8 of the Vertical Total register (index hex 06).                */
/*                                                                                            */
/* Bit 1: Vertical Display Enable End--Bit 8 of the Vertical Display Enable End register      */
/* (index hex 12).                                                                            */
/*                                                                                            */
/* Bit 2: Vertical Retrace Start--Bit 8 of the Vertical Retrace Start register (index hex 10).*/
/*                                                                                            */
/* Bit 3: Start Vertical Blank--Bit 8 of the Start Vertical Blank register (index hex 15).    */
/*                                                                                            */
/* Bit 4: Line Compare--Bit 8 of the Line Compare register (index hex 18).                    */
/*                                                                                            */
/* Bit 5: Cursor Location--Bit 8 of the Cursor Location register (index hex 0A).              */
/*                                                                                            */
/* Vertical Display Enable End Register:                                                      */
/* This is a write-only register pointed to when the value in the CRT Controller address      */
/* register is hex 12. The processor output port address for this register is hex 3B5 or hex  */
/* 3D5.                                                                                       */
/* Vertical Display Enable End Register Format:                                               */
/*  - Bit 0: Low Order Vertical Display Enable End                                            */
/*  - Bit 1: Low Order Vertical Display Enable End                                            */
/*  - Bit 2: Low Order Vertical Display Enable End                                            */
/*  - Bit 3: Low Order Vertical Display Enable End                                            */
/*  - Bit 4: Low Order Vertical Display Enable End                                            */
/*  - Bit 5: Low Order Vertical Display Enable End                                            */
/*  - Bit 6: Low Order Vertical Display Enable End                                            */
/*  - Bit 7: Low Order Vertical Display Enable End                                            */
/* Bit 0-Bit 7: Vertical Display Enable End--These are the low-order 8 bits of the vertical   */
/* display enable end position. This address specifies which scan line ends the active video  */
/* area of the screen. Bit 8 is in the overflow register location hex 07.                     */
/* ------------------------------------------------------------------------------------------ */
	cw = ega_get_cw (ega);

	v = cw * (ega->reg_crt[EGA_CRT_HT] + 2); /* Horizontal Total Register */
	d = (ega->clk_ht != v);
	ega->clk_ht = v;

	v = cw * (ega->reg_crt[EGA_CRT_HD] + 1); /* Horizontal Display Enable End Register */
	d |= (ega->clk_hd != v);
	ega->clk_hd = v;

	v = ega->reg_crt[EGA_CRT_VT]; /* Vertical Total Register */
	v |= (ega->reg_crt[EGA_CRT_OF] << 8) & 0x100; /* Overflow Register */
	v = (v + 1) * ega->clk_ht;
	d |= (ega->clk_vt != v);
	ega->clk_vt = v;

	v = ega->reg_crt[EGA_CRT_VD]; /* Vertical Display Enable End Register */
	v |= (ega->reg_crt[EGA_CRT_OF] << 7) & 0x100; /* Overflow Register */
	v = (v + 1) * ega->clk_ht;
	d |= (ega->clk_vd != v);
	ega->clk_vd = v;
	if (d) {
		ega->update_state |= EGA_UPDATE_DIRTY;
	}
}

/*
 * Get the dot clock
 */
static
unsigned long ega_get_dotclock (ega_t *ega)
{
	unsigned long long clk;
/* ------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Major Components:  */
/* ------------------------------------------------------------------------------ */
/* Sequencer:                                                                     */
/* The Sequencer generates basic memory timings for the dynamic RAMs and the      */
/* character clock for controlling regenerative memory fetches. It allows the     */
/* processor to access memory during active display intervals by inserting        */
/* dedicated processor memory cycles periodically between the display memory      */
/* cycles. Map mask registers are available to protect entire memory maps from    */
/* being changed.                                                                 */
/* ------------------------------------------------------------------------------ */
	clk = ega->video.dotclk[0];
/* ----------------------------------------------------------------------------------- */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Registers:              */
/* ----------------------------------------------------------------------------------- */
/* External Registers:                                                                 */
/* This section contains descriptions of the registers of the Enhanced Graphics Adapter*/
/* that are not contained in an LSI device.                                            */
/* Name:                              Port:  Index:                                    */
/*  - Miscellaneous Output Register  : - 3C2: - -:                                     */
/*  - Feature Control      Register  : - 3?A: - -:                                     */
/*  - Input Status         Register 0: - 3C2: - -:                                     */
/*  - Input Status         Register 1: - 3?2: - -:                                     */
/* ? = B in Monochrome Modes    ? = D in Color Modes                                   */
/* Miscellaneous Output Register:                                                      */
/* This is a write-only register. The processor output port address is hex 3C2. A      */
/* hardware reset causes all bits to reset to zero.                                    */
/* Miscellaneous Output Register Format:                                               */
/*  - Bit 0: I/O Address Select                                                        */
/*  - Bit 1: Enable Ram                                                                */
/*  - Bit 2: Clock Select 0                                                            */
/*  - Bit 3: Clock Select 1                                                            */
/*  - Bit 4: Disable Internal Video Drivers                                            */
/*  - Bit 5: Page Bit For Odd/Even                                                     */
/*  - Bit 6: Horizontal Retrace Polarity                                               */
/*  - Bit 7: Vertical   Retrace Polarity                                               */
/* Bit 0: 3BX/3DX CRTC I/O Address--This bit maps the CRTC I/O addresses for IBM       */
/* Monochrome or Color/Graphics Monitor Adapter emulation. A logical 0 sets CRTC       */
/* addresses to 3BX and Input Status Register 1 's address to 3BA for Monochrome       */
/* emulation. A logical 1 sets CRTC addresses to 3DX and Input Status Register 1's     */
/* address to 3DA for Color/Graphics Monitor Adapter emulation.                        */
/*                                                                                     */
/* Bit 1: Enable RAM--A logical 0 disables RAM from the processor; a logical 1 enables */
/* RAM to respond at addresses designated by the Control Data Select value programmed  */
/* into the Graphics Controllers.                                                      */
/*                                                                                     */
/* Bit 2-Bit 3: Clock Select--These two bits select the clock source according to the  */
/* following table:                                                                    */
/* Bits:                                                                               */
/*  - 3 2:                                                                             */
/* --------                                                                            */
/*  - 0 0-: Selects 14 MHz clock from the processor I/O channel                        */
/*  - 0 1-: Selects 16 MHz clock on-board oscillator                                   */
/*  - 1 0-: Selects external clock source from the feature connector.                  */
/*  - 1 1-: Not used                                                                   */
/*                                                                                     */
/* Bit 4: Disable Internal Video Drivers--A logical 0 activates internal video drivers;*/
/* a logical 1 disables internal video drivers. When the internal video drivers are    */
/* disabled, the source of the direct drive color output becomes the feature connector */
/* direct drive outputs.                                                               */
/*                                                                                     */
/* Bit 5: Page Bit For Odd/Even--Selects between two 64K pages of memory when in the   */
/* Odd/Even modes (0,1,2,3,7). A logical 0 selects the low page of memory; a logical 1 */
/* selects the high page of memory.                                                    */
/*                                                                                     */
/* Bit 6: Horizontal Retrace Polarity--A logical 0 selects positive horizontal retrace;*/
/* a logical 1 selects negative horizontal retrace.                                    */
/*                                                                                     */
/* Bit 7: Vertical   Retrace Polarity--A logical 0 selects positive vertical   retrace;*/
/* a logical 1 selects negative vertical   retrace.                                    */
/* ----------------------------------------------------------------------------------- */
	if (ega->reg[EGA_MOUT] & EGA_MOUT_CS) { /* Miscellaneous Output Register: Clock Select */
		clk *= EGA_PFREQ1;
	}
	else {
		clk *= EGA_PFREQ0;
	}
/* ------------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Display, August 2nd, 1984: Sequencer Registers:     */
/* ------------------------------------------------------------------------------------ */
/* Clocking Mode Register:                                                              */
/* This is write-only register pointed to when the value in the address register is hex */
/* 01. The output port address for this register is hex 3C5.                            */
/* Clocking Mode Register Format:                                                       */
/*  - Bit 0: 8/9 Dot Clocks                                                             */
/*  - Bit 1: Bandwidth                                                                  */
/*  - Bit 2: Shift Load                                                                 */
/*  - Bit 3: Dot Clock                                                                  */
/*  - Bit 4: Not Used                                                                   */
/*  - Bit 5: Not Used                                                                   */
/*  - Bit 6: Not Used                                                                   */
/*  - Bit 7: Not Used                                                                   */
/* Bit 3: Dot Clock--A logical 0 selects normal dot clocks derived from the sequencer   */
/* master clock input. When this bit is set to 1, the master clock will be divided by 2 */
/* to generate the dot clock. All the other timings will be stretched since they are    */
/* derived from the dot clock. Dot clock divided by two is used for 320x200 modes (0, 1,*/
/* 4, 5) to provide a pixel rate of 7 MHz, (9 MHz for mode D).                          */
/* ------------------------------------------------------------------------------------ */
	if (ega->reg_seq[EGA_SEQ_CLOCK] & EGA_SEQ_CLOCK_DC) { /* Sequencer Registers: Dot Clock */
		clk = clk / (2 * EGA_IFREQ);
	}
	else {
		clk = clk / EGA_IFREQ;
	}
	return (clk);
}

/*
 * Get a pointer to the bitmap for a character
 */
static
const unsigned char *ega_get_font (ega_t *ega, unsigned chr, unsigned atr)
{
	const unsigned char *fnt;
	unsigned            ofs;
/* ------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Major Components:  */
/* ------------------------------------------------------------------------------ */
/* Sequencer:                                                                     */
/* The Sequencer generates basic memory timings for the dynamic RAMs and the      */
/* character clock for controlling regenerative memory fetches. It allows the     */
/* processor to access memory during active display intervals by inserting        */
/* dedicated processor memory cycles periodically between the display memory      */
/* cycles. Map mask registers are available to protect entire memory maps from    */
/* being changed.                                                                 */
/* ------------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Display, August 2nd, 1984: Sequencer Registers:     */
/* ------------------------------------------------------------------------------------ */
/* Character Map Select Register:                                                       */
/* This is a write-only register pointed to when the value in the address register is   */
/* hex 03. The output port address for this register is 3C5.                            */
/*  - Bit 0: Character Map Select B                                                     */
/*  - Bit 1: Character Map Select B                                                     */
/*  - Bit 2: Character Map Select A                                                     */
/*  - Bit 3: Character Map Select A                                                     */
/*  - Bit 4: Not Used                                                                   */
/*  - Bit 5: Not Used                                                                   */
/*  - Bit 6: Not Used                                                                   */
/*  - Bit 7: Not Used                                                                   */
/* Bit 0-Bit 1: Character Map Select B--Selects the map used to generate alpha          */
/* characters when attribute bit 3 is a 0, according to the following table:            */
/* Bits     1 0:Map Selected:Table Location:                                            */
/*  - Value 0 0: - 0:         - 1st 8K of Plane 2 Bank 0                                */
/*  - Value 0 1: - 1:         - 2nd 8K of Plane 2 Bank 1                                */
/*  - Value 1 0: - 2:         - 3rd 8K of Plane 2 Bank 2                                */
/*  - Value 1 1: - 3:         - 4th 8K of Plane 2 Bank 3                                */
/* Bit 2-Bit 3: Character Map Select A--Selects the map used to generate alpha          */
/* characters when attribute bit 3 is a 1, according to the following table:            */
/* Bits     3 2:Map Selected:Table Location:                                            */
/*  - Value 0 0: - 0:         - 1st 8K of Plane 2 Bank 0                                */
/*  - Value 0 1: - 1:         - 2nd 8K of Plane 2 Bank 1                                */
/*  - Value 1 0: - 2:         - 3rd 8K of Plane 2 Bank 2                                */
/*  - Value 1 1: - 3:         - 4th 8K of Plane 2 Bank 3                                */
/* In alphanumeric mode, bit 3 of the attribute byte normally has the function of       */
/* turning the foreground intensity on or off. This bit however may be redefined as a   */
/* switch between character sets. This function is enabled when there is a difference   */
/* between the value in Character Map Select A and the value in Character Map Select B. */
/* Whenever these two values are the same, the character select function is disabled.   */
/* The memory mode register bit 1 must be a 1 (indicates the memory extension card is   */
/* installed in the unit) to enable this function; otherwise, bank 0 is always selected.*/
/*                                                                                      */
/* 128K of graphics memory is required to support two character sets. 256K supports four*/
/* character sets. Asynchronous reset clears this register to 0. This should be done    */
/* only when the sequencer is reset.                                                    */
/* ------------------------------------------------------------------------------------ */
	ofs = ega->reg_seq[EGA_SEQ_CMAPSEL]; /* Sequencer Registers: Character Map Select Register */
	if (atr & 0x08) {
		ofs = (ofs >> 2) & 0x03;
	}
	else {
		ofs = ofs & 0x03;
	}
	fnt = ega->mem + 0x20000 + (16384 * ofs) + (32 * (chr & 0xff));
	return (fnt);
}

/*
 * Set the internal screen buffer size
 */
static
int ega_set_buf_size (ega_t *ega, unsigned w, unsigned h)
{
	unsigned long cnt;
	unsigned char *tmp;
/* ------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Major Components:  */
/* ------------------------------------------------------------------------------ */
/* Display Buffer:                                                                */
/* The display buffer on the adapter consists of 64K bytes of dynamic read/write  */
/* memory configured as four 16K byte video bit planes. Two options are available */
/* for expanding the graphics memory. The Graphics Memory Expansion Card plugs    */
/* into the memory expansion connector on the adapter, and adds one bank of 16K   */
/* to each of the four bit planes, increasing the graphics memory to 128K bytes.  */
/* The expansion card also provides DIP sockets for further memory expansion.     */
/* Populating the DIP sockets with the Graphics Memory Module Kit adds two        */
/* additional 16K banks to each bit plane, bringing the graphics memory to its    */
/* maximum of 256K bytes.                                                         */
/* ------------------------------------------------------------------------------ */
	cnt = 3UL * (unsigned long) w * (unsigned long) h;

	if (cnt > ega->bufmax) {
		tmp = realloc (ega->buf, cnt);
		if (tmp == NULL) {
			return (1);
		}

		ega->buf = tmp;
		ega->bufmax = cnt;
	}
	ega->buf_w = w;
	ega->buf_h = h;
	return (0);
}

/*
 * Draw a character in the internal buffer
 */
static
void ega_mode0_update_char (ega_t *ega, unsigned char *dst, unsigned w,
	unsigned cw, unsigned ch, unsigned c, unsigned a, int crs)
{
	unsigned            x, y;
	unsigned            val;
	unsigned            c1, c2;
	unsigned            ull;
	int                 incrs, elg, blk;
	unsigned char       fg[3], bg[3];
	const unsigned char *fnt;
/* ------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Major Components:  */
/* ------------------------------------------------------------------------------ */
/* Attribute Controller:                                                          */
/* The Attribute Controller provides a color palette of 16 colors, each of which  */
/* may be specified separately. Six color outputs are available for driving a     */
/* display. Blinking and underlining are controlled by this chip. This chip takes */
/* data from the display memory and formats it for display on the CRT screen.     */
/*                                                                                */
/* Display Buffer:                                                                */
/* The display buffer on the adapter consists of 64K bytes of dynamic read/write  */
/* memory configured as four 16K byte video bit planes. Two options are available */
/* for expanding the graphics memory. The Graphics Memory Expansion Card plugs    */
/* into the memory expansion connector on the adapter, and adds one bank of 16K   */
/* to each of the four bit planes, increasing the graphics memory to 128K bytes.  */
/* The expansion card also provides DIP sockets for further memory expansion.     */
/* Populating the DIP sockets with the Graphics Memory Module Kit adds two        */
/* additional 16K banks to each bit plane, bringing the graphics memory to its    */
/* maximum of 256K bytes.                                                         */
/* ------------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Attribute Controller Registers:*/
/* ------------------------------------------------------------------------------------------ */
/* Name:                             Port:  Index:                                            */
/*  - Mode Control           Register: - 3C0: - 10:                                           */
/* Mode Control Register:                                                                     */
/* This is a write-only register pointed to by the value in the Attribute address register.   */
/* This value must be hex 10 before writing can take place. The processor output port address */
/* for this register is hex 3C0.                                                              */
/* Mode Control Register Format:                                                              */
/*  - Bit 0: Graphics/Alphanumeric Mode                                                       */
/*  - Bit 1: Display Type                                                                     */
/*  - Bit 2: Enable Line Graphics Character Codes                                             */
/*  - Bit 3: Select Background Intensity or Enable Blink                                      */
/*  - Bit 4: Not Used                                                                         */
/*  - Bit 5: Not Used                                                                         */
/*  - Bit 6: Not Used                                                                         */
/*  - Bit 7: Not Used                                                                         */
/* Bit 3: Enable Blink/Select Background Intensity--A logical 0 selects the background        */
/* intensity of the attribute input. This mode was available on the Monochrome and Color      */
/* Graphics adapters. A logical 1 enables the blink attribute in alphanumeric modes. This bit */
/* must also be set to 1 for blinking graphics modes.                                         */
/* ------------------------------------------------------------------------------------------ */
	blk = 0;

	if (ega->reg_atc[EGA_ATC_MODE] & EGA_ATC_MODE_EB) { /* Mode Control Register: Enable Blink/Select Background Intensity */
		if (a & 0x80) {
			blk = !ega->blink_on;
		}

		a &= 0x7f;
	}
	ega_get_palette (ega, a & 0x0f, fg, fg + 1, fg + 2);
	ega_get_palette (ega, (a >> 4) & 0x0f, bg, bg + 1, bg + 2);
/* ------------------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: CRT Control Registers:         */
/* ------------------------------------------------------------------------------------------ */
/* Name:                       Port:  Index:                                                  */
/*  - Cursor Start:             - 3?5: - 0A                                                   */
/*  - Cursor End:               - 3?5: - 0B                                                   */
/*  - Underline Location:       - 3?5: - 14                                                   */
/* ? = B in Monochrome Modes and D in Color Modes                                             */
/*                                                                                            */
/* Cursor Start Register:                                                                     */
/* This is a write-only register pointed to when the value in the CRT Controller register is  */
/* hex 0A. The processor output port address for this register is hex 3B5 or hex 3D5.         */
/* Cursor Start Register Format:                                                              */
/*  - Bit 0: Row Scan Cursor Begins                                                           */
/*  - Bit 1: Row Scan Cursor Begins                                                           */
/*  - Bit 2: Row Scan Cursor Begins                                                           */
/*  - Bit 3: Row Scan Cursor Begins                                                           */
/*  - Bit 4: Row Scan Cursor Begins                                                           */
/*  - Bit 5: Not Used                                                                         */
/*  - Bit 6: Not Used                                                                         */
/*  - Bit 7: Not Used                                                                         */
/* Bit 0-Bit 4: Cursor Start--This register specifies the row scan of a character line where  */
/* the cursor is to begin. The number programmed should be one less than the starting cursor  */
/* row scan.                                                                                  */
/*                                                                                            */
/* Cursor End Register:                                                                       */
/* This is a write-only register pointed to when the value in the CRT Controller address      */
/* register is hex 0B. The processor output port address for this register is hex 3B5 or hex  */
/* 3D5.                                                                                       */
/* Cursor End Register Format:                                                                */
/*  - Bit 0: Row Scan Cursor Ends                                                             */
/*  - Bit 1: Row Scan Cursor Ends                                                             */
/*  - Bit 2: Row Scan Cursor Ends                                                             */
/*  - Bit 3: Row Scan Cursor Ends                                                             */
/*  - Bit 4: Row Scan Cursor Ends                                                             */
/*  - Bit 5: Cursor Skew Control                                                              */
/*  - Bit 6: Cursor Skew Control                                                              */
/*  - Bit 7: Not Used                                                                         */
/* Bit 0-Bit 4: Cursor End--These bits specify the row scan where the cursor is to end.       */
/*                                                                                            */
/* Bit 5-Bit 6: Cursor Skew--These bits control the skew of the cursor signal.                */
/* Bits 6 5:                                                                                  */
/*  -   0 0: Zero  character clock skew                                                       */
/*  -   0 1: One   character clock skew                                                       */
/*  -   1 0: Two   character clock skew                                                       */
/*  -   1 1: Three character clock skew                                                       */
/*                                                                                            */
/* Underline Location Register:                                                               */
/* This is a write-only register pointed to when the value in the CRT Controller address is   */
/* hex 14. The processor output port address for this register is hex 3B5 or hex 3D5.         */
/* Underline Location Register Format:                                                        */
/*  - Bit 0: Horizontal row scan where underline will occur                                   */
/*  - Bit 1: Horizontal row scan where underline will occur                                   */
/*  - Bit 2: Horizontal row scan where underline will occur                                   */
/*  - Bit 3: Horizontal row scan where underline will occur                                   */
/*  - Bit 4: Horizontal row scan where underline will occur                                   */
/*  - Bit 5: Not Used                                                                         */
/*  - Bit 6: Not Used                                                                         */
/*  - Bit 7: Not Used                                                                         */
/* Bit 0-Bit 4: Underline Location--This register specifies the horizontal row scan on which  */
/* underline will occur. The value programmed is one less than the scan line number desired.  */
/* ------------------------------------------------------------------------------------------ */
	c1 = ega->reg_crt[EGA_CRT_CS] & 0x1f; /* Cursor Start Register */
	c2 = ega->reg_crt[EGA_CRT_CE] & 0x1f; /* Cursor End Register */
	crs = (crs && ega->blink_on);
	incrs = (c2 < c1);

/* ------------------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Attribute Controller Registers:*/
/* ------------------------------------------------------------------------------------------ */
/* Name:                             Port:  Index:                                            */
/*  - Mode Control           Register: - 3C0: - 10:                                           */
/* Mode Control Register:                                                                     */
/* This is a write-only register pointed to by the value in the Attribute address register.   */
/* This value must be hex 10 before writing can take place. The processor output port address */
/* for this register is hex 3C0.                                                              */
/* Mode Control Register Format:                                                              */
/*  - Bit 0: Graphics/Alphanumeric Mode                                                       */
/*  - Bit 1: Display Type                                                                     */
/*  - Bit 2: Enable Line Graphics Character Codes                                             */
/*  - Bit 3: Select Background Intensity or Enable Blink                                      */
/*  - Bit 4: Not Used                                                                         */
/*  - Bit 5: Not Used                                                                         */
/*  - Bit 6: Not Used                                                                         */
/*  - Bit 7: Not Used                                                                         */
/* Bit 2: Enable Line Graphics Character Codes--When this bit is set to 0, the ninth dot will */
/* be the same as the background. A logical 1 enables the special line graphics character     */
/* codes for the IBM Monochrome Display adapter. This bit when enabled forces the ninth dot of*/
/* a line graphic character to be identical to the eighth dot of the character. The line      */
/* graphics character codes for the Monochrome Display Adapter are Hex C0 through Hex DF.     */
/* ------------------------------------------------------------------------------------------ */
	elg = 0;

	if (ega->reg_atc[EGA_ATC_MODE] & EGA_ATC_MODE_ELG) { /* Mode Control Register: Enable Line Graphics Character Codes */
		if ((c >= 0xc0) && (c <= 0xdf)) {
			elg = 1;
		}
	}
/* ------------------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Attribute Controller Registers:*/
/* ------------------------------------------------------------------------------------------ */
/* Name:                             Port:  Index:                                            */
/*  - Mode Control           Register: - 3C0: - 10:                                           */
/* Mode Control Register:                                                                     */
/* This is a write-only register pointed to by the value in the Attribute address register.   */
/* This value must be hex 10 before writing can take place. The processor output port address */
/* for this register is hex 3C0.                                                              */
/* Mode Control Register Format:                                                              */
/*  - Bit 0: Graphics/Alphanumeric Mode                                                       */
/*  - Bit 1: Display Type                                                                     */
/*  - Bit 2: Enable Line Graphics Character Codes                                             */
/*  - Bit 3: Select Background Intensity or Enable Blink                                      */
/*  - Bit 4: Not Used                                                                         */
/*  - Bit 5: Not Used                                                                         */
/*  - Bit 6: Not Used                                                                         */
/*  - Bit 7: Not Used                                                                         */
/* Bit 1: Monochrome Display/Color Display--A logical 0 selects color display attributes. A   */
/* logical 1 selects IBM Monochrome Display attributes.                                       */
/* ------------------------------------------------------------------------------------------ */
	ull = 0xffff;
	if (ega->reg_atc[EGA_ATC_MODE] & EGA_ATC_MODE_ME) { /* Mode Control Register: Monochrome Display/Color Display */
		if ((a & 0x07) == 1) {
			ull = ega->reg_crt[EGA_CRT_ULL] & 0x1f; /* Underline Location Register */
		}
	}
	fnt = ega_get_font (ega, c, a);

	for (y = 0; y < ch; y++) {
		if (blk) {
			val = 0;
		}
		else if (y == ull) {
			val = 0xffff;
		}
		else {
			val = fnt[y] << 1;

			if (elg) {
				val |= (val >> 1) & 1;
			}
		}
		if (crs) {
			if (y == c1) {
				incrs = !incrs;
			}

			if (y == c2) {
				incrs = !incrs;
			}

			if (incrs) {
				val = 0xffff;
			}
		}
		for (x = 0; x < cw; x++) {
			if (val & 0x100) {
				dst[3 * x + 0] = fg[0];
				dst[3 * x + 1] = fg[1];
				dst[3 * x + 2] = fg[2];
			}
			else {
				dst[3 * x + 0] = bg[0];
				dst[3 * x + 1] = bg[1];
				dst[3 * x + 2] = bg[2];
			}
			val <<= 1;
		}
		dst += 3 * w;
	}
}

/*
 * Update text mode
 */
static
void ega_update_text (ega_t *ega)
{
	unsigned            x, y, w, h, cw, ch;
	unsigned            w2, h2;
	unsigned            addr, rptr, rofs, p;
	unsigned            cpos;
	const unsigned char *src;
	unsigned char       *dst;

	w = ega_get_w (ega);
	h = ega_get_h (ega);
	cw = ega_get_cw (ega);
	ch = ega_get_ch (ega);
/* ------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Major Components:  */
/* ------------------------------------------------------------------------------ */
/* Display Buffer:                                                                */
/* The display buffer on the adapter consists of 64K bytes of dynamic read/write  */
/* memory configured as four 16K byte video bit planes. Two options are available */
/* for expanding the graphics memory. The Graphics Memory Expansion Card plugs    */
/* into the memory expansion connector on the adapter, and adds one bank of 16K   */
/* to each of the four bit planes, increasing the graphics memory to 128K bytes.  */
/* The expansion card also provides DIP sockets for further memory expansion.     */
/* Populating the DIP sockets with the Graphics Memory Module Kit adds two        */
/* additional 16K banks to each bit plane, bringing the graphics memory to its    */
/* maximum of 256K bytes.                                                         */
/*                                                                                */
/* The address of the display buffer can be changed to remain compatible with     */
/* other video cards and application software. Four locations are provided. The   */
/* buffer can be configured at segment address hex A0000 for a length of 128K     */
/* bytes, at hex A0000 for a length of 64K bytes, at hex B0000 for a length of 32K*/
/* bytes, or at hex B8000 for a length of 32K bytes.                              */
/* ------------------------------------------------------------------------------ */
	if (ega_set_buf_size (ega, w, h)) {
		return;
	}
	src = ega->mem;
	dst = ega->buf;
/* ------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Major Components:  */
/* ------------------------------------------------------------------------------ */
/* Support Logic:                                                                 */
/* The logic on the card surrounding the LIS modules supports the modules and     */
/* creates latch buses for the CRT controller, the processor, and character       */
/* generator. Two clock sources (14 MHz and 16 MHz) provide the dot rate. The     */
/* clock is multiplexed under processor I/O control. Four I/O registers also      */
/* resident on the card are not part of the LSI devices.                          */
/* ------------------------------------------------------------------------------ */
	addr = ega->latch_addr;
/* ------------------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: CRT Control Registers:         */
/* ------------------------------------------------------------------------------------------ */
/* Name:                       Port:  Index:                                                  */
/*  - Offset:                   - 3?5: - 13                                                   */
/* Offset Register:                                                                           */
/* This is a write-only register pointed to when the value in the CRT Controller address      */
/* register is hex 13. The processor output port address for this register is hex 3B5 or hex  */
/* 3D5.                                                                                       */
/* Offset Register Format:                                                                    */
/*  - Bit 0: Logical line width of the screen                                                 */
/*  - Bit 1: Logical line width of the screen                                                 */
/*  - Bit 2: Logical line width of the screen                                                 */
/*  - Bit 3: Logical line width of the screen                                                 */
/*  - Bit 4: Logical line width of the screen                                                 */
/*  - Bit 5: Logical line width of the screen                                                 */
/*  - Bit 6: Logical line width of the screen                                                 */
/*  - Bit 7: Logical line width of the screen                                                 */
/* Bit 0-Bit 7: Offset--This register specifies the logical line width of the screen. The     */
/* starting memory address for the next character row is larger than the current character row*/
/* by this amount. The Offset Register is programmed with a word address. Depending upon the  */
/* method of clocking the CRT Controller, this word address is either a word or double word   */
/* address.                                                                                   */
/* ------------------------------------------------------------------------------------------ */
	rofs = 2 * ega->reg_crt[EGA_CRT_OFS]; /* Offset Register */
	cpos = ega_get_cursor (ega);
	y = 0;
	while (y < h) {
		h2 = h - y;
		if (h2 > ch) {
			h2 = ch;
		}
		dst = ega->buf + 3UL * y * w;
		rptr = addr;
		x = 0;
		while (x < w) {
			w2 = w - x;
			if (w2 > cw) {
				w2 = cw;
			}
			p = ega_get_crtc_addr (ega, rptr, 0);
			ega_mode0_update_char (ega, dst + 3 * x, w, w2, h2,
				src[p], src[p + 0x10000], rptr == cpos
			);
			rptr = (rptr + 1) & 0xffff;
			x += w2;
		}
		addr = (addr + rofs) & 0xffff;
		y += h2;
	}
}

/*
 * Update graphics mode
 */
static
void ega_update_graphics (ega_t *ega)
{
	unsigned            x, y, w, h;
	unsigned            row, col, cw, ch;
	unsigned            lcmp, hpp;
	unsigned            addr, rptr, rofs, ptr;
	unsigned char       blink1, blink2;
	unsigned            msk, bit;
	unsigned            idx;
	unsigned char       buf[4];
	const unsigned char *src;
	unsigned char       *dst;
/* ------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Major Components:  */
/* ------------------------------------------------------------------------------ */
/* Display Buffer:                                                                */
/* The display buffer on the adapter consists of 64K bytes of dynamic read/write  */
/* memory configured as four 16K byte video bit planes. Two options are available */
/* for expanding the graphics memory. The Graphics Memory Expansion Card plugs    */
/* into the memory expansion connector on the adapter, and adds one bank of 16K   */
/* to each of the four bit planes, increasing the graphics memory to 128K bytes.  */
/* The expansion card also provides DIP sockets for further memory expansion.     */
/* Populating the DIP sockets with the Graphics Memory Module Kit adds two        */
/* additional 16K banks to each bit plane, bringing the graphics memory to its    */
/* maximum of 256K bytes.                                                         */
/*                                                                                */
/* The address of the display buffer can be changed to remain compatible with     */
/* other video cards and application software. Four locations are provided. The   */
/* buffer can be configured at segment address hex A0000 for a length of 128K     */
/* bytes, at hex A0000 for a length of 64K bytes, at hex B0000 for a length of 32K*/
/* bytes, or at hex B8000 for a length of 32K bytes.                              */
/* ------------------------------------------------------------------------------ */
	w = ega_get_w (ega);
	h = ega_get_h (ega);
	cw = ega_get_cw (ega);
	ch = ega_get_ch (ega);
	lcmp = ega_get_line_compare (ega);
	if (ega_set_buf_size (ega, w, h)) {
		return;
	}
/* ------------------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Attribute Controller Registers:*/
/* ------------------------------------------------------------------------------------------ */
/* Name:                             Port:  Index:                                            */
/*  - Mode Control           Register: - 3C0: - 10:                                           */
/* Mode Control Register:                                                                     */
/* This is a write-only register pointed to by the value in the Attribute address register.   */
/* This value must be hex 10 before writing can take place. The processor output port address */
/* for this register is hex 3C0.                                                              */
/* Mode Control Register Format:                                                              */
/*  - Bit 0: Graphics/Alphanumeric Mode                                                       */
/*  - Bit 1: Display Type                                                                     */
/*  - Bit 2: Enable Line Graphics Character Codes                                             */
/*  - Bit 3: Select Background Intensity or Enable Blink                                      */
/*  - Bit 4: Not Used                                                                         */
/*  - Bit 5: Not Used                                                                         */
/*  - Bit 6: Not Used                                                                         */
/*  - Bit 7: Not Used                                                                         */
/* Bit 3: Enable Blink/Select Background Intensity--A logical 0 selects the background        */
/* intensity of the attribute input. This mode was available on the Monochrome and Color      */
/* Graphics adapters. A logical 1 enables the blink attribute in alphanumeric modes. This bit */
/* must also be set to 1 for blinking graphics modes.                                         */
/* ------------------------------------------------------------------------------------------ */
	if (ega->reg_atc[EGA_ATC_MODE] & EGA_ATC_MODE_EB) { /* Mode Control Register: Enable Blink/Select Background Intensity */
		blink1 = 0xff;
		blink2 = ega->blink_on ? 0xff : 0x00;
	}
	else {
		blink1 = 0x00;
		blink2 = 0x00;
	}
	hpp = ega->latch_hpp;

	src = ega->mem;
	dst = ega->buf;
/* ------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Major Components:  */
/* ------------------------------------------------------------------------------ */
/* Support Logic:                                                                 */
/* The logic on the card surrounding the LIS modules supports the modules and     */
/* creates latch buses for the CRT controller, the processor, and character       */
/* generator. Two clock sources (14 MHz and 16 MHz) provide the dot rate. The     */
/* clock is multiplexed under processor I/O control. Four I/O registers also      */
/* resident on the card are not part of the LSI devices.                          */
/* ------------------------------------------------------------------------------ */
	addr = ega->latch_addr;
/* ------------------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: CRT Control Registers:         */
/* ------------------------------------------------------------------------------------------ */
/* Name:                       Port:  Index:                                                  */
/*  - Offset:                   - 3?5: - 13                                                   */
/* Offset Register:                                                                           */
/* This is a write-only register pointed to when the value in the CRT Controller address      */
/* register is hex 13. The processor output port address for this register is hex 3B5 or hex  */
/* 3D5.                                                                                       */
/* Offset Register Format:                                                                    */
/*  - Bit 0: Logical line width of the screen                                                 */
/*  - Bit 1: Logical line width of the screen                                                 */
/*  - Bit 2: Logical line width of the screen                                                 */
/*  - Bit 3: Logical line width of the screen                                                 */
/*  - Bit 4: Logical line width of the screen                                                 */
/*  - Bit 5: Logical line width of the screen                                                 */
/*  - Bit 6: Logical line width of the screen                                                 */
/*  - Bit 7: Logical line width of the screen                                                 */
/* Bit 0-Bit 7: Offset--This register specifies the logical line width of the screen. The     */
/* starting memory address for the next character row is larger than the current character row*/
/* by this amount. The Offset Register is programmed with a word address. Depending upon the  */
/* method of clocking the CRT Controller, this word address is either a word or double word   */
/* address.                                                                                   */
/* ------------------------------------------------------------------------------------------ */
	rofs = 2 * ega->reg_crt[EGA_CRT_OFS]; /* Offset Register */

	row = 0;
	y = 0;
	while (y < h) {
		dst = ega->buf + 3UL * y * w;

		rptr = addr;

		ptr = ega_get_crtc_addr (ega, rptr, row);

		buf[0] = src[ptr + 0x00000];
		buf[1] = src[ptr + 0x10000];
		buf[2] = src[ptr + 0x20000];
		buf[3] = (src[ptr + 0x30000] ^ blink1) | blink2;

		msk = 0x80 >> (hpp & 7);
		bit = (2 * hpp) & 6;
		col = hpp;

		rptr = (rptr + 1) & 0xffff;

		x = 0;
		while (x < w) {
			if (col >= cw) {
				ptr = ega_get_crtc_addr (ega, rptr, row);

				buf[0] = src[ptr + 0x00000];
				buf[1] = src[ptr + 0x10000];
				buf[2] = src[ptr + 0x20000];
				buf[3] = (src[ptr + 0x30000] ^ blink1) | blink2;

				msk = 0x80;
				bit = 0;
				col = 0;

				rptr = (rptr + 1) & 0xffff;
			}
/* ------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Major Components:  */
/* ------------------------------------------------------------------------------ */
/* Graphics Controller:                                                           */
/* The Graphics Controller directs the data from the memory to the attribute      */
/* controller and the processor. In graphics modes, memory data is sent in        */
/* serialized form to the attribute chip. In alpha modes the memory data is sent  */
/* in parallel form, bypassing the graphics controller. The graphics controller   */
/* formats the data for compatible modes and provides color comparators for use in*/
/* color painting modes. Other hardware facilities allow the processor to write 32*/
/* bits in a single memory cycle, (8 bites per plane) for quick color presetting  */
/* of the display areas, and additional logic allows the processor to write data  */
/* to the display on non-byte boundaries.                                         */
/* ------------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Graphics Controller Registers: */
/* ------------------------------------------------------------------------------------------ */
/* Name:                    Port:  Index:                                                     */
/*  - Mode Register         : - 3CF: - 05:                                                    */
/* Mode Register:                                                                             */
/* This is a write-only register pointed to by the value in the Graphics 1 and 2 address      */
/* register. This value must be hex 05 before writing can take place. The processor output    */
/* port address for this register is 3CF.                                                     */
/* Mode Register Format:                                                                      */
/*  - Bit 0: Write Mode                                                                       */
/*  - Bit 1: Write Mode                                                                       */
/*  - Bit 2: Test Condition                                                                   */
/*  - Bit 3: Read  Mode                                                                       */
/*  - Bit 4: Odd/Even                                                                         */
/*  - Bit 5: Test Register Mode                                                               */
/*  - Bit 6: Not Used                                                                         */
/*  - Bit 7: Not Used                                                                         */
/* Bit-Bit 1: Write Mode                                                                      */
/* Bits 1 0:                                                                                  */
/*  -   0 0: Each memory plane is written with the processor data rotated by the number of    */
/*  counts in the rotate register, unless Set/Reset is enabled for the plane. Planes for which*/
/*  Set/Reset is enabled are written with 8 bits of the value contained in the Set/Reset      */
/*  register for that plane.                                                                  */
/*  -   0 1: Each memory plane is written with the contents of the processor latches. These   */
/*  latches are loaded by a processor read operation.                                         */
/*  -   1 0: Memory plane n (0 through 3) is filled with 8 bits of the value of data bit n.   */
/*  -   1 1: Not Valid                                                                        */
/* The logic function specified by the function select register also applies.                 */
/*                                                                                            */
/* Bit 2: Test Condition--A logical 1 directs graphics controller outputs to be placed in high*/
/* impedance state for testing.                                                               */
/*                                                                                            */
/* Bit 3: Read Mode--When this bit is a logical 0, the processor reads data from the memory   */
/* plane selected by the read map select register. When this bit is a logical 1, the processor*/
/* reads the results of the comparison of the 4 memory planes and the color compare register. */
/*                                                                                            */
/* Bit 4: Odd/Even--A logical 1 selects the odd/even addressing mode, which is useful for     */
/* emulation of the Color Graphics Monitor Adapter compatible modes. Normally the value here  */
/* follows the value of the Memory Mode Register bit 3 of the Sequencer.                      */
/*                                                                                            */
/* Bit 5: Shift Register--A logical 1 directs the shift registers on each graphics chip to    */
/* format the serial data stream with even numbered bits on the even numbered maps and odd    */
/* numbered bits on the odd maps.                                                             */
/* ------------------------------------------------------------------------------------------ */
			if (ega->reg_grc[EGA_GRC_MODE] & EGA_GRC_MODE_SR) { /* Mode Register: Shift Register */
				/* CGA 4 color mode */

				idx = (buf[0] >> (6 - bit)) & 0x03;
				bit += 2;

				if (bit > 6) {
					buf[0] = buf[1];
					buf[1] = 0;
					bit = 0;
				}
			}
			else {
				idx = (buf[0] & msk) ? 0x01 : 0x00;
				idx |= (buf[1] & msk) ? 0x02 : 0x00;
				idx |= (buf[2] & msk) ? 0x04 : 0x00;
				idx |= (buf[3] & msk) ? 0x08 : 0x00;
				msk >>= 1;
			}

			ega_get_palette (ega, idx, dst, dst + 1, dst + 2);

			dst += 3;
			col += 1;
			x += 1;
		}
		row += 1;
		if (row >= ch) {
			row = 0;
			addr = (addr + rofs) & 0xffff;
		}
		if (y == lcmp) {
			addr = 0;
		}
		y += 1;
	}
}

/*
 * Update the internal screen buffer when the screen is blank
 */
static
void ega_update_blank (ega_t *ega)
{
	unsigned long x, y;
	int           fx, fy;
	unsigned char *dst;
/* ------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Major Components:  */
/* ------------------------------------------------------------------------------ */
/* Display Buffer:                                                                */
/* The display buffer on the adapter consists of 64K bytes of dynamic read/write  */
/* memory configured as four 16K byte video bit planes. Two options are available */
/* for expanding the graphics memory. The Graphics Memory Expansion Card plugs    */
/* into the memory expansion connector on the adapter, and adds one bank of 16K   */
/* to each of the four bit planes, increasing the graphics memory to 128K bytes.  */
/* The expansion card also provides DIP sockets for further memory expansion.     */
/* Populating the DIP sockets with the Graphics Memory Module Kit adds two        */
/* additional 16K banks to each bit plane, bringing the graphics memory to its    */
/* maximum of 256K bytes.                                                         */
/*                                                                                */
/* The address of the display buffer can be changed to remain compatible with     */
/* other video cards and application software. Four locations are provided. The   */
/* buffer can be configured at segment address hex A0000 for a length of 128K     */
/* bytes, at hex A0000 for a length of 64K bytes, at hex B0000 for a length of 32K*/
/* bytes, or at hex B8000 for a length of 32K bytes.                              */
/* ------------------------------------------------------------------------------ */
	ega_set_buf_size (ega, 320, 200);
	dst = ega->buf;
	for (y = 0; y < 200; y++) {
		fy = (y % 16) < 8;

		for (x = 0; x < 320; x++) {
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
void ega_update (ega_t *ega)
{
	int      show;
	unsigned w, h;
/* ------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Major Components:  */
/* ------------------------------------------------------------------------------ */
/* Attribute Controller:                                                          */
/* The Attribute Controller provides a color palette of 16 colors, each of which  */
/* may be specified separately. Six color outputs are available for driving a     */
/* display. Blinking and underlining are controlled by this chip. This chip takes */
/* data from the display memory and formats it for display on the CRT screen.     */
/*                                                                                */
/* Display Buffer:                                                                */
/* The display buffer on the adapter consists of 64K bytes of dynamic read/write  */
/* memory configured as four 16K byte video bit planes. Two options are available */
/* for expanding the graphics memory. The Graphics Memory Expansion Card plugs    */
/* into the memory expansion connector on the adapter, and adds one bank of 16K   */
/* to each of the four bit planes, increasing the graphics memory to 128K bytes.  */
/* The expansion card also provides DIP sockets for further memory expansion.     */
/* Populating the DIP sockets with the Graphics Memory Module Kit adds two        */
/* additional 16K banks to each bit plane, bringing the graphics memory to its    */
/* maximum of 256K bytes.                                                         */
/*                                                                                */
/* The address of the display buffer can be changed to remain compatible with     */
/* other video cards and application software. Four locations are provided. The   */
/* buffer can be configured at segment address hex A0000 for a length of 128K     */
/* bytes, at hex A0000 for a length of 64K bytes, at hex B0000 for a length of 32K*/
/* bytes, or at hex B8000 for a length of 32K bytes.                              */
/* ------------------------------------------------------------------------------ */
	ega->buf_w = 0;
	ega->buf_h = 0;

	w = ega_get_w (ega);
	h = ega_get_h (ega);

	show = 1;
/* ------------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Display, August 2nd, 1984: Sequencer Registers:     */
/* ------------------------------------------------------------------------------------ */
/* Reset Register:                                                                      */
/* This is a write-only register pointed to when the value in the address register is   */
/* hex 00. The output port address for this register is hex 3C5.                        */
/* Reset Register Format:                                                               */
/*  - Bit 0:Asynchronous Reset                                                          */
/*  - Bit 1: Synchronous Reset                                                          */
/*  - Bit 2: Not Used                                                                   */
/*  - Bit 3: Not Used                                                                   */
/*  - Bit 4: Not Used                                                                   */
/*  - Bit 5: Not Used                                                                   */
/*  - Bit 6: Not Used                                                                   */
/*  - Bit 7: Not Used                                                                   */
/* Bit 0:Asynchronous Reset--A logical 0 commands the sequencer to asynchronous clear   */
/* and halt. All outputs are placed in the high impedance state when this bit is a 0. A */
/* logical 1 commands the sequencer to run unless bit 1 is set to zero. Resetting the   */
/* sequencer with this bit can cause data loss in the dynamic RAMs.                     */
/*                                                                                      */
/* Bit 1: Synchronous Reset--A logical 0 commands the sequencer to synchronous clear and*/
/* halt. Bits 1 and 0 must both be ones to allow the sequencer to operate. Reset the    */
/* sequencer with this bit before changing the Clocking Mode Register, if memory        */
/* contents are to be preserved.                                                        */
/* ------------------------------------------------------------------------------------ */
	if (ega->reg_seq[EGA_SEQ_CLOCK] & EGA_SEQ_CLOCK_SO) {
		show = 0;
	}
	else if ((ega->reg_seq[EGA_SEQ_RESET] & EGA_SEQ_RESET_SR) == 0) {  /* Sequencer Registers:Asynchronous Reset */
		show = 0;
	}
	else if ((ega->reg_seq[EGA_SEQ_RESET] & EGA_SEQ_RESET_ASR) == 0) { /* Sequencer Registers: Synchronous Reset */
		show = 0;
	}
	else if ((w < 16) || (h < 16)) {
		show = 0;
	}
	if (show == 0) {
		ega_update_blank (ega);
		return;
	}
/* ------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Major Components:  */
/* ------------------------------------------------------------------------------ */
/* Graphics Controller:                                                           */
/* The Graphics Controller directs the data from the memory to the attribute      */
/* controller and the processor. In graphics modes, memory data is sent in        */
/* serialized form to the attribute chip. In alpha modes the memory data is sent  */
/* in parallel form, bypassing the graphics controller. The graphics controller   */
/* formats the data for compatible modes and provides color comparators for use in*/
/* color painting modes. Other hardware facilities allow the processor to write 32*/
/* bits in a single memory cycle, (8 bites per plane) for quick color presetting  */
/* of the display areas, and additional logic allows the processor to write data  */
/* to the display on non-byte boundaries.                                         */
/* ------------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Graphics Controller Registers: */
/* ------------------------------------------------------------------------------------------ */
/* Name:                    Port:  Index:                                                     */
/*  - Miscellaneous         : - 3CF: - 06:                                                    */
/* Miscellaneous Register:                                                                    */
/* This is a write-only register pointed to by the value in the Graphics 1 and 2 address      */
/* register. This value must be hex 06 before writing can take place. The processor output    */
/* port for this register is hex 3CF.                                                         */
/* Miscellaneous Register Format:                                                             */
/*  - Bit 0: Graphics Mode                                                                    */
/*  - Bit 1: Chain Odd Maps to Even                                                           */
/*  - Bit 2: Memory Map 0                                                                     */
/*  - Bit 3: Memory Map 1                                                                     */
/*  - Bit 4: Not Used                                                                         */
/*  - Bit 5: Not Used                                                                         */
/*  - Bit 6: Not Used                                                                         */
/*  - Bit 7: Not Used                                                                         */
/* Bit 0: Graphics Mode--This bit controls alpha-mode addressing. A logical 1 selects graphics*/
/* mode. When set to graphics mode, the character generator address latches are disabled.     */
/*                                                                                            */
/* Bit 1: Chain Odd Maps To Even Maps--When set to 1, this bit directs the processor address  */
/* bit 0 to be replaced by a higher order bit and odd/even maps to be selected with odd/even  */
/* values of the processor A0 bit, respectively.                                              */
/*                                                                                            */
/* Bit 2-Bit 3: Memory Map--These bits control the mapping of the regenerative buffer into the*/
/* processor address space.                                                                   */
/* Bits 3 2:                                                                                  */
/*  -   0 0: Hex A000 for 128K bytes.                                                         */
/*  -   0 1: Hex A000 for  64K bytes.                                                         */
/*  -   1 0: Hex B000 for  32K bytes.                                                         */
/*  -   1 1: Hex B800 for  32K bytes.                                                         */
/* If the display adapter is mapped at address hex A000 for 128K bytes, no other adapter can  */
/* be installed in the system.                                                                */
/* ------------------------------------------------------------------------------------------ */
	if (ega->reg_grc[EGA_GRC_MISC] & EGA_GRC_MISC_GM) { /* Miscellaneous Register: Graphics Mode */
		ega_update_graphics (ega);
	}
	else {
		ega_update_text (ega);
	}
}


static
unsigned char ega_mem_get_uint8 (ega_t *ega, unsigned long addr)
{
	unsigned rdmode;
	unsigned a0;
/* ----------------------------------------------------------------------------------- */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Registers:              */
/* ----------------------------------------------------------------------------------- */
/* External Registers:                                                                 */
/* This section contains descriptions of the registers of the Enhanced Graphics Adapter*/
/* that are not contained in an LSI device.                                            */
/* Name:                              Port:  Index:                                    */
/*  - Miscellaneous Output Register  : - 3C2: - -:                                     */
/*  - Feature Control      Register  : - 3?A: - -:                                     */
/*  - Input Status         Register 0: - 3C2: - -:                                     */
/*  - Input Status         Register 1: - 3?2: - -:                                     */
/* ? = B in Monochrome Modes    ? = D in Color Modes                                   */
/* Miscellaneous Output Register:                                                      */
/* This is a write-only register. The processor output port address is hex 3C2. A      */
/* hardware reset causes all bits to reset to zero.                                    */
/* Miscellaneous Output Register Format:                                               */
/*  - Bit 0: I/O Address Select                                                        */
/*  - Bit 1: Enable Ram                                                                */
/*  - Bit 2: Clock Select 0                                                            */
/*  - Bit 3: Clock Select 1                                                            */
/*  - Bit 4: Disable Internal Video Drivers                                            */
/*  - Bit 5: Page Bit For Odd/Even                                                     */
/*  - Bit 6: Horizontal Retrace Polarity                                               */
/*  - Bit 7: Vertical   Retrace Polarity                                               */
/* Bit 0: 3BX/3DX CRTC I/O Address--This bit maps the CRTC I/O addresses for IBM       */
/* Monochrome or Color/Graphics Monitor Adapter emulation. A logical 0 sets CRTC       */
/* addresses to 3BX and Input Status Register 1 's address to 3BA for Monochrome       */
/* emulation. A logical 1 sets CRTC addresses to 3DX and Input Status Register 1's     */
/* address to 3DA for Color/Graphics Monitor Adapter emulation.                        */
/*                                                                                     */
/* Bit 1: Enable RAM--A logical 0 disables RAM from the processor; a logical 1 enables */
/* RAM to respond at addresses designated by the Control Data Select value programmed  */
/* into the Graphics Controllers.                                                      */
/* ----------------------------------------------------------------------------------- */
	if ((ega->reg[EGA_MOUT] & EGA_MOUT_ERAM) == 0) { /* Miscellaneous Output Register: Enable RAM */
		return (0xff);
	}
/* -------------------------------------------------------------------------------- */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Modes of Operation:  */
/* -------------------------------------------------------------------------------- */
/* IBM Color Display:                                                               */
/* The following table describes the modes supported by BIOS on the IBM Color       */
/* Display:                                                                         */
/* Mode:Type: Colors: Alpha Format: Buffer Start: Box Size: Max Pages: Resolution:  */
/*  - 0: - A/N: - 16:  - 40x25:      - B8000:      - 8x8:    - 8    :   - 320x200   */
/*  - 1: - A/N: - 16:  - 40x25:      - B8000:      - 8x8:    - 8    :   - 320x200   */
/*  - 2: - A/N: - 16:  - 80x25:      - B8000:      - 8x8:    - 8    :   - 640x200   */
/*  - 3: - A/N: - 16:  - 80x25:      - B8000:      - 8x8:    - 8    :   - 640x200   */
/*  - 4: - APA: -  4:  - 40x25:      - B8000:      - 8x8:    - 1    :   - 320x200   */
/*  - 5: - APA: -  4:  - 40x25:      - B8000:      - 8x8:    - 1    :   - 320x200   */
/*  - 6: - APA: -  2:  - 80x25:      - B8000:      - 8x8:    - 1    :   - 640x200   */
/*  - D: - APA: - 16:  - 40x25:      - A0000:      - 8x8:    - 2/4/8:   - 320x200   */
/*  - E: - APA: - 16:  - 80x25:      - A0000:      - 8x8:    - 1/2/4:   - 640x200   */
/* Modes 0 through 6 emulate the support provided by the IBM Color/Graphics monitor */
/* Adapter.                                                                         */
/*                                                                                  */
/* Modes 0,2 and 5 are identical to modes 1,3 and 4 respectively at the adapter's   */
/* direct drive interface.                                                          */
/*                                                                                  */
/* The Maximum Pages fields for modes D and E indicate the number of pages supported*/
/* when 64K, 128K or 256K bytes of graphics memory is installed, respectively.      */
/*                                                                                  */
/* IBM Monochrome Display:                                                          */
/* The following table describes the modes supported by BIOS on the IBM Monochrome  */
/* Display.                                                                         */
/* Mode:Type: Colors: Alpha Format: Buffer Start: Box Size: Max Pages: Resolution:  */
/*  - 7: - A/N: - 4:   - 80x25:      - B0000:      - 9x14:   - 8  :     - 720x350   */
/*  - F: - APA: - 4:   - 80x25:      - A0000:      - 8x14:   - 1/2:     - 640x350   */
/* Mode 7 emulates the support provided by the IBM Monochrome Display Adapter.      */
/*                                                                                  */
/* IBM Enhanced Color Display:                                                      */
/* The Enhanced Graphics Adapter supports attachment of the IBM Enhanced Color      */
/* Display. The IBM Enhanced Color Display is capable of running at the standard    */
/* television frequency of 15.75 KHz as well as running 21.85 KHz. The table below  */
/* summarizes the characteristics of the IBM Enhanced Color Display:                */
/*            Parameter            : TV Frequency   : High Resolution:              */
/*             - Horiz    Scan Rate:  - 15.75  KHz  :  - 21.85  KHz                 */
/*             - Vertical Scan Rate:  - 60      Hz  :  - 60      Hz                 */
/*             - Video Bandwidth   :  - 14.318 MHz  :  - 16.257 MHz                 */
/*             - Displayable Colors:  - 16 Maximum  :  - 16 or 64                   */
/*             - Character     Size:  - 7 by 7 Pels :  -   7 by   9 Pels            */
/*             - Character Box Size:  -  8 by 8 Pels:  -   8 by  14 Pels            */
/*             - Maximum Resolution:  - 640x200 Pels:  - 640 by 350 Pels            */
/*             - Alphanumeric Modes:  -   0,1,2,3   :  - 0,1,2,3                    */
/*             - Graphics     Modes:  - 4,5,6,D,E   :  - 10                         */
/* In the television frequency mode, the IBM Enhanced Color Display displays        */
/* information identical in color and resolution to the IBM Color Display.          */
/*                                                                                  */
/* In the high resolution mode, the adapter provides enhanced alphanumeric character*/
/* support. This enhanced alphanumeric support consists of transforming the 8 by 8  */
/* character box into an 8 by 14 character box, and providing 16 colors out of a    */
/* palette of 64 possible display colors. Display colors are changed by altering the*/
/* programming of the color palette registers in the Attribute Controller. In       */
/* alphanumeric modes, any 16 of 64 colors are displayable. the screen resolution is*/
/* 320x350 for modes 0 and 1, and 640x350 for modes 2 and 3.                        */
/*                                                                                  */
/* The resolution displayed on the IBM Enhanced Color Display is selected by the    */
/* switch settings on the Enhanced Graphics Adapter.                                */
/*                                                                                  */
/* The Enhanced Color Display is compatible with all modes listed for the IBM Color */
/* Display. the following table describes additional modes supported by BIOS for the*/
/* IBM Enhanced Color Display:                                                      */
/* Mode#:Type: Colors:  Alpha Format: Buffer Start: Box Size: Max Pages: Resolution:*/
/*  -  0*:- A/N: - 16/64: - 40x25:      - B8000:      - 8x14:   -   8:    - 320x350 */
/*  -  1*:- A/N: - 16/64: - 40x25:      - B8000:      - 8x14:   -   8:    - 320x350 */
/*  -  2*:- A/N: - 16/64: - 80x25:      - B8000:      - 8x14:   -   8:    - 640x350 */
/*  -  3*:- A/N: - 16/64: - 80x25:      - B8000:      - 8x14:   -   8:    - 640x350 */
/*  - 10*:- A/N: -  4/16: - 80x25:      - A8000:      - 8x14:   - 1/2:    - 640x350 */
/*                 16/64:                                                           */
/* * Note that modes 0, 1, 2, and 3, are also listed for IBM Color Display support. */
/* BIOS provides enhanced support for these modes when an Enhanced Color Display is */
/* attached.                                                                        */
/*                                                                                  */
/* The values in the "COLORS" field indicate 16 colors of a 64 color palette or 4   */
/* colors of a sixteen color palette.                                               */
/*                                                                                  */
/* In mode 10, The dual values for the "COLORS" field and the "MAX. PAGES" field    */
/* indicate the support provided when 64K or when greater than 64K of graphics      */
/* memory is installed, respectively.                                               */
/* -------------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Graphics Controller Registers: */
/* ------------------------------------------------------------------------------------------ */
/* Name:                    Port:  Index:                                                     */
/*  - Miscellaneous         : - 3CF: - 06:                                                    */
/* Miscellaneous Register:                                                                    */
/* This is a write-only register pointed to by the value in the Graphics 1 and 2 address      */
/* register. This value must be hex 06 before writing can take place. The processor output    */
/* port for this register is hex 3CF.                                                         */
/* Miscellaneous Register Format:                                                             */
/*  - Bit 0: Graphics Mode                                                                    */
/*  - Bit 1: Chain Odd Maps to Even                                                           */
/*  - Bit 2: Memory Map 0                                                                     */
/*  - Bit 3: Memory Map 1                                                                     */
/*  - Bit 4: Not Used                                                                         */
/*  - Bit 5: Not Used                                                                         */
/*  - Bit 6: Not Used                                                                         */
/*  - Bit 7: Not Used                                                                         */
/* Bit 0: Graphics Mode--This bit controls alpha-mode addressing. A logical 1 selects graphics*/
/* mode. When set to graphics mode, the character generator address latches are disabled.     */
/*                                                                                            */
/* Bit 1: Chain Odd Maps To Even Maps--When set to 1, this bit directs the processor address  */
/* bit 0 to be replaced by a higher order bit and odd/even maps to be selected with odd/even  */
/* values of the processor A0 bit, respectively.                                              */
/*                                                                                            */
/* Bit 2-Bit 3: Memory Map--These bits control the mapping of the regenerative buffer into the*/
/* processor address space.                                                                   */
/* Bits 3 2:                                                                                  */
/*  -   0 0: Hex A000 for 128K bytes.                                                         */
/*  -   0 1: Hex A000 for  64K bytes.                                                         */
/*  -   1 0: Hex B000 for  32K bytes.                                                         */
/*  -   1 1: Hex B800 for  32K bytes.                                                         */
/* If the display adapter is mapped at address hex A000 for 128K bytes, no other adapter can  */
/* be installed in the system.                                                                */
/* ------------------------------------------------------------------------------------------ */
	switch ((ega->reg_grc[EGA_GRC_MISC] >> 2) & 3) { /* Miscellaneous Register */
	case 0: /* 128K at A000 */
		break;

	case 1: /* 64K at A000 */
		if (addr > 0xffff) {
			return (0xff);
		}
		break;

	case 2: /* 32K at B000 */
		if ((addr < 0x10000) || (addr > 0x17fff)) {
			return (0xff);
		}
		addr -= 0x10000;
		break;

	case 3: /* 32K at B800 */
		if ((addr < 0x18000) || (addr > 0x1ffff)) {
			return (0xff);
		}
		addr -= 0x18000;
		break;
	}
	addr &= 0xffff;
	a0 = addr & 1;
/* ------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Major Components:  */
/* ------------------------------------------------------------------------------ */
/* Graphics Controller:                                                           */
/* The Graphics Controller directs the data from the memory to the attribute      */
/* controller and the processor. In graphics modes, memory data is sent in        */
/* serialized form to the attribute chip. In alpha modes the memory data is sent  */
/* in parallel form, bypassing the graphics controller. The graphics controller   */
/* formats the data for compatible modes and provides color comparators for use in*/
/* color painting modes. Other hardware facilities allow the processor to write 32*/
/* bits in a single memory cycle, (8 bites per plane) for quick color presetting  */
/* of the display areas, and additional logic allows the processor to write data  */
/* to the display on non-byte boundaries.                                         */
/* ------------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Display, August 2nd, 1984: Sequencer Registers:     */
/* ------------------------------------------------------------------------------------ */
/* Memory Mode Register:                                                                */
/* This is a write-only register pointed to when the value in the address register is   */
/* hex 04. The processor output port address for this register is 3C5.                  */
/* Memory Mode Register Format:                                                         */
/*  - 0: Alpha                                                                          */
/*  - 1: Extended Memory                                                                */
/*  - 2: Odd/Even                                                                       */
/*  - 3: Not Used                                                                       */
/*  - 4: Not Used                                                                       */
/*  - 5: Not Used                                                                       */
/*  - 6: Not Used                                                                       */
/*  - 7: Not Used                                                                       */
/* Bit 2: Odd/Even--A logical 0 directs even processor addresses to access maps 0 and 2,*/
/* while odd processor addresses access maps 1 and 3. A logical 1 causes processor      */
/* addresses to sequentially access data within a bit map. The maps are accessed        */
/* according to the value in the map mask register.                                     */
/* ------------------------------------------------------------------------------------ */
	if ((ega->reg_seq[EGA_SEQ_MODE] & EGA_SEQ_MODE_OE) == 0) { /* Sequencer Registers: Memory Mode Odd/Even */
		addr &= 0xfffe;
	}
/* ------------------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Graphics Controller Registers: */
/* ------------------------------------------------------------------------------------------ */
/* Name:                    Port:  Index:                                                     */
/*  - Read Map Select       : - 3CF: - 04:                                                    */
/*  - Mode Register         : - 3CF: - 05:                                                    */
/* Read Map Select Register:                                                                  */
/* This is a write-only register pointed to by the value in the Graphics 1 and 2 address      */
/* register. This value must be hex 04 before writing can take place. The processor output    */
/* port address for this register is hex 3CF.                                                 */
/* Read Map Select Register Format:                                                           */
/*  - Bit 0: Map Select 0                                                                     */
/*  - Bit 1: Map Select 1                                                                     */
/*  - Bit 2: Map Select 2                                                                     */
/*  - Bit 3: Not Used                                                                         */
/*  - Bit 4: Not Used                                                                         */
/*  - Bit 5: Not Used                                                                         */
/*  - Bit 6: Not Used                                                                         */
/*  - Bit 7: Not Used                                                                         */
/* Bit 0-Bit 2: Map Select--These bits represent a binary encoded value of the memory plane   */
/* number from which the processor reads data. This register has no effect on the color       */
/* compare read mode described elsewhere in this section.                                     */
/*                                                                                            */
/* Mode Register:                                                                             */
/* This is a write-only register pointed to by the value in the Graphics 1 and 2 address      */
/* register. This value must be hex 05 before writing can take place. The processor output    */
/* port address for this register is 3CF.                                                     */
/* Mode Register Format:                                                                      */
/*  - Bit 0: Write Mode                                                                       */
/*  - Bit 1: Write Mode                                                                       */
/*  - Bit 2: Test Condition                                                                   */
/*  - Bit 3: Read  Mode                                                                       */
/*  - Bit 4: Odd/Even                                                                         */
/*  - Bit 5: Test Register Mode                                                               */
/*  - Bit 6: Not Used                                                                         */
/*  - Bit 7: Not Used                                                                         */
/* Bit-Bit 1: Write Mode                                                                      */
/* Bits 1 0:                                                                                  */
/*  -   0 0: Each memory plane is written with the processor data rotated by the number of    */
/*  counts in the rotate register, unless Set/Reset is enabled for the plane. Planes for which*/
/*  Set/Reset is enabled are written with 8 bits of the value contained in the Set/Reset      */
/*  register for that plane.                                                                  */
/*  -   0 1: Each memory plane is written with the contents of the processor latches. These   */
/*  latches are loaded by a processor read operation.                                         */
/*  -   1 0: Memory plane n (0 through 3) is filled with 8 bits of the value of data bit n.   */
/*  -   1 1: Not Valid                                                                        */
/* The logic function specified by the function select register also applies.                 */
/*                                                                                            */
/* Bit 2: Test Condition--A logical 1 directs graphics controller outputs to be placed in high*/
/* impedance state for testing.                                                               */
/*                                                                                            */
/* Bit 3: Read Mode--When this bit is a logical 0, the processor reads data from the memory   */
/* plane selected by the read map select register. When this bit is a logical 1, the processor*/
/* reads the results of the comparison of the 4 memory planes and the color compare register. */
/*                                                                                            */
/* Bit 4: Odd/Even--A logical 1 selects the odd/even addressing mode, which is useful for     */
/* emulation of the Color Graphics Monitor Adapter compatible modes. Normally the value here  */
/* follows the value of the Memory Mode Register bit 3 of the Sequencer.                      */
/*                                                                                            */
/* Bit 5: Shift Register--A logical 1 directs the shift registers on each graphics chip to    */
/* format the serial data stream with even numbered bits on the even numbered maps and odd    */
/* numbered bits on the odd maps.                                                             */
/* ------------------------------------------------------------------------------------------ */
	rdmode = (ega->reg_grc[EGA_GRC_MODE] >> 3) & 1; /* Mode Register */
/* ------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Major Components:  */
/* ------------------------------------------------------------------------------ */
/* Support Logic:                                                                 */
/* The logic on the card surrounding the LIS modules supports the modules and     */
/* creates latch buses for the CRT controller, the processor, and character       */
/* generator. Two clock sources (14 MHz and 16 MHz) provide the dot rate. The     */
/* clock is multiplexed under processor I/O control. Four I/O registers also      */
/* resident on the card are not part of the LSI devices.                          */
/* ------------------------------------------------------------------------------ */
	ega->latch[0] = ega->mem[addr + 0x00000];
	ega->latch[1] = ega->mem[addr + 0x10000];
	ega->latch[2] = ega->mem[addr + 0x20000];
	ega->latch[3] = ega->mem[addr + 0x30000];
	if (rdmode == 0) {
		unsigned map;
		map = ega->reg_grc[EGA_GRC_READMAP] & 3; /* Read Map Select Register */
		if ((ega->reg_seq[EGA_SEQ_MODE] & EGA_SEQ_MODE_OE) == 0) { /* Sequencer Registers: Memory Mode Odd/Even */
			map = (map & 0x02) + a0;
		}
		return (ega->latch[map]);
	}
	else {
		unsigned char msk, val, c1, c2;
		val = 0x00;
		msk = 0x80;
		while (msk != 0) {
			c1 = (ega->latch[0] & msk) ? 0x01 : 0x00;
			c1 |= (ega->latch[1] & msk) ? 0x02 : 0x00;
			c1 |= (ega->latch[2] & msk) ? 0x04 : 0x00;
			c1 |= (ega->latch[3] & msk) ? 0x08 : 0x00;
/* ------------------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Graphics Controller Registers: */
/* ------------------------------------------------------------------------------------------ */
/* Name:                    Port:  Index:                                                     */
/*  - Color Compare         : - 3CF: - 02:                                                    */
/*  - Color Don't Care      : - 3CF: - 07:                                                    */
/* Color Compare    Register:                                                                 */
/* This is a write-only register pointed to by the value in the Graphics 1 and 2 address      */
/* register. This value must be hex 02 before writing can take place. The processor output    */
/* port address for this register is hex 3CF.                                                 */
/* Enable Set/Reset Register Format:                                                          */
/*  - Bit 0: Enable Set/Reset Bit 0                                                           */
/*  - Bit 1: Enable Set/Reset Bit 1                                                           */
/*  - Bit 2: Enable Set/Reset Bit 2                                                           */
/*  - Bit 3: Enable Set/Reset Bit 3                                                           */
/*  - Bit 4: Not Used                                                                         */
/*  - Bit 5: Not Used                                                                         */
/*  - Bit 6: Not Used                                                                         */
/*  - Bit 7: Not Used                                                                         */
/* Bit 0-Bit 3: Color Compare--These bits represent a 4 bit color value to be compared. If the*/
/* processor sets read mode 1 on the graphics chips, and does a memory read, the data returned*/
/* from the memory cycle will be a 1 in each bit position where the 4 bit planes equal the    */
/* color compare register.                                                                    */
/*                                                                                            */
/* Color Don't Care Register:                                                                 */
/* This is a write-only register and is pointed to by the value in the Graphics 1 and 2       */
/* address register. This value must be hex 07 before writing can take place. The processor   */
/* output port for this register is hex 3CF.                                                  */
/* Color Don't Care Register Format:                                                          */
/*  - Bit 0: Color Plane 0=Don't Care                                                         */
/*  - Bit 1: Color Plane 1=Don't Care                                                         */
/*  - Bit 2: Color Plane 2=Don't Care                                                         */
/*  - Bit 3: Color Plane 3=Don't Care                                                         */
/*  - Bit 4: Not Used                                                                         */
/*  - Bit 5: Not Used                                                                         */
/*  - Bit 6: Not Used                                                                         */
/*  - Bit 7: Not Used                                                                         */
/* Bit 0: Color Don't Care--Compare plane 0=don't care when reading color compare when this   */
/* bit is set to 1.                                                                           */
/*                                                                                            */
/* Bit 1: Color Don't Care--Compare plane 1=don't care when reading color compare when this   */
/* bit is set to 1.                                                                           */
/*                                                                                            */
/* Bit 2: Color Don't Care--Compare plane 2=don't care when reading color compare when this   */
/* bit is set to 1.                                                                           */
/*                                                                                            */
/* Bit 3: Color Don't Care--Compare plane 3=don't care when reading color compare when this   */
/* bit is set to 1.                                                                           */
/* ------------------------------------------------------------------------------------------ */
			c1 &= ega->reg_grc[EGA_GRC_CDC];   /* Color Don't Care Register */

			c2 = ega->reg_grc[EGA_GRC_COLCMP]; /* Color Compare    Register */
			c2 &= ega->reg_grc[EGA_GRC_CDC];   /* Color Don't Care Register */
			if (c1 == c2) {
				val |= msk;
			}
			msk >>= 1;
		}
		return (val);
	}
	return (0);
}

static
unsigned short ega_mem_get_uint16 (ega_t *ega, unsigned long addr)
{
	unsigned short val;
/* ----------------------------------------------------------------------------------- */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Registers:              */
/* ----------------------------------------------------------------------------------- */
/* External Registers:                                                                 */
/* This section contains descriptions of the registers of the Enhanced Graphics Adapter*/
/* that are not contained in an LSI device.                                            */
/* Name:                              Port:  Index:                                    */
/*  - Miscellaneous Output Register  : - 3C2: - -:                                     */
/*  - Feature Control      Register  : - 3?A: - -:                                     */
/*  - Input Status         Register 0: - 3C2: - -:                                     */
/*  - Input Status         Register 1: - 3?2: - -:                                     */
/* ? = B in Monochrome Modes    ? = D in Color Modes                                   */
/* Miscellaneous Output Register:                                                      */
/* This is a write-only register. The processor output port address is hex 3C2. A      */
/* hardware reset causes all bits to reset to zero.                                    */
/* Miscellaneous Output Register Format:                                               */
/*  - Bit 0: I/O Address Select                                                        */
/*  - Bit 1: Enable Ram                                                                */
/*  - Bit 2: Clock Select 0                                                            */
/*  - Bit 3: Clock Select 1                                                            */
/*  - Bit 4: Disable Internal Video Drivers                                            */
/*  - Bit 5: Page Bit For Odd/Even                                                     */
/*  - Bit 6: Horizontal Retrace Polarity                                               */
/*  - Bit 7: Vertical   Retrace Polarity                                               */
/* Bit 1: Enable RAM--A logical 0 disables RAM from the processor; a logical 1 enables */
/* RAM to respond at addresses designated by the Control Data Select value programmed  */
/* into the Graphics Controllers.                                                      */
/* ----------------------------------------------------------------------------------- */
	if ((ega->reg[EGA_MOUT] & EGA_MOUT_ERAM) == 0) { /* Miscellaneous Output Register: Enable RAM */
		return (0);
	}
	val = ega_mem_get_uint8 (ega, addr);
	val |= ega_mem_get_uint8 (ega, addr + 1) << 8;
	return (val);
}

static
void ega_mem_set_uint8 (ega_t *ega, unsigned long addr, unsigned char val)
{
	unsigned      wrmode;
	unsigned      rot;
	unsigned char mapmsk, bitmsk;
	unsigned char esr, set;
	unsigned char col[4];
/* ------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Major Components:  */
/* ------------------------------------------------------------------------------ */
/* Display Buffer:                                                                */
/* The display buffer on the adapter consists of 64K bytes of dynamic read/write  */
/* memory configured as four 16K byte video bit planes. Two options are available */
/* for expanding the graphics memory. The Graphics Memory Expansion Card plugs    */
/* into the memory expansion connector on the adapter, and adds one bank of 16K   */
/* to each of the four bit planes, increasing the graphics memory to 128K bytes.  */
/* The expansion card also provides DIP sockets for further memory expansion.     */
/* Populating the DIP sockets with the Graphics Memory Module Kit adds two        */
/* additional 16K banks to each bit plane, bringing the graphics memory to its    */
/* maximum of 256K bytes.                                                         */
/*                                                                                */
/* The address of the display buffer can be changed to remain compatible with     */
/* other video cards and application software. Four locations are provided. The   */
/* buffer can be configured at segment address hex A0000 for a length of 128K     */
/* bytes, at hex A0000 for a length of 64K bytes, at hex B0000 for a length of 32K*/
/* bytes, or at hex B8000 for a length of 32K bytes.                              */
/* ------------------------------------------------------------------------------ */
/* ----------------------------------------------------------------------------------- */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Registers:              */
/* ----------------------------------------------------------------------------------- */
/* External Registers:                                                                 */
/* This section contains descriptions of the registers of the Enhanced Graphics Adapter*/
/* that are not contained in an LSI device.                                            */
/* Name:                              Port:  Index:                                    */
/*  - Miscellaneous Output Register  : - 3C2: - -:                                     */
/*  - Feature Control      Register  : - 3?A: - -:                                     */
/*  - Input Status         Register 0: - 3C2: - -:                                     */
/*  - Input Status         Register 1: - 3?2: - -:                                     */
/* ? = B in Monochrome Modes    ? = D in Color Modes                                   */
/* Miscellaneous Output Register:                                                      */
/* This is a write-only register. The processor output port address is hex 3C2. A      */
/* hardware reset causes all bits to reset to zero.                                    */
/* Miscellaneous Output Register Format:                                               */
/*  - Bit 0: I/O Address Select                                                        */
/*  - Bit 1: Enable Ram                                                                */
/*  - Bit 2: Clock Select 0                                                            */
/*  - Bit 3: Clock Select 1                                                            */
/*  - Bit 4: Disable Internal Video Drivers                                            */
/*  - Bit 5: Page Bit For Odd/Even                                                     */
/*  - Bit 6: Horizontal Retrace Polarity                                               */
/*  - Bit 7: Vertical   Retrace Polarity                                               */
/* Bit 1: Enable RAM--A logical 0 disables RAM from the processor; a logical 1 enables */
/* RAM to respond at addresses designated by the Control Data Select value programmed  */
/* into the Graphics Controllers.                                                      */
/* ----------------------------------------------------------------------------------- */
	if ((ega->reg[EGA_MOUT] & EGA_MOUT_ERAM) == 0) { /* Miscellaneous Output Register: Enable RAM */
		return;
	}
/* ------------------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Graphics Controller Registers: */
/* ------------------------------------------------------------------------------------------ */
/* Name:                    Port:  Index:                                                     */
/*  - Miscellaneous         : - 3CF: - 06:                                                    */
/* Miscellaneous Register:                                                                    */
/* This is a write-only register pointed to by the value in the Graphics 1 and 2 address      */
/* register. This value must be hex 06 before writing can take place. The processor output    */
/* port for this register is hex 3CF.                                                         */
/* Miscellaneous Register Format:                                                             */
/*  - Bit 0: Graphics Mode                                                                    */
/*  - Bit 1: Chain Odd Maps to Even                                                           */
/*  - Bit 2: Memory Map 0                                                                     */
/*  - Bit 3: Memory Map 1                                                                     */
/*  - Bit 4: Not Used                                                                         */
/*  - Bit 5: Not Used                                                                         */
/*  - Bit 6: Not Used                                                                         */
/*  - Bit 7: Not Used                                                                         */
/* Bit 0: Graphics Mode--This bit controls alpha-mode addressing. A logical 1 selects graphics*/
/* mode. When set to graphics mode, the character generator address latches are disabled.     */
/*                                                                                            */
/* Bit 1: Chain Odd Maps To Even Maps--When set to 1, this bit directs the processor address  */
/* bit 0 to be replaced by a higher order bit and odd/even maps to be selected with odd/even  */
/* values of the processor A0 bit, respectively.                                              */
/*                                                                                            */
/* Bit 2-Bit 3: Memory Map--These bits control the mapping of the regenerative buffer into the*/
/* processor address space.                                                                   */
/* Bits 3 2:                                                                                  */
/*  -   0 0: Hex A000 for 128K bytes.                                                         */
/*  -   0 1: Hex A000 for  64K bytes.                                                         */
/*  -   1 0: Hex B000 for  32K bytes.                                                         */
/*  -   1 1: Hex B800 for  32K bytes.                                                         */
/* If the display adapter is mapped at address hex A000 for 128K bytes, no other adapter can  */
/* be installed in the system.                                                                */
/* ------------------------------------------------------------------------------------------ */
	switch ((ega->reg_grc[EGA_GRC_MISC] >> 2) & 3) { /* Miscellaneous Register */
	case 0: /* 128K at A000 */
		break;

	case 1: /* 64K at A000 */
		if (addr > 0xffff) {
			return;
		}
		break;

	case 2: /* 32K at B000 */
		if ((addr < 0x10000) || (addr > 0x17fff)) {
			return;
		}
		addr -= 0x10000;
		break;

	case 3: /* 32K at B800 */
		if ((addr < 0x18000) || (addr > 0x1ffff)) {
			return;
		}
		addr -= 0x18000;
		break;
	}
	addr &= 0xffff;
/* ------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Major Components:  */
/* ------------------------------------------------------------------------------ */
/* Graphics Controller:                                                           */
/* The Graphics Controller directs the data from the memory to the attribute      */
/* controller and the processor. In graphics modes, memory data is sent in        */
/* serialized form to the attribute chip. In alpha modes the memory data is sent  */
/* in parallel form, bypassing the graphics controller. The graphics controller   */
/* formats the data for compatible modes and provides color comparators for use in*/
/* color painting modes. Other hardware facilities allow the processor to write 32*/
/* bits in a single memory cycle, (8 bites per plane) for quick color presetting  */
/* of the display areas, and additional logic allows the processor to write data  */
/* to the display on non-byte boundaries.                                         */
/* ------------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Graphics Controller Registers: */
/* ------------------------------------------------------------------------------------------ */
/* Name:                    Port:  Index:                                                     */
/*  - Mode Register         : - 3CF: - 05:                                                    */
/* Mode Register:                                                                             */
/* This is a write-only register pointed to by the value in the Graphics 1 and 2 address      */
/* register. This value must be hex 05 before writing can take place. The processor output    */
/* port address for this register is 3CF.                                                     */
/* Mode Register Format:                                                                      */
/*  - Bit 0: Write Mode                                                                       */
/*  - Bit 1: Write Mode                                                                       */
/*  - Bit 2: Test Condition                                                                   */
/*  - Bit 3: Read  Mode                                                                       */
/*  - Bit 4: Odd/Even                                                                         */
/*  - Bit 5: Test Register Mode                                                               */
/*  - Bit 6: Not Used                                                                         */
/*  - Bit 7: Not Used                                                                         */
/* Bit-Bit 1: Write Mode                                                                      */
/* Bits 1 0:                                                                                  */
/*  -   0 0: Each memory plane is written with the processor data rotated by the number of    */
/*  counts in the rotate register, unless Set/Reset is enabled for the plane. Planes for which*/
/*  Set/Reset is enabled are written with 8 bits of the value contained in the Set/Reset      */
/*  register for that plane.                                                                  */
/*  -   0 1: Each memory plane is written with the contents of the processor latches. These   */
/*  latches are loaded by a processor read operation.                                         */
/*  -   1 0: Memory plane n (0 through 3) is filled with 8 bits of the value of data bit n.   */
/*  -   1 1: Not Valid                                                                        */
/* The logic function specified by the function select register also applies.                 */
/*                                                                                            */
/* Bit 2: Test Condition--A logical 1 directs graphics controller outputs to be placed in high*/
/* impedance state for testing.                                                               */
/*                                                                                            */
/* Bit 3: Read Mode--When this bit is a logical 0, the processor reads data from the memory   */
/* plane selected by the read map select register. When this bit is a logical 1, the processor*/
/* reads the results of the comparison of the 4 memory planes and the color compare register. */
/*                                                                                            */
/* Bit 4: Odd/Even--A logical 1 selects the odd/even addressing mode, which is useful for     */
/* emulation of the Color Graphics Monitor Adapter compatible modes. Normally the value here  */
/* follows the value of the Memory Mode Register bit 3 of the Sequencer.                      */
/*                                                                                            */
/* Bit 5: Shift Register--A logical 1 directs the shift registers on each graphics chip to    */
/* format the serial data stream with even numbered bits on the even numbered maps and odd    */
/* numbered bits on the odd maps.                                                             */
/* ------------------------------------------------------------------------------------------ */
	wrmode = ega->reg_grc[EGA_GRC_MODE] & 3; /* Mode Register */
/* ------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Major Components:  */
/* ------------------------------------------------------------------------------ */
/* Sequencer:                                                                     */
/* The Sequencer generates basic memory timings for the dynamic RAMs and the      */
/* character clock for controlling regenerative memory fetches. It allows the     */
/* processor to access memory during active display intervals by inserting        */
/* dedicated processor memory cycles periodically between the display memory      */
/* cycles. Map mask registers are available to protect entire memory maps from    */
/* being changed.                                                                 */
/* ------------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Display, August 2nd, 1984: Sequencer Registers:     */
/* ------------------------------------------------------------------------------------ */
/* Map Mask Register:                                                                   */
/* This is a write-only register pointed to when the value in the address register is   */
/* hex 02. The output port address for this register is hex 3C5.                        */
/*  - Bit 0: 1 Enables Map 0                                                            */
/*  - Bit 1: 1 Enables Map 1                                                            */
/*  - Bit 2: 1 Enables Map 2                                                            */
/*  - Bit 3: 1 Enables Map 3                                                            */
/*  - Bit 4: Not Used                                                                   */
/*  - Bit 5: Not Used                                                                   */
/*  - Bit 6: Not Used                                                                   */
/*  - Bit 7: Not Used                                                                   */
/* Bit 0-Bit 3: Map Mask--A logical 1 in bits 3 through 0 enables the processor to write*/
/* to the corresponding maps 3 through 0. If this register is programmed with a value of*/
/* 0FH, the CPU can perform a 32-bit write operation with only one memory cycle. This   */
/* substantially reduces the overhead on the CPU during display update cycles in        */
/* graphics modes. Data scrolling operations are also enhanced by setting this register */
/* to a value of 0FH and writing the display buffer address with the data stored in the */
/* CPU data latches. This is a read-modify-write operation. When odd/even modes are     */
/* selected, maps 0 and 1 and 2 and 3 should have the same map mask value.              */
/*                                                                                      */
/* Memory Mode Register:                                                                */
/* This is a write-only register pointed to when the value in the address register is   */
/* hex 04. The processor output port address for this register is 3C5.                  */
/* Memory Mode Register Format:                                                         */
/*  - 0: Alpha                                                                          */
/*  - 1: Extended Memory                                                                */
/*  - 2: Odd/Even                                                                       */
/*  - 3: Not Used                                                                       */
/*  - 4: Not Used                                                                       */
/*  - 5: Not Used                                                                       */
/*  - 6: Not Used                                                                       */
/*  - 7: Not Used                                                                       */
/* Bit 0: Alpha--A logical 0 indicates that a non-alpha mode is active. A logical 1     */
/* indicates that alpha mode is active and enables the character generator map select   */
/* function.                                                                            */
/*                                                                                      */
/* Bit 1: Extended Memory--A logical 0 indicates that the memory expansion card is not  */
/* installed. A logical 1 indicates that the memory expansion card is installed and     */
/* enables access to the extended memory through address bits 14 and 15.                */
/*                                                                                      */
/* Bit 2: Odd/Even--A logical 0 directs even processor addresses to access maps 0 and 2,*/
/* while odd processor addresses access maps 1 and 3. A logical 1 causes processor      */
/* addresses to sequentially access data within a bit map. The maps are accessed        */
/* according to the value in the map mask register.                                     */
/* ------------------------------------------------------------------------------------ */
	mapmsk = ega->reg_seq[EGA_SEQ_MAPMASK]; /* Sequencer Registers: Map Mask Register */

	if ((ega->reg_seq[EGA_SEQ_MODE] & EGA_SEQ_MODE_OE) == 0) { /* Sequencer Registers: Memory Mode Odd/Even */
		mapmsk &= (addr & 1) ? 0x0a : 0x05;
		addr &= 0xfffe;
	}
	switch (wrmode) {
	case 0: /* write mode 0 */
/* ------------------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Graphics Controller Registers: */
/* ------------------------------------------------------------------------------------------ */
/* Name:                    Port:  Index:                                                     */
/*  -        Set/Reset      : - 3CF: - 00:                                                    */
/*  - Enable Set/Reset      : - 3CF: - 01:                                                    */
/*  - Data Rotate           : - 3CF: - 03:                                                    */
/* Set/Reset Register:                                                                        */
/* This is a write-only register pointed to by the value in the Graphics 1 and 2 address      */
/* register. This value must be hex 00 before writing can take place. The processor output    */
/* port address for this register is hex 3CF.                                                 */
/* Set/Reset Register Format:                                                                 */
/*  - Bit 0: Set/Reset Bit 0                                                                  */
/*  - Bit 1: Set/Reset Bit 1                                                                  */
/*  - Bit 2: Set/Reset Bit 2                                                                  */
/*  - Bit 3: Set/Reset Bit 3                                                                  */
/*  - Bit 4: Not Used                                                                         */
/*  - Bit 5: Not Used                                                                         */
/*  - Bit 6: Not Used                                                                         */
/*  - Bit 7: Not Used                                                                         */
/* Bit 0-Bit 3: Set/Reset--These bits represent the value written to the respective memory    */
/* planes when the processor does a memory write with write mode 0 selected and set/reset mode*/
/* is enabled. Set/Reset can be enabled on a plane by plane basis with separate OUT commands  */
/* to the Set/Reset register.                                                                 */
/*                                                                                            */
/* Enable Set/Reset Register:                                                                 */
/* This is a write-only register and is pointed to by the value in the Graphics 1 and 2       */
/* address register. This value must be hex 01 before writing can take place. The processor   */
/* output port for this register is hex 3CF.                                                  */
/* Color Compare    Register Format:                                                          */
/*  - Bit 0: Color Compare 0                                                                  */
/*  - Bit 1: Color Compare 1                                                                  */
/*  - Bit 2: Color Compare 2                                                                  */
/*  - Bit 3: Color Compare 3                                                                  */
/* Bit 0-Bit 3: Enable Set/Reset--These bits enable the set/reset function. The respective    */
/* memory plane is written with the value of the Set/Reset register provided the write value  */
/* is 0. When write mode is 0 and Set/Reset is not enabled on a plane, that plane is written  */
/* with the value of the processor data.                                                      */
/*                                                                                            */
/* Data Rotate Register:                                                                      */
/* This is a write-only register pointed to by the value in the Graphics 1 and 2 address      */
/* register. This value must be hex 03 before writing can take place. The processor output    */
/* port address for this register is hex 3CF.                                                 */
/* Data Rotate Register Format:                                                               */
/*  - Bit 0: Rotate Count                                                                     */
/*  - Bit 1: Rotate Count 1                                                                   */
/*  - Bit 2: Rotate Count 2                                                                   */
/*  - Bit 3: Function Select                                                                  */
/*  - Bit 4: Function Select                                                                  */
/*  - Bit 5: Not Used                                                                         */
/*  - Bit 6: Not Used                                                                         */
/*  - Bit 7: Not Used                                                                         */
/* Bit 0-Bit 2: Rotate Count--These bits represent a binary encoded value of the number of    */
/* positions to rotate the processor data bus during processor memory writes. This operation  */
/* is done when the write mode is 0. To write unrotated data the processor must select a count*/
/* of 0.                                                                                      */
/*                                                                                            */
/* Bit 3-Bit 4: Function Select--Data written to memory can operate logically with data       */
/* already in the processor latches. The bit functions are defined in the following table.    */
/* Bits 4 3:                                                                                  */
/*  -   0 0: Data unmodified                                                                  */
/*  -   0 1: Data AND'ed with latched data.                                                   */
/*  -   1 0: Data  OR'ed with latched data.                                                   */
/*  -   1 1: Data XOR'ed with latched data.                                                   */
/* Data may be any of the choices selected by the Write Mode Register except processor        */
/* latches. If rotated data is selected, the rotate applies before the logical function.      */
/* ------------------------------------------------------------------------------------------ */
		rot = ega->reg_grc[EGA_GRC_ROTATE] & 7; /* Data Rotate Register */
		val = ((val >> rot) | (val << (8 - rot))) & 0xff;

		esr = ega->reg_grc[EGA_GRC_ENABLESR];   /* Enable Set/Reset Register */
		set = ega->reg_grc[EGA_GRC_SETRESET];   /*        Set/Reset Register */

		col[0] = (esr & 0x01) ? ((set & 0x01) ? 0xff : 0x00) : val;
		col[1] = (esr & 0x02) ? ((set & 0x02) ? 0xff : 0x00) : val;
		col[2] = (esr & 0x04) ? ((set & 0x04) ? 0xff : 0x00) : val;
		col[3] = (esr & 0x08) ? ((set & 0x08) ? 0xff : 0x00) : val;
		break;
/* ------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Major Components:  */
/* ------------------------------------------------------------------------------ */
/* Support Logic:                                                                 */
/* The logic on the card surrounding the LIS modules supports the modules and     */
/* creates latch buses for the CRT controller, the processor, and character       */
/* generator. Two clock sources (14 MHz and 16 MHz) provide the dot rate. The     */
/* clock is multiplexed under processor I/O control. Four I/O registers also      */
/* resident on the card are not part of the LSI devices.                          */
/* ------------------------------------------------------------------------------ */
	case 1: /* write mode 1 */
		col[0] = ega->latch[0];
		col[1] = ega->latch[1];
		col[2] = ega->latch[2];
		col[3] = ega->latch[3];
		break;

	case 2: /* write mode 2 */
		col[0] = (val & 0x01) ? 0xff : 0x00;
		col[1] = (val & 0x02) ? 0xff : 0x00;
		col[2] = (val & 0x04) ? 0xff : 0x00;
		col[3] = (val & 0x08) ? 0xff : 0x00;
		break;

	default:
		return;
	}
	if (wrmode != 1) {
		switch ((ega->reg_grc[EGA_GRC_ROTATE] >> 3) & 3) { /* Data Rotate Register */
		case 0: /* copy */
			break;

		case 1: /* and */
			col[0] &= ega->latch[0];
			col[1] &= ega->latch[1];
			col[2] &= ega->latch[2];
			col[3] &= ega->latch[3];
			break;

		case 2: /* or */
			col[0] |= ega->latch[0];
			col[1] |= ega->latch[1];
			col[2] |= ega->latch[2];
			col[3] |= ega->latch[3];
			break;

		case 3: /* xor */
			col[0] ^= ega->latch[0];
			col[1] ^= ega->latch[1];
			col[2] ^= ega->latch[2];
			col[3] ^= ega->latch[3];
			break;
		}
	}
/* ------------------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Graphics Controller Registers: */
/* ------------------------------------------------------------------------------------------ */
/* Name:                    Port:  Index:                                                     */
/*  - Bit Mask              : - 3CF: - 08:                                                    */
/* Bit Mask  Register:                                                                        */
/* This is a write-only register and is pointed to by the value in the Graphics 1 and 2       */
/* address register. This value must be hex 08 before writing can take place. The processor   */
/* output port for this register is hex 3CF.                                                  */
/* Bit Mask Register Format:                                                                  */
/*  - Bit 0: 0-immune to change    1-Unimpeded Writes                                         */
/*  - Bit 1: 0-immune to change    1-Unimpeded Writes                                         */
/*  - Bit 2: 0-immune to change    1-Unimpeded Writes                                         */
/*  - Bit 3: 0-immune to change    1-Unimpeded Writes                                         */
/*  - Bit 4: 0-immune to change    1-Unimpeded Writes                                         */
/*  - Bit 5: 0-immune to change    1-Unimpeded Writes                                         */
/*  - Bit 6: 0-immune to change    1-Unimpeded Writes                                         */
/*  - Bit 7: 0-immune to change    1-Unimpeded Writes                                         */
/* Bit 0-Bit 7: Bit Mask--Any bit programmed to n causes the corresponding bit nin each bit   */
/* plane to be immune to change provided that the location being written was the last location*/
/* read by the processor. Bits programmed to a 1 allow unimpeded writes to the corresponding  */
/* bits in the bit planes.                                                                    */
/*                                                                                            */
/* The bit mask applies to any data written by the processor (rotate, AND'ed, OR'ed, XOR'ed,  */
/* DX, and S/R). To preserve bits using the bit mask, data must be latched internally by      */
/* reading the location. When data is written to preserve the bits, the most current data in  */
/* latches is written in those positions. The bit mask applies to all bit planes              */
/* simultaneously.                                                                            */
/* ------------------------------------------------------------------------------------------ */
	bitmsk = ega->reg_grc[EGA_GRC_BITMASK]; /* Bit Mask Register */

	col[0] = (col[0] & bitmsk) | (ega->latch[0] & ~bitmsk);
	col[1] = (col[1] & bitmsk) | (ega->latch[1] & ~bitmsk);
	col[2] = (col[2] & bitmsk) | (ega->latch[2] & ~bitmsk);
	col[3] = (col[3] & bitmsk) | (ega->latch[3] & ~bitmsk);
	if (mapmsk & 0x01) {
		ega->mem[addr + 0x00000] = col[0];
	}
	if (mapmsk & 0x02) {
		ega->mem[addr + 0x10000] = col[1];
	}
	if (mapmsk & 0x04) {
		ega->mem[addr + 0x20000] = col[2];
	}
	if (mapmsk & 0x08) {
		ega->mem[addr + 0x30000] = col[3];
	}
	ega->update_state |= EGA_UPDATE_DIRTY;
}

static
void ega_mem_set_uint16 (ega_t *ega, unsigned long addr, unsigned short val)
{
/* ----------------------------------------------------------------------------------- */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Registers:              */
/* ----------------------------------------------------------------------------------- */
/* External Registers:                                                                 */
/* This section contains descriptions of the registers of the Enhanced Graphics Adapter*/
/* that are not contained in an LSI device.                                            */
/* Name:                              Port:  Index:                                    */
/*  - Miscellaneous Output Register  : - 3C2: - -:                                     */
/*  - Feature Control      Register  : - 3?A: - -:                                     */
/*  - Input Status         Register 0: - 3C2: - -:                                     */
/*  - Input Status         Register 1: - 3?2: - -:                                     */
/* ? = B in Monochrome Modes    ? = D in Color Modes                                   */
/* Miscellaneous Output Register:                                                      */
/* This is a write-only register. The processor output port address is hex 3C2. A      */
/* hardware reset causes all bits to reset to zero.                                    */
/* Miscellaneous Output Register Format:                                               */
/*  - Bit 0: I/O Address Select                                                        */
/*  - Bit 1: Enable Ram                                                                */
/*  - Bit 2: Clock Select 0                                                            */
/*  - Bit 3: Clock Select 1                                                            */
/*  - Bit 4: Disable Internal Video Drivers                                            */
/*  - Bit 5: Page Bit For Odd/Even                                                     */
/*  - Bit 6: Horizontal Retrace Polarity                                               */
/*  - Bit 7: Vertical   Retrace Polarity                                               */
/* Bit 1: Enable RAM--A logical 0 disables RAM from the processor; a logical 1 enables */
/* RAM to respond at addresses designated by the Control Data Select value programmed  */
/* into the Graphics Controllers.                                                      */
/* ----------------------------------------------------------------------------------- */
	if ((ega->reg[EGA_MOUT] & EGA_MOUT_ERAM) == 0) { /* Miscellaneous Output Register: Enable RAM */
		return;
	}
	ega_mem_set_uint8 (ega, addr, val & 0xff);
	ega_mem_set_uint8 (ega, addr + 1, (val >> 8) & 0xff);
}


/*
 * Set an attribute controller register
 */
static
void ega_atc_set_reg (ega_t *ega, unsigned reg, unsigned char val)
{
/* ------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Major Components:  */
/* ------------------------------------------------------------------------------ */
/* Attribute Controller:                                                          */
/* The Attribute Controller provides a color palette of 16 colors, each of which  */
/* may be specified separately. Six color outputs are available for driving a     */
/* display. Blinking and underlining are controlled by this chip. This chip takes */
/* data from the display memory and formats it for display on the CRT screen.     */
/* ------------------------------------------------------------------------------ */
	if (reg > 21) {
		return;
	}
	if (ega->reg_atc[reg] == val) {
		return;
	}
	ega->reg_atc[reg] = val;
	ega->update_state |= EGA_UPDATE_DIRTY;
}


/*
 * Set a sequencer register
 */
static
void ega_seq_set_reg (ega_t *ega, unsigned reg, unsigned char val)
{
/* ------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Major Components:  */
/* ------------------------------------------------------------------------------ */
/* Sequencer:                                                                     */
/* The Sequencer generates basic memory timings for the dynamic RAMs and the      */
/* character clock for controlling regenerative memory fetches. It allows the     */
/* processor to access memory during active display intervals by inserting        */
/* dedicated processor memory cycles periodically between the display memory      */
/* cycles. Map mask registers are available to protect entire memory maps from    */
/* being changed.                                                                 */
/* ------------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Display, August 2nd, 1984: Sequencer Registers:     */
/* ------------------------------------------------------------------------------------ */
/* Name:                   Port:  Index:                                                */
/*  - Address:              - 3C4: -  -:                                                */
/*  - Reset:                - 3C5: - 00:                                                */
/*  - Clocking Mode:        - 3C5: - 01:                                                */
/*  - Map Mask:             - 3C5: - 02:                                                */
/*  - Character Map Select: - 3C5: - 03:                                                */
/*  - Memory Mode:          - 3C5: - 04:                                                */
/* Reset Register:                                                                      */
/* This is a write-only register pointed to when the value in the address register is   */
/* hex 00. The output port address for this register is hex 3C5.                        */
/* Reset Register Format:                                                               */
/*  - Bit 0:Asynchronous Reset                                                          */
/*  - Bit 1: Synchronous Reset                                                          */
/*  - Bit 2: Not Used                                                                   */
/*  - Bit 3: Not Used                                                                   */
/*  - Bit 4: Not Used                                                                   */
/*  - Bit 5: Not Used                                                                   */
/*  - Bit 6: Not Used                                                                   */
/*  - Bit 7: Not Used                                                                   */
/* Bit 0:Asynchronous Reset--A logical 0 commands the sequencer to asynchronous clear   */
/* and halt. All outputs are placed in the high impedance state when this bit is a 0. A */
/* logical 1 commands the sequencer to run unless bit 1 is set to zero. Resetting the   */
/* sequencer with this bit can cause data loss in the dynamic RAMs.                     */
/*                                                                                      */
/* Bit 1: Synchronous Reset--A logical 0 commands the sequencer to synchronous clear and*/
/* halt. Bits 1 and 0 must both be ones to allow the sequencer to operate. Reset the    */
/* sequencer with this bit before changing the Clocking Mode Register, if memory        */
/* contents are to be preserved.                                                        */
/*                                                                                      */
/* Clocking Mode Register:                                                              */
/* This is write-only register pointed to when the value in the address register is hex */
/* 01. The output port address for this register is hex 3C5.                            */
/* Clocking Mode Register Format:                                                       */
/*  - Bit 0: 8/9 Dot Clocks                                                             */
/*  - Bit 1: Bandwidth                                                                  */
/*  - Bit 2: Shift Load                                                                 */
/*  - Bit 3: Dot Clock                                                                  */
/*  - Bit 4: Not Used                                                                   */
/*  - Bit 5: Not Used                                                                   */
/*  - Bit 6: Not Used                                                                   */
/*  - Bit 7: Not Used                                                                   */
/* Bit 0: 8/9 Dot Clocks--A logical 0 directs the sequencer to generate character clocks*/
/* 9 dots wide; a logical 1 directs the sequencer to generate character clocks 8 dots   */
/* wide. Monochrome alphanumeric mode (07H) is the only mode that uses character clocks */
/* 9 dots wide. All other modes must use 8 dots per character clock.                    */
/*                                                                                      */
/* Bit 1: Bandwidth--A logical 0 makes CRT memory cycles occur on 4 out of 5 available  */
/* memory cycles; a logical 1 makes CRT memory cycles occur on 2 out of 5 available     */
/* memory cycles. Medium resolution modes require less data to be fetched from the      */
/* display buffer during the horizontal scan time. This allows the CPU greater access   */
/* time to the display buffer. All high resolution modes must provide the CRTC with 4   */
/* out of 5 memory cycles in order to refresh the display image.                        */
/*                                                                                      */
/* Bit 2: Shift Load--When set to 0, the video serializers are reloaded every character */
/* clock; when set to 1, the video serializers are loaded every other character clock.  */
/* This mode is useful when 16 bits are fetched per cycle and chained together in the   */
/* shift registers.                                                                     */
/*                                                                                      */
/* Bit 3: Dot Clock--A logical 0 selects normal dot clocks derived from the sequencer   */
/* master clock input. When this bit is set to 1, the master clock will be divided by 2 */
/* to generate the dot clock. All the other timings will be stretched since they are    */
/* derived from the dot clock. Dot clock divided by two is used for 320x200 modes (0, 1,*/
/* 4, 5) to provide a pixel rate of 7 MHz, (9 MHz for mode D).                          */
/*                                                                                      */
/* Map Mask Register:                                                                   */
/* This is a write-only register pointed to when the value in the address register is   */
/* hex 02. The output port address for this register is hex 3C5.                        */
/*  - Bit 0: 1 Enables Map 0                                                            */
/*  - Bit 1: 1 Enables Map 1                                                            */
/*  - Bit 2: 1 Enables Map 2                                                            */
/*  - Bit 3: 1 Enables Map 3                                                            */
/*  - Bit 4: Not Used                                                                   */
/*  - Bit 5: Not Used                                                                   */
/*  - Bit 6: Not Used                                                                   */
/*  - Bit 7: Not Used                                                                   */
/* Bit 0-Bit 3: Map Mask--A logical 1 in bits 3 through 0 enables the processor to write*/
/* to the corresponding maps 3 through 0. If this register is programmed with a value of*/
/* 0FH, the CPU can perform a 32-bit write operation with only one memory cycle. This   */
/* substantially reduces the overhead on the CPU during display update cycles in        */
/* graphics modes. Data scrolling operations are also enhanced by setting this register */
/* to a value of 0FH and writing the display buffer address with the data stored in the */
/* CPU data latches. This is a read-modify-write operation. When odd/even modes are     */
/* selected, maps 0 and 1 and 2 and 3 should have the same map mask value.              */
/*                                                                                      */
/* Character Map Select Register:                                                       */
/* This is a write-only register pointed to when the value in the address register is   */
/* hex 03. The output port address for this register is 3C5.                            */
/*  - Bit 0: Character Map Select B                                                     */
/*  - Bit 1: Character Map Select B                                                     */
/*  - Bit 2: Character Map Select A                                                     */
/*  - Bit 3: Character Map Select A                                                     */
/*  - Bit 4: Not Used                                                                   */
/*  - Bit 5: Not Used                                                                   */
/*  - Bit 6: Not Used                                                                   */
/*  - Bit 7: Not Used                                                                   */
/* Bit 0-Bit 1: Character Map Select B--Selects the map used to generate alpha          */
/* characters when attribute bit 3 is a 0, according to the following table:            */
/* Bits     1 0:Map Selected:Table Location:                                            */
/*  - Value 0 0: - 0:         - 1st 8K of Plane 2 Bank 0                                */
/*  - Value 0 1: - 1:         - 2nd 8K of Plane 2 Bank 1                                */
/*  - Value 1 0: - 2:         - 3rd 8K of Plane 2 Bank 2                                */
/*  - Value 1 1: - 3:         - 4th 8K of Plane 2 Bank 3                                */
/* Bit 2-Bit 3: Character Map Select A--Selects the map used to generate alpha          */
/* characters when attribute bit 3 is a 1, according to the following table:            */
/* Bits     3 2:Map Selected:Table Location:                                            */
/*  - Value 0 0: - 0:         - 1st 8K of Plane 2 Bank 0                                */
/*  - Value 0 1: - 1:         - 2nd 8K of Plane 2 Bank 1                                */
/*  - Value 1 0: - 2:         - 3rd 8K of Plane 2 Bank 2                                */
/*  - Value 1 1: - 3:         - 4th 8K of Plane 2 Bank 3                                */
/* In alphanumeric mode, bit 3 of the attribute byte normally has the function of       */
/* turning the foreground intensity on or off. This bit however may be redefined as a   */
/* switch between character sets. This function is enabled when there is a difference   */
/* between the value in Character Map Select A and the value in Character Map Select B. */
/* Whenever these two values are the same, the character select function is disabled.   */
/* The memory mode register bit 1 must be a 1 (indicates the memory extension card is   */
/* installed in the unit) to enable this function; otherwise, bank 0 is always selected.*/
/*                                                                                      */
/* 128K of graphics memory is required to support two character sets. 256K supports four*/
/* character sets. Asynchronous reset clears this register to 0. This should be done    */
/* only when the sequencer is reset.                                                    */
/*                                                                                      */
/* Memory Mode Register:                                                                */
/* This is a write-only register pointed to when the value in the address register is   */
/* hex 04. The processor output port address for this register is 3C5.                  */
/* Memory Mode Register Format:                                                         */
/*  - 0: Alpha                                                                          */
/*  - 1: Extended Memory                                                                */
/*  - 2: Odd/Even                                                                       */
/*  - 3: Not Used                                                                       */
/*  - 4: Not Used                                                                       */
/*  - 5: Not Used                                                                       */
/*  - 6: Not Used                                                                       */
/*  - 7: Not Used                                                                       */
/* Bit 0: Alpha--A logical 0 indicates that a non-alpha mode is active. A logical 1     */
/* indicates that alpha mode is active and enables the character generator map select   */
/* function.                                                                            */
/*                                                                                      */
/* Bit 1: Extended Memory--A logical 0 indicates that the memory expansion card is not  */
/* installed. A logical 1 indicates that the memory expansion card is installed and     */
/* enables access to the extended memory through address bits 14 and 15.                */
/*                                                                                      */
/* Bit 2: Odd/Even--A logical 0 directs even processor addresses to access maps 0 and 2,*/
/* while odd processor addresses access maps 1 and 3. A logical 1 causes processor      */
/* addresses to sequentially access data within a bit map. The maps are accessed        */
/* according to the value in the map mask register.                                     */
/* ------------------------------------------------------------------------------------ */
	if (reg > 4) {
		return;
	}
	if (ega->reg_seq[reg] == val) {
		return;
	}
	switch (reg) {
	case EGA_SEQ_RESET:   /* 0 */ /* Sequencer Registers: Reset Register */
		ega->reg_seq[EGA_SEQ_RESET] = val;
		break;

	case EGA_SEQ_CLOCK:   /* 1 */ /* Sequencer Registers: Clocking Mode Register */
		ega->reg_seq[EGA_SEQ_CLOCK] = val;
		ega->update_state |= EGA_UPDATE_DIRTY;
		break;

	case EGA_SEQ_MAPMASK: /* 2 */ /* Sequencer Registers: Map Mask Register */
		ega->reg_seq[EGA_SEQ_MAPMASK] = val;
		break;

	case EGA_SEQ_CMAPSEL: /* 3 */ /* Sequencer Registers: Character Map Select Register */
		ega->reg_seq[EGA_SEQ_CMAPSEL] = val;
		ega->update_state |= EGA_UPDATE_DIRTY;
		break;

	case EGA_SEQ_MODE:    /* 4 */ /* Sequencer Registers: Memory Mode Register */
		ega->reg_seq[EGA_SEQ_MODE] = val;
		break;
	}
}


/*
 * Set a graphics controller register
 */
static
void ega_grc_set_reg (ega_t *ega, unsigned reg, unsigned char val)
{
/* ------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Major Components:  */
/* ------------------------------------------------------------------------------ */
/* Graphics Controller:                                                           */
/* The Graphics Controller directs the data from the memory to the attribute      */
/* controller and the processor. In graphics modes, memory data is sent in        */
/* serialized form to the attribute chip. In alpha modes the memory data is sent  */
/* in parallel form, bypassing the graphics controller. The graphics controller   */
/* formats the data for compatible modes and provides color comparators for use in*/
/* color painting modes. Other hardware facilities allow the processor to write 32*/
/* bits in a single memory cycle, (8 bites per plane) for quick color presetting  */
/* of the display areas, and additional logic allows the processor to write data  */
/* to the display on non-byte boundaries.                                         */
/* ------------------------------------------------------------------------------ */
	if (reg > 8) {
		return;
	}
	if (ega->reg_grc[reg] == val) {
		return;
	}
	ega->reg_grc[reg] = val;
/* ------------------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Graphics Controller Registers: */
/* ------------------------------------------------------------------------------------------ */
/* Name:                    Port:  Index:                                                     */
/*  - Mode Register         : - 3CF: - 05:                                                    */
/* Mode Register:                                                                             */
/* This is a write-only register pointed to by the value in the Graphics 1 and 2 address      */
/* register. This value must be hex 05 before writing can take place. The processor output    */
/* port address for this register is 3CF.                                                     */
/* Mode Register Format:                                                                      */
/*  - Bit 0: Write Mode                                                                       */
/*  - Bit 1: Write Mode                                                                       */
/*  - Bit 2: Test Condition                                                                   */
/*  - Bit 3: Read  Mode                                                                       */
/*  - Bit 4: Odd/Even                                                                         */
/*  - Bit 5: Test Register Mode                                                               */
/*  - Bit 6: Not Used                                                                         */
/*  - Bit 7: Not Used                                                                         */
/* Bit-Bit 1: Write Mode                                                                      */
/* Bits 1 0:                                                                                  */
/*  -   0 0: Each memory plane is written with the processor data rotated by the number of    */
/*  counts in the rotate register, unless Set/Reset is enabled for the plane. Planes for which*/
/*  Set/Reset is enabled are written with 8 bits of the value contained in the Set/Reset      */
/*  register for that plane.                                                                  */
/*  -   0 1: Each memory plane is written with the contents of the processor latches. These   */
/*  latches are loaded by a processor read operation.                                         */
/*  -   1 0: Memory plane n (0 through 3) is filled with 8 bits of the value of data bit n.   */
/*  -   1 1: Not Valid                                                                        */
/* The logic function specified by the function select register also applies.                 */
/*                                                                                            */
/* Bit 2: Test Condition--A logical 1 directs graphics controller outputs to be placed in high*/
/* impedance state for testing.                                                               */
/*                                                                                            */
/* Bit 3: Read Mode--When this bit is a logical 0, the processor reads data from the memory   */
/* plane selected by the read map select register. When this bit is a logical 1, the processor*/
/* reads the results of the comparison of the 4 memory planes and the color compare register. */
/*                                                                                            */
/* Bit 4: Odd/Even--A logical 1 selects the odd/even addressing mode, which is useful for     */
/* emulation of the Color Graphics Monitor Adapter compatible modes. Normally the value here  */
/* follows the value of the Memory Mode Register bit 3 of the Sequencer.                      */
/*                                                                                            */
/* Bit 5: Shift Register--A logical 1 directs the shift registers on each graphics chip to    */
/* format the serial data stream with even numbered bits on the even numbered maps and odd    */
/* numbered bits on the odd maps.                                                             */
/* ------------------------------------------------------------------------------------------ */
	if (reg == EGA_GRC_MODE) { /* Mode Register */
		ega->update_state |= EGA_UPDATE_DIRTY;
	}
}


/*
 * Get a CRTC register
 */
static
unsigned char ega_crtc_get_reg (ega_t *ega, unsigned reg)
{
/* ------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Major Components:  */
/* ------------------------------------------------------------------------------ */
/* CRT Controller:                                                                */
/* The CRT (Cathode Ray Tube) Controller (CRTC) generates horizontal and vertical */
/* synchronous timings, addressing for the regenerative buffer, cursor and        */
/* underline timings, and refresh addressing for the dynamic RAMs.                */
/* ------------------------------------------------------------------------------ */
	if ((reg < 12) || (reg > 17)) {
		return (0xff);
	}
	return (ega->reg_crt[reg]);
}

/*
 * Set a CRTC register
 */
static
void ega_crtc_set_reg (ega_t *ega, unsigned reg, unsigned char val)
{
/* ------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Major Components:  */
/* ------------------------------------------------------------------------------ */
/* CRT Controller:                                                                */
/* The CRT (Cathode Ray Tube) Controller (CRTC) generates horizontal and vertical */
/* synchronous timings, addressing for the regenerative buffer, cursor and        */
/* underline timings, and refresh addressing for the dynamic RAMs.                */
/* ------------------------------------------------------------------------------ */
	if (reg > 24) {
		return;
	}
	if (ega->reg_crt[reg] == val) {
		return;
	}
/* ------------------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: CRT Control Registers:         */
/* ------------------------------------------------------------------------------------------ */
/* Name:                       Port:  Index:                                                  */
/*  - Vertical Retrace End:     - 3?5: - 11                                                   */
/* ? = B in Monochrome Modes and D in Color Modes                                             */
/*                                                                                            */
/* Vertical Retrace End Register:                                                             */
/* This is a write-only register pointed to when the value in the CRT Controller address      */
/* register is hex 11. The processor output port address for this register is hex 3B5 or hex  */
/* 3D5.                                                                                       */
/* Vertical Retrace End Register Format:                                                      */
/*  - Bit 0: Vertical Retrace End                                                             */
/*  - Bit 1: Vertical Retrace End                                                             */
/*  - Bit 2: Vertical Retrace End                                                             */
/*  - Bit 3: Vertical Retrace End                                                             */
/*  - Bit 4: 0=Clear  Vertical Interrupt                                                      */
/*  - Bit 5: 0=Enable Vertical Interrupt                                                      */
/*  - Bit 6: Not Used                                                                         */
/*  - Bit 7: Not Used                                                                         */
/* Bit 0-Bit 3: Vertical Retrace End--These bits determine the horizontal scan count value    */
/* when the vertical retrace output signal becomes inactive. The register is programmed in    */
/* units of horizontal scan lines. To obtain a vertical retrace signal of width W, the        */
/* following algorithm is used: Value of Start Vertical Retrace Register + width of vertical  */
/* retrace signal in horizontal scan units = 4-bit result to be programmed into the End       */
/* Horizontal Retrace Register.                                                               */
/*                                                                                            */
/* Bit 4: Clear  Vertical Interrupt--A logical 0 will clear a vertical interrupt.             */
/*                                                                                            */
/* Bit 5: Enable Vertical Interrupt--A logical 0 will enable  vertical interrupt.             */
/* ------------------------------------------------------------------------------------------ */
	if (reg == EGA_CRT_VRE) { /* Vertical Retrace End Register */
		if ((val & EGA_CRT_VRE_CVI) == 0) { /* Clear  Vertical Interrupt */
			ega_set_irq (ega, 0);
		}
	}
	ega->reg_crt[reg] = val;
	ega_set_timing (ega);
	ega->update_state |= EGA_UPDATE_DIRTY;
}


/*
 * Get the CRTC index register
 */
static
unsigned char ega_get_crtc_index (ega_t *ega)
{
	return (ega->reg[EGA_CRT_INDEX]);
}

/*
 * Get the CRTC data register
 */
static
unsigned char ega_get_crtc_data (ega_t *ega)
{
	return (ega_crtc_get_reg (ega, ega->reg[EGA_CRT_INDEX] & 0x1f));
}

/*
 * Get the input status register 0
 */
static
unsigned char ega_get_input_status_0 (ega_t *ega)
{
	unsigned bit;
/* ----------------------------------------------------------------------------------- */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Registers:              */
/* ----------------------------------------------------------------------------------- */
/* External Registers:                                                                 */
/* This section contains descriptions of the registers of the Enhanced Graphics Adapter*/
/* that are not contained in an LSI device.                                            */
/* Name:                              Port:  Index:                                    */
/*  - Miscellaneous Output Register  : - 3C2: - -:                                     */
/*  - Feature Control      Register  : - 3?A: - -:                                     */
/*  - Input Status         Register 0: - 3C2: - -:                                     */
/*  - Input Status         Register 1: - 3?2: - -:                                     */
/* ? = B in Monochrome Modes    ? = D in Color Modes                                   */
/* Miscellaneous Output Register:                                                      */
/* This is a write-only register. The processor output port address is hex 3C2. A      */
/* hardware reset causes all bits to reset to zero.                                    */
/* Miscellaneous Output Register Format:                                               */
/*  - Bit 0: I/O Address Select                                                        */
/*  - Bit 1: Enable Ram                                                                */
/*  - Bit 2: Clock Select 0                                                            */
/*  - Bit 3: Clock Select 1                                                            */
/*  - Bit 4: Disable Internal Video Drivers                                            */
/*  - Bit 5: Page Bit For Odd/Even                                                     */
/*  - Bit 6: Horizontal Retrace Polarity                                               */
/*  - Bit 7: Vertical   Retrace Polarity                                               */
/* Bit 0: 3BX/3DX CRTC I/O Address--This bit maps the CRTC I/O addresses for IBM       */
/* Monochrome or Color/Graphics Monitor Adapter emulation. A logical 0 sets CRTC       */
/* addresses to 3BX and Input Status Register 1 's address to 3BA for Monochrome       */
/* emulation. A logical 1 sets CRTC addresses to 3DX and Input Status Register 1's     */
/* address to 3DA for Color/Graphics Monitor Adapter emulation.                        */
/*                                                                                     */
/* Bit 1: Enable RAM--A logical 0 disables RAM from the processor; a logical 1 enables */
/* RAM to respond at addresses designated by the Control Data Select value programmed  */
/* into the Graphics Controllers.                                                      */
/*                                                                                     */
/* Bit 2-Bit 3: Clock Select--These two bits select the clock source according to the  */
/* following table:                                                                    */
/* Bits:                                                                               */
/*  - 3 2:                                                                             */
/* --------                                                                            */
/*  - 0 0-: Selects 14 MHz clock from the processor I/O channel                        */
/*  - 0 1-: Selects 16 MHz clock on-board oscillator                                   */
/*  - 1 0-: Selects external clock source from the feature connector.                  */
/*  - 1 1-: Not used                                                                   */
/*                                                                                     */
/* Bit 4: Disable Internal Video Drivers--A logical 0 activates internal video drivers;*/
/* a logical 1 disables internal video drivers. When the internal video drivers are    */
/* disabled, the source of the direct drive color output becomes the feature connector */
/* direct drive outputs.                                                               */
/*                                                                                     */
/* Bit 5: Page Bit For Odd/Even--Selects between two 64K pages of memory when in the   */
/* Odd/Even modes (0,1,2,3,7). A logical 0 selects the low page of memory; a logical 1 */
/* selects the high page of memory.                                                    */
/*                                                                                     */
/* Bit 6: Horizontal Retrace Polarity--A logical 0 selects positive horizontal retrace;*/
/* a logical 1 selects negative horizontal retrace.                                    */
/*                                                                                     */
/* Bit 7: Vertical   Retrace Polarity--A logical 0 selects positive vertical   retrace;*/
/* a logical 1 selects negative vertical   retrace.                                    */
/* ----------------------------------------------------------------------------------- */
	bit = (ega->reg[EGA_MOUT] >> 2) & 3;
/* ----------------------------------------------------------------------------------- */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Registers:              */
/* ----------------------------------------------------------------------------------- */
/* External Registers:                                                                 */
/* This section contains descriptions of the registers of the Enhanced Graphics Adapter*/
/* that are not contained in an LSI device.                                            */
/* Name:                              Port:  Index:                                    */
/*  - Miscellaneous Output Register  : - 3C2: - -:                                     */
/*  - Feature Control      Register  : - 3?A: - -:                                     */
/*  - Input Status         Register 0: - 3C2: - -:                                     */
/*  - Input Status         Register 1: - 3?2: - -:                                     */
/* ? = B in Monochrome Modes    ? = D in Color Modes                                   */
/* Input Status Register Zero:                                                         */
/* This is a read-only register. The processor input port address is hex 3C2.          */
/* Input Status Register Zero Format:                                                  */
/*  - Bit 0: Not Used                                                                  */
/*  - Bit 1: Not Used                                                                  */
/*  - Bit 2: Not Used                                                                  */
/*  - Bit 3: Not Used                                                                  */
/*  - Bit 4: Switch Sense                                                              */
/*  - Bit 5: Reserved                                                                  */
/*  - Bit 6: Reserved                                                                  */
/*  - Bit 7: CRT Interrupt                                                             */
/* Bit 4: Switch Sense--When set to 1, this bit allows the processor to read the four  */
/* configuration switches on the board. The setting of the CLKSEL field determines     */
/* which switch is being read. The switch configuration can be determined by reading   */
/* byte 40:88H in RAM.                                                                 */
/*  - Bit 3: Switch 4 ; Logical 0 = switch closed                                      */
/*  - Bit 2: Switch 3 ; Logical 0 = switch closed                                      */
/*  - Bit 1: Switch 2 ; Logical 0 = switch closed                                      */
/*  - Bit 0: Switch 1 ; Logical 0 = switch closed                                      */
/* ----------------------------------------------------------------------------------- */
	if (ega->switches & (0x08 >> bit)) {
		ega->reg[EGA_STATUS0] |= EGA_STATUS0_SS;  /* Input Status Register Zerp: Switch Sense */
	}
	else {
		ega->reg[EGA_STATUS0] &= ~EGA_STATUS0_SS; /* Input Status Register Zero: Switch Sense */
	}
	return (ega->reg[EGA_STATUS0]);
}

/*
 * Get the miscellaneous output register
 */
static
unsigned char ega_get_misc_out (ega_t *ega)
{
/* ----------------------------------------------------------------------------------- */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Registers:              */
/* ----------------------------------------------------------------------------------- */
/* External Registers:                                                                 */
/* This section contains descriptions of the registers of the Enhanced Graphics Adapter*/
/* that are not contained in an LSI device.                                            */
/* Name:                              Port:  Index:                                    */
/*  - Miscellaneous Output Register  : - 3C2: - -:                                     */
/*  - Feature Control      Register  : - 3?A: - -:                                     */
/*  - Input Status         Register 0: - 3C2: - -:                                     */
/*  - Input Status         Register 1: - 3?2: - -:                                     */
/* ? = B in Monochrome Modes    ? = D in Color Modes                                   */
/* Miscellaneous Output Register:                                                      */
/* This is a write-only register. The processor output port address is hex 3C2. A      */
/* hardware reset causes all bits to reset to zero.                                    */
/* Miscellaneous Output Register Format:                                               */
/*  - Bit 0: I/O Address Select                                                        */
/*  - Bit 1: Enable Ram                                                                */
/*  - Bit 2: Clock Select 0                                                            */
/*  - Bit 3: Clock Select 1                                                            */
/*  - Bit 4: Disable Internal Video Drivers                                            */
/*  - Bit 5: Page Bit For Odd/Even                                                     */
/*  - Bit 6: Horizontal Retrace Polarity                                               */
/*  - Bit 7: Vertical   Retrace Polarity                                               */
/* Bit 0: 3BX/3DX CRTC I/O Address--This bit maps the CRTC I/O addresses for IBM       */
/* Monochrome or Color/Graphics Monitor Adapter emulation. A logical 0 sets CRTC       */
/* addresses to 3BX and Input Status Register 1 's address to 3BA for Monochrome       */
/* emulation. A logical 1 sets CRTC addresses to 3DX and Input Status Register 1's     */
/* address to 3DA for Color/Graphics Monitor Adapter emulation.                        */
/*                                                                                     */
/* Bit 1: Enable RAM--A logical 0 disables RAM from the processor; a logical 1 enables */
/* RAM to respond at addresses designated by the Control Data Select value programmed  */
/* into the Graphics Controllers.                                                      */
/*                                                                                     */
/* Bit 2-Bit 3: Clock Select--These two bits select the clock source according to the  */
/* following table:                                                                    */
/* Bits:                                                                               */
/*  - 3 2:                                                                             */
/* --------                                                                            */
/*  - 0 0-: Selects 14 MHz clock from the processor I/O channel                        */
/*  - 0 1-: Selects 16 MHz clock on-board oscillator                                   */
/*  - 1 0-: Selects external clock source from the feature connector.                  */
/*  - 1 1-: Not used                                                                   */
/*                                                                                     */
/* Bit 4: Disable Internal Video Drivers--A logical 0 activates internal video drivers;*/
/* a logical 1 disables internal video drivers. When the internal video drivers are    */
/* disabled, the source of the direct drive color output becomes the feature connector */
/* direct drive outputs.                                                               */
/*                                                                                     */
/* Bit 5: Page Bit For Odd/Even--Selects between two 64K pages of memory when in the   */
/* Odd/Even modes (0,1,2,3,7). A logical 0 selects the low page of memory; a logical 1 */
/* selects the high page of memory.                                                    */
/*                                                                                     */
/* Bit 6: Horizontal Retrace Polarity--A logical 0 selects positive horizontal retrace;*/
/* a logical 1 selects negative horizontal retrace.                                    */
/*                                                                                     */
/* Bit 7: Vertical   Retrace Polarity--A logical 0 selects positive vertical   retrace;*/
/* a logical 1 selects negative vertical   retrace.                                    */
/* ----------------------------------------------------------------------------------- */
	return (ega->reg[EGA_MOUT]);
}

/*
 * Get the input status register 1
 */
static
unsigned char ega_get_input_status_1 (ega_t *ega)
{
	unsigned char val;
	unsigned long clk;
	ega_clock (ega, 0);
/* ------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Major Components:  */
/* ------------------------------------------------------------------------------ */
/* Attribute Controller:                                                          */
/* The Attribute Controller provides a color palette of 16 colors, each of which  */
/* may be specified separately. Six color outputs are available for driving a     */
/* display. Blinking and underlining are controlled by this chip. This chip takes */
/* data from the display memory and formats it for display on the CRT screen.     */
/* ------------------------------------------------------------------------------ */
	ega->atc_flipflop = 0;
	clk = ega_get_dotclock (ega);
/* ----------------------------------------------------------------------------------- */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Registers:              */
/* ----------------------------------------------------------------------------------- */
/* External Registers:                                                                 */
/* This section contains descriptions of the registers of the Enhanced Graphics Adapter*/
/* that are not contained in an LSI device.                                            */
/* Name:                              Port:  Index:                                    */
/*  - Miscellaneous Output Register  : - 3C2: - -:                                     */
/*  - Feature Control      Register  : - 3?A: - -:                                     */
/*  - Input Status         Register 0: - 3C2: - -:                                     */
/*  - Input Status         Register 1: - 3?2: - -:                                     */
/* ? = B in Monochrome Modes    ? = D in Color Modes                                   */
/* Input Status Register One:                                                          */
/* This is a read-only register. The processor port address is hex 3BA or hex 3DA.     */
/* Input Status Register One Format:                                                   */
/*  - Bit 0: Display Enable                                                            */
/*  - Bit 1: Light Pen Strobe                                                          */
/*  - Bit 2: Light Pen Switch                                                          */
/*  - Bit 3: Vertical Retrace                                                          */
/*  - Bit 4: Diagnostic 1                                                              */
/*  - Bit 5: Diagnostic 0                                                              */
/*  - Bit 6: Not Used                                                                  */
/* Bit 0: Display Enable--Logical 0 indicates the CRT raster is in a horizontal or     */
/* vertical retrace interval. This bit is the real time status of the display enable   */
/* signal. Some programs use this status bit to restrict screen updates to inactive    */
/* display intervals. The Enhanced Graphics Adapter does not require the CPU to update */
/* the screen buffer during inactive display intervals to avoid glitches in the display*/
/* image.                                                                              */
/* Bit 3: Vertical Retrace--A logical 0 indicates that video information is being      */
/* displayed on the CRT screen; a logical 1 indicates the CRT is in a vertical retrace */
/* interval. This bit can be programmed to interrupt the processor on interrupt level 2*/
/* at the start of the vertical retrace. This is done through bits 4 and 5 of the      */
/* Vertical Retrace End Register of the CRTC.                                          */
/* ----------------------------------------------------------------------------------- */
	val = ega->reg[EGA_STATUS1];
	val &= ~(EGA_STATUS1_DE | EGA_STATUS1_VR);    /* Input Status Register One: Display Enable/Vertical Retrace */
	if (clk >= ega->clk_vd) {
		val |= (EGA_STATUS1_DE | EGA_STATUS1_VR); /* Input Status Register One: Display Enable/Vertical Retrace */
	}
	else if ((clk % ega->clk_ht) >= ega->clk_hd) {
		val |= EGA_STATUS1_DE; /* Input Status Register One: Display Enable */
	}
	val = (val & ~0x30) | ((val + 0x10) & 0x30); /* NOTE: Required by IBM EGA BIOS ROM */
	ega->reg[EGA_STATUS1] = val;
	return (val);
}

/*
 * Get an EGA register
 */
static
unsigned char ega_reg_get_uint8 (ega_t *ega, unsigned long addr)
{
/* ----------------------------------------------------------------------------------- */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Registers:              */
/* ----------------------------------------------------------------------------------- */
/* External Registers:                                                                 */
/* This section contains descriptions of the registers of the Enhanced Graphics Adapter*/
/* that are not contained in an LSI device.                                            */
/* Name:                              Port:  Index:                                    */
/*  - Miscellaneous Output Register  : - 3C2: - -:                                     */
/*  - Feature Control      Register  : - 3?A: - -:                                     */
/*  - Input Status         Register 0: - 3C2: - -:                                     */
/*  - Input Status         Register 1: - 3?2: - -:                                     */
/* ? = B in Monochrome Modes    ? = D in Color Modes                                   */
/* Miscellaneous Output Register:                                                      */
/* This is a write-only register. The processor output port address is hex 3C2. A      */
/* hardware reset causes all bits to reset to zero.                                    */
/* Miscellaneous Output Register Format:                                               */
/*  - Bit 0: I/O Address Select                                                        */
/*  - Bit 1: Enable Ram                                                                */
/*  - Bit 2: Clock Select 0                                                            */
/*  - Bit 3: Clock Select 1                                                            */
/*  - Bit 4: Disable Internal Video Drivers                                            */
/*  - Bit 5: Page Bit For Odd/Even                                                     */
/*  - Bit 6: Horizontal Retrace Polarity                                               */
/*  - Bit 7: Vertical   Retrace Polarity                                               */
/* Bit 0: 3BX/3DX CRTC I/O Address--This bit maps the CRTC I/O addresses for IBM       */
/* Monochrome or Color/Graphics Monitor Adapter emulation. A logical 0 sets CRTC       */
/* addresses to 3BX and Input Status Register 1 's address to 3BA for Monochrome       */
/* emulation. A logical 1 sets CRTC addresses to 3DX and Input Status Register 1's     */
/* address to 3DA for Color/Graphics Monitor Adapter emulation.                        */
/* ----------------------------------------------------------------------------------- */
	if (addr < 0x10) {
		if (ega->reg[EGA_MOUT] & EGA_MOUT_IOS) {        /* Miscellaneous Output Register: I/O Address Select */
			return (0xff);
		}
		addr += 0x20;
	}
	else if (addr > 0x20) {
		if ((ega->reg[EGA_MOUT] & EGA_MOUT_IOS) == 0) { /* Miscellaneous Output Register: I/O Address Select */
			return (0xff);
		}
	}
	switch (addr) {
/* ----------------------------------------------------------------------------------- */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Registers:              */
/* ----------------------------------------------------------------------------------- */
/* External Registers:                                                                 */
/* This section contains descriptions of the registers of the Enhanced Graphics Adapter*/
/* that are not contained in an LSI device.                                            */
/* Name:                              Port:  Index:                                    */
/*  - Miscellaneous Output Register  : - 3C2: - -:                                     */
/*  - Feature Control      Register  : - 3?A: - -:                                     */
/*  - Input Status         Register 0: - 3C2: - -:                                     */
/*  - Input Status         Register 1: - 3?2: - -:                                     */
/* ? = B in Monochrome Modes    ? = D in Color Modes                                   */
/* Input Status Register Zero:                                                         */
/* This is a read-only register. The processor input port address is hex 3C2.          */
/* Input Status Register Zero Format:                                                  */
/*  - Bit 0: Not Used                                                                  */
/*  - Bit 1: Not Used                                                                  */
/*  - Bit 2: Not Used                                                                  */
/*  - Bit 3: Not Used                                                                  */
/*  - Bit 4: Switch Sense                                                              */
/*  - Bit 5: Reserved                                                                  */
/*  - Bit 6: Reserved                                                                  */
/*  - Bit 7: CRT Interrupt                                                             */
/* Bit 4: Switch Sense--When set to 1, this bit allows the processor to read the four  */
/* configuration switches on the board. The setting of the CLKSEL field determines     */
/* which switch is being read. The switch configuration can be determined by reading   */
/* byte 40:88H in RAM.                                                                 */
/*  - Bit 3: Switch 4 ; Logical 0 = switch closed                                      */
/*  - Bit 2: Switch 3 ; Logical 0 = switch closed                                      */
/*  - Bit 1: Switch 2 ; Logical 0 = switch closed                                      */
/*  - Bit 0: Switch 1 ; Logical 0 = switch closed                                      */
/*                                                                                     */
/* Bits 5 and 6: Feature Code--These bits are input from the Feat (0) and Feat (1) pins*/
/* on the feature connector.                                                           */
/*                                                                                     */
/* Bit 7: CRT Interrupt--A logical 1 indicates video is being displayed on the CRT     */
/* screen; a logical 0 indicates that vertical retrace is occurring.                   */
/* ----------------------------------------------------------------------------------- */
	case EGA_STATUS0: /* 3C2 */ /* Input Status Register Zero: */
		return (ega_get_input_status_0 (ega));

	case EGA_GENOA: /* 3C8 */ /* GENOA EGA only */
		return (0x02);

	case EGA_MOUT: /* 3CC */ /* Miscellaneous Output Register */
		return (ega_get_misc_out (ega));

	case EGA_CRT_INDEX0: /* 3D0 */
	case EGA_CRT_INDEX: /* 3D4 */
		return (ega_get_crtc_index (ega));

	case EGA_CRT_DATA0: /* 3D1 */
	case EGA_CRT_DATA: /* 3D5 */
		return (ega_get_crtc_data (ega));
	case EGA_STATUS1: /* 3DA */ /* Input Status Register One */
		return (ega_get_input_status_1 (ega));
	}
	return (0xff);
}

static
unsigned short ega_reg_get_uint16 (ega_t *ega, unsigned long addr)
{
	unsigned short ret;

	ret = ega_reg_get_uint8 (ega, addr);
	ret |= ega_reg_get_uint8 (ega, addr + 1) << 8;

	return (ret);
}


/*
 * Set the attribute controller index/data register
 */
static
void ega_set_atc_index (ega_t *ega, unsigned char val)
{
/* ------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Major Components:  */
/* ------------------------------------------------------------------------------ */
/* Attribute Controller:                                                          */
/* The Attribute Controller provides a color palette of 16 colors, each of which  */
/* may be specified separately. Six color outputs are available for driving a     */
/* display. Blinking and underlining are controlled by this chip. This chip takes */
/* data from the display memory and formats it for display on the CRT screen.     */
/* ------------------------------------------------------------------------------ */
	if (ega->atc_flipflop == 0) {
		ega->reg[EGA_ATC_INDEX] = val;
		ega->atc_flipflop = 1;
	}
	else {
		ega_atc_set_reg (ega, ega->reg[EGA_ATC_INDEX] & 0x1f, val);
		ega->atc_flipflop = 0;
	}
}

/*
 * Set the sequencer index register
 */
static
void ega_set_seq_index (ega_t *ega, unsigned char val)
{
/* ------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Major Components:  */
/* ------------------------------------------------------------------------------ */
/* Sequencer:                                                                     */
/* The Sequencer generates basic memory timings for the dynamic RAMs and the      */
/* character clock for controlling regenerative memory fetches. It allows the     */
/* processor to access memory during active display intervals by inserting        */
/* dedicated processor memory cycles periodically between the display memory      */
/* cycles. Map mask registers are available to protect entire memory maps from    */
/* being changed.                                                                 */
/* ------------------------------------------------------------------------------ */
	ega->reg[EGA_SEQ_INDEX] = val;
}

/*
 * Set the sequencer data register
 */
static
void ega_set_seq_data (ega_t *ega, unsigned char val)
{
/* ------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Major Components:  */
/* ------------------------------------------------------------------------------ */
/* Sequencer:                                                                     */
/* The Sequencer generates basic memory timings for the dynamic RAMs and the      */
/* character clock for controlling regenerative memory fetches. It allows the     */
/* processor to access memory during active display intervals by inserting        */
/* dedicated processor memory cycles periodically between the display memory      */
/* cycles. Map mask registers are available to protect entire memory maps from    */
/* being changed.                                                                 */
/* ------------------------------------------------------------------------------ */
	ega->reg[EGA_SEQ_DATA] = val;
	ega_seq_set_reg (ega, ega->reg[EGA_SEQ_INDEX] & 7, val);
}

/*
 * Set the graphics controller index register
 */
static
void ega_set_grc_index (ega_t *ega, unsigned char val)
{
/* ------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Major Components:  */
/* ------------------------------------------------------------------------------ */
/* Graphics Controller:                                                           */
/* The Graphics Controller directs the data from the memory to the attribute      */
/* controller and the processor. In graphics modes, memory data is sent in        */
/* serialized form to the attribute chip. In alpha modes the memory data is sent  */
/* in parallel form, bypassing the graphics controller. The graphics controller   */
/* formats the data for compatible modes and provides color comparators for use in*/
/* color painting modes. Other hardware facilities allow the processor to write 32*/
/* bits in a single memory cycle, (8 bites per plane) for quick color presetting  */
/* of the display areas, and additional logic allows the processor to write data  */
/* to the display on non-byte boundaries.                                         */
/* ------------------------------------------------------------------------------ */
	ega->reg[EGA_GRC_INDEX] = val;
}

/*
 * Set the graphics controller data register
 */
static
void ega_set_grc_data (ega_t *ega, unsigned char val)
{
/* ------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Major Components:  */
/* ------------------------------------------------------------------------------ */
/* Graphics Controller:                                                           */
/* The Graphics Controller directs the data from the memory to the attribute      */
/* controller and the processor. In graphics modes, memory data is sent in        */
/* serialized form to the attribute chip. In alpha modes the memory data is sent  */
/* in parallel form, bypassing the graphics controller. The graphics controller   */
/* formats the data for compatible modes and provides color comparators for use in*/
/* color painting modes. Other hardware facilities allow the processor to write 32*/
/* bits in a single memory cycle, (8 bites per plane) for quick color presetting  */
/* of the display areas, and additional logic allows the processor to write data  */
/* to the display on non-byte boundaries.                                         */
/* ------------------------------------------------------------------------------ */
	ega->reg[EGA_GRC_DATA] = val;
	ega_grc_set_reg (ega, ega->reg[EGA_GRC_INDEX] & 0x0f, val);
}

/*
 * Set the CRTC index register
 */
static
void ega_set_crtc_index (ega_t *ega, unsigned char val)
{
/* ------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Major Components:  */
/* ------------------------------------------------------------------------------ */
/* CRT Controller:                                                                */
/* The CRT (Cathode Ray Tube) Controller (CRTC) generates horizontal and vertical */
/* synchronous timings, addressing for the regenerative buffer, cursor and        */
/* underline timings, and refresh addressing for the dynamic RAMs.                */
/* ------------------------------------------------------------------------------ */
	ega->reg[EGA_CRT_INDEX] = val;
}

/*
 * Set the CRTC data register
 */
static
void ega_set_crtc_data (ega_t *ega, unsigned char val)
{
/* ------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Major Components:  */
/* ------------------------------------------------------------------------------ */
/* CRT Controller:                                                                */
/* The CRT (Cathode Ray Tube) Controller (CRTC) generates horizontal and vertical */
/* synchronous timings, addressing for the regenerative buffer, cursor and        */
/* underline timings, and refresh addressing for the dynamic RAMs.                */
/* ------------------------------------------------------------------------------ */
	ega->reg[EGA_CRT_DATA] = val;
	ega_crtc_set_reg (ega, ega->reg[EGA_CRT_INDEX] & 0x1f, val);
}

/*
 * Set the miscellaneous output register
 */
static
void ega_set_misc_out (ega_t *ega, unsigned char val)
{
/* ----------------------------------------------------------------------------------- */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Registers:              */
/* ----------------------------------------------------------------------------------- */
/* External Registers:                                                                 */
/* This section contains descriptions of the registers of the Enhanced Graphics Adapter*/
/* that are not contained in an LSI device.                                            */
/* Name:                              Port:  Index:                                    */
/*  - Miscellaneous Output Register  : - 3C2: - -:                                     */
/*  - Feature Control      Register  : - 3?A: - -:                                     */
/*  - Input Status         Register 0: - 3C2: - -:                                     */
/*  - Input Status         Register 1: - 3?2: - -:                                     */
/* ? = B in Monochrome Modes    ? = D in Color Modes                                   */
/* Miscellaneous Output Register:                                                      */
/* This is a write-only register. The processor output port address is hex 3C2. A      */
/* hardware reset causes all bits to reset to zero.                                    */
/* Miscellaneous Output Register Format:                                               */
/*  - Bit 0: I/O Address Select                                                        */
/*  - Bit 1: Enable Ram                                                                */
/*  - Bit 2: Clock Select 0                                                            */
/*  - Bit 3: Clock Select 1                                                            */
/*  - Bit 4: Disable Internal Video Drivers                                            */
/*  - Bit 5: Page Bit For Odd/Even                                                     */
/*  - Bit 6: Horizontal Retrace Polarity                                               */
/*  - Bit 7: Vertical   Retrace Polarity                                               */
/* Bit 0: 3BX/3DX CRTC I/O Address--This bit maps the CRTC I/O addresses for IBM       */
/* Monochrome or Color/Graphics Monitor Adapter emulation. A logical 0 sets CRTC       */
/* addresses to 3BX and Input Status Register 1 's address to 3BA for Monochrome       */
/* emulation. A logical 1 sets CRTC addresses to 3DX and Input Status Register 1's     */
/* address to 3DA for Color/Graphics Monitor Adapter emulation.                        */
/*                                                                                     */
/* Bit 1: Enable RAM--A logical 0 disables RAM from the processor; a logical 1 enables */
/* RAM to respond at addresses designated by the Control Data Select value programmed  */
/* into the Graphics Controllers.                                                      */
/*                                                                                     */
/* Bit 2-Bit 3: Clock Select--These two bits select the clock source according to the  */
/* following table:                                                                    */
/* Bits:                                                                               */
/*  - 3 2:                                                                             */
/* --------                                                                            */
/*  - 0 0-: Selects 14 MHz clock from the processor I/O channel                        */
/*  - 0 1-: Selects 16 MHz clock on-board oscillator                                   */
/*  - 1 0-: Selects external clock source from the feature connector.                  */
/*  - 1 1-: Not used                                                                   */
/*                                                                                     */
/* Bit 4: Disable Internal Video Drivers--A logical 0 activates internal video drivers;*/
/* a logical 1 disables internal video drivers. When the internal video drivers are    */
/* disabled, the source of the direct drive color output becomes the feature connector */
/* direct drive outputs.                                                               */
/*                                                                                     */
/* Bit 5: Page Bit For Odd/Even--Selects between two 64K pages of memory when in the   */
/* Odd/Even modes (0,1,2,3,7). A logical 0 selects the low page of memory; a logical 1 */
/* selects the high page of memory.                                                    */
/*                                                                                     */
/* Bit 6: Horizontal Retrace Polarity--A logical 0 selects positive horizontal retrace;*/
/* a logical 1 selects negative horizontal retrace.                                    */
/*                                                                                     */
/* Bit 7: Vertical   Retrace Polarity--A logical 0 selects positive vertical   retrace;*/
/* a logical 1 selects negative vertical   retrace.                                    */
/* ----------------------------------------------------------------------------------- */
	ega->reg[EGA_MOUT] = val; /* Miscellaneous Output Register */
	ega->update_state |= EGA_UPDATE_DIRTY;
}

/*
 * Set an EGA register
 */
static
void ega_reg_set_uint8 (ega_t *ega, unsigned long addr, unsigned char val)
{
/* ----------------------------------------------------------------------------------- */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Registers:              */
/* ----------------------------------------------------------------------------------- */
/* External Registers:                                                                 */
/* This section contains descriptions of the registers of the Enhanced Graphics Adapter*/
/* that are not contained in an LSI device.                                            */
/* Name:                              Port:  Index:                                    */
/*  - Miscellaneous Output Register  : - 3C2: - -:                                     */
/*  - Feature Control      Register  : - 3?A: - -:                                     */
/*  - Input Status         Register 0: - 3C2: - -:                                     */
/*  - Input Status         Register 1: - 3?2: - -:                                     */
/* ? = B in Monochrome Modes    ? = D in Color Modes                                   */
/* Miscellaneous Output Register:                                                      */
/* This is a write-only register. The processor output port address is hex 3C2. A      */
/* hardware reset causes all bits to reset to zero.                                    */
/* Miscellaneous Output Register Format:                                               */
/*  - Bit 0: I/O Address Select                                                        */
/*  - Bit 1: Enable Ram                                                                */
/*  - Bit 2: Clock Select 0                                                            */
/*  - Bit 3: Clock Select 1                                                            */
/*  - Bit 4: Disable Internal Video Drivers                                            */
/*  - Bit 5: Page Bit For Odd/Even                                                     */
/*  - Bit 6: Horizontal Retrace Polarity                                               */
/*  - Bit 7: Vertical   Retrace Polarity                                               */
/* Bit 0: 3BX/3DX CRTC I/O Address--This bit maps the CRTC I/O addresses for IBM       */
/* Monochrome or Color/Graphics Monitor Adapter emulation. A logical 0 sets CRTC       */
/* addresses to 3BX and Input Status Register 1 's address to 3BA for Monochrome       */
/* emulation. A logical 1 sets CRTC addresses to 3DX and Input Status Register 1's     */
/* address to 3DA for Color/Graphics Monitor Adapter emulation.                        */
/* ----------------------------------------------------------------------------------- */
	if (addr < 0x10) {
		if (ega->reg[EGA_MOUT] & EGA_MOUT_IOS) {        /* Miscellaneous Output Register: I/O Address Select */
			return;
		}

		addr += 0x20;
	}
	else if (addr > 0x20) {
		if ((ega->reg[EGA_MOUT] & EGA_MOUT_IOS) == 0) { /* Miscellaneous Output Register: I/O Address Select */
			return;
		}
	}
/* ------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Major Components:  */
/* ------------------------------------------------------------------------------ */
/* Attribute Controller:                                                          */
/* The Attribute Controller provides a color palette of 16 colors, each of which  */
/* may be specified separately. Six color outputs are available for driving a     */
/* display. Blinking and underlining are controlled by this chip. This chip takes */
/* data from the display memory and formats it for display on the CRT screen.     */
/* ------------------------------------------------------------------------------ */
	switch (addr) {
	case EGA_ATC_INDEX: /* 3C0 */
		ega_set_atc_index (ega, val);
		break;

	case EGA_ATC_INDEX + 1: /* 3C1 */
		ega_set_atc_index (ega, val);
		break;
/* ----------------------------------------------------------------------------------- */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Registers:              */
/* ----------------------------------------------------------------------------------- */
/* External Registers:                                                                 */
/* This section contains descriptions of the registers of the Enhanced Graphics Adapter*/
/* that are not contained in an LSI device.                                            */
/* Name:                              Port:  Index:                                    */
/*  - Miscellaneous Output Register  : - 3C2: - -:                                     */
/*  - Feature Control      Register  : - 3?A: - -:                                     */
/*  - Input Status         Register 0: - 3C2: - -:                                     */
/*  - Input Status         Register 1: - 3?2: - -:                                     */
/* ? = B in Monochrome Modes    ? = D in Color Modes                                   */
/* Miscellaneous Output Register:                                                      */
/* This is a write-only register. The processor output port address is hex 3C2. A      */
/* hardware reset causes all bits to reset to zero.                                    */
/* Miscellaneous Output Register Format:                                               */
/*  - Bit 0: I/O Address Select                                                        */
/*  - Bit 1: Enable Ram                                                                */
/*  - Bit 2: Clock Select 0                                                            */
/*  - Bit 3: Clock Select 1                                                            */
/*  - Bit 4: Disable Internal Video Drivers                                            */
/*  - Bit 5: Page Bit For Odd/Even                                                     */
/*  - Bit 6: Horizontal Retrace Polarity                                               */
/*  - Bit 7: Vertical   Retrace Polarity                                               */
/* ----------------------------------------------------------------------------------- */
	case EGA_MOUT_W: /* 3C2 */ /* Miscellaneous Output Register */
		ega_set_misc_out (ega, val);
		break;
/* ------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Major Components:  */
/* ------------------------------------------------------------------------------ */
/* Sequencer:                                                                     */
/* The Sequencer generates basic memory timings for the dynamic RAMs and the      */
/* character clock for controlling regenerative memory fetches. It allows the     */
/* processor to access memory during active display intervals by inserting        */
/* dedicated processor memory cycles periodically between the display memory      */
/* cycles. Map mask registers are available to protect entire memory maps from    */
/* being changed.                                                                 */
/* ------------------------------------------------------------------------------ */
	case EGA_SEQ_INDEX: /* 3C4 */
		ega_set_seq_index (ega, val);
		break;

	case EGA_SEQ_DATA: /* 3C5 */
		ega_set_seq_data (ega, val);
		break;

	case EGA_MOUT: /* 3CC */ /* Miscellaneous Output Register */
		break;
/* ------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Major Components:  */
/* ------------------------------------------------------------------------------ */
/* Graphics Controller:                                                           */
/* The Graphics Controller directs the data from the memory to the attribute      */
/* controller and the processor. In graphics modes, memory data is sent in        */
/* serialized form to the attribute chip. In alpha modes the memory data is sent  */
/* in parallel form, bypassing the graphics controller. The graphics controller   */
/* formats the data for compatible modes and provides color comparators for use in*/
/* color painting modes. Other hardware facilities allow the processor to write 32*/
/* bits in a single memory cycle, (8 bites per plane) for quick color presetting  */
/* of the display areas, and additional logic allows the processor to write data  */
/* to the display on non-byte boundaries.                                         */
/* ------------------------------------------------------------------------------ */
	case EGA_GRC_INDEX: /* 3CE */
		ega_set_grc_index (ega, val);
		break;

	case EGA_GRC_DATA: /* 3CF */
		ega_set_grc_data (ega, val);
		break;
/* ------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Major Components:  */
/* ------------------------------------------------------------------------------ */
/* CRT Controller:                                                                */
/* The CRT (Cathode Ray Tube) Controller (CRTC) generates horizontal and vertical */
/* synchronous timings, addressing for the regenerative buffer, cursor and        */
/* underline timings, and refresh addressing for the dynamic RAMs.                */
/* ------------------------------------------------------------------------------ */
	case EGA_CRT_INDEX0: /* 3D0 */
	case EGA_CRT_INDEX: /* 3D4 */
		ega_set_crtc_index (ega, val);
		break;

	case EGA_CRT_DATA0: /* 3D1 */
	case EGA_CRT_DATA: /* 3D5 */
		ega_set_crtc_data (ega, val);
		break;
/* ----------------------------------------------------------------------------------- */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Registers:              */
/* ----------------------------------------------------------------------------------- */
/* External Registers:                                                                 */
/* This section contains descriptions of the registers of the Enhanced Graphics Adapter*/
/* that are not contained in an LSI device.                                            */
/* Name:                              Port:  Index:                                    */
/*  - Miscellaneous Output Register  : - 3C2: - -:                                     */
/*  - Feature Control      Register  : - 3?A: - -:                                     */
/*  - Input Status         Register 0: - 3C2: - -:                                     */
/*  - Input Status         Register 1: - 3?2: - -:                                     */
/* ? = B in Monochrome Modes    ? = D in Color Modes                                   */
/* Input Status Register One:                                                          */
/* This is a read-only register. The processor port address is hex 3BA or hex 3DA.     */
/* Input Status Register One Format:                                                   */
/*  - Bit 0: Display Enable                                                            */
/*  - Bit 1: Light Pen Strobe                                                          */
/*  - Bit 2: Light Pen Switch                                                          */
/*  - Bit 3: Vertical Retrace                                                          */
/*  - Bit 4: Diagnostic 1                                                              */
/*  - Bit 5: Diagnostic 0                                                              */
/*  - Bit 6: Not Used                                                                  */
/* Bit 0: Display Enable--Logical 0 indicates the CRT raster is in a horizontal or     */
/* vertical retrace interval. This bit is the real time status of the display enable   */
/* signal. Some programs use this status bit to restrict screen updates to inactive    */
/* display intervals. The Enhanced Graphics Adapter does not require the CPU to update */
/* the screen buffer during inactive display intervals to avoid glitches in the display*/
/* image.                                                                              */
/*                                                                                     */
/* Bit 1: Light Pen Strobe--A logical 0 indicates that the light pen trigger has not   */
/* been set; a logical 1 indicates that the light pen trigger has been set.            */
/*                                                                                     */
/* Bit 2: Light Pen Switch--A logical 0 indicates that the light pen switch is closed; */
/* a logical 1 indicates that the light pen switch is open.                            */
/*                                                                                     */
/* Bit 3: Vertical Retrace--A logical 0 indicates that video information is being      */
/* displayed on the CRT screen; a logical 1 indicates the CRT is in a vertical retrace */
/* interval. This bit can be programmed to interrupt the processor on interrupt level 2*/
/* at the start of the vertical retrace. This is done through bits 4 and 5 of the      */
/* Vertical Retrace End Register of the CRTC.                                          */
/*                                                                                     */
/* Bits 4 and 5: Diagnostic Usage--These bits are selectively connected to two of the  */
/* six color outputs of the Attribute Controller. The Color Plane Enable register      */
/* controls the multiplexer for the video wiring. The following table illustrates the  */
/* combinations available and the color output wiring.                                 */
/* Color Plane Register: Input Status Register One:                                    */
/* Bit 5: Bit 4:         Bit 5:            Bit 4:                                      */
/*  - 0:   - 0:           - Red:            - Blue:                                    */
/*  - 0:   - 1:           - Secondary Blue: - Green:                                   */
/*  - 1:   - 0:           - Secondary Red:  - Secondary Green                          */
/*  - 1:   - 1:           - Not Used:       - Not Used:                                */
/* ----------------------------------------------------------------------------------- */
	case EGA_STATUS1: /* 3DA */ /* Input Status Register One */
		break;

	default:
		if (addr < 0x30) {
			ega->reg[addr] = val;
		}
		break;
	}
}

static
void ega_reg_set_uint16 (ega_t *ega, unsigned long addr, unsigned short val)
{
	ega_reg_set_uint8 (ega, addr, val & 0xff);
	ega_reg_set_uint8 (ega, addr + 1, val >> 8);
}


static
int ega_set_msg (ega_t *ega, const char *msg, const char *val)
{
	if (msg_is_message ("emu.video.blink", msg)) {
		unsigned freq;

		if (msg_get_uint (val, &freq)) {
			return (1);
		}
		ega_set_blink_rate (ega, freq);
		return (0);
	}
	return (-1);
}

static
void ega_set_terminal (ega_t *ega, terminal_t *trm)
{
	ega->term = trm;

	if (ega->term != NULL) {
		trm_open (ega->term, 640, 400);
	}
}

static
mem_blk_t *ega_get_mem (ega_t *ega)
{
	return (ega->memblk);
}

static
mem_blk_t *ega_get_reg (ega_t *ega)
{
	return (ega->regblk);
}

static
void ega_print_regs (ega_t *ega, FILE *fp, const char *name, const unsigned char *reg, unsigned cnt)
{
	unsigned i;

	fprintf (fp, "%s=[%02X", name, reg[0]);

	for (i = 1; i < cnt; i++) {
		fputs ((i & 7) ? " " : "-", fp);
		fprintf (fp, "%02X", reg[i]);
	}

	fputs ("]\n", fp);
}

static
void ega_print_info (ega_t *ega, FILE *fp)
{
	fprintf (fp, "DEV: EGA\n");

	fprintf (fp, "EGA: ADDR=%04X  ROFS=%04X  CURS=%04X  LCMP=%04X\n",
		ega_get_start (ega),
		2 * ega->reg_crt[EGA_CRT_OFS],
		ega_get_cursor (ega),
		ega_get_line_compare (ega)
	);

	fprintf (fp, "CLK: CLK=%lu  HT=%lu HD=%lu  VT=%lu VD=%lu\n",
		ega_get_dotclock (ega),
		ega->clk_ht, ega->clk_hd,
		ega->clk_vt, ega->clk_vd
	);

	fprintf (fp, "MOUT=%02X  ST0=%02X  ST1=%02X\n",
		ega->reg[EGA_MOUT],    /* Miscellaneous Output Register */
		ega->reg[EGA_STATUS0], /* Input Status Register Zero    */
		ega->reg[EGA_STATUS1]  /* Input Status Register One     */
	);
	ega_print_regs (ega, fp, "CRT", ega->reg_crt, 25); /* See CRT       Controller */
	ega_print_regs (ega, fp, "ATC", ega->reg_atc, 22); /* See Attribute Controller */
	ega_print_regs (ega, fp, "SEQ", ega->reg_seq, 5);  /* See Sequencer            */
	ega_print_regs (ega, fp, "GRC", ega->reg_grc, 9);  /* See Graphics  Controller */
	fflush (fp);
}

/*
 * Force a screen update
 */
static
void ega_redraw (ega_t *ega, int now)
{
/* ------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Major Components:  */
/* ------------------------------------------------------------------------------ */
/* Display Buffer:                                                                */
/* The display buffer on the adapter consists of 64K bytes of dynamic read/write  */
/* memory configured as four 16K byte video bit planes. Two options are available */
/* for expanding the graphics memory. The Graphics Memory Expansion Card plugs    */
/* into the memory expansion connector on the adapter, and adds one bank of 16K   */
/* to each of the four bit planes, increasing the graphics memory to 128K bytes.  */
/* The expansion card also provides DIP sockets for further memory expansion.     */
/* Populating the DIP sockets with the Graphics Memory Module Kit adds two        */
/* additional 16K banks to each bit plane, bringing the graphics memory to its    */
/* maximum of 256K bytes.                                                         */
/* ------------------------------------------------------------------------------ */
	if (now) {
		if (ega->term != NULL) {
			ega_update (ega);
			trm_set_size (ega->term, ega->buf_w, ega->buf_h);
			trm_set_lines (ega->term, ega->buf, 0, ega->buf_h);
			trm_update (ega->term);
		}
	}
	ega->update_state |= EGA_UPDATE_DIRTY;
}

static
void ega_clock (ega_t *ega, unsigned long cnt)
{
	unsigned      addr;
	unsigned long clk;

	if (ega->clk_vt < 50000) {
		return;
	}
	clk = ega_get_dotclock (ega);
	if (clk < ega->clk_vd) {
		ega->update_state &= ~EGA_UPDATE_RETRACE;
		return;
	}
	if (clk >= ega->clk_vt) {
		ega->video.dotclk[0] = 0;
		ega->video.dotclk[1] = 0;
		ega->video.dotclk[2] = 0;
/* ------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Major Components:  */
/* ------------------------------------------------------------------------------ */
/* Attribute Controller:                                                          */
/* The Attribute Controller provides a color palette of 16 colors, each of which  */
/* may be specified separately. Six color outputs are available for driving a     */
/* display. Blinking and underlining are controlled by this chip. This chip takes */
/* data from the display memory and formats it for display on the CRT screen.     */
/*                                                                                */
/* Support Logic:                                                                 */
/* The logic on the card surrounding the LIS modules supports the modules and     */
/* creates latch buses for the CRT controller, the processor, and character       */
/* generator. Two clock sources (14 MHz and 16 MHz) provide the dot rate. The     */
/* clock is multiplexed under processor I/O control. Four I/O registers also      */
/* resident on the card are not part of the LSI devices.                          */
/* ------------------------------------------------------------------------------ */
		ega->latch_hpp = ega->reg_atc[EGA_ATC_HPP] & 0x0f;

		addr = ega_get_start (ega);

		if (ega->latch_addr != addr) {
			ega->latch_addr = addr;
			ega->update_state |= EGA_UPDATE_DIRTY;
		}
	}
	if (ega->update_state & EGA_UPDATE_RETRACE) {
		return;
	}
	if (ega->blink_cnt > 0) {
		ega->blink_cnt -= 1;

		if (ega->blink_cnt == 0) {
			ega->blink_cnt = ega->blink_freq;
			ega->blink_on = !ega->blink_on;
/* ------------------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Attribute Controller Registers:*/
/* ------------------------------------------------------------------------------------------ */
/* Name:                             Port:  Index:                                            */
/*  - Mode Control           Register: - 3C0: - 10:                                           */
/* Mode Control Register:                                                                     */
/* This is a write-only register pointed to by the value in the Attribute address register.   */
/* This value must be hex 10 before writing can take place. The processor output port address */
/* for this register is hex 3C0.                                                              */
/* Mode Control Register Format:                                                              */
/*  - Bit 0: Graphics/Alphanumeric Mode                                                       */
/*  - Bit 1: Display Type                                                                     */
/*  - Bit 2: Enable Line Graphics Character Codes                                             */
/*  - Bit 3: Select Background Intensity or Enable Blink                                      */
/*  - Bit 4: Not Used                                                                         */
/*  - Bit 5: Not Used                                                                         */
/*  - Bit 6: Not Used                                                                         */
/*  - Bit 7: Not Used                                                                         */
/* Bit 0: Graphics/Alphanumeric Mode--A logical 0 selects selects alphanumeric mode. A logical*/
/* 1 selects graphics mode.                                                                   */
/*                                                                                            */
/* Bit 3: Enable Blink/Select Background Intensity--A logical 0 selects the background        */
/* intensity of the attribute input. This mode was available on the Monochrome and Color      */
/* Graphics adapters. A logical 1 enables the blink attribute in alphanumeric modes. This bit */
/* must also be set to 1 for blinking graphics modes.                                         */
/* ------------------------------------------------------------------------------------------ */
			if ((ega->reg_atc[EGA_ATC_MODE] & EGA_ATC_MODE_G) == 0) { /* Mode Control Register: Graphics/Alphanumeric Mode */
				ega->update_state |= EGA_UPDATE_DIRTY;
			}
			else if (ega->reg_atc[EGA_ATC_MODE] & EGA_ATC_MODE_EB) {  /* Mode Control Register: Enable Blink/Select Background Intensity */
				ega->update_state |= EGA_UPDATE_DIRTY;
			}
		}
	}
	if (ega->term != NULL) {
		if (ega->update_state & EGA_UPDATE_DIRTY) {
			ega_update (ega);
			trm_set_size (ega->term, ega->buf_w, ega->buf_h);
			trm_set_lines (ega->term, ega->buf, 0, ega->buf_h);
		}
		trm_update (ega->term);
	}
	ega->update_state = EGA_UPDATE_RETRACE;
/* ------------------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: CRT Control Registers:         */
/* ------------------------------------------------------------------------------------------ */
/* Name:                       Port:  Index:                                                  */
/*  - Vertical Retrace End:     - 3?5: - 11                                                   */
/* ? = B in Monochrome Modes and D in Color Modes                                             */
/*                                                                                            */
/* Vertical Retrace End Register:                                                             */
/* This is a write-only register pointed to when the value in the CRT Controller address      */
/* register is hex 11. The processor output port address for this register is hex 3B5 or hex  */
/* 3D5.                                                                                       */
/* Vertical Retrace End Register Format:                                                      */
/*  - Bit 0: Vertical Retrace End                                                             */
/*  - Bit 1: Vertical Retrace End                                                             */
/*  - Bit 2: Vertical Retrace End                                                             */
/*  - Bit 3: Vertical Retrace End                                                             */
/*  - Bit 4: 0=Clear  Vertical Interrupt                                                      */
/*  - Bit 5: 0=Enable Vertical Interrupt                                                      */
/*  - Bit 6: Not Used                                                                         */
/*  - Bit 7: Not Used                                                                         */
/* Bit 0-Bit 3: Vertical Retrace End--These bits determine the horizontal scan count value    */
/* when the vertical retrace output signal becomes inactive. The register is programmed in    */
/* units of horizontal scan lines. To obtain a vertical retrace signal of width W, the        */
/* following algorithm is used: Value of Start Vertical Retrace Register + width of vertical  */
/* retrace signal in horizontal scan units = 4-bit result to be programmed into the End       */
/* Horizontal Retrace Register.                                                               */
/*                                                                                            */
/* Bit 4: Clear  Vertical Interrupt--A logical 0 will clear a vertical interrupt.             */
/*                                                                                            */
/* Bit 5: Enable Vertical Interrupt--A logical 0 will enable  vertical interrupt.             */
/* ------------------------------------------------------------------------------------------ */
	if ((ega->reg_crt[EGA_CRT_VRE] & EGA_CRT_VRE_EVI) == 0) { /* Vertical Retrace End Register: Enable Vertical Interrupt */
		/* vertical retrace interrupt enabled */
		ega_set_irq (ega, 1);
	}
}

void ega_set_irq_fct (ega_t *ega, void *ext, void *fct)
{
	ega->set_irq_ext = ext;
	ega->set_irq = fct;
}

void ega_free (ega_t *ega)
{
	mem_blk_del (ega->memblk);
	mem_blk_del (ega->regblk);
}

void ega_del (ega_t *ega)
{
	if (ega != NULL) {
		ega_free (ega);
		free (ega);
	}
}

void ega_init (ega_t *ega, unsigned long io, unsigned long addr)
{
	unsigned i;

	pce_video_init (&ega->video);

	ega->video.ext = ega;
	ega->video.del = (void *) ega_del;
	ega->video.set_msg = (void *) ega_set_msg;
	ega->video.set_terminal = (void *) ega_set_terminal;
	ega->video.get_mem = (void *) ega_get_mem;
	ega->video.get_reg = (void *) ega_get_reg;
	ega->video.print_info = (void *) ega_print_info;
	ega->video.redraw = (void *) ega_redraw;
	ega->video.clock = (void *) ega_clock;

	ega->term = NULL;

	ega->mem = malloc (256UL * 1024UL);
	if (ega->mem == NULL) {
		return;
	}
	ega->memblk = mem_blk_new (addr, 128UL * 1024UL, 0);
	ega->memblk->ext = ega;
	mem_blk_set_data (ega->memblk, ega->mem, 1);
	ega->memblk->set_uint8 = (void *) ega_mem_set_uint8;
	ega->memblk->set_uint16 = (void *) ega_mem_set_uint16;
	ega->memblk->get_uint8 = (void *) ega_mem_get_uint8;
	ega->memblk->get_uint16 = (void *) ega_mem_get_uint16;
	ega->mem = ega->memblk->data;
	mem_blk_clear (ega->memblk, 0x00);

	ega->regblk = mem_blk_new (io, 0x30, 1);
	ega->regblk->ext = ega;
	ega->regblk->set_uint8 = (void *) ega_reg_set_uint8;
	ega->regblk->set_uint16 = (void *) ega_reg_set_uint16;
	ega->regblk->get_uint8 = (void *) ega_reg_get_uint8;
	ega->regblk->get_uint16 = (void *) ega_reg_get_uint16;
	ega->reg = ega->regblk->data;
	mem_blk_clear (ega->regblk, 0x00);
	for (i = 0; i < 5; i++) {
		ega->reg_seq[i] = 0; /* Sequencer */
	}
	for (i = 0; i < 9; i++) {
		ega->reg_grc[i] = 0; /* Graphics Controller */
	}
	for (i = 0; i < 22; i++) {
		ega->reg_atc[i] = 0; /* Attribute Controller */
	}
	for (i = 0; i < 25; i++) {
		ega->reg_crt[i] = 0; /* CRT Controller */
	}
/* ------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Major Components:  */
/* ------------------------------------------------------------------------------ */
/* Support Logic:                                                                 */
/* The logic on the card surrounding the LIS modules supports the modules and     */
/* creates latch buses for the CRT controller, the processor, and character       */
/* generator. Two clock sources (14 MHz and 16 MHz) provide the dot rate. The     */
/* clock is multiplexed under processor I/O control. Four I/O registers also      */
/* resident on the card are not part of the LSI devices.                          */
/* ------------------------------------------------------------------------------ */
	for (i = 0; i < 4; i++) {
		ega->latch[i] = 0;
	}
	ega->latch_addr = 0;
	ega->latch_hpp = 0;

	ega->atc_flipflop = 0;

	ega->switches = 0x09;
	ega->monitor = EGA_MONITOR_ECD;

	ega->blink_on = 1;
	ega->blink_cnt = 0;
	ega->blink_freq = 16;

	ega->clk_ht = 0;
	ega->clk_hd = 0;
	ega->clk_vt = 0;
	ega->clk_vd = 0;

	ega->bufmax = 0;
	ega->buf = NULL;

	ega->update_state = 0;

	ega->set_irq_ext = NULL;
	ega->set_irq = NULL;
	ega->set_irq_val = 0;
}

ega_t *ega_new (unsigned long io, unsigned long addr)
{
	ega_t *ega;
	ega = malloc (sizeof (ega_t));
	if (ega == NULL) {
		return (NULL);
	}
	ega_init (ega, io, addr);
	return (ega);
}

video_t *ega_new_ini (ini_sct_t *sct)
{
	unsigned long io, addr;
	unsigned      switches;
	unsigned      blink;
	ega_t         *ega;
/* ------------------------------------------------------------------------------ */
/* Quote from IBM Enhanced Graphics Adapter, August 2nd, 1984: Major Components:  */
/* ------------------------------------------------------------------------------ */
/* The address of the display buffer can be changed to remain compatible with     */
/* other video cards and application software. Four locations are provided. The   */
/* buffer can be configured at segment address hex A0000 for a length of 128K     */
/* bytes, at hex A0000 for a length of 64K bytes, at hex B0000 for a length of 32K*/
/* bytes, or at hex B8000 for a length of 32K bytes.                              */
/* ------------------------------------------------------------------------------ */
	ini_get_uint32 (sct, "io", &io, 0x3b0);
	ini_get_uint32 (sct, "address", &addr, 0xa0000);
	ini_get_uint16 (sct, "switches", &switches, 0x09);
	ini_get_uint16 (sct, "blink", &blink, 0);

	pce_log_tag (MSG_INF,
		"VIDEO:", "EGA io=0x%04lx addr=0x%05lx switches=%02X\n",
		io, addr, switches
	);
	ega = ega_new (io, addr);
	if (ega == NULL) {
		return (NULL);
	}
	ega_set_switches (ega, switches);
	ega_set_blink_rate (ega, blink);
	return (&ega->video);
}
