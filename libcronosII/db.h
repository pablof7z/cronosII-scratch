/*  Cronos II Mail Client
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
#ifndef __LIBCRONOSII_DB_H__
#define __LIBCRONOSII_DB_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <glib.h>
#include <gtk/gtk.h>
#include <time.h>

#define C2_TYPE_DB								(c2_db_get_type ())
#define C2_DB(obj)								(GTK_CHECK_CAST (obj, C2_TYPE_DB, C2Db))
#define C2_DB_CLASS(klass)						(GTK_CHECK_CLASS_CAST (klass, C2_TYPE_DB, C2DbClass))
#define C2_IS_DB(obj)							(GTK_CHECK_TYPE (obj, C2_TYPE_DB))
#define C2_IS_DB_CLASS(klass)					(GTK_CHECK_CLASS_TYPE (klass, C2_TYPE_DB))

typedef struct _C2Db C2Db;
typedef struct _C2DbClass C2DbClass;

#if defined (HAVE_CONFIG_H) && defined (BUILDING_C2)
#	include "mailbox.h"
#	include "message.h"
#else
#	include <cronosII.h>
#endif

struct _C2Db
{
	C2Message message;
	
	C2MessageState state;
	gint marked : 1;
	gchar *subject;
	gchar *from;
	gchar *account;
	time_t date;
	
	gint position; /* Within the mailbox list */
	gint mid;

	C2Mailbox *mailbox;
	struct _C2Db *previous;
	struct _C2Db *next;
};

struct _C2DbClass
{
	C2MessageClass parent_class;

	void (*mark_changed) (C2Db *db, gboolean marked);
	void (*state_changed) (C2Db *db, C2MessageState state);
};

GtkType
c2_db_get_type									(void);

C2Db *
c2_db_new										(C2Mailbox *mailbox);

gint
c2_db_messages									(const C2Db *db);

C2Db *
c2_db_message_add								(C2Db *db, const C2Message *message, gint row);

void
c2_db_message_set_state							(C2Db *db, gint row, C2MessageState state);

void
c2_db_message_swap_mark							(C2Db *db, gint row);

gint
c2_db_message_remove							(C2Db *db, gint row);

C2Message *
c2_db_message_get								(C2Db *db, gint row);

gint
c2_db_message_search_by_mid						(const C2Db *db, gint mid);

#ifdef __cplusplus
}
#endif

#endif
