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
	gboolean return_val;
	
	temp = c2_mailbox_get_parent_id(mailbox->id);
	parent = c2_mailbox_get_by_id(mailbox->protocol.IMAP.imap->mailboxes, temp);
	
	return_val = 
		c2_imap_create_folder(mailbox->protocol.IMAP.imap, parent, mailbox->name);
	
	g_free(temp);
	return return_val;
}

gboolean
c2_db_imap_update_structure (C2Mailbox *mailbox)
{
}

gboolean
c2_db_imap_remove_structure (C2Mailbox *mailbox)
{
	return c2_imap_delete_folder(mailbox->protocol.IMAP.imap, mailbox);
}

gint
c2_db_imap_load (C2Mailbox *mailbox)
{
}

void
c2_db_imap_message_add (C2Mailbox *mailbox, C2Db *db)
{
}

void
c2_db_imap_message_remove (C2Mailbox *mailbox, C2Db *db, gint n)
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
}


#if 0
static gchar *
imap_resolve (C2Mailbox *mailbox)
{
	gchar *ip;

	if (c2_net_resolve (mailbox->protocol.imap.host, &ip))
	{
#ifdef USE_DEBUG
		g_print ("Unable to resolve hostname: %s\n", c2_error_get (c2_errno));
#endif
		return NULL;
	}

	return ip;
}

static gint
imap_connect (C2Mailbox *mailbox, const gchar *ip)
{
	if (c2_net_connect (ip, mailbox->protocol.imap.port, &mailbox->protocol.imap.sock) < 0)
	{
#ifdef USE_DEBUG
		g_print ("Unable to connect: %s\n",	c2_error_get (c2_errno));
#endif
		return -1;
	}

	return 0;
}

static gint
imap_login (C2Mailbox *mailbox)
{
L	if (c2_net_send (mailbox->protocol.imap.sock, "A%03d LOGIN \"%s\" \"%s\"\r\n",
						mailbox->protocol.imap.cmnd_n, mailbox->protocol.imap.user,
						mailbox->protocol.imap.pass) < 0)
		return -1;
}

/**
 * c2_db_imap_load
 * @mailbox: C2Mailbox descriptor.
 *
 * This function will load an IMAP mailbox.
 *
 * Return Value:
 * 0 on success or -1.
 **/
gint
c2_db_imap_load (C2Mailbox *mailbox)
{
	gchar *ip;
	
	c2_return_val_if_fail (mailbox, -1, C2EDATA);

	/* If the mailbox is already connected */
	if (mailbox->protocol.imap.sock >= 0)
		return 0;

	/* Resolve */
	if (!(ip = imap_resolve (mailbox)))
		return -1;

	/* Connect */
	if (imap_connect (mailbox, ip) < 0)
	{
		g_free (ip);
		return -1;
	}
	g_free (ip);

	/* Login */
	imap_login (mailbox);

	return 0;
}
#endif
