/*  Cronos II Mail Client /libcronos/smtp.c
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
#include <stdarg.h>

#include "error.h"
#include "smtp.h"
#include "utils.h"
#include "utils-net.h"
#include "i18n.h"

/* hard-hat area, in progress by bosko */
/* feel free to mess around -- help me get this module up to spec faster! */
/* (in progress) TODO: implement keep-alive smtp connection */
/* TODO: implement local sendmail capability */
/* TODO: upgrade this module to use the C2 Net-Object */

static gint
c2_smtp_connect (C2Smtp *smtp);

static gint
c2_smtp_send_headers (C2Smtp *smtp, C2Message *message);

static gboolean
smtp_test_connection(C2Smtp *smtp);

static void
smtp_disconnect(C2Smtp *smtp);

static GList *
get_mail_addresses (const gchar *string);

static void
c2_smtp_set_error(C2Smtp *smtp, gchar *error);

#define DEFAULT_FLAGS C2_SMTP_DONT_PERSIST

#define SOCK_READ_FAILED  _("Internal socket read operation failed")
#define SOCK_WRITE_FAILED _("Internal socket write operation failed")

static C2Smtp *cached_smtp = NULL;

static gint
c2_smtp_connect (C2Smtp *smtp)
{
	gchar *ip = NULL;
	gchar *hostname = NULL;
	gchar *buffer = NULL;
	
	if(smtp->sock && !(smtp->flags==C2_SMTP_DONT_PERSIST)) 
		smtp_disconnect(smtp);
	else if(smtp->sock && smtp->flags==C2_SMTP_DO_PERSIST)
		return 0;
	
	if(c2_net_resolve(smtp->host, &ip) < 1)
	{
		c2_smtp_set_error(smtp, _("Unable to resolve SMTP server hostname"));
		pthread_mutex_unlock(&smtp->lock);
		return -1;
	}
	if(c2_net_connect(ip, smtp->port, &smtp->sock) < 0) 
	{
		c2_smtp_set_error(smtp, _("Unable to connect to SMTP server"));
		pthread_mutex_unlock(&smtp->lock);
		g_free(ip);
		return -1;
	}
	g_free(ip);
	if(c2_net_read(smtp->sock, &buffer) < 0)
	{
		c2_smtp_set_error(smtp, SOCK_READ_FAILED);
		pthread_mutex_unlock(&smtp->lock);
		smtp_disconnect(smtp);
		return -1;
	}
	if(!c2_strneq(buffer, "220", 3))
 	{
		c2_smtp_set_error(smtp, _("SMTP server was not friendly on our connect! May not be RFC compliant"));
		g_free(buffer);
		pthread_mutex_unlock(&smtp->lock);
		smtp_disconnect(smtp);
		return -1;
	}
	g_free(buffer);
	
	/* TODO: implement EHLO */
	hostname = c2_net_get_local_hostname(smtp->sock);
	if(!hostname)
		hostname = g_strdup("CronosII");
	if(c2_net_send(smtp->sock, "HELO %s\r\n", hostname) < 0)
	{
		c2_smtp_set_error(smtp, SOCK_WRITE_FAILED);
		g_free(hostname);
		pthread_mutex_unlock(&smtp->lock);
		smtp_disconnect(smtp);
		return -1;
	}
	g_free(hostname);
	if(c2_net_read(smtp->sock, &buffer) < 0)
	{
		c2_smtp_set_error(smtp, SOCK_READ_FAILED);
		pthread_mutex_unlock(&smtp->lock);
		smtp_disconnect(smtp);
		return -1;
	}
	if(!c2_strneq(buffer, "250", 3))
	{
		c2_smtp_set_error(smtp, 
											_("SMTP server did not respond to 'HELO/EHLO in a friendly way"));
		g_free(buffer);
		pthread_mutex_unlock(&smtp->lock);
		smtp_disconnect(smtp);
		return -1;
	}
	g_free(buffer);
	
	return 0;
}
	

C2Smtp *
c2_smtp_new (C2SMTPType type, ...)
{
	C2Smtp *smtp;
	va_list args;

	smtp = g_new0 (C2Smtp, 1);
	smtp->type = type;
	smtp->sock = 0;
	smtp->error = NULL;
	switch (type)
	{
		case C2_SMTP_REMOTE:
			va_start (args, type);
			smtp->host = g_strdup (va_arg (args, const gchar *));
			smtp->port = va_arg (args, gint);
			smtp->authentication = va_arg (args, gboolean);
			smtp->user = g_strdup (va_arg (args, const gchar *));
			smtp->pass = g_strdup (va_arg (args, const gchar *));
			va_end (args);
			
			if (smtp->flags & C2_SMTP_DO_PERSIST)
			{
				/* Cache the object if it has been marked as persistent
				 * and connect it
				 */
				c2_smtp_connect (smtp);
				cached_smtp = smtp;
			}
			break;
		case C2_SMTP_LOCAL:
			smtp->host = NULL;
			smtp->authentication = FALSE;
			smtp->user = NULL;
			smtp->pass = NULL;
			break;
	}

	/* Initialize the Mutex */
	pthread_mutex_init (&smtp->lock, NULL);
	smtp->flags = DEFAULT_FLAGS;

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

	pthread_mutex_destroy (&smtp->lock);
	
	if (smtp->sock > 0)
		close (smtp->sock);

	if (smtp->error)
		g_free(smtp->error);
	
	g_free (smtp);

	if (!smtp)
		cached_smtp = NULL;
}

gint
c2_smtp_send_message (C2Smtp *smtp, C2Message *message) 
{
	gchar *buffer;
	
	/* Lock the mutex */
  pthread_mutex_lock (&smtp->lock);
	
	if(smtp->type == C2_SMTP_REMOTE) 
	{
		if(!smtp_test_connect(smtp))
			if(c2_smtp_connect(smtp) < 0)
				return -1;
		if(c2_smtp_send_headers(smtp, message) < 0)
			return -1;
		if(c2_net_send(smtp->sock, "%s\n\n%s\n.\r\n", message->header, 
									 message->body) < 0) 
		{
			c2_smtp_set_error(smtp, SOCK_WRITE_FAILED);
			pthread_mutex_unlock(&smtp->lock);
			smtp_disconnect(smtp);
			return -1;
		}
		/* TODO: Implement sending MIME attachments */
		if(c2_net_read(smtp->sock, &buffer) < 0)
		{
			c2_smtp_set_error(smtp, SOCK_READ_FAILED);
			pthread_mutex_unlock(&smtp->lock);
			smtp_disconnect(smtp);
			return -1;
		}
		if(!c2_strneq(buffer, "250", 3))
		{
			c2_smtp_set_error(smtp, _("SMTP server did not respond to our sent messaage in a friendly way"));
			g_free(buffer);
			pthread_mutex_unlock(&smtp->lock);
			smtp_disconnect(smtp);
			return -1;
		}
		if(smtp->flags == C2_SMTP_DONT_PERSIST)
			smtp_disconnect(smtp);
		g_free(buffer);
		pthread_mutex_unlock(&smtp->lock);
		return 0;
	}
	else if(smtp->type == C2_SMTP_LOCAL) {
		/* TODO */
	}

}

static gint
c2_smtp_send_headers(C2Smtp *smtp, C2Message *message)
{
	gchar *buffer;
	gchar *temp;
	GList *to = NULL;
	gint i;
	
  /* this func. needs some way to get the recepient list more efficiently
	 * especially for fields such as BCC... should we add another 
	 * argument to this function and to c2_smtp_send_message? */
	if(!(temp = c2_message_get_header_field(message, "From: ")))
	{
		c2_smtp_set_error(smtp, _("Unable to get 'From:' header from message"));
		pthread_mutex_unlock(&smtp->lock);
		smtp_disconnect(smtp);
		return -1;
	}
	if(c2_net_send(smtp->sock, "MAIL FROM: %s\r\n", temp) < 0)
	{
		c2_smtp_set_error(smtp, SOCK_WRITE_FAILED);
		smtp_disconnect(smtp);
		pthread_mutex_unlock(&smtp->lock);
		smtp_disconnect(smtp);
		g_free(temp);
		return -1;
	}
	g_free(temp);
  if(!(temp = c2_message_get_header_field(message, "To: "))) 
	{
		c2_smtp_set_error(smtp, _("Unable to get 'To:' header from message"));
		pthread_mutex_unlock(&smtp->lock);
		smtp_disconnect(smtp);
		return -1;
	}
	if(c2_net_read(smtp->sock, &buffer) < 0)
	{
		c2_smtp_set_error(smtp, SOCK_READ_FAILED);
		pthread_mutex_unlock(&smtp->lock);
		smtp_disconnect(smtp);
		return -1;
	}
	if(!c2_strneq(buffer, "250", 3))
	{
		c2_smtp_set_error(smtp, _("SMTP server did not reply to 'MAIL FROM:' in a friendly way"));
		g_free(buffer);
    pthread_mutex_unlock(&smtp->lock);
		smtp_disconnect(smtp);
		return -1;
	}
	g_free(buffer);
	to = get_mail_addresses(temp);
	for(i=0; i < g_list_length(to); i++)
	{
		if(c2_net_send(smtp->sock, "RCPT TO: %s\r\n", g_list_nth_data(to, i)) < 0)
		{
			c2_smtp_set_error(smtp, SOCK_WRITE_FAILED);
			pthread_mutex_unlock(&smtp->lock);
			smtp_disconnect(smtp);	
			for(i=0; i < g_list_length(to); i++)
				g_free(g_list_nth_data(to, i));
			g_list_free(to);
			break;
		}
		if(c2_net_read(smtp->sock, &buffer) < 0)
		{
			c2_smtp_set_error(smtp, SOCK_READ_FAILED);
			pthread_mutex_unlock(&smtp->lock);
			smtp_disconnect(smtp);
			for(i=0; i < g_list_length(to); i++)
				g_free(g_list_nth_data(to, i));
			g_list_free(to);
			break;
		}
		if(!c2_strneq(buffer, "250", 3))
		{
			c2_smtp_set_error(smtp, _("SMTP server did not reply to 'RCPT TO:' in a friendly way"));
			g_free(buffer);
			pthread_mutex_unlock(&smtp->lock);
			smtp_disconnect(smtp);
			for(i=0; i < g_list_length(to); i++)
				g_free(g_list_nth_data(to, i));
			g_list_free(to);
			break;
		}
		g_free(buffer);
	}
	for(i=0; i < g_list_length(to); i++)
		g_free(g_list_nth_data(to, i));
	g_list_free(to);
	if(c2_net_send(smtp->sock, "DATA\r\n", temp) < 0) 
	{
		c2_smtp_set_error(smtp, SOCK_WRITE_FAILED);
		pthread_mutex_unlock(&smtp->lock);
		smtp_disconnect(smtp);
		return -1;
	}
	if(c2_net_read(smtp->sock, &buffer) < 0)
	{
		c2_smtp_set_error(smtp, SOCK_READ_FAILED);
		pthread_mutex_unlock(&smtp->lock);
		smtp_disconnect(smtp);
		return -1;
	}
	if(!c2_strneq(buffer, "354", 3))
	{
		c2_smtp_set_error(smtp, _("SMTP server did not reply to 'DATA' in a friendly way"));
		g_free(buffer);
		pthread_mutex_unlock(&smtp->lock);
		smtp_disconnect(smtp);
		return -1;
	}
	g_free(buffer);

	return 0;
}

static gboolean
smtp_test_connection(C2Smtp *smtp)
{
	gchar *buffer;
	
	if(smtp == 0)
		return FALSE;
	if(c2_net_send(smtp->sock, "NOOP\r\n") < 0)
		return FALSE;
	if(c2_net_read(smtp->sock, &buffer) < 0)
		return FALSE;
	
	return TRUE;
}


static void
smtp_disconnect(C2Smtp *smtp)
{
	if(!smtp)
		smtp = cached_smtp;

	c2_net_send(smtp->sock, "QUIT\r\n");
	c2_net_disconnect(smtp->sock);
}

	
/* seperates @string into a GList of email addresses */
static GList *
get_mail_addresses (const gchar *string) 
{
	gint i;
	GString *str = NULL;
	GList *list = NULL;
	
	for(i=0; i < strlen(string); i++) {
		if(((string[i] == ';' || string[i] == ',') && i > 0) || i+1 == strlen(string)) {
			g_list_append(list, str->str);
			g_string_free(str, FALSE);
			str = NULL;
			continue;
		}
		if(!str) str = g_string_new(NULL);
		g_string_append_c(str, string[i]);
	}
	
	return list;
}

static void
c2_smtp_set_error(C2Smtp *smtp, gchar *error) 
{
	if(smtp->error) g_free(error);
	smtp->error = g_strdup(error);
}
