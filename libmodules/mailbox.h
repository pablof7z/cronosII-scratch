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
#ifndef __LIBMODULES_MAILBOX_H__
#define __LIBMODULES_MAILBOX_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <glib.h>

#ifdef HAVE_CONFIG_H
#	include "db.h"
#else
#	include <cronosII.h>
#endif

typedef struct _C2Mailbox
{
	gchar *name;
	guint id;
	guint parent_id;
	
	C2DB *db;
	
	mid_t last_mid;
	guint new_messages;

	gint last_row;

	struct _C2Mailbox *next;
	struct _C2Mailbox *child;
} C2Mailbox;

C2Mailbox *
c2_mailbox_new								(C2Mailbox *head, const gchar *name, gint parent_id);

C2Mailbox *
c2_mailbox_parse							(const gchar *info);

C2Mailbox *
c2_mailbox_append							(C2Mailbox *head, C2Mailbox *mailbox);

gint
c2_mailboxes_next_id						(C2Mailbox *head);

C2Mailbox *
c2_mailbox_search_id						(C2Mailbox *head, gint id);

C2Mailbox *
c2_mailbox_search_name						(C2Mailbox *head, const gchar *name);

gint
c2_mailbox_length							(const C2Mailbox *mbox);

void
c2_mailbox_free								(C2Mailbox *mbox);

mid_t
c2_mailbox_next_mid							(C2Mailbox *mbox);

#ifdef __cplusplus
}
#endif

#endif
