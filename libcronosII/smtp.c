/*  Cronos II - A GNOME mail client
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
#include <unistd.h>

#include "error.h"
#include "smtp.h"
#include "utils.h"
#include "utils-net.h"
#include "i18n.h"

/* hard-hat area, in progress by bosko */
/* feel free to mess around -- help me get this module up to spec faster! */
/* (done!) TODO: implement keep-alive smtp connection */
/* (in progress) TODO: implement local sendmail capability */
/* TODO: upgrade this module to use the C2 Net-Object */
/* (in progress) TODO: create a test-module */
/* TODO: implement BCC */
/* TODO: implement sending of MIME attachments */

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
c2_smtp_set_error(C2Smtp *smtp, const gchar *error);

static gint
c2_smtp_local_write_msg(C2Message *message, gchar *file_name);

static gchar *
c2_smtp_local_get_recepients(C2Message *message);

static gchar *
c2_smtp_local_divide_recepients(gchar *to);

#define DEFAULT_FLAGS C2_SMTP_DONT_PERSIST

#define SOCK_READ_FAILED  _("Internal socket read operation failed")
#define SOCK_WRITE_FAILED _("Internal socket write operation failed")

static C2Smtp *cached_smtp = NULL;

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
			smtp->smtp_local_cmd = NULL;
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
			va_start (args, type);
			smtp->smtp_local_cmd = g_strdup (va_arg(args, const gchar*));
			va_end (args);
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
		if(!smtp_test_connection(smtp))
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
	}
	else if(smtp->type == C2_SMTP_LOCAL) 
	{
		gchar *file_name, *from, *to, *temp, *cmd;

		file_name = c2_get_tmp_file();
		if(c2_smtp_local_write_msg(message, file_name) < 0) 
		{
			g_free(file_name);
			c2_smtp_set_error(smtp, _("System Error: Unable to write message to disk for local SMTP command"));
			pthread_mutex_unlock(&smtp->lock);
			return -1;
		}
		
		/* get the proper to and from headers */
		if(!(from = c2_message_get_header_field(message, "From: ")) || 
			!(to = c2_smtp_local_get_recepients(message)))
		{
			c2_smtp_set_error(smtp, _("Internal C2 Error: Unable to fetch headers in email message"));
			unlink(file_name);
			if(from) g_free(from);
			g_free(file_name);
			pthread_mutex_unlock(&smtp->lock);
			return -1;
		}
				
		cmd = g_strdup(smtp->smtp_local_cmd);
		temp = cmd;
		cmd = c2_str_replace_all(cmd, "%m", file_name);
		g_free(temp);
		temp = cmd;
		cmd = c2_str_replace_all(cmd, "%f", from);
		g_free(temp);
		temp = cmd;
		cmd = c2_str_replace_all(cmd, "%t", to);
		g_free(temp);
		g_free(to); 
		g_free(from);
		
		/* FINALLY execute the command :-) */
		if(system(cmd) < 0)
		{
			c2_smtp_set_error(smtp, _("Problem running local SMTP command to send messages -- Check SMTP settings"));
			unlink(file_name);
			g_free(file_name);
			g_free(cmd);
			pthread_mutex_unlock(&smtp->lock);
			return -1;
		}
		g_free(file_name);
		g_free(cmd);
		unlink(file_name);
	}
	pthread_mutex_unlock(&smtp->lock);
	return 0;
}

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
	
	if(c2_net_resolve(smtp->host, &ip) != 0)
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
	/* hostname = c2_net_get_local_hostname(smtp->sock); */ /* temp */
	hostname = g_strdup("CronosII");
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
		c2_smtp_set_error(smtp, _("Internal C2 Error: Unable to fetch \"From:\" header in email message"));
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
		c2_smtp_set_error(smtp, _("Internal C2 Error: Unable to fetch \"To:\" header in email message"));
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
		if(!c2_strneq(buffer, "250", 3) && !c2_strneq(buffer, "251", 3))
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
	
	if(smtp->sock == 0)
		return FALSE;
	if(c2_net_send(smtp->sock, "NOOP\r\n") < 0)
		return FALSE;
	if(c2_net_read(smtp->sock, &buffer) < 0)
		return FALSE;
	g_free(buffer);
	
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

static gint
c2_smtp_local_write_msg(C2Message *message, gchar *file_name)
{
	FILE *file;
	
	if(!(file = fopen(file_name, "w")) ||
		!fwrite(message->header, strlen(message->header), 1, file) ||
		!fwrite(message->body, strlen(message->body), 1, file)) 
	{
		if(file) 
		{
			fclose(file);
			unlink(file_name);
		}
		return -1;
	}
	fclose(file);
	
	return 0;
}

static gchar *
c2_smtp_local_get_recepients(C2Message *message)
{
	gchar *to, *cc, *temp;
	
	if(!(to = c2_message_get_header_field(message, "To: ")))
	{
		if(to) g_free(to);
		return NULL;
	}

	temp = c2_smtp_local_divide_recepients(to);
	g_free(to);
	to = temp;
	
	cc = c2_message_get_header_field(message, "CC:");
	if(cc)
	{
		temp = c2_smtp_local_divide_recepients(cc);
		g_free(cc);
		cc = temp;
		
		temp = g_strconcat(to, " ", cc, NULL);
		g_free(to);
		g_free(cc);
		to = temp;
	}
	
	return to;
}

/* divides a "name <addy@server.com>; name2 <add2@server2.com>;"...
 * string into a string a command line program like sendmail can
 * understand: "name <addy@server.com" "<name2 <add2@server2.com>"*/
static gchar *
c2_smtp_local_divide_recepients(gchar *to)
{
	gint i;
	gchar *ptr, *ptr2, *temp;
	GString *str = g_string_new(NULL);
	
	for(ptr = ptr2 = to, i = 0; ptr[0]; ptr++, i++)
	{
		if(ptr[0] == ';' || !ptr[1])
		{
			if(!ptr[1]) i++;
			temp = g_strndup(ptr2, i);
			str = g_string_append(str, "\"");
			str = g_string_append(str, temp);
			str = g_string_append(str, "\" ");
			g_free(temp);
			ptr2 = ptr + 1;
			i = -1;
			continue;
		}
	}
	temp = str->str;
	g_string_free(str, FALSE);
	return temp;
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
			if(i+1 == strlen(string)) {
				if(!str) str = g_string_new(NULL);
				g_string_append_c(str, string[i]);
			}
			list = g_list_append(list, str->str);
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
c2_smtp_set_error(C2Smtp *smtp, const gchar *error) 
{
	if(smtp->error) g_free(smtp->error);
	smtp->error = g_strdup(error);
}
