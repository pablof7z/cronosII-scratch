/*  Cronos II - The GNOME mail client
 *  Copyright (C) 2000-2001 Pablo Fernández López
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
 * 		* Pablo Fernández López
 * Code of this file by:
 * 		* Pablo Fernández López
 */
#ifndef __LIBCRONOSII_DB_H__
#define __LIBCRONOSII_DB_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef BUILDING_C2
#	include <config.h>
#else
#	include <cronosII.h>
#endif

#include <glib.h>
#include <gtk/gtk.h>
#include <time.h>

#define C2_TYPE_DB								(c2_db_get_type ())
#ifdef USE_DEBUG
#	define C2_DB(obj)							(GTK_CHECK_CAST (obj, C2_TYPE_DB, C2Db))
#	define C2_DB_CLASS(klass)					(GTK_CHECK_CLASS_CAST (klass, C2_TYPE_DB, C2DbClass))
#else
#	define C2_DB(obj)							((C2Db*)obj)
#	define C2_DB_CLASS(klass)					((C2DbClass*)klass)
#endif
		
#define C2_IS_DB(obj)							(GTK_CHECK_TYPE (obj, C2_TYPE_DB))
#define C2_IS_DB_CLASS(klass)					(GTK_CHECK_CLASS_TYPE (klass, C2_TYPE_DB))

typedef struct _C2Db C2Db;
typedef struct _C2DbClass C2DbClass;

#if defined (HAVE_CONFIG_H) && defined (BUILDING_C2)
#	include "db-cronosII.h"
#	include "db-imap.h"
#	include "db-spool.h"
#	include "mailbox.h"
#	include "message.h"
#else
#	include <cronosII.h>
#endif

#define c2_db_is_first(db)					((db)&&(db->position<=db->prev->position))
#define c2_db_is_last(db)					((db)&&(db->position>=db->next->position))
#define c2_db_lineal_next(db)				(c2_db_is_last (db)?0:(db=db->next))
#define c2_db_is_load(mailbox)				((mailbox)&&(mailbox->db_is_loaded))

struct _C2Db
{
	GtkObject object;

	C2Message *message;
	C2MessageState state;

	gchar *subject;
	gchar *from;
	gchar *account;
	time_t date;

	gint mark : 1;

	gint position;
	gint mid;

	C2Mailbox *mailbox;

	C2Db *prev;
	C2Db *next;
};

struct _C2DbClass
{
	GtkObjectClass parent_class;
};

/*********************
 * [Object Handling] *
 *********************/
GtkType
c2_db_get_type								(void);

C2Db *
c2_db_new									(C2Mailbox *mailbox, gboolean mark, gchar *subject,
											 gchar *from, gchar *account, time_t date,
											 gint mid, gint position);

gint
c2_db_length								(C2Mailbox *mailbox);

gint
c2_db_length_type							(C2Mailbox *mailbox, gint state);

C2Db *
c2_db_get_node								(C2Mailbox *mailbox, gint n);

C2Db *
c2_db_get_node_by_mid						(C2Mailbox *mailbox, gint mid);

/************************
 * [Structure Handling] *
 ************************/
gboolean
c2_db_create_structure						(C2Mailbox *mailbox);

gboolean
c2_db_update_structure						(C2Mailbox *mailbox);

gint
c2_db_remove_structure						(C2Mailbox *mailbox);

/************************
 * [DataBase Iteration] *
 ************************/
void
c2_db_freeze								(C2Mailbox *mailbox);

void
c2_db_thaw									(C2Mailbox *mailbox);

gint
c2_db_load									(C2Mailbox *mailbox);

gboolean
c2_db_message_add							(C2Mailbox *mailbox, C2Message *message);

gboolean
c2_db_message_add_list						(C2Mailbox *mailbox, GList *list);

gboolean
c2_db_message_remove						(C2Mailbox *mailbox, C2Db *db);

gboolean
c2_db_message_remove_by_mid					(C2Mailbox *mailbox, gint mid);

gboolean
c2_db_message_remove_list					(C2Mailbox *mailbox, GList *list);

void
c2_db_message_set_state						(C2Db *db, C2MessageState state);

void
c2_db_message_set_mark						(C2Db *db, gboolean marked);


/***********************
 * [DataBase Querying] *
 ***********************/
gint
c2_db_load_message							(C2Db *db);

void
c2_db_unload_message						(C2Db *db);

/***************************
 * [Miscelaneus Functions] *
 ***************************/
void
c2_db_archive								(C2Mailbox *mailbox);

C2Message *
c2_db_message_get_from_file					(const gchar *path);

#ifdef __cplusplus
}
#endif

#endif
