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
#include <stdio.h>
#include <glib.h>

#include <libcronosII/db.h>
#include <libcronosII/error.h>
#include <libcronosII/utils.h>

static C2Mailbox *mailbox = NULL;

static void
on_cmnd_new (const gchar *w1, const gchar *w2);

static void
on_cmnd_add (const gchar *w1, const gchar *w2);

static void
on_cmnd_remove (const gchar *w1, const gchar *w2);

static void
on_cmnd_load (const gchar *w1, const gchar *w2);

static void
on_cmnd_unload (const gchar *w1, const gchar *w2);

static void
on_cmnd_get (const gchar *w1, const gchar *w2);

static void
on_cmnd_field (const gchar *w1, const gchar *w2);

static void
on_cmnd_status (const gchar *w1, const gchar *w2);

static void
on_cmnd_make (const gchar *w1, const gchar *w2);

static void
on_cmnd_quit (const gchar *w1, const gchar *w2);

static void
on_cmnd_unknown (const gchar *cmnd, const gchar *w2);

static gchar *
get_command (void)
{
	gchar cmnd[80];
	g_print ("db> ");
	fgets (cmnd, sizeof (cmnd), stdin);
	cmnd[strlen (cmnd)-1] = '\0';
	return g_strdup (cmnd);
}

struct {
	gchar *cmnd;
	void (*func) (const gchar *w1, const gchar *w2);
} table[] =
{
	{ "new", on_cmnd_new			},
	{ "add", on_cmnd_add			},
	{ "remove", on_cmnd_remove		},
	{ "load", on_cmnd_load			},
	{ "unload", on_cmnd_unload		},
	{ "get", on_cmnd_get			},
	{ "field", on_cmnd_field		},
	{ "status", on_cmnd_status		},
	{ "make", on_cmnd_make			},
	{ "quit", on_cmnd_quit			},
	{ NULL, on_cmnd_unknown			}
};

gint
main (gint argc, gchar **argv)
{

	gtk_init (&argc, &argv);

	for (;;)
	{
		gchar *cmnd = get_command ();
		gchar *w1 = c2_str_get_word (0, cmnd, ' ');
		gchar *w2 = c2_str_get_word (1, cmnd, ' ');
		gchar *w3 = c2_str_get_word (2, cmnd, ' ');
		gint i;

		for (i = 0;; i++)
		{
			if (!table[i].cmnd)
			{
				table[i].func (cmnd, w3);
				break;
			}
			else if (c2_streq (table[i].cmnd, w1))
			{
				table[i].func (w2, w3);
				g_free (w1);
				g_free (w2);
				g_free (w3);
				g_free (cmnd);
				break;
			}
		}
	}
	return 0;
}

static void
on_cmnd_new (const gchar *w1, const gchar *w2)
{
	if (!mailbox)
		w2 = NULL;
	mailbox = c2_mailbox_new_with_parent (w1, w2, C2_MAILBOX_CRONOSII,
										C2_MAILBOX_SORT_DATE, GTK_SORT_ASCENDING);
}

static void
on_cmnd_add (const gchar *w1, const gchar *w2)
{
	if (mailbox)
		gtk_object_unref (GTK_OBJECT (mailbox));
	if (!strlen (w1))
	{
		g_print ("Usage: add mailbox-name.\n");
		return;
	}
	if (!(mailbox = c2_mailbox_new_with_parent (w1, NULL, C2_MAILBOX_CRONOSII,
					C2_MAILBOX_SORT_DATE, GTK_SORT_ASCENDING)))
		g_print ("Unable to load mailbox: %s\n", c2_error_get (c2_errno));
}

static void
on_cmnd_remove (const gchar *w1, const gchar *w2)
{
	if (mailbox)
		gtk_object_unref (GTK_OBJECT (mailbox));
	mailbox = NULL;
}

static void
on_cmnd_load (const gchar *w1, const gchar *w2)
{
	if (!mailbox)
	{
		g_print ("What do you want me to load?! I have no mailbox!\n");
		return;
	}
	if (mailbox->db)
	{
		g_print ("The mailbox has the db already loaded!\n");
		return;
	}
	if (!(mailbox->db = c2_db_new (mailbox)))
	{
		g_print ("Unable to load db: %s\n", c2_error_get (c2_errno));
		return;
	}
	g_print ("%d messages in database.\n", c2_db_messages (mailbox->db));
}

static void
on_cmnd_unload (const gchar *w1, const gchar *w2)
{
	if (!mailbox)
	{
		g_print ("What do you want me to unload?! I have no mailbox!\n");
		return;
	}
	if (!mailbox->db)
	{
		g_print ("The mailbox had the db unloaded already!\n");
		return;
	}
	gtk_object_unref (GTK_OBJECT (mailbox->db));
	printf ("<%d>\n", mailbox->db ? mailbox->db->mid : -100);
	mailbox->db = NULL;
}

static void
on_cmnd_get (const gchar *w1, const gchar *w2)
{
	C2Message *message;

	if (!mailbox)
	{
		g_print ("Where should I get the message from?! I have no mailbox!\n");
		return;
	}
	if (!mailbox->db)
	{
		g_print ("Oh yeah? Well don't u think u should at least load the db for me?!\n");
		return;
	}
	if (!(message = c2_db_message_get (mailbox->db, atoi (w1))))
	{
		g_print ("Unable to get message: %s\n", c2_error_get (c2_errno));
		return;
	}
	
	c2_message_get_message_body (message);
	g_print ("%s\n", message->body);
}

static void
on_cmnd_field (const gchar *w1, const gchar *w2)
{
	C2Message *message;
	gchar *field;

	if (!mailbox)
	{
		g_print ("Where should I get the message from?! I have no mailbox!\n");
		return;
	}
	if (!mailbox->db)
	{
		g_print ("Oh yeah? Well don't u think u should at least load the db for me?!\n");
		return;
	}
	if (!(message = c2_db_message_get (mailbox->db, atoi (w1))))
	{
		g_print ("Unable to get message: %s\n", c2_error_get (c2_errno));
		return;
	}
	
	field = c2_message_get_header_field (message, w2);
	if (!c2_streq (w2, "Date:"))
		g_print ("%s\n", field);
	else
	{
		time_t date;
		if ((date = c2_date_parse (field)) < 0)
			date = c2_date_parse_fmt2 (field);
		g_print ("%s (%d)\n", field, date);
	}
	g_free (field);
}

static void
on_cmnd_status (const gchar *w1, const gchar *w2)
{
	g_print ("Loaded mailbox: %s\n",
			mailbox ? mailbox->name : "none");
	if (mailbox)
		g_print (mailbox->db ? "Db is loaded.\n" : "Db is unloaded.\n");
}

static void
on_cmnd_make (const gchar *w1, const gchar *w2)
{
	system ("make db");
	if (fork ())
		system ("./db");
	else
		exit (0);
}

static void
on_cmnd_quit (const gchar *w1, const gchar *w2)
{
	exit (0);
}

static void
on_cmnd_unknown (const gchar *cmnd, const gchar *w2)
{
	system (cmnd);
}
