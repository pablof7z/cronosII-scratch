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
#include <config.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "i18n.h"
#include "error.h"
#include "db.h"
#include "db-spool.h"
#include "mailbox.h"
#include "utils.h"
#include "utils-date.h"

/*hmmm this is almost beautifull in its minimalism -pete ;)*/
/*yeah, is the shortest module I ever see! but it doesn't have any bugs! :) -pablo */
/*Ah you coders bugs bugs bugs. No code =nobugs. Code bad code use memory. :(*/
/*Oh, yes, the perfect armony of "the 0 bytes length of the file", just perfect :) -pablo */
/*Cool! I´m coding from Paris with my new laptop! -pablo */
/*Crap! I'm coding in Buenos Aires without electricity (it went off) -pablo */

#define index_file(mbox, v) \
	{ \
		gchar *hd = g_get_home_dir (); \
		v = g_strconcat (hd, "/.c2/spool/", mbox->name, NULL); \
		g_free (hd); \
	}

gboolean
c2_db_spool_create_structure (C2Mailbox *mailbox)
{
	gchar *ipath;
	gint fd;

	if ((fd = open (mailbox->protocol.spool.path, O_CREAT, S_IRUSR | S_IWUSR)) < 0)
	{
		c2_error_set (-errno);
		return FALSE;
	}

	close (fd);

	index_file (mailbox, ipath);
	if ((fd = open (ipath, O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR)) < 0)
	{
		c2_error_set (-errno);
		return FALSE;
	}

	close (fd);
	g_free (ipath);
	
	return TRUE;
}

gboolean
c2_db_spool_update_structure (C2Mailbox *mailbox)
{
}

gboolean
c2_db_spool_remove_structure (C2Mailbox *mailbox)
{
	gchar *ipath;
	
	if ((unlink (mailbox->protocol.spool.path)) < 0)
	{
		c2_error_set (-errno);
		return FALSE;
	}

	index_file (mailbox, ipath);
	if ((unlink (ipath)) < 0)
	{
		c2_error_set (-errno);
		g_free (ipath);
		return FALSE;
	}

	return TRUE;
}

gint
c2_db_spool_load (C2Mailbox *mailbox)
{
}

void
c2_db_spool_message_add (C2Mailbox *mailbox, C2Db *db)
{
}

void
c2_db_spool_message_remove (C2Mailbox *mailbox, C2Db *db, gint n)
{
}

void
c2_db_spool_message_set_state (C2Db *db, C2MessageState state)
{
}

void
c2_db_spool_message_set_mark (C2Db *db, gboolean mark)
{
}

C2Message *
c2_db_spool_load_message (C2Db *db)
{
}

#if 0
static gchar *
get_field_value							(const gchar *field, gint offset);

static time_t
get_date_field_from_date				(const gchar *field);

static time_t
get_date_field_from_from				(const gchar *field);

static void
get_source_desc							(C2Mailbox *mailbox, FILE **source);

static void
get_index_info							(C2Mailbox *mailbox, gint mail, gint *start, gint *hoffset, gint *length);

C2Message *
c2_db_spool_message_get (C2Db *db, gint mid)
{
	FILE *source;
	gint start, hoffset, length;
	C2Message *message = c2_message_new ();

	get_index_info (db->mailbox, mid, &start, &hoffset, &length);
	get_source_desc (db->mailbox, &source);

	fseek (source, start, SEEK_SET);
	printf ("Allocate %d\n", hoffset);
	message->header = g_new0 (gchar, hoffset);
	printf ("%s\n", c2_fd_get_line (source));
	fread (message->header, sizeof (gchar), hoffset, source);
	C2_DEBUG (message->header);
	fseek (source, start+hoffset, SEEK_SET);
	printf ("Allocate %d\n", length-hoffset);
	message->body = g_new0 (gchar, length-hoffset);
	fread (message->body, sizeof (gchar), length-hoffset, source);
	fclose (source);

	return message;
}

gint
c2_db_spool_load (C2Mailbox *mailbox)
{
	gchar *source_path = mailbox->protocol.spool.path;
	gchar *index_path = g_strconcat (g_get_home_dir (), C2_HOME, mailbox->name, ".mbx", NULL);
	FILE *source, *index;
	gchar *buf;
	gint i, len, pos, hoffset, l;
	C2Db *head = NULL, *next, *current = NULL;

	c2_return_val_if_fail (mailbox->type == C2_MAILBOX_SPOOL, -1, C2EDATA);
	if (c2_db_spool_sanity_check (mailbox))
	{
		g_warning ("Sanity check failed... Rebuilding.\n");
		if (c2_db_spool_update_structure (mailbox))
			return -1;
	}		

	/* Open the source file (read-only). */
	if (!(source = fopen (mailbox->protocol.spool.path, "r")))
	{
		c2_error_set (-errno);
		g_warning (_("Unable to update structure for Spool Db: %s\n"), c2_error_get (c2_errno));
		return -1;
	}
	
	/* Open the index file (read-only). */
	buf = g_strconcat (g_get_home_dir (), C2_HOME, mailbox->name, ".mbx", NULL);
	if (!(index = fopen (buf, "r")))
	{
		c2_error_set (-errno);
		g_warning (_("Unable to update structure for Spool Db: %s\n"), c2_error_get (c2_errno));
		fclose (source);
		g_free (buf);
		return -1;
	}
	g_free (buf);

	for (l=1;;l++)
	{
		if (!(buf = c2_fd_get_line (index)))
			break;

		sscanf (buf, "%d %d %d", &pos, &hoffset, &len);
		g_free (buf);

		/* Go to the correct line in the souce file */
		fseek (source, pos, SEEK_SET);

		next = c2_db_new (mailbox);

		for (i=0;;)
		{
			if (!(buf = c2_fd_get_line (source)))
				break;

			i += strlen (buf);

			if (i >= len)
			{
				g_free (buf);
				break;
			}

			/* Check if this is still the header */
			if (!strlen (buf))
			{
				g_free (buf);
				break;
			}

			if (!next->subject && c2_strneq (buf, "Subject:", 8))
				next->subject = get_field_value (buf, 8);
			else if (!next->from && c2_strneq (buf, "From:", 5))
				next->from = get_field_value (buf, 5);
			else if (!next->account && c2_strneq (buf, "X-Account:", 10))
				next->account = get_field_value (buf, 10);
			else if (!next->date && c2_strneq (buf, "From ", 5))
				next->date = get_date_field_from_from (buf);
			else if (!next->date && c2_strneq (buf, "Date:", 5))
				next->date = get_date_field_from_date (buf);

			/* TODO: marked and state, check from other mailers
			 * how those are handled */
			next->position = next->mid = l;
			
			g_free (buf);
		}

		if (current)
			current->next = next;

		if (!head)
			head = next;

		current = next;
	}

	mailbox->db = head;
	
	return 0;
}

static gchar *
get_field_value (const gchar *field, gint offset)
{
	const gchar *ptr;
	
	for (ptr = field+offset; *ptr != '\0' && *ptr != ' '; ptr++)
		;

	return g_strdup (ptr);
}

static time_t
get_date_field_from_date (const gchar *field)
{
	gchar *strdate;
	time_t date;

	strdate = get_field_value (field, 5);
	if ((date = c2_date_parse (strdate)) == -1)
		if ((date = c2_date_parse_fmt2 (strdate)) == -1)
			date = 0;

	g_free (strdate);

	return date;
}

static time_t
get_date_field_from_from (const gchar *field)
{
	const gchar *ptr;

	for (ptr = field+5; *ptr == ' ' && *ptr != '\0'; ptr++)
		;

	for (; *ptr != ' ' && *ptr != '\0'; ptr++)
		;

	for (; *ptr == ' ' && *ptr != '\0'; ptr++)
		;
	
	if (!ptr)
		return 0;

	/* We don´t try with other than c2_date_parse_fmt3 because this format
	 * must be the first one and we don´t want to waste time
	 * checking for something we know is not there.
	 */
	return c2_date_parse_fmt3 (ptr);
}

gint
c2_db_spool_create_structure (C2Mailbox *mailbox)
{
	gchar *path = g_strconcat (g_get_home_dir (), C2_HOME, mailbox->name, ".mbx", NULL);
	gint fd;

	c2_return_val_if_fail (mailbox->type == C2_MAILBOX_SPOOL, -1, C2EDATA);

	if ((fd = open (mailbox->protocol.spool.path, O_CREAT | O_WRONLY, 0600)) < 0)
	{
		c2_error_set (-errno);
		g_warning (_("Unable to create structure for Spool Db (1): %s\n"), c2_error_get (c2_errno));
		g_free (path);
		return -1;
	}
	close (fd);

	if ((fd = open (path, O_CREAT, 0600)) < 0)
	{
		c2_error_set (-errno);
		g_warning (_("Unable to create structure for Spool Db (2): %s\n"), c2_error_get (c2_errno));
		g_free (path);
		return -1;
	}
	close (fd);
	
	c2_db_spool_update_structure (mailbox);
	
	return 0;
}

gint
c2_db_spool_update_structure (C2Mailbox *mailbox)
{
	gchar *buf;
	gint start = -1, length;
	FILE *source, *index;
	gboolean reading_header;
	
	c2_return_val_if_fail (mailbox->type == C2_MAILBOX_SPOOL, -1, C2EDATA);
	
	/* Open the source file (read-only). */
	if (!(source = fopen (mailbox->protocol.spool.path, "r")))
	{
		c2_error_set (-errno);
		g_warning (_("Unable to update structure for Spool Db: %s\n"), c2_error_get (c2_errno));
		return -1;
	}
	
	/* Open the index file (write-only). */
	buf = g_strconcat (g_get_home_dir (), C2_HOME, mailbox->name, ".mbx", NULL);
	if (!(index = fopen (buf, "w")))
	{
		c2_error_set (-errno);
		g_warning (_("Unable to update structure for Spool Db: %s\n"), c2_error_get (c2_errno));
		fclose (source);
		g_free (buf);
		return -1;
	}
	g_free (buf);
	
	/* Go through the source file looking for a From. */
	reading_header = TRUE;
	for (;;)
	{
		if (!(buf = c2_fd_get_line (source)))
		{
			/* We found the end of the last mail */
			if (!reading_header)
				fprintf (index, "%d\n", ftell (source)-start);
			break;
		}

		if (reading_header && !strlen (buf))
		{
			fprintf (index, "%d ", ftell (source)-start);
			g_free (buf);
			reading_header = FALSE;
			continue;
		}

		/* Check if this line is important (must start with 'From ') */
		if (c2_strnne (buf, "From ", 5))
		{
			
			g_free (buf);
			continue;
		}

		if (start < 0)
		{
			/* Then this is the first line we read from the mail */
			start = ftell (source)-strlen (buf)-1;
			fprintf (index, "%d ", start);
			fprintf (stdout, "mail starts at position: %d\n", start);
			reading_header = TRUE;
		} else
		{
			/* Then this is the last line we read from the mail */
			printf ("length = %d-%d-%d-1 (%d)\n", ftell (source), start, strlen (buf), 
						ftell (source)-start-strlen (buf)-1);
			length = ftell (source)-start-strlen (buf)-1;
			fprintf (index, "%d\n", length);
			fprintf (stdout, "length of mail is: %d\n", length);

			/* And we also got the start of the next mail */
			start = start+length;
			fprintf (index, "%d ", start);
			fprintf (stdout, "mail starts at position: %d\n", start);
			reading_header = TRUE;
		}

		g_free (buf);
	}
	
	/* Close everything and return 0. */
	fclose (source);
	fclose (index);

	return 0;
}

gint
c2_db_spool_remove_structure (C2Mailbox *mailbox)
{
	gchar *buf;
	
	c2_return_val_if_fail (mailbox->type == C2_MAILBOX_SPOOL, -1, C2EDATA);
	
	buf = g_strconcat (g_get_home_dir (), C2_HOME, mailbox->name, ".mbx", NULL);
	unlink (mailbox->protocol.spool.path);
	unlink (buf);
	g_free (buf);
}

/**
 * c2_db_spool_sanity_check
 * @mailbox: Mailbox where to act.
 *
 * This function will check if everything looks
 * Ok in the Spool Db.
 * 
 * Return Value:
 * 0 if everything is Ok or -1.
 **/
gint
c2_db_spool_sanity_check (C2Mailbox *mailbox)
{
	/* This works by calculating the size the file
	 * should have (using the index file) and checking
	 * if they match.
	 */
	FILE *index;
	gint calc_size = 0;
	gchar *buf, *size;
	struct stat st;

	c2_return_val_if_fail (mailbox->type == C2_MAILBOX_SPOOL, -1, C2EDATA);

	/* Open the index file (read-only). */
	buf = g_strconcat (g_get_home_dir (), C2_HOME, mailbox->name, ".mbx", NULL);
	if (!(index = fopen (buf, "r")))
	{
		c2_error_set (-errno);
		g_warning (_("Unable to update structure for Spool Db: %s\n"), c2_error_get (c2_errno));
		g_free (buf);
		return -1;
	}
	g_free (buf);

	for (;;)
	{
		if (!(buf = c2_fd_get_line (index)))
			break;

		size = c2_str_get_word (2, buf, ' ');
		g_free (buf);

		calc_size += atoi (size);
		printf ("%d\n", calc_size);
		g_free (size);
	}
	fclose (index);

	if (stat (mailbox->protocol.spool.path, &st) < 0)
		return -1;
	
	calc_size *= sizeof (gchar);
	printf ("%d != %d = %d\n", st.st_size, calc_size, -(st.st_size != calc_size));
	return -(st.st_size != calc_size);
}

static void
get_index_info (C2Mailbox *mailbox, gint mail, gint *start, gint *hoffset, gint *length)
{
	FILE *fd;
	gchar *buf;
	gint i;

	buf = g_strconcat (g_get_home_dir (), C2_HOME, mailbox->name, ".mbx", NULL);
	if (!(fd = fopen (buf, "r")))
	{
		c2_error_set (-errno);
		g_warning (_("Unable to update structure for Spool Db: %s\n"), c2_error_get (c2_errno));
		g_free (buf);
		return;
	}
	g_free (buf);

	for (i = 0; i < mail; i++)
	{
		if (!(buf = c2_fd_get_line (fd)))
			return;
		
		if (i != mail-1)
			g_free (buf);
	}

	sscanf (buf, "%d %d %d", start, hoffset, length);

	g_free (buf);
	fclose (fd);
}

static void
get_source_desc (C2Mailbox *mailbox, FILE **source)
{
	if (!(*source = fopen (mailbox->protocol.spool.path, "r")))
	{
		c2_error_set (-errno);
		g_warning (_("Unable to update structure for Spool Db: %s\n"), c2_error_get (c2_errno));
		return;
	}
}
#endif
