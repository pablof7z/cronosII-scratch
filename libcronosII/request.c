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
c2_request_class_init							(C2RequestClass *klass);

static void
c2_request_init									(C2Request *request);

static void
c2_request_destroy								(GtkObject *object);

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
	RESOLVE,
	CONNECT,
	RETRIEVE,
	DISCONNECT,
	LAST_SIGNAL
};

static guint c2_request_signals[LAST_SIGNAL] = { 0 };

static GtkObjectClass *parent_class = NULL;

static C2Proxy *proxy = NULL;

void
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

static void
c2_request_construct_http (C2Request *request)
{
	gchar *host, *ip;
	gchar *path;
	gchar *tmp, *string;
	gint port = 80; /* Default to 80 */
	guint sock;
	gint tmplength, length = 0;
	gboolean getting_information = TRUE;
	
	parse_url (request->url, &host, &port, &path);
	
	gtk_signal_emit (GTK_OBJECT (request), c2_request_signals[RESOLVE]);
	if (c2_net_resolve (host, &ip))
	{
#ifdef USE_DEBUG
		g_print ("Unable to resolve hostname: %s\n",
					c2_error_get (c2_errno));
#endif
		g_free (host);
		return;
	}

	/* Now try to connect */
	gtk_signal_emit (GTK_OBJECT (request), c2_request_signals[CONNECT]);

	if (c2_net_connect (ip, port, &sock) < 0)
	{
#ifdef USE_DEBUG
		g_print ("Unable to connect: %s\n",
					c2_error_get (c2_errno));
#endif
		g_free (host);
		g_free (ip);
		return;
	}
	g_free (ip);

	gtk_signal_emit (GTK_OBJECT (request), c2_request_signals[RETRIEVE], 0);
	/* Now try to retrieve */
	if (c2_net_send (sock, "GET %s HTTP/1.0\r\n"
						   "User-Agent: Cronos II " VERSION "\r\n"
						   "Host: %s\r\n"
						   "\r\n", path, host) < 0)
	{
#ifdef USE_DEBUG
		g_print ("Unable to query the socket: %s\n", c2_error_get (c2_errno));
#endif
		g_free (host);
		close (sock);
		return;
	}
	g_free (host);
	g_free (path);

	/* Initialize request */
	request->source = NULL;
	request->request_size = -1;
	request->got_size = -1;

	/* Get the answer */
	do
	{
		if (c2_net_read (sock, &string) < 0)
		{
#ifdef USE_DEBUG
			g_print ("Unable to read from the socket: %s\n", c2_error_get (c2_errno));
#endif
			return;
		}
		if (getting_information)
		{
			if (c2_streq (string, "\r\n"))
				getting_information = FALSE;
			else if (c2_strneq (string, "Content-Length:", 15))
			{
				request->request_size = atoi (string+15);
#ifdef USE_DEBUG
				g_print ("Requested size is %d\n", request->request_size);
#endif
			}
			g_free (string);
			continue;
		}
		tmplength = strlen (string);
		length += tmplength;
		gtk_signal_emit (GTK_OBJECT (request), c2_request_signals[RETRIEVE], length);
		tmp = g_strdup_printf ("%s%s", request->source, string);
		g_free (request->source);
		g_free (string);
		request->source = tmp;
	} while (tmplength);

	gtk_signal_emit (GTK_OBJECT (request), c2_request_signals[DISCONNECT], TRUE);
	c2_net_disconnect (sock);
}

static void
c2_request_construct_ftp (C2Request *request)
{
}

static void
c2_request_construct_file (C2Request *request)
{
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
			(GtkClassInitFunc) c2_request_class_init,
			(GtkObjectInitFunc) c2_request_init,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL
		};

		c2_request_type = gtk_type_unique (gtk_object_get_type (), &c2_request_info);
	}

	return c2_request_type;
}

static void
c2_request_class_init (C2RequestClass *klass)
{
	GtkObjectClass *object_class;

	object_class = (GtkObjectClass *) klass;

	parent_class = gtk_type_class (gtk_object_get_type ());

	c2_request_signals[RESOLVE] =
		gtk_signal_new ("resolve",
					GTK_RUN_FIRST,
					object_class->type,
					GTK_SIGNAL_OFFSET (C2RequestClass, resolve),
					gtk_marshal_NONE__NONE, GTK_TYPE_NONE, 0);

	c2_request_signals[CONNECT] =
		gtk_signal_new ("connect",
					GTK_RUN_FIRST,
					object_class->type,
					GTK_SIGNAL_OFFSET (C2RequestClass, connect),
					gtk_marshal_NONE__NONE, GTK_TYPE_NONE, 0);
	
	c2_request_signals[RETRIEVE] =
		gtk_signal_new ("retrieve",
					GTK_RUN_FIRST,
					object_class->type,
					GTK_SIGNAL_OFFSET (C2RequestClass, retrieve),
					gtk_marshal_NONE__INT, GTK_TYPE_NONE, 1,
					GTK_TYPE_INT);
	
	c2_request_signals[DISCONNECT] =
		gtk_signal_new ("disconnect",
					GTK_RUN_FIRST,
					object_class->type,
					GTK_SIGNAL_OFFSET (C2RequestClass, disconnect),
					gtk_marshal_NONE__BOOL, GTK_TYPE_NONE, 1,
					GTK_TYPE_BOOL);

	gtk_object_class_add_signals (object_class, c2_request_signals, LAST_SIGNAL);

	object_class->destroy = c2_request_destroy;

	klass->resolve = NULL;
	klass->connect = NULL;
	klass->retrieve = NULL;
	klass->disconnect = NULL;
}

static void
c2_request_init (C2Request *request)
{
	request->url = NULL;
	request->request_size = 0;
	request->got_size = 0;
	request->source = NULL;
}

static void
c2_request_destroy (GtkObject *object)
{
	C2Request *request;
	
	request = C2_REQUEST (object);
	g_free (request->url);
	g_free (request->source);
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
		g_print ("Unknown protocol '%s' in %s\n",
							url, __PRETTY_FUNCTION__);
#endif
		return NULL;
	}

	return request;
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
