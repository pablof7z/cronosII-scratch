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
#ifndef __CRONOSII_WIDGET_SIDEBAR_H__
#define __CRONOSII_WIDGET_SIDEBAR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <gnome.h>

#define C2_SIDEBAR(obj)						(GTK_CHECK_CAST (obj, c2_sidebar_get_type (), C2Sidebar))
#define C2_SIDEBAR_CLASS(klass)				(GTK_CHECK_CLASS_CAST (klass, c2_sidebar_get_type (), C2SidebarClass))

typedef struct _C2Sidebar C2Sidebar;
typedef struct _C2SidebarClass C2SidebarClass;
typedef struct _C2SidebarList C2SidebarList;

struct _C2SidebarList
{
	gchar *section_name;

	GtkWidget *button;
	GtkWidget *box;

	C2SidebarList *next;
};

struct _C2Sidebar
{
	GtkContainer container;

	GtkWidget *vbox;

	C2SidebarList *list;
};

struct _C2SidebarClass
{
	GtkContainerClass parent_class;

	void (*selection) (C2Sidebar *sidebar, GtkWidget *widget);
};

GtkType
c2_sidebar_get_type							(void);

GtkWidget *
c2_sidebar_new								(void);

void
c2_sidebar_add								(C2Sidebar *sidebar,
											 gchar *section, gchar *subsection,
											 const gchar *pixmap, GtkWidget *widget);

void
c2_sidebar_set_selection					(C2Sidebar *sidebar, const gchar *section,
											 const gchar *subsection);

void
c2_sidebar_get_selection					(C2Sidebar *sidebar, gchar **section, gchar **subsection);

void
c2_sidebar_get_widget						(C2Sidebar *sidebar, const gchar *section,
											 const gchar *subsection);

#ifdef __cplusplus
}
#endif

#endif
