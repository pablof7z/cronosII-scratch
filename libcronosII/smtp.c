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
#include <stdlib.h>

#include "error.h"
#include "smtp.h"
#include "utils.h"
#include "utils-net.h"
#include "i18n.h"

/* hard-hat area, in progress by bosko */
/* feel free to mess around -- help me get this module up to spec faster! */
/* TODO: implement a better error handling according to RFC821 4.2.1 (posted by pablo) */
/* TODO: implement authentication (posted by pablo) */
/* TODO: implement encoding of headers according to ISO's */
/* (done!) TODO: create a test-module */
/* (done!) TODO: implement multiple connections via Net-Object */
/* (done!) TODO: update C2 SMTP to be a real GtkObject w/ signals etc */
/* (done!) TODO: implement sending of MIME attachments */
/* (done!) TODO: implement BCC */
/* (done!) TODO: implement local sendmail capability */
/* (done!) TODO: implement EHLO */
/* (done!) TODO: implement keep-alive smtp connection */
/* (done!) TODO: SMTP will read the message for a "From:" header... */
/* (done!) TODO: fix smtp_update signal to work with multiple connections */

enum
{
	SMTP_UPDATE,
	FINISHED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

/* Private GtkObject functions */
static void
class_init									(C2SMTPClass *klass);

static void
init										(C2SMTP *smtp);

static void
destroy										(GtkObject *object);

/* Remote SMTP server functions */
static gint
c2_smtp_connect								(C2SMTP *smtp, C2NetObjectByte **byte);

static gint
c2_smtp_send_helo							(C2SMTP *smtp, C2NetObjectByte *byte, gboolean esmtp);

static gint
c2_smtp_send_headers						(C2SMTP *smtp, C2NetObjectByte *byte, C2Message *message);

static gint
c2_smtp_send_rcpt							(C2SMTP *smtp, C2NetObjectByte *byte, gchar *to);

static gint
c2_smtp_send_message_contents				(C2SMTP *smtp, C2NetObjectByte *byte, C2Message *message, const gint id);

static gboolean
smtp_test_connection						(C2SMTP *smtp, C2NetObjectByte *byte);

static void
smtp_disconnect								(C2SMTP *smtp, C2NetObjectByte *byte);

/* Local SMTP program functions */
static gint
c2_smtp_local_write_msg						(C2Message *message, gchar *file_name);

static gchar *
c2_smtp_local_get_recepients				(C2Message *message);

static gchar *
c2_smtp_local_divide_recepients				(gchar *to);

/* Misc. functions */
static void
c2_smtp_set_error							(C2SMTP *smtp, const gchar *error);

#define DEFAULT_FLAGS C2_SMTP_DO_NOT_PERSIST | C2_SMTP_DO_NOT_LOSE_PASSWORD

#define SOCK_READ_FAILED  _("Internal socket read operation failed")
#define SOCK_WRITE_FAILED _("Internal socket write operation failed")

static C2NetObject *parent_class = NULL;

GtkType
c2_smtp_get_type (void)
{
	static GtkType type = 0;
	
	if(!type)
	{
		static const GtkTypeInfo info = {
			"C2SMTP",
			sizeof(C2SMTP),
			sizeof(C2SMTPClass),
			(GtkClassInitFunc) class_init,
			(GtkObjectInitFunc) init,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL
		};
		
		type = gtk_type_unique(c2_net_object_get_type(), &info);
	}
	
	return type;
}

static void
class_init (C2SMTPClass *klass)
{
	GtkObjectClass *object_class;
	
	object_class = (GtkObjectClass *) klass;
	
	parent_class = gtk_type_class (c2_net_object_get_type ());
	
	signals[SMTP_UPDATE] =
		gtk_signal_new ("smtp_update",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2SMTPClass, smtp_update),
						c2_marshal_NONE__INT_INT_INT, GTK_TYPE_NONE, 3,
						GTK_TYPE_INT, GTK_TYPE_INT, GTK_TYPE_INT);
	signals[FINISHED] =
		gtk_signal_new ("finished",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2SMTPClass, finished),
						gtk_marshal_NONE__INT_INT, GTK_TYPE_NONE, 2,
						GTK_TYPE_INT, GTK_TYPE_INT);
	
	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);
	
	klass->smtp_update = NULL;
	klass->finished = NULL;
	object_class->destroy = destroy;
}

static void
init (C2SMTP *smtp)
{
	smtp->host = NULL;
	smtp->port = 25;
	smtp->ssl = FALSE;
	smtp->authentication = FALSE;
	smtp->user = NULL;
	smtp->pass = NULL;
	smtp->smtp_local_cmd = NULL;
	smtp->flags = DEFAULT_FLAGS;

	c2_mutex_init (&smtp->lock);
}

static void
destroy (GtkObject *object)
{
	C2SMTP *smtp = C2_SMTP(object);

	c2_mutex_lock(&smtp->lock);
	
	/* [TODO] (Note by Pablo)
	 * Hey, Bosko, this will give you troubles
	 * with C2_SMTP_LOCAL since the C2NetObject
	 * it uses is not using specified states
	 * like offline, maybe you should put
	 * in the c2_smtp_new, if type == C2_SMTP_LOCAL
	 * that the net object must start with the
	 * offline state, and it would be nice that
	 * even thogh you are using a C2_SMTP_LOCAL the
	 * other states are handled by this module
	 * (not by the C2NetObject since you
	 * will not be using its function but a external
	 * cmnd.
	 */
/*	if(!c2_net_object_is_offline(C2_NET_OBJECT(smtp)))
	{
		c2_net_object_disconnect(C2_NET_OBJECT(smtp));
		c2_net_object_destroy_byte (C2_NET_OBJECT (smtp));
#ifdef USE_DEBUG
		g_warning ("Destroying a C2SMTP object which was not offline\n");
#endif
	}*/

	c2_smtp_set_error(smtp, NULL);
	
	if (smtp->host)
		g_free(smtp->host);
	
	if(smtp->user)
		g_free(smtp->user);
	
	if(smtp->pass)
		g_free(smtp->pass);
	
	if(smtp->smtp_local_cmd)
		g_free(smtp->smtp_local_cmd);
	
	c2_mutex_unlock(&smtp->lock);
	c2_mutex_destroy(&smtp->lock);
}

C2SMTP *
c2_smtp_new (C2SMTPType type, ...)
{
	C2SMTP *smtp;
	va_list args;

	smtp = gtk_type_new (C2_TYPE_SMTP);
	
	smtp->type = type;
	c2_smtp_set_error(smtp, NULL);
	
	switch (type)
	{
		case C2_SMTP_REMOTE:
			smtp->smtp_local_cmd = NULL;
			va_start (args, type);
			smtp->host = g_strdup (va_arg (args, const gchar *));
			smtp->port = va_arg (args, gint);
			smtp->ssl = va_arg (args, gint);
			smtp->authentication = va_arg (args, gboolean);
			smtp->user = g_strdup (va_arg (args, const gchar *));
			smtp->pass = g_strdup (va_arg (args, const gchar *));
			va_end (args);
			c2_net_object_construct(C2_NET_OBJECT(smtp), smtp->host, smtp->port, smtp->ssl);
		c2_net_object_set_maximum(C2_NET_OBJECT(smtp), 0); /* default for now */
			break;
		case C2_SMTP_LOCAL:
			va_start (args, type);
			smtp->smtp_local_cmd = g_strdup (va_arg(args, const gchar*));
			va_end (args);
			smtp->host = NULL;
			smtp->port = 0;
			smtp->ssl = 0;
			smtp->authentication = FALSE;
			smtp->user = NULL;
			smtp->pass = NULL;
			break;
	}

	return smtp;
}

void
c2_smtp_set_flags (C2SMTP *smtp, gint flags)
{
	c2_return_if_fail (smtp, C2EDATA);
	
	smtp->flags = flags;
}

gint
c2_smtp_send_message (C2SMTP *smtp, C2Message *message, const gint id) 
{
	C2NetObjectByte *byte = NULL;
	gchar *buffer;

	gtk_object_ref (GTK_OBJECT (message));

	if(smtp->type == C2_SMTP_REMOTE) 
	{
		c2_mutex_lock(&smtp->lock);
		if(smtp->flags & C2_SMTP_DO_PERSIST)
			byte = smtp->persistent;
		if(!byte)
		{
			if(c2_smtp_connect(smtp, &byte) < 0)
			{
				gtk_signal_emit(GTK_OBJECT(smtp), signals[FINISHED], id, -1);
				return -1;
			}
		}
		if(smtp->flags & C2_SMTP_DO_PERSIST)
			smtp->persistent = byte;
		else
			smtp->persistent = NULL;
		if(smtp->flags & C2_SMTP_DO_NOT_PERSIST) c2_mutex_unlock(&smtp->lock);
		if(c2_smtp_send_headers(smtp, byte, message) < 0)
		{	
			gtk_signal_emit(GTK_OBJECT(smtp), signals[FINISHED], id, -1);
			return -1;
		}
		if(c2_smtp_send_message_contents(smtp, byte, message, id) < 0)
		{
			gtk_signal_emit(GTK_OBJECT(smtp), signals[FINISHED], id, -1);
			return -1;
		}
		if(c2_net_object_send(C2_NET_OBJECT(smtp), byte, "\r\n.\r\n") < 0)
		{
			c2_smtp_set_error(smtp, SOCK_WRITE_FAILED);
			smtp_disconnect(smtp, byte);
			gtk_signal_emit(GTK_OBJECT(smtp), signals[FINISHED], id, -1);
			return -1;
		}
		if(c2_net_object_read(C2_NET_OBJECT(smtp), &buffer, byte) < 0)
		{
			c2_smtp_set_error(smtp, SOCK_READ_FAILED);
			smtp_disconnect(smtp, byte);
			gtk_signal_emit(GTK_OBJECT(smtp), signals[FINISHED], id, -1);
			return -1;
		}
		if(!c2_strneq(buffer, "250", 3))
		{
			c2_smtp_set_error(smtp, _("SMTP server did not respond to our sent messaage in a friendly way"));
			g_free(buffer);
			smtp_disconnect(smtp, byte);
			gtk_signal_emit(GTK_OBJECT(smtp), signals[FINISHED], id, -1);
			return -1;
		}
		if(smtp->flags & C2_SMTP_DO_NOT_PERSIST)
			smtp_disconnect(smtp, byte);
		else
			c2_mutex_unlock(&smtp->lock);
		g_free(buffer);
	}
	else if(smtp->type == C2_SMTP_LOCAL) 
	{
		gchar *file_name, *from, *to, *temp, *cmd;

		file_name = c2_get_tmp_file (NULL);
		if(c2_smtp_local_write_msg(message, file_name) < 0) 
		{
			g_free(file_name);
			c2_smtp_set_error(smtp, _("System Error: Unable to write message to disk for local SMTP command"));
			gtk_signal_emit(GTK_OBJECT(smtp), signals[FINISHED], id, -1);
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
			gtk_signal_emit(GTK_OBJECT(smtp), signals[FINISHED], id, -1);
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
			gtk_signal_emit(GTK_OBJECT(smtp), signals[FINISHED], id, -1);
			return -1;
		}
		gtk_signal_emit(GTK_OBJECT(smtp), signals[SMTP_UPDATE], id, 1, 1);
		g_free(file_name);
		g_free(cmd);
		unlink(file_name);
	}

	gtk_object_unref (GTK_OBJECT (message));
	
	gtk_signal_emit(GTK_OBJECT(smtp), signals[FINISHED], id, 1);
	return 0;
}

static gint
c2_smtp_connect (C2SMTP *smtp, C2NetObjectByte **byte)
{
	gchar *ip = NULL;
	gchar *buffer = NULL;
	
	if(!c2_net_object_is_offline(C2_NET_OBJECT(smtp), *byte) && smtp->flags==C2_SMTP_DO_PERSIST)
		return 0;
	
	if(!(*byte = c2_net_object_run(C2_NET_OBJECT(smtp))))
	{
		c2_smtp_set_error(smtp, _("Unable to connect to SMTP server"));
		g_free(ip);
		return -1;
	}
	if(c2_net_object_read(C2_NET_OBJECT(smtp), &buffer, *byte) < 0)
	{
		c2_smtp_set_error(smtp, SOCK_READ_FAILED);
		smtp_disconnect(smtp, *byte);
		return -1;
	}
	if(!c2_strneq(buffer, "220", 3))
 	{
		c2_smtp_set_error(smtp, _("SMTP server was not friendly on our connect! May not be RFC compliant"));
		g_free(buffer);
		smtp_disconnect(smtp, *byte);
		return -1;
	}
	
	if(strstr(buffer, "ESMTP") == NULL)
	{
		g_free(buffer);
		if(c2_smtp_send_helo(smtp, *byte, FALSE) < 0)
			 return -1;
	}
	else
	{
		g_free(buffer);
		if(c2_smtp_send_helo(smtp, *byte, TRUE) < 0)
			return -1;
	}
	
	return 0;
}

static gint
c2_smtp_send_helo (C2SMTP *smtp, C2NetObjectByte *byte, gboolean esmtp)
{
	gchar *hostname, *buffer = NULL;
	
	/* hostname = c2_net_get_local_hostname(C2_NET_OBJECT(smtp)); */
	hostname = g_strdup("localhost.localdomain");
	if(!hostname)
		hostname = g_strdup("localhost.localdomain");
	
	if(esmtp)
	{
		if(c2_net_object_send(C2_NET_OBJECT(smtp), byte, "EHLO %s\r\n", hostname) < 0)
		{
			c2_smtp_set_error(smtp, SOCK_WRITE_FAILED);
			g_free(hostname);
			smtp_disconnect(smtp, byte);
			return -1;
		}
		g_free(hostname);
		do 
		{
			/* Put whatever code in here to mark certain special
			 * ESMTP extensions as working if c2 smtp module
			 * uses them and EHLO reports them */
			if(buffer) g_free(buffer);
			if(c2_net_object_read(C2_NET_OBJECT(smtp), &buffer, byte) < 0)
			{
				c2_smtp_set_error(smtp, SOCK_READ_FAILED);
				smtp_disconnect(smtp, byte);
				return -1;
			}
		}
		while(c2_strneq(buffer+3, "-", 1));
		if(!c2_strneq(buffer, "250", 3))
		{
			c2_smtp_set_error(smtp, 
												_("SMTP server did not respond to 'EHLO in a friendly way"));
			g_free(buffer);
			smtp_disconnect(smtp, byte);
			return -1;
		}
		g_free(buffer);
	}
	else
	{
		if(c2_net_object_send(C2_NET_OBJECT(smtp), byte, "HELO %s\r\n", hostname) < 0)
		{
			c2_smtp_set_error(smtp, SOCK_WRITE_FAILED);
			g_free(hostname);
			smtp_disconnect(smtp, byte);
			return -1;
		}
		g_free(hostname);
		if(c2_net_object_read(C2_NET_OBJECT(smtp), &buffer, byte) < 0)
		{
			c2_smtp_set_error(smtp, SOCK_READ_FAILED);
			smtp_disconnect(smtp, byte);
			return -1;
		}
		if(!c2_strneq(buffer, "250", 3))
		{
			c2_smtp_set_error(smtp,
												_("SMTP server did not respond to HELO in a friendly way"));
			g_free(buffer);
			smtp_disconnect(smtp, byte);
			return -1;
		}
		g_free(buffer);
	}
	
	return 0;
}


static gint
c2_smtp_send_headers(C2SMTP *smtp, C2NetObjectByte *byte, C2Message *message)
{
	gchar *buffer, *cc;
	gchar *temp = NULL, *temp2 = NULL;
	
	if(!(temp2 = c2_message_get_header_field(message, "From:")) 
		 || !(temp = c2_str_get_email(temp2)))
	{
		c2_smtp_set_error(smtp, _("Internal C2 Error: Unable to fetch \"From:\" header in email message"));
		if(temp2) g_free(temp2);
		smtp_disconnect(smtp, byte);
		return -1;
	}
	if(c2_net_object_send(C2_NET_OBJECT(smtp), byte, "MAIL FROM: %s\r\n", temp) < 0)
	{
		c2_smtp_set_error(smtp, SOCK_WRITE_FAILED);
		smtp_disconnect(smtp, byte);
		g_free(temp);
		g_free(temp2);
		return -1;
	}
	g_free(temp2);
	g_free(temp);
	if(!(temp = c2_message_get_header_field(message, "To:")))
	{
		c2_smtp_set_error(smtp, _("Internal C2 Error: Unable to fetch \"To:\" header in email message"));
		return -1;
	}
	if(c2_net_object_read(C2_NET_OBJECT(smtp), &buffer, byte) < 0)
	{
		c2_smtp_set_error(smtp, SOCK_READ_FAILED);
		smtp_disconnect(smtp, byte);
		return -1;
	}
	if(!c2_strneq(buffer, "250", 3))
	{
		c2_smtp_set_error(smtp, _("SMTP server did not reply to 'MAIL FROM:' in a friendly way"));
		g_free(buffer);
		smtp_disconnect(smtp, byte);
		return -1;
	}
	g_free(buffer);
	if((cc  = c2_message_get_header_field(message, "CC:")))
	{
		buffer = g_strconcat(temp, ",", cc, NULL);
		g_free(temp);
		g_free(cc);
	}
	else buffer = temp;
	temp = NULL;
	if((cc  = c2_message_get_header_field(message, "BCC:")))
	{
		buffer = g_strconcat(buffer, ",", cc, NULL);
		g_free(cc);
	}
	if(c2_smtp_send_rcpt(smtp, byte, buffer) < 0)
	{
		smtp_disconnect(smtp, byte);
		return -1;
	}
	g_free(buffer);
	if(c2_net_object_send(C2_NET_OBJECT(smtp), byte, "DATA\r\n") < 0) 
	{
		c2_smtp_set_error(smtp, SOCK_WRITE_FAILED);
		smtp_disconnect(smtp, byte);
		return -1;
	}
	if(c2_net_object_read(C2_NET_OBJECT(smtp), &buffer, byte) < 0)
	{
		c2_smtp_set_error(smtp, SOCK_READ_FAILED);
		smtp_disconnect(smtp, byte);
		return -1;
	}
	if(!c2_strneq(buffer, "354", 3))
	{
		c2_smtp_set_error(smtp, _("SMTP server did not reply to 'DATA' in a friendly way"));
		g_free(buffer);
		smtp_disconnect(smtp, byte);
		return -1;
	}
	g_free(buffer);

	return 0;
}

static gint
c2_smtp_send_message_contents(C2SMTP *smtp, C2NetObjectByte *byte, 
				 			  C2Message *message, const gint id)
{
	/* This function sends the message body so that there is no
	 * bare 'LF' and that all '\n' are sent as '\r\n' */
	gchar *ptr, *start, *buf, *contents = message->header;
	guint len, sent = 0;	
	
	len = strlen(message->body) + 2 + strlen(message->header);
	
	gtk_signal_emit(GTK_OBJECT(smtp), signals[SMTP_UPDATE], id, len, 0);	
	
	while(1) 
	{
		for(ptr = start = contents; *ptr != '\0'; ptr++)
		{
			if(start == ptr && contents == message->header)
			{
				/* if this is the BCC and C2 internal headers, don't send them! */
				if(c2_strneq(ptr, "BCC: ", 4) || c2_strneq(ptr, "X-CronosII", 10)) 
				{
					for( ; *ptr != '\n' && *(ptr+1) != '\0'; ptr++)
						sent++;
					sent++;
					start = *ptr == '\0' ? NULL : ptr + 1;
					continue;
				}
			}
			if(*ptr == '\n' || *(ptr+1) == '\0')
			{
				if (*(ptr+1) == '\0')
					ptr++;
				buf = g_strndup (start, ptr - start);
				if (c2_net_object_send (C2_NET_OBJECT(smtp), byte, "%s\r\n", buf) < 0)
				{
					c2_smtp_set_error (smtp, SOCK_WRITE_FAILED);
					g_free (buf);
					smtp_disconnect (smtp, byte);
					return -1;
				}
				sent += strlen (buf) + 1;
				gtk_signal_emit (GTK_OBJECT (smtp), signals[SMTP_UPDATE], id, len, sent);
				g_free (buf);
				if (*ptr == '\0')
					ptr--;
				start = ptr + 1;
			}
		}
		if(contents == message->header)
			contents = message->body;
		else if(contents == message->body)
			break;
		if(c2_net_object_send(C2_NET_OBJECT(smtp), byte, "\r\n") < 0)
		{
			c2_smtp_set_error(smtp, SOCK_WRITE_FAILED);
			smtp_disconnect(smtp, byte);
			return -1;
		}
	}
	
	return 0;
}

static gboolean
smtp_test_connection(C2SMTP *smtp, C2NetObjectByte *byte)
{
	gchar *buffer;
	
	if(c2_net_object_is_offline(C2_NET_OBJECT(smtp), byte))
		return FALSE;
	if(c2_net_object_send(C2_NET_OBJECT(smtp), byte, "NOOP\r\n") < 0)
		return FALSE;
	if(c2_net_object_read(C2_NET_OBJECT(smtp), &buffer, byte) < 0)
		return FALSE;
	g_free(buffer);
	
	return TRUE;
}

static void
smtp_disconnect(C2SMTP *smtp, C2NetObjectByte *byte)
{	
	c2_net_object_send(C2_NET_OBJECT(smtp), byte, "QUIT\r\n");
	c2_net_object_disconnect(C2_NET_OBJECT(smtp), byte);
	c2_net_object_destroy_byte (C2_NET_OBJECT (smtp), byte);
	
	if(smtp->persistent == byte)
		smtp->persistent = NULL;
}

static gint
c2_smtp_local_write_msg(C2Message *message, gchar *file_name)
{
	FILE *file;
	
	if(!(file = fopen(file_name, "w")) ||
		!fwrite(message->header, strlen(message->header), 1, file)) 
	{
		if(file) 
		{
			fclose(file);
			unlink(file_name);
		}
		return -1;
	}

	if(!fprintf(file, "\n%s\n", message->body))
	{
		fclose(file);
		unlink(file_name);
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
c2_smtp_send_rcpt (C2SMTP *smtp, C2NetObjectByte *byte, gchar *to)
{
	gchar *ptr, *ptr2, *start, *buf;
	
	for(ptr = start = to; *ptr != '\0'; ptr++)
	{
		if(*ptr == ',' || *(ptr+1) == '\0')
		{
			if(*(ptr+1) == '\0') ptr++;
			buf = g_strndup(start, ptr - start);
			start += (ptr - start) + 1;
			
			/* weed out the actual address between the "<>",
			 * for compability reasons w/ some servers */
			if((ptr2 = strstr(buf, "<")))
			{
				gchar *ptr3;
				if((ptr3 = strstr(buf, ">")))
				{
					gchar *final = g_strndup(ptr2+1, ptr3 - (ptr2+1));
					g_free(buf);
					buf = final;
				}
			}
			if(c2_net_object_send(C2_NET_OBJECT(smtp), byte, "RCPT TO: %s\r\n", buf) < 0)
			{
				c2_smtp_set_error(smtp, SOCK_WRITE_FAILED);
				g_free(buf);
				return -1;
			}
			g_free(buf);
			if(c2_net_object_read(C2_NET_OBJECT(smtp), &buf, byte) < 0)
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
	GtkObject *object = GTK_OBJECT(smtp);
	gchar *buf;
	
	if((buf = gtk_object_get_data(object, "error")))
		g_free(buf);
	
	buf = g_strdup(error);
	gtk_object_set_data(object, "error", buf);
}
