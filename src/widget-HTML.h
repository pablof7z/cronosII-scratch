/*  Cronos II - The GNOME mail client
 *  Copyright (C) 2000-2001 Pablo Fernández López
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
#ifndef __WIDGET_HTML_H__
#define __WIDGET_HTML_H__

#ifdef __cplusplus
extern "C" {
#endif

#if defined (HAVE_CONFIG_H) && defined (BUILDING_C2)
#	include <config.h>
#else
#	include <cronosII.h>
#endif
	
#ifdef BUILDING_C2
#	ifdef USE_GTKHTML
#		include "widget-HTML-gtkhtml.h"
#	elif defined (USE_GTKXMHTML)
#		include "widget-HTML-gtkxmhtml.h"
#	else
#		include "widget-HTML-text.h"
#	endif
#endif

#include <gtk/gtk.h>
#include <gtk/gtkwidget.h>
#include <pthread.h>

#if defined (HAVE_CONFIG_H) && defined (BUILDING_C2)
#	include <libcronosII/mime.h>
#	include <libcronosII/utils.h>
#	include <libcronosII/utils-mutex.h>
#	include "widget-application.h"
#else
#	include <cronosII.h>
#endif

#define C2_TYPE_HTML						(c2_html_get_type ())
#define C2_HTML(obj)						(GTK_CHECK_CAST (obj, c2_html_get_type (), C2HTML))
#define C2_HTML_CLASS(klass)				(GTK_CHECK_CLASS_CAST (klass, c2_html_get_type, C2HTMLClass))
#define C2_IS_HTML(obj)						(GTK_CHECK_TYPE (obj, c2_html_get_type ()))

typedef struct _C2HTML C2HTML;
typedef struct _C2HTMLClass C2HTMLClass;
typedef enum _C2HTMLProxyType C2HTMLProxyType;
#ifdef USE_GTKHTML
typedef void (*C2HTMLLinkManager)			(C2HTML *html, const gchar *url, GtkHTMLStream *stream);
#elif defined (USE_GTKXMHTML)
typedef void (*C2HTMLLinkManager)			(C2HTML *html, const gchar *url, C2Pthread2 *data);
#else
typedef void (*C2HTMLLinkManager)			(C2HTML *html, const gchar *url);
#endif

enum _C2HTMLProxyType
{
	C2_HTML_PROXY_HTTP,
	C2_HTML_PROXY_FTP,
	C2_HTML_PROXY_LAST
};

struct _C2HTML
{
#ifdef USE_GTKHTML
	GtkHTML parent;
#elif defined (USE_GTKXMHTML)
	GtkXmHTML parent;
#else
	GtkText parent;
#endif
	C2Mutex lock;

	GData *link_manager_data;

	GdkColor *fore;
	GdkFont *font;

	struct
	{
		gchar *host;
		guint port;
	} proxy[C2_HTML_PROXY_LAST];
	
	C2Application *application;

	GtkWidget *appbar;
	C2Mutex *appbar_lock;
};

struct _C2HTMLClass
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
c2_html_new									(C2Application *application);

void
c2_html_set_editable						(C2HTML *html);

void
c2_html_set_link_manager					(C2HTML *html, const gchar *prefix, C2HTMLLinkManager lm);

void
c2_html_freeze								(C2HTML *html);

void
c2_html_thaw								(C2HTML *html);

void
c2_html_clear								(C2HTML *html);

/* Configuration */
void
c2_html_set_proxy							(C2HTML *html, C2HTMLProxyType type, const gchar *proxy,
											 guint port);

void
c2_html_set_font_default					(C2HTML *html, GdkFont *font, gchar *strfont, gchar *sizes);

/* Data source */
void
c2_html_set_content_from_url				(C2HTML *html, const gchar *url);

void
c2_html_set_content_from_string				(C2HTML *html, const gchar *string);

/* Misc */
void
c2_html_set_line							(C2HTML *html, guint line);

guint
c2_html_get_line							(C2HTML *html);

void
c2_html_install_hints						(C2HTML *html, GtkWidget *appbar, C2Mutex *lock);

#ifdef __cplusplus
}
#endif

#endif
