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
run_imap(C2IMAP *imap)
{	
	GList *list = NULL;
	
	if(c2_imap_init(imap) < 0)
	{	
		printf("failed to login\n");
		return;
	}
	
	if(c2_imap_get_folder_list(imap, &list, "", "*") < 0)
	{
		printf("failed to get folder list\n");
		return;
	}
	
	if(c2_imap_create_folder(imap, NULL, "CronosII") < 0)
	{
		printf("failed to create folder \"CronosII\"\n");
		return;
	}
	printf("created folder \"CronosII\"\n");
	printf("listing folders...\n");
	
	if(c2_imap_get_folder_list(imap, &list, "", "*") < 0)
	{
		printf("failed to get folder list\n");
		return;
	}
	
	if(c2_imap_delete_folder(imap, "CronosII") < 0)
	{
		printf("failed to delete folder \"CronosII\"\n");
		return;
	}
	printf("deleted folder \"CronosII\"\n");
	printf("listing folders...\n");
	
	if(c2_imap_get_folder_list(imap, &list, "", "*") < 0)
	{
		printf("failed to get folder list\n");
		return;
	}
		
	printf("\nCronosII IMAP capability testing Completed successfully!\n");
}

gint
main (gint argc, gchar **argv)
{
	pthread_t thread;
	C2IMAP *imap;

	gtk_init(&argc, &argv);
	
	printf("Welcome to the IMAP module of the C2 Engine Test-Suite\n"
				 "Hit Crtl+C to exit program once tests are complete\n");
	
	imap = c2_imap_new("192.168.1.2", 143, "falling", "password", C2_IMAP_AUTHENTICATION_PLAINTEXT, FALSE);
	
	pthread_create(&thread, NULL, (void*)run_imap, imap);
	
	gtk_main();
	gtk_object_destroy(GTK_OBJECT(imap));
	
	return 0;
}
