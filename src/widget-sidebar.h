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
#ifndef __WIDGET_SIDEBAR_H__
#define __WIDGET_SIDEBAR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <gtk/gtkvbox.h>
#include <gtk/gtkwidget.h>

#define C2_SIDEBAR(obj)						(GTK_CHECK_CAST (obj, c2_sidebar_get_type (), C2Sidebar))
#define C2_SIDEBAR_CLASS(klass)				(GTK_CHECK_CLASS_CAST (klass, c2_sidebar_get_type (), C2SidebarClass))

typedef struct _C2Sidebar C2Sidebar;
typedef struct _C2SidebarClass C2SidebarClass;
typedef struct _C2SidebarSection C2SidebarSection;
typedef struct _C2SidebarSubSection C2SidebarSubSection;
typedef enum _C2SidebarButtonType C2SidebarButtonType;

enum _C2SidebarButtonType
{
	C2_SIDEBAR_BUTTON_JUST_TEXT,
	C2_SIDEBAR_BUTTON_JUST_ICON,
	C2_SIDEBAR_BUTTON_TEXT_UNDER_ICON,
	C2_SIDEBAR_BUTTON_TEXT_NEXT_TO_ICON
};

struct _C2SidebarSection
{
	const gchar *name;
	C2SidebarSubSection *subsection;
	GtkWidget *button;
};

struct _C2SidebarSubSection
{
	const gchar *name;
	const gchar *icon;
};

struct _C2Sidebar
{
	GtkVBox box;

	C2SidebarSection *section;
	C2SidebarSection *selected;

	C2SidebarButtonType buttons_type;
	gint tooltips		: 1;
};

struct _C2SidebarClass
{
	GtkVBoxClass parent_class;

	void (*section_selected) (C2Sidebar *sidebar, const gchar *section, const gchar *subsection);
	void (*subsection_selected) (C2Sidebar *sidebar, const gchar *section, const gchar *subsection);
};

GtkType
c2_sidebar_get_type							(void);

GtkWidget *
c2_sidebar_new								(void);

void
c2_sidebar_set_contents						(C2Sidebar *sidebar, C2SidebarSection *list);

void
c2_sidebar_set_buttons_type					(C2Sidebar *sidebar, C2SidebarButtonType type);

#ifdef __cplusplus
}
#endif

#endif
