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
#include <gnome.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "error.h"
#include "utils.h"

/**
 * c2_strcaseeq
 * @fst: A pointer to a string.
 * @snd: A pointer to a string.
 *
 * Compares both string case-sensitive.
 * Use this function family instead of strcmp
 * when possible since this function will terminate
 * immediately when it founds something different,
 * thus why is faster.
 *
 * Return Value:
 * A boolean indicating whether the string where
 * equal.
 **/
gboolean
c2_strcaseeq (const gchar *fst, const gchar *snd)
{
	const gchar *ptr[2];
	
	for (ptr[0] = fst, ptr[1] = snd; ptr[0] || ptr[1]; ptr[0]++, ptr[1]++)
	{
		if (*ptr[0] == '\0')
		{
			if (*ptr[1] == '\0') return TRUE;
			else return FALSE;
		}
		if (*ptr[1] == '\0') return FALSE;
		if (*ptr[0] != *ptr[1]) return FALSE;
	}
	return TRUE;
}

/**
 * c2_strncaseeq
 * @fst: A pointer to a string.
 * @snd: A pointer to a string.
 * @length: A number indicating how many of the first characters
 * 	    to compare.
 *
 * Compares both string case-sensitive in the first
 * @length characters.
 * Use this function family instead of strcmp
 * when possible since this function will terminate
 * immediately when it founds something different,
 * thus why is faster.
 *
 * Return Value:
 * A boolean indicating whether the string where
 * equal.
 **/
gboolean
c2_strncaseeq (const gchar *fst, const gchar *snd, gint length)
{
	const gchar *ptr[2];
	gint i;
	
	for (ptr[0] = fst, ptr[1] = snd, i = 0; i < length; ptr[0]++, ptr[1]++, i++)
	{
		if (*ptr[0] == '\0')
		{
			if (*ptr[1] == '\0') return TRUE;
			else return FALSE;
		}
		if (*ptr[1] == '\0') return FALSE;
		if (*ptr[0] != *ptr[1]) return FALSE;
	}
	return TRUE;
}

/**
 * c2_streq
 * @fst: A pointer to a string.
 * @snd: A pointer to a string.
 *
 * Compares both string case-insensitive.
 * Use this function family instead of strcmp
 * when possible since this function will terminate
 * immediately when it founds something different,
 * thus why is faster.
 *
 * Return Value:
 * A boolean indicating whether the string where
 * equal.
 **/
gboolean
c2_streq (const gchar *fst, const gchar *snd)
{
	const gchar *ptr[2];
	gchar *_fst;
	gchar *_snd;
	
	if (!fst && snd) return FALSE;
	if (fst && !snd) return FALSE;
	if (!fst && !snd) return TRUE;
	_fst = g_strdup (fst);
	g_strup (_fst);
	_snd = g_strdup (snd);
	g_strup (_snd);
	
	for (ptr[0] = _fst, ptr[1] = _snd; ptr[0] || ptr[1]; ptr[0]++, ptr[1]++)
	{
		if (*ptr[0] == '\0')
		{
			if (*ptr[1] == '\0') return TRUE;
			else return FALSE;
		}
		if (*ptr[1] == '\0') return FALSE;
		if (*ptr[0] != *ptr[1]) return FALSE;
	}
	g_free (_fst);
	g_free (_snd);
	return TRUE;
}

/**
 * c2_strneq
 * @fst: A pointer to a string.
 * @snd: A pointer to a string.
 * @length: A number indicating how many of the first characters
 * 	    to compare.
 *
 * Compares both string case-insensitive in the first
 * @length characters.
 * Use this function family instead of strcmp
 * when possible since this function will terminate
 * immediately when it founds something different,
 * thus why is faster.
 *
 * Return Value:
 * A boolean indicating whether the string where
 * equal.
 **/
gboolean
c2_strneq (const gchar *fst, const gchar *snd, gint length)
{
	const gchar *ptr[2];
	gchar *_fst;
	gchar *_snd;
	gint i;
	
	if (!fst && snd) return FALSE;
	if (fst && !snd) return FALSE;
	if (!fst && !snd) return TRUE;
	if (length < 1) return TRUE;
	
	_fst = g_strdup (fst);
	g_strup (_fst);
	_snd = g_strdup (snd);
	g_strup (_snd);
	
	for (ptr[0] = _fst, ptr[1] = _snd, i = 0; i < length; ptr[0]++, ptr[1]++, i++)
	{
		if (*ptr[0] == '\0')
		{
			if (*ptr[1] == '\0') return TRUE;
			else return FALSE;
		}
		if (*ptr[1] == '\0') return FALSE;
		if (*ptr[0] != *ptr[1]) return FALSE;
	}
	g_free (_fst);
	g_free (_snd);
	return TRUE;
}

/**
 * c2_strstr_case_insensitive
 * @haystack: String where to do the searching.
 * @needle: String to search for.
 *
 * This functions is the case insensitive strstr.
 *
 * Return Value:
 * A pointer to the first match of @needle within
 * @haystack.
 **/
const gchar *
c2_strstr_case_insensitive (const gchar *haystack, const gchar *needle)
{
	gchar *my_haystack;
	gchar *my_needle;
	const gchar *ptr;

	c2_return_val_if_fail (haystack, NULL, C2EDATA);
	c2_return_val_if_fail (needle, NULL, C2EDATA);

	my_haystack = g_strdup (haystack); g_strup (my_haystack);
	my_needle = g_strdup (needle); g_strup (my_needle);

	ptr = strstr (my_haystack, my_needle);

	if (ptr)
		ptr = haystack + (ptr - my_haystack);

	g_free (my_haystack);
	g_free (my_needle);

	return ptr;
}

/**
 * c2_str_replace_all
 * @or_string: A pointer to a string to have the replacements made on it
 * @se_string: The string to replace
 * @re_string: The string to replace @se_string with
 *
 * Search @or_string and finds all instances of se_string, and replaces
 * them with re_string in a new freeable string that is returned.
 *
 * Return Value:
 * A new string with the replacements that should be freed when no 
 * longer needed.
 **/
gchar *
c2_str_replace_all (const gchar *or_string, const gchar *se_string, const gchar *re_string)
{
	gchar *ptr;
	gchar *str, *strptr;
	gint length = 0;
	
	c2_return_val_if_fail (or_string, NULL, C2EDATA);
	g_return_val_if_fail (se_string, NULL);
	g_return_val_if_fail (re_string, NULL);
	
	for (ptr = C2_CHAR (or_string);;)
	{
		if (*ptr == '\0') break;
		if (c2_strneq (ptr, se_string, strlen (se_string)))
		{
			length += strlen (re_string);
			ptr += strlen (se_string);
		} else
		{
			length++;
			ptr++;
		}
	}
	
	str = g_new0 (gchar, length+1);
	str[length] = '\0';
	
	for (ptr = C2_CHAR (or_string), strptr = str;;)
	{
		if (*ptr == '\0') break;
		if (c2_strneq (ptr, se_string, strlen (se_string)))
		{
			strcpy (strptr, re_string);
			strptr += strlen (re_string);
			ptr += strlen (se_string);
		} else
		{
			*strptr = *ptr;
			strptr++;
			ptr++;
		}
	}
	
	return str;
}

/**
 * c2_str_strip_enclosed
 * @str: A pointer to the target string.
 * @open: A character to be used as the opening sign.
 * @close: A character to be used as the closing sign.
 *
 * If the string @str starts with @open and finish with @close
 * this function will remove the first and last character in
 * a copy of @str.
 *
 * Return Value:
 * A copy of @str which will not start with @open and finish
 * with @close.
 **/
gchar *
c2_str_strip_enclosed (const gchar *str, gchar open, gchar close)
{
	gchar *ptr;
	
	c2_return_val_if_fail (str, NULL, C2EDATA);
	
	if (*str == open && *(str+strlen (str)-1) == close)
		ptr = g_strndup (str+1, strlen (str)-2);
	else
		ptr = g_strdup (str);
	
	return ptr;
}

/**
 * c2_str_get_line
 * @str: A pointer to an string object where the next line should be searched.
 *
 * Searches for the next line.
 * Note that this function should be used with a pointer
 * to the original string in a loop by moving the pointer
 * ptr += strlen (return value).
 *
 * Return Value:
 * A freeable string containing the next line of the string @str.
 **/
gchar *
c2_str_get_line (const gchar *str)
{
	gint len=0;
	gint i;
	gchar *string;
	gchar *strptr, *ptr;
	
	c2_return_val_if_fail (str, NULL, C2EDATA);
	
	for (strptr = C2_CHAR (str);; strptr++)
	{
		if (*strptr == '\0')
		{
			if (len == 0) return NULL;
			break;
		}
		len++;
		if (*strptr == '\n') break;
	}
	
	string = g_new0 (gchar, len+1);
	
	for (i=0, ptr = string, strptr = C2_CHAR (str); i <= len; i++, strptr++)
	{
		if (*strptr == '\0')
		{
			break;
		}
		*(ptr++) = *strptr;
		if (*strptr == '\n') break;
	}
	*ptr = '\0';
	
	return string;
}

/**
 * c2_str_get_word
 * @word_n: Position of the desired word.
 * @str: A pointer to a string containing the target.
 * @ch: The character to use as a separator.
 *
 * Finds the word in position @word_n in @str.
 *
 * Return Value:
 * A freeable string containing the desired word or
 * NULL in case it doesn't exists.
 **/
gchar *
c2_str_get_word (guint8 word_n, const gchar *str, gchar ch)
{
  guint8 Ai=0;
  gchar *c = NULL;
  gboolean record = FALSE;
  gboolean quotes = FALSE;
  GString *rtn;
 
  if (str == NULL) return NULL;
  if (word_n == 0) record = TRUE;
  rtn = g_string_new (NULL);
  
  for (c = C2_CHAR (str); *c != '\0'; c++)
  {
    if (*c == '"')
	 {
      if (quotes) quotes = FALSE;
      else quotes = TRUE;
      continue;
    }
    
    if (record && *c == '"') break;
    if (record && *c == ch && !quotes) break; /* If the word ends, break */
    if (record) g_string_append_c (rtn, *c); /* Record this character */
    if (*c == ch && !quotes) Ai++;
    if (*c == ch && Ai == word_n && !quotes) record = TRUE;
  }
  
  c = rtn->str;
  g_string_free (rtn, FALSE);
  
  return c;
}

/**
 * c2_get_tmp_file
 *
 * Gets a tmp file path.
 *
 * Return Value:
 * A freeable string with the path to the tmp file.
 **/
gchar *
c2_get_tmp_file (void)
{
	gchar *temp;
	static gchar *dir = NULL;
	gint fd;

	if (!dir)
		dir = g_get_tmp_dir ();
	
	temp = g_strdup_printf ("%s" G_DIR_SEPARATOR_S "c2-tmp.XXXXXXX", dir);
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
 * 0 is success, 1 on error (c2_errno gets a proper value).
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
		return FALSE;
	}
	
	if (!(dst = fopen (target_path, "wt")))
	{
		c2_error_set (-errno);
		fclose (frm);
		return FALSE;
	}
	
	for (;;)
	{
		if (fread (buf, sizeof (gchar), sizeof (buf), frm) < sizeof (buf)-1)
			break;
		fwrite (buf, sizeof (gchar), sizeof (buf), dst);
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
 * 0 is success, 1 on error (c2_errno gets a proper value).
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
	
	c2_return_val_if_fail (file, FALSE, C2EDATA);
	
	if (stat (file, &st) < 0) return FALSE;
	return TRUE;
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
		for (;;) if ((s = getc (fp)) == EOF) break;
		if (!next) if (fseek (fp, -1, SEEK_CUR) < 0) return FALSE;
		return TRUE;
	}
	
	for (ia = 0, go = 0; s != c; ia++)
	{
		if ((s = getc (fp)) == EOF) break;
		if (s == c)
		{
			if (++go == cant) break;
		}
		
		if (!forward) if (fseek (fp, -2, SEEK_CUR) < 0) return FALSE;
	}
	
	
	if (s != c) return FALSE;
	if (!next)
	{
		if (fseek (fp, -1, SEEK_CUR) < 0) return FALSE;
		ia -= 1;
	}
	
	return TRUE;
}
