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
/* TODO: implement authentication (posted by pablo) */
/* TODO: update this module to use C2 Net-Object */
/* (needs testing) TODO: implement sending of MIME attachments */
/* (in progress) TODO: implement local sendmail capability */
/* (in progress) TODO: create a test-module */
/* (done!) TODO: implement BCC */
/* (done!) TODO: implement EHLO */
/* (done!) TODO: implement keep-alive smtp connection */

static gint
c2_smtp_connect (C2SMTP *smtp);

static gint
c2_smtp_send_helo (C2SMTP *smtp, gboolean esmtp);

static gint
c2_smtp_send_headers (C2SMTP *smtp, C2Message *message);

static gint
c2_smtp_send_rcpt (C2SMTP *smtp, gchar *to);

static gint
c2_smtp_send_message_contents(C2SMTP *smtp, C2Message *message);

static gint
c2_smtp_send_message_mime_headers(C2SMTP *smtp, C2Message *message, gchar **boundry);

static gchar *
c2_smtp_mime_make_message_boundry (void);

static gint
c2_smtp_send_message_mime(C2SMTP *smtp, C2Message *message, gchar *boundry);

static gboolean
smtp_test_connection(C2SMTP *smtp);

static void
smtp_disconnect(C2SMTP *smtp);

static void
c2_smtp_set_error(C2SMTP *smtp, const gchar *error);

static gint
c2_smtp_local_write_msg(C2Message *message, gchar *file_name);

static gchar *
c2_smtp_local_get_recepients(C2Message *message);

static gchar *
c2_smtp_local_divide_recepients(gchar *to);

#define DEFAULT_FLAGS C2_SMTP_DONT_PERSIST

#define SOCK_READ_FAILED  _("Internal socket read operation failed")
#define SOCK_WRITE_FAILED _("Internal socket write operation failed")

static C2SMTP *cached_smtp = NULL;

C2SMTP *
c2_smtp_new (C2SMTPType type, ...)
{
	C2SMTP *smtp;
	va_list args;

	smtp = g_new0 (C2SMTP, 1);
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
c2_smtp_set_flags (C2SMTP *smtp, gint flags)
{
	c2_return_if_fail (smtp, C2EDATA);

	smtp->flags = flags;
	
	if (smtp->flags & C2_SMTP_DO_PERSIST)
	{
		/* Cache the object if it has been marked as persistent
		 * and connect it
		 */
		c2_smtp_connect (smtp);
		cached_smtp = smtp;
	}
}

void
c2_smtp_free (C2SMTP *smtp)
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
c2_smtp_send_message (C2SMTP *smtp, C2Message *message) 
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
		if(c2_smtp_send_message_contents(smtp, message) < 0)
			return -1;
		if(c2_net_send(smtp->sock, "\r\n.\r\n") < 0)
		{
			c2_smtp_set_error(smtp, SOCK_WRITE_FAILED);
			smtp_disconnect(smtp);
			pthread_mutex_unlock(&smtp->lock);
			return -1;
		}
		if(c2_net_read(smtp->sock, &buffer) < 0)
		{
			c2_smtp_set_error(smtp, SOCK_READ_FAILED);
			smtp_disconnect(smtp);
			pthread_mutex_unlock(&smtp->lock);
			return -1;
		}
		if(!c2_strneq(buffer, "250", 3))
		{
			c2_smtp_set_error(smtp, _("SMTP server did not respond to our sent messaage in a friendly way"));
			g_free(buffer);
			smtp_disconnect(smtp);
			pthread_mutex_unlock(&smtp->lock);
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
c2_smtp_connect (C2SMTP *smtp)
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
		smtp_disconnect(smtp);
		pthread_mutex_unlock(&smtp->lock);
		return -1;
	}
	if(!c2_strneq(buffer, "220", 3))
 	{
		c2_smtp_set_error(smtp, _("SMTP server was not friendly on our connect! May not be RFC compliant"));
		g_free(buffer);
		smtp_disconnect(smtp);
		pthread_mutex_unlock(&smtp->lock);
		return -1;
	}
	
	if(strstr(buffer, "ESMTP") == NULL)
	{
		g_free(buffer);
		if(c2_smtp_send_helo(smtp, FALSE) < 0)
			 return -1;
	}
	else
	{
		g_free(buffer);
		if(c2_smtp_send_helo(smtp, TRUE) < 0)
			return -1;
	}
	
	return 0;
}

static gint
c2_smtp_send_helo (C2SMTP *smtp, gboolean esmtp)
{
	gchar *hostname, *buffer = NULL;
	
	/* hostname = c2_net_get_local_hostname(smtp->sock); */
	hostname = g_strdup("localhost.localdomain");
	if(!hostname)
		hostname = g_strdup("localhost.localdomain");
	
	if(esmtp)
	{
		if(c2_net_send(smtp->sock, "EHLO %s\r\n", hostname) < 0)
		{
			c2_smtp_set_error(smtp, SOCK_WRITE_FAILED);
			g_free(hostname);
			smtp_disconnect(smtp);
			pthread_mutex_unlock(&smtp->lock);
			return -1;
		}
		g_free(hostname);
		do 
		{
			/* Put whatever code in here to mark certain special
			 * ESMTP extensions as working if c2 smtp module
			 * uses them and EHLO reports them */
			if(buffer) g_free(buffer);
			if(c2_net_read(smtp->sock, &buffer) < 0)
			{
				c2_smtp_set_error(smtp, SOCK_READ_FAILED);
				smtp_disconnect(smtp);
				pthread_mutex_unlock(&smtp->lock);
				return -1;
			}
		}
		while(c2_strneq(buffer+3, "-", 1));
		if(!c2_strneq(buffer, "250", 3))
		{
			c2_smtp_set_error(smtp, 
												_("SMTP server did not respond to 'EHLO in a friendly way"));
			g_free(buffer);
			smtp_disconnect(smtp);
			pthread_mutex_unlock(&smtp->lock);
			return -1;
		}
		g_free(buffer);
	}
	else
	{
		if(c2_net_send(smtp->sock, "HELO %s\r\n", hostname) < 0)
		{
			c2_smtp_set_error(smtp, SOCK_WRITE_FAILED);
			g_free(hostname);
			smtp_disconnect(smtp);
			pthread_mutex_unlock(&smtp->lock);
			return -1;
		}
		g_free(hostname);
		if(c2_net_read(smtp->sock, &buffer) < 0)
		{
			c2_smtp_set_error(smtp, SOCK_READ_FAILED);
			smtp_disconnect(smtp);
			pthread_mutex_unlock(&smtp->lock);
			return -1;
		}
		if(!c2_strneq(buffer, "250", 3))
		{
			c2_smtp_set_error(smtp,
												_("SMTP server did not respond to HELO in a friendly way"));
			g_free(buffer);
			smtp_disconnect(smtp);
			pthread_mutex_unlock(&smtp->lock);
			return -1;
		}
		g_free(buffer);
	}
	
	return 0;
}


static gint
c2_smtp_send_headers(C2SMTP *smtp, C2Message *message)
{
	gchar *buffer, *cc;
	gchar *temp;
	GList *to = NULL;
	gint i;
	
	if(!(temp = c2_message_get_header_field(message, "From:")))
	{
		c2_smtp_set_error(smtp, _("Internal C2 Error: Unable to fetch \"From:\" header in email message"));
		smtp_disconnect(smtp);
		pthread_mutex_unlock(&smtp->lock);
		return -1;
	}
	if(c2_net_send(smtp->sock, "MAIL FROM: %s\r\n", temp) < 0)
	{
		c2_smtp_set_error(smtp, SOCK_WRITE_FAILED);
		smtp_disconnect(smtp);
		pthread_mutex_unlock(&smtp->lock);
		g_free(temp);
		return -1;
	}
	g_free(temp);
	if(!(temp = c2_message_get_header_field(message, "To:")))
	{
		c2_smtp_set_error(smtp, _("Internal C2 Error: Unable to fetch \"To:\" header in email message"));
		pthread_mutex_unlock(&smtp->lock);
		return -1;
	}
	if(c2_net_read(smtp->sock, &buffer) < 0)
	{
		c2_smtp_set_error(smtp, SOCK_READ_FAILED);
		smtp_disconnect(smtp);
		pthread_mutex_unlock(&smtp->lock);
		return -1;
	}
	if(!c2_strneq(buffer, "250", 3))
	{
		c2_smtp_set_error(smtp, _("SMTP server did not reply to 'MAIL FROM:' in a friendly way"));
		g_free(buffer);
		smtp_disconnect(smtp);
		pthread_mutex_unlock(&smtp->lock);
		return -1;
	}
	g_free(buffer);
	if(cc  = c2_message_get_header_field(message, "CC:"))
	{
		buffer = g_strconcat(temp, ",", cc, NULL);
		g_free(temp);
		g_free(cc);
	}
	else buffer = temp;
	temp = NULL;
	if(cc  = c2_message_get_header_field(message, "BCC:"))
	{
		buffer = g_strconcat(buffer, ",", cc, NULL);
		g_free(cc);
	}
	if(c2_smtp_send_rcpt(smtp, buffer) < 0)
	{
		smtp_disconnect(smtp);
		pthread_mutex_unlock(&smtp->lock);
	}
	g_free(buffer);
	if(c2_net_send(smtp->sock, "DATA\r\n") < 0) 
	{
		c2_smtp_set_error(smtp, SOCK_WRITE_FAILED);
		smtp_disconnect(smtp);
		pthread_mutex_unlock(&smtp->lock);
		return -1;
	}
	if(c2_net_read(smtp->sock, &buffer) < 0)
	{
		c2_smtp_set_error(smtp, SOCK_READ_FAILED);
		smtp_disconnect(smtp);
		pthread_mutex_unlock(&smtp->lock);
		return -1;
	}
	if(!c2_strneq(buffer, "354", 3))
	{
		c2_smtp_set_error(smtp, _("SMTP server did not reply to 'DATA' in a friendly way"));
		g_free(buffer);
		smtp_disconnect(smtp);
		pthread_mutex_unlock(&smtp->lock);
		return -1;
	}
	g_free(buffer);

	return 0;
}

static gint
c2_smtp_send_message_contents(C2SMTP *smtp, C2Message *message)
{
	/* This function sends the message body so that there is no
	 * bare 'LF' and that all '\n' are sent as '\r\n' */
	gchar *ptr, *start, *buf, *contents = message->header;
	gchar *mimeboundry = NULL;
	
	while(1) 
	{
		for(ptr = start = contents; *ptr != '\0'; ptr++)
		{
			if(start == ptr && contents == message->header)
			{
				/* if this is the BCC  and C2 internal headers, don't send them! */
				if(c2_strneq(ptr, "BCC: ", 4) || c2_strneq(ptr, "X-C2", 4)) 
				{
					for( ; *ptr != '\n'; ptr++) ;
					start = ptr + 1;
					continue;
				}
			}
			if(*ptr == '\n' || *(ptr+1) == '\0')
			{
				if(*(ptr+1) == '\0') ptr++;
				buf = g_strndup(start, ptr - start);
				if(c2_net_send(smtp->sock, "%s\r\n", buf) < 0)
				{
					c2_smtp_set_error(smtp, SOCK_WRITE_FAILED);
					g_free(buf);
					if(mimeboundry) g_free(mimeboundry);
					smtp_disconnect(smtp);
					pthread_mutex_unlock(&smtp->lock);
					return -1;
				}
				g_free(buf);
				if(*ptr == '\0') ptr--;
				start = ptr + 1;
			}
		}
		if(contents == message->header)
		{
			if(c2_smtp_send_message_mime_headers(smtp, message, &mimeboundry) < 0)
				return -1;
			contents = message->body;
		}
		else if(contents == message->body)
		{
			if(c2_smtp_send_message_mime(smtp, message, mimeboundry) < 0)
			{	
				g_free(mimeboundry);
				return -1;
			}
			if(mimeboundry) g_free(mimeboundry);
			break;
		}
		if(c2_net_send(smtp->sock, "\r\n") < 0)
		{
			c2_smtp_set_error(smtp, SOCK_WRITE_FAILED);
			smtp_disconnect(smtp);
			if(mimeboundry) g_free(mimeboundry);
			pthread_mutex_unlock(&smtp->lock);
			return -1;
		}

	}
	
	return 0;
}

/* send_message_mime_headers
 * @smtp: the C2SMTP Object to use as network connection
 * @message: the actual message we are in the process of sending
 * @boundry: a string that will get allocated and set to be the boundry
 *           of the MIME message 
 * 
 * Function sends the MIME headers such as Mime-Version: and the text/plain
 * MIME header before sending the actual text of the message 
 * 
 * Return Value:
 * 0 on success, -1 on failure.
 **/
static gint
c2_smtp_send_message_mime_headers(C2SMTP *smtp, C2Message *message, gchar **boundry)
{
	gchar *mimeinfo, *errmsg, *msgheader;
	
	if(!message->mime)
		return 0;
	
	mimeinfo = g_strdup("MIME-Version: 1.0\r\n"
								 "Content-Type: multipart/mixed; boundary=");
	
	*boundry = c2_smtp_mime_make_message_boundry();
	
	errmsg = g_strdup("This is a multipart message in MIME format.\r\n"
										"The fact that you can read this text means that your\r\n"
										"mail client does not understand MIME messages and\r\nthe attachments"
										"enclosed. You should consider moving to another mail client or\r\n"
										"upgrading to a higher version.\r\nFor further information and help"
										"please see http://sourceforge.net/projects/cronosii/ and\r\n"
										"feel free to ask for help on our online forums or email lists");
	
	msgheader = g_strdup("Content-Type: text/plain\r\n"
											 "Content-Transfer-Encoding: 8bit\r\n"
											 "Content-Disposition: inline");
	
	if(c2_net_send(smtp->sock, "%s%s\r\n%s\r\n%s\r\n%s\r\n", mimeinfo, *boundry, errmsg, 
		*boundry, msgheader) < 0)
	{
		c2_smtp_set_error(smtp, SOCK_WRITE_FAILED);
		g_free(mimeinfo);
		g_free(*boundry);
		g_free(errmsg);
		g_free(msgheader);
		smtp_disconnect(smtp);
		pthread_mutex_unlock(&smtp->lock);
		return -1;
	}
	
	g_free(mimeinfo);
	g_free(errmsg);
	g_free(msgheader);
	
	return 0;
}

/* c2_smtp_mime_make_message_boundry
 *
 * Creates a random string of chars for use as a MIME boundary
 * 
 * Return Value:
 * The freeable string containing the boundary
 **/
static gchar *
c2_smtp_mime_make_message_boundry (void) 
{
	gchar *boundry = NULL;
	gchar *ptr;
	gint i;
	
	srand (time (NULL));
	boundry = g_new0 (char, 50);
	sprintf (boundry, "Cronos-II=");
	ptr = boundry+10;
	for (i = 0; i < 39; i++) 
		*(ptr+i) = (rand () % 26)+97; /* From a to z */
	if (*(ptr+i-1) == '-') *(ptr+i-1) = '.';
	*(ptr+i) = '\0';
	
	return boundry;
}


static gint
c2_smtp_send_message_mime(C2SMTP *smtp, C2Message *message, gchar *boundry)
{
	C2Mime *mime;
	
	if(!message->mime)
		return 0;
	
	if(c2_net_send(smtp->sock, "%s\r\n", boundry) < 0)
	{
		c2_smtp_set_error(smtp, SOCK_WRITE_FAILED);
		smtp_disconnect(smtp);
		pthread_mutex_unlock(&smtp->lock);
		return -1;
	}

	for(mime = message->mime; mime; mime = mime->next)
	{
		if(c2_net_send(smtp->sock, "Content-Type: %s\r\nContent-Transfer-"
									"Encoding: %s\r\nContent-Disposition: %s; filename=\"%s\"\r\n\r\n%s%s",
									mime->type, mime->encoding, mime->disposition, mime->id, mime->part, boundry) < 0)
		{
			c2_smtp_set_error(smtp, /*SOCK_WRITE_FAILED*/"Ass Failure");
			smtp_disconnect(smtp);
			pthread_mutex_unlock(&smtp->lock);
			return -1;
		}
		if(!mime->next)
		{
			if(c2_net_send(smtp->sock, "--\r\n") < 0)
		  {
				c2_smtp_set_error(smtp, SOCK_WRITE_FAILED);
				smtp_disconnect(smtp);
				pthread_mutex_unlock(&smtp->lock);
				return -1;
			}
		}
		else
		{
			if(c2_net_send(smtp->sock, "\r\n") < 0)
		  {
				c2_smtp_set_error(smtp, SOCK_WRITE_FAILED);
				smtp_disconnect(smtp);
				pthread_mutex_unlock(&smtp->lock);
				return -1;
			}
		}
	}
	
	return 0;
}

static gboolean
smtp_test_connection(C2SMTP *smtp)
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
smtp_disconnect(C2SMTP *smtp)
{
	if(!smtp)
		smtp = cached_smtp;
	
	c2_net_send(smtp->sock, "QUIT\r\n");
	c2_net_disconnect(smtp->sock);
	
	smtp->sock = 0;
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
	
	cc = c2_message_get_header_field(message, "BCC:");
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

/* divides a "name <addy@server.com>, name2 <add2@server2.com>,"...
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


/* sends RCPT: commands */
static gint
c2_smtp_send_rcpt (C2SMTP *smtp, gchar *to)
{
	gchar *ptr, *start, *buf;
	
	for(ptr = start = to; *ptr != '\0'; ptr++)
	{
		if(*ptr == ',' || *(ptr+1) == '\0')
		{
			if(*(ptr+1) == '\0') ptr++;
			buf = g_strndup(start, ptr - start);
			start += (ptr - start) + 1;
			if(c2_net_send(smtp->sock, "RCPT TO: %s\r\n", buf) < 0)
			{
				c2_smtp_set_error(smtp, SOCK_WRITE_FAILED);
				g_free(buf);
				return -1;
			}
			g_free(buf);
			if(c2_net_read(smtp->sock, &buf) < 0)
			{
				c2_smtp_set_error(smtp, SOCK_READ_FAILED);
				return -1;
			}
			if(!c2_strneq(buf, "250", 3) && !c2_strneq(buf, "251", 3))
			{
				c2_smtp_set_error(smtp, _("SMTP server did not reply to 'RCPT TO:' in a friendly way"));
				g_free(buf);
				return -1;
			}
			g_free(buf);
			if(*ptr == '\0') break;
		}
	}
	
	return 0;
}

static void
c2_smtp_set_error(C2SMTP *smtp, const gchar *error) 
{
	if(smtp->error) g_free(smtp->error);
	smtp->error = g_strdup(error);
}
