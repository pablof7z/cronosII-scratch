/*  Cronos II Mail Client /libcronosII/date-utils.c
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <fnmatch.h>

#include "date-utils.h"
#include "error.h"

static gchar *
fix_timezone (const gchar *tz);

static struct tz_t
{
	char tzname[5];
	unsigned char zhours;
	unsigned char zminutes;
	gboolean zoccident; /* west of UTC? */
} TimeZones[] =
{
/*    Name   Hours*/
	{ "aat",     1,     0,     1 }, /* Atlantic Africa Time */
	{ "adt",     4,     0,     0 }, /* Arabia DST */
	{ "ast",     3,     0,     0 }, /* Arabia */
	{ "bst",     1,     0,     0 }, /* British DST */
	{ "cat",     1,     0,     0 }, /* Central Africa */
	{ "cdt",     5,     0,     1 },
	{ "cest",    2,     0,     0 }, /* Central Europe DST */
	{ "cet",     1,     0,     0 }, /* Central Europe */
	{ "cst",     6,     0,     1 },
	{ "eat",     3,     0,     0 }, /* East Africa */
	{ "edt",     4,     0,     1 },
	{ "eest",    3,     0,     0 }, /* Eastern Europe DST */
	{ "eet",     2,     0,     0 }, /* Eastern Europe */
	{ "egst",    0,     0,     0 }, /* Eastern Greenland DST */
	{ "egt",     1,     0,     1 }, /* Eastern Greenland */
	{ "est",     5,     0,     1 },
	{ "gmt",     0,     0,     0 },
	{ "gst",     4,     0,     0 }, /* Persian Gulf */
	{ "hkt",     8,     0,     0 }, /* Hong Kong */
	{ "ict",     7,     0,     0 }, /* Indochina */
	{ "idt",     3,     0,     0 }, /* Israel DST */
	{ "ist",     2,     0,     0 }, /* Israel */
	{ "jst",     9,     0,     0 }, /* Japan */
	{ "kst",     9,     0,     0 }, /* Korea */
	{ "mdt",     6,     0,     1 },
	{ "met",     1,     0,     0 }, /* this is now officially CET */
	{ "msd",     4,     0,     0 }, /* Moscow DST */
	{ "msk",     3,     0,     0 }, /* Moscow */
	{ "mst",     7,     0,     1 },
	{ "nzdt",   13,     0,     0 }, /* New Zealand DST */
	{ "nzst",   12,     0,     0 }, /* New Zealand */
	{ "pdt",     7,     0,     1 },
	{ "pst",     8,     0,     1 },
	{ "sat",     2,     0,     0 }, /* South Africa */
	{ "smt",     4,     0,     0 }, /* Seychelles */
	{ "sst",    11,     0,     1 }, /* Samoa */
	{ "utc",     0,     0,     0 },
	{ "wat",     0,     0,     0 }, /* West Africa */
	{ "west",    1,     0,     0 }, /* Western Europe DST */
	{ "wet",     0,     0,     0 }, /* Western Europe */
	{ "wgst",    2,     0,     1 }, /* Western Greenland DST */
	{ "wgt",     3,     0,     1 }, /* Western Greenland */
	{ "wst",     8,     0,     0 }, /* Western Australia */
};

/**
 * c2_date_parse
 * @strtime: The time in format "[Tue, ] [0]2 Feb 1984 08:45:00 -0300"
 *
 * Will parse a date into a Unix time.
 *
 * Return Value:
 * The Unix time or -1 in case it was unable to understand the format.
 **/
time_t
c2_date_parse (const gchar *strtime)
{
	gint i, tmp;
	gchar *cpy = g_strdup (strtime);
	gchar *ptr;
	gchar *tz;
	struct tm tm;
	gint tz_hour = 0, tz_min = 0, tz_offset = 0;
	gboolean tz_occident = 0;
	
	c2_return_val_if_fail (strtime, -1, C2EDATA);
	
	/* Go from the ',' through the white spaces */
	if ((ptr = strchr (cpy, ',')))
		ptr++;
	else
		ptr = cpy;
	
	for (; *ptr == ' ' && *ptr != '\0'; ptr++);
	if (!ptr)
	{
		g_free (cpy);
		return -1;
	}
	
	/* Do the actual parsing */
	for (i = 0; (ptr = strtok (ptr, " \t")) != NULL; i++)
	{
		switch (i)
		{
			case 0:
				/* [Weekday, ] day_of_month month year hour:minute:second [Timezone]
				 *             ^^^^^^^^^^^^
				 */
				if (!isdigit (*ptr))
				{
					g_free (cpy);
					return -1;
				}
				tm.tm_mday = atoi (ptr);
				if (tm.tm_mday > 31)
				{
					g_free (cpy);
					return -1;
				}
				break;
			case 1:
				/* [Weekday, ] day_of_month month year hour:minute:second [Timezone]
				 *                          ^^^^^
				 */
				if ((tmp = c2_date_get_month (ptr)) < 0)
				{
					g_free (cpy);
					return -1;
				}
				tm.tm_mon = tmp;
				break;
			case 2:
				/* [Weekday, ] day_of_month month year hour:minute:second [Timezone]
				 *                                ^^^^
				 */
				tm.tm_year = atoi (ptr);
				if (tm.tm_year >= 1900)
					tm.tm_year -= 1900;
				else if(tm.tm_year <= 70)
					tm.tm_year += 100;
				break;
			case 3:
				/* [Weekday, ] day_of_month month year hour:minute:second [Timezone]
				 *                                     ^^^^^^^^^^^^^^^^^^
				 */
				if (sscanf (ptr, "%d:%d:%d", &tm.tm_hour, &tm.tm_min, &tm.tm_sec) == 3)
					;
				else if (sscanf (ptr, "%d:%d", &tm.tm_hour, &tm.tm_min) == 2)
					tm.tm_sec = 0;
				else
				{
					g_free (cpy);
					return -1;
				}
				break;
			case 4:
				/* [Weekday, ] day_of_month month year hour:minute:second [Timezone]
				 *                                                        ^^^^^^^^^^
				 */
				/* Fix timezone. */
				tz = fix_timezone (ptr);
				
				if (*tz == '+' || *tz == '-')
				{
					if (tz[1] && tz[2] && tz[3] && tz[4] &&
							isdigit ((unsigned char) tz[1]) && isdigit ((unsigned char) tz[2]) &&
							isdigit ((unsigned char) tz[3]) && isdigit ((unsigned char) tz[4]))
					{
						tz_hour = (tz[1] - '0') * 10 + (tz[2] - '0');
						tz_min  = (tz[3] - '0') * 10 + (tz[4] - '0');
						
						if (tz[0] == '-')
							tz_occident = 1;
						
					}
				} else
				{
					struct tz_t *ptz = NULL;
					
					ptz = bsearch (tz, TimeZones, sizeof TimeZones/sizeof (struct tz_t),
							sizeof (struct tz_t),
							(int (*)(const void *, const void *)) strcasecmp);
					
					if (ptz)
					{
						tz_hour = ptz->zhours;
						tz_min = ptz->zminutes;
						tz_occident = ptz->zoccident;
					} else
					{
						/* Assume GMT */
						tz_hour = 0;
						tz_min = 0;
						tz_occident = 0;
					}
					
					if (!strcasecmp (ptr, "MET"))
					{
						if ((ptr = strtok (NULL, " \t")))
						{
							if (!strcasecmp (ptr, "DST"))
								tz_hour++;
						}
					}
				}
				tz_offset = tz_hour * 3600 + tz_min * 60;
				if (!tz_occident)
					tz_offset *= -1;
				break;
			default:
				break;
		}
		ptr = NULL;
	}

	if (i < 4)
	{
		g_free (cpy);
		return -1;
	}

	g_free (cpy);
	return mktime (&tm)+tz_offset;
}

/**
 * c2_date_parse_fmt2
 * strtime: A time which contains, somewhere, the string "dd/mm/yyyy" or "mm/dd/yyyy".
 *
 * Will try to convert the date into a Unix time.
 *
 * Return Value:
 * The Unix time or -1 if it couldn't get it.
 **/
time_t
c2_date_parse_fmt2 (const gchar *strtime)
{
	struct tm tm = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	const gchar *ptr;
	gboolean do_date = TRUE, do_time = TRUE;
	
	c2_return_val_if_fail (strtime, -1, C2EDATA);

	if (fnmatch ("*??/??/????*", strtime, 0))
		return -1;

	/* The date is in the string, locate where */
	for (ptr = strtime; *ptr != '\0'; ptr++)
	{
		if (!isdigit (*ptr))
			continue;
		if (do_date)
		{
			if (sscanf (ptr, "%d/%d/%d", &tm.tm_mday, &tm.tm_mon, &tm.tm_year) != 3)
			{
				tm.tm_mday = 0;
				tm.tm_mon = 0;
				tm.tm_year = 0;
			} else
			{
				tm.tm_year -= 1900;
				tm.tm_mon--;
				do_date = FALSE;
			}
		}

		if (do_time)
		{
			if (sscanf (ptr, "%d:%d:%d", &tm.tm_hour, &tm.tm_min, &tm.tm_sec) < 2)
			{
				tm.tm_hour = 0;
				tm.tm_min = 0;
				tm.tm_sec = 0;
			} else
			{
				do_time = FALSE;
			}
		}
	}
	
	return mktime (&tm);
}

const gchar *Months[] = {
	"Jan", "Feb",
	"Mar", "Apr",
	"May", "Jun",
	"Jul", "Aug",
	"Sep", "Oct",
	"Nov", "Dec"
};

gint
c2_date_get_month (const gchar *strmnt)
{
	gint i;
	
	for (i = 0; i < 12; i++)
		if (strncasecmp (strmnt, Months[i], 3) == 0)
			return i;
  return -1;
}

/**
 * tz: A timezone string.
 *
 * This function will fix the @tz timezone
 * in order to be correct.
 *
 * Return Value:
 * A newly allocated mem chunk with the proper
 * timezone in it.
 **/
static gchar *
fix_timezone (const gchar *tz)
{
	const gchar *ptr;
	
	if (*tz != '(')
		return g_strdup (tz);

	for (tz++; *tz == ' '; tz++);
	if ((ptr = strpbrk (tz, " )")) == NULL)
		return g_strdup (tz);
	
	return g_strndup (tz, ptr - tz);
}
