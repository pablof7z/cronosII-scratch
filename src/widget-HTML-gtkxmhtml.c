/*  Cronos II - The GNOME Mail Client
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
#include <config.h>

#ifdef USE_GTKXMHTMLd

#include <gnome.h>
#include <glade/glade.h>
#include <libcronosII/request.h>
#include <libcronosII/net-object.h>
#include <libcronosII/utils.h>
#include <libcronosII/error.h>
#include <pthread.h>

#include "widget-HTML.h"

static void
on_gtkxmhtml_image_load_pthread_request_disconnect (C2Request *request, gboolean success, C2Pthread3 *data)
{
	GtkWidget *widget = GTK_WIDGET (data->v1);
	const gchar *url = C2_CHAR (data->v2);
	XmImageInfo *image = (XmImageInfo *) data->v3;
	XmImageInfo *new_image;
	gchar *tmpfile;
	const gchar *source;
	FILE *fd;
	gint fs;
	
	if (!success)
	{
		/* The fetching failed for whatever reason */
		gdk_threads_enter ();
		new_image = XmHTMLImageDefaultProc (widget, DATADIR "/Cronos II/pixmaps/no-image.png", NULL, 0);
	} else
	{
		/* The fetching was successful:
		 * 1. Get a tmpfile path.
		 * 2. Save the image.
		 * 3. Load the image. */
		tmpfile = c2_get_tmp_file ();
		if (!(fd = fopen (tmpfile, "w")))
		{
#ifdef USE_DEBUG
			g_print ("Unable to open to %s: %s\n", tmpfile, c2_error_get (-errno));
#endif
			gdk_threads_enter ();
			new_image = XmHTMLImageDefaultProc (widget, DATADIR "/Cronos II/pixmaps/no-image.png", NULL, 0);
			g_free (tmpfile);
		} else
		{
			source = c2_request_get_source (request);
			fwrite (source, sizeof (gchar), request->got_size, fd);
			fclose (fd);
			gdk_threads_enter ();
			new_image = XmHTMLImageDefaultProc (widget, tmpfile, NULL, 0);
			g_free (tmpfile);
		}
	}

	gtk_xmhtml_freeze (GTK_XMHTML (widget));
	XmHTMLImageReplace (widget, image, new_image);
	XmHTMLRedisplay (widget);
	gtk_xmhtml_thaw (GTK_XMHTML (widget));
	gdk_threads_leave ();

	gtk_object_unref (GTK_OBJECT (request));
}

static void
on_gtkxmhtml_image_load_pthread (C2Pthread3 *data)
{
	C2Request *request;
	
	request = c2_request_new (C2_CHAR (data->v2));
	gtk_signal_connect (GTK_OBJECT (request), "disconnect",
						GTK_SIGNAL_FUNC (on_gtkxmhtml_image_load_pthread_request_disconnect), data);
	c2_request_run (request);
}

XmImageInfo *
c2_html_gtkxmhtml_image_load (GtkWidget *widget, const gchar *href)
{
	C2Pthread3 *data;
	pthread_t thread;

	data = g_new0 (C2Pthread3, 1);
	data->v1 = widget;
	data->v2 = C2_CHAR (href);
	data->v3 = XmHTMLImageDefaultProc (widget, PKGDATADIR "/pixmaps/loading-image.png", NULL, 0);
	pthread_create (&thread, NULL, C2_PTHREAD_FUNC (on_gtkxmhtml_image_load_pthread), data);
	return data->v3;
}

static void
on_gtkxmhtml_anchor_track (GtkWidget *widget, XmHTMLAnchorCallbackStruct *cbs, GtkWidget *appbar)
{
	gnome_appbar_pop (GNOME_APPBAR (appbar));
	if (cbs->href)
	{
		gnome_appbar_push (GNOME_APPBAR (appbar), cbs->href);
	}
}

#endif
