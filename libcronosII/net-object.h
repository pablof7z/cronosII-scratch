/*  Cronos II Mail Client /libcronosII/net-object.h
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
#ifndef __LIBCRONOSII_NET_OBJECT_H__
#define __LIBCRONOSII_NET_OBJECT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <gtk/gtk.h>

#ifdef BUILDING_C2
#	include "utils-net.h"
#else
#	include <cronosII.h>
#endif

#define C2_TYPE_NET_OBJECT						(c2_net_object_get_type ())
#define C2_NET_OBJECT(obj)						(GTK_CHECK_CAST (obj, C2_TYPE_NET_OBJECT, C2NetObject))
#define C2_NET_OBJECT_CLASS(klass)				(GTK_CHECK_CLASS_CAST (klass, C2_TYPE_NET_OBJECT, C2NetObjectClass))
#define C2_IS_NET_OBJECT(obj)					(GTK_CHECK_TYPE (obj, C2_TYPE_NET_OBJECT))
#define C2_IS_NET_OBJECT_CLASS(klass)			(GTK_CHECK_CLASS_TYPE (klass, C2_TYPE_NET_OBJECT))

typedef struct _C2NetObject C2NetObject;
typedef struct _C2NetObjectClass C2NetObjectClass;
typedef enum _C2NetObjectExchangeType C2NetObjectExchangeType;
typedef enum _C2NetObjectState C2NetObjectState;

enum _C2NetObjectExchangeType
{
	C2_NET_OBJECT_EXCHANGE_SEND,
	C2_NET_OBJECT_EXCHANGE_READ
};

enum _C2NetObjectState
{
	C2_NET_OBJECT_OFF			= 0x0000,
	C2_NET_OBJECT_RESOLVE		= 0x0001,
	C2_NET_OBJECT_CONNECT		= 0x0002,
	C2_NET_OBJECT_EXCHANGE		= 0x0004,
	C2_NET_OBJECT_DISCONNECT	= 0x0008,
	C2_NET_OBJECT_CANCEL		= 0x0010,
	C2_NET_OBJECT_ERROR			= 0x0020
};

struct _C2NetObject
{
	GtkObject object;

	gint sock;
	
	gchar *host;
	guint port;

	gint8 state;
};

struct _C2NetObjectClass
{
	GtkObjectClass parent_class;

	void (*resolve) (C2NetObject *object);
	void (*connect) (C2NetObject *object);
	void (*exchange) (C2NetObject *object, C2NetObjectExchangeType type, gint length);
	void (*disconnect) (C2NetObject *object, gboolean success);

	void (*cancel) (C2NetObject *object);
};

#define c2_net_object_is_offline(x)				((x->state & C2_NET_OBJECT_OFF) ? TRUE : FALSE)

GtkType
c2_net_object_get_type							(void);

C2NetObject *
c2_net_object_new								(const gchar *host, guint port);

void
c2_net_object_construct							(C2NetObject *nobj, const gchar *host, guint port);

void
c2_net_object_disconnect						(C2NetObject *nobj);

void
c2_net_object_disconnect_with_error				(C2NetObject *nobj);

void
c2_net_object_set_flags							(C2NetObject *nobj, gint8 flags);

void
c2_net_object_append_flags						(C2NetObject *nobj, gint8 flags);

gint
c2_net_object_run								(C2NetObject *nobj);

gint
c2_net_object_send								(C2NetObject *nobj, const gchar *fmt, ...);

gint
c2_net_object_read								(C2NetObject *nobj, gchar **string);

void
c2_net_object_cancel							(C2NetObject *nobj);

#ifdef __cplusplus
}
#endif

#endif
