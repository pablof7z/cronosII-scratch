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

#ifdef USE_GTKHTML

#include "widget-HTML.h"
#include <libcronosII/request.h>
#include <libcronosII/net-object.h>
#include <libcronosII/utils.h>
#include <pthread.h>

static void
on_html_http_url_requested_exchange (C2Request *request, C2NetObjectExchangeType type,
									 gint length, C2Pthread2 *data)
{
	GtkHTMLStream *stream = data->v2;
	const gchar *ptr;

	if (type == C2_NET_OBJECT_EXCHANGE_SEND)
		return;

	ptr = (request->source+request->got_size)-length;

	gtk_html_stream_write (stream, ptr, length);
}

static void
on_html_http_url_requested_disconnect (C2Request *request, gboolean success, C2Pthread2 *data)
{
	g_free (data);
	gtk_object_destroy (GTK_OBJECT (request));
}

static void
on_html_http_url_requested (C2Pthread2 *data)
{
	C2Request *request;
	const gchar *url = data->v1;

	request = c2_request_new (url);
	gtk_signal_connect (GTK_OBJECT (request), "exchange",
							GTK_SIGNAL_FUNC (on_html_http_url_requested_exchange), data);	
	gtk_signal_connect (GTK_OBJECT (request), "disconnect",
							GTK_SIGNAL_FUNC (on_html_http_url_requested_disconnect), data);
	c2_request_run (request);
}

static void
on_html_c2dist_url_requested (C2Pthread2 *data)
{
	GtkHTMLStream *stream;
	gchar *file, *path;
	gint length;

	path = g_strdup_printf (PKGDATADIR G_DIR_SEPARATOR_S "%s", ((gchar *) data->v1)+9);
	length = c2_get_file (path, &file);
	g_free (path);

	if (length < 0)
		return;

	stream = (GtkHTMLStream *) data->v2;

	gtk_html_stream_write (stream, file, length);
	g_free (file);
}

void
c2_html_gtkhtml_url_requested (GtkWidget *widget, const gchar *url, GtkHTMLStream *handle, gpointer data)
{
	C2HTML *html = C2_HTML (data);
	C2HTMLLinkManager lm_func;
	gchar *prefix;

	prefix = c2_str_get_word (0, url, ':');

	lm_func = (C2HTMLLinkManager) g_datalist_get_data (&html->link_manager_data, prefix);
	
	if (lm_func)
		lm_func (html, url, handle);
	else
	{
		C2Pthread2 *data;
		pthread_t thread;

		data = g_new0 (C2Pthread2, 1);
		data->v1 = (gpointer) url;
		data->v2 = handle;
		
		/* Try now for built-in supported links */
		if (c2_streq (prefix, "http"))
			pthread_create (&thread, NULL, C2_PTHREAD_FUNC (on_html_http_url_requested), data);
		else if (c2_streq (prefix, "ftp"))
			L
		else if (c2_streq (prefix, "c2dist"))
			pthread_create (&thread, NULL, C2_PTHREAD_FUNC (on_html_c2dist_url_requested), data);

	}
	g_free (prefix);
}

#endif
