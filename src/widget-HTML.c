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
#include <glade/glade.h>
#include <config.h>

#include <libcronosII/error.h>
#include <libcronosII/request.h>
#include <libcronosII/utils.h>

#ifdef BUILDING_C2
#	ifdef USE_GTKHTML
#		include "widget-HTML-gtkhtml.h"
#	elif defined (USE_GTKXMTML)
#		include "widget-HTML-gtkxmhtml.h"
#	else
#		include "widget-HTML-text.h"
#	endif
#endif

#include "widget-HTML.h"

static void
class_init									(C2HTMLClass *klass);

static void
init										(C2HTML *obj);

#if 0
											 GtkWidget *appbar);
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
			"C2HTML",
			sizeof (C2HTML),
			sizeof (C2HTMLClass),
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
class_init (C2HTMLClass *klass)
{
#ifdef USE_GTKHTML
	parent_class = gtk_type_class (gtk_html_get_type ());
#elif defined (USE_GTKXMHTML)
	parent_class = gtk_type_class (gtk_xmhtml_get_type ());
#else
	parent_class = gtk_type_class (gtk_text_get_type ());
#endif
}

static void
init (C2HTML *obj)
{
#if defined (USE_GTKHTML)
	gtk_signal_connect (GTK_OBJECT (obj), "link_clicked",
							GTK_SIGNAL_FUNC (c2_html_gtkhtml_link_clicked), obj);
	gtk_signal_connect (GTK_OBJECT (obj), "on_url",
							GTK_SIGNAL_FUNC (c2_html_gtkhtml_on_url), obj);
	gtk_signal_connect (GTK_OBJECT (obj), "url_requested",
							GTK_SIGNAL_FUNC (c2_html_gtkhtml_url_requested), obj);
	gtk_signal_connect (GTK_OBJECT (obj), "submit",
							GTK_SIGNAL_FUNC (c2_html_gtkhtml_submit), obj);
#elif defined (USE_GTKXMHTML)
	gtk_xmhtml_set_anchor_underline_type (GTK_XMHTML (obj), GTK_ANCHOR_SINGLE_LINE);
	gtk_xmhtml_set_anchor_buttons (GTK_XMHTML (obj), FALSE);
	gtk_xmhtml_set_image_procs (GTK_XMHTML (obj), (XmImageProc) c2_html_gtkxmhtml_image_load,
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
	C2HTML *html;

	html = gtk_type_new (c2_html_get_type ());
	gtk_html_construct (GTK_WIDGET (html));

	return GTK_WIDGET (html);
#elif defined (USE_GTKXMHTML)
	C2HTML *html;
	
	html = gtk_type_new (c2_html_get_type ());

	return GTK_WIDGET (html);
#else
	GtkWidget *html;
	
	html = gtk_widget_new (C2_TYPE_HTML, "hadjustment", NULL, "vadjustment", NULL,	 NULL);
	return html;
#endif
}

void
c2_html_set_editable (C2HTML *html)
{
#ifdef USE_GTKHTML
	html_engine_set_editable (html_engine_new (GTK_WIDGET (html)), TRUE);
#endif
}

void
c2_html_set_link_manager (C2HTML *html, const gchar *prefix, C2HTMLLinkManager lm)
{
	c2_return_if_fail (prefix, C2EDATA);

	g_datalist_set_data (&html->link_manager_data, prefix, lm);
}

void
c2_html_freeze (C2HTML *html)
{
#ifdef USE_GTKHTML
#elif defined (USE_GTKXMHTML)
#else
	gtk_text_freeze (GTK_TEXT (html));
#endif
}

void
c2_html_thaw (C2HTML *html)
{
#ifdef USE_GTKHTML
#elif defined (USE_GTKXMHTML)
#else
	gtk_text_thaw (GTK_TEXT (html));
#endif
}

void
c2_html_clear (C2HTML *html)
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
 * @html: A C2HTML object to configure.
 * @type: Type of proxy information being passed.
 * @proxy: Address of the proxy (may be null).
 * @port: Port of the proxy (may be 0).
 *
 * This function will configure a C2HTML object
 * in order to use certain proxy.
 **/
void
c2_html_set_proxy (C2HTML *html, C2HTMLProxyType type, const gchar *proxy, guint port)
{
	if (html->proxy[type].host)
		g_free (html->proxy[type].host);

	if (proxy)
		html->proxy[type].host = g_strdup (proxy);

	html->proxy[type].port = port;
}

void
c2_html_set_color_default_bg (C2HTML *html, GdkColor *color)
{
#ifdef USE_GTKHTML
#elif defined (USE_GTKXMHTML)
#else
#endif
}

void
c2_html_set_color_default_fg (C2HTML *html, GdkColor *color)
{
#ifdef USE_GTKHTML
#elif defined (USE_GTKXMHTML)
#else
	html->fore = color;
#endif
}

void
c2_html_set_color_usage (C2HTML *html, gboolean allow_switching)
{
#ifdef USE_GTKHTML
#elif defined (USE_GTKXMHTML)
#else
#endif
}

void
c2_html_set_font_default (C2HTML *html, GdkFont *font, gchar *strfont, gchar *sizes)
{
#ifdef USE_GTKHTML
#elif defined (USE_GTKXMHTML)
#else
	html->font = font;
#endif
}

void
c2_html_set_font_usage (C2HTML *html, gboolean allow_switching)
{
#ifdef USE_GTKHTML
#elif defined (USE_GTKXMHTML)
#else
#endif
}

void
c2_html_set_content_from_url (C2HTML *html, const gchar *url)
{
	c2_return_if_fail (url, C2EDATA);
}

void
c2_html_set_content_from_string (C2HTML *html, const gchar *string)
{
	c2_return_if_fail (string, C2EDATA);
L
	c2_html_freeze (html);
L	c2_html_clear (html);
#ifdef USE_GTKHTML
L	gtk_html_load_from_string (GTK_HTML (html), string, strlen (string));
#elif defined (USE_GTKXMHTML)
#else
	gtk_text_insert (GTK_TEXT (html), html->font, html->fore, NULL, string, -1);
#endif
L	c2_html_thaw (html);
}

void
c2_html_set_line (C2HTML *html, guint line)
{
#ifdef USE_GTKHTML
#elif defined (USE_GTKXMHTML)
#else
#endif
}

guint
c2_html_get_line (C2HTML *html)
{
#ifdef USE_GTKHTML
#elif defined (USE_GTKXMHTML)
#else
	return 0;
#endif
}

void
c2_html_install_hints (C2HTML *html, GtkWidget *appbar, pthread_mutex_t *lock)
{
	c2_return_if_fail (appbar, C2EDATA);

	html->appbar = appbar;
	html->appbar_lock = lock;
}

/* GtkXText specific definitions */
static gchar *
html2text (const gchar *string)
{
}
