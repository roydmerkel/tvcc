/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/psi/comment.h                                      *
 * Created:     2013-06-09 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2013 Hampa Hug <hampa@hampa.ch>                          *
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


#ifndef PSI_COMMENT_H
#define PSI_COMMENT_H 1


#include <drivers/psi/psi.h>


int psi_add_comment (psi_img_t *img, const char *str);
int psi_load_comment (psi_img_t *img, const char *fname);
int psi_save_comment (psi_img_t *img, const char *fname);
int psi_set_comment (psi_img_t *img, const char *str);
int psi_show_comment (psi_img_t *img);


#endif
