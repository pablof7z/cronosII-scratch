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

#ifndef USE_ADVANCED_EDITOR
static void
on_text_insert_text							(GtkText *widget, const gchar *text, gint length,
											 gint *position, C2Editor *editor);

static void
on_text_delete_text							(GtkText *widget, gint start_pos, gint end_pos,
											 C2Editor *editor);
#endif

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

	/* Connect to signals */
#ifdef USE_ADVANCED_EDITOR
#error The Advanced editor support has not be finished, you should specify the \
	   --disable-htmleditor flag to the configure script.
#else
	gtk_signal_connect (GTK_OBJECT (editor->text), "insert_text",
						GTK_SIGNAL_FUNC (on_text_insert_text), editor);
	gtk_signal_connect (GTK_OBJECT (editor->text), "delete_text",
						GTK_SIGNAL_FUNC (on_text_delete_text), editor);
#endif

	return GTK_WIDGET (editor);
}

#ifndef USE_ADVANCED_EDITOR
static void
on_text_insert_text (GtkText *widget, const gchar *text, gint length, gint *position, C2Editor *editor)
{
	gchar *buf = g_strndup (text, length);
	
	c2_editor_operations_add (editor, C2_EDITOR_OPERATION_INSERT, buf, length, *position);
	g_free (buf);
}

static void
on_text_delete_text (GtkText *widget, gint start_pos, gint end_pos, C2Editor *editor)
{
	c2_editor_operations_add (editor, C2_EDITOR_OPERATION_DELETE, NULL, end_pos-start_pos, start_pos);
}
#endif

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

void
c2_editor_operations_add (C2Editor *editor, C2EditorOperationType type, const gchar *text,
							gint length, gint position)
{
	C2EditorOperation *op;
	
	printf ("Merge: ");
	if (c2_editor_operations_merge (editor, type, text, length, position))
	{
		printf ("Success.\n");
		return;
	}

	printf ("Failed.\n");

	/* No marging capable, create the new operation and prepend
	 * it to the list of operations */
	op = g_new0 (C2EditorOperation, 1);

	op->type = type;
	op->text = g_strdup (text);
	op->length = length;
	op->position = position;

	if (length > 1)
		op->merge = 0;
	else
		op->merge = 1;

	editor->operations = g_list_prepend (editor->operations, op);
}

/**
 * c2_editor_operations_merge
 * @editor: Editor object.
 * @op: Operation to work with.
 *
 * This function will try to merge two changes
 * in order to act operations not in byte order
 * but with words style.
 * 
 * Return Value:
 * %TRUE if merging was successful, %FALSE if not.
 **/
gboolean
c2_editor_operations_merge (C2Editor *editor, C2EditorOperationType type, const gchar *text,
							gint length, gint position)
{
	C2EditorOperation *last_op;
	gchar *buf;

	/* We can't merge with nothing! */
	if (!editor->operations)
		return FALSE;
	
	/* It is like this because the list is in reverse order */
	last_op = C2_EDITOR_OPERATION (editor->operations->data);

	/* If the op we are trying to merge and the last op aren't
	 * of the same type we can't do the merge.
	 */
	if (last_op->type != type)
		return FALSE;

	/* If the last op doesn't want to merge we don't */
	if (!last_op->merge)
		return FALSE;

	if (type == C2_EDITOR_OPERATION_INSERT)
	{
		/* We can merge */
			
		/* Realloc */
		buf = g_realloc (last_op->text, (last_op->length+length)*sizeof (gchar));

		/* Copy the memory */
		memmove (buf, last_op->text, last_op->length);
		memmove (buf+last_op->length, text, length);
		last_op->text = buf;

		/* Now update the position */
		if (last_op->position > position)
			last_op->position = position;

		/* And now the length */
		last_op->length += length;
	} else
	{
		
	}
	
	return TRUE;
}

void
c2_editor_operations_undo (C2Editor *editor)
{
	C2EditorOperation *op;

	op = C2_EDITOR_OPERATION (editor->operations->data); /**/

	switch (op->type)
	{
		case C2_EDITOR_OPERATION_INSERT:
#ifdef USE_ADVANCED_EDITOR
#else
			printf ("%d %d\n", op->position, op->position+op->length);
			gtk_editable_delete_text (GTK_EDITABLE (editor->text), op->position,
										op->position+op->length);
#endif
			break;
	}
}

void
c2_editor_operations_redo (C2Editor *editor)
{
}
