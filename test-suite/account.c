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
#include <glib.h>
#include <gtk/gtk.h>

#include <libcronosII/account.h>

gint
main (gint argc, gchar **argv)
{
	C2Account *account;
	gchar *buf;

	gtk_init (&argc, &argv);

	printf ("Creating new account...\n");
	account = c2_account_new (C2_ACCOUNT_POP3, "Test Account", "test@localhost");
	printf ("Account created: %s, %s\n", account->name, account->email);

	printf ("Setting Full Name flag...\n");
	c2_account_set_extra_data (account, C2_ACCOUNT_KEY_FULL_NAME, GTK_TYPE_STRING, "User of Test Account");
	printf ("Done.\n\nChecking for the Full Name...\n");
	buf = (gchar *) c2_account_get_extra_data (account, C2_ACCOUNT_KEY_FULL_NAME, NULL);
	printf ("Full Name: %s\n", buf);

	return 0;
}
