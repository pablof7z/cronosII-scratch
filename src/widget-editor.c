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
#include <glade/glade.h>
#include <unistd.h>
#include <sys/stat.h>

#include <libcronosII/account.h>
#include <libcronosII/error.h>
#include <libcronosII/smtp.h>
#include <libcronosII/utils.h>

#include "widget-editor.h"

static void
class_init									(C2EditorClass *klass);

static void
init										(C2Editor *editor);

enum
{
	EDITOR_CHANGED,
	UNDO,
	REDO,
	LAST_SIGNAL
};

static gint signals[LAST_SIGNAL] = { 0 };

static GtkVBoxClass *parent_class = NULL;

GtkType
c2_editor_get_type (void)
{
	static GtkType type = 0;

	if (!type)
	{
		GtkTypeInfo info =
		{
			"C2Editor",
			sizeof (C2Editor),
			sizeof (C2EditorClass),
			(GtkClassInitFunc) class_init,
			(GtkObjectInitFunc) init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL,
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (gtk_vbox_get_type (), &info);
	}

	return type;
}

static void
class_init (C2EditorClass *klass)
{
	GtkObjectClass *object_class;

	object_class = (GtkObjectClass *) klass;

	parent_class = gtk_type_class (gtk_vbox_get_type ());

	signals[EDITOR_CHANGED] =
		gtk_signal_new ("editor_changed",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2EditorClass, editor_changed),
						gtk_marshal_NONE__NONE, GTK_TYPE_NONE, 0);
	signals[UNDO] =
		gtk_signal_new ("undo",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2EditorClass, undo),
						gtk_marshal_NONE__NONE, GTK_TYPE_NONE, 0);
	signals[REDO] =
		gtk_signal_new ("redo",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2EditorClass, redo),
						gtk_marshal_NONE__NONE, GTK_TYPE_NONE, 0);
	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);
	
	klass->editor_changed = NULL;
	klass->undo = NULL;
	klass->redo = NULL;
}

static void
init (C2Editor *editor)
{
	editor->operations = NULL;
	editor->operations_ptr = NULL;
}

GtkWidget *
c2_editor_new (void)
{
	C2Editor *editor;
	GtkWidget *scroll;
#ifdef USE_ADVANCED_EDITOR
	GtkWidget *viewport;
#endif

	editor = gtk_type_new (c2_editor_get_type ());
	GTK_BOX (editor)->spacing = 2;
	GTK_BOX (editor)->homogeneous = FALSE;

#ifdef USE_ADVANCED_EDITOR
	/* Create the Editor Toolbar */
#endif

#ifdef USE_ADVANCED_EDITOR
	viewport = gtk_viewport_new (NULL, NULL);
	gtk_box_pack_end (GTK_BOX (editor), viewport, TRUE, TRUE, 0);
	gtk_widget_show (viewport);
#endif

	scroll = gtk_scrolled_window_new (NULL, NULL);
#ifdef USE_ADVANCED_EDITOR
	gtk_container_add (GTK_CONTAINER (viewport), scroll);
#else
	gtk_box_pack_start (GTK_BOX (editor), scroll, TRUE, TRUE, 0);
#endif
	gtk_widget_show (scroll);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll),
									GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

#ifdef USE_ADVANCED_EDITOR
	editor->text = gtk_html_new ();
	gtk_html_load_empty (GTK_HTML (editor->text));
	gtk_html_set_editable (GTK_HTML (editor->text), TRUE);
#else
	editor->text = gtk_text_new (NULL, NULL);
	gtk_text_set_editable (GTK_TEXT (editor->text), TRUE);
#endif
	gtk_container_add (GTK_CONTAINER (scroll), editor->text);
	gtk_widget_show (editor->text);

	return GTK_WIDGET (editor);
}

void
c2_editor_freeze (C2Editor *editor)
{
#ifdef USE_ADVANCED_EDITOR
#else
	gtk_text_freeze (GTK_TEXT (editor->text));
#endif
}

void
c2_editor_thaw (C2Editor *editor)
{
#ifdef USE_ADVANCED_EDITOR
#else
	gtk_text_thaw (GTK_TEXT (editor->text));
#endif
}

void
c2_editor_clear (C2Editor *editor)
{
#ifdef USE_ADVANCED_EDITOR
#else
	guint length;
#endif
	
#ifdef USE_ADVANCED_EDITOR
	gtk_html_load_empty (GTK_HTML (editor->text));
#else
	length = gtk_text_get_length (GTK_TEXT (editor->text));
	gtk_text_set_point (GTK_TEXT (editor->text), 0); 
	gtk_text_forward_delete (GTK_TEXT (editor->text), length);
#endif
}

void
c2_editor_append (C2Editor *editor, const gchar *string)
{
#ifdef USE_ADVANCED_EDITOR
#else
	guint length;
#endif

#ifdef USE_ADVANCED_EDITOR
#else
	length = strlen (string);

	gtk_text_insert (GTK_TEXT (editor->text), NULL, NULL, NULL, string, length);
#endif
}

gchar *
c2_editor_get_text (C2Editor *editor)
{
#ifdef USE_ADVANCED_EDITOR
#else
	return gtk_editable_get_chars (GTK_EDITABLE (editor->text), 0, -1);
#endif
}
