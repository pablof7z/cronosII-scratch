/*  Cronos II Mail Client  /testsuite/smtp.c
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
#include <gtk/gtk.h>

#include <libcronosII/smtp.h>
#include <libcronosII/message.h>
#include <libcronosII/error.h>
#include <libcronosII/utils.h>

/* file to test the c2 smtp module  -- 
 * by Bosko Blagojevic <falling@users.sourcforge.net> */

gint
main (gint argc, gchar **argv)
{
	C2Smtp *smtp;
	C2Message *msg;
	
	smtp = c2_smtp_new(C2_SMTP_REMOTE, "mail.your.isp.net", 25, FALSE, NULL, NULL);
	
	msg = g_new0(C2Message, 1);
	msg->header = g_strdup("From: testing<testing@cronosii.sourceforge.net>\n"
													"To: your.email@users.sourceforge.net\n"
													"Subject: Testing C2 smtp module!");
	msg->body = g_strdup("Testing 1-2-3");
	
	/* sending the message... please work!! */
	if(c2_smtp_send_message(smtp, msg) == 0)
		printf("it worked! message was sent! check your mail!\n");
	else {
		printf("didn't work... back to the drawing board\n");
		printf("the error was: %s\n", smtp->error);
	}
	
	return 0;
}

