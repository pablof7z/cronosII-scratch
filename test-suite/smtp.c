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
#include <math.h>
#include <gtk/gtk.h>

#include <libcronosII/smtp.h>
#include <libcronosII/message.h>
#include <libcronosII/error.h>
#include <libcronosII/utils.h>

/* file to test the c2 smtp module  -- 
 * by Bosko Blagojevic <falling@users.sourcforge.net> */

struct asdf
{
	C2SMTP *smtp;
	C2Message *msg;
};

static void 
on_smtp_update(C2SMTP *smtp, gint id, guint len, guint sent);

static void
send_my_message(struct asdf *yo)
{
	gint i;
	
	/* sending the message... please work!! */
	for(i=0; i < 1; i++) { if(c2_smtp_send_message(yo->smtp, yo->msg, 1) == 0)
		printf("Sending mail via SMTP worked! Check your email\n");
	else {
		printf("Sending message via SMTP failed... back to the drawing board\n");
		printf("the error was: %s\n", gtk_object_get_data(GTK_OBJECT(yo->smtp), "error"));
	}
	}
}

gint
main (gint argc, gchar **argv)
{
	pthread_t thread, thread2, thread3, thread4, thread5;
	C2SMTP *smtp = NULL;
	C2Message *msg;
	struct asdf *yo = g_new0(struct asdf, 1);
	
	gtk_init(&argc, &argv);
	
	smtp = c2_smtp_new(C2_SMTP_REMOTE, "firewall", 25, FALSE, FALSE, NULL, NULL);
	c2_smtp_set_flags(smtp, C2_SMTP_DO_PERSIST);
	
	gtk_signal_connect(GTK_OBJECT(smtp), "smtp_update", GTK_SIGNAL_FUNC(on_smtp_update), NULL);
	
	msg = c2_message_new();
	msg->header = g_strdup("From: testing <testing@cronosii.net>\n"
													"To: c2-test@ciudad.com.ar\n"
													"Subject: Testing C2 smtp module!");
	msg->body = g_strdup("Testing 1-2-3\n");
	msg->mime = NULL;
	
	yo->smtp = smtp;
	yo->msg = msg;
	
	pthread_create(&thread,  NULL, (void*)send_my_message, yo);
	/* uncomment below to test multi-threading */
	/*pthread_create(&thread2, NULL, (void*)send_my_message, yo);
	pthread_create(&thread3, NULL, (void*)send_my_message, yo);
	pthread_create(&thread4, NULL, (void*)send_my_message, yo);
  pthread_create(&thread5, NULL, (void*)send_my_message, yo);*/
	
	printf("\tHit Crtl+C when sending is done to quit program\n");
	gtk_main();
	
	gtk_object_destroy(GTK_OBJECT(smtp));

	return 0;
}

static void
on_smtp_update(C2SMTP *smtp, gint id, guint len, guint sent)
{
	float len2 = len, sent2 = sent;
	int percent;
	len2 = floorf((sent2/len2)*100.00);
	percent = len2;
	
	printf("%i%s of message sent!\n", percent, "%");
}
