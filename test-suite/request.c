/*  Cronos II - A GNOME mail client
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
#include <stdio.h>

#include <libcronosII/request.h>
#include <libcronosII/utils.h>

static void
on_request_resolve								(C2Request *request);

static void
on_request_connect								(C2Request *request);

static void
on_request_retrieve								(C2Request *request, C2NetObjectExchangeType type, gint length);

static void
on_request_disconnect							(C2Request *request, gboolean success);

gint
main (gint argc, gchar **argv)
{
	C2Request *request;
	
	gtk_init (&argc, &argv);
	if (argc < 2)
	{
		g_print ("Usage: %s URL [Proxy_addr:Proxy_port]\n", argv[0]);
		return 1;
	}

	request = c2_request_new (argv[1]);
	gtk_signal_connect (GTK_OBJECT (request), "resolve",
								GTK_SIGNAL_FUNC (on_request_resolve), NULL);
	gtk_signal_connect (GTK_OBJECT (request), "connect",
								GTK_SIGNAL_FUNC (on_request_connect), NULL);
	gtk_signal_connect (GTK_OBJECT (request), "exchange",
								GTK_SIGNAL_FUNC (on_request_retrieve), NULL);
	gtk_signal_connect (GTK_OBJECT (request), "disconnect",
								GTK_SIGNAL_FUNC (on_request_disconnect), NULL);
	g_print ("Resolving... ");
	c2_request_run (request);
	
	gtk_main ();

	return 0;
}

static void
on_request_resolve (C2Request *request)
{
}

static void
on_request_connect (C2Request *request)
{
}

static void
on_request_retrieve (C2Request *request, C2NetObjectExchangeType type, gint length)
{
}

static void
on_request_disconnect (C2Request *request, gboolean success)
{
	C2_REQUEST (GTK_OBJECT (request));
	fwrite (c2_request_get_source (request), request->got_size, sizeof (gchar), stdout);
	exit (0);
}
