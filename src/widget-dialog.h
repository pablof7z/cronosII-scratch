/*  Cronos II - The GNOME mail client
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
#ifndef __WIDGET_DIALOG_H__
#define __WIDGET_DIALOG_H__

#ifdef __cplusplus
extern "C" {
#endif
	
#include <gnome.h>
#include <pthread.h>
#include <glade/glade.h>

#define C2_DIALOG_TYPE						(c2_dialog_get_type ())
#define C2_DIALOG(obj)						(GTK_CHECK_CAST (obj, c2_dialog_get_type (), C2Dialog))
#define C2_DIALOG_CLASS(klass)				(GTK_CHECK_CLASS_CAST (klass, C2_DIALOG_TYPE, C2DialogClass))

typedef struct _C2Dialog C2Dialog;
typedef struct _C2DialogClass C2DialogClass;

#ifdef BUILDING_C2
#	include "widget-application.h"
#else
#	include <cronosII.h>
#endif

struct _C2Dialog
{
	GnomeDialog dialog;

	C2Application *application;
	GladeXML *xml;
};

struct _C2DialogClass
{
	GnomeDialogClass parent_class;
};

GtkWidget *
c2_dialog_new								(C2Application *application, const gchar *title,
											 const gchar *type, ...);

void
c2_dialog_construct							(C2Dialog *dialog, C2Application *application,
											const gchar *title, const gchar *type, const gchar **buttons);

#ifdef __cplusplus
}
#endif

#endif
