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

#define RED_COLOR	0x7500
#define GREEN_COLOR	0x7500
#define BLUE_COLOR	0x7500

static void
class_init									(C2SidebarClass *klass);

static void
init										(C2Sidebar *sidebar);

static void
on_section_button_clicked					(GtkWidget *button, C2Sidebar *sidebar);

static void
on_subsection_button_clicked				(GtkWidget *button, C2Sidebar *sidebar);

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
	GtkWidget *button;

	sidebar->section = list;

	button = gtk_button_new ();
	gtk_box_pack_start (GTK_BOX (sidebar), button, FALSE, TRUE, 0);
	gtk_widget_set_usize (button, -1, 4);
	gtk_widget_set_sensitive (button, FALSE);
	gtk_widget_show (button);
	
	for (l = list; l->name; l++)
	{
		C2SidebarSubSection *sl;
		GtkWidget *vbox, *viewport, *scroll;
		GSList *bgroup = NULL;
		GtkStyle *style;

		button = gtk_button_new_with_label (l->name);
		gtk_box_pack_start (GTK_BOX (sidebar), button, FALSE, TRUE, 0);
		gtk_widget_show (button);
		l->button = button;

		scroll = gtk_scrolled_window_new (NULL, NULL);
		gtk_box_pack_start (GTK_BOX (sidebar), scroll, TRUE, TRUE, 0);
		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
		gtk_widget_hide (scroll);

		viewport = gtk_viewport_new (NULL, NULL);
		gtk_container_add (GTK_CONTAINER (scroll), viewport);
		gtk_widget_show (viewport);
		style = gtk_style_copy (gtk_widget_get_style (viewport));
		
		style->bg[GTK_STATE_NORMAL].red = RED_COLOR;
		style->bg[GTK_STATE_NORMAL].green = GREEN_COLOR;
		style->bg[GTK_STATE_NORMAL].blue = BLUE_COLOR;

		gdk_color_alloc (gdk_colormap_get_system (), &style->bg[GTK_STATE_NORMAL]);
		gtk_widget_set_style (viewport, style);

		vbox = gtk_vbox_new (FALSE, 0);
		gtk_container_add (GTK_CONTAINER (viewport), vbox);
		gtk_widget_show (vbox);
		
		gtk_object_set_data (GTK_OBJECT (button), "panel", scroll);
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
			style = gtk_style_copy (gtk_widget_get_style (button));
		
			style->bg[GTK_STATE_PRELIGHT].red = RED_COLOR;
			style->bg[GTK_STATE_PRELIGHT].green = GREEN_COLOR;
			style->bg[GTK_STATE_PRELIGHT].blue = BLUE_COLOR;
			style->bg[GTK_STATE_ACTIVE].red = RED_COLOR;
			style->bg[GTK_STATE_ACTIVE].green = GREEN_COLOR;
			style->bg[GTK_STATE_ACTIVE].blue = BLUE_COLOR;
			gdk_color_alloc (gdk_colormap_get_system (), &style->bg[GTK_STATE_NORMAL]);
			gtk_widget_set_style (button, style);
			GTK_WIDGET_UNSET_FLAGS (button, GTK_CAN_FOCUS);
			gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (button), FALSE);
			bgroup = gtk_radio_button_group (GTK_RADIO_BUTTON (button));
			gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
			gtk_widget_show (button);
			
			gtk_object_set_data (GTK_OBJECT (button), "section", l);
			gtk_object_set_data (GTK_OBJECT (button), "subsection", sl);

			gtk_signal_connect (GTK_OBJECT (button), "clicked",
								GTK_SIGNAL_FUNC (on_subsection_button_clicked), sidebar);

			switch (sidebar->buttons_type)
			{
				case C2_SIDEBAR_BUTTON_JUST_TEXT:
					break;
				case C2_SIDEBAR_BUTTON_JUST_ICON:
					break;
				case C2_SIDEBAR_BUTTON_TEXT_UNDER_ICON:
				case C2_SIDEBAR_BUTTON_TEXT_NEXT_TO_ICON:
					if (sidebar->buttons_type == C2_SIDEBAR_BUTTON_TEXT_UNDER_ICON)
						box = gtk_vbox_new (FALSE, 0);
					else
						box = gtk_hbox_new (FALSE, 2);

					gtk_container_add (GTK_CONTAINER (button), box);
					gtk_widget_show (box);

					if (sl->icon)
					{
						GtkWidget *pixmap;
						GtkWidget *icon;
						size_t isize;

						isize = sidebar->buttons_type == C2_SIDEBAR_BUTTON_TEXT_UNDER_ICON ?
									48 : 24;
						
						pixmap = gnome_pixmap_new_from_file_at_size (sl->icon,
										isize, isize);
						icon = gtk_pixmap_new (GNOME_PIXMAP (pixmap)->pixmap,
												GNOME_PIXMAP (pixmap)->mask);
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
			}
		}
	}

	button = gtk_button_new ();
	gtk_box_pack_start (GTK_BOX (sidebar), button, FALSE, TRUE, 0);
	gtk_widget_set_usize (button, -1, 4);
	gtk_widget_set_sensitive (button, FALSE);
	gtk_widget_show (button);
}

static void
on_section_button_clicked (GtkWidget *button, C2Sidebar *sidebar)
{
	C2SidebarSection *l;
	C2SidebarSection *section = NULL;

	for (l = sidebar->section; l->name; l++)
	{
		GtkWidget *vbox = gtk_object_get_data (GTK_OBJECT (l->button), "panel");
		
		if (l->button != button)
			gtk_widget_hide (vbox);
		else
		{
			section = l;
			gtk_widget_show (vbox);
		}
	}

	if (section)
		gtk_signal_emit (GTK_OBJECT (sidebar), signals[SECTION_SELECTED], section->name);
}

static void
on_subsection_button_clicked (GtkWidget *button, C2Sidebar *sidebar)
{
	C2SidebarSection *section = gtk_object_get_data (GTK_OBJECT (button), "section");
	C2SidebarSubSection *subsection = gtk_object_get_data (GTK_OBJECT (button), "subsection");

	gtk_signal_emit (GTK_OBJECT (sidebar), signals[SUBSECTION_SELECTED], section->name, subsection->name);
}

void
c2_sidebar_set_buttons_type (C2Sidebar *sidebar, C2SidebarButtonType type)
{
	sidebar->buttons_type = type;

	/* [TODO] Some kind of reload? */
}
