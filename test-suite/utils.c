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
#include <libcronosII/utils.h>
#include <libcronosII/db.h>

gint
main (gint argc, gchar **argv)
{
	gchar *string = "\"Pablo Fernández Navarro\" <cronosII@users.sourceforge.net>; "
					"Bosko <falling@users.sourceforge.net>,                  "
					"<cronosII-hackers@lists.sourceforge.net>";
	GList *l;

	if (argc > 1)
		string = argv[1];

	C2_DEBUG (string);
	for (l = c2_str_get_emails (string); l; l = g_list_next (l))
		printf ("<email type=\"%s\">%s</email>\n",
					c2_str_is_email ((gchar*) l->data) ? "VALID" : "INVALID", (gchar*) l->data);
	printf ("%s\n", c2_str_are_emails (c2_str_get_emails (string)) ?
					"All email are valid" : "There're invalid email addresses");
	
	return 0;
}
