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

#include "error.h"
#include "utils.h"
#include "utils-net.h"

/* TODO
 * 20011208 (/me is a little bit drunk still) There's no timeout stuff! -Pablo
 */

static C2Cache *
c2_net_get_cache							(const gchar *hostname);

static void
c2_net_set_cache							(C2Cache *cache);

static C2Cache *
c2_net_get_internal_cache					(void);

static void
c2_net_set_internal_cache					(C2Cache *cache);

/**
 * c2_net_resolve
 * @hostname: Hostname to resolve (i.e. "google.com")
 * @ip: A null pointer where the result will be stored.
 *
 * This function resolves a net hostname into
 * an IP address.
 *
 * Return Value:
 * This function returns 0 in success or -1.
 **/
gint
c2_net_resolve (const gchar *hostname, gchar **ip)
{
	struct hostent *host;
	struct sockaddr_in sin;
	C2Cache *cache;
	
	c2_return_val_if_fail (hostname, -1, C2EDATA);

	/* Check if it has already been cached */
	if ((cache = c2_net_get_cache (hostname)))
	{
		*ip = g_strdup (cache->ip);
		return 0;
	}

	if (!(host = gethostbyname (hostname)))
	{
		c2_error_set (C2ERSLV);
		return -1;
	}

	memcpy (&sin.sin_addr, host->h_addr, host->h_length);
	cache = g_new0 (C2Cache, 1);
	cache->hostname = g_strdup (hostname);
	cache->ip = g_strdup (inet_ntoa (sin.sin_addr));
	*ip = g_strdup (cache->ip);

	c2_net_set_cache (cache);

	return 0;
}

/**
 * c2_net_connect
 * @ip: IP number where to connect.
 * @port: Port where to connect.
 * @sock: A created socket.
 *
 * This function will create a connection to the host
 * with IP @ip.
 *
 * Return Value:
 * 0 if success or -1.
 **/
gint
c2_net_connect (const gchar *ip, guint port, gint sock)
{
	struct sockaddr_in server;
	
	c2_return_val_if_fail (ip, -1, C2EDATA);

	/* Setup the information */
	server.sin_family = AF_INET;
	server.sin_port = htons (port);
	server.sin_addr.s_addr = inet_addr (ip);

	if (connect (sock, (struct sockaddr *)&server, sizeof (server)) < 0)
	{
		c2_error_set (-errno);
		return -1;
	}

	return 0;
}

/**
 * c2_net_send
 * @sock: Socket descriptor.
 * @fmt: Printf complaiment string format to send.
 * ...: Printf arguments.
 *
 * This function will write a string to the
 * socket.
 * You can use this function pretty much like
 * fprintf.
 *
 * Return Value:
 * send(2);
 **/
gint
c2_net_send (guint sock, const gchar *fmt, ...)
{
	va_list args;
	gchar *string;
	gint value;
	
	c2_return_val_if_fail (fmt, -1, C2EDATA);

	va_start (args, fmt);
	string = g_strdup_vprintf (fmt, args);
	va_end (args);
	
	if ((value = send (sock, string, strlen (string), 0)) < 0)
		c2_error_set (-errno);
	g_free (string);
	return value;
}

/**
 * c2_net_read
 * @sock: Socket descriptor.
 * @string: A null pointer where result is going to
 *          be stored.
 *
 * This function reads the first 1024 bytes
 * or until it reaches a '\n' (whatever happens first)
 * from the socket.
 *
 * Return Value:
 * The number of bytes read or -1 on error;
 **/
gint
c2_net_read (guint sock, gchar **string)
{
	gchar tmpstring[1024];
	gint bytes = 0, i;
	gchar c[1];
	
	for (i = 0; i < 1023; i++)
	{
		if ((bytes += read (sock, c, 1)) < 0)
		{
			c2_error_set (-errno);
			return -1;
		}
		if (!bytes)
		{
			break;
		}

		tmpstring[i] = *c;
		if (*c == '\n')
		{
			tmpstring[i+1] = '\0';
			break;
		}
	}
	tmpstring[1023] = '\0';
	
	*string = g_strdup (tmpstring);
	return bytes;
}

/**
 * c2_net_disconnect
 * @sock: Socket descriptor.
 *
 * This function will close a socket.
 **/
void
c2_net_disconnect (guint sock)
{
	close (sock);
	sock = 0; /* so you can test if a socket is connected
						 * by doing if(sock==0) */
}

/**
 * c2_net_get_local_hostname
 * @sock: Socket descriptor
 * 
 * This function is used to get the computer hostname.
 * 
 * This function will return the machine's Internet hostname,
 * or if not resolvable by DNS, it will return the machines
 * local hostname. If all else fails, it will return NULL.
 * If the return is not NULL, it should be freed when no
 * longer needed/
 **/
gchar *
c2_net_get_local_hostname (guint sock)
{
	struct sockaddr_in localaddr;
	struct hostent *host = NULL;
	gchar *localhostname = NULL;
	guint addrlen = sizeof (localaddr);
	
	getsockname (sock, (struct sockaddr*) &localaddr, &addrlen);
	host = gethostbyaddr((gchar *)&localaddr.sin_addr, sizeof (localaddr.sin_addr), AF_INET);
	if (host && host->h_name)
		localhostname = g_strdup(host->h_name);
	else
	{
		localhostname = g_new0(gchar, 32);
		if (gethostname (localhostname, 31) < 0)
			localhostname = NULL;
	}
	
	return localhostname;
}

static C2Cache *
c2_net_get_cache (const gchar *hostname)
{
	C2Cache *s;

	c2_return_val_if_fail (hostname, NULL, C2EDATA);
	
	for (s = c2_net_get_internal_cache (); s != NULL; s = s->next)
	{
		if (c2_streq (s->hostname, hostname))
			return s;
	}

	return NULL;
}

static void
c2_net_set_cache (C2Cache *cache)
{
	C2Cache *s, *last;
	
	c2_return_if_fail (cache, C2EDATA);

	/* Check if this is a new insertion or an update */
	for (s = last = c2_net_get_internal_cache (); s != NULL; s = s->next)
	{
		if (c2_streq (s->hostname, cache->hostname))
			break;
		last = s;
	}

	if (s)
	{
		/* Replace the node */
		g_free (s->hostname);
		g_free (s->ip);
		last->next = cache;
		cache->next = s->next;
		g_free (s);
	} else
	{
		/* Append it */
		cache->next = NULL;
		if (last)
			last->next = cache;
		else
			c2_net_set_internal_cache (cache);
	}
}

static C2Cache *internal_cache = NULL;

static C2Cache *
c2_net_get_internal_cache (void)
{
	return internal_cache;
}

static void
c2_net_set_internal_cache (C2Cache *cache)
{
	internal_cache = cache;
}
