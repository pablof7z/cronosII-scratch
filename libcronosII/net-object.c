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
/**
 * Maintainer(s) of this file:
 * 		* Pablo Fernández Navarro
 * Code of this file by:
 * 		* Pablo Fernández Navarro
 * 		* Bosko Blagojevic
 */
#include <gtk/gtk.h>

#include <libcronosII/error.h>
#include <libcronosII/utils.h>

#include "net-object.h"

#define AVAILABLE_BUFFER	-1

static void
class_init									(C2NetObjectClass *klass);

static void
init										(C2NetObject *obj);

static void
destroy										(GtkObject *object);

static void
nobj_disconnect								(C2NetObject *nobj, C2NetObjectByte *byte);

enum
{
	RESOLVE,
	CONNECT,
	EXCHANGE,
	DISCONNECT,
	CANCEL,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static GtkObjectClass *parent_class = NULL;

GtkType
c2_net_object_get_type (void)
{
	static GtkType c2_net_object_type = 0;

	if (!c2_net_object_type)
	{
		GtkTypeInfo c2_net_object_info =
		{
			"C2NetObject",
			sizeof (C2NetObject),
			sizeof (C2NetObjectClass),
			(GtkClassInitFunc) class_init,
			(GtkObjectInitFunc) init,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL
		};

		c2_net_object_type = gtk_type_unique (gtk_object_get_type (), &c2_net_object_info);
	}

	return c2_net_object_type;
}

static void
class_init (C2NetObjectClass *klass)
{
	GtkObjectClass *object_class;

	object_class = (GtkObjectClass *) klass;

	parent_class = gtk_type_class (gtk_object_get_type ());

	signals[RESOLVE] =
		gtk_signal_new ("resolve",
					GTK_RUN_FIRST,
					object_class->type,
					GTK_SIGNAL_OFFSET (C2NetObjectClass, resolve),
					gtk_marshal_NONE__POINTER, GTK_TYPE_NONE, 1,
					GTK_TYPE_POINTER);
	signals[CONNECT] =
		gtk_signal_new ("connect",
					GTK_RUN_FIRST,
					object_class->type,
					GTK_SIGNAL_OFFSET (C2NetObjectClass, connect),
					gtk_marshal_NONE__POINTER, GTK_TYPE_NONE, 1,
					GTK_TYPE_POINTER);
	signals[EXCHANGE] =
		gtk_signal_new ("exchange",
					GTK_RUN_FIRST,
					object_class->type,
					GTK_SIGNAL_OFFSET (C2NetObjectClass, exchange),
					gtk_marshal_NONE__INT_INT_POINTER, GTK_TYPE_NONE, 3,
					GTK_TYPE_ENUM, GTK_TYPE_INT, GTK_TYPE_POINTER);
	signals[DISCONNECT] =
		gtk_signal_new ("disconnect",
					GTK_RUN_FIRST,
					object_class->type,
					GTK_SIGNAL_OFFSET (C2NetObjectClass, disconnect),
					gtk_marshal_NONE__POINTER_INT, GTK_TYPE_NONE, 2,
					GTK_TYPE_POINTER, GTK_TYPE_BOOL);
	signals[CANCEL] =
		gtk_signal_new ("cancel",
					GTK_RUN_FIRST,
					object_class->type,
					GTK_SIGNAL_OFFSET (C2NetObjectClass, cancel),
					gtk_marshal_NONE__POINTER, GTK_TYPE_NONE, 1,
					GTK_TYPE_POINTER);
	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);

	object_class->destroy = destroy;

	klass->resolve = NULL;
	klass->connect = NULL;
	klass->exchange = NULL;
	klass->disconnect = NULL;
	klass->cancel = NULL;
}

static void
init (C2NetObject *nobj)
{
	nobj->bytes = NULL;
	nobj->host = NULL;
	nobj->port = 0;
	nobj->max = 1;
	
	c2_mutex_init(&nobj->lock);
}

static void
destroy (GtkObject *object)
{
	C2NetObject *nobj = C2_NET_OBJECT (object);
	C2NetObjectByte *byte;
	GList *l;
	
	for (l = nobj->bytes; l; l = l->next)
	{
		byte = (C2NetObjectByte*) l->data;
		
		/* Check if the object being destroyed is connected */
		if (!(byte->state & C2_NET_OBJECT_OFF))
		{
#ifdef USE_DEBUG
			g_print ("A C2NetObject object is being destroyed and it hasn't being shutdown.\n");
#endif
			close (byte->sock);
		}
		g_free (byte);
	}

	g_list_free (nobj->bytes);
	g_free (nobj->host);
	c2_mutex_destroy(&nobj->lock);
}

C2NetObject *
c2_net_object_new (const gchar *host, guint port, gboolean ssl)
{
	C2NetObject *nobj;
	
	c2_return_val_if_fail (host, NULL, C2EDATA);
	c2_return_val_if_fail (port > 0, NULL, C2EDATA);

	nobj = gtk_type_new (C2_TYPE_NET_OBJECT);

	c2_net_object_construct (nobj, host, port, ssl);

	return nobj;
}

void
c2_net_object_construct (C2NetObject *nobj, const gchar *host, guint port, gboolean ssl)
{
	c2_return_if_fail (host, C2EDATA);
	c2_return_if_fail (port > 0, C2EDATA);

	nobj->host = g_strdup (host);
	nobj->port = port;
	nobj->ssl = ssl ? 1 : 0;

#ifndef USE_SSL
	if (ssl)
		g_warning ("SSL has not been compiled.\n");
#endif
}

/**
 * c2_net_object_run
 * @nobj: A C2NetObject with valid information in OFF state.
 *
 * This function will resolve and connect (emitting the proper
 * signals and setting the correct state of the object) a C2NetObject.
 *
 * Return Value:
 * C2NetObjectBytes or %NULL.
 **/
C2NetObjectByte *
c2_net_object_run (C2NetObject *nobj)
{
	C2NetObjectByte *byte;
	gchar *ip;

	/* Check that the Net Object supports another connection */
	if (nobj->max)
	{
		if (g_list_length (nobj->bytes) >= nobj->max)
		{
			c2_error_object_set (GTK_OBJECT (nobj), C2NOBJMAX);
			return NULL;
		}
	}

	/* Create the byte */
	byte = g_new0 (C2NetObjectByte, 1);

	/* Create the socket */
	if ((byte->sock = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		c2_error_object_set (GTK_OBJECT (nobj), -errno);
		g_free (byte);
		return NULL;
	}
	
	c2_mutex_lock(&nobj->lock);
	nobj->bytes = g_list_append (nobj->bytes, (gpointer) byte);
	c2_mutex_unlock(&nobj->lock);
	
	/* Fire "resolve" signal */
	byte->state = C2_NET_OBJECT_RESOLVE;
	gtk_signal_emit (GTK_OBJECT (nobj), signals[RESOLVE], byte);

	if (c2_net_resolve (nobj->host, &ip) < 0)
	{
#ifdef USE_DEBUG
		g_warning ("Unable to resolve hostname: %s\n", c2_error_get ());
#endif
		c2_error_object_set (GTK_OBJECT (nobj), -errno);
		gtk_signal_emit (GTK_OBJECT (nobj), signals[DISCONNECT], byte, FALSE);
		return NULL;
	}

	/* Fire "connect" signal */
	byte->state = C2_NET_OBJECT_CONNECT;
	gtk_signal_emit (GTK_OBJECT (nobj), signals[CONNECT], byte);

	if (c2_net_connect (ip, nobj->port, byte->sock) < 0)
	{
#ifdef USE_DEBUG
		g_warning ("Unable to connect: %s\n", c2_error_get ());
#endif
		c2_error_object_set (GTK_OBJECT (nobj), -errno);
		gtk_signal_emit (GTK_OBJECT (nobj), signals[DISCONNECT], byte, FALSE);
		return NULL;
	}
	g_free (ip);

	/* Ok, the state is now connected, we are ready
	 * to send and receive data through this object! :)
	 * 			Pablo Fernández Navarro.
	 */
	
	return byte;
}

static void
nobj_disconnect (C2NetObject *nobj, C2NetObjectByte *byte)
{
	close (byte->sock);
}

/**
 * c2_net_object_send
 * @nobj: The C2NetObject where we need to work.
 * @ident: Ident of the connection
 *         (return value of c2_net_object_run).
 * @fmt: Printf compliant string format to send.
 * ...: Printf arguments and an optional C2NetObjectByte
 *      (optional only if nobj->max == 1).
 *
 * This function will write an string to the
 * C2NetObject.
 * You can use this function pretty much like
 * fprintf.
 *
 * Return Value:
 * bytes sent or -1;
 **/
gint
c2_net_object_send (C2NetObject *nobj, C2NetObjectByte *byte, const gchar *fmt, ...)
{
	va_list args;
	gint value;

	if (nobj->max == 1)
	{
		GList *l;
		l = g_list_nth (nobj->bytes, 0);
		byte = (C2NetObjectByte*) l->data;
	} else if (!byte)
		printf ("Are u stupid!? You haven't specified the byte "
				"and your max is not 1. Gz, I don't wanna work with ya "
				"if you are going to keep doing this fucking mistakes here man.\n"
				"Start coding nicely or I'll punch you! Oh, BTW, "
				"so you can debug me, line is %d, file is %s and function is %s.\n"
				"\t\tLater!\n", __LINE__, __FILE__, __PRETTY_FUNCTION__);
	
	/* Check if the object has been canceled */
	if (byte->state & C2_NET_OBJECT_OFF || byte->state & C2_NET_OBJECT_CANCEL)
	{
		if (!(byte->state & C2_NET_OBJECT_OFF))
		{
			/* This is the first c2_net_object_* call since
			 * the object was marked as canceled,
			 * close the socket, mark it as off
			 * and fire the disconnect signal.
			 */
			byte->state |= C2_NET_OBJECT_OFF;
			gtk_signal_emit (GTK_OBJECT (nobj), signals[DISCONNECT], byte, TRUE);
		}
		return -1;
	}

	va_start (args, fmt);
	if ((value = c2_net_sendv (byte->sock, fmt, args)) < 0)
	{
		/* There was a problem in the sending,
		 * the socket is closed, the state will be set off | error
		 * and fire the disconnect signal.
		 */
		perror ("send()");
		close (byte->sock);
		c2_net_object_set_state (nobj, C2_NET_OBJECT_OFF | C2_NET_OBJECT_ERROR, byte);
		c2_error_object_set (GTK_OBJECT (nobj), -errno);
		gtk_signal_emit (GTK_OBJECT (nobj), signals[DISCONNECT], byte, FALSE);
		va_end (args);
		return -1;
	}
	va_end (args);

	byte->state = C2_NET_OBJECT_EXCHANGE;
	gtk_signal_emit (GTK_OBJECT (nobj), signals[EXCHANGE], C2_NET_OBJECT_EXCHANGE_SEND, value, byte);

	return value;
}

/**
 * c2_net_object_read
 * @nobj: The C2NetObject where we need to work.
 * @string: A null pointer where result is going to
 *          be stored.
 * @...: C2NetObjectByte, might be omitted if the net object
 * 		 does not allow more than one connection at the same time.
 *
 * This function reads the first 1024 bytes
 * or until it reaches a '\n' (whatever happens first)
 * from the socket.
 *
 * Return Value:
 * The number of read bytes or -1;
 **/
gint
c2_net_object_read (C2NetObject *nobj, gchar **string, ...)
{
	gint ret;
	C2NetObjectByte *byte;

	/* Get the byte */
	if (nobj->max == 1)
	{
		GList *l;
		l = g_list_nth (nobj->bytes, 0);
		byte = (C2NetObjectByte*) l->data;
	} else
	{
		va_list args;
		va_start (args, string);
		byte = va_arg (args, C2NetObjectByte*);
		va_end (args);
	}

	/* Check if the object has been canceled */
	if (byte->state & C2_NET_OBJECT_OFF || byte->state & C2_NET_OBJECT_CANCEL)
	{
		if (!(byte->state & C2_NET_OBJECT_OFF))
		{
			/* This is the first c2_net_object_* call since
			 * the object was marked as canceled,
			 * close the socket, mark it as off
			 * and fire the disconnect signal.
			 */
			byte->state |= C2_NET_OBJECT_OFF;
			gtk_signal_emit (GTK_OBJECT (nobj), signals[DISCONNECT], byte, TRUE);
		}
		return -1;
	}

	if ((ret = c2_net_read (byte->sock, string)) < 0)
	{
		/* There was a problem in the sending,
		 * the socket is closed, the state will be set off | error
		 * and fire the disconnect signal.
		 */
		close (byte->sock);
		c2_net_object_set_state (nobj, C2_NET_OBJECT_OFF | C2_NET_OBJECT_ERROR, byte);
		gtk_signal_emit (GTK_OBJECT (nobj), signals[DISCONNECT], FALSE);
		c2_error_object_set (GTK_OBJECT (nobj), -errno);
		return -1;
	}
	byte->state = C2_NET_OBJECT_EXCHANGE;
	gtk_signal_emit (GTK_OBJECT (nobj), signals[EXCHANGE],
					 C2_NET_OBJECT_EXCHANGE_READ, strlen (*string), byte);
	return ret;
}

/**
 * c2_net_object_disconnect
 **/
void
c2_net_object_disconnect (C2NetObject *nobj, ...)
{
	C2NetObjectByte *byte;

	/* Get the byte */
	if (nobj->max == 1)
	{
		GList *l;
		l = g_list_nth (nobj->bytes, 0);
		byte = (C2NetObjectByte*) l->data;
	} else
	{
		va_list args;
		va_start (args, nobj);
		byte = va_arg (args, C2NetObjectByte*);
		va_end (args);
	}
	
	if (byte->state & C2_NET_OBJECT_OFF)
		return;

	close (byte->sock);
	byte->state |= C2_NET_OBJECT_OFF;

	gtk_signal_emit (GTK_OBJECT (nobj), signals[DISCONNECT], byte, TRUE);
}

void
c2_net_object_disconnect_with_error (C2NetObject *nobj, ...)
{
	C2NetObjectByte *byte;

	/* Get the byte */
	if (nobj->max == 1)
	{
		GList *l;
		l = g_list_nth (nobj->bytes, 0);
		byte = (C2NetObjectByte*) l->data;
	} else
	{
		va_list args;
		va_start (args, nobj);
		byte = va_arg (args, C2NetObjectByte*);
		va_end (args);
	}
	
	if (byte->state & C2_NET_OBJECT_OFF)
		return;

	close (byte->sock);
	byte->state |= C2_NET_OBJECT_OFF | C2_NET_OBJECT_ERROR;
	
	gtk_signal_emit (GTK_OBJECT (nobj), signals[DISCONNECT], byte, FALSE);
}

void
c2_net_object_cancel (C2NetObject *nobj, ...)
{
	C2NetObjectByte *byte;

	/* Get the byte */
	if (nobj->max == 1)
	{
		GList *l;
		l = g_list_nth (nobj->bytes, 0);
		byte = (C2NetObjectByte*) l->data;
	} else
	{
		va_list args;
		va_start (args, nobj);
		byte = va_arg (args, C2NetObjectByte*);
		va_end (args);
	}
	
	if (byte->state & C2_NET_OBJECT_OFF)
		return;

	close (byte->sock);
	byte->state |= C2_NET_OBJECT_OFF | C2_NET_OBJECT_CANCEL;
	gtk_signal_emit (GTK_OBJECT (nobj), signals[DISCONNECT], byte, TRUE);
}

/**
 * c2_net_object_destroy_byte
 *
 * After you are done using the byte, USE this function
 * for removing it.
 * NOTE that if you don't call it the byte will keep living
 * in the net object, so you will be one step closer to
 * the maximum buffer of bytes.
 **/
void
c2_net_object_destroy_byte (C2NetObject *nobj, ...)
{
	C2NetObjectByte *byte;
	GList *link;

	c2_return_if_fail_obj (C2_IS_NET_OBJECT (nobj) || g_list_length (nobj->bytes) > 1,
							C2EDATA, GTK_OBJECT (nobj));

	/* Get the byte */
	if (nobj->max == 1)
	{
		link = g_list_nth (nobj->bytes, 0);
		byte = (C2NetObjectByte*) link->data;
	} else
	{
		va_list args;
		va_start (args, nobj);
		byte = va_arg (args, C2NetObjectByte*);
		va_end (args);
		link = g_list_find (nobj->bytes, (gpointer) byte);
	}
	
	if (!byte || !link)
		return;
	
	nobj->bytes = g_list_remove_link (nobj->bytes, link);
	g_free (byte);
}

void
c2_net_object_set_state (C2NetObject *nobj, gint8 state, ...)
{
	C2NetObjectByte *byte;

	/* Get the byte */
	if (nobj->max == 1)
	{
		GList *l;
		l = g_list_nth (nobj->bytes, 0);
		byte = (C2NetObjectByte*) l->data;
	} else
	{
		va_list args;
		va_start (args, state);
		byte = va_arg (args, C2NetObjectByte*);
		va_end (args);
	}
	
	byte->state = state;
}

void
c2_net_object_append_state (C2NetObject *nobj, gint8 state, ...)
{
	C2NetObjectByte *byte;

	/* Get the byte */
	if (nobj->max == 1)
	{
		GList *l;
		l = g_list_nth (nobj->bytes, 0);
		byte = (C2NetObjectByte*) l->data;
	} else
	{
		va_list args;
		va_start (args, state);
		byte = va_arg (args, C2NetObjectByte*);
		va_end (args);
	}
	
	byte->state |= state;
}

/**
 * c2_net_object_set_maximum
 * @nobj: C2NetObject where to work.
 * @max: Maximum number of connections to provide.
 *       (0 = unlimited)
 * 
 * C2NetObject provides just one connection at a time,
 * you can not lunch a connect order before the existent
 * connection is shutted down.
 * This function will prepare the C2NetOjbect to allow
 * several connections at the same time.
 **/
void
c2_net_object_set_maximum (C2NetObject *nobj, guint max)
{
	nobj->max = max;
}

gboolean
c2_net_object_is_offline (C2NetObject *nobj, ...)
{
	C2NetObjectByte *byte;

	/* Get the byte */
	if (nobj->max == 1)
	{
		GList *l;
		l = g_list_nth (nobj->bytes, 0);
		byte = (C2NetObjectByte*) l->data;
	} else
	{
		va_list args;
		va_start (args, nobj);
		byte = va_arg (args, C2NetObjectByte*);
		va_end (args);
	}

	if (byte->state & C2_NET_OBJECT_OFF || !byte)
		return TRUE;

	return FALSE;
}
