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

#include <libcronosII/imap.h>
#include <libcronosII/message.h>
#include <libcronosII/error.h>
#include <libcronosII/utils.h>

/* file to test the c2 imap module  -- 
 * by Bosko Blagojevic <falling@users.sourcforge.net> */

gint
main (gint argc, gchar **argv)
{
	C2IMAP *imap;
	
	gtk_init(&argc, &argv);
	
	imap = c2_imap_new("192.168.1.2", 143, "user", "password", C2_IMAP_AUTHENTICATION_PLAINTEXT, FALSE);
	
	gtk_object_destroy(GTK_OBJECT(imap));
	
	return 0;
}
