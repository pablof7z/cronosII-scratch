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
#ifndef __WIDGET_EDITOR_H__
#define __WIDGET_EDITOR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <gnome.h>

#ifdef BUILDING_C2
#	include <config.h>
#else
#	include <cronosII.h>
#endif

#ifdef USE_GTKHTML
#	include <gtkhtml/gtkhtml.h>
#	include <bonobo.h>
#endif

#define C2_EDITOR(obj)						(GTK_CHECK_CAST (obj, c2_editor_get_type (), C2Editor))
#define C2_EDITOR_CLASS(klass)				(GTK_CHECK_CLASS_CAST (klass, c2_editor_get_type (), C2Editor))

typedef struct _C2Editor C2Editor;
typedef struct _C2EditorClass C2EditorClass;

struct _C2Editor
{
	GtkVBox object;

	GtkWidget *text;

	/* Undo and Redo */
	GList *operations; /* History of operations */
	GList *operations_ptr;
};

struct _C2EditorClass
{
	GtkVBoxClass parent_class;

	void (*editor_changed) (C2Editor *editor);
	void (*undo) (C2Editor *editor);
	void (*redo) (C2Editor *editor);
};

GtkType
c2_editor_get_type							(void);

GtkWidget *
c2_editor_new								(void);

void
c2_editor_freeze							(C2Editor *editor);

void
c2_editor_thaw								(C2Editor *editor);

void
c2_editor_clear								(C2Editor *editor);

void
c2_editor_append							(C2Editor *editor, const gchar *string);

gchar *
c2_editor_get_text							(C2Editor *editor);

void
c2_editor_set_text							(C2Editor *editor, const gchar *text);

/* Undo & Redo */
void
c2_editor_undo								(C2Editor *editor);

void
c2_editor_redo								(C2Editor *editor);

gboolean
c2_editor_undo_available					(C2Editor *editor);

gboolean
c2_editor_redo_available					(C2Editor *editor);

#ifdef __cplusplus
}
#endif

#endif
