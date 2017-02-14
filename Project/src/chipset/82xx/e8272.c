/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/chipset/82xx/e8272.c                                     *
 * Created:     2005-03-06 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2005-2016 Hampa Hug <hampa@hampa.ch>                     *
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


/*
 * FDC 8272 emulator
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "e8272.h"

#ifndef E8272_DEBUG
#define E8272_DEBUG 0
#endif
/* ---------------------------------------------------------------------------------------------- */
/* NEC uPD765: Quote from "SINGLE/DOUBLE DENSITY FLOPPY DISK CONTROLLER"; Description:            */
/* ---------------------------------------------------------------------------------------------- */
/*The uPD765 is an LSI Floppy Disk Controller (FDC) Chip, which contains the circuitry and control*/
/* functions for interfacing a processor to 4 Floppy Disk Drives. It is capable of supporting     */
/* either IBM 3740 single density format (FM), or IBM System 34 Double Density format (MFM)       */
/* including double sided recording. The uPD765 provides control signals which simplify the design*/
/* of an external phase locked loop, and write precompensation circuitry. The FDC simplifies and  */
/* handles most of the burdons associated with implementing a Floppy Disk Interface.              */
/*                                                                                                */
/* Hand-shaking signals are provided in the uPD765 which make DMA operation easy to incorporate   */
/* with the aid of an external DMA Controller chip, such as the uPD8257. The FDC will operate in  */
/* either DMA or Non-DMA mode. In the Non-DMA mode, the FDC generates interrupts to the processor */
/* every time a data byte is available. In the DMA mode, the processor need only load the command */
/* into the FDC and all data transfers occur under control of the uPD765 and DMA controller.      */
/*                                                                                                */
/* There are 15 separate commands which the uPD765 will execute. Each of these commands require   */
/* multiple 8-bit bytes to fully specify the operation which the processor wishes the FDC to      */
/* perform. The following commands are available:                                                 */
/*  - Read Data         - Scan High or Equal - Write Deleted Data                                 */
/*  - Read ID           - Scan  Low or Equal - Seek                                               */
/*  - Read Deleted Data - Specify            - Recalibrate (Restore to Track 0)                   */
/*  - Read a Track      - Write Data         - Sense Interrupt Status                             */
/*  - Scan Equal        - Format a Track     - Sense   Drive   Status                             */
/* ---------------------------------------------------------------------------------------------- */
/* INTERNAL REGISTERS:                                                                            */
/* ---------------------------------------------------------------------------------------------- */
/* The uPD765 contains two registers which may be accessed by the main system proces-sor; a Status*/
/* Register and a Data Register. The 8-bit Main Status Register contains the status information of*/
/* the FDC, and may be accessed at any time. The 8-bit Data Register (actually consists of several*/
/* registers in a stack with only one register pre-sented to the data bus at a time), which stores*/
/* data, commands, parameters, and FDD status information. Data bytes are read out of, or written */
/*into, the Data Register in order to program or or obtain the results after a particular command.*/
/* The Status Register may only be read and is used to facilitate the transfer of data between the*/
/* processor and uPD765.                                                                          */
/*                                                                                                */
/* The relationship between the Status/Data registers and the signals RD, WR, and A0 is shown     */
/* below.                                                                                         */
/* A0:  RD:  WR:  FUNCTION:                                                                       */
/*  - 0: - 0: - 1: - Read Status Register                                                         */
/*  - 0: - 1: - 0: - Illegal                                                                      */
/*  - 0: - 0: - 0: - Illegal                                                                      */
/*  - 1: - 0: - 0: - Illegal                                                                      */
/*  - 1: - 0: - 1: - Read  from Data Register                                                     */
/*  - 1: - 1: - 0: - Write Into Data Register                                                     */
/* ---------------------------------------------------------------------------------------------- */
/* PACKAGE OUTLINE uPD765C:                                                                       */
/* ---------------------------------------------------------------------------------------------- */
/* ITEM: MILLIMETERS:         INCHES:                                                             */
/*  - A:  - 51.5 MAX:          - 2.028 MAX                                                        */
/*  - B:  - 1.62:              - 0.064                                                            */
/*  - C:  - 2.54 +- 0.1:       - 0.10  +- 0.004                                                   */
/*  - D:  - 0.5  +- 0.1:       - 0.019 +- 0.004                                                   */
/*  - E:  - 48.26:             - 1.9                                                              */
/*  - F:  - 1.2  MIN:          - 0.047 MIN                                                        */
/*  - G:  - 2.54 MIN:          - 0.10  MIN                                                        */
/*  - H:  - 0.5  MIN:          - 0.19  MIN                                                        */
/*  - I:  - 5.22 MAX:          - 0.206 MAX                                                        */
/*  - J:  - 5.72 MAX:          - 0.225 MAX                                                        */
/*  - K:  - 15.24:             - 0.600                                                            */
/*  - L:  - 13.2:              - 0.520                                                            */
/*  - M:  - 0.25 + 0.1 - 0.05: - 0.010 + 0.004 - 0.002                                            */
/* ---------------------------------------------------------------------------------------------- */
/* NOTE: See included file for more details.                                                      */
/* ---------------------------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------------------------------- */
/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; Commands:                      */
/* --------------------------------------------------------------------------------------------------- */
/* The diskette controller performs the commands listed below. Each command is initiated by a multibyte*/
/* transfer from the microprocessor, and the result can also be a multibyte transfer back to the micro-*/
/* processor. Because of this multibyte interchange of information between the controller and the      */
/* microprocessor, each command is considered to consist of three phases:                              */
/*  - Command   Phase: The microprocessor issues a series of writes to the controller that direct it to*/
/*  the controller that direct it to perform a specific operation.                                     */
/*  - Execution Phase: The controller performs the specified operation.                                */
/*  - Result    Phase: After completion of the operation, status and other housekeeping information are*/
/*  made available to the microprocessor through a sequence of Read commands from the microprocessor.  */
/*                                                                                                     */
/* The following is a list of controller commands:                                                     */
/*  - Read          Data                                                                               */
/*  - Read Deleted  Data                                                                               */
/*  - Read   a Track                                                                                   */
/*  - Read ID                                                                                          */
/*  - Write         Data                                                                               */
/*  - Write Deleted Data                                                                               */
/*  - Format a Track                                                                                   */
/*  - Scan         Equal                                                                               */
/*  - Scan Low  or Equal                                                                               */
/*  - Scan High or Equal                                                                               */
/*  - Recalibrate                                                                                      */
/*  - Sense Interrupt Status                                                                           */
/*  - Specify                                                                                          */
/*  - Sense   Drive   Status                                                                           */
/*  - Seek                                                                                             */
/* --------------------------------------------------------------------------------------------------- */
/* Symbol Descriptions: Following are descriptions of symbols used in the following section, "Command  */
/* Format."                                                                                            */
/* --------------------------------------------------------------------------------------------------- */
/*  - A 0  : Address Line 0--When clear, A0 selects the Main Status register; when set to 1, it selects*/
/*           the Data register.                                                                        */
/*  - DTL  : Data Length--When N is 00, DTL is the data length to be read from or written to a sector. */
/*  - EOT  : End of Track--The final sector number on a cylinder.                                      */
/*  - GPL  : Gap Length--The length of gap 3 (spacing between sectors excluding the VCO synchronous    */
/*           field).                                                                                   */
/*  - H    : Head Address--The head number, either 0 or 1, as speci-fied in the ID field               */
/*  - HD   : Head--The selected head number, 0 or 1. (H = HD in all command words.)                    */
/*  - HLT  : Head Load Time--The head load time in the selected drive (2 to 256 milliseconds in 2-     */
/*           millisecond increments).                                                                  */
/*  - HUT  : Head Unload Time--The head unload time after a read or write operation (0 to 240          */
/*           milliseconds in 16-millisecond increments).                                               */
/*  - MF   : FM or MFM Mode--A 0 selects FM mode and a 1 selects MFM (MFM is selected only if it is    */
/*           implemented).                                                                             */
/*  - MT   : Multitrack--A 1 selects multitrack operation. (Both HD0 and HD1 will be read or written.) */
/*  - N    : Number--The number of data bytes written in a sector.                                     */
/*  - NCN  : New Cylinder--The new cylinder number for a seek oper-ation.                              */
/*  - ND   : Nondata Mode--This indicates an operation in the nondata mode.                            */
/*  - PCN  : Present Cylinder Number--The cylinder number at the completion of a Sense Interrupt Status*/
/*           command (present position of the head).                                                   */
/*  - R    : Record--The sector number to be read or written.                                          */
/*  - SC   : Sector--The number of sectors per cylinder.                                               */
/*  - SK   : Skip--The skip deleted-data address mark.                                                 */
/*  - SRT  : Stepping Rate--These four bits indicate the stepping rate for the diskette drive as       */
/*           follows:                                                                                  */
/*            - 1111  1 ms                                                                             */
/*            - 1110  2 ms                                                                             */
/*            - 1101  3 ms                                                                             */
/*  - ST0-3: Status 0 through Status 3--The four registers that store status information after a       */
/*           command is executed.                                                                      */
/*  - STP  : Scan Test--If STP is 01, the data in adjacent sectors is compared with the data sent by   */
/*           the microprocessor during a scan operation. If STP is 02, alternate sectors are read and  */
/*           compared.                                                                                 */
/*  - US0-1: Unit Select--The selected driver number, encoded the same as bits 0 and 1 of the Digital  */
/*           Output register.                                                                          */
/* --------------------------------------------------------------------------------------------------- */
#define E8272_CMD_NONE               0  /* uPD765: None */
#define E8272_CMD_SPECIFY            1  /* uPD765: Specify */
#define E8272_CMD_SENSE_DRIVE_STATUS 2  /* uPD765: Sense Drive Status */
#define E8272_CMD_WRITE              3  /* uPD765: Write Data */
#define E8272_CMD_READ               4  /* uPD765: Read Data */
#define E8272_CMD_READ_TRACK         5  /* uPD765: Read a Track */
#define E8272_CMD_RECALIBRATE        6  /* uPD765: Recalibrate (Restore to Track 0) */
#define E8272_CMD_SENSE_INT_STATUS   7  /* uPD765: Sense Interrupt Status */
#define E8272_CMD_READ_ID            8  /* uPD765: Read ID */
#define E8272_CMD_FORMAT             9  /* uPD765: Format a Track */
#define E8272_CMD_SEEK               10 /* uPD765: Seek */

#define E8272_CMD0_MT  0x80
#define E8272_CMD0_MFM 0x40
#define E8272_CMD0_SK  0x20
/* ---------------------------------------------------------------------------------------------- */
/* INTERNAL REGISTERS CONT.):                                                                     */
/* ---------------------------------------------------------------------------------------------- */
/* The bits in the Main Status Register are defined as follows:                                   */
/* ---------------------------------------------------------------------------------------------- */
/* BIT NUMBER: NAME:                 SYMBOL: DESCRIPTION:                                         */
/*  - DB0:      - FDD 0 Busy        : - D0B:  - FDD number 0 is in the Seek mode.                 */
/*  - DB1:      - FDD 1 Busy        : - D1B:  - FDD number 1 is in the Seek mode.                 */
/*  - DB2:      - FDD 2 Busy        : - D2B:  - FDD number 2 is in the Seek mode.                 */
/*  - DB3:      - FDD 3 Busy        : - D3B:  - FDD number 3 is in the Seek mode.                 */
/*  - DB4:      - FDC   Busy        : - CB :  - A read or write command is in process.            */
/*  - DB5:      - Non-DMA mode      : - NDM:  - The FDC is in the non-DMA mode.                   */
/*  - DB6:      - Data Input/Output : - DIO:  - Indicates direction of data transfer between FDC  */
/*                                           and Data Register. If DIO = "1" then transfer is from*/
/*                                            Data Register to the Processor. If DIO = "0", then  */
/*                                            transfer is from the Processor to Data Register.    */
/*  - DB7:      - Request for Master: - RQM:  - Indicates Data Register is ready to send or       */
/*                                            receive data to or from the Processor. Both bits DIO*/
/*                                            and RQM should be used to perform the hand-shaking  */
/*                                          functions of "ready" and "direction" to the processor.*/
/* ---------------------------------------------------------------------------------------------- */
/* The DIO and RQM bits in the Status Register indicate when Data is ready and in which direction */
/* data will be transferred on the Data Bus.                                                      */
/* Data In/Out (DIO):        Out Processor and Into FDC: Out FDC and Into Processor:              */
/*                           ____________________________|           |________                    */
/* Request for Master (RQM): Ready:    ______   _________:    ______ :  :_____                    */
/*                     __    :   |_Not_|    |__|    :    |____|    |_:__|    :                    */
/*                     WR: __:__ :Ready:___ :__:____:____:____:____:_:__:__  :                    */
/*                     RD  __:_|-|_____:__|-|__:____:__  :____:___ __:__:__  :                    */
/*                           :___:_____:____:__:____:_|-|:____:__|-|_:__:____:                    */
/*                           |_A_|__B__|_A_|_B_|__A__|_C_|__D_|__C_|D|_B|__A_|                    */
/* ---------------------------------------------------------------------------------------------- */
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
#define E8272_MSR_D0B 0x01 /* Main Status Register: DB0: FDD 0 Busy:         D0B: */ /* IBM PS/2: Drive 0 Busy             */
#define E8272_MSR_D1B 0x02 /* Main Status Register: DB1: FDD 1 Busy:         D1B: */ /* IBM PS/2: Drive 1 Busy             */
#define E8272_MSR_D2B 0x04 /* Main Status Register: DB2: FDD 2 Busy:         D2B: */ /* IBM PS/2: Reserved                 */
#define E8272_MSR_D3B 0x08 /* Main Status Register: DB3: FDD 3 Busy:         D3B: */ /* IBM PS/2: Reserved                 */
#define E8272_MSR_CB  0x10 /* Main Status Register: DB4: FDC   Busy:         CB : */ /* IBM PS/2: Diskette Controller Busy */
#define E8272_MSR_NDM 0x20 /* Main Status Register: DB5: Non-DMA mode:       NDM: */ /* IBM PS/2: Non-DMA Mode             */
#define E8272_MSR_DIO 0x40 /* Main Status Register: DB6: Data Input/Output:  DIO: */ /* IBM PS/2: Data Input/Output        */
#define E8272_MSR_RQM 0x80 /* Main Status Register: DB7: Request for Master: RQM: */ /* IBM PS/2: Request for Master       */
/* --------------------------------------------------------------------------------------------------- */
/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; Digital Output Register:       */
/* --------------------------------------------------------------------------------------------------- */
/* The Digital Output register (DOR), hex 3F2, is a write-only register that controls drive motors,    */
/* drive selection, and feature enables. All bits are cleared by a reset.                              */
/* --------------------------------------------------------------------------------------------------- */
/* Bit:   Function:                                                                                    */
/*  - 7  : - Motor Enable 3                                                                            */
/*  - 6  : - Motor Enable 2                                                                            */
/*  - 5  : - Motor Enable 1                                                                            */
/*  - 4  : - Motor Enable 0                                                                            */
/*  - 3  : - DMA and Interrupt Enable                                                                  */
/*  - 2  : - -Controller Reset                                                                         */
/*  - 1,0: - Drive Select 0 through 3:                                                                 */
/*         - - 00 selects drive 0                                                                      */
/*         - - 01 selects drive 1                                                                      */
/*         - - 10 selects drive 2                                                                      */
/*         - - 11 selects drive 3                                                                      */
/* --------------------------------------------------------------------------------------------------- */
/* Figure 1-65. Digital Output, Hex 3F2:                                                               */
/* --------------------------------------------------------------------------------------------------- */
#define E8272_DOR_DSEL   0x01 /* Drive Select 0 through 3 */
#define E8272_DOR_RESET  0x04 /* -Controller Reset */
#define E8272_DOR_DMAEN  0x08 /* DMA and Interrupt Enable */
#define E8272_DOR_MOTEN0 0x10 /* Motor Enable 0/1 */
#define E8272_DOR_MOTEN1 0x20 /* Motor Enable 2/3 */
/* ---------------------------------------------------------------------------------------------- */
/* STATUS REGISTER IDENTIFICATION: STATUS REGISTER 0:                                             */
/* ---------------------------------------------------------------------------------------------- */
/* BIT NO.: BIT NAME:         BIT SYMBOL: DESCRIPTION:                                            */
/*  - D7/D6: - Interrupt Code: - IC:       - D7 = 0 and D6 = 0:   Normal Termination of Command,  */
/*                                         (NT). Com-mand was completed and properly executed.    */
/*  - D7/D6: - Interrupt Code: - IC:       - D7 = 0 and D6 = 1: Abnormal Termination of Command,  */
/*                                         (AT). Execution of Command was started, but was not    */
/*                                         successfully completed.                                */
/*  - D7/D6: - Interrupt Code: - IC:       - D7 = 1 and D6 = 0: Invalid Command issue, (IC).      */
/*                                         Command which was issued was never started.            */
/*  - D7/D6: - Interrupt Code: - IC:       - D7 = 1 and D6 = 1:Abnormal Termination because during*/
/*                                         command execution the ready signal from FDD changed    */
/*                                         state.                                                 */
/*  -    D5: - Seek End:       - SE:       - When the FDC completes the SEEK Command, this flag is*/
/*                                         set to 1 (high).                                       */
/*  -    D4: - Equipment Check:- EC:       - If a fault Signal is received from the FDD, or if the*/
/*                                         Track 0 Signal fails to occur after 77 Step Pulses     */
/*                                         (Recalibrate Command) then this flag is set.           */
/*  -    D3: - Not Ready:      - NR:       - When the FDD is in the not-ready state and a read or */
/*                                         write command is issued, this flag is set. If a read or*/
/*                                         write command is issued to Side 1 of a single sided    */
/*                                         drive, then this flag is set.                          */
/*  -    D2: - Head Address:   - HD:       - This  flag  is  used to indicate the state of the    */
/*                                         head at Interrupt.                                     */
/*  -    D1: - Unit Select 1:  - US 1:     - These flags are used to indicate a  Drive Unit Number*/
/*                                         at Interrupt                                           */
/*  -    D0: - Unit Select 0:  - US 0:     - These flags are used to indicate a  Drive Unit Number*/
/*                                         at Interrupt                                           */
/* ---------------------------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------------------------------- */
/* Quote from IBM Personal System/2 Technical Reference Manual: Command Status Registers:              */
/* --------------------------------------------------------------------------------------------------- */
/* The following are definitions of the status registers ST0 through ST3.                              */
/* --------------------------------------------------------------------------------------------------- */
/* Status 0 Register (ST0):                                                                            */
/* --------------------------------------------------------------------------------------------------- */
/* The following are bit definitions for the Status 0 register:                                        */
/*  - Bit 7, 6: Interrupt Code (IC):                                                                   */
/*               - 00: Normal Termination of Command--The command was completed and properly executed. */
/*               - 01: Abrupt Termination of Command--The execution of the command was started but not */
/*               successfully completed.                                                               */
/*               - 10: Invalid Command Issue--The issued command was never started.                    */
/*               - 11: Abnormal Termination--During the execution of a command, the 'ready' signal from*/
/*               the diskette drive changed state.                                                     */
/*  - Bit 5   : Seek End--Set to 1 when the controller completes the Seek command.                     */
/*  - Bit 4   : Equipment Check--Set if a 'fault' signal is received from the diskette drive, or if the*/
/*  'track 0' signal fails to occur after 77 step pulses (Recalibrate command).                        */
/*  - Bit 3   : Not Ready--This flag is set when the diskette drive is in the not-ready state and a    */
/*  Read or Write command is issued.                                                                   */
/*  - Bit 2   : Head Address--Indicates the state of the head at interrupt.                            */
/*  - Bit 1, 0: Unit select 1 and 0 (US 1 and 0)--Indicate a drive's unit number at interrupt.         */
/* --------------------------------------------------------------------------------------------------- */
#define E8272_ST0_IC 0xc0	/* D7/D6: Interrupt Code (IC) */ /* IBM PS/2: Bit 7, 6: Interrupt Code (IC) */
#define E8272_ST0_SE 0x20	/*    D5:    Seek End    (SE) */ /* IBM PS/2: Bit 5   : Seek End            */
/* ---------------------------------------------------------------------------------------------- */
/* STATUS REGISTER IDENTIFICATION: STATUS REGISTER 1:/SYSTEM REGISTER IDENTIFICATION (CONT.):     */
/* STATUS REGISTER 1:                                                                             */
/* ---------------------------------------------------------------------------------------------- */
/* BIT NO.: BIT NAME:         BIT SYMBOL: DESCRIPTION:                                            */
/*  -    D7: - End of Cylinder:- EN:       - When the FDC tries to access a Sector beyond the     */
/*                                         final Sector of a Cylinder, this flag is set.          */
/*  -    D6: -                 -           - Not used. This bit is always 0 (low).                */
/*  -    D5: - Data Error:     - DE:       - When the FDC detects a CRC error in either the ID    */
/*                                         field or the data field, this flag is set.             */
/*  -    D4: - Over Run:       - OR:       - If the FDC is not serviced by the main-systems during*/
/*                                         data transfers, within a certain time interval, this   */
/*                                         flag is set.                                           */
/*  -    D3: -                 -           - Not used. This bit is always 0 (low).                */
/*  -    D2: - No Data:        - ND:       - During execution of READ DATA, WRITE DELETED DATA or */
/*                                         SCAN Command, if FDC cannot find the Sector specified  */
/*                                         in the IDR Register, this flag is set.                 */
/*                                         - During executing the READ ID Command, if the FDC     */
/*                                         cannot read the ID field without an error, then this   */
/*                                         flag is set.                                           */
/*                                         - During the execution of the READ A Cylinder Command, */
/*                                         if the starting sector cannot be found, then this flag */
/*                                         is set.                                                */
/*  -    D1: - Not Writable:   - NW:       - During execution of WRITE DATA, WRITE DELETED DATA or*/
/*                                         Format A Cylinder Com-mand, if the FDC detects a write */
/*                                         protect signal from the FDD, then this flag is set.    */
/*  -    D0: - Missing Address - MA:       - If the FDC cannot detect the ID Address Mark after   */
/*                                         encountering the index hole twice, then this flag is   */
/*                                         set.                                                   */
/*                                         - If the FDC cannot detect the Data Address Mark or    */
/*                                        Deleted Data Address Mark, this flag is set. Also at the*/
/*                                         same time, the MD (Missing Address Mark in Data Field) */
/*                                         of Status Register 2 is set.                           */
/* ---------------------------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------------------------------- */
/* Quote from IBM Personal System/2 Technical Reference Manual: Command Status Registers:              */
/* --------------------------------------------------------------------------------------------------- */
/* The following are definitions of the status registers ST0 through ST3.                              */
/* --------------------------------------------------------------------------------------------------- */
/* Status 1 Register (ST1):                                                                            */
/* --------------------------------------------------------------------------------------------------- */
/* The following are bit definitions for the Status 1 register:                                        */
/*  - Bit 7   : End of Cylinder--Set when the controller tries to gain access to a sector beyond the   */
/*  final sector of a cylinder.                                                                        */
/*  - Bit 6   : Reserved.                                                                              */
/*  - Bit 5   : Dada Error--Set when the controller detects a CRC error in either the ID field or the  */
/*  data field.                                                                                        */
/*  - Bit 4   : Overrun--Set if the controller is not serviced by the main system within a certain time*/
/*  limit during data transfers.                                                                       */
/*  - Bit 3   : Reserved.                                                                              */
/*  - Bit 2   : No Data--Set if the controller cannot find the sector specified in the ID register     */
/*  during the execution of a Read Data, Write Deleted Data, or Scan command. This flag is also set if */
/*  the controller cannot read the ID field without an error during the execution of a Read ID command,*/
/*  or if the starting sector cannot be found during the execution of a Read Cylinder command.         */
/*  - Bit 1   : Not Writable--Set if the controller detects a 'write-protect' signal from the diskette */
/*  drive during execution of a Write Data, Write Deleted Data, or Format a Track command.             */
/*  - Bit 0   : Missing Address Mark--Set if the controller cannot detect the ID address mark. At the  */
/*  same time, bit 0 of the Status 2 register is set.                                                  */
/* --------------------------------------------------------------------------------------------------- */
#define E8272_ST1_EN 0x80	/*    D7: End of Cylinder      (EN)             */ /* IBM PS/2: Bit 7: End of Cylinder      */
/* -------------------- */  /*    D6: Not used. This bit is always 0 (low). */ /* IBM PS/2: Bit 6: Reserved.            */
#define E8272_ST1_DE 0x20	/*    D5: Data Error           (DE)             */ /* IBM PS/2: Bit 5: Data Error           */
#define E8272_ST1_OR 0x10	/*    D4: Over Run             (OR)             */ /* IBM PS/2: Bit 4: Overrun              */
/* -------------------- */  /*    D3: Not used. This bit is always 0 (low). */ /* IBM PS/2: Bit 3: Reserved.            */
#define E8272_ST1_ND 0x04	/*    D2: No Data              (ND)             */ /* IBM PS/2: Bit 2: No Data              */
#define E8272_ST1_NW 0x02	/*    D1: Not Writeable        (NW)             */ /* IBM PS/2: Bit 1: Not Writable         */
#define E8272_ST1_MA 0x01	/*    D0: Missing Address Mark (MA)             */ /* IBM PS/2: Bit 0: Missing Address Mark */
/* ---------------------------------------------------------------------------------------------- */
/* STATUS REGISTER IDENTIFICATION: STATUS REGISTER 1:/SYSTEM REGISTER IDENTIFICATION (CONT.):     */
/* STATUS REGISTER 2:                                                                             */
/* ---------------------------------------------------------------------------------------------- */
/* BIT NO.: BIT NAME:         BIT SYMBOL: DESCRIPTION:                                            */
/*  -    D7: -                 -           - Not used. This bit is always 0 (low).                */
/*  -    D6: - Control Mark:   - CM:       - During executing the READ DATA or SCAN Command, if   */
/*                                         the FDC encounters a Sector which contains a Deleted   */
/*                                         Data Address Mark, this flag is set.                   */
/*  -    D5: - Data Error in   - DD:       - If the FDC detects a CRC error in the data field then*/
/*              Data Field:                this flag is set.                                      */
/*                                                                                                */
/*  -    D4: - Wrong Cylinder: - WC:       - This bit is related with the ND bit, and when the    */
/*                                         contents of C on the medium is different from that     */
/*                                         stored in the IDR, this flag is set.                   */
/*  -    D3: - Scan Equal Hit: - SH:       - During execution, the SCAN Command, if the condition */
/*                                         of "equal" is satisfied, this flag is set.             */
/*  -    D2: -    Scan  Not    - SN:       - During executing the SCAN Command, if the FDC cannot */
/*                Satisfied:              find a Sector on the cylinder which meets the condition,*/
/*                                         then this flag is set.                                 */
/*  -    D1: - Bad Cylinder:   - BC:       - This bit is related with the ND bit, and when the    */
/*                                        content of C on the medium is different from that stored*/
/*                                        in the IDR and the content of C is FF, then this flag is*/
/*                                        set.                                                    */
/*  -    D0: - Missing Address - MD:       - When data is read from the medium, if the FDC cannot */
/*          Mark  in  Data  Field:         find a Data Address Mark or Deleted Address Mark, then */
/*                                         this flag is set.                                      */
/* ---------------------------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------------------------------- */
/* Quote from IBM Personal System/2 Technical Reference Manual: Command Status Registers:              */
/* --------------------------------------------------------------------------------------------------- */
/* The following are definitions of the status registers ST0 through ST3.                              */
/* --------------------------------------------------------------------------------------------------- */
/* Status 2 Register (ST2):                                                                            */
/* --------------------------------------------------------------------------------------------------- */
/* The following are bit definitions for the Status 2 register:                                        */
/*  - Bit 7   : Reserved = 0.                                                                          */
/*  - Bit 6   : Control Mark--This flag is set if the controller encounters a sector that has a deleted*/
/*  -data address mark during execution of a Read Data or Scan command.                                */
/*  - Bit 5   : Data      Error      in Data Field--Set if the controller detects an error in the data.*/
/*  - Bit 4   :Wrong Cylinder--This flag is related to ND and is set when the content of C is different*/
/*  from that stored in the ID reg-ister.                                                              */
/*  - Bit 3   : Scan Equal Hit (SH)--Set if the adjacent sector data equals the microprocessor data    */
/*  during the execution of a Scan command.                                                            */
/*  - Bit 2   : Scan Not Satisfied--Set if the controller cannot find a sector on the cylinder that    */
/*  meets the condition during a Scan command.                                                         */
/*  - Bit 1   : Bad  Cylinder--Related to ND and is set when the contents of C on the medium are       */
/*  different from that stored in the ID reg-ister or when the content of C is hex FF.                 */
/*  - Bit 0   : Missing Address Mark in Data Field--Set if the controller find a data address markor or*/
/*  a deleted-data address mark when data is read.                                                     */
/* --------------------------------------------------------------------------------------------------- */
/* -------------------- */  /*    D7: Not used. This bit is always 0 (low).   */ /* IBM PS/2: Bit 7: Reserved = 0.                      */
#define E8272_ST2_CM 0x40	/*    D6: Control Mark                       (CM) */ /* IBM PS/2: Bit 6: Control Mark                       */
#define E8272_ST2_DD 0x20	/*    D5: Data Error in Data Field           (DD) */ /* IBM PS/2: Bit 5: Data      Error      in Data Field */
#define E8272_ST2_WC 0x10	/*    D4: Wrong Cylinder                     (WC) */ /* IBM PS/2: Bit 4: Wrong Cylinder                     */
#define E8272_ST2_BC 0x02	/*    D1: Bad Cylinder (FF)                  (BC) */ /* IBM PS/2: Bit 1: Bad   Cylinder                     */
#define E8272_ST2_MD 0x01	/*    D0: Missing Address Mark in Data Field (MD) */ /* IBM PS/2: Bit 0: Missing Address Mark in Data Field */
/* ---------------------------------------------------------------------------------------------- */
/* STATUS REGISTER IDENTIFICATION: STATUS REGISTER 1:/SYSTEM REGISTER IDENTIFICATION (CONT.):     */
/* STATUS REGISTER 2:                                                                             */
/* ---------------------------------------------------------------------------------------------- */
/* BIT NO.: BIT NAME:         BIT SYMBOL: DESCRIPTION:                                            */
/*  -    D7: - Fault:          - FT:       - This bit is used to indicate the status of the Fault */
/*                                         signal from the FDD.                                   */
/*  -    D6: - Write Protected:- WP:       - This bit is used to indicate the status of the Write */
/*                                         Protectes signal from the FDD.                         */
/*  -    D5: - Ready:          - RY:       - This bit is used to indicate the status of the Ready */
/*                                         signal from the FDD.                                   */
/*  -    D4: - Track 0:        - T0:       - This bit is used to indicate the status of the Track */
/*                                         0 signal from the FDD.                                 */
/*  -    D3: - Two Side:       - TS:       - This bit is used to indicate the status of the Two   */
/*                                         Side signal from the FDD.                              */
/*  -    D2: - Head Address:   - HD:       - This bit is used to indicate the status of Side      */
/*                                         Select signal to the FDD.                              */
/*  -    D1: - Unit Select 1:  - US 1:     - This bit is used to indicate the status of the Unit  */
/*                                         Select 1 signal to the FDD.                            */
/*  -    D0: - Unit Select 0:  - US 0:     - This bit is used to indicate the status of the Unit  */
/*                                         Select 0 signal to the FDD.                            */
/* ---------------------------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------------------------------- */
/* Quote from IBM Personal System/2 Technical Reference Manual: Command Status Registers:              */
/* --------------------------------------------------------------------------------------------------- */
/* The following are definitions of the status registers ST0 through ST3.                              */
/* --------------------------------------------------------------------------------------------------- */
/* Status 3 Register (ST3):                                                                            */
/* --------------------------------------------------------------------------------------------------- */
/* The following are bit definitions for the Status 3 register:                                        */
/*  - Bit 7   : Fault        --Status of the 'fault'           signal from the diskette drive.         */
/*  - Bit 6   : Write Protect--Status of the '-write protect'  signal from the diskette drive.         */
/*  - Bit 5   : Ready        --Status of the 'ready'           signal from the diskette drive.         */
/*  - Bit 4   : Track 0      --Status of the 'track 0'         signal from the diskette drive.         */
/*  - Bit 3   : Two Side     --Status of the 'two side'        signal from the diskette drive.         */
/*  - Bit 2   : Head Address --Status of the '-head 1 select'  signal from the diskette drive.         */
/*  - Bit 1   : Unit Select 1--Status of the '-drive select 1' signal from the diskette drive.         */
/*  - Bit 0   : Unit Select 0--Status of the '-drive select 0' signal from the diskette drive.         */
/* --------------------------------------------------------------------------------------------------- */
#define E8272_ST3_FT  0x80	/*    D7: Fault            (FT)  */ /* IBM PS/2: Bit 7: Fault         */
#define E8272_ST3_WP  0x40	/*    D6: Write Protected  (WP)  */ /* IBM PS/2: Bit 6: Write Protect */
#define E8272_ST3_RDY 0x20	/*    D5: Ready            (RY)  */ /* IBM PS/2: Bit 5: Ready         */
#define E8272_ST3_T0  0x10	/*    D4: Track 0          (T0)  */ /* IBM PS/2: Bit 4: Track 0       */
#define E8272_ST3_TS  0x08	/*    D3: Two Side         (TS)  */ /* IBM PS/2: Bit 3: Two Side      */
#define E8272_ST3_HD  0x04	/*    D2: Head Address     (HD)  */ /* IBM PS/2: Bit 2: Head Address  */
#define E8272_ST3_US1 0x02	/*    D1: Unit Select 1   (US 1) */ /* IBM PS/2: Bit 1: Unit Select 1 */
#define E8272_ST3_US0 0x00	/*    D0: Unit Select 0   (US 0) */ /* IBM PS/2: Bit 0: Unit Select 0 */

#define E8272_RATE     250000 /* 250    Bits   Per Second (BPS) */
#define E8272_RATE_300 300000 /* 300    Bits   Per Second (BPS) */
#define E8272_RPS      5      /*   5 Rotations Per Second (RPS) */
/* --------------------------------------------------------------------------------------------------- */
/* Quote from IBM Personal System/2 Technical Reference Manual: Signal Description:                    */
/* --------------------------------------------------------------------------------------------------- */
/* All signals are 74HCT series-compatible in both rise and fall times and in interface levels. The    */
/*following are the input signals to the diskette drive. These signal thresholds are +1.0 V dc high and*/
/* +0.8 V dc low.                                                                                      */
/* --------------------------------------------------------------------------------------------------- */
/*  - -Drive Select 0--1: The select lines provide the means to enable or disable the drive interface  */
/*lines. When the signal is active, all control inputs are ignored, and the drive outputs are disabled.*/
/*  The maximum drive-select delay time is 500 ns.                                                     */
/*  - -Motor Enable 0--1: When this signal is made active, the spindle starts to turn. When it is made */
/*  inactive , the spindle slows to a stop.                                                            */
/*  - -Step: An inactive pulse on this line causes the head to move one track. The minimum pulse width */
/*  is 1 us. The direction of the head motion is determined by the state of the '-direction' signal at */
/*  the trailing edge of the '-step' pulse.                                                            */
/*  - -Direction: When this signal is active, the head moves to the next higher track (toward the      */
/*  spindle) for each '-step' pulse. When the signal is inactive, the head moves toward track 0. This  */
/*  signal must be stable for 1 us before and after the trailing edge of the '-step' pulse.            */
/*  - -Head 1 Select: When this signal is active, the upper head (head 1) is selected. When it is      */
/*  inactive, the lower head (head 0) is selected.                                                     */
/*  - -Write Enable: When this signal is active, the write-current circuits are enabled and data can be*/
/*  written under the control of the '-write data' signal. This signal must be active 8 us before data */
/*  can be written.                                                                                    */
/*  - -Write Data: An active pulse on this line writes a 1. These pulses have a 4-, 6-, or 8-us spacing*/
/*  with a width of 250 ns for the 250,000-bps transfer rate. Write precompensation of 125 ns is done  */
/*  by the diskette gate array.                                                                        */
/* --------------------------------------------------------------------------------------------------- */
/* The following are the output signals from the diskette drive. These signal thresholds are +3.7V dc  */
/* high and +0.4 V dc low.                                                                             */
/* --------------------------------------------------------------------------------------------------- */
/*  - -Index: An active pulse of 1 ms indicates the diskette index.                                    */
/*  - -Track 0: When this signal is active, the head is on track 0. This signal is used to determine   */
/*  whether the drive is present. If, after commanding the drive to seek track 0, the '-track 0' signal*/
/*  does not go active, the drive is not present.                                                      */
/*  - -Write Protect: This signal is active when the write-protect window is uncovered. When this      */
/*  happens, the write current circuits are disa-bled.                                                 */
/*  - -Read Data: An active pulse on this line writes a logical 1. The pulse width for the 250,000-bps */
/*  rate is 250 ns.                                                                                    */
/*  - -Diskette Change: This signal is active at power-on and whenever the diskette is removed. It     */
/*  remains active until a diskette is present and a '-step' pulse is received.                        */
/* --------------------------------------------------------------------------------------------------- */
/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; Connector:                     */
/* --------------------------------------------------------------------------------------------------- */
/* The following shows the signals and pin assignments for the con-nector.                             */
/* --------------------------------------------------------------------------------------------------- */
/* ====================== Pin:  I/O:   Signal:          Pin:     I/O:   Signal: ====================== */
/* ===================== -  1: - N/A: - Signal Ground: -  2:    - N/A: - Reserved===================== */
/* ===================== -  3: - N/A: - Signal Ground: -  4:    - N/A: - Reserved===================== */
/* ===================== -  5: - N/A: - Signal Ground: -  6:    - N/A: - Not Connected================ */
/* ===================== -  7: - N/A: - Signal Ground: -  8:    - I  : - -Index ====================== */
/* ===================== -  9: - N/A: - Signal Ground: - 10:    - O  : - -Motor   Enable 1============ */
/* ===================== - 11: - N/A: - Signal Ground: - 12:    - O  : - -Drive   Select 0============ */
/* ===================== - 13: - N/A: - Signal Ground: - 14:    - O  : - -Drive   Select 1============ */
/* ===================== - 15: - N/A: - Signal Ground: - 16:    - O  : - -Motor   Enable 0============ */
/* ===================== - 17: - N/A: - Signal Ground: - 18:    - O  : - -Direction=================== */
/* ===================== - 19: - N/A: - Signal Ground: - 20:    - O  : - -Step  ====================== */
/* ===================== - 21: - N/A: - Signal Ground: - 22:    - O  : - -Write   Data================ */
/* ===================== - 23: - N/A: - Signal Ground: - 24:    - O  : - -Write   Enable============== */
/* ===================== - 25: - N/A: - Signal Ground: - 26:    - I  : - -Track 0===================== */
/* ===================== - 27: - N/A: - Signal Ground: - 28:    - I  : - -Write   Protect============= */
/* ===================== - 29: - N/A: - Signal Ground: - 30:    - I  : - -Read    Data================ */
/* ===================== - 31: - N/A: - Signal Ground: - 32:    - O  : - -Head  1 Select============== */
/* ===================== - 33: - N/A: - Signal Ground: - 34:    - I  : - -Diskette Change============= */
/* ===================== - 35: - N/A: -        Ground: - 36:    - N/A: - Ground ====================== */
/* ===================== - 37: - N/A: -        Ground: - 38:    - O  : - + 5 V dc===================== */
/* ===================== - 39: - N/A: -        Ground: - 40:    - O  : - +12 V dc===================== */
/* --------------------------------------------------------------------------------------------------- */
/* Figure 1-97. Diskette Drive Connector:                                                              */
/* --------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------- */
/* NEC uPD765: Quote from "SINGLE/DOUBLE DENSITY FLOPPY DISK CONTROLLER"; PIN CONFIGURATION:      */
/* ---------------------------------------------------------------------------------------------- */
/* ============================ - RESET:  1:| uPD765 |:40 :    VCC - ============================ */
/* ============================ - RD   :  2:| uPD765 |:39 :RW/SEEK - ============================ */
/* ============================ - WR   :  3:| uPD765 |:38 :LCT/DIR - ============================ */
/* ============================ - CS   :  4:| uPD765 |:37 : FR/STP - ============================ */
/* ============================ - A 0  :  5:| uPD765 |:36 :    HDL - ============================ */
/* ============================ - DB0  :  6:| uPD765 |:35 :    RDY - ============================ */
/* ============================ - DB1  :  7:| uPD765 |:34 :  WP/TS - ============================ */
/* ============================ - DB2  :  8:| uPD765 |:33 :FLT/TR0 - ============================ */
/* ============================ - DB3  :  9:| uPD765 |:32 :    PS0 - ============================ */
/* ============================ - DB4  : 10:| uPD765 |:31 :    PS1 - ============================ */
/* ============================ - DB5  : 11:| uPD765 |:30 :    WDA - ============================ */
/* ============================ - DB6  : 12:| uPD765 |:29 :    US0 - ============================ */
/* ============================ - DB7  : 13:| uPD765 |:28 :    US1 - ============================ */
/* ============================ - DR0  : 14:| uPD765 |:27 :     HD - ============================ */
/* ============================ - DACK : 15:| uPD765 |:26 :    MFM - ============================ */
/* ============================ -   TC : 16:| uPD765 |:25 :     WE - ============================ */
/* ============================ -  IDX : 17:| uPD765 |:24 :    VCC - ============================ */
/* ============================ -  INT : 18:| uPD765 |:23 :     RD - ============================ */
/* ============================ -  CLK : 19:| uPD765 |:22 :    RDW - ============================ */
/* ============================ -  GND : 20:| uPD765 |:21 :    WCK - ============================ */
/* ---------------------------------------------------------------------------------------------- */
/* BLOCK DIAGRAM:                                                                                 */
/* ---------------------------------------------------------------------------------------------- */
/*  - NOTE: See included file "Block Diagram" for more details.                                   */
/* ---------------------------------------------------------------------------------------------- */
/* ABSOLUTE MAXIMUM RATINGS*:                                                                     */
/* ---------------------------------------------------------------------------------------------- */
/*  - Operating Temperature :   0C to + 70C                                                       */
/*  - Storage   Temparature : -40C to +125C                                                       */
/*  - All Output Voltages   : -0.5 to +7 Volts                                                    */
/*  - All  Input Voltages   : -0.5 to +7 Volts                                                    */
/*  - Supply     Voltage VCC: -0.5 to +7 Volts                                                    */
/*  - Power Dissipation     : 1 Watt                                                              */
/* COMMENT: Stress above those listed under "Absolute Maximum Ratings" may cause permanent damage */
/* to the device. This is a stress rating only and functional operation of the device at these or */
/* any other conditions above those indicated in the operational sections of this specification is*/
/* not implied. Exposure to absolute maximum rating conditions for extended periods may affect    */
/* device reliability.                                                                            */
/* *Ta = 25C                                                                                      */
/* ---------------------------------------------------------------------------------------------- */
/* DC CHARCTERISTICS:                                                                             */
/* ---------------------------------------------------------------------------------------------- */
/* Ta = 0C to +70C; VCC = +5V += 5% unless otherwise specified                                    */
/* PARAMETER:             SYMBOL:      LIMITS (MIN) (TYP) (MAX): UNIT: TEST CONDITIONS:           */
/*  -  Input Low  Voltage: -  VIL:      -     -0.5:        0.8:  - V : -                          */
/*  -  Input High Voltage: -  VIH:      -      2.5:  VCC + 0.5:  - V : -                          */
/*  - Output Low  Voltage: -  VOL:      -                  0.45: - V : - IOL = 1.6 mA             */
/*  - Output High Voltage: -  VOH:      -      2.4:        VCC:  - V : - IOH = -150 uA for Data   */
/*                                                                     Bus IOH = -80 uA for other */
/*  -  Input Low  Voltage: -  VIL(<|>): -      0.5:        0.8:  - V : -                   outputs*/
/*  (CLK + WR Clock)                                                                              */
/*  -  Input High Voltage: -  VIH(<|>): -VCC - 0.75: VCC + 0.5:  - V : -                          */
/*  (CLK + WR Clock)                                                                              */
/*  - VCC Supply Current:  -  ICC:     -             85:  130:  - mA: -                           */
/*  - Input Load Current:  -  ILI:     -                   10:  - uA: - V IN = VCC                */
/*  (All Input Pins)                                      -10:  - uA: - V IN = 0V                 */
/*  - High Level Output    - ILOH:     -                   10:  - uA: - VOUT = VCC                */
/*  Leakage Current:                                                                              */
/*  - Low  Level Output    - ILOL:     -                  -10:  - uA: - VOUT = 0V                 */
/*  Leakage Current:                                                                              */
/* ---------------------------------------------------------------------------------------------- */
/* NOTE: (1) Typical values for Ta = 25C and nominal supply voltage.                              */
/* ---------------------------------------------------------------------------------------------- */
/* PIN IDENTIFICATION:                                                                            */
/* ---------------------------------------------------------------------------------------------- */
/* PIN NO.: PIN SYMBOL: PIN NAME: INPUT/OUTPUT: CONNECTION TO: FUNCTION:                          */
/*  -  1:    - RST:      - Reset:  -  Input:     - Processor:   - Places FDC in idle state.       */
/*                                                              Resets output lines to FDD to "0" */
/*                                                              (low).                            */
/*  -  2:    - RD :      - Read:   -  Input(1):  - Processor:   - Control signal for transfer of  */
/*                                                             data from FDC to Data Bus, when "0"*/
/*                                                              (low).                            */
/*  -  3:    - WR :      - Write:  -  Input(1):  - Processor:   - Control signal for transfer of  */
/*                                                              data to FDC via Data Bus, when "0"*/
/*                                                              (low).                            */
/*  -  4:    - CS :      - Chip    -  Input:     - Processor:   - IC selected when "0" (low),     */
/*                        Select:                               allowing RD and WR to be enabled. */
/*  -  5:    - A 0:      - Data/   -  Input(1):  - Processor:   - Selects Data Reg (A0=1) or      */
/*                   Status Reg Select:                         Status Reg (A0=0) contents of the */
/*                                                              FDC to be sent to Data Bus.       */
/*  -  6-13: - DB0-DB7:  - Data    -  Input(1)   - Processor:   - Bi-Directional 8-bit Data Bus.  */
/*                         Bus:       Output:                                                     */
/*  - 14:    - DRQ:      - Data    - Output:     - DMA:         - DMA Request is being made by FDC*/
/*                     DMA Request:                             when DRQ="1".                     */
/*  - 15:    - DACK:     - DMA     -  Input:     - DMA:         - DMA cycle is active when "0"    */
/*                     Acknowledge:                             (low) and Controller is performing*/
/*                                                              DMA transfer.                     */
/*  - 16:    - TC:       - Terminal-  Input:     - DMA:         - Indicates the termination of a  */
/*                                                              DMA trans-fer when "1" (high).    */
/*  - 17:    - IDX:      - Index:  -  Input:     - FDD:         - Indicates the beginning of a    */
/*                                                              disk track.                       */
/*  - 18:    - INT:      - Interrupt:-Output:    - Processor:   - Interrupt Request Generated by  */
/*                                                              FDC.                              */
/*  - 19:    - CLK:      - Clock:  -  Input:     -              - Single Phase 8 MHz Squarewave   */
/*                                                              Clock.                            */
/*  - 20:    - GND:      - Ground: -             -              - D.C. Power Return               */
/*  - 21:    - WCK:      - Write   -  Input:     -              - Write data rate to FDD. FM = 500*/
/*                         Clock:                               kHz, MFM = 1 MHz, with a pulse    */
/*                                                              width of 250 ns for both FM and   */
/*                                                              MFM.                              */
/*  - 22:    - RDW:      - Read    -  Input:     - Phase Lock   - Generated by PLL, and used to   */
/*                     Data Window:                   Loop:     sample data from FDD.             */
/*  - 23:    - RDD:      - Read    -  Input:     - FDD:         - Read data from FDD, containing  */
/*                                                              clock and data bits.              */
/*  - 24:    - VCO:      - VCO     - Output:     - Phase Lock   - Inhibits VCO in PLL when "0"    */
/*                        Sync:                       Loop:     (low), enablces VCO when "1".     */
/*  - 25:    - WE:       - Write   - Output:     - FDD:         - Enables write data into FDD.    */
/*                        Enable:                                                                 */
/*  - 26:    - MFM:      - MFM     - Output:     - Phase Lock   - MFM mode when "1", FM mode when */
/*                        Mode:                       Loop:     when "0".                         */
/*  - 27:    - HD:       - Head    - Output:     - FDD:         - Head 1 selected when "1" (high).*/
/*                                                              Head 0 selected when "0" (low).   */
/*  - 28, 29:- US1, US0: - Unit    - Output:     - FDD:         - FDD Unit Selected.              */
/*                        Select:                                                                 */
/*  - 30:    - WDA:      - Write   - Output:     - FDD:         - Serial clock and data bits to   */
/*                                                              FDD.                              */
/*  - 31, 32:- PS1, PS0: -         - Output:     - FDD:        - Write precompensation status     */
/*                                                             during MFM mode. Determines early  */
/*                                                             late, and normal times.            */
/*  - 33:    - FLT/TR0:  - Fault   -  Input:     - FDD:        - Senses FDD fault condition, in   */
/*                                                             Read/Write mode; and Track 0       */
/*                                                             condition in Seek mode.            */
/*  - 34:    - WP/TS:    - Write   -  Input:     - FDD:        - Sense Write Protect status in    */
/*                   Protect/Two-Side:                         Read/Write mode; and Two Side Media*/
/*                                                             in Seek mode.                      */
/*  - 35:    - RDY:       - Ready: -  Input:     - FDD:        - Indicates FDD is ready to send or*/
/*                                                             receive data.                      */
/*  - 36:    - HDL:       - Head   - Output:     - FDD:        - Command which causes read/write  */
/*                                                             head in FDD to contact diskette.   */
/*  - 37:    - FR/STP:    - Flt    - Output:     - FDD:         - Resets fault F.F. in FDD in Read*/
/*                                                              /Write mode, contains step pulses */
/*                                                              to move head to another cylinder  */
/*                                                              in Seek mode.                     */
/*  - 38:    - LCT/DIR:   - Low    - Output:     - FDD:         - Lowers Write current on inner   */
/*                                                              tracks in Read/Write mode,        */
/*                                                             determines direction head will step*/
/*                                                              in Seek mode.                     */
/*  - 39:    - RW/SEEK:   - Read   - Output:     - FDD:         - When "1" (high) Seek mode       */
/*                         Write/SEEK:                          selected and when "0" (low) Read/ */
/*                                                              Write mode selected.              */
/*  - 40:    - VCC:       - +5V:   -             -              - D.C. Power                      */
/* ---------------------------------------------------------------------------------------------- */
/* NOTE: <1> Disabled when CS = 1.                                                                */
/* ---------------------------------------------------------------------------------------------- */
/* AC CHARACTERISTICS:                                                                            */
/* ---------------------------------------------------------------------------------------------- */
/* PARAMETER:                    SYMBOL:      LIMITS (MIN) (TYP 1) (MAX): UNIT:  TEST CONDITIONS: */
/*  - Clock Period:               - <I>CY:     -      1.0:  8.0:    9.0:   - MHZ: -               */
/*  - Clock Active (High):        - <I>0:      -       35:                 -  ns: -               */
/*  - Clock Rise Time:            - <I>r:      -                     20:   -  ns: -               */
/*  - Clock Fall Time:            - <I>f:      -                     20:   -  ns: -               */
/*  - A0, CS, DACK Set Up Time to - TAR:       -       30:                 -  ns: -               */
/*  RD |v|:                                                                                       */
/*  - A0, CS, DACK Hold Time from - TRA:       -      5.0:                 -  ns: -               */
/*  RD |^|:                                                                                       */
/*  - RD Width:                   - TRR:       -      300:                 -  ns: -               */
/*  - Data    Access    Time from - TRD:       -                    200:   -  ns: -               */
/*  RD |v|:                                                                                       */
/*  - DB to Float Delay Time from - TDF:       -       20:          100:   -  ns: -               */
/*  RD |^|:                                                                                       */
/*  - A0, CS, DACK Set Up Time to - TAW:       -       50:                 -  ns: -               */
/*  WR |v|:                                                                                       */
/*  - A0, CS, DACK  Hold  Time to - TWA:       -       30:                 -  ns: -               */
/*  WR |^|:                                                                                       */
/*  - WR Width:                   - TWW:       -      300:                 -  ns: -               */
/*  - Data Set Up Time to WR |^|: - TDW:       -      250:                 -  ns: -               */
/*  - Data Hold Time from WR |^|: - TWD:       -       30:                 -  ns: -               */
/*  - INT Delay Time from RD |^|: - TRI:       -                    500:   -  ns: -               */
/*  - INT Delay Time from WR |^|: - TWI:       -                    500:   -  ns: -               */
/*  - DRQ Cycle Time:             - TMCY:      -       13:                 -  us: -               */
/*  - DRQ Delay Time from DACK|^|:- TAM:       -                    1.0:   -  us: -               */
/*  - WR or RD Response Time from - TRWM:      -      1.0:                 -  us: -               */
/*  DRQ|^|:                                                                                       */
/*  -  TC  Width:                 - TTC:       -      300:                 -  ns: -               */
/*  - Rest Width:                 - TRST:      -      3.0:                 -  us: -               */
/*  - WCK Cycle  Time:            - TCY:       -            2 or 4(2):     -  us: - MFM = 0       */
/*                                                          1 or 2   :     -  us: - MFM = 1       */
/*  - WCK Active Time (High):     - T0:        -      150:  250:    350:   -  ns: -               */
/*  - WCK  Rise  Time:            - Tr:        -                     30:   -  ns: -               */
/*  - WCK  Fall  Time:            - Tf:        -                     30:   -  ns: -               */
/*  - Pre-Shift Delay Time from   - TCP:       -       20:          150:   -  ns: -               */
/*  WCK |^|:                                                                                      */
/*  -    WDA    Delay Time from   - TCD:       -       20:          150:   -  ns: -               */
/*  WCK |^|:                                                                                      */
/*  - RDD Active Time (High):     - TRDD:      -      100:                 -  ns: -               */
/*  - Window Cycle Time:          - TWCY:      -            2.0:           -  us: - MFM = 0       */
/*                                                          1.0:           -  us: - MFM = 1       */
/*  - Window  Hold Time to/from   - TRDW:      -      100:                 -  ns: -               */
/*  RDD:                          - TWRD:      -      100:                 -  ns: -               */
/*  - US0, 1 Hold Time to RW/SEEK - TUS:       -       12:                 -  us: -               */
/*  |^|:                                                                                          */
/*  - SEEK/RW Hold Time to LOW    - TSD:       -        7:                 -  us: -               */
/*  CURRENT/DIRECTION |^|:                                                                        */
/*  - LOW CURRENT/DIRECTION Hold  - TDST:      -      1.0:                 -  us: - 8 MHz Clock   */
/*  Time to FAULT RESET/STEP |^|:                                                 Period          */
/*  - US0, 1 Hold Time from FAULT - TSTU:      -      1.0:                 -  us: -               */
/*  RESET/STEP |^|:                                                                               */
/*  - STEP Active Time (High):    - TSTP:             5.0:                 -  us: -               */
/*  - LOW CURRENT/DIRECTION Hold  - TSTD:             5.0:                 -  us: -               */
/*  Time form FAULT RESET/STEP |v|:                                                               */
/*  - STEP Cycle Time:            - TSC:              <3>:          <3>:   -  ms: -               */
/*  - FAULT RESET Active Time     - TFR:              8.0:           10:   -  us: - 8 MHz Clock   */
/*  (High):                                                                       Period          */
/* ---------------------------------------------------------------------------------------------- */
/* NOTES: <1> Typical values for Ta = 25C and nominal supply voltage.                             */
/*        <2> The former value of 2 and 1 are applied to Standard Floppy, and the latter value of */
/*        4 and 2 are applied to Mini-floppy                                                      */
/*        <3> Under Software Control. The range is from 1 ms to 16 ms at 8 MHz Clock Period, and 2*/
/*        to 32 ms at 4 MHz Clock Period.                                                         */
/* ---------------------------------------------------------------------------------------------- */
/* CAPACITANCE:                                                                                   */
/* ---------------------------------------------------------------------------------------------- */
/* PARAMETER:                  SYMBOL:      LIMITS (MIN) (TYP) (MAX): UNIT:  TEST CONDITIONS:     */
/*  - Clock  Input Capacitance: - C IN(<I>): -                   35:   - pF:  - All Pins Except   */
/*  -        Input Capacitance: - C IN:      -                   10:   - pF:  Pin Under Test Tied */
/*  -       Output Capacitance: - COUT:      -                   20:   - pF:  to AC Ground        */
/* ---------------------------------------------------------------------------------------------- */
/* TIMING WAVEFORMS:                                                                              */
/* ---------------------------------------------------------------------------------------------- */
/* NOTE: See included files for details.                                                          */
/* ---------------------------------------------------------------------------------------------- */
/* COMMAND SEQUENCE:                                                                              */
/* ---------------------------------------------------------------------------------------------- */
/* The uPD765 is capable of performing 15 different commands. Each command is initiated by a multi*/
/* -byte transfer from the processor, and the result after execution of the command may also be a */
/* multi-byte transfer back to the processor. Because of this multi-byte interchange of informa-  */
/* tion between the uPD765 and the processor, it is convenient to consider each command as        */
/* consistin of three phases:                                                                     */
/*  - Command   Phase: The FDC receives all information required to perform a particular operation*/
/* from the processor.                                                                            */
/*  - Execution Phase: The FDC performs the operation it was instructed to do.                    */
/*  - Result    Phase: After completion of the operation, status, and other housekeeping          */
/*  information are made available to the processor.                                              */
/* ---------------------------------------------------------------------------------------------- */
static void e8272_write_cmd (e8272_t *fdc, unsigned char val);

static
void e8272_drive_init (e8272_drive_t *drv, unsigned pd)
{
	drv->d = pd;
	drv->c = 0;
	drv->h = 0;
	drv->ok = 0;
	drv->sct_cnt = 0;
}

void e8272_init (e8272_t *fdc)
{
	/* 250 Bits Per Second (BPS) */
	unsigned i;
	fdc->disk_300_bps = 0x0;
	fdc->dor = E8272_DOR_RESET;
	fdc->msr = E8272_MSR_RQM; /* Main Status Register: DB7: Request for Master: RQM: */
	fdc->st[0] = 0xc0;
	fdc->st[1] = 0x00;
	fdc->st[2] = 0x00;
	fdc->st[3] = 0x00;
	for (i = 0; i < 4; i++) {
		e8272_drive_init (&fdc->drv[i], i);
	}
	fdc->curdrv = &fdc->drv[0];
	fdc->cmd_i = 0;
	fdc->cmd_n = 0;
	fdc->res_i = 0;
	fdc->res_n = 0;
	fdc->buf_i = 0;
	fdc->buf_n = 0;
	fdc->dma = 1;
	fdc->ready_change = 0;
	fdc->step_rate = 1;
	fdc->accurate = 0;
	fdc->ignore_eot = 0;
	fdc->delay_clock = 0;
	fdc->input_clock = 1000000;
	fdc->track_pos = 0;
	fdc->track_clk = 0;
	fdc->track_size = E8272_RATE / E8272_RPS;
	fdc->index_cnt = 0;
	fdc->set_data = NULL;
	fdc->get_data = NULL;
	fdc->set_tc = NULL;
	fdc->set_clock = NULL;
	fdc->diskop_ext = NULL;
	fdc->diskop = NULL;
	fdc->irq_ext = NULL;
	fdc->irq_val = 0;
	fdc->irq = NULL;
	fdc->dreq_ext = NULL;
	fdc->dreq_val = 0;
	fdc->dreq = NULL;
}

e8272_t *e8272_new (void)
{
	/* 250 Bits Per Second (BPS) */
	e8272_t *fdc;
	fdc = malloc (sizeof (e8272_t));
	if (fdc == NULL) {
		return (NULL);
	}
	e8272_init (fdc);
	return (fdc);
}

void e8272_free (e8272_t *fdc)
{
	/* 250 Bits Per Second (BPS) */
}

void e8272_del (e8272_t *fdc)
{
	/* 250 Bits Per Second (BPS) */
	if (fdc != NULL) {
		e8272_free (fdc);
		free (fdc);
	}
}

void e8272_set_250_bps(e8272_t *fdc, void *ext, void *fct)
{
	/* 250 BPS */
	fdc->disk_300_bps = 0;
	fdc->track_size = E8272_RATE / E8272_RPS;
}

void e8272_set_300_bps(e8272_t *fdc, void *ext, void *fct)
{
	/* 300 BPS */
	fdc->disk_300_bps = 1;
	fdc->track_size = E8272_RATE_300 / E8272_RPS;
}

void e8272_set_500_bps(e8272_t *fdc, void *ext, void *fct)
{
	/* 500 BPS */
	fdc->disk_300_bps = 1;
	fdc->track_size = E8272_RATE_300 / E8272_RPS;
}

void e8272_set_irq_fct (e8272_t *fdc, void *ext, void *fct)
{
	fdc->irq = fct;
	fdc->irq_ext = ext;
}

void e8272_set_dreq_fct (e8272_t *fdc, void *ext, void *fct)
{
	fdc->dreq = fct;
	fdc->dreq_ext = ext;
}


void e8272_set_diskop_fct (e8272_t *fdc, void *ext, void *fct)
{
	fdc->diskop_ext = ext;
	fdc->diskop = fct;
}


void e8272_set_input_clock (e8272_t *fdc, unsigned long clk)
{
	fdc->input_clock = clk;
}

void e8272_set_accuracy (e8272_t *fdc, int accurate)
{
	fdc->accurate = (accurate != 0);
}

void e8272_set_ignore_eot (e8272_t *fdc, int ignore_eot)
{
	fdc->ignore_eot = (ignore_eot != 0);
}

/*
 * Set the IRQ output
 */
static
void e8272_set_irq (e8272_t *fdc, unsigned char val)
{
	if (fdc->irq_val != val) {
#if E8272_DEBUG >= 3
		fprintf (stderr, "E8272: irq = %d\n", val);
#endif
		fdc->irq_val = val;
		if (fdc->irq != NULL) {
			fdc->irq (fdc->irq_ext, val);
		}
	}
}

/*
 * Set the DREQ output
 */
static
void e8272_set_dreq (e8272_t *fdc, unsigned char val)
{
	if (fdc->dreq_val != val) {
		fdc->dreq_val = val;
		if (fdc->dreq != NULL) {
			fdc->dreq (fdc->dreq_ext, val);
		}
	}
}
/* ---------------------------------------------------------------------------------------------- */
/* PROCESSOR INTERFACE:                                                                           */
/* ---------------------------------------------------------------------------------------------- */
/* During Command or Result Phases the Main Status Register (described earlier) must be read by   */
/* the processor before each byte of information is written into or read from the Data Register.  */
/* Bits D6 and D7 in the Main Status Register must be in a 0 and 1 state, respectivy, before each */
/* byte of the command word may be written into the uPD765. Many of the commands require multiple */
/* bytes, and as a result the Main Status Register must both be 1's (D6 = 1 and D7 = 1) before    */
/* reading each byte from the Data Register. Note, this reading of the Main Status Register before*/
/* each byte transfer to the uPD765 is required in only the Command and Result Phases, and NOT    */
/* during the Execution Phase.                                                                    */
/*                                                                                                */
/* During the Execution Phase, the Main Status Register need not be read. If the uPD765 is in the */
/* NON-DMA Mode, then the receipt of each data byte (if uPD765 is reading data from FDD) is       */
/* indicated by an Interrupt signal on pin 18 INT = 1). The generation of a Read signal (RD = 0)  */
/* will reset the Interrupt as well as output the Data onto the Data Bus. If the processor cannot */
/* handle Interrupts fast enough (every 13 us) then it may poll the Main Status Register and then */
/*bit D7 (RQM) functions just like the Interrupt signal. If a Write Command is in process then the*/
/* WR signal performs the reset to the Interrupt signal.                                          */
/*                                                                                                */
/* If the uPD765 is in the DMA Mode, no Interrupts are generated during the Execution Phase. The  */
/* uPD765 generates DRQ's (DMA Requests) when each byte of data is avail-able. The DMA Controller */
/*responds to this request with both a DACK = 0 (DMA Acknowledge) and a RD = 0 (Read signal). When*/
/* the DMA Acknowledge signal goes low (DACK = 0) then the DMA Request is reset (DRQ = 0). If a   */
/* Write Command has been programmed then a WR signal will appear instead of RD. After the        */
/* Execution Phase has been completed (Terminal Count has occurred) then an Interrupt will occur  */
/*(INT = 1). This signifies the beginning of the Result Phase. When the first byte of data is read*/
/* during the Result Phase, the Interrupt is automatically reset (INT = 0).                       */
/*                                                                                                */
/* It is important to note that during the Result Phase all bytes shown in the Command Table must */
/* be read. The Read Data Command, for example, has seven bytes of data in the Result Phase. All  */
/* seven bytes must be read in order to successfully complete the Read Data Command. The uPD765   */
/* will not accept a new command until all seven bytes have been read. Other commands may require */
/* few bytes to be read during the Result Phase.                                                  */
/*                                                                                                */
/* The uPD765 contains five Status Registers. The Main Status Register mentioned above may be read*/
/* by the processor at any time. The other four Status Registers (ST0, ST1, ST2, and ST3) are only*/
/*available during the Result Phase, and may be read only after successfully completing a command.*/
/* The particular command which has been executed determines how many of the Status Registers will*/
/* be read.                                                                                       */
/*                                                                                                */
/* The bytes of data which are sent to the uPD765 to form the Command Phase, and are read out of  */
/* the uPD765 in the Result Phase, must occur in the order shown in the Com-mand Table. That is,  */
/* the Command Code must be sent first and the other bytes sent in the prescribed sequence. No    */
/* foreshortening of the Command or Result Phases are allowed. After the last byte of data in the */
/* Command Phase is sent to the uPD765, the Execution Phase automatically starts. In a similar    */
/* fashion, when the last byte of data is read out in the Result Phase, the command is            */
/* automatically ended and the uPD765 is ready for a new command. A command may be truncated      */
/* (prematurely ended) by simply sending a Terminal Count signal to pin 16 (TC = 1). This is a    */
/* convenient means of ensuring that the processor may always get the uPD765's attention even if  */
/* the disk system hangs up in an abnormal manner.                                                */
/* ---------------------------------------------------------------------------------------------- */
static
void e8272_diskop_init (e8272_diskop_t *p,
	unsigned pd, unsigned pc, unsigned ph, unsigned ps,
	unsigned lc, unsigned lh, unsigned ls, unsigned ln,
	void *buf, unsigned cnt)
{
	p->pd = pd;
	p->pc = pc;
	p->ph = ph;
	p->ps = ps;
	p->lc = lc;
	p->lh = lh;
	p->ls = ls;
	p->ln = ln;
	p->buf = buf;
	p->cnt = cnt;
	p->fill = 0;
}
/* ______________________________DATA BUS_________________________________________________________*/
/* |__PHASE__|_R/W_|_D7__D6__D5__D4__D3__D2__D1__D0__|__________________REMARKS__________________|*/
/* |_______________________________READ DATA_____________________________________________________|*/
/* | Command |  W  | MT  MF  SK   0   0   1   1   0  | Command Codes                             |*/
/* |         |  W  |  X   X   X   X   X   HD  US1 US0|                                           |*/
/* |         |  W  | _______________C_______________ | Second ID information prior to Command    |*/
/* |         |  W  | _______________H_______________ | execution                                 |*/
/* |         |  W  | _______________R_______________ |                                           |*/
/* |         |  W  | _______________N_______________ |                                           |*/
/* |         |  W  | ______________EOT______________ |                                           |*/
/* |         |  W  | ______________GPL______________ |                                           |*/
/* |         |  W  | ______________DTL______________ |                                           |*/
/* |         |     |                                 |                                           |*/
/* |Execution|     |                                 | Data transfer between the FDD and main-   |*/
/* |         |     |                                 | system                                    |*/
/* | Result  |  R  | _____________ST  0_____________ | Status information after command execution|*/
/* |         |  R  | _____________ST  1_____________ |                                           |*/
/* |         |  R  | _____________ST  2_____________ |                                           |*/
/* |         |  R  | _______________C_______________ | Sector ID information after command       |*/
/* |         |  R  | _______________H_______________ | execution                                 |*/
/* |         |  R  | _______________R_______________ |                                           |*/
/* |         |  R  | _______________N_______________ |                                           |*/
/* |_________|_____|_________________________________|___________________________________________|*/
static
unsigned e8272_diskop_read (e8272_t *fdc, void *buf, unsigned *cnt,
	unsigned pd, unsigned pc, unsigned ph, unsigned ps, unsigned s)
{
	unsigned       r;
	e8272_diskop_t p;
	if (fdc->diskop == NULL) {
		return (E8272_ERR_NO_ID);
	}
	if (*cnt > 8192) {
		*cnt = 0;
		return (E8272_ERR_OTHER);
	}
	e8272_diskop_init (&p, pd, pc, ph, ps, 0, 0, s, 0, buf, *cnt);
	r = fdc->diskop (fdc->diskop_ext, E8272_DISKOP_READ, &p);
	*cnt = p.cnt;
	return (r);
}

static
unsigned e8272_diskop_write (e8272_t *fdc, void *buf, unsigned *cnt,
	unsigned pd, unsigned pc, unsigned ph, unsigned ps, unsigned s)
{
	unsigned       r;
	e8272_diskop_t p;
	if (fdc->diskop == NULL) {
		return (E8272_ERR_NO_ID);
	}
	if (*cnt > 8192) {
		*cnt = 0;
		return (E8272_ERR_OTHER);
	}
	e8272_diskop_init (&p, pd, pc, ph, ps, 0, 0, s, 0, buf, *cnt);
	r = fdc->diskop (fdc->diskop_ext, E8272_DISKOP_WRITE, &p);
	*cnt = p.cnt;
	return (r);
}

static
int e8272_diskop_format (e8272_t *fdc,
	unsigned pd, unsigned pc, unsigned ph, unsigned ps,
	const unsigned char *id, unsigned fill)
{
	unsigned       r;
	e8272_diskop_t p;
	if (fdc->diskop == NULL) {
		return (1);
	}
	e8272_diskop_init (&p, pd, pc, ph, ps, id[0], id[1], id[2], id[3], NULL, 0);
	p.fill = fill;
	r = fdc->diskop (fdc->diskop_ext, E8272_DISKOP_FORMAT, &p);
	return (r);
}

static
unsigned e8272_diskop_readid (e8272_t *fdc,
	unsigned pd, unsigned pc, unsigned ph, unsigned ps,
	unsigned char *id)
{
	unsigned       r;
	e8272_diskop_t p;
	if (fdc->diskop == NULL) {
		return (1);
	}
	e8272_diskop_init (&p, pd, pc, ph, ps, 0, 0, 0, 0, NULL, 0);
	r = fdc->diskop (fdc->diskop_ext, E8272_DISKOP_READID, &p);
	id[0] = p.lc;
	id[1] = p.lh;
	id[2] = p.ls;
	id[3] = p.ln;
	return (r);
}

/*
 * Read all the IDs on a track into fdc->curdrv->sct.
 */
static
void e8272_read_track (e8272_t *fdc)
{
	unsigned      i;
	unsigned char id[4];
	unsigned long ofs, cnt;
	e8272_drive_t *drv;
	drv = fdc->curdrv;
	if (drv->ok) {
		return;
	}
	drv->sct_cnt = 0;
	for (i = 0; i < E8272_MAX_SCT; i++) {
		if (e8272_diskop_readid (fdc, drv->d, drv->c, drv->h, i, id)) {
			break;
		}
		drv->sct[i].c = id[0];
		drv->sct[i].h = id[1];
		drv->sct[i].s = id[2];
		drv->sct[i].n = id[3];
		drv->sct_cnt += 1;
	}
	ofs = 8 * 150;
	cnt = fdc->track_size - ofs;
	for (i = 0; i < drv->sct_cnt; i++) {
		drv->sct[i].ofs = ofs + (2UL * i * cnt) / (2UL * drv->sct_cnt + 1);
	}
	drv->ok = 1;
}

static
void e8272_select_cylinder (e8272_t *fdc, unsigned pd, unsigned pc)
{
	e8272_drive_t *drv;
	drv = &fdc->drv[pd & 3];
	drv->ok = 0;
	drv->c = pc;
	drv->sct_cnt = 0;
	fdc->curdrv = drv;
}

static
void e8272_select_head (e8272_t *fdc, unsigned pd, unsigned ph)
{
	e8272_drive_t *drv;
	drv = &fdc->drv[pd & 3];
	drv->ok = 0;
	drv->h = ph;
	drv->sct_cnt = 0;
	fdc->curdrv = drv;
}

/*
 * Get the last ID before the current track position.
 */
static
unsigned e8272_get_current_id (e8272_t *fdc)
{
	unsigned i;
	if (fdc->curdrv->sct_cnt == 0) {
		return (0);
	}
	for (i = 0; i < fdc->curdrv->sct_cnt; i++) {
		if (fdc->curdrv->sct[i].ofs > fdc->track_pos) {
			if (i == 0) {
				return (fdc->curdrv->sct_cnt - 1);
			}
			else {
				return (i - 1);
			}
		}
	}
	return (fdc->curdrv->sct_cnt - 1);
}

/*
 * Convert bits to input clock.
 */
static
unsigned long e8272_bits_to_clock (e8272_t *fdc, unsigned long bits)
{
	if (fdc->disk_300_bps = 1)
	{
		unsigned long clk;
		clk = ((unsigned long long) bits * fdc->input_clock) / E8272_RATE_300;
		return (clk);
	}
	else
	{
		unsigned long clk;
		clk = ((unsigned long long) bits * fdc->input_clock) / E8272_RATE;
		return (clk);
	}
}

static
void e8272_delay_bits (e8272_t *fdc, unsigned long bits, int accurate)
{
	if ((fdc->accurate == 0) && (accurate == 0)) {
		fdc->track_pos += bits;
		while (fdc->track_pos >= fdc->track_size) {
			fdc->track_pos -= fdc->track_size;
			fdc->index_cnt += 1;
		}
		fdc->delay_clock = 0;
	}
	else {
		fdc->delay_clock = e8272_bits_to_clock (fdc, bits);
	}
}

static
void e8272_delay_index (e8272_t *fdc, int accurate)
{
	e8272_delay_bits (fdc, fdc->track_size - fdc->track_pos, accurate);
}

static
void e8272_delay_id (e8272_t *fdc, unsigned id, int accurate)
{
	unsigned long bits, pos;
	if (id >= fdc->curdrv->sct_cnt) {
		e8272_delay_index (fdc, accurate);
		return;
	}
	pos = fdc->curdrv->sct[id].ofs;
	if (pos < fdc->track_pos) {
		bits = pos - fdc->track_pos + fdc->track_size;
	}
	else {
		bits = pos - fdc->track_pos;
	}
	bits += 128;
	e8272_delay_bits (fdc, bits, accurate);
}

static
void e8272_delay_next_id (e8272_t *fdc, int accurate)
{
	unsigned id;
	id = e8272_get_current_id (fdc);
	id += 1;
	if (id >= fdc->curdrv->sct_cnt) {
		id = 0;
	}
	e8272_delay_id (fdc, id, accurate);
}


/*
 * Move to the next sector ID for read and write commands
 */
static
void e8272_next_id (e8272_t *fdc)
{
	if (fdc->ignore_eot || (fdc->cmd[4] != fdc->cmd[6])) {
		fdc->cmd[4] += 1;
		return;
	}
	fdc->cmd[4] = 1;
	if (fdc->cmd[0] & E8272_CMD0_MT) {
		fdc->cmd[1] ^= 0x04;
		fdc->cmd[3] ^= 0x01;
		if ((fdc->cmd[1] & 0x04) == 0) {
			fdc->cmd[2] += 1;
		}
	}
	else {
		fdc->cmd[2] += 1;
	}
	fdc->curdrv->ok = 0;
	fdc->curdrv->h = fdc->cmd[3] & 1;
	fdc->curdrv->sct_cnt = 0;
	e8272_read_track (fdc);
}

/*
 * Request a data transfer byte from (rd == 0) / to (rd != 0) the CPU.
 */
static
void e8272_request_data (e8272_t *fdc, int rd)
{
	if (rd) {
		fdc->msr |= E8272_MSR_DIO; /* Main Status Register: DB6: Data Intput/Output: DIO: */
	}
	else {
		fdc->msr &= ~E8272_MSR_DIO; /* Main Status Register: DB6: Data Intput/Output: DIO: */
	}
	if (fdc->dma) {
		fdc->msr &= ~E8272_MSR_NDM; /* Main Status Register: DB5: Non-DMA mode: NDM: */
		e8272_set_dreq (fdc, 1);
	}
	else {
		fdc->msr |= E8272_MSR_NDM;  /* Main Status Register: DB5: Non-DMA mode: NDM: */
		e8272_set_irq (fdc, 1);
	}
	fdc->msr |= E8272_MSR_RQM | E8272_MSR_CB;
	/* Main Status Register: DB7: Request for Master: RQM: */ /* Main Status Register: DB4: FDD Busy: CB: */
}

/*
 * Request a control byte transfer from (rd == 0) / to (rd != 0) the CPU.
 */
static
void e8272_request_control (e8272_t *fdc, int rd)
{
	if (rd) {
		fdc->msr |= (E8272_MSR_DIO | E8272_MSR_CB);
		/* Main Status Register: DB6: Data Intput/Output: DIO: */ /* Main Status Register: DB4: FDD Busy: CB: */
	}
	else {
		fdc->msr &= ~(E8272_MSR_DIO | E8272_MSR_CB);
		/* Main Status Register: DB6: Data Intput/Output: DIO: */ /* Main Status Register: DB4: FDD Busy: CB: */
	}
	if (fdc->dma) {
		e8272_set_dreq (fdc, 0);
	}
	fdc->msr |= E8272_MSR_RQM;  /* Main Status Register: DB7: Request for Master: RQM: */
	fdc->msr &= ~E8272_MSR_NDM; /* Main Status Register: DB5: Non-DMA mode: NDM: */
}


/*
 * Ready for next command
 */
static
void cmd_done (e8272_t *fdc)
{
	fdc->set_data = e8272_write_cmd;
	fdc->get_data = NULL;
	fdc->set_tc = NULL;
	fdc->set_clock = NULL;
	e8272_request_control (fdc, 0);
}

/*
 * Write a command byte
 */
static
void cmd_set_command (e8272_t *fdc, unsigned char val)
{
	fdc->cmd[fdc->cmd_i++] = val;
	if (fdc->cmd_i < fdc->cmd_n) {
		return;
	}
	fdc->set_data = NULL;
	fdc->msr &= ~E8272_MSR_RQM; /* Main Status Register: DB7: Request for Master: RQM: */
	fdc->msr |= E8272_MSR_CB;   /* Main Status Register: DB4: FDD Busy: CB: */
	if (fdc->start_cmd != NULL) {
		fdc->start_cmd (fdc);
	}
}

/*
 * Get a result byte
 */
static
unsigned char cmd_get_result (e8272_t *fdc)
{
	unsigned char val;
	val = fdc->res[fdc->res_i++];
	if (fdc->res_i >= fdc->res_n) {
		cmd_done (fdc);
	}
#if E8272_DEBUG >= 3
	fprintf (stderr, "E8272: get result (%02X)\n", val);
#endif
	return (val);
}

/*
 * Enter the result phase
 */
static
void cmd_result (e8272_t *fdc, unsigned cnt)
{
	fdc->res_i = 0;
	fdc->res_n = cnt;
	fdc->set_data = NULL;
	fdc->get_data = cmd_get_result;
	fdc->set_tc = NULL;
	fdc->set_clock = NULL;
	e8272_request_control (fdc, 1);
}
/*****************************************************************************
 * read
 *****************************************************************************/
static void cmd_read_clock (e8272_t *fdc, unsigned long cnt);
/* ______________________________DATA BUS_________________________________________________________*/
/* |__PHASE__|_R/W_|_D7__D6__D5__D4__D3__D2__D1__D0__|__________________REMARKS__________________|*/
/* |_______________________________READ DATA_____________________________________________________|*/
/* | Command |  W  | MT  MF  SK   0   0   1   1   0  | Command Codes                             |*/
/* |         |  W  |  X   X   X   X   X   HD  US1 US0|                                           |*/
/* |         |  W  | _______________C_______________ | Second ID information prior to Command    |*/
/* |         |  W  | _______________H_______________ | execution                                 |*/
/* |         |  W  | _______________R_______________ |                                           |*/
/* |         |  W  | _______________N_______________ |                                           |*/
/* |         |  W  | ______________EOT______________ |                                           |*/
/* |         |  W  | ______________GPL______________ |                                           |*/
/* |         |  W  | ______________DTL______________ |                                           |*/
/* |         |     |                                 |                                           |*/
/* |Execution|     |                                 | Data transfer between the FDD and main-   |*/
/* |         |     |                                 | system                                    |*/
/* | Result  |  R  | _____________ST  0_____________ | Status information after command execution|*/
/* |         |  R  | _____________ST  1_____________ |                                           |*/
/* |         |  R  | _____________ST  2_____________ |                                           |*/
/* |         |  R  | _______________C_______________ | Sector ID information after command       |*/
/* |         |  R  | _______________H_______________ | execution                                 |*/
/* |         |  R  | _______________R_______________ |                                           |*/
/* |         |  R  | _______________N_______________ |                                           |*/
/* |_________|_____|_________________________________|___________________________________________|*/
static
void cmd_read_tc (e8272_t *fdc)
{
#if E8272_DEBUG >= 2
	fprintf (stderr, "E8272: CMD=%02X D=%u  READ TC"
		" (pc=%u, ph=%u c=%u, h=%u, s=%u, n=%u eot=%u)\n",
		fdc->cmd[0], fdc->curdrv->d, fdc->curdrv->c, fdc->curdrv->h,
		fdc->cmd[2], fdc->cmd[3], fdc->cmd[4], fdc->cmd[5], fdc->cmd[6]
	);
#endif
	/* head and unit */
	fdc->st[0] = (fdc->st[0] & ~0x07) | (fdc->cmd[1] & 0x07);
	fdc->res[0] = fdc->st[0];  /* IBM PS/2: Byte 0: Status Register 0 */ /* ST 0: Status information after command execution */
	fdc->res[1] = fdc->st[1];  /* IBM PS/2: Byte 1: Status Register 1 */ /* ST 1: Status information after command execution */
	fdc->res[2] = fdc->st[2];  /* IBM PS/2: Byte 2: Status Register 2 */ /* ST 2: Status information after command execution */
	fdc->res[3] = fdc->cmd[2]; /* IBM PS/2: Byte 3: Cylinder Number   */
	fdc->res[4] = fdc->cmd[3]; /* IBM PS/2: Byte 4: Head Address      */
	fdc->res[5] = fdc->cmd[4]; /* IBM PS/2: Byte 5: Sector   Number   */
	fdc->res[6] = fdc->cmd[5]; /* IBM PS/2: Byte 6: Number of Data Bytes in Sector */
	cmd_result (fdc, 7);
	e8272_set_irq (fdc, 1);
}
/* --------------------------------------------------------------------------------------------------- */
/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; Command Format:                */
/* --------------------------------------------------------------------------------------------------- */
/* The following are commands that may be issued to the controller. An X is used to indicate a don't   */
/* care condition.                                                                                     */
/* --------------------------------------------------------------------------------------------------- */
/* Read Data:                                                                                          */
/* --------------------------------------------------------------------------------------------------- */
/* Command Phase:                                                                                      */
/*  - MT  = Multitrack                                                                                 */
/*  - MF  = MFM Mode                                                                                   */
/*  - SK  = Skip Deleted-Data Address Mark                                                             */
/*  - HD  = Head Number                                                                                */
/*  - USx = Unit Select                                                                                */
/* -----------7--6--5--4---3--2---1---0--------------------------------------------------------------- */
/*  - Byte 0: MT MF SK  0  0  1   1   0                                                                */
/*  - Byte 1: X  X   X  X  X  HD US1 US0                                                               */
/*  - Byte 2: Cylinder Number                                                                          */
/*  - Byte 3: Head Address                                                                             */
/*  - Byte 4: Sector   Number                                                                          */
/*  - Byte 5: Number of Data Bytes in Sector                                                           */
/*  - Byte 6: End of Track                                                                             */
/*  - Byte 7: Gap  Length                                                                              */
/*  - Byte 8: Data Length                                                                              */
/* --------------------------------------------------------------------------------------------------- */
/* Figure 1-69. Read Data Command:                                                                     */
/* --------------------------------------------------------------------------------------------------- */
/* -----------7--6--5--4---3--2---1---0--------------------------------------------------------------- */
/*  - Byte 0: Status Register 0                                                                        */
/*  - Byte 1: Status Register 1                                                                        */
/*  - Byte 2: Status register 2                                                                        */
/*  - Byte 3: Cylinder Number                                                                          */
/*  - Byte 4: Head Address                                                                             */
/*  - Byte 5: Sector   Number                                                                          */
/*  - Byte 6: Number of Data Bytes in Sector                                                           */
/* --------------------------------------------------------------------------------------------------- */
/* Figure 1-70. Read Data Result:                                                                      */
/* --------------------------------------------------------------------------------------------------- */
static
void cmd_read_error (e8272_t *fdc, unsigned err)
{
#if E8272_DEBUG >= 1
	fprintf (stderr,
		"E8272: CMD=%02X D=%u  READ ERROR"
		" (pc=%u, ph=%u c=%u, h=%u, s=%u, n=%u eot=%u)\n",
		fdc->cmd[0], fdc->cmd[1] & 3,
		fdc->curdrv->c, fdc->curdrv->h,
		fdc->cmd[2], fdc->cmd[3], fdc->cmd[4], fdc->cmd[5], fdc->cmd[6]
	);
#endif
	fdc->read_error = 1;
	/* abnormal termination */
	fdc->st[0] = (fdc->st[0] & 0x3f) | 0x40;
	if (err & E8272_ERR_NO_ID) {
		fdc->st[1] |= E8272_ST1_MA | E8272_ST1_ND;
	}
	else if (err & E8272_ERR_CRC_ID) {
		fdc->st[1] |= E8272_ST1_DE;
	}
	else if (err & E8272_ERR_NO_DATA) {
		fdc->st[1] |= E8272_ST1_MA;
		fdc->st[2] |= E8272_ST2_MD;
	}
	else if (err & E8272_ERR_DEL_DAM) {
		fdc->st[2] |= E8272_ST2_CM;
	}
	else if (err & E8272_ERR_CRC_DATA) {
		fdc->st[1] |= E8272_ST1_DE;
		fdc->st[2] |= E8272_ST2_DD;
	}
	else {
		fdc->st[1] |= E8272_ST1_EN | E8272_ST1_ND;
	}
}
/* ---------------------------------------------------------------------------------------------- */
/* FUNCTIONAL DESCRIPTION OF COMMANDS: READ DATA                                                  */
/* ---------------------------------------------------------------------------------------------- */
/*A set of nine (9) byte words are required to place the FDC into the Read Data Mode. After the   */
/*Read Data command has been issued the FDC loads the head (if it is in the unloaded state), waits*/
/* the specified head settling time (defined in the Specify Command), and begins reading the ID   */
/* Address Marks and ID fields. When the current sector number ("R") stored in the ID Register    */
/*(IDR) compares with the sector number read off the diskette, then the FDC outputs data (from the*/
/* data field) byte-to-byte to the main system via the data bus.                                  */
/*                                                                                                */
/*After completion of the read operation from the current sector, the Sector Number is incremented*/
/* by one, and the data from the next sector is read and output on the data bus. This continuous  */
/*read function is called a "Multi-Sector Read Operation." The Read Data Command may be terminated*/
/*by the receipt of a Terminal Count signal. Upon receipt of this signal, the FDC stops outputting*/
/*data to the processor, but will continue to read data from the current sector, check CRC (Cyclic*/
/* Redundancy Count) bytes, and then at the end of the sector terminate the Read Data Command.    */
/*                                                                                                */
/* The amount of data which can handled with a single command to the FDC depnds upon MT (multi-   */
/* track), MF (MFM/FM), and N (Number of Bytes/Sector). Table 1 below shows the Transfer Capacity.*/
/* ---------------------------------------------------------------------------------------------- */
/*                                    Maximum Transfer Capacity                                   */
/* Multi-Track MT: MFM/FM MF: Bytes/Sector N: (Bytes/Sector) (Number of Sectors):Final Sector Read*/
/*                                                                               from Diskette:   */
/*  - 0:            - 0:       - 00:           - ( 128) (26) =  3,328:            - 26 at Side 0  */
/*                                                                               or 26 at Side 1  */
/*  - 0:            - 1:       - 01:           - ( 256) (26) =  6,656:            - 26 at Side 0  */
/*                                                                               or 26 at Side 1  */
/*  - 1:            - 0:       - 00:           - ( 128) (52) =  6,656:            - 26 at Side 1  */
/*  - 1:            - 1:       - 01:           - ( 256) (52) = 13,312:            - 26 at Side 1  */
/*  - 0:            - 0:       - 01:           - ( 256) (15) =  3,840:            - 15 at Side 0  */
/*                                                                               or 15 at Side 1  */
/*  - 0:            - 1:       - 02:           - ( 512) (15) =  7,680:            - 15 at Side 0  */
/*                                                                               or 15 at Side 1  */
/*  - 1:            - 0:       - 01:           - ( 256) (30) =  7,680:            - 15 at Side 1  */
/*  - 1:            - 1:       - 02:           - ( 512) (30) = 15,360:            - 15 at Side 1  */
/*  - 0:            - 0:       - 02:           - ( 512) ( 8) =  4,096             -  8 at Side 0  */
/*                                                                               or  8 at Side 1  */
/*  - 0:            - 1:       - 03:           - (1024) ( 8) =  8,192:            -  8 at Side 0  */
/*                                                                               or  8 at Side 1  */
/*  - 1:            - 0:       - 02:           - ( 512) (16) =  8,192:            -  8 at Side 1  */
/*  - 1:            - 1:       - 03:           - (1024) (16) = 16,384:            -  8 at Side 1  */
/* ---------------------------------------------------------------------------------------------- */
/* Table 1. Transfer Capacity:                                                                    */
/* ---------------------------------------------------------------------------------------------- */
/*The "multi-track" function (MT) allows the FDC to read data from both sides of the diskette. For*/
/* a particular cylinder, data will be transferred starting at Sector 0, Side 0 and completing at */
/* Sector L, Side 1 (Sector L = last sector on the side). Note, this function pertains to only one*/
/* cylinder (the same track) on each side of the diskette.                                        */
/*                                                                                                */
/* When N = 0, then DTL defines the data length which the FDC must treat as a sector. If DTL is   */
/* smaller than the actual data length in a Sector, the data beyond DTL in the Sector, is not sent*/
/* to the Data Bus. The FDC reads (internally) the complete Sector performing the CRC check, and  */
/* depending upon the manner of com-mand termination, may perform a Multi-Sector Read Operation.  */
/* When N is non-zero, then DTL has no meaning and should be set to FF Hexidecimal.               */
/*                                                                                                */
/* If the FDC detects the Index Hole twice without finding the right sector, (indicated in "R"),  */
/* then the FDC sets the ND (No Data) flag in Status Register 1 to a 1 (high), and terminates the */
/* Read Data Command. (Status Register 0 also has bits 7 and 6 set to 0 and 1 respectively.)      */
/*                                                                                                */
/*After reading the ID and Data Fields in each sector, the FDC checks the CRC bytes. If read error*/
/* is detected (incorrect CRC in ID field), the FDC sets the DE (Data Error) flag in Status       */
/* Register 1 to a 1 (high), and if a CRC error occurs in the Data Field the FDC also sets the DD */
/*(Data Error in Data Field) flag in Status Register 2 to a 1 (high), and terminates the Read Data*/
/* Command. (Status Register 0 also has bits 7 and 6 set to 0 and 1 respectively.)                */
/*                                                                                                */
/* Status Register 2 to a 1 (high), and terminates the Read Data Command.                         */
/*                                                                                                */
/* If the FDC reads a Deleted Data Address Mark off the diskette,a nd the SK bit (bit D5 in the   */
/* first Command Word) is not set (SK = 0), then the FDC sets the DM (Control Mark) flag in Status*/
/* Register 2 to a 1 (high), and terminates the Read Data Command, after reading all the data in  */
/*the Sector. If SK = 1, the FDC skips the sector with the Deleted Data Address Mark and reads the*/
/* next sector.                                                                                   */
/*                                                                                                */
/* During disk data transfers between the FDC and the processor, via the data bus, the FDC must be*/
/* serviced by the processor every 27 us in the FM Mode, and every 13 us in the MFM Mode, or the  */
/*FDC sets the OR (Over Run) flag in Status Register 1 to a 1 (high), and terminates the Read Data*/
/* Command.                                                                                       */
/*                                                                                                */
/* If the processor terminates a read (or write) operation in the FDC, then the ID information in */
/* the Result Phase is dependent upon the state of the MT bit and EOT byte. Table 2 shows the     */
/* values for C, H, R, and N, when the processor terminates the Command.                          */
/* ---------------------------------------------------------------------------------------------- */
/* FUNCTIONAL DESCRIPTION OF COMMANDS (CONT.):                                                    */
/* ---------------------------------------------------------------------------------------------- */
/* MT:  EOT:  Final Sector Transferred    ID Information Result   C:       H:     R:        N:    */
/*                  to Processor:                 Phase:                                          */
/*  - 0: - 01: - Sector  1 to 25 at Side 0: -                      - NC   : - NC : - R + 1 : - NC */
/*  - 0: - 0F: - Sector  1 to 14 at Side 0: -                      - NC   : - NC : - R + 1 : - NC */
/*  - 0: - 0B: - Sector  1 to  7 at Side 0: -                      - NC   : - NC : - R + 1 : - NC */
/*  - 0: - 1A: - Sector 26       at Side 0: -                      - C + 1: - NC : - R = 01: - NC */
/*  - 0: - 0F: - Sector 15       at Side 0: -                      - C + 1: - NC : - R = 01: - NC */
/*  - 0: - 08: - Sector  8       at Side 0: -                      - C + 1: - NC : - R = 01: - NC */
/*  - 0: - 1A: - Sector  1 to 25 at Side 1: -                      - NC   : - NC : - R + 1 : - NC */
/*  - 0: - 0F: - Sector  1 to 14 at Side 1: -                      - NC   : - NC : - R + 1 : - NC */
/*  - 0: - 08: - Sector  1 to  7 at Side 1: -                      - NC   : - NC : - R + 1 : - NC */
/*  - 0: - 1A: - Sector 26       at Side 1: -                      - C + 1: - NC : - R = 01: - NC */
/*  - 0: - 0F: - Sector 15       at Side 1: -                      - C + 1: - NC : - R = 01: - NC */
/*  - 0: - 08: - Sector  8       at Side 1: -                      - C + 1: - NC : - R = 01: - NC */
/*  - 1: - 1A: - Sector  1 to 25 at Side 0: -                      - NC   : - NC : - R + 1 : - NC */
/*  - 1: - 0F: - Sector  1 to 14 at Side 0: -                      - NC   : - NC : - R + 1 : - NC */
/*  - 1: - 08: - Sector  1 to  7 at Side 0: -                      - NC   : - NC : - R + 1 : - NC */
/*  - 1: - 1A: - Sector 26       at Side 0: -                      - NC   : - LSB: - R = 01: - NC */
/*  - 1: - 0F: - Sector 15       at Side 0: -                      - NC   : - LSB: - R = 01: - NC */
/*  - 1: - 08: - Sector  8       at Side 0: -                      - NC   : - LSB: - R = 01: - NC */
/*  - 1: - 1A: - Sector  1 to 25 at Side 1: -                      - NC   : - NC : - R + 1 : - NC */
/*  - 1: - 0F: - Sector  1 to 14 at Side 1: -                      - NC   : - NC : - R + 1 : - NC */
/*  - 1: - 08: - Sector  1 to  7 at Side 1: -                      - NC   : - NC : - R + 1 : - NC */
/*  - 1: - 1A: - Sector 26       at Side 1: -                      - C + 1: - LSB: - R = 01: - NC */
/*  - 1: - 0F: - Sector 15       at Side 1: -                      - C + 1: - LSB: - R = 01: - NC */
/*  - 1: - 08: - Sector  8       at Side 1: -                      - C + 1: - LSB: - R = 01: - NC */
/* ---------------------------------------------------------------------------------------------- */
/* NOTES: 1 NC  (No Change)            : The same value as the one at the beginning of command    */
/*                                       execution.                                               */
/*        2 LSB (Least Significant Bit): The least significant bit of H is complemented.          */
/* ---------------------------------------------------------------------------------------------- */
/* Table 2: ID Information When Processor Terminates Command:                                     */
/* ---------------------------------------------------------------------------------------------- */
static
unsigned char cmd_read_get_data (e8272_t *fdc)
{
	unsigned char val;
	val = fdc->buf[fdc->buf_i++];
	if (fdc->dma) {
		e8272_set_dreq (fdc, 0);
	}
	if (fdc->buf_i < fdc->buf_n) {
		e8272_request_data (fdc, 1);
		return (val);
	}
	fdc->msr &= ~(E8272_MSR_RQM | E8272_MSR_DIO); /* Main Status Register: DB7: Request for Master: RQM: */
	fdc->get_data = NULL;                         /* Main Status Register: DB6: Data Intput/Output: DIO: */
	fdc->set_clock = cmd_read_clock;
	if (fdc->read_error) {
		cmd_read_tc (fdc);
	}
	else {
		e8272_next_id (fdc);
	}
	return (val);
}

static
void cmd_read_clock (e8272_t *fdc, unsigned long cnt)
{
	unsigned    id;
	unsigned    c, h, s, n;
	unsigned    cnt1, cnt2;
	unsigned    err;
	e8272_sct_t *sct;
	if (fdc->index_cnt >= 2) {
		cmd_read_error (fdc, E8272_ERR_NO_ID);
		cmd_read_tc (fdc);
		return;
	}
	if (fdc->delay_clock > 0) {
		return;
	}
	id = e8272_get_current_id (fdc);
	e8272_delay_next_id (fdc, 0);
	if ((fdc->cmd[0] & E8272_CMD0_MFM) == 0) {
		return;
	}
	c = fdc->cmd[2]; /* IBM PS/2: Cylinder Number                */
	h = fdc->cmd[3]; /* IBM PS/2: Head Address                   */
	s = fdc->cmd[4]; /* IBM PS/2: Sector   Number                */
	n = fdc->cmd[5]; /* IBM PS/2: Number of Data Bytes in Sector */
	if (id >= fdc->curdrv->sct_cnt) {
		return;
	}
	sct = &fdc->curdrv->sct[id];
	if ((sct->c != c) || (sct->h != h) || (sct->s != s)) {
		/* wrong id */
		return;
	}
	if (sct->n < n) {
		/* sector too small */
		return;
	}
	fdc->index_cnt = 0;
	if (n > 6) {
		cmd_read_error (fdc, E8272_ERR_OTHER);
		cmd_read_tc (fdc);
		return;
	}
	if (n == 0) {
		cnt1 = fdc->cmd[8]; /* IBM PS/2: Data Length */
	}
	else {
		cnt1 = 128 << n;
	}
	cnt2 = cnt1;
	err = e8272_diskop_read (fdc, fdc->buf, &cnt2,
		fdc->curdrv->d, fdc->curdrv->c, fdc->curdrv->h, id, s
	);
	if (err & (E8272_ERR_CRC_ID | E8272_ERR_NO_DATA)) {
		cmd_read_error (fdc, err);
		cmd_read_tc (fdc);
		return;
	}
	if ((fdc->read_deleted != 0) != ((err & E8272_ERR_DEL_DAM) != 0)) {
		if ((fdc->cmd[0] & E8272_CMD0_SK)) {
			e8272_next_id (fdc);
			fdc->st[2] |= E8272_ST2_CM;
			return;
		}
	}
	err &= ~E8272_ERR_DEL_DAM;
	fdc->buf_i = 0;
	fdc->buf_n = cnt2;
	if (err) {
		cmd_read_error (fdc, err);
	}
	if (fdc->buf_n > 0) {
		e8272_request_data (fdc, 1);
		fdc->set_clock = NULL;
		fdc->get_data = cmd_read_get_data;
	}
	else {
		cmd_read_tc (fdc);
	}
}

static
void cmd_read (e8272_t *fdc)
{
	e8272_select_head (fdc, fdc->cmd[1] & 3, (fdc->cmd[1] >> 2) & 1);
	e8272_read_track (fdc);
#if E8272_DEBUG >= 1
	fprintf (stderr, "E8272: CMD=%02X D=%u"
		"  READ (pc=%u, ph=%u, c=%u, h=%u, s=%u, n=%u, eot=%u)\n",
		fdc->cmd[0], fdc->curdrv->d, fdc->curdrv->c, fdc->curdrv->h,
		fdc->cmd[2], fdc->cmd[3], fdc->cmd[4], fdc->cmd[5], fdc->cmd[6]
	);
#endif
	fdc->st[0] = 0;
	fdc->st[1] = 0;
	fdc->st[2] = 0;
	fdc->read_error = 0;
	fdc->read_deleted = 0;
	fdc->index_cnt = 0;
	e8272_delay_next_id (fdc, 0);
	fdc->set_clock = cmd_read_clock;
	fdc->set_tc = cmd_read_tc;
}
/* ______________________________DATA BUS_________________________________________________________*/
/* |__PHASE__|_R/W_|_D7__D6__D5__D4__D3__D2__D1__D0__|__________________REMARKS__________________|*/
/* |______________________________READ DELETED DATA______________________________________________|*/
/* |         |     |                                 |                                           |*/
/* | Command |  W  | MT  MF  SK   0   1   1   0   0  | Command Codes                             |*/
/* |         |  W  |  X   X   X   X   X   HD  US1 US0|                                           |*/
/* |         |  W  | _______________C_______________ | Sector ID information prior to Command    |*/
/* |         |  W  | _______________H_______________ | execution                                 |*/
/* |         |  W  | _______________R_______________ |                                           |*/
/* |         |  W  | _______________N_______________ |                                           |*/
/* |         |  W  | ______________EOT______________ |                                           |*/
/* |         |  W  | ______________GPL______________ |                                           |*/
/* |         |  W  | ______________DTL______________ |                                           |*/
/* |         |     |                                 |                                           |*/
/* |Execution|     |                                 | Data-transfer between the FDD and main-   |*/
/* |         |     |                                 | system.                                   |*/
/* |         |     |                                 | FDC reads all of cylinders contents from  |*/
/* |         |     |                                 | index hole to EOT.                        |*/
/* | Result  |  R  | _____________ST  0_____________ | Status information after command execution|*/
/* |         |  R  | _____________ST  1_____________ |                                           |*/
/* |         |  R  | _____________ST  2_____________ |                                           |*/
/* |         |  R  | _______________C_______________ | Sector ID information after command       |*/
/* |         |  R  | _______________H_______________ | execution                                 |*/
/* |         |  R  | _______________R_______________ |                                           |*/
/* |         |  R  | _______________N_______________ |                                           |*/
/* |_________|_____|_________________________________|___________________________________________|*/
/*****************************************************************************
 * read deleted data
 *****************************************************************************/
 /* --------------------------------------------------------------------------------------------------- */
 /* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; Command Format:                */
 /* --------------------------------------------------------------------------------------------------- */
 /* The following are commands that may be issued to the controller. An X is used to indicate a don't   */
 /* care condition.                                                                                     */
 /* --------------------------------------------------------------------------------------------------- */
 /* Read Deleted Data:                                                                                  */
 /* --------------------------------------------------------------------------------------------------- */
 /* Command Phase:                                                                                      */
 /*  - MT  = Multitrack                                                                                 */
 /*  - MF  = MFM Mode                                                                                   */
 /*  - SK  = Skip Deleted-Data Address Mark                                                             */
 /*  - HD  = Head Number                                                                                */
 /*  - USx = Unit Select                                                                                */
 /* -----------7--6--5--4---3--2---1---0--------------------------------------------------------------- */
 /*  - Byte 0: MT MF SK  0  1  1   0   0                                                                */
 /*  - Byte 1: X  X   X  X  X  HD US1 US0                                                               */
 /*  - Byte 2: Cylinder Number                                                                          */
 /*  - Byte 3: Head Address                                                                             */
 /*  - Byte 4: Sector   Number                                                                          */
 /*  - Byte 5: Number of Data Bytes in Sector                                                           */
 /*  - Byte 6: End of Track                                                                             */
 /*  - Byte 7: Gap  Length                                                                              */
 /*  - Byte 8: Data Length                                                                              */
 /* --------------------------------------------------------------------------------------------------- */
 /* Figure 1-71. Read Deleted Data Command:                                                             */
 /* --------------------------------------------------------------------------------------------------- */
 /* -----------7--6--5--4---3--2---1---0--------------------------------------------------------------- */
 /*  - Byte 0: Status Register 0                                                                        */
 /*  - Byte 1: Status Register 1                                                                        */
 /*  - Byte 2: Status register 2                                                                        */
 /*  - Byte 3: Cylinder Number                                                                          */
 /*  - Byte 4: Head Address                                                                             */
 /*  - Byte 5: Sector   Number                                                                          */
 /*  - Byte 6: Number of Data Bytes in Sector                                                           */
 /* --------------------------------------------------------------------------------------------------- */
 /* Figure 1-72. Read Deleted Data Result:                                                              */
 /* --------------------------------------------------------------------------------------------------- */
static
void cmd_read_deleted (e8272_t *fdc)
{
	e8272_select_head (fdc, fdc->cmd[1] & 3, (fdc->cmd[1] >> 2) & 1);
	e8272_read_track (fdc);
#if E8272_DEBUG >= 1
	fprintf (stderr, "E8272: CMD=%02X D=%u"
		"  READ DELETED (pc=%u, ph=%u, c=%u, h=%u, s=%u, n=%u, eot=%u)\n",
		fdc->cmd[0], fdc->curdrv->d, fdc->curdrv->c, fdc->curdrv->h,
		fdc->cmd[2], fdc->cmd[3], fdc->cmd[4], fdc->cmd[5], fdc->cmd[6]
	);
#endif
	fdc->st[0] = 0;
	fdc->st[1] = 0;
	fdc->st[2] = 0;
	fdc->read_error = 0;
	fdc->read_deleted = 1;
	fdc->index_cnt = 0;
	e8272_delay_next_id (fdc, 0);
	fdc->set_clock = cmd_read_clock;
	fdc->set_tc = cmd_read_tc;
}
/* ---------------------------------------------------------------------------------------------- */
/*  READ DELETED DATA:                                                                            */
/* ---------------------------------------------------------------------------------------------- */
/* This command is the same as the  Read Data Command except that when the FDC detects a Data     */
/* Address Mark at the beginning of a Data Field (and SK = 0 (low), it will read all the data in  */
/* the sector and set the MD flag in Status Register 2 to a 1 (high), and then terminate the      */
/* command. If SK = 1, then the FDC skips the sector with the Data Address Mark and reads the next*/
/* sector.                                                                                        */
/* ---------------------------------------------------------------------------------------------- */
/*****************************************************************************
 * read track
 *****************************************************************************/
/* ______________________________DATA BUS_________________________________________________________*/
/* |__PHASE__|_R/W_|_D7__D6__D5__D4__D3__D2__D1__D0__|__________________REMARKS__________________|*/
/* |______________________________READ_A_TRACK___________________________________________________|*/
/* |         |     |                                 |                                           |*/
/* | Command |  W  |  0  MF  SK   0   0   0   1   0  | Command Codes                             | */
/* |         |  W  |  X   X   X   X   X   HD  US1 US0|                                           |*/
/* |         |  W  | _______________C_______________ | Sector ID information prior to Command    |*/
/* |         |  W  | _______________H_______________ | execution                                 |*/
/* |         |  W  | _______________R_______________ |                                           |*/
/* |         |  W  | _______________N_______________ |                                           |*/
/* |         |  W  | ______________EOT______________ |                                           |*/
/* |         |  W  | ______________GPL______________ |                                           |*/
/* |         |  W  | ______________DTL______________ |                                           |*/
/* |         |     |                                 |                                           |*/
/* |Execution|     |                                 | Data-transfer between the FDD and main-   |*/
/* |         |     |                                 | system.                                   |*/
/* |         |     |                                 | FDC reads all of cylinders contents from  |*/
/* |         |     |                                 | index hole to EOT.                        |*/
/* | Result  |  R  | _____________ST  0_____________ | Status information after command execution|*/
/* |         |  R  | _____________ST  1_____________ |                                           |*/
/* |         |  R  | _____________ST  2_____________ |                                           |*/
/* |         |  R  | _______________C_______________ | Sector ID information after command       |*/
/* |         |  R  | _______________H_______________ | execution                                 |*/
/* |         |  R  | _______________R_______________ |                                           |*/
/* |         |  R  | _______________N_______________ |                                           |*/
/* |_________|_____|_________________________________|___________________________________________|*/
static void cmd_read_track_clock (e8272_t *fdc, unsigned long cnt);
/* ---------------------------------------------------------------------------------------------- */
/* FUNCTIONAL DESCRIPTION OF COMMANDS (CON.T):  READ A TRACK:                                     */
/* ---------------------------------------------------------------------------------------------- */
/* This command is similar to the  READ DATA Command except that this is a continuous READ        */
/* operation where the entire contents of the track are read. Immediately after encountering the  */
/* INDEX HOLE, the FDC starts reading all data on the track, Gap bytes, Address Marks and Data are*/
/* all read as a continous data stream. If the FDC finds an error in the ID or DATA CRC check     */
/* bytes, it continues to read data from the track. The FDC compares the ID information read from */
/* each sector with the value stored in the IDR, and sets the ND flag of Status Register 1 to a 1 */
/* (high) if there is no comparison. Multi-track or skip operations are not allowed with this     */
/* command.                                                                                       */
/*                                                                                                */
/*This command terminates when the EOT number of sectors have been read (EOTmax = FFhex = 255dec).*/
/* If the FDC does not find an ID Address Mark on the diskette after it encounters the INDEX HOLE */
/*for the second time, then it sets the MA (missing address mark) flag in Status Register 1 to a 1*/
/* (high),  and termi-nates the command.(Status Register 0 has bits 7 and 6 set to 0 and 1        */
/* respectively).                                                                                 */
/* ---------------------------------------------------------------------------------------------- */
static
void cmd_read_track_tc (e8272_t *fdc)
{
#if E8272_DEBUG >= 2
	fprintf (stderr, "E8272: CMD=%02X D=%u  READ TRACK TC\n",
		fdc->cmd[0], fdc->cmd[1] & 3
	);
#endif
	/* head and unit */
	fdc->st[0] = (fdc->st[0] & ~0x07) | (fdc->cmd[1] & 0x07);
	fdc->res[0] = fdc->st[0];  /* ST 0:   Status   information after Command execution */ /* IBM PS/2: Status Register 0              */
	fdc->res[1] = fdc->st[1];  /* ST 1:   Status   information after Command execution */ /* IBM PS/2: Status Register 1              */
	fdc->res[2] = fdc->st[2];  /* ST 2:   Status   information after Command execution */ /* IBM PS/2: Status Register 2              */
	fdc->res[3] = fdc->cmd[2]; /*  C  : Sector ID  information after Command execution */ /* IBM PS/2: Cylinder Number                */
	fdc->res[4] = fdc->cmd[3]; /*  H  : Sector ID  information after Command execution */ /* IBM PS/2: Head Address                   */
	fdc->res[5] = fdc->cmd[4]; /*  R  : Sector ID  information after Command execution */ /* IBM PS/2: Sector Number                  */
	fdc->res[6] = fdc->cmd[5]; /*  N  : Sector ID  information after Command execution */ /* IBM PS/2: Number of Data Bytes in Sector */
	cmd_result (fdc, 7);
	e8272_set_irq (fdc, 1);
}
/* --------------------------------------------------------------------------------------------------- */
/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; Command Format:                */
/* --------------------------------------------------------------------------------------------------- */
/* The following are commands that may be issued to the controller. An X is used to indicate a don't   */
/* care condition.                                                                                     */
/* --------------------------------------------------------------------------------------------------- */
/* Read A Track:                                                                                       */
/* --------------------------------------------------------------------------------------------------- */
/* Command Phase:                                                                                      */
/*  - MF  = MFM Mode                                                                                   */
/*  - SK  = Skip Deleted-Data Address Mark                                                             */
/*  - HD  = Head Number                                                                                */
/*  - USx = Unit Select                                                                                */
/* -----------7--6--5--4---3--2---1---0--------------------------------------------------------------- */
/*  - Byte 0: 0  MF SK  0  0  0   1   0                                                                */
/*  - Byte 1: X  X   X  X  X  HD US1 US0                                                               */
/*  - Byte 2: Cylinder Number                                                                          */
/*  - Byte 3: Head Address                                                                             */
/*  - Byte 4: Sector   Number                                                                          */
/*  - Byte 5: Number of Data Bytes in Sector                                                           */
/*  - Byte 6: End of Track                                                                             */
/*  - Byte 7: Gap  Length                                                                              */
/*  - Byte 8: Data Length                                                                              */
/* --------------------------------------------------------------------------------------------------- */
/* Figure 1-73. Read a Track Command:                                                                  */
/* --------------------------------------------------------------------------------------------------- */
/* Result Phase:                                                                                       */
/* -----------7--6--5--4---3--2---1---0--------------------------------------------------------------- */
/*  - Byte 0: Status Register 0                                                                        */
/*  - Byte 1: Status Register 1                                                                        */
/*  - Byte 2: Status register 2                                                                        */
/*  - Byte 3: Cylinder Number                                                                          */
/*  - Byte 4: Head Address                                                                             */
/*  - Byte 5: Sector   Number                                                                          */
/*  - Byte 6: Number of Data Bytes in Sector                                                           */
/* --------------------------------------------------------------------------------------------------- */
/* Figure 1-74. Read a Track Result:                                                                   */
/* --------------------------------------------------------------------------------------------------- */
static
void cmd_read_track_error (e8272_t *fdc, unsigned err)
{
#if E8272_DEBUG >= 1
	fprintf (stderr, "E8272: read track error (%04X)\n", err);
#endif
	/* abnormal termination */
	fdc->st[0] = (fdc->st[0] & 0x3f) | 0x40;
	if (err & E8272_ERR_NO_ID) {
		fdc->st[1] |= E8272_ST1_MA;
	}
	else {
		fdc->st[1] |= E8272_ST1_EN | E8272_ST1_ND;
	}
	cmd_read_track_tc (fdc);
}

static
unsigned char cmd_read_track_get_data (e8272_t *fdc)
{
	unsigned char val;
	val = fdc->buf[fdc->buf_i++];
	if (fdc->dma) {
		e8272_set_dreq (fdc, 0);
	}
	if (fdc->buf_i < fdc->buf_n) {
		e8272_request_data (fdc, 1);
		return (val);
	}
	fdc->msr &= ~(E8272_MSR_RQM | E8272_MSR_DIO); /* Main Status Register: DB7: Request for Master: RQM: */
	fdc->read_track_cnt += 1;                     /* Main Status Register: DB6: Data Intput/Output: DIO: */
	if (fdc->read_track_cnt >= fdc->cmd[6]) {
		cmd_read_track_tc (fdc);
		return (val);
	}
	fdc->get_data = NULL;
	fdc->set_clock = cmd_read_track_clock;
	return (val);
}

static
void cmd_read_track_clock (e8272_t *fdc, unsigned long cnt)
{
	unsigned    id;
	unsigned    bcnt, err;
	e8272_sct_t *sct;
	if (fdc->index_cnt >= 2) {
		cmd_read_track_error (fdc, E8272_ERR_NO_ID);
		return;
	}
	if (fdc->delay_clock > 0) {
		return;
	}
	id = e8272_get_current_id (fdc);
	e8272_delay_next_id (fdc, 0);
	if ((fdc->cmd[0] & E8272_CMD0_MFM) == 0) {
		return;
	}
	if (id >= fdc->curdrv->sct_cnt) {
		return;
	}
	fdc->index_cnt = 0;
	sct = &fdc->curdrv->sct[id];
	if ((sct->c == fdc->cmd[2]) && (sct->h == fdc->cmd[3])) {
		if (sct->s == fdc->cmd[4]) {
			fdc->st[1] &= ~E8272_ST1_ND;
		}
	}
#if E8272_DEBUG >= 1
	fprintf (stderr,
		"E8272: CMD=%02X D=%u  READ TRACK CONT"
		" (pc=%u, ph=%u, c=%u, h=%u, s=%u, n=%u eot=%u rs=%u)\n",
		fdc->cmd[0], fdc->curdrv->d, fdc->curdrv->c, fdc->curdrv->h,
		fdc->cmd[2], fdc->cmd[3], fdc->cmd[4], fdc->cmd[5], fdc->cmd[6], sct->s
	);
#endif
	bcnt = 8192;
	err = e8272_diskop_read (fdc, fdc->buf, &bcnt,
		fdc->curdrv->d, fdc->curdrv->c, fdc->curdrv->h,
		id, sct->s
	);
	if (err & E8272_ERR_CRC_ID) {
		fdc->st[1] |= E8272_ST1_DE;
	}
	else if (err & E8272_ERR_CRC_DATA) {
		fdc->st[1] |= E8272_ST1_DE;
		fdc->st[2] |= E8272_ST2_DD;
	}
	else if (err & E8272_ERR_CRC_ID) {
		fdc->st[1] |= E8272_ST1_DE;
	}
	fdc->buf_i = 0;
	fdc->buf_n = bcnt;
	err &= ~(E8272_ERR_DATALEN | E8272_ERR_CRC_DATA | E8272_ERR_CRC_ID);
	if (err) {
		cmd_read_track_error (fdc, err);
		return;
	}
	if (fdc->buf_n > 0) {
		e8272_request_data (fdc, 1);
		fdc->set_clock = NULL;
		fdc->get_data = cmd_read_track_get_data;
	}
}

static
void cmd_read_track (e8272_t *fdc)
{
	e8272_select_head (fdc, fdc->cmd[1] & 3, (fdc->cmd[1] >> 2) & 1);
	e8272_read_track (fdc);
#if E8272_DEBUG >= 1
	fprintf (stderr,
		"E8272: CMD=%02X D=%u  READ TRACK"
		" (pc=%u, ph=%u, c=%u, h=%u s=%u, n=%u, eot=%u)\n",
		fdc->cmd[0], fdc->curdrv->d, fdc->curdrv->c, fdc->curdrv->h,
		fdc->cmd[2], fdc->cmd[3], fdc->cmd[4], fdc->cmd[5], fdc->cmd[6]
	);
#endif
	fdc->st[0] = 0;
	fdc->st[1] = E8272_ST1_ND;
	fdc->st[2] = 0;
	fdc->read_track_cnt = 0;
	fdc->index_cnt = 0;
	e8272_delay_id (fdc, 0, 1);
	fdc->set_clock = cmd_read_track_clock;
	fdc->set_tc = cmd_read_track_tc;
}
/*****************************************************************************
 * read id
 *****************************************************************************/
/* ______________________________DATA BUS_________________________________________________________*/
/* |__PHASE__|_R/W_|_D7__D6__D5__D4__D3__D2__D1__D0__|__________________REMARKS__________________|*/
/* |______________________________READ ID________________________________________________________|*/
/* |         |     |                                 |                                           |*/
/* | Command |  W  |  0  MF   0   0   1   0   1   0  | Command Codes                             |*/
/* |         |  W  |  X   X   X   X   X   HD  US1 US0|                                           |*/
/* |         |  W  | _______________C_______________ | Sector ID information prior to Command    |*/
/* |         |  W  | _______________H_______________ | execution                                 |*/
/* |         |  W  | _______________R_______________ |                                           |*/
/* |         |  W  | _______________N_______________ |                                           |*/
/* |         |  W  | ______________EOT______________ |                                           |*/
/* |         |  W  | ______________GPL______________ |                                           |*/
/* |         |  W  | ______________DTL______________ |                                           |*/
/* |         |     |                                 |                                           |*/
/* |Execution|     |                                 | Data-transfer between the FDD and main-   |*/
/* |         |     |                                 | system.                                   |*/
/* |         |     |                                 | FDC reads all of cylinders contents from  |*/
/* |         |     |                                 | index hole to EOT.                        |*/
/* | Result  |  R  | _____________ST  0_____________ | Status information after command execution|*/
/* |         |  R  | _____________ST  1_____________ |                                           |*/
/* |         |  R  | _____________ST  2_____________ |                                           |*/
/* |         |  R  | _______________C_______________ | Sector ID information after command       |*/
/* |         |  R  | _______________H_______________ | execution                                 |*/
/* |         |  R  | _______________R_______________ |                                           |*/
/* |         |  R  | _______________N_______________ |                                           |*/
/* |_________|_____|_________________________________|___________________________________________|*/
/* ---------------------------------------------------------------------------------------------- */
/* FUNCTIONAL DESCRIPTION OF COMMANDS (CON.T):  READ ID:                                          */
/* ---------------------------------------------------------------------------------------------- */
/*The  READ ID Command is used to give the present position of the recording head. The FDC stores */
/* the values from the first ID Field it is able to read. If no proper ID Address Mark is found on*/
/* the diskette, before the INDEX HOLE is encountered for the second time then the MA (Missing    */
/* Address Mark) flag in Status Register 1 is set to a 1 (high), and if no data is found then the */
/*ND (No Data) flag is also set in Status Register 1 to a 1 (high). The command is then terminated*/
/* with Bits 7 and 6 in Status Register 0 set to 0 and 1 respectively.                            */
/* ---------------------------------------------------------------------------------------------- */
static
void cmd_read_id_error (e8272_t *fdc)
{
#if E8272_DEBUG >= 1
	fprintf (stderr,
		"E8272: CMD=%02X D=%u  READ ID ERROR (pc=%u ph=%u)\n",
		fdc->cmd[0], fdc->curdrv->d, fdc->curdrv->c, fdc->curdrv->h
	);
#endif
	fdc->st[0] = 0x40 | (fdc->cmd[1] & 7);
	fdc->st[1] |= E8272_ST1_MA | E8272_ST1_ND;
	fdc->res[0] = fdc->st[0]; /* IBM PS/2: Status Register 0              */
	fdc->res[1] = fdc->st[1]; /* IBM PS/2: Status Register 1              */
	fdc->res[2] = fdc->st[2]; /* IBM PS/2: Status Register 2              */
	fdc->res[3] = 0;          /* IBM PS/2: Cylinder Number                */
	fdc->res[4] = 0;          /* IBM PS/2: Head Address                   */
	fdc->res[5] = 0;          /* IBM PS/2: Sector   Number                */
	fdc->res[6] = 0;          /* IBM PS/2: Number of Data Bytes in Sector */
	cmd_result (fdc, 7);
	e8272_set_irq (fdc, 1);
}
/* --------------------------------------------------------------------------------------------------- */
/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; Command Format:                */
/* --------------------------------------------------------------------------------------------------- */
/* The following are commands that may be issued to the controller. An X is used to indicate a don't   */
/* care condition.                                                                                     */
/* --------------------------------------------------------------------------------------------------- */
/* Read ID:                                                                                            */
/* --------------------------------------------------------------------------------------------------- */
/* Command Phase:                                                                                      */
/*  - MF  = MFM Mode                                                                                   */
/*  - HD  = Head Number                                                                                */
/*  - USx = Unit Select                                                                                */
/* -----------7--6--5--4---3--2---1---0--------------------------------------------------------------- */
/*  - Byte 0: 0  MF  0  0  1  0   1   0                                                                */
/*  - Byte 1: X  X   X  X  X  HD US1 US0                                                               */
/* --------------------------------------------------------------------------------------------------- */
/* Figure 1-75. Read ID Command:                                                                       */
/* --------------------------------------------------------------------------------------------------- */
/* Result Phase:                                                                                       */
/* -----------7--6--5--4---3--2---1---0--------------------------------------------------------------- */
/*  - Byte 0: Status Register 0                                                                        */
/*  - Byte 1: Status Register 1                                                                        */
/*  - Byte 2: Status register 2                                                                        */
/*  - Byte 3: Cylinder Number                                                                          */
/*  - Byte 4: Head Address                                                                             */
/*  - Byte 5: Sector   Number                                                                          */
/*  - Byte 6: Number of Data Bytes in Sector                                                           */
/* --------------------------------------------------------------------------------------------------- */
/* Figure 1-76. Read ID Result:                                                                        */
/* --------------------------------------------------------------------------------------------------- */
static
void cmd_read_id_clock (e8272_t *fdc, unsigned long cnt)
{
	unsigned    id;
	e8272_sct_t *sct;
	if (fdc->index_cnt >= 2) {
		cmd_read_id_error (fdc);
		return;
	}
	if (fdc->delay_clock > 0) {
		return;
	}
	e8272_delay_next_id (fdc, 1);
	if ((fdc->cmd[0] & E8272_CMD0_MFM) == 0) {
		return;
	}
	id = e8272_get_current_id (fdc);
	if (id >= fdc->curdrv->sct_cnt) {
		return;
	}
	sct = &fdc->curdrv->sct[id];
#if E8272_DEBUG >= 1
	fprintf (stderr,
		"E8272: CMD=%02X D=%u  READ ID (pc=%u ph=%u id=[%02x %02x %02x %02x])\n",
		fdc->cmd[0], fdc->curdrv->d, fdc->curdrv->c, fdc->curdrv->h,
		sct->c, sct->h, sct->s, sct->n
	);
#endif
	fdc->st[0] = (fdc->st[0] & ~0x07) | (fdc->cmd[1] & 0x07);
	fdc->res[0] = fdc->st[0]; /* IBM PS/2: Status Register 0              */
	fdc->res[1] = fdc->st[1]; /* IBM PS/2: Status Register 1              */
	fdc->res[2] = fdc->st[2]; /* IBM PS/2: Status Register 2              */
	fdc->res[3] = sct->c;     /* IBM PS/2: Cylinder Number                */
	fdc->res[4] = sct->h;     /* IBM PS/2: Head Address                   */
	fdc->res[5] = sct->s;     /* IBM PS/2: Sector   Number                */
	fdc->res[6] = sct->n;     /* IBM PS/2: Number of Data Bytes in Sector */
	cmd_result (fdc, 7);
	e8272_set_irq (fdc, 1);
}

static
void cmd_read_id (e8272_t *fdc)
{
	e8272_select_head (fdc, fdc->cmd[1] & 3, (fdc->cmd[1] >> 2) & 1);
	e8272_read_track (fdc);
#if E8272_DEBUG >= 2
	fprintf (stderr, "E8272: CMD=%02X D=%u  READ ID (pc=%u, ph=%u)\n",
		fdc->cmd[0], fdc->curdrv->d, fdc->curdrv->c, fdc->curdrv->h
	);
#endif
	fdc->index_cnt = 0;
	fdc->st[0] = 0; /* IBM PS/2: Status Register 0 */
	fdc->st[1] = 0; /* IBM PS/2: Status Register 1 */
	fdc->st[2] = 0; /* IBM PS/2: Status Register 2 */
	e8272_delay_next_id (fdc, 1);
	fdc->set_clock = cmd_read_id_clock;
}
/*****************************************************************************
 * write
 *****************************************************************************/
static void cmd_write_clock (e8272_t *fdc, unsigned long cnt);
/* ______________________________DATA BUS_________________________________________________________*/
/* |__PHASE__|_R/W_|_D7__D6__D5__D4__D3__D2__D1__D0__|__________________REMARKS__________________|*/
/* |______________________________WRITE DATA_____________________________________________________| */
/* | Command |  W  | MT  MF   0   0   0   1   0   0  | Command Codes                             | */
/* |         |  W  |  X   X   X   X   X   HD  US1 US0|                                                 | */
/* |         |  W  | _______________C_______________ | Sector ID information prior to Command    |*/
/* |         |  W  | _______________H_______________ | execution                                 |*/
/* |         |  W  | _______________R_______________ |                                           |*/
/* |         |  W  | _______________N_______________ |                                           |*/
/* |         |  W  | ______________EOT______________ |                                           |*/
/* |         |  W  | ______________GPL______________ |                                           |*/
/* |         |  W  | ______________DTL______________ |                                           |*/
/* |         |     |                                 |                                           |*/
/* |Execution|     |                                 | Data-transfer between the FDD and main-   |*/
/* |         |     |                                 | system.                                   |*/
/* |         |     |                                 | FDC reads all of cylinders contents from  |*/
/* |         |     |                                 | index hole to EOT.                        |*/
/* | Result  |  R  | _____________ST  0_____________ | Status information after command execution|*/
/* |         |  R  | _____________ST  1_____________ |                                           |*/
/* |         |  R  | _____________ST  2_____________ |                                           |*/
/* |         |  R  | _______________C_______________ | Sector ID information after command       |*/
/* |         |  R  | _______________H_______________ | execution                                 |*/
/* |         |  R  | _______________R_______________ |                                           |*/
/* |         |  R  | _______________N_______________ |                                           |*/
/* |_________|_____|_________________________________|___________________________________________|*/
static
void cmd_write_tc (e8272_t *fdc)
{
#if E8272_DEBUG >= 2
	fprintf (stderr, "E8272: CMD=%02X D=%u WRITE TC\n",
		fdc->cmd[0], fdc->cmd[1] & 3
	);
#endif
	fdc->st[0] = (fdc->st[0] & ~0x07) | (fdc->cmd[1] & 0x07);
	fdc->res[0] = fdc->st[0];  /* IBM PS/2: Status Register 0              */
	fdc->res[1] = fdc->st[1];  /* IBM PS/2: Status Register 1              */
	fdc->res[2] = fdc->st[2];  /* IBM PS/2: Status Register 2              */
	fdc->res[3] = fdc->cmd[2]; /* IBM PS/2: Cylinder Number                */
	fdc->res[4] = fdc->cmd[3]; /* IBM PS/2: Head Address                   */
	fdc->res[5] = fdc->cmd[4]; /* IBM PS/2: Sector   Number                */
	fdc->res[6] = fdc->cmd[5]; /* IBM PS/2: Number of Data Bytes in Sector */
	cmd_result (fdc, 7);
	e8272_set_irq (fdc, 1);
}
/* --------------------------------------------------------------------------------------------------- */
/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; Command Format:                */
/* --------------------------------------------------------------------------------------------------- */
/* The following are commands that may be issued to the controller. An X is used to indicate a don't   */
/* care condition.                                                                                     */
/* --------------------------------------------------------------------------------------------------- */
/* Write Data:                                                                                         */
/* --------------------------------------------------------------------------------------------------- */
/* Command Phase:                                                                                      */
/*  - MT  = Multitrack                                                                                 */
/*  - MF  = MFM Mode                                                                                   */
/*  - HD  = Head Number                                                                                */
/*  - USx = Unit Select                                                                                */
/* -----------7--6--5--4---3--2---1---0--------------------------------------------------------------- */
/*  - Byte 0: MT MF  0  0  0  1   0   0                                                                */
/*  - Byte 1: X  X   X  X  X  HD US1 US0                                                               */
/*  - Byte 2: Cylinder Number                                                                          */
/*  - Byte 3: Head Address                                                                             */
/*  - Byte 4: Sector   Number                                                                          */
/*  - Byte 5: Number of Data Bytes in Sector                                                           */
/*  - Byte 6: End of Track                                                                             */
/*  - Byte 7: Gap  Length                                                                              */
/*  - Byte 8: Data Length                                                                              */
/* --------------------------------------------------------------------------------------------------- */
/* Figure 1-77. Write Data Command:                                                                    */
/* --------------------------------------------------------------------------------------------------- */
/* Result Phase:                                                                                       */
/* -----------7--6--5--4---3--2---1---0--------------------------------------------------------------- */
/*  - Byte 0: Status Register 0                                                                        */
/*  - Byte 1: Status Register 1                                                                        */
/*  - Byte 2: Status register 2                                                                        */
/*  - Byte 3: Cylinder Number                                                                          */
/*  - Byte 4: Head Address                                                                             */
/*  - Byte 5: Sector   Number                                                                          */
/*  - Byte 6: Number of Data Bytes in Sector                                                           */
/* --------------------------------------------------------------------------------------------------- */
/* Figure 1-78. Write Data Result:                                                                     */
/* --------------------------------------------------------------------------------------------------- */
static
void cmd_write_error (e8272_t *fdc, unsigned err)
{
#if E8272_DEBUG >= 1
	fprintf (stderr,
		"E8272: CMD=%02X D=%u  WRITE ERROR"
		" (pc=%u, ph=%u c=%u, h=%u, s=%u, n=%u eot=%u)\n",
		fdc->cmd[0], fdc->curdrv->d, fdc->curdrv->c, fdc->curdrv->h,
		fdc->cmd[2], fdc->cmd[3], fdc->cmd[4], fdc->cmd[5], fdc->cmd[6]
	);
#endif
	fdc->st[0] = (fdc->st[0] & 0x3f) | 0x40;
	if (err & E8272_ERR_WPROT) {
		fdc->st[1] |= E8272_ST1_NW;
	}
	if (err & E8272_ERR_NO_ID) {
		fdc->st[1] |= E8272_ST1_MA;
	}
	cmd_write_tc (fdc);
}
/* ---------------------------------------------------------------------------------------------- */
/* WRITE DATA                                                                                     */
/* ---------------------------------------------------------------------------------------------- */
/* A set of nine (9) bytes are required to set the FDC into the Write Data mode. After the Write  */
/* Data command has been issued the FDC loads the head (if it is in the unloaded state), waits the*/
/* specified heat settling time (defined in the Specify Command), and begins reading ID Fields.   */
/* When the current sector number ("R"), stored in the ID Register (IRD) compares with the sector */
/* number read off the diskette, then the FDC takes data from the processor byte-bybyte via the   */
/* data bus, and outputs it to the FDD.                                                           */
/*                                                                                                */
/* After writing data into the current sector, the Sector Number stored in "R" is incremented by  */
/* one, and the next data field is written into. The FDC continues this "Multi-Sector Write       */
/* Operation" until the issuance of a Terminal Count signal. If a Terminal Count signal is sent to*/
/*the FDC it continues writing into the current sector to complete the data field. If the Terminal*/
/* Count signal is received while a data field is being written then the remainder of the data    */
/* field is filled with 00 (zeros).                                                               */
/*                                                                                                */
/* The FDC reads the ID field of each sector and checks the CRC bytes. If the FDC detects a read  */
/* error (incorrect CRC) in one of the ID Fields, it sets the DE (Data Error) flag of Status      */
/* Register 1 to a 1 (high), and terminates the Write Data Command. (Status Register 0 also has   */
/* bits 7 and 6 set to 0 and 1 respectively.)                                                     */
/*                                                                                                */
/* The Write Command operates in much the same manner as the Read Command. The following items are*/
/* the same, and one should referto the Read Data Command for details:                            */
/*  - Transfer Capacity         - Head Unload Time Interval                                       */
/*  - EN (End of Cylinder) Flag - ID Information when the processor terminates command (see Table */
/*                              2)                                                                */
/*  - ND     (No Data)     Flag - Definition of DTL when N = 0 and when N =/= 0                   */
/* In the Write Data mode, data transfers between the processor and FDC, via the Data Bus, must   */
/* occur every 31 us in                                                                           */
/* the FM mode, and every 15 us in the MFM mode. If the time interval between data transfers is   */
/* longer than this then the FDC sets the OR (Over Run) flag in Status Register 1 to a 1 (high),  */
/* and terminates the Write Data Command. (Status Register 0 also has bit 7 and 6 set to 0 and 1  */
/* respectively.)                                                                                 */
/* ---------------------------------------------------------------------------------------------- */
static
void cmd_write_set_data (e8272_t *fdc, unsigned char val)
{
	unsigned err, cnt;
	fdc->buf[fdc->buf_i++] = val;
	if (fdc->buf_i < fdc->buf_n) {
		return;
	}
	if (fdc->dma) {
		e8272_set_dreq (fdc, 0);
	}
	cnt = fdc->buf_n;
	err = e8272_diskop_write (fdc, fdc->buf, &cnt,
		fdc->curdrv->d, fdc->curdrv->c, fdc->curdrv->h,
		fdc->write_id, fdc->cmd[4] /* IBM PS/2: Head Address */
	);
	if (err) {
		cmd_write_error (fdc, err);
	}
	else {
		e8272_next_id (fdc);
		fdc->set_data = NULL;
		fdc->set_clock = cmd_write_clock;
	}
}
/* ---------------------------------------------------------------------------------------------- */
/* WRITE DELETED DATA:                                                                            */
/* ---------------------------------------------------------------------------------------------- */
/*This command is the same as the Write Data Command except a Deleted Data Address Mark is written*/
/* at the beginning of the Data Field instead of the normal Data Address Mark.                    */
/* ---------------------------------------------------------------------------------------------- */
static
void cmd_write_clock (e8272_t *fdc, unsigned long cnt)
{
	unsigned    id;
	unsigned    c, h, s, n;
	e8272_sct_t *sct;
	if (fdc->index_cnt >= 2) {
		cmd_write_error (fdc, E8272_ERR_NO_ID);
		return;
	}
	if (fdc->delay_clock > 0) {
		return;
	}
	id = e8272_get_current_id (fdc);
	e8272_delay_next_id (fdc, 0);
	if ((fdc->cmd[0] & E8272_CMD0_MFM) == 0) {
		return;
	}
	c = fdc->cmd[2]; /* IBM PS/2: Cylinder Number                */
	h = fdc->cmd[3]; /* IBM PS/2: Head Address                   */
	s = fdc->cmd[4]; /* IBM PS/2: Sector   Number                */
	n = fdc->cmd[5]; /* IBM PS/2: Number of Data Bytes in Sector */
	if (id >= fdc->curdrv->sct_cnt) {
		return;
	}
	sct = &fdc->curdrv->sct[id];
	if ((sct->c != c) || (sct->h != h) || (sct->s != s)) {
		/* wrong id */
		return;
	}
	if (sct->n < n) {
		/* sector too small */
		return;
	}
	fdc->index_cnt = 0;
	if (n > 6) {
		cmd_write_error (fdc, E8272_ERR_OTHER);
		return;
	}
	fdc->buf_i = 0;
	if (n == 0) {
		fdc->buf_n = fdc->cmd[8]; /* IBM PS/2: Data Length */
	}
	else {
		fdc->buf_n = 128 << n;
	}
	fdc->write_id = id;
	fdc->set_data = cmd_write_set_data;
	fdc->set_clock = NULL;
	e8272_request_data (fdc, 0);
}
/* --------------------------------------------------------------------------------------------------- */
/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; Command Format:                */
/* --------------------------------------------------------------------------------------------------- */
/* The following are commands that may be issued to the controller. An X is used to indicate a don't   */
/* care condition.                                                                                     */
/* --------------------------------------------------------------------------------------------------- */
/* Write Deleted Data:                                                                                 */
/* --------------------------------------------------------------------------------------------------- */
/* Command Phase:                                                                                      */
/*  - MT  = Multitrack                                                                                 */
/*  - MF  = MFM Mode                                                                                   */
/*  - HD  = Head Number                                                                                */
/*  - USx = Unit Select                                                                                */
/* -----------7--6--5--4---3--2---1---0--------------------------------------------------------------- */
/*  - Byte 0: MT MF  0  0  1  0   0   1                                                                */
/*  - Byte 1: X  X   X  X  X  HD US1 US0                                                               */
/*  - Byte 2: Cylinder Number                                                                          */
/*  - Byte 3: Head Address                                                                             */
/*  - Byte 4: Sector   Number                                                                          */
/*  - Byte 5: Number of Data Bytes in Sector                                                           */
/*  - Byte 6: End of Track                                                                             */
/*  - Byte 7: Gap  Length                                                                              */
/*  - Byte 8: Data Length                                                                              */
/* --------------------------------------------------------------------------------------------------- */
/* Figure 1-77. Write Data Command:                                                                    */
/* --------------------------------------------------------------------------------------------------- */
/* Result Phase:                                                                                       */
/* -----------7--6--5--4---3--2---1---0--------------------------------------------------------------- */
/*  - Byte 0: Status Register 0                                                                        */
/*  - Byte 1: Status Register 1                                                                        */
/*  - Byte 2: Status register 2                                                                        */
/*  - Byte 3: Cylinder Number                                                                          */
/*  - Byte 4: Head Address                                                                             */
/*  - Byte 5: Sector   Number                                                                          */
/*  - Byte 6: Number of Data Bytes in Sector                                                           */
/* --------------------------------------------------------------------------------------------------- */
/* Figure 1-78. Write Data Result:                                                                     */
/* --------------------------------------------------------------------------------------------------- */
/* ______________________________DATA BUS_________________________________________________________*/
/* |__PHASE__|_R/W_|_D7__D6__D5__D4__D3__D2__D1__D0__|__________________REMARKS__________________|*/
/* |______________________WRITE_DELETED_DATA_____________________________________________________|*/
/* |         |     |                                 |                                           |*/
/* | Command |  W  | MT  MF   0   0   1   0   0   1  | Command Codes                             |*/
/* |         |  W  | _______________C_______________ | Sector ID information prior to Command    |*/
/* |         |  W  | _______________H_______________ | execution                                 |*/
/* |         |  W  | _______________R_______________ |                                           |*/
/* |         |  W  | _______________N_______________ |                                           |*/
/* |         |  W  | ______________EOT______________ |                                           |*/
/* |         |  W  | ______________GPL______________ |                                           |*/
/* |         |  W  | ______________DTL______________ |                                           |*/
/* |         |     |                                 |                                           |*/
/* |Execution|     |                                 | Data-transfer between the FDD and main-   |*/
/* |         |     |                                 | system.                                   |*/
/* |         |     |                                 | FDC reads all of cylinders contents from  |*/
/* |         |     |                                 | index hole to EOT.                        |*/
/* | Result  |  R  | _____________ST  0_____________ | Status information after command execution|*/
/* |         |  R  | _____________ST  1_____________ |                                           |*/
/* |         |  R  | _____________ST  2_____________ |                                           |*/
/* |         |  R  | _______________C_______________ | Sector ID information after command       |*/
/* |         |  R  | _______________H_______________ | execution                                 |*/
/* |         |  R  | _______________R_______________ |                                           |*/
/* |         |  R  | _______________N_______________ |                                           |*/
/* |_________|_____|_________________________________|___________________________________________|*/
static
void cmd_write (e8272_t *fdc)
{
	e8272_select_head (fdc, fdc->cmd[1] & 3, (fdc->cmd[1] >> 2) & 1);
	e8272_read_track (fdc);
#if E8272_DEBUG >= 1
	fprintf (stderr,
		"E8272: CMD=%02X D=%u  WRITE"
		" (pc=%u, ph=%u c=%u, h=%u, s=%u, n=%u)\n",
		fdc->cmd[0], fdc->curdrv->d, fdc->curdrv->c, fdc->curdrv->h,
		fdc->cmd[2], fdc->cmd[3], fdc->cmd[4], fdc->cmd[5]
	);
#endif
	fdc->st[0] = 0; /* IBM PS/2: Status Register 0 */
	fdc->st[1] = 0; /* IBM PS/2: Status Register 1 */
	fdc->st[2] = 0; /* IBM PS/2: Status Register 2 */
	fdc->index_cnt = 0;
	e8272_delay_next_id (fdc, 0);
	fdc->set_tc = cmd_write_tc;
	fdc->set_clock = cmd_write_clock;
}
/*****************************************************************************
 * format
 *****************************************************************************/
/* ---------------------------------------------------------------------------------------------- */
/* FUNCTIONAL DESCRIPTION OF COMMANDS (CON.T):FORMAT A TRACK:                                     */
/* ---------------------------------------------------------------------------------------------- */
/*The Format Command allows an entire track to be formatted. After the INDEX HOLE is detected,    */
/*Data is written on the Diskette; Gaps, Address Marks, ID Fields and Data Fields, all per the IBM*/
/* System 34 (Double Density) or System 3740 (Single Density) Format are recorded. The particular */
/* format which will be written is controlled by the values programmed into N (number of bytes/   */
/*sector), SC (sectors/cylinder), GPL (Gap Length), and D (Data Pattern) which are supplied by the*/
/* processor during the Command Phase. The Data Field is filled with the Byteof data stored in D. */
/* The ID Field for each sector is supplied by the processor; that is, four data requests per     */
/* sector are made by the FDC for C (Cylinder Number), H (Head Number), R (Sector Number) and N   */
/* (Number of Bytes/Sector). This allows the diskette to be formatted with non-sequential sector  */
/* numbers, if desired.                                                                           */
/*                                                                                                */
/* After formatting each sector, the processor must send new values for C, H, R, and N to the     */
/* uPD765 for each sector on the track. The contents of the R register is incremented by one after*/
/* each sector is formatted, thus, the R register contains a value of R + 1 when it is read during*/
/* the Result Phase. This incrementing and formatting continues for the whole cylinder until the  */
/* FDC encounters the INDEX HOLE for the second time, whereupon it terminates the command.        */
/*                                                                                                */
/* If a FAULT signal is received from the FDD at the end of a write operation, then the FDC sets  */
/* the EC flag of Status Register 0 to a 1 (high), and terminates the command after setting bits 7*/
/* and 6 of Status Register 0 to 0 and 1 respectively. Also the loss of a READY signal at the     */
/* beginning of a command execution phase causes bits 7 and 6 of Status Register 0 to be set to 0 */
/* and 1 respecitvely.                                                                            */
/*                                                                                                */
/* Table 3 shows the relationship between N, SC, and GPL for various sector sizes:                */
/* ---------------------------------------------------------------------------------------------- */
/* FORMAT:     SECTOR SIZE:         N:    SC:      GPL (1):   GPL (2):  REMARKS:                  */
/*  -  FM Mode: -  128 bytes/Sector: - 00: - 1A(16): -.07(16): - 1B(16): - IBM Diskette 1         */
/*  -  FM Mode: -  256:              - 01: - 0F(16): - 0E(16): - 2A(16): - IBM Diskette 2         */
/*  -  FM Mode: -  512:              - 02: - 08:     - 1B(16): - 3A(16): -                        */
/*  -  FM Mode: - 1024 bytes/Sector: - 03: - 04:     -         -         -                        */
/*  -  FM Mode: - 2048:              - 04: - 02:     -         -         -                        */
/*  -  FM Mode: - 4096:              - 05: - 01:     -         -         -                        */
/*  - MFM Mode: -  256:              - 01: - 1A(16): - 0E(16): - 36(16): - IBM Diskette 2D        */
/*  - MFM Mode: -  512:              - 02: - 0F(16): - 1B(16): - 54(16): -                        */
/*  - MFM Mode: - 1024:              - 03: - 08:     - 35(16): - 74(16): - IBM Diskette 2D        */
/*  - MFM Mode: - 2048:              - 04: - 04:     -         -         -                        */
/*  - MFM Mode: - 4096:              - 05: - 02:     -         -         -                        */
/*  - MFM Mode: - 8192:              - 06: - 01:     -         -         -                        */
/* ---------------------------------------------------------------------------------------------- */
/* NOTE: (1) Suggested values of GPL in Read or Write Commands to avoid splice point between data */
/*           field and ID field of contiguous sections.                                           */
/*       (2) Suggested valued of GPL in format command.                                           */
/* ---------------------------------------------------------------------------------------------- */
/* Table 3:                                                                                       */
/* ---------------------------------------------------------------------------------------------- */
static void cmd_format_clock (e8272_t *fdc, unsigned long cnt);
/* ______________________________DATA BUS_________________________________________________________*/
/* |__PHASE__|_R/W_|_D7__D6__D5__D4__D3__D2__D1__D0__|__________________REMARKS__________________|*/
/* |____________________________FORMAT_A_TRACK___________________________________________________|*/
/* | Command |  W  |  0  MF   0   0   1   1   0   1  | Command Codes                             |*/
/* |         |  W  |  X   X   X   X   X   HD  US1 US0|                                           |*/
/* |         |  W  | _______________C_______________ | Sector ID information prior to Command    |*/
/* |         |  W  | _______________H_______________ | execution                                 |*/
/* |         |  W  | _______________R_______________ |                                           |*/
/* |         |  W  | _______________N_______________ |                                           |*/
/* |         |  W  | ______________EOT______________ |                                           |*/
/* |         |  W  | ______________GPL______________ |                                           |*/
/* |         |  W  | ______________DTL______________ |                                           |*/
/* |         |     |                                 |                                           |*/
/* |Execution|     |                                 | Data-transfer between the FDD and main-   |*/
/* |         |     |                                 | system.                                   |*/
/* |         |     |                                 | FDC reads all of cylinders contents from  |*/
/* |         |     |                                 | index hole to EOT.                        |*/
/* | Result  |  R  | _____________ST  0_____________ | Status information after command execution|*/
/* |         |  R  | _____________ST  1_____________ |                                           |*/
/* |         |  R  | _____________ST  2_____________ |                                           |*/
/* |         |  R  | _______________C_______________ | Sector ID information after command       |*/
/* |         |  R  | _______________H_______________ | execution                                 |*/
/* |         |  R  | _______________R_______________ |                                           |*/
/* |         |  R  | _______________N_______________ |                                           |*/
/* |_________|_____|_________________________________|___________________________________________|*/
static
void cmd_format_tc (e8272_t *fdc)
{
#if E8272_DEBUG >= 2
	fprintf (stderr, "E8272: CMD=%02X D=%u  FORMAT TC\n",
		fdc->cmd[0], fdc->cmd[1] & 3
	);
#endif
	fdc->st[0] = (fdc->st[0] & ~0x07) | (fdc->cmd[1] & 0x07);
	fdc->res[0] = fdc->st[0];                /* IBM PS/2: Status Register 0   /Status Register 0              */
	fdc->res[1] = fdc->st[1];                /* IBM PS/2: Status Register 1   /Status Register 1              */
	fdc->res[2] = fdc->st[2];                /* IBM PS/2: Status Register 2   /Status Register 2              */
	fdc->res[3] = fdc->curdrv->c;            /* IBM PS/2: Sectors per Cylinder/Cylinder Number                */
	fdc->res[4] = (fdc->cmd[1] >> 2) & 0x01; /* IBM PS/2: Gap Length          /Head Address                   */
	fdc->res[5] = fdc->cmd[3] + 1;           /* IBM PS/2: Data                /Sector   Number                */
	fdc->res[6] = fdc->cmd[2];               /* IBM PS/2:                     /Number of Data Bytes in Sector */
	cmd_result (fdc, 7);
	e8272_set_irq (fdc, 1);
}
/* --------------------------------------------------------------------------------------------------- */
/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; Command Format:                */
/* --------------------------------------------------------------------------------------------------- */
/* The following are commands that may be issued to the controller. An X is used to indicate a don't   */
/* care condition.                                                                                     */
/* --------------------------------------------------------------------------------------------------- */
/* Format A Track:                                                                                     */
/* --------------------------------------------------------------------------------------------------- */
/* Command Phase:                                                                                      */
/*  - MT  = Multitrack                                                                                 */
/*  - HD  = Head Number                                                                                */
/*  - USx = Unit Select                                                                                */
/* -----------7--6--5--4---3--2---1---0--------------------------------------------------------------- */
/*  - Byte 0: 0  MF  0  0  1  1   0   1                                                                */
/*  - Byte 1: X  X   X  X  X  HD US1 US0                                                               */
/*  - Byte 2: Number of Data Bytes in Sector                                                           */
/*  - Byte 3: Sector per Cylinder                                                                      */
/*  - Byte 4: Gap  Length                                                                              */
/*  - Byte 5: Data                                                                                     */
/* --------------------------------------------------------------------------------------------------- */
/* Figure 1-81. Format a Track Command:                                                                */
/* --------------------------------------------------------------------------------------------------- */
/* Result Phase:                                                                                       */
/* -----------7--6--5--4---3--2---1---0--------------------------------------------------------------- */
/*  - Byte 0: Status Register 0                                                                        */
/*  - Byte 1: Status Register 1                                                                        */
/*  - Byte 2: Status register 2                                                                        */
/*  - Byte 3: Cylinder Number                                                                          */
/*  - Byte 4: Head Address                                                                             */
/*  - Byte 5: Sector   Number                                                                          */
/*  - Byte 6: Number of Data Bytes in Sector                                                           */
/* --------------------------------------------------------------------------------------------------- */
/* Figure 1-78. Format a Track Result:                                                                 */
/* --------------------------------------------------------------------------------------------------- */
static
void cmd_format_error (e8272_t *fdc, unsigned err)
{
#if E8272_DEBUG >= 1
	fprintf (stderr, "E8272: format error\n");
#endif
	fdc->st[0] = 0x40;
	if (err & E8272_ERR_WPROT) {
		fdc->st[1] |= E8272_ST1_NW;
	}
	cmd_format_tc (fdc);
}

static
void cmd_format_set_data (e8272_t *fdc, unsigned char val)
{
	unsigned d, c, h, n;
	unsigned gpl, fill;
	fdc->buf[fdc->buf_i++] = val;
	if (fdc->buf_i < 4) {
		return;
	}
	if (fdc->dma) {
		e8272_set_dreq (fdc, 0);
	}
	d = fdc->curdrv->d;
	c = fdc->curdrv->c;
	h = fdc->curdrv->h;
	n = fdc->cmd[2];    /* Number of Data Bytes in Sector */
	gpl = fdc->cmd[4];  /* IBM PS/2: Gap Length           */
	fill = fdc->cmd[5]; /* IBM PS/2: Data                 */
#if E8272_DEBUG >= 1
	fprintf (stderr,
		"E8272: CMD=%02X D=%u  FORMAT SECTOR "
		"(c=%u, h=%u, n=%u, sc=%u g=%u, f=0x%02x, id=[%02x %02x %02x %02x])\n",
		fdc->cmd[0], d, c, h, n, fdc->cmd[3], gpl, fill,
		fdc->buf[0], fdc->buf[1], fdc->buf[2], fdc->buf[3]
	);
#endif
	fdc->curdrv->ok = 0;
	if (e8272_diskop_format (fdc, d, c, h, fdc->format_cnt, fdc->buf, fill)) {
		cmd_format_error (fdc, E8272_ERR_WPROT);
		return;
	}
	fdc->res[3] = fdc->buf[0];     /* IBM PS/2: Cylinder Number      */
	fdc->res[4] = fdc->buf[1];     /* IBM PS/2: Head Address         */
	fdc->res[5] = fdc->buf[2] + 1; /* IBM PS/2: Sector   Number      */
	fdc->res[6] = fdc->buf[3];     /* Number of Data Bytes in Sector */
	fdc->format_cnt += 1;
	if (fdc->format_cnt >= fdc->cmd[3]) {
		e8272_delay_index (fdc, 0);
	}
	else {
		e8272_delay_bits (fdc, 8UL * (62 + (128 << n) + gpl), 0);
	}
	fdc->set_data = NULL;
	fdc->set_clock = cmd_format_clock;
}

static
void cmd_format_clock (e8272_t *fdc, unsigned long cnt)
{
	if (fdc->delay_clock > 0) {
		return;
	}
	if (fdc->format_cnt >= fdc->cmd[3]) {
		cmd_format_tc (fdc);
		return;
	}
	fdc->buf_i = 0;
	fdc->buf_n = 4;
	fdc->set_data = cmd_format_set_data;
	fdc->set_clock = NULL;
	e8272_request_data (fdc, 0);
}

static
void cmd_format (e8272_t *fdc)
{
	e8272_select_head (fdc, fdc->cmd[1] & 3, (fdc->cmd[1] >> 2) & 1);
#if E8272_DEBUG >= 1
	fprintf (stderr,
		"E8272: CMD=%02X D=%u  FORMAT (pc=%u, ph=%u, n=%u sc=%u gpl=%u d=%02x)\n",
		fdc->cmd[0], fdc->curdrv->d, fdc->curdrv->c, fdc->curdrv->h,
		fdc->cmd[2], fdc->cmd[3], fdc->cmd[4], fdc->cmd[5]
	);
#endif
	fdc->st[0] = 0; /* IBM PS/2: Status Register 0 */
	fdc->st[1] = 0; /* IBM PS/2: Status Register 1 */
	fdc->st[2] = 0; /* IBM PS/2: Status Register 2 */
	if ((fdc->cmd[0] & E8272_CMD0_MFM) == 0) {
		cmd_format_error (fdc, 0);
		return;
	}
	fdc->format_cnt = 0;
	e8272_delay_index (fdc, 0);
	fdc->set_tc = cmd_format_tc;
	fdc->set_clock = cmd_format_clock;
}
/* ______________________________DATA BUS_________________________________________________________*/
/* |__PHASE__|_R/W_|_D7__D6__D5__D4__D3__D2__D1__D0__|__________________REMARKS__________________|*/
/* |_____________________________RECALIBRATE_____________________________________________________|*/
/* | Command |  W  |  0   0   0   0   0   1   1   1  | Command Codes                             |*/
/* |         |  W  |  X   X   X   X   X   0   US1 US0|                                           |*/
/* |         |     |                                 |                                           |*/
/* |Execution|_____|_________________________________|_Head_retracted_to_Track_0.________________|*/
/*****************************************************************************
 * recalibrate
 *****************************************************************************/
/* ---------------------------------------------------------------------------------------------- */
/* FUNCTIONAL DESCRIPTION OF COMMANDS (CONT.): RECALIBRATE:                                       */
/* ---------------------------------------------------------------------------------------------- */
/* The function of this command is to retract the read/write head within the FDD to the Track 0   */
/* position. The FDC clears the contents of the PCN counter, and checks the status of the Track 0 */
/*signal from the FDD. As long as the Track 0 signal is low, the Direction signal remains 1 (high)*/
/* and Step Pulses are issued. When the Track 0 signal goes high, the SE (SEEK END) flag in Status*/
/* Register 0 is set to a 1 (high) and the command is terminated. If the Track 0 signal is still  */
/* low after 77 Step Pulse have been issued, the FDC sets the SE (SEEK END) and EC (EQUIPMENT     */
/* CHECK) flags of Status Register 0 to both 1s (highs), and terminates the command after bits 7  */
/* and 6 of Status Register 0 is set to 0 and 1 respectively.                                     */
/*                                                                                                */
/* The ability to do overlap RECALIBRATE Commands to multiple FDDs and the loss of the READY      */
/* signal, as described in the SEEK Command, also applies to the RECALIBRATE Command.             */
/* ---------------------------------------------------------------------------------------------- */
static
void cmd_recalibrate_clock (e8272_t *fdc, unsigned long cnt)
{
	unsigned drv;
	if (fdc->delay_clock > 0) {
		return;
	}
	drv = fdc->cmd[1] & 0x03;
	fdc->msr &= ~(0x01 << drv);
	fdc->st[0] = (fdc->cmd[1] & 0x07) | E8272_ST0_SE;
	e8272_set_irq (fdc, 1);
	cmd_done (fdc);
}
/* --------------------------------------------------------------------------------------------------- */
/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; Command Format:                */
/* --------------------------------------------------------------------------------------------------- */
/* The following are commands that may be issued to the controller. An X is used to indicate a don't   */
/* care condition.                                                                                     */
/* --------------------------------------------------------------------------------------------------- */
/* Recalibrate:                                                                                        */
/* --------------------------------------------------------------------------------------------------- */
/* Command Phase:                                                                                      */
/*  - USx = Unit Select                                                                                */
/* -----------7--6--5--4---3--2---1---0--------------------------------------------------------------- */
/*  - Byte 0: 0  0  0   0  0  1   1   1                                                                */
/*  - Byte 1: X  X   X  X  X  0  US1 US0                                                               */
/* --------------------------------------------------------------------------------------------------- */
/* Figure 1-89. Recalibrate Command:                                                                   */
/* --------------------------------------------------------------------------------------------------- */
/* Result Phase:                                                                                       */
/* --------------------------------------------------------------------------------------------------- */
/* This command has no result phase.                                                                   */
/* --------------------------------------------------------------------------------------------------- */
static
void cmd_recalibrate (e8272_t *fdc)
{
	unsigned pd, steps;
	pd = fdc->cmd[1] & 0x03;
#if E8272_DEBUG >= 1
	fprintf (stderr, "E8272: CMD=%02X D=%u  RECALIBRATE\n",
		fdc->cmd[0], pd
	);
#endif
	steps = fdc->drv[pd].c;
	e8272_select_cylinder (fdc, pd, 0);
	fdc->msr |= 0x01 << pd;
	if (fdc->accurate) {
		fdc->delay_clock = steps * ((fdc->step_rate * fdc->input_clock) / 1000);
	}
	else {
		fdc->delay_clock = 0;
	}
	fdc->set_clock = cmd_recalibrate_clock;
}
/* ---------------------------------------------------------------------------------------------- */
/* FUNCTIONAL DESCRIPTION OF COMMANDS (CONT.): SCAN COMMANDS:                                     */
/* ---------------------------------------------------------------------------------------------- */
/* The SCAN Commands allow data which is being read from the diskette to be compared against data */
/* which is being supplied from the main system (Processor NON-DMA mode, and DMA Controller in DMA*/
/* mode). The FDC ompares the data on a byte-by-byte basis, and looks for a sector of data which  */
/* meets the condi-tions of DFDD = DProcessor, DFDD < DProcessor, or DFDD < DProcessor. Ones      */
/*complement arith-metic is used for comparison (FF = largest number, 00 = smallest number). After*/
/* a whole sector of data is compared, if the conditions are not met, the sector number is        */
/* incremented (R + STP -> R), and the scan operation is continued. The scan operations continues */
/* until one of the following conditions occur; the con-ditions for scan are met (equal, low, or  */
/* high), the last sector on the track is reached (EOT), or the terminal count signal is received.*/
/*                                                                                                */
/*If the conditions for scan are met then the FDC sets the SH (Scan Hit) flag of Status Register 2*/
/* to a 1 (high), and terminates the Scan Command. If the conditions for scan are not met between */
/*the starting sector (as specified by R) and the last sector on the cylinder (EOT), then FDC sets*/
/* the SN (Scan Not Satisfied) flag of Status Register 2 to a 1 (high), and terminates the Scan   */
/* Command. The receipt of a TERMINAL COUNT signal from the Processor or DMA Controller during the*/
/* scan operation will cause the FDC to complete the comparison of the particular byte which is in*/
/* process, and then to terminate the com-mand. Table 4 shows the status of bits SH and SN under  */
/* various conditions of SCAN.                                                                    */
/* ---------------------------------------------------------------------------------------------- */
/* COMMAND:              STATUS REGISTER 2  STATUS REGISTER 2  COMMENTS:                          */
/*                          BIT 2 = SN:        BIT 3 = SN:                                        */
/*  - Scan         Equal: - 0:               - 1:                - DFDD  =  DProcessor            */
/*  - Scan         Equal: - 1:               - 0:                - DFDD =/= DProcessor            */
/*  - Scan  Low or Equal: - 0:               - 1:                - DFDD  =  DProcessor            */
/*  - Scan  Low or Equal: - 0:               - 0:                - DFDD  <  DProcessor            */
/*  - Scan  Low or Equal: - 1:               - 0:                - DFDD </= DProcessor            */
/*  - Scan High or Equal: - 0:               - 1:                - DFDD  =  DProcessor            */
/*  - Scan High or Equal: - 0:               - 0:                - DFDD  <  DProcessor            */
/*  - Scan High or Equal: - 1:               - 0:                - DFDD =/> DProcessor            */
/* ---------------------------------------------------------------------------------------------- */
/* Table 4:                                                                                       */
/* ---------------------------------------------------------------------------------------------- */
/* If the FDC encounters a Deleted Data Address Mark on one of the sectors (and SK = 0), then it  */
/* regards the sector as the last sector on the cylinder, sets CM (Control Mark) flag of Status   */
/* Register 2 to a 1 (high) and terminates the command. If SK = 1, the FDC skips the sector with  */
/* the Deleted Address Mark, and reads the next sector. In the second case (SK = 1), the FDC sets */
/* the CM (Control Mark) flag of Status Register 2 to a 1 (high) in order to show that a Deleted  */
/* Sector had been encountered.                                                                   */
/*                                                                                                */
/* When either STP (contiguous sectors = 01, or alternate sectors = 02 sectors are read) or the MT*/
/*(Multi-Track) are programmed, it is necessary to remember that the last sector on the track must*/
/* be read. For example, if STP = 02, MT = 0, the sectors are numbered sequentially 1 through 26, */
/* and we start the Scan Command at sector 21; the following will happend. Sectors 21, 23, and 25 */
/* will be read, then the next sector (26) will be skipped and the Index Hole will be encountered */
/* before the EOT value of 26 can be read. This will result in an abnormal termination of the     */
/* command. If the EOT had been set at 25 or the scanning started at sector 20, then the Scan     */
/* Command would be completed in a normal manner.                                                 */
/*                                                                                                */
/* During the Scan Command data is supplied by either the processor or DMA Controller for         */
/* comparison against the data read from the diskette. In order to avoid having the OR (Over Run) */
/*flag set in Status Register 1, it is necessary to have the data available in less than 27 us (FM*/
/* Mode) or 13 us (MFM Mode). If an Overrun occurs the FDC ends the command with bits 7 and 6 of  */
/* Status Register 0 set to 0 and 1, respectively.                                                */
/* ---------------------------------------------------------------------------------------------- */
/* ______________________________DATA BUS_________________________________________________________*/
/* |__PHASE__|_R/W_|_D7__D6__D5__D4__D3__D2__D1__D0__|__________________REMARKS__________________|*/
/* |__________________________________SEEK_______________________________________________________|*/
/* | Command |  W  |  0   0   0   0   1   1   1   1  | Command Codes                             |*/
/* |         |  W  |  X   X   X   X   X   HD US1 US0 |                                           |*/
/* |         |     |                                 |                                           |*/
/* |Execution|  W  |________________NCN______________| Head is positioned over proper Cylinder on|*/
/* |_________|_____|_________________________________|_Diskette__________________________________|*/
/* --------------------------------------------------------------------------------------------------- */
/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; Command Format:                */
/* --------------------------------------------------------------------------------------------------- */
/* The following are commands that may be issued to the controller. An X is used to indicate a don't   */
/* care condition.                                                                                     */
/* --------------------------------------------------------------------------------------------------- */
/* Scan         Equal:                                                                                 */
/* --------------------------------------------------------------------------------------------------- */
/* Command Phase:                                                                                      */
/*  - MT  = Multitrack                                                                                 */
/*  - MF  = MFM Mode                                                                                   */
/*  - SK  = Skip Deleted-Data Address Mark                                                             */
/*  - HD  = Head Number                                                                                */
/*  - USx = Unit Select                                                                                */
/* -----------7--6--5--4---3--2---1---0--------------------------------------------------------------- */
/*  - Byte 0: MT MF SK  1  0  0   0   1                                                                */
/*  - Byte 1: X  X   X  X  X  HD US1 US0                                                               */
/*  - Byte 2: Cylinder Number                                                                          */
/*  - Byte 3: Head Address                                                                             */
/*  - Byte 4: Sector   Number                                                                          */
/*  - Byte 5: Number of Data Bytes in Sector                                                           */
/*  - Byte 6: End of Track                                                                             */
/*  - Byte 7: Gap  Length                                                                              */
/*  - Byte 8: Scan Test                                                                                */
/* --------------------------------------------------------------------------------------------------- */
/* Figure 1-83. Scan         Equal Command:                                                            */
/* --------------------------------------------------------------------------------------------------- */
/* Result Phase:                                                                                       */
/* -----------7--6--5--4---3--2---1---0--------------------------------------------------------------- */
/*  - Byte 0: Status Register 0                                                                        */
/*  - Byte 1: Status Register 1                                                                        */
/*  - Byte 2: Status register 2                                                                        */
/*  - Byte 3: Cylinder Number                                                                          */
/*  - Byte 4: Head Address                                                                             */
/*  - Byte 5: Sector   Number                                                                          */
/*  - Byte 6: Number of Data Bytes in Sector                                                           */
/* --------------------------------------------------------------------------------------------------- */
/* Figure 1-84. Scan         Equal Result:                                                             */
/* --------------------------------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------------------------------- */
/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; Command Format:                */
/* --------------------------------------------------------------------------------------------------- */
/* The following are commands that may be issued to the controller. An X is used to indicate a don't   */
/* care condition.                                                                                     */
/* --------------------------------------------------------------------------------------------------- */
/* Scan Low  or Equal:                                                                                 */
/* --------------------------------------------------------------------------------------------------- */
/* Command Phase:                                                                                      */
/*  - MT  = Multitrack                                                                                 */
/*  - MF  = MFM Mode                                                                                   */
/*  - SK  = Skip Deleted-Data Address Mark                                                             */
/*  - HD  = Head Number                                                                                */
/*  - USx = Unit Select                                                                                */
/* -----------7--6--5--4---3--2---1---0--------------------------------------------------------------- */
/*  - Byte 0: MT MF SK  1  1  0   0   1                                                                */
/*  - Byte 1: X  X   X  X  X  HD US1 US0                                                               */
/*  - Byte 2: Cylinder Number                                                                          */
/*  - Byte 3: Head Address                                                                             */
/*  - Byte 4: Sector   Number                                                                          */
/*  - Byte 5: Number of Data Bytes in Sector                                                           */
/*  - Byte 6: End of Track                                                                             */
/*  - Byte 7: Gap  Length                                                                              */
/*  - Byte 8: Scan Test                                                                                */
/* --------------------------------------------------------------------------------------------------- */
/* Figure 1-85. Scan Low  or Equal Command:                                                            */
/* --------------------------------------------------------------------------------------------------- */
/* Result Phase:                                                                                       */
/* -----------7--6--5--4---3--2---1---0--------------------------------------------------------------- */
/*  - Byte 0: Status Register 0                                                                        */
/*  - Byte 1: Status Register 1                                                                        */
/*  - Byte 2: Status register 2                                                                        */
/*  - Byte 3: Cylinder Number                                                                          */
/*  - Byte 4: Head Address                                                                             */
/*  - Byte 5: Sector   Number                                                                          */
/*  - Byte 6: Number of Data Bytes in Sector                                                           */
/* --------------------------------------------------------------------------------------------------- */
/* Figure 1-86. Scan Low  or Equal Result:                                                             */
/* --------------------------------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------------------------------- */
/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; Command Format:                */
/* --------------------------------------------------------------------------------------------------- */
/* The following are commands that may be issued to the controller. An X is used to indicate a don't   */
/* care condition.                                                                                     */
/* --------------------------------------------------------------------------------------------------- */
/* Scan High or Equal:                                                                                 */
/* --------------------------------------------------------------------------------------------------- */
/* Command Phase:                                                                                      */
/*  - MT  = Multitrack                                                                                 */
/*  - MF  = MFM Mode                                                                                   */
/*  - SK  = Skip Deleted-Data Address Mark                                                             */
/*  - HD  = Head Number                                                                                */
/*  - USx = Unit Select                                                                                */
/* -----------7--6--5--4---3--2---1---0--------------------------------------------------------------- */
/*  - Byte 0: MT MF SK  1  1  1   0   1                                                                */
/*  - Byte 1: X  X   X  X  X  HD US1 US0                                                               */
/*  - Byte 2: Cylinder Number                                                                          */
/*  - Byte 3: Head Address                                                                             */
/*  - Byte 4: Sector   Number                                                                          */
/*  - Byte 5: Number of Data Bytes in Sector                                                           */
/*  - Byte 6: End of Track                                                                             */
/*  - Byte 7: Gap  Length                                                                              */
/*  - Byte 8: Scan Test                                                                                */
/* --------------------------------------------------------------------------------------------------- */
/* Figure 1-85. Scan High or Equal Command:                                                            */
/* --------------------------------------------------------------------------------------------------- */
/* Result Phase:                                                                                       */
/* -----------7--6--5--4---3--2---1---0--------------------------------------------------------------- */
/*  - Byte 0: Status Register 0                                                                        */
/*  - Byte 1: Status Register 1                                                                        */
/*  - Byte 2: Status register 2                                                                        */
/*  - Byte 3: Cylinder Number                                                                          */
/*  - Byte 4: Head Address                                                                             */
/*  - Byte 5: Sector   Number                                                                          */
/*  - Byte 6: Number of Data Bytes in Sector                                                           */
/* --------------------------------------------------------------------------------------------------- */
/* Figure 1-86. Scan High or Equal Result:                                                             */
/* --------------------------------------------------------------------------------------------------- */
/*****************************************************************************
 * seek
 *****************************************************************************/
/* ---------------------------------------------------------------------------------------------- */
/* FUNCTIONAL DESCRIPTION OF COMMANDS (CONT.): SEEK:                                              */
/* ---------------------------------------------------------------------------------------------- */
/* The read/write head within the FDD is moved from cylinder to cylinder under control of the Seek*/
/* Command. The FDC compares the PCN (Present Cylinder Number) which is the current head position */
/* with the NCN (New Cylinder Number), and if there is a difference performs the following        */
/* operation:                                                                                     */
/*  -PCN < NCN: Direction signal to FDD set to a 1 (high), and step pulses are issued. (Step In.) */
/*  -PCN > NCN: Direction signal to FDD set to a 0  (low), and step pulses are issued. (Step Out.)*/
/* The rate at which Step Pulses are issued is controlled by SRT (Stepping Rate Time) in the      */
/* SPECIFY Com-mand. After each Step Pulse is issued NCN is compared against PCN, and when NCN =  */
/* PCN, then the SE (Seek End) flag is set in Status Register 0 to a 1 (high), and the command is */
/* terminated.                                                                                    */
/*                                                                                                */
/* During the Command Phase of the Seek operation the FDC is in the FDC BUSY state, but during the*/
/* Execution Phase it is in the NON BUSY state. While the FDC is in the NON BUSY state, another   */
/* Seek Command may be issued, and in this manner parallel seek operations may be done on up to 4 */
/* Drives at once.                                                                                */
/*                                                                                                */
/* If an FDD is in a NOT READY state at the beginning of the command execution phase or during the*/
/* seek operation, then the NR (NOT READY) flag is set in Status Register 0 to a 1 (high), and the*/
/* command is terminated after bits 7 and 6 of Status Register 0 and 1 respectively.              */
/* ---------------------------------------------------------------------------------------------- */
static
void cmd_seek_clock (e8272_t *fdc, unsigned long cnt)
{
	unsigned drv;
	if (fdc->delay_clock > 0) {
		return;
	}
	drv = fdc->cmd[1] & 0x03;
	fdc->msr &= ~(0x01 << drv);
	fdc->st[0] = (fdc->cmd[1] & 0x07) | E8272_ST0_SE;
	e8272_set_irq (fdc, 1);
	cmd_done (fdc);
}
/* --------------------------------------------------------------------------------------------------- */
/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; Command Format:                */
/* --------------------------------------------------------------------------------------------------- */
/* The following are commands that may be issued to the controller. An X is used to indicate a don't   */
/* care condition.                                                                                     */
/* --------------------------------------------------------------------------------------------------- */
/* Seek:                                                                                               */
/* --------------------------------------------------------------------------------------------------- */
/* Command Phase:                                                                                      */
/*  - USx = Unit Select                                                                                */
/* -----------7--6--5--4---3--2---1---0--------------------------------------------------------------- */
/*  - Byte 0: 0  0  0   0  1  1   1   1                                                                */
/*  - Byte 1: X  X  X   X  X  0  US1 US0                                                               */
/*  - Byte 2: New Cylinder Number for Seek                                                             */
/* --------------------------------------------------------------------------------------------------- */
/* Figure 1-95. Seek Command:                                                                          */
/* --------------------------------------------------------------------------------------------------- */
/* Result Phase:                                                                                       */
/* --------------------------------------------------------------------------------------------------- */
/* This command has no result phase.                                                                   */
/* --------------------------------------------------------------------------------------------------- */
static
void cmd_seek (e8272_t *fdc)
{
	unsigned pd, pc, steps;
	pd = fdc->cmd[1] & 3;
	pc = fdc->cmd[2];
#if E8272_DEBUG >= 1
	fprintf (stderr, "E8272: CMD=%02X D=%u  SEEK (pc=%u)\n",
		fdc->cmd[0], pd, pc
	);
#endif
	if (fdc->drv[pd].c < pc) {
		steps = pc - fdc->drv[pd].c;
	}
	else {
		steps = fdc->drv[pd].c - pc;
	}
	e8272_select_cylinder (fdc, pd, pc);
	fdc->msr |= 0x01 << pd;
	if (fdc->accurate) {
		fdc->delay_clock = steps * ((fdc->step_rate * fdc->input_clock) / 1000);
	}
	else {
		fdc->delay_clock = 0;
	}
	fdc->set_clock = cmd_seek_clock;
}
/* ______________________________DATA BUS_________________________________________________________*/
/* |__PHASE__|_R/W_|_D7__D6__D5__D4__D3__D2__D1__D0__|__________________REMARKS__________________|*/
/* |_____________________SENSE_INTERRUPT_STATUS__________________________________________________|*/
/* | Command |  W  |  0   0   0   0   1   0   0   0  | Command Codes                             |*/
/* |         |     |                                 |                                           |*/
/* | Result  |  R  | ______________STO______________ | Status information at the end of seek-    |*/
/* |_________|__R__|_______________PCN_______________|_operation_about_the_FDC.__________________|*/
/*****************************************************************************
 * sense interrupt status
 *****************************************************************************/
static
void cmd_sense_int_status_clock (e8272_t *fdc, unsigned long cnt)
{
	unsigned i, n;
	if (fdc->delay_clock > 0) {
		return;
	}
	fdc->res[0] = fdc->st[0];
	fdc->res[1] = fdc->curdrv->c;
	n = 2;
	if (fdc->st[0] & E8272_ST0_SE) {
		/* reset interrupt condition */
		fdc->st[0] &= 0x3f;
		fdc->st[0] &= ~(E8272_ST0_SE);
	}
	else if (fdc->ready_change) {
		fdc->res[0] = 0xc0;
		for (i = 0; i < 4; i++) {
			if (fdc->ready_change & (1 << i)) {
				fdc->res[0] |= i;
				fdc->ready_change &= ~(1 << i);
				break;
			}
		}
	}
	else {
		fdc->res[0] = 0x80;
		n = 1;
	}
	e8272_set_irq (fdc, 0);
	cmd_result (fdc, n);
}

static
void cmd_sense_int_status (e8272_t *fdc)
{
#if E8272_DEBUG >= 1
	fprintf (stderr, "E8272: CMD=%02X D=*  SENSE INTERRUPT STATUS\n",
		fdc->cmd[0]
	);
#endif
	fdc->delay_clock = fdc->accurate ? (fdc->input_clock / 10000) : 0;
	fdc->set_clock = cmd_sense_int_status_clock;
}
/* ---------------------------------------------------------------------------------------------- */
/* FUNCTIONAL DESCRIPTION OF COMMANDS (CONT.): SENSE INTERRUPT STATUS:                            */
/* ---------------------------------------------------------------------------------------------- */
/* An Interrupt signal is generated by the FDC for one of the following reasons:                  */
/*  - 1. Upon entering the Result Phase of:                                                       */
/*  - - a.  Read         Data Command                                                             */
/*  - - b.  Read   a Track    Command                                                             */
/*  - - c.  Read      ID      Command                                                             */
/*  - - d.  Read Deleted Data Command                                                             */
/*  - - e. Write         Data Command                                                             */
/*  - - f. Format a Cylinder  Command                                                             */
/*  - - g. Write Deleted Data Command                                                             */
/*  - - h. Scan               Commands                                                            */
/*  - 2. Ready Line of FDD changes state                                                          */
/*  - 3. End of Seek or Recalibrate Command                                                       */
/*  - 4. During Execution Phase in the NON-DMA Mode                                               */
/*Interrupts caused by reasons 1 and 4 above occur during normal command operations and are easily*/
/* dis-cernible by the processor. However, interrupts caused by reasons 2 and 3 above may be      */
/*uniquely identified with the aid of the Sense Interrupt Status Command. This command when issued*/
/* resets the interrupt signal and via bits 5, 6, and 7 of Status Register 0 identifies the cause */
/* of the interrupt.                                                                              */
/* ---------------------------------------------------------------------------------------------- */
/* SEEK END BIT 5: INTERRUPT CODE BIT 6: INTERRUPT CODE BIT 7: CAUSE:                             */
/*  - 0:            - 1:                  - 1:                  - Ready Line changed state, either*/
/*                                                              polarity                          */
/*  - 1:            - 0:                  - 0:                  -   Normal Termination of Seek or */
/*                                                              Recalibrate Command               */
/*  - 1:            - 1:                  - 0:                  - Abnormal Termination of Seek or */
/*                                                              Recalibrate Command               */
/* ---------------------------------------------------------------------------------------------- */
/* Neither the Seek or Recalibrate Command have a Result Phase. Therefore, it is mandatory to use */
/* the Sense Interrupt Status Command after these commands to effectively terminate them and to   */
/* provide verification of where the head is is positioned (PCN).                                 */
/* ---------------------------------------------------------------------------------------------- */
/* ______________________________DATA BUS_________________________________________________________*/
/* |__PHASE__|_R/W_|_D7__D6__D5__D4__D3__D2__D1__D0__|__________________REMARKS__________________|*/
/* |_________________________SENSE_DRIVE_STATUS__________________________________________________|*/
/* | Command |  W  |  0   0   0   0   0   1   0   1  | Command Codes                             |*/
/* |         |  W  |  X   X   X   X   X   HD US1 US0 |                                           |*/
/* |         |     |                                 |                                           |*/
/* |_Result__|__R__|_______________ST 3______________|_Status_information_about_FDD._____________|*/
/*****************************************************************************
 * sense drive status
 *****************************************************************************/
static
void cmd_sense_drive_status_clock (e8272_t *fdc, unsigned long cnt)
{
	unsigned d;
	if (fdc->delay_clock > 0) {
		return;
	}
	d = fdc->cmd[1] & 0x03;
	fdc->st[3] &= ~E8272_ST3_T0;
	fdc->st[3] |= E8272_ST3_RDY | E8272_ST3_TS;
	if (fdc->drv[d].c == 0) {
		fdc->st[3] |= E8272_ST3_T0;
	}
	fdc->res[0] = fdc->st[3];
	cmd_result (fdc, 1);
}
/* --------------------------------------------------------------------------------------------------- */
/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; Command Format:                */
/* --------------------------------------------------------------------------------------------------- */
/* The following are commands that may be issued to the controller. An X is used to indicate a don't   */
/* care condition.                                                                                     */
/* --------------------------------------------------------------------------------------------------- */
/* Sense Drive Status:                                                                                 */
/* --------------------------------------------------------------------------------------------------- */
/* Command Phase:                                                                                      */
/*  - USx = Unit Select                                                                                */
/*  - HUT = Head Number                                                                                */
/* -----------7--6--5--4---3--2---1---0--------------------------------------------------------------- */
/*  - Byte 0: 0  0  0   0  0  1   0   0                                                                */
/*  - Byte 1: X  X  X   X  X  HD US1 US0                                                               */
/* --------------------------------------------------------------------------------------------------- */
/* Figure 1-93. Sense Drive Status Command:                                                            */
/* --------------------------------------------------------------------------------------------------- */
/* Result Phase:                                                                                       */
/* -----------7--6--5--4---3--2---1---0--------------------------------------------------------------- */
/*  - Byte 0: Status 3 Register                                                                        */
/* --------------------------------------------------------------------------------------------------- */
/* Figure 1-94. Sense Drive Status Result:                                                             */
/* --------------------------------------------------------------------------------------------------- */
static
void cmd_sense_drive_status (e8272_t *fdc)
{
#if E8272_DEBUG >= 1
	fprintf (stderr, "E8272: CMD=%02X D=%u  SENSE DRIVE STATUS\n",
		fdc->cmd[0], fdc->cmd[1] & 3
	);
#endif
	fdc->delay_clock = fdc->accurate ? (fdc->input_clock / 10000) : 0;
	fdc->set_clock = cmd_sense_drive_status_clock;
}
/* ---------------------------------------------------------------------------------------------- */
/* FUNCTIONAL DESCRIPTION OF COMMANDS (CONT.): SPECIFY:                                           */
/* ---------------------------------------------------------------------------------------------- */
/* This command may be used by the processor whenever it wishes to obtain the status of the FDDs. */
/* Status Register 3 contains the Drive Status information.                                       */
/* ---------------------------------------------------------------------------------------------- */
/* ______________________________DATA BUS_________________________________________________________*/
/* |__PHASE__|_R/W_|_D7__D6__D5__D4__D3__D2__D1__D0__|___________________REMARKS_________________|*/
/* |_____________________SENSE_INTERRUPT_STATUS__________________________________________________| */
/* | Command |  W  |  0   0   0   0   0   0   1   1  | Command Codes                             | */
/* |         |     |                                 |                                           | */
/* |         |  W  | ____SRT___><_____________HUT___ |                                           | */
/* |_________|__W__|_______HLT__________________>_ND_|___________________________________________| */
/*****************************************************************************
 * specify
 *****************************************************************************/
static
void cmd_specify (e8272_t *fdc)
{
	unsigned char srt, hut, hlt, nd;
	srt = 16 - ((fdc->cmd[1] >> 4) & 0x0f); /* IBM PS/2: SRT = Diskette Stepping Rate */
	hut = 16 * (fdc->cmd[1] & 0x0f);        /* IBM PS/2: HUT = Head Unload Time       */
	hlt = 2 * ((fdc->cmd[2] >> 1) & 0x7f);  /* IBM PS/2: HLT = Head   Load Time       */
	nd = (fdc->cmd[2] & 0x01);              /* IBM PS/2: ND  = NonData Mode           */
#if E8272_DEBUG >= 1
	fprintf (stderr,
		"E8272: CMD=%02X D=*  SPECIFY (srt=%ums, hut=%ums, hlt=%ums, dma=%d)\n",
		fdc->cmd[0], srt, hut, hlt, nd == 0
	);
#else
	(void) hlt;           /* IBM PS/2: HLT = Head   Load Time       */
	(void) hut;           /* IBM PS/2: HUT = Head Unload Time       */
#endif
	fdc->dma = (nd == 0); /* IBM PS/2: ND  = NonData Mode           */
	fdc->step_rate = srt; /* IBM PS/2: SRT = Diskette Stepping Rate */
	cmd_done (fdc);
}
/* --------------------------------------------------------------------------------------------------- */
/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; Command Format:                */
/* --------------------------------------------------------------------------------------------------- */
/* The following are commands that may be issued to the controller. An X is used to indicate a don't   */
/* care condition.                                                                                     */
/* --------------------------------------------------------------------------------------------------- */
/* Specify:                                                                                            */
/* --------------------------------------------------------------------------------------------------- */
/* Command Phase:                                                                                      */
/*  - SRT = Diskette Stepping Rate                                                                     */
/*  - HUT = Head Unload Time                                                                           */
/*  - HLT = Head   Load Time                                                                           */
/*  - ND  = NonData Mode                                                                               */
/* -----------7--6--5--4---3--2---1---0--------------------------------------------------------------- */
/*  - Byte 0: 0  0  0   0  0  0   1   1                                                                */
/*  - Byte 1:     SRT         HUT                                                                      */
/*  - Byte 2:          HLT           ND                                                                */
/* --------------------------------------------------------------------------------------------------- */
/* Figure 1-92. Specify Command:                                                                       */
/* --------------------------------------------------------------------------------------------------- */
/* Result Phase:                                                                                       */
/* --------------------------------------------------------------------------------------------------- */
/* This command has no result phase.                                                                   */
/* --------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------- */
/* FUNCTIONAL DESCRIPTION OF COMMANDS (CONT.): SPECIFY:                                           */
/* ---------------------------------------------------------------------------------------------- */
/*The Specify Command sets the initial values for each of the three internal timers. The HUT (Head*/
/* Unload Time) defines the time from the end of the Execution Phase of one of the Read/Write     */
/* Commands to the head unload state. This timer is programmable from 0 to 240 ms in increments of*/
/* 16 ms (00 = 0 ms, 01 = 16 ms, 02 = 32 ms, etc.). The SRT (Step Rate Time) defines the time     */
/* interval between adjacent step pulses. This timer is programmable from 1 to 16 ms in increments*/
/* of 1 ms (F = 1 ms, E = 2 ms, D = 3 ms, etc.). The HLT (Head Load Time) defines the time between*/
/* when the Head Load signal goes high and when the Read/Write operation starts. This timer is    */
/* programmable from 2 to 256 ms in increments of 2 ms (00 = 2 ms, 01 = 4 ms, 02 = 6 ms, etc.).   */
/*                                                                                                */
/* The time intervals mentioned above are a direct function of the clock (CLK on pin 19). Times   */
/* indicated above are for an 8 MHz clock, if the clock was reduced to 4 MHz (mini-floppy         */
/* application) then all time intervals are increased by a factor of 2.                           */
/*                                                                                                */
/* The choice of DMA or NON-DMA operation is made by the ND (NON-DMA) bit. When this bit is high  */
/* (ND = 1) the NON-DMA mode is selected, and when ND = 0 the DMA mode is selected.               */
/* ---------------------------------------------------------------------------------------------- */
/* ______________________________DATA BUS_________________________________________________________*/
/* |__PHASE__|_R/W_|_D7__D6__D5__D4__D3__D2__D1__D0__|__________________REMARKS__________________|*/
/* |_______________________________INVALID_______________________________________________________|*/
/* |         |     |                                 |                                           |*/
/* | Command |  W  | _________Invalid Codes_________ |Invalid Command Codes (NoOp - FDC goes into|*/
/* |         |     |                                 | Standby State)                            |*/
/* |         |     |                                 |                                           |*/
/* | Result  |  R  |_______________ST 0______________| ST 0 = 80                                 |*/
/* |_________|_____|_________________________________|___________(16)____________________________|*/
/*****************************************************************************
 * invalid
 *****************************************************************************/
/* ---------------------------------------------------------------------------------------------- */
/* FUNCTIONAL DESCRIPTION OF COMMANDS (CONT.): INVALID:                                           */
/* ---------------------------------------------------------------------------------------------- */
/* If an invalid command is sent to the FDC (a command not defined above), then the FDC will      */
/* terminate the command after bits 7 and 6 of Status Register 0 are set to 1 and 0 respectively. */
/* A Sense Interrupt Status Command must be sent after a Seek or Recalibrate Interrupt, otherwise */
/* the FDC will consider the next command to be an Invalid Command.                               */
/*                                                                                                */
/* In some applications the user may wish to use this command as a No-Op command, to place the FDC*/
/* in a standby or no operation state.                                                            */
/* ---------------------------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------------------------------- */
/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; Command Format:                */
/* --------------------------------------------------------------------------------------------------- */
/* The following are commands that may be issued to the controller. An X is used to indicate a don't   */
/* care condition.                                                                                     */
/* --------------------------------------------------------------------------------------------------- */
/* Invalid Commands:                                                                                   */
/* --------------------------------------------------------------------------------------------------- */
/* Result Phase:                                                                                       */
/* -----------7--6--5--4---3--2---1---0--------------------------------------------------------------- */
/*  - Byte 0: Status 0 Register                                                                        */
/* --------------------------------------------------------------------------------------------------- */
/* Figure 1-96. Invalid Command Result:                                                                */
/* --------------------------------------------------------------------------------------------------- */
static
void cmd_invalid (e8272_t *fdc)
{
#if E8272_DEBUG >= 0
	fprintf (stderr, "E8272: CMD=%02X D=? INVALID\n", fdc->cmd[0]);
#endif
	fdc->res[0] = 0x80;
	cmd_result (fdc, 1);
}
/* ---------------------------------------------------------------------------------------------- */
/* COMMAND SYMBOL DESCRIPTION:                                                                    */
/* ---------------------------------------------------------------------------------------------- */
/* SYMBOL:NAME:              DESCRIPTION:                                                         */
/*  - A0 : - Address Line 0:  - A0 controls selection of Main Status Register (A0 = 0) or Data    */
/*                            Register (A0 = 1)                                                   */
/*  - C  : - Cylinder Number: - C stands for the current/selected Cylinder (track) number 0       */
/*                            through 76 of the medium.                                           */
/*  - D  : - Data:            - D stands for the data pattern which is going to be written into a */
/*                            Sector.                                                             */
/*  -D7-D0:- Data Bus:        - 8-bit Data Bus, where D7 stands for a most significant bit, and D0*/
/*                            stands for a least significant bit.                                 */
/*  - DTL: - Data Length:     - When N is defined as D0, DTL stands for the data length which     */
/*                            users are going to read out or write into the Sector.               */
/*  - EOT: - End of Track:    - EOT stands for the final Sector number on a Cylinder.             */
/*  - GPL: - Gap Length:      - GPL stands for the length of Gap 3 (spacing between Sectors       */
/*                            excluding VCO Sync. Field).                                         */
/*  - H  : - Head Address:    - H stands for head number 0 or 1, as specified in ID field.        */
/*  - HD : - Head:            - HD stands for a selected head number 0 or 1. (H = HD in all       */
/*                            command words).                                                     */
/*  - HLT: - Head Load Time:  - HLT stands for the head load time in the FDD (2 to 256 ms in 2 ms */
/*                            increments).                                                        */
/*  - HUT: - Head Unload Time:- HUT stands for the had unload time after a read or write operation*/
/*                            has occurred (0 to 240 ms in 16 ms increments).                     */
/*  - MF : - FM or MFM Mode:  - If MF is low, FM mode is selected, and if it is high, MFM mode is */
/*                            selected.                                                           */
/*  - MT : - Multi-Track:     - If MT is high, a multi-track operation is to be performed. (A     */
/*                            cylinder under both HD0 and HD1 will be read or written.)           */
/* ---------------------------------------------------------------------------------------------- */
/* COMMAND SYMBOL DESCRIPTION (CONT):                                                             */
/* ---------------------------------------------------------------------------------------------- */
/* SYMBOL:NAME:                  DESCRIPTION:                                                     */
/*  - N  : - Number:                 - N stands for the number of data bytes written in a Sector. */
/*  - NCN: - New Cylinder Number:    - NCN stands for a new Cylinder number, which is going to be */
/*                                   reached as a result of the Seek operation. Desired position  */
/*                                   of Head.                                                     */
/*  - ND : - Non-DMA Mode:           - ND stands for operation in the Non-DMA Mode.               */
/*  - PCN: - Present Cylinder Number:-PCN stands for the Cylinder number at the com-pletion of    */
/*                                   SENSE INTERRUPT STATUS Command. Position of Head at present  */
/*                                   time.                                                        */
/*  - R  : - Record:                 - R stands for the Sector number, which will be read or      */
/*                                   written.                                                     */
/*  - R/W: - Read/Write:             - R/W stands for either Read (R) or Write (W) signal.        */
/*  - SC : - Sector:                 - SC indicates the number of Sectors per Cylinder.           */
/*  - SRT: - Step Rate Time:         - SRT stands for the Stepping Rate for the FDD. (1 to 16 ms  */
/*                                   in 1 ms increments.) Must be defined for each of the four    */
/*                                   drives.                                                      */
/*  - ST 0:- Status 0:               - ST 0-3 stand for one of four registers which store the     */
/*                                   status information after a command has been executed. This   */
/*  - ST 1:- Status 1:              information is available during the result phase after command*/
/*  - ST 2:- Status 2:               execution. These registers should not be con-fused with the  */
/*  - ST 3:- Status 3:               main status register (selected by A0 = 0). ST 0-3 may be read*/
/*                                 only after a com-mand has been executed and contain information*/
/*                                   relevant to that particular command.                         */
/*  - STP: -                         - During a Scan operation, if STP = 1, the data in contiguous*/
/*                                   sectors is compared byte by byte with data sent from the     */
/*                                  processor (or DMA); and if STP = 2, then alternate sectors are*/
/*                                   read and compared.                                           */
/*  - US0, US1: - Unit Select:       - US stands for a selected drive number 0 or 1.              */
/* ---------------------------------------------------------------------------------------------- */
static struct {
	unsigned char mask;
	unsigned char val;
	unsigned      cnt;
	void          (*start_cmd) (e8272_t *fdc);
} cmd_tab[] = {
	{ 0x1f, 0x06, 9, cmd_read },
	{ 0x1f, 0x0c, 9, cmd_read_deleted },
	{ 0x1f, 0x02, 9, cmd_read_track },
	{ 0xbf, 0x0a, 2, cmd_read_id },
	{ 0x3f, 0x05, 9, cmd_write },
	{ 0xbf, 0x0d, 6, cmd_format },
	{ 0xff, 0x07, 2, cmd_recalibrate },
	{ 0xff, 0x0f, 3, cmd_seek },
	{ 0xff, 0x08, 1, cmd_sense_int_status },
	{ 0xff, 0x04, 2, cmd_sense_drive_status },
	{ 0xff, 0x03, 3, cmd_specify },
	{ 0x00, 0x00, 1, cmd_invalid }
};
/* ______________________________DATA BUS_________________________________________________________*/
/* |__PHASE__|_R/W_|_D7__D6__D5__D4__D3__D2__D1__D0__|__________________REMARKS__________________|*/
/* |______________________________SCAN_EQUAL_____________________________________________________|*/
/* | Command |  W  | MT  MF  SK   1   0   0   0   1  | Command Codes                             |*/
/* |         |  W  |  X   X   X   X   X   HD  US1 US0|                                           |*/
/* |         |  W  | _______________C_______________ | Sector ID information prior to Command    |*/
/* |         |  W  | _______________H_______________ | execution                                 |*/
/* |         |  W  | _______________R_______________ |                                           |*/
/* |         |  W  | _______________N_______________ |                                           |*/
/* |         |  W  | ______________EOT______________ |                                           |*/
/* |         |  W  | ______________GPL______________ |                                           |*/
/* |         |  W  | ______________DTL______________ |                                           |*/
/* |         |     |                                 |                                           |*/
/* |Execution|     |                                 | Data-transfer between the FDD and main-   |*/
/* |         |     |                                 | system.                                   |*/
/* |         |     |                                 | FDC reads all of cylinders contents from  |*/
/* |         |     |                                 | index hole to EOT.                        |*/
/* | Result  |  R  | _____________ST  0_____________ | Status information after command execution|*/
/* |         |  R  | _____________ST  1_____________ |                                           |*/
/* |         |  R  | _____________ST  2_____________ |                                           |*/
/* |         |  R  | _______________C_______________ | Sector ID information after command       |*/
/* |         |  R  | _______________H_______________ | execution                                 |*/
/* |         |  R  | _______________R_______________ |                                           |*/
/* |         |  R  | _______________N_______________ |                                           |*/
/* |_________|_____|_________________________________|___________________________________________|*/
/* ______________________________DATA BUS_________________________________________________________*/
/* |__PHASE__|_R/W_|_D7__D6__D5__D4__D3__D2__D1__D0__|__________________REMARKS__________________|*/
/* |_______________________SCAN_LOW_OR_EQUAL_____________________________________________________|*/
/* | Command |  W  | MT  MF  SK   1   1   0   0   1  | Command Codes                             |*/
/* |         |  W  |  X   X   X   X   X   HD  US1 US0|                                           |*/
/* |         |  W  | _______________C_______________ | Sector ID information prior to Command    |*/
/* |         |  W  | _______________H_______________ | execution                                 |*/
/* |         |  W  | _______________R_______________ |                                           |*/
/* |         |  W  | _______________N_______________ |                                           |*/
/* |         |  W  | ______________EOT______________ |                                           |*/
/* |         |  W  | ______________GPL______________ |                                           |*/
/* |         |  W  | ______________DTL______________ |                                           |*/
/* |         |     |                                 |                                           |*/
/* |Execution|     |                                 | Data-transfer between the FDD and main-   |*/
/* |         |     |                                 | system.                                   |*/
/* |         |     |                                 | FDC reads all of cylinders contents from  |*/
/* |         |     |                                 | index hole to EOT.                        |*/
/* | Result  |  R  | _____________ST  0_____________ | Status information after command execution|*/
/* |         |  R  | _____________ST  1_____________ |                                           |*/
/* |         |  R  | _____________ST  2_____________ |                                           |*/
/* |         |  R  | _______________C_______________ | Sector ID information after command       |*/
/* |         |  R  | _______________H_______________ | execution                                 |*/
/* |         |  R  | _______________R_______________ |                                           |*/
/* |         |  R  | _______________N_______________ |                                           |*/
/* |_________|_____|_________________________________|___________________________________________|*/
/*
 * Write the first command byte.
 */
static
void e8272_write_cmd (e8272_t *fdc, unsigned char val)
{
	unsigned i, n;
	n = sizeof (cmd_tab) / sizeof (cmd_tab[0]);
	fdc->get_data = NULL;
	fdc->set_data = NULL;
	fdc->set_tc = NULL;
	fdc->set_clock = NULL;
	fdc->start_cmd = NULL;
	for (i = 0; i < n; i++) {
		if ((val & cmd_tab[i].mask) == cmd_tab[i].val) {
			fdc->cmd_i = 0;
			fdc->cmd_n = cmd_tab[i].cnt;
			fdc->set_data = cmd_set_command;
			fdc->start_cmd = cmd_tab[i].start_cmd;
			fdc->set_data (fdc, val);
			return;
		}
	}
}

void e8272_reset (e8272_t *fdc)
{
#if E8272_DEBUG >= 1
	fprintf (stderr, "E8272: reset\n");
#endif
	fdc->msr = E8272_MSR_RQM;
	fdc->st[0] = 0x00;
	fdc->st[1] = 0x00;
	fdc->st[2] = 0x00;
	fdc->st[3] = 0x00;
	fdc->cmd_i = 0;
	fdc->cmd_n = 0;
	fdc->res_i = 0;
	fdc->res_n = 0;
	fdc->buf_i = 0;
	fdc->buf_n = 0;
	fdc->ready_change = 0x0f;
	fdc->delay_clock = 0;
	fdc->set_data = e8272_write_cmd;
	fdc->get_data = NULL;
	fdc->set_tc = NULL;
	fdc->set_clock = NULL;
	e8272_set_irq (fdc, 0);
	e8272_set_dreq (fdc, 0);
}

static
void e8272_write_dor (e8272_t *fdc, unsigned char val)
{
	if (((fdc->dor) ^ val) & E8272_DOR_RESET) {
		if (val & E8272_DOR_RESET) {
			e8272_set_irq (fdc, 1);
		}
		else {
			e8272_reset (fdc);
		}
	}
	fdc->dor = val;
}

void e8272_write_data (e8272_t *fdc, unsigned char val)
{
	e8272_set_irq (fdc, 0);
	if ((fdc->msr & E8272_MSR_RQM) == 0) { /* Main Status Register: DB7: Request for Master: RQM: */
		if (fdc->dreq_val == 0) {
			return;
		}
	}
	if (fdc->msr & E8272_MSR_DIO) {        /* Main Status Register: DB6: Data Intput/Output: DIO: */
		return;
	}
	if (fdc->set_data != NULL) {
		fdc->set_data (fdc, val);
	}
}

static
unsigned char e8272_read_dor (e8272_t *fdc)
{
	return (fdc->dor);
}

static
unsigned char e8272_read_msr (e8272_t *fdc)
{
	return (fdc->msr); /* Main Status Register: */
}

unsigned char e8272_read_data (e8272_t *fdc)
{
	unsigned char val;
	e8272_set_irq (fdc, 0);
	val = 0;
	if (fdc->get_data != NULL) {
		val = fdc->get_data (fdc);
	}
#if E8272_DEBUG >= 3
	fprintf (stderr, "E8272: read data: %02X\n", val);
#endif
	return (val);
	return (0);
}

unsigned char e8272_get_uint8 (e8272_t *fdc, unsigned long addr)
{
	unsigned char ret;
	switch (addr) {
	case 0x02:
		ret = e8272_read_dor (fdc);
		break;
	case 0x04:
		ret = e8272_read_msr (fdc); /* Main Status Register: */
		break;
	case 0x05:
		ret = e8272_read_data (fdc);
		break;
	default:
		ret = 0xff;
#if E8272_DEBUG >= 1
		fprintf (stderr, "E8272: get %04lx -> %02x\n", addr, ret);
#endif
		break;
	}
#if E8272_DEBUG >= 3
	fprintf (stderr, "E8272: get %04lx -> %02x\n", addr, ret);
#endif
	return (ret);
}

void e8272_set_uint8 (e8272_t *fdc, unsigned long addr, unsigned char val)
{
#if E8272_DEBUG >= 3
	fprintf (stderr, "E8272: set %04lx <- %02x\n", addr, val);
#endif
	switch (addr) {
	case 0x02:
		e8272_write_dor (fdc, val);
		break;
	case 0x05:
		e8272_write_data (fdc, val);
		break;
	default:
#if E8272_DEBUG >= 1
		fprintf (stderr, "E8272: set %04lx <- %02x\n", addr, val);
#endif
		break;
	}
}

void e8272_set_tc (e8272_t *fdc, unsigned char val)
{
	if (val == 0) {
		return;
	}
#if E8272_DEBUG >= 2
	fprintf (stderr, "E8272: TC\n");
#endif
	if (fdc->set_tc != NULL) {
		fdc->set_tc (fdc);
	}
}

void e8272_clock (e8272_t *fdc, unsigned long n)
{
	if (fdc->disk_300_bps = 1)
	{
		fdc->track_clk += E8272_RATE_300 * n;
		fdc->track_pos += fdc->track_clk / fdc->input_clock;
		fdc->track_clk %= fdc->input_clock;
		if (fdc->track_pos >= fdc->track_size) {
			fdc->track_pos -= fdc->track_size;
			fdc->index_cnt += 1;
		}
		if (n < fdc->delay_clock) {
			fdc->delay_clock -= n;
		}
		else {
			fdc->delay_clock = 0;
		}
		if (fdc->set_clock != NULL) {
			fdc->set_clock(fdc, n);
		}
	}
	else
	{
		fdc->track_clk += E8272_RATE * n;
		fdc->track_pos += fdc->track_clk / fdc->input_clock;
		fdc->track_clk %= fdc->input_clock;
		if (fdc->track_pos >= fdc->track_size) {
			fdc->track_pos -= fdc->track_size;
			fdc->index_cnt += 1;
		}
		if (n < fdc->delay_clock) {
			fdc->delay_clock -= n;
		}
		else {
			fdc->delay_clock = 0;
		}
		if (fdc->set_clock != NULL) {
			fdc->set_clock(fdc, n);
		}
	}
}
/* ---------------------------------------------------------------------------------------------- */
/* SYSTEM CONFIGURATION:                                                                          */
/* ---------------------------------------------------------------------------------------------- */
/*        MEMORIES:                                                                               */
/*           ^                                                                                    */
/*           |                                                                                    */
/*           v                                                                                    */
/*    8080 SYSTEM BUS:                     8080 SYSTEM BUS:                                       */
/*    ^                                           ^                                               */
/*    | DB0-7                                     | A0                                            */
/*    | MEMR                                      | DB0-7                                         */
/*    | IOR                                       | RD                                            */
/*    | MEMW                                      | WR                                            */
/*    | IOW                                       | CS                                            */
/*    | CS                                        | INT                                           */
/*    | HRQ                                       | RESET                                         */
/*    | HLDA                                      v                                               */
/*    v                                       uPD765 FDC: DATA WINDOW:  PLL:                      */
/*                                            uPD765 FDC:<--------------PLL:                      */
/*                                            uPD765 FDC: RD DATA:      PLL:---->DRIVE INTERFACE  */
/* uPD8257 DMA CONTROLLER: DRQ:               uPD765 FDC:<--------------PLL:---->DRIVE INTERFACE  */
/* uPD8257 DMA CONTROLLER:<-------------------uPD765 FDC:      WR DATA:          DRIVE INTERFACE  */
/* uPD8257 DMA CONTROLLER: DACK:              uPD765 FDC:<---------------------->DRIVE INTERFACE  */
/* uPD8257 DMA CONTROLLER: TC TERMINAL COUNT: uPD765 FDC: INPUT CONTROL:         DRIVE INTERFACE  */
/*                                            uPD765 FDC:<---------------------->DRIVE INTERFACE  */
/*                                            uPD765 FDC:OUTPUT CONTROL:         DRIVE INTERFACE  */
/*                                            uPD765 FDC:<---------------------->DRIVE INTERFACE  */
/* ---------------------------------------------------------------------------------------------- */
