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
#include <libcronosII/error.h>

#include "widget-dialog.h"

static void
class_init									(C2DialogClass *klass);

static void
init										(C2Dialog *window);

static void
destroy										(GtkObject *object);

enum
{
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static GtkDialogClass *parent_class = NULL;

GtkType
c2_dialog_get_type (void)
{
	static GtkType type = 0;

	if (!type)
	{
		static GtkTypeInfo info =
		{
			"C2Dialog",
			sizeof (C2Dialog),
			sizeof (C2DialogClass),
			(GtkClassInitFunc) class_init,
			(GtkObjectInitFunc) init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL
		};

		type = gtk_type_unique (gnome_dialog_get_type (), &info);
	}

	return type;
}

static void
class_init (C2DialogClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *) klass;
	
	parent_class = gtk_type_class (gnome_dialog_get_type ());

	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);

	object_class->destroy = destroy;
}

static void
init (C2Dialog *dialog)
{
	dialog->application = NULL;
	dialog->xml = NULL;
}

static void
destroy (GtkObject *object)
{
	C2Dialog *dialog;

	dialog = C2_DIALOG (object);

	c2_application_window_remove (dialog->application, GTK_WINDOW (dialog));

	if (dialog->xml)
		gtk_object_unref (GTK_OBJECT (dialog->xml));
}

GtkWidget *
c2_dialog_new (C2Application *application, const gchar *title, ...)
{
	C2Dialog *dialog;
	va_list args;

	dialog = gtk_type_new (c2_dialog_get_type ());
	va_start (args, title);
	gnome_dialog_construct (GNOME_DIALOG (dialog), title, args);
	va_end (args);
	dialog->application = application;

	c2_application_window_add (application, GTK_WINDOW (dialog));

	return GTK_WIDGET (dialog);
}

void
c2_dialog_construct (C2Dialog *dialog, C2Application *application, const gchar *title, const gchar **buttons)
{
	dialog->application = application;

	gnome_dialog_constructv (GNOME_DIALOG (dialog), title, buttons);
}
