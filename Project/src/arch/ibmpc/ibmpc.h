/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/ibmpc/ibmpc.h                                       *
 * Created:     2001-05-01 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2001-2011 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_IBMPC_H
#define PCE_IBMPC_H 1


#include "cassette.h"
#include "ems.h"
#include "keyboard.h"
#include "speaker.h"
#include "xms.h"

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
#include <devices/video/video.h>

#include <drivers/block/block.h>
#include <drivers/video/terminal.h>
#include <lib/brkpt.h>
#include <libini/libini.h>

#define IBMPS2_PARALLEL_PORT_OUTPUT_ENABLE 7 /* Parallel Port Output Enable */
#define IBMPS2_RESERVED_1                  6 /* Reserved = 0                */
#define IBMPS2_RESERVED_2                  5 /* Reserved = 0                */
#define IBMPS2_SERIAL_CS                   4 /* Serial        CS            */
#define IBMPS2_DISKETTE_CS                 3 /* Diskette      CS            */
#define IBMPS2_VIDEO_CS                    2 /* Video         CS            */
#define IBMPS2_PARALLEL_PORT_CS            1 /* Parallel Port CS            */
#define IBMPS2_FIXED_DISK_CS               0 /* Fixed Disk    CS            */

#define IBMPS2_IO_PORT_61_RESERVED_7        7 /* Reserved                 */
#define IBMPS2_IO_PORT_61_RESERVED_6        6 /* Reserved                 */
#define IBMPS2_IO_PORT_61_ENA_IO_CH_CK      5 /* -ENA I/O CH     CK       */
#define IBMPS2_IO_PORT_61_ENA_RAM_PARITY_CK 4 /* -ENA RAM PARITY CK       */
#define IBMPS2_IO_PORT_61_RESERVED_3        3 /* Reserved                 */
#define IBMPS2_IO_PORT_61_RESERVED_2        2 /* Reserved                 */
#define IBMPS2_IO_PORT_61_BEEPER_DATA       1 /* Beeper Data              */
#define IBMPS2_IO_PORT_61_TIMER_2_GATE      0 /* Timer 2 Gate (to beeper) */

#define IBMPS2_IO_PORT_62_PARITY                7 /* Parity                */
#define IBMPS2_IO_PORT_62_IO_CH_CK              6 /* I/O CH CK             */
#define IBMPS2_IO_PORT_62_TIMER_2_OUTPUT        5 /* Timer 2 Output        */
#define IBMPS2_IO_PORT_62_RESERVED_4            4 /* Reserved              */
#define IBMPS2_IO_PORT_62_RESERVED_3            3 /* Reserved              */
#define IBMPS2_IO_PORT_62_DISK_INSTALLED        2 /* Disk Installed        */
#define IBMPS2_IO_PORT_62_COPROCESSOR_INSTALLED 1 /* Coprocessor Installed */
#define IBMPS2_IO_PORT_62_RESERVED_0            0 /* Reserved              */

#define IBMPS2_SYSTEM_BOARD_RAM_CONTROL_STATUS_REGISTER_PARITY_CHECK_POINTER      7 /* Parity Check Pointer     */
#define IBMPS2_SYSTEM_BOARD_RAM_CONTROL_STATUS_REGISTER_ENABLE_RAM_90000_TO_9FFFF 6 /* -Enable RAM, 90000-9FFFF */
#define IBMPS2_SYSTEM_BOARD_RAM_CONTROL_STATUS_REGISTER_ENABLE_RAM_80000_TO_8FFFF 5 /* -Enable RAM, 80000-8FFFF */
#define IBMPS2_SYSTEM_BOARD_RAM_CONTROL_STATUS_REGISTER_ENABLE_RAM_70000_TO_7FFFF 4 /* -Enable RAM, 70000-7FFFF */
#define IBMPS2_SYSTEM_BOARD_RAM_CONTROL_STATUS_REGISTER_ENABLE_RAM_60000_TO_6FFFF 3 /* -Enable RAM, 60000-6FFFF */
#define IBMPS2_SYSTEM_BOARD_RAM_CONTROL_STATUS_REGISTER_ENABLE_RAM_50000_TO_5FFFF 2 /* -Enable RAM, 50000-5FFFF */
#define IBMPS2_SYSTEM_BOARD_RAM_CONTROL_STATUS_REGISTER_ENABLE_RAM_40000_TO_4FFFF 1 /* -Enable RAM, 40000-4FFFF */
#define IBMPS2_SYSTEM_BOARD_RAM_CONTROL_STATUS_REGISTER_REMAP_LOW_MEMORY          0 /* Remap Low Memory         */
/* ---------------------------------------------------------------------------------------------- */
/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; Diskette Drive Interface: */
/* ---------------------------------------------------------------------------------------------- */
/* The diskette gate array contains the decode logic for the internal reg-isters, the write logic,*/
/* and the read logic. The gate array:                                                            */
/*  - Controls the clock signals needed for read and write                                        */
/*  - Controls write precompensation                                                              */
/*  - Selects the data rate of transfer                                                           */
/*  - Provides a mask for the interrupt and DMA request lines                                     */
/*  - Provides phase error detection for input to the phase-lock loop                             */
/* The phase detector/amplifier and the voltage controlled oscillator (VCO) make up the phase-lock*/
/* loop (PLL). They adjust the clock used during data read to keep it in phase with the data      */
/* signal.                                                                                        */
/*                                                                                                */
/* The drives connect to the system board through a single 40-pin con-nector, which supplies all  */
/* signals necessary to operate two diskette drives. The diskette drives are attached to the      */
/* connector through an internal, flat cable.                                                     */
/* ---------------------------------------------------------------------------------------------- */
/* Gate Array Registers:                                                                          */
/* ---------------------------------------------------------------------------------------------- */
/*The diskette gate array has five registers: three registers that show the status of signals used*/
/* in diskette operations, and two registers that control certain interface signals.              */
/* ---------------------------------------------------------------------------------------------- */
/* RAS Port A Registers:                                                                          */
/* ---------------------------------------------------------------------------------------------- */
/* The RAS Port A register, hex 3F0, is a read-only register that shows the status of the         */
/* corresponding signals.                                                                         */
/* ---------------------------------------------------------------------------------------------- */
/* Bit: Function:                                                                                 */
/*  - 7: - IRQ8                                                                                   */
/*  - 6: - DRQ2                                                                                   */
/*  - 5: - Step (latched)                                                                         */
/*  - 4: - Track 0                                                                                */
/*  - 3: - -Head 1 Select                                                                         */
/*  - 2: - Index                                                                                  */
/*  - 1: - Write Protect                                                                          */
/*  - 0: - -Direction                                                                             */
/* ---------------------------------------------------------------------------------------------- */
/* Figure 1-63. RAS Port A, Hex 3F0                                                               */
/* ---------------------------------------------------------------------------------------------- */
#define IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_A_IRQ8          7
#define IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_A_DRQ2          6
#define IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_A_STEP_LATCHED  5
#define IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_A_TRACK_0       4
#define IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_A_HEAD_1_SELECT 3
#define IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_A_INDEX         2
#define IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_A_WRITE_PROTECT 1
#define IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_A_DIRECTION     0
/* ------------------------------------------------------------------------ */
/* IBM PC (Model 5150/5160): Quote from "Technical Reference"; Bibliography */
/* ------------------------------------------------------------------------ */
/* Intel Corporation. The 8086 Family User's Manual. This manual introduces */
/* the 8086 family of microprocessing components and serves as a reference  */
/* in system design and implementation.                                     */
/*                                                                          */
/* Intel Corporation. 8086/8087/8088 Macro Assembly Reference Manual for    */
/* 8088/8085 Based Development System. This manual describes the 8086/8087/ */
/* 8088 Macro Assembly Language, and is intended for use by persons who are */
/* familiar with assembly language.                                         */
/*                                                                          */
/* Intel Corporation. Component Data Catalog This book describes Intel      */
/* components and their technical specifications.                           */
/*                                                                          */
/* Motorola, Inc. The Complete Microcomputer Data Library. This book        */
/* describes Motorola components and their technical specifications.        */
/*                                                                          */
/* National Semiconductor Corporation. 250 Asynchronous Communications      */
/* Element book documents physical and operating characteristics of the INS */
/* 8250.                                                                    */
/* ------------------------------------------------------------------------ */
typedef struct ibmpc_t {
	unsigned           pc;
	e8086_t            *cpu;
	video_t            *video;
	disks_t            *dsk;
	disk_t             *dsk0;
	memory_t           *mem;
	mem_blk_t          *ram;
	memory_t           *prt;
	nvram_t            *nvr;
	e8237_t            dma;
	e8253_t            pit;
	e8255_t            ppi;
	e8259_t            pic;
	pc_kbd_t           kbd;
	pc_cassette_t      *cas;
	pc_speaker_t       spk;
	unsigned           model;
	unsigned char      ppi_port_a[2];
	unsigned char      ppi_port_b;
	unsigned char      ppi_port_c[2];
	unsigned char      m24_config[2];
	unsigned long      ibmps2_config;
	unsigned long      ibmps2_io_port_61;
	unsigned long      ibmps2_io_port_62;
	unsigned long      ibmps2_system_board_ram_control_status_register;
	dev_fdc_t          *fdc;
	hdc_t              *hdc;
	ems_t              *ems;
	xms_t              *xms;
	terminal_t         *trm;
	serport_t          *serport[4];
	parport_t          *parport[4];
	ini_sct_t          *cfg;
	bp_set_t           bps;
	unsigned           bootdrive;
	unsigned long      dma_page[4];
	unsigned char      timer1_out;
	unsigned char      dack0;
	char               patch_bios_init;
	char               patch_bios_int19;
	char               memtest;
	/* allow int 1a to get real time */
	int                support_rtc;
	unsigned           fd_cnt;
	unsigned           hd_cnt;
	unsigned long      sync_clock2_sim;
	unsigned long      sync_clock2_real;
	unsigned long      sync_interval;
	/* cpu speed factor */
	unsigned           speed_current;
	unsigned           speed_saved;
	unsigned long      speed_clock_extra;
	unsigned           current_int;
	unsigned long      clk_div[4];
	unsigned long      ps2model25_clk_div[8];
	unsigned long      clock1;
	unsigned long      clock2;
	unsigned           brk;
	char               pause;
	/* Power Supply Unit */
	unsigned long  psu_watt_1; /* Power Supply Wattage Rating (first  digits as in "300") */
	unsigned long  psu_watt_2; /* Power Supply Wattage Rating (second digits as in ".00") */
	/* Number of power connectors currently in use */
	unsigned long  psu_number_of_plus_5_vdc_connectors_in_use;
	unsigned long  psu_number_of_minus_5_vdc_connectors_in_use;
	unsigned long  psu_number_of_plus12_vdc_connectors_in_use;
	unsigned long  psu_number_of_minus12_vdc_connectors_in_use;
	/* +5 VDC (Direct Current) */
	unsigned long  psu_dc_output_voltage_nominal_plus_5_vdc_minimum_current_amps;
	unsigned long  psu_dc_output_voltage_nominal_plus_5_vdc_maximum_current_amps;
	unsigned long  psu_dc_output_voltage_nominal_plus_5_vdc_regulation_tolerance_plus_percentage;
	unsigned long  psu_dc_output_voltage_nominal_plus_5_vdc_regulation_tolerance_minus_percentage;
	/* -5 VDC (Direct Current) */
	unsigned long  psu_dc_output_voltage_nominal_minus_5_vdc_minimum_current_amps;
	unsigned long  psu_dc_output_voltage_nominal_minus_5_vdc_maximum_current_amps;
	unsigned long  psu_dc_output_voltage_nominal_minus_5_vdc_regulation_tolerance_plus_percentage;
	unsigned long  psu_dc_output_voltage_nominal_minus_5_vdc_regulation_tolerance_minus_percentage;
	/* +12VDC (Direct Current) */
	unsigned long  psu_dc_output_voltage_nominal_plus12_vdc_minimum_current_amps;
	unsigned long  psu_dc_output_voltage_nominal_plus12_vdc_maximum_current_amps;
	unsigned long  psu_dc_output_voltage_nominal_plus12_vdc_regulation_tolerance_plus_percentage;
	unsigned long  psu_dc_output_voltage_nominal_plus12_vdc_regulation_tolerance_minus_percentage;
	/* -12VDC (Direct Current) */
	unsigned long  psu_dc_output_voltage_nominal_minus12_vdc_minimum_current_amps;
	unsigned long  psu_dc_output_voltage_nominal_minus12_vdc_maximum_current_amps;
	unsigned long  psu_dc_output_voltage_nominal_minus12_vdc_regulation_tolerance_plus_percentage;
	unsigned long  psu_dc_output_voltage_nominal_minus12_vdc_regulation_tolerance_minus_percentage;
	/* +12VDC (Direct Current) (secondary) */
	unsigned long  psu_dc_output_voltage_nominal_secondary_plus12_vdc_minimum_current_amps;
	unsigned long  psu_dc_output_voltage_nominal_secondary_plus12_vdc_maximum_current_amps;
	unsigned long  psu_dc_output_voltage_nominal_secondary_plus12_vdc_regulation_tolerance_plus_percentage;
	unsigned long  psu_dc_output_voltage_nominal_secondary_plus12_vdc_regulation_tolerance_minus_percentage;
	/* -12VDC (Direct Current) (secondary) */
	unsigned long  psu_dc_output_voltage_nominal_secondary_minus12_vdc_minimum_current_amps;
	unsigned long  psu_dc_output_voltage_nominal_secondary_minus12_vdc_maximum_current_amps;
	unsigned long  psu_dc_output_voltage_nominal_secondary_minus12_vdc_regulation_tolerance_plus_percentage;
	unsigned long  psu_dc_output_voltage_nominal_secondary_minus12_vdc_regulation_tolerance_minus_percentage;
	/* Voltage Limits */
	unsigned long  psu_dc_output_voltage_plus_5_maximum_rated_voltage_limit;
	unsigned long  psu_dc_output_voltage_plus_5_nominal_value_limit;
	unsigned long  psu_dc_output_voltage_plus12_maximum_rated_voltage_limit;
	unsigned long  psu_dc_output_voltage_plus12_nominal_value_limit;
	/* Voltage Limits (secondary) */
	unsigned long  psu_dc_output_voltage_secondary_plus12_maximum_rated_voltage_limit;
	unsigned long  psu_dc_output_voltage_secondary_plus12_nominal_value_limit;
	/* Power Good Signal */
	unsigned long  psu_dc_power_good_signal_ttl_compatible_up_value;
	unsigned long  psu_dc_power_good_signal_ttl_compatible_up_value_ua;
	unsigned long  psu_dc_power_good_signal_ttl_compatible_down_value;
	unsigned long  psu_dc_power_good_signal_ttl_compatible_down_value_ua;
	/* Power Good Signal (+5 VDC) */
	unsigned long  psu_dc_power_good_signal_output_voltage_down_plus_5_vdc_under_voltage_nominal_sense_level;
	unsigned long  psu_dc_power_good_signal_output_voltage_down_plus_5_vdc_over_voltage_nominal_sense_level;
	/* Power Good Signal (-5 VDC) */
	unsigned long  psu_dc_power_good_signal_output_voltage_down_minus_5_vdc_under_voltage_nominal_sense_level;
	unsigned long  psu_dc_power_good_signal_output_voltage_down_minus_5_vdc_over_voltage_nominal_sense_level;
	/* Power Good Signal (+12VDC) */
	unsigned long  psu_dc_power_good_signal_output_voltage_down_plus12_vdc_under_voltage_nominal_sense_level;
	unsigned long  psu_dc_power_good_signal_output_voltage_down_plus12_vdc_over_voltage_nominal_sense_level;
	/* Power Good Signal (-12VDC) */
	unsigned long  psu_dc_power_good_signal_output_voltage_down_minus12_vdc_under_voltage_nominal_sense_level;
	unsigned long  psu_dc_power_good_signal_output_voltage_down_minus12_vdc_over_voltage_nominal_sense_level;
	/* Power Good Signal (+12VDC) (secondary) */
	unsigned long  psu_dc_power_good_signal_output_voltage_down_secondary_plus12_vdc_under_voltage_nominal_sense_level;
	unsigned long  psu_dc_power_good_signal_output_voltage_down_secondary_plus12_vdc_over_voltage_nominal_sense_level;
	/* Power Good Signal (-12VDC) (secondary) */
	unsigned long  psu_dc_power_good_signal_output_voltage_down_secondary_minus12_vdc_under_voltage_nominal_sense_level;
	unsigned long  psu_dc_power_good_signal_output_voltage_down_secondary_minus12_vdc_over_voltage_nominal_sense_level;
	/* Power Good Signal Turn-On Delay */
	unsigned long  psu_dc_power_good_signal_turn_on_delay;
	/* Power Good Signal */
	unsigned long  psu_dc_power_good_signal;
	/* Power Supply Unit */
} ibmpc_t;

#define ibmps2_get_control_register(pc) ((pc)->ibmps2_config)  /* Retieve  FLAGS register for Intel 80186 CPU and lower */
#define ibmps2_get_cs(pc, f) (((pc)->ibmps2_config & (f)) != 0)
#define ibmps2_get_parallel_port_output_enable(pc) (((pc)->ibmps2_config & IBMPS2_PARALLEL_PORT_OUTPUT_ENABLE) != 0) /* Sign        Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_get_reserved_1(pc) (((pc)->ibmps2_config & IBMPS2_RESERVED_1) != 0) /* Carry       Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_get_reserved_2(pc) (((pc)->ibmps2_config & IBMPS2_RESERVED_2) != 0) /* Parity      Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_get_serial_cs(pc) (((pc)->ibmps2_config & IBMPS2_SERIAL_CS) != 0) /* Accumulator Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_get_diskette_cs(pc) (((pc)->ibmps2_config & IBMPS2_DISKETTE_CS) != 0) /* Zero        Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_get_video_cs(pc) (((pc)->ibmps2_config & IBMPS2_VIDEO_CS) != 0) /* Overflow    Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_get_parallel_port_cs(pc) (((pc)->ibmps2_config & IBMPS2_PARALLEL_PORT_CS) != 0) /* Direction   Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_get_fixed_disk_cs(pc) (((pc)->ibmps2_config & IBMPS2_FIXED_DISK_CS) != 0) /* Interrupt   Flag (Intel 8086/8088 CPU and higher) */

#define ibmps2_get_io_port_61(pc) ((pc)->ibmps2_io_port_61)  /* Retieve  FLAGS register for Intel 80186 CPU and lower */
#define ibmps2_get_io_61(pc, f) (((pc)->ibmps2_io_port_61 & (f)) != 0)
#define ibmps2_get_io_port_61_reserved_7(pc) (((pc)->ibmps2_io_port_61 & IBMPS2_IO_PORT_61_RESERVED_7) != 0) /* Sign        Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_get_io_port_61_reserved_6(pc) (((pc)->ibmps2_io_port_61 & IBMPS2_IO_PORT_61_RESERVED_6) != 0) /* Carry       Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_get_io_port_61_ena_io_ch_ck(pc) (((pc)->ibmps2_io_port_61 & IBMPS2_IO_PORT_61_ENA_IO_CH_CK) != 0) /* Parity      Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_get_io_port_61_ena_ram_parity_ck(pc) (((pc)->ibmps2_io_port_61 & IBMPS2_IO_PORT_61_ENA_RAM_PARITY_CK) != 0) /* Accumulator Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_get_io_port_61_reserved_3(pc) (((pc)->ibmps2_io_port_61 & IBMPS2_IO_PORT_61_RESERVED_3) != 0) /* Zero        Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_get_io_port_61_reserved_2(pc) (((pc)->ibmps2_io_port_61 & IBMPS2_IO_PORT_61_RESERVED_2) != 0) /* Overflow    Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_get_io_port_61_beeper_data(pc) (((pc)->ibmps2_io_port_61 & IBMPS2_IO_PORT_61_BEEPER_DATA) != 0) /* Direction   Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_get_io_port_61_timer_2_gate(pc) (((pc)->ibmps2_io_port_61 & IBMPS2_IO_PORT_61_TIMER_2_GATE) != 0) /* Interrupt   Flag (Intel 8086/8088 CPU and higher) */

#define ibmps2_get_io_port_62(pc) ((pc)->ibmps2_io_port_62)  /* Retieve  FLAGS register for Intel 80186 CPU and lower */
#define ibmps2_get_io_62(pc, f) (((pc)->ibmps2_io_port_62 & (f)) != 0)
#define ibmps2_get_io_port_62_parity(pc) (((pc)->ibmps2_io_port_62 & IBMPS2_IO_PORT_62_PARITY) != 0) /* Sign        Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_get_io_port_62_io_ch_ck(pc) (((pc)->ibmps2_io_port_62 & IBMPS2_IO_PORT_62_IO_CH_CK) != 0) /* Carry       Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_get_io_port_62_timer_2_output(pc) (((pc)->ibmps2_io_port_62 & IBMPS2_IO_PORT_62_TIMER_2_OUTPUT) != 0) /* Parity      Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_get_io_port_62_reserved_4(pc) (((pc)->ibmps2_io_port_62 & IBMPS2_IO_PORT_62_RESERVED_4) != 0) /* Accumulator Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_get_io_port_62_reserved_3(pc) (((pc)->ibmps2_io_port_62 & IBMPS2_IO_PORT_62_RESERVED_3) != 0) /* Zero        Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_get_io_port_62_disk_installed(pc) (((pc)->ibmps2_io_port_62 & IBMPS2_IO_PORT_62_DISK_INSTALLED) != 0) /* Overflow    Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_get_io_port_62_coprocessor_installed(pc) (((pc)->ibmps2_io_port_62 & IBMPS2_IO_PORT_62_COPROCESSOR_INSTALLED) != 0) /* Direction   Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_get_io_port_62_reserved_0(pc) (((pc)->ibmps2_io_port_62 & IBMPS2_IO_PORT_62_RESERVED_0) != 0) /* Interrupt   Flag (Intel 8086/8088 CPU and higher) */

#define ibmps2_get_system_board_ram_control_status_register(pc) ((pc)->ibmps2_system_board_ram_control_status_register)  /* Retieve  FLAGS register for Intel 80186 CPU and lower */
#define ibmps2_get_system_board_ram_csr(pc, f) (((pc)->ibmps2_system_board_ram_control_status_register & (f)) != 0)
#define ibmps2_get_system_board_ram_control_status_register_parity_check_pointer(pc) (((pc)->ibmps2_system_board_ram_control_status_register & IBMPS2_SYSTEM_BOARD_RAM_CONTROL_STATUS_REGISTER_PARITY_CHECK_POINTER) != 0) /* Sign        Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_get_system_board_ram_control_status_register_enable_ram_90000_to_9ffff(pc) (((pc)->ibmps2_system_board_ram_control_status_register & IBMPS2_SYSTEM_BOARD_RAM_CONTROL_STATUS_REGISTER_ENABLE_RAM_90000_TO_9FFFF) != 0) /* Carry       Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_get_system_board_ram_control_status_register_enable_ram_80000_to_8ffff(pc) (((pc)->ibmps2_system_board_ram_control_status_register & IBMPS2_SYSTEM_BOARD_RAM_CONTROL_STATUS_REGISTER_ENABLE_RAM_80000_TO_8FFFF) != 0) /* Parity      Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_get_system_board_ram_control_status_register_enable_ram_70000_to_7ffff(pc) (((pc)->ibmps2_system_board_ram_control_status_register & IBMPS2_SYSTEM_BOARD_RAM_CONTROL_STATUS_REGISTER_ENABLE_RAM_70000_TO_7FFFF) != 0) /* Accumulator Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_get_system_board_ram_control_status_register_enable_ram_60000_to_6ffff(pc) (((pc)->ibmps2_system_board_ram_control_status_register & IBMPS2_SYSTEM_BOARD_RAM_CONTROL_STATUS_REGISTER_ENABLE_RAM_60000_TO_6FFFF) != 0) /* Zero        Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_get_system_board_ram_control_status_register_enable_ram_50000_to_5ffff(pc) (((pc)->ibmps2_system_board_ram_control_status_register & IBMPS2_SYSTEM_BOARD_RAM_CONTROL_STATUS_REGISTER_ENABLE_RAM_50000_TO_5FFFF) != 0) /* Overflow    Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_get_system_board_ram_control_status_register_enable_ram_40000_to_4ffff(pc) (((pc)->ibmps2_system_board_ram_control_status_register & IBMPS2_SYSTEM_BOARD_RAM_CONTROL_STATUS_REGISTER_ENABLE_RAM_40000_TO_4FFFF) != 0) /* Direction   Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_get_system_board_ram_control_status_register_remap_low_memory(pc) (((pc)->ibmps2_system_board_ram_control_status_register & IBMPS2_SYSTEM_BOARD_RAM_CONTROL_STATUS_REGISTER_REMAP_LOW_MEMORY) != 0) /* Interrupt   Flag (Intel 8086/8088 CPU and higher) */

#define ibmps2_set_control_register(pc, v) do { (pc)->ibmps2_config = (v) & 0xffffffffU; } while (0)      /* Set  FLAGS register for Intel  8086 */

#define ibmps2_set_cs(pc, f, v) \
	do { if (v) (pc)->ibmps2_config |= (f); else (pc)->ibmps2_config &= ~(f); } while (0)

#define ibmps2_set_parallel_port_output_enable(pc, v) ibmps2_set_cs (pc, IBMPS2_PARALLEL_PORT_OUTPUT_ENABLE, v) /* Carry       Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_set_reserved_1(pc, v) ibmps2_set_cs (pc, IBMPS2_RESERVED_1, v) /* Parity      Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_set_reserved_2(pc, v) ibmps2_set_cs (pc, IBMPS2_RESERVED_2, v) /* Accumulator Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_set_serial_cs(pc, v) ibmps2_set_cs (pc, IBMPS2_SERIAL_CS, v) /* Zero        Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_set_diskette_cs(pc, v) ibmps2_set_cs (pc, IBMPS2_DISKETTE_CS, v) /* Overflow    Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_set_video_cs(pc, v) ibmps2_set_cs (pc, IBMPS2_VIDEO_CS, v) /* Sign        Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_set_parallel_port_cs(pc, v) ibmps2_set_cs (pc, IBMPS2_PARALLEL_PORT_CS, v) /* Direction   Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_set_fixed_disk_cs(pc, v) ibmps2_set_cs (pc, IBMPS2_FIXED_DISK_CS, v) /* Interrupt   Flag (Intel 8086/8088 CPU and higher) */

#define ibmps2_set_io_port_61(pc, v) do { (pc)->ibmps2_io_port_61 = (v) & 0xffffffffU; } while (0)      /* Set  FLAGS register for Intel  8086 */

#define ibmps2_set_io_61(pc, f, v) \
	do { if (v) (pc)->ibmps2_io_port_61 |= (f); else (pc)->ibmps2_io_port_61 &= ~(f); } while (0)

#define ibmps2_set_io_port_61_reserved_7(pc, v) ibmps2_set_io_61 (pc, IBMPS2_IO_PORT_61_RESERVED_7, v) /* Carry       Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_set_io_port_61_reserved_6(pc, v) ibmps2_set_io_61 (pc, IBMPS2_IO_PORT_61_RESERVED_6, v) /* Parity      Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_set_io_port_61_ena_io_ch_ck(pc, v) ibmps2_set_io_61 (pc, IBMPS2_IO_PORT_61_ENA_IO_CH_CK, v) /* Accumulator Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_set_io_port_61_ena_ram_parity_ck(pc, v) ibmps2_set_io_61 (pc, IBMPS2_IO_PORT_61_ENA_RAM_PARITY_CK, v) /* Zero        Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_set_io_port_61_reserved_3(pc, v) ibmps2_set_io_61 (pc, IBMPS2_IO_PORT_61_RESERVED_3, v) /* Overflow    Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_set_io_port_61_reserved_2(pc, v) ibmps2_set_io_61 (pc, IBMPS2_IO_PORT_61_RESERVED_2, v) /* Sign        Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_set_io_port_61_beeper_data(pc, v) ibmps2_set_io_61 (pc, IBMPS2_IO_PORT_61_BEEPER_DATA, v) /* Direction   Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_set_io_port_61_timer_2_gate(pc, v) ibmps2_set_io_61 (pc, IBMPS2_IO_PORT_61_TIMER_2_GATE, v) /* Interrupt   Flag (Intel 8086/8088 CPU and higher) */

#define ibmps2_set_io_port_62(pc, v) do { (pc)->ibmps2_io_port_62 = (v) & 0xffffffffU; } while (0)      /* Set  FLAGS register for Intel  8086 */

#define ibmps2_set_io_62(pc, f, v) \
	do { if (v) (pc)->ibmps2_io_port_62 |= (f); else (pc)->ibmps2_io_port_62 &= ~(f); } while (0)

#define ibmps2_set_io_port_62_parity(pc, v) ibmps2_set_io_62 (pc, IBMPS2_IO_PORT_62_PARITY, v) /* Carry       Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_set_io_port_62_io_ch_ck(pc, v) ibmps2_set_io_62 (pc, IBMPS2_IO_PORT_62_IO_CH_CK, v) /* Parity      Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_set_io_port_62_timer_2_output(pc, v) ibmps2_set_io_62 (pc, IBMPS2_IO_PORT_62_TIMER_2_OUTPUT, v) /* Accumulator Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_set_io_port_62_reserved_4(pc, v) ibmps2_set_io_62 (pc, IBMPS2_IO_PORT_62_RESERVED_4, v) /* Zero        Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_set_io_port_62_reserved_3(pc, v) ibmps2_set_io_62 (pc, IBMPS2_IO_PORT_62_RESERVED_3, v) /* Overflow    Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_set_io_port_62_disk_installed(pc, v) ibmps2_set_io_62 (pc, IBMPS2_IO_PORT_62_DISK_INSTALLED, v) /* Sign        Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_set_io_port_62_coprocessor_installed(pc, v) ibmps2_set_io_62 (pc, IBMPS2_IO_PORT_62_COPROCESSOR_INSTALLED, v) /* Direction   Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_set_io_port_62_reserved_0(pc, v) ibmps2_set_io_62 (pc, IBMPS2_IO_PORT_62_RESERVED_0, v) /* Interrupt   Flag (Intel 8086/8088 CPU and higher) */

#define ibmps2_set_system_board_ram_control_status_register(pc, v) do { (pc)->ibmps2_system_board_ram_control_status_register = (v) & 0xffffffffU; } while (0)      /* Set  FLAGS register for Intel  8086 */

#define ibmps2_set_system_board_ram_csr(pc, f, v) \
	do { if (v) (pc)->ibmps2_system_board_ram_control_status_register |= (f); else (pc)->ibmps2_system_board_ram_control_status_register &= ~(f); } while (0)

#define ibmps2_set_system_board_ram_control_status_register_parity_check_pointer(pc, v) ibmps2_set_system_board_ram_csr (pc, IBMPS2_SYSTEM_BOARD_RAM_CONTROL_STATUS_REGISTER_PARITY_CHECK_POINTER, v) /* Carry       Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_set_system_board_ram_control_status_register_enable_ram_90000_to_9ffff(pc, v) ibmps2_set_system_board_ram_csr (pc, IBMPS2_SYSTEM_BOARD_RAM_CONTROL_STATUS_REGISTER_ENABLE_RAM_90000_TO_9FFFF, v) /* Accumulator Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_set_system_board_ram_control_status_register_enable_ram_80000_to_8ffff(pc, v) ibmps2_set_system_board_ram_csr (pc, IBMPS2_SYSTEM_BOARD_RAM_CONTROL_STATUS_REGISTER_ENABLE_RAM_80000_TO_8FFFF, v) /* Zero        Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_set_system_board_ram_control_status_register_enable_ram_70000_to_7ffff(pc, v) ibmps2_set_system_board_ram_csr (pc, IBMPS2_SYSTEM_BOARD_RAM_CONTROL_STATUS_REGISTER_ENABLE_RAM_70000_TO_7FFFF, v) /* Overflow    Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_set_system_board_ram_control_status_register_enable_ram_60000_to_6ffff(pc, v) ibmps2_set_system_board_ram_csr (pc, IBMPS2_SYSTEM_BOARD_RAM_CONTROL_STATUS_REGISTER_ENABLE_RAM_60000_TO_6FFFF, v) /* Sign        Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_set_system_board_ram_control_status_register_enable_ram_50000_to_5ffff(pc, v) ibmps2_set_system_board_ram_csr (pc, IBMPS2_SYSTEM_BOARD_RAM_CONTROL_STATUS_REGISTER_ENABLE_RAM_50000_TO_5FFFF, v) /* Direction   Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_set_system_board_ram_control_status_register_enable_ram_40000_to_4ffff(pc, v) ibmps2_set_system_board_ram_csr (pc, IBMPS2_SYSTEM_BOARD_RAM_CONTROL_STATUS_REGISTER_ENABLE_RAM_40000_TO_4FFFF, v) /* Interrupt   Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_set_system_board_ram_control_status_register_remap_low_memory(pc, v) ibmps2_set_system_board_ram_csr (pc, IBMPS2_SYSTEM_BOARD_RAM_CONTROL_STATUS_REGISTER_REMAP_LOW_MEMORY, v) /* Interrupt   Flag (Intel 8086/8088 CPU and higher) */

extern ibmpc_t *par_pc;
ibmpc_t *pc_new (ini_sct_t *ini);
void pc_del (ibmpc_t *pc);
int pc_set_serport_driver (ibmpc_t *pc, unsigned port, const char *driver);
int pc_set_serport_file (ibmpc_t *pc, unsigned port, const char *fname);
int pc_set_parport_driver (ibmpc_t *pc, unsigned port, const char *driver);
int pc_set_parport_file (ibmpc_t *pc, unsigned port, const char *fname);
const char *pc_intlog_get (ibmpc_t *pc, unsigned n);
void pc_intlog_set (ibmpc_t *pc, unsigned n, const char *expr);
int pc_intlog_check (ibmpc_t *pc, unsigned n);
unsigned pc_get_pcex_seg (ibmpc_t *pc);
/* ------------------------------------------------------------------------ */
/* IBM PS/2 Model 25: Quote from "Technical Reference"; General Description */
/* ------------------------------------------------------------------------ */
/* The IBM Personal System/2TM Model 25 (Type 8525) is a highly inte-       */
/* grated system using five gate arrays: two for microprocessor             */
/* support, two for video support, and one for the diskette controller      */
/* support. The major functional areas are:                                 */
/*                                                                          */
/* -  8086-2 microprocessor and its support logic                           */
/*                                                                          */
/* -  512K read/write memory standard, with an additional 128K              */
/*    optional for a total of 640K                                          */
/*                                                                          */
/* -  64K read-only memory (ROM)                                            */
/*                                                                          */
/* -  Input/Output (I/O) channel                                            */
/*                                                                          */
/* -  Integrated I/O functions                                              */
/*                                                                          */
/*     -  Color/graphis subsystem                                           */
/*     -  Diskette drive interface                                          */
/*     -  Fixed disk connector                                              */
/*     -  Serial port                                                       */
/*     -  Parallel port                                                     */
/*     -  Keyboard and pointing device ports                                */
/*                                                                          */
/* System dc power and a 'power good' signal from the power supply          */
/* enter the system board through a 12-pin connector. Other connectors      */
/* on the system board are for attaching the keyboard, pointing device,     */
/* coprocessor, display, earphone, serial and parallel devices, fan, and    */
/* storage media.                                                           */
/*                                                                          */
/* Two 62-pin card-edge sockets are attached to a vertical expansion bus    */
/* that is mounted to the system board. The I/O channel is extended         */
/* to these two I/O slots.                                                  */
/* ------------------------------------------------------------------------ */
/* ---------------------------------------------------------------------------------------------- */
/* IBM PS/2 Model 25: Quote from "Technical Reference"; System Functional Diagram                 */
/* ---------------------------------------------------------------------------------------------- */
/*                                                            _____                               */
/*                                                           _|__ |                               */
/*                                                           |  | |                               */
/*     __________               ___               I/O Channel|  | |                               */
/*     |  8086  |               | |                          |  | |                               */
/*     | D9-D7  |<----------->^<--|_|-->^<---->^<---->^<---->|  |_|                               */
/*     |        |             |   ___   |      |      |      |__|                                 */
/*     | D8-D15 |<---<^>----->|<->| |___|      |      |                                           */
/*     |________|     |       | |_|          __|__  __|__                                         */
/*                    |       |              |___|  |___|                                         */
/*                    |       |                ^      ^_____________                              */
/*                    |       |                |  ____|______  ____|________  ___                 */
/* ____________       |       |                |  | 64K     |  | Video     |__| | Display         */
/* |  System  |<______|------<V>_____________  |  | Video   |  | Subsystem |  |_|                 */
/* |  Support |       |       | | 8237A-5   |_<V> | Memory  |  |___________|                      */
/* |  Gate    |<_____<V> <V> <V>| DMA       |  |  |_________|                                     */
/* |  Array   |           |   | |___________|  |                                                  */
/* |__________|           |   |                |                                                  */
/*                        |   |                |  ____________           ___                      */
/* ____________           |   |                |  | I/O      |___________| | Pointing             */
/* |   8087   |           |   |     ___       <V>-| Support  |           |_| Device               */
/* |  D0-D7   |___________|__<V>____| |____    |  | Gate     |___________| |                      */
/* |          |           |         |_|   |    |  | Array    |           |_| Keyboard             */
/* |          |           |        ___    |    |  |__________|____       __________               */
/* |  D8-D15  |__________<V>_______| |____|    |                 |_______|  Beeper|               */
/* |__________|                    |_|| |      |  __________     |       |________|___            */
/*                                    | |      |  | System |     |_______| Earphone  |            */
/* ____________                       | |     <V>-| Timer  |             | Connector |            */
/* | 64K      |______________________<V>|      |  |________|             |___________|            */
/* | ROM      |_______________________|<V>     |  __________             ___                      */
/* |__________|                       | |     <V>_| Serial |_____________| |Async                 */
/* ____________                       | |      |  | Port   |             |_|                      */
/* | 640K     |_______________________| |      |  |________|___          ___                      */
/* | RAM      |_________________________|     <V>_| Parallel  |__________| |Printer               */
/* |__________|                                |  | Port      |          |_|                      */
/*                                             |  |___________|   __________                      */
/*                                            <V>_| Diskette  |___| Drives |                      */
/*                                             |  | Interface |   |________|                      */
/*                                             |  |___________|                                   */
/*                                             |                         ___                      */
/*                                             |_________________________| |Fixed                 */
/* Figure 1-1.  System Functional Diagram                                |_|Disk                  */
/* ---------------------------------------------------------------------------------------------- */
/*!***************************************************************************
 * @short Reset the PC
 *****************************************************************************/
void pc_reset (ibmpc_t *pc);
unsigned long pc_get_clock2 (ibmpc_t *pc);
/*!***************************************************************************
 * @short Reset the clock counters
 *****************************************************************************/
void pc_clock_reset (ibmpc_t *pc);
/*!***************************************************************************
 * @short Synchronize the clocks after a discontinuity
 *****************************************************************************/
void pc_clock_discontinuity (ibmpc_t *pc);
/*!***************************************************************************
 * @short Clock the pc
 *****************************************************************************/
void pc_clock (ibmpc_t *pc, unsigned long cnt);
/*!***************************************************************************
 * @short Set the specific CPU model to be emulated
 *****************************************************************************/
int pc_set_cpu_model (ibmpc_t *pc, const char *str);
/*!***************************************************************************
 * @short Set the emulated cpu clock frequency as a multiple of 4.77 MHz
 *****************************************************************************/
void pc_set_speed (ibmpc_t *pc, unsigned factor);
void pc_set_bootdrive (ibmpc_t *pc, unsigned drv);
unsigned pc_get_bootdrive (ibmpc_t *pc);


#endif
