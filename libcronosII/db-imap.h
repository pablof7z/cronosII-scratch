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
 * 		* Bosko Blagojevic
 * Code of this file by:
 * 		* Bosko Blagojevic
 */
#ifndef __LIBCRONOSII_DB_IMAP_H__
#define __LIBCRONOSII_DB_IMAP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <glib.h>

#if defined (HAVE_CONFIG_H) && defined (BUILDING_C2)
#	include "db.h"
#	include "mailbox.h"
#else
#	include <cronosII.h>
#endif

gboolean
c2_db_imap_create_structure					(C2Mailbox *mailbox);

gboolean
c2_db_imap_update_structure					(C2Mailbox *mailbox);

gboolean
c2_db_imap_remove_structure					(C2Mailbox *mailbox);

void
c2_db_imap_compact							(C2Mailbox *mailbox, size_t *cybtes, size_t *tbytes);

gint
c2_db_imap_load								(C2Mailbox *mailbox);

gboolean
c2_db_imap_message_add						(C2Mailbox *mailbox, C2Db *db);

gint
c2_db_imap_message_remove					(C2Mailbox *mailbox, GList *list);
	
void
c2_db_imap_freeze (C2Mailbox *mailbox);

void
c2_db_imap_thaw (C2Mailbox *mailbox);

void
c2_db_imap_message_set_state				(C2Db *db, C2MessageState state);

void
c2_db_imap_message_set_mark					(C2Db *db, gboolean mark);

C2Message *
c2_db_imap_load_message						(C2Db *db);

#ifdef __cplusplus
}
#endif

#endif
