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
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>

#include "i18n.h"
#include "db.h"
#include "error.h"
#include "mailbox.h"
#include "utils.h"

#define READ_ONLY "rt"
#define WRITE_ONLY "at"
#define READ_WRITE "rt+"

/* [TODO]
 * 010916 - Make a function that returns the last MID of a Mailbox.
 */

static FILE *
get_index									(C2Mailbox *mailbox, const gchar *mode, gchar **path);

static gint
add_message									(C2Mailbox *mailbox, FILE *fd, C2Db *db);

static gint
remove_message								(C2Mailbox *mailbox, FILE *fd, gint *line,
											 gint n, C2Db **db);

gboolean
c2_db_cronosII_create_structure (C2Mailbox *mailbox)
{
	gchar *path = g_strconcat (g_get_home_dir (), C2_HOME, mailbox->name,
								".mbx" G_DIR_SEPARATOR_S, NULL);
	FILE *fd;

	if (mkdir (path, 0700) < 0)
	{
		c2_error_object_set (GTK_OBJECT (mailbox), -errno);
		g_warning (_("Unable to create structure for Cronos II Db: %s\n"), c2_error_get ());
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

static FILE *
get_index (C2Mailbox *mailbox, const gchar *mode, gchar **path)
{
	gchar *_path;
	FILE *fd;

	/* Calculate the path */
	_path = g_strconcat (g_get_home_dir (), C2_HOME, mailbox->name, ".mbx" G_DIR_SEPARATOR_S "index", NULL);

	/* Open the file */
	if (!(fd = fopen (_path, mode)))
	{
		C2_DEBUG (_path);
		c2_error_object_set (GTK_OBJECT (mailbox), -errno);
		return NULL;
	}

	if (path)
		*path = _path;
	else
		g_free (_path);

	return fd;
}

static gint
add_message (C2Mailbox *mailbox, FILE *fd, C2Db *db)
{
	gchar *buf;
	
	fprintf (fd, "%c\r\r%d\r%s\r%s\r%d\r%s\r%d\n",
				db->state, db->mark, db->subject, db->from, db->date, db->account, db->mid);

	buf = g_strdup_printf ("%s" C2_HOME "%s.mbx/%d", g_get_home_dir (), mailbox->name, db->mid);
	if (!(fd = fopen (buf, "wt")))
	{
		g_free (buf);
		return -errno;
	}

	fprintf (fd, "%s\n\n%s", db->message->header, db->message->body);
	fclose (fd);
	g_free (buf);

	return 0;
}

static gint
remove_message (C2Mailbox *mailbox, FILE *fd, gint *line, gint n, C2Db **db)
{
	gchar *mpath;
	
	for (;;)
	{
		if (fgetc (fd) == '?')
		{
			if (!c2_fd_move_to (fd, '\n', 1, TRUE, TRUE))
			{
				printf ("Error!\n"); L
				return -1;
			}
			continue;
		}
		fseek (fd, -1, SEEK_CUR);

		if ((*line)++ != n)
		{
			*db = (*db)->next;
			
			if (!c2_fd_move_to (fd, '\n', 1, TRUE, TRUE))
			{
				printf ("Error!\n"); L
				return -1;
			}
			continue;
		}

		fputc ('?', fd);
		if (!c2_fd_move_to (fd, '\n', 1, TRUE, TRUE))
		{
			*db = (*db)->next;
			printf ("Error!\n"); L
			return -1;
		}

		/* Remove mail file */
		mpath = g_strdup_printf ("%s" C2_HOME "%s.mbx" G_DIR_SEPARATOR_S "%d",
								 g_get_home_dir (), mailbox->name, (*db)->mid);
		if (unlink (mpath) < 0)
		{
			printf ("Error!\n"); L
			perror (mpath);
		}
		perror (mpath);
		g_free (mpath);
		*db = (*db)->next;
		break;
	}

	gtk_signal_emit_by_name (GTK_OBJECT (mailbox), "changed_mailbox",
							 C2_MAILBOX_CHANGE_REMOVE, (*db)->prev);
	
	return 0;
}

void
c2_db_cronosII_freeze (C2Mailbox *mailbox)
{
	mailbox->protocol.cronosII.fd = get_index (mailbox, READ_WRITE, NULL);
}

void
c2_db_cronosII_thaw (C2Mailbox *mailbox)
{
	fclose (mailbox->protocol.cronosII.fd);
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
	C2Db *head = NULL, *current = NULL, *next;
	gchar *line, *buf;
	gboolean mark;
	FILE *fd;
	gint i, mid;
	time_t date;

	c2_return_val_if_fail_obj (mailbox, -1, C2EDATA, GTK_OBJECT (mailbox));	

	if (!(fd = get_index (mailbox, READ_ONLY, NULL)))
		return -1;

	for (i = 0; (line = c2_fd_get_line (fd)) != NULL;)
	{
		if (*line == '?')
		{
			g_free (line);
			continue;
		}

		buf = c2_str_get_word (1, line, '\r');
		if (c2_streq (buf, "MARK"))
			mark = 1;
		else
			mark = 0;
		g_free (buf);

		buf = c2_str_get_word (5, line, '\r');
		date = atoi (buf);
		g_free (buf);

		buf = c2_str_get_word (7, line, '\r');
		mid = atoi (buf);
		g_free (buf);

		next = c2_db_new (mailbox, mark, c2_str_get_word (3, line, '\r'),
							c2_str_get_word (4, line, '\r'),
							c2_str_get_word (6, line, '\r'),
							date, mid, i+1);
		
		buf = c2_str_get_word (0, line, '\r');
		if (buf)
		{
			next->state = (C2MessageState) *buf;
		} else
			next->state = C2_MESSAGE_READED;
		g_free (buf);

		if (current)
			current->next = next;

		if (!mailbox->db)
			mailbox->db = next;

		current = next;

		g_free (line);
		i++;
	}

	return i;
}

gboolean
c2_db_cronosII_message_add (C2Mailbox *mailbox, C2Db *db)
{
	gint mid = 0;
	FILE *fd;

	if (mailbox->db)
		mid = mailbox->db->prev->prev->mid+1;

	if (!mid)
		mid = 1;

	db->mid = mid;
	
	if (!mailbox->freezed)
	{
		if (!(fd = get_index (mailbox, WRITE_ONLY, NULL)))
			return FALSE;
	} else
	{
		fd = mailbox->protocol.cronosII.fd;
		fseek (fd, 0, SEEK_END);
	}

	if ((mid = add_message (mailbox, fd, db)))
	{
		fclose (fd);
		c2_error_object_set (GTK_OBJECT (mailbox), mid);
		return FALSE;
	}

	if (!mailbox->freezed)
		fclose (fd);

	return TRUE;
}

#if 0

"This will become obsolete the 2001/11/05: Remove it"

gboolean
c2_db_cronosII_message_add_list (C2Mailbox *mailbox, GList *list)
{
	FILE *fd;
	GList *l;

	if (!mailbox->freezed)
	{
		if (!(fd = get_index (mailbox, WRITE_ONLY, NULL)))
			return FALSE;
	} else
		fd = mailbox->protocol.cronosII.fd;

	for (l = list; l; l = g_list_next (l))
	{
		C2Db *db = C2_DB (l->data);
		gint mid = 0;
		
		if (mailbox->db)
			mid = mailbox->db->prev->prev->mid+1;
		
		if (!mid)
			mid = 1;
		
		db->mid = mid;
		
		if ((mid = add_message (mailbox, fd, db)))
		{
			fclose (fd);
			c2_error_object_set (GTK_OBJECT (mailbox), mid);
			return FALSE;
		}
	}
	
	if (!mailbox->freezed)
		fclose (fd);

	return TRUE;
}
#endif

gint
c2_db_cronosII_message_remove (C2Mailbox *mailbox, GList *list)
{
	FILE *fd;
	C2Db *db;
	GList *l;
	gint line = 0;
	gint retval = 0;

	/* Remove from index first */
	if (!(fd = get_index (mailbox, READ_WRITE, NULL)))
		return -1;
	
	fseek (fd, 0, SEEK_SET);

	for (l = list, line = 0, db = mailbox->db; l; l = g_list_next (l))
	{
		gint n = GPOINTER_TO_INT (l->data);

		if (remove_message (mailbox, fd, &line, n, &db) < 0)
		{
			retval = -1;
			break;
		}
	}
	
	fclose (fd);

	return retval;
}

#if 0

"This will become obsolete the 2001/11/05: Remove it"

gint
c2_db_cronosII_move_list (C2Mailbox *fmailbox, C2Mailbox *tmailbox, GList *list)
{
	GList *l;
	C2Db *db;
	gint line;
	FILE *ffd, *tfd;
	gint retval = 0;

	if (!(ffd = get_index (fmailbox, READ_WRITE, NULL)))
	{
#ifdef USE_DEBUG
		perror ("Opening fmailbox's index");
#endif
		c2_error_set (C2INTERNAL);
		return -1;
	}

	if (!(tfd = get_index (tmailbox, WRITE_ONLY, NULL)))
	{
#ifdef USE_DEBUG
		perror ("Opening tmailbox's index");
#endif
		c2_error_set (C2INTERNAL);
		fclose (ffd);
		return -1;
	}

	for (l = list, db = fmailbox->db, line = 0; l; l = g_list_next (l))
	{
		gint n = GPOINTER_TO_INT (l->data);
		
		if (remove_message (fmailbox, ffd, &line, n, &db) < 0)
		{
			retval = -1;
			break;
		}

		if (add_message (tmailbox, tfd, db) < 0)
		{
			retval = -1;
			break;
		}
	}

	return retval;
}
#endif

void
c2_db_cronosII_message_set_state (C2Db *db, C2MessageState state)
{
}

void
c2_db_cronosII_message_set_mark (C2Db *db, gboolean mark)
{
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
	
}
