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
 */
#include <glib.h>
#include <config.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

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

#define  MOD	"Spool"
#define DMOD	TRUE

#define QUEUE_TIMEOUT	10
#define CHANGE_TIMEOUT	15
#define QUEUE_STRING	"queue"
#define UNKNOWN_ACCOUNT	"External Account"

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

struct _C2DbSpoolQueueItem
{
	C2DbSpoolQueueAction action;
	C2Db *db;
	gpointer edata;

	gint mid;
};

static gboolean
line_is_start_of_mail						(const gchar *line);

static void
c2_spool_queue_item_new						(C2Mailbox *mailbox, C2Db *db,
											 C2DbSpoolQueueAction action, gpointer edata);

static C2DbSpoolQueue *
queue_item_new								(C2DbSpoolQueue *queue, C2DbSpoolQueueAction action,
											 C2Db *db, gpointer edata, gint mid);

static C2DbSpoolQueue *
queue_get									(C2Mailbox *mailbox);

static void
queue_set									(C2Mailbox *mailbox, C2DbSpoolQueue *queue);

static void
queue_sort									(C2Mailbox *mailbox);

static void
queue_rid_of_ambigous_commands				(C2Mailbox *mailbox);

static gboolean
check_commit								(C2Mailbox *mailbox);

static void
commit										(C2Mailbox *mailbox);

static void
index_create								(C2Mailbox *mailbox);

static gboolean
index_is_sync								(C2Mailbox *mailbox);

static C2Db *
load_mail									(C2Mailbox *mailbox, FILE *spool, gint iPosition, gint iMid);

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
static void
c2_spool_queue_item_new (C2Mailbox *mailbox, C2Db *db, C2DbSpoolQueueAction action, gpointer edata)
{
	queue_set (mailbox, queue_item_new (queue_get (mailbox), action, db, edata, db ? db->mid : -1));
}

static C2DbSpoolQueue *
queue_item_new (C2DbSpoolQueue *queue, C2DbSpoolQueueAction action, C2Db *db, gpointer edata, gint mid)
{
	C2DbSpoolQueueItem *item/*, *li*/;
	
	/* This is done later... */
	/* Search the Queue looking for an action over this mid
	 * to see if we are cancelling an action that wasn't
	 * committed yet (i.e. unmarking a mail that the queue
	 * says it should be marked). */	

	item = g_new0 (C2DbSpoolQueueItem, 1);
	item->action = action;
	item->db = db;
	item->edata = edata;
	item->mid = mid;

	return g_list_append (queue, item);
}

static void
queue_idle (gpointer data)
{
	C2Mailbox *mailbox;
	
	mailbox = C2_MAILBOX (data);

	if (check_commit (mailbox))
		commit (mailbox);
}

static gint
queue_timeout (gpointer data)
{
	gtk_idle_add (queue_idle, data);

	return TRUE;
}

static gint
change_timeout (gpointer data)
{
	C2Mailbox *mailbox = C2_MAILBOX (data);

	c2_mutex_lock (&mailbox->lock);

	if (!index_is_sync (mailbox))
	{
		gtk_object_destroy (GTK_OBJECT (mailbox->db));
		mailbox->db = NULL;

		c2_db_spool_load (mailbox);
	}
	
	c2_mutex_unlock (&mailbox->lock);
	
	return TRUE;
}

static C2DbSpoolQueue *
queue_get (C2Mailbox *mailbox)
{
	return (C2DbSpoolQueue*) gtk_object_get_data (GTK_OBJECT (mailbox), QUEUE_STRING);
}

static void
queue_set (C2Mailbox *mailbox, C2DbSpoolQueue *queue)
{
	
	gtk_object_set_data (GTK_OBJECT (mailbox), QUEUE_STRING, (gpointer) queue);
}

static gint
queue_sorting_func (gconstpointer data1, gconstpointer data2)
{
	C2DbSpoolQueueItem *item1, *item2;

	item1 = (C2DbSpoolQueueItem*) data1;
	item2 = (C2DbSpoolQueueItem*) data2;

	if (item1->db->mid < item2->db->mid)
		return -1;
	if (item1->db->mid > item2->db->mid)
		return 1;
	
	/* Add ~ * */
	if (item1->action == C2_DB_SPOOL_QUEUE_ADD)
		return -1;
	
	/* * ~ Add */
	if (item2->action == C2_DB_SPOOL_QUEUE_ADD)
		return 1;

	/* Remove ~ * */
	if (item1->action == C2_DB_SPOOL_QUEUE_REMOVE)
		return -1;

	/* * ~ Remove */
	if (item2->action == C2_DB_SPOOL_QUEUE_REMOVE)
		return 1;

	/* State ~ * */
	if (item1->action == C2_DB_SPOOL_QUEUE_STATE)
		return -1;

	/* * ~ State */
	if (item2->action == C2_DB_SPOOL_QUEUE_STATE)
		return 1;

	/* Mark ~ * */
	if (item1->action == C2_DB_SPOOL_QUEUE_MARK)
		return -1;

	/* * ~ Mark */
	if (item2->action == C2_DB_SPOOL_QUEUE_MARK)
		return 1;

	g_assert_not_reached ();
	return 0;
}

static void
queue_sort (C2Mailbox *mailbox)
{
	/* Sort by:
	 *  1. MID (ascending),
	 *  2. Action (Add, Remove, State, Mark)
	 */
	queue_set (mailbox, g_list_sort (queue_get (mailbox), queue_sorting_func));
}

static void
queue_rid_of_ambigous_commands (C2Mailbox *mailbox)
{
	GList *queue, *l, *l2;
	C2DbSpoolQueueItem *item1, *item2;
	
	/* Get rid of ambigous commands (same db node with some of the following actions).
	 *  1. Add-Remove (Eliminate both);
	 *  2. State (Eliminate all but the last one)
	 *  3. Mark (Eliminate all but the last one)
	 */
	queue = queue_get (mailbox);

	for (l = queue; l; l = l->next)
	{
		for (l2 = l->next; l2; l2 = l2->next)
		{
			item1 = (C2DbSpoolQueueItem*) l->data;
			item2 = (C2DbSpoolQueueItem*) l2->data;

			/* Check if it is same mid */
			if (item1->db->mid != item2->db->mid)
				continue;

			if ((item1->action == C2_DB_SPOOL_QUEUE_ADD && item2->action == C2_DB_SPOOL_QUEUE_REMOVE) ||
				(item1->action == C2_DB_SPOOL_QUEUE_STATE && item2->action == C2_DB_SPOOL_QUEUE_STATE) ||
				(item1->action == C2_DB_SPOOL_QUEUE_MARK && item2->action == C2_DB_SPOOL_QUEUE_MARK))
			{
				g_free (item1);
				l->data = NULL;
				
				queue = g_list_remove_link (queue, l);
			}
		}
	}
}

/********************
 * COMMIT FUNCTIONS *
 ********************/
static gboolean
check_commit (C2Mailbox *mailbox)
{
	if (queue_get (mailbox))
		return TRUE;
	else
		return FALSE;

	return TRUE;
}

static void
commit (C2Mailbox *mailbox)
{
	C2DbSpoolQueue *queue, *l = NULL;
	FILE *spool, *tmp;
	gchar *tmp_path = NULL;
	C2DbSpoolQueueAction qaction = 0;
	C2Db *qdb = NULL;
	gpointer qedata;
	gint qmid;
	gint last_mail_proc, i;
	gchar *line;
	gint from_reached;
	
	/* First of all sort the list */
	queue_sort (mailbox);

	/* Now get rid of all ambigous commands */
	queue_rid_of_ambigous_commands (mailbox);
	
	/* Get the Queue */
	queue = queue_get (mailbox);

	/* Open the spool file */
	if (!(spool = fopen (mailbox->protocol.spool.path, "r")))
	{
		c2_error_object_set (GTK_OBJECT (mailbox), -errno);
		C2_DEBUG_(perror (tmp_path););
		fclose (spool);
		return;
	}
	
	/* Open a tmp file */
	tmp_path = c2_get_tmp_file (NULL);
	C2_PRINTD (MOD, "Temporal file '%s'\n", tmp_path);
	if (!(tmp = fopen (tmp_path, "w")))
	{
		c2_error_object_set (GTK_OBJECT (mailbox), -errno);
		C2_DEBUG_(perror (tmp_path););
		fclose (spool);
		g_free (tmp_path);
		return;
	}

	for (i = 1, last_mail_proc = 0, l = queue; i < c2_db_length (mailbox) || l;)
	{
		C2_PRINTD (MOD, "c2_db_length() = '%d' -- l = %s -- i = '%d'\n", c2_db_length (mailbox), l ? "TRUE" : "FALSE", i);

		/* Check if we should get a new item where to act */
		if (!qdb)
		{
			/* The pointer hasn't been initializated and
			 * there are more queued actions.
			 */
			if (l)
			{
				/* Get the data */
				C2DbSpoolQueueItem *item = (C2DbSpoolQueueItem*) l->data;
				
				qaction = item->action;
				qdb = item->db;
				qedata = item->edata;
				qmid = item->mid;

				g_free (item);
				l->data = NULL;
				l = l->next;
			}
		}

		/* Check if we reached the mail where we need to act */
		if (i == qmid)
		{
			C2_PRINTD (MOD, "qmid = '%d'\n", qmid);
			
			/* Is time to act according to the action specified */
			switch (qaction)
			{
				case C2_DB_SPOOL_QUEUE_ADD:
					/* We are going to add the message */
					{
						gchar *from;
						gchar *buf;
						gchar timestamp[80];
						C2Message *message;
						struct tm *tm;
						const gchar *ptr;
						gchar *line;
						
						message = qdb->message;
						from = c2_message_get_header_field (message, "From:");
						
						if (from)
						{
							buf = c2_str_get_email (from);
							g_free (from);
							
							if (buf)
								from = buf;
							else
								from = g_strdup ("(nobody)");
						} else
						{
							from = g_strdup ("(nobody)");
						}
						
						tm = localtime (&qdb->date);
						
						setlocale (LC_ALL, "C");
						strftime (timestamp, 80, "%a %b %d %H:%M:%S %Y", tm);
						setlocale (LC_ALL, "");
						
						fprintf (tmp, "From %s %s\n", from, timestamp);

						/* Write the header */
						for (ptr = message->header;;)
						{
							if (!(line = c2_str_get_line (ptr)))
								break;

							ptr += strlen (line);

							if (c2_strneq (line, "X-CronosII-Account:", 20) ||
							    c2_strneq (line, "X-CronosII-State:", 7) ||
							    c2_strneq (line, "X-CronosII-Mark:", 16))
								continue;

							fprintf (tmp, "%s", line);
							g_free (line);
						}
						
						fprintf (tmp, "X-CronosII-State: %d\n", qdb->state);
						fprintf (tmp, "X-CronosII-Mark: %d\n", qdb->mark);
						fprintf (tmp, "X-CronosII-Account: %s\n", qdb->account);
						fprintf (tmp, "\n");
						fprintf (tmp, "%s", message->body);
					}
					
					break;
				case C2_DB_SPOOL_QUEUE_REMOVE:
					C2_PRINTD (MOD, "action = C2_DB_SPOOL_QUEUE_REMOVE\n");
					break;
				case C2_DB_SPOOL_QUEUE_MARK:
					C2_PRINTD (MOD, "action = C2_DB_SPOOL_QUEUE_MARK\n");
					break;
				case C2_DB_SPOOL_QUEUE_STATE:
					C2_PRINTD (MOD, "action = C2_DB_SPOOL_QUEUE_STATE\n");
					break;
				default:
					g_assert_not_reached ();
			}

			/* Now is time to unref the C2Db and the C2Message */
		} else
		{
			/* We are going to write until we reach the next 'From ' */
			for (from_reached = 0;;)
			{
				if (!(line = c2_fd_get_line (spool)))
					break;
				
				if (c2_strneq (line, "From ", 5))
				{
					if (!from_reached)
					{
						from_reached++;
					} else
					{
						/* We reached a new mail, get back and finish the for */
						fseek (spool, -(strlen (line)+1), SEEK_CUR);
						
						g_free (line);
						break;
					}
				}
				
				/* Write */
				fprintf (tmp, "%s\n", line);
				g_free (line);
			}
		}
	}
	
	/* Close the spool file */
	/* Close the tmp file */
	fclose (tmp);

	/* Overlap the spool file with the tmp file */
	if (c2_file_binary_move (tmp_path, mailbox->protocol.spool.path) < 0)
	{
		C2_DEBUG_ (perror ("c2_file_binary_move()"););
	}

	/* Rebuild the Index file */
	if (queue)
		index_create (mailbox);

	g_list_free (queue);
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
		C2_PRINTD (MOD, "spool file failed\n");
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
		C2_PRINTD (MOD, "index path = '%s'\n", index_path);
		C2_DEBUG_ (perror ("index_create()"););
		g_free (index_path);
		return;
	}
	g_free (index_path);

	/* Go through the spool file */
	for (;;)
	{
		if (!(line = c2_fd_get_line (spool)))
			break;

		if (line_is_start_of_mail (line))
		{
			/* Write in the index file the line where the mail we found
			 * begins.
			 */
			C2_PRINTD (MOD, "Start of mail: '%s'\n", line);
			fprintf (index, "%016x\n", ftell (spool)-strlen (line)-1);
		}

		g_free (line);
	}

	/* Add a line with the last position of the spool file */
	fprintf (index, "%016x", ftell (spool));

	fclose (index);
	fclose (spool);
}

static gboolean
index_is_sync (C2Mailbox *mailbox)
{
	FILE *index;
	struct stat stbf;
	gchar *path, buffer[17];
	gint sSize, iSize;

	/* Get the size of the spool file */
	stat (mailbox->protocol.spool.path, &stbf);
	sSize = stbf.st_size;
	
	/* Open the index file */
	index_file (mailbox, path);
	if (!(index = fopen (path, "r")))
	{
		g_free (path);
		C2_DEBUG_ (perror ("fopen"););
		return FALSE;
	}
	g_free (path);

	/* Move 17 bytes back from the end of the file */
	if (fseek (index, -16, SEEK_END) < 0)
	{
		fclose (index);
		return FALSE;
	}

	fread (buffer, sizeof (gchar), 17, index);
	buffer[17] = 0;

	sscanf (buffer, "%x", &iSize);

	if (iSize != sSize)
		return FALSE;

	return TRUE;
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
		C2_PRINTD (MOD, "spool path = '%s'\n",
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
		C2_PRINTD (MOD, "index path = '%s'\n",
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
	return TRUE;
}

gboolean
c2_db_spool_remove_structure (C2Mailbox *mailbox)
{
	return TRUE;
}

void
c2_db_spool_compact (C2Mailbox *mailbox, size_t *cybtes, size_t *tbytes)
{
}

void
c2_db_spool_freeze (C2Mailbox *mailbox)
{
	/* Perhaps we can make a good use of this crap? */
	L
}

void
c2_db_spool_thaw (C2Mailbox *mailbox)
{
	L
}

gint
c2_db_spool_load (C2Mailbox *mailbox)
{
	FILE *index, *spool;
	C2Db *current_db = NULL, *next_db;
	gchar *path, buffer[17];
	gint iFilePosition, iPosition;
	
	/* Test if the index is in sync */
	if (!index_is_sync (mailbox))
		index_create (mailbox);

	/* Start the loading */
	
	/* Open the index file */
	index_file (mailbox, path);
	if (!(index = fopen (path, "r")))
	{
		C2_PRINTD (MOD, "Failed to open the index file\n");
		C2_DEBUG_ (perror (path););
		g_free (path);
		
		return -1;
	}
	g_free (path);

	/* Open the spool file */
	if (!(spool = fopen (mailbox->protocol.spool.path, "a+")))
	{
		C2_PRINTD (MOD, "Failed to open the spool file\n");
		C2_DEBUG_ (perror (path););
		fclose (index);

		return -1;
	}

	buffer[16] = 0;

	/* Go through the files */
	for (iPosition = 0;; iPosition++)
	{
		if (fread (buffer, sizeof (gchar), 16, index) <= 0)
			break;

		/* Move the fd to the next line */
		if (fseek (index, 1, SEEK_CUR) < 0)
		{
			/* Break here, we don't want the last number,
			 * since is just a control number, it doesn't
			 * indicates the position of the start of a mail.
			 */
			break;
		}
		
		sscanf (buffer, "%x", &iFilePosition);

		/* Go to the position specified by the index file in the
		 * spool file.
		 */
		fseek (spool, iFilePosition, SEEK_SET);

		/* Load the mail */
		if (!(next_db = load_mail (mailbox, spool, iPosition+1, iFilePosition)))
			break;

		/* Append the mail */
		if (current_db)
			current_db->next = next_db;
		
		current_db = next_db;
	}

	/* Close the index file */
	fclose (index);

	/* Close the spool file */
	fclose (spool);

	/* Add the Queue Committing timeout */
	gtk_timeout_add (C2_SECONDS_IN_MS (QUEUE_TIMEOUT), queue_timeout, (gpointer) mailbox);

	/* Add a timeout that checks that the spool wasn't changed by a third party */
	gtk_timeout_add (C2_SECONDS_IN_MS (CHANGE_TIMEOUT), change_timeout, (gpointer) mailbox);

	return iPosition;
}

static C2Db *
load_mail (C2Mailbox *mailbox, FILE *spool, gint iPosition, gint iMid)
{
	C2Db *db;
	gchar *cSubject=NULL,
	      *cFrom=NULL,
		  *cAccount=NULL,
		  cMark=FALSE,
		  cState=C2_MESSAGE_READED;
	time_t tDate = -1;
	gchar *cLine, **cCurrentHeader=NULL, *cPtr;
	gint iLines;
	gboolean has_state = FALSE;

	for (iLines = 0;; iLines++)
	{
		if (!(cLine = c2_fd_get_line (spool)))
			return NULL;

		if (!iLines)
		{
			/* Here we have to get the date */
			gint iSpaces=0, iLastWasSpace=0;

			/* Move two words ahead */
			for (cPtr = cLine; *cPtr != '\0'; cPtr++)
			{
				if (*cPtr == ' ')
					iLastWasSpace=1;
				else if (iLastWasSpace)
				{
					iSpaces++;
					iLastWasSpace=0;

					if (iSpaces == 2)
						break;
				}
			}

			/* We should be pointing to the start of the date */
			if (cPtr)
				tDate = c2_date_parse_fmt3 (cPtr);
		} else
		{
			cPtr = cLine;
			
			/* Here we might get all the data */
			if (c2_strneq (cLine, "From:", 5)) /* From */
			{
				cCurrentHeader = &cFrom;
				cPtr += 5;
			} else if (c2_strneq (cLine, "Subject:", 8)) /* Subject */
			{
				cCurrentHeader = &cSubject;
				cPtr += 8;
			} else if (c2_strneq (cLine, "X-CronosII-Account:", 19)) /* Account */
			{
				cCurrentHeader = &cAccount;
				cPtr += 19;
			} else if (c2_strneq (cLine, "X-CronosII-Mark:", 16))
			{
				cMark = atoi ((cPtr+17));
				goto noheader;
			} else if (c2_strneq (cLine, "X-CronosII-State:", 17))
			{
				cState = atoi ((cPtr+18));
				has_state = TRUE;
				goto noheader;
			} else if (!has_state && c2_strneq (cLine, "Status:", 7))
			{
				/* Move through the spaces */
				for (cPtr += 7; *cPtr == ' ' && *cPtr != '\0'; cPtr++)
					;
				
				if (c2_streq (cPtr, "O"))
					cState = C2_MESSAGE_UNREADED;
				else if (c2_streq (cPtr, "RO"))
					cState = C2_MESSAGE_READED;
			} else if (tDate == -1 && c2_strneq (cLine, "Date:", 5)) /* Date */
			{
				/* In case the parsing of the date has failed we
				 * can use this field.
				 */
			} else if ((cCurrentHeader) &&
					   (*cLine == ' ' || *cLine == '\t')) /* More data for the previous header */
			{
				gint iLen = strlen (*cCurrentHeader)+strlen (cPtr);
				gchar *cTmp = g_realloc (*cCurrentHeader, iLen);

				strcpy (cTmp, *cCurrentHeader);
				strcat (cTmp, cPtr);
				cTmp[iLen] = 0;
				*cCurrentHeader = cTmp;
			} else if (c2_streq (cLine, "") ||
					   ((*cLine != '\t' && *cLine != ' ') && !strchr (cLine, ':'))) /* End of header */
			{
				g_free (cLine);
				break;
			} else /* Nothing matched */
			{
noheader:
				cCurrentHeader = NULL;
			}

			if (cCurrentHeader)
			{
				/* Move through the spaces */
				for (; *cPtr == ' ' && *cPtr != '\0'; cPtr++)
					;

				if (!cPtr)
					break;
				
				*cCurrentHeader = g_strdup (cPtr);
			}
		}

		g_free (cLine);
	}

	/* Validate the account */
	if (!cAccount)
		cAccount = g_strdup (UNKNOWN_ACCOUNT);

	/* Create the DB */
	db = c2_db_new (mailbox, cMark, cSubject, cFrom,
					cAccount, tDate, iMid, iPosition);
	db->state = cState;

	return db;
}

gboolean
c2_db_spool_message_add (C2Mailbox *mailbox, C2Db *db)
{
	/* We have to let know the object that we are using
	 * it, so it doesn't gets freed.
	 */
	gtk_object_ref (GTK_OBJECT (db));

	/* Set the mid */
	if (!c2_db_is_load (mailbox))
		c2_db_load (mailbox);

	gtk_object_ref (GTK_OBJECT (db->message));
	
	if (mailbox->db)
		db->mid = mailbox->db->prev->mid+1;
	else
		db->mid = 1;

	/* Add to the queue */
	c2_spool_queue_item_new (mailbox, db, C2_DB_SPOOL_QUEUE_ADD, NULL);

	return TRUE;
}

gboolean
c2_db_spool_message_remove (C2Mailbox *mailbox, GList *list)
{
	GList *l;
	
	for (l = list; l; l = l->next)
		c2_spool_queue_item_new (mailbox, C2_DB (list->data), C2_DB_SPOOL_QUEUE_REMOVE, NULL);

	return TRUE;
}

void
c2_db_spool_message_set_state (C2Db *db, C2MessageState state)
{
	c2_spool_queue_item_new (db->mailbox, db, C2_DB_SPOOL_QUEUE_STATE, (gpointer) state);
}

void
c2_db_spool_message_set_mark (C2Db *db, gboolean mark)
{
	c2_spool_queue_item_new (db->mailbox, db, C2_DB_SPOOL_QUEUE_MARK, (gpointer) mark);
}

C2Message *
c2_db_spool_load_message (C2Db *db)
{
	FILE *fd;
	C2Message *message;
	gchar *chunk = NULL, c;
	gint iLength=0;

	/* Open the spool file */
	if (!(fd = fopen (db->mailbox->protocol.spool.path, "r")))
	{
		c2_error_set (-errno);
		return NULL;
	}

	/* Go to the beggining of the mail */
	fseek (fd, db->mid, SEEK_SET);

	/* Move one line forward (we don't mind about the 'From em@il...' line */
	do
	{
		c = fgetc (fd);
		iLength++;
	} while (c != '\n' && c != '\0');

	if (c == '\0')
	{
		g_free (chunk);
		return NULL;
	}

	/* Alloc */
	if (c2_db_is_last (db))
	{
		/* Go to the end of the file to see until where we have
		 * to load.
		 */
		fseek (fd, 0, SEEK_END);
		iLength = ftell (fd) - db->mid;
		fseek (fd, db->mid, SEEK_SET);
	} else
		iLength = db->next->mid - db->mid - iLength;

	chunk = g_new0 (gchar, iLength-1);

	/* Read the data */
	fread (chunk, sizeof (gchar), iLength-1, fd);
	
	/* Create the message */
	message = c2_message_new ();
	c2_message_set_message (message, chunk);

	g_free (chunk);

	fclose (fd);

	return message;
}
