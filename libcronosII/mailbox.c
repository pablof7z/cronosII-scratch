/*  Cronos II
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
#include <stdlib.h>
#include <config.h>

#include "error.h"
#include "mailbox.h"
#include "utils.h"

static C2Mailbox *
c2_mailbox_init									(C2Mailbox *mailbox);

static void
c2_mailbox_class_init							(C2MailboxClass *klass);

static void
c2_mailbox_destroy								(GtkObject *object);

static void
c2_mailbox_insert								(C2Mailbox *head, C2Mailbox *mailbox);

enum
{
	CHANGED_MAILBOXES,
	CHANGED_MAILBOX,
	LAST_SIGNAL
};

static guint c2_mailbox_signals[LAST_SIGNAL] = { 0 };

static GtkObjectClass *parent_class = NULL;

GtkType
c2_mailbox_get_type (void)
{
	static GtkType c2_mailbox_type = 0;

	if (!c2_mailbox_type)
	{
		static const GtkTypeInfo c2_mailbox_info =
		{
			"C2Mailbox",
			sizeof (C2Mailbox),
			sizeof (C2MailboxClass),
			(GtkClassInitFunc) c2_mailbox_class_init,
			(GtkObjectInitFunc) c2_mailbox_init,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL
		};

		c2_mailbox_type = gtk_type_unique (gtk_object_get_type (), &c2_mailbox_info);
	}

	return c2_mailbox_type;
}

static void
c2_mailbox_class_init (C2MailboxClass *klass)
{
	GtkObjectClass *object_class;

	object_class = (GtkObjectClass *) klass;

	parent_class = gtk_type_class (C2_TYPE_MAILBOX);

	c2_mailbox_signals[CHANGED_MAILBOXES] =
		gtk_signal_new ("changed_mailboxes",
					GTK_RUN_FIRST,
					object_class->type,
					GTK_SIGNAL_OFFSET (C2MailboxClass, changed_mailboxes),
					gtk_marshal_NONE__NONE, GTK_TYPE_NONE, 0);

	c2_mailbox_signals[CHANGED_MAILBOX] =
		gtk_signal_new ("changed_mailbox",
					GTK_RUN_FIRST,
					object_class->type,
					GTK_SIGNAL_OFFSET (C2MailboxClass, changed_mailbox),
					gtk_marshal_NONE__POINTER, GTK_TYPE_NONE, 1,
					GTK_TYPE_POINTER);

	gtk_object_class_add_signals (object_class, c2_mailbox_signals, LAST_SIGNAL);

	object_class->destroy = c2_mailbox_destroy;

	klass->changed_mailboxes = NULL;
	klass->changed_mailbox = NULL;
}

static C2Mailbox *
c2_mailbox_init (C2Mailbox *mailbox)
{
	mailbox->name = NULL;
	mailbox->id = NULL;
	mailbox->selection = -1;
	mailbox->db = NULL;
	mailbox->last_mid = -1;
	mailbox->next = NULL;
	mailbox->child = NULL;

	return mailbox;
}

static void
c2_mailbox_destroy_node (C2Mailbox *mailbox)
{
	c2_return_if_fail (mailbox, C2EDATA);

	g_free (mailbox->name);
	g_free (mailbox->id);
	gtk_object_unref (mailbox->db);
	if (mailbox->child)
		c2_mailbox_destroy (GTK_OBJECT (mailbox->child));
}

static void
c2_mailbox_destroy (GtkObject *object)
{
	C2Mailbox *mailbox, *l;

	c2_return_if_fail (C2_IS_MAILBOX (object), C2EDATA);

	mailbox = C2_MAILBOX (object);
	
	for (l = mailbox; l != NULL;)
		c2_mailbox_destroy_node (l);
}

C2Mailbox *
c2_mailbox_new (const gchar *name, const gchar *id, C2MailboxType type,
				C2MailboxSortBy sort_by, GtkSortType sort_type)
{
	static C2Mailbox *head = NULL;
	C2Mailbox *mailbox;

	c2_return_val_if_fail (name, NULL, C2EDATA);
	
	mailbox = gtk_type_new (C2_TYPE_MAILBOX);
	mailbox->name = g_strdup (name);
	mailbox->id = g_strdup (id);
	mailbox->type = type;

	mailbox->sort_by = sort_by;
	mailbox->sort_type = sort_type;
	
	mailbox->selection = -1;
	mailbox->db = NULL;
	mailbox->last_mid = -1;

	if (!head)
		head = mailbox;
	else
		c2_mailbox_insert (head, mailbox);

	return head;
}

C2Mailbox *
c2_mailbox_new_with_parent (const gchar *name, const gchar *parent_id,
							C2MailboxSortBy sort_by, GtkSortType sort_type)
{
	return NULL;
}

static void
c2_mailbox_insert (C2Mailbox *head, C2Mailbox *mailbox)
{
	C2Mailbox *l;
	gchar *ptr_id;
	gint id_length;
	
	c2_return_if_fail (head, C2EDATA);
	c2_return_if_fail (mailbox, C2EDATA);

	id_length = strlen (mailbox->id);
	for (ptr_id = mailbox->id, l = head; l != NULL && *ptr_id != '\0'; l = l->next)
	{
		if (*l->id == *ptr_id)
		{
			l = l->child;
			ptr_id++;
			if (id_length <= (ptr_id - mailbox->id)-1)
				break;
		}
	}

	if (!l)
	{
#ifdef USE_DEBUG
		g_print ("Couldn't find parent for %s (Id %s)\n", mailbox->name, mailbox->id);
#endif
		c2_error_set (C2EDATA);
		return;
	}
	
#ifdef USE_DEBUG
	g_print ("Parent of '%s' (Id %s) is %s (Id %s)\n", mailbox->name, mailbox->id,
			l->name, l->id);
#endif

	if (l->child)
	{
		for (l = l->child; l->next != NULL; l = l->next);
		l->next = mailbox;
	} else
		l->child = mailbox;

	/* TODO Here the "changed_mailboxes" signal should be emmited
	 * since the tree has changed once it reaches this point.
	 */
}
