/*****************************************************************************
* pce                                                                       *
*****************************************************************************/

/*****************************************************************************
* File name:   src/devices/fdc.h                                            *
* Created:     2007-09-06 by Hampa Hug <hampa@hampa.ch>                     *
* Copyright:   (C) 2007-2011 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_DEVICES_FDC_H
#define PCE_DEVICES_FDC_H 1


#include <chipset/82xx/e8272.h>

#include <devices/device.h>
#include <devices/memory.h>

#include <drivers/block/block.h>

#define IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_A_IRQ8          7
#define IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_A_DRQ2          6
#define IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_A_STEP_LATCHED  5
#define IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_A_TRACK_0       4
#define IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_A_HEAD_1_SELECT 3
#define IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_A_INDEX         2
#define IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_A_WRITE_PROTECT 1
#define IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_A_DIRECTION     0

#define IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_B_RESERVED             7
#define IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_B_DRIVE_SELECT_1       6
#define IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_B_DRIVE_SELECT_0       5
#define IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_B_WRITE_DATA_LATCHED   4
#define IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_B_READ_DATA_LATCHED    3
#define IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_B_WRITE_ENABLE_LATCHED 2
#define IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_B_DRIVE_SELECT_3       1
#define IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_B_DRIVE_SELECT_2       0

#define IBMPS2_DISKETTE_DRIVE_INTERFACE_DIGITAL_INPUT_DISKETTE_CHANGE      7
#define IBMPS2_DISKETTE_DRIVE_INTERFACE_DIGITAL_INPUT_RESERVED_3           6
#define IBMPS2_DISKETTE_DRIVE_INTERFACE_DIGITAL_INPUT_RESERVED_2           5
#define IBMPS2_DISKETTE_DRIVE_INTERFACE_DIGITAL_INPUT_RESERVED_1           4
#define IBMPS2_DISKETTE_DRIVE_INTERFACE_DIGITAL_INPUT_DMA_ENABLE           3
#define IBMPS2_DISKETTE_DRIVE_INTERFACE_DIGITAL_INPUT_NO_WRITE_PRECOMP     2
#define IBMPS2_DISKETTE_DRIVE_INTERFACE_DIGITAL_INPUT_250K_BPS_RATE_SELECT 1
#define IBMPS2_DISKETTE_DRIVE_INTERFACE_DIGITAL_INPUT_RESERVED_0           0

typedef struct {
	unsigned      fdc;
	device_t      dev;
	e8272_t       e8272;
	mem_blk_t     blk;
	disks_t       *dsks;
	disk_t        *dsk;
	unsigned      drive[4];
	unsigned      dma_enable;
	unsigned      fdc_250_bps; /* 250 Bits Per Second (BPS) */
	unsigned      fdc_300_bps; /* 300 Bits Per Second (BPS) */
	unsigned      fdc_500_bps; /* 500 Bits Per Second (BPS) */
	unsigned long fdc_track_half;
	unsigned long fdc_track_total;
	unsigned      fdc_write_precomp;
	unsigned      ibmpc_at; /* Used to specify IBM Personal Computer/AT operation */
	unsigned      ibmps2;   /* Used to specify IBM Personal  System /2  operation */
							/* RAS   Port A   Register: */
	unsigned long ibmps2_fdc_ras_port_a;
	/* RAS   Port B   Register: */
	unsigned long ibmps2_fdc_ras_port_b;
	/* Digital Output Register: */
	unsigned long ibmps2_fdc_digital_input;
} dev_fdc_t;

#define ibmps2_get_fdc_ras_port_a(fdc) ((fdc)->ibmps2_fdc_ras_port_a)  /* Retieve  FLAGS register for Intel 80186 CPU and lower */
#define ibmps2_get_fdc_port_a(fdc, f) (((fdc)->ibmps2_fdc_ras_port_a & (f)) != 0)
#define ibmps2_get_fdc_ras_port_a_irq6(fdc) (((fdc)->ibmps2_fdc_ras_port_a & IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_A_IRQ8) != 0) /* Sign        Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_get_fdc_ras_port_a_drq2(fdc) (((fdc)->ibmps2_fdc_ras_port_a & IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_A_DRQ2) != 0) /* Carry       Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_get_fdc_ras_port_a_step_latched(fdc) (((fdc)->ibmps2_fdc_ras_port_a & IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_A_STEP_LATCHED) != 0) /* Parity      Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_get_fdc_ras_port_a_track_0(fdc) (((fdc)->ibmps2_fdc_ras_port_a & IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_A_TRACK_0) != 0) /* Accumulator Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_get_fdc_ras_port_a_head_1_select(fdc) (((fdc)->ibmps2_fdc_ras_port_a & IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_A_HEAD_1_SELECT) != 0) /* Zero        Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_get_fdc_ras_port_a_index(fdc) (((fdc)->ibmps2_fdc_ras_port_a & IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_A_INDEX) != 0) /* Overflow    Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_get_fdc_ras_port_a_write_protect(fdc) (((fdc)->ibmps2_fdc_ras_port_a & IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_A_WRITE_PROTECT) != 0) /* Direction   Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_get_fdc_ras_port_a_direction(fdc) (((fdc)->ibmps2_fdc_ras_port_a & IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_A_DIRECTION) != 0) /* Interrupt   Flag (Intel 8086/8088 CPU and higher) */

#define ibmps2_set_fdc_ras_port_a(fdc, v) do { (fdc)->ibmps2_fdc_ras_port_a = (v) & 0xffffffffU; } while (0)      /* Set  FLAGS register for Intel  8086 */
#define ibmps2_set_fdc_port_a(fdc, f, v) \
	do { if (v) (fdc)->ibmps2_fdc_ras_port_a |= (f); else (fdc)->ibmps2_fdc_ras_port_a &= ~(f); } while (0)
#define ibmps2_set_fdc_ras_port_a_irq6(fdc, v) ibmps2_set_fdc_port_a (fdc, IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_A_IRQ8, v) /* Carry       Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_set_fdc_ras_port_a_drq2(fdc, v) ibmps2_set_fdc_port_a (fdc, IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_A_DRQ2, v) /* Parity      Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_set_fdc_ras_port_a_step_latched(fdc, v) ibmps2_set_fdc_port_a (fdc, IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_A_STEP_LATCHED, v) /* Accumulator Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_set_fdc_ras_port_a_track_0(fdc, v) ibmps2_set_fdc_port_a (fdc, IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_A_TRACK_0, v) /* Zero        Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_set_fdc_ras_port_a_head_1_select(fdc, v) ibmps2_set_fdc_port_a (fdc, IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_A_HEAD_1_SELECT, v) /* Overflow    Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_set_fdc_ras_port_a_index(fdc, v) ibmps2_set_fdc_port_a (fdc, IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_A_INDEX, v) /* Sign        Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_set_fdc_ras_port_a_write_protect(fdc, v) ibmps2_set_fdc_port_a (fdc, IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_A_WRITE_PROTECT, v) /* Direction   Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_set_fdc_ras_port_a_direction(fdc, v) ibmps2_set_fdc_port_a (fdc, IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_A_DIRECTION, v) /* Interrupt   Flag (Intel 8086/8088 CPU and higher) */

#define ibmps2_get_fdc_ras_port_b(fdc) ((fdc)->ibmps2_fdc_ras_port_b)  /* Retieve  FLAGS register for Intel 80186 CPU and lower */
#define ibmps2_get_fdc_port_b(fdc, f) (((fdc)->ibmps2_fdc_ras_port_b & (f)) != 0)
#define ibmps2_get_fdc_ras_port_b_reserved(fdc) (((fdc)->ibmps2_fdc_ras_port_b & IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_B_RESERVED) != 0) /* Sign        Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_get_fdc_ras_port_b_drive_select_1(fdc) (((fdc)->ibmps2_fdc_ras_port_b & IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_B_DRIVE_SELECT_1) != 0) /* Carry       Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_get_fdc_ras_port_b_drive_select_0(fdc) (((fdc)->ibmps2_fdc_ras_port_b & IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_B_DRIVE_SELECT_0) != 0) /* Parity      Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_get_fdc_ras_port_b_write_data_latched(fdc) (((fdc)->ibmps2_fdc_ras_port_b & IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_B_WRITE_DATA_LATCHED) != 0) /* Accumulator Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_get_fdc_ras_port_b_read_data_latched(fdc) (((fdc)->ibmps2_fdc_ras_port_b & IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_B_READ_DATA_LATCHED) != 0) /* Zero        Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_get_fdc_ras_port_b_write_enable_latched(fdc) (((fdc)->ibmps2_fdc_ras_port_b & IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_B_WRITE_ENABLE_LATCHED) != 0) /* Overflow    Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_get_fdc_ras_port_b_drive_select_3(fdc) (((fdc)->ibmps2_fdc_ras_port_b & IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_B_DRIVE_SELECT_3) != 0) /* Direction   Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_get_fdc_ras_port_b_drive_select_2(fdc) (((fdc)->ibmps2_fdc_ras_port_b & IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_B_DRIVE_SELECT_2) != 0) /* Interrupt   Flag (Intel 8086/8088 CPU and higher) */

#define ibmps2_set_fdc_ras_port_b(fdc, v) do { (fdc)->ibmps2_fdc_ras_port_b = (v) & 0xffffffffU; } while (0)      /* Set  FLAGS register for Intel  8086 */
#define ibmps2_set_fdc_port_b(fdc, f, v) \
	do { if (v) (fdc)->ibmps2_fdc_ras_port_b |= (f); else (fdc)->ibmps2_fdc_ras_port_b &= ~(f); } while (0)
#define ibmps2_set_fdc_ras_port_b_reserved(fdc, v) ibmps2_set_fdc_port_b (fdc, IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_B_RESERVED, v) /* Carry       Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_set_fdc_ras_port_b_drive_select_1(fdc, v) ibmps2_set_fdc_port_b (fdc, IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_B_DRIVE_SELECT_1, v) /* Parity      Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_set_fdc_ras_port_b_drive_select_0(fdc, v) ibmps2_set_fdc_port_b (fdc, IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_B_DRIVE_SELECT_0, v) /* Accumulator Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_set_fdc_ras_port_b_write_data_latched(fdc, v) ibmps2_set_fdc_port_b (fdc, IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_B_WRITE_DATA_LATCHED, v) /* Zero        Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_set_fdc_ras_port_b_read_data_latched(fdc, v) ibmps2_set_fdc_port_b (fdc, IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_B_READ_DATA_LATCHED, v) /* Overflow    Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_set_fdc_ras_port_b_write_enable_latched(fdc, v) ibmps2_set_fdc_port_b (fdc, IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_B_WRITE_ENABLE_LATCHED, v) /* Sign        Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_set_fdc_ras_port_b_drive_select_3(fdc, v) ibmps2_set_fdc_port_b (fdc, IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_B_DRIVE_SELECT_3, v) /* Direction   Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_set_fdc_ras_port_b_drive_select_2(fdc, v) ibmps2_set_fdc_port_b (fdc, IBMPS2_DISKETTE_DRIVE_INTERFACE_RAS_PORT_B_DRIVE_SELECT_2, v) /* Interrupt   Flag (Intel 8086/8088 CPU and higher) */

#define ibmps2_get_fdc_digital_input(fdc) ((fdc)->ibmps2_fdc_digital_input)  /* Retieve  FLAGS register for Intel 80186 CPU and lower */
#define ibmps2_get_fdc_dir(fdc, f) (((fdc)->ibmps2_fdc_digital_input & (f)) != 0)
#define ibmps2_get_fdc_digital_input_diskette_change(fdc) (((fdc)->ibmps2_fdc_digital_input & IBMPS2_DISKETTE_DRIVE_INTERFACE_DIGITAL_INPUT_DISKETTE_CHANGE) != 0) /* Sign        Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_get_fdc_digital_input_reserved_3(fdc) (((fdc)->ibmps2_fdc_digital_input & IBMPS2_DISKETTE_DRIVE_INTERFACE_DIGITAL_INPUT_RESERVED_3) != 0) /* Carry       Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_get_fdc_digital_input_reserved_2(fdc) (((fdc)->ibmps2_fdc_digital_input & IBMPS2_DISKETTE_DRIVE_INTERFACE_DIGITAL_INPUT_RESERVED_2) != 0) /* Parity      Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_get_fdc_digital_input_reserved_1(fdc) (((fdc)->ibmps2_fdc_digital_input & IBMPS2_DISKETTE_DRIVE_INTERFACE_DIGITAL_INPUT_RESERVED_1) != 0) /* Accumulator Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_get_fdc_digital_input_dma_enable(fdc) (((fdc)->ibmps2_fdc_digital_input & IBMPS2_DISKETTE_DRIVE_INTERFACE_DIGITAL_INPUT_DMA_ENABLE) != 0) /* Zero        Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_get_fdc_digital_input_no_write_precomp(fdc) (((fdc)->ibmps2_fdc_digital_input & IBMPS2_DISKETTE_DRIVE_INTERFACE_DIGITAL_INPUT_NO_WRITE_PRECOMP) != 0) /* Overflow    Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_get_fdc_digital_input_250k_bps_rate_select(fdc) (((fdc)->ibmps2_fdc_digital_input & IBMPS2_DISKETTE_DRIVE_INTERFACE_DIGITAL_INPUT_250K_BPS_RATE_SELECT) != 0) /* Direction   Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_get_fdc_digital_input_reserved_0(fdc) (((fdc)->ibmps2_fdc_digital_input & IBMPS2_DISKETTE_DRIVE_INTERFACE_DIGITAL_INPUT_RESERVED_0) != 0) /* Interrupt   Flag (Intel 8086/8088 CPU and higher) */

#define ibmps2_set_fdc_digital_input(fdc, v) do { (fdc)->ibmps2_fdc_digital_input = (v) & 0xffffffffU; } while (0)      /* Set  FLAGS register for Intel  8086 */
#define ibmps2_set_fdc_dir(fdc, f, v) \
	do { if (v) (fdc)->ibmps2_fdc_digital_input |= (f); else (fdc)->ibmps2_fdc_digital_input &= ~(f); } while (0)
#define ibmps2_set_fdc_digital_input_diskette_change(fdc, v) ibmps2_set_fdc_dir (fdc, IBMPS2_DISKETTE_DRIVE_INTERFACE_DIGITAL_INPUT_DISKETTE_CHANGE, v) /* Carry       Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_set_fdc_digital_input_reserved_3(fdc, v) ibmps2_set_fdc_dir (fdc, IBMPS2_DISKETTE_DRIVE_INTERFACE_DIGITAL_INPUT_RESERVED_3, v) /* Parity      Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_set_fdc_digital_input_reserved_2(fdc, v) ibmps2_set_fdc_dir (fdc, IBMPS2_DISKETTE_DRIVE_INTERFACE_DIGITAL_INPUT_RESERVED_2, v) /* Accumulator Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_set_fdc_digital_input_reserved_1(fdc, v) ibmps2_set_fdc_dir (fdc, IBMPS2_DISKETTE_DRIVE_INTERFACE_DIGITAL_INPUT_RESERVED_1, v) /* Zero        Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_set_fdc_digital_input_dma_enable(fdc, v) ibmps2_set_fdc_dir (fdc, IBMPS2_DISKETTE_DRIVE_INTERFACE_DIGITAL_INPUT_DMA_ENABLE, v) /* Overflow    Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_set_fdc_digital_input_no_write_precomp(fdc, v) ibmps2_set_fdc_dir (fdc, IBMPS2_DISKETTE_DRIVE_INTERFACE_DIGITAL_INPUT_NO_WRITE_PRECOMP, v) /* Sign        Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_set_fdc_digital_input_250k_bps_rate_select(fdc, v) ibmps2_set_fdc_dir (fdc, IBMPS2_DISKETTE_DRIVE_INTERFACE_DIGITAL_INPUT_250K_BPS_RATE_SELECT, v) /* Direction   Flag (Intel 8086/8088 CPU and higher) */
#define ibmps2_set_fdc_digital_input_reserved_0(fdc, v) ibmps2_set_fdc_dir (fdc, IBMPS2_DISKETTE_DRIVE_INTERFACE_DIGITAL_INPUT_RESERVED_0, v) /* Interrupt   Flag (Intel 8086/8088 CPU and higher) */

dev_fdc_t *dev_fdc_new(unsigned long addr);
void dev_fdc_del(dev_fdc_t *fdc);
void dev_fdc_mem_add_io(dev_fdc_t *fdc, memory_t *io);
void dev_fdc_mem_rmv_io(dev_fdc_t *fdc, memory_t *io);
void dev_fdc_reset(dev_fdc_t *fdc);
void dev_fdc_set_disks(dev_fdc_t *fdc, disks_t *dsks);
void dev_fdc_set_drive(dev_fdc_t *fdc, unsigned fdcdrv, unsigned drive);
unsigned dev_fdc_get_drive(dev_fdc_t *fdc, unsigned fdcdrv);
void dev_fdc_ibmpc_at(dev_fdc_t *fdc); /* Used to specify IBM Personal Computer/AT operation */
void dev_fdc_ibmps2(dev_fdc_t *fdc);   /* Used to specify IBM Personal  System /2  operation */
void dev_fdc_ibmps2_irq6(dev_fdc_t *fdc);   /* Used to specify IBM Personal  System /2  operation */
void dev_fdc_ibmps2_drq2(dev_fdc_t *fdc);   /* Used to specify IBM Personal  System /2  operation */
											/* IBM PS/2: RAS Port A Register */

#endif
