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
 * Comparates both string case-sensitive.
 * Use this function family instead of strcmp
 * when posible since this function will terminate
 * immeditely when it founds something different,
 * thus why is faster.
 *
 * Return Value:
 * A boolean indicating wheter the string where
 * equal.
 **/
gboolean
c2_strcaseeq (const gchar *fst, const gchar *snd) {
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
 * 	    to comparate.
 *
 * Comparates both string case-sensitive in the first
 * @length characters.
 * Use this function family instead of strcmp
 * when posible since this function will terminate
 * immeditely when it founds something different,
 * thus why is faster.
 *
 * Return Value:
 * A boolean indicating wheter the string where
 * equal.
 **/
gboolean
c2_strncaseeq (const gchar *fst, const gchar *snd, gint length) {
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
 * Comparates both string case-insensitive.
 * Use this function family instead of strcmp
 * when posible since this function will terminate
 * immeditely when it founds something different,
 * thus why is faster.
 *
 * Return Value:
 * A boolean indicating wheter the string where
 * equal.
 **/
gboolean
c2_streq (const gchar *fst, const gchar *snd) {
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
 * 	    to comparate.
 *
 * Comparates both string case-insensitive in the first
 * @length characters.
 * Use this function family instead of strcmp
 * when posible since this function will terminate
 * immeditely when it founds something different,
 * thus why is faster.
 *
 * Return Value:
 * A boolean indicating wheter the string where
 * equal.
 **/
gboolean
c2_strneq (const gchar *fst, const gchar *snd, gint length) {
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
 * c2_str_replace_all
 * @or_string: A pointer to a string to have the replacements made on it
 * @se_string: The string to replace
 * @re_string: The string to replace @se_string with
 *
 * Search @or_string and finds all instances of se_string, and replaces
 * them with re_string in a new freeble string that is returned.
 *
 * Return Value:
 * A new string with the replacements that should be freed when no 
 * longer needed.
 **/
gchar *
c2_str_replace_all (const gchar *or_string, const gchar *se_string, const gchar *re_string) {
	gchar *ptr;
	gchar *str, *strptr;
	gint length = 0;
	
	c2_return_val_if_fail (or_string, NULL, C2EDATA);
	g_return_val_if_fail (se_string, NULL);
	g_return_val_if_fail (re_string, NULL);
	
	for (ptr = CHAR (or_string);;)
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
	
	for (ptr = CHAR (or_string), strptr = str;;)
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
 * c2_str_get_line
 * @str: A pointer to an string object where the next line should be searched.
 *
 * Searchs for the next line.
 * Note that this function should be used with a pointer
 * to the original string in a loop by moving the pointer
 * ptr += strlen (return value).
 *
 * Return Value:
 * A freeable string containing the next line of the string @str.
 **/
gchar *
c2_str_get_line (const gchar *str) {
	gint len=0;
	gint i;
	gchar *string;
	gchar *strptr, *ptr;
	
	c2_return_val_if_fail (str, NULL, C2EDATA);
	
	for (strptr = CHAR (str);; strptr++)
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
	
	for (i=0, ptr = string, strptr = CHAR (str); i <= len; i++, strptr++)
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
c2_str_get_word (guint8 word_n, const gchar *str, gchar ch) {
  guint8 Ai=0;
  gchar *c = NULL;
  gboolean record = FALSE;
  gboolean quotes = FALSE;
  GString *rtn;
 
  if (str == NULL) return NULL;
  if (word_n == 0) record = TRUE;
  rtn = g_string_new (NULL);
  
  for (c = CHAR (str); *c != '\0'; c++)
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
 * c2_fd_get_line
 * @fd: File descriptor.
 *
 * Will get a line from the file descriptor
 * @fd.
 *
 * Return Value:
 * The line readed or NULL in case of error.
 **/
gchar *
c2_fd_get_line (FILE *fd) {
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
			if (len == 1) return NULL;
			break;
		}
		if (buf == '\n') break;
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
		if (buf == '\n') break;
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
c2_fd_get_word (FILE *fd) {
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
				g_warning (_("Bad syntaxis, quotes aren't closed.\n"));
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
 * c2_file_exists
 * @file: A pointer to a character object containing the file path.
 *
 * Checks if the file exists.
 *
 * Return Value:
 * Return TRUE if the files exists or FALSE if it doesn't.
 **/
gboolean
c2_file_exists (const gchar *file) {
	struct stat st;
	
	c2_return_val_if_fail (file, FALSE, C2EDATA);
	
	if (stat (file, &st) < 0) return FALSE;
	return TRUE;
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
 * descripted modificators.
 *
 * Return Value:
 * TRUE if the search was succesfully or FALSE.
 **/
gboolean
c2_fd_move_to (FILE *fp, gchar c, guint8 cant, gboolean forward, gboolean next) {
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
