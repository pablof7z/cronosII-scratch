/*  Cronos II Mail Client
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
#include <gnome.h>
#include <config.h>
#include <pthread.h>

#ifdef USE_GTKHTML
#	include <gtkhtml/gtkhtml.h>
#elif defined (USE_GTKXMHTML)
#	include <gtk-xmhtml/gtk-xmhtml.h>
#else
#endif

#include <libcronosII/error.h>
#include <libcronosII/request.h>
#include <libcronosII/utils.h>

#include "widget-part.h"

static void
c2_part_class_init								(C2PartClass *klass);

static void
c2_part_init									(C2Part *part);

#ifdef USE_GTKHTML
#elif defined (USE_GTKXMHTML)
static XmImageInfo *
on_gtkxmhtml_image_load							(GtkWidget *widget, const gchar *href);

static void
on_gtkxmhtml_anchor_track						(GtkWidget *widget, XmHTMLAnchorCallbackStruct *cbs,
												 GtkWidget *appbar);
#else
#endif

enum
{
	LAST_SIGNAL
};

static gint c2_part_signals[LAST_SIGNAL] = { 0 };

static GtkXmHTMLClass *parent_class = NULL;

guint
c2_part_get_type (void)
{
	static guint c2_part_type = 0;

	if (!c2_part_type)
	{
		GtkTypeInfo c2_part_info =
		{
			"C2Part",
			sizeof (C2Part),
			sizeof (C2PartClass),
			(GtkClassInitFunc) c2_part_class_init,
			(GtkObjectInitFunc) c2_part_init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL,
			(GtkClassInitFunc) NULL
		};

		c2_part_type = gtk_type_unique (gtk_xmhtml_get_type (), &c2_part_info);
	}
	
	return c2_part_type;
}

static void
c2_part_class_init (C2PartClass *klass)
{
	parent_class = gtk_type_class (gtk_xmhtml_get_type ());
}

static void
c2_part_init (C2Part *part)
{
#if defined (USE_GTKHTML)
#elif defined (USE_GTKXMHTML)
	gtk_xmhtml_set_anchor_underline_type (GTK_XMHTML (part),
											GTK_ANCHOR_SINGLE_LINE);
	gtk_xmhtml_set_anchor_buttons (GTK_XMHTML (part), FALSE);
	gtk_xmhtml_set_image_procs (GTK_XMHTML (part), (XmImageProc) on_gtkxmhtml_image_load,
								NULL, NULL, NULL);
#else
#endif
	part->appbar = NULL;
}

GtkWidget *
c2_part_new (void)
{
	C2Part *part;
	
	part = gtk_type_new (c2_part_get_type ());
	return GTK_WIDGET (part);
}

void
c2_part_install_hints (C2Part *part, GtkWidget *appbar)
{
#ifdef USE_GTKHTML
#elif defined (USE_GTKXMHTML)
	gtk_signal_connect (GTK_OBJECT (part), "anchor_track",
						on_gtkxmhtml_anchor_track, appbar);
#else
#endif
	part->appbar = appbar;
}

void
c2_part_set_part (C2Part *part, C2Mime *mime)
{
	c2_return_if_fail (mime, C2EDATA);

#ifdef USE_GTKHTML
#elif defined (USE_GTKXMHTML)
	gtk_xmhtml_freeze (GTK_XMHTML (part));
	gtk_xmhtml_source (GTK_XMHTML (part), c2_mime_get_part (mime));
	gtk_xmhtml_thaw (GTK_XMHTML (part));
#else
#endif
}

#ifdef USE_GTKHTML
#elif defined (USE_GTKXMHTML)
/* GtkXmHTML specific functions */
static void
on_gtkxmhtml_image_load_pthread_request_disconnect (C2Request *request, gboolean success, C2Pthread3 *data)
{
	GtkWidget *widget = GTK_WIDGET (data->v1);
	const gchar *url = C2_CHAR (data->v2);
	XmImageInfo *image = (XmImageInfo *) data->v3;
	XmImageInfo *new_image;
	gchar *tmpfile;
	const gchar *source;
	gchar tmpfile2[] = "/tmp/c2-mmm-log.XXXXX";
	FILE *fd;
	gint fs;
L	
	if (!success)
	{
		/* The fetching failed for whatever reason */
		gdk_threads_enter ();
		new_image = XmHTMLImageDefaultProc (widget, DATADIR "/cronosII/pixmaps/no-image.png", NULL, 0);
	} else
	{
		/* The fetching was successfull:
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
			new_image = XmHTMLImageDefaultProc (widget, DATADIR "/cronosII/pixmaps/no-image.png", NULL, 0);
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

static XmImageInfo *
on_gtkxmhtml_image_load (GtkWidget *widget, const gchar *href)
{
	C2Pthread3 *data;
	pthread_t thread;

	data = g_new0 (C2Pthread3, 1);
	data->v1 = widget;
	data->v2 = C2_CHAR (href);
	data->v3 = XmHTMLImageDefaultProc (widget, DATADIR "/cronosII/pixmaps/loading-image.png", NULL, 0);
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
#else
#endif
