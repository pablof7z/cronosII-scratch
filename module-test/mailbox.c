/*  Cronos II
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

#include "libmodules/utils.h"
#include "libmodules/mailbox.h"

static void
print_tree (C2Mailbox *head, gint indentation)
{
	gint i;
	C2Mailbox *l;

	for (l = head; l != NULL; l = l->next)
	{
		for (i = 0; i < indentation; i++)
			printf ("\t");
		printf ("+ %s\n", l->name);
		if (l->child)
			print_tree (l->child, indentation+1);
	}
}

int
main (int argc, char **argv)
{
	gint i;
	C2Mailbox *list = NULL;
	
	if (argc < 2)
	{
		printf ("Usage: %s \"self_id%%name%%parent_id\"\n"
				"You can write as many mailboxes as you wish\n"
				"\n"
				"Options:\n"
				"-aName Parent\tAdds a mailbox with name Name child of Parent\n", argv[0]);
		return 1;
	}

	for (i = 1; i < argc; i++)
	{
		if (c2_strneq (argv[i], "-a", 2))
		{
			C2Mailbox *mbox;
			
			gchar *name = g_strdup (argv[i++]+2);
			gchar *parent = argv[i];

			if (c2_streq (name, parent))
			{
				mbox = c2_mailbox_new (list, name, -1);
			} else
			{
				C2Mailbox *parent_mbox = c2_mailbox_search_name (list, parent);
				mbox = c2_mailbox_new (list, name, parent_mbox ? parent_mbox->id : -1);
			}
			list = c2_mailbox_append (list, mbox);
		} else
		{
			C2Mailbox *mbox;
			gchar *str = c2_str_replace_all (argv[i], "%", "\r");
			mbox = c2_mailbox_parse (str);
			list = c2_mailbox_append (list, mbox);
			g_free (str);
		}
	}

	print_tree (list, 0);

	return 0;
}
