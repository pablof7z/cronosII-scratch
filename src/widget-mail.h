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
/**
 * Maintainer(s) of this file:
 * 		* Pablo Fernández
 * Code of this file by:
 * 		* Pablo Fernández
 */
#ifndef __WIDGET_MAIL_H__
#define __WIDGET_MAIL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <gtk/gtkvbox.h>

#if defined (HAVE_CONFIG_H) && defined (BUILDING_C2)
#	include <libcronosII/message.h>
#	include <config.h>
#	include "widget-part.h"
#	include "widget-application.h"
#else
#	include <cronosII.h>
#endif

#define C2_MAIL(obj)						GTK_CHECK_CAST (obj, c2_mail_get_type (), C2Mail)
#define C2_MAIL_CLASS(klass)				GTK_CHECK_CLASS_CAST (klass, c2_mail_get_type (), C2MailClass)
#define C2_IS_MAIL(obj)						GTK_CHECK_TYPE (obj, c2_mail_get_type ())
#define C2_MAIL_CLASS_FW(obj)				(C2_MAIL_CLASS (((GtkObject*)(obj))->klass))

typedef struct _C2Mail C2Mail;
typedef struct _C2MailClass C2MailClass;

struct _C2Mail
{
	GtkVBox vbox;

	C2Application *application;
	GtkWidget *window;

	C2Message *message;

	/* Search options */
	gchar *search_string;
	gboolean case_sensitive : 1;
	gboolean stop_at_end : 1;

	/* Search internal data */
	gboolean started_at_begining : 1;
	gint search_position;
	
	/* Headers */
	GtkWidget *headers;
	GtkWidget *from_label[2];
	GtkWidget *to_label[2];
	GtkWidget *cc_label[2];
	GtkWidget *subject_label[2];
	GtkWidget *attachments_button;
	GtkWidget *attachments_list;
	GtkWidget *attachments_scroll;
	gint headers_visible : 1;

	/* Part Shower */
	GtkWidget *body;
};

struct _C2MailClass
{
	GtkVBoxClass parent_class;

	void (*search) (C2Mail *mail, const gchar *string);
	void (*next_search_match) (C2Mail *mail);
};

guint
c2_mail_get_type							(void);

GtkWidget *
c2_mail_new									(C2Application *application);

void
c2_mail_construct							(C2Mail *mail, C2Application *application);

void
c2_mail_set_file							(C2Mail *mail, const gchar *path);

void
c2_mail_set_string							(C2Mail *mail, const gchar *string);

void
c2_mail_set_message							(C2Mail *mail, C2Message *message);

C2Message *
c2_mail_get_message							(C2Mail *mail);

void
c2_mail_set_headers_visible					(C2Mail *mail, gboolean show);

gboolean
c2_mail_get_headers_visible					(C2Mail *mail);

void
c2_mail_install_hints						(C2Mail *mail, GtkWidget *appbar, C2Mutex *lock);

GtkWidget *
c2_mail_attachments_tool_new				(C2Mail *mail);

#ifdef __cplusplus
}
#endif

#endif
