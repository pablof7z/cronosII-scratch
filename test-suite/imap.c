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
#include <gtk/gtk.h>
#include <pthread.h>

#include <libcronosII/imap.h>
#include <libcronosII/message.h>
#include <libcronosII/error.h>
#include <libcronosII/utils.h>

/* file to test the c2 imap module  -- 
 * by Bosko Blagojevic <falling@users.sourcforge.net> */

void 
print_imap_tree(C2IMAP *imap, C2Mailbox *parent, gchar *tab)
{
	C2Mailbox *ptr;
	
	if(!parent)
		parent = imap->mailboxes;
	else
		parent = parent->child;
	
	for(ptr = parent; ptr; ptr = ptr->next)
	{
		printf("%s%s\n", tab ? tab : "", ptr->name);
		if(ptr->child)
		{
			if(!tab)
				print_imap_tree(imap, ptr, g_strdup("+---"));
			else 
				print_imap_tree(imap, ptr, g_strconcat(tab, "---", NULL));
		}
	}
	
	if(tab) g_free(tab);
}

void
run_imap(C2IMAP *imap)
{	
	GList *list = NULL;
	C2Mailbox *mailbox;
	
	if(c2_imap_init(imap) < 0)
	{	
		printf("failed to login\n");
		exit(-1);
	}
	
	if(c2_imap_populate_folders(imap) < 0)
	{
		printf("failed to populate the IMAP Folders tree");
		exit(-1);
	}
 
	printf("listing folders: \n");
	print_imap_tree(imap, NULL, NULL);

	printf("\nCreating top-level folder CronosII...");

	if(!(mailbox = c2_mailbox_new_with_parent(&imap->mailboxes, "CronosII", "3-2-0", C2_MAILBOX_IMAP,
								 C2_MAILBOX_SORT_DATE, GTK_SORT_ASCENDING, imap, TRUE)))
	{
		printf("failure!\n");
		printf("Error was: %s\n", gtk_object_get_data(GTK_OBJECT(imap), "error"));
		exit(-1);
	}
	printf("success!\n\n");
	
	printf("listing folders: \n");
	print_imap_tree(imap, NULL, NULL);
	
	printf("Deleting top-level folder CronosII...");
  if(c2_mailbox_remove(&imap->mailboxes, mailbox))
	{
		printf("failure!\n");
		printf("Error was: %s", gtk_object_get_data(GTK_OBJECT(imap), "error"));
		exit(-1);
	}
  printf("success!\n");
	
	printf("\nCronosII IMAP capability testing completed successfully!\n");
	exit(0);
}

gint
main (gint argc, gchar **argv)
{
	gchar *host, *user, *pass;
	gint port;
	pthread_t thread;
	C2IMAP *imap;

	gtk_init(&argc, &argv);
	
	printf("Welcome to the IMAP module of the C2 Engine Test-Suite\n");
	
	if (argc > 4)
	{
		host = g_strdup (argv[1]);
		port = atoi (argv[2]);
		user = g_strdup (argv[3]);
		pass = g_strdup (argv[4]);
	} else
	{
		host = g_strdup ("192.168.1.2");
		port = 143;
		user = g_strdup ("falling");
		pass = g_strdup ("password");
	}
	imap = c2_imap_new (host, port, user, pass, NULL, C2_IMAP_AUTHENTICATION_PLAINTEXT, FALSE);
	
	pthread_create(&thread, NULL, (void*)run_imap, imap);
	
	gtk_main();
	gtk_object_destroy(GTK_OBJECT(imap));
	
	return 0;
}
