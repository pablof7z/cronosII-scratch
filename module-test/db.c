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
	C2DBMessage *message;
	const char *err;

	if (argc < 2) {
		printf ("Usage: %s Database\n", argv[0]);
		return 0;
	}

	/* Load the database */
	if (!(db = c2_db_load (argv[1])) && c2_errno) {
		err = c2_error_get (c2_errno);
		printf ("Database couldn't been loaded: %s\n", err);
		return 0;
	}

	printf ("%d mensajes\n", g_list_length (db->head));

	if (!(message = c2_db_message_get (db, 0))) {
		err = c2_error_get (c2_errno);
		printf ("Couldn't get message: %s\n", err);
		return 0;
	}
	printf ("%s\n", message->message);

	c2_db_unload (db);
	printf ("Database unloaded\n");
	
	return 0;
}
