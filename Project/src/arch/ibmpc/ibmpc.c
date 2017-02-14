/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/ibmpc/ibmpc.c                                       *
 * Created:     1999-04-16 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 1999-2012 Hampa Hug <hampa@hampa.ch>                     *
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


#include "main.h"
#include "ibmpc.h"
#include "ibmps2.h"
#include "m24.h"
#include "msg.h"

#include <stdlib.h>
#include <string.h>

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include <lib/brkpt.h>
#include <lib/inidsk.h>
#include <lib/iniram.h>
#include <lib/initerm.h>
#include <lib/load.h>
#include <lib/log.h>
#include <lib/string.h>
#include <lib/sysdep.h>

#include <chipset/82xx/e8237.h> /* Intel 8237 DMA (Direct Memory Access) Controller */
#include <chipset/82xx/e8250.h> /* Intel 8250 UART (Universal Asynchronous Receiver/Transmitter) Controller */
#include <chipset/82xx/e8253.h> /* Intel 8253 PIT (Programmable Interval Timer) Controller */
#include <chipset/82xx/e8255.h> /* Intel 8255 PPI (Programmable Peripheral Interface) Controller */
#include <chipset/82xx/e8259.h> /* Intel 8259 PIC (Programmable Interrupt Controller) */
#include <chipset/82xx/e8272.h> /* Intel 8272 FDC (Floppy Disk Controller) */

#include <cpu/e8086/e8086.h> /* Intel 8086 CPU (Central Processing Unit) */
/* #include <cpu/e8087/e8087.h> */ /* Disabled by default for current release */

#include <devices/fdc.h>
#include <devices/hdc.h>
#include <devices/memory.h>
#include <devices/nvram.h>
#include <devices/parport.h>
#include <devices/serport.h>
#include <devices/serport8.h>
#include <devices/video/mda.h>
#include <devices/video/hgc.h>
#include <devices/video/cga.h>
#include <devices/video/ega.h>
#include <devices/video/vga.h>
#include <devices/video/olivetti.h>
#include <devices/video/plantronics.h>
#include <devices/video/wy700.h>
/* #include <devices/video/compaq.h> */ /* Disabled by default for current release */

#include <drivers/block/block.h>
#include <drivers/video/terminal.h>
#include <libini/libini.h>

#define PCE_IBMPC_SLEEP 25000
void pc_e86_hook (void *ext, unsigned char op1, unsigned char op2);
static char *par_intlog[256];

static
unsigned char pc_get_port8 (ibmpc_t *pc, unsigned long addr)
{
	unsigned char val;
	if (m24_get_port8 (pc, addr, &val) == 0) {
		return (val);
	}
	else if (ibmps2_get_port8(pc, addr, &val) == 0) {
		return (val);
	}
	/* ------------------------------------------------------------------- */
	/* IBM PC (Model 5150/5160): Quote from "Technical Reference"; DMA:    */
	/* ------------------------------------------------------------------- */
	/* The microprocessor is supported by a set of high-function support   */
	/* devices providing four channels of 20-bit direct-memory access      */
	/* (DMA), three 16-bit timer/counter channels, and eight               */
	/* prioritized interrupt levels.                                       */
	/*                                                                     */
	/* Three of the four DMA channels are available on the I/O bus and     */
	/* support high-speed data transfers between I/O devices and           */
	/* memory without microprocessor intervention. The fourth DMA          */
	/* channel is programmed to refresh the system dynamic memory.         */
	/* This is done by programming a channel of the timer/counter          */
	/* device to periodically request a dummy DMA transfer. This           */
	/* action creates a memory-read cycle, which is available to refresh   */
	/* dynamic storage both on the system board and in the system          */
	/* expansion slots. All DMA data transfers, except the refresh         */
	/* channel, take five microprocessor clocks of 210-ns, or 1.05-us if   */
	/* the microprocessor ready line is not deactivated. Refresh DMA       */
	/* cycles take four clocks or 840-ns.                                  */
	/* ------ Listed under System Interrupts for Model 5160 Manual: ------ */
	/* Of the eight prioritized levels of interrupt, six are bussed to the */
	/* system expansion slots for use by feature cards. Two levels are     */
	/* used on the system board. Level 0, the highest priority, is         */
	/* attached to Channel 0 of the timer/counter device and provides a    */
	/* periodic interrupt for the time-of-day clock. Level 1 is attached   */
	/* to the keyboard adapter circuits and receives an interrupt for each */
	/* scan code sent by the keyboard. The non-maskable interrupt          */
	/* (NMI) of the 8088 is used to report memory parity errors.           */
	/* ------------------------------------------------------------------- */
	/* --------------------------------------------------------------------------------------- */
	/* IBM PC (Model 5150/5160): Quote from "Technical Reference"; I/O Channel Description:    */
	/* --------------------------------------------------------------------------------------- */
	/* Signal Name:                                                                            */
	/*  - +DRQ2 (Input): DMA Request 1 to 3: These lines are asynchronous channel requests     */
	/*  used by peripheral devices to gain DMA service. They are prioritized with DRQ3 being   */
	/*  the lowest and DRQ1 being the highest. A DRQ line must be held high until the          */
	/*  corresponding DACK line goes active.                                                   */
	/* ------------------------------------------------------------------------------------------ */
	/* IBM Personal Computer Model 25/30: Quote from "Technical Reference"; I/O Channel; Signal   */
	/* Description:                                                                               */
	/* ------------------------------------------------------------------------------------------ */
	/* The following is a description of the I/O channel signal lines. All lines are TTL-         */
	/* compatible. The (O), (I), or (I/O) notation refers to output, input, or input and output.  */
	/*  - DRQ1-DRQ3 (I): DMA Request 1 to 3: These lines are asynchronous channel requests used by*/
	/*  peripheral devices to gain DMA ser-vices. They are prioritized with DRQ1 being the highest*/
	/*  and DRQ3 being the lowest. A request is generated by bringing a request line to an active */
	/*  level. A request line is held active until the corresponding acknowledge line goes active.*/
	/* ------------------------------------------------------------------------------------------ */
	val = 0xff;
	switch (addr) {
	case 0x0081:
		val = pc->dma_page[2] >> 16;
		break;
	case 0x0082:
		val = pc->dma_page[3] >> 16;
		break;
	case 0x0083:
		val = pc->dma_page[1] >> 16;
		break;
	case 0x0087:
		val = pc->dma_page[0] >> 16;
		break;
	}
#ifdef DEBUG_PORTS
	pc_log_deb ("get port 8 %04lX <- %02X\n", addr, val);
#endif
	return (val);
	/* ------------------------------------------------------------------------------------------ */
	/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; DMA Controller:       */
	/* ------------------------------------------------------------------------------------------ */
	/* The 8237 Direct Memory Access (DMA) controller and its support logic in the gate array     */
	/* support four channels of 20 address bit DMA. It operates at 4 MHz and handles only 8-bit   */
	/* data transfers. The DMA channel assignments and page register addresses are:               */
	/* ------------------------------------------------------------------------------------------ */
	/* Level:  Assignment:                                                                        */
	/*  - DRQ0: - Not Available                                                                   */
	/*  - DRQ1: - Not Used                                                                        */
	/*  - DRQ2: - Diskette                                                                        */
	/*  - DRQ3: - Fixed Disk                                                                      */
	/* ------------------------------------------------------------------------------------------ */
	/* Figure 1-7. DMA Channel Assignments:                                                       */
	/* ------------------------------------------------------------------------------------------ */
	/* Hex Address: DMA Page Register:                                                            */
	/*  - 081:       - Channel 2:                                                                 */
	/*  - 082:       - Channel 3:                                                                 */
	/*  - 083:       - Channel 1:                                                                 */
	/*  - 087:       - Channel 0:                                                                 */
	/* ------------------------------------------------------------------------------------------ */
	/* Figure 1-8. DMA Page Register Addresses:                                                   */
	/* ------------------------------------------------------------------------------------------ */
	/* Three of the DMA channels (1, 2, and 3) are available on the I/O bus and support high-speed*/
	/* data transfers between I/O devices and memory without microprocessor intervention.         */
	/*                                                                                            */
	/* DMA data transfers take six clock cycles of 250 ns, or 1.5 micro-seconds. I/O CH RDY can be*/
	/* pulled inactive to add wait states to allow more time for slower devices.                  */
	/* ------------------------------------------------------------------------------------------ */
}

static
unsigned short pc_get_port16 (ibmpc_t *pc, unsigned long addr)
{
#ifdef DEBUG_PORTS
	pc_log_deb ("get port 16 %04lX\n", addr);
#endif
	return (0xffff);
}

static
void pc_set_port8 (ibmpc_t *pc, unsigned long addr, unsigned char val)
{
#ifdef DEBUG_PORTS
	pc_log_deb ("set port 8 %04lX <- %02X\n", addr, val);
#endif
	if (m24_set_port8 (pc, addr, val) == 0) {
		return;
	}
	else if (ibmps2_set_port8(pc, addr, val) == 0) {
		return;
	}
	/* ------------------------------------------------------------------- */
	/* IBM PC (Model 5150/5160): Quote from "Technical Reference"; DMA:    */
	/* ------------------------------------------------------------------- */
	/* The microprocessor is supported by a set of high-function support   */
	/* devices providing four channels of 20-bit direct-memory access      */
	/* (DMA), three 16-bit timer/counter channels, and eight               */
	/* prioritized interrupt levels.                                       */
	/*                                                                     */
	/* Three of the four DMA channels are available on the I/O bus and     */
	/* support high-speed data transfers between I/O devices and           */
	/* memory without microprocessor intervention. The fourth DMA          */
	/* channel is programmed to refresh the system dynamic memory.         */
	/* This is done by programming a channel of the timer/counter          */
	/* device to periodically request a dummy DMA transfer. This           */
	/* action creates a memory-read cycle, which is available to refresh   */
	/* dynamic storage both on the system board and in the system          */
	/* expansion slots. All DMA data transfers, except the refresh         */
	/* channel, take five microprocessor clocks of 210-ns, or 1.05-us if   */
	/* the microprocessor ready line is not deactivated. Refresh DMA       */
	/* cycles take four clocks or 840-ns.                                  */
	/* ------ Listed under System Interrupts for Model 5160 Manual: ------ */
	/* Of the eight prioritized levels of interrupt, six are bussed to the */
	/* system expansion slots for use by feature cards. Two levels are     */
	/* used on the system board. Level 0, the highest priority, is         */
	/* attached to Channel 0 of the timer/counter device and provides a    */
	/* periodic interrupt for the time-of-day clock. Level 1 is attached   */
	/* to the keyboard adapter circuits and receives an interrupt for each */
	/* scan code sent by the keyboard. The non-maskable interrupt          */
	/* (NMI) of the 8088 is used to report memory parity errors.           */
	/* ------------------------------------------------------------------- */
	/* --------------------------------------------------------------------------------------- */
	/* IBM PC (Model 5150/5160): Quote from "Technical Reference"; I/O Channel Description:    */
	/* --------------------------------------------------------------------------------------- */
	/* Signal Name:                                                                            */
	/*  - +DRQ2 (Input): DMA Request 1 to 3: These lines are asynchronous channel requests     */
	/*  used by peripheral devices to gain DMA service. They are prioritized with DRQ3 being   */
	/*  the lowest and DRQ1 being the highest. A DRQ line must be held high until the          */
	/*  corresponding DACK line goes active.                                                   */
	/* --------------------------------------------------------------------------------------- */
	/* ------------------------------------------------------------------------------------------ */
	/* IBM Personal Computer Model 25/30: Quote from "Technical Reference"; I/O Channel; Signal   */
	/* Description:                                                                               */
	/* ------------------------------------------------------------------------------------------ */
	/* The following is a description of the I/O channel signal lines. All lines are TTL-         */
	/* compatible. The (O), (I), or (I/O) notation refers to output, input, or input and output.  */
	/*  - DRQ1-DRQ3 (I): DMA Request 1 to 3: These lines are asynchronous channel requests used by*/
	/*  peripheral devices to gain DMA ser-vices. They are prioritized with DRQ1 being the highest*/
	/*  and DRQ3 being the lowest. A request is generated by bringing a request line to an active */
	/*  level. A request line is held active until the corresponding acknowledge line goes active.*/
	/* ------------------------------------------------------------------------------------------ */
	switch (addr) {
	case 0x0081:
		pc->dma_page[2] = (unsigned long) (val & 0x0f) << 16;
		break;
	case 0x0082:
		pc->dma_page[3] = (unsigned long) (val & 0x0f) << 16;
		break;
	case 0x0083:
		pc->dma_page[1] = (unsigned long) (val & 0x0f) << 16;
		break;
	case 0x0087:
		pc->dma_page[0] = (unsigned long) (val & 0x0f) << 16;
		break;
	}
	/* ------------------------------------------------------------------------------------------ */
	/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; DMA Controller:       */
	/* ------------------------------------------------------------------------------------------ */
	/* The 8237 Direct Memory Access (DMA) controller and its support logic in the gate array     */
	/* support four channels of 20 address bit DMA. It operates at 4 MHz and handles only 8-bit   */
	/* data transfers. The DMA channel assignments and page register addresses are:               */
	/* ------------------------------------------------------------------------------------------ */
	/* Level:  Assignment:                                                                        */
	/*  - DRQ0: - Not Available                                                                   */
	/*  - DRQ1: - Not Used                                                                        */
	/*  - DRQ2: - Diskette                                                                        */
	/*  - DRQ3: - Fixed Disk                                                                      */
	/* ------------------------------------------------------------------------------------------ */
	/* Figure 1-7. DMA Channel Assignments:                                                       */
	/* ------------------------------------------------------------------------------------------ */
	/* Hex Address: DMA Page Register:                                                            */
	/*  - 081:       - Channel 2:                                                                 */
	/*  - 082:       - Channel 3:                                                                 */
	/*  - 083:       - Channel 1:                                                                 */
	/*  - 087:       - Channel 0:                                                                 */
	/* ------------------------------------------------------------------------------------------ */
	/* Figure 1-8. DMA Page Register Addresses:                                                   */
	/* ------------------------------------------------------------------------------------------ */
	/* Three of the DMA channels (1, 2, and 3) are available on the I/O bus and support high-speed*/
	/* data transfers between I/O devices and memory without microprocessor intervention.         */
	/*                                                                                            */
	/* DMA data transfers take six clock cycles of 250 ns, or 1.5 micro-seconds. I/O CH RDY can be*/
	/* pulled inactive to add wait states to allow more time for slower devices.                  */
	/* ------------------------------------------------------------------------------------------ */
	/* ------------------------------------------------------------------- */
	/* IBM PC (Model 5150/5160):Quote from"Technical Reference"I/O Channel:*/
	/* ------------------------------------------------------------------- */
	/* The I/O channel is an extension of the 8088 microprocessor bus.     */
	/* It is, however, demultiplexed, repowered, and enhanced by the       */
	/* addition of interrupts and direct memory access (DMA)               */
	/* functions.                                                          */
	/*                                                                     */
	/* The I/O channel contains an 8-bit bidirectional data bus, 20        */
	/* address lines, 6 levels of interrupt, control lines for memory and  */
	/* I/O read or write, clock and timing lines, 3 channels of DMA        */
	/* control lines, memory refresh timing control lines, a               */
	/* channel-check line, and power and ground for the adapters. Four     */
	/* voltage levels are provided for I/O cards: +5 Vdc, -5 Vdc, +12      */
	/* Vdc, and -12 Vdc. These functions are provided in a 62-pin          */
	/* connector with 100-mil card tab spacing.                            */
	/*                                                                     */
	/* A 'ready' line is available on the I/O channel to allow operation   */
	/* with slow I/O or memory devices. If the channel's ready line is     */
	/* not activated by an addressed device, all                           */
	/* microprocessor-generated memory read and write cycles take four     */
	/* 210-ns clocks or 840-ns/byte. All microprocessor-generated I/O      */
	/* read and write cycles require five clocks for a cycle time of       */
	/* 1.05-us/byte. All DMA transfers require five clocks for a cycle     */
	/* time of 1.05-us/byte. Refresh cycles occur once every 72 clocks     */
	/* (approximately 15-us) and require four clocks or approximately      */
	/* 7% of the bus bandwidth.                                            */
	/*                                                                     */
	/* I/O devices are addressed using I/O mapped address space. The       */
	/* channel is designed so that 512 (5150)/768 (5160) I/O device        */
	/* addresses are available to the I/O channel cards.                   */
	/*                                                                     */
	/* A 'channel check' line exists for reporting error conditions to the */
	/* microprocessor. Activating this line results in a non-maskable      */
	/* interrupt (NMI) to the 8088 microprocessor. Memory expansion        */
	/* options use this line to report parity errors.                      */
	/*                                                                     */
	/* The I/O channel is repowered to provide sufficient drive to power   */
	/* all five system unit (5150)/all eight (J1 through J8) (5160)        */
	/* expansion slots, assuming two low-power Schottky loads per slot. The*/
	/* IBM I/O adapters typically use only one load.                       */
	/* -------- Applies to Model 5160 Technical Reference Manual: -------- */
	/* Timing requirements on slot J8 are much stricter than those on slots*/
	/* J1 through J7. Slot J8 also requires the card to provide a signal   */
	/* designating when the card is selected.                              */
	/* ------------------------------------------------------------------- */
}

static
void pc_set_port16 (ibmpc_t *pc, unsigned long addr, unsigned short val)
{
#ifdef DEBUG_PORTS
	pc_log_deb ("set port 16 %04lX <- %04X\n", addr, val);
#endif
}


static
void pc_dma2_set_mem8 (ibmpc_t *pc, unsigned long addr, unsigned char val)
{
	/* --------------------------------------------------------------------------------------- */
	/* IBM PC (Model 5150/5160): Quote from "Technical Reference"; I/O Channel Description:    */
	/* --------------------------------------------------------------------------------------- */
	/* Signal Name:                                                                            */
	/*  - +DRQ2 (Input): DMA Request 1 to 3: These lines are asynchronous channel requests     */
	/*  used by peripheral devices to gain DMA service. They are prioritized with DRQ3 being   */
	/*  the lowest and DRQ1 being the highest. A DRQ line must be held high until the          */
	/*  corresponding DACK line goes active.                                                   */
	/* --------------------------------------------------------------------------------------- */
	/* ------------------------------------------------------------------------------------------ */
	/* IBM Personal Computer Model 25/30: Quote from "Technical Reference"; I/O Channel; Signal   */
	/* Description:                                                                               */
	/* ------------------------------------------------------------------------------------------ */
	/* The following is a description of the I/O channel signal lines. All lines are TTL-         */
	/* compatible. The (O), (I), or (I/O) notation refers to output, input, or input and output.  */
	/*  - DRQ1-DRQ3 (I): DMA Request 1 to 3: These lines are asynchronous channel requests used by*/
	/*  peripheral devices to gain DMA ser-vices. They are prioritized with DRQ1 being the highest*/
	/*  and DRQ3 being the lowest. A request is generated by bringing a request line to an active */
	/*  level. A request line is held active until the corresponding acknowledge line goes active.*/
	/* ------------------------------------------------------------------------------------------ */
	mem_set_uint8 (pc->mem, pc->dma_page[2] + addr, val);
	/* ------------------------------------------------------------------------------------------ */
	/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; DMA Controller:       */
	/* ------------------------------------------------------------------------------------------ */
	/* The 8237 Direct Memory Access (DMA) controller and its support logic in the gate array     */
	/* support four channels of 20 address bit DMA. It operates at 4 MHz and handles only 8-bit   */
	/* data transfers. The DMA channel assignments and page register addresses are:               */
	/* ------------------------------------------------------------------------------------------ */
	/* Level:  Assignment:                                                                        */
	/*  - DRQ0: - Not Available                                                                   */
	/*  - DRQ1: - Not Used                                                                        */
	/*  - DRQ2: - Diskette                                                                        */
	/*  - DRQ3: - Fixed Disk                                                                      */
	/* ------------------------------------------------------------------------------------------ */
	/* Figure 1-7. DMA Channel Assignments:                                                       */
	/* ------------------------------------------------------------------------------------------ */
	/* Hex Address: DMA Page Register:                                                            */
	/*  - 081:       - Channel 2:                                                                 */
	/*  - 082:       - Channel 3:                                                                 */
	/*  - 083:       - Channel 1:                                                                 */
	/*  - 087:       - Channel 0:                                                                 */
	/* ------------------------------------------------------------------------------------------ */
	/* Figure 1-8. DMA Page Register Addresses:                                                   */
	/* ------------------------------------------------------------------------------------------ */
	/* Three of the DMA channels (1, 2, and 3) are available on the I/O bus and support high-speed*/
	/* data transfers between I/O devices and memory without microprocessor intervention.         */
	/*                                                                                            */
	/* DMA data transfers take six clock cycles of 250 ns, or 1.5 micro-seconds. I/O CH RDY can be*/
	/* pulled inactive to add wait states to allow more time for slower devices.                  */
	/* ------------------------------------------------------------------------------------------ */
}

static
unsigned char pc_dma2_get_mem8 (ibmpc_t *pc, unsigned long addr)
{
	/* --------------------------------------------------------------------------------------- */
	/* IBM PC (Model 5150/5160): Quote from "Technical Reference"; I/O Channel Description:    */
	/* --------------------------------------------------------------------------------------- */
	/* Signal Name:                                                                            */
	/*  - +DRQ2 (Input): DMA Request 1 to 3: These lines are asynchronous channel requests     */
	/*  used by peripheral devices to gain DMA service. They are prioritized with DRQ3 being   */
	/*  the lowest and DRQ1 being the highest. A DRQ line must be held high until the          */
	/*  corresponding DACK line goes active.                                                   */
	/* --------------------------------------------------------------------------------------- */
	/* ------------------------------------------------------------------------------------------ */
	/* IBM Personal Computer Model 25/30: Quote from "Technical Reference"; I/O Channel; Signal   */
	/* Description:                                                                               */
	/* ------------------------------------------------------------------------------------------ */
	/* The following is a description of the I/O channel signal lines. All lines are TTL-         */
	/* compatible. The (O), (I), or (I/O) notation refers to output, input, or input and output.  */
	/*  - DRQ1-DRQ3 (I): DMA Request 1 to 3: These lines are asynchronous channel requests used by*/
	/*  peripheral devices to gain DMA ser-vices. They are prioritized with DRQ1 being the highest*/
	/*  and DRQ3 being the lowest. A request is generated by bringing a request line to an active */
	/*  level. A request line is held active until the corresponding acknowledge line goes active.*/
	/* ------------------------------------------------------------------------------------------ */
	return (mem_get_uint8 (pc->mem, pc->dma_page[2] + addr));
	/* ------------------------------------------------------------------------------------------ */
	/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; DMA Controller:       */
	/* ------------------------------------------------------------------------------------------ */
	/* The 8237 Direct Memory Access (DMA) controller and its support logic in the gate array     */
	/* support four channels of 20 address bit DMA. It operates at 4 MHz and handles only 8-bit   */
	/* data transfers. The DMA channel assignments and page register addresses are:               */
	/* ------------------------------------------------------------------------------------------ */
	/* Level:  Assignment:                                                                        */
	/*  - DRQ0: - Not Available                                                                   */
	/*  - DRQ1: - Not Used                                                                        */
	/*  - DRQ2: - Diskette                                                                        */
	/*  - DRQ3: - Fixed Disk                                                                      */
	/* ------------------------------------------------------------------------------------------ */
	/* Figure 1-7. DMA Channel Assignments:                                                       */
	/* ------------------------------------------------------------------------------------------ */
	/* Hex Address: DMA Page Register:                                                            */
	/*  - 081:       - Channel 2:                                                                 */
	/*  - 082:       - Channel 3:                                                                 */
	/*  - 083:       - Channel 1:                                                                 */
	/*  - 087:       - Channel 0:                                                                 */
	/* ------------------------------------------------------------------------------------------ */
	/* Figure 1-8. DMA Page Register Addresses:                                                   */
	/* ------------------------------------------------------------------------------------------ */
	/* Three of the DMA channels (1, 2, and 3) are available on the I/O bus and support high-speed*/
	/* data transfers between I/O devices and memory without microprocessor intervention.         */
	/*                                                                                            */
	/* DMA data transfers take six clock cycles of 250 ns, or 1.5 micro-seconds. I/O CH RDY can be*/
	/* pulled inactive to add wait states to allow more time for slower devices.                  */
	/* ------------------------------------------------------------------------------------------ */
}

static
void pc_dma3_set_mem8 (ibmpc_t *pc, unsigned long addr, unsigned char val)
{
	/* --------------------------------------------------------------------------------------- */
	/* IBM PC (Model 5150/5160): Quote from "Technical Reference"; I/O Channel Description:    */
	/* --------------------------------------------------------------------------------------- */
	/* Signal Name:                                                                            */
	/*  - +DRQ2 (Input): DMA Request 1 to 3: These lines are asynchronous channel requests     */
	/*  used by peripheral devices to gain DMA service. They are prioritized with DRQ3 being   */
	/*  the lowest and DRQ1 being the highest. A DRQ line must be held high until the          */
	/*  corresponding DACK line goes active.                                                   */
	/* --------------------------------------------------------------------------------------- */
	/* ------------------------------------------------------------------------------------------ */
	/* IBM Personal Computer Model 25/30: Quote from "Technical Reference"; I/O Channel; Signal   */
	/* Description:                                                                               */
	/* ------------------------------------------------------------------------------------------ */
	/* The following is a description of the I/O channel signal lines. All lines are TTL-         */
	/* compatible. The (O), (I), or (I/O) notation refers to output, input, or input and output.  */
	/*  - DRQ1-DRQ3 (I): DMA Request 1 to 3: These lines are asynchronous channel requests used by*/
	/*  peripheral devices to gain DMA ser-vices. They are prioritized with DRQ1 being the highest*/
	/*  and DRQ3 being the lowest. A request is generated by bringing a request line to an active */
	/*  level. A request line is held active until the corresponding acknowledge line goes active.*/
	/* ------------------------------------------------------------------------------------------ */
	mem_set_uint8 (pc->mem, pc->dma_page[3] + addr, val);
	/* ------------------------------------------------------------------------------------------ */
	/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; DMA Controller:       */
	/* ------------------------------------------------------------------------------------------ */
	/* The 8237 Direct Memory Access (DMA) controller and its support logic in the gate array     */
	/* support four channels of 20 address bit DMA. It operates at 4 MHz and handles only 8-bit   */
	/* data transfers. The DMA channel assignments and page register addresses are:               */
	/* ------------------------------------------------------------------------------------------ */
	/* Level:  Assignment:                                                                        */
	/*  - DRQ0: - Not Available                                                                   */
	/*  - DRQ1: - Not Used                                                                        */
	/*  - DRQ2: - Diskette                                                                        */
	/*  - DRQ3: - Fixed Disk                                                                      */
	/* ------------------------------------------------------------------------------------------ */
	/* Figure 1-7. DMA Channel Assignments:                                                       */
	/* ------------------------------------------------------------------------------------------ */
	/* Hex Address: DMA Page Register:                                                            */
	/*  - 081:       - Channel 2:                                                                 */
	/*  - 082:       - Channel 3:                                                                 */
	/*  - 083:       - Channel 1:                                                                 */
	/*  - 087:       - Channel 0:                                                                 */
	/* ------------------------------------------------------------------------------------------ */
	/* Figure 1-8. DMA Page Register Addresses:                                                   */
	/* ------------------------------------------------------------------------------------------ */
	/* Three of the DMA channels (1, 2, and 3) are available on the I/O bus and support high-speed*/
	/* data transfers between I/O devices and memory without microprocessor intervention.         */
	/*                                                                                            */
	/* DMA data transfers take six clock cycles of 250 ns, or 1.5 micro-seconds. I/O CH RDY can be*/
	/* pulled inactive to add wait states to allow more time for slower devices.                  */
	/* ------------------------------------------------------------------------------------------ */
}

static
unsigned char pc_dma3_get_mem8 (ibmpc_t *pc, unsigned long addr)
{
	/* --------------------------------------------------------------------------------------- */
	/* IBM PC (Model 5150/5160): Quote from "Technical Reference"; I/O Channel Description:    */
	/* --------------------------------------------------------------------------------------- */
	/* Signal Name:                                                                            */
	/*  - +DRQ2 (Input): DMA Request 1 to 3: These lines are asynchronous channel requests     */
	/*  used by peripheral devices to gain DMA service. They are prioritized with DRQ3 being   */
	/*  the lowest and DRQ1 being the highest. A DRQ line must be held high until the          */
	/*  corresponding DACK line goes active.                                                   */
	/* --------------------------------------------------------------------------------------- */
	/* ------------------------------------------------------------------------------------------ */
	/* IBM Personal Computer Model 25/30: Quote from "Technical Reference"; I/O Channel; Signal   */
	/* Description:                                                                               */
	/* ------------------------------------------------------------------------------------------ */
	/* The following is a description of the I/O channel signal lines. All lines are TTL-         */
	/* compatible. The (O), (I), or (I/O) notation refers to output, input, or input and output.  */
	/*  - DRQ1-DRQ3 (I): DMA Request 1 to 3: These lines are asynchronous channel requests used by*/
	/*  peripheral devices to gain DMA ser-vices. They are prioritized with DRQ1 being the highest*/
	/*  and DRQ3 being the lowest. A request is generated by bringing a request line to an active */
	/*  level. A request line is held active until the corresponding acknowledge line goes active.*/
	/* ------------------------------------------------------------------------------------------ */
	return (mem_get_uint8 (pc->mem, pc->dma_page[3] + addr));
	/* ------------------------------------------------------------------------------------------ */
	/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; DMA Controller:       */
	/* ------------------------------------------------------------------------------------------ */
	/* The 8237 Direct Memory Access (DMA) controller and its support logic in the gate array     */
	/* support four channels of 20 address bit DMA. It operates at 4 MHz and handles only 8-bit   */
	/* data transfers. The DMA channel assignments and page register addresses are:               */
	/* ------------------------------------------------------------------------------------------ */
	/* Level:  Assignment:                                                                        */
	/*  - DRQ0: - Not Available                                                                   */
	/*  - DRQ1: - Not Used                                                                        */
	/*  - DRQ2: - Diskette                                                                        */
	/*  - DRQ3: - Fixed Disk                                                                      */
	/* ------------------------------------------------------------------------------------------ */
	/* Figure 1-7. DMA Channel Assignments:                                                       */
	/* ------------------------------------------------------------------------------------------ */
	/* Hex Address: DMA Page Register:                                                            */
	/*  - 081:       - Channel 2:                                                                 */
	/*  - 082:       - Channel 3:                                                                 */
	/*  - 083:       - Channel 1:                                                                 */
	/*  - 087:       - Channel 0:                                                                 */
	/* ------------------------------------------------------------------------------------------ */
	/* Figure 1-8. DMA Page Register Addresses:                                                   */
	/* ------------------------------------------------------------------------------------------ */
	/* Three of the DMA channels (1, 2, and 3) are available on the I/O bus and support high-speed*/
	/* data transfers between I/O devices and memory without microprocessor intervention.         */
	/*                                                                                            */
	/* DMA data transfers take six clock cycles of 250 ns, or 1.5 micro-seconds. I/O CH RDY can be*/
	/* pulled inactive to add wait states to allow more time for slower devices.                  */
	/* ------------------------------------------------------------------------------------------ */
}


static
unsigned char pc_ppi_get_port_a (ibmpc_t *pc)
{
	if (pc->ppi_port_b & 0x80) {
		return (pc->ppi_port_a[0]);
	}
	else {
		pc->ppi_port_a[1] = pc_kbd_get_key (&pc->kbd);

		return (pc->ppi_port_a[1]);
	}
}

static
unsigned char pc_ppi_get_port_c (ibmpc_t *pc)
{
	if (pc->model & PCE_IBMPC_5160) {
		if (pc->ppi_port_b & 0x08) {
			return (pc->ppi_port_c[1]);
		}
		else {
			return (pc->ppi_port_c[0]);
		}
	}
	else if (pc->model & PCE_IBMPS2_MODEL25) {
		if (pc->ppi_port_b & 0x08) {
			return (pc->ppi_port_c[1]);
		}
		else {
			return (pc->ppi_port_c[0]);
		}
	}
	/*	else if (pc->model & PCE_COMPAQ_DESKPRO) {
		if (pc->ppi_port_b & 0x08) {
			return (pc->ppi_port_c[1]);
		}
		else {
			return (pc->ppi_port_c[0]);
		}
	} */
	else if (pc->model & PCE_COMPAQ_PORTABLE) {
		if (pc->ppi_port_b & 0x04) {
			return (pc->ppi_port_c[0]);
		}
		else {
			return (pc->ppi_port_c[1]);
		}
	}
	else if (pc->model & PCE_COMPAQ_PORTABLE_PLUS) {
		if (pc->ppi_port_b & 0x04) {
			return (pc->ppi_port_c[0]);
		}
		else {
			return (pc->ppi_port_c[1]);
		}
	}
	else {
		if (pc->cas != NULL) {
			if (pc_cas_get_inp (pc->cas)) {
				pc->ppi_port_c[0] |= 0x10;
				pc->ppi_port_c[1] |= 0x10;
			}
			else {
				pc->ppi_port_c[0] &= ~0x10;
				pc->ppi_port_c[1] &= ~0x10;
			}
		}
		if (pc->ppi_port_b & 0x04) {
			return (pc->ppi_port_c[0]);
		}
		else {
			return (pc->ppi_port_c[1]);
		}
	}
}

static
void pc_ppi_set_port_b (ibmpc_t *pc, unsigned char val)
{
	unsigned char old;
	/* ---------------------------------------------------------------------------- */
	/* IBM PC (Model 5150): Quote from "Technical Reference"; System Timers:        */
	/* ---------------------------------------------------------------------------- */
	/* Three programmable timer/counters are used by the system as follows: Channel */
	/* 0 is a general-purpose timer providing a constant time base for implementing */
	/* a time-of-day clock, Channel 1 times and requests refresh cycles from the    */
	/* the Direct Memory Access (DMA) channel, and Channel 2 supports the tone      */
	/* generation for the speaker. Each channel has a minimum timing resolution of  */
	/* 1.05-us.                                                                     */
	/* ---------------------------------------------------------------------------- */
	/* IBM PC (Model 5160): Quote from "Technical Reference"; System Timers:        */
	/* ---------------------------------------------------------------------------- */
	/* Three programmable timer/counters are used by the system as follows: Channel */
	/* 0 is used as a general-purpose timer providing a constant time base for      */
	/* implementing a time-of-day clock; Channel 1 is used to time and request      */
	/* refresh cycles from the DMA channel; and Channel 2 is used to support the    */
	/* tone generation for the audio speaker. Each channel has a minimum timing     */
	/* resolution of 1.05-us.                                                       */
	/* ---------------------------------------------------------------------------- */
	old = pc->ppi_port_b;
	pc->ppi_port_b = val;
	pc_kbd_set_clk (&pc->kbd, val & 0x40);
	pc_kbd_set_enable (&pc->kbd, (val & 0x80) == 0);
	/* ------------------------------------------------------------------------------------------ */
	/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; System Timer:         */
	/* ------------------------------------------------------------------------------------------ */
	/* The system timer is an 8253 programmable interval timer/counter, or equivalent, that       */
	/* functions as an arrangement of four external I/O ports (hex 0040 through 0043). It receives*/
	/*its 1.19 MHz clock from the I/O support gate array. These ports are treated as counters; the*/
	/* fourth, address hex 0043, is a control register for mode programming.                      */
	/*                                                                                            */
	/* The content of a selected counter may be latched without disturbing the counting operation */
	/*by writing to the control register. If the content of the counter is critical to a program's*/
	/* operation, a second read is recommended for verification.                                  */
	/* -------------------________________------------------------------------------------------- */
	/* System Bus:------>:|System Timer: |                                                        */
	/*          __________| - Gate 0     |                                                        */
	/*          |  _______| - Clock  in 0|                                                        */
	/* +5 Vdc __|__|______| - Gate 1     |                                                        */
	/* -RAS_SIP ---|---___| - Clock  in 1|                                                        */
	/* I/O Port  __|______| - Gate 2     |                                                        */
	/* Hex 0061__| |______| - Clock  in 2|                                                        */
	/* Port Bit 0  |      | - Clock Out 0|_______________IRQ 0________                            */
	/* 1.19 Mhz____|      | - Clock Out 1|    |_Driver_|                                          */
	/*                    | - Clock Out 2|______|__ _|_______                                     */
	/* I/O Port           ----------------  | AND | | Low   |--> To Beeper                        */
	/* Hex 0061 ----------------------------|-----| | Pass  |                                     */
	/* Port Bit 1                                   | Filter|                                     */
	/* ---------------------------------------------|-------|------------------------------------ */
	/* Figure 1-4. System Timer Block Diagram                                                     */
	/* ------------------------------------------------------------------------------------------ */
	/* The three timers are programmable and are used by the system as follows:                   */
	/*                                                                                            */
	/* Channel 0 is a general-purpose timer providing a constant time base.                       */
	/* Channel 0:   System Timer:                                                                 */
	/*  - GATE    0: - Tied High                                                                  */
	/*  - CLK IN  0: - 1.19 MHz OSC                                                               */
	/*  - CLK OUT 0: - IRQ 0                                                                      */
	/* Channel 1 is for internal diagnostic tests.                                                */
	/* Channel 1:   Diagnostic:                                                                   */
	/*  - GATE    1: - Tied High                                                                  */
	/*  - CLK IN  1: - -RAS_SIP from system support gate array                                    */
	/*  - CLK OUT 2: - Not connected                                                              */
	/* Channel 2 supports the tone generation for the audio.                                      */
	/* Channel 2:   Tone Generation:                                                              */
	/*  - GATE    2: Controlled by bit 0 at port hex 61                                           */
	/*  - CLK IN  2: 1.19 MHz OSC                                                                 */
	/*  - CLK OUT 2: To the beeper data of the I/O support gate array                             */
	/* ------------------------------------------------------------------------------------------ */
	/* ------------------------------------------------------------------------------------------ */
    /* IBM PC (Model 5150/5160): Quote from "Technical Reference"; Speaker Circuit:               */
    /* ------------------------------------------------------------------------------------------ */
    /* The sound system has a small, permanent magnet, 57.15 mm (2-1/5 in.) speaker.              */
    /* --------------------------- IBM PC Model 5150 Technical Reference: ----------------------- */
    /* The speaker can be driven from one or both of two sources:                                 */
    /*  - An 8255A-5 programmable peripheral interface (PPI) output bit. The address and bit are  */
    /*  defined in the "I/O Address Map".                                                         */
    /*  - A timer clock channel, the output of which is programmable within the functions of the  */
    /*  8253-5 timer when using a 1.19-MHz clock input. The timer gate also is controlled by an   */
    /*  8255A-5 PPI output-port bit. Address and bit assignment are in the "I/O Address Map".     */
    /* --------------------------- IBM PC Model 5160 Technical Reference: ----------------------- */
    /* The speaker's control circuits and driver are on the system board. The speaker connects    */
	/* through a 2-wire interface that attaches to a 3-pin connector on the system board.         */
    /*                                                                                            */
    /* The speaker drive circuit is capable of approximately 1/2 watt of power. The control       */
	/* circuits allow the speaker to be driven three different ways: 1.) a direct program control */
	/* register bit may be toggled to generate a pulse train; 2.) the output from Channel 2 of the*/
	/* timer counter may be programmed to generate a waveform to the speaker; 3.) the clock input */
	/* to the timer counter can be modulated with a program-controlled I/O register bit. All three*/
	/* methods may be performed simultaneously.                                                   */
    /* ------------------------------------------------------------------------------------------ */
    /* Speaker Drive System Block Diagram (1.19 MHz):                                             */
    /* ------------------------------------------------------------------------------------------ */
    /* PPI Bit 1, I/O Address Hex 0061:          -> AND:->Driver:->Low Pass Filter:-> To Speaker: */
    /* -> Clock In 2, Gate 2, Timer Clock Out 2: -> AND:->Driver:->Low Pass Filter:-> To Speaker: */
    /* PPI Bit 0, I/O Address Hex 0061:                                                           */
    /* ------------------------------------------------------------------------------------------ */
    /* Speaker Tone Generation:                                                                   */
    /* ------------------------------------------------------------------------------------------ */
    /* Channel 2 (Tone generation for speaker):                                                   */
    /*  - Gate 2: Controller by 8255A-5 PPI Bit (See I/O Map)                                     */
    /*  - Clock In  2: 1.19318-MHz OSC                                                            */
    /*  - Clock Out 2: Used to drive speaker                                                      */
    /* ------------------------------------------------------------------------------------------ */
	e8253_set_gate (&pc->pit, 2, val & 0x01);
	if ((old ^ val) & 0x02) {
		pc_speaker_set_msk (&pc->spk, val & 0x02);
	}
	/* ------------------------------------------------------------------- */
	/* IBM PC (Model 5150): Quote from "Technical Reference":              */
	/* ------------------------------------------------------------------- */
	/* The system board contains circuits for attaching an audio           */
	/* cassette, the keyboard, and the speaker. The cassette adapter       */
	/* allows the attachment of any good quality audio cassette through    */
	/* the earphone output and either the microphone or auxiliary          */
	/* inputs. The system board has a jumper for either input. This        */
	/* interface also provides a cassette motor control for transport      */
	/* starting and stopping under program control. The interface reads    */
	/* and writes the audio cassette at a data rate of between 1,000 and   */
	/* 2,000 baud. The baud rate is variable and depend on data            */
	/* content, because a different bit-cell time is used for 0's and 1's. */
	/* For diagnostic purposes, the tape interface can loop read to write  */
	/* for testing the system board's circuits. The ROM cassette           */
	/* software blocks cassette data and generates a cyclic redundancy     */
	/* check (CRC) to check this data.                                     */
	/* ------------------------------------------------------------------- */
	if (pc->model & PCE_IBMPC_5150) {
		if ((old ^ val) & 0x08) {
			/* cassette motor change */
			if (pc->cas != NULL) {
				pc_cas_set_motor (pc->cas, (val & 0x08) == 0);
				if (val & 0x08) {
					/* motor off: restore clock */
					pc->speed_current = pc->speed_saved;
					pc->speed_clock_extra = 0;
				}
				else {
					/* motor on: set clock to 4.77 MHz */
					pc->speed_saved = pc->speed_current;
					pc->speed_current = 1;
					pc->speed_clock_extra = 0;
				}
			}
		}
	}
    /* --------------------------------------------------------------------------------------- */
    /* IBM PC (Model 5150): Quote from "Technical Reference"; Cassette Interface:              */
    /* --------------------------------------------------------------------------------------- */
    /* The cassette interface is controlled through software. An output from the 8253 timer    */
    /* controls the data to the cassette recorder through pin 5 of the cassette DIN connector  */
    /* at the rear of the system board. The cassette input data is read by an input port bit   */
    /* of the 8255A-5 PPI. This data is received through pin 4 of the cassette connector.      */
    /* Software algorithms are used to generate and read cassette data. The cassette drive     */
    /* motor is controlled through pins 1 and 3 of the cassette connector. The drive motor     */
    /* on/off switching is controlled by an 8255A-5 PPI output-port bit (hex 61, bit 3). The   */
    /* 8255A-5 address and bit assignments are defined in "I/O Address Map" earlier in this    */
    /* section.                                                                                */
    /*                                                                                         */
    /* A 2 by 2 Berg pin and a jumper are used on the cassette 'data out' line. The jumper     */
    /* allows use of the 'data out' line as a 0.075-Vdc microphone input when placed across    */
    /* the M and C of the Berg Pins. A 0.68-Vdc auxiliary input to the cassette recorder is    */
    /* available when the jumper is placed across the A and C of the Berg Pins. The "System    */
    /* Board Component Diagram" shows the location of the cassette Berg pins.                  */
    /* --------------------------------------------------------------------------------------- */
    /*               Microphone Input (0.075 Vdc):  Auxiliary Input (0.68 Vdc):                */
    /*               ----------------------------- -----------------------------               */
    /*                      M            A                 M            A                      */
    /*               ----------------------------- -----------------------------               */
    /*               |      O            X       | |       X            O      |               */
    /*               |      O            X       | |       X            O      |               */
    /*               ----------------------------- -----------------------------               */
    /*                      C            C                 C            C                      */
    /*               ----------------------------- -----------------------------               */
    /*               Microphone Input  (0.075 Vdc)  Auxiliary Input  (0.68 Vdc)                */
    /* --------------------------------------------------------------------------------------- */
    /* Cassette Circuit Block Diagrams:                                                        */
    /* --------------------------------------------------------------------------------------- */
    /* Circuit block diagrams for the cassette-interface read hardware, write hardware, and    */
    /* motor control are illustrated below.                                                    */
    /* --------------------------------------------------------------------------------------- */
    /* Cassette-Interface Read Hardware Block Diagram:                                         */
    /* --------------------------------------------------------------------------------------- */
    /*             GND: ----> 18k Ohm Resistor: -|-> 1M Ohm Resistor: -|                       */
    /*                                           |                     |                       */
    /* |-->0.047 uF Capacitor:-18k Ohm Resistor:-|-> MC1741: ----------|->18k Ohm Resistor: -|-*//*Cassette Data In:*/
    /* |                                         |      |      OP AMP                        | */
    /* |                                         |     -5V                                   | */
    /* |                                         V                                           | */
    /* V Data From Cassette Recorder Jack:   18k Ohm Resistor:GND:->Silicon Diode VIR=.4V:<--| */
    /*                                           |                          Cathode:           */
    /*                                      GND: V                                             */
    /* --------------------------------------------------------------------------------------- */
    /* Cassette Interface Write Hardware Block Diagram:                                        */
    /* --------------------------------------------------------------------------------------- */
    /*                                               V +5V:                                    */
    /* 8253 Timer #2 Output: --|--> 74LS38 (OR): ----|> 3.9k Ohm R:                            */
    /*                         |---------------------|                                         */
    /*                                               |> 4.7k Ohm R:                            */
    /*                         0:--------------------|                                         */
    /*                         0.678V to AUX Input   |> 1.2k Ohm R:                            */
    /*                         0:--------------------|                                         */
    /*                         0.075V to MIC Input   |> 150  Ohm R:                            */
    /*                                               |                                         */
    /*                                               |> GND:                                   */
    /* --------------------------------------------------------------------------------------- */
    /* Cassette Interface Write Hardware Block Diagram:                                        */
    /* --------------------------------------------------------------------------------------- */
    /*                                                         +5V                             */
    /*                                                          |                              */
    /* 8253 Timer #2 Output: --|--> 74LS38 (OR):           3.9k Ohm R:                         */
    /*                         |--> 74LS38 (OR):----------------|                              */
    /*                                                          V                              */
    /*                                                     4.7k Ohm R:                         */
    /*                                     0 :------------------|                              */
    /*                                       0.678V to AUX:     V                              */
    /*                                                     1.2k Ohm R:                         */
    /*                                     0 :------------------|                              */
    /*                                     0.075V to MIC Input: V                              */
    /*                                                     150  Ohm R:                         */
    /*                                                          |--> GND:                      */
    /* --------------------------------------------------------------------------------------- */
    /* Cassette Motor Control Block Diagram:                                                   */
    /* --------------------------------------------------------------------------------------- */
    /*                                +5V                                                      */
    /*                                 ^                                                       */
    /*                                 |                                                       */
    /*                                 V                                                       */
    /*                             4.7 kOhm:   SN75475:   +5V                                  */
    /*                                 ^    +5V:-->Clamp:  ^       Relay:                      */
    /*                                 |                   |              N/O:---------------- */
    /*                                 |             S:----|---->Coil:                         */
    /*                                 |             C:----|           Cassette Motor Control: */
    /*                                 |           VCC:----|                                   */
    /*             |--> 74LS38 (OR): --|------>In: Out:--------->Coil:                         */
    /* Motor On: --|                                                      COM:---------------- */
    /* --------------------------------------------------------------------------------------- */
    /* Cassette Interface Connector Specifications (5-Pin DIN Connector):                      */
    /* --------------------------------------------------------------------------------------- */
    /* Pin:  Signal                            : Electrical Characteristics            :       */
    /*  - 1: Motor Control                     : Common from Relay                             */
    /*  - 2: Ground                            :                                               */
    /*  - 3: Motor Control                     : Relay N.O (6 Vdc at 1A)                       */
    /*  - 4: Data In                           : 500nA at += 13V - at 1,000 - 2,000 Baud       */
    /*  - 5: Data Out (Microphone or Auxiliary): 250 uA at 0.68 VDC or ** 0.075 Vdc            */
    /* --------------------------------------------------------------------------------------- */
    /* *: All voltages and currents are maximum ratings and should not be exceeded.            */
    /* **: Data out can be chosen using a jumper located on the system board. (Auxiliary ->0.68*/
    /* Vdc or Microphone ->0.075 Vdc).                                                         */
    /*                                                                                         */
    /* Interchange of these voltages on the cassette recorder could lead to damage of recorder */
    /* inputs.                                                                                 */
    /* --------------------------------------------------------------------------------------- */
}

static
void pc_set_dack0 (ibmpc_t *pc, unsigned char val)
{
	/* --------------------------------------------------------------------------------------- */
	/* IBM PC (Model 5150/5160): Quote from "Technical Reference"; I/O Channel Description:    */
	/* --------------------------------------------------------------------------------------- */
	/* Signal Name:                                                                            */
	/*  - +DRQ2 (Input): DMA Request 1 to 3: These lines are asynchronous channel requests     */
	/*  used by peripheral devices to gain DMA service. They are prioritized with DRQ3 being   */
	/*  the lowest and DRQ1 being the highest. A DRQ line must be held high until the          */
	/*  corresponding DACK line goes active.                                                   */
	/*  - -DACK3 (Output): DMA Acknowledge 0 to 3: These lines are used to acknowledge DMA     */
	/*  requests (DRQ1-DRG3) and refresh system dynamic memory (-DACK0). They are active low.  */
	/*  - -DACK1 (Output): See -DACK3                                                          */
	/*  - -DACK0 (Output): See -DACK3                                                          */
	/* --------------------------------------------------------------------------------------- */
	/* ------------------------------------------------------------------------------------------ */
	/* IBM Personal Computer Model 25/30: Quote from "Technical Reference"; I/O Channel; Signal   */
	/* Description:                                                                               */
	/* ------------------------------------------------------------------------------------------ */
	/* The following is a description of the I/O channel signal lines. All lines are TTL-         */
	/* compatible. The (O), (I), or (I/O) notation refers to output, input, or input and output.  */
	/*  - DRQ1-DRQ3 (I): DMA Request 1 to 3: These lines are asynchronous channel requests used by*/
	/*  peripheral devices to gain DMA ser-vices. They are prioritized with DRQ1 being the highest*/
	/*  and DRQ3 being the lowest. A request is generated by bringing a request line to an active */
	/*  level. A request line is held active until the corresponding acknowledge line goes active.*/
	/* ------------------------------------------------------------------------------------------ */
	if (pc->model & PCE_IBMPS2_MODEL25) {
		/* Refresh Memory Pin (B19) on IBM Personal System/2  Model 25/30      */
 		pc->dack0 = (val != 0);
		if (pc->dack0) {
			e8237_set_dreq0(&pc->dma, 0);
		}
	}
	else {
		/* DMA Acknowledge 0 Pin    on IBM Personal Computer (Model 5150/5160) */
 		pc->dack0 = (val != 0);
		if (pc->dack0) {
			e8237_set_dreq0(&pc->dma, 0);
		}
	}
	/* ------------------------------------------------------------------------------------------ */
	/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; DMA Controller:       */
	/* ------------------------------------------------------------------------------------------ */
	/* The 8237 Direct Memory Access (DMA) controller and its support logic in the gate array     */
	/* support four channels of 20 address bit DMA. It operates at 4 MHz and handles only 8-bit   */
	/* data transfers. The DMA channel assignments and page register addresses are:               */
	/* ------------------------------------------------------------------------------------------ */
	/* Level:  Assignment:                                                                        */
	/*  - DRQ0: - Not Available                                                                   */
	/*  - DRQ1: - Not Used                                                                        */
	/*  - DRQ2: - Diskette                                                                        */
	/*  - DRQ3: - Fixed Disk                                                                      */
	/* ------------------------------------------------------------------------------------------ */
	/* Figure 1-7. DMA Channel Assignments:                                                       */
	/* ------------------------------------------------------------------------------------------ */
	/* Hex Address: DMA Page Register:                                                            */
	/*  - 081:       - Channel 2:                                                                 */
	/*  - 082:       - Channel 3:                                                                 */
	/*  - 083:       - Channel 1:                                                                 */
	/*  - 087:       - Channel 0:                                                                 */
	/* ------------------------------------------------------------------------------------------ */
	/* Figure 1-8. DMA Page Register Addresses:                                                   */
	/* ------------------------------------------------------------------------------------------ */
	/* Three of the DMA channels (1, 2, and 3) are available on the I/O bus and support high-speed*/
	/* data transfers between I/O devices and memory without microprocessor intervention.         */
	/*                                                                                            */
	/* DMA data transfers take six clock cycles of 250 ns, or 1.5 micro-seconds. I/O CH RDY can be*/
	/* pulled inactive to add wait states to allow more time for slower devices.                  */
	/* ------------------------------------------------------------------------------------------ */
}

static
void pc_set_timer1_out (ibmpc_t *pc, unsigned char val)
{
	/* --------------------------------------------------------------------------------------- */
	/* IBM PC (Model 5150/5160): Quote from "Technical Reference"; I/O Channel Description:    */
	/* --------------------------------------------------------------------------------------- */
	/* Signal Name:                                                                            */
	/*  - +DRQ2 (Input): DMA Request 1 to 3: These lines are asynchronous channel requests     */
	/*  used by peripheral devices to gain DMA service. They are prioritized with DRQ3 being   */
	/*  the lowest and DRQ1 being the highest. A DRQ line must be held high until the          */
	/*  corresponding DACK line goes active.                                                   */
	/*  - -DACK3 (Output): DMA Acknowledge 0 to 3: These lines are used to acknowledge DMA     */
	/*  requests (DRQ1-DRG3) and refresh system dynamic memory (-DACK0). They are active low.  */
	/*  - -DACK1 (Output): See -DACK3                                                          */
	/*  - -DACK0 (Output): See -DACK3                                                          */
	/* --------------------------------------------------------------------------------------- */
	/* ------------------------------------------------------------------------------------------ */
	/* IBM Personal Computer Model 25/30: Quote from "Technical Reference"; I/O Channel; Signal   */
	/* Description:                                                                               */
	/* ------------------------------------------------------------------------------------------ */
	/* The following is a description of the I/O channel signal lines. All lines are TTL-         */
	/* compatible. The (O), (I), or (I/O) notation refers to output, input, or input and output.  */
	/*  - -DACK1 - -DACK3 (O): -DMA Acknowledge 1 to 3: These lines are used by the controller to */
	/*  acknowledge DMA requests. DACK0 is not available on the Type 8525's I/O channel.          */
	/*  - DRQ1 - DRQ3 (I): DMA Request 1 to 3: These lines are asynchronous channel requests used */
	/*  by peripheral devices to gain DMA ser-vices. They are prioritized with DRQ1 being the     */
	/*  highest and DRQ3 being the lowest. A request is generated by bringing a request line to an*/
	/*  active level. A request line is held active until the corresponding acknowledge line goes */
	/*  active.                                                                                   */
	/* ------------------------------------------------------------------------------------------ */
	/* ------------------------------------------------------------------------------------------ */
	/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; DMA Controller:       */
	/* ------------------------------------------------------------------------------------------ */
	/* The 8237 Direct Memory Access (DMA) controller and its support logic in the gate array     */
	/* support four channels of 20 address bit DMA. It operates at 4 MHz and handles only 8-bit   */
	/* data transfers. The DMA channel assignments and page register addresses are:               */
	/* ------------------------------------------------------------------------------------------ */
	/* Level:  Assignment:                                                                        */
	/*  - DRQ0: - Not Available                                                                   */
	/*  - DRQ1: - Not Used                                                                        */
	/*  - DRQ2: - Diskette                                                                        */
	/*  - DRQ3: - Fixed Disk                                                                      */
	/* ------------------------------------------------------------------------------------------ */
	/* Figure 1-7. DMA Channel Assignments:                                                       */
	/* ------------------------------------------------------------------------------------------ */
	/* Hex Address: DMA Page Register:                                                            */
	/*  - 081:       - Channel 2:                                                                 */
	/*  - 082:       - Channel 3:                                                                 */
	/*  - 083:       - Channel 1:                                                                 */
	/*  - 087:       - Channel 0:                                                                 */
	/* ------------------------------------------------------------------------------------------ */
	/* Figure 1-8. DMA Page Register Addresses:                                                   */
	/* ------------------------------------------------------------------------------------------ */
	/* Three of the DMA channels (1, 2, and 3) are available on the I/O bus and support high-speed*/
	/* data transfers between I/O devices and memory without microprocessor intervention.         */
	/*                                                                                            */
	/* DMA data transfers take six clock cycles of 250 ns, or 1.5 micro-seconds. I/O CH RDY can be*/
	/* pulled inactive to add wait states to allow more time for slower devices.                  */
	/* ------------------------------------------------------------------------------------------ */
	/* ------------------------------------------------------------------------------------------ */
	/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; System Timer:         */
	/* ------------------------------------------------------------------------------------------ */
	/* The system timer is an 8253 programmable interval timer/counter, or equivalent, that       */
	/* functions as an arrangement of four external I/O ports (hex 0040 through 0043). It receives*/
	/*its 1.19 MHz clock from the I/O support gate array. These ports are treated as counters; the*/
	/* fourth, address hex 0043, is a control register for mode programming.                      */
	/*                                                                                            */
	/* The content of a selected counter may be latched without disturbing the counting operation */
	/*by writing to the control register. If the content of the counter is critical to a program's*/
	/* operation, a second read is recommended for verification.                                  */
	/* -------------------________________------------------------------------------------------- */
	/* System Bus:------>:|System Timer: |                                                        */
	/*          __________| - Gate 0     |                                                        */
	/*          |  _______| - Clock  in 0|                                                        */
	/* +5 Vdc __|__|______| - Gate 1     |                                                        */
	/* -RAS_SIP ---|---___| - Clock  in 1|                                                        */
	/* I/O Port  __|______| - Gate 2     |                                                        */
	/* Hex 0061__| |______| - Clock  in 2|                                                        */
	/* Port Bit 0  |      | - Clock Out 0|_______________IRQ 0________                            */
	/* 1.19 Mhz____|      | - Clock Out 1|    |_Driver_|                                          */
	/*                    | - Clock Out 2|______|__ _|_______                                     */
	/* I/O Port           ----------------  | AND | | Low   |--> To Beeper                        */
	/* Hex 0061 ----------------------------|-----| | Pass  |                                     */
	/* Port Bit 1                                   | Filter|                                     */
	/* ---------------------------------------------|-------|------------------------------------ */
	/* Figure 1-4. System Timer Block Diagram                                                     */
	/* ------------------------------------------------------------------------------------------ */
	/* The three timers are programmable and are used by the system as follows:                   */
	/*                                                                                            */
	/* Channel 0 is a general-purpose timer providing a constant time base.                       */
	/* Channel 0:   System Timer:                                                                 */
	/*  - GATE    0: - Tied High                                                                  */
	/*  - CLK IN  0: - 1.19 MHz OSC                                                               */
	/*  - CLK OUT 0: - IRQ 0                                                                      */
	/* Channel 1 is for internal diagnostic tests.                                                */
	/* Channel 1:   Diagnostic:                                                                   */
	/*  - GATE    1: - Tied High                                                                  */
	/*  - CLK IN  1: - -RAS_SIP from system support gate array                                    */
	/*  - CLK OUT 2: - Not connected                                                              */
	/* Channel 2 supports the tone generation for the audio.                                      */
	/* Channel 2:   Tone Generation:                                                              */
	/*  - GATE    2: Controlled by bit 0 at port hex 61                                           */
	/*  - CLK IN  2: 1.19 MHz OSC                                                                 */
	/*  - CLK OUT 2: To the beeper data of the I/O support gate array                             */
	/* ------------------------------------------------------------------------------------------ */
	/* ------------------------------------------------------------------------------------------ */
	/* IBM Personal Computer Model 25/30: Quote from "Technical Reference"; I/O Channel; Signal   */
	/* Description:                                                                               */
	/* ------------------------------------------------------------------------------------------ */
	/* The following is a description of the I/O channel signal lines. All lines are TTL-         */
	/* compatible. The (O), (I), or (I/O) notation refers to output, input, or input and output.  */
	/*  - -DACK1 - -DACK3 (O): -DMA Acknowledge 1 to 3: These lines are used by the controller to */
	/*  acknowledge DMA requests. DACK0 is not available on the Type 8525's I/O channel.          */
	/*  - DRQ1 - DRQ3 (I): DMA Request 1 to 3: These lines are asynchronous channel requests used */
	/*  by peripheral devices to gain DMA ser-vices. They are prioritized with DRQ1 being the     */
	/*  highest and DRQ3 being the lowest. A request is generated by bringing a request line to an*/
	/*  active level. A request line is held active until the corresponding acknowledge line goes */
	/*  active.                                                                                   */
	/* ------------------------------------------------------------------------------------------ */
	if (pc->model & PCE_IBMPS2_MODEL25) {
		/* Refresh Memory Pin (B19) on IBM Personal System/2  Model 25/30      */
		if (pc->dack0 == 0) {
			if ((pc->timer1_out == 0) && (val != 0)) {
				e8237_set_dreq0(&pc->dma, 1);
			}
		}
		pc->timer1_out = (val != 0);
	}
	else {
		/* DMA Acknowledge 0 Pin    on IBM Personal Computer (Model 5150/5160) */
		if (pc->dack0 == 0) {
			if ((pc->timer1_out == 0) && (val != 0)) {
				e8237_set_dreq0(&pc->dma, 1);
			}
		}
		pc->timer1_out = (val != 0);
	}
	/* ---------------------------------------------------------------------------- */
	/* IBM PC (Model 5150): Quote from "Technical Reference"; System Timers:        */
	/* ---------------------------------------------------------------------------- */
	/* Three programmable timer/counters are used by the system as follows: Channel */
	/* 0 is a general-purpose timer providing a constant time base for implementing */
	/* a time-of-day clock, Channel 1 times and requests refresh cycles from the    */
	/* the Direct Memory Access (DMA) channel, and Channel 2 supports the tone      */
	/* generation for the speaker. Each channel has a minimum timing resolution of  */
	/* 1.05-us.                                                                     */
	/* ---------------------------------------------------------------------------- */
	/* IBM PC (Model 5160): Quote from "Technical Reference"; System Timers:        */
	/* ---------------------------------------------------------------------------- */
	/* Three programmable timer/counters are used by the system as follows: Channel */
	/* 0 is used as a general-purpose timer providing a constant time base for      */
	/* implementing a time-of-day clock; Channel 1 is used to time and request      */
	/* refresh cycles from the DMA channel; and Channel 2 is used to support the    */
	/* tone generation for the audio speaker. Each channel has a minimum timing     */
	/* resolution of 1.05-us.                                                       */
	/* ---------------------------------------------------------------------------- */
	/* ------------------------------------------------------------------------------------------ */
	/* IBM PC (Model 5150/5160): Quote from "Technical Reference"; Speaker Circuit:               */
	/* ------------------------------------------------------------------------------------------ */
	/* The sound system has a small, permanent magnet, 57.15 mm (2-1/5 in.) speaker.              */
	/* --------------------------- IBM PC Model 5150 Technical Reference: ----------------------- */
	/* The speaker can be driven from one or both of two sources:                                 */
	/*  - An 8255A-5 programmable peripheral interface (PPI) output bit. The address and bit are  */
	/*  defined in the "I/O Address Map".                                                         */
	/*  - A timer clock channel, the output of which is programmable within the functions of the  */
	/*  8253-5 timer when using a 1.19-MHz clock input. The timer gate also is controlled by an   */
	/*  8255A-5 PPI output-port bit. Address and bit assignment are in the "I/O Address Map".     */
	/* --------------------------- IBM PC Model 5160 Technical Reference: ----------------------- */
	/* The speaker's control circuits and driver are on the system board. The speaker connects    */
	/* through a 2-wire interface that attaches to a 3-pin connector on the system board.         */
	/*                                                                                            */
	/* The speaker drive circuit is capable of approximately 1/2 watt of power. The control       */
	/* circuits allow the speaker to be driven three different ways: 1.) a direct program control */
	/* register bit may be toggled to generate a pulse train; 2.) the output from Channel 2 of the*/
	/* timer counter may be programmed to generate a waveform to the speaker; 3.) the clock input */
	/* to the timer counter can be modulated with a program-controlled I/O register bit. All three*/
	/* methods may be performed simultaneously.                                                   */
	/* ------------------------------------------------------------------------------------------ */
	/* Speaker Drive System Block Diagram (1.19 MHz):                                             */
	/* ------------------------------------------------------------------------------------------ */
	/* PPI Bit 1, I/O Address Hex 0061:          -> AND:->Driver:->Low Pass Filter:-> To Speaker: */
	/* -> Clock In 2, Gate 2, Timer Clock Out 2: -> AND:->Driver:->Low Pass Filter:-> To Speaker: */
	/* PPI Bit 0, I/O Address Hex 0061:                                                           */
	/* ------------------------------------------------------------------------------------------ */
	/* Speaker Tone Generation:                                                                   */
	/* ------------------------------------------------------------------------------------------ */
	/* Channel 2 (Tone generation for speaker):                                                   */
	/*  - Gate 2: Controller by 8255A-5 PPI Bit (See I/O Map)                                     */
	/*  - Clock In  2: 1.19318-MHz OSC                                                            */
	/*  - Clock Out 2: Used to drive speaker                                                      */
	/* ------------------------------------------------------------------------------------------ */
}

static
void pc_set_timer2_out (ibmpc_t *pc, unsigned char val)
{
	/* ---------------------------------------------------------------------------- */
	/* IBM PC (Model 5150): Quote from "Technical Reference"; System Timers:        */
	/* ---------------------------------------------------------------------------- */
	/* Three programmable timer/counters are used by the system as follows: Channel */
	/* 0 is a general-purpose timer providing a constant time base for implementing */
	/* a time-of-day clock, Channel 1 times and requests refresh cycles from the    */
	/* the Direct Memory Access (DMA) channel, and Channel 2 supports the tone      */
	/* generation for the speaker. Each channel has a minimum timing resolution of  */
	/* 1.05-us.                                                                     */
	/* ---------------------------------------------------------------------------- */
	/* IBM PC (Model 5160): Quote from "Technical Reference"; System Timers:        */
	/* ---------------------------------------------------------------------------- */
	/* Three programmable timer/counters are used by the system as follows: Channel */
	/* 0 is used as a general-purpose timer providing a constant time base for      */
	/* implementing a time-of-day clock; Channel 1 is used to time and request      */
	/* refresh cycles from the DMA channel; and Channel 2 is used to support the    */
	/* tone generation for the audio speaker. Each channel has a minimum timing     */
	/* resolution of 1.05-us.                                                       */
	/* ---------------------------------------------------------------------------- */
	if (pc->model & PCE_IBMPC_5150) {
		if (val) {
			pc->ppi_port_c[0] |= 0x20;
			pc->ppi_port_c[1] |= 0x20;
		}
		else {
			pc->ppi_port_c[0] &= ~0x20;
			pc->ppi_port_c[1] &= ~0x20;
		}
		if (pc->cas != NULL) {
			pc_cas_set_out (pc->cas, val);
		}
	}
	/* ------------------------------------------------------------------------------------------ */
	/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; System Timer:         */
	/* ------------------------------------------------------------------------------------------ */
	/* The system timer is an 8253 programmable interval timer/counter, or equivalent, that       */
	/* functions as an arrangement of four external I/O ports (hex 0040 through 0043). It receives*/
	/*its 1.19 MHz clock from the I/O support gate array. These ports are treated as counters; the*/
	/* fourth, address hex 0043, is a control register for mode programming.                      */
	/*                                                                                            */
	/* The content of a selected counter may be latched without disturbing the counting operation */
	/*by writing to the control register. If the content of the counter is critical to a program's*/
	/* operation, a second read is recommended for verification.                                  */
	/* -------------------________________------------------------------------------------------- */
	/* System Bus:------>:|System Timer: |                                                        */
	/*          __________| - Gate 0     |                                                        */
	/*          |  _______| - Clock  in 0|                                                        */
	/* +5 Vdc __|__|______| - Gate 1     |                                                        */
	/* -RAS_SIP ---|---___| - Clock  in 1|                                                        */
	/* I/O Port  __|______| - Gate 2     |                                                        */
	/* Hex 0061__| |______| - Clock  in 2|                                                        */
	/* Port Bit 0  |      | - Clock Out 0|_______________IRQ 0________                            */
	/* 1.19 Mhz____|      | - Clock Out 1|    |_Driver_|                                          */
	/*                    | - Clock Out 2|______|__ _|_______                                     */
	/* I/O Port           ----------------  | AND | | Low   |--> To Beeper                        */
	/* Hex 0061 ----------------------------|-----| | Pass  |                                     */
	/* Port Bit 1                                   | Filter|                                     */
	/* ---------------------------------------------|-------|------------------------------------ */
	/* Figure 1-4. System Timer Block Diagram                                                     */
	/* ------------------------------------------------------------------------------------------ */
	/* The three timers are programmable and are used by the system as follows:                   */
	/*                                                                                            */
	/* Channel 0 is a general-purpose timer providing a constant time base.                       */
	/* Channel 0:   System Timer:                                                                 */
	/*  - GATE    0: - Tied High                                                                  */
	/*  - CLK IN  0: - 1.19 MHz OSC                                                               */
	/*  - CLK OUT 0: - IRQ 0                                                                      */
	/* Channel 1 is for internal diagnostic tests.                                                */
	/* Channel 1:   Diagnostic:                                                                   */
	/*  - GATE    1: - Tied High                                                                  */
	/*  - CLK IN  1: - -RAS_SIP from system support gate array                                    */
	/*  - CLK OUT 2: - Not connected                                                              */
	/* Channel 2 supports the tone generation for the audio.                                      */
	/* Channel 2:   Tone Generation:                                                              */
	/*  - GATE    2: Controlled by bit 0 at port hex 61                                           */
	/*  - CLK IN  2: 1.19 MHz OSC                                                                 */
	/*  - CLK OUT 2: To the beeper data of the I/O support gate array                             */
	/* ------------------------------------------------------------------------------------------ */
	/* ------------------------------------------------------------------------------------------ */
	/* IBM PC (Model 5150/5160): Quote from "Technical Reference"; Speaker Circuit:               */
	/* ------------------------------------------------------------------------------------------ */
	/* The sound system has a small, permanent magnet, 57.15 mm (2-1/5 in.) speaker.              */
	/* --------------------------- IBM PC Model 5150 Technical Reference: ----------------------- */
	/* The speaker can be driven from one or both of two sources:                                 */
	/*  - An 8255A-5 programmable peripheral interface (PPI) output bit. The address and bit are  */
	/*  defined in the "I/O Address Map".                                                         */
	/*  - A timer clock channel, the output of which is programmable within the functions of the  */
	/*  8253-5 timer when using a 1.19-MHz clock input. The timer gate also is controlled by an   */
	/*  8255A-5 PPI output-port bit. Address and bit assignment are in the "I/O Address Map".     */
	/* --------------------------- IBM PC Model 5160 Technical Reference: ----------------------- */
	/* The speaker's control circuits and driver are on the system board. The speaker connects    */
	/* through a 2-wire interface that attaches to a 3-pin connector on the system board.         */
	/*                                                                                            */
	/* The speaker drive circuit is capable of approximately 1/2 watt of power. The control       */
	/* circuits allow the speaker to be driven three different ways: 1.) a direct program control */
	/* register bit may be toggled to generate a pulse train; 2.) the output from Channel 2 of the*/
	/* timer counter may be programmed to generate a waveform to the speaker; 3.) the clock input */
	/* to the timer counter can be modulated with a program-controlled I/O register bit. All three*/
	/* methods may be performed simultaneously.                                                   */
	/* ------------------------------------------------------------------------------------------ */
	/* Speaker Drive System Block Diagram (1.19 MHz):                                             */
	/* ------------------------------------------------------------------------------------------ */
	/* PPI Bit 1, I/O Address Hex 0061:          -> AND:->Driver:->Low Pass Filter:-> To Speaker: */
	/* -> Clock In 2, Gate 2, Timer Clock Out 2: -> AND:->Driver:->Low Pass Filter:-> To Speaker: */
	/* PPI Bit 0, I/O Address Hex 0061:                                                           */
	/* ------------------------------------------------------------------------------------------ */
	/* Speaker Tone Generation:                                                                   */
	/* ------------------------------------------------------------------------------------------ */
	/* Channel 2 (Tone generation for speaker):                                                   */
	/*  - Gate 2: Controller by 8255A-5 PPI Bit (See I/O Map)                                     */
	/*  - Clock In  2: 1.19318-MHz OSC                                                            */
	/*  - Clock Out 2: Used to drive speaker                                                      */
	/* ------------------------------------------------------------------------------------------ */
	else if (pc->model & PCE_COMPAQ_PORTABLE) {
		if (val) {
			pc->ppi_port_c[0] |= 0x20;
			pc->ppi_port_c[1] |= 0x20;
		}
		else {
			pc->ppi_port_c[0] &= ~0x20;
			pc->ppi_port_c[1] &= ~0x20;
		}
	}
	else if (pc->model & PCE_COMPAQ_PORTABLE_PLUS) {
		if (val) {
			pc->ppi_port_c[0] |= 0x20;
			pc->ppi_port_c[1] |= 0x20;
		}
		else {
			pc->ppi_port_c[0] &= ~0x20;
			pc->ppi_port_c[1] &= ~0x20;
		}
	}
	pc_speaker_set_out (&pc->spk, val);
}

static
void pc_set_mouse (void *ext, int dx, int dy, unsigned button)
{
	chr_mouse_set (dx, dy, button);
}


static
void pc_set_video_mode (ibmpc_t *pc, unsigned mode)
{
	if (pc->model & PCE_IBMPC_5150) {
		pc->ppi_port_a[0] &= ~0x30;
		pc->ppi_port_a[0] |= (mode & 0x03) << 4;
	}
	else if (pc->model & PCE_IBMPC_5160) {
		pc->ppi_port_c[1] &= ~0x03;
		pc->ppi_port_c[1] |= mode & 0x03;
	}
	else if (pc->model & PCE_IBMPS2_MODEL25) {
		if (ibmps2_get_video_cs(pc) >= 0x1)
		{
			/* Chip Select signal not in effect; read/write operations allowed */
			pc->ppi_port_c[1] &= ~0x03;
			pc->ppi_port_c[1] |= mode & 0x03;
		}
		/* Chip Select signal in effect; read/write operations blocked */
	}
	/*	else if (pc->model & PCE_IBMPC_5140) { NOTE: DO NOT ENABLE UNTIL CODE IS READY!!!!
		pc->ppi_port_c[1] &= ~0x03;
		pc->ppi_port_c[1] |= mode & 0x03;
	} */
	else if (pc->model & PCE_COMPAQ_PORTABLE) {
		pc->ppi_port_a[0] &= ~0x30;
		pc->ppi_port_a[0] |= (mode & 0x03) << 4;
	}
	else if (pc->model & PCE_COMPAQ_PORTABLE_PLUS) {
		pc->ppi_port_a[0] &= ~0x30;
		pc->ppi_port_a[0] |= (mode & 0x03) << 4;
	}
	m24_set_video_mode (pc, mode);
}

static
void pc_set_floppy_count (ibmpc_t *pc, unsigned cnt)
{
	pc->fd_cnt = cnt;
	if (pc->model & PCE_IBMPC_5150) {
		pc->ppi_port_a[0] &= ~0xc1;
		if (cnt > 0) {
			pc->ppi_port_a[0] |= 0x01;
			pc->ppi_port_a[0] |= ((cnt - 1) & 3) << 6;
		}
	}
	else if (pc->model & PCE_IBMPC_5160) {
		pc->ppi_port_c[1] &= ~0x0c;
		if (cnt > 0) {
			pc->ppi_port_c[1] |= ((cnt - 1) & 3) << 2;
		}
	}
	else if (pc->model & PCE_IBMPS2_MODEL25) {
		if (ibmps2_get_diskette_cs(pc) >= 0x1)
		{
			/* Chip Select signal not in effect; read/write operations allowed */
			pc->ppi_port_c[1] &= ~0x0c;
			if (cnt > 0) {
				pc->ppi_port_c[1] |= ((cnt - 1) & 3) << 2;
			}
		}
		/* Chip Select signal in effect; read/write operations blocked */
	}
	/* 	else if (pc->model & PCE_IBMPC_5140) { WARNING: DO NOT ENABLE UNTIL CODE IS READY!!!!
		pc->ppi_port_c[1] &= ~0x0c;
		if (cnt > 0) {
			pc->ppi_port_c[1] |= ((cnt - 1) & 3) << 2;
		}
	} */
	else if (pc->model & PCE_COMPAQ_PORTABLE) {
		pc->ppi_port_a[0] &= ~0xc1;
		if (cnt > 0) {
			pc->ppi_port_a[0] |= 0x01;
			pc->ppi_port_a[0] |= ((cnt - 1) & 3) << 6;
		}
	}
	else if (pc->model & PCE_COMPAQ_PORTABLE_PLUS) {
		pc->ppi_port_a[0] &= ~0xc1;
		if (cnt > 0) {
			pc->ppi_port_a[0] |= 0x01;
			pc->ppi_port_a[0] |= ((cnt - 1) & 3) << 6;
		}
	}
}

static
void pc_set_ram_size (ibmpc_t *pc, unsigned long cnt)
{
	if (pc->model & PCE_IBMPC_5150) {
		/* --------------------------------------------------------------- */
		/* Quote from "Technical Reference"; System Memory Map:            */
		/* --------------------------------------------------------------- */
		/* The following pages contain the System Memory Map.              */
		/* System Memory Map for 24/256K System Board (Part 1 of 2)        */
		/* -Start Address :- ------------------Function:------------------ */
		/* Decimal:   Hex  : 64 to 256K Read/Write Memory on System Board  */
		/* |  0  | | 00000 | 64 to 256K Read/Write Memory on System Board  */
		/* | 16K | | 04000 | 64 to 256K Read/Write Memory on System Board  */
		/* | 32K | | 08000 | 64 to 256K Read/Write Memory on System Board  */
		/* | 48K | | 0C000 | 64 to 256K Read/Write Memory on System Board  */
		/* ----------------- --------------------------------------------- */
		/* | 64K | | 10000 | 64 to 256K Read/Write Memory on System Board  */
		/* | 80K | | 14000 | 64 to 256K Read/Write Memory on System Board  */
		/* | 96K | | 18000 | 64 to 256K Read/Write Memory on System Board  */
		/* |112K | | 1C000 | 64 to 256K Read/Write Memory on System Board  */
		/* ----------------- --------------------------------------------- */
		/* |128K | | 20000 | 64 to 256K Read/Write Memory on System Board  */
		/* |144K | | 24000 | 64 to 256K Read/Write Memory on System Board  */
		/* |160K | | 28000 | 64 to 256K Read/Write Memory on System Board  */
		/* |176K | | 2C000 | 64 to 256K Read/Write Memory on System Board  */
		/* ----------------- --------------------------------------------- */
		/* |192K | | 30000 | 64 to 256K Read/Write Memory on System Board  */
		/* |208K | | 34000 | 64 to 256K Read/Write Memory on System Board  */
		/* |224K | | 38000 | 64 to 256K Read/Write Memory on System Board  */
		/* |240K | | 3C000 | 64 to 256K Read/Write Memory on System Board  */
		/* ----------------- --------------------------------------------- */
		/* |256K | | 40000 | Up to 384K Read/Write Memory in I/O Channel   */
		/* |272K | | 44000 | Up to 384K Read/Write Memory in I/O Channel   */
		/* |288K | | 48000 | Up to 384K Read/Write Memory in I/O Channel   */
		/* |304K | | 4C000 | Up to 384K Read/Write Memory in I/O Channel   */
		/* ----------------- --------------------------------------------- */
		/* |320K | | 50000 | Up to 384K Read/Write Memory in I/O Channel   */
		/* |336K | | 54000 | Up to 384K Read/Write Memory in I/O Channel   */
		/* |352K | | 58000 | Up to 384K Read/Write Memory in I/O Channel   */
		/* |368K | | 5C000 | Up to 384K Read/Write Memory in I/O Channel   */
		/* ----------------- --------------------------------------------- */
		/* |384K | | 60000 | Up to 384K Read/Write Memory in I/O Channel   */
		/* |400K | | 64000 | Up to 384K Read/Write Memory in I/O Channel   */
		/* |416K | | 68000 | Up to 384K Read/Write Memory in I/O Channel   */
		/* |432K | | 6C000 | Up to 384K Read/Write Memory in I/O Channel   */
		/* ----------------- --------------------------------------------- */
		/* |448K | | 70000 | Up to 384K Read/Write Memory in I/O Channel   */
		/* |464K | | 74000 | Up to 384K Read/Write Memory in I/O Channel   */
		/* |480K | | 78000 | Up to 384K Read/Write Memory in I/O Channel   */
		/* |496K | | 7C000 | Up to 384K Read/Write Memory in I/O Channel   */
		/* ----------------- --------------------------------------------- */
		/* |512K | | 80000 | Up to 384K Read/Write Memory in I/O Channel   */
		/* |528K | | 84000 | Up to 384K Read/Write Memory in I/O Channel   */
		/* |544K | | 88000 | Up to 384K Read/Write Memory in I/O Channel   */
		/* |560K | | 8C000 | Up to 384K Read/Write Memory in I/O Channel   */
		/* ----------------- --------------------------------------------- */
		/* |576K | | 90000 | Up to 384K Read/Write Memory in I/O Channel   */
		/* |592K | | 94000 | Up to 384K Read/Write Memory in I/O Channel   */
		/* |608K | | 98000 | Up to 384K Read/Write Memory in I/O Channel   */
		/* |624K | | 9C000 | Up to 384K Read/Write Memory in I/O Channel   */
		/* ----------------- --------------------------------------------- */
		pc->ppi_port_a[0] &= 0xf3;
		pc->ppi_port_c[0] &= 0xf0;
		pc->ppi_port_c[1] &= 0xfe;
		if (cnt <= 65536) {
			cnt = (cnt <= 16384) ? 0 : ((cnt - 16384) / 16384);
			pc->ppi_port_a[0] |= (cnt & 3) << 2;
		}
		else {
			cnt = (cnt - 65536) / 32768;
			pc->ppi_port_a[0] |= 0x0c;
			pc->ppi_port_c[0] |= cnt & 0x0f;
			pc->ppi_port_c[1] |= (cnt >> 4) & 0x01;
		}
		/* --------------------------------------------------------------- */
		/* Quote from "Technical Reference"; System Memory Map:            */
		/* --------------------------------------------------------------- */
		/* System Memory Map for 24/256K System Board (Part 2 of 2)        */
		/* -Start Address :- ------------------Function:------------------ */
		/* Decimal:   Hex  : 128K Reserved                                 */
		/* | 640K| | A0000 | 128K Reserved                                 */
		/* | 656K| | A4000 | 128K Reserved                                 */
		/* | 672K| | A8000 | 128K Reserved                                 */
		/* | 688K| | AC000 | 128K Reserved                                 */
		/* ----------------- --------------------------------------------- */
		/* | 704K| | B0000 | Monochrome                                    */
		/* ----------------- --------------------------------------------- */
		/* | 720K| | B4000 | 128K Reserved                                 */
		/* ----------------- --------------------------------------------- */
		/* | 736K| | B8000 | Color/Graphics                                */
		/* ----------------- --------------------------------------------- */
		/* | 752K| | BC000 | 128K Reserved                                 */
		/* ----------------- --------------------------------------------- */
		/* | 768K| | C0000 | 192K Read Only Memory Expansion and Control   */
		/* | 784K| | C4000 | 192K Read Only Memory Expansion and Control   */
		/* ----------------- --------------------------------------------- */
		/* | 800K| | C8000 | Fixed Disk Control                            */
		/* ----------------- --------------------------------------------- */
		/* | 816K| | CC000 | 192K Read Only Memory Expansion and Control   */
		/* ----------------- --------------------------------------------- */
		/* | 832K| | D0000 | 192K Read Only Memory Expansion and Control   */
		/* | 848K| | D4000 | 192K Read Only Memory Expansion and Control   */
		/* | 864K| | D8000 | 192K Read Only Memory Expansion and Control   */
		/* | 880K| | DC000 | 192K Read Only Memory Expansion and Control   */
		/* ----------------- --------------------------------------------- */
		/* | 896K| | E0000 | 192K Read Only Memory Expansion and Control   */
		/* | 912K| | E4000 | 192K Read Only Memory Expansion and Control   */
		/* | 928K| | E8000 | 192K Read Only Memory Expansion and Control   */
		/* | 944K| | EC000 | 192K Read Only Memory Expansion and Control   */
		/* ----------------- --------------------------------------------- */
		/* | 960K| | F0000 | Reserved                                      */
		/* ----------------- --------------------------------------------- */
		/* | 976K| | F4000 | | 48K Base System ROM                         */
		/* | 992K| | F8000 | | 48K Base System ROM                         */
		/* |1008K| | FC000 | | 48K Base System ROM                         */
		/* ----------------- --------------------------------------------- */
	}
	else if (pc->model & PCE_IBMPC_5160) {
		/* --------------------------------------------------------------- */
		/* Quote from "Technical Reference"; System Memory Map:            */
		/* --------------------------------------------------------------- */
		/* The following pages contain the System Memory Map.              */
		/* System Memory Map                          (Part 1 of 2)        */
		/* -Start Address :- ------------------Function:------------------ */
		/* Decimal:   Hex  :   128-256K Read/Write Memory on System Board  */
		/* |  0  | | 00000 |   128-256K Read/Write Memory on System Board  */
		/* | 16K | | 04000 |   128-256K Read/Write Memory on System Board  */
		/* | 32K | | 08000 |   128-256K Read/Write Memory on System Board  */
		/* | 48K | | 0C000 |   128-256K Read/Write Memory on System Board  */
		/* ----------------- --------------------------------------------- */
		/* | 64K | | 10000 |   128-256K Read/Write Memory on System Board  */
		/* | 80K | | 14000 |   128-256K Read/Write Memory on System Board  */
		/* | 96K | | 18000 |   128-256K Read/Write Memory on System Board  */
		/* |112K | | 1C000 |   128-256K Read/Write Memory on System Board  */
		/* ----------------- --------------------------------------------- */
		/* |128K | | 20000 |   128-256K Read/Write Memory on System Board  */
		/* |144K | | 24000 |   128-256K Read/Write Memory on System Board  */
		/* |160K | | 28000 |   128-256K Read/Write Memory on System Board  */
		/* |176K | | 2C000 |   128-256K Read/Write Memory on System Board  */
		/* ----------------- --------------------------------------------- */
		/* |192K | | 30000 |   128-256K Read/Write Memory on System Board  */
		/* |208K | | 34000 |   128-256K Read/Write Memory on System Board  */
		/* |224K | | 38000 |   128-256K Read/Write Memory on System Board  */
		/* |240K | | 3C000 |   128-256K Read/Write Memory on System Board  */
		/* ----------------- --------------------------------------------- */
		/* |256K | | 40000 | Up to 384K Read/Write Memory in I/O Channel   */
		/* |272K | | 44000 | Up to 384K Read/Write Memory in I/O Channel   */
		/* |288K | | 48000 | Up to 384K Read/Write Memory in I/O Channel   */
		/* |304K | | 4C000 | Up to 384K Read/Write Memory in I/O Channel   */
		/* ----------------- --------------------------------------------- */
		/* |320K | | 50000 | Up to 384K Read/Write Memory in I/O Channel   */
		/* |336K | | 54000 | Up to 384K Read/Write Memory in I/O Channel   */
		/* |352K | | 58000 | Up to 384K Read/Write Memory in I/O Channel   */
		/* |368K | | 5C000 | Up to 384K Read/Write Memory in I/O Channel   */
		/* ----------------- --------------------------------------------- */
		/* |384K | | 60000 | Up to 384K Read/Write Memory in I/O Channel   */
		/* |400K | | 64000 | Up to 384K Read/Write Memory in I/O Channel   */
		/* |416K | | 68000 | Up to 384K Read/Write Memory in I/O Channel   */
		/* |432K | | 6C000 | Up to 384K Read/Write Memory in I/O Channel   */
		/* ----------------- --------------------------------------------- */
		/* |448K | | 70000 | Up to 384K Read/Write Memory in I/O Channel   */
		/* |464K | | 74000 | Up to 384K Read/Write Memory in I/O Channel   */
		/* |480K | | 78000 | Up to 384K Read/Write Memory in I/O Channel   */
		/* |496K | | 7C000 | Up to 384K Read/Write Memory in I/O Channel   */
		/* ----------------- --------------------------------------------- */
		/* |512K | | 80000 | Up to 384K Read/Write Memory in I/O Channel   */
		/* |528K | | 84000 | Up to 384K Read/Write Memory in I/O Channel   */
		/* |544K | | 88000 | Up to 384K Read/Write Memory in I/O Channel   */
		/* |560K | | 8C000 | Up to 384K Read/Write Memory in I/O Channel   */
		/* ----------------- --------------------------------------------- */
		/* |576K | | 90000 | Up to 384K Read/Write Memory in I/O Channel   */
		/* |592K | | 94000 | Up to 384K Read/Write Memory in I/O Channel   */
		/* |608K | | 98000 | Up to 384K Read/Write Memory in I/O Channel   */
		/* |624K | | 9C000 | Up to 384K Read/Write Memory in I/O Channel   */
		/* ----------------- --------------------------------------------- */
		cnt = cnt >> 16;
		if (cnt > 0) {
			cnt -= 1;
			if (cnt > 3) {
				cnt = 3;
			}
		}
		pc->ppi_port_c[0] &= 0xf3;
		pc->ppi_port_c[0] |= (cnt << 2) & 0x0c;
		/* --------------------------------------------------------------- */
		/* Quote from "Technical Reference"; System Memory Map:            */
		/* --------------------------------------------------------------- */
		/* System Memory Map                          (Part 2 of 2)        */
		/* -Start Address :- ------------------Function:------------------ */
		/* Decimal:   Hex  : 128K Reserved                                 */
		/* | 640K| | A0000 | 128K Reserved                                 */
		/* | 656K| | A4000 | 128K Reserved                                 */
		/* | 672K| | A8000 | 128K Reserved                                 */
		/* | 688K| | AC000 | 128K Reserved                                 */
		/* ----------------- --------------------------------------------- */
		/* | 704K| | B0000 | Monochrome                                    */
		/* ----------------- --------------------------------------------- */
		/* | 720K| | B4000 | 128K Reserved                                 */
		/* ----------------- --------------------------------------------- */
		/* | 736K| | B8000 | Color/Graphics                                */
		/* ----------------- --------------------------------------------- */
		/* | 752K| | BC000 | 128K Reserved                                 */
		/* ----------------- --------------------------------------------- */
		/* | 768K| | C0000 | 192K Read Only Memory Expansion and Control   */
		/* | 784K| | C4000 | 192K Read Only Memory Expansion and Control   */
		/* ----------------- --------------------------------------------- */
		/* | 800K| | C8000 | Fixed Disk Control                            */
		/* ----------------- --------------------------------------------- */
		/* | 816K| | CC000 | 192K Read Only Memory Expansion and Control   */
		/* ----------------- --------------------------------------------- */
		/* | 832K| | D0000 | 192K Read Only Memory Expansion and Control   */
		/* | 848K| | D4000 | 192K Read Only Memory Expansion and Control   */
		/* | 864K| | D8000 | 192K Read Only Memory Expansion and Control   */
		/* | 880K| | DC000 | 192K Read Only Memory Expansion and Control   */
		/* ----------------- --------------------------------------------- */
		/* | 896K| | E0000 | 192K Read Only Memory Expansion and Control   */
		/* | 912K| | E4000 | 192K Read Only Memory Expansion and Control   */
		/* | 928K| | E8000 | 192K Read Only Memory Expansion and Control   */
		/* | 944K| | EC000 | 192K Read Only Memory Expansion and Control   */
		/* ----------------- --------------------------------------------- */
		/* | 960K| | F0000 | | 64K Base System ROM BIOS and BASIC          */
		/* | 976K| | F4000 | | 64K Base System ROM BIOS and BASIC          */
		/* | 992K| | F8000 | | 64K Base System ROM BIOS and BASIC          */
		/* |1008K| | FC000 | | 64K Base System ROM BIOS and BASIC          */
		/* ----------------- --------------------------------------------- */
	}
	else if (pc->model & PCE_IBMPS2_MODEL25) {
		/* -------------------------------------------------------------------------------------- */
		/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; Read/Write Memory:*/
		/* -------------------------------------------------------------------------------------- */
		/* The system board supports 640K bytes of read/write memory. The first 128K is optional  */
		/* and consists of four 64K by 4-bit and two 64K by 1-bit chips. Sockets are provided on  */
		/* the system board for this optional memory.                                             */
		/*                                                                                        */
		/* The next 512K (from 128K to 640K) is standard and is arranged as two banks of 256K by  */
		/* 9-bit single-inline packages (SIPs). All read/write memory is parity-checked.          */
		/*                                                                                        */
		/* The System Board RAM Control/Status register, hex 6B, is part of the system gate array */
		/* and may be used to remap memory. Remapping occurs when the power-on self-test (POST)   */
		/* senses memory on the I/O channel that is in contention with system memory. Also, if the*/
		/* first 128K is not installed or a failure in the first 128K is sensed, POST remaps the  */
		/* remainder of memory to allow the system to operate.                                    */
		/* -------------------------------------------------------------------------------------- */
		/* Bit: Function:                                                                         */
		/*  - 7: - Parity Check Pointer:                                                          */
		/*       - - 1 = Lower 128K failed                                                        */
		/*       - - 0 = Upper 512K failed                                                        */
		/* - 6:  - -Enable RAM, 90000-9FFFF                                                       */
		/* - 5:  - -Enable RAM, 80000-8FFFF                                                       */
		/* - 4:  - -Enable RAM, 70000-7FFFF                                                       */
		/* - 3:  - -Enable RAM, 60000-6FFFF                                                       */
		/* - 2:  - -Enable RAM, 50000-5FFFF                                                       */
		/* - 1:  - -Enable RAM, 40000-4FFFF                                                       */
		/* - 0:  - Remap Low Memory                                                               */
		/* -------------------------------------------------------------------------------------- */
		/* Figure 1-11. System Board RAM Control/Status Register:                                 */
		/* -------------------------------------------------------------------------------------- */
		cnt = cnt >> 16;
		if (cnt > 0) {
			cnt -= 1;
			if (cnt > 3) {
				cnt = 3;
			}
		}
		pc->ppi_port_c[0] &= 0xf3;
		pc->ppi_port_c[0] |= (cnt << 2) & 0x0c;
	}
	/* else if (pc->model & PCE_IBMPC_5140) { NOTE: DO NOT ENABLE UNTIL CODE IS READY!!!!
		cnt = cnt >> 16;
		if (cnt > 0) {
			cnt -= 1;
			if (cnt > 3) {
				cnt = 3;
			}
		}
		pc->ppi_port_c[0] &= 0xf3;
		pc->ppi_port_c[0] |= (cnt << 2) & 0x0c;
	} */
	else if (pc->model & PCE_COMPAQ_PORTABLE) {
		pc->ppi_port_a[0] &= 0xf3;
		pc->ppi_port_c[0] &= 0xf0;
		pc->ppi_port_c[1] &= 0xfe;
		if (cnt <= 65536) {
			cnt = (cnt <= 16384) ? 0 : ((cnt - 16384) / 16384);
			pc->ppi_port_a[0] |= (cnt & 3) << 2;
		}
		else {
			cnt = (cnt - 65536) / 32768;
			pc->ppi_port_a[0] |= 0x0c;
			pc->ppi_port_c[0] |= cnt & 0x0f;
			pc->ppi_port_c[1] |= (cnt >> 4) & 0x01;
		}
	}
	else if (pc->model & PCE_COMPAQ_PORTABLE_PLUS) {
		pc->ppi_port_a[0] &= 0xf3;
		pc->ppi_port_c[0] &= 0xf0;
		pc->ppi_port_c[1] &= 0xfe;
		if (cnt <= 65536) {
			cnt = (cnt <= 16384) ? 0 : ((cnt - 16384) / 16384);
			pc->ppi_port_a[0] |= (cnt & 3) << 2;
		}
		else {
			cnt = (cnt - 65536) / 32768;
			pc->ppi_port_a[0] |= 0x0c;
			pc->ppi_port_c[0] |= cnt & 0x0f;
			pc->ppi_port_c[1] |= (cnt >> 4) & 0x01;
		}
	}
}


static
void pc_setup_psu(ibmpc_t *pc, ini_sct_t *ini)
{
	/* unsigned   fdcnt;
	int        patch_init, patch_int19, memtest; */
	const char *psumodel;
	ini_sct_t  *sct;
	/* Power Supply Unit */
	sct = ini_next_sct(ini, NULL, "power");
	if (sct == NULL) {
		sct = ini;
	}
	ini_get_string(sct, "psumodel", &psumodel, "0000");
	/* ini_get_uint16(sct, "boot", &pc->bootdrive, 128);
	ini_get_uint16(sct, "floppy_disk_drives", &fdcnt, 2);
	ini_get_bool(sct, "rtc", &pc->support_rtc, 1);
	ini_get_bool(sct, "patch_bios_init", &patch_init, 1);
	ini_get_bool(sct, "patch_bios_int19", &patch_int19, 1);
	ini_get_bool(sct, "memtest", &memtest, 1); */
	if (strcmp(psumodel, "0000") == 0) {
		pce_log(MSG_INF, "Power Supply: 300  Watt\n");
		/* Power Supply Wattage Rating */
		pc->psu_watt_1 = 0x300; /* Power Supply Wattage Rating (first  digits as in "300") */
		pc->psu_watt_2 = 0x000; /* Power Supply Wattage Rating (second digits as in ".00") */
		/* +5 VDC (Direct Current) */
		pc->psu_dc_output_voltage_nominal_plus_5_vdc_minimum_current_amps = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_plus_5_vdc_maximum_current_amps = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_plus_5_vdc_regulation_tolerance_plus_percentage = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_plus_5_vdc_regulation_tolerance_minus_percentage = 0x0050000000;
		/* -5 VDC (Direct Current) */
		pc->psu_dc_output_voltage_nominal_minus_5_vdc_minimum_current_amps = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_minus_5_vdc_maximum_current_amps = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_minus_5_vdc_regulation_tolerance_plus_percentage = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_minus_5_vdc_regulation_tolerance_minus_percentage = 0x0050000000;
		/* +12VDC (Direct Current) */
		pc->psu_dc_output_voltage_nominal_plus12_vdc_minimum_current_amps = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_plus12_vdc_maximum_current_amps = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_plus12_vdc_regulation_tolerance_plus_percentage = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_plus12_vdc_regulation_tolerance_minus_percentage = 0x0050000000;
		/* -12VDC (Direct Current) */
		pc->psu_dc_output_voltage_nominal_minus12_vdc_minimum_current_amps = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_minus12_vdc_maximum_current_amps = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_minus12_vdc_regulation_tolerance_plus_percentage = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_minus12_vdc_regulation_tolerance_minus_percentage = 0x0050000000;
		/* +12VDC (Direct Current) (secondary) */
		pc->psu_dc_output_voltage_nominal_secondary_plus12_vdc_minimum_current_amps = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_secondary_plus12_vdc_maximum_current_amps = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_secondary_plus12_vdc_regulation_tolerance_plus_percentage = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_secondary_plus12_vdc_regulation_tolerance_minus_percentage = 0x0050000000;
		/* -12VDC (Direct Current) (secondary) */
		pc->psu_dc_output_voltage_nominal_secondary_minus12_vdc_minimum_current_amps = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_secondary_minus12_vdc_maximum_current_amps = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_secondary_minus12_vdc_regulation_tolerance_plus_percentage = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_secondary_minus12_vdc_regulation_tolerance_minus_percentage = 0x0050000000;
		/* Voltage Limits */
		pc->psu_dc_output_voltage_plus_5_maximum_rated_voltage_limit = 0x0050000000;
		pc->psu_dc_output_voltage_plus_5_nominal_value_limit = 0x0050000000;
		pc->psu_dc_output_voltage_plus12_maximum_rated_voltage_limit = 0x0050000000;
		pc->psu_dc_output_voltage_plus12_nominal_value_limit = 0x0050000000;
		/* Voltage Limits (secondary) */
		pc->psu_dc_output_voltage_secondary_plus12_maximum_rated_voltage_limit = 0x0050000000;
		pc->psu_dc_output_voltage_secondary_plus12_nominal_value_limit = 0x0050000000;
		/* Power Good Signal */
		pc->psu_dc_power_good_signal_ttl_compatible_up_value = 0x0050000000;
		pc->psu_dc_power_good_signal_ttl_compatible_up_value_ua = 0x0050000000;
		pc->psu_dc_power_good_signal_ttl_compatible_down_value = 0x0050000000;
		pc->psu_dc_power_good_signal_ttl_compatible_down_value_ua = 0x0050000000;
		/* Power Good Signal (+5 VDC) */
		pc->psu_dc_power_good_signal_output_voltage_down_plus_5_vdc_under_voltage_nominal_sense_level = 0x0050000000;
		pc->psu_dc_power_good_signal_output_voltage_down_plus_5_vdc_over_voltage_nominal_sense_level = 0x0050000000;
		/* Power Good Signal (-5 VDC) */
		pc->psu_dc_power_good_signal_output_voltage_down_minus_5_vdc_under_voltage_nominal_sense_level = 0x0050000000;
		pc->psu_dc_power_good_signal_output_voltage_down_minus_5_vdc_over_voltage_nominal_sense_level = 0x0050000000;
		/* Power Good Signal (+12VDC) */
		pc->psu_dc_power_good_signal_output_voltage_down_plus12_vdc_under_voltage_nominal_sense_level = 0x0050000000;
		pc->psu_dc_power_good_signal_output_voltage_down_plus12_vdc_over_voltage_nominal_sense_level = 0x0050000000;
		/* Power Good Signal (-12VDC) */
		pc->psu_dc_power_good_signal_output_voltage_down_minus12_vdc_under_voltage_nominal_sense_level = 0x0050000000;
		pc->psu_dc_power_good_signal_output_voltage_down_minus12_vdc_over_voltage_nominal_sense_level = 0x0050000000;
		/* Power Good Signal (+12VDC) (secondary) */
		pc->psu_dc_power_good_signal_output_voltage_down_secondary_plus12_vdc_under_voltage_nominal_sense_level = 0x0050000000;
		pc->psu_dc_power_good_signal_output_voltage_down_secondary_plus12_vdc_over_voltage_nominal_sense_level = 0x0050000000;
		/* Power Good Signal (-12VDC) (secondary) */
		pc->psu_dc_power_good_signal_output_voltage_down_secondary_minus12_vdc_under_voltage_nominal_sense_level = 0x0050000000;
		pc->psu_dc_power_good_signal_output_voltage_down_secondary_minus12_vdc_over_voltage_nominal_sense_level = 0x0050000000;
		/* Power Good Signal Turn-On Delay */
		pc->psu_dc_power_good_signal_turn_on_delay = 0x000;
		/* Power Good Signal */
		pc->psu_dc_power_good_signal = 0x01;
	}
	else if (strcmp(psumodel, "5150") == 0) {
		pce_log(MSG_INF, "Power Supply: 63.5 Watt 4 level (IBM PC Technical Reference)\n");
		pce_log(MSG_INF, "Power Supply for Expansion Unit: 130 Watt 4 level (IBM PC Technical Reference)\n");
		/* Power Supply Wattage Rating */
		pc->psu_watt_1 = 0x0063; /* Power Supply Wattage Rating (first  digits as in "300") */
		pc->psu_watt_2 = 0x0005; /* Power Supply Wattage Rating (second digits as in ".00") */
		/* +5 VDC (Direct Current) */
		pc->psu_dc_output_voltage_nominal_plus_5_vdc_minimum_current_amps = 0x00000230;
		pc->psu_dc_output_voltage_nominal_plus_5_vdc_maximum_current_amps = 0x00000700;
		pc->psu_dc_output_voltage_nominal_plus_5_vdc_regulation_tolerance_plus_percentage = 0x0005;
		pc->psu_dc_output_voltage_nominal_plus_5_vdc_regulation_tolerance_minus_percentage = 0x0004;
		/* -5 VDC (Direct Current) */
		pc->psu_dc_output_voltage_nominal_minus_5_vdc_minimum_current_amps = 0x00000000;
		pc->psu_dc_output_voltage_nominal_minus_5_vdc_maximum_current_amps = 0x00000040;
		pc->psu_dc_output_voltage_nominal_minus_5_vdc_regulation_tolerance_plus_percentage = 0x0010;
		pc->psu_dc_output_voltage_nominal_minus_5_vdc_regulation_tolerance_minus_percentage = 0x0008;
		/* +12VDC (Direct Current) */
		pc->psu_dc_output_voltage_nominal_plus12_vdc_minimum_current_amps = 0x00000000;
		pc->psu_dc_output_voltage_nominal_plus12_vdc_maximum_current_amps = 0x00000200;
		pc->psu_dc_output_voltage_nominal_plus12_vdc_regulation_tolerance_plus_percentage = 0x0005;
		pc->psu_dc_output_voltage_nominal_plus12_vdc_regulation_tolerance_minus_percentage = 0x0004;
		/* -12VDC (Direct Current) */
		pc->psu_dc_output_voltage_nominal_minus12_vdc_minimum_current_amps = 0x00000000;
		pc->psu_dc_output_voltage_nominal_minus12_vdc_maximum_current_amps = 0x00000025;
		pc->psu_dc_output_voltage_nominal_minus12_vdc_regulation_tolerance_plus_percentage = 0x0010;
		pc->psu_dc_output_voltage_nominal_minus12_vdc_regulation_tolerance_minus_percentage = 0x0009;
		/* +12VDC (Direct Current) (secondary) */
		pc->psu_dc_output_voltage_nominal_secondary_plus12_vdc_minimum_current_amps = 0x00000000;
		pc->psu_dc_output_voltage_nominal_secondary_plus12_vdc_maximum_current_amps = 0x00000200;
		pc->psu_dc_output_voltage_nominal_secondary_plus12_vdc_regulation_tolerance_plus_percentage = 0x0005;
		pc->psu_dc_output_voltage_nominal_secondary_plus12_vdc_regulation_tolerance_minus_percentage = 0x0004;
		/* -12VDC (Direct Current) (secondary) */
		pc->psu_dc_output_voltage_nominal_secondary_minus12_vdc_minimum_current_amps = 0x00000000;
		pc->psu_dc_output_voltage_nominal_secondary_minus12_vdc_maximum_current_amps = 0x00000025;
		pc->psu_dc_output_voltage_nominal_secondary_minus12_vdc_regulation_tolerance_plus_percentage = 0x0010;
		pc->psu_dc_output_voltage_nominal_secondary_minus12_vdc_regulation_tolerance_minus_percentage = 0x0009;
		/* Voltage Limits */
		pc->psu_dc_output_voltage_plus_5_maximum_rated_voltage_limit = 0x2000;
		pc->psu_dc_output_voltage_plus_5_nominal_value_limit = 0x0025;
		pc->psu_dc_output_voltage_plus12_maximum_rated_voltage_limit = 0x5000;
		pc->psu_dc_output_voltage_plus12_nominal_value_limit = 0x0060;
		/* Voltage Limits (secondary) */
		pc->psu_dc_output_voltage_secondary_plus12_maximum_rated_voltage_limit = 0x5000;
		pc->psu_dc_output_voltage_secondary_plus12_nominal_value_limit = 0x0060;
		/* Power Good Signal */
		pc->psu_dc_power_good_signal_ttl_compatible_up_value = 0x00000055;
		pc->psu_dc_power_good_signal_ttl_compatible_up_value_ua = 0x0060;
		pc->psu_dc_power_good_signal_ttl_compatible_down_value = 0x00000004;
		pc->psu_dc_power_good_signal_ttl_compatible_down_value_ua = 0x0500;
		/* Power Good Signal (+5 VDC) */
		pc->psu_dc_power_good_signal_output_voltage_down_plus_5_vdc_under_voltage_nominal_sense_level = 0x00000400;
		pc->psu_dc_power_good_signal_output_voltage_down_plus_5_vdc_over_voltage_nominal_sense_level = 0x00000590;
		/* Power Good Signal (-5 VDC) */
		pc->psu_dc_power_good_signal_output_voltage_down_minus_5_vdc_under_voltage_nominal_sense_level = 0x00000400;
		pc->psu_dc_power_good_signal_output_voltage_down_minus_5_vdc_over_voltage_nominal_sense_level = 0x00000590;
		/* Power Good Signal (+12VDC) */
		pc->psu_dc_power_good_signal_output_voltage_down_plus12_vdc_under_voltage_nominal_sense_level = 0x00000960;
		pc->psu_dc_power_good_signal_output_voltage_down_plus12_vdc_over_voltage_nominal_sense_level = 0x00001420;
		/* Power Good Signal (-12VDC) */
		pc->psu_dc_power_good_signal_output_voltage_down_minus12_vdc_under_voltage_nominal_sense_level = 0x00000960;
		pc->psu_dc_power_good_signal_output_voltage_down_minus12_vdc_over_voltage_nominal_sense_level = 0x00001420;
		/* Power Good Signal (+12VDC) (secondary) */
		pc->psu_dc_power_good_signal_output_voltage_down_secondary_plus12_vdc_under_voltage_nominal_sense_level = 0x00000960;
		pc->psu_dc_power_good_signal_output_voltage_down_secondary_plus12_vdc_over_voltage_nominal_sense_level = 0x00001420;
		/* Power Good Signal (-12VDC) (secondary) */
		pc->psu_dc_power_good_signal_output_voltage_down_secondary_minus12_vdc_under_voltage_nominal_sense_level = 0x00000960;
		pc->psu_dc_power_good_signal_output_voltage_down_secondary_minus12_vdc_over_voltage_nominal_sense_level = 0x00001420;
		/* Power Good Signal Turn-On Delay */
		pc->psu_dc_power_good_signal_turn_on_delay = 0x0100;
		/* Power Good Signal */
		pc->psu_dc_power_good_signal = 0x01;
	}
	else if (strcmp(psumodel, "5160") == 0) {
		pce_log(MSG_INF, "Power Supply: 130  Watt 4 level (IBM PC Technical Reference)\n");
		pce_log(MSG_INF, "Power Supply for Expansion Unit: 130 Watt 4 level (IBM PC Technical Reference)\n");
		/* Power Supply Wattage Rating */
		pc->psu_watt_1 = 0x0130; /* Power Supply Wattage Rating (first  digits as in "300") */
		pc->psu_watt_2 = 0x0000; /* Power Supply Wattage Rating (second digits as in ".00") */
		/* +5 VDC (Direct Current) */
		pc->psu_dc_output_voltage_nominal_plus_5_vdc_minimum_current_amps = 0x00000230;
		pc->psu_dc_output_voltage_nominal_plus_5_vdc_maximum_current_amps = 0x00001500;
		pc->psu_dc_output_voltage_nominal_plus_5_vdc_regulation_tolerance_plus_percentage = 0x0005;
		pc->psu_dc_output_voltage_nominal_plus_5_vdc_regulation_tolerance_minus_percentage = 0x0004;
		/* -5 VDC (Direct Current) */
		pc->psu_dc_output_voltage_nominal_minus_5_vdc_minimum_current_amps = 0x00000000;
		pc->psu_dc_output_voltage_nominal_minus_5_vdc_maximum_current_amps = 0x00000030;
		pc->psu_dc_output_voltage_nominal_minus_5_vdc_regulation_tolerance_plus_percentage = 0x0010;
		pc->psu_dc_output_voltage_nominal_minus_5_vdc_regulation_tolerance_minus_percentage = 0x0008;
		/* +12VDC (Direct Current) */
		pc->psu_dc_output_voltage_nominal_plus12_vdc_minimum_current_amps = 0x00000004;
		pc->psu_dc_output_voltage_nominal_plus12_vdc_maximum_current_amps = 0x00000420;
		pc->psu_dc_output_voltage_nominal_plus12_vdc_regulation_tolerance_plus_percentage = 0x0005;
		pc->psu_dc_output_voltage_nominal_plus12_vdc_regulation_tolerance_minus_percentage = 0x0004;
		/* -12VDC (Direct Current) */
		pc->psu_dc_output_voltage_nominal_minus12_vdc_minimum_current_amps = 0x00000000;
		pc->psu_dc_output_voltage_nominal_minus12_vdc_maximum_current_amps = 0x00000025;
		pc->psu_dc_output_voltage_nominal_minus12_vdc_regulation_tolerance_plus_percentage = 0x0010;
		pc->psu_dc_output_voltage_nominal_minus12_vdc_regulation_tolerance_minus_percentage = 0x0009;
		/* +12VDC (Direct Current) (secondary) */
		pc->psu_dc_output_voltage_nominal_secondary_plus12_vdc_minimum_current_amps = 0x00000004;
		pc->psu_dc_output_voltage_nominal_secondary_plus12_vdc_maximum_current_amps = 0x00000420;
		pc->psu_dc_output_voltage_nominal_secondary_plus12_vdc_regulation_tolerance_plus_percentage = 0x0005;
		pc->psu_dc_output_voltage_nominal_secondary_plus12_vdc_regulation_tolerance_minus_percentage = 0x0004;
		/* -12VDC (Direct Current) (secondary) */
		pc->psu_dc_output_voltage_nominal_secondary_minus12_vdc_minimum_current_amps = 0x00000000;
		pc->psu_dc_output_voltage_nominal_secondary_minus12_vdc_maximum_current_amps = 0x00000025;
		pc->psu_dc_output_voltage_nominal_secondary_minus12_vdc_regulation_tolerance_plus_percentage = 0x0010;
		pc->psu_dc_output_voltage_nominal_secondary_minus12_vdc_regulation_tolerance_minus_percentage = 0x0009;
		/* Voltage Limits */
		pc->psu_dc_output_voltage_plus_5_maximum_rated_voltage_limit = 0x2000;
		pc->psu_dc_output_voltage_plus_5_nominal_value_limit = 0x0025;
		pc->psu_dc_output_voltage_plus12_maximum_rated_voltage_limit = 0x5000;
		pc->psu_dc_output_voltage_plus12_nominal_value_limit = 0x0060;
		/* Voltage Limits (secondary) */
		pc->psu_dc_output_voltage_secondary_plus12_maximum_rated_voltage_limit = 0x5000;
		pc->psu_dc_output_voltage_secondary_plus12_nominal_value_limit = 0x0060;
		/* Power Good Signal */
		pc->psu_dc_power_good_signal_ttl_compatible_up_value = 0x00000055;
		pc->psu_dc_power_good_signal_ttl_compatible_up_value_ua = 0x0060;
		pc->psu_dc_power_good_signal_ttl_compatible_down_value = 0x00000004;
		pc->psu_dc_power_good_signal_ttl_compatible_down_value_ua = 0x0500;
		/* Power Good Signal (+5 VDC) */
		pc->psu_dc_power_good_signal_output_voltage_down_plus_5_vdc_under_voltage_nominal_sense_level = 0x00000400;
		pc->psu_dc_power_good_signal_output_voltage_down_plus_5_vdc_over_voltage_nominal_sense_level = 0x00000590;
		/* Power Good Signal (-5 VDC) */
		pc->psu_dc_power_good_signal_output_voltage_down_minus_5_vdc_under_voltage_nominal_sense_level = 0x00000400;
		pc->psu_dc_power_good_signal_output_voltage_down_minus_5_vdc_over_voltage_nominal_sense_level = 0x00000590;
		/* Power Good Signal (+12VDC) */
		pc->psu_dc_power_good_signal_output_voltage_down_plus12_vdc_under_voltage_nominal_sense_level = 0x00000960;
		pc->psu_dc_power_good_signal_output_voltage_down_plus12_vdc_over_voltage_nominal_sense_level = 0x00001420;
		/* Power Good Signal (-12VDC) */
		pc->psu_dc_power_good_signal_output_voltage_down_minus12_vdc_under_voltage_nominal_sense_level = 0x00000960;
		pc->psu_dc_power_good_signal_output_voltage_down_minus12_vdc_over_voltage_nominal_sense_level = 0x00001420;
		/* Power Good Signal (+12VDC) (secondary) */
		pc->psu_dc_power_good_signal_output_voltage_down_secondary_plus12_vdc_under_voltage_nominal_sense_level = 0x00000960;
		pc->psu_dc_power_good_signal_output_voltage_down_secondary_plus12_vdc_over_voltage_nominal_sense_level = 0x00001420;
		/* Power Good Signal (-12VDC) (secondary) */
		pc->psu_dc_power_good_signal_output_voltage_down_secondary_minus12_vdc_under_voltage_nominal_sense_level = 0x00000960;
		pc->psu_dc_power_good_signal_output_voltage_down_secondary_minus12_vdc_over_voltage_nominal_sense_level = 0x00001420;
		/* Power Good Signal Turn-On Delay */
		pc->psu_dc_power_good_signal_turn_on_delay = 0x0100;
		/* Power Good Signal */
		pc->psu_dc_power_good_signal = 0x01;
	}
	else if (strcmp(psumodel, "5155") == 0) {
		pce_log(MSG_INF, "Power Supply: 114  Watt         (IBM PC Technical Reference)\n");
		pce_log(MSG_INF, "Power Supply for Expansion Unit: 130 Watt 4 level (IBM PC Technical Reference)\n");
		/* Power Supply Wattage Rating */
		pc->psu_watt_1 = 0x0114; /* Power Supply Wattage Rating (first  digits as in "300") */
		pc->psu_watt_2 = 0x0000; /* Power Supply Wattage Rating (second digits as in ".00") */
		/* +5 VDC (Direct Current) */
		pc->psu_dc_output_voltage_nominal_plus_5_vdc_minimum_current_amps = 0x00000230;
		pc->psu_dc_output_voltage_nominal_plus_5_vdc_maximum_current_amps = 0x00001120;
		pc->psu_dc_output_voltage_nominal_plus_5_vdc_regulation_tolerance_plus_percentage = 0x0005;
		pc->psu_dc_output_voltage_nominal_plus_5_vdc_regulation_tolerance_minus_percentage = 0x0004;
		/* -5 VDC (Direct Current) */
		pc->psu_dc_output_voltage_nominal_minus_5_vdc_minimum_current_amps = 0x00000000;
		pc->psu_dc_output_voltage_nominal_minus_5_vdc_maximum_current_amps = 0x00000030;
		pc->psu_dc_output_voltage_nominal_minus_5_vdc_regulation_tolerance_plus_percentage = 0x0010;
		pc->psu_dc_output_voltage_nominal_minus_5_vdc_regulation_tolerance_minus_percentage = 0x0008;
		/* +12VDC (Direct Current) */
		pc->psu_dc_output_voltage_nominal_plus12_vdc_minimum_current_amps = 0x00000004;
		pc->psu_dc_output_voltage_nominal_plus12_vdc_maximum_current_amps = 0x00000290;
		pc->psu_dc_output_voltage_nominal_plus12_vdc_regulation_tolerance_plus_percentage = 0x0005;
		pc->psu_dc_output_voltage_nominal_plus12_vdc_regulation_tolerance_minus_percentage = 0x0004;
		/* -12VDC (Direct Current) */
		pc->psu_dc_output_voltage_nominal_minus12_vdc_minimum_current_amps = 0x00000000;
		pc->psu_dc_output_voltage_nominal_minus12_vdc_maximum_current_amps = 0x00000025;
		pc->psu_dc_output_voltage_nominal_minus12_vdc_regulation_tolerance_plus_percentage = 0x0010;
		pc->psu_dc_output_voltage_nominal_minus12_vdc_regulation_tolerance_minus_percentage = 0x0009;
		/* +12VDC (Direct Current) (secondary) */
		pc->psu_dc_output_voltage_nominal_secondary_plus12_vdc_minimum_current_amps = 0x00000004;
		pc->psu_dc_output_voltage_nominal_secondary_plus12_vdc_maximum_current_amps = 0x00000290;
		pc->psu_dc_output_voltage_nominal_secondary_plus12_vdc_regulation_tolerance_plus_percentage = 0x0005;
		pc->psu_dc_output_voltage_nominal_secondary_plus12_vdc_regulation_tolerance_minus_percentage = 0x0004;
		/* -12VDC (Direct Current) (secondary) */
		pc->psu_dc_output_voltage_nominal_secondary_minus12_vdc_minimum_current_amps = 0x00000000;
		pc->psu_dc_output_voltage_nominal_secondary_minus12_vdc_maximum_current_amps = 0x00000025;
		pc->psu_dc_output_voltage_nominal_secondary_minus12_vdc_regulation_tolerance_plus_percentage = 0x0010;
		pc->psu_dc_output_voltage_nominal_secondary_minus12_vdc_regulation_tolerance_minus_percentage = 0x0009;
		/* Voltage Limits */
		pc->psu_dc_output_voltage_plus_5_maximum_rated_voltage_limit = 0x2000;
		pc->psu_dc_output_voltage_plus_5_nominal_value_limit = 0x0025;
		pc->psu_dc_output_voltage_plus12_maximum_rated_voltage_limit = 0x5000;
		pc->psu_dc_output_voltage_plus12_nominal_value_limit = 0x0060;
		/* Voltage Limits (secondary) */
		pc->psu_dc_output_voltage_secondary_plus12_maximum_rated_voltage_limit = 0x5000;
		pc->psu_dc_output_voltage_secondary_plus12_nominal_value_limit = 0x0060;
		/* Power Good Signal */
		pc->psu_dc_power_good_signal_ttl_compatible_up_value = 0x00000055;
		pc->psu_dc_power_good_signal_ttl_compatible_up_value_ua = 0x0060;
		pc->psu_dc_power_good_signal_ttl_compatible_down_value = 0x00000004;
		pc->psu_dc_power_good_signal_ttl_compatible_down_value_ua = 0x0500;
		/* Power Good Signal (+5 VDC) */
		pc->psu_dc_power_good_signal_output_voltage_down_plus_5_vdc_under_voltage_nominal_sense_level = 0x00000450;
		pc->psu_dc_power_good_signal_output_voltage_down_plus_5_vdc_over_voltage_nominal_sense_level = 0x00000650;
		/* Power Good Signal (-5 VDC) */
		pc->psu_dc_power_good_signal_output_voltage_down_minus_5_vdc_under_voltage_nominal_sense_level = 0x00000430;
		pc->psu_dc_power_good_signal_output_voltage_down_minus_5_vdc_over_voltage_nominal_sense_level = 0x00000650;
		/* Power Good Signal (+12VDC) */
		pc->psu_dc_power_good_signal_output_voltage_down_plus12_vdc_under_voltage_nominal_sense_level = 0x00001080;
		pc->psu_dc_power_good_signal_output_voltage_down_plus12_vdc_over_voltage_nominal_sense_level = 0x00001560;
		/* Power Good Signal (-12VDC) */
		pc->psu_dc_power_good_signal_output_voltage_down_minus12_vdc_under_voltage_nominal_sense_level = 0x00001020;
		pc->psu_dc_power_good_signal_output_voltage_down_minus12_vdc_over_voltage_nominal_sense_level = 0x00001560;
		/* Power Good Signal (+12VDC) (secondary) */
		pc->psu_dc_power_good_signal_output_voltage_down_secondary_plus12_vdc_under_voltage_nominal_sense_level = 0x00001080;
		pc->psu_dc_power_good_signal_output_voltage_down_secondary_plus12_vdc_over_voltage_nominal_sense_level = 0x00001560;
		/* Power Good Signal (-12VDC) (secondary) */
		pc->psu_dc_power_good_signal_output_voltage_down_secondary_minus12_vdc_under_voltage_nominal_sense_level = 0x00001020;
		pc->psu_dc_power_good_signal_output_voltage_down_secondary_minus12_vdc_over_voltage_nominal_sense_level = 0x00001560;
		/* Power Good Signal Turn-On Delay */
		pc->psu_dc_power_good_signal_turn_on_delay = 0x0100;
		/* Power Good Signal */
		pc->psu_dc_power_good_signal = 0x01;
	}
	else if (strcmp(psumodel, "custom") == 0) {
		pce_log(MSG_INF, "Power Supply: 300  Watt\n");
		/* Power Supply Wattage Rating */
		pc->psu_watt_1 = 0x300; /* Power Supply Wattage Rating (first  digits as in "300") */
		pc->psu_watt_2 = 0x000; /* Power Supply Wattage Rating (second digits as in ".00") */
		/* +5 VDC (Direct Current) */
		pc->psu_dc_output_voltage_nominal_plus_5_vdc_minimum_current_amps = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_plus_5_vdc_maximum_current_amps = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_plus_5_vdc_regulation_tolerance_plus_percentage = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_plus_5_vdc_regulation_tolerance_minus_percentage = 0x0050000000;
		/* -5 VDC (Direct Current) */
		pc->psu_dc_output_voltage_nominal_minus_5_vdc_minimum_current_amps = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_minus_5_vdc_maximum_current_amps = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_minus_5_vdc_regulation_tolerance_plus_percentage = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_minus_5_vdc_regulation_tolerance_minus_percentage = 0x0050000000;
		/* +12VDC (Direct Current) */
		pc->psu_dc_output_voltage_nominal_plus12_vdc_minimum_current_amps = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_plus12_vdc_maximum_current_amps = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_plus12_vdc_regulation_tolerance_plus_percentage = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_plus12_vdc_regulation_tolerance_minus_percentage = 0x0050000000;
		/* -12VDC (Direct Current) */
		pc->psu_dc_output_voltage_nominal_minus12_vdc_minimum_current_amps = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_minus12_vdc_maximum_current_amps = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_minus12_vdc_regulation_tolerance_plus_percentage = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_minus12_vdc_regulation_tolerance_minus_percentage = 0x0050000000;
		/* +12VDC (Direct Current) (secondary) */
		pc->psu_dc_output_voltage_nominal_secondary_plus12_vdc_minimum_current_amps = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_secondary_plus12_vdc_maximum_current_amps = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_secondary_plus12_vdc_regulation_tolerance_plus_percentage = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_secondary_plus12_vdc_regulation_tolerance_minus_percentage = 0x0050000000;
		/* -12VDC (Direct Current) (secondary) */
		pc->psu_dc_output_voltage_nominal_secondary_minus12_vdc_minimum_current_amps = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_secondary_minus12_vdc_maximum_current_amps = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_secondary_minus12_vdc_regulation_tolerance_plus_percentage = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_secondary_minus12_vdc_regulation_tolerance_minus_percentage = 0x0050000000;
		/* Voltage Limits */
		pc->psu_dc_output_voltage_plus_5_maximum_rated_voltage_limit = 0x0050000000;
		pc->psu_dc_output_voltage_plus_5_nominal_value_limit = 0x0050000000;
		pc->psu_dc_output_voltage_plus12_maximum_rated_voltage_limit = 0x0050000000;
		pc->psu_dc_output_voltage_plus12_nominal_value_limit = 0x0050000000;
		/* Voltage Limits (secondary) */
		pc->psu_dc_output_voltage_secondary_plus12_maximum_rated_voltage_limit = 0x0050000000;
		pc->psu_dc_output_voltage_secondary_plus12_nominal_value_limit = 0x0050000000;
		/* Power Good Signal */
		pc->psu_dc_power_good_signal_ttl_compatible_up_value = 0x0050000000;
		pc->psu_dc_power_good_signal_ttl_compatible_up_value_ua = 0x0050000000;
		pc->psu_dc_power_good_signal_ttl_compatible_down_value = 0x0050000000;
		pc->psu_dc_power_good_signal_ttl_compatible_down_value_ua = 0x0050000000;
		/* Power Good Signal (+5 VDC) */
		pc->psu_dc_power_good_signal_output_voltage_down_plus_5_vdc_under_voltage_nominal_sense_level = 0x0050000000;
		pc->psu_dc_power_good_signal_output_voltage_down_plus_5_vdc_over_voltage_nominal_sense_level = 0x0050000000;
		/* Power Good Signal (-5 VDC) */
		pc->psu_dc_power_good_signal_output_voltage_down_minus_5_vdc_under_voltage_nominal_sense_level = 0x0050000000;
		pc->psu_dc_power_good_signal_output_voltage_down_minus_5_vdc_over_voltage_nominal_sense_level = 0x0050000000;
		/* Power Good Signal (+12VDC) */
		pc->psu_dc_power_good_signal_output_voltage_down_plus12_vdc_under_voltage_nominal_sense_level = 0x0050000000;
		pc->psu_dc_power_good_signal_output_voltage_down_plus12_vdc_over_voltage_nominal_sense_level = 0x0050000000;
		/* Power Good Signal (-12VDC) */
		pc->psu_dc_power_good_signal_output_voltage_down_minus12_vdc_under_voltage_nominal_sense_level = 0x0050000000;
		pc->psu_dc_power_good_signal_output_voltage_down_minus12_vdc_over_voltage_nominal_sense_level = 0x0050000000;
		/* Power Good Signal (+12VDC) (secondary) */
		pc->psu_dc_power_good_signal_output_voltage_down_secondary_plus12_vdc_under_voltage_nominal_sense_level = 0x0050000000;
		pc->psu_dc_power_good_signal_output_voltage_down_secondary_plus12_vdc_over_voltage_nominal_sense_level = 0x0050000000;
		/* Power Good Signal (-12VDC) (secondary) */
		pc->psu_dc_power_good_signal_output_voltage_down_secondary_minus12_vdc_under_voltage_nominal_sense_level = 0x0050000000;
		pc->psu_dc_power_good_signal_output_voltage_down_secondary_minus12_vdc_over_voltage_nominal_sense_level = 0x0050000000;
		/* Power Good Signal Turn-On Delay */
		pc->psu_dc_power_good_signal_turn_on_delay = 0x000;
		/* Power Good Signal */
		pc->psu_dc_power_good_signal = 0x01;
	}
	else {
		pce_log(MSG_INF, "Power Supply: 300  Watt\n");
		/* Power Supply Wattage Rating */
		pc->psu_watt_1 = 0x300; /* Power Supply Wattage Rating (first  digits as in "300") */
		pc->psu_watt_2 = 0x000; /* Power Supply Wattage Rating (second digits as in ".00") */
		/* +5 VDC (Direct Current) */
		pc->psu_dc_output_voltage_nominal_plus_5_vdc_minimum_current_amps = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_plus_5_vdc_maximum_current_amps = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_plus_5_vdc_regulation_tolerance_plus_percentage = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_plus_5_vdc_regulation_tolerance_minus_percentage = 0x0050000000;
		/* -5 VDC (Direct Current) */
		pc->psu_dc_output_voltage_nominal_minus_5_vdc_minimum_current_amps = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_minus_5_vdc_maximum_current_amps = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_minus_5_vdc_regulation_tolerance_plus_percentage = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_minus_5_vdc_regulation_tolerance_minus_percentage = 0x0050000000;
		/* +12VDC (Direct Current) */
		pc->psu_dc_output_voltage_nominal_plus12_vdc_minimum_current_amps = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_plus12_vdc_maximum_current_amps = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_plus12_vdc_regulation_tolerance_plus_percentage = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_plus12_vdc_regulation_tolerance_minus_percentage = 0x0050000000;
		/* -12VDC (Direct Current) */
		pc->psu_dc_output_voltage_nominal_minus12_vdc_minimum_current_amps = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_minus12_vdc_maximum_current_amps = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_minus12_vdc_regulation_tolerance_plus_percentage = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_minus12_vdc_regulation_tolerance_minus_percentage = 0x0050000000;
		/* +12VDC (Direct Current) (secondary) */
		pc->psu_dc_output_voltage_nominal_secondary_plus12_vdc_minimum_current_amps = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_secondary_plus12_vdc_maximum_current_amps = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_secondary_plus12_vdc_regulation_tolerance_plus_percentage = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_secondary_plus12_vdc_regulation_tolerance_minus_percentage = 0x0050000000;
		/* -12VDC (Direct Current) (secondary) */
		pc->psu_dc_output_voltage_nominal_secondary_minus12_vdc_minimum_current_amps = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_secondary_minus12_vdc_maximum_current_amps = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_secondary_minus12_vdc_regulation_tolerance_plus_percentage = 0x0050000000;
		pc->psu_dc_output_voltage_nominal_secondary_minus12_vdc_regulation_tolerance_minus_percentage = 0x0050000000;
		/* Voltage Limits */
		pc->psu_dc_output_voltage_plus_5_maximum_rated_voltage_limit = 0x0050000000;
		pc->psu_dc_output_voltage_plus_5_nominal_value_limit = 0x0050000000;
		pc->psu_dc_output_voltage_plus12_maximum_rated_voltage_limit = 0x0050000000;
		pc->psu_dc_output_voltage_plus12_nominal_value_limit = 0x0050000000;
		/* Voltage Limits (secondary) */
		pc->psu_dc_output_voltage_secondary_plus12_maximum_rated_voltage_limit = 0x0050000000;
		pc->psu_dc_output_voltage_secondary_plus12_nominal_value_limit = 0x0050000000;
		/* Power Good Signal */
		pc->psu_dc_power_good_signal_ttl_compatible_up_value = 0x0050000000;
		pc->psu_dc_power_good_signal_ttl_compatible_up_value_ua = 0x0050000000;
		pc->psu_dc_power_good_signal_ttl_compatible_down_value = 0x0050000000;
		pc->psu_dc_power_good_signal_ttl_compatible_down_value_ua = 0x0050000000;
		/* Power Good Signal (+5 VDC) */
		pc->psu_dc_power_good_signal_output_voltage_down_plus_5_vdc_under_voltage_nominal_sense_level = 0x0050000000;
		pc->psu_dc_power_good_signal_output_voltage_down_plus_5_vdc_over_voltage_nominal_sense_level = 0x0050000000;
		/* Power Good Signal (-5 VDC) */
		pc->psu_dc_power_good_signal_output_voltage_down_minus_5_vdc_under_voltage_nominal_sense_level = 0x0050000000;
		pc->psu_dc_power_good_signal_output_voltage_down_minus_5_vdc_over_voltage_nominal_sense_level = 0x0050000000;
		/* Power Good Signal (+12VDC) */
		pc->psu_dc_power_good_signal_output_voltage_down_plus12_vdc_under_voltage_nominal_sense_level = 0x0050000000;
		pc->psu_dc_power_good_signal_output_voltage_down_plus12_vdc_over_voltage_nominal_sense_level = 0x0050000000;
		/* Power Good Signal (-12VDC) */
		pc->psu_dc_power_good_signal_output_voltage_down_minus12_vdc_under_voltage_nominal_sense_level = 0x0050000000;
		pc->psu_dc_power_good_signal_output_voltage_down_minus12_vdc_over_voltage_nominal_sense_level = 0x0050000000;
		/* Power Good Signal (+12VDC) (secondary) */
		pc->psu_dc_power_good_signal_output_voltage_down_secondary_plus12_vdc_under_voltage_nominal_sense_level = 0x0050000000;
		pc->psu_dc_power_good_signal_output_voltage_down_secondary_plus12_vdc_over_voltage_nominal_sense_level = 0x0050000000;
		/* Power Good Signal (-12VDC) (secondary) */
		pc->psu_dc_power_good_signal_output_voltage_down_secondary_minus12_vdc_under_voltage_nominal_sense_level = 0x0050000000;
		pc->psu_dc_power_good_signal_output_voltage_down_secondary_minus12_vdc_over_voltage_nominal_sense_level = 0x0050000000;
		/* Power Good Signal Turn-On Delay */
		pc->psu_dc_power_good_signal_turn_on_delay = 0x000;
		/* Power Good Signal */
		pc->psu_dc_power_good_signal = 0x01;
	}
}

static
void pc_setup_system (ibmpc_t *pc, ini_sct_t *ini)
{
	unsigned   fdcnt;
	int        patch_init, patch_int19, memtest;
	const char *model;
	ini_sct_t  *sct;
	pc->fd_cnt = 0;
	pc->hd_cnt = 0;
	pc->current_int = 0;
	pc->brk = 0;
	pc->pause = 0;
	/* Power Supply Unit */
	/* pc->psu_watt_1 = 0x300; */
	/* pc->psu_watt_2 = 0x000; */
	sct = ini_next_sct (ini, NULL, "system");
	if (sct == NULL) {
		sct = ini;
	}
	ini_get_string (sct, "model", &model, "5150");
	ini_get_uint16 (sct, "boot", &pc->bootdrive, 128);
	ini_get_uint16 (sct, "floppy_disk_drives", &fdcnt, 2);
	ini_get_bool (sct, "rtc", &pc->support_rtc, 1);
	ini_get_bool (sct, "patch_bios_init", &patch_init, 1);
	ini_get_bool (sct, "patch_bios_int19", &patch_int19, 1);
	ini_get_bool (sct, "memtest", &memtest, 1);
	pce_log_tag (MSG_INF, "SYSTEM:",
		"model=%s floppies=%u patch-init=%d patch-int19=%d\n",
		model, fdcnt, patch_init, patch_int19
	);
	if (strcmp (model, "5150") == 0) {
		pce_log (MSG_INF, "Machine: IBM Personal Computer (Model 5150)\n");
		pc->model = PCE_IBMPC_5150;
		/* -------------------------------------------------------------------------------------- */
		/* Quote from "Technical Reference"; 1-2 System Board:                                    */
		/* -------------------------------------------------------------------------------------- */
		/* The system board fits horizontally in the base of the system unit and is approximately */
		/* 215.5 mm (8-1/2 in.) x 304.8 mm (12 in.). It is a multilayer, single-land-per-channel  */
		/* design with ground and internal planes provided. DC power and a signal from the power  */
		/*supply enter the board through two six-pin connectors. Other connectors on the board are*/
		/* for attaching the keyboard, audio cassette and speaker. Five 64-pin card-edge sockets  */
		/* are also mounted on the board. The I/O channel is bussed across these five I/O slots.  */
		/*                                                                                        */
		/* Two dual-in-line package (DIP) switches (two eight-switch packs) are mounted on the    */
		/* board and can be read under program control. The DIP switches provide the system       */
		/*software with information about the installed options, how much storage the system board*/
		/* has, what type of dispaly adapter is installed, what operation modes are desired when  */
		/* power is switched on (color or black-and-white, 80 or 40-character lines), and the     */
		/* number of diskette drives attached.                                                    */
        /*                                                                                        */
        /*The system board consists of five functional areas: the microprocessor subsystem and its*/
		/* support elements, the read-only memory (ROM) subsystem, the read/write (R/W) memory    */
		/* subsystem, integrated I/O adapters, and the I/O channel. The read/write memory is also */
		/* referred to as random access memory (RAM). All are described in this section.          */
        /* -------------------------------------------------------------------------------------- */
        /* System Board Component Diagram:                                                        */
        /* -------------------------------------------------------------------------------------- */
        /*  - System Expansion Slots: J1 J2 J3 J4 J5 - Cassette I/O: J6 - Keyboard I/O: J7        */
        /*  - System Board Power Connections - Clock Chip Trimmer - IBM Math Coprocessor - Intel  */
		/*  8088 Microprocessor - DIP Switch Block 2 - DIP Switch Block 1 - 64 to 256K Read/Write */
		/* Memory with Parity Checking - Pin 1 - Speaker Output - Cassette Microphone or Auxiliary*/
		/*  Select                                                                                */
		/* -------------------------------------------------------------------------------------- */
	}
	else if (strcmp (model, "5160") == 0) {
		/* -------------------------------------------------------------------------------------- */
		/* Quote from "Technical Reference"; System Unit:                                         */
		/* -------------------------------------------------------------------------------------- */
		/* The system unit is the center of your IBM Personal Computer XT system. The system unit */
		/* contains the system board, which features eight expansion slots, the 8088              */
		/* microprocessor, 40K of ROM (includes BASIC), 128K of base R/W memory, and an audio     */
		/* speaker. A power supply is located in the system unit to supply dc voltages to the     */
		/* system board and internal drives.                                                      */
		/* -------------------------------------------------------------------------------------- */
		pce_log (MSG_INF, "Machine: IBM Personal Computer/XT (Model 5160)\n");
		pc->model = PCE_IBMPC_5160;
		/* -------------------------------------------------------------------------------------- */
		/* Quote from "Technical Reference";, System Board:                                       */
		/* -------------------------------------------------------------------------------------- */
		/* The system board fits horizontally in the base of the system unit of the Personal      */
		/* Computer XT and Portable Personal Computer and is approximately 215.9 mm by 304.8 mm   */
		/* (8-1/2 x 12 in.). It is a multilayer, single-land-per channel design with ground and   */
		/* internal planes provided. DC power and a signal from the power supply enter the board  */
		/* through two 6-pin connectors. Other connectors on the board are for attaching the      */
		/* keyboard and speaker. Eight 62-pin card-edge sockets are also mounted on the board. The*/
		/* I/O channel is bussed across these eight I/O slots. Slot J8 is slightly different from */
		/* the others in that any card placed in it is expected to respond with a 'card selected' */
		/* signal whenever the card is selected.                                                  */
		/*                                                                                        */
		/* A dual in-line package (DIP) switch (one eight-switch pack) is mounted on the board and*/
		/* can be read under program control. The DIP switch provides the system programs with    */
		/* information about the installed options, how much storage the system board has, what   */
		/* type of display adapter is installed, what operation modes are desired when power is   */
		/* switched on (color or black-and-white, 80- or 40-character lines), and the number of   */
		/* diskette drives attached.                                                              */
        /*                                                                                        */
        /* The system board contains the adapter circuits for attaching the serial interface from */
		/*the keyboard. These circuits generate an interrupt to the microprocessor when a complete*/
		/* scan code is received. The interface can request execution of a diagnostic test in the */
		/* keyboard.                                                                              */
        /*                                                                                        */
        /* The system board consists of five functional areas: the processor subsystem and its    */
		/* support elements, the ROM subsystem, the R/W memory subsystem, integrated I/O adapters,*/
		/* and the I/O channel. All are described in this section.                                */
        /* -------------------------------------------------------------------------------------- */
        /* System Board Component Diagram:                                                        */
        /* -------------------------------------------------------------------------------------- */
        /*  - System Expansion Slots: J1 J2 J3 J4 J5 J6 J7 J8           - Keyboard I/O: J9        */
        /*  - System Board Power Connections                      - IBM Math Coprocessor - Intel  */
		/*  8088 Microprocessor - System Configuration DIP Switches       - 64 to 256K Read/Write */
		/*  Memory with Parity Checking - Pin 1 - Speaker Output - ROM BIOS - ROM BASIC           */
        /* -------------------------------------------------------------------------------------- */
	}
	else if (strcmp (model, "5155") == 0) {
		pce_log (MSG_INF, "Machine: IBM Portable PC (Model 5155)\n");
		pc->model = PCE_IBMPC_5160;
		/* -------------------------------------------------------------------------------------- */
		/* Quote from "Technical Reference"; 1-2 System Board:                                    */
		/* -------------------------------------------------------------------------------------- */
		/* See above (IBM PC Model 5160)                                                          */
		/* -------------------------------------------------------------------------------------- */
	}
	else if (strcmp (model, "5140") == 0) {
		pce_log (MSG_INF, "Machine: IBM PC Convertible (Model 5140)\n");
		/* pc->model = PCE_IBMPC_5140; WARNING: DO NOT ENABLE UNTIL CODE IS READY!!!! */
		pc->model = PCE_IBMPC_5160;
	}
	else if (strcmp(model, "25") == 0) {
		pce_log(MSG_INF, "Machine: IBM Personal System/2 (Model 25)\n");
		pc->model = PCE_IBMPS2_MODEL25;
		ibmps2_set_control_register(pc, 0x11111111);
	}
	else if (strcmp(model, "model25") == 0) {
		pce_log(MSG_INF, "Machine: IBM Personal System/2 (Model 25)\n");
		pc->model = PCE_IBMPS2_MODEL25;
		ibmps2_set_control_register(pc, 0x11111111);
	}
	else if (strcmp(model, "ps2model25") == 0) {
		pce_log(MSG_INF, "Machine: IBM Personal System/2 (Model 25)\n");
		pc->model = PCE_IBMPS2_MODEL25;
		ibmps2_set_control_register(pc, 0x11111111);
	}
	else if (strcmp(model, "8525") == 0) {
		pce_log(MSG_INF, "Machine: IBM Personal System/2 (Model 25)\n");
		pc->model = PCE_IBMPS2_MODEL25;
		ibmps2_set_control_register(pc, 0x11111111);
	}
	else if (strcmp(model, "30") == 0) {
		pce_log(MSG_INF, "Machine: IBM Personal System/2 (Model 30)\n");
		pc->model = PCE_IBMPS2_MODEL25;
		ibmps2_set_control_register(pc, 0x11111111);
	}
	else if (strcmp(model, "model30") == 0) {
		pce_log(MSG_INF, "Machine: IBM Personal System/2 (Model 30)\n");
		pc->model = PCE_IBMPS2_MODEL25;
		ibmps2_set_control_register(pc, 0x11111111);
	}
	else if (strcmp(model, "ps2model30") == 0) {
		pce_log(MSG_INF, "Machine: IBM Personal System/2 (Model 30)\n");
		pc->model = PCE_IBMPS2_MODEL25;
		ibmps2_set_control_register(pc, 0x11111111);
	}
	else if (strcmp(model, "8530") == 0) {
		pce_log(MSG_INF, "Machine: IBM Personal System/2 (Model 30)\n");
		pc->model = PCE_IBMPS2_MODEL25;
		ibmps2_set_control_register(pc, 0x11111111);
	}
	else if (strcmp (model, "m24") == 0) {
		pce_log (MSG_INF, "Machine: Olivetti M24\n");
		pc->model = PCE_IBMPC_5160 | PCE_IBMPC_M24;
	}
	else if (strcmp (model, "6300") == 0) {
		pce_log (MSG_INF, "Machine: AT&T 6300\n");
		pc->model = PCE_IBMPC_5160 | PCE_IBMPC_M24;
	}
	else if (strcmp (model, "6060") == 0) {
		pce_log (MSG_INF, "Machine: Xerox 6060\n");
		pc->model = PCE_IBMPC_5160 | PCE_IBMPC_M24;
	}
	else if (strcmp (model, "1600") == 0) {
		pce_log (MSG_INF, "Machine: Logabax 1600\n");
		pc->model = PCE_IBMPC_5160 | PCE_IBMPC_M24;
	}
	else if (strcmp (model, "portable") == 0) {
		pce_log (MSG_INF, "Machine: COMPAQ Portable\n");
		pc->model = PCE_COMPAQ_PORTABLE;
	}
	else if (strcmp (model, "portableplus") == 0) {
		pce_log (MSG_INF, "Machine: COMPAQ Portable Plus\n");
		pc->model = PCE_COMPAQ_PORTABLE_PLUS;
	}
	else if (strcmp (model, "deskpro") == 0) {
		pce_log (MSG_INF, "Machine: COMPAQ Deskpro\n");
		/* pc->model = PCE_COMPAQ_DESKPRO; WARNING: DO NOT ENABLE UNTIL CODE IS READY!!!! */
		pc->model = PCE_IBMPC_5160;
	}
	else {
		pce_log (MSG_ERR, "ERROR: Incorrect Personal Computer model specified: (%s)\n", model);
		pc->model = PCE_IBMPC_5150;
	}
	pc->patch_bios_init = patch_init != 0;
	pc->patch_bios_int19 = patch_int19 != 0;
	pc->memtest = (memtest != 0);
	pc->ppi_port_a[0] = 0x30 | 0x0c;
	pc->ppi_port_a[1] = 0;
	pc->ppi_port_b = 0x08;
	pc->ppi_port_c[0] = 0;
	pc->ppi_port_c[1] = 0;
	if (pc->model & PCE_IBMPC_5160) {
		pc->ppi_port_c[0] |= 0x01;
	}
	else if (pc->model & PCE_IBMPS2_MODEL25) {
		pc->ppi_port_c[0] |= 0x01;
	}
	pc_set_floppy_count (pc, fdcnt);
}

static
void pc_setup_mem (ibmpc_t *pc, ini_sct_t *ini)
{
	unsigned long ram;
	pc->mem = mem_new();
	ini_get_ram (pc->mem, ini, &pc->ram);
	ini_get_rom (pc->mem, ini);
	/* --------------------------------------------------------------- */
	/* IBM PC (Model 5150): Quote from "Technical Reference"; ROM:     */
	/* --------------------------------------------------------------- */
	/* The system board supports both Read Only Memory (ROM) and       */
	/* Random Access Memory (RAM). It has space for up to 512K of      */
	/* ROM or Eraseable Programmable ROM (EPROM). Six module           */
	/* sockets are provided, each of which can accept an 8K or 8 byte  */
	/* device. Five sockets are populated with 40K of ROM. This        */
	/* ROM contains the cassette BASIC interpreter, cassette operating */
	/* system, power-on selftest, Input/Output (I/O) drivers, dot      */
	/* patterns for 128 characters in graphics mode, and a diskette    */
	/* bootstrap loader. The ROM is packaged in 28-pin modules and     */
	/* has an access time of 250-ns and a cycle time of 375-ns.        */
	/* --------------------------------------------------------------- */
	/* IBM PC (Model 5160): Quote from "Technical Reference"; ROM:     */
	/* --------------------------------------------------------------- */
	/* The system board supports both read only memory (ROM) and read/ */
	/* write (R/W) memory. It has space for 64K by 8 of ROM or erasable*/
	/* programmable read-only memory (EPROM). Two module sockets are   */
	/* provided, each of which can accept a 32K or 8K device. One      */
	/* socket has 32K by 8 of ROM, the other 8K by 8 bytes. This ROM   */
	/* contains the power-on self test, I/O drivers, dot patterns for  */
	/* 128 characters in graphics mode, and a diskette bootstrap       */
	/* loader. The ROM is packaged in 28-pin modules and has an access */
	/* time and a cycle time of 250-ns each.                           */
	/* --------------------------------------------------------------- */
	if (pc->ram != NULL) {
		ram = mem_blk_get_size (pc->ram);
	}
	else {
		ram = 65536;
	}
	pc_set_ram_size (pc, ram);
	/* ------------------------------------------------------------------------------------------ */
	/* IBM PC (Model 5150): Quote from "Technical Reference"; RAM:                                */
	/* ------------------------------------------------------------------------------------------ */
	/* The RAM on the system board is as shown in the following chart.                            */
	/* ------------------------------------------------------------------------------------------ */
	/* | System Board: |Minimum Storage:|Maximum Storage:|Memory Modules:|Soldered  (Bank 0  ):|  */
	/*                                                                   |Pluggable (Bank 1-3):|  */
	/* -----------------------------------------------------------------------------------------* */
	/* |    64/256K    |       64K      |      256K      |  64K by 1 bit |    1 Bank  of 9    |   */
	/*                                                                   |    3 Banks of 9    |   */
	/* ------------------------------------------------------------------------------------------ */
	/* Memory greater than the system board's maximum is obtained by adding memory cards in the   */
	/* expansion slots. All memory is parity-checked and consists of dynamic 64K by 1 bit chips   */
	/* with an access time of 250-ns and a cycle time of 410-ns.                                  */
	/* ------------------------------------------------------------------------------------------ */
	/* IBM PC (Model 5160): Quote from "Technical Reference"; RAM:                                */
	/* ------------------------------------------------------------------------------------------ */
	/* The system board also has from 128K by 9 to 256K by 9 of R/W memory. A minimum system would*/
	/* have 128K of memory, with module sockets for an additional 128K. Memory greater than the   */
	/* system board's maximum of 256K is obtained by adding memory cards in the expansion slots.  */
	/*The memory consists of dynamic 64K by 1 chips with an access time of 200-ns and a cycle time*/
	/* of 345-ns. All R/W memory is parity-checked.                                               */
	/* ------------------------------------------------------------------------------------------ */
	if ((pc->ram != NULL) && (pc->memtest == 0)) {
		mem_set_uint16_le (pc->mem, 0x0472, 0x1234);
	}
	/* ------------------------------------------------------------------------------------------ */
	/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; Read/Write Memory:    */
	/* ------------------------------------------------------------------------------------------ */
	/* The system board supports 640K bytes of read/write memory. The first 128K is optional and  */
	/*consists of four 64K by 4-bit and two 64K by 1-bit chips. Sockets are provided on the system*/
	/* board for this optional memory.                                                            */
	/*                                                                                            */
	/* The next 512K (from 128K to 640K) is standard and is arranged as two banks of 256K by 9-bit*/
	/* single-inline packages (SIPs). All read/write memory is parity-checked.                    */
	/*                                                                                            */
	/* The System Board RAM Control/Status register, hex 6B, is part of the system gate array and */
	/* may be used to remap memory. Remapping occurs when the power-on self-test (POST) senses    */
	/* memory on the I/O channel that is in contention with system memory. Also, if the first 128K*/
	/* is not installed or a failure in the first 128K is sensed, POST remaps the remainder of    */
	/* memory to allow the system to operate.                                                     */
	/* ------------------------------------------------------------------------------------------ */
	/* Bit: Function:                                                                             */
	/*  - 7: - Parity Check Pointer:                                                              */
	/*       - - 1 = Lower 128K failed                                                            */
	/*       - - 0 = Upper 512K failed                                                            */
	/* - 6:  - -Enable RAM, 90000-9FFFF                                                           */
	/* - 5:  - -Enable RAM, 80000-8FFFF                                                           */
	/* - 4:  - -Enable RAM, 70000-7FFFF                                                           */
	/* - 3:  - -Enable RAM, 60000-6FFFF                                                           */
	/* - 2:  - -Enable RAM, 50000-5FFFF                                                           */
	/* - 1:  - -Enable RAM, 40000-4FFFF                                                           */
	/* - 0:  - Remap Low Memory                                                                   */
	/* ------------------------------------------------------------------------------------------ */
	/* Figure 1-11. System Board RAM Control/Status Register:                                     */
	/* ------------------------------------------------------------------------------------------ */
	/* ------------------------------------------------------------------------------------------ */
	/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; ROM:                  */
	/* ------------------------------------------------------------------------------------------ */
	/* The system board has 64K by 8-bits of ROM or erasable program-mable read-only memory       */
	/* (EPROM). Two module sockets are pro-vided; both sockets have 32K by 8-bits of ROM. This ROM*/
	/* contains POST, BIOS, dot patterns for 128 characters in graphics mode, and a diskette      */
	/* bootstrap loader. The ROM is packaged in 28-pin modules.                                   */
	/* ------------------------------------------------------------------------------------------ */
}

static
void pc_setup_ports (ibmpc_t *pc, ini_sct_t *ini)
{
	pc->prt = mem_new();
	mem_set_fct (pc->prt, pc,
		pc_get_port8, pc_get_port16, NULL,
		pc_set_port8, pc_set_port16, NULL
	);
}

static
void pc_setup_nvram (ibmpc_t *pc, ini_sct_t *ini)
{
	ini_sct_t     *sct;
	const char    *fname;
	unsigned long base, size;
	pc->nvr = NULL;
	sct = ini_next_sct (ini, NULL, "nvram");
	if (sct == NULL) {
		return;
	}
	ini_get_string (sct, "file", &fname, NULL);
	ini_get_uint32 (sct, "base", &base, 0);
	ini_get_uint32 (sct, "size", &size, 65536);
	pce_log_tag (MSG_INF, "NVRAM:", "addr=0x%08lx size=%lu file=%s\n",
		base, size, (fname == NULL) ? "<>" : fname
	);
	pc->nvr = nvr_new (base, size);
	if (pc->nvr == NULL) {
		pce_log (MSG_ERR, "ERROR: Unable to install system battery.\n");
		return;
	}
	nvr_set_endian (pc->nvr, 0);
	mem_add_blk (pc->mem, nvr_get_mem (pc->nvr), 0);
	if (fname != NULL) {
		if (nvr_set_fname (pc->nvr, fname)) {
			pce_log (MSG_ERR, "ERROR: Unable to load file for system battery: (%s)\n", fname);
		}
	}
}

static
void pc_setup_cpu (ibmpc_t *pc, ini_sct_t *ini)
{
	ini_sct_t     *sct;
	const char    *model;
	unsigned      speed;
	sct = ini_next_sct (ini, NULL, "cpu");
	ini_get_string (sct, "model", &model, "8088");
	ini_get_uint16 (sct, "speed", &speed, 0);
	pce_log_tag (MSG_INF, "CPU:", "model=%s speed=%uX\n",
		model, speed
	);
	pc->cpu = e86_new();
	if (pc_set_cpu_model (pc, model)) {
		pce_log (MSG_ERR, "ERROR: Incorrect CPU frequency specified: (%s)\n", model);
	}
	e86_set_mem(pc->cpu, pc->mem,
		(e86_get_uint8_f)&mem_get_uint8,
		(e86_set_uint8_f)&mem_set_uint8,
		(e86_get_uint16_f)&mem_get_uint16_le,
		(e86_set_uint16_f)&mem_set_uint16_le,
		(e86_get_uint32_f)&mem_get_uint32_le,
		(e86_set_uint32_f)&mem_set_uint32_le
	);
	e86_set_prt(pc->cpu, pc->prt,
		(e86_get_uint8_f)&mem_get_uint8,
		(e86_set_uint8_f)&mem_set_uint8,
		(e86_get_uint16_f)&mem_get_uint16_le,
		(e86_set_uint16_f)&mem_set_uint16_le,
		(e86_get_uint32_f)&mem_get_uint32_le,
		(e86_set_uint32_f)&mem_set_uint32_le
	);
	if (pc->ram != NULL) {
		e86_set_ram (pc->cpu, pc->ram->data, pc->ram->size);
	}
	else {
		e86_set_ram (pc->cpu, NULL, 0);
	}
	pc->cpu->op_ext = pc;
	pc->cpu->op_hook = &pc_e86_hook;
	pc->speed_current = speed;
	pc->speed_saved = speed;
	pc->speed_clock_extra = 0;
}

static
void pc_setup_dma (ibmpc_t *pc, ini_sct_t *ini)
{
	unsigned long addr;
	ini_sct_t     *sct;
	mem_blk_t     *blk;
	/* ------------------------------------------------------------------- */
	/* IBM PC (Model 5150/5160): Quote from "Technical Reference"; DMA:    */
	/* ------------------------------------------------------------------- */
	/* The microprocessor is supported by a set of high-function support   */
	/* devices providing four channels of 20-bit direct-memory access      */
	/* (DMA), three 16-bit timer/counter channels, and eight               */
	/* prioritized interrupt levels.                                       */
	/*                                                                     */
	/* Three of the four DMA channels are available on the I/O bus and     */
	/* support high-speed data transfers between I/O devices and           */
	/* memory without microprocessor intervention. The fourth DMA          */
	/* channel is programmed to refresh the system dynamic memory.         */
	/* This is done by programming a channel of the timer/counter          */
	/* device to periodically request a dummy DMA transfer. This           */
	/* action creates a memory-read cycle, which is available to refresh   */
	/* dynamic storage both on the system board and in the system          */
	/* expansion slots. All DMA data transfers, except the refresh         */
	/* channel, take five microprocessor clocks of 210-ns, or 1.05-us if   */
	/* the microprocessor ready line is not deactivated. Refresh DMA       */
	/* cycles take four clocks or 840-ns.                                  */
	/* ------ Listed under System Interrupts for Model 5160 Manual: ------ */
	/* Of the eight prioritized levels of interrupt, six are bussed to the */
	/* system expansion slots for use by feature cards. Two levels are     */
	/* used on the system board. Level 0, the highest priority, is         */
	/* attached to Channel 0 of the timer/counter device and provides a    */
	/* periodic interrupt for the time-of-day clock. Level 1 is attached   */
	/* to the keyboard adapter circuits and receives an interrupt for each */
	/* scan code sent by the keyboard. The non-maskable interrupt          */
	/* (NMI) of the 8088 is used to report memory parity errors.           */
	/* ------------------------------------------------------------------- */
	sct = ini_next_sct (ini, NULL, "dmac");
	ini_get_uint32 (sct, "address", &addr, 0);
	pce_log_tag (MSG_INF, "DMAC:", "addr=0x%08x size=0x%04x\n", addr, 16);
	/* --------------------------------------------------------------------------------------- */
	/* IBM PC (Model 5150/5160): Quote from "Technical Reference"; I/O Channel Description:    */
	/* --------------------------------------------------------------------------------------- */
	/* Signal Name:                                                                            */
	/*  - +DRQ2 (Input): DMA Request 1 to 3: These lines are asynchronous channel requests     */
	/*  used by peripheral devices to gain DMA service. They are prioritized with DRQ3 being   */
	/*  the lowest and DRQ1 being the highest. A DRQ line must be held high until the          */
	/*  corresponding DACK line goes active.                                                   */
	/* --------------------------------------------------------------------------------------- */
	/* ------------------------------------------------------------------------------------------ */
	/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; I/O Channel; Signal   */
	/* Description:                                                                               */
	/* ------------------------------------------------------------------------------------------ */
	/* The following is a description of the I/O channel signal lines. All lines are TTL-         */
	/* compatible. The (O), (I), or (I/O) notation refers to output, input, or input and output.  */
	/*  - DRQ1-DRQ3 (I): DMA Request 1 to 3: These lines are asynchro-nous channel requests used  */
	/*  by peripheral devices to gain DMA ser-vices. They are prioritized with DRQ1 being the     */
	/*  highest and DRQ3 being the lowest. A request line is held active until the corresponding  */
	/*  acknowledge line goes active.                                                             */
	/* ------------------------------------------------------------------------------------------ */
	pc->dma_page[0] = 0;
	pc->dma_page[1] = 0;
	pc->dma_page[2] = 0;
	pc->dma_page[3] = 0;
	/* --------------------------------------------------------------------------------------- */
	/* IBM PC (Model 5150/5160): Quote from "Technical Reference"; I/O Channel Description:    */
	/* --------------------------------------------------------------------------------------- */
	/* Signal Name:                                                                            */
	/*  - -DACK3 (Output): DMA Acknowledge 0 to 3: These lines are used to acknowledge DMA     */
	/*  requests (DRQ1-DRG3) and refresh system dynamic memory (-DACK0). They are active low.  */
	/*  - -DACK1 (Output): See -DACK3                                                          */
	/*  - -DACK0 (Output): See -DACK3                                                          */
	/* --------------------------------------------------------------------------------------- */
	/* ------------------------------------------------------------------------------------------ */
	/* IBM Personal Computer Model 25/30: Quote from "Technical Reference"; I/O Channel; Signal   */
	/* Description:                                                                               */
	/* ------------------------------------------------------------------------------------------ */
	/* The following is a description of the I/O channel signal lines. All lines are TTL-         */
	/* compatible. The (O), (I), or (I/O) notation refers to output, input, or input and output.  */
	/*  - -DACK1 - -DACK3 (O): -DMA Acknowledge 1 to 3: These lines are used by the controller to */
	/*  acknowledge DMA requests. DACK0 is not available on the Type 8525's I/O channel.          */
	/*  - DRQ1 - DRQ3 (I): DMA Request 1 to 3: These lines are asynchronous channel requests used */
	/*  by peripheral evices to gain DMA ser-vices. They are prioritized with DRQ1 being the      */
	/*  highest and DRQ3 being the lowest. A request is generated by bringing a request line to an*/
	/*  active level. A request line is held active until the corresponding acknowledge line goes */
	/*  active.                                                                                   */
	/* ------------------------------------------------------------------------------------------ */
	if (pc->model & PCE_IBMPS2_MODEL25) {
		/* Refresh Memory Pin (B19) on IBM Personal System/2  Model 25/30      */
		pc->dack0 = 0;
		e8237_init(&pc->dma);
		blk = mem_blk_new(addr, 16, 0);
		if (blk == NULL) {
			pce_log(MSG_ERR, "ERROR: Unable to allocate memory for Direct Memory Access (DMA) controller.\n");
			return;
		}
		mem_blk_set_fct(blk, &pc->dma,
			e8237_get_uint8, e8237_get_uint16, e8237_get_uint32,
			e8237_set_uint8, e8237_set_uint16, e8237_set_uint32
		);
		mem_add_blk(pc->prt, blk, 1);
		/* This is a hack. HLDA should be connected to the CPU core. Instead,
		* this will keep it permanently at high. */
		e8237_set_hlda(&pc->dma, 1);
		/* -------------------------------------------------------------------------------------- */
		/* IBM PC (Model 5150/5160): Quote from "Technical Reference"; I/O Channel Description:   */
		/* -------------------------------------------------------------------------------------- */
		/* Signal Name:                                                                           */
		/*  - -DACK3 (Output): DMA Acknowledge 0 to 3: These lines are used to acknowledge DMA    */
		/*  requests (DRQ1-DRG3) and refresh system dynamic memory (-DACK0). They are active low. */
		/*  - +DRQ3 (Input): See +DRQ2                                                            */
		/*  - -DACK1 (Output): See -DACK3                                                         */
		/*  - -DACK0 (Output): See -DACK3                                                         */
		/* -------------------------------------------------------------------------------------- */
		/* -------------------------------------------------------------------------------------- */
		/* IBM Personal Computer Model 25/30: Quote from "Technical Reference"; I/O Channel;      */
		/* Signal Description:                                                                    */
		/* -------------------------------------------------------------------------------------- */
		/* The following is a description of the I/O channel signal lines. All lines are TTL-     */
		/* compatible. The (O), (I), or (I/O) notation refers to output, input, or input and      */
		/* output.                                                                                */
		/*  - -DACK1 - -DACK3 (O): -DMA Acknowledge 1 to 3: These lines are used by the controller*/
		/*  to acknowledge DMA requests. DACK0 is not available on the Type 8525's I/O channel.   */
		/* -------------------------------------------------------------------------------------- */
		e8237_set_dack_fct(&pc->dma, 0, pc, pc_set_dack0);
	}
	else {
		/* DMA Acknowledge 0 Pin    on IBM Personal Computer (Model 5150/5160) */
		pc->dack0 = 0;
		e8237_init(&pc->dma);
		blk = mem_blk_new(addr, 16, 0);
		if (blk == NULL) {
			pce_log(MSG_ERR, "ERROR: Unable to allocate memory for Direct Memory Access (DMA) controller.\n");
			return;
		}
		mem_blk_set_fct(blk, &pc->dma,
			e8237_get_uint8, e8237_get_uint16, e8237_get_uint32,
			e8237_set_uint8, e8237_set_uint16, e8237_set_uint32
		);
		mem_add_blk(pc->prt, blk, 1);
		/* This is a hack. HLDA should be connected to the CPU core. Instead,
		* this will keep it permanently at high. */
		e8237_set_hlda(&pc->dma, 1);
		/* -------------------------------------------------------------------------------------- */
		/* IBM PC (Model 5150/5160): Quote from "Technical Reference"; I/O Channel Description:   */
		/* -------------------------------------------------------------------------------------- */
		/* Signal Name:                                                                           */
		/*  - -DACK3 (Output): DMA Acknowledge 0 to 3: These lines are used to acknowledge DMA    */
		/*  requests (DRQ1-DRG3) and refresh system dynamic memory (-DACK0). They are active low. */
		/*  - +DRQ3 (Input): See +DRQ2                                                            */
		/*  - -DACK1 (Output): See -DACK3                                                         */
		/*  - -DACK0 (Output): See -DACK3                                                         */
		/* -------------------------------------------------------------------------------------- */
		e8237_set_dack_fct(&pc->dma, 0, pc, pc_set_dack0);
	}
	/* ------------------------------------------------------------------------------------------ */
	/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; DMA Controller:       */
	/* ------------------------------------------------------------------------------------------ */
	/* The 8237 Direct Memory Access (DMA) controller and its support logic in the gate array     */
	/* support four channels of 20 address bit DMA. It operates at 4 MHz and handles only 8-bit   */
	/* data transfers. The DMA channel assignments and page register addresses are:               */
	/* ------------------------------------------------------------------------------------------ */
	/* Level:  Assignment:                                                                        */
	/*  - DRQ0: - Not Available                                                                   */
	/*  - DRQ1: - Not Used                                                                        */
	/*  - DRQ2: - Diskette                                                                        */
	/*  - DRQ3: - Fixed Disk                                                                      */
	/* ------------------------------------------------------------------------------------------ */
	/* Figure 1-7. DMA Channel Assignments:                                                       */
	/* ------------------------------------------------------------------------------------------ */
	/* Hex Address: DMA Page Register:                                                            */
	/*  - 081:       - Channel 2:                                                                 */
	/*  - 082:       - Channel 3:                                                                 */
	/*  - 083:       - Channel 1:                                                                 */
	/*  - 087:       - Channel 0:                                                                 */
	/* ------------------------------------------------------------------------------------------ */
	/* Figure 1-8. DMA Page Register Addresses:                                                   */
	/* ------------------------------------------------------------------------------------------ */
	/* Three of the DMA channels (1, 2, and 3) are available on the I/O bus and support high-speed*/
	/* data transfers between I/O devices and memory without microprocessor intervention.         */
	/*                                                                                            */
	/* DMA data transfers take six clock cycles of 250 ns, or 1.5 micro-seconds. I/O CH RDY can be*/
	/* pulled inactive to add wait states to allow more time for slower devices.                  */
	/* ------------------------------------------------------------------------------------------ */
}

static
void pc_setup_pic (ibmpc_t *pc, ini_sct_t *ini)
{
	/* ------------------------------------------------------------------------- */
	/* IBM PC (Model 5150): Quote from "Technical Reference"; System Interrupts: */
	/* ------------------------------------------------------------------------- */
	/* Of the eight prioritized levels of interrupts, six are bussed to the      */
	/* system expansion slots for use by feature cards. Two levels are           */
	/* used on the system board.                                                 */
	/*                                                                           */
	/* Level 0, the highest priority, is attached to Channel 0 of the            */
	/* timer/counter and provides a periodic interrupt for the                   */
	/* time-of-day clock.                                                        */
	/*                                                                           */
	/* Level 1 is attached to the keyboard adapter circuits and receives         */
	/* an interrupt for each scan code sent by the keyboard. The                 */
	/* non-maskable interrupt (NMI) of the 8088 is used to report                */
	/* memory parity errors.                                                     */
	/*                                                                           */
	/* ------------------------------------------------------------------------- */
	/* |  Number: |  Usage:                                                    | */
	/* ------------------------------------------------------------------------- */
	/* | NMI:     | Parity                                                     | */
	/* | NMI:     | 8087                                                       | */
	/* | 0  :     | Timer                                                      | */
	/* | 1  :     | Keyboard                                                   | */
	/* | 2  :     | Reserved                                                   | */
	/* | 3  :     | Asynchronous Communications (Alternate)                    | */
	/* | 3  :     | SDLC Communications                                        | */
	/* | 3  :     | BSC  Communications                                        | */
	/* | 3  :     | Cluster (Primary)                                          | */
	/* | 4  :     | Asynchronous Communications (Primary)                      | */
	/* | 4  :     | SDLC Communications                                        | */
	/* | 4  :     | BSC  Communications                                        | */
	/* | 5  :     | Fixed Disk                                                 | */
	/* | 6  :     | Diskette                                                   | */
	/* | 7  :     | Printer                                                    | */
	/* | 7  :     | Cluster (Alternate)                                        | */
	/* ------------ ------------------------------------------------------------ */
	ini_sct_t     *sct;
	mem_blk_t     *blk;
	unsigned long addr;
	sct = ini_next_sct (ini, NULL, "pic");
	ini_get_uint32 (sct, "address", &addr, 0x0020);
	pce_log_tag (MSG_INF, "PIC:", "addr=0x%08x size=0x%04x\n", addr, 2);
	e8259_init (&pc->pic);
	blk = mem_blk_new (addr, 2, 0);
	if (blk == NULL) {
		return;
	}
	mem_blk_set_fct (blk, &pc->pic,
		e8259_get_uint8, e8259_get_uint16, e8259_get_uint32,
		e8259_set_uint8, e8259_set_uint16, e8259_set_uint32
	);
	mem_add_blk (pc->prt, blk, 1);
	e8259_set_int_fct (&pc->pic, pc->cpu, e86_irq);
	/* --------------------------------------------------------------------------------------- */
	/* IBM PC (Model 5150/5160): Quote from "Technical Reference"; I/O Channel Description:    */
	/* --------------------------------------------------------------------------------------- */
	/* Signal Name:                                                                            */
	/*  - +IRQ2 (Input): Interrupt Request 2 to 7: These lines are used to signal the          */
	/*  microprocessor that an I/O device required attention. They are prioritized with IRQ2   */
	/*  as the highest priority and IRQ7 as the lowest. An Interrupt Request is generated by   */
	/*  raising an IRQ line (low to high) and holding it high until it is acknowledged by the  */
	/*  the microprocessor (interrupt service routine).                                        */
	/* --------------------------------------------------------------------------------------- */
	/* ------------------------------------------------------------------------------------------ */
	/* IBM Personal Computer Model 25/30: Quote from "Technical Reference"; I/O Channel; Signal   */
	/* Description:                                                                               */
	/* ------------------------------------------------------------------------------------------ */
	/* The following is a description of the I/O channel signal lines. All lines are TTL-         */
	/* compatible. The (O), (I), or (I/O) notation refers to output, input, or input and output.  */
	/*  - IRQ2-IRQ7 (I): Interrupt requests 2 through 7: These lines are used to signal the       */
	/*  microprocessor that an I/O device requires atten-tion. They are prioritized with IRQ2 as  */
	/*  the highest priority and IRQ7 as the lowest. When an interrupt is generated, the request  */
	/*  line is held active until it is acknowledged by the microprocessor.                       */
	/* ------------------------------------------------------------------------------------------ */
	e86_set_inta_fct (pc->cpu, &pc->pic, e8259_inta);
	/* ------------------------------------------------------------------------------------------ */
	/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; Interrupts:           */
	/* ------------------------------------------------------------------------------------------ */
	/* The interrupt controller has eight levels of interrupts that are handled according to      */
	/* priority in the I/O support gate array. Two levels are used only on the system board. Level*/
	/* 0, the highest priority, is attached to Channel 0 of the timer/counter and provides a      */
	/* periodic interrupt for the timer tick. Level 1 is shared by the keyboard and the pointing  */
	/* device. It is handled by a BIOS routine pointed to by inter-rupt hex 71. Level 2 is        */
	/* available to the video subsystem, and level 7 is available to the parallel port; however,  */
	/* the BIOS routines do not use interrupts 2 and 7. Level 4 is used by the serial port.       */
	/*                                                                                            */
	/* The controller also has inputs from coprocessor's '-interrupt', the memory controller's    */
	/*'-parity', and '-I/O channel check' signals. These three inputs are used to generate the NMI*/
	/* to the 8086-2.                                                                             */
	/*                                                                                            */
	/* The following table shows the hardware interrupts and their avail-ability to the I/O       */
	/* channel.                                                                                   */
	/* ------------------------------------------------------------------------------------------ */
	/* Level:  System Board:                   I/O Channel:                                       */
	/*  - NMI : - Parity Check and Coprocessor: - I/O Channel Check:                              */
	/*  - IRQ0: - Timer Channel 0:              - Not Available:                                  */
	/*  - IRQ1: - Keyboard Pointing Device:     - Not Available:                                  */
	/*  - IRQ2: - Video:                        -     Available:                                  */
	/*  - IRQ4: - Serial Port:                  -     Available:                                  */
	/*  - IRQ5: - Fixed Disk:                   -     Available:                                  */
	/*  - IRQ6: - Diskette Drive:               -     Available:                                  */
	/*  - IRQ7: - Parallel Port:                -     Available:                                  */
	/* NOTE: Interrupts are available to the I/O channel if they are not enabled by the system    */
	/* board function normally assigned to that interrupt.                                        */
	/* ------------------------------------------------------------------------------------------ */
	/* Figure 1-9. Hardware Interrupt Listing:                                                    */
	/* ------------------------------------------------------------------------------------------ */
}

static
void pc_setup_pit (ibmpc_t *pc, ini_sct_t *ini)
{
	ini_sct_t     *sct;
	mem_blk_t     *blk;
	unsigned long addr;
	/* ---------------------------------------------------------------------------- */
	/* IBM PC (Model 5150): Quote from "Technical Reference"; System Timers:        */
	/* ---------------------------------------------------------------------------- */
	/* Three programmable timer/counters are used by the system as follows: Channel */
	/* 0 is a general-purpose timer providing a constant time base for implementing */
	/* a time-of-day clock, Channel 1 times and requests refresh cycles from the    */
	/* the Direct Memory Access (DMA) channel, and Channel 2 supports the tone      */
	/* generation for the speaker. Each channel has a minimum timing resolution of  */
	/* 1.05-us.                                                                     */
	/* ---------------------------------------------------------------------------- */
	/* IBM PC (Model 5160): Quote from "Technical Reference"; System Timers:        */
	/* ---------------------------------------------------------------------------- */
	/* Three programmable timer/counters are used by the system as follows: Channel */
	/* 0 is used as a general-purpose timer providing a constant time base for      */
	/* implementing a time-of-day clock; Channel 1 is used to time and request      */
	/* refresh cycles from the DMA channel; and Channel 2 is used to support the    */
	/* tone generation for the audio speaker. Each channel has a minimum timing     */
	/* resolution of 1.05-us.                                                       */
	/* ---------------------------------------------------------------------------- */
	/* Initialize Intel 8253-5 System Timer: */
	sct = ini_next_sct (ini, NULL, "pit");
	ini_get_uint32 (sct, "address", &addr, 0x0040);
	pce_log_tag (MSG_INF, "PIT:", "addr=0x%08lx size=0x%04x\n",
		addr, 4
	);
	/* ------------------------------------------------------------------------------------------ */
	/* IBM PC (Model 5150/5160): Quote from "Technical Reference"; Speaker Circuit:               */
	/* ------------------------------------------------------------------------------------------ */
	/* The sound system has a small, permanent magnet, 57.15 mm (2-1/5 in.) speaker.              */
	/* --------------------------- IBM PC Model 5150 Technical Reference: ----------------------- */
	/* The speaker can be driven from one or both of two sources:                                 */
	/*  - An 8255A-5 programmable peripheral interface (PPI) output bit. The address and bit are  */
	/*  defined in the "I/O Address Map".                                                         */
	/*  - A timer clock channel, the output of which is programmable within the functions of the  */
	/*  8253-5 timer when using a 1.19-MHz clock input. The timer gate also is controlled by an   */
	/*  8255A-5 PPI output-port bit. Address and bit assignment are in the "I/O Address Map".     */
	/* --------------------------- IBM PC Model 5160 Technical Reference: ----------------------- */
	/* The speaker's control circuits and driver are on the system board. The speaker connects    */
	/* through a 2-wire interface that attaches to a 3-pin connector on the system board.         */
	/*                                                                                            */
	/* The speaker drive circuit is capable of approximately 1/2 watt of power. The control       */
	/* circuits allow the speaker to be driven three different ways: 1.) a direct program control */
	/* register bit may be toggled to generate a pulse train; 2.) the output from Channel 2 of the*/
	/* timer counter may be programmed to generate a waveform to the speaker; 3.) the clock input */
	/* to the timer counter can be modulated with a program-controlled I/O register bit. All three*/
	/* methods may be performed simultaneously.                                                   */
	/* ------------------------------------------------------------------------------------------ */
	/* Speaker Drive System Block Diagram (1.19 MHz):                                             */
	/* ------------------------------------------------------------------------------------------ */
	/* PPI Bit 1, I/O Address Hex 0061:          -> AND:->Driver:->Low Pass Filter:-> To Speaker: */
	/* -> Clock In 2, Gate 2, Timer Clock Out 2: -> AND:->Driver:->Low Pass Filter:-> To Speaker: */
	/* PPI Bit 0, I/O Address Hex 0061:                                                           */
	/* ------------------------------------------------------------------------------------------ */
	/* Speaker Tone Generation:                                                                   */
	/* ------------------------------------------------------------------------------------------ */
	/* Channel 2 (Tone generation for speaker):                                                   */
	/*  - Gate 2: Controller by 8255A-5 PPI Bit (See I/O Map)                                     */
	/*  - Clock In  2: 1.19318-MHz OSC                                                            */
	/*  - Clock Out 2: Used to drive speaker                                                      */
	/* ------------------------------------------------------------------------------------------ */
	pc->timer1_out = 0;
	e8253_init (&pc->pit);
	blk = mem_blk_new (addr, 4, 0);
	if (blk == NULL) {
		return;
	}
	mem_blk_set_fct (blk, &pc->pit,
		e8253_get_uint8, e8253_get_uint16, e8253_get_uint32,
		e8253_set_uint8, e8253_set_uint16, e8253_set_uint32
	);
	mem_add_blk (pc->prt, blk, 1);
	e8253_set_gate (&pc->pit, 0, 1);
	e8253_set_gate (&pc->pit, 1, 1);
	e8253_set_gate (&pc->pit, 2, 1);
	e8253_set_out_fct (&pc->pit, 0, &pc->pic, e8259_set_irq0);
	e8253_set_out_fct (&pc->pit, 1, pc, pc_set_timer1_out);
	e8253_set_out_fct (&pc->pit, 2, pc, pc_set_timer2_out);
	/* ------------------------------------------------------------------------------------------ */
	/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; System Timer:         */
	/* ------------------------------------------------------------------------------------------ */
	/* The system timer is an 8253 programmable interval timer/counter, or equivalent, that       */
	/* functions as an arrangement of four external I/O ports (hex 0040 through 0043). It receives*/
	/*its 1.19 MHz clock from the I/O support gate array. These ports are treated as counters; the*/
	/* fourth, address hex 0043, is a control register for mode programming.                      */
	/*                                                                                            */
	/* The content of a selected counter may be latched without disturbing the counting operation */
	/*by writing to the control register. If the content of the counter is critical to a program's*/
	/* operation, a second read is recommended for verification.                                  */
	/* -------------------________________------------------------------------------------------- */
	/* System Bus:------>:|System Timer: |                                                        */
	/*          __________| - Gate 0     |                                                        */
	/*          |  _______| - Clock  in 0|                                                        */
	/* +5 Vdc __|__|______| - Gate 1     |                                                        */
	/* -RAS_SIP ---|---___| - Clock  in 1|                                                        */
	/* I/O Port  __|______| - Gate 2     |                                                        */
	/* Hex 0061__| |______| - Clock  in 2|                                                        */
	/* Port Bit 0  |      | - Clock Out 0|_______________IRQ 0________                            */
	/* 1.19 Mhz____|      | - Clock Out 1|    |_Driver_|                                          */
	/*                    | - Clock Out 2|______|__ _|_______                                     */
	/* I/O Port           ----------------  | AND | | Low   |--> To Beeper                        */
	/* Hex 0061 ----------------------------|-----| | Pass  |                                     */
	/* Port Bit 1                                   | Filter|                                     */
	/* ---------------------------------------------|-------|------------------------------------ */
	/* Figure 1-4. System Timer Block Diagram                                                     */
	/* ------------------------------------------------------------------------------------------ */
	/* The three timers are programmable and are used by the system as follows:                   */
	/*                                                                                            */
	/* Channel 0 is a general-purpose timer providing a constant time base.                       */
	/* Channel 0:   System Timer:                                                                 */
	/*  - GATE    0: - Tied High                                                                  */
	/*  - CLK IN  0: - 1.19 MHz OSC                                                               */
	/*  - CLK OUT 0: - IRQ 0                                                                      */
	/* Channel 1 is for internal diagnostic tests.                                                */
	/* Channel 1:   Diagnostic:                                                                   */
	/*  - GATE    1: - Tied High                                                                  */
	/*  - CLK IN  1: - -RAS_SIP from system support gate array                                    */
	/*  - CLK OUT 2: - Not connected                                                              */
	/* Channel 2 supports the tone generation for the audio.                                      */
	/* Channel 2:   Tone Generation:                                                              */
	/*  - GATE    2: Controlled by bit 0 at port hex 61                                           */
	/*  - CLK IN  2: 1.19 MHz OSC                                                                 */
	/*  - CLK OUT 2: To the beeper data of the I/O support gate array                             */
	/* ------------------------------------------------------------------------------------------ */
}

static
void pc_setup_ppi (ibmpc_t *pc, ini_sct_t *ini)
{
	ini_sct_t     *sct;
	mem_blk_t     *blk;
	unsigned long addr;
    /* Initialize Intel 8255A-5 Programmable Peripheral Interface (used for PC Speaker): */
	/* ------------------------------------------------------------------------------------------ */
	/* IBM PC (Model 5150/5160): Quote from "Technical Reference"; Speaker Circuit:               */
	/* ------------------------------------------------------------------------------------------ */
	/* The sound system has a small, permanent magnet, 57.15 mm (2-1/5 in.) speaker.              */
	/* --------------------------- IBM PC Model 5150 Technical Reference: ----------------------- */
	/* The speaker can be driven from one or both of two sources:                                 */
	/*  - An 8255A-5 programmable peripheral interface (PPI) output bit. The address and bit are  */
	/*  defined in the "I/O Address Map".                                                         */
	/*  - A timer clock channel, the output of which is programmable within the functions of the  */
	/*  8253-5 timer when using a 1.19-MHz clock input. The timer gate also is controlled by an   */
	/*  8255A-5 PPI output-port bit. Address and bit assignment are in the "I/O Address Map".     */
	/* --------------------------- IBM PC Model 5160 Technical Reference: ----------------------- */
	/* The speaker's control circuits and driver are on the system board. The speaker connects    */
	/* through a 2-wire interface that attaches to a 3-pin connector on the system board.         */
	/*                                                                                            */
	/* The speaker drive circuit is capable of approximately 1/2 watt of power. The control       */
	/* circuits allow the speaker to be driven three different ways: 1.) a direct program control */
	/* register bit may be toggled to generate a pulse train; 2.) the output from Channel 2 of the*/
	/* timer counter may be programmed to generate a waveform to the speaker; 3.) the clock input */
	/* to the timer counter can be modulated with a program-controlled I/O register bit. All three*/
	/* methods may be performed simultaneously.                                                   */
	/* ------------------------------------------------------------------------------------------ */
	/* Speaker Drive System Block Diagram (1.19 MHz):                                             */
	/* ------------------------------------------------------------------------------------------ */
	/* PPI Bit 1, I/O Address Hex 0061:          -> AND:->Driver:->Low Pass Filter:-> To Speaker: */
	/* -> Clock In 2, Gate 2, Timer Clock Out 2: -> AND:->Driver:->Low Pass Filter:-> To Speaker: */
	/* PPI Bit 0, I/O Address Hex 0061:                                                           */
	/* ------------------------------------------------------------------------------------------ */
	/* Speaker Tone Generation:                                                                   */
	/* ------------------------------------------------------------------------------------------ */
	/* Channel 2 (Tone generation for speaker):                                                   */
	/*  - Gate 2: Controller by 8255A-5 PPI Bit (See I/O Map)                                     */
	/*  - Clock In  2: 1.19318-MHz OSC                                                            */
	/*  - Clock Out 2: Used to drive speaker                                                      */
	/* ------------------------------------------------------------------------------------------ */
	sct = ini_next_sct (ini, NULL, "ppi");
	ini_get_uint32 (sct, "address", &addr, 0x0060);
	pce_log_tag (MSG_INF, "PPI:", "addr=0x%08x size=0x%04x\n", addr, 4);
	e8255_init (&pc->ppi);
	pc->ppi.port[0].read_ext = pc;
	pc->ppi.port[0].read = (void *) pc_ppi_get_port_a;
	pc->ppi.port[1].write_ext = pc;
	pc->ppi.port[1].write = (void *) pc_ppi_set_port_b;
	pc->ppi.port[2].read_ext = pc;
	pc->ppi.port[2].read = (void *) pc_ppi_get_port_c;
    /* ------------------------------------------------------------------------------------------ */
    /* IBM PC (Model 5150/5160): Quote from Technical Reference; 8255A I/O Bit Map:               */
    /* ------------------------------------------------------------------------------------------ */
    /* The 8255A I/O Bit Map shows the inputs and outputs for the Command/Mode register on the    */
	/*system board. Also shown here are the switch settings for the memory, display, and number of*/
	/* diskette drives. The following page contains the I/O bit map.                              */
    /* ------------------------------------------------------------------------------------------ */
    /* Note: A plus (+) indicates a bit value of 1 performs the specified function.               */
    /*       A minus (-) indicates a bit value of 0 performs the specified function.              */
    /*       PA Bit = 0 implies switch ''ON.'' PA bit = 1 implies switch ''OFF.''                 */
    /* ------------------------------------------------------------------------------------------ */
    /* Hex Port Number 0060:                                                                      */
    /*  -  INPUT PA0: +Keyboard Scan Code                       (SW1-1) (Model 5150)/Diagnostic   */
	/*                or IPL 5-1/4 Diskette Drive                            Outputs (Model 5160  */
    /*  -  INPUT PA1: or Reserved                               (SW1-2) (Model 5150)/(Model 5160) */
    /*  -  INPUT PA2: or System Board Read/Write Memory Size   *(SW1-3) (Model 5150)/(Model 5160) */
    /*  -  INPUT PA3: or System Board Read/Write Memory Size   *(SW1-4) (Model 5150)/(Model 5160) */
    /*  -  INPUT PA4: or +Display Type 1                      **(SW1-5) (Model 5150)/(Model 5160) */
    /*  -  INPUT PA5: or +Display Type 2                      **(SW1-6) (Model 5150)/(Model 5160) */
    /*  -  INPUT PA6: or No. of 5-1/4 Drives                 ***(SW1-7) (Model 5150)/(Model 5160) */
    /*  -  INPUT PA7: or No. of 5-1/4 Drives                 ***(SW1-8) (Model 5150)/(Model 5160) */
    /* Hex Port Number 0061:                                                                      */
    /*  - OUTPUT PB0: +Timer 2 Gate Speaker                                                       */
    /*  - OUTPUT PB1: +Speaker Data                                                               */
    /*  - OUTPUT PB2: +(Read Read/Write Memory Size) (Model 5150)/Spare (Model 5160)              */
    /*              or (Read Spare Keys)                                                          */
	/*  - OUTPUT PB3: +Cassette Motor Off            (Model 5150)/Read High Switches Or Read Low  */
	/*                                                                      Switches (Model 5160) */
    /*  - OUTPUT PB4: -Enable Read/Write Memory      (Model 5150)/-Enable RAM Parity Check        */
    /*  - OUTPUT PB5: -Enable I/O Channel Check                                      (Model 5160) */
    /*  - OUTPUT PB6: -Hold Keyboard Clock Low                                                    */
    /*  - OUTPUT PB7: -(Enable Keyboard + (Clear Keyboard and Enable Sense Switches))(Model 5150)/*/
	/*               (Enable Keyboard Or + Clear Keyboard))                          (Model 5160) */
    /* Hex Port Number 0062:                                                                      */
    /*  -  INPUT PC0: I/O Read/Write Memory (Sw2-1) (5150)/Loop on POST (Sw-1) (5160) Or I/O Read/*/
	/*                                              Write (5150)/ Display 0 (**Sw-5) (Model 5160) */
    /*  -  INPUT PC1: I/O Read/Write Memory (Sw2-2) (5150)/+Co-Processor Installed (Sw-2)  (5160) */
	/*     Or -- Binary Value X 32K -- Memory (Sw2-5) (5150)/     Display 1 (**Sw-6) (Model 5160) */
    /*  -  INPUT PC2: I/O Read/Write Memory (Sw2-3) (5150)/+Planar RAM Size 0 (*Sw-3)      (5160) */
	/*                             Or I/O Read/Write (5150)/#5-1/4 Drives   (***Sw-7)(Model 5160) */
    /*  -  INPUT PC3: I/O Read/Write Memory (Sw2-4) (5150)/+Planar RAM Size 1 (*Sw-4)      (5160) */
	/*                            Or Memory (Sw2-5) (5150)/#5-1/4 Drives 1  (***Sw-8)(Model 5160) */
    /*  -  INPUT PC4: +Cassette Data In       (Model 5150)/Spare                     (Model 5160) */
    /*  -  INPUT PC5: +Timer Channel 2 Out                                                        */
    /*  -  INPUT PC6: +I/O Channel Check                                                          */
    /*  -  INPUT PC7: +Read/Write Memory Parity Check (Model 5150)/+RAM Parity Check (Model 5160) */
    /* Hex Port Number 0063:    Hex 99:                                                           */
    /*  - Command/Mode Register: - 7 6 5 4 3 2 1 0                                                */
    /*  - Mode Register Value  : - 1 0 0 1 1 0 0 1                                                */
    /* ------------------------------------------------------------------------------------------ */
    /* *   PA3:    PA2:    Amount of Memory                                                       */
    /*    Sw1-4:  Sw1-3:   Located on System Board (Model 5150)/On System Board (Model 5160)      */
    /*      0       0                               64K (Model 5160)                              */
    /*      0       1                              128K (Model 5160)                              */
    /*      1       0                              192K (Model 5160)                              */
    /*      1       1      64 to 256K (Model 5150)/256K (Model 5160)                              */
    /* ------------------------------------------------------------------------------------------ */
    /* **  PA5:    PA4:    Display at Power-Up Mode                                               */
    /*    Sw1-6:  Sw1-5:                                                                          */
    /*      0       0      Reserved                                                               */
    /*      0       1      Color 40 X 25 (BW Mode)                                                */
    /*      1       0      Color 80 X 25 (BW Mode)                                                */
    /*      1       1      IBM Monochrome (80 X 25)                                               */
    /* ------------------------------------------------------------------------------------------ */
    /* *** PA7:    PA6:    Number of 5-1/4" Drives                                                */
    /*    Sw1-8:  Sw1-7:   in System:                                                             */
    /*      0       0                1                                                            */
    /*      0       1                2                                                            */
    /*      1       0                3                                                            */
    /*      1       1                4                                                            */
    /* ------------------------------------------------------------------------------------------ */
    /* NOTE: A plus (+) indicates a bit value of 1 performs the specified function.               */
    /*       A minus (-) indicates a bit value of 0 performs the specified function.              */
    /*       PA Bit = 0 implies switch ''ON.'' PA Bit = 1 implies switch ''OFF.''                 */
    /* ------------------------------------------------------------------------------------------ */
    /* 8255A I/O Bit Map:                                                                         */
    /* ------------------------------------------------------------------------------------------ */
	blk = mem_blk_new (addr, 4, 0);
	if (blk == NULL) {
		return;
	}
	mem_blk_set_fct (blk, &pc->ppi,
		e8255_get_uint8, e8255_get_uint16, e8255_get_uint32,
		e8255_set_uint8, e8255_set_uint16, e8255_set_uint32
	);
	mem_add_blk (pc->prt, blk, 1);
	e8255_set_uint8 (&pc->ppi, 3, 0x99);
    /* ------------------------------------------------------------------------------------------ */
    /* Speaker Connector:                                                                         */
    /* ------------------------------------------------------------------------------------------ */
    /* Pin/Function:                                                                              */
    /*  - 1: Data                                                                                 */
    /*  - 2: Key                                                                                  */
    /*  - 3: Ground                                                                               */
    /*  - 4: +5 Volts                                                                             */
    /* ------------------------------------------------------------------------------------------ */
    /* The speaker drive circuit is capable of about 1/2 watt of power. The control circuits allow*/
	/* the speaker to be driven three ways: (1) a direct program control register bit may be      */
	/*toggled to generate a pulse train; (2) the output from Channel 2 of the timer/counter device*/
	/* may be programmed to generate a waveform to the speaker; (3) the clock input to the timer/ */
	/* counter device can be modulated with a program-controlled I/O register bit. All three      */
	/* methods may be performed simultaneously.                                                   */
    /* ------------------------------------------------------------------------------------------ */
}

static
void pc_setup_kbd (ibmpc_t *pc, ini_sct_t *ini)
{
	pc_kbd_init (&pc->kbd);
	pc_kbd_set_irq_fct (&pc->kbd, &pc->pic, e8259_set_irq1);
	/* --------------------------------------------------------------------------------------- */
	/* IBM PC (Model 5150/5160): Quote from "Technical Reference"; I/O Channel Description:    */
	/* --------------------------------------------------------------------------------------- */
	/* Signal Name:                                                                            */
	/*  - +IRQ2 (Input): Interrupt Request 2 to 7: These lines are used to signal the          */
	/*  microprocessor that an I/O device required attention. They are prioritized with IRQ2   */
	/*  as the highest priority and IRQ7 as the lowest. An Interrupt Request is generated by   */
	/*  raising an IRQ line (low to high) and holding it high until it is acknowledged by the  */
	/*  the microprocessor (interrupt service routine).                                        */
	/* --------------------------------------------------------------------------------------- */
	/* ------------------------------------------------------------------------------------------ */
	/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; Interrupts:           */
	/* ------------------------------------------------------------------------------------------ */
	/* The interrupt controller has eight levels of interrupts that are handled according to      */
	/* priority in the I/O support gate array. Two levels are used only on the system board. Level*/
	/* 0, the highest priority, is attached to Channel 0 of the timer/counter and provides a      */
	/* periodic interrupt for the timer tick. Level 1 is shared by the keyboard and the pointing  */
	/* device. It is handled by a BIOS routine pointed to by inter-rupt hex 71. Level 2 is        */
	/* available to the video subsystem, and level 7 is available to the parallel port; however,  */
	/* the BIOS routines do not use interrupts 2 and 7. Level 4 is used by the serial port.       */
	/*                                                                                            */
	/* The controller also has inputs from coprocessor's '-interrupt', the memory controller's    */
	/*'-parity', and '-I/O channel check' signals. These three inputs are used to generate the NMI*/
	/* to the 8086-2.                                                                             */
	/*                                                                                            */
	/* The following table shows the hardware interrupts and their avail-ability to the I/O       */
	/* channel.                                                                                   */
	/* ------------------------------------------------------------------------------------------ */
	/* Level:  System Board:                   I/O Channel:                                       */
	/*  - NMI : - Parity Check and Coprocessor: - I/O Channel Check:                              */
	/*  - IRQ0: - Timer Channel 0:              - Not Available:                                  */
	/*  - IRQ1: - Keyboard Pointing Device:     - Not Available:                                  */
	/*  - IRQ2: - Video:                        -     Available:                                  */
	/*  - IRQ4: - Serial Port:                  -     Available:                                  */
	/*  - IRQ5: - Fixed Disk:                   -     Available:                                  */
	/*  - IRQ6: - Diskette Drive:               -     Available:                                  */
	/*  - IRQ7: - Parallel Port:                -     Available:                                  */
	/* NOTE: Interrupts are available to the I/O channel if they are not enabled by the system    */
	/* board function normally assigned to that interrupt.                                        */
	/* ------------------------------------------------------------------------------------------ */
	/* Figure 1-9. Hardware Interrupt Listing:                                                    */
	/* ------------------------------------------------------------------------------------------ */
}

static
void pc_setup_cassette (ibmpc_t *pc, ini_sct_t *ini)
{
	const char    *fname;
	const char    *mode;
	unsigned long pos;
	int           enable, append, pcm, filter;
	ini_sct_t     *sct;
	/* ------------------------------------------------------------------- */
	/* IBM PC (Model 5150): Quote from "Technical Reference":              */
	/* ------------------------------------------------------------------- */
	/* The system board contains circuits for attaching an audio           */
	/* cassette, the keyboard, and the speaker. The cassette adapter       */
	/* allows the attachment of any good quality audio cassette through    */
	/* the earphone output and either the microphone or auxiliary          */
	/* inputs. The system board has a jumper for either input. This        */
	/* interface also provides a cassette motor control for transport      */
	/* starting and stopping under program control. The interface reads    */
	/* and writes the audio cassette at a data rate of between 1,000 and   */
	/* 2,000 baud. The baud rate is variable and depend on data            */
	/* content, because a different bit-cell time is used for 0's and 1's. */
	/* For diagnostic purposes, the tape interface can loop read to write  */
	/* for testing the system board's circuits. The ROM cassette           */
	/* software blocks cassette data and generates a cyclic redundancy     */
	/* check (CRC) to check this data.                                     */
	/* ------------------------------------------------------------------- */
	/* Initialize Cassette Tape Interface (IBM PC Model 5150): */
	pc->cas = NULL;
	if ((pc->model & PCE_IBMPC_5150) == 0) {
		return;
	}
	sct = ini_next_sct (ini, NULL, "cassette");
	if (sct == NULL) {
		return;
	}
	ini_get_bool (sct, "enable", &enable, 1);
	if (enable == 0) {
		return;
	}
	ini_get_string (sct, "file", &fname, NULL);
	ini_get_string (sct, "mode", &mode, "load");
	ini_get_uint32 (sct, "position", &pos, 0);
	ini_get_bool (sct, "append", &append, 0);
	ini_get_bool (sct, "filter", &filter, 1);
	if (ini_get_bool (sct, "pcm", &pcm, 0)) {
		pcm = -1;
	}
	pce_log_tag (MSG_INF, "CASSETTE:",
		"file=%s mode=%s pcm=%d filter=%d pos=%lu append=%d\n",
		(fname != NULL) ? fname : "<none>",
		mode, pcm, filter, pos, append
	);
	pc->cas = pc_cas_new();
	if (pc->cas == NULL) {
		pce_log (MSG_ERR, "ERROR: Allocation for cassette tape failed.\n");
		return;
	}
	if (pc_cas_set_fname (pc->cas, fname)) {
		pce_log (MSG_ERR, "ERROR: Unable to open following cassette tape file: (%s)\n", fname);
	}
	if (strcmp (mode, "load") == 0) {
		pc_cas_set_mode (pc->cas, 0);
	}
	else if (strcmp (mode, "save") == 0) {
		pc_cas_set_mode (pc->cas, 1);
	}
	else {
		pce_log (MSG_ERR, "ERROR: Invalid option specified for cassette tape drive: (%s)\n", mode);
	}
	if (append) {
		pc_cas_append (pc->cas);
	}
	else {
		pc_cas_set_position (pc->cas, pos);
	}
	if (pcm >= 0) {
		pc_cas_set_pcm (pc->cas, pcm);
	}
	pc_cas_set_filter (pc->cas, filter);
    /* --------------------------------------------------------------------------------------- */
    /* IBM PC (Model 5150): Quote from "Technical Reference"; Cassette Interface:              */
    /* --------------------------------------------------------------------------------------- */
    /* The cassette interface is controlled through software. An output from the 8253 timer    */
    /* controls the data to the cassette recorder through pin 5 of the cassette DIN connector  */
    /* at the rear of the system board. The cassette input data is read by an input port bit   */
    /* of the 8255A-5 PPI. This data is received through pin 4 of the cassette connector.      */
    /* Software algorithms are used to generate and read cassette data. The cassette drive     */
    /* motor is controlled through pins 1 and 3 of the cassette connector. The drive motor     */
    /* on/off switching is controlled by an 8255A-5 PPI output-port bit (hex 61, bit 3). The   */
    /* 8255A-5 address and bit assignments are defined in "I/O Address Map" earlier in this    */
    /* section.                                                                                */
    /*                                                                                         */
    /* A 2 by 2 Berg pin and a jumper are used on the cassette 'data out' line. The jumper     */
    /* allows use of the 'data out' line as a 0.075-Vdc microphone input when placed across    */
    /* the M and C of the Berg Pins. A 0.68-Vdc auxiliary input to the cassette recorder is    */
    /* available when the jumper is placed across the A and C of the Berg Pins. The "System    */
    /* Board Component Diagram" shows the location of the cassette Berg pins.                  */
    /* --------------------------------------------------------------------------------------- */
    /*               Microphone Input (0.075 Vdc):  Auxiliary Input (0.68 Vdc):                */
    /*               ----------------------------- -----------------------------               */
    /*                      M            A                 M            A                      */
    /*               ----------------------------- -----------------------------               */
    /*               |      O            X       | |       X            O      |               */
    /*               |      O            X       | |       X            O      |               */
    /*               ----------------------------- -----------------------------               */
    /*                      C            C                 C            C                      */
    /*               ----------------------------- -----------------------------               */
    /*               Microphone Input  (0.075 Vdc)  Auxiliary Input  (0.68 Vdc)                */
    /* --------------------------------------------------------------------------------------- */
    /* Cassette Circuit Block Diagrams:                                                        */
    /* --------------------------------------------------------------------------------------- */
    /* Circuit block diagrams for the cassette-interface read hardware, write hardware, and    */
    /* motor control are illustrated below.                                                    */
    /* --------------------------------------------------------------------------------------- */
    /* Cassette-Interface Read Hardware Block Diagram:                                         */
    /* --------------------------------------------------------------------------------------- */
    /*             GND: ----> 18k Ohm Resistor: -|-> 1M Ohm Resistor: -|                       */
    /*                                           |                     |                       */
    /* |-->0.047 uF Capacitor:-18k Ohm Resistor:-|-> MC1741: ----------|->18k Ohm Resistor: -|-*//*Cassette Data In:*/
    /* |                                         |      |      OP AMP                        | */
    /* |                                         |     -5V                                   | */
    /* |                                         V                                           | */
    /* V Data From Cassette Recorder Jack:   18k Ohm Resistor:GND:->Silicon Diode VIR=.4V:<--| */
    /*                                           |                          Cathode:           */
    /*                                      GND: V                                             */
    /* --------------------------------------------------------------------------------------- */
    /* Cassette Interface Write Hardware Block Diagram:                                        */
    /* --------------------------------------------------------------------------------------- */
    /*                                               V +5V:                                    */
    /* 8253 Timer #2 Output: --|--> 74LS38 (OR): ----|> 3.9k Ohm R:                            */
    /*                         |---------------------|                                         */
    /*                                               |> 4.7k Ohm R:                            */
    /*                         0:--------------------|                                         */
    /*                         0.678V to AUX Input   |> 1.2k Ohm R:                            */
    /*                         0:--------------------|                                         */
    /*                         0.075V to MIC Input   |> 150  Ohm R:                            */
    /*                                               |                                         */
    /*                                               |> GND:                                   */
    /* --------------------------------------------------------------------------------------- */
    /* Cassette Interface Write Hardware Block Diagram:                                        */
    /* --------------------------------------------------------------------------------------- */
    /*                                                         +5V                             */
    /*                                                          |                              */
    /* 8253 Timer #2 Output: --|--> 74LS38 (OR):           3.9k Ohm R:                         */
    /*                         |--> 74LS38 (OR):----------------|                              */
    /*                                                          V                              */
    /*                                                     4.7k Ohm R:                         */
    /*                                     0 :------------------|                              */
    /*                                       0.678V to AUX:     V                              */
    /*                                                     1.2k Ohm R:                         */
    /*                                     0 :------------------|                              */
    /*                                     0.075V to MIC Input: V                              */
    /*                                                     150  Ohm R:                         */
    /*                                                          |--> GND:                      */
    /* --------------------------------------------------------------------------------------- */
    /* Cassette Motor Control Block Diagram:                                                   */
    /* --------------------------------------------------------------------------------------- */
    /*                                +5V                                                      */
    /*                                 ^                                                       */
    /*                                 |                                                       */
    /*                                 V                                                       */
    /*                             4.7 kOhm:   SN75475:   +5V                                  */
    /*                                 ^    +5V:-->Clamp:  ^       Relay:                      */
    /*                                 |                   |              N/O:---------------- */
    /*                                 |             S:----|---->Coil:                         */
    /*                                 |             C:----|           Cassette Motor Control: */
    /*                                 |           VCC:----|                                   */
    /*             |--> 74LS38 (OR): --|------>In: Out:--------->Coil:                         */
    /* Motor On: --|                                                      COM:---------------- */
    /* --------------------------------------------------------------------------------------- */
    /* Cassette Interface Connector Specifications (5-Pin DIN Connector):                      */
    /* --------------------------------------------------------------------------------------- */
    /* Pin:  Signal                            : Electrical Characteristics            :       */
    /*  - 1: Motor Control                     : Common from Relay                             */
    /*  - 2: Ground                            :                                               */
    /*  - 3: Motor Control                     : Relay N.O (6 Vdc at 1A)                       */
    /*  - 4: Data In                           : 500nA at += 13V - at 1,000 - 2,000 Baud       */
    /*  - 5: Data Out (Microphone or Auxiliary): 250 uA at 0.68 VDC or ** 0.075 Vdc            */
    /* --------------------------------------------------------------------------------------- */
    /* *: All voltages and currents are maximum ratings and should not be exceeded.            */
    /* **: Data out can be chosen using a jumper located on the system board. (Auxiliary ->0.68*/
    /* Vdc or Microphone ->0.075 Vdc).                                                         */
    /*                                                                                         */
    /* Interchange of these voltages on the cassette recorder could lead to damage of recorder */
    /* inputs.                                                                                 */
    /* --------------------------------------------------------------------------------------- */
}

static
void pc_setup_speaker (ibmpc_t *pc, ini_sct_t *ini)
{
	const char    *driver;
	unsigned      volume;
	unsigned long srate;
	unsigned long lowpass;
	ini_sct_t     *sct;
	/* Initialize PC Speaker: */
	pc_speaker_init (&pc->spk);
	pc_speaker_set_clk_fct (&pc->spk, pc, pc_get_clock2);
	sct = ini_next_sct (ini, NULL, "speaker");
	if (sct == NULL) {
		return;
	}
	ini_get_string (sct, "driver", &driver, NULL);
	ini_get_uint16 (sct, "volume", &volume, 500);
	ini_get_uint32 (sct, "sample_rate", &srate, 44100);
	ini_get_uint32 (sct, "lowpass", &lowpass, 0);
	pce_log_tag (MSG_INF, "SPEAKER:", "volume=%u srate=%lu lowpass=%lu driver=%s\n",
		volume, srate, lowpass,
		(driver != NULL) ? driver : "<none>"
	);
	if (driver != NULL) {
		if (pc_speaker_set_driver (&pc->spk, driver, srate)) {
			pce_log (MSG_ERR,
				"ERROR: Unable to connect PC speaker to following: (%s)\n",
				driver
			);
		}
	}
	pc_speaker_set_lowpass (&pc->spk, lowpass);
	pc_speaker_set_volume (&pc->spk, volume);
}

static
void pc_setup_terminal (ibmpc_t *pc, ini_sct_t *ini)
{
	/* Open terminal window: */
	pc->trm = ini_get_terminal (ini, par_terminal);
	if (pc->trm == NULL) {
		return;
	}
	trm_set_key_fct (pc->trm, &pc->kbd, pc_kbd_set_key);
	trm_set_mouse_fct (pc->trm, pc, pc_set_mouse);
	trm_set_msg_fct (pc->trm, pc, pc_set_msg);
}

static
int pc_setup_mda (ibmpc_t *pc, ini_sct_t *sct)
{
	/* Initialize Monochrome Display Adapter (MDA): */
	pc->video = mda_new_ini (sct);
	if (pc->video == NULL) {
		return (1);
	}
	mem_add_blk (pc->mem, pce_video_get_mem (pc->video), 0);
	mem_add_blk (pc->prt, pce_video_get_reg (pc->video), 0);
	pc_set_video_mode (pc, 3);
	return (0);
}

static
int pc_setup_olivetti (ibmpc_t *pc, ini_sct_t *sct)
{
	/* Initialize Olivetti M24/AT&T 6300 Display Adapter: */
	pc->video = m24_new_ini (sct);
	if (pc->video == NULL) {
		return (1);
	}
	mem_add_blk (pc->mem, pce_video_get_mem (pc->video), 0);
	mem_add_blk (pc->prt, pce_video_get_reg (pc->video), 0);
	pc_set_video_mode (pc, 2);
	return (0);
}

static
int pc_setup_plantronics (ibmpc_t *pc, ini_sct_t *sct)
{
	/* Initialize Plantronics ColorPlus Display Adapter: */
	pc->video = pla_new_ini (sct);
	if (pc->video == NULL) {
		return (1);
	}
	mem_add_blk (pc->mem, pce_video_get_mem (pc->video), 0);
	mem_add_blk (pc->prt, pce_video_get_reg (pc->video), 0);
	pc_set_video_mode (pc, 2);
	return (0);
}

static
int pc_setup_wy700 (ibmpc_t *pc, ini_sct_t *sct)
{
	/* Initialize WYSE WY700 Display Adapter: */
	pc->video = wy700_new_ini (sct);
	if (pc->video == NULL) {
		return (1);
	}
	mem_add_blk (pc->mem, pce_video_get_mem (pc->video), 0);
	mem_add_blk (pc->prt, pce_video_get_reg (pc->video), 0);
	pc_set_video_mode (pc, 2);
	return (0);
}

static
int pc_setup_hgc (ibmpc_t *pc, ini_sct_t *sct)
{
	/* Initialize Hercules Graphics Card (HGC): */
	pc->video = hgc_new_ini (sct);
	if (pc->video == NULL) {
		return (1);
	}
	mem_add_blk (pc->mem, pce_video_get_mem (pc->video), 0);
	mem_add_blk (pc->prt, pce_video_get_reg (pc->video), 0);
	pc_set_video_mode (pc, 3);
	return (0);
}

/* static
int pc_setup_compaq (ibmpc_t *pc, ini_sct_t *sct)
{
	/* Initialize COMPAQ display adapter (MDA/CGA): */
	/* pc->video = compaq_new_ini (sct);
	if (pc->video == NULL) {
		return (1);
	}
	mem_add_blk (pc->mem, pce_video_get_mem (pc->video), 0);
	mem_add_blk (pc->prt, pce_video_get_reg (pc->video), 0);
	pc_set_video_mode (pc, 2);
	return (0);
}*/

static
int pc_setup_cga (ibmpc_t *pc, ini_sct_t *sct)
{
	/* Initialize IBM Color Graphics Adapter (CGA): */
	pc->video = cga_new_ini (sct);
	if (pc->video == NULL) {
		return (1);
	}
	mem_add_blk (pc->mem, pce_video_get_mem (pc->video), 0);
	mem_add_blk (pc->prt, pce_video_get_reg (pc->video), 0);
	pc_set_video_mode (pc, 2);
	return (0);
}

static
int pc_setup_ega (ibmpc_t *pc, ini_sct_t *sct)
{
	int      enable_irq;
	unsigned irq;
	/* Initialize IBM Enhanced Graphics Adapter (EGA): */
	pc->video = ega_new_ini (sct);
	if (pc->video == NULL) {
		return (1);
	}
	ini_get_bool (sct, "enable_irq", &enable_irq, 0);
	ini_get_uint16 (sct, "irq", &irq, 2);
	pce_log_tag (MSG_INF, "VIDEO:", "EGA irq=%u (%s)\n",
		irq, enable_irq ? "enabled" : "disabled"
	);
	if (enable_irq) {
		ega_set_irq_fct (pc->video->ext,
			&pc->pic, e8259_get_irq_fct (&pc->pic, irq)
		);
	}
	mem_add_blk (pc->mem, pce_video_get_mem (pc->video), 0);
	mem_add_blk (pc->prt, pce_video_get_reg (pc->video), 0);
	pc_set_video_mode (pc, 0);
	return (0);
}

static
int pc_setup_vga (ibmpc_t *pc, ini_sct_t *sct)
{
	int      enable_irq;
	unsigned irq;
	/* Initialize IBM Video Graphics Array (VGA): */
	pc->video = vga_new_ini (sct);
	if (pc->video == NULL) {
		return (1);
	}
	ini_get_bool (sct, "enable_irq", &enable_irq, 0);
	ini_get_uint16 (sct, "irq", &irq, 2);
	pce_log_tag (MSG_INF, "VIDEO:", "VGA irq=%u (%s)\n",
		irq, enable_irq ? "enabled" : "disabled"
	);
	if (enable_irq) {
		vga_set_irq_fct (pc->video->ext,
			&pc->pic, e8259_get_irq_fct (&pc->pic, irq)
		);
	}
	mem_add_blk (pc->mem, pce_video_get_mem (pc->video), 0);
	mem_add_blk (pc->prt, pce_video_get_reg (pc->video), 0);
	pc_set_video_mode (pc, 0);
	return (0);
}

static
void pc_setup_video (ibmpc_t *pc, ini_sct_t *ini)
{
	const char *dev;
	ini_sct_t  *sct;
	/* Initialize Video Display: */
	pc->video = NULL;
	sct = ini_next_sct (ini, NULL, "video");
	ini_get_string (sct, "device", &dev, "cga");
	if (par_video != NULL) {
		while ((sct != NULL) && (strcmp (par_video, dev) != 0)) {
			sct = ini_next_sct (ini, sct, "video");
			ini_get_string (sct, "device", &dev, "cga");
		}
		if (sct == NULL) {
			dev = par_video;
		}
	}
	pce_log_tag (MSG_INF, "VIDEO:", "device=%s\n", dev);
	if (strcmp (dev, "vga") == 0) {
		pc_setup_vga (pc, sct);
	}
	else if (strcmp (dev, "ega") == 0) {
		pc_setup_ega (pc, sct);
	}
	else if (strcmp (dev, "cga") == 0) {
		pc_setup_cga (pc, sct);
	}
	else if (strcmp (dev, "hgc") == 0) {
		pc_setup_hgc (pc, sct);
	}
	else if (strcmp (dev, "mda") == 0) {
		pc_setup_mda (pc, sct);
	}
	else if (strcmp (dev, "olivetti") == 0) {
		pc_setup_olivetti (pc, sct);
	}
	else if (strcmp (dev, "plantronics") == 0) {
		pc_setup_plantronics (pc, sct);
	}
	else if ((strcmp (dev, "wyse") == 0) || (strcmp (dev, "wy700") == 0)) {
		pc_setup_wy700 (pc, sct);
	}
	else if ((strcmp (dev, "compaq") == 0) || (strcmp (dev, "portable") == 0) || (strcmp (dev, "deskpro") == 0)) {
        /* pc_setup_compaq (pc, sct); /*WARNING: Enable only when development of C programming calls is finished!!!!*/
		pc_setup_cga (pc, sct);
	}
	else {
		pce_log (MSG_ERR, "ERROR: Incorrect video display adapter specified: (%s)\n", dev);
	}
	if (pc->video == NULL) {
		pce_log (MSG_ERR, "ERROR: Unable to initialize display adapter.\n");
	}
	if (pc->video != NULL) {
		ini_get_ram (pc->mem, sct, &pc->ram);
		ini_get_rom (pc->mem, sct);
		pce_load_mem_ini (pc->mem, sct);
		if (pc->trm != NULL) {
			pce_video_set_terminal (pc->video, pc->trm);
		}
	}
}

static
void pc_setup_disks (ibmpc_t *pc, ini_sct_t *ini)
{
	/* Initialize Disk Drives: */
	ini_sct_t *sct;
	disk_t    *dsk;
	pc->dsk0 = NULL;
	pc->dsk = dsks_new();
	sct = NULL;
	while ((sct = ini_next_sct (ini, sct, "disk")) != NULL) {
		if (ini_get_disk (sct, &dsk)) {
			pce_log (MSG_ERR, "ERROR: Unable to initialize disk drive.\n");
			continue;
		}
		if (dsk == NULL) {
			continue;
		}
		dsks_add_disk (pc->dsk, dsk);
		if (dsk_get_drive (dsk) >= 0x80) {
			pc->hd_cnt += 1;
		}
	}
}

static
void pc_setup_fdc (ibmpc_t *pc, ini_sct_t *ini)
{
	/* Initialize Floppy Disk Controller: */
	ini_sct_t     *sct;
	int           accurate, ignore_eot;
	unsigned long addr;
	unsigned      irq;
	unsigned      drv[4];
	if (pc->model & PCE_IBMPS2_MODEL25)
	{
		/* IBM Personal System/2 Model 25/30 */
		/* -------------------------------------------------------------------------------------- */
		/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; Interrupts:       */
		/* -------------------------------------------------------------------------------------- */
		/* The interrupt controller has eight levels of interrupts that are handled according to  */
		/* priority in the I/O support gate array. Two levels are used only on the system board.  */
		/* Level 0, the highest priority, is attached to Channel 0 of the timer/counter and       */
		/* provides a periodic interrupt for the timer tick. Level 1 is shared by the keyboard and*/
		/* the pointing device. It is handled by a BIOS routine pointed to by inter-rupt hex 71.  */
		/* Level 2 is available to the video subsystem, and level 7 is available to the parallel  */
		/* port; however, the BIOS routines do not use interrupts 2 and 7. Level 4 is used by the */
		/* serial port.                                                                           */
		/*                                                                                        */
		/* The controller also has inputs from coprocessor's '-interrupt', the memory controller's*/
		/*'-parity', and '-I/O channel check' signals. These three inputs are used to generate the*/
		/* NMI to the 8086-2.                                                                     */
		/*                                                                                        */
		/* The following table shows the hardware interrupts and their avail-ability to the I/O   */
		/* channel.                                                                               */
		/* -------------------------------------------------------------------------------------- */
		/* Level:  System Board:                   I/O Channel:                                   */
		/*  - NMI : - Parity Check and Coprocessor: - I/O Channel Check:                          */
		/*  - IRQ0: - Timer Channel 0:              - Not Available:                              */
		/*  - IRQ1: - Keyboard Pointing Device:     - Not Available:                              */
		/*  - IRQ2: - Video:                        -     Available:                              */
		/*  - IRQ4: - Serial Port:                  -     Available:                              */
		/*  - IRQ5: - Fixed Disk:                   -     Available:                              */
		/*  - IRQ6: - Diskette Drive:               -     Available:                              */
		/*  - IRQ7: - Parallel Port:                -     Available:                              */
		/* NOTE: Interrupts are available to the I/O channel if they are not enabled by the system*/
		/* board function normally assigned to that interrupt.                                    */
		/* -------------------------------------------------------------------------------------- */
		/* Figure 1-9. Hardware Interrupt Listing:                                                */
		/* -------------------------------------------------------------------------------------- */
		pc->fdc = NULL;
		sct = ini_next_sct(ini, NULL, "fdc");
		if (sct == NULL) {
			return;
		}
		ini_get_uint32(sct, "address", &addr, 0x3f0);
		ini_get_uint16(sct, "irq", &irq, 6);
		ini_get_bool(sct, "accurate", &accurate, 0);
		ini_get_bool(sct, "ignore_eot", &ignore_eot, 0);
		ini_get_uint16(sct, "drive0", &drv[0], 0xffff);
		ini_get_uint16(sct, "drive1", &drv[1], 0xffff);
		ini_get_uint16(sct, "drive2", &drv[2], 0xffff);
		ini_get_uint16(sct, "drive3", &drv[3], 0xffff);
		pce_log_tag(MSG_INF, "FDC:",
			"addr=0x%08lx irq=%u accurate=%d eot=%d drv=[%u %u %u %u]\n",
			addr, irq, accurate, !ignore_eot,
			drv[0], drv[1], drv[2], drv[3]
		);
		pc->fdc = dev_fdc_new(addr);
		if (pc->fdc == NULL) {
			pce_log(MSG_ERR, "ERROR: Unable to initialize floppy disk controller.\n");
			return;
		}
		/* dev_fdc_ibmps2; */
		dev_fdc_mem_add_io(pc->fdc, pc->prt);
		dev_fdc_set_disks(pc->fdc, pc->dsk);
		dev_fdc_set_drive(pc->fdc, 0, drv[0]);
		dev_fdc_set_drive(pc->fdc, 1, drv[1]);
		dev_fdc_set_drive(pc->fdc, 2, drv[2]);
		dev_fdc_set_drive(pc->fdc, 3, drv[3]);
		e8272_set_input_clock(&pc->fdc->e8272, PCE_IBMPS2_MODEL25_CLK2);
		e8272_set_accuracy(&pc->fdc->e8272, accurate != 0);
		e8272_set_ignore_eot(&pc->fdc->e8272, ignore_eot != 0);
		e8272_set_irq_fct(&pc->fdc->e8272, &pc->pic, e8259_get_irq_fct(&pc->pic, irq));
		dev_fdc_ibmps2_irq6; /* IRQ6 */
		e8272_set_dreq_fct(&pc->fdc->e8272, &pc->dma, e8237_set_dreq2);
		dev_fdc_ibmps2_drq2; /* DRQ2 */
		/* -------------------------------------------------------------------------------------- */
		/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; I/O Channel;      */
		/* Signal Description:                                                                    */
		/* -------------------------------------------------------------------------------------- */
		/* The following is a description of the I/O channel signal lines. All lines are TTL-     */
		/* compatible. The (O), (I), or (I/O) notation refers to output, input, or input and      */
		/* output.                                                                                */
		/*  - TC (O): Terminal Count: This line provides a pulse when the ter-minal count for any */
		/*  DMA channel is reached.                                                               */
		/* -------------------------------------------------------------------------------------- */
		e8237_set_tc_fct(&pc->dma, 2, &pc->fdc->e8272, e8272_set_tc);
		/* -------------------------------------------------------------------------------------- */
		/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; I/O Channel;      */
		/* Signal Description:                                                                    */
		/* -------------------------------------------------------------------------------------- */
		/* The following is a description of the I/O channel signal lines. All lines are TTL-     */
		/* compatible. The (O), (I), or (I/O) notation refers to output, input, or input and      */
		/* output.                                                                                */
		/*  - -IOR (O): -I/O Read: This command line instructs an I/O device to drive its data    */
		/*  onto the data bus. This signal is driven by the micro-processor or the DMA controller.*/
		/* -------------------------------------------------------------------------------------- */
		pc->dma.chn[2].iord_ext = &pc->fdc->e8272;
		pc->dma.chn[2].iord = (void *)e8272_read_data;
		/* -------------------------------------------------------------------------------------- */
		/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; I/O Channel;      */
		/* Signal Description:                                                                    */
		/* -------------------------------------------------------------------------------------- */
		/* The following is a description of the I/O channel signal lines. All lines are TTL-     */
		/* compatible. The (O), (I), or (I/O) notation refers to output, input, or input and      */
		/* output.                                                                                */
		/*  - -IOW (O): -I/O Write: This command line instructs an I/O device to read the data on */
		/*  the data bus. This signal is driven by the micro-processor or the DMA controller.     */
		/* -------------------------------------------------------------------------------------- */
		pc->dma.chn[2].iowr_ext = &pc->fdc->e8272;
		pc->dma.chn[2].iowr = (void *)e8272_write_data;
		/* -------------------------------------------------------------------------------------- */
		/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; I/O Channel;      */
		/* Signal Description:                                                                    */
		/* -------------------------------------------------------------------------------------- */
		/* The following is a description of the I/O channel signal lines. All lines are TTL-     */
		/* compatible. The (O), (I), or (I/O) notation refers to output, input, or input and      */
		/* output.                                                                                */
		/*  - -MEMR (O): -Memory Read: This command line instructs memory to drive its data       */
		/*  present on the data bus. This signal is driven by the micro-processor or the DMA      */
		/*  controller.                                                                           */
		/* -------------------------------------------------------------------------------------- */
		pc->dma.chn[2].memrd_ext = pc;
		pc->dma.chn[2].memrd = (void *)pc_dma2_get_mem8;
		/* -------------------------------------------------------------------------------------- */
		/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; I/O Channel;      */
		/* Signal Description:                                                                    */
		/* -------------------------------------------------------------------------------------- */
		/* The following is a description of the I/O channel signal lines. All lines are TTL-     */
		/* compatible. The (O), (I), or (I/O) notation refers to output, input, or input and      */
		/* output.                                                                                */
		/*  - -MEMW (O): -Memory Write: This command line instructs memory to store the data      */
		/*  present on the data bus. This signal is driven by the micro-processor or the DMA      */
		/* controller.                                                                            */
		/* -------------------------------------------------------------------------------------- */
		pc->dma.chn[2].memwr_ext = pc;
		pc->dma.chn[2].memwr = (void *)pc_dma2_set_mem8;
		/* -------------------------------------------------------------------------------------- */
		/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; DMA Controller:   */
		/* -------------------------------------------------------------------------------------- */
		/* The 8237 Direct Memory Access (DMA) controller and its support logic in the gate array */
		/*support four channels of 20 address bit DMA. It operates at 4 MHz and handles only 8-bit*/
		/* data transfers. The DMA channel assignments and page register addresses are:           */
		/* -------------------------------------------------------------------------------------- */
		/* Level:  Assignment:                                                                    */
		/*  - DRQ0: - Not Available                                                               */
		/*  - DRQ1: - Not Used                                                                    */
		/*  - DRQ2: - Diskette                                                                    */
		/*  - DRQ3: - Fixed Disk                                                                  */
		/* -------------------------------------------------------------------------------------- */
		/* Figure 1-7. DMA Channel Assignments:                                                   */
		/* -------------------------------------------------------------------------------------- */
		/* Hex Address: DMA Page Register:                                                        */
		/*  - 081:       - Channel 2:                                                             */
		/*  - 082:       - Channel 3:                                                             */
		/*  - 083:       - Channel 1:                                                             */
		/*  - 087:       - Channel 0:                                                             */
		/* -------------------------------------------------------------------------------------- */
		/* Figure 1-8. DMA Page Register Addresses:                                               */
		/* -------------------------------------------------------------------------------------- */
		/* Three of the DMA channels (1, 2, and 3) are available on the I/O bus and support high- */
		/*speed data transfers between I/O devices and memory without microprocessor intervention.*/
		/*                                                                                        */
		/*DMA data transfers take six clock cycles of 250 ns, or 1.5 micro-seconds. I/O CH RDY can*/
		/* be pulled inactive to add wait states to allow more time for slower devices.           */
		/* -------------------------------------------------------------------------------------- */
	}
	else
	{
		/* -------------------------------------------------------------------------------------- */
		/* IBM PC (Model 5150/5160): Quote from "Technical Reference"; I/O Channel Description:   */
		/* -------------------------------------------------------------------------------------- */
		/* Signal Name:                                                                           */
		/*  - +IRQ2 (Input): Interrupt Request 2 to 7: These lines are used to signal the         */
		/*  microprocessor that an I/O device required attention. They are prioritized with IRQ2  */
		/*  as the highest priority and IRQ7 as the lowest. An Interrupt Request is generated by  */
		/*  raising an IRQ line (low to high) and holding it high until it is acknowledged by the */
		/*  the microprocessor (interrupt service routine).                                       */
		/* -------------------------------------------------------------------------------------- */
		pc->fdc = NULL;
		sct = ini_next_sct(ini, NULL, "fdc");
		if (sct == NULL) {
			return;
		}
		ini_get_uint32(sct, "address", &addr, 0x3f0);
		ini_get_uint16(sct, "irq", &irq, 6);
		ini_get_bool(sct, "accurate", &accurate, 0);
		ini_get_bool(sct, "ignore_eot", &ignore_eot, 0);
		ini_get_uint16(sct, "drive0", &drv[0], 0xffff);
		ini_get_uint16(sct, "drive1", &drv[1], 0xffff);
		ini_get_uint16(sct, "drive2", &drv[2], 0xffff);
		ini_get_uint16(sct, "drive3", &drv[3], 0xffff);
		pce_log_tag(MSG_INF, "FDC:",
			"addr=0x%08lx irq=%u accurate=%d eot=%d drv=[%u %u %u %u]\n",
			addr, irq, accurate, !ignore_eot,
			drv[0], drv[1], drv[2], drv[3]
		);
		pc->fdc = dev_fdc_new(addr);
		if (pc->fdc == NULL) {
			pce_log(MSG_ERR, "ERROR: Unable to initialize floppy disk controller.\n");
			return;
		}
		dev_fdc_mem_add_io(pc->fdc, pc->prt);
		dev_fdc_set_disks(pc->fdc, pc->dsk);
		dev_fdc_set_drive(pc->fdc, 0, drv[0]);
		dev_fdc_set_drive(pc->fdc, 1, drv[1]);
		dev_fdc_set_drive(pc->fdc, 2, drv[2]);
		dev_fdc_set_drive(pc->fdc, 3, drv[3]);
		e8272_set_input_clock(&pc->fdc->e8272, PCE_IBMPC_CLK2);
		e8272_set_accuracy(&pc->fdc->e8272, accurate != 0);
		e8272_set_ignore_eot(&pc->fdc->e8272, ignore_eot != 0);
		e8272_set_irq_fct(&pc->fdc->e8272, &pc->pic, e8259_get_irq_fct(&pc->pic, irq));
		e8272_set_dreq_fct(&pc->fdc->e8272, &pc->dma, e8237_set_dreq2);
		/* -------------------------------------------------------------------------------------- */
		/* IBM PC (Model 5150/5160): Quote from "Technical Reference"; I/O Channel Description:   */
		/* -------------------------------------------------------------------------------------- */
		/* Signal Name:                                                                           */
		/*  - +T/C (Output): Terminal Count: This line provides a pulse when the terminal count   */
		/*  for any DMA channel is reached. This signal is active high.                           */
		/* -------------------------------------------------------------------------------------- */
		e8237_set_tc_fct(&pc->dma, 2, &pc->fdc->e8272, e8272_set_tc);
		/* -------------------------------------------------------------------------------------- */
		/* IBM PC (Model 5150/5160): Quote from "Technical Reference"; I/O Channel Description:   */
		/* -------------------------------------------------------------------------------------- */
		/* Signal Name:                                                                           */
		/*  - -IOR (Output): -I/O Read Command: This command line instructs an I/O device to drive*/
		/*  its data onto the data bus. It may be driven by the microprocessor or the DMA         */
		/*  controller. This signal is active low.                                                */
		/* -------------------------------------------------------------------------------------- */
		pc->dma.chn[2].iord_ext = &pc->fdc->e8272;
		pc->dma.chn[2].iord = (void *)e8272_read_data;
		/* -------------------------------------------------------------------------------------- */
		/* IBM PC (Model 5150/5160): Quote from "Technical Reference"; I/O Channel Description:   */
		/* -------------------------------------------------------------------------------------- */
		/* Signal Name:                                                                           */
		/*  - -IOW (Output): -I/O Write Command: This command line instructs an I/O device to read*/
		/*  the data on the data bus. It may be driven by the microprocessor or the DMA           */
		/*  controller. This signal is active low.                                                */
		/* -------------------------------------------------------------------------------------- */
		pc->dma.chn[2].iowr_ext = &pc->fdc->e8272;
		pc->dma.chn[2].iowr = (void *)e8272_write_data;
		/* -------------------------------------------------------------------------------------- */
		/* IBM PC (Model 5150/5160): Quote from "Technical Reference"; I/O Channel Description:   */
		/* -------------------------------------------------------------------------------------- */
		/* Signal Name:                                                                           */
		/*  - -MEMR (Output): -Memory Read Command: This command line instructs the memory to     */
		/*  drive its data onto the data bus. It may be driven by the microprocessor or the DMA   */
		/*  controller. This signal is active low.                                                */
		/* -------------------------------------------------------------------------------------- */
		pc->dma.chn[2].memrd_ext = pc;
		pc->dma.chn[2].memrd = (void *)pc_dma2_get_mem8;
		/* -------------------------------------------------------------------------------------- */
		/* IBM PC (Model 5150/5160): Quote from "Technical Reference"; I/O Channel Description:   */
		/* -------------------------------------------------------------------------------------- */
		/* Signal Name:                                                                           */
		/*  - -MEMW (Output): -Memory Write Command: This command line instructs the memory to    */
		/*  store the data present on the data bus. It may be driven by the microprocessor or the */
		/*  DMA controller. This signal is active low.                                            */
		/* -------------------------------------------------------------------------------------- */
		pc->dma.chn[2].memwr_ext = pc;
		pc->dma.chn[2].memwr = (void *)pc_dma2_set_mem8;
	}
}

static
void pc_setup_hdc (ibmpc_t *pc, ini_sct_t *ini)
{
	unsigned      irq;
	unsigned long addr;
	unsigned      drv[2];
	unsigned      cfg;
	unsigned      sectors;
	const char    *cfg_id;
	ini_sct_t     *sct;
	/* --------------------------------------------------------------------------------------- */
	/* IBM PC (Model 5150/5160): Quote from "Technical Reference"; I/O Channel Description:    */
	/* --------------------------------------------------------------------------------------- */
	/* Signal Name:                                                                            */
	/*  - +IRQ2 (Input): Interrupt Request 2 to 7: These lines are used to signal the          */
	/*  microprocessor that an I/O device required attention. They are prioritized with IRQ2   */
	/*  as the highest priority and IRQ7 as the lowest. An Interrupt Request is generated by   */
	/*  raising an IRQ line (low to high) and holding it high until it is acknowledged by the  */
	/*  the microprocessor (interrupt service routine).                                        */
	/* --------------------------------------------------------------------------------------- */
	/* ------------------------------------------------------------------------------------------ */
	/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; I/O Channel; Signal   */
	/* Description:                                                                               */
	/* ------------------------------------------------------------------------------------------ */
	/* The following is a description of the I/O channel signal lines. All lines are TTL-         */
	/* compatible. The (O), (I), or (I/O) notation refers to output, input, or input and output.  */
	/*  - IRQ2-IRQ7 (I): Interrupt requests 2 through 7: These lines are used to signal the       */
	/*  microprocessor that an I/O device requires atten-tion. They are prioritized with IRQ2 as  */
	/*  the highest priority and IRQ7 as the lowest. When an interrupt is generated, the request  */
	/*  line is held active unti lit is acknowledged by the microprocessor.                       */
	/* ------------------------------------------------------------------------------------------ */
	/* Initialize Hard Disk Controller: */
	pc->hdc = NULL;
	sct = ini_next_sct (ini, NULL, "hdc");
	if (sct == NULL) {
		return;
	}
	ini_get_uint32 (sct, "address", &addr, 0x320);
	ini_get_uint16 (sct, "irq", &irq, 5);
	ini_get_uint16 (sct, "drive0", &drv[0], 0xffff);
	ini_get_uint16 (sct, "drive1", &drv[1], 0xffff);
	ini_get_uint16 (sct, "sectors", &sectors, 17);
	ini_get_string (sct, "config_id", &cfg_id, "");
	ini_get_uint16 (sct, "switches", &cfg, 0);
	pce_log_tag (MSG_INF, "HDC:",
		"addr=0x%08lx irq=%u drv=[%u %u] sectors=%u switches=0x%02x id=\"%s\"\n",
		addr, irq, drv[0], drv[1], sectors, cfg, cfg_id
	);
	pc->hdc = hdc_new (addr);
	if (pc->hdc == NULL) {
		pce_log (MSG_ERR, "ERROR: Unable to initialize hard disk controller.\n");
		return;
	}
	hdc_mem_add_io (pc->hdc, pc->prt);
	hdc_set_disks (pc->hdc, pc->dsk);
	hdc_set_drive (pc->hdc, 0, drv[0]);
	hdc_set_drive (pc->hdc, 1, drv[1]);
	hdc_set_config_id (pc->hdc, (const unsigned char *) cfg_id, strlen (cfg_id));
	hdc_set_config (pc->hdc, cfg);
	hdc_set_sectors (pc->hdc, sectors);
	hdc_set_irq_fct (pc->hdc, &pc->pic, e8259_get_irq_fct (&pc->pic, irq));
	/* -------------------------------------------------------------------------------------- */
	/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; Interrupts:       */
	/* -------------------------------------------------------------------------------------- */
	/* The interrupt controller has eight levels of interrupts that are handled according to  */
	/* priority in the I/O support gate array. Two levels are used only on the system board.  */
	/* Level 0, the highest priority, is attached to Channel 0 of the timer/counter and       */
	/* provides a periodic interrupt for the timer tick. Level 1 is shared by the keyboard and*/
	/* the pointing device. It is handled by a BIOS routine pointed to by inter-rupt hex 71.  */
	/* Level 2 is available to the video subsystem, and level 7 is available to the parallel  */
	/* port; however, the BIOS routines do not use interrupts 2 and 7. Level 4 is used by the */
	/* serial port.                                                                           */
	/*                                                                                        */
	/* The controller also has inputs from coprocessor's '-interrupt', the memory controller's*/
	/*'-parity', and '-I/O channel check' signals. These three inputs are used to generate the*/
	/* NMI to the 8086-2.                                                                     */
	/*                                                                                        */
	/* The following table shows the hardware interrupts and their avail-ability to the I/O   */
	/* channel.                                                                               */
	/* -------------------------------------------------------------------------------------- */
	/* Level:  System Board:                   I/O Channel:                                   */
	/*  - NMI : - Parity Check and Coprocessor: - I/O Channel Check:                          */
	/*  - IRQ0: - Timer Channel 0:              - Not Available:                              */
	/*  - IRQ1: - Keyboard Pointing Device:     - Not Available:                              */
	/*  - IRQ2: - Video:                        -     Available:                              */
	/*  - IRQ4: - Serial Port:                  -     Available:                              */
	/*  - IRQ5: - Fixed Disk:                   -     Available:                              */
	/*  - IRQ6: - Diskette Drive:               -     Available:                              */
	/*  - IRQ7: - Parallel Port:                -     Available:                              */
	/* NOTE: Interrupts are available to the I/O channel if they are not enabled by the system*/
	/* board function normally assigned to that interrupt.                                    */
	/* -------------------------------------------------------------------------------------- */
	/* Figure 1-9. Hardware Interrupt Listing:                                                */
	/* -------------------------------------------------------------------------------------- */
	/* --------------------------------------------------------------------------------------- */
	/* IBM PC (Model 5150/5160): Quote from "Technical Reference"; I/O Channel Description:    */
	/* --------------------------------------------------------------------------------------- */
	/* Signal Name:                                                                            */
	/*  - +DRQ2 (Input): DMA Request 1 to 3: These lines are asynchronous channel requests     */
	/*  used by peripheral devices to gain DMA service. They are prioritized with DRQ3 being   */
	/*  the lowest and DRQ1 being the highest. A DRQ line must be held high until the          */
	/*  corresponding DACK line goes active.                                                   */
	/* --------------------------------------------------------------------------------------- */
	/* ------------------------------------------------------------------------------------------ */
	/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; I/O Channel; Signal   */
	/* Description:                                                                               */
	/* ------------------------------------------------------------------------------------------ */
	/* The following is a description of the I/O channel signal lines. All lines are TTL-         */
	/* compatible. The (O), (I), or (I/O) notation refers to output, input, or input and output.  */
	/*  - DRQ2-DRQ3 (I): DMA Requests 1 to 3: These lines are asyachro-nous channel requests used */
	/*  by peripheral devices to gain DMA ser-vices. They are prioritized with DRQ1 being the     */
	/*  highest and DRQ3 being the lowest. A request is generated by bringing a request line to an*/
	/*  active level. A request line is held active until the corresponding acknowledge line goes */
	/*  active.                                                                                   */
	/* ------------------------------------------------------------------------------------------ */
	hdc_set_dreq_fct (pc->hdc, &pc->dma, e8237_set_dreq3);
	/* --------------------------------------------------------------------------------------- */
	/* IBM PC (Model 5150/5160): Quote from "Technical Reference"; I/O Channel Description:    */
	/* --------------------------------------------------------------------------------------- */
	/* Signal Name:                                                                            */
	/*  - -IOR (Output): -I/O Read Command: This command line instructs an I/O device to drive */
	/*  its data onto the data bus. It may be driven by the microprocessor or the DMA          */
	/*  controller. This signal is active low.                                                 */
	/* --------------------------------------------------------------------------------------- */
	/* ------------------------------------------------------------------------------------------ */
	/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; I/O Channel; Signal   */
	/* Description:                                                                               */
	/* ------------------------------------------------------------------------------------------ */
	/* The following is a description of the I/O channel signal lines. All lines are TTL-         */
	/* compatible. The (O), (I), or (I/O) notation refers to output, input, or input and output.  */
	/*  - -IOR (O): -I/O Read: This command line instructs an I/O device to drive its data onto   */
	/*  the data bus. This signal is driven by the micro-processor or the DMA controller.         */
	/* ------------------------------------------------------------------------------------------ */
	pc->dma.chn[3].iord_ext = pc->hdc;
	pc->dma.chn[3].iord = (void *) hdc_read_data;
	/* --------------------------------------------------------------------------------------- */
	/* IBM PC (Model 5150/5160): Quote from "Technical Reference"; I/O Channel Description:    */
	/* --------------------------------------------------------------------------------------- */
	/* Signal Name:                                                                            */
	/*  - -IOW (Output): -I/O Write Command: This command line instructs an I/O device to read */
	/*  the data on the data bus. It may be driven by the microprocessor or the DMA            */
	/*  controller. This signal is active low.                                                 */
	/* --------------------------------------------------------------------------------------- */
	/* ------------------------------------------------------------------------------------------ */
	/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; I/O Channel; Signal   */
	/* Description:                                                                               */
	/* ------------------------------------------------------------------------------------------ */
	/* The following is a description of the I/O channel signal lines. All lines are TTL-         */
	/* compatible. The (O), (I), or (I/O) notation refers to output, input, or input and output.  */
	/*  - -IOW (O): -I/O Write: This command line instructs an I/O device to read the data on the */
	/*  data bus. This signal is driven by the micro-processor or the DMA controller.             */
	/* ------------------------------------------------------------------------------------------ */
	pc->dma.chn[3].iowr_ext = pc->hdc;
	pc->dma.chn[3].iowr = (void *) hdc_write_data;
	/* --------------------------------------------------------------------------------------- */
	/* IBM PC (Model 5150/5160): Quote from "Technical Reference"; I/O Channel Description:    */
	/* --------------------------------------------------------------------------------------- */
	/* Signal Name:                                                                            */
	/*  - -MEMR (Output): -Memory Read Command: This command line instructs the memory to      */
	/*  drive its data onto the data bus. It may be driven by the microprocessor or the DMA    */
	/*  controller. This signal is active low.                                                 */
	/* --------------------------------------------------------------------------------------- */
	/* ------------------------------------------------------------------------------------------ */
	/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; I/O Channel; Signal   */
	/* Description:                                                                               */
	/* ------------------------------------------------------------------------------------------ */
	/* The following is a description of the I/O channel signal lines. All lines are TTL-         */
	/* compatible. The (O), (I), or (I/O) notation refers to output, input, or input and output.  */
	/*  - -MEMR (O): -Memory Read: This command line instructs memory to drive its data onto the  */
	/*  data bus. This signal is driven by the micro-processor or the DMA controller.             */
	/* ------------------------------------------------------------------------------------------ */
	pc->dma.chn[3].memrd_ext = pc;
	pc->dma.chn[3].memrd = (void *) pc_dma3_get_mem8;
	/* --------------------------------------------------------------------------------------- */
	/* IBM PC (Model 5150/5160): Quote from "Technical Reference"; I/O Channel Description:    */
	/* --------------------------------------------------------------------------------------- */
	/* Signal Name:                                                                            */
	/*  - -MEMW (Output): -Memory Write Command: This command line instructs the memory to     */
	/*  store the data present on the data bus. It may be driven by the microprocessor or the  */
	/*  DMA controller. This signal is active low.                                             */
	/* --------------------------------------------------------------------------------------- */
	/* ------------------------------------------------------------------------------------------ */
	/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; I/O Channel; Signal   */
	/* Description:                                                                               */
	/* ------------------------------------------------------------------------------------------ */
	/* The following is a description of the I/O channel signal lines. All lines are TTL-         */
	/* compatible. The (O), (I), or (I/O) notation refers to output, input, or input and output.  */
	/*  - -MEMW (O): -Memory Write: This command line instructs memory to store the data present  */
	/*  on the data bus. This signal is driven by the micro-processor or the DMA controller.      */
	/* ------------------------------------------------------------------------------------------ */
	pc->dma.chn[3].memwr_ext = pc;
	pc->dma.chn[3].memwr = (void *) pc_dma3_set_mem8;
	/* -------------------------------------------------------------------------------------- */
	/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; DMA Controller:   */
	/* -------------------------------------------------------------------------------------- */
	/* The 8237 Direct Memory Access (DMA) controller and its support logic in the gate array */
	/*support four channels of 20 address bit DMA. It operates at 4 MHz and handles only 8-bit*/
	/* data transfers. The DMA channel assignments and page register addresses are:           */
	/* -------------------------------------------------------------------------------------- */
	/* Level:  Assignment:                                                                    */
	/*  - DRQ0: - Not Available                                                               */
	/*  - DRQ1: - Not Used                                                                    */
	/*  - DRQ2: - Diskette                                                                    */
	/*  - DRQ3: - Fixed Disk                                                                  */
	/* -------------------------------------------------------------------------------------- */
	/* Figure 1-7. DMA Channel Assignments:                                                   */
	/* -------------------------------------------------------------------------------------- */
	/* Hex Address: DMA Page Register:                                                        */
	/*  - 081:       - Channel 2:                                                             */
	/*  - 082:       - Channel 3:                                                             */
	/*  - 083:       - Channel 1:                                                             */
	/*  - 087:       - Channel 0:                                                             */
	/* -------------------------------------------------------------------------------------- */
	/* Figure 1-8. DMA Page Register Addresses:                                               */
	/* -------------------------------------------------------------------------------------- */
	/* Three of the DMA channels (1, 2, and 3) are available on the I/O bus and support high- */
	/*speed data transfers between I/O devices and memory without microprocessor intervention.*/
	/*                                                                                        */
	/*DMA data transfers take six clock cycles of 250 ns, or 1.5 micro-seconds. I/O CH RDY can*/
	/* be pulled inactive to add wait states to allow more time for slower devices.           */
	/* -------------------------------------------------------------------------------------- */
}

static
void pc_setup_parport (ibmpc_t *pc, ini_sct_t *ini)
{
	/* Initialize Parallel Port: */
	unsigned        i;
	unsigned long   addr;
	const char      *driver;
	ini_sct_t       *sct;
	static unsigned defbase[4] = { 0x378, 0x278, 0x3bc, 0x2bc };
	/* Initialize Parralel Port: */
	for (i = 0; i < 4; i++) {
		pc->parport[i] = NULL;
	}
	i = 0;
	sct = NULL;
	while ((sct = ini_next_sct (ini, sct, "parport")) != NULL) {
		if (i >= 4) {
			break;
		}
		ini_get_uint32 (sct, "address", &addr, defbase[i]);
		ini_get_string (sct, "driver", &driver, NULL);
		pce_log_tag (MSG_INF,
			"PARPORT:", "LPT%u addr=0x%04lx driver=%s\n",
			i + 1, addr, (driver == NULL) ? "<none>" : driver
		);
		pc->parport[i] = parport_new (addr);
		if (pc->parport[i] == NULL) {
			pce_log (MSG_ERR,
				"ERROR: Unable to initialize parallel port LPT%u.\n", i + 1
			);
		}
		else {
			if (driver != NULL) {
				if (parport_set_driver (pc->parport[i], driver)) {
					pce_log (MSG_ERR,
						"ERROR: Unable to open following parallel port device: (%s)\n",
						driver
					);
				}
			}
			mem_add_blk (pc->prt, parport_get_reg (pc->parport[i]), 0);
			i += 1;
		}
	}
}

static
void pc_setup_serport (ibmpc_t *pc, ini_sct_t *ini)
{
	unsigned      i;
	unsigned long addr;
	unsigned      irq;
	unsigned      multi, multi_read, multi_write;
	const char    *driver;
	const char    *log;
	const char    *chip;
	ini_sct_t     *sct;
	/* --------------------------------------------------------------------------------------- */
	/* IBM PC (Model 5150/5160): Quote from "Technical Reference"; I/O Channel Description:    */
	/* --------------------------------------------------------------------------------------- */
	/* Signal Name:                                                                            */
	/*  - +IRQ2 (Input): Interrupt Request 2 to 7: These lines are used to signal the          */
	/*  microprocessor that an I/O device required attention. They are prioritized with IRQ2   */
	/*  as the highest priority and IRQ7 as the lowest. An Interrupt Request is generated by   */
	/*  raising an IRQ line (low to high) and holding it high until it is acknowledged by the  */
	/*  the microprocessor (interrupt service routine).                                        */
	/* --------------------------------------------------------------------------------------- */
	/* ------------------------------------------------------------------------------------------ */
	/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; I/O Channel; Signal   */
	/* Description:                                                                               */
	/* ------------------------------------------------------------------------------------------ */
	/* The following is a description of the I/O channel signal lines. All lines are TTL-         */
	/* compatible. The (O), (I), or (I/O) notation refers to output, input, or input and output.  */
	/*  - IRQ2-IRQ7 (I): Interrupt requests 2 through 7: These lines are used to signal the       */
	/*  microprocessor that an I/O device requires atten-tion. They are prioritized with IRQ2 as  */
	/*  the highest priority and IRQ7 as the lowest. When an interrupt is generated, the request  */
	/*  line is held active unti lit is acknowledged by the microprocessor.                       */
	/* ------------------------------------------------------------------------------------------ */
	static unsigned long defbase[4] = { 0x3f8, 0x2f8, 0x3e8, 0x2e8 };
	static unsigned      defirq[4] = { 4, 3, 4, 3 };
	/* Initialize Intel 8250 UART chip: */
	for (i = 0; i < 4; i++) {
		pc->serport[i] = NULL;
	}
	i = 0;
	sct = NULL;
	while ((sct = ini_next_sct (ini, sct, "serial")) != NULL) {
		if (i >= 4) {
			break;
		}
		if (ini_get_uint32 (sct, "address", &addr, defbase[i])) {
			ini_get_uint32 (sct, "io", &addr, defbase[i]);
		}
		ini_get_uint16 (sct, "irq", &irq, defirq[i]);
		ini_get_string (sct, "uart", &chip, "8250");
		ini_get_uint16 (sct, "multichar", &multi, 1);
		ini_get_uint16 (sct, "multichar_read", &multi_read, multi);
		ini_get_uint16 (sct, "multichar_write", &multi_write, multi);
		ini_get_string (sct, "driver", &driver, NULL);
		ini_get_string (sct, "log", &log, NULL);
		pce_log_tag (MSG_INF,
			"SERPORT:", "COM%u addr=0x%04lx irq=%u multichar=r=%u/w=%u uart=%s\n",
			i + 1, addr, irq, multi_read, multi_write, chip
		);
		if (driver != NULL) {
			pce_log_tag (MSG_INF,
				"SERPORT:", "COM%u driver=%s\n",
				i + 1, driver
			);
		}
		pc->serport[i] = ser_new (addr, 0);
		if (pc->serport[i] == NULL) {
			pce_log (MSG_ERR, "ERROR: Unable to initialize following serial port: [%04X/%u -> %s]\n",
				addr, irq, (driver == NULL) ? "<none>" : driver
			);
		}
		else {
			if (driver != NULL) {
				if (ser_set_driver (pc->serport[i], driver)) {
					pce_log (MSG_ERR,
						"ERROR: Unable to initialize following serial port device: (%s)\n",
						driver
					);
				}
			}
			if (log != NULL) {
				if (ser_set_log (pc->serport[i], log)) {
					pce_log (MSG_ERR,
						"ERROR: Unable to open LOG file for serial port device: (%s)\n",
						log
					);
				}
			}
			if (e8250_set_chip_str (&pc->serport[i]->uart, chip)) {
				pce_log (MSG_ERR, "ERROR: Incorrect UART chip specified for serial port device: (%s)\n", chip);
			}
			e8250_set_multichar (&pc->serport[i]->uart, multi_read, multi_write);
			e8250_set_irq_fct (&pc->serport[i]->uart,
                /* Interface with Intel 8259 Programmable Interrupt Controller (PIC): */
				&pc->pic, e8259_get_irq_fct (&pc->pic, irq)
			);
			mem_add_blk (pc->prt, ser_get_reg (pc->serport[i]), 0);
			i += 1;
		}
	}
	/* -------------------------------------------------------------------------------------- */
	/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; Interrupts:       */
	/* -------------------------------------------------------------------------------------- */
	/* The interrupt controller has eight levels of interrupts that are handled according to  */
	/* priority in the I/O support gate array. Two levels are used only on the system board.  */
	/* Level 0, the highest priority, is attached to Channel 0 of the timer/counter and       */
	/* provides a periodic interrupt for the timer tick. Level 1 is shared by the keyboard and*/
	/* the pointing device. It is handled by a BIOS routine pointed to by inter-rupt hex 71.  */
	/* Level 2 is available to the video subsystem, and level 7 is available to the parallel  */
	/* port; however, the BIOS routines do not use interrupts 2 and 7. Level 4 is used by the */
	/* serial port.                                                                           */
	/*                                                                                        */
	/* The controller also has inputs from coprocessor's '-interrupt', the memory controller's*/
	/*'-parity', and '-I/O channel check' signals. These three inputs are used to generate the*/
	/* NMI to the 8086-2.                                                                     */
	/*                                                                                        */
	/* The following table shows the hardware interrupts and their avail-ability to the I/O   */
	/* channel.                                                                               */
	/* -------------------------------------------------------------------------------------- */
	/* Level:  System Board:                   I/O Channel:                                   */
	/*  - NMI : - Parity Check and Coprocessor: - I/O Channel Check:                          */
	/*  - IRQ0: - Timer Channel 0:              - Not Available:                              */
	/*  - IRQ1: - Keyboard Pointing Device:     - Not Available:                              */
	/*  - IRQ2: - Video:                        -     Available:                              */
	/*  - IRQ4: - Serial Port:                  -     Available:                              */
	/*  - IRQ5: - Fixed Disk:                   -     Available:                              */
	/*  - IRQ6: - Diskette Drive:               -     Available:                              */
	/*  - IRQ7: - Parallel Port:                -     Available:                              */
	/* NOTE: Interrupts are available to the I/O channel if they are not enabled by the system*/
	/* board function normally assigned to that interrupt.                                    */
	/* -------------------------------------------------------------------------------------- */
	/* Figure 1-9. Hardware Interrupt Listing:                                                */
	/* -------------------------------------------------------------------------------------- */
	/* ------------------------------------------------------------------------------------------ */
	/* IBM PC (Model 5150/5160): Quote from Technical Reference: Communications:                  */
	/* ------------------------------------------------------------------------------------------ */
	/* Information processing equipment used for communications is called data terminal equipment */
	/* (DTE). Equipment used to connect the DTO to the communications line is called data         */
	/* communications equipment (DCE).                                                            */
	/*                                                                                            */
	/*An adapter is used to connect the data terminal equipment to the data communications line as*/
	/* shown in the following illustration:                                                       */
	/*  - Data Terminal      ->       Data Communications       : ->Communications Line:          */
	/*      Equipment      :              Equipment:                                              */
	/*  - EIA/CCITT Adapter: ->       Cable Conforming to                                         */
	/*                              EIA or CCITT Standards      : ->Modem Clock: - > Voice Line:  */
	/* The EIA/CCITT adapter allows data terminal equipment to be connected to data communications*/
	/* equipment using EIA or CCITT standardized connections. An external modem is shown in this  */
	/*example; however, other types of data communications equipment can also be connected to data*/
	/* terminal equipment using EIA or CCITT standardized connections.                            */
	/*                                                                                            */
	/*EIA standards are labeled RS-x (Recommended Standards-x) and CCITT standards are labeled V.x*/
	/* or X.x, where x is the number of the standard.                                             */
	/*                                                                                            */
	/* The EIA RS-232 interface standard defines the connector type, pin numbers, line names, and */
	/* signal levels used to connect data terminal equipment to data communications equipment for */
	/* the purpose of transmitting and receiving data. Since the RS-232 standard was developed, it*/
	/* has been revised three times. The three revised standards are the RS-232A, the RS-232B, and*/
	/* the presently used RS-232C.                                                                */
	/*                                                                                            */
	/* The CCITT V.24 interface standard is equivalent to the RS-232C standard; therefore, the    */
	/* descriptions of the EIA standards also apply to the CCITT standards.                       */
	/*                                                                                            */
	/* The following is an illustration of data terminal equipment connected to an external modem */
	/* using connections defined by the RS-232C interface standard:                               */
	/*  - Data Terminal Equipment: -> Data Communications Equipment: - >                          */
	/*  - Adapter: -> Cable Conforming To -> Modem: -> Communications Line:                       */
	/*                 RS-232C Standard:                                                          */
	/*  - EIA/CCITT Line Number: ------------------------------------> Telephone Co. Lead Number: */
	/*  - Data Terminal Equipment: ------------------------------------------------------> Modem: */
	/* ------------------------------------------------------------------------------------------ */
	/* External Modem Cable Connector:                                                            */
	/* ------------------------------------------------------------------------------------------ */
	/* -  1: Protective Ground             : AA/101            : ------------------------> Modem: */
	/* -  2: Transmitted Data              : BA/103            : ------------------------> Modem: */
	/* -  3: Received    Data              : BB/104            : ------------------------> Modem: */
	/* -  4: Request to Send               : CA/105            : ------------------------> Modem: */
	/* -  5: Clear   to Send               : CB/106            : ------------------------> Modem: */
	/* -  6: Data Set Ready                : CC/107            : ------------------------> Modem: */
	/* -  7: Signal Ground                 : AB/102            : ------------------------> Modem: */
	/* -  8: Received Line Signal Detector : CF/109            : ------------------------> Modem: */
	/* -  9:                                                                                      */
	/* - 10:                                                                                      */
	/* - 11: Select Standby                : ''/116            : ------------------------> Modem: */
	/* - 12:                                                                                      */
	/* - 13:                                                                                      */
	/* - 14:                                                                                      */
	/* - 15: Transmit Signal Element Timing: DB/114            : ------------------------> Modem: */
	/* - 16:                                                                                      */
	/* - 17: Receive  Signal Element Timing: DD/115            : ------------------------> Modem: */
	/* - 18: Test                          : ''/'''            : ------------------------> Modem: */
	/* - 19:                                                                                      */
	/* - 20: Data Terminal Ready                                                                  */
	/*       Connect Data Set to Line      : CD/108.2 /''/108.1: ------------------------> Modem: */
	/* - 21:                                                                                      */
	/* - 22: Ring Indicator                : DE/125            : ------------------------> Modem: */
	/* - 23: Speed Select                  : CH/111            : ------------------------> Modem: */
	/* - 24:                                                                                      */
	/* - 25:                                                                                      */
	/* ------------------------------------------------------------------------------------------ */
	/* External Modem Connector:                                                                  */
	/* -------------------------------------------------------   *Not used when business machine  */
	/* | 13: 12: 11: 10: 9:  8:  7:  6:  5:  4:  3:  2:  1:  |   clocking is used.                */
	/* | O   O   O   O   O   O   O   O   O   O   O   O   O   |  **Not standardized by EIA         */
	/* |   O   O   O   O   O   O   O   O   O   O   O   O     |(Electronic Industries Association).*/
	/* |  25: 24: 23: 22: 21: 20: 19: 18: 17: 16: 15: 14:    | *** Not standardized by CCITT.     */
	/* | |                       |    (Modem) DCE            |                                    */
	/* | |-----Data Terminal-----|----Data Communications----|                                    */
	/* |       Equipment         |    Equipment              |                                    */
	/* |                    Pin Number                       |                                    */
	/* -------------------------------------------------------                                    */
	/* ------------------------------------------------------------------------------------------ */
	/* Establishing a Communications Link:                                                        */
	/* ------------------------------------------------------------------------------------------ */
	/* The following bar graphs represent normal timing sequences of operation during the         */
	/* establishment of communications for both switched (dial-up) and nonswitched (direct line)  */
	/* networks.                                                                                  */
	/* Switched    Timing Sequence:                                                               */
	/* - Data Terminal Ready: ___|--------------------------------------------------------------- */
	/* - Data Set      Ready: ______|------------------------------------------------------------ */
	/* - Request to Send    : _________|-------------------------------------------------|_______ */
	/* - Clear   to Send    : _____________|------------------------------------------------|____ */
	/* - Transmitted Data   : ________________|------------------------------------------|_______ */
	/* Nonswitched Timing Sequence:                                                               */
	/* - Data Terminal Ready: ___|--------------------------------------------------------------- */
	/* - Data Set      Ready: ______|------------------------------------------------------------ */
	/* - Request to Send    : _________|-----------------------------------------------------|___ */
	/* - Clear   to Send    : _____________|----------------------------------------------------|_*/
	/* - Transmitted Data   : ________________|------------------------------------------|_______ */
	/* The following examples show how a link is established on a nonswitched point-to-point line,*/
	/* a nonswitched multipoint line, and a switched point-to-point line.                         */
	/* ------------------------------------------------------------------------------------------ */
	/* Establishing a Link on a Nonswitched Point-to-Point Line:                                  */
	/* ------------------------------------------------------------------------------------------ */
	/* -  1. The terminals at both locations activate the 'data terminal ready' lines 1 and 8.    */
	/* -  2. Normally the 'data set ready' lines 2 and 9 from the modems are active whenever the  */
	/* modems are powered on.                                                                     */
	/* -  3. Terminal A activates the 'request to send' line, which causes the modem at terminal A*/
	/* to generate a carrier signal.                                                              */
	/* -  4. Modem B detects the carrier, and activates the 'received line signal detector' line  */
	/* (sometimes called data carrier detect) 16. Modem B also activates the 'receiver signal     */
	/* element timing' line (sometimes called receive clock) 11 to send receive clock signals to  */
	/* the terminal. Some modems activate the clock signals whenever the modem is powered on.     */
    /* -  5. After a specified delay, modem A activates the 'clear to send' line 4 which indicates*/
	/* to terminal A that the modem is ready to transmit data.                                    */
    /* -  6. Terminal A serializes the data to be transmitted through the serdes) and transmits   */
	/* the data one bit at a time (synchronized by the transmit clock) onto the 'transmitted data'*/
	/* line 6 to the modem.                                                                       */
    /* -  7. The modem modulates the carrier signal with the data and transmits it to the modem B */
	/* 5.                                                                                         */
    /* -  8. Modem B demodulates the data from the carrier signal and sends it to terminal B on   */
	/* the 'received data' line 12.                                                               */
    /* -  9. Terminal B deserializes the data (through the serdes) using the receive clock signals*/
	/* (on the 'receiver signal element timing' line) 11 from the modem.                          */
    /* - 10. After terminal A completes its transmission, it deactivates the 'request to send'    */
	/* line 3, which causes the modem to turn off the carrier and deactivate the 'clear to send'  */
	/* line 4.                                                                                    */
    /* - 11. Terminal A and modem A now become receivers and wait for a response from terminal B, */
	/* indicating that all data has reached terminal B. Modem A begins an echo delay (50 to 150   */
	/* milliseconds) to ensure that all echoes on the line have diminished before it begins       */
	/* receiving. An echo is a reflection of the transmitted signal. If the transmitting modem    */
	/* changed to receive too soon, it could receive a reflection (echo) of the signal it just    */
	/* transmitted.                                                                               */
    /* - 12. Modem B deactivates the 'received line signal detector' line 10 and, if necessary,   */
	/* deactivates the receive clock signals on the 'receiver signal element timing, line 11.     */
    /* - 13. Terminal B now becomes the transmitter to respond to the request from terminal A. To */
	/* transmit data, terminal B activates the 'request to send' line 13, which causes modem B to */
	/* transmit a carrier to modem A.                                                             */
    /* - 14. Modem B begins a delay that is longer than the echo delay at modem A before turning  */
	/* on the 'clear to send' line. The longer delay (called request-to-send delay) ensures that  */
	/*modem A is ready to receive when terminal B begins transmitting data. After the delay, modem*/
	/* B activates the 'clear to send' line 14 to indicate that terminal B can begin transmitting */
	/* its response.                                                                              */
    /* - 15. After the echo delay at modem A, modem A sense the carrier from modem B (the carrier */
	/*was activated in step 13 when terminal B activated the 'request to send' line) and activates*/
	/* the 'received line signal detector; line 7 to terminal A.                                  */
    /* - 16. Modem A and terminal A are ready to receive the response from terminal B. Remember,  */
	/* the response was not transmitted until after the request-to-send to clear-to-send delay at */
	/* modem B (step 14).                                                                         */
    /* ------------------------------------------------------------------------------------------ */
    /* Establishing a Link on a Nonswitched Multipoint Line:                                      */
    /* ------------------------------------------------------------------------------------------ */
    /* -  1. The control station serializes the address for the tributary or secondary station    */
	/* (AA) and sends its address to the modem on the 'transmitted data' line 2.                  */
    /* -  2. Since the 'request to send' line and, therefore, the modem carrier, is active        */
	/*continuously 1, the modem immediately modulates the carrier with the address, and, thus, the*/
	/* address is transmitted to all modems on the the line.                                      */
    /* -  3. All tributary modems, including the modem for station A, demodulate the address and  */
	/* send it to their terminals on the 'received data' line 5.                                  */
    /* -  4. Only station A responds to the address; the other stations ignore the address and    */
	/* continue monitoring their 'received data' line. To respond to the poll, station A activates*/
	/*its 'request to send' line 6, which causes the modem to begin transmitting a carrier signal.*/
    /* -  5. The control station's modem receives the carrier and activates the 'received line    */
	/* signal detector' line 3 and the 'receiver signal element timing' line 4 (to send clock     */
	/* signals to the control station). Some modems activate the clock signals as soon as they are*/
	/* powered on.                                                                                */
    /* -  6. After a short delay to allow the control station modem to receive the carrier, the   */
	/* tributary modem activates the 'clear to send' line 7.                                      */
    /* -  7. When station A detects the active 'clear to send' line, it transmits its response.   */
	/*(For this example, assume that station A has no data to send; therefore, it transmits an EOT*/
	/* 8.)                                                                                        */
    /* -  8. After transmitting the EOT, station A deactivates the 'request to send' line 6. The  */
	/* causes the modem to deactivate the carrier and the 'clear to send' line 7.                 */
    /* -  9. When the modem at the control station (host) detects the absence of the carrier, it  */
	/* deactivates the 'received line signal detector' line 3.                                    */
    /* - 10. Tributary station A is now in receive mode waiting for the next poll or select       */
	/* transmission from the control station.                                                     */
    /* ------------------------------------------------------------------------------------------ */
    /* Establishing a Link on a Switched Point-To-Point Line:                                     */
    /* ------------------------------------------------------------------------------------------ */
    /* -  1. Terminal A is in communications mode; therefore, the 'data terminal ready' line 1 is */
	/* active. Terminal B is in communication mode waiting for a call from terminal A.            */
    /* -  2. When the terminal A operator lifts the telephone handset, the 'switch hook' line from*/
	/* the coupler is activated 3.                                                                */
    /* -  3. Modem A detects the 'switch hook' line and activates the 'off hook' line 4, which    */
	/* causes the coupler to connect the telephone set to the line and activate the 'coupler cut- */
	/* through' line 5 to the modem.                                                              */
    /* -  4. Modem A activates the 'data modem ready' line 6 to the coupler (the 'data modem      */
	/* ready' line is on continuously in some modems).                                            */
    /* -  5. The terminal A operator sets the exclusion key or talk/data switch to the talk       */
	/* position to connect the handset to the communications line. The operator then dials the    */
	/* terminal B number.                                                                         */
    /* -  6. When the telephone at terminal B rings, the coupler activates the 'ring indicate'    */
	/* line to modem B 10. Modem B indicates that the 'ring indicate' line was activated by       */
	/* activating the 'ring indicator' line 13 to terminal B.                                     */
    /* -  7. Terminal B activates the 'data terminal ready' line to modem B 12 which activates the*/
	/* autoanswer circuits in modem B. (The 'data terminal ready' line might already be active in */
	/* some terminals.)                                                                           */
    /* -  8. The autoanswer circuits in modem B activate the 'off hook' line to the coupler 8.    */
    /* -  9. The coupler connects modem B to the communications line through the 'data tip' and   */
	/* 'data ring' lines11 and activates the 'coupler cut-through' line 9 to the modem. Modem B   */
	/* then transmits an answer tone to terminal A.                                               */
    /* - 10. The terminal A operator hears the tone and sets the exclusion key or talk/data switch*/
	/* to the data position (or performs an equivalent operation) to connect modem A to the       */
	/* communications line through the 'data set ready' line 2 indicating to terminal A that the  */
	/* modem is connected to the communications line.                                             */
    /*                                                                                            */
    /* The sequence of the remaining steps to establish the data link is the same as the sequence */
	/* required on a nonswitched point-to-point line. When the terminals have completed their     */
	/* transmission, they both deactivate the 'data terminal ready' line to disconnect the modems */
	/* from the line.                                                                             */
    /* ------------------------------------------------------------------------------------------ */
}

static
void pc_setup_ems (ibmpc_t *pc, ini_sct_t *ini)
{
	/* Initialize Expanded Memory Boards: */
	ini_sct_t *sct;
	mem_blk_t *mem;
	pc->ems = NULL;
	sct = ini_next_sct (ini, NULL, "ems");
	if (sct == NULL) {
		return;
	}
	pc->ems = ems_new (sct);
	if (pc->ems == NULL) {
		return;
	}
	mem = ems_get_mem (pc->ems);
	if (mem != NULL) {
		mem_add_blk (pc->mem, mem, 0);
	}
}

static
void pc_setup_xms (ibmpc_t *pc, ini_sct_t *ini)
{
	/* Initialize Extended Memory: */
	ini_sct_t *sct;
	mem_blk_t *mem;
	pc->xms = NULL;
	sct = ini_next_sct (ini, NULL, "xms");
	if (sct == NULL) {
		return;
	}
	pc->xms = xms_new (sct);
	if (pc->xms == NULL) {
		return;
	}
	mem = xms_get_umb_mem (pc->xms);
	if (mem != NULL) {
		mem_add_blk (pc->mem, mem, 0);
	}
	mem = xms_get_hma_mem (pc->xms);
	if (mem != NULL) {
		mem_add_blk (pc->mem, mem, 0);
	}
}

ibmpc_t *pc_new (ini_sct_t *ini)
{
	/* Power On Machine: */
	ibmpc_t *pc;
	pc = malloc (sizeof (ibmpc_t));
	if (pc == NULL) {
		return (NULL);
	}
	pc->cfg = ini;
	bps_init (&pc->bps);         /* Baud Rate for serial port devices and modems */
	pc_setup_psu(pc, ini);       /* Power Supply */
	pc_setup_system (pc, ini);   /* System Board */
	pc_setup_m24 (pc, ini);      /* Olivetti M24, AT&T 6300, Xerox 6060, and Logabax 1600 */
	pc_setup_mem (pc, ini);      /* System RAM */
	pc_setup_ports (pc, ini);    /* System Ports */
	pc_setup_nvram (pc, ini);    /* NVRAM/backup battery */
	pc_setup_cpu (pc, ini);      /* Microprocessor/CPU */
	pc_setup_dma (pc, ini);      /* Direct Memory Access Controller */
	pc_setup_pic (pc, ini);      /* Programmable Interrupt Controller */
	pc_setup_pit (pc, ini);      /* Intel 8253-5 System Timer */
	pc_setup_ppi (pc, ini);      /* Intel 8255A-5 Programmable Peripheral Interface (used for PC Speaker) */
	pc_setup_kbd (pc, ini);      /* Keyboard controller */
	pc_setup_cassette (pc, ini); /* Cassette tape interface (IBM PC Model 5150) */
	pc_setup_speaker (pc, ini);  /* PC speaker */
	pc_setup_terminal (pc, ini); /* Emulator Terminal Window */
	pc_setup_video (pc, ini);    /* Video display unit */
	if (pc->trm != NULL) {
		trm_open (pc->trm, 640, 480);
		trm_set_msg_trm (pc->trm, "term.title", "The Virtual Computer Collection");
	}
	pc_setup_disks (pc, ini);    /* Disk Drives */
	pc_setup_fdc (pc, ini);      /* Floppy Disk Controller */
	pc_setup_hdc (pc, ini);      /* Hard Disk Controller */
	pc_setup_serport (pc, ini);  /* Serial Port */
	pc_setup_parport (pc, ini);  /* Parallel Port */
	pc_setup_ems (pc, ini);      /* Expanded Memory */
	pc_setup_xms (pc, ini);      /* Extended Memory */
	pce_load_mem_ini (pc->mem, ini);
	mem_move_to_front (pc->mem, 0xfe000);
	pc_clock_reset (pc);
	/* --------------------------------------------------------------------------------------- */
	/* IBM PC (Model 5150/5160): Quote from "Technical Reference"; I/O Channel Description:    */
	/* --------------------------------------------------------------------------------------- */
	/* Signal Name:                                                                            */
	/*  - +RESET DRV (Output): Reset Drive: This line is used to reset or initialize system    */
	/*  logic upon power-up or during a low line-voltage outage. This signal is synchronized   */
	/*  to the falling edge of CLK and is active high.                                         */
	/* --------------------------------------------------------------------------------------- */
	return (pc);
}

void pc_del_xms (ibmpc_t *pc)
{
	/* Shut Down Extended Memory: */
	xms_del (pc->xms);
	pc->xms = NULL;
}

void pc_del_ems (ibmpc_t *pc)
{
	/* Shut Down Expanded Memory Boards: */
	ems_del (pc->ems);
	pc->ems = NULL;
}

void pc_del_parport (ibmpc_t *pc)
{
	unsigned i;
	/* Shut Down Parallel Port: */
	for (i = 0; i < 4; i++) {
		if (pc->parport[i] != NULL) {
			parport_del (pc->parport[i]);
		}
	}
}

void pc_del_serport (ibmpc_t *pc)
{
	unsigned i;
	/* Shut Down Serial Port: */
	for (i = 0; i < 4; i++) {
		if (pc->serport[i] != NULL) {
			ser_del (pc->serport[i]);
		}
	}
}

void pc_del (ibmpc_t *pc)
{
	/* Shut Down Machine: */
	if (pc == NULL) {
		return;
	}
	bps_free (&pc->bps);
	pc_del_xms (pc); /* Shut Down Extended Memory */
	pc_del_ems (pc); /* Shut Down Expanded Memory Boards */
	pc_del_parport (pc); /* Shut Down Parallel Port */
	pc_del_serport (pc); /* Shut Down Serial Port */
	hdc_del (pc->hdc); /* Shut Down Hard Disk Controller */
	dev_fdc_del (pc->fdc); /* Shut Down Floppy Disk Controller */
	dsks_del (pc->dsk); /* Shut Down Disk Drives */
	pce_video_del (pc->video); /* Shut Down Video Display */
	trm_del (pc->trm); /* Close Emulator Terminal Window */
	pc_speaker_free (&pc->spk); /* Shut Down PC Speaker */
	pc_cas_del (pc->cas); /* Shut Down Cassette Tape Interface (IBM PC Model 5150) */
	e8237_free (&pc->dma); /* Shut Down Intel 8237 Direct Memory Access (DMA) Controller */
	e8255_free (&pc->ppi); /* Shut Down Intel 8255 Programmable Peripheral Interface (used for PC Speaker) */
	e8253_free (&pc->pit); /* Shut Down Intel 8253 System Timer */
	e8259_free (&pc->pic); /* Shut Down Intel 8259 Programmable Interrupt Controller (PIC) */
	e86_del (pc->cpu); /* Shut Down CPU */
	nvr_del (pc->nvr); /* Shut Down NVRAM/backup battery */
	mem_del (pc->mem); /* Shut Down system memory */
	mem_del (pc->prt); /* Shut Down system memory reserved for use by ports */
	ini_sct_del (pc->cfg);
	free (pc);
}

int pc_set_serport_driver (ibmpc_t *pc, unsigned port, const char *driver)
{
	if (pc->model & PCE_IBMPS2_MODEL25)
	{
		/* IBM Personal System/2 Model 25/30 */
		if (ibmps2_get_serial_cs(pc) >= 1)
		{
			/* Chip Select signal not in effect; read/write operations allowed */
			/* Attach devices to serial port: */
			if ((port >= 4) || (pc->serport[port] == NULL)) {
				return (1);
			}
			if (ser_set_driver(pc->serport[port], driver)) {
				return (1);
			}
			return (0);
		}
		/* Chip Select signal in effect; read/write operations blocked */
	}
	else
	{
		/* Attach devices to serial port: */
		if ((port >= 4) || (pc->serport[port] == NULL)) {
			return (1);
		}
		if (ser_set_driver(pc->serport[port], driver)) {
			return (1);
		}
		return (0);
	}
}

int pc_set_serport_file (ibmpc_t *pc, unsigned port, const char *fname)
{
	int  r;
	char *driver;
	/* Output serial port to file: */
	driver = str_cat_alloc ("stdio:file=", fname);
	r = pc_set_serport_driver (pc, port, driver);
	free (driver);
	return (r);
}

int pc_set_parport_driver (ibmpc_t *pc, unsigned port, const char *driver)
{
	/* Attach devices to parralel port: */
	if (pc->model & PCE_IBMPS2_MODEL25)
	{
		/* IBM Personal System/2 Model 25/30 */
		if (ibmps2_get_serial_cs(pc) >= 1)
		{
			/* Chip Select signal not in effect; read/write operations allowed */
			/* Attach devices to serial port: */
			if ((port >= 4) || (pc->parport[port] == NULL)) {
				return (1);
			}
			if (parport_set_driver(pc->parport[port], driver)) {
				return (1);
			}
			return (0);
		}
		/* Chip Select signal in effect; read/write operations blocked */
	}
	else
	{
		/* Attach devices to serial port: */
		if ((port >= 4) || (pc->parport[port] == NULL)) {
			return (1);
		}
		if (parport_set_driver(pc->parport[port], driver)) {
			return (1);
		}
		return (0);
	}
}

int pc_set_parport_file (ibmpc_t *pc, unsigned port, const char *fname)
{
	int  r;
	char *driver;
	if (pc->model & PCE_IBMPS2_MODEL25)
	{
		/* IBM Personal System/2 Model 25/30 */
		if (ibmps2_get_parallel_port_output_enable(pc) >= 1)
		{
			/* Chip Select signal not in effect; read/write operations allowed */
			/* Output parallel port to file: */
			driver = str_cat_alloc("stdio:file=", fname);
			r = pc_set_parport_driver(pc, port, driver);
			free(driver);
			return (r);
		}
		/* Chip Select signal in effect; read/write operations blocked */
	}
	else
	{
		/* Output parallel port to file: */
		driver = str_cat_alloc("stdio:file=", fname);
		r = pc_set_parport_driver(pc, port, driver);
		free(driver);
		return (r);
	}
}

const char *pc_intlog_get (ibmpc_t *pc, unsigned n)
{
	return (par_intlog[n & 0xff]);
}

void pc_intlog_set (ibmpc_t *pc, unsigned n, const char *expr)
{
	char **str;
	str = &par_intlog[n & 0xff];
	free (*str);
	if ((expr == NULL) || (*expr == 0)) {
		*str = NULL;
		return;
	}
	*str = str_copy_alloc (expr);
}

int pc_intlog_check (ibmpc_t *pc, unsigned n)
{
	unsigned long val;
	const char    *str;
	cmd_t         cmd;
	str = par_intlog[n & 0xff];
	if (str == NULL) {
		return (0);
	}
	cmd_set_str (&cmd, str);
	if (cmd_match_uint32 (&cmd, &val)) {
		if (val) {
			return (1);
		}
	}
	return (0);
}

/*
 * Get the segment address of the PCE ROM extension
 */
unsigned pc_get_pcex_seg (ibmpc_t *pc)
{
	unsigned i;
	unsigned seg;
	unsigned v1, v2;
	for (i = 0; i < 32; i++) {
		seg = ((i < 16) ? 0xf000 : 0xc000) + (i & 15) * 0x0100;
		v1 = e86_get_mem16 (pc->cpu, seg, 0x0004);
		v2 = e86_get_mem16 (pc->cpu, seg, 0x0006);
		if ((v1 == 0x4350) && (v2 == 0x5845)) {
			return (seg);
		}
	}
	return (0);
}

void pc_patch_bios (ibmpc_t *pc)
{
	unsigned seg;
	if (pc->patch_bios_init == 0) {
		return;
	}
	if (e86_get_mem8 (pc->cpu, 0xf000, 0xfff0) != 0xea) {
		return;
	}
	if (e86_get_mem16 (pc->cpu, 0xf000, 0xfff3) != 0xf000) {
		return;
	}
	seg = pc_get_pcex_seg (pc);
	if (seg == 0) {
		return;
	}
	pc_log_deb ("Modifying BIOS ROM assembly routine at (0x%04x)\n", seg);
	mem_set_uint8_rw (pc->mem, 0xffff1, 0x0c);
	mem_set_uint8_rw (pc->mem, 0xffff2, 0x00);
	mem_set_uint8_rw (pc->mem, 0xffff3, seg & 0xff);
	mem_set_uint8_rw (pc->mem, 0xffff4, (seg >> 8) & 0xff);
}

void pc_reset (ibmpc_t *pc)
{
	unsigned i;
	/* --------------------------------------------------------------------------------------- */
	/* IBM PC (Model 5150/5160): Quote from "Technical Reference"; I/O Channel Description:    */
	/* --------------------------------------------------------------------------------------- */
	/* Signal Name:                                                                            */
	/*  - +RESET DRV (Output): Reset Drive: This line is used to reset or initialize system    */
	/*  logic upon power-up or during a low line-voltage outage. This signal is synchronized   */
	/*  to the falling edge of CLK and is active high.                                         */
	/* --------------------------------------------------------------------------------------- */
	/* ------------------------------------------------------------------------------------------ */
	/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; I/O Channel; Signal   */
	/* Description:                                                                               */
	/* ------------------------------------------------------------------------------------------ */
	/* The following is a description of the I/O channel signal lines. All lines are TTL-         */
	/* compatible. The (O), (I), or (I/O) notation refers to output, input, or input and output.  */
	/*  - RESET DRV (O): Reset Drive: This line is used to reset or initialize systtem logic upon */
	/*  power-up or during a low-line voltage. This signal is synchronized to the falling edge of */
	/*  CLK.                                                                                      */
	/* ------------------------------------------------------------------------------------------ */
	pc_log_deb ("reset pc\n");
	pc_patch_bios (pc);
	/* Reset CPU: */
	e86_reset (pc->cpu);
	/* Reset Intel 8237 Direct Memory Access (DMA) controller: */
	e8237_reset (&pc->dma);
	/* Reset Intel 8253 System Timer: */
	e8253_reset (&pc->pit);
	/* Reset Intel 8259 Programmable Interrupt Controller (PIC): */
	e8259_reset (&pc->pic);
	/* Reset Keyboard Controller: */
	pc_kbd_reset (&pc->kbd);
	for (i = 0; i < 4; i++) {
		if (pc->serport[i] != NULL) {
			/* Reset Serial Port: */
			ser_reset (pc->serport[i]);
		}
	}
	if (pc->fdc != NULL) {
		/* Reset Floppy Disk Controller: */
		dev_fdc_reset (pc->fdc);
	}
	if (pc->hdc != NULL) {
        /* Reset Hard Disk Controller: */
		hdc_reset (pc->hdc);
	}
	if (pc->xms != NULL) {
		/* Reset Extended Memory: */
		xms_reset (pc->xms);
	}
	if (pc->ems != NULL) {
		/* Reset Expanded Memory Board: */
		ems_reset (pc->ems);
	}
}

/*
 * Get the 1.19 MHz system clock
 */
unsigned long pc_get_clock2 (ibmpc_t *pc)
{
    /* --------------------------------------------------------------- */
    /* 1. IBM Personal Computer (Model 5150/5160):                     */
    /* --------------------------------------------------------------- */
    /* Quote from "Technical Reference"; Microprocessor:               */
    /* --------------------------------------------------------------- */
    /* The heart of the system board is the Intel 8088 Microprocessor. */
    /* This is an 8-bit external-bus version of Intel's 16-bit 8086    */
    /* Microprocessor, and is software-compatible with the 8086. Thus, */
    /* the 8088 supports 16-bit operations, including multiply and     */
    /* divide, and supports 20 bits of addressing (1 megabyte of       */
    /* storage). It also operates in maximum mode, so a                */
    /* comicroprocessor can be added as a feature. The microprocessor  */
    /* operates at 4.77-Mhz. This frequency, which is derived from a   */
    /* 14.31818-MHz crystal, is divided by 3 for the microprocessor    */
    /* clock, and by 4 to obtain the 3.58 MHz color burst signal       */
    /* required for color televisions.                                 */
    /*                                                                 */
    /* At the 4.77 MHz clock rate, the 8088 bus cycles are four clocks */
    /* of 210-ns, or 840-ns. I/O cycles take five 210-ns clocks or     */
    /* 1.04-us.                                                        */
    /* --------------------------------------------------------------- */
	return (pc->clock2);
}


void pc_clock_reset (ibmpc_t *pc)
{
	if (pc->model & PCE_IBMPS2_MODEL25) {
		/* IBM Personal System/2 Model 25/30 */
		unsigned i;
		/* -------------------------------------------------------------------------------------- */
		/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; System Timer:     */
		/* -------------------------------------------------------------------------------------- */
		/* The system timer is an 8253 programmable interval timer/counter, or equivalent, that   */
		/* functions as an arrangement of four external I/O ports (hex 0040 through 0043). It     */
		/* receives its 1.19 MHz clock from the I/O support gate array. These ports are treated as*/
		/* counters; the fourth, address hex 0043, is a control register for mode programming.    */
		/*                                                                                        */
		/* The content of a selected counter may be latched without disturbing the counting       */
		/* operation by writing to the control register. If the content of the counter is critical*/
		/* to a program's operation, a second read is recommended for verification.               */
		/* -------------------________________--------------------------------------------------- */
		/* System Bus:------>:|System Timer: |                                                    */
		/*          __________| - Gate 0     |                                                    */
		/*          |  _______| - Clock  in 0|                                                    */
		/* +5 Vdc __|__|______| - Gate 1     |                                                    */
		/* -RAS_SIP ---|---___| - Clock  in 1|                                                    */
		/* I/O Port  __|______| - Gate 2     |                                                    */
		/* Hex 0061__| |______| - Clock  in 2|                                                    */
		/* Port Bit 0  |      | - Clock Out 0|_______________IRQ 0________                        */
		/* 1.19 Mhz____|      | - Clock Out 1|    |_Driver_|                                      */
		/*                    | - Clock Out 2|______|__ _|_______                                 */
		/* I/O Port           ----------------  | AND | | Low   |--> To Beeper                    */
		/* Hex 0061 ----------------------------|-----| | Pass  |                                 */
		/* Port Bit 1                                   | Filter|                                 */
		/* ---------------------------------------------|-------|-------------------------------- */
		/* Figure 1-4. System Timer Block Diagram                                                 */
		/* -------------------------------------------------------------------------------------- */
		/* The three timers are programmable and are used by the system as follows:               */
		/*                                                                                        */
		/* Channel 0 is a general-purpose timer providing a constant time base.                   */
		/* Channel 0:   System Timer:                                                             */
		/*  - GATE    0: - Tied High                                                              */
		/*  - CLK IN  0: - 1.19 MHz OSC                                                           */
		/*  - CLK OUT 0: - IRQ 0                                                                  */
		/* Channel 1 is for internal diagnostic tests.                                            */
		/* Channel 1:   Diagnostic:                                                               */
		/*  - GATE    1: - Tied High                                                              */
		/*  - CLK IN  1: - -RAS_SIP from system support gate array                                */
		/*  - CLK OUT 2: - Not connected                                                          */
		/* Channel 2 supports the tone generation for the audio.                                  */
		/* Channel 2:   Tone Generation:                                                          */
		/*  - GATE    2: Controlled by bit 0 at port hex 61                                       */
		/*  - CLK IN  2: 1.19 MHz OSC                                                             */
		/*  - CLK OUT 2: To the beeper data of the I/O support gate array                         */
		/* -------------------------------------------------------------------------------------- */
		/* -------------------------------------------------------------------------------------- */
		/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; I/O Channel;      */
		/* Signal Description:                                                                    */
		/* -------------------------------------------------------------------------------------- */
		/* The following is a description of the I/O channel signal lines. All lines are TTL-     */
		/* compatible. The (O), (I), or (I/O) notation refers to output, input, or input and      */
		/* output.                                                                                */
		/*  - RESET DRV (O): Reset Drive: This line is used to reset or initialize systtem logic  */
		/*  upon power-up or during a low-line voltage. This signal is synchronized to the falling*/
		/*  edge of CLK.                                                                          */
		/* -------------------------------------------------------------------------------------- */
		for (i = 0; i < 8; i++) {
			pc->ps2model25_clk_div[i] = 0;
		}
		pc->sync_clock2_sim = 0;
		pc->sync_clock2_real = 0;
		pce_get_interval_us(&pc->sync_interval);
		pc->speed_clock_extra = 0;
		pc->clock1 = 0;
		pc->clock2 = 0;
		/* -------------------------------------------------------------------------------------- */
		/* IBM PS/2 (Model 25/30): Quote from "Technical Reference"; System Clock Generator:      */
		/* -------------------------------------------------------------------------------------- */
		/* The system clock generator uses a 48 MHz input that is internally divided to give the  */
		/* output clock signal of 8 MHz with a 33% duty cycle. It also generates a 1.84 MHz signal*/
		/* for the serial port.                                                                   */
		/*                                                                                        */
		/* The clock generator generates the 'reset' signal after sensing the 'power good' signal */
		/* from the power supply.                                                                 */
		/* -------------------------------------------------------------------------------------- */
	}
	else {
		/* IBM Personal Computer (Model 5150/5160) and IBM Portable PC (Model 5155) */
		unsigned i;
		/* -------------------------------------------------------------------------------------- */
		/* IBM PC (Model 5150/5160): Quote from "Technical Reference"; I/O Channel Description:   */
		/* -------------------------------------------------------------------------------------- */
		/* Signal Name:                                                                           */
		/*  - +RESET DRV (Output): Reset Drive: This line is used to reset or initialize system   */
		/*  logic upon power-up or during a low line-voltage outage. This signal is synchronized  */
		/*  to the falling edge of CLK and is active high.                                        */
		/* -------------------------------------------------------------------------------------- */
		for (i = 0; i < 4; i++) {
			pc->clk_div[i] = 0;
		}
		pc->sync_clock2_sim = 0;
		pc->sync_clock2_real = 0;
		pce_get_interval_us(&pc->sync_interval);
		pc->speed_clock_extra = 0;
		pc->clock1 = 0;
		pc->clock2 = 0;
	}
}

void pc_clock_discontinuity (ibmpc_t *pc)
{
	pc->sync_clock2_real = pc->sync_clock2_sim;
	pce_get_interval_us (&pc->sync_interval);
	pc->speed_clock_extra = 0;
}

/*
 * Synchronize the system clock with real time
 */
static
void pc_clock_delay (ibmpc_t *pc)
{
	if (pc->model & PCE_IBMPS2_MODEL25)
	{
		/* IBM Personal System/2 Model 25/30 */
		unsigned long vclk;
		unsigned long rclk;
		unsigned long us;
		vclk = pc->sync_clock2_sim;
		rclk = pce_get_interval_us(&pc->sync_interval);
		rclk = (PCE_IBMPS2_MODEL25_CLK2 * (unsigned long long) rclk) / 1000000;
		rclk += pc->sync_clock2_real;
		if (vclk < rclk) {
			pc->sync_clock2_sim = 0;
			pc->sync_clock2_real = rclk - vclk;
			if (pc->speed_clock_extra > 0) {
				pc->speed_clock_extra -= 1;
			}
			if (pc->sync_clock2_real > PCE_IBMPS2_MODEL25_CLK2) {
				pc->sync_clock2_real = 0;
				pce_log(MSG_INF, "NOTE: One second skipped.\n");
			}
			return;
		}
		else {
			pc->speed_clock_extra += 1;
		}
		vclk -= rclk;
		pc->sync_clock2_sim = vclk;
		pc->sync_clock2_real = 0;
		us = (1000000 * (unsigned long long) vclk) / PCE_IBMPS2_MODEL25_CLK2;
		if (us > PCE_IBMPC_SLEEP) {
			pce_usleep(us);
		}
	}
	else
	{
		unsigned long vclk;
		unsigned long rclk;
		unsigned long us;
		vclk = pc->sync_clock2_sim;
		rclk = pce_get_interval_us(&pc->sync_interval);
		rclk = (PCE_IBMPC_CLK2 * (unsigned long long) rclk) / 1000000;
		rclk += pc->sync_clock2_real;
		if (vclk < rclk) {
			pc->sync_clock2_sim = 0;
			pc->sync_clock2_real = rclk - vclk;
			if (pc->speed_clock_extra > 0) {
				pc->speed_clock_extra -= 1;
			}
			if (pc->sync_clock2_real > PCE_IBMPC_CLK2) {
				pc->sync_clock2_real = 0;
				pce_log(MSG_INF, "NOTE: One second skipped.\n");
			}
			return;
		}
		else {
			pc->speed_clock_extra += 1;
		}
		vclk -= rclk;
		pc->sync_clock2_sim = vclk;
		pc->sync_clock2_real = 0;
		us = (1000000 * (unsigned long long) vclk) / PCE_IBMPC_CLK2;
		if (us > PCE_IBMPC_SLEEP) {
			pce_usleep(us);
		}
	}
}

void pc_clock (ibmpc_t *pc, unsigned long cnt)
{
	unsigned      i;
	unsigned long spd;
	unsigned long clk;
	/* */
	if (cnt == 0) {
		if (pc->model & PCE_IBMPS2_MODEL25) {
			cnt = 8;
		}
		else {
			cnt = 4;
		}
		/* cnt = 4; */
	}
	if (pc->speed_current == 0) {
		e86_clock(pc->cpu, cnt + pc->speed_clock_extra);
		spd = 4;
	}
	else {
		e86_clock(pc->cpu, cnt);
		spd = 4 * pc->speed_current;
	}
	if (pc->model & PCE_IBMPS2_MODEL25) {
		spd + 4;
	}
	pc->clock1 += cnt;
	if (pc->clock1 < spd) {
		return;
	}
	pc->clock1 -= spd;
	pc->sync_clock2_sim += 1;
	pc->clock2 += 1;
	pce_video_clock0(pc->video, 1, 1);
	e8253_clock(&pc->pit, 1);
	pc->clk_div[0] += 1;
	if (pc->clk_div[0] >= 8) {
		clk = pc->clk_div[0] & ~7UL;
		pc->clk_div[1] += clk;
		pc->clk_div[0] &= 7;
		pc_kbd_clock(&pc->kbd, clk);
		if (pc->cas != NULL) {
			pc_cas_clock(pc->cas, clk);
		}
		e8237_clock(&pc->dma, clk);
		pce_video_clock1(pc->video, 0);
		for (i = 0; i < 4; i++) {
			if (pc->serport[i] != NULL) {
				e8250_clock(&pc->serport[i]->uart, clk);
			}
		}
		if (pc->clk_div[1] >= 1024) {
			clk = pc->clk_div[1] & ~1023UL;
			pc->clk_div[1] &= 1023;
			pc->clk_div[2] += clk;
			if (pc->trm != NULL) {
				trm_check(pc->trm);
			}
			if (pc->fdc != NULL) {
				e8272_clock(&pc->fdc->e8272, clk);
			}
			if (pc->hdc != NULL) {
				hdc_clock(pc->hdc, clk);
			}
			pc_speaker_clock(&pc->spk, clk);
			for (i = 0; i < 4; i++) {
				if (pc->serport[i] != NULL) {
					ser_clock(pc->serport[i], clk);
				}
			}
			if (pc->clk_div[2] >= 16384) {
				pc->clk_div[2] &= 16383;
				pc_clock_delay(pc);
			}
		}
	}
	/* 	if (pc->model & PCE_IBMPS2_MODEL25) {
		/* IBM Personal System/2 Model 25/30 */
/* 		if (cnt == 0) {
		pc->ps2model25_clk_div[0] += 1;
		if (pc->ps2model25_clk_div[0] >= 8) {
			clk = pc->ps2model25_clk_div[0] & ~7UL;
			pc->ps2model25_clk_div[1] += clk;
			pc->ps2model25_clk_div[0] &= 7;
			/* Set Keyboard Controller clock: */
/* 			pc_kbd_clock(&pc->kbd, clk);
			if (pc->cas != NULL) {
				/* Set cassette tape interface clock (IBM PC Model 5150): */
/* 				pc_cas_clock(pc->cas, clk);
			}
			/* Set Intel 8237 clock: */
/* 			e8237_clock(&pc->dma, clk);
			/* Set video display clock: */
/* 			pce_video_clock1(pc->video, 0);
			for (i = 0; i < 4; i++) {
				if (pc->serport[i] != NULL) {
					/* Set Intel 8250 serial port controller: */
/* 					e8250_clock(&pc->serport[i]->uart, clk);
				}
			}
			if (pc->ps2model25_clk_div[1] >= 1024) {
				clk = pc->ps2model25_clk_div[1] & ~1023UL;
				pc->ps2model25_clk_div[1] &= 1023;
				pc->ps2model25_clk_div[2] += clk;
				if (pc->trm != NULL) {
					trm_check(pc->trm);
				}
				if (pc->fdc != NULL) {
					/* Set Intel 8272 Floppy Disk Controller clock: */
/* 					e8272_clock(&pc->fdc->e8272, clk);
				}
				if (pc->hdc != NULL) {
					/* Set Hard Disk Controller clock: */
/* 					hdc_clock(pc->hdc, clk);
				}
				/* Set PC Speaker clock: */
/* 				pc_speaker_clock(&pc->spk, clk);
				for (i = 0; i < 4; i++) {
					if (pc->serport[i] != NULL) {
						/* Set serial port controller clock: */
/* 						ser_clock(pc->serport[i], clk);
					}
				}
				if (pc->ps2model25_clk_div[2] >= 16384) {
					pc->ps2model25_clk_div[2] &= 16383;
					pc_clock_delay(pc);
				}
			}
		}
		/* -------------------------------------------------------------------------------------- */
		/* IBM PS/2 (Model 25/30): Quote from "Technical Reference"; System Clock Generator:      */
		/* -------------------------------------------------------------------------------------- */
		/* The system clock generator uses a 48 MHz input that is internally divided to give the  */
		/* output clock signal of 8 MHz with a 33% duty cycle. It also generates a 1.84 MHz signal*/
		/* for the serial port.                                                                   */
		/*                                                                                        */
		/* The clock generator generates the 'reset' signal after sensing the 'power good' signal */
		/* from the power supply.                                                                 */
		/* -------------------------------------------------------------------------------------- */
		/*
		cnt = 8; /* 8 Mhz CPU clock */
/* 		}
		if (pc->speed_current == 0) {
			e86_clock(pc->cpu, cnt + pc->speed_clock_extra);
			spd = 8; /* 8 Mhz CPU clock */
/* 		}
		else {
			e86_clock(pc->cpu, cnt);
			spd = 8 * pc->speed_current; /* 8 Mhz CPU clock */
/* 		}
		pc->clock1 += cnt;
		if (pc->clock1 < spd) {
			return;
		}
		pc->clock1 -= spd;
		pc->sync_clock2_sim += 1;
		pc->clock2 += 1;
		pce_video_clock0(pc->video, 1, 1);
		/* ------------------------------------------------------------------------------------------------------- */
		/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; System Timer:                      */
		/* ------------------------------------------------------------------------------------------------------- */
		/* The system timer is an 8253 programmable interval timer/counter, or equivalent, that functions as an    */
		/* arrangement of four external I/O ports (hex 0040 through 0043). It receives its 1.19 MHz clock from the */
		/* I/O support gate array. These ports are treated as counters; the fourth, address hex 0043, is a control */
		/* register for mode programming.                                                                          */
		/*                                                                                                         */
		/* The content of a selected counter may be latched without disturbing the counting operation by writing to*/
		/* the control register. If the content of the counter is critical to a program's operation, a second read */
		/* is recommended for verification.                                                                        */
		/* -------------------________________-------------------------------------------------------------------- */
		/* System Bus:------>:|System Timer: |                                                                     */
		/*          __________| - Gate 0     |                                                                     */
		/*          |  _______| - Clock  in 0|                                                                     */
		/* +5 Vdc __|__|______| - Gate 1     |                                                                     */
		/* -RAS_SIP ---|---___| - Clock  in 1|                                                                     */
		/* I/O Port  __|______| - Gate 2     |                                                                     */
		/* Hex 0061__| |______| - Clock  in 2|                                                                     */
		/* Port Bit 0  |      | - Clock Out 0|_______________IRQ 0________                                         */
		/* 1.19 Mhz____|      | - Clock Out 1|    |_Driver_|                                                       */
		/*                    | - Clock Out 2|______|__ _|_______                                                  */
		/* I/O Port           ----------------  | AND | | Low   |--> To Beeper                                     */
		/* Hex 0061 ----------------------------|-----| | Pass  |                                                  */
		/* Port Bit 1                                   | Filter|                                                  */
		/* ---------------------------------------------|-------|------------------------------------------------- */
		/* Figure 1-4. System Timer Block Diagram                                                                  */
		/* ------------------------------------------------------------------------------------------------------- */
		/* The three timers are programmable and are used by the system as follows:                                */
		/*                                                                                                         */
		/* Channel 0 is a general-purpose timer providing a constant time base.                                    */
		/* Channel 0:   System Timer:                                                                              */
		/*  - GATE    0: - Tied High                                                                               */
		/*  - CLK IN  0: - 1.19 MHz OSC                                                                            */
		/*  - CLK OUT 0: - IRQ 0                                                                                   */
		/* Channel 1 is for internal diagnostic tests.                                                             */
		/* Channel 1:   Diagnostic:                                                                                */
		/*  - GATE    1: - Tied High                                                                               */
		/*  - CLK IN  1: - -RAS_SIP from system support gate array                                                 */
		/*  - CLK OUT 2: - Not connected                                                                           */
		/* Channel 2 supports the tone generation for the audio.                                                   */
		/* Channel 2:   Tone Generation:                                                                           */
		/*  - GATE    2: Controlled by bit 0 at port hex 61                                                        */
		/*  - CLK IN  2: 1.19 MHz OSC                                                                              */
		/*  - CLK OUT 2: To the beeper data of the I/O support gate array                                          */
		/* ------------------------------------------------------------------------------------------------------- */
		/* Set Intel 8253 System Timer clock: */
/* 		e8253_clock(&pc->pit, 1);
		pc->ps2model25_clk_div[0] += 1;
		if (pc->ps2model25_clk_div[0] >= 8) {
			clk = pc->ps2model25_clk_div[0] & ~7UL;
			pc->ps2model25_clk_div[1] += clk;
			pc->ps2model25_clk_div[0] &= 7;
			/* Set Keyboard Controller clock: */
/* 			pc_kbd_clock(&pc->kbd, clk);
			if (pc->cas != NULL) {
				/* Set cassette tape interface clock (IBM PC Model 5150): */
/* 				pc_cas_clock(pc->cas, clk);
			}
			/* Set Intel 8237 clock: */
/* 			e8237_clock(&pc->dma, clk);
			/* Set video display clock: */
/* 			pce_video_clock1(pc->video, 0);
			for (i = 0; i < 4; i++) {
				if (pc->serport[i] != NULL) {
					/* Set Intel 8250 serial port controller: */
/* 					e8250_clock(&pc->serport[i]->uart, clk);
				}
			}
			if (pc->ps2model25_clk_div[1] >= 1024) {
				clk = pc->ps2model25_clk_div[1] & ~1023UL;
				pc->ps2model25_clk_div[1] &= 1023;
				pc->ps2model25_clk_div[2] += clk;
				if (pc->trm != NULL) {
					trm_check(pc->trm);
				}
				if (pc->fdc != NULL) {
					/* Set Intel 8272 Floppy Disk Controller clock: */
/* 					e8272_clock(&pc->fdc->e8272, clk);
				}
				if (pc->hdc != NULL) {
					/* Set Hard Disk Controller clock: */
/* 					hdc_clock(pc->hdc, clk);
				}
				/* Set PC Speaker clock: */
/* 				pc_speaker_clock(&pc->spk, clk);
				for (i = 0; i < 4; i++) {
					if (pc->serport[i] != NULL) {
						/* Set serial port controller clock: */
/* 						ser_clock(pc->serport[i], clk);
					}
				}
				if (pc->ps2model25_clk_div[2] >= 16384) {
					pc->ps2model25_clk_div[2] &= 16383;
					pc_clock_delay(pc);
				}
			}
		}
	}
	else {
		/* IBM Personal Computer (Model 5150/5160) and IBM Portable PC (Model 5155) */
		/* NOTE for IBM PC machines with Intel 8088 CPU: See information below on Intel 8088 CPU clock. */
/* 		if (cnt == 0) {
			/* --------------------------------------------------------------------------------------- */
			/* IBM PC (Model 5150/5160): Quote from "Technical Reference"; I/O Channel Description:    */
			/* --------------------------------------------------------------------------------------- */
			/* Signal Name:                                                                            */
			/*  - CLK (Output): System clock: It is a divide-by-three of the oscillator and has a      */
			/*  210-ns (4.77-MHz) The clock has a 33% duty cycle.                                      */
			/* --------------------------------------------------------------------------------------- */
/* 			cnt = 4; /* 4 Mhz CPU clock */
/* 		}
		if (pc->speed_current == 0) {
			e86_clock(pc->cpu, cnt + pc->speed_clock_extra);
			spd = 4; /* 4 Mhz CPU clock */
/* 		}
		else {
			e86_clock(pc->cpu, cnt);
			spd = 4 * pc->speed_current; /* 4 Mhz CPU clock */
/* 		}
		pc->clock1 += cnt;
		if (pc->clock1 < spd) {
			return;
		}
		pc->clock1 -= spd;
		pc->sync_clock2_sim += 1;
		pc->clock2 += 1;
		pce_video_clock0(pc->video, 1, 1);
		/* ---------------------------------------------------------------------------- */
		/* IBM PC (Model 5150): Quote from "Technical Reference"; System Timers:        */
		/* ---------------------------------------------------------------------------- */
		/* Three programmable timer/counters are used by the system as follows: Channel */
		/* 0 is a general-purpose timer providing a constant time base for implementing */
		/* a time-of-day clock, Channel 1 times and requests refresh cycles from the    */
		/* the Direct Memory Access (DMA) channel, and Channel 2 supports the tone      */
		/* generation for the speaker. Each channel has a minimum timing resolution of  */
		/* 1.05-us.                                                                     */
		/* ---------------------------------------------------------------------------- */
		/* IBM PC (Model 5160): Quote from "Technical Reference"; System Timers:        */
		/* ---------------------------------------------------------------------------- */
		/* Three programmable timer/counters are used by the system as follows: Channel */
		/* 0 is used as a general-purpose timer providing a constant time base for      */
		/* implementing a time-of-day clock; Channel 1 is used to time and request      */
		/* refresh cycles from the DMA channel; and Channel 2 is used to support the    */
		/* tone generation for the audio speaker. Each channel has a minimum timing     */
		/* resolution of 1.05-us.                                                       */
		/* ---------------------------------------------------------------------------- */
		/* Set Intel 8253 System Timer clock: */
/* 		e8253_clock(&pc->pit, 1);
		pc->clk_div[0] += 1;
		if (pc->clk_div[0] >= 8) {
			clk = pc->clk_div[0] & ~7UL;
			pc->clk_div[1] += clk;
			pc->clk_div[0] &= 7;
			/* Set Keyboard Controller clock: */
/* 			pc_kbd_clock(&pc->kbd, clk);
			if (pc->cas != NULL) {
				/* Set cassette tape interface clock (IBM PC Model 5150): */
/* 				pc_cas_clock(pc->cas, clk);
			}
			/* Set Intel 8237 clock: */
/* 			e8237_clock(&pc->dma, clk);
			/* Set video display clock: */
/* 			pce_video_clock1(pc->video, 0);
			for (i = 0; i < 4; i++) {
				if (pc->serport[i] != NULL) {
					/* Set Intel 8250 serial port controller: */
/* 					e8250_clock(&pc->serport[i]->uart, clk);
				}
			}
			if (pc->clk_div[1] >= 1024) {
				clk = pc->clk_div[1] & ~1023UL;
				pc->clk_div[1] &= 1023;
				pc->clk_div[2] += clk;
				if (pc->trm != NULL) {
					trm_check(pc->trm);
				}
				if (pc->fdc != NULL) {
					/* Set Intel 8272 Floppy Disk Controller clock: */
/* 					e8272_clock(&pc->fdc->e8272, clk);
				}
				if (pc->hdc != NULL) {
					/* Set Hard Disk Controller clock: */
/* 					hdc_clock(pc->hdc, clk);
				}
				/* Set PC Speaker clock: */
/* 				pc_speaker_clock(&pc->spk, clk);
				for (i = 0; i < 4; i++) {
					if (pc->serport[i] != NULL) {
						/* Set serial port controller clock: */
/* 						ser_clock(pc->serport[i], clk);
					}
				}
				if (pc->clk_div[2] >= 16384) {
					pc->clk_div[2] &= 16383;
					pc_clock_delay(pc);
				}
			}
		}
	} */
}

int pc_set_cpu_model (ibmpc_t *pc, const char *str)
{
	if (strcmp (str, "8086") == 0) {
		e86_set_8086 (pc->cpu);
		/* -------------------------------------------------------------------------------------- */
		/* 1. IBM Personal System/2 Model 25/30:                                                  */
		/* -------------------------------------------------------------------------------------- */
		/* Quote from "Technical Reference"; Microprocessor:                                      */
		/* -------------------------------------------------------------------------------------- */
		/* The Intel 8086-2 is a 16-bit microprocessor with a 16-bit external data bus, operating */
		/* at 8 MHz. The microprocessor supports the same 20-bit addressing as IBM Personal       */
		/* Computers that use the 8088 microprocessor. The Type 8525 uses a 16-bit data bus with  */
		/* the system's read-only and read/write memory, and an 8-bit data bus for all I/O and    */
		/* direct memory access (DMA) operations.                                                 */
		/*                                                                                        */
		/* The memory read and write are 16-bit operations and take four clock cycles of 125 ns,  */
		/* with no wait states, for a cycle time of 500 ns. Normal I/O operations are 8-bit       */
		/* operations and take eight clock cycles, including four wait states, for a cycle time of*/
		/*1 us. A signal on the I/O channel, I/O channel ready (I/O CH RDY), allows slower devices*/
		/* to add more wait states to I/O and DMA operations (see "I/O Channel" later in this     */
		/* section).                                                                              */
		/*                                                                                        */
		/* Logic has been added to the system board to support options for the IBM Personal       */
		/* Computer family. This includes converting 16-bit operations to sequential 8-bit        */
		/* operations, inserting wait states into all I/O and DMA operations, and delaying        */
		/* microprocessor cycles to ensure address setup times greater than or equal to the 8088- */
		/* based systems.                                                                         */
		/*                                                                                        */
		/* The 8086-2 supports 16-bit operations, including multiply and divide, and 20-bit       */
		/* addressing to access 1M (M = 1,048,576) of address space. It also operates in maximum  */
		/* mode, so a math coprocessor can be added as a feature. Memory is mapped as follows:    */
		/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
		/* Hex Address:    Function:                                                              */
		/*  - 00000 - 9FFFF - 640K Read/Write Memory                                              */
		/*  - A0000 - BFFFF - Video Buffer                                                        */
		/*  - C0000 - EFFFF - Reserved for BIOS on I/O Channel                                    */
		/*  - F0000 - FFFFF - System ROM                                                          */
		/* Figure 1-2. Memory Map                                                                 */
		/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
		/* The microprocessor is supported by two high-function support devices: a system support */
		/* gate array and an I/O support gate array.                                              */
		/* -------------------------------------------------------------------------------------- */
	}
	else if (strcmp (str, "8088") == 0) {
		e86_set_8088 (pc->cpu);
		/* -------------------------------------------------------------------------------------- */
		/* 1. IBM Personal Computer (Model 5150/5160):                                            */
		/* -------------------------------------------------------------------------------------- */
		/* Quote from "Technical Reference"; Microprocessor:                                      */
		/* -------------------------------------------------------------------------------------- */
		/* The heart of the system board is the Intel 8088 Microprocessor. This is an 8-bit       */
		/* external bus version of Intel's 16-bit 8086 Microprocessor, and is software-compatible */
		/*with the 8086. Thus, the 8088 supports 16-bit operations, including multiply and divide,*/
		/* and supports 20 bits of addressing (1 megabyte of storage). It also operates in maximum*/
		/* mode, so a comicroprocessor can be added as a comicroprocessor can be added as a       */
		/* feature. The microprocessor operates at 4.77 Mhz. This frequency, which is derived from*/
		/*a 14.31818-MHz crystal, is divided by 3 for the microprocessor clock, and by 4 to obtain*/
		/* the 3.58 MHz color burst signal required for color televisions.                        */
        /*                                                                                        */
        /* At the 4.77 MHz clock rate, the 8088 bus cycles are four clocks of 210-ns, or 840-ns.  */
		/* I/O cycles take five 210-ns clocks or 1:04-us.                                         */
        /* -------------------------------------------------------------------------------------- */
    	/* Quote from "Technical Reference"; I/O Channel:                                         */
    	/* -------------------------------------------------------------------------------------- */
		/* The I/O channel is an extension of the 8088 microprocessor bus. It is, however,        */
		/* demultiplexed, repowered, and enhanced by the addition of interrupts and direct memory */
		/* access (DMA) functions.                                                                */
		/*                                                                                        */
		/* The I/O channel contains an 8-bit bidirectional data bus, 20 address lines, 6 levels of*/
		/* interrupt, control lines for memory and I/O read or write, clock and timing lines, 3   */
		/* channels of DMA control lines, memory refresh timing control lines, a channel-check    */
		/* line, and power and ground for the adapters. Four voltage levels are provided for I/O  */
		/* cards: +5 Vdc, -5 Vdc, +12 Vdc, and -12 Vdc. These functions are provided in a 62-pin  */
		/* connector with 100-mil card tab spacing.                                               */
		/*                                                                                        */
		/* A 'ready' line is available on the I/O channel to allow operation with slow I/O or     */
		/*memory devices. If the channel's ready line is not activated by an addressed device, all*/
		/*microprocessor-generated memory read and write cycles take four 210-ns clocks or 840-ns/*/
		/* byte. All microprocessor-generated I/O read and write cycles require five clocks for a */
		/* cycle time of 1.05-us/byte. All DMA transfers require five clocks for a cycle time of  */
		/* 1.05-us/byte. Refresh cycles occur once every 72 clocks (approximately 15-us) and      */
		/* require four clocks or approximately 7% of the bus bandwidth.                          */
		/*                                                                                        */
		/* I/O devices are addressed using I/O mapped address space. The channel is designed so   */
		/*that 512 (5150) /768 (5160) I/O device addresses are available to the I/O channel cards.*/
		/*                                                                                        */
		/* A 'channel check' line exists for reporting error conditions to the microprocessor.    */
		/* Activating this line results in a non-maskable interrupt (NMI) to the 8088             */
		/* microprocessor. Memory expansion options this line to report parity errors.            */
		/*                                                                                        */
		/* The I/O channel is repowered to provide sufficient drive to power all five system unit */
		/*(5150)/all eight (J1 through J8) (5160) expansion slots, assuming two low-power Schottky*/
		/* loads per slot. The IBM I/O adapters typically use only one load.                      */
		/* ------------------------ Applies to Model 5160 Technical Reference Manual: ----------- */
		/*Timing requirements on slot J8 are much stricter than those on slots J1 through J7. Slot*/
		/* J8 also requires the card to provide a signal designating when the card is selected.   */
		/* -------------------------------------------------------------------------------------- */
    	/* -------------------------------------------------------------------------------------- */
    	/* e86_set_8087 (pc->cpu); NOTE: Please leave disabled until emulated Intel 8087 is       */
		/* available to test.                                                                     */
    	/* -------------------------------------------------------------------------------------- */
    	/* Section 2. Coprocessor:                                                                */
    	/* -------------------------------------------------------------------------------------- */
    	/* Description:                                                                           */
    	/* -------------------------------------------------------------------------------------- */
    	/* The Math Coprocessor (8087) enables the IBM Personal Computer to perform high-speed    */
		/* arthmetic, logarithmic functions, and trigonometric operations with extreme accuracy.  */
    	/*                                                                                        */
    	/* The 8087 coprocessor works in parallel with the microprocessor. The parallel operation */
		/* decreases operating time by allowing the coprocessor to do mathematical calculations   */
		/* while the microprocessor continues to do other functions.                              */
    	/*                                                                                        */
    	/* The first five bits of every instruction's operation code for the coprocessor are      */
		/*identical (binary 11011). When the microprocessor and the coprocessor see this operation*/
		/* code, the microprocessor calculates the address of any variables in memory, while the  */
		/* coprocessor checks the instruction. The coprocessor takes the memory address from the  */
		/* microprocessor if necessary. To gain access to locations in memory, the coprocessor    */
		/*takes the local bus from the microprocessor when the microprocessor finishes its current*/
		/* instruction. When the coprocessor is finished with the memory transfer, it returns the */
		/* local bus to the microprocessor.                                                       */
        /*                                                                                        */
        /* The IBM Math Coprocessor works with seven numeric data types divided into the three    */
		/* classes listed below.                                                                  */
        /* - Binary integers (3 types)                                                            */
        /* - Decimal integers (1 type)                                                            */
        /* - Real numbers (3 types)                                                               */
        /* -------------------------------------------------------------------------------------- */
        /* Programming Interface:                                                                 */
        /* -------------------------------------------------------------------------------------- */
        /* The coprocessor extends the data types, registers, and instructions to the             */
		/* microprocessor.                                                                        */
        /*                                                                                        */
        /*The coprocessor has eight 80-bit registers, which provide the equivalent capacity of the*/
		/*40 16-bit registers found in the microprocessor. The register space allows constants and*/
		/* temporary results to be held in registers during calculations, thus reducing memory    */
		/* access and improving speed as well as bus availability. The register space can be used */
		/* as a stack or as a fixed register set. When used as a stack, only the top two stack    */
		/* elements are operated on. The figure below shows representations of large and small    */
		/* numbers in each data type.                                                             */
        /* -------------------------------------------------------------------------------------- */
        /* Data Types:                                                                            */
        /* -------------------------------------------------------------------------------------- */
        /* Data Type:          Bits: Significant Digits Approximate Range                         */
		/*                                   (Decminal):    (decimal):                            */
        /*  - Word  Integer  :  - 16  -  4                - -32,768 <= X <= + 32, 767             */
        /*  - Short Integer  :  - 32  -  9                - -2x10(9)<= X <= 2x10(9)               */
        /*  - Long  Integer  :  - 64  - 18                - -9x10(18)<= X <= 9x10(18)             */
        /*  - Packed Decimal :  - 80  - 18                - -99...99<= X <= + 99...99 (18 digits) */
        /*  - Short     Real*:  - 32  -  6-7              - 8.43x10-(37)<= |X| <= 3.37x10(38)     */
        /*  - Long      Real*:  - 64  - 15-16             - 4.19x10-(307)<= |X| <= 1.67x10(308)   */
        /*  - Temporary Real :  - 80  - 19                - 3.4x10-(4932)<=|X| <= 1.2x10(4932)    */
        /* -------------------------------------------------------------------------------------- */
        /*  *The short and long real data types correspond to the single and double precision data*/
		/*  types.                                                                                */
        /* -------------------------------------------------------------------------------------- */
        /* Hardware Interface:                                                                    */
        /* -------------------------------------------------------------------------------------- */
        /*The coprocessor uses the same clock generator and system bus interface components as the*/
        /* microprocessor. The coprocessor is wired directly into the microprocessor. The         */
		/* microprocessor's queue status lines (QS0 and QS1) enable the coprocessor to obtain and */
		/* decode instructions simultaneously with the microprocessor. The coprocessor's 'busy'   */
		/* signal informs the microprocessor that it is executing; the microprocessor's WAIT      */
		/* instruction forces the microprocessor to wait until the coprocessor is finished        */
		/* executing (WAIT FOR NOT BUSY).                                                         */
        /*                                                                                        */
        /* When an incorrect instruction is sent to the coprocessor (for example, divide by 0 or  */
		/* load a full register), the coprocessor can signal the microprocessor with an interrupt.*/
		/* There are three conditions that will disable the coprocessor interrupt to the          */
		/* microprocessor:                                                                        */
        /*  - 1. Exception and interrupt-enable bits of the control word are set to 1's.          */
        /*  - 2. System-board switch-block 1, switch 2, set in the On position.                   */
        /*  - 3. Non-maskable interrupt (NMI) register (REG) is set to zero.                      */
        /* At power-on time, the NMI REG is cleared to disable the NMI. Any program using the     */
		/* coprocessor's interrupt capability must ensure that conditions 2 and 3 are never met   */
		/* during the operation of the software or an ''Endless WAIT'' will occur. And ''Endless  */
		/* WAIT'' will have the microprocessor waiting for the 'not busy' signal from the         */
		/* coprocessor while the coprocessor is waiting for the microprocessor to interrupt.      */
        /*                                                                                        */
        /* Because a memory parity error may also cause an interrupt to the microprocessor NMI    */
		/* line, the program should check the coprocessor status for an exception condition. If a */
		/*coprocessor exception condition is not found, control should be passed to the normal NMI*/
		/*handler. If an 8087 exception condition is found, the program may clear the exception by*/
		/* executing the FNSAVE or the FNCLEX instruction, and the exception can be identified and*/
		/* acted upon.                                                                            */
        /*                                                                                        */
        /* The NMI REG and the coprocessor's interrupt are tied to the NMI line through the NMI   */
		/* interrupt logic. Minor modifications to programs designed for use with a coprocessor   */
		/* must be made before the programs will be compatible with the IBM Personal Computer Math*/
		/* Coprocessor.                                                                           */
        /* -------------------------------------------------------------------------------------- */
        /* Coprocessor Interconnection:                                                           */
        /* -------------------------------------------------------------------------------------- */
        /* See bitmap file for more information.                                                  */
        /* -------------------------------------------------------------------------------------- */
        /* Detailed information for the internal functions of the Intel 8087 Coprocessor can be   */
		/* found in the books listed in the Bibliography.                                         */
        /* -------------------------------------------------------------------------------------- */
	}
	else if (strcmp (str, "80186") == 0) {
		e86_set_80186 (pc->cpu);
	}
	else if (strcmp (str, "80188") == 0) {
		e86_set_80188 (pc->cpu);
	}
	else if (strcmp (str, "v30") == 0) {
		e86_set_v30 (pc->cpu);
	}
	else if (strcmp (str, "v20") == 0) {
		e86_set_v20 (pc->cpu);
	}
	else if (strcmp (str, "80286") == 0) {
		/* e86_set_80286 (pc->cpu); */
		e86_set_80386(pc->cpu);
	}
	else if (strcmp(str, "80386") == 0) {
		e86_set_80386(pc->cpu);
	}
	else if (strcmp(str, "80386_a0") == 0) {
		e86_set_80386(pc->cpu);
	}
	else if (strcmp(str, "80386_a1") == 0) {
		e86_set_80386(pc->cpu);
	}
	else if (strcmp(str, "80386_a2") == 0) {
		e86_set_80386(pc->cpu);
	}
	else if (strcmp(str, "80386_b0") == 0) {
		e86_set_80386(pc->cpu);
	}
	else if (strcmp(str, "80386_b1") == 0) {
		e86_set_80386(pc->cpu);
	}
	else if (strcmp(str, "80386_c0") == 0) {
		e86_set_80386(pc->cpu);
	}
	else if (strcmp(str, "80386_d0") == 0) {
		e86_set_80386(pc->cpu);
	}
	else if (strcmp(str, "80386_d1") == 0) {
		e86_set_80386(pc->cpu);
	}
	else if (strcmp(str, "80386_d2") == 0) {
		e86_set_80386(pc->cpu);
	}
	else if (strcmp(str, "80386sx") == 0) {
		e86_set_80386(pc->cpu);
	}
	else if (strcmp(str, "80386sl") == 0) {
		e86_set_80386(pc->cpu);
	}
	else if (strcmp(str, "80386dx") == 0) {
		e86_set_80386(pc->cpu);
	}
	else if (strcmp(str, "80386dxl") == 0) {
		e86_set_80386(pc->cpu);
	}
	else if (strcmp(str, "80386_disassembler") == 0) {
		e86_set_80386_disassembler(pc->cpu);
	}
	else if (strcmp(str, "80386_a0_disassembler") == 0) {
		e86_set_80386_disassembler(pc->cpu);
	}
	else if (strcmp(str, "80386_a1_disassembler") == 0) {
		e86_set_80386_disassembler(pc->cpu);
	}
	else if (strcmp(str, "80386_a2_disassembler") == 0) {
		e86_set_80386_disassembler(pc->cpu);
	}
	else if (strcmp(str, "80386_b0_disassembler") == 0) {
		e86_set_80386_disassembler(pc->cpu);
	}
	else if (strcmp(str, "80386_b1_disassembler") == 0) {
		e86_set_80386_disassembler(pc->cpu);
	}
	else if (strcmp(str, "80386_b2_disassembler") == 0) {
		e86_set_80386_disassembler(pc->cpu);
	}
	else if (strcmp(str, "80386_c0_disassembler") == 0) {
		e86_set_80386_disassembler(pc->cpu);
	}
	else if (strcmp(str, "80386_d0_disassembler") == 0) {
		e86_set_80386_disassembler(pc->cpu);
	}
	else if (strcmp(str, "80386_d1_disassembler") == 0) {
		e86_set_80386_disassembler(pc->cpu);
	}
	else if (strcmp(str, "80386_d2_disassembler") == 0) {
		e86_set_80386_disassembler(pc->cpu);
	}
	else {
		return (1);
	}
	/* ------------------------------------------------------------------------------------------ */
	/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; DMA Controller:       */
	/* ------------------------------------------------------------------------------------------ */
	/* The 8237 Direct Memory Access (DMA) controller and its support logic in the gate array     */
	/* support four channels of 20 address bit DMA. It operates at 4 MHz and handles only 8-bit   */
	/* data transfers. The DMA channel assignments and page register addresses are:               */
	/* ------------------------------------------------------------------------------------------ */
	/* Level:  Assignment:                                                                        */
	/*  - DRQ0: - Not Available                                                                   */
	/*  - DRQ1: - Not Used                                                                        */
	/*  - DRQ2: - Diskette                                                                        */
	/*  - DRQ3: - Fixed Disk                                                                      */
	/* ------------------------------------------------------------------------------------------ */
	/* Figure 1-7. DMA Channel Assignments:                                                       */
	/* ------------------------------------------------------------------------------------------ */
	/* Hex Address: DMA Page Register:                                                            */
	/*  - 081:       - Channel 2:                                                                 */
	/*  - 082:       - Channel 3:                                                                 */
	/*  - 083:       - Channel 1:                                                                 */
	/*  - 087:       - Channel 0:                                                                 */
	/* ------------------------------------------------------------------------------------------ */
	/* Figure 1-8. DMA Page Register Addresses:                                                   */
	/* ------------------------------------------------------------------------------------------ */
	/* Three of the DMA channels (1, 2, and 3) are available on the I/O bus and support high-speed*/
	/* data transfers between I/O devices and memory without microprocessor intervention.         */
	/*                                                                                            */
	/* DMA data transfers take six clock cycles of 250 ns, or 1.5 micro-seconds. I/O CH RDY can be*/
	/* pulled inactive to add wait states to allow more time for slower devices.                  */
	/* ------------------------------------------------------------------------------------------ */
	/* -------------------------------------------------------------------------------------- */
	/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; Interrupts:       */
	/* -------------------------------------------------------------------------------------- */
	/* The interrupt controller has eight levels of interrupts that are handled according to  */
	/* priority in the I/O support gate array. Two levels are used only on the system board.  */
	/* Level 0, the highest priority, is attached to Channel 0 of the timer/counter and       */
	/* provides a periodic interrupt for the timer tick. Level 1 is shared by the keyboard and*/
	/* the pointing device. It is handled by a BIOS routine pointed to by inter-rupt hex 71.  */
	/* Level 2 is available to the video subsystem, and level 7 is available to the parallel  */
	/* port; however, the BIOS routines do not use interrupts 2 and 7. Level 4 is used by the */
	/* serial port.                                                                           */
	/*                                                                                        */
	/* The controller also has inputs from coprocessor's '-interrupt', the memory controller's*/
	/*'-parity', and '-I/O channel check' signals. These three inputs are used to generate the*/
	/* NMI to the 8086-2.                                                                     */
	/*                                                                                        */
	/* The following table shows the hardware interrupts and their avail-ability to the I/O   */
	/* channel.                                                                               */
	/* -------------------------------------------------------------------------------------- */
	/* Level:  System Board:                   I/O Channel:                                   */
	/*  - NMI : - Parity Check and Coprocessor: - I/O Channel Check:                          */
	/*  - IRQ0: - Timer Channel 0:              - Not Available:                              */
	/*  - IRQ1: - Keyboard Pointing Device:     - Not Available:                              */
	/*  - IRQ2: - Video:                        -     Available:                              */
	/*  - IRQ4: - Serial Port:                  -     Available:                              */
	/*  - IRQ5: - Fixed Disk:                   -     Available:                              */
	/*  - IRQ6: - Diskette Drive:               -     Available:                              */
	/*  - IRQ7: - Parallel Port:                -     Available:                              */
	/* NOTE: Interrupts are available to the I/O channel if they are not enabled by the system*/
	/* board function normally assigned to that interrupt.                                    */
	/* -------------------------------------------------------------------------------------- */
	/* Figure 1-9. Hardware Interrupt Listing:                                                */
	/* -------------------------------------------------------------------------------------- */
	/* -------------------------------------------------------------------------------------- */
	/* IBM PC (Model 5150/5160): Quote from Technical Reference; I/O Channel Diagram:         */
	/* -------------------------------------------------------------------------------------- */
	/* Signal Name:  Rear Panel: Signal Name:                                                 */
	/*  - GND:        - B 1: - A 1: - -I/OCH CK:                                              */
	/*  - +RESET DRV: - B 2: - A 2: - +D7:                                                    */
	/*  - +5V:        - B 3: - A 3: - +D6:                                                    */
	/*  - +IRQ2:      - B 4: - A 4: - +D5:                                                    */
	/*  - -5VDC:      - B 5: - A 5: - +D4:                                                    */
	/*  - +DRQ2:      - B 6: - A 6: - +D3:                                                    */
	/*  - -12V:       - B 7: - A 7: - +D2:                                                    */
	/*  - Reserved:   - B 8: - A 8: - +D1:                                                    */
	/*  - +12V:       - B 9: - A 9: - +D0:                                                    */
	/*  - GND:        - B10: - A10: - +I/O CH RDY:                                            */
	/*  - -MEMW:      - B11: - A11: - +AEN:                                                   */
	/*  - -MEMR:      - B12: - A12: - +A19:                                                   */
	/*  - -IOW:       - B13: - A13: - +A18:                                                   */
	/*  - -IOR:       - B14: - A14: - +A17:                                                   */
	/*  - -DACK3:     - B15: - A15: - +A16:                                                   */
	/*  - +DRQ3:      - B16: - A16: - +A15:                                                   */
	/*  - -DACK1:     - B17: - A17: - +A14:                                                   */
	/*  - +DRQ1:      - B18: - A18: - +A13:                                                   */
	/*  - -DACK0:     - B19: - A19: - +A12:                                                   */
	/*  - CLK:        - B20: - A20: - +A11:                                                   */
	/*  - +IRQ7:      - B21: - A21: - +A10:                                                   */
	/*  - +IRQ6:      - B22: - A22: - +A 9:                                                   */
	/*  - +IRQ5:      - B23: - A23: - +A 8:                                                   */
	/*  - +IRQ4:      - B24: - A24: - +A 7:                                                   */
	/*  - +IRQ3:      - B25: - A25: - +A 6:                                                   */
	/*  - -DACK2:     - B26: - A26: - +A 5:                                                   */
	/*  - +T/C:       - B27: - A27: - +A 4:                                                   */
	/*  - +ALE:       - B28: - A28: - +A 3:                                                   */
	/*  - +5V:        - B29: - A29: - +A 2:                                                   */
	/*  - +OSC:       - B30: - A30: - +A 1:                                                   */
	/*  - +GND:       - B31: - A31: - +A 0:                                                   */
	/* -------------------------------------------------------------------------------------- */
	/* IBM PC (Model 5150/5160): Quote from "Technical Reference"; I/O Channel Description:   */
	/* -------------------------------------------------------------------------------------- */
	/* Signal Name:                                                                           */
	/*  - GND:                                                                                */
	/*  - +RESET DRV (Output): Reset Drive: This line is used to reset or initialize system   */
	/*  logic upon power-up or during a low line-voltage outage. This signal is synchronized  */
	/*  to the falling edge of CLK and is active high.                                        */
	/*  - +5V:                                                                                */
	/*  - +IRQ2 (Input): Interrupt Request 2 to 7: These lines are used to signal the         */
	/*  microprocessor that an I/O device required attention. They are prioritized with IRQ2  */
	/*  as the highest priority and IRQ7 as the lowest. An Interrupt Request is generated by  */
	/*  raising an IRQ line (low to high) and holding it high until it is acknowledged by the */
	/*  the microprocessor (interrupt service routine).                                       */
	/*  - -5VDC:                                                                              */
	/*  - +DRQ2 (Input): DMA Request 1 to 3: These lines are asynchronous channel requests    */
	/*  used by peripheral devices to gain DMA service. They are prioritized with DRQ3 being  */
	/*  the lowest and DRQ1 being the highest. A DRQ line must be held high until the         */
	/*  corresponding DACK line goes active.                                                  */
	/*  - -12V:                                                                               */
	/*  - Reserved:                                                                           */
	/*  - +12V:                                                                               */
	/*  - GND:                                                                                */
	/*  - -MEMW (Output): -Memory Write Command: This command line instructs the memory to    */
	/*  store the data present on the data bus. It may be driven by the microprocessor or the */
	/*  DMA controller. This signal is active low.                                            */
	/*  - -MEMR (Output): -Memory Read Command: This command line instructs the memory to     */
	/*  drive its data onto the data bus. It may be driven by the microprocessor or the DMA   */
	/*  controller. This signal is active low.                                                */
	/*  - -IOW (Output): -I/O Write Command: This command line instructs an I/O device to read*/
	/*  the data on the data bus. It may be driven by the microprocessor or the DMA           */
	/*  controller. This signal is active low.                                                */
	/*  - -IOR (Output): -I/O Read Command: This command line instructs an I/O device to drive*/
	/*  its data onto the data bus. It may be driven by the microprocessor or the DMA         */
	/*  controller. This signal is active low.                                                */
	/*  - -DACK3 (Output): DMA Acknowledge 0 to 3: These lines are used to acknowledge DMA    */
	/*  requests (DRQ1-DRG3) and refresh system dynamic memory (-DACK0). They are active low. */
	/*  - +DRQ3 (Input): See +DRQ2                                                            */
	/*  - -DACK1 (Output): See -DACK3                                                         */
	/*  - +DRQ1 (Input): See +DRQ2                                                            */
	/*  - -DACK0 (Output): See -DACK3                                                         */
	/*  - CLK (Output): System clock: It is a divide-by-three of the oscillator and has a     */
	/*  210-ns (4.77-MHz) The clock has a 33% duty cycle.                                     */
	/*  - +IRQ7 (Input): See +IRQ2                                                            */
	/*  - +IRQ6 (Input): See +IRQ2                                                            */
	/*  - +IRQ5 (Input): See +IRQ2                                                            */
	/*  - +IRQ4 (Input): See +IRQ2                                                            */
	/*  - +IRQ3 (Input): See +IRQ2                                                            */
	/*  - -DACK2 (Output): See -DACK3                                                         */
	/*  - +T/C (Output): Terminal Count: This line provides a pulse when the terminal count   */
	/*  for any DMA channel is reached. This signal is active high.                           */
	/*  - +ALE (Output): Address Latch Enable: This line is provided by the 8288 Bus          */
	/*  Controller and is used on the system board to latch valid addresses from the          */
	/*  microprocessor. It is available to the I/O channel as an indicator of a valid         */
	/*  microprocessor address (when used with AEN). Microprocessor addresses are latched with*/
	/*  the falling edge of ALE.                                                              */
	/*  - +5V:                                                                                */
	/*  - +OSC (Output): Oscillator: High-speed clock with a 70-ns period (14.31818-MHz). It  */
	/*  has a 50% duty cycle.                                                                 */
	/*  - +GND:                                                                               */
	/*  - -I/O CH CK (Input): -I/O Channel Check: This line provides the microprocessor with  */
	/*  parity (error) information on memory or devices in the I/O channel. When this signal  */
	/*  is active low, a parity error is indicated.                                           */
	/*  - +D7 (Input/Output): Data Bits 0 to 7: These lines provide data bus bits 0 to 7 for  */
	/*  - +D6 (Input/Output): the microprocessor, memory, and I/O devices. D0 is the least    */
	/*  - +D5 (Input/Output): significant bit (LSB) and D7 is the most significant bit (MSB). */
	/*  - +D4 (Input/Output): These lines are active high.                                    */
	/*  - +D3 (Input/Output): See above                                                       */
	/*  - +D2 (Input/Output): See above                                                       */
	/*  - +D1 (Input/Output): See above                                                       */
	/*  - +D0 (Input/Output): See above                                                       */
	/*  - +I/O CH RDY (Input): I/O Channel Ready: This line, normally high (ready), is pulled */
	/*  low (not ready) by a memory or I/O device to lengthen I/O or memory cycles. It allows */
	/*  slower devices to attach to the I/O channel with a minimum of difficulty. Any slow    */
	/*  device using this line should drive it low immediately upon detecting a valid address */
	/*  and a Read or Write command. This line should never be held low longer than 10 clock  */
	/*  cycles. Machine cycles (I/O or memory) are extended by an integral number of clock    */
	/*  cycles (210-ns).                                                                      */
	/*  - +AEN (Output): Address Enable: This line is used to de-gate the microprocessor and  */
	/*  other devices from the I/O channel to allow DMA transfers to take place. When this    */
	/*  line is active (high), the DMA controller has control of the address bus, data bus,   */
	/*  Read command lines (memory and I/O), and the Write command lines (memory and I/O).    */
	/*  - +A19 (Output): Address bits 0 to 19: These lines are used to address memory and I/O */
	/*  - +A18 (Output): devices within the system. The 20 address lines allow access of up to*/
	/*  - +A17 (Output): 1M-byte of memory. A0 is the least significant bit (LSB) and A19 is  */
	/*  - +A16 (Output): the most significant bit (MSB). These lines are generated by either  */
	/*  - +A15 (Output): the microprocessor or DMA controller. They are active high.          */
	/*  - +A14 (Output): See above                                                            */
	/*  - +A13 (Output): See above                                                            */
	/*  - +A12 (Output): See above                                                            */
	/*  - +A11 (Output): See above                                                            */
	/*  - +A10 (Output): See above                                                            */
	/*  - +A9 (Output): See above                                                             */
	/*  - +A8 (Output): See above                                                             */
	/*  - +A7 (Output): See above                                                             */
	/*  - +A6 (Output): See above                                                             */
	/*  - +A5 (Output): See above                                                             */
	/*  - +A4 (Output): See above                                                             */
	/*  - +A3 (Output): See above                                                             */
	/*  - +A2 (Output): See above                                                             */
	/*  - +A1 (Output): See above                                                             */
	/*  - +A0 (Output): See above                                                             */
	/* -------------------------------------------------------------------------------------- */
	/* I/O Address Map:                                                                       */
	/* -------------------------------------------------------------------------------------- */
	/* NOTE: These are the addresses decoded by the current set of adapter cards. IBM may use */
	/* any of the unlisted addresses for future use.                                          */
	/* -------------------------------------------------------------------------------------- */
	/* | Hex Range |                                  Usage:                                  */
	/* -------------------------------------------------------------------------------------- */
	/* |  000-00F  | DMA Chip 8237A-5                                                       | */
	/* |  020-021  | Interrupt 8259A                                                        | */
	/* |  040-043  | Timer 8253-5                                                           | */
	/* |  060-063  | PPI 8255A-5                                                            | */
	/* |  080-083  | DMA Page Registers                                                     | */
	/* |    OAX    | NMI Mask Register                                                      | */
	/* | NOTE: At power-on time, the Non Mask Interrupt into the 8088 is masked off. This   | */
	/* | mask bit can be set and reset through system software as follows:                  | */
	/* |  - Set   mask: Write hex 80 to I/O Address hex A0 (enable  NMI)                    | */
	/* |  - Clear mask: Write hex 00 to I/O Address hex A0 (disable NMI)                    | */
	/* |  200-20F  | Game Control                                                           | */
	/* |  210-217  | Expansion Unit                                                         | */
	/* |  2F8-2FF  | Asynchronous Communications (Secondary)                                | */
	/* |  300-31F  | Prototype Card                                                         | */
	/* |  320-32F  | Fixed Disk                                                             | */
	/* |  378-37F  | Printer                                                                | */
	/* |  380-38C  | SDLC Communications                                                    | */
	/* |  380-389  | Binary Synchronous Communications (Secondary)                          | */
	/* | NOTE: SDLC Communications and Secondary Binary Synchronous Communications cannot be| */
	/* | used together because their hex addresses overlap.                                 | */
	/* |  390-393  | Cluster                                                                | */
	/* |  3A0-3A9  | Binary Synchronous Communications (Primary)                            | */
	/* |  3B0-3BF  | IBM Monochrome Display/Printer                                         | */
	/* |  3D0-3DF  | Color/Graphics                                                         | */
	/* |  3F0-3F7  | Diskette                                                               | */
	/* |  3F8-3FF  | Asynchronous Communications (Primary)                                  | */
	/* |  790-793  | Cluster (Adapter 1)                                                    | */
	/* |  B90-B93  | Cluster (Adapter 2)                                                    | */
	/* | 1390-1393 | Cluster (Adapter 3)                                                    | */
	/* | 2390-2393 | Cluster (Adapter 4)                                                    | */
	/* -------------------------------------------------------------------------------------- */
    return (0);
    /* -------------------------------------------------------------------------------------- */
    /* System-Board Switch Settings:                                                          */
    /* -------------------------------------------------------------------------------------- */
    /* All system board switch settings for total system memory, number of diskette drives,   */
    /* and type of display adapter are described under "Switch Settings" in the IBM Personal  */
    /* Computer Guide to Operations. The diagram showing the system board switch locations    */
    /* follows.                                                                               */
    /* -------------------------------------------------------------------------------------- */
    /* Model 5150:                                                                            */
    /* -------------------------------------------------------------------------------------- */
    /* System Expansion Slots  : Cassette I/O:    Keyboard I/O:System Board Power Connections:*/
    /*  - J1 - J2 - J3 - J4 - J5  - J6             - J7                                       */
    /*                  IBM Math Coprocessor: Intel 8088 Microprocessor: Clock Chip Trimmer:  */
    /*                  DIP Switch Block 1:   DIP Switch Block 2:                             */
    /*                   - 1:                  - 2:                                           */
    /* Read Only Memory:                                                                      */
    /* 64 to 256K Read/Write Memory with Parity Checking:                                     */
    /* Pin 1: Speaker Output: Cassette Microphone or Auxiliary Select:                        */
    /*  - P3   - P3            - P4                                                           */
    /* -------------------------------------------------------------------------------------- */
    /* Model 5160:                                                                            */
    /* -------------------------------------------------------------------------------------- */
    /* System Expansion Slots: Clock Chip Trimmer:Keyboard I/O:System Board Power Connections:*/
    /*  - J1 - J2 - J3 - J4 - J5 - J6 - J7 - J8    - J9                                       */
    /*                  IBM Math Coprocessor: Intel 8088 Microprocessor: ROM BIOS: ROM BASIC: */
    /*                  System Configuration DIP Switches:                                    */
    /*                                                                                        */
    /* Up to 256K Read/Write Memory with parity Checking:                                     */
    /* Pin 1: Speaker Output:                                                                 */
    /*  - P3   - P3                                                                           */
    /* -------------------------------------------------------------------------------------- */
    /* Specifications:                                                                        */
    /* -------------------------------------------------------------------------------------- */
    /* The following voltages are available on the system-board I/O channel:                  */
    /*  -  +5 Vdc +=  5% on 2 connector pins                                                  */
    /*  -  -5 Vdc += 10% on 1 connector pin                                                   */
    /*  - +12 Vdc +=  5% on 1 connector pin                                                   */
    /*  - -12 Vdc += 10% on 1 connector pin                                                   */
    /*    GND (Ground)   on 3 connector pins                                                  */
    /* -------------------------------------------------------------------------------------- */
    /* Card Specifications:                                                                   */
    /* -------------------------------------------------------------------------------------- */
    /* The specifications for option cards follow.                                            */
    /* ---- FILE WILL BE INCLUDED IN NEXT RELEASE ----                                        */
    /* -------------------------------------------------------------------------------------- */
    /* Logic Diagrams:                                                                        */
    /* -------------------------------------------------------------------------------------- */
    /* The following pages contain the logic diagrams for the system board.                   */
    /*  - 64/256K System Board (Model 5150)/System Board (Model 5160) (Sheet  1 of 10)        */
    /*  - 64/256K System Board (Model 5150)/System Board (Model 5160) (Sheet  2 of 10)        */
    /*  - 64/256K System Board (Model 5150)/System Board (Model 5160) (Sheet  3 of 10)        */
    /*  - 64/256K System Board (Model 5150)/System Board (Model 5160) (Sheet  4 of 10)        */
    /*  - 64/256K System Board (Model 5150)/System Board (Model 5160) (Sheet  5 of 10)        */
    /*  - 64/256K System Board (Model 5150)/System Board (Model 5160) (Sheet  6 of 10)        */
    /*  - 64/256K System Board (Model 5150)/System Board (Model 5160) (Sheet  7 of 10)        */
    /*  - 64/256K System Board (Model 5150)/System Board (Model 5160) (Sheet  8 of 10)        */
    /*  - 64/256K System Board (Model 5150)/System Board (Model 5160) (Sheet  9 of 10)        */
    /*  - 64/256K System Board (Model 5150)/System Board (Model 5160) (Sheet 10 of 10)        */
    /* -------------------------------------------------------------------------------------- */
    /* Data Flow Diagrams (5150/5160):                                                        */
    /* -------------------------------------------------------------------------------------- */
    /* The system board data flow diagram follows.                                            */
    /* -------------------------------------------------------------------------------------- */
    /*  - System Board Data Flow (Part 1 of 2)                                                */
    /*  - System Board Data Flow (Part 2 of 2)                                                */
    /* -------------------------------------------------------------------------------------- */
    /* NOTE: Please see bitmap files included in source directory tree for each sheet.        */
    /* -------------------------------------------------------------------------------------- */
}

void pc_set_speed (ibmpc_t *pc, unsigned factor)
{
	pc->speed_current = factor;
	pc->speed_clock_extra = 0;
	pce_log_tag (MSG_INF, "CPU:", "setting speed to %uX\n", pc->speed_current);
}

void pc_set_bootdrive (ibmpc_t *pc, unsigned drv)
{	
	pc->bootdrive = drv;
}

unsigned pc_get_bootdrive (ibmpc_t *pc)
{
	return (pc->bootdrive);
}
