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
#include <config.h>
#include <gnome.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <libcronosII/account.h>
#include <libcronosII/pop3.h>
#include <libcronosII/imap.h>
#include <libcronosII/smtp.h>
#include <libcronosII/error.h>
#include <libcronosII/utils.h>

#include "preferences.h"
#include "widget-application.h"
#include "widget-composer.h"
#include "widget-dialog.h"
#include "widget-dialog-preferences.h"
#include "widget-transfer-item.h"
#include "widget-transfer-list.h"
#include "widget-window.h"

static void
class_init									(C2ApplicationClass *klass);

static void
init										(C2Application *application);

static void
destroy										(GtkObject *object);

void
on_mailbox_changed_mailboxes				(C2Mailbox *mailbox, C2Application *application);

static void
on_outbox_changed_mailbox					(C2Mailbox *mailbox, C2MailboxChangeType change, C2Db *db,
											 C2Application *application);

static void
on_preferences_changed						(C2DialogPreferences *preferences,
											 C2DialogPreferencesKey key, gpointer value);

enum
{
	PREFERENCES_CHANGED,
	RELOAD_MAILBOXES,
	WINDOW_CHANGED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static GtkObjectClass *parent_class = NULL;

GtkType
c2_application_get_type (void)
{
	static GtkType type = 0;

	if (!type)
	{
		GtkTypeInfo info =
		{
			"C2Application",
			sizeof (C2Application),
			sizeof (C2ApplicationClass),
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
class_init (C2ApplicationClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *) klass;

	parent_class = gtk_type_class (gtk_object_get_type ());

	signals[PREFERENCES_CHANGED] =
		gtk_signal_new ("application_preferences_changed",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2ApplicationClass, application_preferences_changed),
						gtk_marshal_NONE__INT_POINTER, GTK_TYPE_NONE, 2,
						GTK_TYPE_ENUM, GTK_TYPE_POINTER);
	signals[RELOAD_MAILBOXES] =
		gtk_signal_new ("reload_mailboxes",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2ApplicationClass, reload_mailboxes),
						gtk_marshal_NONE__NONE, GTK_TYPE_NONE, 0);
	signals[WINDOW_CHANGED] =
		gtk_signal_new ("window_changed",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2ApplicationClass, window_changed),
						gtk_marshal_NONE__NONE, GTK_TYPE_NONE, 0);
	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);

	klass->application_preferences_changed = NULL;
	klass->reload_mailboxes = NULL;
	klass->window_changed = NULL;
}

static void
load_mailbox (C2Mailbox *mailbox)
{
	c2_mailbox_load_db (mailbox);
}

static void
init (C2Application *application)
{
	gboolean active, incoming_ssl, outgoing_authentication, outgoing_ssl, keep_copy;
	C2AccountType account_type;
	C2SMTPType outgoing_protocol;
	gchar *tmp, *buf;
	gint quantity = gnome_config_get_int_with_default ("/"PACKAGE"/Mailboxes/quantity=0", NULL);
	gint i;
	gboolean load_mailboxes_at_start;
	
	application->open_windows = NULL;
	application->tmp_files = NULL;
	application->account = NULL;
	application->mailbox = NULL;

	/* Load accounts */
	for (i = 1;; i++)
	{
		C2Account *account;
		gchar *name, *email, *buf;
		C2AccountType account_type;
		gint type, outgoing, l;
		gpointer value;
		
		tmp = g_strdup_printf ("/"PACKAGE"/Account %d/", i);
		gnome_config_push_prefix (tmp);

		if (!(name = gnome_config_get_string ("account_name")) ||
			!(email = gnome_config_get_string ("identity_email")))
		{
			gnome_config_pop_prefix ();
			g_free (tmp);
			break;
		}

		account_type = gnome_config_get_int ("type");

		account = c2_account_new (account_type, name, email);

		for (l = 0; l < C2_ACCOUNT_KEY_LAST; l++)
		{
			switch (l)
			{
				case C2_ACCOUNT_KEY_FULL_NAME:
					buf = "identity_name";
					type = GTK_TYPE_STRING;
					break;
				case C2_ACCOUNT_KEY_ORGANIZATION:
					buf = "identity_organization";
					type = GTK_TYPE_STRING;
					break;
				case C2_ACCOUNT_KEY_REPLY_TO:
					buf = "identity_reply_to";
					type = GTK_TYPE_STRING;
					break;
				case C2_ACCOUNT_KEY_SIGNATURE_PLAIN:
					buf = "options_signature plain";
					type = GTK_TYPE_STRING;
					break;
				case C2_ACCOUNT_KEY_SIGNATURE_HTML:
					buf = "options_signature_html";
					type = GTK_TYPE_STRING;
					break;
				case C2_ACCOUNT_KEY_ACTIVE:
					buf = "options_auto_check";
					type = GTK_TYPE_BOOL;
					break;
				default:
					goto ignore;
			}

			switch (type)
			{
				case GTK_TYPE_STRING:
					value = gnome_config_get_string (buf);
					break;
				case GTK_TYPE_BOOL:
					value = (gpointer) gnome_config_get_bool (buf);
					break;
				case GTK_TYPE_INT:
					value = (gpointer) gnome_config_get_int (buf);
					break;
				default:
#ifdef USE_DEBUG
					g_warning ("Unable to handle type %d for creating accounts: %s:%d\n",
									type, __FILE__, __LINE__);
#endif
					break;
			}

			c2_account_set_extra_data (account, l, type, value);
ignore:
		}

		switch (account_type)
		{
			case C2_ACCOUNT_POP3:
			case C2_ACCOUNT_IMAP:
				{
					gchar *host, *user, *pass;
					gint port, flags = 0, days;
					gboolean ssl;
					C2POP3AuthenticationMethod auth_method;
					
					host = gnome_config_get_string ("incoming_server_hostname");
					port = gnome_config_get_int ("incoming_server_port");
					user = gnome_config_get_string ("incoming_server_username");
					pass = gnome_config_get_string ("incoming_server_password");
					ssl = gnome_config_get_bool ("incoming_server_ssl");
					auth_method = gnome_config_get_int ("incoming_auth_method");
					if (!gnome_config_get_bool ("incoming_auth_remember"))
						flags |= C2_POP3_DO_NOT_LOSE_PASSWORD;
					else
						flags |= C2_POP3_DO_LOSE_PASSWORD;
					if (gnome_config_get_bool ("options_multiple_access_leave"))
					{
						flags |= C2_POP3_DO_KEEP_COPY;

						if (gnome_config_get_bool ("options_multiple_access_remove"))
							days = gnome_config_get_int ("options_multiple_access_remove_value");
						else
							days = 0;
					} else
						flags |= C2_POP3_DO_NOT_KEEP_COPY;
					
					

					if (account_type == C2_ACCOUNT_POP3)
					{
						C2POP3 *pop3;
						pop3 = c2_pop3_new (host, port, user, pass, ssl);
						c2_pop3_set_auth_method (pop3, auth_method);
						c2_pop3_set_leave_copy (pop3, days ? TRUE : FALSE, days);
						c2_pop3_set_flags (pop3, flags);
						c2_account_set_extra_data (account, C2_ACCOUNT_KEY_INCOMING, GTK_TYPE_OBJECT, pop3);
					} else if (account_type == C2_ACCOUNT_IMAP)
					{
						C2IMAP *imap;
						imap = c2_imap_new (host, port, user, pass, "",
											C2_IMAP_AUTHENTICATION_PLAINTEXT, ssl);

						/* [TODO] There is more of the IMAP object to load... */
						c2_account_set_extra_data (account, C2_ACCOUNT_KEY_INCOMING, GTK_TYPE_OBJECT, imap);
					}
				}
				break;
		}

		outgoing = gnome_config_get_int ("outgoing_protocol");

		switch (outgoing)
		{
			case C2_SMTP_REMOTE:
				{
					C2SMTP *smtp;
					
					gchar *host, *user, *pass;
					gint port;
					gboolean auth, ssl;

					host = gnome_config_get_string ("outgoing_server_hostname");
					port = gnome_config_get_int ("outgoing_server_port");
					auth = gnome_config_get_bool ("outgoing_server_authentication");
					user = gnome_config_get_string ("outgoing_server_username");
					pass = gnome_config_get_string ("outgoing_server_password");
					ssl = gnome_config_get_bool ("outgoing_server_ssl");

					smtp = c2_smtp_new (outgoing, host, port, auth, user, pass, ssl);
					c2_account_set_extra_data (account, C2_ACCOUNT_KEY_OUTGOING, GTK_TYPE_OBJECT, smtp);
				}
				break;
			case C2_SMTP_LOCAL:
				{
					C2SMTP *smtp;
					smtp = c2_smtp_new (outgoing);
					c2_account_set_extra_data (account, C2_ACCOUNT_KEY_OUTGOING, GTK_TYPE_OBJECT, smtp);
					break;
				}
		}

		application->account = c2_account_append (application->account, account);

		gnome_config_pop_prefix ();
		g_free (tmp);
	}

	
	load_mailboxes_at_start = c2_preferences_get_general_options_start_load ();
	for (application->mailbox = NULL, i = 1; i <= quantity; i++)
	{
		gchar *name;
		gchar *id;
		C2MailboxType type;
		C2MailboxSortBy sort_by;
		GtkSortType sort_type;
		gchar *host, *user, *pass, *path;
		gint port;
		gchar *query = g_strdup_printf ("/"PACKAGE"/Mailbox %d/", i);
		C2Mailbox *mailbox;
		
		gnome_config_push_prefix (query);
		if (!(name = gnome_config_get_string ("name")))
		{
			gnome_config_pop_prefix ();
			g_free (query);
			continue;
		}

		id = gnome_config_get_string ("id");
		type = gnome_config_get_int ("type");
		sort_by = gnome_config_get_int ("sort_by");
		sort_type = gnome_config_get_int ("sort_type");

		switch (type)
		{
			case C2_MAILBOX_CRONOSII:
				mailbox = c2_mailbox_new (&application->mailbox, name, id, type, sort_by, sort_type);
				break;
			case C2_MAILBOX_IMAP:
				host = gnome_config_get_string ("host");
				port = gnome_config_get_int ("port");
				user = gnome_config_get_string ("user");
				pass = gnome_config_get_string ("pass");
				path = gnome_config_get_string ("path");
				mailbox = c2_mailbox_new (&application->mailbox, name, id, type, sort_by, sort_type, host, port, user, pass, path);
				g_free (host);
				g_free (user);
				g_free (pass);
				g_free (path);
				break;
			case C2_MAILBOX_SPOOL:
				path = gnome_config_get_string ("path");
				mailbox = c2_mailbox_new (&application->mailbox, name, id, type, sort_by, sort_type, path);
				g_free (path);
				break;
		}

		/* [TODO]
		 * Maybe fireing a separated thread where to do this? */
		if (load_mailboxes_at_start)
		{
			pthread_t thread;
			
			pthread_create (&thread, NULL, C2_PTHREAD_FUNC (load_mailbox), mailbox);
		}

		g_free (name);
		g_free (id);
		gnome_config_pop_prefix ();
		g_free (query);

		/* Connect the first mailbox */
		if (i == 1)
			gtk_signal_connect (GTK_OBJECT (mailbox), "changed_mailboxes",
								GTK_SIGNAL_FUNC (on_mailbox_changed_mailboxes), application);

		/* Connect to the change_mailbox of the outbox mailbox
		 * to know when we have to send a message.
		 */
		if (c2_streq (mailbox->name, C2_MAILBOX_OUTBOX))
		{
			gtk_signal_connect (GTK_OBJECT (mailbox), "changed_mailbox",
								GTK_SIGNAL_FUNC (on_outbox_changed_mailbox), application);
		}

#if FALSE
		/* We have to connect to the "db_loaded" signal */
		gtk_signal_connect (GTK_OBJECT (mbox), "db_loaded",
						GTK_SIGNAL_FUNC (on_mailbox_db_loaded), NULL);
#endif
	}

	buf = c2_preferences_get_interface_fonts_readed_mails ();
	application->fonts_gdk_readed_mails = gdk_font_load (buf);
	g_free (buf);
	buf = c2_preferences_get_interface_fonts_unreaded_mails ();
	application->fonts_gdk_unreaded_mails = gdk_font_load (buf);
	g_free (buf);
	buf = c2_preferences_get_interface_fonts_unreaded_mailbox ();
	application->fonts_gdk_unreaded_mailbox = gdk_font_load (buf);
	g_free (buf);
	buf = c2_preferences_get_interface_fonts_composer_body ();
	application->fonts_gdk_composer_body = gdk_font_load (buf);
	g_free (buf);
	buf = c2_preferences_get_interface_fonts_message_body ();
	application->fonts_gdk_message_body = gdk_font_load (buf);
	g_free (buf);
}

static void
destroy (GtkObject *object)
{
	C2Application *application = C2_APPLICATION (object);
	GSList *l;

	g_free (application->name);

	for (l = application->open_windows; l; l = g_slist_next (l))
		gtk_widget_destroy (GTK_WIDGET (l->data));
	g_slist_free (l);

	for (l = application->tmp_files; l; l = g_slist_next (l))
		unlink ((gchar *) l->data);
	g_slist_free (l);

	c2_mailbox_destroy_tree (application->mailbox);

	c2_account_free_all (application->account);

	gtk_main_quit ();
}

void
on_mailbox_changed_mailboxes (C2Mailbox *mailbox, C2Application *application)
{
	gtk_signal_emit (GTK_OBJECT (application), signals[RELOAD_MAILBOXES]);
}

static void
on_outbox_changed_mailbox (C2Mailbox *mailbox, C2MailboxChangeType change, C2Db *db,
							C2Application *application)
{
	C2Account *account;
	C2SMTP *smtp;
	GtkWidget *tl;
	C2TransferItem *ti;
	gchar *buf;

	if (!C2_IS_DB (db))
		return;

	/* I think c2 won't notice when a composer adds a message
	 * to the outbox mailbox when the mailbox is freezed
	 */
	if (change != C2_MAILBOX_CHANGE_ANY)
		return;

	db = db->next;

	c2_db_load_message (db);

	buf = c2_message_get_header_field (db->message, "\nX-CronosII-Send-Type:");
	if (((C2ComposerSendType) atoi (buf)) != C2_COMPOSER_SEND_NOW)
	{
		g_free (buf);
		gtk_object_unref (GTK_OBJECT (db->message));
		return;
	}
	g_free (buf);

	buf = c2_message_get_header_field (db->message, "\nX-CronosII-Account:");
	account = c2_account_get_by_name (application->account, buf);
	g_free (buf);
	if (!account)
		return;

	smtp = (C2SMTP*) c2_account_get_extra_data (account, C2_ACCOUNT_KEY_OUTGOING, NULL);

	tl = c2_application_window_get (application, C2_WIDGET_TRANSFER_LIST_TYPE);
	if (!tl || !C2_IS_TRANSFER_LIST (tl))
		tl = c2_transfer_list_new (application);
	
	ti = c2_transfer_item_new (application, account, C2_TRANSFER_ITEM_SEND, smtp, db);
	c2_transfer_list_add_item (C2_TRANSFER_LIST (tl), ti);
	c2_transfer_item_start (ti);

	gtk_widget_show (tl);
	gdk_window_raise (tl->window);
}

C2Application *
c2_application_new (const gchar *name)
{
	C2Application *application;

	application = gtk_type_new (c2_application_get_type ());

	application->name = g_strdup (name);

	gtk_signal_connect (GTK_OBJECT (application), "destroy",
						GTK_SIGNAL_FUNC (destroy), NULL);

	return application;
}


/**
 * c2_application_window_add
 * @application: C2Application where to work.
 * @window: GtkWindow object to add.
 * 
 * Each C2Application window needs to keep track of
 * every window that a child of its opens, with this
 * function you will be able to add a C2Window to
 * the application.
 **/
void
c2_application_window_add (C2Application *application, GtkWindow *window)
{
	application->open_windows = g_slist_append (application->open_windows, window);
	gtk_object_ref (GTK_OBJECT (application));

	/* Special care for special windows */
	if (C2_IS_WINDOW (GTK_WIDGET (window)) ||
		C2_IS_DIALOG (GTK_WIDGET (window)))
	{
		const gchar *type;
		
		type = gtk_object_get_data (GTK_OBJECT (window), "type");
		if (c2_streq (type, "preferences"))
			gtk_signal_connect (GTK_OBJECT (window), "changed",
								GTK_SIGNAL_FUNC (on_preferences_changed), window);
	}

	if (gnome_preferences_get_dialog_centered ())
		gtk_window_set_position (window, GTK_WIN_POS_CENTER);

	gtk_signal_emit (GTK_OBJECT (application), signals[WINDOW_CHANGED]);
}

/**
 * c2_application_window_remove
 * @application: C2Application where to work.
 * @window: GtkWindow object to remove (must be added before).
 * 
 * This function will remove a previously added C2Window
 * from the application.
 **/
void
c2_application_window_remove (C2Application *application, GtkWindow *window)
{
	application->open_windows = g_slist_remove (application->open_windows, window);
	gtk_object_unref (GTK_OBJECT (application));

	if (GTK_OBJECT (application)->ref_count <= 1)
		gtk_object_unref (GTK_OBJECT (application));

	gtk_signal_emit (GTK_OBJECT (application), signals[WINDOW_CHANGED]);
}

/**
 * c2_application_window_get
 * @application: Application where to work.
 * @type: Type of window to get.
 *
 * This function will search for a GtkWindow
 * of type @type.
 *
 * Return Value:
 * The widget or %NULL.
 **/
GtkWidget *
c2_application_window_get (C2Application *application, const gchar *type)
{
	GtkWidget *widget;
	gpointer data;
	GSList *l;
	
	c2_return_val_if_fail (application || type, NULL, C2EDATA);

	for (l = application->open_windows; l; l = g_slist_next (l))
	{
		widget = GTK_WIDGET (l->data);
		data = gtk_object_get_data (GTK_OBJECT (widget), "type");
		if (c2_streq ((gchar*)data, type))
			return widget;
	}
}

static void
on_preferences_changed (C2DialogPreferences *preferences, C2DialogPreferencesKey key, gpointer value)
{
	C2Application *application = C2_DIALOG (preferences)->application;
	gchar *buf;

	switch (key)
	{
		case C2_DIALOG_PREFERENCES_KEY_INTERFACE_FONTS_READED_MAILS:
			application->fonts_gdk_readed_mails = gdk_font_load ((gchar*)value);
			break;
		case C2_DIALOG_PREFERENCES_KEY_INTERFACE_FONTS_UNREADED_MAILS:
			application->fonts_gdk_unreaded_mails = gdk_font_load ((gchar*)value);
			break;
		case C2_DIALOG_PREFERENCES_KEY_INTERFACE_FONTS_UNREADED_MAILBOX:
			application->fonts_gdk_unreaded_mailbox = gdk_font_load ((gchar*)value);
			break;
#ifdef USE_ADVANCED_EDITOR
#else
		case C2_DIALOG_PREFERENCES_KEY_INTERFACE_FONTS_COMPOSER_BODY:
			application->fonts_gdk_composer_body = gdk_font_load ((gchar*)value);
			break;
#endif
#if defined (USE_GTKHTML) || defined (USE_GTKXMHTML)
#else
		case C2_DIALOG_PREFERENCES_KEY_INTERFACE_FONTS_MESSAGE_BODY:
			application->fonts_gdk_message_body = gdk_font_load ((gchar*)value);
			break;
#endif
	}
	
	gtk_signal_emit (GTK_OBJECT (application), signals[PREFERENCES_CHANGED],
					key, value);
}

/**
 * c2_app_get_mailbox_configuration_id_by_name
 * @name: Name of searched mailbox.
 *
 * This function will return the Configuration ID
 * of the mailbox with name @name.
 * Mailboxes, in the c2 configuration file are
 * stored in sections, each mailbox is a separated
 * section, with an ID, in the form:
 * [Mailbox $configuration_id]
 * name=@name
 * id=0-0-1
 *
 * Return Value:
 * Configuration ID of mailbox or -1.
 **/
gint
c2_app_get_mailbox_configuration_id_by_name (const gchar *name)
{
	gchar *prefix;
	gchar *gname;
	gint max = gnome_config_get_int_with_default ("/Cronos II/Mailboxes/quantity=-1", NULL);
	gint i;
	
	c2_return_val_if_fail (name, -1, C2EDATA);

	for (i = 1; i <= max; i++)
	{
		prefix = g_strdup_printf ("/Cronos II/Mailbox %d/", i);
		gnome_config_push_prefix (prefix);

		gname = gnome_config_get_string ("name");

		if (c2_streq (gname, name))
		{
			g_free (gname);
			gnome_config_pop_prefix ();
			g_free (prefix);
			return i;
		}

		g_free (gname);
		gnome_config_pop_prefix ();
		g_free (prefix);
	}
	
	return -1;
}
