/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/ibmpc/m24.h                                         *
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


#ifndef PCE_IBMPC_IBMPS2_H
#define PCE_IBMPC_IBMPS2_H 1


#include "ibmpc.h"
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

int ibmps2_get_port8 (ibmpc_t *pc, unsigned long addr, unsigned char *val);
int ibmps2_set_port8 (ibmpc_t *pc, unsigned long addr, unsigned char val);

void ibmps2_set_video_mode (ibmpc_t *pc, unsigned mode);

/* void pc_setup_m24 (ibmpc_t *pc, ini_sct_t *ini); */


#endif
