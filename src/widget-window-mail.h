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
#ifndef __WIDGET_WINDOW_MAIL_H__
#define __WIDGET_WINDOW_MAIL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <gnome.h>
#include <pthread.h>
#include <glade/glade.h>

#if defined (HAVE_CONFIG_H) && defined (BUILDING_C2)
#	include "widget-window.h"
#	include "widget-window-main.h"
#else
#	include <cronosII.h>
#endif

#define C2_WINDOW_MAIL(obj)					(GTK_CHECK_CAST (obj, c2_window_mail_get_type (), C2WindowMail))
#define C2_WINDOW_MAIL_CLASS(klass)			(GTK_CHECK_CLASS_CAST (klass, c2_window_mail_get_type (), C2WindowMailClass))
#define C2_IS_WINDOW_MAIL(obj)				(GTK_CHECK_TYPE (obj, c2_window_mail_get_type ()))
#define C2_WINDOW_MAIL_CLASS_FW(obj)		(C2_WINDOW_MAIL_CLASS (((GtkObject*)(obj))->klass))

typedef struct _C2WindowMail C2WindowMail;
typedef struct _C2WindowMailClass C2WindowMailClass;

struct _C2WindowMail
{
	C2Window window;

	GtkWidget *toolbar;
	GtkWidget *mail;

	/* Data */
	C2Db *db;
	C2Message *message;

	gint read_only : 1;
};

struct _C2WindowMailClass
{
	C2WindowClass parent_class;

	void (*close) (C2WindowMail *wmail);
	void (*delete) (C2WindowMail *wmail);
	void (*forward) (C2WindowMain *wmain);
	void (*print) (C2WindowMail *wmail);
	void (*next) (C2WindowMail *wmail);
	void (*previous) (C2WindowMail *wmail);
	void (*reply) (C2WindowMail *wmail);
	void (*reply_all) (C2WindowMail *wmail);
	void (*save) (C2WindowMail *wmail);
	void (*search) (C2WindowMail *wmail);
};

GtkType
c2_window_mail_get_type						(void);

GtkWidget *
c2_window_mail_new							(C2Application *application);

void
c2_window_mail_construct					(C2WindowMail *wmail, C2Application *application);

void
c2_window_mail_set_db						(C2WindowMail *wmail, C2Db *db);

void
c2_window_mail_set_message					(C2WindowMail *wmail, C2Message *message);

#ifdef __cplusplus
}
#endif

#endif
