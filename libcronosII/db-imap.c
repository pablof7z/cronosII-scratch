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
#include "db.h"
#include "db-imap.h"
#include "error.h"
#include "mailbox.h"
#include "utils.h"
#include "utils-net.h"

gboolean
c2_db_imap_create_structure (C2Mailbox *mailbox)
{
	gchar *temp;
	C2Mailbox *parent;
	C2IMAP *imap = mailbox->protocol.IMAP.imap;
	gboolean return_val;
	
	temp = c2_mailbox_get_parent_id(mailbox->id);
	parent = c2_mailbox_get_by_id(imap->mailboxes, temp);
	
	c2_mutex_lock(&imap->lock);
	return_val = 
		c2_imap_create_mailbox(imap, parent, mailbox->name);
	c2_mutex_unlock(&imap->lock);
	
	g_free(temp);
	return return_val;
}

gboolean
c2_db_imap_update_structure (C2Mailbox *mailbox)
{
	return TRUE;
}

gboolean
c2_db_imap_remove_structure (C2Mailbox *mailbox)
{
	C2IMAP *imap = mailbox->protocol.IMAP.imap;
	gboolean retval;
	
	c2_mutex_lock(&imap->lock);
	retval = c2_imap_delete_mailbox(imap, mailbox);
	c2_mutex_unlock(&imap->lock);
	
	return retval;
}

/* NOTE: coded to either work on a locked 
 * IMAP object or an unlocked one */
gint
c2_db_imap_load (C2Mailbox *mailbox)
{
	C2IMAP *imap = mailbox->protocol.IMAP.imap;
	gint retval = 0, trylock;
	
	trylock = c2_mutex_trylock(&imap->lock);
	if(!mailbox->protocol.IMAP.noselect)
		retval = c2_imap_load_mailbox(mailbox->protocol.IMAP.imap, mailbox);
	if(trylock == 0)
		c2_mutex_unlock(&imap->lock);
	
	return retval;
}

gboolean
c2_db_imap_message_add (C2Mailbox *mailbox, C2Db *db)
{
	C2IMAP *imap = mailbox->protocol.IMAP.imap;
	gint retval;
	
	c2_mutex_lock(&imap->lock);
	retval = c2_imap_message_add(imap, mailbox, db);
	c2_mutex_unlock(&imap->lock);
	
	if(retval < 0)
		return FALSE;
	else
		return TRUE;
}

gint
c2_db_imap_message_remove (C2Mailbox *mailbox, GList *list)
{
	C2IMAP *imap = mailbox->protocol.IMAP.imap;
	gint retval;
	
	c2_mutex_lock(&imap->lock);
	retval = c2_imap_message_remove(imap, list);
	c2_mutex_unlock(&imap->lock);
	
	return retval;
}

void
c2_db_imap_freeze (C2Mailbox *mailbox)
{
}

void
c2_db_imap_thaw (C2Mailbox *mailbox)
{
}

void
c2_db_imap_message_set_state (C2Db *db, C2MessageState state)
{
}

void
c2_db_imap_message_set_mark (C2Db *db, gboolean mark)
{
}

C2Message *
c2_db_imap_load_message (C2Db *db)
{
	C2IMAP *imap = C2_MAILBOX(db->mailbox)->protocol.IMAP.imap;
	C2Message *message;
	
	c2_mutex_lock(&imap->lock);
	message = c2_imap_load_message(imap, db);
	c2_mutex_unlock(&imap->lock);
	
	return message;
}
