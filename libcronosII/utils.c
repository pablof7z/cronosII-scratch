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
 * 		* Bosko Blagojevic
 **/
#include <glib.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>

#include "error.h"
#include "i18n.h"
#include "utils.h"

typedef void (*C2Signal_NONE__INT_INT_INT)		(GtkObject *object, gint arg1, gint arg2, gint arg3,
													gpointer user_data);

void
c2_marshal_NONE__INT_INT_INT (GtkObject *object, GtkSignalFunc func, gpointer func_data, GtkArg * args)
{
	C2Signal_NONE__INT_INT_INT rfunc;
	rfunc = (C2Signal_NONE__INT_INT_INT) func;
	(*rfunc) (object, GTK_VALUE_INT (args[0]), GTK_VALUE_INT (args[1]), GTK_VALUE_INT (args[2]), func_data);
}



typedef gint (*C2Signal_INT__POINTER_POINTER_POINTER)	(GtkObject *object, gpointer arg1,
														 gpointer arg2,
														 gpointer arg3, gpointer user_data);

void
c2_marshal_INT__POINTER_POINTER_POINTER (GtkObject *object, GtkSignalFunc func, gpointer func_data, GtkArg * args)
{
	C2Signal_INT__POINTER_POINTER_POINTER rfunc;
	gint *return_val;
	return_val = GTK_RETLOC_INT (args[3]);
	rfunc = (C2Signal_INT__POINTER_POINTER_POINTER) func;
	*return_val = (*rfunc) (object, GTK_VALUE_POINTER (args[0]), GTK_VALUE_POINTER (args[1]),
							GTK_VALUE_POINTER (args[2]), func_data);
}


