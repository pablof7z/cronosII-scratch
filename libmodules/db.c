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
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <unistd.h>

#include "db.h"
#include "error.h"
#include "utils.h"

static gchar *
c2_db_path_mail								(const gchar *mbox, mid_t mid);

static gchar *
c2_db_path									(const gchar *mbox);

/**
 * c2_db_load
 * @db_name: Name of database to load (i.e. "Inbox").
 *
 * Loads a database.
 * 
 * Return Value:
 * A C2DB object containing the loaded database or NULL
 * in case it couldn't be loaded for some reason.
 * (c2_errno will take the proper value)
 **/
C2DB *
c2_db_load (const gchar *db_name)
{
	gchar *path, *line, *buf;
	FILE *fd;
	
	C2DB *db;
	C2DBNode *node;
	int i;
	
	c2_return_val_if_fail (db_name, NULL, C2EDATA);
	
	/* Get the path */
	path = c2_db_path (db_name);

	/* Open the file */
	if ((fd = fopen (path, "r")) == NULL)
	{
		c2_error_set (-errno);
		return NULL;
	}

	/* Create the C2DB object */
	db = c2_db_new ();
	db->mbox = g_strdup (db_name);
	db->head = NULL;

	/* File opened read it */
	for (i = 0;(line = c2_fd_get_line (fd));)
	{
		/* Lines starting with '?' must not
		 * be readen */
		if (*line == '?')
		{
			g_free (line);
			continue;
		}
		
		node = c2_db_node_new ();
		node->row = i++;
		buf = c2_str_get_word (0, line, '\r');
		switch (*buf)
		{
			case 'N':
				node->status = C2_DB_NODE_UNREAD; break;
			case 'R':
				node->status = C2_DB_NODE_REPLIED; break;
			case 'F':
				node->status = C2_DB_NODE_FORWARDED; break;
			case ' ':
				node->status = C2_DB_NODE_READED; break;
			default:
				node->status = C2_DB_NODE_READED;
		}
		g_free (buf);
		buf = c2_str_get_word (1, line, '\r');
		if (c2_streq (buf, "MARK")) node->marked = TRUE;
		else node->marked = FALSE;
		g_free (buf);
		node->headers[0] = c2_str_get_word (3, line, '\r');
		node->headers[1] = c2_str_get_word (4, line, '\r');
		node->headers[2] = c2_str_get_word (5, line, '\r');
		node->headers[3] = c2_str_get_word (6, line, '\r');
		buf = c2_str_get_word (7, line, '\r');
		node->mid = atoi (buf);
		g_free (buf);
		g_free (line);

		db->head = g_list_append (db->head, node);
	}

	fclose (fd);

	return db;
}

/**
 * c2_db_unload
 * @db_d: DB Descriptor.
 *
 * Unloads a database.
 **/
void
c2_db_unload (C2DB *db_d)
{
	GList *l;
	C2DBNode *node;

	c2_return_if_fail (db_d, C2EDATA);

	for (l = db_d->head; l != NULL; l = l->next)
	{
		node = l->data;
		g_free (node->headers[0]);
		g_free (node->headers[1]);
		g_free (node->headers[2]);
		g_free (node->headers[3]);
		g_free (node);
		l->data = NULL;
	}

	g_list_free (db_d->head);
	g_free (db_d->mbox);
	g_free (db_d);
}

/**
 * c2_db_message_add
 * @db_d: DB Descriptor.
 * @message: Message to be added.
 * @row: Row where to insert the message (-1 for the last one).
 * 
 * Will add a message in the specified database.
 *
 * Return Value:
 * 0 on success or 1 on failure. In this last case
 * c2_errno will be set the error.
 **/
gint
c2_db_message_add (C2DB *db_d, const gchar *message, gint row)
{
	return 0;
}

/**
 * c2_db_message_remove
 * @db_d: DB Descriptor.
 * @row: Row to remove from the DB (-1 for all).
 * 
 * Will remove the row of the specified database.
 *
 * Return Value:
 * 0 on success or 1 on failure. In this last case
 * c2_errno will be set the error.
 **/
gint
c2_db_message_remove (C2DB *db_d, gint row)
{
	c2_return_val_if_fail (db_d, 1, C2EDATA);
	return 0;
}

/**
 * c2_db_message_get
 * @db_d: DB Descriptor.
 * @row: Row to get.
 *
 * Will get a message from the specified database.
 * 
 * Return Value:
 * A C2Message object with the message or
 * NULL in case of error.
 **/
C2Message *
c2_db_message_get (C2DB *db_d, int row)
{
	GList *l;
	C2DBNode *node;
	C2Message *message = NULL;
	gchar *path;
	
	c2_return_val_if_fail (db_d, NULL, C2EDATA);

	/* Get the node */
	l = g_list_nth (db_d->head, row);
	c2_return_val_if_fail (l, NULL, C2ENOMSG);
	node = l->data;
	c2_return_val_if_fail (node, NULL, C2ENOMSG);

	/* Get the message */
	path = c2_db_path_mail (db_d->mbox, node->mid);
	message = c2_db_message_get_from_file (path);
	g_free (path);
	
	return message;
}

/**
 * message_get_message_from_file
 * @filename: A pointer to a char object which contains the path to a file.
 *
 * Loads the file @filename into a Message object.
 *
 * Return Value:
 * The Message object with the message or NULL in case of error.
 **/
C2Message *
c2_db_message_get_from_file (const gchar *filename)
{
	const gchar *path;
	C2Message *message;
	struct stat *stat_buf;
	gint len;
	FILE *fd;
	
	c2_return_val_if_fail (filename, NULL, C2EDATA);
	
	path = filename; 
	stat_buf = g_new0 (struct stat, 1);
	
	if (stat (path, stat_buf) < 0 || !stat_buf) {
		c2_error_set (-errno);
		return NULL;
	}
	
	len = ((gint) stat_buf->st_size * sizeof (gchar));
	g_free (stat_buf);
	
	message = g_new0 (C2Message, 1);
	message->mbox = NULL;
	message->mid = 0;
	message->message = g_new0 (gchar, len+1);
	message->header = NULL;
	message->body = NULL;
	message->mime = NULL;
	
	if (!(fd = fopen (path, "r")))
	{
		c2_error_set (-errno);
		return NULL;
	}
	
	fread (message->message, sizeof (gchar), len, fd);
	fclose (fd);
	
	return message;
}

/**
 * c2_db_message_search_by_mid
 * @db_d: DB descriptor.
 * @mid: mid to search.
 *
 * Searchs in the database @db_d for the mid @mid.
 *
 * Return Value:
 * The row where @mid was found.
 **/
gint
c2_db_message_search_by_mid (const C2DB *db_d, mid_t mid)
{
	C2DBNode *node;
	GList *l;
	int i = 0;
	
	c2_return_val_if_fail (db_d, -1, C2EDATA);
	
	for (l = db_d->head; l != NULL; l = l->next, i++)
	{
		node = l->data;
		if (node->mid == mid) return i;
	}
	return -1;
}

static gchar *
c2_db_path_mail (const gchar *mbox, mid_t mid)
{
	static gchar *home = NULL;
	
	c2_return_val_if_fail (mbox, NULL, C2EDATA);

	if (*mbox == '/') return g_strdup (mbox);
	
	if (!home)
		home = g_get_home_dir ();
	
	return (g_strdup_printf ("%s%c.CronosII%c%s.mbx%c%d",
					home, G_DIR_SEPARATOR, G_DIR_SEPARATOR, mbox, G_DIR_SEPARATOR, mid));
}

static gchar *
c2_db_path (const gchar *mbox)
{
	static gchar *home = NULL;
	
	c2_return_val_if_fail (mbox, NULL, C2EDATA);
	
	if (*mbox == '/') return g_strdup (mbox);

	if (!home)
		home = g_get_home_dir ();
	
	return (g_strdup_printf ("%s%c.CronosII%c%s.mbx%cindex",
					home, G_DIR_SEPARATOR, G_DIR_SEPARATOR, mbox, G_DIR_SEPARATOR));
}
