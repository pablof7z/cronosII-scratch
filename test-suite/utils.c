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
#include <libcronosII/utils-filter.h>

gint
main (gint argc, gchar **argv)
{
	C2Message *message;
	const gchar *field = argv[2], *match = argv[1];
	gboolean res;

	gtk_init (&argc, &argv);

	message = c2_db_message_get_from_file ("message.eml");

	printf ("I'm going to search '%s' in '%s'.\n\n", match, field);

	res = c2_filter_match_message (message, C2_FILTER_MATCH_CONTAINS, FALSE, field, match);
	printf ("Result: %d\n", res);

	return 0;
}
