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

#ifdef USE_ADVANCED_EDITOR
#	include <gtkhtml/gtkhtml.h>
#else
#	include <gtk/gtktext.h>
#endif

#ifdef USE_DEBUG
#	define C2_EDITOR(obj)						(GTK_CHECK_CAST (obj, c2_editor_get_type (), C2Editor))
#	define C2_EDITOR_CLASS(klass)				(GTK_CHECK_CLASS_CAST (klass, c2_editor_get_type (), C2Editor))
#else
#	define C2_EDITOR(obj)						((C2Editor*)obj)
#	define C2_EDITOR_CLASS(klass)				((C2EditorClass*)klass)
#endif
		
#define C2_IS_EDITOR(obj)					(GTK_CHECK_TYPE (obj, c2_editor_get_type ()))
#define C2_EDITOR_OPERATION(obj)			((C2EditorOperation*)obj)

typedef struct _C2Editor C2Editor;
typedef struct _C2EditorClass C2EditorClass;
typedef struct _C2EditorOperation C2EditorOperation;
typedef enum _C2EditorOperationType C2EditorOperationType;

enum _C2EditorOperationType
{
	C2_EDITOR_OPERATION_INSERT,
	C2_EDITOR_OPERATION_DELETE
};

struct _C2EditorOperation
{
	C2EditorOperationType type;
	gchar *text;
	gint length;
	gint position;

	gint merge : 1;
};

struct _C2Editor
{
	GtkVBox object;

	GtkWidget *text;

	/* Implementation specific data */
#ifdef USE_ADVANCED_EDITOR
#else
	GdkFont *font;
#endif

	/* Undo and Redo */
#ifndef USE_ADVANCED_EDITOR
	guint insert_signal, delete_signal;
#endif
	GSList *undo;
	GSList *redo;
};

struct _C2EditorClass
{
	GtkVBoxClass parent_class;

	void (*editor_changed) (C2Editor *editor);
	void (*undo) (C2Editor *editor);
	void (*redo) (C2Editor *editor);
	void (*undo_available) (C2Editor *editor, gboolean available);
	void (*redo_available) (C2Editor *editor, gboolean available);
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
c2_editor_append							(C2Editor *editor, const gchar *string,
											 gint red, gint green, gint blue);

gchar *
c2_editor_get_text							(C2Editor *editor);

void
c2_editor_set_text							(C2Editor *editor, const gchar *text);

/* Operations (Undo & Redo) */
void
c2_editor_operations_add					(C2Editor *editor, C2EditorOperationType type,
											 const gchar *text, gint length, gint position);

gboolean
c2_editor_operations_merge					(C2Editor *editor, C2EditorOperationType type,
											 const gchar *text, gint length, gint position);

void
c2_editor_operations_undo					(C2Editor *editor);

void
c2_editor_operations_redo					(C2Editor *editor);

gboolean
c2_editor_operations_undo_available			(C2Editor *editor);

gboolean
c2_editor_operations_redo_available			(C2Editor *editor);

#ifdef __cplusplus
}
#endif

#endif
