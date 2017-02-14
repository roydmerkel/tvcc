/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/chipset/82xx/e8255.c                                     *
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


#include <stdlib.h>
#include <stdio.h>

#include "e8255.h"


void e8255_init (e8255_t *ppi)
{
	unsigned i;

	ppi->group_a_mode = 0;
	ppi->group_b_mode = 0;
	ppi->mode = 0x80;

	for (i = 0; i < 3; i++) {
		ppi->port[i].val_inp = 0;
		ppi->port[i].val_out = 0;
		ppi->port[i].inp = 0;
		ppi->port[i].read_ext = NULL;
		ppi->port[i].read = NULL;
		ppi->port[i].write_ext = NULL;
		ppi->port[i].write = NULL;
	}
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
}

e8255_t *e8255_new (void)
{
	e8255_t *ppi;

	ppi = malloc (sizeof (e8255_t));
	if (ppi == NULL) {
		return (NULL);
	}

	e8255_init (ppi);

	return (ppi);
}

void e8255_free (e8255_t *ppi)
{
}

void e8255_del (e8255_t *ppi)
{
	if (ppi != NULL) {
		e8255_free (ppi);
		free (ppi);
	}
}

void e8255_set_inp (e8255_t *ppi, unsigned p, unsigned char val)
{
	ppi->port[p].val_inp = val;
}

void e8255_set_out (e8255_t *ppi, unsigned p, unsigned char val)
{
	ppi->port[p].val_out = val;

	if (ppi->port[p].inp != 0xff) {
		val &= ~ppi->port[p].inp;
		if (ppi->port[p].write != NULL) {
			ppi->port[p].write (ppi->port[p].write_ext, val);
		}
	}
}

unsigned char e8255_get_inp (e8255_t *ppi, unsigned p)
{
	unsigned char val;

	if (ppi->port[p].inp != 0x00) {
		if (ppi->port[p].read != NULL) {
			ppi->port[p].val_inp = ppi->port[p].read (ppi->port[p].read_ext);
		}
	}

	val = ppi->port[p].val_inp & ppi->port[p].inp;
	val |= ppi->port[p].val_out & ~ppi->port[p].inp;

	return (val);
}

unsigned char e8255_get_out (e8255_t *ppi, unsigned p)
{
	return (ppi->port[p].val_out | ppi->port[p].inp);
}


void e8255_set_inp_a (e8255_t *ppi, unsigned char val)
{
	e8255_set_inp (ppi, 0, val);
}

void e8255_set_inp_b (e8255_t *ppi, unsigned char val)
{
	e8255_set_inp (ppi, 1, val);
}

void e8255_set_inp_c (e8255_t *ppi, unsigned char val)
{
	e8255_set_inp (ppi, 2, val);
}

void e8255_set_out_a (e8255_t *ppi, unsigned char val)
{
	e8255_set_out (ppi, 0, val);
}

void e8255_set_out_b (e8255_t *ppi, unsigned char val)
{
	e8255_set_out (ppi, 1, val);
}

void e8255_set_out_c (e8255_t *ppi, unsigned char val)
{
	e8255_set_out (ppi, 2, val);
}

unsigned char e8255_get_inp_a (e8255_t *ppi)
{
	return (e8255_get_inp (ppi, 0));
}

unsigned char e8255_get_inp_b (e8255_t *ppi)
{
	return (e8255_get_inp (ppi, 1));
}

unsigned char e8255_get_inp_c (e8255_t *ppi)
{
	return (e8255_get_inp (ppi, 2));
}

unsigned char e8255_get_out_a (e8255_t *ppi)
{
	return (e8255_get_out (ppi, 0));
}

unsigned char e8255_get_out_b (e8255_t *ppi)
{
	return (e8255_get_out (ppi, 1));
}

unsigned char e8255_get_out_c (e8255_t *ppi)
{
	return (e8255_get_out (ppi, 2));
}

void e8255_set_uint8 (e8255_t *ppi, unsigned long addr, unsigned char val)
{
	switch (addr) {
		case 0:
			e8255_set_out (ppi, 0, val);
			break;

		case 1:
			e8255_set_out (ppi, 1, val);
			break;

		case 2:
			e8255_set_out (ppi, 2, val);
			break;

		case 3:
			if (val & 0x80) {
				ppi->mode = val;

				ppi->group_a_mode = (val >> 5) & 0x03;
				ppi->group_b_mode = (val >> 2) & 0x01;
				ppi->port[0].inp = (val & 0x10) ? 0xff : 0x00;
				ppi->port[1].inp = (val & 0x02) ? 0xff : 0x00;
				ppi->port[2].inp = (val & 0x01) ? 0x0f : 0x00;
				ppi->port[2].inp |= (val & 0x08) ? 0xf0 : 0x00;
			}
			else {
				unsigned bit;

				bit = (val >> 1) & 0x07;

				if (val & 1) {
					val = ppi->port[2].val_out | (1 << bit);
				}
				else {
					val = ppi->port[2].val_out & ~(1 << bit);
				}

				e8255_set_out (ppi, 2, val);
			}

			break;
	}
}

void e8255_set_uint16 (e8255_t *ppi, unsigned long addr, unsigned short val)
{
	e8255_set_uint8 (ppi, addr, val & 0xff);
}

void e8255_set_uint32 (e8255_t *ppi, unsigned long addr, unsigned long val)
{
	e8255_set_uint8 (ppi, addr, val & 0xff);
}

unsigned char e8255_get_uint8 (e8255_t *ppi, unsigned long addr)
{
	switch (addr) {
		case 0:
			return (e8255_get_inp (ppi, 0));

		case 1:
			return (e8255_get_inp (ppi, 1));

		case 2:
			return (e8255_get_inp (ppi, 2));

		case 3:
			return (ppi->mode);
	}

	return (0);
}

unsigned short e8255_get_uint16 (e8255_t *ppi, unsigned long addr)
{
	return (e8255_get_uint8 (ppi, addr));
}

unsigned long e8255_get_uint32 (e8255_t *ppi, unsigned long addr)
{
	return (e8255_get_uint8 (ppi, addr));
}
