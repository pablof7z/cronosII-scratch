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
#include <glib.h>
#include <unistd.h>
#include <config.h>

#include "error.h"
#include "pop3.h"

#define DEFAULT_FLAGS C2_POP3_DONT_KEEP_COPY | C2_POP3_DONT_LOSE_PASSWORD
/**
 * c2_pop3_new
 * @user: Username.
 * @pass: Password (might be NULL).
 * @host: Hostame.
 * @port: Port.
 *
 * This function will create a new C2Pop3 with
 * the data you pass it and with some default configuration.
 * 
 * Return Value:
 * The allocated C2Pop3 object or NULL if there was an error.
 **/
C2Pop3 *
c2_pop3_new (const gchar *user, const gchar *pass, const gchar *host, gint port)
{
	C2Pop3 *pop3;
	
	c2_return_val_if_fail (user || host, NULL, C2EDATA);
	
	pop3 = g_new0 (C2Pop3, 1);

	pop3->user = g_strdup (user);
	pop3->pass = g_strdup (pass);
	pop3->host = g_strdup (host);
	pop3->port = port;
	pop3->flags = DEFAULT_FLAGS;
	pop3->wrong_pass_cb = NULL;
	pop3->sock = -1;

	return pop3;
}

/**
 * c2_pop3_set_flags
 * @pop3: An allocated C2Pop3 object.
 * @flags: Flags to set in the object.
 *
 * This function will force to change the flags
 * of a C2Pop3 object.
 * Flags let the object customize according
 * to the users preferences.
 **/
void
c2_pop3_set_flags (C2Pop3 *pop3, gint flags)
{
	c2_return_if_fail (pop3, C2EDATA);

	pop3->flags = flags;
}

/**
 * c2_pop3_set_wrong_pass_cb
 * @pop3: C2Pop3 object.
 * @func: C2Pop3GetPass function.
 *
 * This function sets the function that will be called when a wrong
 * password in this C2Pop3 object is found.
 * The C2Pop3GetPass type function should return the new password
 * or NULL if it want's to cancel the fetching.
 */
void
c2_pop3_set_wrong_pass_cb (C2Pop3 *pop3, C2Pop3GetPass func)
{
	c2_return_if_fail (pop3, C2EDATA);

	pop3->wrong_pass_cb = func;
}

/**
 * c2_pop3_free
 * @pop3: C2Pop3 object.
 *
 * This functions frees the C2Pop3 object.
 **/
void
c2_pop3_free (C2Pop3 *pop3)
{
	c2_return_if_fail (pop3, C2EDATA);

	if (pop3->sock > 0)
#ifdef USE_DEBUG
	{
		g_print ("A C2Pop3 object is being freed while a connection is "
				 "being used (%s)!\n", pop3->user);
#endif
		close (pop3->sock);
#ifdef USE_DEBUG
	}
#endif
	g_free (pop3->user);
	g_free (pop3->pass);
	g_free (pop3->host);
	g_free (pop3);
}

/**
 * c2_pop3_fetchmail
 * @pop3: Loaded C2Pop3 object.
 *
 * This function will download
 * messages from 
 **/
gint
c2_pop3_fetchmail (C2Pop3 *pop3)
{
	c2_return_val_if_fail (pop3, -1, C2EDATA);
}
