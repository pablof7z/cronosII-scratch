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
 * 		* Pablo Fernández Navarro
 * Code of this file by:
 * 		* Pablo Fernández Navarro
 */
#ifndef __LIBCRONOSII_DB_CRONOSII_H__
#define __LIBCRONOSII_DB_CRONOSII_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <glib.h>

#if defined (HAVE_CONFIG_H) && defined (BUILDING_C2)
#	include "db.h"
#	include "mailbox.h"
#	include "message.h"
#else
#	include <cronosII.h>
#endif

gboolean
c2_db_cronosII_create_structure				(C2Mailbox *mailbox);

gboolean
c2_db_cronosII_update_structure				(C2Mailbox *mailbox);

gboolean
c2_db_cronosII_remove_structure				(C2Mailbox *mailbox);

void
c2_db_cronosII_compact						(C2Mailbox *mailbox, size_t *cbytes, size_t *tbytes);

void
c2_db_cronosII_freeze						(C2Mailbox *mailbox);

void
c2_db_cronosII_thaw							(C2Mailbox *mailbox);

gint
c2_db_cronosII_load							(C2Mailbox *mailbox);

gboolean
c2_db_cronosII_message_add					(C2Mailbox *mailbox, C2Db *db);

gboolean
c2_db_cronosII_message_add_list				(C2Mailbox *mailbox, GList *list);

gint
c2_db_cronosII_message_remove				(C2Mailbox *mailbox, GList *list);

gint
c2_db_cronosII_move_list					(C2Mailbox *fmailbox, C2Mailbox *tmailbox, GList *list);

void
c2_db_cronosII_message_set_state			(C2Db *db, C2MessageState state);

void
c2_db_cronosII_message_set_mark				(C2Db *db, gboolean mark);

C2Message *
c2_db_cronosII_load_message					(C2Db *db);

#ifdef __cplusplus
}
#endif

#endif
