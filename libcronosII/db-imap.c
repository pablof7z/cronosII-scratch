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
#include "db.h"
#include "db-imap.h"
#include "mailbox.h"
#include "utils.h"
#include "utils-net.h"

/**
 * c2_db_imap_load
 * @mailbox: C2Mailbox object.
 *
 * This function will connect and select an
 * IMAP mailbox.
 *
 * Return Value:
 * The C2Db object.
 **/
gint
c2_db_imap_load (C2DbIMAP *dbimap)
{
	
}

static void
c2_db_imap_connect (C2DbIMAP *dbimap)
{
	gchar *ip;
	
	c2_return_val_if_fail (dbimap, -1, C2EDATA);

	/* If the mailbox is already connected */
	if (C2_DB (dbimap)->mailbox->protocol.imap.sock >= 0)
		return 0;

	/* Resolve */
	gtk_signal_emit (GTK_OBJECT (dbimap), c2_db_imap_signals[RESOLVE]);
	if (c2_net_resolve (C2_DB (dbimap)->mailbox->protocol.imap.host, &ip))
	{
#ifdef USE_DEBUG
		g_print ("Unable to resolve hostname: %s\n",
					c2_error_get (c2_errno));
#endif
		gtk_signal_emit (GTK_OBJECT (dbimap), c2_db_imap_signals[DISCONNECT], FALSE);
		return -1;
	}

	/* Connect */
	gtk_signal_emit (GTK_OBJECT (dbimap), c2_db_imap_signals[CONNECT]);
	if (c2_net_connect (ip, C2_DB (dbimap)->mailbox->protocol.imap.port,
				&C2_DB (dbimap)->mailbox->protocol.imap.sock) < 0)
	{
#ifdef USE_DEBUG
		g_print ("Unable to connect: %s\n",
					c2_error_get (c2_errno));
#endif
		g_free (ip);
		gtk_signal_emit (GTK_OBJECT (dbimap), c2_db_imap_signals[DISCONNECT], FALSE);
		return -1;
	}
	g_free (ip);
}
