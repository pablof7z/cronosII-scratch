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
#ifndef __LIBCRONOSII_UTILS_NET_H__
#define __LIBCRONOSII_UTILS_NET_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdarg.h>

#ifndef MAXHOSTNAMELEN
#	define MAXHOSTNAMELEN 64
#endif

typedef struct _C2Cache C2Cache;

struct _C2Cache
{
	gchar *hostname;
	gchar *ip;

	C2Cache *next;
};

gint
c2_net_resolve								(const gchar *hostname, gchar **ip);

gint
c2_net_connect								(const gchar *ip, guint port, gint sock);

gint
c2_net_printf								(guint sock, const gchar *fmt, ...);

gint
c2_net_read									(guint sock, gchar **string);

void
c2_net_disconnect							(guint sock);

gchar *
c2_net_get_local_hostname					(guint sock);
	
#ifdef __cplusplus
}
#endif

#endif
