/*  Cronos II Mail Client /libcronosII/error.c
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
#include <stdio.h>

#include "error.h"
#include "i18n.h"

static const gchar *err_list[] =
{
	/* C2SUCCESS */		N_("Success"),
	/* C2EDATA */		N_("Data exception"),
	/* C2ENOMSG */		N_("No such message"),
	/* C2EBUSY */		N_("Process is busy"),
	/* C2ERSLV */		N_("Unknown hostname")
};

/**
 * c2_error_set
 * @err: Error number.
 *
 * Will set the error code in the internal errno.
 **/
void
c2_error_set (gint err)
{
	c2_errno = err;
}

/**
 * c2_error_get
 * @err: Error number.
 *
 * Will get the error string according to the code.
 *
 * Return Value:
 * A non freeable string describing the error.
 **/
const gchar *
c2_error_get (gint err)
{
	if (err >= 0) return err_list[err];
	else return g_strerror (err*(-1));
}





