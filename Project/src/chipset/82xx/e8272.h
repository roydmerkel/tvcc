/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/chipset/82xx/e8272.h                                     *
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


#ifndef PCE_E8272_H
#define PCE_E8272_H 1
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
#define E8272_ERR_OK       0x00
#define E8272_ERR_OTHER    0x01
#define E8272_ERR_NO_ID    0x02
#define E8272_ERR_NO_DATA  0x04
#define E8272_ERR_CRC_ID   0x08
#define E8272_ERR_CRC_DATA 0x10
#define E8272_ERR_DEL_DAM  0x20
#define E8272_ERR_DATALEN  0x40
#define E8272_ERR_WPROT    0x80

#define E8272_MAX_SCT 128

#define E8272_DISKOP_READ   0
#define E8272_DISKOP_WRITE  1
#define E8272_DISKOP_FORMAT 3
#define E8272_DISKOP_READID 4

typedef struct {
	unsigned char  c;
	unsigned char  h;
	unsigned char  s;
	unsigned short n;
	unsigned short ofs;
} e8272_sct_t;

typedef struct {
	unsigned      d;       /* Disk Drive   */
	unsigned      c;       /* Cylinder     */
	unsigned      h;       /* Head         */
	char          ok;      /* OK           */
	unsigned      sct_cnt; /* Sector Count */
	unsigned      sct_track_half;
	unsigned      sct_track_total;
	unsigned      sct_write_precomp;
	e8272_sct_t   sct[E8272_MAX_SCT];
} e8272_drive_t;

typedef struct {
	unsigned char pd;
	unsigned char pc;
	unsigned char ph;
	unsigned char ps;
	unsigned char lc;
	unsigned char lh;
	unsigned char ls;
	unsigned char ln;
	void          *buf;
	unsigned      cnt;
	unsigned char fill;
	unsigned      diskop_track_half;
	unsigned      diskop_track_total;
	unsigned      diskop_write_precomp;
} e8272_diskop_t;

typedef struct e8272_t {
	unsigned       disk_300_bps;
	unsigned char  dor;
	unsigned char  msr;
	unsigned char  st[4];
	e8272_drive_t  drv[4];
	e8272_drive_t  *curdrv;
	unsigned       cmd_i;
	unsigned       cmd_n;
	unsigned char  cmd[16];
	unsigned       res_i;
	unsigned       res_n;
	unsigned char  res[16];
	unsigned       buf_i;
	unsigned       buf_n;
	unsigned char  buf[8192];
	char           dma;
	unsigned char  ready_change;
	unsigned short step_rate;
	char           accurate;
	char           ignore_eot;
	unsigned long  delay_clock;
	unsigned long  input_clock;
	unsigned long  track_pos;
	unsigned long  track_clk;
	unsigned long  track_size;
	unsigned short index_cnt;
	char           read_error;
	char           read_deleted;
	unsigned short read_track_cnt;
	unsigned short write_id;
	unsigned short format_cnt;
	void           (*set_data) (struct e8272_t *fdc, unsigned char val);
	unsigned char  (*get_data) (struct e8272_t *fdc);
	void           (*set_tc) (struct e8272_t *fdc);
	void           (*set_clock) (struct e8272_t *fdc, unsigned long cnt);
	void           (*start_cmd) (struct e8272_t *fdc);
	void           *diskop_ext;
	unsigned       (*diskop) (void *ext, unsigned op, e8272_diskop_t *p);
	/* the interrupt function */
	void           *irq_ext;
	unsigned char  irq_val;
	void           (*irq) (void *ext, unsigned char val);
	void           *dreq_ext;
	unsigned char  dreq_val;
	void           (*dreq) (void *ext, unsigned char val);
	unsigned long  e8272_track_half;
	unsigned long  e8272_track_total;
	unsigned       e8272_write_precomp;
} e8272_t;
/* ---------------------------------------------------------------------------------------------------------------- */
/* NEC uPD765: Quote from "SINGLE/DOUBLE DENSITY FLOPPY DISK CONTROLLER"; Features:                                 */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Address mark detection circuitry is internal to the FDC which simplifies the phase locked loop and read          */
/* electronics. The track stepping rate, head load time, and head unload time may be programmed by the user. The    */
/* uPD765 offers many additional features such as multiple sector transfers in both read and write with a signel    */
/* command, and full IBM compatibility in both single and double density modes.                                     */
/*  - IBM Compatible in Both Single and Double Density Recording Formats                                            */
/*  - Programmable Data Record Lengths: 128, 256, 512, or 1024 Bytes/Sector                                         */
/*  - Multi-Sector and Multi-Track Transfer Capability                                                              */
/*  - Drive Up to 4 Floppy Disks                                                                                    */
/*  - Data Scan Capability - Will Scan a Single Sector or an Entire Cylinder's Worth of Data Fields, Comparing on a */
/*  Byte by Byte Basis, Data in the Processor's Memory with Data Read from the Diskette                             */
/*  - Data Transfers in DMA or Non-DMA Mode                                                                         */
/*  - Parallel Seek Operations on Up to Four Drives                                                                 */
/*  - Compatible with Most Microprocessors Including 8080A, 8085A, uPD780 (780TM)                                   */
/*  - Single Phase 8 MHz Clock                                                                                      */
/*  - Single +5 Volt Power Supply                                                                                   */
/*  - Available in t0 Pin Plastic Dual-in-Line Package                                                              */
/* ---------------------------------------------------------------------------------------------------------------- */
void e8272_init(e8272_t *fdc);
e8272_t *e8272_new (void);
void e8272_free (e8272_t *fdc);
void e8272_del (e8272_t *fdc);

void e8272_set_250_bps(e8272_t *fdc, void *ext, void *fct); /* 250 Bits Per Second (BPS) */
void e8272_set_300_bps(e8272_t *fdc, void *ext, void *fct); /* 300 Bits Per Second (BPS) */
void e8272_set_500_bps(e8272_t *fdc, void *ext, void *fct); /* 500 Bits Per Second (BPS) */

void e8272_set_irq_fct (e8272_t *fdc, void *ext, void *fct);
void e8272_set_dreq_fct (e8272_t *fdc, void *ext, void *fct);
void e8272_set_diskop_fct (e8272_t *fdc, void *ext, void *fct);

void e8272_set_input_clock (e8272_t *fdc, unsigned long clk);
void e8272_set_accuracy (e8272_t *fdc, int accurate);
void e8272_set_ignore_eot (e8272_t *fdc, int ignore_eot);

void e8272_write_data (e8272_t *fdc, unsigned char val);
unsigned char e8272_read_data (e8272_t *fdc);

unsigned char e8272_get_uint8 (e8272_t *fdc, unsigned long addr);
void e8272_set_uint8 (e8272_t *fdc, unsigned long addr, unsigned char val);

void e8272_reset (e8272_t *fdc);
void e8272_set_tc (e8272_t *fdc, unsigned char val);
void e8272_clock (e8272_t *fdc, unsigned long n);

#endif
