/*  Cronos II Mail Client
 *  Copyright (C) 2000-2001 Pablo Fern�ndez Navarro
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
#include <glib.h>

#include "error.h"
#include "smtp.h"
#include "utils.h"

static gint
c2_smtp_connect									(C2Smtp *smtp);

#define DEFAULT_FLAGS C2_SMTP_DONT_PERSIST

static C2Smtp *cached_smtp = NULL;

static gint
c2_smtp_connect (C2Smtp *smtp)
{
	c2_return_val_if_fail (smtp, -1, C2EDATA);
L	
	return 0;
}

C2Smtp *
c2_smtp_new (const gchar *address, gint port)
{
	C2Smtp *smtp;

	c2_return_val_if_fail (address, NULL, C2EDATA);

	smtp = g_new0 (C2Smtp, 1);

	smtp->address = g_strdup (address);
	smtp->port = port;

	/* Initialize the Mutex */
	pthread_mutex_init (&smtp->lock, NULL);
	smtp->flags = DEFAULT_FLAGS;

	if (smtp->flags & C2_SMTP_DO_PERSIST)
	{
		/* Cache the object if it has been marked as persistent
		 * and connect it
		 */
		c2_smtp_connect (smtp);
		cached_smtp = smtp;
	}

	return smtp;
}

void
c2_smtp_set_flags (C2Smtp *smtp, gint flags)
{
	c2_return_if_fail (smtp, C2EDATA);

	smtp->flags = flags;
}

void
c2_smtp_free (C2Smtp *smtp)
{
	if (!smtp)
		smtp = cached_smtp;
	
	c2_return_if_fail (smtp, C2EDATA);

	g_free (smtp->address);
	pthread_mutex_destroy (&smtp->lock);
	
	if (smtp->sock > 0)
		close (smtp->sock);

	g_free (smtp);

	if (!smtp)
		cached_smtp = NULL;
}