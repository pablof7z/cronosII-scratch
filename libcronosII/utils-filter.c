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
/**
 * Maintainer(s) of this file:
 * 		* Pablo Fernández Navarro
 * Code of this file by:
 * 		* Pablo Fernández Navarro
 */
#include <string.h>
#include <stdarg.h>
#include <fnmatch.h>

#include "error.h"
#include "utils-date.h"
#include "utils-filter.h"

#define unsign(x)	((x>=0)?(x):(x*(-1)))

static gboolean
apply_rule (C2Message *message, C2FilterRule *rule)
{
	gboolean sensitive = rule->sensitive;
	const gchar *field = rule->field, *match = rule->match;
	
	/* First figure out where we should search */
	if (c2_streq (field, C2_FILTER_FIELD_DATE))
	{ /* Date Search */
		const gchar *ptr;
		gchar *sdate, *fdate, *tdate;
		time_t date, matchdate[2];
		gint allowed_difference;

		sdate = c2_message_get_header_field (message, "Date:");
		if (!sdate)
			return FALSE;

		/* Calculate the Message Date */
		if ((date = c2_date_parse (sdate)) >= 0);
		else if ((date = c2_date_parse_fmt2 (sdate)) >= 0);
		else if ((date = c2_date_parse_fmt3 (sdate)) >= 0);
		else
		{
			g_free (sdate);
			return FALSE;
		}
		g_free (sdate);
		
		/* Find how the match date is make up */
		if ((ptr = strstr (match, "|")))
		{
			fdate = g_strndup (match, ptr-match);
			tdate = g_strdup (ptr+1);
		} else
		{
			fdate = g_strdup (match);
			tdate = NULL;
		}
		
		/* Calculate From Date */
		if ((matchdate[0] = c2_date_parse (fdate)) >= 0);
		else if ((matchdate[0] = c2_date_parse_fmt2 (fdate)) >= 0);
		else if ((matchdate[0] = c2_date_parse_fmt3 (fdate)) >= 0);
		else
		{
			g_free (fdate);
			g_free (tdate);
			return FALSE;
		}

		/* Calculate To Date */
		if (tdate)
		{
			if ((matchdate[1] = c2_date_parse (tdate)) >= 0);
			else if ((matchdate[1] = c2_date_parse_fmt2 (tdate)) >= 0);
			else if ((matchdate[1] = c2_date_parse_fmt3 (tdate)) >= 0);
			else
			{
				g_free (fdate);
				g_free (tdate);
				return FALSE;
			}
		} else
			matchdate[1] = -1;

		if (matchdate[1] >= 0 && matchdate[0] > matchdate[1])
			return FALSE;

		/* Calculate the allowed difference */
		allowed_difference = (matchdate[1] >= 0) ? matchdate[1]-matchdate[0] : 0;

		if (unsign (date-matchdate[0]) <= allowed_difference)
			return TRUE;
		else
			return FALSE;
	}
	
	else if (c2_streq (field, C2_FILTER_FIELD_HEADER))
	{ /* Header Search */
		gint retval;
		
		if ((retval = fnmatch (match, message->header, sensitive ? 0 : (1 << 4))))
			return FALSE;

		return TRUE;
	}
	
	else if (c2_streq (field, C2_FILTER_FIELD_BODY))
	{ /* Body Search */
		gint retval;
		
		if ((retval = fnmatch (match, message->body, sensitive ? 0 : (1 << 4))))
			return FALSE;

		return TRUE;
	}
	
	else if (c2_streq (field, C2_FILTER_FIELD_MESSAGE))
	{ /* Message Search */
		gint retval;
		
		if ((retval = fnmatch (match, message->header, sensitive ? 0 : (1 << 4))))
			if ((retval = fnmatch (match, message->body, sensitive ? 0 : (1 << 4))))
				return FALSE;

		return TRUE;
	}
	
	else
	{ /* Some field Search (hopelly) */
		gchar *fstr;
		gint retval;

		fstr = c2_message_get_header_field (message, field);

		if (!fstr)
			return FALSE;

		retval = fnmatch (match, fstr, sensitive ? 0 : (1 << 4));
		g_free (fstr);

		return !retval ? TRUE : FALSE;
	}

	return FALSE;
}

static gboolean
match_message_full (C2Message *message, GSList *list)
{
	GSList *l;
	gboolean retval = TRUE;
	
	for (l = list; l; l = g_slist_next (l))
	{
		C2FilterRule *rule = ((C2FilterRule*)l->data);

		retval = apply_rule (message, rule);
	}

	return retval;
}

/**
 * c2_filter_match_message_full
 * @message: Message where to work.
 * @func: Callback function (might be %NULL).
 * @...: List of Rules ended with a %NULL.
 * 
 * Return Value:
 * %TRUE if the string was found or %FALSE.
 **/
gboolean
c2_filter_match_message_full (C2Message *message, C2FilterCallback func, ...)
{
	va_list args;
	gboolean retval;
	GSList *list = NULL;
	
	c2_return_val_if_fail (C2_IS_MESSAGE (message), C2EDATA, FALSE);

	va_start (args, func);
	for (;;)
	{
		C2FilterRule *rule = va_arg (args, C2FilterRule*);
		if (!rule)
			break;

		list = g_slist_append (list, rule);
	}
	va_end (args);

	retval = match_message_fullv (message, func, list);
	g_slist_free (list);

	if (func)
		func (GTK_OBJECT (message), retval);
	
	return retval;
}

/**
 * c2_filter_match_mailbox_full
 * @mailbox: Mailbox where to work.
 * @func: Callback function (might be %NULL).
 * @...: List of rules to apply ending with a %NULL.
 *
 * Return Value:
 * The list of mails that apply to the rule(s) in Virtual DB.
 **/
C2VDb *
c2_filter_match_mailbox_full (C2Mailbox *mailbox, C2FilterCallback func, ...)
{
	va_list args;
	GSList *list = NULL;
	C2Db *head = NULL, *l;

	/* Create the list of rules */
	va_start (args, func);
	for (;;)
	{
		C2FilterRule *rule = va_arg (args, C2FilterRule*);
		if (!rule)
			break;

		list = g_slist_append (list, rule);
	}
	va_end (args);

	if (mailbox->db)
	{
		l = mailbox->db;
		
		do
		{
			C2Message *message;
			C2VDb *vdb;
			gboolean retval;
			
			if (c2_db_load_message (l) < 0)
				continue;

			message = l->message;

			gtk_object_ref (GTK_OBJECT (message));
			retval = match_message_full (message, list);
			gtk_object_unref (GTK_OBJECT (message));
			
			if (retval)
			{
				vdb = c2_vdb_new (l);
				c2_vdb_append (head, vdb);
			}
		} while (c2_db_lineal_next (l));
	}

	g_slist_free (list);

	return head;
}
