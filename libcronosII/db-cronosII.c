/*  Cronos II - The GNOME mail client
 *  Copyright (C) 2000-2001 Pablo Fernández López
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
/**
 * Maintainer(s) of this file:
 * 		* Pablo Fernández López
 * Code of this file by:
 * 		* Pablo Fernández López
 */

/* If you are reading this for the first time
 * you probably want to read the notes in the
 * file db.c
 */

#include <glib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>

#include "i18n.h"
#include "db.h"
#include "error.h"
#include "mailbox.h"
#include "utils.h"

/* [TODO]
 * 010916 - Make a function that returns the last MID of a Mailbox.
 */

/**
 * cache_index
 * @mailbox:
 *
 * This function will try to open a file descriptor
 * to the index file of the mailbox @mailbox.
 *
 * Return Value:
 * %TRUE if the file descriptor was opened will exist
 * after the call to this function or %FALSE.
 **/
static gboolean
cache_index (C2Mailbox *mailbox)
{
	gchar *path;
	
	if (mailbox->protocol.cronosII.fd)
		return TRUE;

	path = g_strconcat (g_get_home_dir (), C2_HOME, mailbox->name,
							".mbx" G_DIR_SEPARATOR_S "index", NULL);
	if (!(mailbox->protocol.cronosII.fd = fopen (path, "rt+")))
	{
		g_free (path);
		c2_error_object_set (GTK_OBJECT (mailbox), -errno);
		return FALSE;
	}
	
	g_free (path);
	return TRUE;
}

static gint
get_mid_from_position (C2Mailbox *mailbox, gint position)
{
	C2Db *db;

	if (mailbox->db)
	{
		db = mailbox->db;

		do
			if (db->position == position)
				return db->mid;
		while (c2_db_lineal_next (db));
	}

	return -1;
}

static void
_lock (C2Mailbox *mailbox)
{
	c2_mutex_lock (&mailbox->protocol.cronosII.lock);
}

static void
_try_lock (C2Mailbox *mailbox)
{
	c2_mutex_trylock (&mailbox->protocol.cronosII.lock);
}

static void
_unlock (C2Mailbox *mailbox)
{
	c2_mutex_unlock (&mailbox->protocol.cronosII.lock);
}

static void
_rewind (C2Mailbox *mailbox)
{
	rewind (mailbox->protocol.cronosII.fd);
	mailbox->protocol.cronosII.mid = -1;
}

/**
 * This function will find the MID of the line
 * where the FD is currently set or the MID of
 * the next valid line.
 * The FD will be positionated at the start of
 * the line.
 **/
static gint
mid_in_line (C2Mailbox *mailbox, gint *length)
{
	gchar c;
	gint line_length;
	gint sep;
	gint mid = -1;
	FILE *fd;

	fd = mailbox->protocol.cronosII.fd;
	
	/* Go through the line */
start:
	for (sep = 0, line_length = 0; fread (&c, 1, sizeof (gchar), fd);)
	{
		/* EOF reached */
		if (c == EOF)
		{
			_rewind (mailbox);
			return -1;
		}

		/* A D reached */
		if (!sep && c == 'D')
		{
			for (; fread (&c, 1, sizeof (gchar), fd);)
			{
				if (c == '\n')
					goto start;
				else if (c == EOF)
				{
					_rewind (mailbox);
					return -1;
				}
			}
		}

		/* A \n reached */
		if (c == '\n')
			goto start;
			
		line_length++;
			
		/* A \r reached */
		if (c == '\r')
		{
			if (++sep == 7)
			{
				gchar buffer[11];
				gint bi = 0;

				for (bi = 0;; bi++)
				{
					line_length++;
					fread (&buffer[bi], 1, sizeof (gchar), fd);
				
					if (buffer[bi] == '\n' || buffer[bi] == EOF)
						break;
				}

				buffer[bi] = 0;
					
				mid = atoi (buffer);
				mailbox->protocol.cronosII.mid = mid;

				fseek (fd, -(line_length), SEEK_CUR);
				break;
			}
		}
	}

	if (length)
		*length = line_length;
	
	return mid;
}

/**
 * This function will do the same
 * thing as mid_in_line but will
 * go to the previous line instead
 * of to the next line.
 **/
static gint
mid_in_line_reverse (C2Mailbox *mailbox, gint *length)
{
	gchar c;
	gint line_length;
	gint sep;
	gint mid = -1;
	FILE *fd;

	fd = mailbox->protocol.cronosII.fd;
	
	/* Go through the line */
start:
	for (sep = 0, line_length = 0; fread (&c, 1, sizeof (gchar), fd);)
	{
		/* EOF reached */
		if (c == EOF)
		{
			_rewind (mailbox);
			return -1;
		}

		/* A D reached */
		if (c == 'D')
		{
			if (!c2_fd_move_to (mailbox->protocol.cronosII.fd, '\n', 2, FALSE, TRUE))
			{
				c2_error_object_set (GTK_OBJECT (mailbox), c2_errno);
				_rewind (mailbox);
				return -1;
			}
		}

		/* A \n reached */
		if (c == '\n')
			goto start;
			
		line_length++;
			
		/* A \r reached */
		if (c == '\r')
		{
			if (++sep == 7)
			{
				gchar buffer[11];
				gint bi = 0;

				for (bi = 0;; bi++)
				{
					line_length++;
					fread (&buffer[bi], 1, sizeof (gchar), fd);
				
					if (buffer[bi] == '\n' || buffer[bi] == EOF)
						break;
				}

				buffer[bi] = 0;
					
				mid = atoi (buffer);
				mailbox->protocol.cronosII.mid = mid;

				fseek (fd, -(line_length), SEEK_CUR);
				break;
			}
		}
	}

	if (length)
		*length = line_length;
	
	return mid;
}

/**
 * This function will go to an specific MID
 * in an index file.
 **/
static gboolean
goto_mid (C2Mailbox *mailbox, gint req_mid)
{
	gboolean found = FALSE;
	gint line_length;
	
	if (!cache_index (mailbox))
		return FALSE;

	if (mailbox->protocol.cronosII.mid < req_mid)
	{
current_mid_less_than_req_mid:

		/* Go through the lines */
		for (;;)
		{
			gint fmid;

			if ((fmid = mid_in_line (mailbox, &line_length)) < 0)
			{
				found = FALSE;
				_rewind (mailbox);
				break;
			}
			
			if (fmid == req_mid)
			{
				found = TRUE;
				break;
			} else
				fseek (mailbox->protocol.cronosII.fd, line_length, SEEK_CUR);
		}
	} else if (mailbox->protocol.cronosII.mid > req_mid)
	{
		/* FIXME This is an ugly thing to do, lets change it */
		_rewind (mailbox);
		goto current_mid_less_than_req_mid;

		/* Go through the lines backwards */
		for (;;)
		{
			if (!c2_fd_move_to (mailbox->protocol.cronosII.fd, '\n', 1, FALSE, TRUE))
			{
move_error:
				c2_error_object_set (GTK_OBJECT (mailbox), c2_errno);
				found = FALSE;
				_rewind (mailbox);
				break;
			} else
			{
				if (!c2_fd_move_to (mailbox->protocol.cronosII.fd, '\r', 7, FALSE, FALSE))
					goto move_error;
				if (fseek (mailbox->protocol.cronosII.fd, -1, SEEK_CUR) < 0)
				{
					c2_error_set (-errno);
					goto move_error;
				}

				if (mid_in_line_reverse (mailbox, &line_length) == req_mid)
				{
					found = TRUE;
					break;
				} else
					fseek (mailbox->protocol.cronosII.fd, -(line_length+1), SEEK_CUR);
			}
		}
	} else
		found = TRUE;

	return found;
}




/******************************************
 * Structure functions
 ******************************************/
gboolean
c2_db_cronosII_create_structure (C2Mailbox *mailbox)
{
	gchar *path = g_strconcat (g_get_home_dir (), C2_HOME, mailbox->name,
								".mbx" G_DIR_SEPARATOR_S, NULL);

	if (mkdir (path, 0700) < 0)
	{
		c2_error_object_set (GTK_OBJECT (mailbox), -errno);
		g_warning (_("Unable to create structure for Cronos II Db: %s\n"),
						c2_error_object_get (GTK_OBJECT (mailbox)));
		return FALSE;
	}

	path = g_realloc (path, strlen (path)+6);
	memcpy (path+strlen (path), "index\0", 6);
	
	fclose (fopen (path, "w"));
	g_free (path);

	return TRUE;
}

gboolean
c2_db_cronosII_update_structure (C2Mailbox *mailbox)
{
	return TRUE;
}

gboolean
c2_db_cronosII_remove_structure (C2Mailbox *mailbox)
{
	gchar *directory; 
	gchar *path;
	DIR *dir;
	struct dirent *dentry;

	directory = g_strconcat (g_get_home_dir (), C2_HOME, mailbox->name, ".mbx"
								G_DIR_SEPARATOR_S, NULL);

	if (!(dir = opendir (directory)))
	{
		c2_error_object_set (GTK_OBJECT (mailbox), -errno);
#ifdef USE_DEBUG
		g_warning ("Unable to open directory for removing: %s\n",
								c2_error_get ());
#endif
		return FALSE;
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

	return TRUE;
}

gboolean
c2_db_cronosII_goto_mid (C2Mailbox *mailbox, gint mid)
{
	return goto_mid (mailbox, mid);
}

void
c2_db_cronosII_freeze (C2Mailbox *mailbox)
{
}

void
c2_db_cronosII_thaw (C2Mailbox *mailbox)
{
}

/**
 * c2_db_cronosII_load
 * @mailbox: Mailbox to load.
 *
 * Will load the Cronos II Mailbox.
 *
 * Return Value:
 * Number of loaded messages or -1 in case of error.
 **/
gint
c2_db_cronosII_load (C2Mailbox *mailbox)
{
	C2Db *current = NULL, *next;
	FILE *fd;
	gint position, mid;
	gchar c, *line, *buf;
	gboolean mark;
	time_t date;

	c2_return_val_if_fail_obj (mailbox, -1, C2EDATA, GTK_OBJECT (mailbox));	

	/* Initialization */
	if (!cache_index (mailbox))
		return -1;
	
	_lock (mailbox);
	fd = mailbox->protocol.cronosII.fd;
	_rewind (mailbox);
	
	/* Go through the lines */
	for (position = 0;;)
	{
		if (fread (&c, 1, sizeof (gchar), fd) < 1)
			break;

		/* A D has been reached */
		if (c == 'D')
		{
			/* Lets go to the next line.. */
			if (!c2_fd_move_to (fd, '\n', 1, TRUE, TRUE))
				break;

			/* .. and start again */
			continue;
		}

		/* Now that we know this is an interesting line,
		 * lets grab it.
		 */
		if (!(line = c2_fd_get_line (fd)))
		{
			c2_error_object_set (GTK_OBJECT (mailbox), c2_errno);
			break;
		}

		/* Create the new C2Db */
		buf = c2_str_get_word (5, line, '\r');
		date = atoi (buf);
		g_free (buf);

		buf = c2_str_get_word (7, line, '\r');
		mid = atoi (buf);
		g_free (buf);
		mailbox->protocol.cronosII.mid = mid;

		buf = c2_str_get_word (2, line, '\r');
		mark = atoi (buf);
		g_free (buf);

		next = c2_db_new (mailbox, mark, c2_str_get_word (3, line, '\r'),
							c2_str_get_word (4, line, '\r'),
							c2_str_get_word (6, line, '\r'),
							date, mid, ++position);

		switch (c)
		{
			default:
			case ' ': next->state = C2_MESSAGE_READED; break;
			case 'N': next->state = C2_MESSAGE_UNREADED; break;
			case 'R': next->state = C2_MESSAGE_REPLIED; break;
			case 'F': next->state = C2_MESSAGE_FORWARDED; break;
		}

		/* Append it to the list */
		if (current)
			current->next = next;

		if (!mailbox->db)
			mailbox->db = next;

		current = next;
	}
	
	_unlock (mailbox);
	_rewind (mailbox);

	return position;
}

gboolean
c2_db_cronosII_message_add (C2Mailbox *mailbox, C2Db *db)
{
	gint mid = 0;
	FILE *fd;
	gchar *buf;
	C2MessageState state;

	/* Initialization */
	if (!cache_index (mailbox))
		return FALSE;

	_lock (mailbox);
	fd = mailbox->protocol.cronosII.fd;

	/* Get the MID */
	if (mailbox->db)
		mid = mailbox->db->prev->prev->mid+1;

	if (!mid)
		mid = 1;

	db->mid = mid;
	
	fseek (fd, 0, SEEK_END);

	switch (db->state)
	{
		case C2_MESSAGE_READED: state = ' '; break;
		default:
		case C2_MESSAGE_UNREADED: state = 'N'; break;
		case C2_MESSAGE_REPLIED: state = 'R'; break;
		case C2_MESSAGE_FORWARDED: state = 'F'; break;
	}

	fprintf (fd, "%c\r\r%d\r%s\r%s\r%d\r%s\r%d\n",
					state, db->mark, db->subject, db->from,
					db->date, db->account, db->mid);

	/* Now write to its own file */
	buf = g_strdup_printf ("%s" C2_HOME "%s.mbx/%d", g_get_home_dir (), mailbox->name, db->mid);
	if (!(fd = fopen (buf, "wt")))
	{
		g_free (buf);
		c2_error_object_set (GTK_OBJECT (mailbox), -errno);
		return FALSE;
	}
	g_free (buf);

	fprintf (fd, "%s\n\n%s", db->message->header, db->message->body);
	fclose (fd);

	_rewind (mailbox);
	_unlock (mailbox);

	return TRUE;
}

gint
c2_db_cronosII_message_remove (C2Mailbox *mailbox, GList *list)
{
	GList *l;
	gint retval = 0;
	gchar *basedir;
	gchar *strmid;
	gchar *path = NULL;
	gint basedir_length, length;
	C2Db *db;

	_lock (mailbox);

	basedir = g_strdup_printf ("%s" C2_HOME "%s.mbx" G_DIR_SEPARATOR_S, g_get_home_dir (), mailbox->name);
	basedir_length = strlen (basedir);
	
	for (l = list; l; l = g_list_next (l))
	{
		db = (C2Db*) l->data;
		
		if (!goto_mid (mailbox, db->mid))
			continue;

		fputc ('D', mailbox->protocol.cronosII.fd);
		fseek (mailbox->protocol.cronosII.fd, -1, SEEK_CUR);
		strmid = g_strdup_printf ("%d", db->mid);
		length = basedir_length + strlen (strmid) + 1;
		path = g_realloc (path, length);
		strcpy (path, basedir);
		strcat (path, strmid);
		path[length-1] = 0;
		g_free (strmid);

		if (unlink (path) < 0)
		{
			c2_error_object_set (GTK_OBJECT (mailbox), -errno);
			perror ("unlink");
			C2_DEBUG (path);
		}
	}

	g_free (path);
	g_free (basedir);

	_unlock (mailbox);
	
	return retval;
}

void
c2_db_cronosII_message_set_state (C2Db *db, C2MessageState state)
{
	C2Mailbox *mailbox;
	gchar c;

	switch (state)
	{
		case C2_MESSAGE_UNREADED:
			c = 'N';
			break;
		case C2_MESSAGE_READED:
			c = ' ';
			break;
		case C2_MESSAGE_REPLIED:
			c = 'R';
			break;
		case C2_MESSAGE_FORWARDED:
			c = 'F';
			break;
		default:
			g_assert_not_reached ();
	}

	mailbox = db->mailbox;
	_lock (mailbox);

	if (!goto_mid (mailbox, db->mid))
		return;

	fputc (c, mailbox->protocol.cronosII.fd);
	fseek (mailbox->protocol.cronosII.fd, -1, SEEK_CUR);

	_unlock (mailbox);
}

void
c2_db_cronosII_message_set_mark (C2Db *db, gboolean mark)
{
	C2Mailbox *mailbox;
	FILE *fd;
	gchar c;
	gint pos;

	mailbox = db->mailbox;
	_lock (mailbox);

	if (!goto_mid (mailbox, db->mid))
		return;

	fd = mailbox->protocol.cronosII.fd;

	/* Move to the 2nd \r */
	pos = ftell (fd);
	if (c2_fd_move_to (fd, '\r', 2, TRUE, TRUE) < 0)
	{
		fseek (fd, pos, SEEK_SET);
		_rewind (mailbox);
		_unlock (mailbox);
		return;
	}
	
	fputc (mark ? '1' : '0', fd);
	fseek (fd, pos, SEEK_SET);

	_unlock (mailbox);
}

C2Message *
c2_db_cronosII_load_message (C2Db *db)
{
	C2Message *message;
	static gchar *home = NULL;
	gchar *path, *string;
	struct stat stat_buf;
	FILE *fd;
	gint length;
	gint mid = db->mid;

	message = c2_message_new ();

	if (!home)
		home = g_get_home_dir ();

	path = g_strdup_printf ("%s" C2_HOME "%s.mbx" G_DIR_SEPARATOR_S "%d",
							home, db->mailbox->name, mid);
	
	if (stat (path, &stat_buf) < 0)
	{
		c2_error_object_set (GTK_OBJECT (db), -errno);
#ifdef USE_DEBUG
		g_print ("Stating the file failed: %s\n", path);
#endif
		gtk_object_destroy (GTK_OBJECT (message));
		return NULL;
	}

	length = ((gint) stat_buf.st_size * sizeof (gchar));

	string = g_new0 (gchar, length+1);

	if (!(fd = fopen (path, "r")))
	{
		c2_error_object_set (GTK_OBJECT (db), -errno);
		gtk_object_destroy (GTK_OBJECT (message));
		return NULL;
	}

	fread (string, sizeof (gchar), length, fd);
	fclose (fd);

	c2_message_set_message (message, string);
	g_free (string);

	g_free (path);
	
	return message;
}



C2Message *
c2_db_cronosII_message_get (C2Db *db, gint mid)
{
	return NULL;
}
