/*  Cronos II - The GNOME mail client
 *  Copyright (C) 2000-2001 Pablo Fernández
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
/**
 * Maintainer(s) of this file:
 * 		* Pablo Fernández
 * Code of this file by:
 * 		* Pablo Fernández
 */
#include <config.h>

#include "widget-transfer-list.h"
#include "preferences.h"

#include <libcronosII/error.h>

#define MOD		"[Widget] Transfer List"
#define DMOD	FALSE

static void
class_init									(C2TransferListClass *klass);

static void
init										(C2TransferList *tl);

static void
destroy										(GtkObject *object);

static void
on_close_button_toggled						(GtkWidget *widget, gpointer data);

static void
on_button0_clicked							(GtkWidget *widget, C2TransferList *tl);

static void
on_button1_clicked							(GtkWidget *widget, C2TransferList *tl);

static void
on_transfer_item_finish						(C2TransferItem *ti, C2TransferList *tl);

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

static void
destroy (GtkObject *object)
{
	C2TransferList *tl = C2_TRANSFER_LIST (object);
	GSList *l;

	for (l = tl->list; l; l = g_slist_next (l))
	{
		if (C2_IS_TRANSFER_ITEM (l->data))
			gtk_object_destroy (GTK_OBJECT (l->data));
	}
	
	g_slist_free (tl->list);

	printf ("Emitiendo la señal de desconexión %d\n", __LINE__);
	gtk_signal_emit (GTK_OBJECT (tl), signals[FINISH]);
}

GtkWidget *
c2_transfer_list_new (C2Application *application)
{
	C2TransferList *tl;
	GtkWidget *button, *vbox;
	const gchar *buttons[] =
	{
		N_("Stop all"),
		GNOME_STOCK_BUTTON_CLOSE,
		NULL
	};

	tl = gtk_type_new (c2_transfer_list_get_type ());

	GTK_BOX (GNOME_DIALOG (tl)->vbox)->spacing = 0;

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (tl)->vbox), vbox, TRUE, TRUE, 0);
	gtk_widget_show (vbox);
	tl->vbox = vbox;

	button = gtk_check_button_new_with_label (_("Close when finished."));
	gtk_box_pack_end (GTK_BOX (vbox), button, FALSE, TRUE, 0);
	gtk_widget_show (button);
	tl->close = button;
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button),
							c2_preferences_get_widget_transfer_list_autoclose ());
	gtk_signal_connect (GTK_OBJECT (button), "toggled",
						GTK_SIGNAL_FUNC (on_close_button_toggled), NULL);

	c2_dialog_construct (C2_DIALOG (tl), application, _("Send & Receive"),
						C2_WIDGET_TRANSFER_LIST_TYPE, PKGDATADIR "/pixmaps/send-receive.png", buttons);

	gnome_dialog_button_connect (GNOME_DIALOG (tl), 0, on_button0_clicked, tl);
	gnome_dialog_button_connect (GNOME_DIALOG (tl), 1, on_button1_clicked, tl);

	gtk_signal_connect (GTK_OBJECT (tl), "destroy",
						GTK_SIGNAL_FUNC (destroy), NULL);

	return GTK_WIDGET (tl);
}

static void
on_close_button_toggled (GtkWidget *widget, gpointer data)
{
	c2_preferences_set_widget_transfer_list_autoclose (GTK_TOGGLE_BUTTON (widget)->active);
	c2_preferences_commit ();
}

static void
on_button0_clicked (GtkWidget *widget, C2TransferList *tl)
{
	GSList *list;

	for (list = tl->list; list; list = g_slist_next (list))
	{
		C2TransferItem *tl = C2_TRANSFER_ITEM (list->data);
		
		if (!tl)
			break;

		gtk_signal_emit_by_name (GTK_OBJECT (tl->cancel_button), "clicked");
	}
}

static void
on_button1_clicked (GtkWidget *widget, C2TransferList *tl)
{
	gtk_widget_destroy (GTK_WIDGET (tl));
}

void
c2_transfer_list_add_item (C2TransferList *tl, C2TransferItem *ti)
{
	c2_return_if_fail (C2_IS_TRANSFER_ITEM (ti), C2EDATA);

	C2_PRINTD (MOD, "Account = '%s'\n", ti->account->name);

	/* If it a retrieve TI and we already have a retrieve TI for this
	 * account, remove the old one.
	 */
	if (ti->type == C2_TRANSFER_ITEM_RECEIVE)
	{
		GSList *l;

		for (l = tl->list; l != NULL; )
		{
			GSList *next = g_slist_next(l);
			C2TransferItem *cti = (C2TransferItem*) l->data;

			if (cti->type == C2_TRANSFER_ITEM_RECEIVE &&
				cti->account == ti->account)
			{
				tl->list = g_slist_remove (tl->list, cti);
				gtk_object_destroy (GTK_OBJECT (cti));
			}
			l = next;
		}
	}

	tl->list = g_slist_append (tl->list, ti);

	gtk_box_pack_start (GTK_BOX (tl->vbox), ti->event, FALSE, TRUE, 0);

	gtk_object_set_data (GTK_OBJECT (ti), "transfer list", tl);
	gtk_signal_connect (GTK_OBJECT (ti), "finish",
						GTK_SIGNAL_FUNC (on_transfer_item_finish), tl);
}

static gint
on_last_finish_timeout (GtkWidget *widget)
{
	C2TransferList *tl = C2_TRANSFER_LIST (widget);
	
	gtk_widget_destroy (widget);
	
	return FALSE;
}

static void
on_transfer_item_finish (C2TransferItem *ti, C2TransferList *tl)
{
	GtkWidget *progress;
	GSList *list;

	if (GTK_TOGGLE_BUTTON (tl->close)->active && g_slist_length (tl->list) == 1)
		gtk_timeout_add (2500, (GtkFunction) on_last_finish_timeout, tl);
	else
	{
		printf ("<%d>\n", g_slist_length (tl->list));
		tl->list = g_slist_remove (tl->list, ti);
		printf ("<%d>\n", g_slist_length (tl->list));
	}
	
	progress = ti->progress_byte;
	
	gtk_widget_hide (progress);

	progress = ti->progress_mail;
	
	gtk_progress_set_percentage (GTK_PROGRESS (progress), 1.0);
	gtk_widget_set_sensitive (ti->cancel_button, FALSE);

	if (!g_slist_length (tl->list))
	{
		printf ("Emitiendo la señal de desconexión %d\n", __LINE__);
		gtk_signal_emit (GTK_OBJECT (tl), signals[FINISH]);
	}
}
