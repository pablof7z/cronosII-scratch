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
#include <gnome.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include <libcronosII/utils.h>

#include "widget-sidebar.h"

static void
class_init									(C2SidebarClass *klass);

static void
init										(C2Sidebar *sidebar);

static void
on_section_button_clicked					(GtkWidget *button, C2Sidebar *sidebar);

enum
{
	SECTION_SELECTED,
	SUBSECTION_SELECTED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static GtkVBoxClass *parent_class = NULL;

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

		type = gtk_type_unique (gtk_vbox_get_type (), &info);
	}

	return type;
}

static void
class_init (C2SidebarClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *) klass;
	
	parent_class = gtk_type_class (gtk_vbox_get_type ());

	signals[SECTION_SELECTED] =
		gtk_signal_new ("section_selected",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2SidebarClass, section_selected),
						gtk_marshal_NONE__POINTER, GTK_TYPE_NONE, 1,
						GTK_TYPE_STRING);
	signals[SUBSECTION_SELECTED] =
		gtk_signal_new ("subsection_selected",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2SidebarClass, subsection_selected),
						gtk_marshal_NONE__POINTER_POINTER, GTK_TYPE_NONE, 2,
						GTK_TYPE_STRING, GTK_TYPE_STRING);
	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);

	klass->section_selected = NULL;
	klass->subsection_selected = NULL;
}

static void
init (C2Sidebar *sidebar)
{
	sidebar->buttons_type = C2_SIDEBAR_BUTTON_TEXT_UNDER_ICON;
	sidebar->tooltips = 1;
}

GtkWidget *
c2_sidebar_new (void)
{
	C2Sidebar *sidebar;

	sidebar = gtk_type_new (c2_sidebar_get_type ());
	GTK_BOX (sidebar)->spacing = 0;
	GTK_BOX (sidebar)->homogeneous = FALSE;

	return GTK_WIDGET (sidebar);
}

void
c2_sidebar_set_contents (C2Sidebar *sidebar, C2SidebarSection *list)
{
	C2SidebarSection *l;
	GtkTooltips *tooltips = sidebar->tooltips ? gtk_tooltips_new () : NULL;

	sidebar->section = list;

	for (l = list; l->name; l++)
	{
		C2SidebarSubSection *sl;
		GtkWidget *button, *vbox, *viewport;
		GSList *bgroup = NULL;

		button = gtk_button_new_with_label (l->name);
		gtk_box_pack_start (GTK_BOX (sidebar), button, FALSE, TRUE, 0);
		gtk_widget_show (button);
		l->button = button;

		viewport = gtk_scrolled_window_new (NULL, NULL);
		gtk_box_pack_start (GTK_BOX (sidebar), viewport, TRUE, TRUE, 0);
		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (viewport), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
		gtk_widget_hide (viewport);

		vbox = gtk_vbox_new (FALSE, 10);
		gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (viewport), vbox);
//		gtk_container_add (GTK_CONTAINER (viewport), vbox);
//		gtk_box_pack_start (GTK_BOX (sidebar), vbox, TRUE, TRUE, 0);
		gtk_widget_show (vbox);
		
		gtk_object_set_data (GTK_OBJECT (button), "panel", viewport);
		gtk_signal_connect (GTK_OBJECT (button), "clicked",
							GTK_SIGNAL_FUNC (on_section_button_clicked), sidebar);
		
		button = gtk_radio_button_new (bgroup);
		bgroup = gtk_radio_button_group (GTK_RADIO_BUTTON (button));
		gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, TRUE, 0);
		gtk_button_clicked (GTK_BUTTON (button));
		
		for (sl = l->subsection; sl->name; sl++)
		{
			GdkPixbuf *pixbuf;
			GtkWidget *box;
			
			button = gtk_radio_button_new (bgroup);
			gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
			gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (button), FALSE);
			bgroup = gtk_radio_button_group (GTK_RADIO_BUTTON (button));
			gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
			gtk_widget_show (button);
			
			gtk_object_set_data (GTK_OBJECT (button), "section", l);
			gtk_object_set_data (GTK_OBJECT (button), "subsection", sl);

			switch (sidebar->buttons_type)
			{
				case C2_SIDEBAR_BUTTON_JUST_TEXT:
					break;
				case C2_SIDEBAR_BUTTON_JUST_ICON:
					break;
				case C2_SIDEBAR_BUTTON_TEXT_UNDER_ICON:
					box = gtk_vbox_new (FALSE, 0);
					gtk_container_add (GTK_CONTAINER (button), box);
					gtk_widget_show (box);

					if (sl->icon)
					{
						GdkPixmap *pixmap;
						GdkBitmap *mask;
						GtkWidget *icon;
						
						pixbuf = gdk_pixbuf_new_from_file (sl->icon);
						gdk_pixbuf_render_pixmap_and_mask (pixbuf, &pixmap, &mask, 100);
						gdk_pixbuf_unref (pixbuf);
						icon = gtk_pixmap_new (pixmap, mask);
						gtk_box_pack_start (GTK_BOX (box), icon, FALSE, FALSE, 0);
						gtk_widget_show (icon);
					}

					if (sl->name)
					{
						GtkWidget *label = gtk_label_new (sl->name);

						gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
						gtk_box_pack_start (GTK_BOX (box), label, FALSE, TRUE, 0);
						gtk_widget_show (label);

						if (sidebar->tooltips)
							gtk_tooltips_set_tip (tooltips, button, sl->name, NULL);
					}

					break;
				case C2_SIDEBAR_BUTTON_TEXT_NEXT_TO_ICON:
					break;
			}
		}
	}
}

static void
on_section_button_clicked (GtkWidget *button, C2Sidebar *sidebar)
{
	C2SidebarSection *l;

	for (l = sidebar->section; l->name; l++)
	{
		GtkWidget *vbox = gtk_object_get_data (GTK_OBJECT (l->button), "panel");
		
		if (l->button != button)
			gtk_widget_hide (vbox);
		else
			gtk_widget_show (vbox);
	}
}
