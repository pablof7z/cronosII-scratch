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
#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>

#include "error.h"
#include "i18n.h"
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
			if (*ptr[1] == '\0')
				return TRUE;
			else
				return FALSE;
		}
		if (*ptr[1] == '\0')
			return FALSE;
		if (*ptr[0] != *ptr[1])
			return FALSE;
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
		if (*ptr[0] != *ptr[1])
			return FALSE;
		if (*ptr[0] == '\0')
		{
			if (*ptr[1] == '\0')
				return TRUE;
			else
				return FALSE;
		}
		if (*ptr[1] == '\0')
			return FALSE;
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
	gboolean retval = TRUE;
	
	if (!fst && snd)
		return FALSE;
	if (fst && !snd)
		return FALSE;
	if (!fst && !snd)
		return TRUE;
	_fst = g_strdup (fst);
	g_strup (_fst);
	_snd = g_strdup (snd);
	g_strup (_snd);
	
	for (ptr[0] = _fst, ptr[1] = _snd; ptr[0] || ptr[1]; ptr[0]++, ptr[1]++)
	{
		if (*ptr[0] != *ptr[1])
		{
			retval = FALSE;
			break;
		}
		if (*ptr[0] == '\0')
		{
			if (*ptr[1] == '\0')
				retval = TRUE;
			else
				retval = FALSE;
			break;
		}
		if (*ptr[1] == '\0')
		{
			retval = FALSE;
			break;
		}
	}
	
	g_free (_fst);
	g_free (_snd);
	return retval;
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
	gboolean retval = TRUE;
	
	if (!fst && snd)
	{
		return FALSE;
	}
	if (fst && !snd)
		return FALSE;
	if (!fst && !snd)
		return TRUE;
	if (length < 1)
		return TRUE;
	
	_fst = g_strdup (fst);
	g_strup (_fst);
	_snd = g_strdup (snd);
	g_strup (_snd);
	
	for (ptr[0] = _fst, ptr[1] = _snd, i = 0; i < length; ptr[0]++, ptr[1]++, i++)
	{
		if (*ptr[1] == '\0')
		{
			retval = FALSE;
			break;
		}
		if (*ptr[0] != *ptr[1])
		{
			retval = FALSE;
			break;
		}
		if (*ptr[0] == '\0')
		{
			if (*ptr[1] == '\0')
				retval = TRUE;
			else
				retval = FALSE;
			break;
		}
	}
	
	g_free (_fst);
	g_free (_snd);
	return retval;
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
	
	/* It would be better to do a for
	 * counting how many strstr we get.
	 */
	for (ptr = C2_CHAR (or_string);;)
	{
		if (*ptr == '\0')
			break;
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
		if (*ptr == '\0')
			break;
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
	gint length;
	
	c2_return_val_if_fail (str, NULL, C2EDATA);

	length = strlen (str);
	
	if ((*str == open && *(str+length-1) == close) &&
		 length > 1)
		ptr = g_strndup (str+1, strlen (str)-2);
	else
		ptr = g_strdup (str);
	
	return ptr;
}

/**
 * c2_str_get_enclosed_text
 * @str:	String to process.
 * @enc1:	Opening character.
 * @enc2:	Closing character.
 * @args:	Number of values in @....
 * @...:	List of possible characters that
 * 			will force to finish the process.
 *
 * This function will get the string between
 * @enc1 and @enc2 in @str, which can not
 * contain any of the characters in @....
 *
 * Return Value:
 * The text or %NULL.
 **/
gchar *
c2_str_get_enclosed_text (const gchar *str, gchar enc1, gchar enc2, guint args, ...)
{
	const gchar *ptr, *start, *end;
	gchar forcers[args];
	va_list vargs;
	gint i, level;

	/* Load the value of ... to forcers */
	va_start (vargs, args);
	for (i = 0; i < args; i++)
		forcers[i] = va_arg (vargs, gint);
	va_end (vargs);
	
	/* Process */
	for (level = 0, start = end = NULL, ptr = str; ptr; ptr++)
	{
		if (*ptr == enc1)
		{
			if (!start)
				start = ptr;
			else
				level++;
		} else if (*ptr == enc2)
		{
			if (start)
			{
				if (level > 0)
					level--;
				else
				{
					end = ptr;
					break;
				}
			}
		}

		for (i = 0; i < args; i++)
			if (forcers[i] == *ptr)
				return NULL;
	}

	/* Move pointer away from enc1 and enc2 */
	start++;
	
	return g_strndup (start, end-start);
}

/**
 * c2_str_get_enclosed_text_backward
 * @str:	String to process.
 * @enc1:	Opening character.
 * @enc2:	Closing character.
 * @args:	Number of values in @....
 * @...:	List of possible characters that
 * 			will force to finish the process.
 *
 * This function will get the string between
 * @enc1 and @enc2 in @str, which can not
 * contain any of the characters in @....
 * It will search the string from the end to the start.
 *
 * Return Value:
 * The text or %NULL.
 **/
gchar *
c2_str_get_enclosed_text_backward (const gchar *str, gchar enc1, gchar enc2, guint args, ...)
{
	const gchar *ptr, *start, *end;
	gchar forcers[args];
	va_list vargs;
	gint i, level, length;

	/* Load the value of ... to forcers */
	va_start (vargs, args);
	for (i = 0; i < args; i++)
		forcers[i] = va_arg (vargs, gint);
	va_end (vargs);
	
	/* Process */
	length = strlen (str);
	for (level = 0, start = end = NULL, ptr = (str+length); ptr; ptr--)
	{
		if (*ptr == enc2)
		{
			if (!end)
				end = ptr;
			else
				level++;
		} else if (*ptr == enc1)
		{
			if (end)
			{
				if (level > 0)
					level--;
				else
				{
					start = ptr;
					break;
				}
			}
		}

		for (i = 0; i < args; i++)
			if (forcers[i] == *ptr)
				return NULL;
	}

	/* Move pointer away from enc1 and enc2 */
	start++;
	
	return g_strndup (start, end-start);
}

/**
 * c2_str_count_lines
 * @str: A pointer to a string
 * 
 * Countes the number of lines in the string.
 * 
 * Return Value:
 * Number of lines in the string, or 0 if the string is NULL
 **/
gint
c2_str_count_lines (const gchar *str)
{
	gchar *ptr;
	gint counter = 1;
	
	if(!str)
		return 0;
	
	for (ptr = C2_CHAR (str); *ptr; ptr++)
		if (*ptr == '\n' && *(ptr+1))
			counter++;
	
	return counter;
}

/**
 * c2_str_get_line
 * @str: A pointer to a string object where the next line should be searched.
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
			if (len == 0)
				return NULL;
			break;
		}
		len++;
		if (*strptr == '\n')
			break;
	}
	
	string = g_new0 (gchar, len+1);
	
	for (i=0, ptr = string, strptr = C2_CHAR (str); i <= len; i++, strptr++)
	{
		if (*strptr == '\0')
			break;
		*(ptr++) = *strptr;
		if (*strptr == '\n')
			break;
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
	
	if (str == NULL)
		return NULL;
	if (word_n == 0)
		record = TRUE;
	rtn = g_string_new (NULL);
	
	for (c = C2_CHAR (str); *c != '\0'; c++)
	{
		if (*c == '"')
		{
			if (quotes)
				quotes = FALSE;
			else
				quotes = TRUE;
			continue;
		}
			
		if (record && *c == '"')
			break;
		if (record && *c == ch && !quotes)
			break; /* If the word ends, break */
		if (record)
			g_string_append_c (rtn, *c); /* Record this character */
		if (*c == ch && !quotes)
			Ai++;
		if (*c == ch && Ai == word_n && !quotes)
			record = TRUE;
	}
	
	c = rtn->str;
	g_string_free (rtn, FALSE);
	
	return c;
}

#define BUFFER 4096
#define APPEND(c) \
	buffer[buf_pos++] = c; \
	wcntl++;
#define RESET \
	/* Reset values */ \
	wrap = FALSE; \
	wcntl = 0; \
	last_wrappable_position = NULL;
#define FLUSH \
	if (buf_pos == BUFFER-1) \
	{ \
		/* Flush everything */ \
		if (!gstring) \
			gstring = g_string_new (buffer); \
		else \
			g_string_append (gstring, buffer); \
		buffer[0] = 0; \
		buf_pos = 0; \
	}
gchar *
c2_str_wrap (const gchar *str, guint8 position)
{
	const gchar *ptr;
	GString *gstring = NULL;
	gchar buffer[BUFFER];
	gchar *retval;
	guint wcntl, buf_pos;
	gboolean wrap = FALSE;
	gchar *last_wrappable_position = NULL;

	buffer[0] = 0;
	
	for (ptr = str, wcntl = 0, buf_pos = 0; *ptr != '\0'; ptr++)
	{
		buffer[buf_pos+1] = 0;

		/* Set the information if we reach the wrap point */
		if (!wrap && wcntl > position)
		{
			wrap = TRUE;

			/* If there are pending wrapping positions
			 * we can use them with this code.
			 */
			if (last_wrappable_position)
			{
				*last_wrappable_position = '\n';
				RESET;
			}
		}
		
		/* New lines will reset our wrapping code. */
		if (*ptr == '\n')
		{
			RESET;
		} else if (*ptr == ' ' ||
				   *ptr == '\t')
		{
			if (wrap)
			{
				/* Wrap */
				APPEND ('\n');
				RESET;
				FLUSH;
				wrap = FALSE;
			} else
			{
				/* Store position for future wraps */
				last_wrappable_position = buffer+buf_pos;
			}
		}
				
		APPEND (*ptr);
		FLUSH;
	}

	if (gstring)
	{
		retval = gstring->str;
		g_string_free (gstring, FALSE);
	} else
		retval = g_strdup (buffer);
	
	return retval;
}
#undef BUFFER
#undef APPEND
#undef RESET
#undef FLUSH

/**
 * c2_str_text_to_html
 * @str: String to process.
 * @proc_email: Wether string that looks like E-Mail should be
 *              converted to links.
 *
 * This function will convert the string @str into an
 * HTML string.
 * Note that this function will not put any other tag
 * that the string requires, i.e. it won't put any
 * <html> tag or <body> tag, all it does is stuff like
 * converting < into &lt; and that stuff.
 *
 * Return Value:
 * A new allocated HTML string.
 **/
#define BUFFER 80
#define APPEND(c) \
	{ \
		buffer[buf_pos++] = c; \
		buffer[buf_pos] = 0; \
		FLUSH; \
	}
#define APPEND_S(str) \
	{ \
		gchar *__ptr__; \
		for (__ptr__ = str; *__ptr__ != '\0'; __ptr__++) \
			APPEND (*__ptr__); \
	}
#define FLUSH \
	if (buf_pos >= BUFFER-1) \
	{ \
		/* Flush everything */ \
		if (!gstring) \
			gstring = g_string_new (buffer); \
		else \
			g_string_append (gstring, buffer); \
		bzero (buffer, BUFFER); \
		buf_pos = 0; \
	}
gchar *
c2_str_text_to_html (const gchar *str, gboolean proc_email)
{
	gchar buffer[BUFFER];
	const gchar *ptr;
	gchar *retval;
	gint buf_pos;
	GString *gstring = NULL;
	gboolean new_word = TRUE;
	
	if (!str)
		return NULL;

	bzero (buffer, BUFFER);
	
	for (ptr = str, buf_pos = 0; *ptr != '\0'; ptr++)
	{
		if (new_word && proc_email)
		{
			gchar *buf = c2_str_get_word (0, ptr, ' ');
			gint buflength;
			
			if (c2_str_is_email (buf))
			{
				gchar *link, *email;
				size_t length;

				email = c2_str_get_email (buf);
				link = g_strdup_printf ("<a href=\"mailto:%s\">%s</a>", email, buf);
				length = strlen (email);
				g_free (email);

				APPEND_S (link);
				g_free (link);

				ptr += length;
				buffer[buf_pos] = 0;
			} else if (*buf == '<' && *(buf+(buflength = strlen (buf))-1) == '>')
			{
				gchar *email = g_strndup (buf+1, buflength-2);
				gchar *link;
				gint length;

				link = g_strdup_printf ("<a href=\"mailto:%s\">&lt;%s&gt;</a>", email, email);
				length = strlen (buf);
				g_free (email);

				APPEND_S (link);
				g_free (link);

				ptr += length;
				buffer[buf_pos] = 0;
			}

			g_free (buf);
		}
		
		if (*ptr == '<')
		{
			APPEND_S ("&lt;");
			new_word = TRUE; /* I know, this is not a new word,
								but it is for what we want to see:
								an email address */
		} else if (*ptr == '>')
		{
			APPEND_S ("&gt;");
		} else if (*ptr == ' ')
		{
			new_word = TRUE;
			APPEND (' ');
		} else
			APPEND (*ptr);
	}

	if (gstring)
	{
		retval = g_strconcat (gstring->str, buffer, NULL);
		g_string_free (gstring, FALSE);
	} else
		retval = g_strdup (buffer);
	
	return retval;
}
#undef BUFFER
#undef APPEND
#undef FLUSH

/**
 * c2_str_get_striped_subject
 * @subject: Subject to process
 *
 * This function will strip all prefixes
 * and sufixes that a subject might get.
 * The actual implementation supports:
 * Prefixes:
 * 	o "Re:"
 * 	o "Re[xxxx]:"
 * 	o "Fw:"
 * 	o "Fw[xxxx]:"
 *
 * Sufixes:
 *
 * Return Value:
 * A freeble string with the striped
 * subject.
 **/
gchar *
c2_str_get_striped_subject (const gchar *subject)
{
	const gchar *ptr;
	gint start = 0, end = -1;

	for (ptr = subject; *ptr != '\0'; ptr++)
	{
		if (c2_strneq (ptr, "Re", 2))
		{
			ptr += 2;
			
			if (*ptr == '[')
			{
				for (; *ptr != '\0'; ptr++)
					if (*ptr == ']')
						break;
				
				if (*ptr == '\0' || *(ptr+1) != ':')
				{
					/* This didn't work.. that's it, finish searching for the prefixes */
					goto re_didnt_work;
				}

				/* This worked, go to the next space */
				for (ptr += 2; *ptr != '\0'; ptr++)
					if (*ptr != ' ')
						break;

				start = ptr - subject;
			} else if (*ptr == ':')
			{
				/* This worked, go to the next space */
				for (ptr++; *ptr != '\0'; ptr++)
					if (*ptr != ' ')
						break;
				
				start = ptr - subject;
			} else
			{
re_didnt_work:
				ptr -= 2;
				break;
			}
		} else if (c2_strneq (ptr, "Fwd", 3))
		{
			ptr += 3;
			
			if (*ptr == '[')
			{
				for (; *ptr != '\0'; ptr++)
					if (*ptr == ']')
						break;
				
				if (*ptr == '\0' || *(ptr+1) != ':')
				{
					/* This didn't work.. that's it, finish searching for the prefixes */
					goto fwd_didnt_work;
				}

				/* This worked, go to the next space */
				for (ptr += 2; *ptr != '\0'; ptr++)
					if (*ptr != ' ')
						break;

				start = ptr - subject;
			} else if (*ptr == ':')
			{
				/* This worked, go to the next space */
				for (ptr++; *ptr != '\0'; ptr++)
					if (*ptr != ' ')
						break;
				
				start = ptr - subject;
			} else
			{
fwd_didnt_work:
				ptr -= 3;
				break;
			}
		}
	}

	if (end >= 0)
		return g_strndup (subject+start, end);

	return g_strdup (subject+start);
}

/**
 * c2_str_get_emails
 * @string: String to process.
 *
 * This function will look for several
 * email addresses in the formats supported
 * by c2_str_get_email, using ',[ ]' or ';[ ]'
 * as separators.
 * Read the comments for c2_str_get_email for more.
 *
 * Return Value:
 * A GList containing the different emails or %NULL.
 **/
GList *
c2_str_get_emails (const gchar *string)
{
	GList *emails = NULL;
	gchar *ptr;
	gchar *copy, *buf;

	copy = g_strdup (string);
	
	for (ptr = copy; (ptr = strtok (ptr, ",;")); ptr = NULL)
	{
		buf = c2_str_get_email (ptr);
		emails = g_list_append (emails, buf);
	}

	g_free (copy);

	return emails;
}

/**
 * c2_str_get_email
 * @string: String to process.
 *
 * This function searches for a
 * what might be an email address.
 * Note that this function will not
 * evaluate if the result is or is not
 * an e-mail address, it will just determine
 * it for common practices when writing an
 * email address.
 *
 * This function supports the following formats:
 * 1. [["]Name["]] <email> [Something]
 * 2. email
 * 
 * Return Value:
 * The email address found or %NULL.
 **/
gchar *
c2_str_get_email (const gchar *string)
{
	const gchar *ptr, *start = NULL, *end = NULL;
	gchar *email = NULL;
	gint length;

	if (!(length = strlen (string)))
		return NULL;

	/* Look for format [["]Name["]] <email> [Something] */
	for (ptr = string+length; ptr != string; ptr--)
	{
		if (!end)
		{
			if (*ptr == '>')
				end = ptr;
		} else
		{
			if (*ptr == '<')
				start = ptr;
		}
	}

	if (start && end)
	{
		email = g_strndup (start+1, end-start-1);
		goto finish;
	} else
	{
		/* If we don't have the complete information
		 * reset the values.
		 */
		start = NULL;
		end = NULL;
	}

	/* Look for the format email */
	for (ptr = string; *ptr != '\0'; ptr++)
		if (*ptr != ' ')
			break;
	if (ptr)
		email = g_strdup (ptr);
	goto finish;

finish:
	return email;
}

/**
 * c2_str_get_sender
 * @string: String to process
 *
 * This function will look for the sender
 * from a From: header.
 *
 * Formats:
 * "$name" <$email> = $name
 * $name <$email> = $name
 * $str = $str
 * <$email> = email
 * 
 * Return Value:
 * The sender or %NULL.
 **/
gchar *
c2_str_get_sender (const gchar *string)
{
	const gchar *ptr;
	
	if (!string || !strlen (string))
		return NULL;

	/* If it starts with " we will return all the quoted string */
	if (*string == '"')
	{
		for (ptr = string+1; *ptr != '\0' && *ptr != '"'; ptr++)
			;
		if (ptr)
			return g_strndup (string+1, ptr-string-1);
	}

	/* If it starts with < we will retorn the quoted string */
	if (*string == '<')
	{
		for (ptr = string+1; *ptr != '\0' && *ptr != '>'; ptr++)
			;
		if (ptr)
			return g_strndup (string+1, ptr-string-1);
	}

	/* If it contains a < we will return whatever there is before of it
	 * (If there's something). */
	if ((ptr = strchr (string, '<')))
	{
		const gchar *ptr2;

		for (--ptr; ptr != string; ptr--)
			if (*ptr != ' ' && *ptr != '\t')
				break;

		for (ptr2 = string; ptr2 <= ptr; ptr2++)
			if (*ptr2 != ' ' && *ptr2 != '\t')
				return g_strndup (ptr2, ptr-ptr2+1);
	}

	return g_strdup (string);
}

/**
 * c2_str_is_email
 * @email: String to evaluate.
 *
 * This function evaluates if @email is a valid
 * inet email address.
 *
 * Return Value:
 * %TRUE if it is an email or %FALSE.
 **/
gboolean
c2_str_is_email (const gchar *email)
{
	const gchar *ptr;
	gint length;
#if 0
	gboolean dot = TRUE;
#endif
	
	/* Get the length of the mail */
	length = strlen (email);
	
	/* An empty string is not a mail address */
	if (!length)
		return FALSE;

	/* A mail address must have an @ symbol */
	for (ptr = email; *ptr != '\0'; ptr++)
		if (*ptr == '@')
			goto passed1;
	
	return FALSE;
	
passed1:
	/* We don't want to evaluate the @ symbol again */
	ptr++;
	for (; *ptr != '\0'; ptr++)
	{
		/* A mail address might have a . */
		if (*ptr == '.')
		{
#if 0
			dot = TRUE;
#endif
			continue;
		}

		/* Check if characters are valid */
		if ((*ptr != '-' && *ptr < '0') || /* It can't have anything below '0' but the '-' symbol */
			(*ptr > 'z') || /* It can't have anything over 'z' */
			(*ptr > '9' && *ptr < 'A') || /* It can't be anything between '9' and 'A' */
			(*ptr > 'Z' && *ptr < 'a')) /* It can't be anything between 'Z' and 'a' */
			return FALSE;
	}

	/* Evaluate the last character */
	ptr = email+length-1;
	if ((*ptr < '0') || /* It can't have anything below '0' */
		(*ptr > 'z') || /* It can't be anything over 'z' */
		(*ptr > '9' && *ptr < 'A') || /* It can't be anything between '9' and 'A' */
		(*ptr > 'Z' && *ptr < 'a')) /* It can't be anything between 'Z' and 'a' */
		return FALSE;
	else
		return TRUE; /* Was return dot */

	return FALSE;
}

/**
 * c2_str_are_emails
 * @list: List of email addresses to check.
 *
 * This function checks if all emails in @list
 * are valid email address.
 * Read c2_str_is_email for more.
 *
 * Return Value:
 * %TRUE if all emails are valid or %FALSE.
 **/
gboolean
c2_str_are_emails (GList *list)
{
	GList *l;

	if (!list)
		return FALSE;

	for (l = list; l; l = g_list_next (l))
	{
		if (!c2_str_is_email ((gchar*) l->data))
			return FALSE;
	}
	
	return TRUE;
}

/**
 * c2_str_decode_iso_8859_1
 * @string: String to process
 *
 * This function will decode a string encoded
 * with ISO 8859-1 (Latin1).
 *
 * Return Value:
 * The decoded string or %NULL in case the process
 * failed or the string wasn't encoded with ISO 8859-1.
 **/
gchar *
c2_str_decode_iso_8859_1 (const gchar *string)
{
	gint o_length = strlen (string);
	gchar buffer[o_length];
	const gchar *ptr;
	gint i;
	gboolean changed = FALSE;
	
	c2_return_val_if_fail (string, NULL, C2EDATA);
	
	/* Clear the buffer */
	memset (buffer, 0, o_length*sizeof (gchar));
	
	/* Go through the words */
	for (ptr = string, i = 0; *ptr != '\0'; ptr++)
	{
		/* Check if this is encoded with ISO 8859-1 */
		if (c2_strnne (ptr, "=?iso-8859-1?Q?", 15))
		{
			/* Move to the next word */
			for (; *ptr != '\0' && *ptr != ' '; ptr++)
				buffer[i++] = *ptr;
			
			/* *ptr might be a ' ' or a '\0', either way,
			 * we rather want it in the decoded string.
			 */
			buffer[i++] = *ptr;
			
			continue;
		}
		
		changed = TRUE;
	
		/* Go in the loop. Start processing this word. */
		printf ("Processed word: '");
		for (ptr += 15; *ptr != '\0' && *ptr != ' '; ptr++)
		{
			if (*ptr == '=')
			{
				gint code;
			
				sscanf (ptr+1, "%02X", &code);
				sprintf (&buffer[i++], "%c", (gchar) code);
				ptr+=2;
			} else if (*ptr == '?' && *(ptr+1) == '=')
				ptr++;
			else
				buffer[i++] = *ptr;
		}
	}
	
	if (!changed)
		return NULL;

	return g_strdup (buffer);
}

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

typedef void (*C2Signal_NONE__INT_INT_INT)		(GtkObject *object, gint arg1, gint arg2, gint arg3,
													gpointer user_data);

void
c2_marshal_NONE__INT_INT_INT (GtkObject *object, GtkSignalFunc func, gpointer func_data, GtkArg * args)
{
	C2Signal_NONE__INT_INT_INT rfunc;
	rfunc = (C2Signal_NONE__INT_INT_INT) func;
	(*rfunc) (object, GTK_VALUE_INT (args[0]), GTK_VALUE_INT (args[1]), GTK_VALUE_INT (args[2]), func_data);
}



typedef gint (*C2Signal_INT__POINTER_POINTER_POINTER)	(GtkObject *object, gpointer arg1,
														 gpointer arg2,
														 gpointer arg3, gpointer user_data);

void
c2_marshal_INT__POINTER_POINTER_POINTER (GtkObject *object, GtkSignalFunc func, gpointer func_data, GtkArg * args)
{
	C2Signal_INT__POINTER_POINTER_POINTER rfunc;
	gint *return_val;
	return_val = GTK_RETLOC_INT (args[3]);
	rfunc = (C2Signal_INT__POINTER_POINTER_POINTER) func;
	*return_val = (*rfunc) (object, GTK_VALUE_POINTER (args[0]), GTK_VALUE_POINTER (args[1]),
							GTK_VALUE_POINTER (args[2]), func_data);
}
