/*  Cronos II Mail Client
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
#include <stdarg.h>
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

static void
c2_mailbox_recreate_tree_ids					(C2Mailbox *head);

static C2Mailbox *
c2_mailbox_search_by_id							(C2Mailbox *head, const gchar *id);

static void
c2_mailbox_set_head								(C2Mailbox *mailbox);

static gchar *
c2_mailbox_create_id_from_parent				(C2Mailbox *parent);

enum
{
	CHANGED_MAILBOXES,
	CHANGED_MAILBOX,
	LAST_SIGNAL
};

static guint c2_mailbox_signals[LAST_SIGNAL] = { 0 };

static GtkObjectClass *parent_class = NULL;

static C2Mailbox *mailbox_head = NULL;

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

	parent_class = gtk_type_class (gtk_object_get_type ());

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
	if (mailbox->db)
		gtk_object_unref (GTK_OBJECT (mailbox->db));
	if (mailbox->child)
		gtk_object_unref (GTK_OBJECT (mailbox->child));
}

static void
c2_mailbox_destroy (GtkObject *object)
{
	C2Mailbox *mailbox, *l, *n;

	c2_return_if_fail (C2_IS_MAILBOX (object), C2EDATA);

	mailbox = C2_MAILBOX (object);
	if (c2_mailbox_get_head () == mailbox)
		c2_mailbox_set_head (mailbox->next);
	
	for (l = mailbox->child; l != NULL; l = n)
	{
		n = l->next;
		c2_mailbox_destroy_node (l);
	}
	mailbox->child = NULL;
	c2_mailbox_destroy_node (mailbox);
}

/**
 * c2_mailbox_new
 * @name: Name of mailbox.
 * @id: Id of mailbox.
 * @type: Type of mailbox.
 * @sort_by: Column to sort the mailbox by.
 * @sort_type: Wheter to use ascending or descending sorting.
 * ...: Specific information about the mailbox according to the type
 * 		of mailbox.
 * 		Cronos II Mailbox's type: Null.
 * 		IMAP Mailbox's type: gchar *server, gint port, gchar *user, gchar *pass.
 * 		MySQL Mailbox's type: gchar *server, gint port, gchar *db, gchar *user, gchar *pass.
 *
 * This function will allocate a new C2Mailbox object
 * and fill it with required information.
 * This function should be used when the mailbox already
 * has an Id assigned, when you are creating a new mailbox
 * and you don't know about the Id (you just know the Id
 * of the parent) you should use c2_mailbox_new_with_parent.
 * 
 * This function will also insert the mailbox in the mailboxes hash
 * tree according to the Id.
 *
 * Return Value:
 * The head of the hash tree.
 **/
C2Mailbox *
c2_mailbox_new (const gchar *name, const gchar *id, C2MailboxType type,
				C2MailboxSortBy sort_by, GtkSortType sort_type, ...)
{
	C2Mailbox *mailbox;
	va_list edata;

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

	switch (type)
	{
		case C2_MAILBOX_CRONOSII:
			break;
		case C2_MAILBOX_IMAP:
			va_start (edata, sort_type);
			mailbox->protocol.imap.server = va_arg (edata, gchar *);
			mailbox->protocol.imap.port = va_arg (edata, gint);
			mailbox->protocol.imap.user = va_arg (edata, gchar *);
			mailbox->protocol.imap.pass = va_arg (edata, gchar *);
			va_end (edata);
			break;
#ifdef USE_MYSQL
		case C2_MAILBOX_MYSQL:
			va_start (edata, sort_type);
			mailbox->protocol.mysql.server = va_arg (edata, gchar *);
			mailbox->protocol.mysql.port = va_arg (edata, gint);
			mailbox->protocol.mysql.db = va_arg (edata, gchar *);
			mailbox->protocol.mysql.user = va_arg (edata, gchar *);
			mailbox->protocol.mysql.pass = va_arg (edata, gchar *);
			va_end (edata);
			break;
#endif
		default:
			g_print ("Unknown mailbox type in %s (%s:%d): %d\n",
					__PRETTY_FUNCTION__, __FILE__, __LINE__, type);
	}

	if (!mailbox_head)
		mailbox_head = mailbox;
	else
		c2_mailbox_insert (mailbox_head, mailbox);

	gtk_signal_emit (GTK_OBJECT (mailbox_head),
						c2_mailbox_signals[CHANGED_MAILBOXES]);

	return mailbox_head;
}

C2Mailbox *
c2_mailbox_new_with_parent (const gchar *name, const gchar *parent_id, C2MailboxType type,
							C2MailboxSortBy sort_by, GtkSortType sort_type)
{
	C2Mailbox *parent;
	C2Mailbox *value;
	gchar *id;
	
	c2_return_val_if_fail (name, NULL, C2EDATA);

	if (parent_id)
	{
		if (!(parent = c2_mailbox_search_by_id (mailbox_head, parent_id)))
		{
			c2_error_set (C2EDATA);
			return NULL;
		}
		
		id = c2_mailbox_create_id_from_parent (parent);
	} else
	{
		if ((parent = c2_mailbox_get_head ()))
		{
			for (; parent->next; parent = parent->next);
			id = g_strdup_printf ("%d", atoi (parent->id)+1);
		} else
			id = g_strdup ("1");
	}
	value = c2_mailbox_new (name, id, type, sort_by, sort_type);
	g_free (id);
	
	return value;
}

/**
 * c2_mailbox_destroy_tree
 *
 * Unref the whole mailboxes tree.
 **/
void
c2_mailbox_destroy_tree (void)
{
	C2Mailbox *l;

	for (l = c2_mailbox_get_head (); l != NULL; l = l->next)
		gtk_object_unref (GTK_OBJECT (l));
	gtk_signal_emit (GTK_OBJECT (mailbox_head),
						c2_mailbox_signals[CHANGED_MAILBOXES]);
	c2_mailbox_set_head (NULL);
}

static void
c2_mailbox_insert (C2Mailbox *head, C2Mailbox *mailbox)
{
	C2Mailbox *l;
	gchar *id;
	
	c2_return_if_fail (head, C2EDATA);
	c2_return_if_fail (mailbox, C2EDATA);

	if (!C2_MAILBOX_IS_TOPLEVEL (mailbox))
	{
		id = g_strndup (mailbox->id, strlen (mailbox->id)-1);
		l = c2_mailbox_search_by_id (head, id);
		g_free (id);
		
		if (!l)
		{
			c2_error_set (C2EDATA);
			return;
		}
		
		if (l->child)
		{
			for (l = l->child; l->next != NULL; l = l->next);
			l->next = mailbox;
		} else
			l->child = mailbox;
	} else
	{
		for (l = head; l->next != NULL; l = l->next);
		l->next = mailbox;
	}
	gtk_signal_emit (GTK_OBJECT (mailbox_head),
						c2_mailbox_signals[CHANGED_MAILBOXES]);
}

/**
 * c2_mailbox_remove
 * @mailbox: Mailbox to remove.
 *
 * Removes a mailbox and all of its children.
 **/
void
c2_mailbox_remove (C2Mailbox *mailbox)
{
	gchar *parent_id;
	C2Mailbox *parent, *l;
	gint position;
	
	c2_return_if_fail (mailbox, C2EDATA);

	if (!C2_MAILBOX_IS_TOPLEVEL (mailbox))
	{
		position = strlen (mailbox->id)-1;
		parent_id = g_strndup (mailbox->id, position);
		parent = c2_mailbox_search_by_id (mailbox_head, parent_id);
		g_free (parent_id);
		
		if (!parent)
		{
			c2_error_set (C2EDATA);
			return;
		}

		/* Also compare the first child */
		if (*(parent->child->id+position) != *(mailbox->id+position))
		{
			for (l = parent->child; l->next != NULL; l = l->next)
			{
				if (*(l->next->id+position) == *(mailbox->id+position))
				{
					l->next = l->next->next;
					l = l->next;
					gtk_object_unref (GTK_OBJECT (mailbox));
					break;
				}
			}
		} else
		{
			parent->child = parent->child->next;
			gtk_object_unref (GTK_OBJECT (mailbox));
		}
	} else
	{
		gchar *bro_id;
		C2Mailbox *bro;

		bro_id = g_strdup_printf ("%d", (*mailbox->id)-49);

		if (atoi (bro_id) != -1)
		{
			if (!(bro = c2_mailbox_search_by_id (c2_mailbox_get_head (), bro_id)))
			{
				c2_error_set (C2EDATA);
				return;
			}

			bro->next = mailbox->next;
		} else
		{
			mailbox_head = mailbox->next;
			gtk_object_unref (GTK_OBJECT (mailbox));
		}

		g_free (bro_id);
	}

	c2_mailbox_recreate_tree_ids (NULL);
	
	gtk_signal_emit (GTK_OBJECT (mailbox_head),
						c2_mailbox_signals[CHANGED_MAILBOXES]);

	return;
}

static void
c2_mailbox_recreate_tree_ids (C2Mailbox *head)
{
	C2Mailbox *l;
	gint i;

	if (!head)
	{
		/* Toplevel */
		for (l = c2_mailbox_get_head (), i = 0; l != NULL; l = l->next, i++)
		{
			g_free (l->id);
			l->id = g_strdup_printf ("%d", i);
			if (l->child)
				c2_mailbox_recreate_tree_ids (l);
		}
	} else
	{
		/* Everything below the toplevel */
		for (l = head->child, i = 0; l != NULL; l = l->next, i++)
		{
			g_free (l->id);
			l->id = g_strdup_printf ("%s%d", head->id, i);
			if (l->child)
				c2_mailbox_recreate_tree_ids (l);
		}
	}
}

static C2Mailbox *
c2_mailbox_search_by_id (C2Mailbox *head, const gchar *id)
{
	const gchar *ptr_id;
	C2Mailbox *l;
	gint pos = 0;
	
	c2_return_val_if_fail (head, NULL, C2EDATA);
	c2_return_val_if_fail (id, NULL, C2EDATA);

	for (ptr_id = id, l = head; l != NULL;)
	{
		if (*(l->id+pos) == *ptr_id)
		{
			if (*(++ptr_id) == '\0')
				break;
			l = l->child;
			pos++;
		} else
			l = l->next;
	}

	return l;
}

static gchar *
c2_mailbox_create_id_from_parent (C2Mailbox *parent)
{
	C2Mailbox *l;
	gchar *id;
	
	if (!parent)
	{
		for (l = c2_mailbox_get_head (); l->next != NULL; l = l->next);
		id = g_strdup (l->id);
		/* Add 1 to the previous ID */
		*(id+strlen (id)-1) = (*(id+strlen (id)-1)-48)+1;
	} else
	{
		if (parent->child)
		{
			for (l = parent->child; l->next != NULL; l = l->next);
			id = g_strdup (l->id);
			/* Add 1 to the previous ID */
			*(id+strlen (id)-1) = (*(id+strlen (id)-1)-48)+1;
		} else
			id = g_strdup_printf ("%s0", parent->id);
	}

	return id;
}

/**
 * c2_mailbox_get_head
 *
 * Gets the head of the mailbox tree.
 *
 * Return Value:
 * Head of mailbox tree or NULL if the tree is empty.
 **/
C2Mailbox *
c2_mailbox_get_head (void)
{
	return mailbox_head;
}

static void
c2_mailbox_set_head (C2Mailbox *mailbox)
{
	mailbox_head = mailbox;
}

/**
 * c2_mailbox_get_by_name
 * @head: Head where to search.
 * @name: Name of mailbox to get.
 *
 * Searches recursively for a mailbox
 * with name @name.
 *
 * Return Value:
 * The requested mailbox or NULL.
 **/
const C2Mailbox *
c2_mailbox_get_by_name (const C2Mailbox *head, const gchar *name)
{
	const C2Mailbox *l;
	
	c2_return_val_if_fail (name, NULL, C2EDATA);

	for (l = head ? head : c2_mailbox_get_head (); l != NULL; l = l->next)
	{
		if (c2_streq (l->name, name))
			return l;
		if (l->child)
		{
			const C2Mailbox *s;
			if ((s = c2_mailbox_get_by_name (l->child, name)))
				return s;
		}
	}

	return NULL;
}
