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
#include <stdarg.h>
#include <gtk/gtk.h>

#include <libcronosII/db.h>
#include <libcronosII/error.h>
#include <libcronosII/utils.h>

static void
cmnd_close									(GList *list);

static void
create_args									(const gchar *cmnd, GList **list);

static gchar *
get_cmnd									(void);

C2Mailbox *mailbox = NULL;

static void
cmnd_open (GList *list)
{
	gchar *name;

	name = (gchar*) g_list_nth_data (list, 0);

	if (!name || !strlen (name))
	{
		printf ("Name not specified.\n");
		return;
	}

	mailbox = NULL;
	mailbox = c2_mailbox_new (&mailbox, name,
						c2_mailbox_create_id_from_parent (mailbox, NULL), C2_MAILBOX_CRONOSII, 0, 0);

	if (!C2_IS_MAILBOX (mailbox))
	{
		printf ("Error opening mailbox.\n");
		cmnd_close (NULL);
		return;
	}

	if (c2_mailbox_load_db (mailbox) < 0)
	{
		printf ("Error loading mailbox.\n");
		cmnd_close (NULL);
		return;
	}
}

static void
cmnd_close (GList *list)
{
	gtk_object_destroy (GTK_OBJECT (mailbox));
	mailbox = NULL;
}

static void
cmnd_mstat (GList *list)
{
	const gchar *name;
	gint mails, unreaded;

	if (C2_IS_MAILBOX (mailbox))
	{
		name = mailbox->name;
		mails = c2_db_length (mailbox);
		unreaded = c2_db_length_type (mailbox, C2_MESSAGE_UNREADED);
	} else
	{
		name = "No mailbox open";
		mails = unreaded = 0;
	}
	
	printf (" Name: %15s\n"
			"Mails: %15d\tNew Mails: %10d\n",
			name, mails, unreaded);
}

static void
cmnd_list (GList *list)
{
	C2Db *db;
	gint i = 0;

	db = mailbox->db;
	if (db)
	{
		do
		{
			gchar subject[40];

			memset (subject, 0, sizeof (subject));
			if (strlen (db->subject) > 39)
			{
				strncpy (subject, db->subject, 36);
				strcat (subject, "[..]");
			} else
				strcpy (subject, db->subject);
			subject[40] = 0;

			printf ("%02d.- %04d. %40s\n", i++, db->position, subject);
		} while (c2_db_lineal_next (db));
	}
}


static void
cmnd_retr (GList *list)
{
	C2Db *db = NULL;
	gchar *buf;
	gint n;

	buf = (gchar*) g_list_nth_data (list, 0);
	n = atoi (buf);

	db = c2_db_get_node (mailbox, n);
	if (c2_db_load_message (db) < 0)
	{
		printf ("Loading of message failed.\n");
		return;
	}

	gtk_object_ref (GTK_OBJECT (db->message));

	printf ("From: %s\n"
			"Subject: %s\n"
			"\n"
			"%s\n",
			c2_message_get_header_field (db->message, "\nFrom:"),
			c2_message_get_header_field (db->message, "\nSubject:"),
			g_strndup (db->message->body, 100));

	gtk_object_destroy (GTK_OBJECT (db->message));
}

static void
cmnd_dele (GList *list)
{
	GList *dlist = NULL;
	GList *l;

	for (l = list; l; l = g_list_next (l))
		dlist = g_list_append (dlist, atoi ((gchar*)l->data));

	printf ("[ About to delete %d mails ]\n\n", g_list_length (dlist));
	printf ("\n[ Returned with %d ]\n", c2_db_message_remove_list (mailbox, dlist));
	g_list_free (dlist);
}


static void
cmnd_quit (GList *list)
{
	printf ("Bye!\n");
	exit (0);
}

struct arg
{
	gchar *cmnd;
	void (*func) (GList *list);
} arg_table[] =
{
	"open", cmnd_open,
	"close", cmnd_close,
	"mstat", cmnd_mstat,
	"list", cmnd_list,
	
	"retr", cmnd_retr,
	"dele", cmnd_dele,
	
	"quit", cmnd_quit,
	NULL, NULL
};

gint
main (gint argc, gchar **argv)
{
	g_thread_init (NULL);
	gtk_init (&argc, &argv);

	for (;;)
	{
		gchar *cmnd = get_cmnd ();
		gchar *word;
		gint i;

		word = c2_str_get_word (0, cmnd, ' ');
		if (!strlen (word))
		{
			g_free (word);
			g_free (cmnd);
			continue;
		}
		
		for (i = 0;; i++)
		{
			if (c2_streq (arg_table[i].cmnd, word))
			{
				GList *list;
				
				create_args (cmnd, &list);
				arg_table[i].func (list);
				break;
			} else if (!arg_table[i].cmnd)
			{
				printf ("Command unknown: %s\n", word);
				break;
			}
		}

		g_free (word);
		g_free (cmnd);
	}

	return 0;
}

/* This function is really uneffective, but I don't give a shit */
static void
create_args (const gchar *cmnd, GList **list)
{
	gint i;
	gchar *word;

	*list = NULL;
	
	for (i = 1;; i++)
	{
		word = c2_str_get_word (i, cmnd, ' ');

		if (!strlen (word))
			break;

		*list = g_list_append (*list, word);
	}
}

static gchar *
get_cmnd (void)
{
	gchar buf[80];
	gchar ptr;
	gchar *cmnd = NULL;
	gint len = 0;

	printf ("db-test> ");
	fflush (stdout);

	buf[len] = 0;
	while (fread (&ptr, 1, sizeof (gchar), stdin) > 0)
	{
		if (ptr == '\n')
			break;

		buf[len+1] = 0;
		buf[len++] = ptr;

		if (len == 79)
		{
			if (cmnd)
			{
				gchar *tmp;

				tmp = g_strdup_printf ("%s%s", cmnd, buf);
				g_free (cmnd);
				cmnd = tmp;
			} else
				cmnd = g_strdup (buf);

			len = 0;
		}
	}

	if (cmnd)
	{
		gchar *tmp;

		tmp = g_strdup_printf ("%s%s", cmnd, buf);
		g_free (cmnd);
		cmnd = tmp;
	} else
		cmnd = g_strdup (buf);

	return cmnd;
}
