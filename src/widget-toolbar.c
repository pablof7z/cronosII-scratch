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
 *  GNU General Public License for more details._
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include "widget-toolbar.h"

#include <libcronosII/utils.h>

/* TODO
 * 20011208 Some type of scroll when the toolbar doesn't fit in the window.
 */

static void
class_init									(C2ToolbarClass *klass);

static void
init										(C2Toolbar *toolbar);

static void
clear_toolbar								(C2Toolbar *toolbar);

static void
create_toolbar								(C2Toolbar *toolbar);

static GtkWidget *
create_item_button							(C2Toolbar *toolbar, GtkWidget *button,
											 const gchar *pixmap, const gchar *label,
											 const gchar *tooltip, gboolean force_label);

static GtkWidget *
create_item_button_content					(C2Toolbar *toolbar, const gchar *pixmap,
											 const gchar *label, gboolean force_label);

static GtkWidget *
create_item_space							(C2Toolbar *toolbar);

enum
{
	CHANGED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static GtkHBoxClass *parent_class = NULL;

struct Item
{
	C2ToolbarItemType type;

	gchar *name;

	union
	{
		struct
		{
			const gchar *label;
			const gchar *pixmap;
			const gchar *tooltip;
			gboolean force_label;
			GtkWidget *widget;
		} button;
		struct
		{
			GtkWidget *widget;
			const gchar *tooltip;
		} widget;
		struct
		{
			GtkWidget *widget;
		} space;
	} inf;
};

GtkType
c2_toolbar_get_type (void)
{
	static GtkType type = 0;

	if (!type)
	{
		static GtkTypeInfo info =
		{
			"C2Toolbar",
			sizeof (C2Toolbar),
			sizeof (C2ToolbarClass),
			(GtkClassInitFunc) class_init,
			(GtkObjectInitFunc) init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL
		};

		type = gtk_type_unique (gtk_hbox_get_type (), &info);
	}

	return type;
}

static void
class_init (C2ToolbarClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *) klass;
	
	parent_class = gtk_type_class (gtk_hbox_get_type ());
	signals[CHANGED] =
		gtk_signal_new ("changed",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2ToolbarClass, changed),
						gtk_marshal_NONE__NONE, GTK_TYPE_NONE, 0);
	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);

	klass->changed = NULL;
}

static void
init (C2Toolbar *toolbar)
{
	toolbar->style = C2_TOOLBAR_TEXT_UNDER_ICON;
	toolbar->items = NULL;
	toolbar->space_size = 2;
	toolbar->space_style = GTK_TOOLBAR_SPACE_LINE;
	toolbar->tooltips = 1;
	toolbar->gtktooltips = NULL;
	toolbar->freezed = 0;
}

GtkWidget *
c2_toolbar_new (C2ToolbarStyle style)
{
	C2Toolbar *toolbar;

	toolbar = gtk_type_new (c2_toolbar_get_type ());
	toolbar->style = style;
	GTK_BOX (toolbar)->spacing = 2;
	GTK_BOX (toolbar)->homogeneous = FALSE;
	gtk_container_set_border_width (GTK_CONTAINER (toolbar), 1);

	return GTK_WIDGET (toolbar);
}

void
c2_toolbar_freeze (C2Toolbar *toolbar)
{
	toolbar->freezed = 1;
}

void
c2_toolbar_thaw (C2Toolbar *toolbar)
{
	toolbar->freezed = 0;
	create_toolbar (toolbar);
}

void
c2_toolbar_clear (C2Toolbar *toolbar)
{
	clear_toolbar (toolbar);
	g_list_free (toolbar->items);
	toolbar->items = NULL;

	if (!toolbar->freezed)
		gtk_signal_emit (GTK_OBJECT (toolbar), signals[CHANGED]);
}

void
c2_toolbar_set_style (C2Toolbar *toolbar, C2ToolbarStyle style)
{
	toolbar->style = style;

	if (!toolbar->freezed)
		create_toolbar (toolbar);
}

void
c2_toolbar_set_tooltips (C2Toolbar *toolbar, gboolean active)
{
	toolbar->tooltips = active ? 1 : 0;

	if (active)
		gtk_tooltips_enable (toolbar->gtktooltips);
	else
		gtk_tooltips_disable (toolbar->gtktooltips);
}

GtkWidget *
c2_toolbar_append_button (C2Toolbar *toolbar, gchar *name, gchar *pixmap, gchar *label,
						  gchar *tooltip, gboolean force_label)
{
	struct Item *item;

	item = g_new0 (struct Item, 1);
	item->type = C2_TOOLBAR_BUTTON;
	item->name = name;
	item->inf.button.pixmap = pixmap;
	item->inf.button.label = label;
	item->inf.button.tooltip = tooltip;
	item->inf.button.force_label = force_label;

	toolbar->items = g_list_append (toolbar->items, item);

	item->inf.button.widget = create_item_button (toolbar, NULL, pixmap, label, tooltip, force_label);
	
	if (!toolbar->freezed)
		gtk_signal_emit (GTK_OBJECT (toolbar), signals[CHANGED]);
	
	return item->inf.button.widget;
}

void
c2_toolbar_append_widget (C2Toolbar *toolbar, gchar *name, GtkWidget *widget, gchar *tooltip)
{
	if (!toolbar->freezed)
		gtk_signal_emit (GTK_OBJECT (toolbar), signals[CHANGED]);
}

void
c2_toolbar_append_space (C2Toolbar *toolbar)
{
	struct Item *item;

	item = g_new0 (struct Item, 1);
	item->type = C2_TOOLBAR_SPACE;

	toolbar->items = g_list_append (toolbar->items, item);

	item->inf.space.widget = create_item_space (toolbar);
}

static void
clear_toolbar (C2Toolbar *toolbar)
{
	GList *l;

	/* Remove content */
	for (l = gtk_container_children (GTK_CONTAINER (toolbar)); l; l = g_list_next (l))
		gtk_container_remove (GTK_CONTAINER (toolbar), GTK_WIDGET (l->data));

	if (!toolbar->freezed)
		gtk_signal_emit (GTK_OBJECT (toolbar), signals[CHANGED]);
}

static void
create_toolbar (C2Toolbar *toolbar)
{
	GList *l;
	struct Item *item;
	
	if (toolbar->freezed)
		return;
	
	clear_toolbar (toolbar);

	/* Add content */
	for (l = toolbar->items; l; l = g_list_next (l))
	{
		item = (struct Item*) l->data;
		
		switch (item->type)
		{
			case C2_TOOLBAR_BUTTON:
				item->inf.button.widget =
						create_item_button (toolbar, item->inf.button.widget,
									item->inf.button.pixmap,
									item->inf.button.label,
									item->inf.button.tooltip,
									item->inf.button.force_label);
				gtk_box_pack_start (GTK_BOX (toolbar), item->inf.button.widget, FALSE, FALSE, 0);
				break;
			case C2_TOOLBAR_WIDGET:
				break;
			case C2_TOOLBAR_SPACE:
				gtk_box_pack_start (GTK_BOX (toolbar), item->inf.space.widget, FALSE, FALSE, 0);
				break;
		}
	}

	if (!toolbar->freezed)
		gtk_signal_emit (GTK_OBJECT (toolbar), signals[CHANGED]);
}

static GtkWidget *
create_item_button (C2Toolbar *toolbar, GtkWidget *button, const gchar *pixmap,
					const gchar *label, const gchar *tooltip, gboolean force_label)
{
	if (!button)
	{
		button = gtk_button_new ();
		GTK_WIDGET_UNSET_FLAGS (button, GTK_CAN_FOCUS);
		if (gnome_preferences_get_toolbar_relief_btn ())
			gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NORMAL);
		else
			gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
		gtk_widget_ref (button);

		if (!toolbar->gtktooltips)
			toolbar->gtktooltips = gtk_tooltips_new ();
		gtk_tooltips_set_tip (toolbar->gtktooltips, button, tooltip, NULL);
	} else
	{
		GList *l;

		/* Remove content */
		for (l = gtk_container_children (GTK_CONTAINER (button)); l; l = g_list_next (l))
			gtk_container_remove (GTK_CONTAINER (button), GTK_WIDGET (l->data));
	}

	gtk_container_add (GTK_CONTAINER (button),
						create_item_button_content (toolbar, pixmap, label, force_label));

	if (!toolbar->freezed)
		gtk_box_pack_start (GTK_BOX (toolbar), button, FALSE, FALSE, 0);	

	gtk_widget_show (button);

	return button;
}

static GtkWidget *
create_item_button_content (C2Toolbar *toolbar, const gchar *pixmap, const gchar *label,
					gboolean force_label)
{
	GtkWidget *box;
	GtkWidget *wpixmap;
	GtkWidget *wlabel;
	gboolean icon, text;

	switch (toolbar->style)
	{
		case C2_TOOLBAR_JUST_TEXT:
			text = TRUE;
			icon = FALSE;
			box = gtk_vbox_new (FALSE, 0);
			break;
		case C2_TOOLBAR_JUST_ICON:
			text = FALSE;
			icon = TRUE;
			box = gtk_vbox_new (FALSE, 0);
			break;
		case C2_TOOLBAR_TEXT_UNDER_ICON:
			text = TRUE;
			icon = TRUE;
			box = gtk_vbox_new (FALSE, 0);
			break;
		default:
		case C2_TOOLBAR_TEXT_BESIDE_ICON:
			text = force_label;
			icon = TRUE;
			box = gtk_hbox_new (FALSE, 2);
			break;
	}

	gtk_widget_show (box);

	if (pixmap)
	{
		size_t isize;

		isize = toolbar->style == C2_TOOLBAR_TEXT_BESIDE_ICON ? 24 : 24;
		wpixmap = gnome_pixmap_new_from_file_at_size (pixmap, isize, isize);
		gtk_box_pack_start (GTK_BOX (box), wpixmap, TRUE, TRUE, 0);
		if (icon)
			gtk_widget_show (wpixmap);
		else
			gtk_widget_hide (wpixmap);
	}

	if (label)
	{
		wlabel = gtk_label_new (label);
		gtk_box_pack_start (GTK_BOX (box), wlabel, FALSE, FALSE, 0);
		if (text)
			gtk_widget_show (wlabel);
		else
			gtk_widget_hide (wlabel);
	}

	return box;
}

static GtkWidget *
create_item_space (C2Toolbar *toolbar)
{
	GtkWidget *align;
	GtkWidget *vsep;

	align = gtk_alignment_new (0.5, 0.5, 1, 0.75);
	if (!toolbar->freezed)
		gtk_box_pack_start (GTK_BOX (toolbar), align, FALSE, FALSE, 0);
	
	vsep = gtk_vseparator_new ();
	gtk_container_add (GTK_CONTAINER (align), vsep);
	gtk_widget_show (vsep);
	gtk_widget_show (align);
	gtk_widget_ref (align);

	return align;
}

GtkWidget *
c2_toolbar_get_item (C2Toolbar *toolbar, const gchar *name)
{
	GList *l;

	for (l = toolbar->items; l; l = g_list_next (l))
	{
		struct Item *item = (struct Item*) l->data;

		if (c2_streq (item->name, name))
		{
			switch (item->type)
			{
				case C2_TOOLBAR_BUTTON:
					return item->inf.button.widget;
				case C2_TOOLBAR_WIDGET:
					return item->inf.widget.widget;
				case C2_TOOLBAR_SPACE:
					return item->inf.space.widget;
			}
		}
	}

	return NULL;
}
