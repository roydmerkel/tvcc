/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/chipset/82xx/e8255.h                                     *
 * Created:     2003-04-17 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2009 Hampa Hug <hampa@hampa.ch>                     *
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


/* PPI 8255 */
/* only mode 0 is supported */

/* --------------------------------------------------------------------------------------- */
/* IBM PC (Model 5150) Technical Reference:                                                */
/* --------------------------------------------------------------------------------------- */
/* 8255A I/O Bit Map:                                                                      */
/* --------------------------------------------------------------------------------------- */
/* The 8255A I/O Bit Map shows the inputs and outputs for the Command/Mode register on the */
/* system board. Also shown are the switch settings for the memory, display, and number of */
/* diskette drives. The following page contains the I/O bit map.                           */
/* --------------------------------------------------------------------------------------- */
/* Note: A plus (+) indicates a bit value of 1 performs the specified function.            */
/*       A minus (-) indicates a bit value of 0 performs the specified function.           */
/*       PA Bit = 0 implies switch ''ON.'' PA bit = 1 implies switch ''OFF.''              */
/* --------------------------------------------------------------------------------------- */
/* Hex Port Number 0060:                                                                   */
/* - INPUT:                                                                                */
/* - PA0:                  12 3 4567                                                       */
/* - +Keyboard Scan Code: 012 3 4567                                                       */
/* Or:                                                                                     */
/* - IPL 5-1/4 Diskette Drive             (SW1-1)                                          */
/* - Reserved                             (SW1-2)                                          */
/* - System Board Read/Write Memory Size *(SW1-3)                                          */
/* - System Board Read/Write Memory Size *(SW1-4)                                          */
/* - +Display Type 1                    **(SW1-5)                                          */
/* - +Display Type 2                    **(SW1-6)                                          */
/* - No. of 5-1/4 Drives               ***(SW1-7)                                          */
/* - No. of 5-1/4 Drives               ***(SW1-8)                                          */
/* Hex Port Number 0061:                                                                   */
/* - OUTPUT:                                                                               */
/* - PB0: +Timer 2 Gate Speaker                                                            */
/* -   1: +Speaker Data                                                                    */
/* -   2: +(Read Read/Write Memory Size) or (Read Spare Key)                               */
/* -   3: +Cassette Motor Off                                                              */
/* -   4: -Enable Read/Write Memory                                                        */
/* -   5: -Enable I/O Channel Check                                                        */
/* -   6: -Hold Keyboard Clock Low                                                         */
/* -   7: -(Enable Keyboard or + (Clear Keyboard and Enable Sense Switches) )              */
/* Hex Port Number 0062:                                                                   */
/* - INPUT:                                                                                */
/* - PC0: I/O Read/Write Memory (Sw2-1) Or I/O Read/Write Memory (Sw2-5)                   */
/* -   1: I/O Read/Write Memory (Sw2-2) -- Binary Value X 32K -- Or I/O Read/Write Memory (Sw2-5) */
/* -   2: I/O Read/Write Memory (Sw2-3) Or I/O Read/Write Memory (Sw2-5)                   */
/* -   3: I/O Read/Write Memory (Sw2-4) Or I/O Read/Write Memory (Sw2-5)                   */
/* -   4: +Cassette Data In                                                                */
/* -   5: +Timer Channel 2 Out                                                             */
/* -   6: +I/O Channel Check                                                               */
/* -   7: +Read/Write Memory Parity Check                                                  */
/* Hex Port Number 0063:                                                                   */
/* - Command/Mode Register:                                                                */
/* - - Mode Register Value:                                                                */
/* - - - Hex 99:                                                                           */
/* - - - - 76543210                                                                        */
/* - - - - 10011001                                                                        */
/* --------------------------------------------------------------------------------------- */
/* *   PA3:    PA2:    Amount of Memory                                                    */
/*    Sw1-4:  Sw1-3:   Located on System Board                                             */
/*      1       1             64 to 256K                                                   */
/* --------------------------------------------------------------------------------------- */
/* **  PA5:    PA4:    Display at Power-Up Mode                                            */
/*    Sw1-6:  Sw1-5:                                                                       */
/*      0       0      Reserved                                                            */
/*      0       1      Color 40 X 25 (BW Mode)                                             */
/*      1       0      Color 80 X 25 (BW Mode)                                             */
/*      1       1      IBM Monochrome (80 X 25)                                            */
/* --------------------------------------------------------------------------------------- */
/* *** PA7:    PA6:    Number of 5-1/4" Drives                                             */
/*    Sw1-8:  Sw1-7:   in System:                                                          */
/*      0       0                1                                                         */
/*      0       1                2                                                         */
/*      1       0                3                                                         */
/*      1       1                4                                                         */
/* --------------------------------------------------------------------------------------- */
/* NOTE: A plus (+) indicates a bit value of 1 performs the specified function.            */
/*       A minus (-) indicates a bit value of 0 performs the specified function.           */
/*       PA Bit = 0 implies switch ''ON.'' PA bit = 1 implies switch ''OFF.''              */
/* --------------------------------------------------------------------------------------- */
/* 8255A I/O Bit Map:                                                                      */
/* --------------------------------------------------------------------------------------- */

#ifndef PCE_E8255_H
#define PCE_E8255_H 1


typedef struct {
	unsigned char val_inp;
	unsigned char val_out;

	unsigned char inp;

	void          *read_ext;
	unsigned char (*read) (void *ext);

	void          *write_ext;
	void          (*write) (void *ext, unsigned char val);
} e8255_port_t;


typedef struct {
	unsigned char group_a_mode;
	unsigned char group_b_mode;

	unsigned char mode;

	e8255_port_t  port[3];
} e8255_t;


void e8255_init (e8255_t *ppi);
e8255_t *e8255_new (void);
void e8255_free (e8255_t *ppi);
void e8255_del (e8255_t *ppi);

void e8255_set_inp (e8255_t *ppi, unsigned p, unsigned char val);
void e8255_set_out (e8255_t *ppi, unsigned p, unsigned char val);
unsigned char e8255_get_inp (e8255_t *ppi, unsigned p);
unsigned char e8255_get_out (e8255_t *ppi, unsigned p);

void e8255_set_inp_a (e8255_t *ppi, unsigned char val);
void e8255_set_inp_b (e8255_t *ppi, unsigned char val);
void e8255_set_inp_c (e8255_t *ppi, unsigned char val);

void e8255_set_out_a (e8255_t *ppi, unsigned char val);
void e8255_set_out_b (e8255_t *ppi, unsigned char val);
void e8255_set_out_c (e8255_t *ppi, unsigned char val);

unsigned char e8255_get_inp_a (e8255_t *ppi);
unsigned char e8255_get_inp_b (e8255_t *ppi);
unsigned char e8255_get_inp_c (e8255_t *ppi);

unsigned char e8255_get_out_a (e8255_t *ppi);
unsigned char e8255_get_out_b (e8255_t *ppi);
unsigned char e8255_get_out_c (e8255_t *ppi);

void e8255_set_uint8 (e8255_t *ppi, unsigned long addr, unsigned char val);
void e8255_set_uint16 (e8255_t *ppi, unsigned long addr, unsigned short val);
void e8255_set_uint32 (e8255_t *ppi, unsigned long addr, unsigned long val);

unsigned char e8255_get_uint8 (e8255_t *ppi, unsigned long addr);
unsigned short e8255_get_uint16 (e8255_t *ppi, unsigned long addr);
unsigned long e8255_get_uint32 (e8255_t *ppi, unsigned long addr);


#endif
