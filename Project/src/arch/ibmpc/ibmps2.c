/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/ibmpc/m24.c                                         *
 * Created:     2012-01-07 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012 Hampa Hug <hampa@hampa.ch>                          *
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

/* IBM PS/2 Model 25/30 specific code */


#include "main.h"
#include "ibmpc.h"
#include <devices/fdc.h>
#include <drivers/block/block.h>

#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include <lib/log.h>
#include <libini/libini.h>

/* ---------------------------------------------------------------------------------------------- */
/* Quote from IBM Personal System2 Model 25 Technical Reference: System Support Gate Array:       */
/* ---------------------------------------------------------------------------------------------- */
/* The system support gate array contains the bus controller, the memory controller and parity    */
/* checker, the wait-state generator and bus conversion logic, the system clock generator, and the*/
/* DMA page register and support logic.                                                           */
/* ---------------------------------------------------------------------------------------------- */
/* Bus Controller:                                                                                */
/* ---------------------------------------------------------------------------------------------- */
/* The Type 8525 has three bus masters on the local bus: the micro-processor, the coprocessor, and*/
/* the system support gate array. The gate array seizes the bus to generate memory refresh and DMA*/
/* bus cycles. It controls two request/grant lines (CPU RQ/GT and NPU RQ/GT). One is connected to */
/* the microprocessor and the other to the coprocessor.                                           */
/*                                                                                                */
/* When the coprocessor is not installed, the gate array generates a request pulse to the         */
/* microprocessor to get control of the bus. The microprocessor then gives control of the bus and */
/* pulses the same line to indicate a grant. When the coprocessor is installed, the gate array    */
/* generates this pulse to the coprocessor. If the coprocessor has control of the bus, it grants  */
/* control to the gate array. If the coprocessor is not in control, it relays the request back to */
/* the micro-processor and relays the grant back to the gate array. This arrange-ment gives the   */
/* gate array the highest priority use of the bus.                                                */
/* ---------------------------------------------------------------------------------------------- */
/* Memory Controller and Parity Checker:                                                          */
/* ---------------------------------------------------------------------------------------------- */
/* The memory controller functions of the gate array control memory and generate the memory       */
/*refresh. Memory must be refreshed once every 4 ms. Memory refresh takes nine clock cycles of 125*/
/* ns through a dedicated refresh channel within the gate array.                                  */
/*                                                                                                */
/* The parity checker function generates the parity bits for system memory and activates the      */
/* '-parity check' signal when a parity error is detected. Only read/write memory on the system   */
/* board is checked.                                                                              */
/* ---------------------------------------------------------------------------------------------- */
/* Bus Conversion Logic and Wait-State Generator:                                                 */
/* ---------------------------------------------------------------------------------------------- */
/* The bus conversion logic converts word transfers to I/O devices into 2 single-byte transfers.  */
/* Sixteen-bit are only supported for the system's read-only and read/write memory.               */
/*                                                                                                */
/* Additional logic generates the needed wait states for the micro-processor bus cycles to I/O    */
/* devices. The logic monitors the 'I/O CH RDY' line to determine the wait states required.       */
/* ---------------------------------------------------------------------------------------------- */
static
unsigned char ibmps2_rtc_get_uint8 (ibmpc_t *pc, unsigned long addr)
{
	time_t    t;
	struct tm *tm;
	unsigned  yrs, mon, day, hrs, min, sec, dec;
	if (pc->support_rtc) {
#ifdef HAVE_GETTIMEOFDAY
		struct timeval tv;
		if (gettimeofday (&tv, NULL)) {
			t = time (NULL);
			dec = 0;
		}
		else {
			t = tv.tv_sec;
			dec = tv.tv_usec / 100000;
		}
#else
		t = time (NULL);
#endif
		tm = localtime (&t);
		yrs = 1900 + tm->tm_year;
		mon = tm->tm_mon;
		day = tm->tm_mday;
		hrs = tm->tm_hour;
		min = tm->tm_min;
		sec = tm->tm_sec;
	}
	else {
		yrs = 1980;
		mon = 0;
		day = 0;
		hrs = 0;
		min = 0;
		sec = 0;
		dec = 0;
	}
	switch (addr) {
	case 0x01:
		return (dec % 10);
	case 0x02:
		return (sec % 10);
	case 0x03:
		return (sec / 10);
	case 0x04:
		return (min % 10);
	case 0x05:
		return (min / 10);
	case 0x06:
		return (hrs % 10);
	case 0x07:
		return (hrs / 10);
	case 0x08:
		return (day % 10);
	case 0x09:
		return (day / 10);
	case 0x0b:
		return ((mon + 1) % 10);
	case 0x0c:
		return ((mon + 1) / 10);
	case 0x0f:
		return ((yrs < 1980) ? 0 : ((yrs - 1980) & 7));
	default:
		pc_log_deb ("ibmps2 rtc get port 8 %04lX\n", addr + 0x70);
		break;
	}
	return (0xff);
}
/* ---------------------------------------------------------------------------------------------- */
/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; I/O Support Gate Array:   */
/* ---------------------------------------------------------------------------------------------- */
/* The I/O support gate array contains the chip select (CS) logic; key-board and pointing device  */
/* controller, and I/O ports. It also contains the interrupt controller.                          */
/* ---------------------------------------------------------------------------------------------- */
/* Chip Select Logic:                                                                             */
/* ---------------------------------------------------------------------------------------------- */
/* The gate array controls the following CS signals on the system board:                          */
/*  - Serial     port                                                                             */
/*  - Diskette   controller                                                                       */
/*  - Video      controller                                                                       */
/*  - Parallel   port                                                                             */
/*  - Fixed disk controller                                                                       */
/* Each select line can be disabled by programming the System Board Control register, address hex */
/*65. When the bit is set to 1, that function of the system board is enabled. Bit 7, parallel port*/
/* output enable, enables the parallel port's output drivers.                                     */
/*                                                                                                */
/* When the signal is enabled, the CS signal is generated to start a read or write operation, and */
/* the read and write signals to the I/O channel are blocked. When the signal is disabled, the CS */
/* signal is not gener-ated, and all read and write operations are directed to the I/O channel.   */
/* This register is read/write.                                                                   */
/* ---------------------------------------------------------------------------------------------- */
/* Bit : Function:                                                                                */
/*  - 7:  - Parallel Port Output Enable                                                           */
/*  - 6:  - Reserved   = 0                                                                        */
/*  - 5:  - Reserved   = 0                                                                        */
/*  - 4:  - Serial     CS                                                                         */
/*  - 3:  - Diskette   CS                                                                         */
/*  - 2:  - Video      CS                                                                         */
/*  - 1:  - Parallel   CS                                                                         */
/*  - 0:  - Fixed Disk CS                                                                         */
/* ---------------------------------------------------------------------------------------------- */
/* Figure 1-3. System Board Control Register, Hex 65                                              */
/* ---------------------------------------------------------------------------------------------- */
int ibmps2_get_port8 (ibmpc_t *pc, dev_fdc_t *fdc, disk_t *dsk, unsigned long addr, unsigned long val)
{
	unsigned disk_change_line;
	if ((pc->model & PCE_IBMPS2_MODEL25) == 0) {
		return (1);
	}
	/* if ((addr >= 0x70) && (addr <= 0x7f)) {
		*val = ibmps2_rtc_get_uint8 (pc, addr - 0x70);
		return (0);
	} */
	switch (addr) {
	case 0x60:
		/* Placeholder until function is added */
		return (0);
	case 0x61:
		val = ibmps2_get_io_port_61(pc);
		return (0);
	case 0x62:
		val = ibmps2_get_io_port_62(pc);
		return (0);
	case 0x65:
		val = ibmps2_get_control_register(pc);
		return (0);
	case 0x3F0:
		val = ibmps2_get_fdc_ras_port_a(fdc);
		return (0);
	case 0x3F1:
		val = ibmps2_get_fdc_ras_port_b(fdc);
		return (0);
	/* case 0x3F4: */
		/* NOTE: See file "e8272.c" for more details: */
		/* --------------------------------------------------------------------------------------------------- */
		/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; Main Status Register, Hex 3F4: */
		/* --------------------------------------------------------------------------------------------------- */
		/* This register is read-only and is used to facilitate the transfer of data between the microprocessor*/
		/* and the controller.                                                                                 */
		/* --------------------------------------------------------------------------------------------------- */
		/* Bit:   Function:                                                                                    */
		/*  - 7  : - Request for Master                                                                        */
		/*  - 6  : - Data Input/Output                                                                         */
		/*  - 5  : - Non-DMA Mode                                                                              */
		/*  - 4  : - Diskette Controller Busy                                                                  */
		/*  - 3,2: - Reserved                                                                                  */
		/*  - 1  : - Drive 1 Busy                                                                              */
		/*  - 0  : - Drive 0 Busy                                                                              */
		/* --------------------------------------------------------------------------------------------------- */
		/* Figure 1-68. Main Status Register:                                                                  */
		/* --------------------------------------------------------------------------------------------------- */
		/* The bits are defined as follows:                                                                    */
		/*  - Bit  7   : The data register is ready for transfer with the micro-processor.                     */
		/*  - Bit  6   : This bit indicates the direction of data transfer between the diskette controller and */
		/*  the microprocessor. If this bit is set to 1, the transfer is from the controller to the micro-     */
		/*  processor; if it is clear, the transfer is from the micro-processor.                               */
		/*  - Bit  5   : When this bit is set to 1, the controller is in the non-DMA mode.                     */
		/*  - Bit  4   : When this bit is set to 1, a Read or Write command is being executed.                 */
		/*  - Bits 3, 2: Reserved                                                                              */
		/*  - Bit  1   : Drive 1 Busy--When set to 1, diskette drive 1 is in the seek mode.                    */
		/*  - Bit  0   : Drive 0 Busy--When set to 1, diskette drive 0 is in the seek mode.                    */
		/* --------------------------------------------------------------------------------------------------- */
	case 0x3F7:
		disk_change_line = dsk->new_disk;
		ibmps2_set_fdc_digital_input_diskette_change(fdc, 0x0);
		if (disk_change_line == 0x1)
		{
			/* Disk change line in effect */
			ibmps2_set_fdc_digital_input_diskette_change(fdc, 0x1);
			dsk->new_disk = 0x0;
		}
		val = ibmps2_get_fdc_digital_input(fdc);
		return (0);
	}
	/* ------------------------------------------------------------------------------------------ */
	/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; Diskette Drive        */
	/* Interface:                                                                                 */
	/* ------------------------------------------------------------------------------------------ */
	/* The diskette gate array contains the decode logic for the internal reg-isters, the write   */
	/* logic, and the read logic. The gate array:                                                 */
	/*  - Controls the clock signals needed for read and write                                    */
	/*  - Controls write precompensation                                                          */
	/*  - Selects the data rate of transfer                                                       */
	/*  - Provides a mask for the interrupt and DMA request lines                                 */
	/*  - Provides phase error detection for input to the phase-lock loop                         */
	/* The phase detecter/amplifier and the voltage controlled oscillator (VC)) make up the phase-*/
	/* lock loop (PLL). They adjust the clock used during data read to keep it in phase with the  */
	/* data signal.                                                                               */
	/*                                                                                            */
	/* The drives connect to the system board through a single 40-pin con-nector, which supplies  */
	/* all signals necessary to operate two diskette drives. The diskette drives are attached to  */
	/* the connector through an internal, flat cable.                                             */
	/* ------------------------------------------------------------------------------------------ */
	return (1);
}
/* ---------------------------------------------------------------------------------------------- */
/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; I/O Ports:                */
/* ---------------------------------------------------------------------------------------------- */
/* The output port hex 60 is used by BIOS to store keystrokes. The output port hex  is used as    */
/* beeper control, enables I/O channel check, and parity checks, and is read/write. The input port*/
/* hex 62 contains the status of certain signals on the system board and is read-only. The bit    */
/* descriptions of ports hex 61 and hex 62 follow.                                                */
/* ---------------------------------------------------------------------------------------------- */
/* Bit: Function:                                                                                 */
/*  - 7: - Reserved                                                                               */
/*  - 6: - Reserved                                                                               */
/*  - 5: - -ENA I/O CH CK                                                                         */
/*  - 4: - -ENA RAM Parity CK                                                                     */
/*  - 3: - Reserved                                                                               */
/*  - 2: - Reserved                                                                               */
/*  - 1: - Beeper Data                                                                            */
/*  - 0: - Timer 2 Gate (to beeper)                                                               */
/* ---------------------------------------------------------------------------------------------- */
/* Figure 1.5. Output Port, 61:                                                                   */
/* ---------------------------------------------------------------------------------------------- */
/* Port 1 Bit:Connected to:         Description:                                                  */
/*  - 7-6:     - Not connected:      - Reserved                                                   */
/*  -   5:     - -ENA I/O CH     CK: - When set to 1, this bit stops -I/O CH CK from generating a */
/*                                   non-maskable interrupt (NMI). When cleared to 0, an NMI is   */
/*                                   generated when -I/O CH CK goes active.                       */
/*  -   4:     - -ENA RAM PARITY CK: - When set to 1, this bit stops a memory parity error from   */
/*                                  generating an NMI. When cleared to 0, an NMI is generated when*/
/*                                   a memory parity error is sensed.                             */
/*  - 3-2:     - Not connected:      - Reserved                                                   */
/*  -   1:     - Beeper Data:        - This bit gates the output of timer 2. It is used to disable*/
/*                                   the timer's sound source or modify its output. When set to 1,*/
/*                                   this bit enables the output; when cleared, it forces the     */
/*                                   output to zero.                                              */
/*  -   0:     - Timer 2 Gate:       - This line is routed to the timer input at GATE 2. When this*/
/*                                   bit is cleared to 0, the timer operation is halted. This bit */
/*                                   and bit 1 (beeper data) control the operation of the timer's */
/*                                   sound source.                                                */
/* ---------------------------------------------------------------------------------------------- */
/* Bit: Function:                                                                                 */
/*  - 7: - Parity                                                                                 */
/*  - 6: - I/O CH CK                                                                              */
/*  - 5: - Timer 2 Output                                                                         */
/*  - 4: - Reserved                                                                               */
/*  - 3: - Reserved                                                                               */
/*  - 2: - -Disk Installed                                                                        */
/*  - 1: - Coprocessor installed                                                                  */
/*  - 0: - Reserved                                                                               */
/* ---------------------------------------------------------------------------------------------- */
/* Figure 1.5.  Input Port, 62:                                                                   */
/* ---------------------------------------------------------------------------------------------- */
/* Port 1 Bit:Connected to:            Description:                                               */
/*  -   7:     - Parity      :          - When set to 1, this bit indicates that a memory parity  */
/*                                      error has occurred.                                       */
/*  -   6:     - I/O CH     CK:         - When set to 1, this bit indicates that -I/O CH CK is    */
/*                                      active                                                    */
/*  -   5:     - Timer 2 Output:        - When set to 1, this bit indicates that -I/O CH CK is    */
/*                                      active.                                                   */
/*  - 4-3:     - Not connected:         - Reserved.                                               */
/*  -   2:     - Disk installed  :      - When cleared to 0, this bit indicates that the fixed    */
/*                                      disk and controller are installed.                        */
/*  -   1:     - Coprocessor installed: - When set to 1, this bit indicates that the Math         */
/*                                      Coprocessor is installed.                                 */
/*  -   0:     - Not connected:         - Reserved.                                               */
/* ---------------------------------------------------------------------------------------------- */
int ibmps2_set_port8(ibmpc_t *pc, dev_fdc_t *fdc, unsigned long addr, unsigned long val)
{
	if ((pc->model & PCE_IBMPS2_MODEL25) == 0) {
		return (1);
	}
	switch (addr) {
	case 0x60:
		/* Placeholder until function is added */
		return (0);
	case 0x61:
		ibmps2_set_io_port_61(pc, val);
		return (0);
	case 0x62:
		ibmps2_set_io_port_62(pc, val);
		return (0);
	case 0x65:
		ibmps2_set_control_register(pc, val);
		return (0);
	case 0x6b:
		ibmps2_set_system_board_ram_control_status_register(pc, val);
		return (0);
	/* case 0x3F0:
		ibmps2_set_fdc_ras_port_a(fdc, val);
		return (0); */
	/* case 0x3F2: */
		/* NOTE: See file "e8272.c" for more details: */
		/* -------------------------------------------------------------------------------------- */
		/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; Digital Output    */
		/* Register:                                                                              */
		/* -------------------------------------------------------------------------------------- */
		/*The Digital Output register (DOR), hex 3F2, is a write-only register that controls drive*/
		/* motors, drive selection, and feature enables. All bits are cleared by a reset.         */
		/* -------------------------------------------------------------------------------------- */
		/* Bit:   Function:                                                                       */
		/*  - 7  : - Motor Enable 3                                                               */
		/*  - 6  : - Motor Enable 2                                                               */
		/*  - 5  : - Motor Enable 1                                                               */
		/*  - 4  : - Motor Enable 0                                                               */
		/*  - 3  : - DMA and Interrupt Enable                                                     */
		/*  - 2  : - -Controller Reset                                                            */
		/*  - 1,0: - Drive Select 0 through 3:                                                    */
		/*         - - 00 selects drive 0                                                         */
		/*         - - 01 selects drive 1                                                         */
		/*         - - 10 selects drive 2                                                         */
		/*         - - 11 selects drive 3                                                         */
		/* -------------------------------------------------------------------------------------- */
		/* Figure 1-65. Digital Output, Hex 3F2:                                                  */
		/* -------------------------------------------------------------------------------------- */
	}
	return (1);
}
/* ---------------------------------------------------------------------------------------------- */
/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; Read/Write Memory:        */
/* ---------------------------------------------------------------------------------------------- */
/* The system board supports 640K bytes of read/write memory. The first 128K is optional and      */
/* consists of four 64K by 4-bit and two 64K by 1-bit chips. Sockets are provided on the system   */
/* board for this optional memory.                                                                */
/*                                                                                                */
/* The next 512K (from 128K to 640K) is standard and is arranged as two banks of 256K by 9-bit    */
/* single-inline packages (SIPs). All read/write memory is parity-checked.                        */
/*                                                                                                */
/* The System Board RAM Control/Status register, hex 6B, is part of the system gate array and may */
/* be used to remap memory. Remapping occurs when the power-on self-test (POST) senses memory on  */
/* the I/O channel that is in contention with system memory. Also, if the first 128K is not       */
/*installed or a failure in the first 128K is sensed, POST remaps the remainder of memory to allow*/
/* the system to operate.                                                                         */
/* ---------------------------------------------------------------------------------------------- */
/* Bit: Function:                                                                                 */
/*  - 7: - Parity Check Pointer:                                                                  */
/*       - - 1 = Lower 128K failed                                                                */
/*       - - 0 = Upper 512K failed                                                                */
/* - 6:  - -Enable RAM, 90000-9FFFF                                                               */
/* - 5:  - -Enable RAM, 80000-8FFFF                                                               */
/* - 4:  - -Enable RAM, 70000-7FFFF                                                               */
/* - 3:  - -Enable RAM, 60000-6FFFF                                                               */
/* - 2:  - -Enable RAM, 50000-5FFFF                                                               */
/* - 1:  - -Enable RAM, 40000-4FFFF                                                               */
/* - 0:  - Remap Low Memory                                                                       */
/* ---------------------------------------------------------------------------------------------- */
/* Figure 1-11. System Board RAM Control/Status Register:                                         */
/* ---------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------- */
/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; Keyboard and Pointing     */
/* Device Controller:                                                                             */
/* ---------------------------------------------------------------------------------------------- */
/* The interface logic for the keyboard and the pointing device is the same allowing the keyboard */
/* and pointing device to plug into either of the two 6-pin connectors at the rear of the system. */
/*                                                                                                */
/* The interface receives the serial data and checks the parity. The data is then presented to the*/
/* system at the interface's output buffer, I/O port hex 60.                                      */
/* ---------------------------------------------------------------------------------------------- */
/* void ibmps2_set_video_mode (ibmpc_t *pc, unsigned mode)
{
	if (pc->model & PCE_IBMPS2_MODEL25) {
		pc->ibmps2_config[1] &= 0xcf;
		pc->ibmps2_config[1] |= (mode & 3) << 4;
	}
} */

/* void pc_setup_ibmps2(ibmpc_t *pc, ini_sct_t *ini)
{
	unsigned  sw0, sw1;
	ini_sct_t *sct;
	pc->ibmps2_config[0] = 0;
	pc->ibmps2_config[1] = 0;
	if ((pc->model & PCE_IBMPS2_MODEL25) == 0) {
		return;
	}
	sct = ini_next_sct (ini, NULL, "m24");
	if (sct == NULL) {
		return;
	}
	ini_get_uint16 (sct, "switches0", &sw0, 0x00);
	ini_get_uint16 (sct, "switches1", &sw1, 0x64);
	pce_log_tag (MSG_INF, "M24:",
		"switches0=0x%02X switches1=0x%02X\n", sw0, sw1
	);
	pc->m24_config[0] = sw0;
	pc->m24_config[1] = sw1;
} */
/* ---------------------------------------------------------------------------------------------- */
/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; I/O Channel;              */
/* ---------------------------------------------------------------------------------------------- */
/* The I/O channel is an extension of the 8086-2 microprocessor bus that is demultiplexed,        */
/* repowered, and enhanced by the addition of inter-rupts and DMA functions.                      */
/*                                                                                                */
/* The I/O channel contains:                                                                      */
/*  - An 8-bit, bidirectional data bus                                                            */
/*  - Twenty address lines                                                                        */
/*  - Six levels of interrupts                                                                    */
/*  - Control lines for memory and I/O read and write                                             */
/*  - Clock and timing lines                                                                      */
/*  - Three channels of DMA control lines                                                         */
/*  - Memory-refresh control lines                                                                */
/*  - A channel check line                                                                        */
/*  - Power and ground for the adapters.                                                          */
/* Four voltage levels are provided for I/O cards. The maximum avail-able values (for each slot)  */
/* in the following chart are for systems with two diskette drives.                               */
/*  - + 5 V dc (+ 5%, - 3%) at 1.9  A maximum                                                     */
/*  - - 5 V dc (+10%, - 8%) at 0.055A maximum                                                     */
/*  - +12 V dc (+ 5%, - 3%) at 0.72 A maximum                                                     */
/*  - -12 V dc (+10%, - 8%) at 0.117A maximum                                                     */
/*The maximum available values (for each slot) in the following chart are for systems with a fixed*/
/* disk and a diskette drive.                                                                     */
/*  - + 5 V dc (+ 5%, - 3%) at 1.8  A maximum                                                     */
/*  - - 5 V dc (+10%, - 8%) at 0.055A maximum                                                     */
/*  - +12 V dc (+ 5%, - 3%) at 0.33 A maximum                                                     */
/*  - -12 V dc (+10%, - 8%) at 0.117A maximum                                                     */
/* The 'I/O CH RDY' line is available on the I/O channel to allow opera-tion with slow I/O or     */
/* memory devices. I/O CH RDY is made inactive by an addressed device to lengthen the operation.  */
/* For each clock cycle that the line is held low, one wait state is added to the I/O and DMA     */
/* operations.                                                                                    */
/*                                                                                                */
/* I/O devices are addressed using mapped I/O address space. The channel is designed so that over */
/* 64,000 device addresses are available to the adapters on the I/O channel.                      */
/*                                                                                                */
/* The following is the I/O address map for the IBM Personal System/2 Model 25. Hex 0100 to FFFF  */
/* are available for use by adapters on the I/O channel, except for those addresses noted.        */
/* ---------------------------------------------------------------------------------------------- */
/* Hex Range:    Device:                                                                          */
/*  - 0000-001F : - DMA Controller, 8237A-5                                                       */
/*  - 0020-003F : - Interrupt Controller                                                          */
/*  - 0040-005F : - Timer                                                                         */
/*  - 0060-0062 : - I/O Ports                                                                     */
/*  - 0063-006F : - System Board/Control and Status                                               */
/*                                                                                                */
/*  - 0080-008F : - DMA Page Registers                                                            */
/*  - 00A0-00AF*: - Interrupt Controller Extension                                                */
/*                                                                                                */
/*  - 0320-032F : - Fixed Disk                                                                    */
/*  - 0378-037F : - Parallel Port                                                                 */
/*  - 03C0-03DF : - Video Subsystem                                                               */
/*  - 03F0-03F7 : - Diskette                                                                      */
/*  - 03F*-03FF : - Serial Port                                                                   */
/* NOTE: I/O Addresses hex 000 to 0FF, are reserved for the system board I/O.                     */
/* *The NMI mask can be set and reset through system software as follows:                         */
/*  - Write hex 80 to I/O address hex A0 ( enable NMI)                                            */
/*  - Write hex 00 to I/O address hex A0 (disable NMI)                                            */
/* ---------------------------------------------------------------------------------------------- */
/* Figure 1-12. I/O Address Map:                                                                  */
/* ---------------------------------------------------------------------------------------------- */
/* The 'I/O channel check' signal (-I/O CH CK) causes an NMI to the microprocessor.               */
/* ---------------------------------------------------------------------------------------------- */
/* Connectors:                                                                                    */
/* ---------------------------------------------------------------------------------------------- */
/* The I/O channel is repowered to provide sufficient power for both 62-pin connectors, assuming  */
/* two low-power Schottky (LS) loads per slot. IBM adapters typically use only one load per       */
/* adapter.                                                                                       */
/*                                                                                                */
/* The following figure shows the pin numbering and signal assignments for the I/O channel        */
/* connectors:                                                                                    */
/* ---------------------------------------------------------------------------------------------- */
/* ===================== - Ground     : B 1: Rear Panel : A 1: -I/O CH CK - ===================== */
/* ===================== - RESET DRV  : B 2: Rear Panel : A 2: D 7        - ===================== */
/* ===================== - + 5 V      : B 3: Rear Panel : A 3: D 6        - ===================== */
/* ===================== - IRQ 2      : B 4: Rear Panel : A 4: D 5        - ===================== */
/* ===================== - - 5 V      : B 5: Rear Panel : A 5: D 4        - ===================== */
/* ===================== - DRQ 2      : B 6: Rear Panel : A 6: D 3        - ===================== */
/* ===================== - -12 V      : B 7: Rear Panel : A 7: D 2        - ===================== */
/* ===================== - Reserved   : B 8: Rear Panel : A 8: D 1        - ===================== */
/* ===================== - +12 V      : B 9: Rear Panel : A 9: D 0        - ===================== */
/* ===================== - Ground     : B10: Rear Panel : A10: I/O CH RDY - ===================== */
/* ===================== - -MEMW      : B11: Rear Panel : A11: AEN        - ===================== */
/* ===================== - -MEMR      : B12: Rear Panel : A12: A19        - ===================== */
/* ===================== - -IOW       : B13: Rear Panel : A13: A18        - ===================== */
/* ===================== - -IOR       : B14: Rear Panel : A14: A17        - ===================== */
/* ===================== - -DACK3     : B15: Rear Panel : A15: A16        - ===================== */
/* ===================== - DRQ 3      : B16: Rear Panel : A16: A15        - ===================== */
/* ===================== - -DACK1     : B17: Rear Panel : A17: A14        - ===================== */
/* ===================== - DRQ 1      : B18: Rear Panel : A18: A13        - ===================== */
/* ===================== - -MREF      : B19: Rear Panel : A19: A12        - ===================== */
/* ===================== - CLK        : B20: Rear Panel : A20: A11        - ===================== */
/* ===================== - IRQ 7      : B21: Rear Panel : A21: A10        - ===================== */
/* ===================== - IRQ 6      : B22: Rear Panel : A22: A 9        - ===================== */
/* ===================== - IRQ 5      : B23: Rear Panel : A23: A 8        - ===================== */
/* ===================== - IRQ 4      : B24: Rear Panel : A24: A 7        - ===================== */
/* ===================== - IRQ 3      : B25: Rear Panel : A25: A 6        - ===================== */
/* ===================== - -DACK2     : B26: Rear Panel : A26: A 5        - ===================== */
/* ===================== - TC         : B27: Rear Panel : A27: A 4        - ===================== */
/* ===================== - ALE        : B28: Rear Panel : A28: A 3        - ===================== */
/* ===================== - + 5 V      : B29: Rear Panel : A29: A 2        - ===================== */
/* ===================== - OSC        : B30: Rear Panel : A30: A 1        - ===================== */
/* ===================== - Ground     : B31: Rear Panel : A31: A 0        - ===================== */
/* ---------------------------------------------------------------------------------------------- */
/* Figure 1-13. I/O Channel:                                                                      */
/* ---------------------------------------------------------------------------------------------- */
/* Signal Description:                                                                            */
/* ---------------------------------------------------------------------------------------------- */
/* The following is a description of the I/O channel signal lines. All lines are TTL-compatible.  */
/* The (O), (I), or (I/O) notation refers to output, input, or input and output.                  */
/*  - A0-A19 (O): Address bits 0 to 19: These lines are used to address memory and I/O devices    */
/*  within the system. The 20 address lines allow access to 1M of address space. Only the lower 16*/
/*  lines are used in I/O addressing, and all 16 should b decoded by I/O devices. A0 is the least */
/*  significant and A19 is the most significant. These lines are generated by either the          */
/*  microprocessor or the DMA controller.                                                         */
/*  - AEN (O): Address Enable: This line is used to de-gate the micro-processor and other devices */
/*  from the I/O channel to allow DMA trans-fers to take place. When this line is active, the DMA */
/*  controller has control of the address bus, data bus, and Read and Write command lines. When   */
/*  this line is inactive, the microprocessor has control. This line should be part of the adapter*/
/*  -select decode to prevent incor-rect adapter selects during DMA operations.                   */
/*  - ALE (O): Address Latch Enable: This line is provided by the bus controller and is used on   */
/*  the system board to latch valid addresses from the microprocessor. Addresses are valid at the */
/*  falling edge of ALE and are latched onto the bus while ALE is inactive. This signal is forced */
/*  active during DMA cycles.                                                                     */
/*  - CLK (O): System clock: This is the system clock signal with a fre-quency of 8 MHz and a 33% */
/*  duty cycle.                                                                                   */
/*  - D0-D7 (I/O): Data bits 0 to 7: These lines provide data bus bits 0 to 7 for the             */
/*  microprocessor, memory, and I/O devices.                                                      */
/*  - -DACK1 - -DACK3 (O): -DMA Acknowledge 1 to 3: These lines are used by the controller to     */
/*  acknowledge DMA requests. DACK0 is not available on the Type 8525's I/O channel.              */
/*  - DRQ1-DRQ3 (I): DMA Request 1 to 3: These lines are asynchro-nous channel requests used by   */
/*  peripheral devices to gain DMA ser-vices. They are prioritized with DRQ1 being the highest and*/
/*  DRQ3 being the lowest. A request is generated by bringing a request line to an active level. A*/
/*  request line is held active until the corresponding acknowledge line goes active.             */
/*  - -I/O CH CK (I): -I/O Channel Check: This line generates an NMI. It is a driven active to    */
/*  indicate an uncorrectable error and held active for at least two clock cycles.                */
/*  - I/O CH RDY (I): I/O Channel Ready: This line, normally active (ready), is pulled inactive   */
/*  (not ready) by a memory or I/O device to lengthen I/O or memory cycles. It allows slower      */
/*  devices to attach to the I/O channel with a minimum of difficulty. Any slow device using this */
/*  line should drive it inactive immediately after detecting a valid address and a Read or Write */
/* command. For every clock cycle this line is inactive, one wait state is added. This line should*/
/*  not be held inactive longer than 17 clock cycles.                                             */
/*  - -IOR (O): -I/O Read: This command line instructs an I/O device to drive its data onto the   */
/*  data bus. This signal is driven by the micro-processor or the DMA controller.                 */
/*  - -IOW (O): -I/O Write: This command line instructs an I/O device to read the data on the data*/
/*  bus. This signal is driven by the micro-processor or the DMA controller.                      */
/*  - IRQ2-IRQ7 (I): Interrupt requests 2 through 7: These lines are used to signal the           */
/*  microprocessor that an I/O device requires atten-tion. They are prioritized with IRQ2 as the  */
/*  highest priority and IRQ7 as the lowest. When an interrupt is generated, the request line is  */
/*  held active until it is acknowledged by the microprocessor.                                   */
/*  - -MEMR (O): -Memory Read: This command line instructs memory to drive its data onto the data */
/*  bus. This signal is driven by the micro-processor or the DMA controller.                      */
/*  - -MEMW (O): -Memory Write: This command line instructs memory to store the data present on   */
/*  the data bus. This signal is driven by the microprocessor or the DMA controller.              */
/*  - -MREF (I/O): -Memory Refresh: This line indicates a refresh cycle.                          */
/*  - OSC (O): Oscillator: This is a high-speed clock with a 70-ns period (14.31818 MHz). It has a*/
/*  50% duty cycle.                                                                               */
/*  - RESET DRV (O): Reset Drive: This line is used to reset or initialize systtem logic upon     */
/*  power-up or during a low-line voltage. This signal is synchronized to the falling edge of CLK.*/
/*  - TC (O): Terminal Count: This line provides a pulse when the ter-minal count for any DMA     */
/*  channel is reached.                                                                           */
/* ---------------------------------------------------------------------------------------------- */
/* Signal Description:                                                                            */
/* ---------------------------------------------------------------------------------------------- */
/* The following diagrams show the I/O signal timings for I/O and memory operations.              */
/* ---------------------------------------------------------------------------------------------- */
/* Symbol: Description:                               Min (ns): Max (ns):                         */
/*  - t 1:  - Address valid to ALE inactive:           -  20     -                                */
/*  - t 2:  - ALE inactive to Command active:          -  60:    -                                */
/*  - t 3:  - Command active from AEN inactive:        -  95:    -                                */
/*  - t 4:  - Command pulse width:                     - 605:    -                                */
/*  - t 5:  - Address hold from Command inactive:      -  45:    -                                */
/*  - t 6:  - Data valid from Read active:             -         - 540:                           */
/*  - t 7:  - Data hold from Read inactive:            -   0:                                     */
/*  - t 8:  - Data vlid from Write active:             -         - 120:                           */
/*  - t 9:  - Data hold from Write inactive:           -  25:                                     */
/*  - t10:  - I/O CH RDY inactive from Command active: -         - 325:                           */
/*  - t11:  - Read Data valid from I/O CH RDY active:  -         -   0:                           */
/*  - t12:  - Command inactive from I/O CH RDY active: - 160:    -                                */
/* ---------------------------------------------------------------------------------------------- */
/* Figure 1-14. 8-Bit I/O Timing:                                                                 */
/* ---------------------------------------------------------------------------------------------- */
/* Symbol: Description:                               Min (ns): Max (ns):                         */
/*  - t 1:  - Address valid to ALE inactive:           -  20     -                                */
/*  - t 2:  - ALE inactive to Command active:          -  60:    -                                */
/*  - t 3:  - Command active from AEN inactive:        -  95:    -                                */
/*  - t 4:  - Command pulse width:                     - 395:    -                                */
/*  - t 5:  - Address hold from Command inactive:      -  45:    -                                */
/*  - t 6:  - Data valid from Read active:             -         - 315:                           */
/*  - t 7:  - Data hold from Read inactive:            -   0:                                     */
/*  - t 8:  - Data vlid from Write active:             -         - 120:                           */
/*  - t 9:  - Data hold from Write inactive:           -  25:                                     */
/*  - t10:  - I/O CH RDY inactive from Command active: -         - 115:                           */
/*  - t11:  - Read Data valid from I/O CH RDY active:  -         -   0:                           */
/*  - t12:  - Command inactive from I/O CH RDY active: - 160:    -                                */
/* ---------------------------------------------------------------------------------------------- */
/* Figure 1-15. 8-Bit Memory Timing:                                                              */
/* ---------------------------------------------------------------------------------------------- */
/* Symbol: Description:                               Min (ns): Max (ns):                         */
/*  - t 1:  - Address valid to ALE inactive:           -  20     -                                */
/*  - t 2:  - ALE inactive to Command active:          -  60:    -                                */
/*  - t 3:  - Command active from AEN inactive:        -  95:    -                                */
/*  - t 4:  - Command pulse width:                     - 605:    -                                */
/*  - t 5:  - Address hold from Command inactive:      -  45:    -                                */
/*  - t 6:  - Data valid from Read active:             -         - 540:                           */
/*  - t 7:  - Data hold from Read inactive:            -   0:                                     */
/*  - t 8:  - Data vlid from Write active:             -         - 120:                           */
/*  - t 9:  - Data hold from Write inactive:           -  25:                                     */
/*  - t10:  - I/O CH RDY inactive from Command active: -         - 325:                           */
/*  - t11:  - Read Data valid from I/O CH RDY active:  -         -   0:                           */
/*  - t12:  - Command inactive from I/O CH RDY active: - 160:    -                                */
/* ---------------------------------------------------------------------------------------------- */
/* Figure 1-16. 16-Bit I/O Timing:                                                                */
/* ---------------------------------------------------------------------------------------------- */
/* Symbol: Description:                               Min (ns): Max (ns):                         */
/*  - t 1:  - Address valid to ALE inactive:           -  20     -                                */
/*  - t 2:  - ALE inactive to Command active:          -  60:    -                                */
/*  - t 3:  - Command active from AEN inactive:        -  95:    -                                */
/*  - t 4:  - Command pulse width:                     - 395:    -                                */
/*  - t 5:  - Address hold from Command inactive:      -  45:    -                                */
/*  - t 6:  - Data valid from Read active:             -         - 315:                           */
/*  - t 7:  - Data hold from Read inactive:            -   0:                                     */
/*  - t 8:  - Data vlid from Write active:             -         - 120:                           */
/*  - t 9:  - Data hold from Write inactive:           -  25:                                     */
/*  - t10:  - I/O CH RDY inactive from Command active: -         - 115:                           */
/*  - t11:  - Read Data valid from I/O CH RDY active:  -         -   0:                           */
/*  - t12:  - Command inactive from I/O CH RDY active: - 160:    -                                */
/* ---------------------------------------------------------------------------------------------- */
/* Figure 1-17. 16-Bit Memory Timing:                                                             */
/* ---------------------------------------------------------------------------------------------- */
/* Symbol: Description:                               Min (ns): Max (ns):                         */
/*  - t 1:  - -MREF active to -MEMR active:            - 155     -                                */
/*  - t 2:  - Address valid to -MEMR active:           -  75:    -                                */
/*  - t 3:  - -MEMR pulse width:                       - 230:    -                                */
/*  - t 4:  - -MEMR inactive to -MREF inactive:        -  10:    -                                */
/*  - t 5:  - -MEMR active to -MREF inactive:          -  45:    -  60                            */
/*  - t 6:  - I/O CH RDY pulse width:                  -         - 600:                           */
/*  - t 7:  - -MEMR inactive from I/O CH RDY active:   -   0:                                     */
/* ---------------------------------------------------------------------------------------------- */
/* Figure 1-18. Memory Refresh Timing:                                                            */
/* ---------------------------------------------------------------------------------------------- */
/* Symbol: Description:                               Min (ns): Max (ns):                         */
/*  - t 1:  - -DACK active to -DRQ inactive:           -   0:    -                                */
/*  - t 2:  - -DACK active to -IOW inactive:           - 200:    -                                */
/*  - t 3:  - -IOW inactive to -DACK inactive:         -   0:    -                                */
/*  - t 4:  - -IOW pulse width:                        - 250:    -                                */
/*  - t 5:  - -AEN active to -IOW active:              - 500:    -                                */
/*  - t 6:  - -IOW inactive to AEN inactive:           -  25:    -                                */
/*  - t 7:  - -IOW active from -MEMR active:           -           360:                           */
/*  - t 8:  - -IOW inactive to -MEMR inactive:         -   0:                                     */
/*  - t 9:  - Address valid to -MEMR active:           -   0:                                     */
/*  - t10:  - -MEMR pulse width:                       - 470:                                     */
/*  - t11:  - -MEMR active to I/O CH RDY inactive:     -           200:                           */
/*  - t12:  - -MEMR inactive from I/O CH RDY active:   - 200:                                     */
/*  - t13:  - TC active setup to -IOW inactive:        - 290:                                     */
/*  - t14:  - TC inactive from -IOW inactive:          -   0:                                     */
/* ---------------------------------------------------------------------------------------------- */
/* Figure 1-19. DMA Read Timing:                                                                  */
/* ---------------------------------------------------------------------------------------------- */
/* Symbol: Description:                               Min (ns): Max (ns):                         */
/*  - t 1:  - -DACK active to -DRQ inactive:           -   0:    -                                */
/*  - t 2:  - -DACK active to -IOW inactive:           -   0:    -                                */
/*  - t 3:  - -IOW inactive to -DACK inactive:         -   0:    -                                */
/*  - t 4:  - -IOW pulse width:                        - 470:    -                                */
/*  - t 5:  - -AEN active to -IOW active:              - 300:    -                                */
/*  - t 6:  - -IOW inactive to AEN inactive:           -   0:    -                                */
/*  - t 7:  - -IOW active from -MEMR active:           -  55:                                     */
/*  - t 8:  - -IOW inactive to -MEMR inactive:         -   0:                                     */
/*  - t 9:  - Address valid to -MEMR active:           - 140:                                     */
/*  - t10:  - -MEMR pulse width:                       - 250:                                     */
/*  - t11:  - -MEMR active to I/O CH RDY inactive:     -            30:                           */
/*  - t12:  - -MEMR inactive from I/O CH RDY active:   - 200:                                     */
/*  - t13:  - TC active setup to -IOW inactive:        - 290:                                     */
/*  - t14:  - TC inactive from -IOW inactive:          -   0:                                     */
/* ---------------------------------------------------------------------------------------------- */
/* Figure 1-20. DMA Write Timing:                                                                 */
/* ---------------------------------------------------------------------------------------------- */


