#include <stdio.h>
#include <glib.h>

#include "libmodules/db.h"
#include "libmodules/error.h"

int
main (int argc, char **argv) {
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
