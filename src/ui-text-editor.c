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
#include <gnome.h>
#include <glade/glade.h>

#include "object-ui-data.h"

#define GLADE_FILE PKGDATADIR G_DIR_SEPARATOR_S "ui-text-editor.glade"

static void
wnd_main_draw									(void);

void
on_about_activate								(void);

static GladeXML *wnd_main_xml = NULL;

gint
main (gint argc, gchar **argv)
{
	g_thread_init (NULL);
	
	gnome_init ("UI Text Editor", "1.0", argc, argv);
	glade_gnome_init ();

	wnd_main_draw ();

	gtk_main ();

	return 0;
}

static void
wnd_main_draw (void)
{
	GtkWidget *widget;
	
	wnd_main_xml = glade_xml_new (GLADE_FILE, "wnd_main");

	glade_xml_signal_connect (wnd_main_xml, "on_about_activate", GTK_SIGNAL_FUNC (on_about_activate));

	/* Display window */
	widget = glade_xml_get_widget (wnd_main_xml, "wnd_main");
	gtk_widget_show (widget);
}

void
on_about_activate (void)
{
	GladeXML *xml = glade_xml_new (GLADE_FILE, "dlg_about");
	GtkWidget *widget = glade_xml_get_widget (xml, "dlg_about");

	gnome_dialog_run_and_close (GNOME_DIALOG (widget));
	gtk_object_destroy (GTK_OBJECT (xml));
}
