/*  Cronos II - A GNOME mail client
 *  Copyright (C) 2000-2001 Pablo Fernández
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
/**
 * Maintainer(s) of this file:
 * 		* Pablo Fernández
 * Code of this file by:
 * 		* Pablo Fernández
 */
#include <gtk/gtk.h>
#include <config.h>
#include <string.h>
#include <stdlib.h>

#include "request.h"
#include "error.h"
#include "utils.h"
#include "utils-str.h"
#include "utils-net.h"

static void
class_init										(C2RequestClass *klass);

static void
init											(C2Request *request);

static void
destroy											(GtkObject *object);

/*static void
c2_request_construct_http						(C2Request *request);

static void
c2_request_construct_ftp						(C2Request *request);

static void
c2_request_construct_file						(C2Request *request);*/

static void
parse_url										(const gchar *or_url, gchar **host, guint *port, gchar **path);

enum
{
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static C2NetObjectClass *parent_class = NULL;

#if FALSE
/*
 * The request module will use, by now, the gnome-download
 * program, distributed with gnome-core (>= 1.2, < 1.2?)
 * since Http is giving me too much problems and I don't have so much
 * time to waste with it.
 * 
 * Future versions (probably before Cronos II Scratch becomes Cronos II)
 * should recode it to use the internal functions and take care of
 * everything on its own.
 */
static C2Proxy *http_proxy = NULL;
static C2Proxy *ftp_proxy = NULL;

static void
c2_request_construct (C2Request *request)
{
	c2_return_if_fail (request, C2EDATA);

	switch (request->protocol)
	{
		case C2_REQUEST_HTTP:
			c2_request_construct_http (request);
			break;
		case C2_REQUEST_FTP:
			c2_request_construct_ftp (request);
			break;
		case C2_REQUEST_FILE:
			c2_request_construct_file (request);
			break;
	}
}

static gint
http_retrieve (C2Request *request)
{
	gchar *tmp;
	gint bytes, total_bytes;
	gchar *ptr;
	gchar *retval = NULL;
	gchar *buffer;
	FILE *fd;

	/* Retrieve */
	if (c2_net_object_send (C2_NET_OBJECT (request), NULL
							"GET %s HTTP/1.0\r\n"
							"Host: %s\r\n"
							"\r\n",
							gtk_object_get_data (GTK_OBJECT (request), "path"),
							C2_NET_OBJECT (request)->host) < 0)
	{
#ifdef USE_DEBUG
		g_print ("Unable to query the socket: %s\n", c2_error_get (c2_errno));
#endif
		return -1;
	}

	/* Initialize request */
	request->source = NULL;
	request->got_size = 0;
	total_bytes = 0;

	/* Read */
	while ((bytes = c2_net_object_read (C2_NET_OBJECT (request), &buffer)) > 0)
	{
		fprintf (fd, "%s", buffer);
		request->source = g_realloc (request->source, total_bytes+bytes);
		memcpy (request->source + total_bytes, buffer, bytes);
		
		total_bytes += bytes;
	}
	fclose (fd);

	/* Calculate got_size */
	ptr = strstr (request->source, "\r\n\r\n")+4;
	request->got_size = total_bytes - (ptr - request->source);

	/* Set the information of the source properly */
	tmp = g_strdup (ptr);
	g_free (request->source);
	request->source = tmp;

	/* TODO Check the answer:
	 * TODO Someone that knows a little of HTTP/1.0 and HTTP/1.1 error
	 * TODO codes, please, develop this.
	 */
	if (c2_strneq (request->source, "HTTP"))
	{
	}
}

static void
c2_request_construct_http (C2Request *request)
{
	gchar *host, *path, *tmp, *tmp2;
	gint port;
	gint sock;
	gint tmplength, length = 0;

	/* Parse URL */
	parse_url (request->url, &host, &port, &path);

	C2_NET_OBJECT (request)->host = host;
	C2_NET_OBJECT (request)->port = port;

	gtk_object_set_data (GTK_OBJECT (request), "path", path);

	/* Connect */
	if (c2_net_object_run (C2_NET_OBJECT (request)) < 0)
	{
		gtk_object_unref (GTK_OBJECT (request));
		return;
	}

	/* Retrieve */
	if (http_retrieve (request) < 0)
	{
		gtk_object_unref (GTK_OBJECT (request));
		return;
	}

	/* Disconnect */
	c2_net_object_disconnect (C2_NET_OBJECT (request));
	c2_net_object_destroy_byte (C2_NET_OBJECT (request));
}

static void
c2_request_construct_ftp (C2Request *request)
{
}

static void
c2_request_construct_file (C2Request *request)
{
}

void
c2_request_set_proxy (C2ProxyType type, const gchar *addr, guint port, const gchar *ignore)
{
	C2Proxy *proxy = (type == C2_PROXY_HTTP) ? http_proxy : ftp_proxy;
	
	if (proxy)
	{
		g_free (proxy->host);
		g_free (proxy->ignore);
		g_free (proxy);
	}
	
	proxy = g_new0 (C2Proxy, 1);
	proxy->host = g_strdup (addr);
	proxy->port = port;
	proxy->ignore = g_strdup (ignore);
}

void
c2_request_get_proxy (C2ProxyType type, gchar **addr, gint *port, gchar **ignore)
{
	C2Proxy *proxy = (type == C2_PROXY_HTTP) ? http_proxy : ftp_proxy;
	
	if (!proxy)
	{
		addr = NULL;
		port = NULL;
		ignore = NULL;
	} else
	{
		addr = &proxy->host;
		port = &proxy->port;
		ignore = &proxy->ignore;
	}
}
#endif

static void
c2_request_construct (C2Request *request)
{
	gchar *cmnd = g_strconcat ("lynx -source ", request->url, NULL);
	gchar buffer[1024], *ptr, *tmp;
	FILE *fd;
	gint bytes, total_bytes = 0;

	/* We will emit a fake signal (connect), is fake because
	 * we don't even know if it will be able to connect and
	 * even if it does connect.
	 * This should change when gnome-download stops being
	 * used.
	 */
	gtk_signal_emit_by_name (GTK_OBJECT (request), "connect");
	
	/* Connect through a pipe to gnome-download */
	/* TODO This implementation should change to use
	 * c2 specific functions and not a third party
	 * like gnome-download.
	 */
	if (!(fd = popen (cmnd, "r")))
	{
		gtk_signal_emit_by_name (GTK_OBJECT (request), "disconnect", FALSE);
		g_free (cmnd);
		return;
	}
	g_free (cmnd);

	/* We probably shouldn't load everything in memory,
	 * but save to a file, so we use don't eat all the memory
	 * when downloading Linux 2.4.
	 */
	while ((bytes = fread (buffer, sizeof (gchar), sizeof (buffer), fd)))
	{
		/* If this is the first line and starts with Content-Type, fuck it and the next one too */
		if (!total_bytes)
		{
			if (c2_strneq (buffer, "Content-Type:", 13))
			{
				/* Ignore this line and the next one */
				for (ptr = buffer; *ptr != '\0' && *ptr != '\n'; ptr++);
				ptr+=2;
			} else
				ptr = buffer;
			bytes -= ptr-buffer;
		} else
			ptr = buffer;

		/* This f*cking sh*t is still giving problems downloading
		 * the f*cking images through the motherf*cker http, the only
		 * code we share with the other s*cking implementation is
		 * the code that follows, so I guess this is the damn sh*t that's
		 * giving problems, somehow, but I'm sick of this sh*t, should
		 * get back later with it (or other could do it, I don't want
		 * to see any http crap any more! (sh*t, I'm really pissed!)
		 */
		tmp = g_new0 (gchar, total_bytes+bytes);
		memcpy (tmp, request->source, total_bytes);
		memcpy (tmp+total_bytes, ptr, bytes);
		g_free (request->source);
		request->source = tmp;
		total_bytes += bytes;
		request->got_size = total_bytes;
		gtk_signal_emit_by_name (GTK_OBJECT (request), "exchange", C2_NET_OBJECT_EXCHANGE_READ, bytes);
	}
	
//	request->source[total_bytes] = 0;
	
	pclose (fd);

	gtk_signal_emit_by_name (GTK_OBJECT (request), "disconnect", TRUE);
}

void
c2_request_run (C2Request *request)
{
	c2_request_construct (request);
}

const gchar *
c2_request_get_source (C2Request *request)
{
	return request->source;
}

static void
parse_url (const gchar *or_url, gchar **host, guint *port, gchar **path)
{
	const gchar *start, *end;
	gchar *tmp;
	
	c2_return_if_fail (or_url, C2EDATA);

	if (!(start = strstr (or_url, "://")))
		return;
	start += 3;
	
	/* Go through the '/' */
	for (end = start; *end != '\0'; end++)
		if (*end == '/')
			break;
	
	if (end)
		*host = g_strndup (start, end-start);
	else
		*host = g_strdup (start);
	
	/* Now check if the port is in the address */
	if ((start = strstr (*host, ":")))
	{
		start++;
		*port = atoi (start);
		tmp = g_strndup (*host, start-(*host)-1);
		g_free (*host);
		*host = tmp;
	}
	
	/* Get the path */
	if (end)
		*path = g_strdup (end);
	else
		*path = g_strdup_printf ("/");
}

C2Request *
c2_request_new (const gchar *url)
{
	C2Request *request;
	
	c2_return_val_if_fail (url, NULL, C2EDATA);

	request = gtk_type_new (C2_TYPE_REQUEST);

	/* Load the URL */
	request->url = g_strdup (url);

	/* Load the type */
	if (c2_strneq (url, "http:", 5))
		request->protocol = C2_REQUEST_HTTP;
	else if (c2_strneq (url, "ftp:", 4))
		request->protocol = C2_REQUEST_FTP;
	else if (c2_strneq (url, "file:", 5))
		request->protocol = C2_REQUEST_FILE;
	else
	{
		gtk_object_unref (GTK_OBJECT (request));
#ifdef USE_DEBUG
		g_print ("Unknown protocol '%s' in %s\n", url, __PRETTY_FUNCTION__);
#endif
		return NULL;
	}

	return request;
}

GtkType
c2_request_get_type (void)
{
	static GtkType c2_request_type = 0;

	if (!c2_request_type)
	{
		static const GtkTypeInfo c2_request_info =
		{
			"C2Request",
			sizeof (C2Request),
			sizeof (C2RequestClass),
			(GtkClassInitFunc) class_init,
			(GtkObjectInitFunc) init,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL
		};

		c2_request_type = gtk_type_unique (c2_net_object_get_type (), &c2_request_info);
	}

	return c2_request_type;
}

static void
class_init (C2RequestClass *klass)
{
	GtkObjectClass *object_class;

	object_class = (GtkObjectClass *) klass;

	parent_class = gtk_type_class (c2_net_object_get_type ());

	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);

	object_class->destroy = destroy;
}

static void
init (C2Request *request)
{
	request->url = NULL;
	request->request_size = 0;
	request->got_size = 0;
	request->source = NULL;
}

static void
destroy (GtkObject *object)
{
	C2Request *request;
	gchar *path = (gchar*) gtk_object_get_data (object, "path");
	
	request = C2_REQUEST (object);
	if (path)
		g_free (path);
	g_free (request->url);
	g_free (request->source);
}
