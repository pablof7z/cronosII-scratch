/*  Cronos II Mail Client
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

#include "db.h"
#include "db-cronosII.h"
#include "error.h"
#include "mailbox.h"
#include "utils.h"

C2Db *
c2_db_cronosII_load (C2Mailbox *mailbox)
{
	C2Db *head = NULL, *current = NULL, *next;
	
	gchar *path, *line, *buf;
	FILE *fd;

	gint i;
	
	c2_return_val_if_fail (mailbox, NULL, C2EDATA);

	/* Calculate the path */
	path = g_strconcat (g_get_home_dir (), G_DIR_SEPARATOR_S ".CronosII" G_DIR_SEPARATOR_S,
						mailbox->name, ".mbx" G_DIR_SEPARATOR_S "index", NULL);
	
	/* Open the file */
	if (!(fd = fopen (path, "rt")))
	{
		C2_DEBUG (path);
		c2_error_set (-errno);
		g_free (path);
		return NULL;
	}

	for (i = 0;(line = c2_fd_get_line (fd)) != NULL;)
	{
		if (*line == '?')
		{
			g_free (line);
			continue;
		}

		next = c2_db_new (NULL);
		next->message.message = NULL;
		
		buf = c2_str_get_word (0, line, '\r');
		if (buf)
			next->state = (C2MessageState) *buf;
		else
			next->state = C2_MESSAGE_READED;
		g_free (buf);

		buf = c2_str_get_word (1, line, '\r');
		if (c2_streq (buf, "MARK"))
			next->marked = 1;
		else
			next->marked = 0;
		g_free (buf);

		next->subject = c2_str_get_word (3, line, '\r');
		next->from = c2_str_get_word (4, line, '\r');
		next->account = c2_str_get_word (6, line, '\r');
		
		buf = c2_str_get_word (5, line, '\r');
		next->date = atoi (buf);
		g_free (buf);

		next->position = i++;
		
		buf = c2_str_get_word (7, line, '\r');
		next->mid = atoi (buf);
		g_free (buf);

		next->mailbox = mailbox;

		next->next = NULL;
		next->previous = current;

		if (current)
			current->next = next;

		if (!head)
			head = next;

		current = next;

		g_free (line);
	}

	return head;
}

C2Message *
c2_db_cronosII_message_get (C2Db *db, gint mid)
{
	C2Message *message;
	static gchar *home = NULL;
	gchar *path;
	struct stat stat_buf;
	FILE *fd;
	gint length;

	message = c2_message_new ();

	if (!home)
		home = g_get_home_dir ();

	path = g_strdup_printf ("%s" G_DIR_SEPARATOR_S ".CronosII/%s.mbx/%d",
							home, db->mailbox->name, mid);
	
	if (stat (path, &stat_buf) < 0)
	{
		c2_error_set (-errno);
#ifdef USE_DEBUG
		g_print ("Stating the file failed: %s\n", path);
#endif
		return NULL;
	}

	length = ((gint) stat_buf.st_size * sizeof (gchar));

	message->message = g_new0 (gchar, length+1);

	if (!(fd = fopen (path, "r")))
	{
		c2_error_set (-errno);
		return NULL;
	}

	fread (message->message, sizeof (gchar), length, fd);
	fclose (fd);

	g_free (path);
	
	return message;
}
