/*  Cronos II Mail Client /libcronosII/net-object.c
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

#include <libcronosII/error.h>
#include <libcronosII/utils.h>

#include "net-object.h"

static void
class_init										(C2NetObjectClass *klass);

static void
init											(C2NetObject *obj);

static void
destroy											(GtkObject *object);

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

/**
 * c2_net_object_run
 * @nobj: A C2NetObject with valid information in OFF state.
 *
 * This function will resolve and connect (emitting the proper
 * signals and setting the correct state of the object) a C2NetObject.
 *
 * Return Value:
 * 0 if success or -1.
 **/
gint
c2_net_object_run (C2NetObject *nobj)
{
	gchar *ip;

	/* Check that we are not already connect */
	if (nobj->state != C2_NET_OBJECT_OFF)
	{
		g_warning ("Running a C2NetObject which is not off (%d).\n", nobj->state);
		return -1;
	}
	
	/* Fire "resolve" signal */
	nobj->state = C2_NET_OBJECT_RESOLVE;
	gtk_signal_emit (GTK_OBJECT (nobj), signals[RESOLVE]);
	
	if (c2_net_resolve (nobj->host, &ip) < 0)
	{
#ifdef USE_DEBUG
		g_warning ("Unable to resolve hostname: %s\n", c2_error_get (c2_errno));
#endif
		gtk_signal_emit (GTK_OBJECT (nobj), signals[DISCONNECT], FALSE);
		return -1;
	}

	/* Fire "connect" signal */
	nobj->state = C2_NET_OBJECT_CONNECT;
	gtk_signal_emit (GTK_OBJECT (nobj), signals[CONNECT]);

	if (c2_net_connect (ip, nobj->port, &nobj->sock) < 0)
	{
#ifdef USE_DEBUG
		g_warning ("Unable to connect: %s\n", c2_error_get (c2_errno));
#endif
		gtk_signal_emit (GTK_OBJECT (nobj), signals[DISCONNECT], FALSE);
		g_free (ip);
		return -1;
	}
	g_free (ip);

	/* Ok, the state is now connected, we are ready
	 * to send and receive data through this object! :)
	 * 			Pablo Fernández Navarro.
	 */
	return 0;
}

/**
 * c2_net_object_send
 * @nobj: The C2NetObject where we need to work.
 * @fmt: Printf compliant string format to send.
 * ...: Printf arguments.
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
c2_net_object_send (C2NetObject *nobj, const gchar *fmt, ...)
{
	va_list args;
	gchar *string;
	gint value;
	
	c2_return_val_if_fail (fmt, -1, C2EDATA);

	/* Check if the object has been canceled */
	if (nobj->state & C2_NET_OBJECT_OFF || nobj->state & C2_NET_OBJECT_CANCEL)
	{
		if (!(nobj->state & C2_NET_OBJECT_OFF))
		{
			/* This is the first c2_net_object_* call since
			 * the object was marked as canceled,
			 * close the socket, mark it as off
			 * and fire the disconnect signal.
			 */
			close (nobj->sock);
			nobj->state |= C2_NET_OBJECT_OFF;
			gtk_signal_emit (GTK_OBJECT (nobj), signals[DISCONNECT], TRUE);
		}
		return -1;
	}

	va_start (args, fmt);
	string = g_strdup_vprintf (fmt, args);
	va_end (args);
	
	if ((value = send (nobj->sock, string, strlen (string), 0)) < 0)
	{
		/* There was a problem in the sending,
		 * the socket is closed, the state will be set off | error
		 * and fire the disconnect signal.
		 */
		close (nobj->sock);
		c2_net_object_set_flags (nobj, C2_NET_OBJECT_OFF | C2_NET_OBJECT_ERROR);
		gtk_signal_emit (GTK_OBJECT (nobj), signals[DISCONNECT], FALSE);
		c2_error_set (-errno);
		g_free (string);
		return -1;
	}

	nobj->state = C2_NET_OBJECT_EXCHANGE;
	gtk_signal_emit (GTK_OBJECT (nobj), signals[EXCHANGE], C2_NET_OBJECT_EXCHANGE_SEND, strlen (string));

#ifdef USE_DEBUG
	g_print ("C: %s", string);
#endif
	g_free (string);

	return value;
}

/**
 * c2_net_object_read
 * @nobj: The C2NetObject where we need to work.
 * @string: A null pointer where result is going to
 *          be stored.
 *
 * This function reads the first 1024 bytes
 * or until it reaches a '\n' (whatever happens first)
 * from the socket.
 *
 * Return Value:
 * The number of read bytes or -1;
 **/
gint
c2_net_object_read (C2NetObject *nobj, gchar **string)
{
	gint ret;

	/* Check if the object has been canceled */
	if (nobj->state & C2_NET_OBJECT_OFF || nobj->state & C2_NET_OBJECT_CANCEL)
	{
		if (!(nobj->state & C2_NET_OBJECT_OFF))
		{
			/* This is the first c2_net_object_* call since
			 * the object was marked as canceled,
			 * close the socket, mark it as off
			 * and fire the disconnect signal.
			 */
			close (nobj->sock);
			nobj->state |= C2_NET_OBJECT_OFF;
			gtk_signal_emit (GTK_OBJECT (nobj), signals[DISCONNECT], TRUE);
		}
		return -1;
	}

	if ((ret = c2_net_read (nobj->sock, string)) < 0)
	{
		/* There was a problem in the sending,
		 * the socket is closed, the state will be set off | error
		 * and fire the disconnect signal.
		 */
		close (nobj->sock);
		c2_net_object_set_flags (nobj, C2_NET_OBJECT_OFF | C2_NET_OBJECT_ERROR);
		gtk_signal_emit (GTK_OBJECT (nobj), signals[DISCONNECT], FALSE);
		c2_error_set (-errno);
		return -1;
	}
	nobj->state = C2_NET_OBJECT_EXCHANGE;
//	gtk_signal_emit (GTK_OBJECT (nobj), signals[EXCHANGE], C2_NET_OBJECT_EXCHANGE_READ, strlen (*string));
	return ret;
}

/**
 * c2_net_object_disconnect
 **/
void
c2_net_object_disconnect (C2NetObject *nobj)
{
	if (nobj->state & C2_NET_OBJECT_OFF)
		return;

	close (nobj->sock);
	nobj->state |= C2_NET_OBJECT_OFF;
	gtk_signal_emit (GTK_OBJECT (nobj), signals[DISCONNECT], TRUE);
}

void
c2_net_object_disconnect_with_error (C2NetObject *nobj)
{
	if (nobj->state & C2_NET_OBJECT_OFF)
		return;

	close (nobj->sock);
	nobj->state |= C2_NET_OBJECT_OFF | C2_NET_OBJECT_ERROR;
	gtk_signal_emit (GTK_OBJECT (nobj), signals[DISCONNECT], TRUE);
}

void
c2_net_object_cancel (C2NetObject *nobj)
{
	if (nobj->state & C2_NET_OBJECT_OFF)
		return;

	close (nobj->sock);
	nobj->state |= C2_NET_OBJECT_OFF | C2_NET_OBJECT_CANCEL;
	gtk_signal_emit (GTK_OBJECT (nobj), signals[DISCONNECT], TRUE);
}

void
c2_net_object_set_flags (C2NetObject *nobj, gint8 flags)
{
	nobj->state = flags;
}

void
c2_net_object_append_flags (C2NetObject *nobj, gint8 flags)
{
	nobj->state |= flags;
}

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
					gtk_marshal_NONE__NONE, GTK_TYPE_NONE, 0);

	signals[CONNECT] =
		gtk_signal_new ("connect",
					GTK_RUN_FIRST,
					object_class->type,
					GTK_SIGNAL_OFFSET (C2NetObjectClass, connect),
					gtk_marshal_NONE__NONE, GTK_TYPE_NONE, 0);

	signals[EXCHANGE] =
		gtk_signal_new ("exchange",
					GTK_RUN_FIRST,
					object_class->type,
					GTK_SIGNAL_OFFSET (C2NetObjectClass, exchange),
					gtk_marshal_NONE__ENUM, GTK_TYPE_NONE, 2,
					GTK_TYPE_ENUM, GTK_TYPE_INT);

	signals[DISCONNECT] =
		gtk_signal_new ("disconnect",
					GTK_RUN_FIRST,
					object_class->type,
					GTK_SIGNAL_OFFSET (C2NetObjectClass, disconnect),
					gtk_marshal_NONE__BOOL, GTK_TYPE_NONE, 1,
					GTK_TYPE_BOOL);

	signals[CANCEL] =
		gtk_signal_new ("cancel",
					GTK_RUN_FIRST,
					object_class->type,
					GTK_SIGNAL_OFFSET (C2NetObjectClass, cancel),
					gtk_marshal_NONE__NONE, GTK_TYPE_NONE, 0);

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
	nobj->sock = 0;
	nobj->host = NULL;
	nobj->port = 0;
	nobj->state = C2_NET_OBJECT_OFF;
}

C2NetObject *
c2_net_object_new (const gchar *host, guint port)
{
	C2NetObject *nobj;
	
	c2_return_val_if_fail (host, NULL, C2EDATA);
	c2_return_val_if_fail (port > 0, NULL, C2EDATA);

	nobj = gtk_type_new (C2_TYPE_NET_OBJECT);

	c2_net_object_construct (nobj, host, port);

	return nobj;
}

void
c2_net_object_construct (C2NetObject *nobj, const gchar *host, guint port)
{
	c2_return_if_fail (host, C2EDATA);
	c2_return_if_fail (port > 0, C2EDATA);

	nobj->host = g_strdup (host);
	nobj->port = port;
}

static void
destroy (GtkObject *object)
{
	C2NetObject *nobj = C2_NET_OBJECT (object);
	
	/* Check if the object being destroyed is connected */
	if (!(nobj->state & C2_NET_OBJECT_OFF))
	{
#ifdef USE_DEBUG
		g_print ("A C2NetObject object is being destroyed and it hasn't being shutdown.\n");
#endif
		close (nobj->sock);
	}

	g_free (nobj->host);
}
