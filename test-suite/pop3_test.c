/*  Cronos II Mail Client  /testsuite/db.c
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
#include <stdio.h>
#include <glib.h>

#include <libcronosII/pop3.h>
#include <libcronosII/account.h>
#include <libcronosII/error.h>
#include <libcronosII/utils.h>

gint
main (gint argc, gchar **argv)
{

	C2Pop3 	*pop3 = NULL;
	gint	flags =	0;
	C2Account *act = NULL;

	gtk_init (&argc, &argv);

	flags = C2_POP3_DO_KEEP_COPY | C2_POP3_DO_USE_APOP;

      	pop3 = c2_pop3_new("USERNAME","PASSWOrD","mail.account.org",110);
	act  = c2_account_new("a","a","a","aa","a",TRUE,C2_ACCOUNT_POP3,C2_SMTP_LOCAL,C2_ACCOUNT_SIGNATURE_STATIC,"a",TRUE);
	act->protocol.pop3 = pop3;

	c2_pop3_set_flags(pop3,flags);

//	if(c2_net_object_run (C2_NET_OBJECT (pop3)) < 0)
//	{
//		
//	}

	c2_pop3_fetchmail(act);


	//c2_net_object_disconnect_with_error (C2_NET_OBJECT (pop3));



	return 0;
}

