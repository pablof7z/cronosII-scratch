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
#include <pthread.h>

#include <libcronosII/pop3.h>
#include <libcronosII/error.h>

#include "main.h"
#include "preferences.h"
#include "widget-transfer-item.h"
#include "widget-transfer-list.h"

/* [TODO]
 * 20011102 Make the width dynamically.
 */

#define MOD		"Widget Transfer Item"
#ifdef USE_DEBUG
#	define DMOD	TRUE
#else
#	define DMOD	FALSE
#endif

#define MAX_CHARS_IN_LABEL		33
#define WIDGET_WIDTH			210

static void
class_init									(C2TransferItemClass *klass);

static void
init										(C2TransferItem *ti);

static void
destroy										(GtkObject *object);

static void
on_enter_notify_event						(GtkWidget *table, GdkEventCrossing *e, C2TransferItem *ti);

static void
on_leave_notify_event						(GtkWidget *table, GdkEventCrossing *e, C2TransferItem *ti);

static void
on_cancel_clicked							(GtkWidget *button, C2TransferItem *ti);

static void
on_pop3_resolve								(GtkObject *object, C2NetObjectByte *byte, C2TransferItem *ti);

static void
on_pop3_login								(GtkObject *object, C2TransferItem *ti);

static gboolean
on_pop3_login_failed						(C2POP3 *pop3, const gchar *error, gchar **user,
											 gchar **pass, C2Mutex *mutex);

static void
on_pop3_uidl								(GtkObject *object, gint nth, gint mails, C2TransferItem *ti);

static void
on_pop3_status								(GtkObject *object, gint mails, C2TransferItem *ti);

static void
on_pop3_retrieve							(GtkObject *object, gint16 nth, gint32 received,
											 gint32 total, C2TransferItem *ti);

static void
on_pop3_synchronize							(GtkObject *object, gint nth, gint mails, C2TransferItem *ti);

static void
on_pop3_disconnect							(GtkObject *object, C2NetObjectByte *byte, gboolean success,
											 C2TransferItem *ti);

static void
on_smtp_smtp_update							(C2SMTP *smtp, gint id,
											 guint length, guint bytes, C2TransferItem *ti);

static void
on_smtp_finished							(C2SMTP *smtp, gint id,
											 gboolean success, C2TransferItem *ti);

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
	ti->popup = NULL;
	ti->popup_label = NULL;
	ti->popup_progress = NULL;
	ti->popup_label_content = NULL;
	ti->mouse_is_in_popup = 0;
	ti->account = NULL;
	ti->application = NULL;
	ti->finished = 0;
}

static void
destroy (GtkObject *object)
{
	C2TransferItem *ti = C2_TRANSFER_ITEM (object);

	if (GTK_IS_WIDGET (ti->popup))
		gtk_widget_destroy (ti->popup);

	if (GTK_IS_WIDGET (ti->event))
		gtk_widget_destroy (ti->event);

	if (GTK_IS_WIDGET (ti->table))
		gtk_widget_destroy (ti->table);
}

C2TransferItem *
c2_transfer_item_new (C2Application *application, C2Account *account, C2TransferItemType type, ...)
{
	C2TransferItem *ti;
	va_list args;

	ti = gtk_type_new (c2_transfer_item_get_type ());

	va_start (args, type);
	c2_transfer_item_construct (ti, application, account, type, args);
	va_end (args);
	gtk_signal_connect (GTK_OBJECT (ti), "destroy",
						GTK_SIGNAL_FUNC (destroy), NULL);

#ifdef USE_DEBUG
	if (_debug_widget_transfer_item)
		C2_PRINTD (MOD, "Account = '%s' -- type = '%d'\n", account->name, type);
#endif

	return ti;
}

void
c2_transfer_item_construct (C2TransferItem *ti, C2Application *application, C2Account *account,
C2TransferItemType type, va_list args)
{
	GtkTooltips *tooltips;
	GtkWidget *label, *button, *pixmap, *box, *bpixmap;
	gchar *buffer, *subject;
	
	ti->account = account;
	ti->type = type;
	ti->application = application;

	/* Store the extra-information */
	if (type == C2_TRANSFER_ITEM_RECEIVE)
	{
		ti->type_info.receive.mails_r = 0;
	} else if (type == C2_TRANSFER_ITEM_SEND)
	{
		ti->type_info.send.smtp = va_arg (args, C2SMTP *);
		ti->type_info.send.db = va_arg (args, C2Db *);
	} else
		g_assert_not_reached ();

	ti->event = gtk_event_box_new ();
	gtk_widget_show (ti->event);
	gtk_signal_connect (GTK_OBJECT (ti->event), "enter_notify_event",
						GTK_SIGNAL_FUNC (on_enter_notify_event), ti);
	gtk_signal_connect (GTK_OBJECT (ti->event), "leave_notify_event",
						GTK_SIGNAL_FUNC (on_leave_notify_event), ti);

	ti->table = gtk_table_new (1, 4, FALSE);
	gtk_container_add (GTK_CONTAINER (ti->event), ti->table);
	gtk_table_set_col_spacings (GTK_TABLE (ti->table), 4);
	gtk_widget_show (ti->table);

	tooltips = gtk_tooltips_new ();

	if (type == C2_TRANSFER_ITEM_RECEIVE)
	{
		pixmap = gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/receive.png");
		buffer = g_strdup_printf (_("Receive «%s»"), account->name);
	} else
	{
		pixmap = gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/send.png");

		if (ti->type_info.send.db->message)
			subject = c2_message_get_header_field (ti->type_info.send.db->message, "Subject:");
		else
			subject = g_strdup ("");
		buffer = g_strdup_printf (_("Send «%s»"), subject);
		g_free (subject);
	}
	
	gtk_widget_show (pixmap);
	
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
	gtk_widget_set_usize (label, WIDGET_WIDTH, -1);

	box = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (box);

	ti->progress_mail = gtk_progress_bar_new ();
	gtk_box_pack_start (GTK_BOX (box), ti->progress_mail, TRUE, TRUE, 0);
	gtk_widget_show (ti->progress_mail);

	ti->progress_byte = gtk_progress_bar_new ();
	gtk_box_pack_start (GTK_BOX (box), ti->progress_byte, FALSE, TRUE, 0);
	gtk_widget_set_usize (ti->progress_byte, -1, 6);

	button = gtk_button_new ();

	bpixmap = gnome_stock_pixmap_widget_new (ti->table, GNOME_STOCK_PIXMAP_STOP);
	gtk_container_add (GTK_CONTAINER (button), bpixmap);
	gtk_widget_show (bpixmap);

	gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
	gtk_tooltips_set_tip (tooltips, button, _("Cancel the transfer"), NULL);
	gtk_widget_show (button);
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
						GTK_SIGNAL_FUNC (on_cancel_clicked), ti);
	ti->cancel_button = button;

	gtk_table_attach (GTK_TABLE (ti->table), pixmap, 0, 1, 0, 1, 0, 0, 0, 0);
	gtk_table_attach (GTK_TABLE (ti->table), label, 1, 2, 0, 1, GTK_FILL, 0, 0, 0);
	gtk_table_attach (GTK_TABLE (ti->table), box, 2, 3, 0, 1, 0, 0, 0, 0);
	gtk_table_attach (GTK_TABLE (ti->table), button, 3, 4, 0, 1, 0, 0, 0, 0);
}

static void
popup_set_label (C2TransferItem *ti, const gchar *label)
{
	gchar *buf;

	buf = c2_str_wrap (label, 45);
	label = buf;

	ti->popup_label_content = label;

	if (GTK_IS_LABEL (ti->popup_label))
		gtk_label_set_text (GTK_LABEL (ti->popup_label), label);
}

static void
popup_set_progress (C2TransferItem *ti, const gchar *label, gfloat value, gfloat min, gfloat max)
{
	if (label)
		ti->popup_progress_content = label;
	if (min > -2)
		ti->popup_progress_min = min;
	if (value > -2)
		ti->popup_progress_value = value;
	if (max > -2)
		ti->popup_progress_max = max;
	
	if (GTK_IS_PROGRESS (ti->popup_progress))
	{
		if (label)
			gtk_progress_set_format_string (GTK_PROGRESS (ti->popup_progress), label);

		if (min == -1)
			gtk_progress_set_percentage (GTK_PROGRESS (ti->popup_progress), value);
		else if (min > -1)
			gtk_progress_configure (GTK_PROGRESS (ti->popup_progress), value, min, max);
		else if (value > -1)
			gtk_progress_set_value (GTK_PROGRESS (ti->popup_progress), value);
	}
}

static void
on_popup_enter_notify_event (GtkWidget *popup, GdkEventCrossing *e, C2TransferItem *ti)
{
	ti->mouse_is_in_popup = 1;
}

static void
on_popup_leave_notify_event (GtkWidget *popup, GdkEventCrossing *e, C2TransferItem *ti)
{
	ti->mouse_is_in_popup = 0;
}

static void
on_enter_notify_event (GtkWidget *table, GdkEventCrossing *e, C2TransferItem *ti)
{
	GtkWidget *viewport;
	GtkWidget *table;
	GtkWidget *pixmap;
	GtkWidget *label;
	GtkWidget *tl;
	GtkWidget *event, *event2;
	GtkWidget *box;
	gint x, y;
	gint mx, my;
	gchar *buffer, *subject;
	GtkStyle *style;
	GdkColor blue = { 0, 0x6b00, 0x5800, 0xee00 };

	/* Check if we already have the popup */
	if (GTK_IS_WINDOW (ti->popup) && ti->popup->window)
		return;

	ti->popup = gtk_window_new (GTK_WINDOW_POPUP);
	gtk_widget_set_events (ti->popup, GDK_POINTER_MOTION_MASK);
	gtk_widget_set_extension_events (ti->popup, GDK_EXTENSION_EVENTS_CURSOR);
	gtk_signal_connect (GTK_OBJECT (ti->popup), "enter_notify_event",
							GTK_SIGNAL_FUNC (on_popup_enter_notify_event), ti);
	gtk_signal_connect (GTK_OBJECT (ti->popup), "leave_notify_event",
							GTK_SIGNAL_FUNC (on_popup_leave_notify_event), ti);
	style = gtk_style_copy (gtk_widget_get_style (ti->popup));
	style->bg[GTK_STATE_NORMAL] = style->black;
	gtk_widget_set_style (ti->popup, style);
	gtk_container_set_border_width (GTK_CONTAINER (ti->popup), 1);

	event = gtk_event_box_new ();
	gtk_container_add (GTK_CONTAINER (ti->popup), event);
	gtk_widget_show (event);

	viewport = gtk_frame_new (NULL);
	gtk_container_add (GTK_CONTAINER (event), viewport);
	gtk_widget_show (viewport);
	gtk_frame_set_shadow_type (GTK_FRAME (viewport), GTK_SHADOW_ETCHED_OUT);

	table = gtk_table_new (2, 3, FALSE);
	gtk_container_add (GTK_CONTAINER (viewport), table);
	gtk_widget_show (table);
	gtk_container_set_border_width (GTK_CONTAINER (table), 3);

	/* Create the first line */
	if (ti->type == C2_TRANSFER_ITEM_RECEIVE)
	{
		pixmap = gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/receive.png");
		buffer = g_strdup_printf (_("Receive mails for account %s"), ti->account->name);
	} else
	{
		pixmap = gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/send.png");

		if (ti->type_info.send.db->message)
		{
			subject = c2_message_get_header_field (ti->type_info.send.db->message, "Subject:");
			buffer = g_strdup_printf (_("Send mail \"%s\" with account %s"), subject, ti->account->name);
		} else
		{
			subject = g_strdup ("");
			buffer = g_strdup_printf (_("Send mail with account %s"), ti->account->name);
		}
		
		g_free (subject);
	}

	event = gtk_event_box_new ();
	gtk_table_attach_defaults (GTK_TABLE (table), event, 1, 2, 0, 1);
	gtk_widget_show (event);
	style = gtk_style_copy (gtk_widget_get_style (event));
	style->bg[GTK_STATE_NORMAL] = style->white;
	gtk_widget_set_style (event, style);
	gtk_container_set_border_width (GTK_CONTAINER (event), 2);

	gdk_color_alloc (gdk_colormap_get_system (), &blue);
	event2 = gtk_event_box_new ();
	gtk_container_add (GTK_CONTAINER (event), event2);
	gtk_widget_show (event2);
	style = gtk_style_copy (gtk_widget_get_style (event2));
	style->bg[GTK_STATE_NORMAL] = blue;
	gtk_widget_set_style (event2, style);
	gtk_container_set_border_width (GTK_CONTAINER (event2), 1);

	box = gtk_hbox_new (FALSE, 2);
	gtk_container_add (GTK_CONTAINER (event2), box);
	gtk_widget_show (box);
	gtk_container_set_border_width (GTK_CONTAINER (box), 1);

	gtk_box_pack_start (GTK_BOX (box), pixmap, FALSE, FALSE, 0);
	gtk_widget_show (pixmap);

	label = gtk_label_new (buffer);
	gtk_box_pack_start (GTK_BOX (box), label, TRUE, TRUE, 0);
	gtk_widget_show (label);
	style = gtk_style_copy (gtk_widget_get_style (label));
	style->font = gdk_font_load (c2_font_bold);
	style->fg[GTK_STATE_NORMAL] = style->white;
	gtk_widget_set_style (label, style);

	/* Create the progress bar */
	ti->popup_progress = gtk_progress_bar_new ();
	gtk_table_attach_defaults (GTK_TABLE (table), ti->popup_progress, 0, 2, 1, 2);
	gtk_widget_show (ti->popup_progress);
	gtk_progress_set_show_text (GTK_PROGRESS (ti->popup_progress), TRUE);
	
	buffer = gtk_progress_get_current_text (GTK_PROGRESS (ti->progress_mail));
	gtk_progress_set_format_string (GTK_PROGRESS (ti->popup_progress), buffer);
	g_free (buffer);

	gtk_progress_configure (GTK_PROGRESS (ti->popup_progress), ti->popup_progress_value,
				ti->popup_progress_min, ti->popup_progress_max);

	/* Create the label */
	event = gtk_event_box_new ();
	gtk_table_attach_defaults (GTK_TABLE (table), event, 0, 2, 2, 3);
	gtk_widget_show (event);
	style = gtk_style_copy (gtk_widget_get_style (event));
	style->bg[GTK_STATE_NORMAL] = style->black;
	gtk_widget_set_style (event, style);
	gtk_container_set_border_width (GTK_CONTAINER (event), 2);
	
	label = gtk_event_box_new ();
	gtk_container_add (GTK_CONTAINER (event), label);
	event = label;
	gtk_widget_show (event);
	style = gtk_style_copy (gtk_widget_get_style (event));
	style->bg[GTK_STATE_NORMAL] = style->white;
	gtk_widget_set_style (event, style);
	gtk_container_set_border_width (GTK_CONTAINER (event), 1);

	ti->popup_label = gtk_label_new (ti->popup_progress_content);
	gtk_container_add (GTK_CONTAINER (event), ti->popup_label);
	gtk_widget_show (ti->popup_label);
	style = gtk_style_copy (gtk_widget_get_style (ti->popup_label));
	style->font = gdk_font_load (c2_font_italic);
	gtk_widget_set_style (ti->popup_label, style);
	
	tl = GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (ti), "transfer list"));
	gdk_window_get_position (tl->window, &x, &y);
	gtk_widget_get_pointer (tl, &mx, &my);
	gtk_widget_realize (ti->popup);

	if (x-ti->popup->allocation.width+5 < 0)
		x = ti->popup->allocation.width-5;
	gtk_widget_popup (ti->popup, x-ti->popup->allocation.width+5, y+my);
}

static gint
on_leave_notify_event_timeout (C2TransferItem *ti)
{
	if (ti->mouse_is_in_popup)
	{
		gtk_timeout_add (500, on_leave_notify_event_timeout, ti);
		return FALSE;
	}

	gtk_widget_destroy (ti->popup);
	ti->popup = NULL;

	return FALSE;
}

static void
on_leave_notify_event (GtkWidget *table, GdkEventCrossing *e, C2TransferItem *ti)
{
	/* We have to give some time so the enter_notify_event of the popup
	 * might be emitted if needed.
	 */
	gtk_timeout_add (10, on_leave_notify_event_timeout, ti);
}

static void
on_cancel_clicked (GtkWidget *button, C2TransferItem *ti)
{
	C2Account *account = ti->account;
	
	ti->finished = 1;
	gtk_signal_emit (GTK_OBJECT (ti), signals[CANCEL], account);

	if (account->type == C2_ACCOUNT_POP3)
	{
		C2POP3 *pop3;

		pop3 = C2_POP3 (c2_account_get_extra_data (account, C2_ACCOUNT_KEY_INCOMING, NULL));
		c2_pop3_cancel (pop3);
	}

	gtk_progress_set_format_string (GTK_PROGRESS (ti->progress_mail), _("Cancelling"));
	if (GTK_IS_PROGRESS (ti->popup_progress))
	{
		gtk_progress_set_format_string (GTK_PROGRESS (ti->popup_progress), _("Cancelling"));
	}

	popup_set_label (ti, _("The action is being cancelled by your petission, hold on..."));
}

static void
fire_c2_account_check_in_its_own_thread (C2Account *account)
{
	c2_account_check (account);
}

static void
c2_transfer_item_start_pop3_thread (C2Pthread3 *data)
{
	C2POP3 *pop3 = C2_POP3 (data->v1);
	C2Account *account = C2_ACCOUNT (data->v2);
	C2Mailbox *inbox = C2_MAILBOX (data->v3);

	g_free (data);
	
	c2_pop3_fetchmail (pop3, account, inbox);
}

static gint
smtp_get_id_from_ti (C2TransferItem *ti)
{
	C2TransferList *tl;
	GSList *l;
	gint id = 1;

	tl = C2_TRANSFER_LIST (gtk_object_get_data (GTK_OBJECT (ti), "transfer list"));

	for (l = tl->list; l; l = g_slist_next (l))
	{
		C2TransferItem *lti = C2_TRANSFER_ITEM (l->data);

		if (lti->type == C2_TRANSFER_ITEM_SEND)
			id = lti->type_info.send.id+1;
	}

	return id;
}

static C2TransferItem *
smtp_get_ti_from_id (C2TransferItem *ti, gint id)
{
	C2TransferList *tl;
	GSList *l;

	tl = C2_TRANSFER_LIST (gtk_object_get_data (GTK_OBJECT (ti), "transfer list"));

	for (l = tl->list; l; l = g_slist_next (l))
	{
		C2TransferItem *lti = C2_TRANSFER_ITEM (l->data);

		if (id == lti->type_info.send.id)
			return lti;
	}

	return NULL;
}

static void
c2_transfer_item_start_smtp_thread (C2TransferItem *ti)
{
	gint id;

	id = smtp_get_id_from_ti (ti);
	ti->type_info.send.id = id;
	c2_smtp_send_message (ti->type_info.send.smtp, ti->type_info.send.db->message, id);
}

void
c2_transfer_item_start (C2TransferItem *ti)
{
	pthread_t thread;

	if (ti->type == C2_TRANSFER_ITEM_RECEIVE)
	{
		if (ti->account->type == C2_ACCOUNT_POP3)
		{
			C2POP3 *pop3 = C2_POP3 (c2_account_get_extra_data (ti->account, C2_ACCOUNT_KEY_INCOMING, NULL));
			C2Pthread3 *data;
			C2Mailbox *inbox;

			if (!(inbox = c2_mailbox_get_by_usage (ti->application->mailbox, C2_MAILBOX_USE_AS_INBOX)))
			{
				/* There's no Inbox mailbox, create it */
				ti->application->mailbox = c2_mailbox_new_with_parent (
											&ti->application->mailbox, C2_MAILBOX_INBOX, NULL,
											C2_MAILBOX_CRONOSII, 0, 0);
				if (!(inbox = ti->application->mailbox))
				{
					gtk_progress_set_show_text (GTK_PROGRESS (ti->progress_mail), TRUE);
					gtk_progress_set_format_string (GTK_PROGRESS (ti->progress_mail),
									_("No Inbox mailbox"));
					if (GTK_IS_PROGRESS (ti->popup_progress))
					{
						gtk_progress_set_format_string (GTK_PROGRESS (ti->popup_progress), _("No Inbox mailbox"));
					}
					popup_set_label (ti, _("There is no mailbox marked to be use as Inbox, thus "
											   "there is no place where to save mails downloaded.\n"
											   "The action will be cancelled now.\n"
											   "Before checking again make sure you mark mailbox as Inbox.\n"
											   "\n"
											   "For more information read the Cronos II User Manual."));
					on_pop3_disconnect (NULL, FALSE, NULL, ti);
					return;
				}
				gtk_signal_emit_by_name (GTK_OBJECT (inbox), "changed_mailboxes");
			}
			
			gtk_signal_connect (GTK_OBJECT (pop3), "resolve",
								GTK_SIGNAL_FUNC (on_pop3_resolve), ti);

			gtk_signal_connect (GTK_OBJECT (pop3), "login",
								GTK_SIGNAL_FUNC (on_pop3_login), ti);

			gtk_object_set_data (GTK_OBJECT (pop3), "login_failed::data", ti);
			C2_POP3_CLASS_FW (pop3)->login_failed = on_pop3_login_failed;

			gtk_signal_connect (GTK_OBJECT (pop3), "uidl",
								GTK_SIGNAL_FUNC (on_pop3_uidl), ti);

			gtk_signal_connect (GTK_OBJECT (pop3), "status",
								GTK_SIGNAL_FUNC (on_pop3_status), ti);

			gtk_signal_connect (GTK_OBJECT (pop3), "retrieve",
								GTK_SIGNAL_FUNC (on_pop3_retrieve), ti);

			gtk_signal_connect (GTK_OBJECT (pop3), "synchronize",
								GTK_SIGNAL_FUNC (on_pop3_synchronize), ti);

			gtk_signal_connect (GTK_OBJECT (pop3), "disconnect",
								GTK_SIGNAL_FUNC (on_pop3_disconnect), ti);

			gtk_progress_set_show_text (GTK_PROGRESS (ti->progress_mail), TRUE);
			gtk_progress_set_format_string (GTK_PROGRESS (ti->progress_mail), _("Resolving"));
			if (GTK_IS_PROGRESS (ti->popup_progress))
			{
				gtk_progress_set_format_string (GTK_PROGRESS (ti->popup_progress), _("Resolving"));
			}
			popup_set_label (ti, _("Cronos II is looking up for the server that this account "
									   "is associated to."));

			data = g_new0 (C2Pthread3, 1);
			data->v1 = (gpointer) pop3;
			data->v2 = (gpointer) ti->account;
			data->v3 = (gpointer) inbox;
			pthread_create (&thread, NULL, C2_PTHREAD_FUNC (c2_transfer_item_start_pop3_thread), data);
		} else if (ti->account->type == C2_ACCOUNT_IMAP)
		{
			/* Nothing to do since IMAP accounts are not
			 * checked like accounts but as mailboxes. */
		} else
		{
#ifdef USE_DEBUG
			printf ("%d\n", ti->account->type);
			g_assert_not_reached ();
#endif
			return;
		}
	} else if (ti->type == C2_TRANSFER_ITEM_SEND)
	{
		pthread_t thread;

		/*gtk_signal_connect (GTK_OBJECT (ti->type_info.send.smtp), "resolve",
							GTK_SIGNAL_FUNC (on_smtp_resolve), ti);*/
		gtk_signal_connect (GTK_OBJECT (ti->type_info.send.smtp), "smtp_update",
							GTK_SIGNAL_FUNC (on_smtp_smtp_update), ti);
		gtk_signal_connect (GTK_OBJECT (ti->type_info.send.smtp), "finished",
							GTK_SIGNAL_FUNC (on_smtp_finished), ti);

		gtk_progress_set_show_text (GTK_PROGRESS (ti->progress_mail), TRUE);
		gtk_progress_set_format_string (GTK_PROGRESS (ti->progress_mail), _("Resolving"));
		if (GTK_IS_PROGRESS (ti->popup_progress))
		{
			gtk_progress_set_format_string (GTK_PROGRESS (ti->popup_progress), _("Resolving"));
		}
		popup_set_label (ti, _("Cronos II is looking up for the server that this account "
									   "is associated to."));
		
		pthread_create (&thread, NULL, C2_PTHREAD_FUNC (c2_transfer_item_start_smtp_thread), ti);
	} else
		g_assert_not_reached ();
}

static void
on_pop3_resolve (GtkObject *object, C2NetObjectByte *byte, C2TransferItem *ti)
{
	gdk_threads_enter ();
	gtk_progress_set_format_string (GTK_PROGRESS (ti->progress_mail), _("Connecting"));
	if (GTK_IS_PROGRESS (ti->popup_progress))
	{
		gtk_progress_set_format_string (GTK_PROGRESS (ti->popup_progress), _("Connecting"));
	}
	popup_set_label (ti, _("Cronos II is trying to connect to the server in order to initialize "
							   "the transaction of e-mails."));
	gdk_threads_leave ();
}

static void
on_pop3_login (GtkObject *object, C2TransferItem *ti)
{
	gdk_threads_enter ();
	/* [FIXME]
	 * Action to login? Loggining?
	 */
	gtk_progress_set_format_string (GTK_PROGRESS (ti->progress_mail), _("Loggining"));
	if (GTK_IS_PROGRESS (ti->popup_progress))
	{
		gtk_progress_set_format_string (GTK_PROGRESS (ti->popup_progress), _("Loggining"));
	}
	popup_set_label (ti, _("Cronos II is logging into your account in the server."));
	gdk_threads_leave ();
}

static void
on_pop3_login_failed_ok_clicked (GtkWidget *widget, C2Pthread2 *data)
{
	C2Mutex *lock = (C2Mutex*) data->v1;
	
	GPOINTER_TO_INT (data->v2) = 0;
	
	c2_mutex_unlock (lock);
}

static void
on_pop3_login_failed_cancel_clicked (GtkWidget *widget, C2Pthread2 *data)
{
	C2Mutex *lock = (C2Mutex*) data->v1;
	
	GPOINTER_TO_INT (data->v2) = 1;
	
	c2_mutex_unlock (lock);
}

static gboolean
on_pop3_login_failed_dialog_delete_event (GtkWidget *widget, GdkEvent *e, C2Pthread2 *data)
{
	C2Mutex *lock = (C2Mutex*) data->v1;
	
	GPOINTER_TO_INT (data->v2) = 1;
	
	c2_mutex_unlock (lock);
	
	return FALSE;
}

static gboolean
on_pop3_login_failed (C2POP3 *pop3, const gchar *error, gchar **user, gchar **pass, C2Mutex *mutex)
{
	GladeXML *xml;
	GtkWidget *dialog;
	GtkWidget *contents;
	C2TransferItem *ti;
	C2Account *account;
	C2Pthread2 *data;
	C2Mutex local_lock;
	gchar *label, *buffer;
	gint button;

	account = C2_ACCOUNT (gtk_object_get_data (GTK_OBJECT (pop3), "account"));
	data = g_new0 (C2Pthread2, 1);
	c2_mutex_init (&local_lock);
	c2_mutex_lock (&local_lock);
	data->v1 = (gpointer) &local_lock;
	
	gdk_threads_enter ();
	ti = C2_TRANSFER_ITEM (gtk_object_get_data (GTK_OBJECT (pop3), "login_failed::data"));
	xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("dialogs"), "dlg_req_pass_contents");
	contents = glade_xml_get_widget (xml, "dlg_req_pass_contents");
	dialog = c2_dialog_new (ti->application, _("Login failed"), "req_pass", NULL,
							GNOME_STOCK_BUTTON_OK, GNOME_STOCK_BUTTON_CANCEL, NULL);
	C2_DIALOG (dialog)->xml = xml;
	gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (dialog)->vbox), contents, TRUE, TRUE, 0);
	
	contents = glade_xml_get_widget (xml, "error_label");
	gtk_label_get (GTK_LABEL (contents), &buffer);
	label = g_strdup_printf ("%s '%s'.", buffer, error);
	gtk_label_set_text (GTK_LABEL (contents), label);
	
	contents = glade_xml_get_widget (xml, "account");
	gtk_entry_set_text (GTK_ENTRY (contents), account->name);
	contents = glade_xml_get_widget (xml, "user");
	gtk_entry_set_text (GTK_ENTRY (contents), pop3->user);
	contents = glade_xml_get_widget (xml, "pass");
	gtk_entry_set_text (GTK_ENTRY (contents), pop3->pass);
	gtk_widget_grab_focus (contents);
	
	gnome_dialog_button_connect (GNOME_DIALOG (dialog), 0,
								GTK_SIGNAL_FUNC (on_pop3_login_failed_ok_clicked), data);
	gnome_dialog_button_connect (GNOME_DIALOG (dialog), 1,
								GTK_SIGNAL_FUNC (on_pop3_login_failed_cancel_clicked), data);
	gtk_signal_connect (GTK_OBJECT (dialog), "delete_event",
								GTK_SIGNAL_FUNC (on_pop3_login_failed_dialog_delete_event), data);
	gtk_widget_show (dialog);
	
	gdk_threads_leave ();
	
	c2_mutex_lock (&local_lock);
	c2_mutex_unlock (&local_lock);
	c2_mutex_destroy (&local_lock);
	
	button = GPOINTER_TO_INT (data->v2);
	gdk_threads_enter ();
	if (!button)
	{
		gint nth;
		
		contents = glade_xml_get_widget (xml, "user");
		*user = gtk_entry_get_text (GTK_ENTRY (contents));
		contents = glade_xml_get_widget (xml, "pass");
		*pass = gtk_entry_get_text (GTK_ENTRY (contents));
		
		nth = c2_account_get_position (ti->application->account, account);
		buffer = g_strdup_printf ("/"PACKAGE"/Account %d/", nth);
		gnome_config_push_prefix (buffer);
		gnome_config_set_string ("incoming_server_username", *user);
		if (c2_pop3_get_save_password (pop3))
			gnome_config_set_string ("incoming_server_password", *pass);
		gnome_config_pop_prefix ();
		gnome_config_sync ();
		g_free (buffer);
	}
	
	gtk_widget_destroy (dialog);
	gdk_threads_leave ();
	c2_mutex_unlock (mutex);
	
	return !button;
}

static void
on_pop3_uidl (GtkObject *object, gint nth, gint mails, C2TransferItem *ti)
{
#ifdef USE_DEBUG
	if (_debug_widget_transfer_item)
		C2_PRINTD (MOD, "POP3 UIDL: %d/%d (%s)\n", nth, mails, ti->account->name);
#endif

	gdk_threads_enter ();
	if (!nth)
	{
		gtk_progress_configure (GTK_PROGRESS (ti->progress_mail), 0, 0, mails);
		gtk_progress_set_format_string (GTK_PROGRESS (ti->progress_mail), _("Getting mail list"));
		if (GTK_IS_PROGRESS (ti->popup_progress))
		{
			gtk_progress_configure (GTK_PROGRESS (ti->popup_progress), 0, 0, mails);
			gtk_progress_set_format_string (GTK_PROGRESS (ti->popup_progress), _("Getting mail list"));
		}
		popup_set_label (ti, _("Cronos II is retrieving a list of e-mails from the server "
								   "to learn the mails that should be downloaded from it."));
	}

	gtk_progress_set_value (GTK_PROGRESS (ti->progress_mail), nth);
	if (GTK_IS_PROGRESS (ti->popup_progress))
		gtk_progress_set_value (GTK_PROGRESS (ti->popup_progress), nth);
	gdk_threads_leave ();

#ifdef USE_DEBUG
	if (_debug_widget_transfer_item)
		C2_PRINTD (MOD, "UIDL inform finished (%s)\n", ti->account->name);
#endif
}

static void
on_pop3_status (GtkObject *object, gint mails, C2TransferItem *ti)
{
	gchar *buf, *buf2;

#ifdef USE_DEBUG
	if (_debug_widget_transfer_item)
		C2_PRINTD (MOD, "%d mails to download (%s)\n", mails, ti->account->name);
#endif

	gdk_threads_enter ();
/*#ifdef USE_DEBUG
	if (_debug_widget_transfer_item)
		C2_PRINTD (MOD, "Entered in the GDK thread (%s)\n", ti->account->name);
#endif
	gtk_progress_configure (GTK_PROGRESS (ti->progress_mail), 0, 0, mails);

#ifdef USE_DEBUG
	if (_debug_widget_transfer_item)
		C2_PRINTD (MOD, "Configured the progress mail (%s)\n", mails, ti->account->name);
#endif
	popup_set_progress (ti, NULL, 0, 0, mails);
	
	ti->type_info.receive.mails_r = mails;

#ifdef USE_DEBUG
	if (_debug_widget_transfer_item)
		C2_PRINTD (MOD, "Progress bar updated (%s)\n", mails, ti->account->name);
#endif

	if (!mails)
	{
		gtk_progress_set_percentage (GTK_PROGRESS (ti->progress_mail), 1.0);
		popup_set_progress (ti, "", -1, -1, -1);
	} else
	{
		if (mails == 1)
		{
			gtk_progress_set_format_string (GTK_PROGRESS (ti->progress_mail), _("%u message."));
			popup_set_progress (ti, _("There is one new message to download."), -2, -2, -2);
			popup_set_label (ti, _("There is one new message to download."));
		} else
		{
			gtk_progress_set_format_string (GTK_PROGRESS (ti->progress_mail), _("%u messages."));
			popup_set_progress (ti, _("%u messages."), -2, -2, -2);
			
			buf = c2_application_str_number_to_string (mails);
			buf2 = g_strdup_printf (("There are %s new messages to download."), buf);
			popup_set_label (ti, buf2);
			g_free (buf);
		}
	
		gtk_progress_configure (GTK_PROGRESS (ti->progress_mail), 0, 0, mails);
		popup_set_progress (ti, NULL, 0, 0, mails);
	}*/
	gdk_threads_leave ();

#ifdef USE_DEBUG
	if (_debug_widget_transfer_item)
		C2_PRINTD (MOD, "Status inform finished (%s)\n", ti->account->name);
#endif
}

static void
on_pop3_retrieve (GtkObject *object, gint16 nth, gint32 received, gint32 total, C2TransferItem *ti)
{
	gchar *buf;

	gdk_threads_enter ();
	
	if (!received)
	{
		if (nth == 1)
		{
			gtk_progress_set_format_string (GTK_PROGRESS (ti->progress_mail), _("Receiving %v of %u"));
			popup_set_progress (ti, _("Receiving %v of %u"), -2, -2, -2);
		}
		
		gtk_progress_set_value (GTK_PROGRESS (ti->progress_mail), nth);
		popup_set_progress (ti, NULL, nth, -2, -2);
		
		gtk_widget_show (ti->progress_byte);
		gtk_progress_configure (GTK_PROGRESS (ti->progress_byte), 0, 0, total);
		buf = g_strdup_printf (_("The message %d is being downloaded"), nth);
		popup_set_label (ti, buf);
		g_free (buf);
	} else if (total)
	{
		gtk_progress_set_value (GTK_PROGRESS (ti->progress_byte), received);
		buf = g_strdup_printf ("The message %d is being downloaded (%d %%)", nth, (received*100)/total);
		popup_set_label (ti, buf);
		g_free (buf);
	} else
	{
		gtk_widget_hide (ti->progress_byte);
		buf = g_strdup_printf (_("The message %d is been downloaded."), nth);
		popup_set_label (ti, buf);
		g_free (buf);
	}
	gdk_threads_leave ();
}

static void
on_pop3_synchronize (GtkObject *object, gint nth, gint mails, C2TransferItem *ti)
{
	gdk_threads_enter ();
	if (!nth)
	{
		gtk_progress_configure (GTK_PROGRESS (ti->progress_mail), 0, 0, mails);
		gtk_progress_set_format_string (GTK_PROGRESS (ti->progress_mail), _("Synchronizing"));

		popup_set_progress (ti, _("Synchronizing"), 0, 0, mails);
		popup_set_label (ti, _("Cronos II is synchronizing the local information with the server information."));
	}

	gtk_progress_set_value (GTK_PROGRESS (ti->progress_mail), nth);
	popup_set_progress (ti, NULL, nth, -2, -2);

	gdk_threads_leave ();
}

static void
on_pop3_disconnect (GtkObject *object, C2NetObjectByte *byte, gboolean success, C2TransferItem *ti)
{
	C2Account *account = ti->account;
	C2POP3 *pop3 = C2_POP3 (c2_account_get_extra_data (account, C2_ACCOUNT_KEY_INCOMING, NULL));
	gchar *str = NULL;
	gchar *estr = NULL;

	gdk_threads_enter ();
	if (success)
	{
		/* In mail%s the %s is an s in case of plural (translators can change it). There is
		 * one more %s in case some language (like Spanish) need it. */
		if (ti->type_info.receive.mails_r)
		{
			str = g_strdup_printf (_("Received %d new mail%s"), ti->type_info.receive.mails_r,
					(ti->type_info.receive.mails_r>1)?_("s"):"", (ti->type_info.receive.mails_r>1)?_("s"):"");
			estr = g_strdup_printf (_("%d new mail%s have been downloaded from this account."),
									ti->type_info.receive.mails_r,
					(ti->type_info.receive.mails_r>1)?_("s"):"", (ti->type_info.receive.mails_r>1)?_("s"):"");
		
		} else
		{
			str = g_strdup_printf (_("No new mails available"), ti->type_info.receive.mails_r,
					(ti->type_info.receive.mails_r>1)?_("s"):"");
			estr = g_strdup_printf (_("There are no new mails in this account.\n"
									  "\n"
									  "This means that you didn't received any new mail "
									  "since the last time you've checked."));
		}
	} else
	{
		if (c2_error_object_get_id (object))
		{
			str = g_strdup_printf (_("Failure: %s"), c2_error_object_get (object));
			estr = g_strdup_printf (_("The retrieve of new mails has failed due to the following reason:\n%s"),
									c2_error_object_get (object));
		} else if (pop3->canceled)
		{
			str = g_strdup_printf (_("Failure: %s"), error_list[C2_CANCEL_USER]);
			estr = g_strdup_printf (_("The retrieve didn't complete because you cancelled it."));
		} else
		{
			str = g_strdup (_("Failure"));
			estr = g_strdup_printf (_("An unknown fail has occurred. Try again. If this problem persist "
									  "get in touch with the Cronos II Hackers Team."));
		}
	}
	gtk_progress_set_format_string (GTK_PROGRESS (ti->progress_mail), str);
	gtk_progress_set_percentage (GTK_PROGRESS (ti->progress_mail), 1.0);
	
	popup_set_progress (ti, str, -1, -1, -1);
	popup_set_label (ti, estr);
	g_free (estr);
	
	gtk_widget_set_sensitive (ti->cancel_button, FALSE);
	g_free (str);

	ti->finished = 1;

	/* Disconnect the signals that we connected to the POP3 object */
	gtk_signal_disconnect_by_func (GTK_OBJECT (pop3),
						GTK_SIGNAL_FUNC (on_pop3_resolve), ti);

	gtk_signal_disconnect_by_func (GTK_OBJECT (pop3),
									GTK_SIGNAL_FUNC (on_pop3_login), ti);

	gtk_object_set_data (GTK_OBJECT (pop3), "login_failed::data", NULL);
	C2_POP3_CLASS_FW (pop3)->login_failed = NULL;

	gtk_signal_disconnect_by_func (GTK_OBJECT (pop3),
						GTK_SIGNAL_FUNC (on_pop3_uidl), ti);

	gtk_signal_disconnect_by_func (GTK_OBJECT (pop3),
									GTK_SIGNAL_FUNC (on_pop3_status), ti);

	gtk_signal_disconnect_by_func (GTK_OBJECT (pop3),
									GTK_SIGNAL_FUNC (on_pop3_retrieve), ti);

	gtk_signal_disconnect_by_func (GTK_OBJECT (pop3),
									GTK_SIGNAL_FUNC (on_pop3_synchronize), ti);

	gtk_signal_disconnect_by_func (GTK_OBJECT (pop3),
									GTK_SIGNAL_FUNC (on_pop3_disconnect), ti);

	gtk_signal_emit (GTK_OBJECT (ti), signals[FINISH]);

	gdk_threads_leave ();
}

static void
on_smtp_smtp_update (C2SMTP *smtp, gint id, guint length, guint bytes, C2TransferItem *ti)
{
	C2TransferItem *_ti;
	
	gdk_threads_enter ();
	if (!(_ti = smtp_get_ti_from_id (ti, id)))
	{
		gdk_threads_leave ();
		return;
	}

	if (!bytes)
	{
		gtk_progress_configure (GTK_PROGRESS (ti->progress_mail), 0, 0, length);
		gtk_progress_set_format_string (GTK_PROGRESS (ti->progress_mail), _("Sending"));

		if (GTK_IS_PROGRESS (ti->popup_progress))
		{
			gtk_progress_configure (GTK_PROGRESS (ti->popup_progress), 0, 0, length);
			gtk_progress_set_format_string (GTK_PROGRESS (ti->popup_progress), _("Sending"));
		}

		popup_set_label (ti, _("Cronos II is sending the message."));
	}

	gtk_progress_set_value (GTK_PROGRESS (ti->progress_mail), bytes);

	if (GTK_IS_PROGRESS (ti->popup_progress))
		gtk_progress_set_value (GTK_PROGRESS (ti->popup_progress), bytes);

	gdk_threads_leave ();
}

static void
on_smtp_finished (C2SMTP *smtp, gint id, gboolean success, C2TransferItem *ti)
{
	C2TransferItem *_ti;
	C2Mailbox *outbox;
	C2Mailbox *sent_items;
	gchar *str, *estr;

	gdk_threads_enter ();

	gtk_signal_disconnect_by_func (GTK_OBJECT (smtp), GTK_SIGNAL_FUNC (on_smtp_smtp_update), ti);
	gtk_signal_disconnect_by_func (GTK_OBJECT (smtp), GTK_SIGNAL_FUNC (on_smtp_finished), ti);

	if (!(_ti = smtp_get_ti_from_id (ti, id)))
	{
		gdk_threads_leave ();
		return;
	}
	
	gtk_progress_set_format_string (GTK_PROGRESS (ti->progress_mail),
									success ? _("Message Sent") : _("Failed"));
	gtk_progress_set_percentage (GTK_PROGRESS (ti->progress_mail), 1.0);

	if (GTK_IS_PROGRESS (ti->popup_progress))
	{
		gtk_progress_set_format_string (GTK_PROGRESS (ti->popup_progress),
									success ? _("Message Sent") : _("Failed"));
		gtk_progress_set_percentage (GTK_PROGRESS (ti->popup_progress), 1.0);
	}

	if (success)
		popup_set_label (ti, _("The message was successfully sent to the specified destinataries."));
	else
	{
		estr = c2_error_object_get (GTK_OBJECT (smtp));
		if (!estr)
			estr = c2_error_get ();
		
		if (!estr)
			str = g_strdup_printf (_("The delivery of the message failed for an unknown reason."));
		else
			str = g_strdup_printf (_("The delivery of the message failed.\n%s."), estr);
		g_free (estr);

		popup_set_label (ti, str);
	}

	gtk_widget_set_sensitive (ti->cancel_button, FALSE);

	ti->finished = 1;
	gtk_signal_emit (GTK_OBJECT (ti), signals[FINISH]);

	if (!(outbox = c2_mailbox_get_by_usage (ti->application->mailbox,
											C2_MAILBOX_USE_AS_OUTBOX)))
	{
		g_warning (_("There's no mailbox marked as %s!\n"), C2_MAILBOX_OUTBOX);
		gdk_threads_leave ();
		return;
	}

	if (c2_preferences_get_general_options_outgoing_sent_items ())
	{
		/* Move the message from «Outbox» to «Sent Items» */
		if (!(sent_items = c2_mailbox_get_by_usage (ti->application->mailbox,
													C2_MAILBOX_USE_AS_SENT_ITEMS)))
		{
			g_warning (_("There's no mailbox marked as %s!\n"), C2_MAILBOX_SENT_ITEMS);

			gdk_threads_leave ();
			return;
		}

		gtk_object_set_data (GTK_OBJECT (ti->type_info.send.db->message), "state",
											(gpointer) C2_MESSAGE_READED);
		gdk_threads_leave ();
		c2_db_message_add (sent_items, ti->type_info.send.db->message);
		gdk_threads_enter ();
	}

	gdk_threads_leave ();
	c2_db_message_remove (outbox, ti->type_info.send.db);
	gdk_threads_enter ();


	gtk_object_unref (GTK_OBJECT (ti->type_info.send.db->message));

	gdk_threads_leave ();

	printf ("%s\n", __PRETTY_FUNCTION__);
}
