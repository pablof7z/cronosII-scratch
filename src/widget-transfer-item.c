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
#include "widget-transfer-item.h"

static void
class_init									(C2TransferItemClass *klass);

static void
init										(C2TransferItem *ti);

static void
destroy										(GtkObject *object);

enum
{
	STATE_CHANGED,
	FINISH,
	CANCEL,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static GtkContainerClass *parent_class = NULL;

GtkType
c2_transfer_item_get_type (void)
{
	static GtkType type = 0;

	if (!type)
	{
		static GtkTypeInfo info =
		{
			"C2TransferItem",
			sizeof (C2TransferItem),
			sizeof (C2TransferItemClass),
			(GtkClassInitFunc) class_init,
			(GtkObjectInitFunc) init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL
		};

		type = gtk_type_unique (gtk_container_get_type (), &info);
	}

	return type;
}

static void
class_init (C2TransferItemClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *) klass;

	parent_class = gtk_type_class (gtk_container_get_type ());

	signals[STATE_CHANGED] =
		gtk_signal_new ("state_changed",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2TransferItemClass, state_changed),
						gtk_marshal_NONE__INT, GTK_TYPE_NONE, 1,
						GTK_TYPE_INT);
	signals[FINISH] =
		gtk_signal_new ("finish",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2TransferItemClass, finish),
						gtk_marshal_NONE__NONE, GTK_TYPE_NONE, 0);
	signals[CANCEL] =
		gtk_signal_new ("cancel",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2TransferItemClass, cancel),
						gtk_marshal_NONE__POINTER, GTK_TYPE_NONE, 1,
						GTK_TYPE_POINTER);
	gtk_object_class_add_signals(object_class, signals, LAST_SIGNAL);

	klass->state_changed = NULL;
	klass->finish = NULL;
	klass->cancel = NULL;
}

static void
init (C2TransferItem *ti)
{
	ti->state = 0;
	ti->tooltip = NULL;
	ti->account = NULL;
}

GtkWidget *
c2_transfer_item_new (C2Account *account)
{
	C2TransferItem *ti;
	GtkWidget *table, *label, *button, *pixmap;

	ti = gtk_type_new (c2_transfer_item_get_type ());

	table = gtk_table_new (2, 3, FALSE);
	gtk_table_set_col_spacings (GTK_TABLE (table), 4);
	gtk_table_set_row_spacings (GTK_TABLE (table), 4);
	gtk_container_add (GTK_CONTAINER (ti), table);
	gtk_widget_show (table);

	label = gtk_label_new (account->name);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 0, 1);
	gtk_widget_show (label);

	ti->progress_mail = gtk_progress_bar_new ();
	gtk_table_attach_defaults (GTK_TABLE (table), ti->progress_mail, 1, 2, 0, 1);
	gtk_widget_show (ti->progress_mail);

	ti->progress_byte = gtk_progress_bar_new ();
	gtk_table_attach_defaults (GTK_TABLE (table), ti->progress_byte, 1, 2, 1, 2);
	gtk_widget_show (ti->progress_byte);

	button = gtk_button_new ();

	pixmap = gnome_pixmap_new_from_file (GNOME_STOCK_PIXMAP_STOP);
	gtk_container_add (GTK_CONTAINER (button), pixmap);
	gtk_widget_show (pixmap);

	gtk_table_attach_defaults (GTK_TABLE (table), button, 2, 3, 0, 1);
	gtk_widget_show (button);

	return GTK_WIDGET (ti);
}
