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
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <config.h>

#include "error.h"
#include "mailbox.h"
#include "utils.h"

static C2Mailbox *
init										(C2Mailbox *mailbox);

static void
class_init									(C2MailboxClass *klass);

static void
destroy										(GtkObject *object);

static void
c2_mailbox_insert							(C2Mailbox *head, C2Mailbox *mailbox);

static void
c2_mailbox_recreate_tree_ids				(C2Mailbox *head);

#define c2_mailbox_search_by_id(x,y)		_c2_mailbox_search_by_id (x, y, 1)

#define c2_mailbox_get_parent(x,y)			c2_mailbox_search_by_id (x, c2_mailbox_get_parent_id (y->id))

static C2Mailbox *
_c2_mailbox_search_by_id					(C2Mailbox *head, const gchar *id, guint level);

enum
{
	CHANGED_MAILBOXES,
	CHANGED_MAILBOX,
	DB_LOADED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

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
			(GtkClassInitFunc) class_init,
			(GtkObjectInitFunc) init,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL
		};

		c2_mailbox_type = gtk_type_unique (gtk_object_get_type (), &c2_mailbox_info);
	}

	return c2_mailbox_type;
}

static void
class_init (C2MailboxClass *klass)
{
	GtkObjectClass *object_class;

	object_class = (GtkObjectClass *) klass;

	parent_class = gtk_type_class (gtk_object_get_type ());

	signals[CHANGED_MAILBOXES] =
		gtk_signal_new ("changed_mailboxes",
					GTK_RUN_FIRST,
					object_class->type,
					GTK_SIGNAL_OFFSET (C2MailboxClass, changed_mailboxes),
					gtk_marshal_NONE__NONE, GTK_TYPE_NONE, 0);

	signals[CHANGED_MAILBOX] =
		gtk_signal_new ("changed_mailbox",
					GTK_RUN_FIRST,
					object_class->type,
					GTK_SIGNAL_OFFSET (C2MailboxClass, changed_mailbox),
					gtk_marshal_NONE__INT_POINTER, GTK_TYPE_NONE, 2,
					GTK_TYPE_ENUM, GTK_TYPE_POINTER);

	signals[DB_LOADED] =
		gtk_signal_new ("db_loaded",
					GTK_RUN_FIRST,
					object_class->type,
					GTK_SIGNAL_OFFSET (C2MailboxClass, db_loaded),
					gtk_marshal_NONE__BOOL, GTK_TYPE_NONE, 1,
					GTK_TYPE_BOOL);
	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);

	klass->changed_mailboxes = NULL;
	klass->changed_mailbox = NULL;
	klass->db_loaded = NULL;
}

static C2Mailbox *
init (C2Mailbox *mailbox)
{
	mailbox->name = NULL;
	mailbox->id = NULL;
	mailbox->selection = -1;
	mailbox->db_is_loaded = 0;
	mailbox->db = NULL;
	mailbox->last_mid = -1;
	mailbox->next = NULL;
	mailbox->child = NULL;
	mailbox->freezed = 0;
	mailbox->signals_queued = 0;
	if(mailbox->type == C2_MAILBOX_IMAP)
	{
		mailbox->protocol.IMAP.noinferiors = FALSE;
		mailbox->protocol.IMAP.noselect = FALSE;
		mailbox->protocol.IMAP.marked = FALSE;
		mailbox->protocol.IMAP.imap_name = NULL;
	}

	c2_mutex_init (&mailbox->lock);
	
	return mailbox;
}

static void
c2_mailbox_destroy_node (C2Mailbox *mailbox)
{
	c2_return_if_fail (mailbox, C2EDATA);

	c2_mutex_lock(&mailbox->lock);
	switch (mailbox->type)
	{
		case C2_MAILBOX_SPOOL:
			g_free (mailbox->protocol.spool.path);
			break;
	}
	
	g_free (mailbox->name);
	g_free (mailbox->id);
	if (mailbox->db)
		gtk_object_unref (GTK_OBJECT (mailbox->db));
	if (mailbox->child)
		gtk_object_unref (GTK_OBJECT (mailbox->child));
	if(mailbox->type == C2_MAILBOX_IMAP)
		if(mailbox->protocol.IMAP.imap_name)
			g_free(mailbox->protocol.IMAP.imap_name);
	
	c2_mutex_unlock(&mailbox->lock);
	c2_mutex_destroy(&mailbox->lock);
}

static void
destroy (GtkObject *object)
{
	C2Mailbox *mailbox, *l, *n;

	mailbox = C2_MAILBOX (object);
	c2_mailbox_destroy_node (mailbox);
}

/**
 * _c2_mailbox_new
 * @head: Where to append the mailbox.
 * @name: Name of mailbox.
 * @id: Id of mailbox.
 * @independent: TRUE if the call is not from an function-envelopment, you want to say TRUE.
 * @type: Type of mailbox.
 * @sort_by: Column to sort the mailbox by.
 * @sort_type: Whether to use ascending or descending sorting.
 * ...: Specific information about the mailbox according to the type
 * 		of mailbox.
 * 		Cronos II Mailbox's type: Null.
 * 		IMAP Mailbox's type: C2IMAP *imap.
 * 		Spool Mailbox's type: gchar *path.
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
 * The new mailbox object.
 **/
C2Mailbox *
_c2_mailbox_new (C2Mailbox **head, const gchar *name, const gchar *id, gboolean independent,
				C2MailboxType type, C2MailboxSortBy sort_by, GtkSortType sort_type, ...)
{
	C2Mailbox *mailbox;
	va_list edata;

	c2_return_val_if_fail (name, NULL, C2EDATA);
	c2_return_val_if_fail (id, NULL, C2EDATA);
	
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
			mailbox->protocol.IMAP.imap = va_arg (edata, C2IMAP *);
			va_end (edata);
			break;
		case C2_MAILBOX_SPOOL:
			va_start (edata, sort_type);
			mailbox->protocol.spool.path = g_strdup (va_arg (edata, gchar *));
			va_end (edata);
			break;
#ifdef USE_DEBUG
		default:
			g_print ("Unknown mailbox type in %s (%s:%d): %d\n",
					__PRETTY_FUNCTION__, __FILE__, __LINE__, type);
#endif
	}

	if (*head)
		c2_mailbox_insert (*head, mailbox);
	else
		*head = mailbox;

	if (independent)
			gtk_signal_emit (GTK_OBJECT (*head),
							signals[CHANGED_MAILBOXES]);

	gtk_signal_connect (GTK_OBJECT (mailbox), "destroy",
						GTK_SIGNAL_FUNC (destroy), NULL);
	
	return mailbox;
}

C2Mailbox *
c2_mailbox_new_with_parent (C2Mailbox **head, const gchar *name, const gchar *parent_id, C2MailboxType type,
							C2MailboxSortBy sort_by, GtkSortType sort_type, ...)
{
	C2Mailbox *value;
	gchar *id;
	gint port;
	va_list edata;
	
	c2_return_val_if_fail (name, NULL, C2EDATA);

	if (parent_id)
	{
		C2Mailbox *parent;

		if (!(parent = c2_mailbox_search_by_id (*head, parent_id)))
		{
			c2_error_set (C2EDATA);
			return NULL;
		}
		
		id = c2_mailbox_create_id_from_parent (*head, parent);
	} else
		id = c2_mailbox_create_id_from_parent (*head, NULL);

	switch (type)
	{
		C2IMAP *imap;
		gchar *path;
		
		case C2_MAILBOX_CRONOSII:
			value = _c2_mailbox_new (head, name, id, FALSE, type, sort_by, sort_type);
			break;
		case C2_MAILBOX_IMAP:
			va_start (edata, sort_type);
			imap = va_arg (edata, C2IMAP *);
	
			value = _c2_mailbox_new (head, name, id, FALSE, type, sort_by, sort_type, imap);
			va_end (edata);
			break;
		case C2_MAILBOX_SPOOL:
			va_start (edata, sort_type);
			path = va_arg (edata, gchar *);
		
			value = _c2_mailbox_new (head, name, id, FALSE, type, sort_by, sort_type, path);
			va_end (edata);
			break;
	}
	g_free (id);

	/* Create the structure */
	if (c2_db_create_structure (value) < 0)
	{
		gtk_object_destroy (GTK_OBJECT (value));
		return NULL;
	}

	c2_db_load (value);

	gtk_signal_emit (GTK_OBJECT (*head),
							signals[CHANGED_MAILBOXES]);
	
	return value;
}

/**
 * c2_mailbox_destroy_tree
 * @head: First mailbox where to start destroying
 *
 * Unref the whole mailboxes tree.
 **/
void
c2_mailbox_destroy_tree (C2Mailbox *head)
{
	C2Mailbox *l;

	for (l = head; l != NULL; l = l->next)
		gtk_object_destroy (GTK_OBJECT (l));
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
		/* Get the ID of the parent */
		id = c2_mailbox_get_parent_id (mailbox->id);
		l = c2_mailbox_search_by_id (head, id);
		g_free (id);
		
		if (!l)
		{
			c2_error_object_set (GTK_OBJECT (mailbox), C2EDATA);
			c2_mutex_unlock(&mailbox->lock);
			return;
		}
		
		if (l->child)
		{
			for (l = l->child; l->next != NULL; l = l->next)
				;
			l->next = mailbox;
		} else
			l->child = mailbox;
	} else
	{
		for (l = head; l->next != NULL; l = l->next)
			;
		l->next = mailbox;
	}
	gtk_signal_emit (GTK_OBJECT (head),
						signals[CHANGED_MAILBOXES]);
}

/**
 * c2_mailbox_update
 * @mailbox: Mailbox to work on.
 * @name: New mailbox name.
 * @id: New mailbox id.
 * @type: Type of mailbox.
 * ...: Specific information about the mailbox according to the type
 * 		of mailbox.
 * 		Cronos II Mailbox's type: Null.
 * 		IMAP Mailbox's type: Null.
 * 		Spool Mailbox's type: gchar *path.
 * 
 * This function will update a C2Mailbox object.
 **/
void
c2_mailbox_update (C2Mailbox *mailbox, const gchar *name, const gchar *id, C2MailboxType type, ...)
{
	va_list edata;
	
	c2_return_if_fail (mailbox, C2EDATA);

	/* [TODO]
	 * This is what this function should do;
	 * 
	 * If mailbox->type != type:
	 *      c2_db_{type}_structure_create ();
	 *      for (db = mailbox->db; db = db->next)
	 *      	c2_db_{mailbox->type}_load_message (db);
	 *      	c2_db_{type}_add_message (db->message);
	 *      c2_db_{mailbox->type}_structure_remove ();
	 * Else
	 *      Do what it currently does.
	 */

	c2_mutex_lock(&mailbox->lock);
	
	g_free (mailbox->name);
	mailbox->name = g_strdup (name);

	switch (mailbox->type)
	{
		case C2_MAILBOX_SPOOL:
			g_free (mailbox->protocol.spool.path);
			mailbox->protocol.spool.path = NULL;
			break;
	}

	switch (type)
	{
		case C2_MAILBOX_CRONOSII:
			mailbox->type = C2_MAILBOX_CRONOSII;
			break;
		case C2_MAILBOX_IMAP:
			mailbox->type = C2_MAILBOX_IMAP;
			break;
		case C2_MAILBOX_SPOOL:
			va_start (edata, type);
			mailbox->type = C2_MAILBOX_SPOOL;
			mailbox->protocol.spool.path = g_strdup (va_arg (edata, gchar *));
			va_end (edata);
			break;
	}

	gtk_signal_emit (GTK_OBJECT (mailbox), signals[CHANGED_MAILBOXES]);
	c2_mutex_unlock(&mailbox->lock);
}

/**
 * c2_mailbox_remove
 * @head: Pointer to the head mailbox.
 * @mailbox: Mailbox to remove.
 *
 * Removes a mailbox and all of its children.
 * 
 * TODO: Ability to delete the mailbox with ID "0".
 **/
void
c2_mailbox_remove (C2Mailbox **head, C2Mailbox *mailbox)
{
	C2Mailbox *parent = NULL, *previous = NULL, *next;
	gchar *parent_id, *previous_id, *next_id;
	gint my_id;
	
	c2_return_if_fail (mailbox, C2EDATA);
	
	c2_mutex_lock(&mailbox->lock);

	/* First get the parent/previous and next mailboxes */
	if (!(my_id = c2_mailbox_get_id (mailbox->id, -1)))
	{
		/* This mailbox has no previous mailbox in the same level */
		if (!(parent_id = c2_mailbox_get_parent_id (mailbox->id)))
		{	
			c2_mutex_unlock(&mailbox->lock);
			return;
		}
		
		if (!(parent = c2_mailbox_search_by_id (*head, parent_id)))
		{
			c2_mutex_unlock(&mailbox->lock);
			return;
		}
		
		g_free (parent_id);
	} else
	{
		/* This mailbox has a previous mailbox in the same level */
		if (!C2_MAILBOX_IS_TOPLEVEL (mailbox))
			previous_id = g_strdup_printf ("%s-%d", c2_mailbox_get_parent_id (mailbox->id), my_id-1);
		else
			previous_id = g_strdup_printf ("%d", my_id-1);
		
		if (!(previous = c2_mailbox_search_by_id (*head, previous_id)))
		{
			c2_mutex_unlock(&mailbox->lock);
			return;
		}
		
		g_free (previous_id);
	}
	
	/* Here we should already have the parent or the previous mailbox */
	/* Get the next mailbox */
	next = mailbox->next;

	if (parent)
		parent->child = next;
	else
		previous->next = next;

	if (!parent && (parent = c2_mailbox_get_parent (*head, mailbox)))
		;
	else
		parent = *head;
	
	c2_mailbox_recreate_tree_ids (parent);

	gtk_signal_emit (GTK_OBJECT (*head), signals[CHANGED_MAILBOXES]);

	/* Remove the structure */
#ifdef USE_ARCHIVER
	c2_db_archive (mailbox);
#endif
	c2_db_remove_structure (mailbox);

	c2_mutex_unlock(&mailbox->lock);
	gtk_object_unref (GTK_OBJECT (mailbox));
}

static void
c2_mailbox_recreate_tree_ids (C2Mailbox *head)
{
	C2Mailbox *l;
	gint i;

	if (!strstr (head->id, "-"))
	{
		/* TOPLEVEL */
		for (l = head, i = 0; l != NULL; l = l->next, i++)
		{
			g_free (l->id);
			l->id = g_strdup_printf ("%d", i);
			if (l->child)
				c2_mailbox_recreate_tree_ids (l);
		}
	} else
	{
		/* Everything below the TOPLEVEL */
		for (l = head->child, i = 0; l != NULL; l = l->next, i++)
		{
			g_free (l->id);
			l->id = g_strdup_printf ("%s-%d", head->id, i);
			if (l->child)
				c2_mailbox_recreate_tree_ids (l);
		}
	}
}

static C2Mailbox *
_c2_mailbox_search_by_id (C2Mailbox *head, const gchar *id, guint level)
{
	const gchar *ptr;
	C2Mailbox *l;
	gint pos = 0;
	gint top_id;
	gint ck_id;

	c2_return_val_if_fail (head, NULL, C2EDATA);
	c2_return_val_if_fail (id, NULL, C2EDATA);

	top_id = c2_mailbox_get_id (id, level);

	for (l = head; l != NULL; l = l->next)
	{
		ck_id = c2_mailbox_get_id (l->id, level);
		if (top_id == ck_id)
		{
			if (c2_mailbox_get_level (id)-level == 0)
			{
				return l;
			}
			else
			{
				return _c2_mailbox_search_by_id (l->child, id, level+1);
			}
		}
	}

	return l;
}

/**
 * c2_mailbox_create_id_from_parent
 * @head: Head of the mailboxes list.
 * @parent: Mailbox to be parent of the new ID.
 *
 * This function will create an ID usinf @parent
 * as the parent of the new ID.
 *
 * Return Value:
 * The new ID:
 **/
gchar *
c2_mailbox_create_id_from_parent (C2Mailbox *head, C2Mailbox *parent)
{
	C2Mailbox *l;
	gchar *id;
	
	if (!parent)
	{
		if (head)
		{
			for (l = head; l->next; l = l->next)
				;
			id = g_strdup_printf ("%d", atoi (l->id)+1);
		} else
			id = g_strdup ("0");
	} else
	{
		if (parent->child)
		{
			for (l = parent->child; l->next != NULL; l = l->next)
				;
			id = g_strdup_printf ("%s-%d", c2_mailbox_get_parent_id (l->id), c2_mailbox_get_id (l->id, -1)+1);
		} else
			id = g_strdup_printf ("%s-0", parent->id);
	}

	return id;
}

/**
 * c2_mailbox_get_level
 * @id: Mailbox ID.
 *
 * This function will get the level
 * of the mailbox with ID @id in the
 * mailboxes tree.
 *
 * Return Value:
 * The level of the mailbox.
 **/
gint
c2_mailbox_get_level (const gchar *id)
{
	gint i;
	const gchar *ptr;

	c2_return_val_if_fail (id, -1, C2EDATA);

	for (i = 1, ptr = id; (ptr = strstr (ptr, "-")); i++)
		ptr++;

	return i;
}

/**
 * c2_mailbox_get_complete_id
 * @id: ID String.
 * @number: Number of id to get.
 * 
 * This function will get certain ID by position in
 * a valid mailbox ID.
 *
 * Return Value:
 * The ID required or null if it wasn't found.
 **/
gchar *
c2_mailbox_get_complete_id (const gchar *id, guint number)
{
	gint i;
	const gchar *ptr;
	
	c2_return_val_if_fail (id, NULL, C2EDATA);

	for (ptr = id, i = 0; i < number; i++, ptr++)
	{
		if (!(ptr = strstr (ptr, "-")))
		{
			if (!number)
				g_strdup (id);
			else
				return NULL;
		}
	}

	/* This is to avoid the use of return g_strndup (id, ptr-id-1) which
	 * will be a bug when ptr is == to id
	 */
	if (ptr != id)
		ptr--;

	return g_strndup (id, ptr-id);
}

/**
 * c2_mailbox_get_id
 * @id: ID String.
 * @number: Number of id to get (-1 for last one).
 * 
 * This function will get certain ID by position in
 * a valid mailbox ID.
 * The difference between this function and c2_mailbox_get_complete_id
 * is that this function will return "70" for
 * c2_mailbox_get_id ("50-60-70-84-47", 3),
 * while c2_mailbox_get_complete_id will return
 * "50-60-70" in the same call.
 *
 * Return Value:
 * The ID required or -1 if it wasn't found.
 **/
gint
c2_mailbox_get_id (const gchar *id, gint number)
{
	gint i;
	const gchar *ptr;
	
	c2_return_val_if_fail (id, -1, C2EDATA);

	if (number < 0)
	{
		for (ptr = id+strlen (id)-1; *ptr != '\0' && *ptr != '-'; ptr--);
		if (!ptr)
			return -1;
	
		return atoi (++ptr);
	}
		
	for (ptr = id, i = 1; i < number; i++)
		if (!(ptr = strstr (ptr, "-")))
			return -1;

	return atoi (ptr);
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
C2Mailbox *
c2_mailbox_get_by_name (C2Mailbox *head, const gchar *name)
{
	C2Mailbox *l;
	
	c2_return_val_if_fail (name, NULL, C2EDATA);

	for (l = head; l != NULL; l = l->next)
	{
		if (c2_streq (l->name, name))
			return l;
		if (l->child)
		{
			C2Mailbox *s;
			if ((s = c2_mailbox_get_by_name (l->child, name)))
				return s;
		}
	}

	return NULL;
}

/**
 * c2_mailbox_load_db
 * @mailbox: C2Mailbox object to load.
 *
 * This function is the point of connection
 * with the Db module. It will load the Db
 * of the mailbox and fire the db_loaded
 * signal when done.
 *
 * Return Value:
 * %TRUE if the db is loaded correctly, or %FALSE.
 **/
gboolean
c2_mailbox_load_db (C2Mailbox *mailbox)
{
	c2_return_if_fail (mailbox, C2EDATA);

	/* Check if it is already loaded */
	if (mailbox->db)
	{
		gtk_signal_emit (GTK_OBJECT (mailbox), signals[DB_LOADED], TRUE);
		return TRUE;
	}

	/* We must load the db */
	if (c2_db_load (mailbox) < 0)
	{
		gtk_signal_emit (GTK_OBJECT (mailbox), signals[DB_LOADED], FALSE);
		return FALSE;
	}

	gtk_signal_emit (GTK_OBJECT (mailbox), signals[DB_LOADED], TRUE);
	return TRUE;
}
