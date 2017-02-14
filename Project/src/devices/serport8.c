/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/serport.c                                        *
 * Created:     2003-09-04 by Hampa Hug <hampa@hampa.ch>                     *
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


#include <config.h>

#include <stdio.h>
#include <stdlib.h>

#include "serport8.h"


static void ser_uart_check_setup_8mhz(serport_8mhz_t *ser);
static void ser_uart_check_status_8mhz(serport_8mhz_t *src);
static void ser_uart_check_out_8mhz(serport_8mhz_t *ser, unsigned char val);
static void ser_uart_check_inp_8mhz(serport_8mhz_t *ser, unsigned char val);


static
unsigned char ser_get_uint8_8mhz(serport_8mhz_t *ser, unsigned long addr)
{
	return (e8250_get_uint8 (&ser->uart, addr >> ser->addr_shift_8mhz));
}

static
unsigned short ser_get_uint16_8mhz(serport_8mhz_t *ser, unsigned long addr)
{
	return (e8250_get_uint8 (&ser->uart, addr >> ser->addr_shift_8mhz));
}

static
unsigned long ser_get_uint32_8mhz(serport_8mhz_t *ser, unsigned long addr)
{
	return (e8250_get_uint8 (&ser->uart, addr >> ser->addr_shift_8mhz));
}

static
void ser_set_uint8_8mhz(serport_8mhz_t *ser, unsigned long addr, unsigned char val)
{
	e8250_set_uint8 (&ser->uart, addr >> ser->addr_shift_8mhz, val);
}

static
void ser_set_uint16_8mhz(serport_8mhz_t *ser, unsigned long addr, unsigned short val)
{
	e8250_set_uint8 (&ser->uart, addr >> ser->addr_shift_8mhz, val & 0xff);
}

static
void ser_set_uint32_8mhz(serport_8mhz_t *ser, unsigned long addr, unsigned long val)
{
	e8250_set_uint8 (&ser->uart, addr >> ser->addr_shift_8mhz, val & 0xff);
}

void ser_init_8mhz(serport_8mhz_t *ser, unsigned long base, unsigned shift)
{
	ser->io_8mhz = base;

	ser->addr_shift_8mhz = shift;

	e8250_init (&ser->uart);
	e8250_set_send_fct (&ser->uart, ser, ser_uart_check_out_8mhz);
	e8250_set_recv_fct (&ser->uart, ser, ser_uart_check_inp_8mhz);
	e8250_set_setup_fct (&ser->uart, ser, ser_uart_check_setup_8mhz);
	e8250_set_check_fct (&ser->uart, ser, ser_uart_check_status_8mhz);

	e8250_set_multichar (&ser->uart, 1, 1);
	e8250_set_bit_clk_div (&ser->uart, 10);

	mem_blk_init (&ser->port, base, 8 << shift, 0);
	ser->port.ext = ser;
	ser->port.get_uint8 = (void *) ser_get_uint8_8mhz;
	ser->port.set_uint8 = (void *) ser_set_uint8_8mhz;
	ser->port.get_uint16 = (void *) ser_get_uint16_8mhz;
	ser->port.set_uint16 = (void *) ser_set_uint16_8mhz;
	ser->port.get_uint32 = (void *) ser_get_uint32_8mhz;
	ser->port.set_uint32 = (void *) ser_set_uint32_8mhz;

	ser->bps_8mhz = 2400;
	ser->databits_8mhz = 8;
	ser->stopbits_8mhz = 1;
	ser->parity_8mhz = E8250_PARITY_N;

	ser->dtr_8mhz = 0;
	ser->rts_8mhz = 0;

	ser->inp_idx_8mhz = 0;
	ser->inp_cnt_8mhz = 0;

	ser->out_idx_8mhz = 0;
	ser->out_cnt_8mhz = 0;

	ser->check_out_8mhz = 0;
	ser->check_inp_8mhz = 1;

	ser->cdrv = NULL;

	/* ready to receive data */
	e8250_set_dsr (&ser->uart, 1);
	e8250_set_cts (&ser->uart, 1);
}

serport_8mhz_t *ser_new_8mhz(unsigned long base, unsigned shift)
{
	serport_8mhz_t *ser;

	ser = malloc (sizeof (serport_8mhz_t));
	if (ser == NULL) {
		return (NULL);
	}

	ser_init_8mhz(ser, base, shift);

	return (ser);
}

void ser_free_8mhz(serport_8mhz_t *ser)
{
	e8250_free (&ser->uart);
	mem_blk_free (&ser->port);

	if (ser->cdrv != NULL) {
		chr_close (ser->cdrv);
	}
}

void ser_del_8mhz(serport_8mhz_t *ser)
{
	if (ser != NULL) {
		ser_free_8mhz(ser);
		free (ser);
	}
}

mem_blk_t *ser_get_reg_8mhz(serport_8mhz_t *ser)
{
	return (&ser->port);
}

e8250_t *ser_get_uart_8mhz(serport_8mhz_t *ser)
{
	return (&ser->uart);
}

int ser_set_driver_8mhz(serport_8mhz_t *ser, const char *name)
{
	if (ser->cdrv != NULL) {
		chr_close (ser->cdrv);
	}

	ser->cdrv = chr_open (name);

	if (ser->cdrv == NULL) {
		return (1);
	}

	return (0);
}

int ser_set_log_8mhz(serport_8mhz_t *ser, const char *fname)
{
	if (ser->cdrv == NULL) {
		return (1);
	}

	return (chr_set_log (ser->cdrv, fname));
}

void ser_reset_8mhz(serport_8mhz_t *ser)
{
	e8250_reset (&ser->uart);

	ser->bps_8mhz = 2400;
	ser->databits_8mhz = 8;
	ser->stopbits_8mhz = 1;
	ser->parity_8mhz = E8250_PARITY_N;

	ser->dtr_8mhz = 0;
	ser->rts_8mhz = 0;

	ser->inp_idx_8mhz = 0;
	ser->inp_cnt_8mhz = 0;

	ser->out_idx_8mhz = 0;
	ser->out_cnt_8mhz = 0;

	ser->check_out_8mhz = 0;
	ser->check_inp_8mhz = 1;

	e8250_set_dsr (&ser->uart, 1);
	e8250_set_cts (&ser->uart, 1);
}

static
void ser_flush_output_8mhz(serport_8mhz_t *ser)
{
	unsigned cnt;

	if (ser->out_cnt_8mhz == 0) {
		return;
	}

	cnt = chr_write (ser->cdrv, ser->out_buf_8mhz + ser->out_idx_8mhz, ser->out_cnt_8mhz);

	ser->out_idx_8mhz += cnt;
	ser->out_cnt_8mhz -= cnt;

	if (ser->out_cnt_8mhz == 0) {
		ser->out_idx_8mhz = 0;
	}
}

static
void ser_process_output_8mhz(serport_8mhz_t *ser)
{
	ser_flush_output_8mhz(ser);

	if (ser->out_cnt_8mhz > 0) {
		return;
	}

	while (1) {
		if (e8250_get_out (&ser->uart, ser->out_buf_8mhz + ser->out_idx_8mhz + ser->out_cnt_8mhz, 1)) {
			ser_flush_output_8mhz(ser);
			break;
		}

		ser->out_cnt_8mhz += 1;

		if ((ser->out_idx_8mhz + ser->out_cnt_8mhz) >= PCE_SERPORT_8MHZ_BUF) {
			ser_flush_output_8mhz(ser);

			if (ser->out_cnt_8mhz > 0) {
				break;
			}
		}
	}

	ser->check_out_8mhz = ((e8250_out_empty (&ser->uart) == 0) || (ser->out_cnt_8mhz > 0));
}

static
void ser_fill_input_8mhz(serport_8mhz_t *ser)
{
	unsigned idx, cnt;

	if (ser->inp_cnt_8mhz == 0) {
		ser->inp_idx_8mhz = 0;
	}

	idx = ser->inp_idx_8mhz + ser->inp_cnt_8mhz;
	cnt = PCE_SERPORT_8MHZ_BUF - idx;

	if (cnt == 0) {
		return;
	}

	cnt = chr_read (ser->cdrv, ser->inp_buf_8mhz + idx, cnt);

	ser->inp_cnt_8mhz += cnt;
}

static
void ser_process_input_8mhz(serport_8mhz_t *ser)
{
	while (e8250_inp_full (&ser->uart) == 0) {
		if (ser->inp_cnt_8mhz == 0) {
			ser_fill_input_8mhz(ser);

		}

		if (ser->inp_cnt_8mhz == 0) {
			break;
		}

		e8250_receive (&ser->uart, ser->inp_buf_8mhz[ser->inp_idx_8mhz]);

		ser->inp_idx_8mhz += 1;
		ser->inp_cnt_8mhz -= 1;
	}

	ser->check_inp_8mhz = (e8250_inp_full (&ser->uart) == 0);
}

static
void ser_uart_check_setup_8mhz(serport_8mhz_t *ser)
{
	unsigned val;

	ser->bps_8mhz = e8250_get_bps (&ser->uart);
	ser->databits_8mhz = e8250_get_databits (&ser->uart);
	ser->stopbits_8mhz = e8250_get_stopbits (&ser->uart);
	ser->parity_8mhz = e8250_get_parity (&ser->uart);

	chr_set_params (ser->cdrv, ser->bps_8mhz, ser->databits_8mhz, ser->parity_8mhz, ser->stopbits_8mhz);

	ser->dtr_8mhz = e8250_get_dtr (&ser->uart);
	ser->rts_8mhz = e8250_get_rts (&ser->uart);

	val = 0;
	val |= ser->dtr_8mhz ? PCE_CHAR_DTR : 0;
	val |= ser->rts_8mhz ? PCE_CHAR_RTS : 0;

	chr_set_ctl (ser->cdrv, val);
}

static
void ser_uart_check_status_8mhz(serport_8mhz_t *ser)
{
	unsigned val;

	if (chr_get_ctl (ser->cdrv, &val)) {
		val = PCE_CHAR_DSR | PCE_CHAR_CTS | PCE_CHAR_CD;
	}

	e8250_set_dsr (&ser->uart, (val & PCE_CHAR_DSR) != 0);
	e8250_set_cts (&ser->uart, (val & PCE_CHAR_CTS) != 0);
	e8250_set_dcd (&ser->uart, (val & PCE_CHAR_CD) != 0);
	e8250_set_ri (&ser->uart, (val & PCE_CHAR_RI) != 0);
}

/* 8250 output buffer is not empty */
static
void ser_uart_check_out_8mhz(serport_8mhz_t *ser, unsigned char val)
{
	ser->check_out_8mhz = 1;
}

/* 8250 input buffer is not full */
static
void ser_uart_check_inp_8mhz(serport_8mhz_t *ser, unsigned char val)
{
	ser->check_inp_8mhz = 1;
}

void ser_receive_8mhz(serport_8mhz_t *ser, unsigned char val)
{
	e8250_receive (&ser->uart, val);
}

void ser_clock_8mhz(serport_8mhz_t *ser, unsigned n)
{
	if (ser->check_out_8mhz) {
		ser_process_output_8mhz(ser);
	}

	if (ser->check_inp_8mhz) {
		ser_process_input_8mhz(ser);
	}

	ser_uart_check_status_8mhz(ser);
}
