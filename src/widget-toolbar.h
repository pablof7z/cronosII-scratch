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
#ifndef __WIDGET_TOOLBAR_H__
#define __WIDGET_TOOLBAR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <gnome.h>

#define C2_TOOLBAR(obj)						(GTK_CHECK_CAST (obj, c2_toolbar_get_type (), C2Toolbar))
#define C2_TOOLBAR_CLASS(klass)				(GTK_CHECK_CLASS_CAST (klass, c2_toolbar_get_type (), C2ToolbarClass))

typedef struct _C2Toolbar C2Toolbar;
typedef struct _C2ToolbarClass C2ToolbarClass;
typedef struct _C2ToolbarItem C2ToolbarItem;
typedef enum _C2ToolbarItemType C2ToolbarItemType;
typedef enum _C2ToolbarStyle C2ToolbarStyle;

enum _C2ToolbarStyle
{
	C2_TOOLBAR_TEXT_BESIDE_ICON = 0,
	C2_TOOLBAR_TEXT_UNDER_ICON = 1,
	C2_TOOLBAR_JUST_TEXT = 2,
	C2_TOOLBAR_JUST_ICON = 3,
};

enum _C2ToolbarItemType
{
	C2_TOOLBAR_BUTTON,
	C2_TOOLBAR_WIDGET,
	C2_TOOLBAR_SPACE
};

struct _C2ToolbarItem
{
	C2ToolbarItemType type;
	
	gchar *button_label, *button_pixmap, *button_tooltip;
	gboolean button_force_label;
	GtkSignalFunc button_func;
	gpointer button_data;

	GtkWidget *widget;
	gchar *widget_tooltip;
};

struct _C2Toolbar
{
	GtkHBox box;

	GList *items;
	
	C2ToolbarStyle style;
	gint space_size;
	GtkToolbarSpaceStyle space_style;
	GtkTooltips *gtktooltips;

	gint tooltips: 1;
	gint freezed : 1;
};

struct _C2ToolbarClass
{
	GtkHBoxClass parent_class;

	void (*changed) (C2Toolbar *toolbar);
};

GtkType
c2_toolbar_get_type							(void);

GtkWidget *
c2_toolbar_new								(C2ToolbarStyle style);

void
c2_toolbar_freeze							(C2Toolbar *toolbar);

void
c2_toolbar_clear							(C2Toolbar *toolbar);

void
c2_toolbar_thaw								(C2Toolbar *toolbar);

void
c2_toolbar_set_style						(C2Toolbar *toolbar, C2ToolbarStyle style);

void
c2_toolbar_set_tooltips						(C2Toolbar *toolbar, gboolean active);

GtkWidget *
c2_toolbar_append_button					(C2Toolbar *toolbar, gchar *pixmap, gchar *label,
											 gchar *tooltip, gboolean force_label);

void
c2_toolbar_append_widget					(C2Toolbar *toolbar, GtkWidget *widget, gchar *tooltip);

void
c2_toolbar_append_space						(C2Toolbar *toolbar);

#ifdef __cplusplus
}
#endif

#endif
