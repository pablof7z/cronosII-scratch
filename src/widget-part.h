/*  Cronos II Mail Client
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
#ifndef __CRONOSII_WIDGET_PART_H__
#define __CRONOSII_WIDGET_PART_H__

#ifdef __cplusplus
extern "C" {
#endif

#if defined (HAVE_CONFIG_H) && defined (BUILDING_C2)
#	include <config.h>
#else
#	include <cronosII.h>
#endif
	
#ifdef USE_GTKHTML
#	include <gtkhtml/gtkhtml.h>
#elif defined (USE_GTKXMHTML)
#	include <gtk-xmhtml/gtk-xmhtml.h>
#else
#	include <gtk/gtktext.h>
#endif

#include <gtk/gtk.h>
#include <gtk/gtkwidget.h>

#define C2_PART(obj)							GTK_CHECK_CAST (obj, c2_part_get_type (), C2Part)
#define C2_PART_CLASS(klass)					GTK_CHECK_CLASS_CAST (klass, c2_part_get_type, C2PartClass)
#define C2_IS_PART(obj)							GTK_CHECK_TYPE (obj, c2_part_get_type ())

typedef struct _C2Part C2Part;
typedef struct _C2PartClass C2PartClass;

struct _C2Part
{
#ifdef USE_GTKHTML
#elif defined (USE_GTKXMHTML)
	GtkXmHTML parent;
#else
#endif
};

struct _C2PartClass
{
#ifdef USE_GTKHTML
#elif defined (USE_GTKXMHTML)
	GtkXmHTMLClass parent_class;
#else
#endif
};

guint
c2_part_get_type								(void);

GtkWidget *
c2_part_new										(void);

void
c2_part_set_contents							(C2Part *part, const gchar *contents);

void
c2_part_set_contents_from_url					(C2Part *part, const gchar *url);

#ifdef __cplusplus
}
#endif

#endif
