/*****************************************************************************
* pce                                                                       *
*****************************************************************************/

/*****************************************************************************
* File name:   src/devices/fdc.c                                            *
* Created:     2007-09-06 by Hampa Hug <hampa@hampa.ch>                     *
* Copyright:   (C) 2007-2013 Hampa Hug <hampa@hampa.ch>                     *
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
#include <string.h>

#include <devices/fdc.h>

#include <drivers/block/blkpsi.h>


/*
* This is the glue code between the 8272 FDC, block devices and
* the rest of the emulator.
*/


static
struct {
	unsigned v1;
	unsigned v2;
} errmap[] = {
	{ PCE_BLK_PSI_NO_ID,    E8272_ERR_NO_ID },
	{ PCE_BLK_PSI_NO_DATA,  E8272_ERR_NO_DATA },
	{ PCE_BLK_PSI_CRC_ID,   E8272_ERR_CRC_ID },
	{ PCE_BLK_PSI_CRC_DATA, E8272_ERR_CRC_DATA },
	{ PCE_BLK_PSI_DEL_DAM,  E8272_ERR_DEL_DAM },
	{ PCE_BLK_PSI_DATALEN,  E8272_ERR_DATALEN },
	{ PCE_BLK_PSI_WPROT,    E8272_ERR_WPROT },
	{ 0, 0 }
};

static void dev_fdc_del_dev(device_t *dev);

/*
* Map an error code from PCE_BLK_PSI_* to E8272_ERR_*.
*/
static
unsigned dev_fdc_map_error(unsigned err)
{
	unsigned i;
	unsigned ret;
	ret = 0;
	i = 0;
	while (errmap[i].v1 != 0) {
		if (err & errmap[i].v1) {
			err &= ~errmap[i].v1;
			ret |= errmap[i].v2;
		}
		i += 1;
	}
	if (err) {
		ret |= E8272_ERR_OTHER;
	}
	return (ret);
}

static
unsigned dev_fdc_diskop_read(dev_fdc_t *fdc, e8272_diskop_t *p, e8272_drive_t *drv, e8272_t *reg)
{
	unsigned ret, err, write_protect;
	disk_t   *dsk;
	/* dsk->dsk_500_kb_ibmpc = 0x1;
	dsk->dsk_640_kb_ibmpc = 0x1; */
	if (fdc->ibmps2 == 0x1)
	{
		/* IBM Personal System/2 Model 25/30 */
		ibmps2_set_fdc_digital_input_no_write_precomp(fdc, 0x1); /* Write Precompensation signal disabled by default        */
		unsigned long track_half, track_pos;                     /* Compare  first half of tracks to current track position */
		dsk = dsks_get_disk(fdc->dsks, fdc->drive[p->pd & 3]);   /* Retrieve disk from disk drive                           */
		dsk->dsk_500_kb_ibmpc = 0x1;                             /* Force IBM PC disk format by default for 500 KB disks    */
		dsk->dsk_640_kb_ibmpc = 0x1;                             /* Force IBM PC disk format by default for 640 KB disks    */
		track_half = dsk->track_half;                            /* Retrieve first half of tracks                           */
		track_pos = reg->track_pos;                              /* Retrieve current track position                         */
		reg->e8272_track_half = track_half;                      /* Send     first half of tracks to floppy disk controller */
		if (track_pos > track_half)
		{
			/* Write Precompensation */
			ibmps2_set_fdc_digital_input_no_write_precomp(fdc, 0x0);
		}
		/* -------- */
		ibmps2_set_fdc_ras_port_a_track_0(fdc, 0x0);
		ibmps2_set_fdc_ras_port_a_head_1_select(fdc, 0x0);
		ibmps2_set_fdc_ras_port_a_direction(fdc, 0x1);
		ibmps2_set_fdc_ras_port_b_read_data_latched(fdc, 0x1);
		ibmps2_set_fdc_ras_port_b_write_data_latched(fdc, 0x0);
		unsigned index, dma_enable;
		index = reg->index_cnt;
		dma_enable = reg->dma;
		fdc->dma_enable = dma_enable;
		ibmps2_set_fdc_ras_port_a_index(fdc, index);
		ibmps2_set_fdc_digital_input_dma_enable(fdc, dma_enable);
		if (drv->c = 0x0)
		{
			/* Track 0 */
			ibmps2_set_fdc_ras_port_a_track_0(fdc, 0x1);
			ibmps2_set_fdc_ras_port_a_direction(fdc, 0x0);
		}
		if (drv->h = 0x1)
		{
			/*  Head 1 Select */
			ibmps2_set_fdc_ras_port_a_head_1_select(fdc, 0x1);
		}
		if (reg->index_cnt >= 1) {
			/* Index */
			unsigned cnt;
			ibmps2_set_fdc_ras_port_a_index(fdc, cnt);
		}
		if (dsk == NULL) {
			p->cnt = 0;
			return (E8272_ERR_NO_DATA);
		}
		if (dsk->dsk_bps > 0x00300000)
		{
			/* 500 BPS */

			e8272_set_500_bps;
			ibmps2_set_fdc_digital_input_250k_bps_rate_select(fdc, 0x0); /* Placeholder only; 300 BPS selected */
		}
		else if (dsk->dsk_bps == 0x00300000)
		{
			/* 300 BPS */
			e8272_set_300_bps;
			ibmps2_set_fdc_digital_input_250k_bps_rate_select(fdc, 0x0);
		}
		else
		{
			/* 250 BPS */
			e8272_set_250_bps;
			ibmps2_set_fdc_digital_input_250k_bps_rate_select(fdc, 0x1);
		}
		if (dsk_get_type(dsk) != PCE_DISK_PSI) {
			if (p->cnt < 512) {
				p->cnt = 0;
				return (E8272_ERR_OTHER);
			}
			if (dsk_read_chs(dsk, p->buf, p->pc, p->ph, p->ls, 1)) {
				p->cnt = 0;
				return (E8272_ERR_NO_DATA);
			}
			p->cnt = 512;
			return (0);
		}
		err = dsk_psi_read_chs(dsk->ext, p->buf, &p->cnt, p->pc, p->ph, p->ps, 1);
		ret = dev_fdc_map_error(err);
		return (ret);
	}
	else
	{
		/* */
		dsk = dsks_get_disk(fdc->dsks, fdc->drive[p->pd & 3]);
		if (dsk == NULL) {
			p->cnt = 0;
			return (E8272_ERR_NO_DATA);
		}
		if (dsk_get_type(dsk) != PCE_DISK_PSI) {
			if (p->cnt < 512) {
				p->cnt = 0;
				return (E8272_ERR_OTHER);
			}
			if (dsk_read_chs(dsk, p->buf, p->pc, p->ph, p->ls, 1)) {
				p->cnt = 0;
				return (E8272_ERR_NO_DATA);
			}
			p->cnt = 512;
			return (0);
		}
		err = dsk_psi_read_chs(dsk->ext, p->buf, &p->cnt, p->pc, p->ph, p->ps, 1);
		ret = dev_fdc_map_error(err);
		return (ret);
	}
}

static
unsigned dev_fdc_diskop_write(dev_fdc_t *fdc, e8272_diskop_t *p, e8272_t *reg)
{
	unsigned err, ret;
	disk_t   *dsk;
	/* dsk->dsk_500_kb_ibmpc = 0x1;
	dsk->dsk_640_kb_ibmpc = 0x1; */
	if (fdc->ibmps2 == 0x1)
	{
		/* IBM Personal System/2 Model 25/30 */
		ibmps2_set_fdc_digital_input_no_write_precomp(fdc, 0x1); /* Write Precompensation signal disabled by default        */
		unsigned long track_half, track_pos;                     /* Compare  first half of tracks to current track position */
		dsk = dsks_get_disk(fdc->dsks, fdc->drive[p->pd & 3]);   /* Retrieve disk from disk drive                           */
		dsk->dsk_500_kb_ibmpc = 0x1;                             /* Force IBM PC disk format by default for 500 KB disks    */
		dsk->dsk_640_kb_ibmpc = 0x1;                             /* Force IBM PC disk format by default for 640 KB disks    */
		track_half = dsk->track_half;                            /* Retrieve first half of tracks                           */
		track_pos = reg->track_pos;                              /* Retrieve current track position                         */
		reg->e8272_track_half = track_half;                      /* Send     first half of tracks to floppy disk controller */
		if (track_pos > track_half)
		{
			/* Write Precompensation */
			ibmps2_set_fdc_digital_input_no_write_precomp(fdc, 0x0);
		}
		/* -------- */
		ibmps2_set_fdc_ras_port_a_write_protect(fdc, 0x0);
		ibmps2_set_fdc_ras_port_b_read_data_latched(fdc, 0x0);
		ibmps2_set_fdc_ras_port_b_write_data_latched(fdc, 0x1);
		ibmps2_set_fdc_ras_port_b_write_enable_latched(fdc, 0x1);
		unsigned index, dma_enable;
		index = reg->index_cnt;
		dma_enable = reg->dma;
		fdc->dma_enable = dma_enable;
		ibmps2_set_fdc_ras_port_a_index(fdc, index);
		ibmps2_set_fdc_digital_input_dma_enable(fdc, dma_enable);
		if (dsk == NULL) {
			p->cnt = 0;
			return (E8272_ERR_NO_DATA);
		}
		if (dsk->readonly == 1) {
			/* Disk is write-protected */
			ibmps2_set_fdc_ras_port_a_write_protect(fdc, 0x1);
			ibmps2_set_fdc_ras_port_b_write_data_latched(fdc, 0x0);
			ibmps2_set_fdc_ras_port_b_write_enable_latched(fdc, 0x0);
		}
		if (dsk_get_type(dsk) != PCE_DISK_PSI) {
			if (p->cnt != 512) {
				return (E8272_ERR_OTHER);
			}
			if (dsk_write_chs(dsk, p->buf, p->pc, p->ph, p->ls, 1)) {
				p->cnt = 0;
				return (E8272_ERR_OTHER);
			}
			return (0);
		}
		err = dsk_psi_write_chs(dsk->ext, p->buf, &p->cnt, p->pc, p->ph, p->ps, 1);
		ret = dev_fdc_map_error(err);
		return (ret);
	}
	else
	{
		/* */
		dsk = dsks_get_disk(fdc->dsks, fdc->drive[p->pd & 3]);
		if (dsk == NULL) {
			p->cnt = 0;
			return (E8272_ERR_NO_DATA);
		}
		if (dsk_get_type(dsk) != PCE_DISK_PSI) {
			if (p->cnt != 512) {
				return (E8272_ERR_OTHER);
			}
			if (dsk_write_chs(dsk, p->buf, p->pc, p->ph, p->ls, 1)) {
				p->cnt = 0;
				return (E8272_ERR_OTHER);
			}
			return (0);
		}
		err = dsk_psi_write_chs(dsk->ext, p->buf, &p->cnt, p->pc, p->ph, p->ps, 1);
		ret = dev_fdc_map_error(err);
		return (ret);
	}
}

static
unsigned dev_fdc_diskop_format(dev_fdc_t *fdc, e8272_diskop_t *p, e8272_drive_t *drv, e8272_t *reg)
{
	unsigned      cnt;
	disk_t        *dsk;
	unsigned char buf[512];
	/* dsk->dsk_500_kb_ibmpc = 0x1;
	dsk->dsk_640_kb_ibmpc = 0x1; */
	if (fdc->ibmps2 == 0x1)
	{
		/* IBM Personal System/2 Model 25/30 */
		ibmps2_set_fdc_digital_input_no_write_precomp(fdc, 0x1); /* Write Precompensation signal disabled by default        */
		unsigned long track_half, track_pos;                     /* Compare  first half of tracks to current track position */
		dsk = dsks_get_disk(fdc->dsks, fdc->drive[p->pd & 3]);   /* Retrieve disk from disk drive                           */
		dsk->dsk_500_kb_ibmpc = 0x1;                             /* Force IBM PC disk format by default for 500 KB disks    */
		dsk->dsk_640_kb_ibmpc = 0x1;                             /* Force IBM PC disk format by default for 640 KB disks    */
		track_half = dsk->track_half;                            /* Retrieve first half of tracks                           */
		track_pos = reg->track_pos;                              /* Retrieve current track position                         */
		reg->e8272_track_half = track_half;                      /* Send     first half of tracks to floppy disk controller */
		if (track_pos > track_half)
		{
			/* Write Precompensation */
			ibmps2_set_fdc_digital_input_no_write_precomp(fdc, 0x0);
		}
		/* -------- */
		ibmps2_set_fdc_ras_port_a_track_0(fdc, 0x0);
		ibmps2_set_fdc_ras_port_a_direction(fdc, 0x1);
		unsigned index, dma_enable;
		index = reg->index_cnt;
		dma_enable = reg->dma;
		fdc->dma_enable = dma_enable;
		ibmps2_set_fdc_ras_port_a_index(fdc, index);
		ibmps2_set_fdc_digital_input_dma_enable(fdc, dma_enable);
		if (drv->c = 0x0)
		{
			/* Track 0 */
			ibmps2_set_fdc_ras_port_a_track_0(fdc, 0x1);
			ibmps2_set_fdc_ras_port_a_direction(fdc, 0x0);
		}
		if (drv->h = 0x1)
		{
			/*  Head 1 Select */
			ibmps2_set_fdc_ras_port_a_head_1_select(fdc, 0x1);
		}
		if (dsk == NULL) {
			return (E8272_ERR_NO_DATA);
		}
		cnt = 128 << p->ln;
		if (dsk_get_type(dsk) != PCE_DISK_PSI) {
			if (cnt != 512) {
				return (E8272_ERR_OTHER);
			}
			memset(buf, p->fill, 512);
			if (dsk_write_chs(dsk, buf, p->pc, p->ph, p->ps + 1, 1)) {
				return (E8272_ERR_OTHER);
			}
			return (0);
		}
		if (p->ps == 0) {
			dsk_psi_erase_track(dsk->ext, p->pc, p->ph);
		}
		if (dsk_psi_format_sector(dsk->ext, p->pc, p->ph, p->lc, p->lh, p->ls, cnt, p->fill)) {
			return (E8272_ERR_OTHER);
		}
		return (0);
	}
	else
	{
		/* */
		dsk = dsks_get_disk(fdc->dsks, fdc->drive[p->pd & 3]);
		if (dsk == NULL) {
			return (E8272_ERR_NO_DATA);
		}
		cnt = 128 << p->ln;
		if (dsk_get_type(dsk) != PCE_DISK_PSI) {
			if (cnt != 512) {
				return (E8272_ERR_OTHER);
			}
			memset(buf, p->fill, 512);
			if (dsk_write_chs(dsk, buf, p->pc, p->ph, p->ps + 1, 1)) {
				return (E8272_ERR_OTHER);
			}
			return (0);
		}
		if (p->ps == 0) {
			dsk_psi_erase_track(dsk->ext, p->pc, p->ph);
		}
		if (dsk_psi_format_sector(dsk->ext, p->pc, p->ph, p->lc, p->lh, p->ls, cnt, p->fill)) {
			return (E8272_ERR_OTHER);
		}
		return (0);
	}
}

static
unsigned dev_fdc_diskop_readid(dev_fdc_t *fdc, e8272_diskop_t *p, e8272_t *reg)
{
	disk_t        *dsk;
	unsigned char buf[512];
	unsigned      lc, lh, ls, cnt, cnt_id;
	/* dsk->dsk_500_kb_ibmpc = 0x1;
	dsk->dsk_640_kb_ibmpc = 0x1; */
	if (fdc->ibmps2 == 0x1)
	{
		/* IBM Personal System/2 Model 25/30 */
		ibmps2_set_fdc_digital_input_no_write_precomp(fdc, 0x1); /* Write Precompensation signal disabled by default        */
		unsigned long track_half, track_pos;                     /* Compare  first half of tracks to current track position */
		dsk = dsks_get_disk(fdc->dsks, fdc->drive[p->pd & 3]);   /* Retrieve disk from disk drive                           */
		dsk->dsk_500_kb_ibmpc = 0x1;                             /* Force IBM PC disk format by default for 500 KB disks    */
		dsk->dsk_640_kb_ibmpc = 0x1;                             /* Force IBM PC disk format by default for 640 KB disks    */
		track_half = dsk->track_half;                            /* Retrieve first half of tracks                           */
		track_pos = reg->track_pos;                              /* Retrieve current track position                         */
		reg->e8272_track_half = track_half;                      /* Send     first half of tracks to floppy disk controller */
		if (track_pos > track_half)
		{
			/* Write Precompensation */
			ibmps2_set_fdc_digital_input_no_write_precomp(fdc, 0x0);
		}
		/* -------- */
		unsigned index, dma_enable;
		index = reg->index_cnt;
		dma_enable = reg->dma;
		fdc->dma_enable = dma_enable;
		ibmps2_set_fdc_ras_port_a_index(fdc, index);
		ibmps2_set_fdc_digital_input_dma_enable(fdc, dma_enable);
		if (dsk == NULL) {
			return (E8272_ERR_NO_ID);
		}
		if (dsk_get_type(dsk) != PCE_DISK_PSI) {
			if (dsk_read_chs(dsk, buf, p->pc, p->ph, p->ps + 1, 1)) {
				return (E8272_ERR_NO_ID);
			}
			p->lc = p->pc;
			p->lh = p->ph;
			p->ls = p->ps + 1;
			p->ln = 2;
			return (0);
		}
		if (dsk_psi_read_id(dsk->ext, p->pc, p->ph, p->ps, &lc, &lh, &ls, &cnt, &cnt_id)) {
			return (E8272_ERR_NO_ID);
		}
		p->lc = lc;
		p->lh = lh;
		p->ls = ls;
		p->ln = 0;
		while (cnt_id > 128) {
			cnt_id >>= 1;
			p->ln += 1;
		}
		return (0);
	}
	else
	{
		/* */
		dsk = dsks_get_disk(fdc->dsks, fdc->drive[p->pd & 3]);
		if (dsk == NULL) {
			return (E8272_ERR_NO_ID);
		}
		if (dsk_get_type(dsk) != PCE_DISK_PSI) {
			if (dsk_read_chs(dsk, buf, p->pc, p->ph, p->ps + 1, 1)) {
				return (E8272_ERR_NO_ID);
			}
			p->lc = p->pc;
			p->lh = p->ph;
			p->ls = p->ps + 1;
			p->ln = 2;
			return (0);
		}
		if (dsk_psi_read_id(dsk->ext, p->pc, p->ph, p->ps, &lc, &lh, &ls, &cnt, &cnt_id)) {
			return (E8272_ERR_NO_ID);
		}
		p->lc = lc;
		p->lh = lh;
		p->ls = ls;
		p->ln = 0;
		while (cnt_id > 128) {
			cnt_id >>= 1;
			p->ln += 1;
		}
		return (0);
	}
}

static
unsigned dev_fdc_diskop(dev_fdc_t *fdc, unsigned op, e8272_diskop_t *p, e8272_drive_t *drv, e8272_t *reg)
{
	switch (op) {
	case E8272_DISKOP_READ:
		if (fdc->ibmps2 == 0x1)
		{
			/* IBM Personal System/2 Model 25/30 */
			return (dev_fdc_diskop_read(fdc, p, drv, reg));
		}
		else
		{
			/* */
			return (dev_fdc_diskop_read(fdc, p, drv, reg));
		}
	case E8272_DISKOP_WRITE:
		if (fdc->ibmps2 == 0x1)
		{
			/* IBM Personal System/2 Model 25/30 */
			return (dev_fdc_diskop_write(fdc, p, reg));
		}
		else
		{
			/* */
			return (dev_fdc_diskop_write(fdc, p, reg));
		}
	case E8272_DISKOP_FORMAT:
		if (fdc->ibmps2 == 0x1)
		{
			/* IBM Personal System/2 Model 25/30 */
			return (dev_fdc_diskop_format(fdc, p, drv, reg));
		}
		else
		{
			/* */
			return (dev_fdc_diskop_format(fdc, p, drv, reg));
		}
	case E8272_DISKOP_READID:
		if (fdc->ibmps2 == 0x1)
		{
			/* IBM Personal System/2 Model 25/30 */
			return (dev_fdc_diskop_readid(fdc, p, reg));
		}
		else
		{
			/* */
			return (dev_fdc_diskop_readid(fdc, p, reg));
		}
	}
	return (E8272_ERR_OTHER);
}

static
void dev_fdc_clock(dev_fdc_t *fdc, unsigned n)
{
	if (fdc->ibmps2 == 0x1)
	{
		/* IBM Personal System/2 Model 25/30 */
		e8272_clock(&fdc->e8272, n);
	}
	else
	{
		/* */
		e8272_clock(&fdc->e8272, n);
	}
}

dev_fdc_t *dev_fdc_new(unsigned long addr)
{
	unsigned  i;
	dev_fdc_t *fdc;
	fdc = malloc(sizeof(dev_fdc_t));
	if (fdc == NULL) {
		return (NULL);
	}
	dev_init(&fdc->dev, fdc, "fdc");
	fdc->dev.del = dev_fdc_del_dev;
	fdc->dev.clock = (void *)dev_fdc_clock;
	e8272_init(&fdc->e8272);
	e8272_set_diskop_fct(&fdc->e8272, fdc, dev_fdc_diskop);
	mem_blk_init(&fdc->blk, addr, 8, 0);
	mem_blk_set_fct(&fdc->blk, &fdc->e8272,
		e8272_get_uint8, NULL, NULL,
		e8272_set_uint8, NULL, NULL
	);
	for (i = 0; i < 4; i++) {
		fdc->drive[i] = 0xffff;
	}
	return (fdc);
}

void dev_fdc_del(dev_fdc_t *fdc)
{
	if (fdc != NULL) {
		mem_blk_free(&fdc->blk);
		e8272_free(&fdc->e8272);
		free(fdc);
	}
}

static
void dev_fdc_del_dev(device_t *dev)
{
	dev_fdc_del(dev->ext);
}

void dev_fdc_mem_add_io(dev_fdc_t *fdc, memory_t *io)
{
	mem_add_blk(io, &fdc->blk, 0);
}

void dev_fdc_mem_rmv_io(dev_fdc_t *fdc, memory_t *io)
{
	mem_rmv_blk(io, &fdc->blk);
}


void dev_fdc_reset(dev_fdc_t *fdc)
{
	if (fdc->ibmps2 == 0x1)
	{
		/* IBM Personal System/2 Model 25/30 */
		e8272_reset(&fdc->e8272);
	}
	else
	{
		/* */
		e8272_reset(&fdc->e8272);
	}
}

void dev_fdc_set_disks(dev_fdc_t *fdc, disks_t *dsks)
{
	if (fdc->ibmps2 == 0x1)
	{
		/* IBM Personal System/2 Model 25/30 */
		fdc->dsks = dsks;
	}
	else
	{
		/* */
		fdc->dsks = dsks;
	}
}

void dev_fdc_set_drive(dev_fdc_t *fdc, unsigned fdcdrv, unsigned drive)
{
	if (fdc->ibmps2 == 0x1)
	{
		/* IBM Personal System/2 Model 25/30 */
		if (fdcdrv < 4) {
			if (fdcdrv == 0x03)
			{
				/* RAS Port B, Hex 3F1: Bit 1: -Drive Select 3 */
				fdc->drive[fdcdrv] = drive;
				ibmps2_set_fdc_ras_port_b_drive_select_1(fdc, 0x0);
				ibmps2_set_fdc_ras_port_b_drive_select_0(fdc, 0x0);
				ibmps2_set_fdc_ras_port_b_drive_select_3(fdc, 0x1);
				ibmps2_set_fdc_ras_port_b_drive_select_2(fdc, 0x0);
			}
			else if (fdcdrv == 0x02)
			{
				/* RAS Port B, Hex 3F1: Bit 0: -Drive Select 2 */
				fdc->drive[fdcdrv] = drive;
				ibmps2_set_fdc_ras_port_b_drive_select_1(fdc, 0x0);
				ibmps2_set_fdc_ras_port_b_drive_select_0(fdc, 0x0);
				ibmps2_set_fdc_ras_port_b_drive_select_3(fdc, 0x0);
				ibmps2_set_fdc_ras_port_b_drive_select_2(fdc, 0x1);
			}
			else if (fdcdrv == 0x01)
			{
				/* RAS Port B, Hex 3F1: Bit 6: -Drive Select 1 */
				fdc->drive[fdcdrv] = drive;
				ibmps2_set_fdc_ras_port_b_drive_select_1(fdc, 0x1);
				ibmps2_set_fdc_ras_port_b_drive_select_0(fdc, 0x0);
				ibmps2_set_fdc_ras_port_b_drive_select_3(fdc, 0x0);
				ibmps2_set_fdc_ras_port_b_drive_select_2(fdc, 0x0);
			}
			else
			{
				/* RAS Port B, Hex 3F1: Bit 5: -Drive Select 0 */
				fdc->drive[fdcdrv] = drive;
				ibmps2_set_fdc_ras_port_b_drive_select_1(fdc, 0x0);
				ibmps2_set_fdc_ras_port_b_drive_select_0(fdc, 0x1);
				ibmps2_set_fdc_ras_port_b_drive_select_3(fdc, 0x0);
				ibmps2_set_fdc_ras_port_b_drive_select_2(fdc, 0x0);
			}
		}
	}
	else
	{
		/* */
		if (fdcdrv < 4) {
			fdc->drive[fdcdrv] = drive;
		}
	}
}

unsigned dev_fdc_get_drive(dev_fdc_t *fdc, unsigned fdcdrv)
{
	if (fdc->ibmps2 == 0x1)
	{
		/* IBM Personal System/2 Model 25/30 */
		if (fdcdrv < 4) {
			if (fdcdrv == 0x03)
			{
				/* RAS Port B, Hex 3F1: Bit 1: -Drive Select 3 */
				return (fdc->drive[fdcdrv]);
			}
			else if (fdcdrv == 0x02)
			{
				/* RAS Port B, Hex 3F1: Bit 0: -Drive Select 2 */
				return (fdc->drive[fdcdrv]);
			}
			else if (fdcdrv == 0x01)
			{
				/* RAS Port B, Hex 3F1: Bit 6: -Drive Select 1 */
				return (fdc->drive[fdcdrv]);
			}
			else
			{
				/* RAS Port B, Hex 3F1: Bit 5: -Drive Select 0 */
				return (fdc->drive[fdcdrv]);
			}
		}
		return (0xffff);
	}
	else
	{
		/* */
		if (fdcdrv < 4) {
			return (fdc->drive[fdcdrv]);
		}
		return (0xffff);
	}
}

void dev_fdc_ibmpc_at(dev_fdc_t *fdc)
{
	/* Used to specify IBM Personal Computer/AT operation */
	fdc->ibmpc_at = 0x1;
}

void dev_fdc_ibmps2(dev_fdc_t *fdc)
{
	/* Used to specify IBM Personal  System /2  operation */
	fdc->ibmps2 = 0x1;
}

void dev_fdc_ibmps2_irq6(dev_fdc_t *fdc)
{
	/* Used to specify IBM Personal  System /2  operation */
	ibmps2_set_fdc_ras_port_a_irq6(fdc, 0x1);
}

void dev_fdc_ibmps2_drq2(dev_fdc_t *fdc)
{
	/* Used to specify IBM Personal  System /2  operation */
	ibmps2_set_fdc_ras_port_a_drq2(fdc, 0x1);
}
