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

#define MAX_CHARS_IN_LABEL		25

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

static GtkObjectClass *parent_class = NULL;

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

		type = gtk_type_unique (gtk_object_get_type (), &info);
	}

	return type;
}

static void
class_init (C2TransferItemClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *) klass;

	parent_class = gtk_type_class (gtk_object_get_type ());

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

C2TransferItem *
c2_transfer_item_new (C2Account *account, C2TransferItemType type, ...)
{
	C2TransferItem *ti;
	va_list args;

	ti = gtk_type_new (c2_transfer_item_get_type ());

	va_start (args, type);
	c2_transfer_item_construct (ti, account, type, args);
	va_end (args);

	return ti;
}

void
c2_transfer_item_construct (C2TransferItem *ti, C2Account *account, C2TransferItemType type, va_list args)
{
	GtkTooltips *tooltips;
	GtkWidget *label, *button, *pixmap, *box;
	gchar *buffer, *subject;
	
	ti->type = type;

	/* Store the extra-information */
	if (type == C2_TRANSFER_ITEM_RECEIVE)
	{
	} else if (type == C2_TRANSFER_ITEM_SEND)
	{
		ti->type_info.send.smtp = va_arg (args, C2SMTP *);
		ti->type_info.send.message = va_arg (args, C2Message *);
	} else
		g_assert_not_reached ();

	tooltips = gtk_tooltips_new ();

	if (type == C2_TRANSFER_ITEM_RECEIVE)
	{
		pixmap = gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/receive.png");
		buffer = g_strdup_printf (_("Receive «%s»"), account->name);
	} else
	{
		pixmap = gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/send.png");

		if (ti->type_info.send.message)
			subject = c2_message_get_header_field (ti->type_info.send.message, "Subject:");
		else
			subject = NULL;
		if (!subject)
			subject = g_strdup (_("This is a mail with a really long subject"));
		buffer = g_strdup_printf (_("Send «%s»"), subject);
		g_free (subject);
	}
	
	gtk_widget_show (pixmap);
	ti->c1 = pixmap;
	
	if (strlen (buffer) > MAX_CHARS_IN_LABEL)
	{
		gchar *short_buffer = g_new0 (gchar, MAX_CHARS_IN_LABEL);
		strncpy (short_buffer, buffer, MAX_CHARS_IN_LABEL-3);
		strcat (short_buffer, "...");
		g_free (buffer);
		buffer = short_buffer;
	}
	
	label = gtk_label_new (buffer);
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_widget_show (label);
	ti->c2 = label;

	box = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (box);
	ti->c3 = box;

	ti->progress_mail = gtk_progress_bar_new ();
	gtk_box_pack_start (GTK_BOX (box), ti->progress_mail, FALSE, TRUE, 0);
	gtk_widget_show (ti->progress_mail);

	ti->progress_byte = gtk_progress_bar_new ();
	gtk_box_pack_start (GTK_BOX (box), ti->progress_byte, FALSE, TRUE, 0);
	gtk_widget_show (ti->progress_byte);
	gtk_widget_set_usize (ti->progress_byte, -1, 5);

	button = gtk_button_new ();

	pixmap = gnome_stock_pixmap_widget_new (GTK_WIDGET (ti), GNOME_STOCK_PIXMAP_STOP);
	gtk_container_add (GTK_CONTAINER (button), pixmap);
	gtk_widget_show (pixmap);

	gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
	gtk_tooltips_set_tip (tooltips, button, _("Cancel the transfer"), NULL);
	gtk_widget_show (button);

	ti->c4 = button;
}

#if 0
void
c2_transfer_item_start (C2TransferItem *ti)
{
	if (ti->type == C2_TRANSFER_ITEM_RECEIVE)
	{
		c2_account_
}
#endif
