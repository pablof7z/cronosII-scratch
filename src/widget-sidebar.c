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
#include "widget-sidebar.h"

static void
class_init									(C2SidebarClass *klass);

static void
init										(C2Sidebar *sidebar);

static void
destroy										(GtkObject *obj);

enum
{
	SELECTION,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static GtkContainerClass *parent_class = NULL;

GtkType
c2_sidebar_get_type (void)
{
	static GtkType type = 0;

	if (!type)
	{
		static GtkTypeInfo info =
		{
			"C2Sidebar",
			sizeof (C2Sidebar),
			sizeof (C2SidebarClass),
			(GtkClassInitFunc) class_init,
			(GtkObjectInitFunc) init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL
		};

		type = gtk_type_unique (gtk_container_get_type (), &info);
	}

	return type;
}

static void
class_init (C2SidebarClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *) klass;

	parent_class = gtk_type_class (gtk_container_get_type ());

	signals[SELECTION] =
		gtk_signal_new ("selection",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2SidebarClass, selection),
						gtk_marshal_NONE__POINTER_POINTER, GTK_TYPE_NONE, 2,
						GTK_TYPE_STRING, GTK_TYPE_STRING);
	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);

	klass->selection = NULL;

	object_class->destroy = destroy;
}

static void
init (C2Sidebar *sidebar)
{
	sidebar->vbox = gtk_vbox_new (FALSE, 0);
	sidebar->list = NULL;
}

static void
destroy (GtkObject *obj)
{
}

GtkWidget *
c2_sidebar_new (void)
{
	C2Sidebar *sidebar;

	sidebar = gtk_type_new (c2_sidebar_get_type ());

	return GTK_WIDGET (sidebar);
}

void
c2_sidebar_add (C2Sidebar *sidebar, gchar *section, gchar *subsection, const gchar *pixmap,
				GtkWidget *widget)
{
}
