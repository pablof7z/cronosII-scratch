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
#ifndef __WIDGET_WINDOW_H__
#define __WIDGET_WINDOW_H__

#ifdef __cplusplus
extern "C" {
#endif
	
#include <gtk/gtk.h>
#include <pthread.h>
#include <glade/glade.h>

#define C2_WINDOW_TYPE						(c2_window_get_type ())
#define C2_WINDOW(obj)						(GTK_CHECK_CAST (obj, c2_window_get_type (), C2Window))
#define C2_WINDOW_CLASS(klass)				(GTK_CHECK_CLASS_CAST (klass, C2_WINDOW_TYPE, C2WindowClass))
#define C2_IS_WINDOW(obj)					(GTK_CHECK_TYPE (obj, c2_window_get_type ()))

typedef struct _C2Window C2Window;
typedef struct _C2WindowClass C2WindowClass;
typedef enum _C2WindowReportType C2WindowReportType;

#ifdef BUILDING_C2
#	include "widget-application.h"
#else
#	include <cronosII.h>
#endif

enum _C2WindowReportType
{
	C2_WINDOW_REPORT_MESSAGE,
	C2_WINDOW_REPORT_DEBUG,
	C2_WINDOW_REPORT_WARNING,
	C2_WINDOW_REPORT_ERROR /* Destroies the application */
};

struct _C2Window
{
	GnomeApp window;

	C2Application *application;
	GladeXML *xml;

	pthread_mutex_t status_lock;
	pthread_mutex_t progress_lock;
};

struct _C2WindowClass
{
	GnomeAppClass parent_class;
};

GtkType
c2_window_get_type							(void);

GtkWidget *
c2_window_new								(C2Application *application, const gchar *title,
											 gchar *type, gchar *icon);

void
c2_window_construct							(C2Window *window, C2Application *application,
											 const gchar *title, gchar *type, gchar *icon);

void
c2_window_set_contents						(C2Window *window, GtkWidget *widget);

void
c2_window_set_contents_from_glade			(C2Window *window, const gchar *dscp);

void
c2_window_report							(C2Window *window, C2WindowReportType type, const gchar *fmt, ...);

void
c2_window_set_activity						(C2Window *window, gboolean state);

#define c2_window_set_window_icon(w,i)		gtk_object_set_data (GTK_OBJECT (w), "icon", i)

#ifdef __cplusplus
}
#endif

#endif
