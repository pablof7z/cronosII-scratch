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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include <libcronosII/error.h>
#include <libcronosII/utils.h>
#include <libcronosII/mailbox.h>

static void
print_tree (C2Mailbox *mailbox)
{
	C2Mailbox *l;
	gint i;
	static gint indentation = -1;
	
	c2_return_if_fail (mailbox, C2EDATA);

	indentation++;

	for (l = mailbox; l != NULL; l = l->next)
	{
		for (i = 0; i < indentation; i++)
			g_print ("\t");

		g_print ("%s (%s)\n", l->name, l->id);

		if (l->child)
			print_tree (l->child);
	}

	indentation--;
}

gint
main (gint argc, gchar **argv)
{
	gint i;

	gtk_init (&argc, &argv);

	if (argc < 7)
	{
		g_print ("Usage: %s OPTIONS\n"
				 "-a Name Self_Id Type Sort_by Sort_type ...\n"
				 "\n", argv[0]);
		return 1;
	}

	for (i = 1; i < argc; i++)
	{
		if (c2_streq (argv[i], "-a"))
		{
			gchar *name = argv[++i];
			gchar *id = argv[++i];
			C2MailboxType type;
			C2MailboxSortBy sort_by;
			GtkSortType sort_type;

			gchar *server;
			gint port;
			gchar *user;
			gchar *pass;

			gchar *db;

			if (c2_streq (argv[++i], "C2_MAILBOX_CRONOSII"))
				type = C2_MAILBOX_CRONOSII;
			else if (c2_streq (argv[i], "C2_MAILBOX_IMAP"))
				type = C2_MAILBOX_IMAP;
#ifdef USE_MYSQL
			else if (c2_streq (argv[i], "C2_MAILBOX_MYSQL"))
				type = C2_MAILBOX_MYSQL;
#endif
			else
			{
				g_print ("Unsupported mailbox type: %s\n", argv[i]);
				return 1;
			}

			if (c2_streq (argv[++i], "C2_MAILBOX_SORT_STATUS"))
				sort_by = C2_MAILBOX_SORT_STATUS;
			else if (c2_streq (argv[i], "C2_MAILBOX_SORT_UNUSED1"))
				sort_by = C2_MAILBOX_SORT_UNUSED1;
			else if (c2_streq (argv[i], "C2_MAILBOX_SORT_UNUSED2"))
				sort_by = C2_MAILBOX_SORT_UNUSED2;
			else if (c2_streq (argv[i], "C2_MAILBOX_SORT_SUBJECT"))
				sort_by = C2_MAILBOX_SORT_SUBJECT;
			else if (c2_streq (argv[i], "C2_MAILBOX_SORT_FROM"))
				sort_by = C2_MAILBOX_SORT_FROM;
			else if (c2_streq (argv[i], "C2_MAILBOX_SORT_DATE"))
				sort_by = C2_MAILBOX_SORT_DATE;
			else if (c2_streq (argv[i], "C2_MAILBOX_SORT_ACCOUNT"))
				sort_by = C2_MAILBOX_SORT_ACCOUNT;
			else if (c2_streq (argv[i], "C2_MAILBOX_SORT_ACCOUNT"))
				sort_by = C2_MAILBOX_SORT_ACCOUNT;
			else if (c2_streq (argv[i], "C2_MAILBOX_SORT_MID"))
				sort_by = C2_MAILBOX_SORT_MID;
			else
			{
				g_print ("Unsupported sort column: %s\n", argv[i]);
				return 1;
			}

			if (c2_streq (argv[++i], "GTK_SORT_ASCENDING"))
				sort_type = GTK_SORT_ASCENDING;
			else if (c2_streq (argv[i], "GTK_SORT_DESCENDING"))
				sort_type = GTK_SORT_DESCENDING;
			else
			{
				g_print ("Unsupported sort type: %s\n", argv[i]);
				return 1;
			}

			switch (type)
			{
				case C2_MAILBOX_CRONOSII:
					c2_mailbox_new (name, id, type, sort_by, sort_type);
					break;
				case C2_MAILBOX_IMAP:
					server = argv[++i];
					port = atoi (argv[++i]);
					user = argv[++i];
					pass = argv[++i];
					c2_mailbox_new (name, id, type, sort_by, sort_type, server, port, user, pass);
					break;
				case C2_MAILBOX_MYSQL:
					server = argv[++i];
					port = atoi (argv[++i]);
					db = argv[++i];
					user = argv[++i];
					pass = argv[++i];
					c2_mailbox_new (name, id, type, sort_by, sort_type, server, port, db, user, pass);
					break;
			}
		}
	}

	print_tree (c2_mailbox_get_head ());

	c2_mailbox_remove (c2_mailbox_get_head ());

	print_tree (c2_mailbox_get_head ());

	return 0;
}
