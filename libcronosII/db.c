/*  Cronos II Mail Client /libcronosII/db.c
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

static void
c2_db_init										(C2Db *db);

static void
c2_db_class_init								(C2DbClass *class);

static void
c2_db_destroy									(GtkObject *object);

enum
{
	MARK_CHANGED,
	STATE_CHANGED,
	LAST_SIGNAL
};

static guint c2_db_signals[LAST_SIGNAL] = { 0 };

static C2MessageClass *parent_class = NULL;


GtkType
c2_db_get_type (void)
{
	static GtkType c2_db_type = 0;

	if (!c2_db_type)
	{
		static const GtkTypeInfo c2_db_info = {
			"C2Db",
			sizeof (C2Db),
			sizeof (C2DbClass),
			(GtkClassInitFunc) c2_db_class_init,
			(GtkObjectInitFunc) c2_db_init,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL
		};

		c2_db_type = gtk_type_unique (C2_TYPE_MESSAGE, &c2_db_info);
	}

	return c2_db_type;
}

static void
c2_db_class_init (C2DbClass *klass)
{
	GtkObjectClass *object_class;

	object_class = (GtkObjectClass *) klass;

	parent_class = gtk_type_class (C2_TYPE_MESSAGE);

	c2_db_signals[MARK_CHANGED] =
		gtk_signal_new ("mark_changed",
					GTK_RUN_FIRST,
					object_class->type,
					GTK_SIGNAL_OFFSET (C2DbClass, mark_changed),
					gtk_marshal_NONE__BOOL, GTK_TYPE_NONE, 1,
					GTK_TYPE_BOOL);

	c2_db_signals[STATE_CHANGED] =
		gtk_signal_new ("state_changed",
					GTK_RUN_FIRST,
					object_class->type,
					GTK_SIGNAL_OFFSET (C2DbClass, state_changed),
					gtk_marshal_NONE__ENUM, GTK_TYPE_NONE, 1,
					GTK_TYPE_ENUM);

	gtk_object_class_add_signals (object_class, c2_db_signals, LAST_SIGNAL);

	object_class->destroy = c2_db_destroy;

	klass->mark_changed = NULL;
	klass->state_changed = NULL;
}

static void
c2_db_init (C2Db *db)
{
	db->marked = 0;
	db->subject = NULL;
	db->from = NULL;
	db->account = NULL;
	db->date = (time_t) 0;
	db->position = -1;
	db->mid = -1;
	db->previous = NULL;
	db->next = NULL;
}

C2Db *
c2_db_new (C2Mailbox *mailbox)
{
	C2Db *db = NULL;
	
	c2_return_val_if_fail (mailbox, NULL, C2EDATA);
	
	db = gtk_type_new (C2_TYPE_DB);
	db->mailbox = mailbox;

	return db;
}

static void
c2_db_destroy (GtkObject *object)
{
	C2Db *db, *l;

	c2_return_if_fail (C2_IS_DB (object), C2EDATA);

	db = C2_DB (object);
	
	if (!db->previous)
	{
		for (l = db; l != NULL;)
		{
			db = l->next;
			gtk_object_unref (GTK_OBJECT (l));
			l = db;
		}
	} else
	{
		g_free (db->subject);
		g_free (db->account);
		g_free (db->from);
		g_free (db);
	}
}

gint
c2_db_load (C2Mailbox *mailbox)
{
	gint (*func) (C2Mailbox *mailbox);
	
	c2_return_val_if_fail (mailbox, -1, C2EDATA);

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
#ifdef USE_DEBUG
		default:
			g_print ("Request for unsupported mailbox in %s:%s:%d: %d\n",
							__PRETTY_FUNCTION__, __FILE__, __LINE__, mailbox->type);
			return -1;
#endif
	}

	return func (mailbox);
}

/**
 * c2_db_messages
 * @db: Database descriptor.
 *
 * This function will return the number of
 * messages in a loaded database.
 *
 * Return Value:
 * Messages in db.
 **/
gint
c2_db_messages (const C2Db *db)
{
	const C2Db *l;
	gint i;
	
	c2_return_val_if_fail (db, 0, C2EDATA);

	for (l = db, i = 0; l != NULL; i++, l = l->next);
	return i;
}

/**
 * c2_db_message_add
 * @db: DB Descriptor (head).
 * @message: Message to be added.
 * @row: Row where to insert the message (-1 for the last one).
 * 
 * Will add a message in the specified database.
 *
 * Return Value:
 * The node where the message has been stored.
 **/
C2Db *
c2_db_message_add (C2Db *db, const C2Message *message, gint row)
{
	return 0;
}

void
c2_db_message_set_state (C2Db *db, gint row, C2MessageState state)
{
}

void
c2_db_message_swap_mark (C2Db *db, gint row)
{
}

gint
c2_db_message_remove (C2Db *db, gint row)
{
	return 0;
}

void
c2_db_message_load (C2Db *db)
{
	c2_return_if_fail (db, C2EDATA);

	db->message = *c2_db_message_get (db->mailbox->db, db->position);
	C2_MESSAGE (db)->mime = C2_MIME (c2_mime_new (C2_MESSAGE (db)));
}

C2Message *
c2_db_message_get (C2Db *db, gint row)
{
	C2Message *message = NULL;
	C2Db *l;
	gint mid, i;
	
	/* Get the mid of the message */
	for (l = db, i = 0; i < row && l != NULL; i++, l = l->next);
	c2_return_val_if_fail (l, NULL, C2EDATA);

	if (l)
	{
		return C2_MESSAGE (l);
	}

	mid = l->mid;
	
	switch (db->mailbox->type)
	{
		case C2_MAILBOX_CRONOSII:
/*			if ((message = c2_db_cronosII_message_get (db, mid)))
				l->message = *message;*/
			break;
		case C2_MAILBOX_IMAP:
			/* TODO message = c2_db_message_get_imap (db, mid); TODO */
			break;
	}
	
	return message;
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

	c2_message_set_message (message, string);
	g_free (string);

	return message;
}

/**
 * c2_db_message_search_by_mid
 * @db_d: DB descriptor.
 * @mid: mid to search.
 *
 * Searchs in the database @db_d for the mid @mid.
 *
 * Return Value:
 * The row where @mid was found.
 **/
gint
c2_db_message_search_by_mid (const C2Db *db_d, gint mid)
{
	return 0;
}

/**
 * c2_db_archive
 * @mailbox: Mailbox to archive.
 *
 * This function will remove a db and
 * archive all mails in ~/.c2/archive
 **/
void
c2_db_archive (C2Mailbox *mailbox)
{
	/* 1. Make sure the Db is loaded */
	/* 2. Go in a loop from the first message to the last one
	 *    calling c2_db_spool_add_message.
	 * 3. Call c2_db_remove_structure.
	 */
}

gint
c2_db_create_structure (C2Mailbox *mailbox)
{
	gint (*func) (C2Mailbox *mailbox);
	
	switch (mailbox->type)
	{
		case C2_MAILBOX_CRONOSII:
			func = c2_db_cronosII_create_structure;
			break;
		case C2_MAILBOX_IMAP:
			/* TODO */
			return 0;
			break;
		case C2_MAILBOX_SPOOL:
			func = c2_db_spool_create_structure;
			break;
	}

	return func (mailbox);
}

void
c2_db_remove_structure (C2Mailbox *mailbox)
{
	void (*func) (C2Mailbox *mailbox);

	C2_DEBUG (mailbox->name);
	switch (mailbox->type)
	{
		case C2_MAILBOX_CRONOSII:
			func = c2_db_cronosII_remove_structure;
			break;
		case C2_MAILBOX_IMAP:
			/* TODO */
			return;
			break;
		case C2_MAILBOX_SPOOL:
			func = c2_db_spool_remove_structure;
			break;
	}

	func (mailbox);
}
