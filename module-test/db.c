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
#include <glib.h>

#include "libmodules/db.h"
#include "libmodules/error.h"

int
main (int argc, char **argv)
{
	C2DB *db;
	C2Message *message;
	const char *err;
	gchar *field;

	if (argc < 4) {
		printf ("Usage: %s Database # Field\n", argv[0]);
		return 0;
	}

	/* Load the database */
	if (!(db = c2_db_load (argv[1], C2_METHOD_CRONOSII)) && c2_errno) {
		err = c2_error_get (c2_errno);
		printf ("Database couldn't been loaded: %s\n", err);
		return 0;
	}

	printf ("%d mensajes\n", g_list_length (db->head));

	if (!(message = c2_db_message_get (db, atoi (argv[2])))) {
		err = c2_error_get (c2_errno);
		printf ("Couldn't get message: %s\n", err);
		return 0;
	}
	field = c2_message_get_header_field (message, NULL, argv[3]);
	printf ("%s\n", field);

	c2_db_unload (db);
	printf ("Database unloaded\n");
	return 0;
}
