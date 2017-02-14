/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/parport.c                                        *
 * Created:     2003-04-29 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2011 Hampa Hug <hampa@hampa.ch>                     *
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

#include "memory.h"
#include "parport.h"


/* --------------------------------------------------------------------- */
/* IBM Monochrome Display and Printer Adapter: Printer Adapter Function: */
/* --------------------------------------------------------------------- */
/* Description:                                                          */
/* --------------------------------------------------------------------- */
/* The printer adapter portion of the IBM Monochrome Display and Printer */
/* adapter is specifically designed to attach printers with a parallel-  */
/* port interface, but it can be used as a general input/output port for */
/* any device or application that matches its input/output capabilities. */
/* It has 12 TTL-buffer output points, which are latched and can be      */
/* written and read under program control using the microprocessor In or */
/* Out instruction. The adapter also has five steady-state input points  */
/* that may be read using the microprocessor's In instructions.          */
/*                                                                       */
/* In addition, one input can also be used to create a microprocessor    */
/* interrupt. This interrupt can be enabled and disabled under program   */
/* control. A reset from the power-on circuit is also ORed with a program*/
/* output point, allowing a device to receive a 'power-on reset' when the*/
/* system unit's microprocessor is reset.                                */
/*                                                                       */
/* The input/output signals are made available at the back of the adapter*/
/* through a right-angle, printed-circuit-board-mounted, 25-pin, D-shell */
/* connector. This connector protrudes through the rear panel of the     */
/* system unit or expansion unit, where a cable may be attached.         */
/*                                                                       */
/* When this adapter is used to attach a printer, data or printer        */
/* commands are loaded into an 8-bit, latched, output port, and the      */
/* strobe line is activated, writing data to the printer. The program    */
/* then may read the input ports for printer status indicating when the  */
/* next character can be written, or it may use the interrupt line to    */
/* indicate "not busy" to the software.                                  */
/*                                                                       */
/* The output ports may also be read at the card's interface for         */
/* diagnostic loop functions. This allows faults to be isolated to the   */
/* adapter or the attaching device.                                      */
/* --------------------------------------------------------------------- */
/* The following is a block diagram of the printer adapter portion of the*/
/* Monochrome Display and Printer Adapter.                               */
/* ----INSERT FILE NAME HERE----                                         */
/* --------------------------------------------------------------------- */
/* Programming Considerations:                                           */
/* --------------------------------------------------------------------- */
/* The printer adapter portion of the IBM Monochrome Display and Printer */
/* Adapter responds to five I/O instructions: two output and three input.*/
/* The output instructions transfer data into 2 latches whose outputs are*/
/* presented on pins of a 25-pin D-shell connector.                      */
/*                                                                       */
/* Two of the three input instructions allow the system unit's           */
/* microprocessor to read back the contents of the two latches. The third*/
/* allows the system unit's microprocessor to read the real-time status  */
/* from a group of pins on the connector.                                */
/*                                                                       */
/* A description of each instruction follows.                            */
/* --------------------------------------------------------------------- */
/* IBM Monochrome Display & Printer Adapter:                             */
/* --------------------------------------------------------------------- */
/* Output to address hex 3BC:                                            */
/* --------------------------------------------------------------------- */
/* - Pin 9: Bit 7                                                        */
/* - Pin 8: Bit 6                                                        */
/* - Pin 7: Bit 5                                                        */
/* - Pin 6: Bit 4                                                        */
/* The instruction captures data from the data bus and is present on the */
/* respective pins. Each of these pins is capable of sourcing 2.6 mA and */
/* sinking 24 mA.                                                        */
/*                                                                       */
/* It is essential that the external device does not try to pull these   */
/* lines to ground.                                                      */
/* --------------------------------------------------------------------- */
/* Output to address hex 3BE:                                            */
/* --------------------------------------------------------------------- */
/* - IRQ Enable: Bit 4                                                   */
/* This instruction causes the latch to capture the five least           */
/* significant bits of the data bus. The four least significant bits     */
/* present their outputs, or inverted versions of their outputs, to the  */
/* respective pins as shown in the previous figure. If bit 4 is written  */
/* as a 1, the card will interrupt the system unit's microprocessor on   */
/* the condition that pin 10 changes from high to low.                   */
/*                                                                       */
/* These pins are driven by open-collector drivers pulled to +5 Vdc      */
/* through 4.7 kX resistors. They can each sink approximately 7 mA and   */
/* maintain 0.8 volts down-level.                                        */
/* --------------------------------------------------------------------- */
/* Input from address hex 3BC:                                           */
/* --------------------------------------------------------------------- */
/* This instruction presents the system unit's microprocessor with data  */
/* present on the pins associated with the output to hex 3BC. This should*/
/* normally reflect the exact value that was last written to hex 3BC. If */
/* an external device should be driving data on these pins at the time of*/
/* an input (in violation of usage ground rules), this data will be ORed */
/* with the latch contents.                                              */
/* --------------------------------------------------------------------- */
/* Input from address hex 3BD:                                           */
/* --------------------------------------------------------------------- */
/* This instruction presents the real-time status to the system unit's   */
/* microprocessor from the pins as follows.                              */
/*  - Bit 7: Pin 11                                                      */
/*  - Bit 6: Pin 10                                                      */
/*  - Bit 5: Pin 12                                                      */
/*  - Bit 4: Pin 13                                                      */
/*  - Bit 3: Pin 15                                                      */
/*  - Bit 2: -                                                           */
/*  - Bit 1: -                                                           */
/*  - Bit 0: -                                                           */
/* --------------------------------------------------------------------- */
/* Input from address hex 3BE:                                           */
/* --------------------------------------------------------------------- */
/* This instruction causes the data present on pins 1, 14, 16, 17, and   */
/* the IRQ bit to be read by the system unit's microprocessor. In the    */
/* absence of external drive applied to these pins, data read by the     */
/* system unit's microprocessor will match data last written to hex 3BE  */
/* in the same bit positions. Notice that data bits 0-2 are not included.*/
/* If external drivers are dotted to these pins, that data will be ORed  */
/* with data applied to the pins by the hex 3BE latch.                   */
/*  - Bit 7:                                                             */
/*  - Bit 6:                                                             */
/*  - Bit 5:                                                             */
/*  - Bit 4: IRQ Enable: Por = 0                                         */
/*  - Bit 3: Pin 17    : Por = 1                                         */
/*  - Bit 2: Pin 16    : Por = 0                                         */
/*  - Bit 1: Pin 14    : Por = 1                                         */
/*  - Bit 0: Pin  1    : Por = 1                                         */
/* These pins assume the states shown after a reset from the system      */
/* unit's microprocessor.                                                */
/* --------------------------------------------------------------------- */
/* Specifications:                                                       */
/* --------------------------------------------------------------------- */
/* Connector Specifications:                                             */
/* --------------------------------------------------------------------- */
/*    _____________________________                                      */
/* 13 \ o o o o o o o o o o o o o /  1                                   */
/* 14  \ o o o o o o o o o o o o /  25                                   */
/*      \-----------------------/                                        */
/*         At Standard TTL Levels: Pin Number:                           */
/* Printer: - -Strobe              : -  1      IBM Monochrome Display and*/
/* Printer: - +Data Bit 0          : -  2      Printer Adapter           */
/* Printer: - +Data Bit 1          : -  3      IBM Monochrome Display and*/
/* Printer: - +Data Bit 2          : -  4      Printer Adapter           */
/* Printer: - +Data Bit 3          : -  5      IBM Monochrome Display and*/
/* Printer: - +Data Bit 4          : -  6      Printer Adapter           */
/* Printer: - +Data Bit 5          : -  7      IBM Monochrome Display and*/
/* Printer: - +Data Bit 6          : -  8      Printer Adapter           */
/* Printer: - +Data Bit 7          : -  9      IBM Monochrome Display and*/
/* Printer: - -Acknowledge         : - 10      Printer Adapter           */
/* Printer: - +Busy                : - 11      IBM Monochrome Display and*/
/* Printer: - +P.End (out of paper): - 12      Printer Adapter           */
/* Printer: - +Select              : - 13      IBM Monochrome Display and*/
/* Printer: - -Auto Feed           : - 14      Printer Adapter           */
/* Printer: - -Error               : - 15      IBM Monochrome Display and*/
/* Printer: - -Initialize Printer  : - 16      Printer Adapter           */
/* Printer: - -Select Input        : - 17      IBM Monochrome Display and*/
/* Printer: - Ground               : - 18-25   Printer Adapter           */
/* --------------------------------------------------------------------- */
/* Logic Diagrams:                                                       */
/* --------------------------------------------------------------------- */
/* Monochrome Display Adapter (Sheet  1 of 10):                          */
/* Monochrome Display Adapter (Sheet  2 of 10):                          */
/* Monochrome Display Adapter (Sheet  3 of 10):                          */
/* Monochrome Display Adapter (Sheet  4 of 10):                          */
/* Monochrome Display Adapter (Sheet  5 of 10):                          */
/* Monochrome Display Adapter (Sheet  6 of 10):                          */
/* Monochrome Display Adapter (Sheet  7 of 10):                          */
/* Monochrome Display Adapter (Sheet  8 of 10):                          */
/* Monochrome Display Adapter (Sheet  9 of 10):                          */
/* Monochrome Display Adapter (Sheet 10 of 10):                          */
/*  - Please see included files for details (NOTE: Will be included in   */
/*  upcoming release)                                                    */
/* --------------------------------------------------------------------- */
void parport_init (parport_t *par, unsigned long base)
{
	par->io = base;

	mem_blk_init (&par->port, base, 4, 0);
	par->port.ext = par;
	par->port.get_uint8 = (mem_get_uint8_f) &parport_get_uint8;
	par->port.set_uint8 = (mem_set_uint8_f) &parport_set_uint8;
	par->port.get_uint16 = (mem_get_uint16_f) &parport_get_uint16;
	par->port.set_uint16 = (mem_set_uint16_f) &parport_set_uint16;

	par->status = 0xdf;
	par->control = 0;
	par->data = 0;

	par->cdrv = NULL;
}

parport_t *parport_new (unsigned long base)
{
	parport_t *par;

	par = malloc (sizeof (parport_t));
	if (par == NULL) {
		return (NULL);
	}

	parport_init (par, base);

	return (par);
}

void parport_free (parport_t *par)
{
	chr_close (par->cdrv);

	mem_blk_free (&par->port);
}

void parport_del (parport_t *par)
{
	if (par != NULL) {
		parport_free (par);
		free (par);
	}
}

mem_blk_t *parport_get_reg (parport_t *par)
{
	return (&par->port);
}

int parport_set_driver (parport_t *par, const char *name)
{
	if (par->cdrv != NULL) {
		chr_close (par->cdrv);
	}

	par->cdrv = chr_open (name);

	if (par->cdrv == NULL) {
		par->status &= ~(PARPORT_BSY | PARPORT_OFF | PARPORT_ERR);
	}
	else {
		par->status |= (PARPORT_BSY | PARPORT_OFF | PARPORT_ERR);
	}

	if (par->cdrv == NULL) {
		return (1);
	}

	return (0);
}

void parport_set_control (parport_t *par, unsigned char val)
{
	par->control = val;

	if (val & PARPORT_STR) {
		chr_write (par->cdrv, &par->data, 1);

		par->status |= PARPORT_BSY;
		par->status &= ~PARPORT_ACK;
	}
	else {
		par->status |= PARPORT_ACK;
	}
}

void parport_set_uint8 (parport_t *par, unsigned long addr, unsigned char val)
{
	switch (addr) {
	case 0x00:
		par->data = val;
		break;

	case 0x01:
		break;

	case 0x02:
		parport_set_control (par, val);
		break;
	}
}

void parport_set_uint16 (parport_t *par, unsigned long addr, unsigned short val)
{
	parport_set_uint8 (par, addr, val);
}

unsigned char parport_get_uint8 (parport_t *par, unsigned long addr)
{
	switch (addr) {
	case 0x00:
		return (par->data);

	case 0x01:
		return (par->status);

	case 0x02:
		return (par->control);

	default:
		return (0xff);
	}
}

unsigned short parport_get_uint16 (parport_t *par, unsigned long addr)
{
	return (parport_get_uint8 (par, addr));
}
