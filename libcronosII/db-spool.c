/*  Cronos II - The GNOME mail client
 *  Copyright (C) 2000-2001 Pablo Fernández López
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
 * 		* Pablo Fernández López
 * Code of this file by:
 * 		* Pablo Fernández López
 */
#include <glib.h>
#include <config.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "i18n.h"
#include "error.h"
#include "db.h"
#include "db-spool.h"
#include "mailbox.h"
#include "utils.h"
#include "utils-date.h"

/*hmmm this is almost beautifull in its minimalism -pete ;)*/
/*yeah, is the shortest module I ever see! but it doesn't have any bugs! :) -pablo */
/*Ah you coders bugs bugs bugs. No code =nobugs. Code bad code use memory. :(*/
/*Oh, yes, the perfect armony of "the 0 bytes length of the file", just perfect :) -pablo */
/*Cool! I´m coding from Paris with my new laptop! -pablo */
/*Crap! I'm coding in Buenos Aires without electricity (it went off) -pablo */
/*Cool! I'm coding from Madrid, missing and loving my very best friends Vicky and Sol! -pablo */

#define QUEUE_TIMEOUT	60
#define QUEUE_STRING	"queue"

#define index_file(mbox, value) \
	{ \
		gchar *hd = g_get_home_dir (); \
		value = g_strconcat (hd, "/.c2/spool/", mbox->name, NULL); \
	}

typedef GList C2DbSpoolQueue;
typedef struct _C2DbSpoolQueueItem C2DbSpoolQueueItem;
typedef enum _C2DbSpoolQueueAction C2DbSpoolQueueAction;

enum _C2DbSpoolQueueAction
{
	C2_DB_SPOOL_QUEUE_ADD,
	C2_DB_SPOOL_QUEUE_REMOVE,
	C2_DB_SPOOL_QUEUE_MARK,
	C2_DB_SPOOL_QUEUE_STATE
};

struct _C2DbSpoolQueue
{
	C2DbSpoolQueueAction action;

	gpointer edata;

	gint mid;
};

static C2DbSpoolQueue *
queue_item_new (C2DbSpoolQueue *queue, C2DbSpoolQueueAction action, gpointer edata, gint mid);

static gboolean
line_is_start_of_mail						(const gchar *line);

static gboolean
check_commit								(C2Mailbox *mailbox);

static void
commit										(C2Mailbox *mailbox);

/*********************
 * UTILITY FUNCTIONS *
 *********************/
static gboolean
line_is_start_of_mail (const gchar *line)
{
	const gchar *ptr;

	ptr = line;
	
	/* Check that the line starts with "From " */
	if (c2_strnne (ptr, "From ", 5))
		return FALSE;
	
	/* Go through the blank spaces */
	for (ptr += 5; *ptr == ' ' && *ptr != '\0'; ptr++)
		;
	if (!ptr)
		return FALSE;

	/* Should come something that might look like an
	 * email address, but it might also be a local username,
	 * so we will just move one word forward ignoring what
	 * it looks like.
	 */
	for (; *ptr != ' ' && *ptr != '\0'; ptr++)
		;
	if (!ptr)
		return FALSE;
	
	/* Go through the blank spaces */
	for (; *ptr == ' ' && *ptr != '\0'; ptr++)
		;
	if (!ptr)
		return FALSE;

	if (c2_date_parse_fmt3 (ptr) == -1)
		return FALSE;

	/* Check that there's nothing else */
	ptr += 25;

	if (strlen (ptr))
		return FALSE;

	return TRUE;
}

/*******************
 * QUEUE FUNCTIONS *
 *******************/
static C2DbSpoolQueue *
queue_item_new (C2DbSpoolQueue *queue, C2DbSpoolQueueAction action, gpointer edata, gint mid)
{
	C2DbSpoolQueueItem *item;

	item = g_new0 (C2DbSpoolQueueItem, 1);
	item->action = action;
	item->edata = edata;
	item->mid = mid;

	/* Search the Queue looking for an action over this mid
	 * to see if we are cancelling an action that wasn't
	 * committed yet (i.e. unmarking a mail that the queue
	 * says it should be marked). */

	return g_list_append (queue, item);
}


/********************
 * COMMIT FUNCTIONS *
 ********************/
static gboolean
check_commit (C2Mailbox *mailbox)
{
	
}

static void
commit (C2Mailbox *mailbox)
{
		
}

/****************************
 * INDEX CREATION FUNCTIONS *
 ****************************/
static void
index_create (C2Mailbox *mailbox)
{
	FILE *spool, *index;
	gchar *index_path, *line;

	/* Open the spool file */
	if (!(spool = fopen (mailbox->protocol.spool.path, "a+")))
	{
		c2_error_object_set (GTK_OBJECT (mailbox), -errno);
		C2_PRINTD ("spool -- index_create() -- spool file failed\n");
		C2_DEBUG_ (perror ("index_create()"););
		return;
	}
	fseek (spool, 0, SEEK_SET);

	/* Open the index file */
	index_file (mailbox, index_path);
	if (!(index = fopen (index_path, "w")))
	{
		fclose (spool);
		c2_error_object_set (GTK_OBJECT (mailbox), -errno);
		C2_PRINTD ("spool -- index_create() -- index path = '%s'\n", index_path);
		C2_DEBUG_ (perror ("index_create()"););
		g_free (index_path);
		return;
	}
	g_free (index_path);

	/* Go through the spool file */
	C2_PRINTD ("spool -- index_create() -- starting for on '%s'\n", mailbox->protocol.spool.path);
	for (;;)
	{
		if (!(line = c2_fd_get_line (spool)))
			break;

		if (line_is_start_of_mail (line))
		{
			if (ftell (spool) < 0)
			{
				C2_PRINTD ("spool -- index_create() -- ftell (spool) failed\n");
				C2_DEBUG_ (perror ("index_create()"););
			}
			
			/* Write in the index file the line where the mail we found
			 * begins.
			 */
			fprintf (index, "%016x\n", ftell (spool)-strlen (line)-1);
			C2_PRINTD ("spool -- index_create() -- Adding 0x%x\n", ftell (spool)-strlen (line)-1);
		}

		g_free (line);
	}
	C2_PRINTD ("spool -- index_create() -- ending for\n");

	fclose (index);
	fclose (spool);
}

/***********************
 * STRUCTURE FUNCTIONS *
 ***********************/
gboolean
c2_db_spool_create_structure (C2Mailbox *mailbox)
{
	int fd;
	gchar *path;

	/* Try to create the spool file */
	if (!(fd = open (mailbox->protocol.spool.path, O_CREAT, S_IRUSR | S_IWUSR)))
	{
		c2_error_object_set (GTK_OBJECT (mailbox), -errno);
		C2_PRINTD ("c2_db_spool_create_structure() -- spool path = '%s'\n",
						mailbox->protocol.spool.path);
		C2_DEBUG_ (perror ("c2_db_spool_create_structure()"););
		return FALSE;
	}
	close (fd);

	/* Try to create the index file */
	index_file (mailbox, path);
	if (!(fd = open (mailbox->protocol.spool.path, O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR)))
	{
		c2_error_object_set (GTK_OBJECT (mailbox), -errno);
		C2_PRINTD ("c2_db_spool_create_structure() -- index path = '%s'\n",
						path);
		C2_DEBUG_ (perror ("c2_db_spool_create_structure()"););
		g_free (path);
		return FALSE;
	}
	g_free (path);

	return TRUE;
}

gboolean
c2_db_spool_update_structure (C2Mailbox *mailbox)
{
}

gboolean
c2_db_spool_remove_structure (C2Mailbox *mailbox)
{
}

gint
c2_db_spool_load (C2Mailbox *mailbox)
{
	C2_PRINTD ("spool -- c2_db_spool_load() -- Start\n");
	index_create (mailbox);
	C2_PRINTD ("spool -- c2_db_spool_load() -- End\n");
}

void
c2_db_spool_message_add (C2Mailbox *mailbox, C2Db *db)
{
}

void
c2_db_spool_message_remove (C2Mailbox *mailbox, C2Db *db, gint n)
{
}

void
c2_db_spool_message_set_state (C2Db *db, C2MessageState state)
{
}

void
c2_db_spool_message_set_mark (C2Db *db, gboolean mark)
{
}

C2Message *
c2_db_spool_load_message (C2Db *db)
{
}
