/*  Cronos II - The GNOME mail client
 *  Copyright (C) 2000-2001 Pablo Fernández
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
/**
 * Maintainer(s) of this file:
 * 		* Pablo Fernández
 * Code of this file by:
 * 		* Pablo Fernández
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
	/* C2ERSLV */		N_("Unknown hostname"),
	/* C2USRCNCL */		N_("Action cancelled by user"),
	/* C2INTERNAL */	N_("Internal error"),
	/* C2NOBJ */		N_("Internal error: Net Object has reached its maximal allocated buffer")
};

/**
 * c2_error_get
 *
 * Will get the error string according to the code.
 *
 * Return Value:
 * A non free-able string describing the error.
 **/
const gchar *
c2_error_get (void)
{
	if (c2_errno >= 0 && c2_errno != C2CUSTOM)
		return err_list[c2_errno];
	else if (c2_errno >= 0)
		return c2_errstr;
	else
		return g_strerror (c2_errno*(-1));
}

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
 * c2_error_set_custom
 * @err: Error string.
 *
 * Sets a custom error string.
 **/
void
c2_error_set_custom (gchar *err)
{
	c2_errno = C2CUSTOM;
	c2_errstr = err;
}

/**
 * c2_error_object_get
 * @object: GtkObject where to get the error from.
 *
 * This function will get an error from a GtkObject.
 *
 * Return Value:
 * Error that was set in the object.
 **/
const gchar *
c2_error_object_get (GtkObject *object)
{
	gint err = c2_error_object_get_id (object);
	
	if (err >= 0 && err != C2CUSTOM)
		return err_list[err];
	else if (err >= 0)
		return ((gchar*)gtk_object_get_data (object, "c2_errstr"));
	else
		return g_strerror (err*(-1));

	return NULL;
}

/**
 * c2_error_object_get_id
 * @object: GtkObject where to get the error from.
 *
 * This function will get the error ID assigned to a GtkObject.
 *
 * Return Value:
 * The Error ID that was set in the object.
 **/
gint
c2_error_object_get_id (GtkObject *object)
{
	return GPOINTER_TO_INT (gtk_object_get_data (object, "c2_errno"));
}

/**
 * c2_error_object_set
 * @object: GtkObject where to set the error.
 *
 * This function will set an error in an object.
 **/
void
c2_error_object_set (GtkObject *object, gint err)
{
	gtk_object_set_data (object, "c2_errno", (gpointer) err);
}

/**
 * c2_error_object_set_custom
 * @object: GtkObject where to set the error.
 * @err: Error to set.
 *
 * This function will set a custom error in an object.
 **/
void
c2_error_object_set_custom (GtkObject *object, gchar *err)
{
	gtk_object_set_data (object, "c2_errno", (gpointer) C2CUSTOM);
	gtk_object_set_data (object, "c2_errstr", err);
}
