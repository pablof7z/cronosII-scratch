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
#include <pthread.h>

#include <libcronosII/pop3.h>

#include "preferences.h"
#include "widget-transfer-item.h"
#include "widget-transfer-list.h"

/* [TODO]
 * 20011102 Make the width dynamically.
 */

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
on_pop3_disconnect							(GtkObject *object, gboolean success, C2NetObjectByte *byte,
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
	ti->account = NULL;
	ti->application = NULL;
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
			subject = NULL;
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
	gtk_box_pack_start (GTK_BOX (box), ti->progress_mail, FALSE, TRUE, 0);
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
on_popup_motion_notify_event (GtkWidget *popup, GdkEventMotion *e, C2TransferItem *ti)
{
	printf ("%fx%f\n", e->x, e->y);
	gtk_window_reposition (GTK_WINDOW (popup), e->x, e->y);
}

static void
on_enter_notify_event (GtkWidget *table, GdkEventCrossing *e, C2TransferItem *ti)
{
	ti->popup = gtk_window_new (GTK_WINDOW_POPUP);
	gtk_widget_set_events (ti->popup, GDK_POINTER_MOTION_MASK);
	gtk_widget_set_extension_events (ti->popup, GDK_EXTENSION_EVENTS_CURSOR);
	gtk_signal_connect (GTK_OBJECT (ti->popup), "motion_notify_event",
						GTK_SIGNAL_FUNC (on_popup_motion_notify_event), ti);
	gtk_widget_popup (ti->popup, e->x, e->y);
}

static void
on_leave_notify_event (GtkWidget *table, GdkEventCrossing *e, C2TransferItem *ti)
{
	gtk_widget_destroy (ti->popup);
}

static void
on_cancel_clicked (GtkWidget *button, C2TransferItem *ti)
{
	C2Account *account = ti->account;
	
	gtk_signal_emit (GTK_OBJECT (ti), signals[CANCEL], account);

	/* [XXX] This won't be here, I'm testing */
	gtk_signal_emit (GTK_OBJECT (ti), signals[FINISH]);

#if 0
	if (account->type == C2_ACCOUNT_POP3)
		c2_pop3_cancel (account->protocol.pop3);
#endif
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

			if (!(inbox = c2_mailbox_get_by_name (ti->application->mailbox, C2_MAILBOX_INBOX)))
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
		
		pthread_create (&thread, NULL, C2_PTHREAD_FUNC (c2_transfer_item_start_smtp_thread), ti);
	} else
		g_assert_not_reached ();
}

static void
on_pop3_resolve (GtkObject *object, C2NetObjectByte *byte, C2TransferItem *ti)
{
	gdk_threads_enter ();
	gtk_progress_set_format_string (GTK_PROGRESS (ti->progress_mail), _("Connecting"));
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
	xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("cronosII"), "dlg_req_pass_contents");
	contents = glade_xml_get_widget (xml, "dlg_req_pass_contents");
	dialog = c2_dialog_new (ti->application, _("Login failed"), "req_pass",
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
		if (!(pop3->flags & C2_POP3_DO_LOSE_PASSWORD))
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
	gdk_threads_enter ();
	if (!nth)
	{
		gtk_progress_configure (GTK_PROGRESS (ti->progress_mail), 0, 0, mails);
		gtk_progress_set_format_string (GTK_PROGRESS (ti->progress_mail), _("Getting mail list"));
	}

	gtk_progress_set_value (GTK_PROGRESS (ti->progress_mail), nth);
	gdk_threads_leave ();
}

static void
on_pop3_status (GtkObject *object, gint mails, C2TransferItem *ti)
{
	gdk_threads_enter ();
	gtk_progress_configure (GTK_PROGRESS (ti->progress_mail), 0, 0, mails);

	if (!mails)
		gtk_progress_set_percentage (GTK_PROGRESS (ti->progress_mail), 1.0);
	else
	{
		if (mails == 1)
			gtk_progress_set_format_string (GTK_PROGRESS (ti->progress_mail), _("%u message."));
		else
			gtk_progress_set_format_string (GTK_PROGRESS (ti->progress_mail), _("%u messages."));
	
		gtk_progress_configure (GTK_PROGRESS (ti->progress_mail), 0, 0, mails);
	}
	gdk_threads_leave ();
}

static void
on_pop3_retrieve (GtkObject *object, gint16 nth, gint32 received, gint32 total, C2TransferItem *ti)
{
	gdk_threads_enter ();
	
	if (!received)
	{
		if (nth == 1)
			gtk_progress_set_format_string (GTK_PROGRESS (ti->progress_mail), _("Receiving %v of %u"));
		gtk_progress_set_value (GTK_PROGRESS (ti->progress_mail), nth);
		
		gtk_widget_show (ti->progress_byte);
		gtk_progress_configure (GTK_PROGRESS (ti->progress_byte), 0, 0, total);
	} else if (total)
		gtk_progress_set_value (GTK_PROGRESS (ti->progress_byte), received);
	else
		gtk_widget_hide (ti->progress_byte);
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
	}

	gtk_progress_set_value (GTK_PROGRESS (ti->progress_mail), nth);
	gdk_threads_leave ();
}

static void
on_pop3_disconnect (GtkObject *object, gboolean success, C2NetObjectByte *byte, C2TransferItem *ti)
{
	gdk_threads_enter ();
	gtk_progress_set_format_string (GTK_PROGRESS (ti->progress_mail), _("Completed"));
	gtk_progress_set_percentage (GTK_PROGRESS (ti->progress_mail), 1.0);
	gtk_widget_set_sensitive (ti->cancel_button, FALSE);
	gdk_threads_leave ();

	gtk_signal_emit (GTK_OBJECT (ti), signals[FINISH]);
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
	}

	gtk_progress_set_value (GTK_PROGRESS (ti->progress_mail), bytes);

	gdk_threads_leave ();
}

static void
on_smtp_finished (C2SMTP *smtp, gint id, gboolean success, C2TransferItem *ti)
{
	C2TransferItem *_ti;
	C2Mailbox *outbox;
	C2Mailbox *sent_items;
	
	gdk_threads_enter ();
	if (!(_ti = smtp_get_ti_from_id (ti, id)))
	{
		gdk_threads_leave ();
		return;
	}
	
	gtk_progress_set_format_string (GTK_PROGRESS (ti->progress_mail),
									success ? _("Completed") : _("Failed"));
	gtk_progress_set_percentage (GTK_PROGRESS (ti->progress_mail), 1.0);
	gtk_widget_set_sensitive (ti->cancel_button, FALSE);

	gtk_signal_emit (GTK_OBJECT (ti), signals[FINISH]);

	gdk_threads_leave ();

	if (!(outbox = c2_mailbox_get_by_name (ti->application->mailbox,
											C2_MAILBOX_OUTBOX)))
	{
		g_warning ("There's no «%s» mailbox\n", C2_MAILBOX_OUTBOX);
		return;
	}

	if (c2_preferences_get_general_options_outgoing_sent_items ())
	{
		/* Move the message from «Outbox» to «Sent Items» */
		if (!(sent_items = c2_mailbox_get_by_name (ti->application->mailbox,
													C2_MAILBOX_SENT_ITEMS)))
		{
			g_warning ("There's no «%s» mailbox\n", C2_MAILBOX_SENT_ITEMS);
			return;
		}

		gtk_object_set_data (GTK_OBJECT (ti->type_info.send.db->message), "state",
											C2_MESSAGE_READED);
		c2_db_message_add (sent_items, ti->type_info.send.db->message);
	}

	c2_db_message_remove (outbox, ti->type_info.send.db->position-1);
		

	gtk_object_unref (GTK_OBJECT (ti->type_info.send.db->message));
}
