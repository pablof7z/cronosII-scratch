/*  Cronos II Mail Client /libcronosII/error.h
 *  Copyright (C) 2000-2001 Pablo Fernández Navarro
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#ifndef __LIBCRONOSII_ERROR_H__
#define __LIBCRONOSII_ERROR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <glib.h>
#include <errno.h>
	
#define c2_return_if_fail(condition, errnum) if (!(condition)) { \
		c2_error_set (errnum); \
		g_return_if_fail (condition); \
	}

#define c2_return_val_if_fail(condition, value, errnum) if (!(condition)) { \
		c2_error_set (errnum); \
		g_return_val_if_fail (condition, value); \
	}

enum
{
	C2SUCCESS,
	C2EDATA,
	C2ENOMSG,
	C2EBUSY,
	C2ERSLV,
	
	C2CUSTOM,
	C2ELAST
};

/* Own errno variable to keep track of our errors */
gint c2_errno;
const gchar *c2_errstr;

const gchar *
c2_error_get									(gint err);

void
c2_error_set									(gint err);

void
c2_error_set_custom								(const gchar *err);

#ifdef __cplusplus
}
#endif

#endif
