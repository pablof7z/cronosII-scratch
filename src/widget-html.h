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
#ifndef __CRONOSII_WIDGET_HTML_H__
#define __CRONOSII_WIDGET_HTML_H__

#ifdef __cplusplus
extern "C" {
#endif

#if defined (HAVE_CONFIG_H) && defined (BUILDING_C2)
#	include <config.h>
#else
#	include <cronosII.h>
#endif
	
#ifdef USE_GTKHTML
#	include <gtkhtml/gtkhtml.h>
#elif defined (USE_GTKXMHTML)
#	include <gtk-xmhtml/gtk-xmhtml.h>
#endif

#include <gtk/gtk.h>
#include <gtk/gtkwidget.h>
#include <pthread.h>

#if defined (HAVE_CONFIG_H) && defined (BUILDING_C2)
#	include <libcronosII/mime.h>
#else
#	include <cronosII.h>
#endif

#define C2_TYPE_HTML							(c2_html_get_type ())
#define C2_HTML(obj)							GTK_CHECK_CAST (obj, c2_html_get_type (), C2Html)
#define C2_HTML_CLASS(klass)					GTK_CHECK_CLASS_CAST (klass, c2_html_get_type, C2HtmlClass)
#define C2_IS_HTML(obj)							GTK_CHECK_TYPE (obj, c2_html_get_type ())

typedef struct _C2Html C2Html;
typedef struct _C2HtmlClass C2HtmlClass;
typedef enum _C2HtmlProxyType C2HtmlProxyType;
typedef void (*C2HtmlLinkManager)			(C2Html *html, const gchar *url, GtkHTMLStream *stream);

enum _C2HtmlProxyType
{
	C2_HTML_PROXY_HTTP,
	C2_HTML_PROXY_FTP,
	C2_HTML_PROXY_LAST
};

struct _C2Html
{
#ifdef USE_GTKHTML
	GtkHTML parent;
#elif defined (USE_GTKXMHTML)
	GtkXmHTML parent;
#else
	GtkText parent;
#endif
	pthread_mutex_t lock;

	GData *link_manager_data;

	GdkColor *fore;
	GdkFont *font;

	struct
	{
		gchar *host;
		guint port;
	} proxy[C2_HTML_PROXY_LAST];
			

	GtkWidget *appbar;
	pthread_mutex_t *appbar_lock;
};

struct _C2HtmlClass
{
#ifdef USE_GTKHTML
	GtkHTMLClass parent_class;
#elif defined (USE_GTKXMHTML)
	GtkXmHTMLClass parent_class;
#else
	GtkTextClass parent_class;
#endif
};

GtkType
c2_html_get_type							(void);

GtkWidget *
c2_html_new									(void);

void
c2_html_set_editable						(C2Html *html);

void
c2_html_set_link_manager					(C2Html *html, const gchar *prefix, C2HtmlLinkManager lm);

void
c2_html_freeze									(C2Html *html);

void
c2_html_thaw									(C2Html *html);

void
c2_html_clear									(C2Html *html);

/* Configuration */
void
c2_html_set_proxy								(C2Html *html, C2HtmlProxyType type, const gchar *proxy,
												 guint port);

void
c2_html_set_color_default_bg					(C2Html *html, GdkColor *color);

void
c2_html_set_color_default_fg					(C2Html *html, GdkColor *color);

void
c2_html_set_color_usage							(C2Html *html, gboolean allow_switching);

void
c2_html_set_font_default						(C2Html *html, GdkFont *font, gchar *strfont, gchar *sizes);

void
c2_html_set_font_usage							(C2Html *html, gboolean allow_switching);

/* Data source */
void
c2_html_set_content_from_url					(C2Html *html, const gchar *url);

void
c2_html_set_content_from_string					(C2Html *html, const gchar *string);

/* Misc */
void
c2_html_set_line								(C2Html *html, guint line);

guint
c2_html_get_line								(C2Html *html);

void
c2_html_install_hints							(C2Html *html, GtkWidget *appbar, pthread_mutex_t *lock);

#ifdef __cplusplus
}
#endif

#endif
