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
#include <gtk/gtk.h>
#include <config.h>
#include <string.h>

#include "request.h"
#include "error.h"
#include "utils.h"
#include "utils-net.h"

static void
class_init										(C2RequestClass *klass);

static void
init											(C2Request *request);

static void
destroy											(GtkObject *object);

static void
c2_request_construct_http						(C2Request *request);

static void
c2_request_construct_ftp						(C2Request *request);

static void
c2_request_construct_file						(C2Request *request);

static void
parse_url										(const gchar *or_url, gchar **host, guint *port, gchar **path);

enum
{
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static C2NetObjectClass *parent_class = NULL;

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
http_retrieve (C2Request *request, gint *sock)
{
	gchar *tmp;
	gint bytes, total_bytes;
	gchar *ptr;
	gchar *retval = NULL;
	gchar buffer[BUFSIZ];

	/* Retrieve */
	gtk_signal_emit (GTK_OBJECT (request), c2_request_signals[RETRIEVE], 0);
	if (c2_net_object_send (C2_NET_OBJECT (request), "GET %s HTTP/1.0\r\n"
						   "Host: %s\r\n"
						   "\r\n", gtk_object_get_data (GTK_OBJECT (request), "path"),
						   C2_NET_OBJECT (request)->host) < 0)
	{
#ifdef USE_DEBUG
		g_print ("Unable to query the socket: %s\n", c2_error_get (c2_errno));
#endif
		g_free (host);
		close (*sock);
		gtk_signal_emit (GTK_OBJECT (request), c2_request_signals[DISCONNECT], FALSE);
		return -1;
	}
	g_free (host);
	g_free (path);

	/* Initialize request */
	request->source = NULL;
	request->got_size = 0;
	total_bytes = 0;
	while ((bytes = read (*sock, buffer, sizeof (buffer))) > 0)
	{
		request->source = g_realloc (request->source, total_bytes+bytes);
		memcpy (request->source + total_bytes, buffer, bytes);
		
		total_bytes += bytes;
		
		gtk_signal_emit (GTK_OBJECT (request), c2_request_signals[RETRIEVE], request->got_size);
	}

	/* Calculate got_size */
	ptr = strstr (request->source, "\r\n\r\n")+4;
	request->got_size = total_bytes - (ptr - request->source);
	tmp = g_malloc (request->got_size);
	memcpy (tmp, ptr, request->got_size);
	g_free (request->source);
	request->source = tmp;

	/* Check the answer:
	 */ 
	
	gtk_signal_emit (GTK_OBJECT (request), c2_request_signals[DISCONNECT], TRUE);
	c2_net_disconnect (*sock);
}

static void
c2_request_construct_http (C2Request *request)
{
	gchar *tmp, *tmp2;
	gint sock;
	gint tmplength, length = 0;

	/* Parse URL */
	parse_url (request->url, &host, &port, &path);

	C2_NET_OBJECT (request)->host = host;
	C2_NET_OBJECT (request)->port = port;

	gtk_object_set_data (GTK_OBJECT (request), "path", path);
	
	if (c2_net_object_run (C2_NET_OBJECT (request)) < 0)
	{
		gtk_object_unref (GTK_OBJECT (request));
		return;
	}
	if (http_connect (request, &sock) && http_retrieve (request, &sock));
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
c2_request_set_proxy (const gchar *addr, guint port, const gchar *ignore)
{
	c2_return_if_fail (addr, C2EDATA);

	if (proxy)
		g_free (proxy);
	
	proxy = g_new0 (C2Proxy, 1);
	proxy->addr = addr;
	proxy->port = port;
	proxy->ignore = ignore;
}

void
c2_request_get_proxy (const gchar **addr, const gint *port, const gchar **ignore)
{
	if (!proxy)
	{
		addr = NULL;
		port = NULL;
		ignore = NULL;
	} else
	{
		addr = &proxy->addr;
		port = &proxy->port;
		ignore = &proxy->ignore;
	}
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

		c2_request_type = gtk_type_unique (gtk_object_get_type (), &c2_request_info);
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
