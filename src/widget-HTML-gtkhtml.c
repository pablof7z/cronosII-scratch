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
#include <config.h>

#ifdef USE_GTKHTML

#include <gtkhtml/htmlform.h>
#include <gtkhtml/gtkhtml-stream.h>
#include <glade/glade.h>

#include "preferences.h"
#include "widget-application.h"
#include "widget-HTML.h"
#include "widget-composer.h"
#include <libcronosII/request.h>
#include <libcronosII/net-object.h>
#include <libcronosII/utils.h>
#include <libcronosII/utils-str.h>
#include <pthread.h>

static void
on_html_http_link_clicked_exchange (C2Request *request, C2NetObjectExchangeType type,
									 gint length, C2Pthread2 *data)
{
}

static void
on_html_http_link_clicked_disconnect (C2Request *request, gboolean success, C2Pthread3 *data)
{
	GtkHTML *html = GTK_HTML (data->v1);
	GtkHTMLStream *stream = data->v3;
	const gchar *ptr;
	gint length = request->got_size;

	ptr = (request->source+request->got_size)-length;

	gtk_html_stream_write (stream, request->source, request->got_size);
	gtk_html_end (html, stream, success ? GTK_HTML_STREAM_OK : GTK_HTML_STREAM_ERROR);
}

static void
on_html_http_link_clicked (C2Pthread3 *data)
{
	C2Request *request;
	const gchar *url = data->v2;
	GtkHTML *html = GTK_HTML (data->v1);
	GtkHTMLStream *stream;

	stream = gtk_html_begin (html);
	data->v3 = (gpointer) stream;
	
	request = c2_request_new (url);
	gtk_signal_connect (GTK_OBJECT (request), "exchange",
							GTK_SIGNAL_FUNC (on_html_http_link_clicked_exchange), data);
	gtk_signal_connect (GTK_OBJECT (request), "disconnect",
							GTK_SIGNAL_FUNC (on_html_http_link_clicked_disconnect), data);
	c2_request_run (request);
}

void
c2_html_gtkhtml_link_clicked (GtkHTML *gtkhtml, const gchar *url, gpointer data)
{
	gchar *prefix, *buf;

	prefix = c2_str_get_word (0, url, ':');

	buf = c2_preferences_get_interface_html_links ();
	if (c2_streq (buf, "default"))
	{
		if (c2_streq (prefix, "mailto"))
		{
			GtkWidget *composer;

			if ((composer = c2_composer_new (C2_HTML (data)->application)))
			{
				c2_composer_set_contents_from_link (C2_COMPOSER (composer), url);
				gtk_widget_show (composer);
			}
		} else
			gnome_url_show (url);
	} else
	{
/*		data3 = g_new0 (C2Pthread3, 1);
		data3->v1 = (gpointer) gtkhtml;
		data3->v2 = (gpointer) url;

		} else if (c2_streq (prefix, "http"))
			pthread_create (&thread, NULL, C2_PTHREAD_FUNC (on_html_http_link_clicked), data3);
		else if (c2_streq (prefix, "ftp"))
		L*/
	}

	g_free (prefix);
	g_free (buf);
}

void
c2_html_gtkhtml_on_url (GtkHTML *gtkhtml, const gchar *url, gpointer data)
{
	C2Mutex *mutex;
	
	if (!GNOME_IS_APPBAR (C2_HTML (gtkhtml)->appbar))
		return;

	mutex = C2_HTML (gtkhtml)->appbar_lock;
	
	if (mutex)
	{
		if (c2_mutex_trylock (mutex))
			return;
	}

	gnome_appbar_set_status (GNOME_APPBAR (C2_HTML (gtkhtml)->appbar),
							url ? url : "");

	c2_mutex_unlock (mutex);
}

static void
on_html_http_url_requested_exchange (C2Request *request, C2NetObjectExchangeType type,
									 gint length, C2Pthread2 *data)
{
}

static void
on_html_http_url_requested_disconnect (C2Request *request, C2NetObjectByte *byte, gboolean success, C2Pthread2 *data)
{
	GtkHTMLStream *stream = data->v2;

	gtk_html_stream_write (stream, request->source, request->got_size);
	gtk_html_stream_close (stream, GTK_HTML_STREAM_OK);
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

void
c2_html_gtkhtml_submit (GtkHTML *gtkhtml, const gchar *method, const gchar *url, const gchar *encoding,
gpointer data)
{
	HTMLEngine *engine = gtkhtml->engine;
	HTMLForm *form;

	if (!g_list_length (engine->formList))
	{
		L
		return;
	}
	printf ("%d--\n", g_list_length (engine->formList));
	form = engine->form;
	if (!form)
		L
}

#endif
