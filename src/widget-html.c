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
#include <gnome.h>
#include <config.h>

#include <libcronosII/error.h>
#include <libcronosII/request.h>
#include <libcronosII/utils.h>

#ifdef USE_GTKHTML
#	include <gtkhtml/gtkhtml.h>
#	include <gtkhtml/htmlengine.h>
#elif defined (USE_GTKXMTML)
#	include <gtk-xmhtml/gtk-xmhtml.h>
#else
#endif

#include "widget-html.h"

static void
class_init									(C2HtmlClass *klass);

static void
init										(C2Html *obj);

#ifdef USE_GTKHTML
/* GtkHTML specific definitions */
static void
on_html_url_requested						(GtkWidget *widget, const gchar *url, GtkHTMLStream *handle,
											 gpointer data);

#elif defined (USE_GTKXMHTML)
/* GtkXmHTML specific definitions */
static XmImageInfo *
on_gtkxmhtml_image_load						(GtkWidget *widget, const gchar *href);

static void
on_gtkxmhtml_anchor_track					(GtkWidget *widget, XmHTMLAnchorCallbackStruct *cbs,
											 GtkWidget *appbar);
#else
/* GtkText specific definitions */
static gchar *
html2text									(const gchar *string);

#endif

enum
{
	LAST_SIGNAL
};

static gint signals[LAST_SIGNAL] = { 0 };

#ifdef USE_GTKHTML
static GtkHTML *parent_class = NULL;
#elif defined (USE_GTKXMHTML)
static GtkXmHTMLClass *parent_class = NULL;
#else
static GtkTextClass *parent_class = NULL;
#endif

GtkType
c2_html_get_type (void)
{
	static GtkType type = 0;

	if (!type)
	{
		GtkTypeInfo info =
		{
			"C2Html",
			sizeof (C2Html),
			sizeof (C2HtmlClass),
			(GtkClassInitFunc) class_init,
			(GtkObjectInitFunc) init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL,
			(GtkClassInitFunc) NULL
		};

#ifdef USE_GTKHTML
		type = gtk_type_unique (gtk_html_get_type (), &info);
#elif defined (USE_GTKXMHTML)
#else
		type = gtk_type_unique (gtk_text_get_type (), &info);
#endif
	}

	return type;
}

static void
class_init (C2HtmlClass *klass)
{
#ifdef USE_GTKHTML
	parent_class = gtk_type_class (gtk_html_get_type ());
#elif defined (USE_GTKXMHTML)
#else
	parent_class = gtk_type_class (gtk_text_get_type ());
#endif
}

static void
init (C2Html *obj)
{
#if defined (USE_GTKHTML)
	gtk_signal_connect (GTK_OBJECT (obj), "url_requested",
							GTK_SIGNAL_FUNC (on_html_url_requested), obj);
#elif defined (USE_GTKXMHTML)
	gtk_xmhtml_set_anchor_underline_type (GTK_XMHTML (obj), GTK_ANCHOR_SINGLE_LINE);
	gtk_xmhtml_set_anchor_buttons (GTK_XMHTML (obj), FALSE);
	gtk_xmhtml_set_image_procs (GTK_XMHTML (obj), (XmImageProc) on_gtkxmhtml_image_load,
					NULL, NULL, NULL);
#else
#endif

	pthread_mutex_init (&obj->lock, NULL);
	g_datalist_init (&obj->link_manager_data);
	obj->appbar = NULL;
	obj->appbar_lock = NULL;
}

GtkWidget *
c2_html_new (void)
{
#if defined (USE_GTKHTML)
	GtkWidget *html;

	html = gtk_type_new (c2_html_get_type ());
	gtk_html_construct (html);
	c2_html_set_content_from_url (C2_HTML (html), "file://" PKGDATADIR G_DIR_SEPARATOR_S "about.html");

	return html;
#elif defined (USE_GTKXMHTML)
	C2Html *html;
	
	html = gtk_type_new (c2_html_get_type ());

	return GTK_WIDGET (html);
#else
	GtkWidget *html;
	
	html = gtk_widget_new (C2_TYPE_HTML, "hadjustment", NULL, "vadjustment", NULL,	 NULL);
	return html;
#endif
}

void
c2_html_set_editable (C2Html *html)
{
#ifdef USE_GTKHTML
	html_engine_set_editable (html_engine_new (GTK_WIDGET (html)), TRUE);
#endif
}

void
c2_html_set_link_manager (C2Html *html, const gchar *prefix, C2HtmlLinkManager lm)
{
	c2_return_if_fail (prefix, C2EDATA);

	g_datalist_set_data (&html->link_manager_data, prefix, lm);
}

void
c2_html_freeze (C2Html *html)
{
#ifdef USE_GTKHTML
#elif defined (USE_GTKXMHTML)
#else
	gtk_text_freeze (GTK_TEXT (html));
#endif
}

void
c2_html_thaw (C2Html *html)
{
#ifdef USE_GTKHTML
#elif defined (USE_GTKXMHTML)
#else
	gtk_text_thaw (GTK_TEXT (html));
#endif
}

void
c2_html_clear (C2Html *html)
{
#ifdef USE_GTKHTML
#elif defined (USE_GTKXMHTML)
#else
	gchar *buf;
	
	buf = gtk_editable_get_chars (GTK_EDITABLE (html), 0, -1);
	gtk_text_set_point (GTK_TEXT (html), 0);
	gtk_text_forward_delete (GTK_TEXT (html), gtk_text_get_length (GTK_TEXT (html)));
	g_free (buf);
#endif
}

/**
 * c2_html_set_proxy
 * @html: A C2Html object to configure.
 * @type: Type of proxy information being passed.
 * @proxy: Address of the proxy (may be null).
 * @port: Port of the proxy (may be 0).
 *
 * This function will configure a C2Html object
 * in order to use certain proxy.
 **/
void
c2_html_set_proxy (C2Html *html, C2HtmlProxyType type, const gchar *proxy, guint port)
{
	if (html->proxy[type].host)
		g_free (html->proxy[type].host);

	if (proxy)
		html->proxy[type].host = g_strdup (proxy);

	html->proxy[type].port = port;
}

void
c2_html_set_color_default_bg (C2Html *html, GdkColor *color)
{
#ifdef USE_GTKHTML
#elif defined (USE_GTKXMHTML)
#else
#endif
}

void
c2_html_set_color_default_fg (C2Html *html, GdkColor *color)
{
#ifdef USE_GTKHTML
#elif defined (USE_GTKXMHTML)
#else
	html->fore = color;
#endif
}

void
c2_html_set_color_usage (C2Html *html, gboolean allow_switching)
{
#ifdef USE_GTKHTML
#elif defined (USE_GTKXMHTML)
#else
#endif
}

void
c2_html_set_font_default (C2Html *html, GdkFont *font, gchar *strfont, gchar *sizes)
{
#ifdef USE_GTKHTML
#elif defined (USE_GTKXMHTML)
#else
	html->font = font;
#endif
}

void
c2_html_set_font_usage (C2Html *html, gboolean allow_switching)
{
#ifdef USE_GTKHTML
#elif defined (USE_GTKXMHTML)
#else
#endif
}

void
c2_html_set_content_from_url (C2Html *html, const gchar *url)
{
	c2_return_if_fail (url, C2EDATA);
}

void
c2_html_set_content_from_string (C2Html *html, const gchar *string)
{
	c2_return_if_fail (string, C2EDATA);

	c2_html_freeze (html);
	c2_html_clear (html);
#ifdef USE_GTKHTML
	gtk_html_load_from_string (GTK_HTML (html), string, strlen (string));
#elif defined (USE_GTKXMHTML)
#else
	gtk_text_insert (GTK_TEXT (html), html->font, html->fore, NULL, string, -1);
#endif
	c2_html_thaw (html);
}

void
c2_html_set_line (C2Html *html, guint line)
{
#ifdef USE_GTKHTML
#elif defined (USE_GTKXMHTML)
#else
#endif
}

guint
c2_html_get_line (C2Html *html)
{
#ifdef USE_GTKHTML
#elif defined (USE_GTKXMHTML)
#else
	return 0;
#endif
}

void
c2_html_install_hints (C2Html *html, GtkWidget *appbar, pthread_mutex_t *lock)
{
	c2_return_if_fail (appbar, C2EDATA);

	html->appbar = appbar;
	html->appbar_lock = lock;
}

#ifdef USE_GTKHTML
/* GtkHTML specific definitions */
static void
on_html_http_url_requested_exchange (C2Request *request, C2NetObjectExchangeType type,
									 gint length, C2Pthread2 *data)
{
	GtkHTMLStream *stream = data->v2;
	const gchar *ptr;

	if (type == C2_NET_OBJECT_EXCHANGE_SEND)
		return;

	printf ("length: %d\n", length);
	printf ("<%d, %d>\n", request->got_size, length);
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
	
static void
on_html_url_requested (GtkWidget *widget, const gchar *url, GtkHTMLStream *handle, gpointer data)
{
	C2Html *html = C2_HTML (data);
	C2HtmlLinkManager lm_func;
	gchar *prefix;

	prefix = c2_str_get_word (0, url, ':');

	lm_func = (C2HtmlLinkManager) g_datalist_get_data (&html->link_manager_data, prefix);
	
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

#elif defined (USE_GTKXMHTML)
/* GtkXmHTML specific definitions */
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

static XmImageInfo *
on_gtkxmhtml_image_load (GtkWidget *widget, const gchar *href)
{
	C2Pthread3 *data;
	pthread_t thread;

	data = g_new0 (C2Pthread3, 1);
	data->v1 = widget;
	data->v2 = C2_CHAR (href);
	data->v3 = XmHTMLImageDefaultProc (widget, DATADIR "/Cronos II/pixmaps/loading-image.png", NULL, 0);
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
/* GtkXText specific definitions */
static gchar *
html2text (const gchar *string)
{
}
#endif
