/*  Cronos II Mail Client /libcronosII/db-cronosII.c
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
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>

#include "i18n.h"
#include "db.h"
#include "error.h"
#include "mailbox.h"
#include "utils.h"

gint
c2_db_cronosII_load (C2Mailbox *mailbox)
{
	C2Db *head = NULL, *current = NULL, *next;
	gchar *path, *line, *buf;
	FILE *fd;
	gint i;

	c2_return_val_if_fail (mailbox, -1, C2EDATA);

	/* Calculate the path */
	path = g_strconcat (g_get_home_dir (), C2_HOME,	mailbox->name, ".mbx" G_DIR_SEPARATOR_S "index", NULL);
	C2_DEBUG (path);
	
	/* Open the file */
	if (!(fd = fopen (path, "rt")))
	{
		C2_DEBUG (path);
		c2_error_set (-errno);
		g_free (path);
		return -1;
	}

	for (i = 0;(line = c2_fd_get_line (fd)) != NULL;)
	{
		if (*line == '?')
		{
			g_free (line);
			continue;
		}

		next = c2_db_new (mailbox);
		/* next->message.message = NULL; What is this for??? */
		
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

		next->next = NULL;
		next->previous = current;

		if (current)
			current->next = next;

		if (!mailbox->db)
			mailbox->db = next;

		current = next;

		g_free (line);
	}

	mailbox->db = head;
	return 0;
}

C2Message *
c2_db_cronosII_message_get (C2Db *db, gint mid)
{
	C2Message *message;
	static gchar *home = NULL;
	gchar *path, *string;
	struct stat stat_buf;
	FILE *fd;
	gint length;

	message = c2_message_new ();

	if (!home)
		home = g_get_home_dir ();

	path = g_strdup_printf ("%s" G_DIR_SEPARATOR_S C2_HOME G_DIR_SEPARATOR_S "%s.mbx" G_DIR_SEPARATOR_S "%d",
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

	string = g_new0 (gchar, length+1);

	if (!(fd = fopen (path, "r")))
	{
		c2_error_set (-errno);
		return NULL;
	}

	fread (string, sizeof (gchar), length, fd);
	fclose (fd);

	c2_message_set_message (message, string);
	g_free (string);

	g_free (path);
	
	return message;
}

gint
c2_db_cronosII_create_structure (C2Mailbox *mailbox)
{
	gchar *path = g_strconcat (g_get_home_dir (), C2_HOME, mailbox->name,
								".mbx" G_DIR_SEPARATOR_S, NULL);
	FILE *fd;

	if (mkdir (path, 0700) < 0)
	{
		c2_error_set (-errno);
		g_warning (_("Unable to create structure for Cronos II Db: %s\n"), c2_error_get (c2_errno));
		g_free (path);
		return -1;
	}

	path = g_realloc (path, strlen (path)+6);
	memcpy (path+strlen (path), "index\0", 6);
	
	fclose (fopen (path, "w"));

	return 0;
}

gint
c2_db_cronosII_update_structure (C2Mailbox *mailbox)
{
}

gint
c2_db_cronosII_remove_structure (C2Mailbox *mailbox)
{
	gchar *directory = g_strconcat (g_get_home_dir (), C2_HOME,
									mailbox->name, ".mbx" G_DIR_SEPARATOR_S, NULL);
	gchar *path;
	DIR *dir;
	struct dirent *dentry;	

	if (!(dir = opendir (directory)))
	{
		c2_error_set (-errno);
		g_warning ("Unable to open directory for removing: %s\n", c2_error_get (c2_errno));
		return -1;
	}

	for (; (dentry = readdir (dir)); )
	{
		path = g_strconcat (directory, dentry->d_name, NULL);
		
		if (!c2_file_is_directory (path))
			unlink (path);
		g_free (path);
	}

	closedir (dir);
	rmdir (directory);
	g_free (directory);

	return 0;
}
