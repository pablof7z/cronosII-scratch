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
#include <glib.h>
#include "utils.h"
#include "imap.h"

static void
class_init									(C2IMAPClass *klass);

static void
init										(C2IMAP *imap);

static void
destroy										(GtkObject *object);

enum
{
	LOGIN_FAILED,
	MAILBOX_LIST,
	INCOMING_MAIL,
	LOGOUT,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static C2NetObjectClass *parent_class = NULL;

GtkType
c2_imap_get_type (void)
{
	static GtkType type = 0;

	if (!type)
	{
		static GtkTypeInfo info =
		{
			"C2IMAP",
			sizeof (C2IMAP),
			sizeof (C2IMAPClass),
			(GtkClassInitFunc) class_init,
			(GtkObjectInitFunc) init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL
		};

		type = gtk_type_unique (c2_net_object_get_type (), &info);
	}

	return type;
}

static void
class_init (C2IMAPClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass*) klass;

	signals[LOGIN_FAILED] =
		gtk_signal_new ("login_failed",
						GTK_RUN_LAST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2IMAPClass, login_failed),
						c2_marshal_POINTER__POINTER, GTK_TYPE_STRING, 1,
						GTK_TYPE_STRING);
	signals[MAILBOX_LIST] =
		gtk_signal_new ("mailbox_list",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2IMAPClass, mailbox_list),
						gtk_marshal_NONE__POINTER, GTK_TYPE_NONE, 1,
						GTK_TYPE_POINTER);
	signals[INCOMING_MAIL] =
		gtk_signal_new ("incoming_mail",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2IMAPClass, incoming_mail),
						gtk_marshal_NONE__NONE, GTK_TYPE_NONE, 0);
	signals[LOGOUT] =
		gtk_signal_new ("logout",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2IMAPClass, logout),
						gtk_marshal_NONE__NONE, GTK_TYPE_NONE, 0);
	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);

	klass->login_failed = NULL;
	klass->mailbox_list = NULL;
	klass->incoming_mail = NULL;
	klass->logout = NULL;
}

static void
init (C2IMAP *imap)
{
	imap->auth = 0;
	imap->cmnd = 0;
	imap->mailboxes = NULL;
	imap->selected_mailbox = NULL;
}

C2IMAP *
c2_imap_new (gchar *host, gint port, gchar *user, gchar *pass, gboolean ssl)
{
	C2IMAP *imap;

	imap = gtk_type_new (c2_imap_get_type ());
	imap->user = user;
	imap->pass = pass;

	c2_net_object_construct (C2_NET_OBJECT (imap), host, port, ssl);

	return imap;
}
