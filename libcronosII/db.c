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

#define FIRST_MID			1

static void
init										(C2Db *db);

static void
class_init									(C2DbClass *class);

static void
destroy										(GtkObject *object);

enum
{
	DELETED,
	UPDATED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static GtkObjectClass *parent_class = NULL;

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

	parent_class = gtk_type_class (C2_TYPE_MESSAGE);

	signals[DELETED] =
		gtk_signal_new ("deleted",
					GTK_RUN_FIRST,
					object_class->type,
					GTK_SIGNAL_OFFSET (C2DbClass, deleted),
					gtk_marshal_NONE__NONE, GTK_TYPE_NONE, 0);

	signals[UPDATED] =
		gtk_signal_new ("updated",
					GTK_RUN_FIRST,
					object_class->type,
					GTK_SIGNAL_OFFSET (C2DbClass, updated),
					gtk_marshal_NONE__NONE, GTK_TYPE_NONE, 0);

	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);

	object_class->destroy = destroy;

	klass->deleted = NULL;
	klass->updated = NULL;
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
	C2Db *db;

	c2_return_if_fail (C2_IS_DB (object), C2EDATA);

	db = C2_DB (object);

	gtk_object_destroy (GTK_OBJECT (db->message));
	g_free (db->subject);
	g_free (db->from);
	g_free (db->account);
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

	if (mailbox->db)
	{
		last = mailbox->db->prev;
		last->next = db;
		mailbox->db->prev = db;
	} else
	{
		last = db;
		mailbox->db = db;
	}

	db->prev = last;
	db->next = mailbox->db;

	return db;
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
	gboolean (*func) (C2Mailbox *mailbox);
	
	c2_return_val_if_fail (mailbox, FALSE, C2EDATA);

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
	}

	return func (mailbox);
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
	gboolean (*func) (C2Mailbox *mailbox);

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
	}

	return func (mailbox);
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
	gboolean (*func) (C2Mailbox *mailbox);

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
	}

	return func (mailbox);
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
 * 0 if everything was OK, 1 if there was some
 * error, in the last case use c2_error_get
 * to get the error string.
 **/
gint
c2_db_load (C2Mailbox *mailbox)
{
	gint (*func) (C2Mailbox *mailbox);
	
	c2_return_val_if_fail (mailbox, 1, C2EDATA);

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

	return func (mailbox);
}

/**
 * c2_db_message_add [VFUNCTION]
 * @mailbox: Mailbox where to act.
 * @message: Message to be added.
 * 
 * Appends a message to the mailbox.
 **/
void
c2_db_message_add (C2Mailbox *mailbox, C2Message *message)
{
	gboolean (*func) (C2Mailbox *mailbox, C2Db *db);
	C2Db *l, *db;
	gchar *buf;
	gboolean marked = FALSE;
	time_t date;

	if (!mailbox->db)
		c2_mailbox_load_db (mailbox);

	/* Note for developers of VFUNCTIONS about this function:
	 *   The VFunction should just append the node to the
	 *   mailbox, this function will handle itself the
	 *   appending to the dynamic list.
	 *
	 *   The mid of the db passed to the function should be changed
	 *   by the VFUNCTION to a proper value.
	 */
	
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

	db = c2_db_new (mailbox, marked, c2_message_get_header_field (message, "Subject:"),
					c2_message_get_header_field (message, "From:"),
					c2_message_get_header_field (message, "X-CronosII-Account:"),
					date, 0, mailbox->db ? mailbox->db->prev->position+1 : 1);
	db->message = message;
	db->state = *(c2_message_get_header_field (message, "X-CronosII-State:"));
	if (!db->state)
		db->state = C2_MESSAGE_UNREADED;
	
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

	if (!func (mailbox, db))
		return;
	gtk_object_destroy (GTK_OBJECT (message));
	db->message = NULL;

	gtk_signal_emit_by_name (GTK_OBJECT (mailbox), "changed_mailbox",
							C2_MAILBOX_CHANGE_ADD_REMOVE, db->prev);
}

/**
 * c2_db_message_remove [VFUNCTION]
 * @mailbox: Mailbox where to act.
 * @db: Node to remove.
 * @n: Number of mails to delete.
 *
 * This function will remove a message
 * from the mailbox.
 **/
void
c2_db_message_remove (C2Mailbox *mailbox, GList *list)
{
}
#if 0
	void (*func) (C2Mailbox *mailbox, C2Db *db, gint n);
	C2Db *prev, *next, *l;
	gint i;
	
	/* Note for developers of VFUNCTIONS about this function:
	 *   The VFunction should just remove the node from
	 *   the mailbox, this function will handle itself
	 *   the removing from the linked list.
	 */
	c2_return_if_fail (mailbox, C2EDATA);
	c2_return_if_fail (db, C2EDATA);
	c2_return_if_fail (db->mailbox == mailbox, C2EDATA);

	prev = db->prev;
	next = c2_db_get_node (mailbox, db->position+n);

	if (prev)
		prev->next = next;
	if (next)
		next->prev = prev;
	if (!prev)
		mailbox->db = next;

#ifdef USE_DEBUG
	g_print ("Mail %d has been removed from the list, status: %dx%d (%d-%d)\n",
					db->mid, prev ? 1 : 0, next ? 1 : 0, prev->position, next->position);
#endif

	/* Rebuild the positions */
	for (l = next, i = prev->position+1; c2_db_lineal_next (l); l = l->next, i++)
		l->position = i;

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

	func (mailbox, db, n);

	for (l = db, i = 0; c2_db_lineal_next (l) && i < n; l = l->next, i++)
		gtk_object_destroy (GTK_OBJECT (l));
}
#endif

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
	void (*func) (C2Db *db, C2MessageState state);
	
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
	}

	func (db, state);
	gtk_signal_emit (GTK_OBJECT (db), UPDATED);
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
	void (*func) (C2Db *db, gboolean mark);
	
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
			func = c2_db_imap_message_set_mark;
			break;
		case C2_MAILBOX_SPOOL:
			func = c2_db_spool_message_set_mark;
			break;
	}

	func (db, mark);
	gtk_signal_emit (GTK_OBJECT (db), UPDATED);
}

/**
 * c2_db_load_message [VFUNCTION]
 * @db: C2Db node to load.
 *
 * This function will load a message from the mailbox.
 **/
void
c2_db_load_message (C2Db *db)
{
	C2Message *(*func) (C2Db *db);

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

	db->message = func (db);
	if (db->message)
		db->message->mime = c2_mime_new (db->message);
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
