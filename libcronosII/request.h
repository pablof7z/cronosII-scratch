/*  Cronos II - The GNOME mail client
 *  Copyright (C) 2000-2001 Pablo Fernández López
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
 * 		* Pablo Fernández López
 * Code of this file by:
 * 		* Pablo Fernández López
 */
#ifndef __LIBCRONOSII_REQUEST_H__
#define __LIBCRONOSII_REQUEST_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <gtk/gtk.h>

#ifdef BUILDING_C2
#	include "net-object.h"
#else
#	include <cronosII.h>
#endif

#define C2_TYPE_REQUEST							(c2_request_get_type ())
#define C2_REQUEST(obj)							(GTK_CHECK_CAST (obj, C2_TYPE_REQUEST, C2Request))
#define C2_REQUEST_CLASS(klass)					(GTK_CHECK_CLASS (klass, C2_TYPE_REQUEST, C2Request))
#define C2_IS_REQUEST(obj)						(GTK_CHECK_TYPE (obj, C2_TYPE_REQUEST))
#define C2_IS_REQUEST_CLASS(klass)				(GTK_CHECK_CLASS_TYPE (klass, C2_TYPE_REQUEST))

typedef struct _C2Request C2Request;
typedef struct _C2RequestClass C2RequestClass;
typedef struct _C2Proxy C2Proxy;
typedef enum _C2ProxyType C2ProxyType;
typedef enum _C2RequestProtocol C2RequestProtocol;

enum _C2ProxyType
{
	C2_PROXY_HTTP,
	C2_PROXY_FTP
};

struct _C2Proxy
{
	gchar *host;
	gint port;
	gchar *ignore;
};

enum _C2RequestProtocol
{
	C2_REQUEST_HTTP,
	C2_REQUEST_FILE,
	C2_REQUEST_FTP
};

struct _C2Request
{
	C2NetObject object;

	gchar *url;
	C2RequestProtocol protocol;
	size_t request_size, got_size;

	gchar *source;
};

struct _C2RequestClass
{
	C2NetObjectClass parent_class;
};

GtkType
c2_request_get_type								(void);

C2Request *
c2_request_new									(const gchar *url);

void
c2_request_run									(C2Request *request);

void
c2_request_set_proxy							(C2ProxyType type, const gchar *addr, guint port,
												 const gchar *ignore);

void
c2_request_get_proxy							(C2ProxyType type, gchar **addr, gint *port, gchar **ignore);

const gchar *
c2_request_get_source							(C2Request *request);

#ifdef __cplusplus
}
#endif

#endif
