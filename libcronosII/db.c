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
 * 		* Bosko Blagojevic
 */
/*******************************************
 *   NOTES                                 *
 *******************************************
 * The Cronos II Database Object uses
 * the position of the C2Db as the referrer
 * for all operation in exception to
 * functions that end its name with _by_mid.
 * All of the GList arguments for several
 * operations at once require a list of
 * C2Db's objects in the list as reference.
 * 
 * Virtual Functions have to follow this
 * schema, although they can sort out the
 * MID of any C2Db by getting the object
 * with the given position and using the
 * mid set in the object (db_object->mid),
 * like the db-cronosII does in its
 * version 3.0
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/stat.h>

#include "db.h"
#include "error.h"
#include "utils.h"
#include "utils-date.h"
#include "utils-mutex.h"

#define MOD "C2Db"
#ifdef USE_DEBUG
#	define DMOD TRUE
#else
#	define DMOD FALSE
#endif

#define FIRST_MID			1

static void
init										(C2Db *db);

static void
class_init									(C2DbClass *class);

static void
destroy										(GtkObject *object);

static void
message_destroy								(C2Message *message, C2Db *db);

enum
{
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static GtkObjectClass *parent_class = NULL;

GList *c2_db_protocol_list = NULL;

GtkType
c2_db_get_type (void)
{
	static GtkType type = 0;

	if (!type)
	{
		static const GtkTypeInfo info = {
			"C2Db",
			sizeof (C2Db),
			sizeof (C2DbClass),
			(GtkClassInitFunc) class_init,
			(GtkObjectInitFunc) init,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (gtk_object_get_type (), &info);
	}

	return type;
}

static void
class_init (C2DbClass *klass)
{
	GtkObjectClass *object_class;

	object_class = (GtkObjectClass *) klass;

	parent_class = gtk_type_class (gtk_object_get_type ());
}

static void
init (C2Db *db)
{
	db->prev = NULL;
	db->next = NULL;
}

static void
destroy (GtkObject *object)
{
	C2Db *db, *prev, *next;
	C2Mailbox *mailbox;
	gint position;

	c2_return_if_fail (C2_IS_DB (object), C2EDATA);

#ifdef USE_DEBUG
	if (_debug_db)
		C2_PRINTD (MOD, "Destroying a C2Db object\n");
#endif

	db = C2_DB (object);
	prev = db->prev;
	next = db->next;
	position = db->position;

	mailbox = db->mailbox;

	if (db->message)
		gtk_object_unref (GTK_OBJECT (db->message));
	g_free (db->subject);
	g_free (db->from);
	g_free (db->account);

	/* There's just one mail in the db */
	if (db == prev)
	{
		mailbox->db = NULL;
	} else
	{
		prev->next = next;
		next->prev = prev;

		/* Is the first mail of the db */
		if (mailbox->db == db)
			mailbox->db = next;

		/* Is not the last mail of the db */
		if (mailbox->db != next)
		{
			prev = next;
			do
			{
				next = prev;
				next = next->next->position > next->position ? next->next : NULL;
				
				prev->position = position++;
				prev = next;
			} while (prev);
		}
	}

#ifdef USE_DEBUG
	if (_debug_db)
		C2_PRINTD (MOD, "C2Db object destroyed\n");
#endif
}

static void
message_destroy (C2Message *message, C2Db *db)
{
#ifdef USE_DEBUG
	if (_debug_db)
		C2_PRINTD (MOD, "The message that this C2Db object contained was destroyed, returning to NULL\n");
#endif
	db->message = NULL;
}

/**
 * c2_db_new
 * @mailbox: The C2Mailbox object that needs to get load.
 * @mark: %TRUE if you want this C2Db object to be marked
 * 		  as important.
 * @subject: A pointer which the C2Db object will control
 * 			 with the "subject" of the message.
 * @from: A pointer which the C2Db object will control
 * 		  with the "from" of the message.
 * @account: A pointer which the C2Db object will
 * 			 control with the "account" of the message.
 * @date: Date described in unix time.
 * @mid: Message-ID of the message.
 *
 * This function will create a C2Db object with the proper
 * information setted up.
 * A C2Db object represents a message in a database, which
 * is stored in a mailbox.
 * Note that the gchar variables that this function needs
 * will be controled by this function, you should not pass
 * an string that will be freed later.
 *
 * Return Value:
 * The brand new created C2Db.
 **/
C2Db *
c2_db_new (C2Mailbox *mailbox, gboolean mark, gchar *subject, gchar *from,
			gchar *account, time_t date, gint mid, gint position)
{
	C2Db *db = NULL;
	C2Db *last;

#ifdef USE_DEBUG
	if (_debug_db)
		C2_PRINTD (MOD, "Creating a new C2Db object\n");
#endif

	c2_return_val_if_fail (mailbox, NULL, C2EDATA);
	
	db = gtk_type_new (C2_TYPE_DB);
	db->mailbox = mailbox;
	db->mark = mark;
	db->subject = subject;
	db->from = from;
	db->account = account;
	db->date = date;
	db->mid = mid;
	db->position = position;

	/* Take care of the appending of the node to the
	 * mailbox.
	 */
	if (mailbox->db)
	{
		/* If the mailbox already has a first member just
		 * move to the last node (using the circular list
		 * feature) and append it there.
		 */
		last = mailbox->db->prev;
		last->next = db;
		mailbox->db->prev = db;
	} else
	{
		/* If the mailbox doesn't has a first member just
		 * set it as the first one AND as the last one.
		 */
		last = db;
		mailbox->db = db;
	}

	/* Now connect the prev pointer of the node to what
	 * we mark as the last element.
	 */
	db->prev = last;

	/* And set the circular list feature on again. */
	db->next = mailbox->db;
	mailbox->db->prev = db;

	gtk_signal_connect (GTK_OBJECT (db), "destroy",
						GTK_SIGNAL_FUNC (destroy), NULL);

	return db;
}

void
c2_db_set_message (C2Db *db, C2Message *message)
{
	if (C2_IS_MESSAGE (db->message))
		gtk_signal_disconnect_by_func (GTK_OBJECT (db->message),
								GTK_SIGNAL_FUNC (message_destroy), db);
	if (C2_IS_MESSAGE (message))
		gtk_signal_connect (GTK_OBJECT (message), "destroy",
							GTK_SIGNAL_FUNC (message_destroy), db);

	db->message = message;
}

/**
 * c2_db_length
 * @mailbox: Mailbox where to act.
 *
 * Counts the number of messages in the mailbox.
 *
 * Return Value:
 * Number of mails in the mailbox.
 **/
gint
c2_db_length (C2Mailbox *mailbox)
{
	c2_return_val_if_fail (mailbox, 0, C2EDATA);

	if (mailbox->db)
		return mailbox->db->prev->position;
	else
		return 0;
}

/**
 * c2_db_length_type
 * @mailbox: Mailbox where to act.
 * @state: State of messages to count.
 *
 * Counts the number of messages in the mailbox of a special type.
 *
 * Return Value:
 * Number of mails in the mailbox.
 **/
gint
c2_db_length_type (C2Mailbox *mailbox, gint state)
{
	gint i = 0;
	C2Db *l;

	c2_return_val_if_fail (mailbox, 0, C2EDATA);

	if (mailbox->db)
	{
		l = mailbox->db;

		do
		{
			if (l->state == state)
				i++;
		} while (c2_db_lineal_next (l));
	}

	return i;
}

/**
 * c2_db_get_node
 * @mailbox: Mailbox where to act.
 * @n: Number of node to get (starting at 0).
 *
 * This function will get the C2Db object from the mailbox
 * that occupies the position @n.
 *
 * Return Value:
 * The C2Db object.
 **/
C2Db *
c2_db_get_node (C2Mailbox *mailbox, gint n)
{
	C2Db *l = NULL;
	gint i;
	
	c2_return_val_if_fail (mailbox, NULL, C2EDATA);

	if (mailbox->db)
	{
		l = mailbox->db;
		i = 0;

		do
		{
			if (i++ >= n)
				break;
		} while (c2_db_lineal_next (l));
	}

	return l;
}

C2Db *
c2_db_get_node_by_mid (C2Mailbox *mailbox, gint mid)
{
	C2Db *l = NULL;

	if (mailbox->db)
	{
		l = mailbox->db;
		
		do
		{
			if (l->mid == mid)
				break;
		} while (c2_db_lineal_next (l));
	}

	return l;
}

/**
 * c2_db_create_structure [VFUCTION]
 * @mailbox: The mailbox that needs to create the structure.
 *
 * This function will create a proper structure where to store
 * the messages.
 *
 * Return Value:
 * %TRUE if the structure was created succesfully.
 **/
gboolean
c2_db_create_structure (C2Mailbox *mailbox)
{
	gboolean (*func) (C2Mailbox *mailbox) = NULL;
	gboolean retval = FALSE;
	
	c2_return_val_if_fail (mailbox, FALSE, C2EDATA);

#ifdef USE_DEBUG
	if (_debug_db)
		C2_PRINTD (MOD, "Creating the structure for a C2Db of %s\n", mailbox->name);
#endif

	switch (mailbox->type)
	{
		case C2_MAILBOX_CRONOSII:
			func = c2_db_cronosII_create_structure;
			break;
		case C2_MAILBOX_IMAP:
			func = c2_db_imap_create_structure;
			break;
		case C2_MAILBOX_SPOOL:
			func = c2_db_spool_create_structure;
			break;
		default:
			g_assert_not_reached ();
			return FALSE;
	}

	if (func)
		retval = func (mailbox);

#ifdef USE_DEBUG
	if (_debug_db)
		C2_PRINTD (MOD, "Structure created\n");
#endif
	

	return retval;
}

/**
 * c2_db_update_structure [VFUNCTION]
 * @mailbox: The mailbox where to act.
 *
 * This function will check if the mailbox's structure
 * needs to be updated for whatever reason (depends
 * on the type of mailbox), and if it does, it will
 * update it.
 *
 * Return Value:
 * %TRUE if the mailbox's structure needed to get updated.
 **/
gboolean
c2_db_update_structure (C2Mailbox *mailbox)
{
	gboolean (*func) (C2Mailbox *mailbox) = NULL;
	gboolean retval = FALSE;

#ifdef USE_DEBUG
	if (_debug_db)
		C2_PRINTD (MOD, "Updating the structure for a C2Db of %s\n", mailbox->name);
#endif

	switch (mailbox->type)
	{
		case C2_MAILBOX_CRONOSII:
			func = c2_db_cronosII_update_structure;
			break;
		case C2_MAILBOX_IMAP:
			func = c2_db_imap_update_structure;
			break;
		case C2_MAILBOX_SPOOL:
			func = c2_db_spool_update_structure;
			break;
		default:
			g_assert_not_reached ();
			return FALSE;
	}

	if (func)
		retval = func (mailbox);

#ifdef USE_DEBUG
	if (_debug_db)
		C2_PRINTD (MOD, "Structure updated\n");
#endif

	return retval;
}

/**
 * c2_db_remove_structure [VFUNCTION]
 * @mailbox: The mailbox where to act.
 *
 * This function will remove the structure that mailbox
 * used when it existed. Using this function means that
 * the mailbox will be permanently deleted.
 *
 * Return Value:
 * %TRUE if the mailbox's structure was able to remove.
 **/
gboolean
c2_db_remove_structure (C2Mailbox *mailbox)
{
	gboolean (*func) (C2Mailbox *mailbox) = NULL;
	gboolean retval = FALSE;

#ifdef USE_DEBUG
	if (_debug_db)
		C2_PRINTD (MOD, "Removing the structure for a C2Db of %s\n", mailbox->name);
#endif

	switch (mailbox->type)
	{
		case C2_MAILBOX_CRONOSII:
			func = c2_db_cronosII_remove_structure;
			break;
		case C2_MAILBOX_IMAP:
			func = c2_db_imap_remove_structure;
			break;
		case C2_MAILBOX_SPOOL:
			func = c2_db_spool_remove_structure;
			break;
		default:
			g_assert_not_reached ();
			return FALSE;
	}

	if (func)
		retval = func (mailbox);

#ifdef USE_DEBUG
	if (_debug_db)
		C2_PRINTD (MOD, "Structure removed\n");
#endif

	return retval;
}

void
c2_db_compact (C2Mailbox *mailbox)
{
	size_t cbytes, tbytes;
	void (*func) (C2Mailbox *mailbox, size_t *cbytes, size_t *tbytes) = NULL;

#ifdef USE_DEBUG
	if (_debug_db)
		C2_PRINTD (MOD, "Compacting the structure for a C2Db of %s\n", mailbox->name);
#endif

	c2_mutex_lock (&mailbox->lock);
	switch (mailbox->type)
	{
		case C2_MAILBOX_CRONOSII:
			func = c2_db_cronosII_compact;
			break;
		case C2_MAILBOX_IMAP:
			func = c2_db_imap_compact;
			break;
		case C2_MAILBOX_SPOOL:
			func = c2_db_imap_compact;
			break;
	}

	if (func)
		func (mailbox, &cbytes, &tbytes);
	c2_mutex_unlock (&mailbox->lock);

#ifdef USE_DEBUG
	if (_debug_db)
		C2_PRINTD (MOD, "The structured was compacted\n");
#endif

	gtk_signal_emit_by_name (GTK_OBJECT (mailbox), "compacted", cbytes, tbytes);

#ifdef USE_DEBUG
	if (_debug_db)
		C2_PRINTD (MOD, "The application was informed about the compacting\n");
#endif
}

void
c2_db_freeze (C2Mailbox *mailbox)
{
	void (*func) (C2Mailbox *mailbox) = NULL;

	c2_return_if_fail (C2_IS_MAILBOX (mailbox), C2EDATA);

	c2_mutex_lock (&mailbox->lock);
	mailbox->freezed = 1;
	
	switch (mailbox->type)
	{
		case C2_MAILBOX_CRONOSII:
			func = c2_db_cronosII_freeze;
			break;
		case C2_MAILBOX_IMAP:
			func = c2_db_imap_freeze;
			break;
		case C2_MAILBOX_SPOOL:
			func = c2_db_spool_freeze;
			break;
		default:
			g_assert_not_reached ();
			return;
	}

#ifdef USE_DEBUG
	if (_debug_db)
		C2_PRINTD (MOD, "Freezing %s\n", mailbox->name);
#endif

	if (func)
		func (mailbox);
}

void
c2_db_thaw (C2Mailbox *mailbox)
{
	void (*func) (C2Mailbox *mailbox) = NULL;

	c2_return_if_fail (C2_IS_MAILBOX (mailbox), C2EDATA);

	mailbox->freezed = 0;
	
	switch (mailbox->type)
	{
		case C2_MAILBOX_CRONOSII:
			func = c2_db_cronosII_thaw;
			break;
		case C2_MAILBOX_IMAP:
			func = c2_db_imap_thaw;
			break;
		case C2_MAILBOX_SPOOL:
			func = c2_db_spool_thaw;
			break;
		default:
			g_assert_not_reached ();
			return;
	}

#ifdef USE_DEBUG
	if (_debug_db)
		C2_PRINTD (MOD, "Thawing %s\n", mailbox->name);
#endif

	if (func)
		func (mailbox);

	c2_mutex_unlock (&mailbox->lock);

	/* Now emit queued signals */
	if (mailbox->signals_queued)
	{
#ifdef USE_DEBUG
	if (_debug_db)
		C2_PRINTD (MOD, "Informing the application about the thaw\n");
#endif
		gtk_signal_emit_by_name (GTK_OBJECT (mailbox), "changed_mailbox",
						C2_MAILBOX_CHANGE_ANY, mailbox->db);
#ifdef USE_DEBUG
	if (_debug_db)
		C2_PRINTD (MOD, "Application informed\n");
#endif
	}
}

/**
 * c2_db_load [VFUNCTION]
 * @mailbox: Mailbox to load.
 *
 * This function will ensure that the
 * mailbox is loaded, if its not loaded,
 * it does it.
 * 
 * Return Value:
 * The number of loaded messages.
 **/
gint
c2_db_load (C2Mailbox *mailbox)
{
	gint (*func) (C2Mailbox *mailbox) = NULL;
	gint retval;
	
	c2_return_val_if_fail (C2_IS_MAILBOX (mailbox), FALSE, C2EDATA);

#ifdef USE_DEBUG
	if (_debug_db)
		C2_PRINTD (MOD, "Mailbox %s will be loaded\n", mailbox->name);
#endif
	
	c2_mutex_lock (&mailbox->lock);
	
	switch (mailbox->type)
	{
		case C2_MAILBOX_CRONOSII:
			func = c2_db_cronosII_load;
			break;
		case C2_MAILBOX_IMAP:
			func = c2_db_imap_load;
			break;
		case C2_MAILBOX_SPOOL:
			func = c2_db_spool_load;
			break;
	}

	if (!((retval = func (mailbox)) < 0))
		mailbox->db_is_loaded = 1;

#ifdef USE_DEBUG
	if (_debug_db)
		C2_PRINTD (MOD, "Mailbox loaded\n");
#endif

	c2_mutex_unlock (&mailbox->lock);

	return retval;
}

static C2Db *
add_message (C2Mailbox *mailbox, C2Message *message)
{
	C2Db *db;
	gchar *buf;
	gboolean marked = FALSE;
	gchar *subject, *from, *account;
	time_t date;

#ifdef USE_DEBUG
	if (_debug_db)
		C2_PRINTD (MOD, "Adding a DB to the DB list\n");
#endif
	
	buf = c2_message_get_header_field (message, "X-Priority:");
	if (c2_streq (buf, "highest") || c2_streq (buf, "high"))
		marked = TRUE;
	g_free (buf);

	if ((buf = c2_message_get_header_field (message, "Date:")))
	{
		date = c2_date_parse (buf);
		g_free (buf);
	} else
		date = time (NULL);

	subject = c2_message_get_header_field (message, "Subject:");
	if (mailbox->use_as & C2_MAILBOX_USE_AS_OUTBOX ||
		mailbox->use_as & C2_MAILBOX_USE_AS_SENT_ITEMS)
		from = c2_message_get_header_field (message, "To:");
	else
		from = c2_message_get_header_field (message, "From:");
	account = c2_message_get_header_field (message, "X-CronosII-Account:");

	db = c2_db_new (mailbox, marked, subject ? subject : "",
					from ? from : "",
					account ? account : "",
					date, 0, mailbox->db ? mailbox->db->prev->position+1 : 1);
	c2_db_set_message (db, message);
	buf = gtk_object_get_data (GTK_OBJECT (message), "state");
	if (!buf)
	{
		buf = c2_message_get_header_field (message, "X-CronosII-State:");
		if (!buf)
			db->state = C2_MESSAGE_UNREADED;
		else
			db->state = atoi (buf);
	} else
		db->state = (C2MessageState) buf;

	return db;
}

/**
 * c2_db_message_add [VFUNCTION]
 * @mailbox: Mailbox where to act.
 * @message: Message to be added.
 * 
 * Appends a message to the mailbox.
 *
 * Return Value:
 * %TRUE on success, %FALSE on error.
 **/
gboolean
c2_db_message_add (C2Mailbox *mailbox, C2Message *message)
{
	gint retval = 0;
	GList *list;

	list = g_list_append (NULL, message);
	retval = c2_db_message_add_list (mailbox, list);
	g_list_free (list);
	
	return retval;
}

/**
 * c2_db_message_add_list [VFUNCTION]
 * @mailbox: Mailbox where to act.
 * @list: A list of C2Message.
 * 
 * Appends several messages to the mailbox.
 *
 * Return Value:
 * %TRUE on success, %FALSE on error.
 **/
gboolean
c2_db_message_add_list (C2Mailbox *mailbox, GList *list)
{
	gboolean (*func) (C2Mailbox *mailbox, C2Db *db) = NULL;
	C2Db *db = NULL;
	GList *l;
	gboolean thaw = FALSE;

#ifdef USE_DEBUG
	if (_debug_db)
		C2_PRINTD (MOD, "There's a list of %d mails to add to the list\n", g_list_length (list));
#endif

	if (!mailbox->freezed)
	{
		c2_db_freeze (mailbox);
		thaw = TRUE;
	}

	if (!c2_db_is_load (mailbox))
	{
		c2_mutex_unlock (&mailbox->lock);
		c2_mailbox_load_db (mailbox);
		c2_mutex_lock (&mailbox->lock);
	}

	switch (mailbox->type)
	{
		case C2_MAILBOX_CRONOSII:
			func = c2_db_cronosII_message_add;
			break;
		case C2_MAILBOX_IMAP:
			func = c2_db_imap_message_add;
			break;
		case C2_MAILBOX_SPOOL:
			func = c2_db_spool_message_add;
			break;
	}

	/* Note for developers of VFUNCTIONS about this function:
	 *   The VFunction should just append the node to the
	 *   mailbox, this function will handle itself the
	 *   appending to the dynamic list of C2Db's.
	 *
	 *   The mid of the db passed to the function should be changed
	 *   by the VFUNCTION to a proper value.
	 */
	for (l = list; l; l = g_list_next (l))
	{
		C2Message *message = C2_MESSAGE (l->data);

		gtk_object_ref (GTK_OBJECT (message));

		db = add_message (mailbox, message);
		func (mailbox, db);

		gtk_object_unref (GTK_OBJECT (message));
	}

#ifdef USE_DEBUG
	if (_debug_db)
		C2_PRINTD (MOD, "List added to the database\n");
#endif
	
	mailbox->db_is_loaded = 1;

	if (thaw)
	{
#ifdef USE_DEBUG
	if (_debug_db)
		C2_PRINTD (MOD, "Thawing the database\n");
#endif
		c2_db_thaw (mailbox);
#ifdef USE_DEBUG
	if (_debug_db)
		C2_PRINTD (MOD, "Informing the application that the mailbox has changed\n");
#endif
		gtk_signal_emit_by_name (GTK_OBJECT (mailbox), "changed_mailbox",
								 C2_MAILBOX_CHANGE_ADD, db->prev);
#ifdef USE_DEBUG
	if (_debug_db)
		C2_PRINTD (MOD, "Application informed.\n");
#endif
	} else
	{
		/* I don't own the freezing, so I have to
		 * queue the signaling.
		 */
		mailbox->signals_queued = 1;
	}

	return TRUE;
}

/*static gint
db_message_remove_sort (gconstpointer a, gconstpointer b)
{
	gint ga = GPOINTER_TO_INT (a), gb = GPOINTER_TO_INT (b);
	
	return ga > gb;
}*/


gboolean
c2_db_message_remove (C2Mailbox *mailbox, C2Db *db)
{
	gboolean retval;
	GList *list;

	list = g_list_append (NULL, db);
	retval = c2_db_message_remove_list (mailbox, list);
	g_list_free (list);
	
	return retval;
}

gboolean
c2_db_message_remove_by_mid (C2Mailbox *mailbox, gint mid)
{
	gint retval = 0;
	GList *list;
	C2Db *db;

	if (!(db = c2_db_get_node_by_mid (mailbox, mid)))
	{
		c2_error_object_set (GTK_OBJECT (mailbox), C2EDATA);
		return FALSE;
	}
	
	list = g_list_append (NULL, db);
	retval = c2_db_message_remove_list (mailbox, list);
	g_list_free (list);
	
	return retval;
}

/**
 * c2_db_message_remove_list [VFUNCTION]
 * @mailbox: Mailbox where to act.
 * @list: A GList of the C2Db's to delete
 *
 * This function will remove a list of mails
 * from the mailbox.
 *
 * Return Value:
 * %TRUE on success, %FALSE on error.
 **/
gboolean
c2_db_message_remove_list (C2Mailbox *mailbox, GList *list)
{
	gboolean (*func) (C2Mailbox *mailbox, GList *list) = NULL;
	GList *l;
	
	C2Db *db;
	gint retval, first_position = -1;
	gboolean thaw = FALSE;

#ifdef USE_DEBUG
	if (_debug_db)
		C2_PRINTD (MOD, "There's a list of %d mails to be removed from the mailbox %s\n", g_list_length (list), mailbox->name);
#endif

	if (!mailbox->freezed)
	{
		c2_db_freeze (mailbox);
		thaw = TRUE;
	}

	switch (mailbox->type)
	{
		case C2_MAILBOX_CRONOSII:
			func = c2_db_cronosII_message_remove;
			break;
		case C2_MAILBOX_IMAP:
			func = c2_db_imap_message_remove;
			break;
		case C2_MAILBOX_SPOOL:
			func = c2_db_spool_message_remove;
			break;
	}

	/* Remove from the db */
	if ((retval = func (mailbox, list)))
		/*return FALSE*/;

#ifdef USE_DEBUG
	if (_debug_db)
		C2_PRINTD (MOD, "Mails removed (retval = %d)\n", retval);
#endif

	/* Remove from the loaded db list */
	for (l = list; l; l = g_list_next (l))
	{
		db = (C2Db*) l->data;

		db->prev->next = db->next;
		db->next->prev = db->prev;

		/* We want to know which was the mail with the
		 * shortest position.
		 */
		if (first_position < 0 || first_position > db->position)
			first_position = db->position;

		gtk_object_destroy (GTK_OBJECT (db));
	}
	
	db = c2_db_get_node (mailbox, first_position ? first_position-1 : 0);

	if (thaw)
	{
#ifdef USE_DEBUG
	if (_debug_db)
		C2_PRINTD (MOD, "Mailbox will be thawed\n");
#endif
		c2_db_thaw (mailbox);
#ifdef USE_DEBUG
	if (_debug_db)
		C2_PRINTD (MOD, "Application will be informed\n");
#endif
		gtk_signal_emit_by_name (GTK_OBJECT (mailbox), "changed_mailbox",
								 C2_MAILBOX_CHANGE_REMOVE, db);
#ifdef USE_DEBUG
	if (_debug_db)
		C2_PRINTD (MOD, "Application informed\n");
#endif
	} else
	{
		/* I don't own the freezing, so I have to
		 * queue the signaling.
		 */
		mailbox->signals_queued = 1;
	}

	return retval;
}

gboolean
c2_db_message_move (C2Mailbox *fmailbox, C2Mailbox *tmailbox, GList *list)
{
	return FALSE;
}

/**
 * c2_db_message_set_state [VFUNCTION]
 * @db: C2Db node where to act.
 * @state: State of message.
 *
 * This function will change the state
 * of the message.
 * State can be readed, unreaded,
 * replied and forwarded.
 **/
void
c2_db_message_set_state (C2Db *db, C2MessageState state)
{
	void (*func) (C2Db *db, C2MessageState state) = NULL;
	
	/* Note for developers of VFUNCTIONS about this function:
	 *   The VFunction should just update the mailbox.
	 */
	db->state = state;

	switch (db->mailbox->type)
	{
		case C2_MAILBOX_CRONOSII:
			func = c2_db_cronosII_message_set_state;
			break;
		case C2_MAILBOX_IMAP:
			func = c2_db_imap_message_set_state;
			break;
		case C2_MAILBOX_SPOOL:
			func = c2_db_spool_message_set_state;
			break;
		default:
			g_assert_not_reached ();
			break;
	}

#ifdef USE_DEBUG
	if (_debug_db)
		C2_PRINTD (MOD, "Informing the application about the change of state of a mail\n");
#endif

	gtk_signal_emit_by_name (GTK_OBJECT (db->mailbox), "changed_mailbox",
							C2_MAILBOX_CHANGE_STATE, db);

#ifdef USE_DEBUG
	if (_debug_db)
		C2_PRINTD (MOD, "App informed. Setting the state of a mail\n");
#endif
	func (db, state);
#ifdef USE_DEBUG
	if (_debug_db)
		C2_PRINTD (MOD, "State set\n");
#endif
}

/**
 * c2_db_message_set_mark [VFUNCTION]
 * @db: C2Db node where to act.
 * @mark: %TRUE if the mark should be active, %FALSE if not.
 *
 * This function will set the state of the mark.
 **/
void
c2_db_message_set_mark (C2Db *db, gboolean mark)
{
	void (*func) (C2Db *db, gboolean mark) = NULL;
	
	/* Note for developers of VFUNCTIONS about this function:
	 *   The VFunction should just update the mailbox.
	 */
	
	db->mark = mark;

	switch (db->mailbox->type)
	{
		case C2_MAILBOX_CRONOSII:
			func = c2_db_cronosII_message_set_mark;
			break;
		case C2_MAILBOX_IMAP:
			//func = c2_db_imap_message_set_mark;
			break;
		case C2_MAILBOX_SPOOL:
			func = c2_db_spool_message_set_mark;
			break;
	}

#ifdef USE_DEBUG
	if (_debug_db)
		C2_PRINTD (MOD, "Informing the application about the change of mark of a mail\n");
#endif

	gtk_signal_emit_by_name (GTK_OBJECT (db->mailbox), "changed_mailbox",
							C2_MAILBOX_CHANGE_STATE, db);

#ifdef USE_DEBUG
	if (_debug_db)
		C2_PRINTD (MOD, "App informed. Setting the mark of a mail\n");
#endif
	
	func (db, mark);
	
#ifdef USE_DEBUG
	if (_debug_db)
		C2_PRINTD (MOD, "Mark set\n");
#endif
}

/**
 * c2_db_load_message [VFUNCTION]
 * @db: C2Db node to load.
 *
 * This function will load a message from the mailbox.
 *
 * Return Value:
 * %TRUE on success, %FALSE on error.
 **/
gboolean
c2_db_load_message (C2Db *db)
{
	C2Message *(*func) (C2Db *db) = NULL;

	c2_return_val_if_fail (C2_IS_DB (db), FALSE, C2EDATA);

#ifdef USE_DEBUG
	if (_debug_db)
		C2_PRINTD (MOD, "Loading a message\n");
#endif

	if (C2_IS_MESSAGE (db->message))
	{
#ifdef USE_DEBUG
	if (_debug_db)
	{
		C2_PRINTD (MOD, "The message was already cached\n");
#ifdef USE_MESSAGE_CACHE
		C2_PRINTD (MOD, "Cronos II was built with message caching enabled\n");
#else
		C2_PRINTD (MOD, "Cronos II was built with message caching disabled! (ERROR!)\n");
#endif
	}
#endif
		return TRUE;
	}

	switch (db->mailbox->type)
	{
		case C2_MAILBOX_CRONOSII:
			func = c2_db_cronosII_load_message;
			break;
		case C2_MAILBOX_IMAP:
			func = c2_db_imap_load_message;
			break;
		case C2_MAILBOX_SPOOL:
			func = c2_db_spool_load_message;
			break;
	}

	c2_db_set_message (db, func (db));

#ifdef USE_DEBUG
	if (_debug_db)
		C2_PRINTD (MOD, "Message loaded\n");
#endif
	
	if (db->message)
	{
		db->message->mime = c2_mime_new (db->message);

#ifdef USE_DEBUG
	if (_debug_db)
		C2_PRINTD (MOD, "MIME parsed\n");
#endif
		return TRUE;
	}

#ifdef USE_DEBUG
	if (_debug_db)
		C2_PRINTD (MOD, "Message loading failed!\n");
#endif
	
	return FALSE;
}

/**
 * c2_db_unload_message
 * @db: C2Db node to unload.
 *
 * This function will unload a C2Db node,
 * this is useful to free the space
 * used by the message, but the node
 * is still usable for listing the
 * mailbox.
 **/
void
c2_db_unload_message (C2Db *db)
{
	gtk_object_destroy (GTK_OBJECT (db->message));
}

/**
 * c2_db_archive [TODO]
 * @mailbox: Mailbox to archive.
 *
 * This function is to archive a mailbox into
 * the permanent-deleted mailbox (a mailbox that
 * the user can't access too easy, a way to recover
 * permanent-deleted mails.
 **/
void
c2_db_archive (C2Mailbox *mailbox)
{
}

/**
 * c2_db_message_get_from_file
 * @path: Path to the file.
 *
 * This function will load a file understanding it
 * as a message.
 *
 * Return Value:
 * The message or NULL.
 **/
C2Message *
c2_db_message_get_from_file (const gchar *path)
{
	C2Message *message;
	gchar *string;
	struct stat stat_buf;
	FILE *fd;
	gint length;

	c2_return_val_if_fail (path, NULL, C2EDATA);

	if (stat (path, &stat_buf) < 0)
	{
		c2_error_set (-errno);
#ifdef USE_DEBUG
		g_print ("Stating the file failed: %s\n", path);
#endif
		return NULL;
	}

	length = ((gint) stat_buf.st_size * sizeof (gchar));

	string = g_new0 (gchar, length+1);

	if (!(fd = fopen (path, "r")))
	{
		c2_error_set (-errno);
		return NULL;
	}

	fread (string, sizeof (gchar), length, fd);
	fclose (fd);

	message = c2_message_new ();
	c2_message_set_message (message, string);
	g_free (string);

	return message;
}

/**
 * c2_db_get_protocol
 * @name: Name of the protocol to get.
 *
 * This function will return a C2Protocol*
 * called @name to be used to get that 
 * protocol callback functions.
 *
 * Return Value:
 * The C2Protocol pointer on success,
 * or NULL otherwise.
 **/
C2DbProtocol *
c2_db_get_protocol (const gchar *name)
{
	GList *ptr;
	C2DbProtocol *proto;

	for(ptr = c2_db_protocol_list; ptr != NULL; ptr = ptr->next)
	{
		proto = ptr->data;
		if(c2_streq(name, proto->name))
			return proto;
	}

	return NULL;
}

/**
 * c2_db_add_protocol
 * @name: Name of the new protocol.
 * @create_struct: Pointer to callback function for creating
 *                 a new structure via that protocol.
 * @update_struct: Pointer to callback function for updating
 *                 a new structure via that protocol.
 * ...
 * ... (list continues for remaining items)
 *
 * This function registers a new protocol the Cronos II DB 
 * engine can use to store and access emails. Please fill in
 * the function links below (use empty functions for the ones
 * that your db does not support instead of passing NULL
 * pointers). Also feel free to use the gpointer 
 * mailbox->protocol.other.obj to store a pointer to your
 * custom protocol object.
 *
 * Return Value:
 * 0 on success, -1 otherwise (if protocol name already used) 
 **/
gint
c2_db_add_protocol (const gchar *name, gboolean (create_struct) (C2Mailbox*),
		gboolean (update_struct) (C2Mailbox*), gboolean (remove_struct)(C2Mailbox*),
		void (compact) (C2Mailbox*, size_t*, size_t*), void (freeze) (C2Mailbox*),
		void (thaw) (C2Mailbox*), gint (load) (C2Mailbox*), gboolean (add_list) (C2Mailbox*,C2Db*),
		gboolean (rem_list) (C2Mailbox*, GList*), void (set_state) (C2Db*, C2MessageState),
		void set_mark (C2Db*, gboolean), C2Message* (load_message) (C2Db*))
{
	C2DbProtocol *proto;

	if(c2_db_get_protocol)
		return -1; /* protocol by that name already exists! */
	
	proto = g_new0 (C2DbProtocol, 1);
	proto->name = g_strdup(name);
	
	proto->create_struct = create_struct;
	proto->update_struct = update_struct;
	proto->remove_struct = remove_struct;
	proto->compact = compact;
	proto->freeze = freeze;
	proto->thaw = thaw;
	proto->load = load;
	proto->add_list = add_list;
	proto->rem_list = rem_list;
	proto->set_state = set_state;
	proto->set_mark = set_mark;
	proto->load_message = load_message;

	c2_db_protocol_list = g_list_append(c2_db_protocol_list, proto);
	return 0;
}

/**
 * c2_db_remove_protocol
 *
 * @name: Name of the protocol to remove.
 *
 * This function removes the dynamic protocol
 * entitled @name.
 *
 * Return Value:
 * 0 on success, -1 otherwise
 **/
gint
c2_db_remove_protocol (const gchar *name)
{	
        GList *ptr;
        C2DbProtocol *proto;

        for(ptr = c2_db_protocol_list; ptr != NULL; ptr = ptr->next)
        {
                proto = ptr->data;
                if(c2_streq(name, proto->name))
        	{
			c2_db_protocol_list = g_list_remove(c2_db_protocol_list, proto);
			g_free(proto->name);
			g_free(proto);
			return 0;
		}
	}

	return -1;
}
