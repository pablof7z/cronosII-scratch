/*  Cronos II - The GNOME mail client
 *  Copyright (C) 2000-2001 Pablo Fernández
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
 * 		* Pablo Fernández
 * Code of this file by:
 * 		* Pablo Fernández
 * 		* Bosko Blagojevic
 **/
#include <glib.h>
//#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>

#include "error.h"
#include "i18n.h"
#include "utils.h"
#include "utils-file.h"

/**
 * c2_get_tmp_file
 * @template: Template to use for the file name, %NULL if
 *            the name does not require any special name.
 *            Read mkstemp(3) before using this function.
 *
 * Gets a tmp file path.
 *
 * Return Value:
 * A freeable string with the path to the tmp file.
 **/
gchar *
c2_get_tmp_file (const gchar *template)
{
	gchar *temp;
	static gchar *dir = NULL;
	gint fd;

	if (!dir)
		dir = g_get_tmp_dir ();
	
	temp = g_strdup_printf ("%s" G_DIR_SEPARATOR_S "%s", dir, template ? template : "c2-tmp.XXXXXXX");
	fd = mkstemp (temp);
	close (fd);
	
	return temp;
}

/**
 * c2_get_file
 * @path: Path to load.
 * @string: String where file will be loaded.
 *
 * This function will load a file.
 *
 * Return Value:
 * Length of the file loaded.
 **/
gint
c2_get_file (const gchar *path, gchar **string)
{
	FILE *fd;
	struct stat buf;
	gint length;

	c2_return_val_if_fail (path, -1, C2EDATA);

	if (!(fd = fopen (path, "r")))
	{
		c2_error_set (-errno);
		return -1;
	}

	stat (path, &buf);

	length = ((gint) buf.st_size * sizeof (gchar));

	*string = g_new0 (gchar, length);

	fread (*string, sizeof (gchar), length, fd);
	fclose (fd);

	return length;
}

/**
 * c2_fd_get_line
 * @fd: File descriptor.
 *
 * Will get a line from the file descriptor
 * @fd.
 *
 * Return Value:
 * The line read or NULL in case of error.
 **/
#if 0
gchar *
c2_fd_get_line (FILE *fd)
{
	gchar buf[1024];
	gchar byte;
	gchar *retval = NULL, *ptr;
	gint i;

restart_loop:
L	buf[0] = 0;
L	for (i = 0; i<1024; i++)
	{
L		buf[i+1] = 0;
L		
L		if ((byte = fgetc (fd)) == EOF)
		{
L			fseek (fd, 0, SEEK_END);
			break;
		}
		if (byte == '\n')
			break;
L
L		buf[i] = byte;

		if (i == 1023)
		{
L			if (retval)
			{
L				ptr = g_strconcat (retval, buf, NULL);
L				g_free (retval);
L				retval = ptr;
			} else
			{
L				retval = g_strdup (buf);
			}
L			C2_DEBUG (retval);
L			goto restart_loop;
		}
	}
L
L	if (retval)
	{
L		ptr = g_strconcat (retval, buf, NULL);
L		g_free (retval);
L		retval = ptr;
L	} else
	{
L		retval = g_strdup (buf);
	}
	if (*retval == '\0')
	{
L		return NULL;
	}

L	return retval;
}
#else
gchar *
c2_fd_get_line (FILE *fd)
{
	glong pos = ftell (fd);
	gint len = 1;
	gint i;
	gchar buf;
	gchar *str;
	gchar *ptr;

	c2_return_val_if_fail (pos >= 0, NULL, -errno);
	
	for (;;)
	{
		if ((buf = fgetc (fd)) == EOF)
		{
			if (len == 1)
				return NULL;
			break;
		}
		if (buf == '\n')
			break;
		len++;
	}

	str = g_new0 (gchar, len+1);
	fseek (fd, pos, SEEK_SET);
	
	for (i=0, ptr = str; i < len; i++)
	{
		if ((buf = fgetc (fd)) == EOF)
		{
			fseek (fd, 0, SEEK_END);
			break;
		}
		if (buf == '\n')
			break;
		*(ptr++) = buf;
	}
	*ptr = '\0';
	
	return str;
}
#endif

/**
 * c2_fd_get_word
 * @fd: File Descriptor.
 *
 * Gets a word from the file descriptor.
 *
 * Return Value:
 * A freeable string with the word or NULL.
 **/
gchar *
c2_fd_get_word (FILE *fd)
{
	GString *str;
	gboolean inside_quotes = FALSE;
	gchar buf;
	gchar *rtn;
	
	/* Move to a valid char */
	for (;;)
	{
		if ((buf = fgetc (fd)) == EOF) return NULL;
		if (buf >= 48 && buf <= 57) break;
		if (buf >= 65 && buf <= 90) break;
		if (buf >= 97 && buf <= 122) break;
		if (buf == 34) break;
		if (buf == '*')
		{
			if (c2_fd_move_to (fd, '\n', 1, TRUE, TRUE) == EOF) fseek (fd, -1, SEEK_CUR);
			return c2_fd_get_word (fd);
		}
	}
	
	fseek (fd, -1, SEEK_CUR);
	str = g_string_new (NULL);
	
	for (;;)
	{
		if ((buf = fgetc (fd)) == EOF)
		{
			if (inside_quotes)
				g_warning (_("Bad syntaxes, quotes aren't closed.\n"));
			break;
		}
		
		if (buf == 34)
		{
			if (inside_quotes) inside_quotes = FALSE;
			else inside_quotes = TRUE;
			continue;
		}
		
		if (!inside_quotes && buf == ' ') break;
		if (!inside_quotes && buf == '\n') break;
		
		g_string_append_c (str, buf);
	}
	
	fseek (fd, -1, SEEK_CUR);
	rtn = str->str;
	g_string_free (str, FALSE);
	
	return rtn;
}

/**
 * c2_file_binary_copy
 * @from_path: Path to the file that is going to be copied.
 * @target_path: Path to the file where is going to be done the copy.
 *
 * This function copies a file into other
 * in a binary way.
 *
 * Return Value:
 * 0 is success, -1 on error (c2_errno gets a proper value).
 **/
gint
c2_file_binary_copy (const gchar *from_path, const gchar *target_path)
{
	gchar buf[100];
	FILE *frm;
	FILE *dst;
	
	c2_return_val_if_fail (from_path, 1, C2EDATA);
	c2_return_val_if_fail (target_path, 1, C2EDATA);

	if (!(frm = fopen (from_path, "rt")))
	{
		c2_error_set (-errno);
		return -1;
	}
	
	if (!(dst = fopen (target_path, "wt")))
	{
		c2_error_set (-errno);
		fclose (frm);
		return -1;
	}
	
	for (;!feof (frm);)
	{
		memset (buf, 0, sizeof (buf)*sizeof (gchar));
		fwrite (buf, sizeof (gchar),
			fread (buf, sizeof (gchar), sizeof (buf), frm), dst);
	}
	
	fclose (frm);
	fclose (dst);
	
	return 0;
}

/**
 * c2_file_binary_move
 * @from_path: Path to the file that is going to be moved.
 * @target_path: Path to the file where is going to be done the move.
 *
 * This function moves a file into other
 * in a binary way.
 *
 * Return Value:
 * 0 is success, -1 on error (c2_errno gets a proper value).
 **/
gint
c2_file_binary_move (const gchar *from_path, const gchar *target_path)
{
	if (c2_file_binary_copy (from_path, target_path))
		return 1;
	unlink (from_path);
	return 0;
}

/**
 * c2_file_exists
 * @file: A pointer to a character object containing the file path.
 *
 * Checks if the file exists.
 *
 * Return Value:
 * Return TRUE if the files exists or FALSE if it doesn't.
 **/
gboolean
c2_file_exists (const gchar *file)
{
	struct stat st;
	
	c2_return_val_if_fail (file, -1, C2EDATA);
	
	if (stat (file, &st) < 0)
		return -1;
	
	return 0;
}

/**
 * c2_file_is_directory
 * @file: A pointer to a character object containing the file path.
 *
 * Checks if the file is a directory.
 *
 * Return Value:
 * Return TRUE if the files is a directory or FALSE if it isn't.
 **/
gboolean
c2_file_is_directory (const gchar *file)
{
	struct stat st;
	
	c2_return_val_if_fail (file, FALSE, C2EDATA);
	
	if (stat (file, &st) < 0) return FALSE;
	return S_ISDIR (st.st_mode);
}

/**
 * c2_fd_move_to
 * @fp: File descriptor.
 * @c: Character to look for.
 * @cant: Times to repeat the search.
 * @forward: TRUE if you want the search to happen forward.
 * @next: TRUE if you want the file carrier to be set after the
 *        requested character.
 *
 * This function will search for a character and move the file carrier
 * all over the file descriptor searching for it with the previously
 * described modifiers.
 *
 * Return Value:
 * TRUE if the search was successfully or FALSE.
 **/
gboolean
c2_fd_move_to (FILE *fp, gchar c, guint8 cant, gboolean forward, gboolean next)
{
	gchar s=' ';
	int ia;
	guint8 go;
	
	if (c == EOF)
	{
		for (;;)
			if ((s = getc (fp)) == EOF)
				break;
		if (!next)
			if (fseek (fp, -1, SEEK_CUR) < 0)
			{
				c2_error_set (-errno);
				return FALSE;
			}
		return TRUE;
	}
	
	for (ia = 0, go = 0;; ia++)
	{
		if ((s = getc (fp)) == EOF)
			break;
		if (s == c)
			if (++go == cant)
				break;
		
		if (!forward)
			if (fseek (fp, -2, SEEK_CUR) < 0)
			{
				c2_error_set (-errno);
				return FALSE;
			}
	}
	
	
	if (s != c)
		return FALSE;
	if (!next)
	{
		if (fseek (fp, -1, SEEK_CUR) < 0)
		{
			c2_error_set (-errno);
			return FALSE;
		}
		ia -= 1;
	}
	
	return TRUE;
}

