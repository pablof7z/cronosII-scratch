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
#include "widget-transfer-list.h"

static void
class_init									(C2TransferListClass *klass);

static void
init										(C2TransferList *tl);

static void
destroy										(GtkObject *object);

enum
{
	FINISH,
	CANCEL_ALL,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static C2DialogClass *parent_class = NULL;

GtkType
c2_transfer_list_get_type (void)
{
	static GtkType type = 0;

	if (!type)
	{
		static GtkTypeInfo info =
		{
			"C2TransferList",
			sizeof (C2TransferList),
			sizeof (C2TransferListClass),
			(GtkClassInitFunc) class_init,
			(GtkObjectInitFunc) init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL
		};

		type = gtk_type_unique (c2_dialog_get_type (), &info);
	}

	return type;
}

static void
class_init (C2TransferListClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *) klass;

	parent_class = gtk_type_class (c2_dialog_get_type ());

	signals[FINISH] =
		gtk_signal_new ("finish",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2TransferListClass, finish),
						gtk_marshal_NONE__NONE, GTK_TYPE_NONE, 0);
	signals[CANCEL_ALL] =
		gtk_signal_new ("cancel_all",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2TransferListClass, cancel_all),
						gtk_marshal_NONE__NONE, GTK_TYPE_NONE, 0);
	gtk_object_class_add_signals(object_class, signals, LAST_SIGNAL);

	klass->finish = NULL;
	klass->cancel_all = NULL;
}

static void
init (C2TransferList *tl)
{
	tl->list = NULL; 
}

GtkWidget *
c2_transfer_list_new (C2Application *application)
{
	C2TransferList *tl;
	const gchar *buttons[] =
	{
		N_("Stop all"),
		GNOME_STOCK_BUTTON_CLOSE,
		NULL
	};

	tl = gtk_type_new (c2_transfer_list_get_type ());

	c2_dialog_construct (C2_DIALOG (tl), application, _("Message Transfer"), buttons);

	return GTK_WIDGET (tl);
}
