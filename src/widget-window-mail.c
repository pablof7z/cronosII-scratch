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
#include "widget-window-mail.h"

static void
class_init					(C2WindowMailClass *klass);

static void
init						(C2WindowMail *wmail);

static void
destroy						(C2WindowMail *wmail);

enum
{
	LAST_SIGNAL
};

GtkType
c2_window_mail_get_type (void)
{
	static GtkType type = 0;

	if (!type)
	{
		static GtkTypeInfo info =
		{
			"C2WindowMail",
			sizeof (C2WindowMail),
			sizeof (C2WindowMailClass),
			(GtkClassInitFunc) class_init,
			(GtkObjectInitFunc) init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL
		};

		type = gtk_type_unique (c2_window_get_type (), &info);
	}

	return type;
}

static void
class_init (C2WindowMailClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *) klass;
	
	parent_class = gtk_type_class (c2_window_get_type ());

	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);
}

static void
init (C2WindowMail *wmail)
{
	wmail->toolbar = NULL;
	wmail->db = NULL;
	wmail->message = NULL;
	wmail->read_only = 0;
}

static void
destroy (C2WindowMail *wmail)
{
	if (C2_IS_DB (wmail->db))
		gtk_object_unref (GTK_OBJECT (wmail->db));

	if (C2_IS_MESSAGE (wmail->message))
		gtk_object_unref (GTK_OBJECT (wmail->message));
}

GtkWidget *
c2_window_mail_new (C2Application *application)
{
	C2WindowMail *wmail;

	wmail = gtk_type_new (c2_window_mail_get_type ());
	
	c2_window_mail_construct (wmail, application);

	return GTK_WIDGET (wmail);
}

void
c2_window_mail_construct (C2WindowMail *wmail, C2Application *application)
{
	c2_window_construct 
}
