/*  Cronos II Mail Client /src/widget-mail.h
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
#ifndef __CRONOSII_WIDGET_MAIL__
#define __CRONOSII_WIDGET_MAIL__

#ifdef __cplusplus
extern "C" {
#endif

#include <gtk/gtkvbox.h>

#if defined (HAVE_CONFIG_H) && defined (BUILDING_C2)
#	include <libcronosII/message.h>
#	include <config.h>
#else
#	include <cronosII.h>
#endif

#include "widget-part.h"
	
#define C2_MAIL(obj)							GTK_CHECK_CAST (obj, c2_mail_get_type (), C2Mail)
#define C2_MAIL_CLASS(klass)					GTK_CHECK_CLASS_CAST (klass, c2_mail_get_type (), C2MailClass)
#define C2_IS_MAIL(obj)							GTK_CHECK_TYPE (obj, c2_mail_get_type ())

typedef struct _C2Mail C2Mail;
typedef struct _C2MailClass C2MailClass;

struct _C2Mail
{
	GtkVBox vbox;

	C2Message *message;
	
	/* Headers */
	GtkWidget *from_label;
	GtkWidget *to_label;
	GtkWidget *cc_label;
	GtkWidget *bcc_label;
	GtkWidget *subject_label;
	GtkWidget *priority_label;
	GtkWidget *from;
	GtkWidget *to;
	GtkWidget *cc;
	GtkWidget *bcc;
	GtkWidget *subject;
	GtkWidget *priority;

	gint showing_from		: 1;
	gint showing_to			: 1;
	gint showing_cc			: 1;
	gint showing_bcc		: 1;
	gint showing_subject	: 1;
	gint showing_priority	: 1;

	GtkWidget *table;

	/* Part Shower */
	GtkWidget *body;
};

struct _C2MailClass
{
	GtkVBoxClass parent_class;
};

guint
c2_mail_get_type								(void);

GtkWidget *
c2_mail_new										(void);

void
c2_mail_construct								(C2Mail *mail);

void
c2_mail_set_message								(C2Mail *mail, C2Message *message);

C2Message *
c2_mail_get_message								(C2Mail *mail);

void
c2_mail_set_show_field							(C2Mail *mail, const gchar *field, gboolean show);

gboolean
c2_mail_get_show_field							(C2Mail *mail, const gchar *field);

void
c2_mail_install_hints							(C2Mail *mail, GtkWidget *appbar, pthread_mutex_t *lock);


#ifdef __cplusplus
}
#endif

#endif
