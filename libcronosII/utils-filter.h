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
#ifndef __LIBCRONOSII_UTILS_FILTER_H__
#define __LIBCRONOSII_UTILS_FILTER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <glib.h>

#include <libcronosII/message.h>
#include <libcronosII/db.h>
#include <libcronosII/vdb.h>

/* This are just for compatibility */
#define C2_FILTER_FIELD_TO					"To:"
#define C2_FILTER_FIELD_FROM				"From:"
#define C2_FILTER_FIELD_SUBJECT				"Subject:"

/* This are really useful since they differ from common fields */
#define C2_FILTER_FIELD_DATE				"_Date_"
#define C2_FILTER_FIELD_HEADER				"_Header_"	/* Complete Header */
#define C2_FILTER_FIELD_BODY				"_Body_"	/* Complete Body */
#define C2_FILTER_FIELD_MESSAGE				"_Message_"	/* Complete Message */

typedef struct _C2FilterRule C2FilterRule;

struct _C2FilterRule
{
	gboolean sensitive : 1;
	const gchar *field, *match;
};

typedef void (*C2FilterCallback)			(GtkObject *object, gboolean matched);

C2FilterRule *
c2_filter_rule_new							(gboolean sensitive, const gchar *field, const gchar *match);

void
c2_filter_rule_destroy						(C2FilterRule *rule);

#define c2_filter_match_message(message, ...) \
											c2_filter_match_message_full (message, NULL, ##args)

gboolean
c2_filter_match_message_full				(C2Message *message, C2FilterCallback func, ...);

#define c2_filter_match_mailbox(mailbox, ...) \
											c2_filter_match_mailbox_full (mailbox, NULL, ##args)

C2VDb *
c2_filter_match_mailbox_full				(C2Mailbox *mailbox, C2FilterCallback func, ...);

#ifdef __cplusplus
}
#endif

#endif
