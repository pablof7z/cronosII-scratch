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

#include <libcronosII/error.h>
#include <libcronosII/utils.h>

#include "widget-application.h"

static void
class_init									(C2ApplicationClass *klass);

static void
init										(C2Application *application);

static void
destroy										(GtkObject *object);

void
on_mailbox_changed_mailboxes				(C2Mailbox *mailbox, C2Application *application);

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

	object_class->destroy = destroy;
}

static void
init (C2Application *application)
{
	C2Account *account;
	gchar *account_name, *full_name, *organization, *email, *reply_to, *incoming_host,
			*incoming_user, *incoming_pass, *outgoing_host, *outgoing_user, *outgoing_pass,
			*signature_plain, *signature_html;
	gint incoming_port, incoming_flags, outgoing_port;
	gboolean active, incoming_ssl, outgoing_authentication, outgoing_ssl, keep_copy;
	C2AccountType account_type;
	C2SMTPType outgoing_protocol;
	gchar *tmp, *buf;	
	gint quantity = gnome_config_get_int_with_default ("/"PACKAGE"/Mailboxes/quantity=0", NULL);
	gint i;
	
	application->open_windows = NULL;
	application->tmp_files = NULL;
	application->account = NULL;
	application->mailbox = NULL;

	/* Load accounts */
	for (i = 0;; i++)
	{
		tmp = g_strdup_printf ("/"PACKAGE"/Account %d/", i);
		gnome_config_push_prefix (tmp);

		if (!(account_name = gnome_config_get_string ("name")))
		{
			gnome_config_pop_prefix ();
			g_free (tmp);
			break;
		}

		full_name = gnome_config_get_string ("full_name");
		organization = gnome_config_get_string ("organization");
		email = gnome_config_get_string ("email");
		reply_to = gnome_config_get_string ("reply_to");
		active = gnome_config_get_bool ("options.active");

		account_type = gnome_config_get_int ("incoming_protocol");

		switch (account_type)
		{
			case C2_ACCOUNT_POP3:
			case C2_ACCOUNT_IMAP:
				incoming_host = gnome_config_get_string ("incoming_hostname");
				incoming_port = gnome_config_get_int ("incoming_port");
				incoming_user = gnome_config_get_string ("incoming_username");
				incoming_pass = gnome_config_get_string ("incoming_password");
				incoming_flags = gnome_config_get_int ("incoming_flags");
				incoming_ssl = gnome_config_get_bool ("incoming_ssl");
				break;
		}

		outgoing_protocol = gnome_config_get_int ("outgoing_protocol");

		switch (outgoing_protocol)
		{
			case C2_SMTP_REMOTE:
				outgoing_host = gnome_config_get_string ("outgoing_hostname");
				outgoing_port = gnome_config_get_int ("outgoing_port");
				outgoing_authentication = gnome_config_get_bool ("outgoing_authentication");
				outgoing_user = gnome_config_get_string ("outgoing_username");
				outgoing_pass = gnome_config_get_string ("outgoing_password");
				outgoing_ssl = gnome_config_get_bool ("outgoing_ssl");
				break;
		}

		signature_plain = gnome_config_get_string ("signature.plain");
		signature_html = gnome_config_get_string ("signature.html");

		switch (outgoing_protocol)
		{
			case C2_SMTP_REMOTE:
				account = c2_account_new (account_name, full_name, organization, email, reply_to,
											active, signature_plain, signature_html, account_type,
											outgoing_protocol, incoming_host, incoming_port,
											incoming_user, incoming_pass, incoming_ssl,
											incoming_flags,
											outgoing_host, outgoing_port, outgoing_ssl,
											outgoing_authentication, outgoing_user, outgoing_pass);
				break;
			case C2_SMTP_LOCAL:
				account = c2_account_new (account_name, full_name, organization, email, reply_to,
											active, signature_plain, signature_html, account_type,
											outgoing_protocol, incoming_host, incoming_port, incoming_user,
											incoming_pass, incoming_flags, incoming_ssl);
				break;
		}
		
		application->account = c2_account_append (application->account, account);

		gnome_config_pop_prefix ();
		g_free (tmp);
	}

	application->options_check_timeout = gnome_config_get_int_with_default
									("/Cronos II/Options/check_timeout=" DEFAULT_OPTIONS_CHECK_TIMEOUT, NULL);
	application->options_wrap_outgoing_text = gnome_config_get_int_with_default
							("/Cronos II/Options/wrap_outgoing_text=" DEFAULT_OPTIONS_WRAP_OUTGOING_TEXT, NULL);
	application->options_mark_timeout = gnome_config_get_int_with_default
									("/Cronos II/Options/mark_timeout=" DEFAULT_OPTIONS_MARK_TIMEOUT, NULL);
	application->options_prepend_character = gnome_config_get_string_with_default
									("/Cronos II/Options/prepend_character=" DEFAULT_OPTIONS_PREPEND_CHARACTER, NULL);
	application->options_empty_garbage = gnome_config_get_int_with_default
									("/Cronos II/Options/empty_garbage=" DEFAULT_OPTIONS_EMPTY_GARBAGE, NULL);
	application->options_use_outbox = gnome_config_get_int_with_default
									("/Cronos II/Options/use_outbox=" DEFAULT_OPTIONS_USE_OUTBOX, NULL);
	application->options_check_at_start = gnome_config_get_int_with_default
									("/Cronos II/Options/check_at_start=" DEFAULT_OPTIONS_CHECK_AT_START, NULL);
	application->options_mt_mode = gnome_config_get_int_with_default
									("/Cronos II/Options/mt_mode=" DEFAULT_OPTIONS_MT_MODE, NULL);
	application->options_default_mime = gnome_config_get_int_with_default
									("/Cronos II/Options/default_mime=" DEFAULT_OPTIONS_DEFAULT_MIME, NULL);	
	printf ("<<<<%d\n", application->options_default_mime);
	
	application->interface_title = gnome_config_get_string_with_default
									("/Cronos II/Interface/title=" DEFAULT_INTERFACE_TITLE, NULL);
	application->interface_toolbar = gnome_config_get_int_with_default
									("/Cronos II/Interface/toolbar=" DEFAULT_INTERFACE_TOOLBAR, NULL);
	application->interface_date_fmt = gnome_config_get_string_with_default
									("/Cronos II/Interface/date_fmt=" DEFAULT_INTERFACE_DATE_FMT, NULL);

	application->fonts_message_body = gnome_config_get_string_with_default
									("/Cronos II/Fonts/message_body=" DEFAULT_FONTS_MESSAGE_BODY, NULL);
	application->fonts_unreaded_message = gnome_config_get_string_with_default
									("/Cronos II/Fonts/unreaded_message=" DEFAULT_FONTS_UNREADED_MESSAGE, NULL);
	application->fonts_readed_message = gnome_config_get_string_with_default
									("/Cronos II/Fonts/readed_message=" DEFAULT_FONTS_READED_MESSAGE, NULL);
	application->fonts_unreaded_mailbox = gnome_config_get_string_with_default
									("/Cronos II/Fonts/unreaded_mailbox=" DEFAULT_FONTS_UNREADED_MAILBOX, NULL);
	application->fonts_source = gnome_config_get_int_with_default
									("/Cronos II/Fonts/source=" DEFAULT_FONTS_SOURCE, NULL);
	
	application->colors_replying_original_message.red = gnome_config_get_int_with_default
			("/Cronos II/Colors/replying_original_message_red=" DEFAULT_COLORS_REPLYING_ORIGINAL_MESSAGE_RED, NULL);
	application->colors_replying_original_message.green = gnome_config_get_int_with_default
		("/Cronos II/Colors/replying_original_message_green=" DEFAULT_COLORS_REPLYING_ORIGINAL_MESSAGE_GREEN, NULL);
	application->colors_replying_original_message.blue = gnome_config_get_int_with_default
			("/Cronos II/Colors/replying_original_message_blue=" DEFAULT_COLORS_REPLYING_ORIGINAL_MESSAGE_BLUE, NULL);
	application->colors_message_bg.red = gnome_config_get_int_with_default
				("/Cronos II/Colors/message_bg_red=" DEFAULT_COLORS_MESSAGE_BG_RED, NULL);
	application->colors_message_bg.green = gnome_config_get_int_with_default
				("/Cronos II/Colors/replying_original_message_green=" DEFAULT_COLORS_MESSAGE_BG_GREEN, NULL);
	application->colors_message_bg.blue = gnome_config_get_int_with_default
				("/Cronos II/Colors/message_bg_blue=" DEFAULT_COLORS_MESSAGE_BG_BLUE, NULL);	
	application->colors_message_fg.red = gnome_config_get_int_with_default
				("/Cronos II/Colors/message_fg_red=" DEFAULT_COLORS_MESSAGE_FG_RED, NULL);
	application->colors_message_fg.green = gnome_config_get_int_with_default
				("/Cronos II/Colors/message_fg_green=" DEFAULT_COLORS_MESSAGE_FG_GREEN, NULL);
	application->colors_message_fg.blue = gnome_config_get_int_with_default
				("/Cronos II/Colors/message_fg_blue=" DEFAULT_COLORS_MESSAGE_FG_BLUE, NULL);
	application->colors_message_source = gnome_config_get_int_with_default
									("/Cronos II/Colors/message_source=" DEFAULT_COLORS_MESSAGE_SOURCE, NULL);

	buf = g_strconcat ("/Cronos II/Paths/saving=", g_get_home_dir (), NULL);
	application->paths_saving = gnome_config_get_string_with_default
									(buf, NULL);
	g_free (buf);
	buf = g_strconcat ("/Cronos II/Paths/download=", g_get_home_dir (), NULL);
	application->paths_download = gnome_config_get_string_with_default
									(buf, NULL);
	g_free (buf);
	buf = g_strconcat ("/Cronos II/Paths/get=", g_get_home_dir (), NULL);
	application->paths_get = gnome_config_get_string_with_default
									(buf, NULL);
	g_free (buf);
	application->paths_always_use = gnome_config_get_int_with_default
									("/Cronos II/Paths/always_use=" DEFAULT_PATHS_ALWAYS_USE, NULL);

	application->advanced_http_proxy_addr = gnome_config_get_string_with_default
									("/Cronos II/Advanced/http_proxy_addr=" DEFAULT_ADVANCED_HTTP_PROXY_ADDR, NULL);
	application->advanced_http_proxy_port = gnome_config_get_int_with_default
									("/Cronos II/Advanced/http_proxy_port=" DEFAULT_ADVANCED_HTTP_PROXY_PORT, NULL);
	application->advanced_http_proxy = gnome_config_get_int_with_default
									("/Cronos II/Advanced/http_proxy=" DEFAULT_ADVANCED_HTTP_PROXY, NULL);
	application->advanced_ftp_proxy_addr = gnome_config_get_string_with_default
									("/Cronos II/Advanced/ftp_proxy_addr=" DEFAULT_ADVANCED_FTP_PROXY_ADDR, NULL);
	application->advanced_ftp_proxy_port = gnome_config_get_int_with_default
									("/Cronos II/Advanced/ftp_proxy_port=" DEFAULT_ADVANCED_FTP_PROXY_PORT, NULL);
	application->advanced_ftp_proxy = gnome_config_get_int_with_default
									("/Cronos II/Advanced/ftp_proxy=" DEFAULT_ADVANCED_FTP_PROXY, NULL);
	application->advanced_persistent_smtp_addr = gnome_config_get_string_with_default
							("/Cronos II/Advanced/persistent_smtp_addr=" DEFAULT_ADVANCED_PERSISTENT_SMTP_ADDR, NULL);
	application->advanced_persistent_smtp_port = gnome_config_get_int_with_default
							("/Cronos II/Advanced/persistent_smtp_port=" DEFAULT_ADVANCED_PERSISTENT_SMTP_PORT, NULL);
	application->advanced_persistent_smtp = gnome_config_get_int_with_default
									("/Cronos II/Advanced/persistent_smtp=" DEFAULT_ADVANCED_PERSISTENT_SMTP, NULL);
	application->advanced_use_internal_browser = gnome_config_get_int_with_default
							("/Cronos II/Advanced/use_internal_browser=" DEFAULT_ADVANCED_USE_INTERNAL_BROWSER, NULL);
	application->advanced_load_mailboxes_at_start = gnome_config_get_int_with_default
							("/Cronos II/Advanced/load_mailboxes_at_start=" DEFAULT_ADVANCED_LOAD_MAILBOXES_AT_START, NULL);

	application->rc_hpan = gnome_config_get_int_with_default ("/Cronos II/Rc/hpan=" DEFAULT_RC_HPAN, NULL);
	application->rc_vpan = gnome_config_get_int_with_default ("/Cronos II/Rc/vpan=" DEFAULT_RC_VPAN, NULL);
	application->rc_clist[0] = gnome_config_get_int_with_default ("/Cronos II/Rc/clist[0]=" DEFAULT_RC_CLIST_0, NULL);
	application->rc_clist[1] = gnome_config_get_int_with_default ("/Cronos II/Rc/clist[1]=" DEFAULT_RC_CLIST_1, NULL);
	application->rc_clist[2] = gnome_config_get_int_with_default ("/Cronos II/Rc/clist[2]=" DEFAULT_RC_CLIST_2, NULL);
	application->rc_clist[3] = gnome_config_get_int_with_default ("/Cronos II/Rc/clist[3]=" DEFAULT_RC_CLIST_3, NULL);
	application->rc_clist[4] = gnome_config_get_int_with_default ("/Cronos II/Rc/clist[4]=" DEFAULT_RC_CLIST_4, NULL);
	application->rc_clist[5] = gnome_config_get_int_with_default ("/Cronos II/Rc/clist[5]=" DEFAULT_RC_CLIST_5, NULL);
	application->rc_clist[6] = gnome_config_get_int_with_default ("/Cronos II/Rc/clist[6]=" DEFAULT_RC_CLIST_6, NULL);
	application->rc_clist[7] = gnome_config_get_int_with_default ("/Cronos II/Rc/clist[7]=" DEFAULT_RC_CLIST_7, NULL);
	application->rc_width = gnome_config_get_int_with_default ("/Cronos II/Rc/width=" DEFAULT_RC_WIDTH, NULL);
	application->rc_height = gnome_config_get_int_with_default ("/Cronos II/Rc/height=" DEFAULT_RC_HEIGHT, NULL);
	application->rc_showable_headers[C2_SHOWABLE_HEADERS_PREVIEW] = gnome_config_get_int_with_default
								("/Cronos II/Rc/showable_headers_preview=" DEFAULT_RC_SHOWABLE_HEADERS_PREVIEW, NULL);
	application->rc_showable_headers[C2_SHOWABLE_HEADERS_MESSAGE] = gnome_config_get_int_with_default
								("/Cronos II/Rc/showable_headers_message=" DEFAULT_RC_SHOWABLE_HEADERS_MESSAGE, NULL);
	application->rc_showable_headers[C2_SHOWABLE_HEADERS_COMPOSE] = gnome_config_get_int_with_default
								("/Cronos II/Rc/showable_headers_compose=" DEFAULT_RC_SHOWABLE_HEADERS_COMPOSE, NULL);
	application->rc_showable_headers[C2_SHOWABLE_HEADERS_SAVE] = gnome_config_get_int_with_default
								("/Cronos II/Rc/showable_headers_save=" DEFAULT_RC_SHOWABLE_HEADERS_SAVE, NULL);
	application->rc_showable_headers[C2_SHOWABLE_HEADERS_PRINT] = gnome_config_get_int_with_default
								("/Cronos II/Rc/showable_headers_print=" DEFAULT_RC_SHOWABLE_HEADERS_PRINT, NULL);

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
				mailbox = c2_mailbox_new (name, id, type, sort_by, sort_type);
				break;
			case C2_MAILBOX_IMAP:
				host = gnome_config_get_string ("host");
				port = gnome_config_get_int ("port");
				user = gnome_config_get_string ("user");
				pass = gnome_config_get_string ("pass");
				path = gnome_config_get_string ("path");
				mailbox = c2_mailbox_new (name, id, type, sort_by, sort_type, host, port, user, pass, path);
				g_free (host);
				g_free (user);
				g_free (pass);
				g_free (path);
				break;
			case C2_MAILBOX_SPOOL:
				path = gnome_config_get_string ("path");
				mailbox = c2_mailbox_new (name, id, type, sort_by, sort_type, path);
				g_free (path);
				break;
		}

		if (application->advanced_load_mailboxes_at_start)
			c2_mailbox_load_db (mailbox);

		g_free (name);
		g_free (id);
		gnome_config_pop_prefix ();
		g_free (query);

		/* Connect the first mailbox */
		if (i == 1)
			gtk_signal_connect (GTK_OBJECT (mailbox), "changed_mailboxes",
								GTK_SIGNAL_FUNC (on_mailbox_changed_mailboxes), application);

#if FALSE
		/* We have to connect to the "db_loaded" signal */
		gtk_signal_connect (GTK_OBJECT (mbox), "db_loaded",
						GTK_SIGNAL_FUNC (on_mailbox_db_loaded), NULL);
#endif
	}

	application->mailbox = c2_mailbox_get_head ();

	application->fonts_gdk_message_body = gdk_font_load (application->fonts_message_body);
	application->fonts_gdk_readed_message = gdk_font_load (application->fonts_readed_message);
	application->fonts_gdk_unreaded_message = gdk_font_load (application->fonts_unreaded_message);
	application->fonts_gdk_unreaded_mailbox = gdk_font_load (application->fonts_unreaded_mailbox);

	gdk_color_alloc (gdk_colormap_get_system (), &application->colors_replying_original_message);
	gdk_color_alloc (gdk_colormap_get_system (), &application->colors_message_bg);
	gdk_color_alloc (gdk_colormap_get_system (), &application->colors_message_fg);
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

	c2_mailbox_destroy_tree ();

	g_free (application->options_prepend_character);
	
	c2_account_free_all (application->account);

	g_free (application->interface_title);
	g_free (application->interface_date_fmt);
	g_free (application->fonts_message_body);
	g_free (application->fonts_unreaded_message);
	g_free (application->fonts_readed_message);
	g_free (application->fonts_unreaded_mailbox);
	g_free (application->paths_saving);
	g_free (application->paths_download);
	g_free (application->paths_get);
	g_free (application->advanced_http_proxy_addr);
	g_free (application->advanced_ftp_proxy_addr);
	g_free (application->advanced_persistent_smtp_addr);

	gtk_main_quit ();
}

void
on_mailbox_changed_mailboxes (C2Mailbox *mailbox, C2Application *application)
{
	application->mailbox = c2_mailbox_get_head ();
	gtk_signal_emit (GTK_OBJECT (application), signals[RELOAD_MAILBOXES]);
}

C2Application *
c2_application_new (const gchar *name)
{
	C2Application *application;

	application = gtk_type_new (c2_application_get_type ());

	application->name = g_strdup (name);

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
#ifdef USE_DEBUG
	g_print ("Adding a window, reffing to %d.\n", GTK_OBJECT (application)->ref_count);
#endif

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

#ifdef USE_DEBUG
	g_print ("Removing a window, unreffing to %d.\n", GTK_OBJECT (application)->ref_count);
#endif

	gtk_signal_emit (GTK_OBJECT (application), signals[WINDOW_CHANGED]);
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
