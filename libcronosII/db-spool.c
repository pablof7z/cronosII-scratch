/*  Cronos II Mail Client /libcronosII/db-spool.c
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
#include "db-spool.h"
#include "mailbox.h"
#include "utils.h"
/*hmmm this is almost beautifull in its minimalism -pete ;)*/
/*yeah, is the shortest module I ever see! but it doesn't have any bugs! :) -pablo */
/*Ah you coders bugs bugs bugs. No code =nobugs. Code bad code use memory. :(*/
/*Oh, yes, the perfect armony of "the 0 bytes length of the file", just perfect :) -pablo */

gint
c2_db_spool_load (C2Mailbox *mailbox)
{
	gchar *source_path = mailbox->protocol.spool.path;
	gchar *index_path = g_strconcat (g_get_home_dir (), C2_HOME, mailbox->name, ".mbx", NULL);
	FILE *source, *index;
	gchar *buf;

	c2_return_val_if_fail (mailbox->type == C2_MAILBOX_SPOOL, -1, C2EDATA);
	c2_db_spool_sanity_check (mailbox);

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
	return 0;
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
	gint start = -1, length, i;
	FILE *source, *index;
	
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
	for (i = 1;;)
	{
		if (!(buf = c2_fd_get_line (source)))
		{
			/* We found the end of the last mail */
			fprintf (index, "%d\n", ftell (source)-start);
			fprintf (stdout, "length of last mail is: %d\n", ftell (source)-start);
			break;
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
			fprintf (index, "%d: %d\r", i++, start);
			fprintf (stdout, "mail starts at position: %d\n", start);
		} else
		{
			/* Then this is the last line we read from the mail */
			length = ftell (source)-strlen (buf)-1-start;
			fprintf (index, "%d\n", length);
			fprintf (stdout, "length of mail is: %d\n", length);

			/* And we also got the start of the next mail */
			start = start+length;
			fprintf (index, "%d: %d\r", i++, start);
			fprintf (stdout, "mail starts at position: %d\n", start);
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
	c2_return_val_if_fail (mailbox->type == C2_MAILBOX_SPOOL, -1, C2EDATA);
	
	unlink (mailbox->protocol.spool.path);
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

		size = c2_str_get_word (1, buf, '\r');
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

/*should this return anything when done ?? -pete..(I would add it if I knew how) */
/*you're right... */
