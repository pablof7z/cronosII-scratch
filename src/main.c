/*  Cronos II Mail Client /src/main.c
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

#include <libcronosII/mailbox.h>
#include <libcronosII/error.h>

#include "c2-app.h"
#include "c2-main-window.h"
#include "main-window.h"

static gint
c2_config_init									(void);

static void
c2_init (gint argc, gchar **argv)
{
	static struct poptOption options[] = {
		{
			N_("compose"), 'm', POPT_ARG_NONE,
			NULL, 0,
			N_("Compose a new email."), NULL
		},
		{
			N_("account"), 'a', POPT_ARG_STRING,
			NULL, 0,
			N_("Set the Account field."),
			N_("Account")
		},
		{
			N_("to"), 't', POPT_ARG_STRING,
			NULL, 0,
			N_("Set the To field."),
			N_("Address")
		},
		{
			N_("cc"), 'c', POPT_ARG_STRING,
			NULL, 0,
			N_("Set the CC field."),
			N_("Address")
		},
		{
			N_("bcc"), 'b', POPT_ARG_STRING,
			NULL, 0,
			N_("Set the BCC field."),
			N_("Address")
		},
		{
			N_("subject"), 's', POPT_ARG_STRING,
			NULL, 0,
			N_("Set the Subject field."),
			N_("Subject")
		},
		{
			N_("body"), 'o', POPT_ARG_STRING,
			NULL, 0,
			N_("Set the Body."),
			N_("Text")
		},
		{
			"mailto", 'l', POPT_ARG_STRING,
			NULL, 0,
			N_("Compose a new email decoding the argument as a mailto: link"),
			"mailto:email@somewhere."
		}
	};
	gnome_init_with_popt_table ("Cronos II", VERSION, argc, argv, options, 0, NULL);
	glade_gnome_init ();
}

gint
main (gint argc, gchar **argv)
{
	g_thread_init (NULL);
	
	/* Language bindings */
	gtk_set_locale ();
#ifdef ENABLE_NLS
#ifdef HAVE_SETLOCALE_H
  setlocale (LC_ALL, "");
#endif
	bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);
#endif

	/* Initialization of GNOME and Glade */
	c2_init (argc, argv);

	if (!c2_config_init ())
	{
		c2_app_init ();
		c2_window_new ();
		
		gdk_threads_enter ();
		gtk_main ();
		gdk_threads_leave ();
		gnome_config_sync ();
	}

	return 0;
}

static void
load_mailboxes								(void);

/**
 * c2_config_init
 *
 * Will load the configuration.
 *
 * Return Value:
 * 0 if success or 1.
 **/
static gint
c2_config_init (void)
{
	C2Account *account;
	gchar *account_name, *person_name, *organization, *email, *reply_to, *pop3_host,
			*pop3_user, *pop3_pass, *smtp_host, *smtp_user, *smtp_pass,
			*signature;
	gint pop3_port, pop3_flags, smtp_port;
	gboolean active, smtp_authentication, keep_copy, signature_automatic;
	C2AccountType account_type;
	C2AccountSignatureType signature_type;
	C2SMTPType smtp_type;
	gchar *tmp, *buf;
	gint i;
	
	c2_app.tooltips = gtk_tooltips_new ();
	c2_app.open_windows = NULL;
	c2_app.tmp_files = NULL;

	/* Check if the configuration exists */
	tmp = gnome_config_get_string ("/cronosII/CronosII/Version");
	if (!tmp)
	{
#if FALSE /* TODO */
		gdk_threads_enter ();
		c2_install_new ();
		gdk_threads_leave ();
#endif
	}
	
	/* Get mailboxes */
	load_mailboxes ();

	/* Get accounts */
	c2_app.account = NULL;
	
	for (i = 0;; i++)
	{
		tmp = g_strdup_printf ("/cronosII/Account %d/", i);
		gnome_config_push_prefix (tmp);

		if (!(account_name = gnome_config_get_string ("name")))
			break;

		person_name = gnome_config_get_string ("per_name");
		organization = gnome_config_get_string ("organization");
		email = gnome_config_get_string ("email");
		reply_to = gnome_config_get_string ("reply_to");
		active = gnome_config_get_bool ("options.active");

		account_type = gnome_config_get_int ("protocol_type");

		switch (account_type)
		{
			case C2_ACCOUNT_POP3:
				pop3_host = gnome_config_get_string ("pop3_hostname");
				pop3_port = gnome_config_get_int ("pop3_port");
				pop3_user = gnome_config_get_string ("pop3_username");
				pop3_pass = gnome_config_get_string ("pop3_password");
				pop3_flags = gnome_config_get_int ("pop3_flags");
				break;
		}

		smtp_type = gnome_config_get_int ("smtp_type");

		switch (smtp_type)
		{
			case C2_SMTP_REMOTE:
				smtp_host = gnome_config_get_string ("smtp_hostname");
				smtp_port = gnome_config_get_int ("smtp_port");
				smtp_authentication = gnome_config_get_bool ("smtp_authentication");
				smtp_user = gnome_config_get_string ("smtp_username");
				smtp_pass = gnome_config_get_string ("smtp_password");
				break;
		}

		signature_type = gnome_config_get_int ("signature.type");
		signature = gnome_config_get_string ("signature.string");
		signature_automatic = gnome_config_get_bool ("signature.automatic");

		switch (smtp_type)
		{
			case C2_SMTP_REMOTE:
				account = c2_account_new (account_name, person_name, organization, email, reply_to,
											active, account_type, smtp_type, signature_type, signature,
											signature_automatic, pop3_host, pop3_port, pop3_user, pop3_pass,
											pop3_flags,
											smtp_host, smtp_port, smtp_authentication, smtp_user, smtp_pass);
				break;
			case C2_SMTP_LOCAL:
				account = c2_account_new (account_name, person_name, organization, email, reply_to,
											active, account_type, smtp_type, signature_type, signature,
											signature_automatic, pop3_host, pop3_port, pop3_user, pop3_pass,
											pop3_flags);
				break;
		}
		
		c2_app.account = c2_account_append (c2_app.account, account);

		gnome_config_pop_prefix ();
		g_free (tmp);
	}
	

	c2_app.options_check_timeout = gnome_config_get_int_with_default
									("/cronosII/Options/check_timeout=" DEFAULT_OPTIONS_CHECK_TIMEOUT, NULL);
	c2_app.options_mark_timeout = gnome_config_get_int_with_default
									("/cronosII/Options/mark_timeout=" DEFAULT_OPTIONS_MARK_TIMEOUT, NULL);
	c2_app.options_prepend_character = gnome_config_get_string_with_default
									("/cronosII/Options/prepend_character=" DEFAULT_OPTIONS_PREPEND_CHARACTER, NULL);
	c2_app.options_empty_garbage = gnome_config_get_int_with_default
									("/cronosII/Options/empty_garbage=" DEFAULT_OPTIONS_EMPTY_GARBAGE, NULL);
	c2_app.options_use_outbox = gnome_config_get_int_with_default
									("/cronosII/Options/use_outbox=" DEFAULT_OPTIONS_USE_OUTBOX, NULL);
	c2_app.options_check_at_start = gnome_config_get_int_with_default
									("/cronosII/Options/check_at_start=" DEFAULT_OPTIONS_CHECK_AT_START, NULL);
	c2_app.options_mt_mode = gnome_config_get_int_with_default
									("/cronosII/Options/mt_mode=" DEFAULT_OPTIONS_MT_MODE, NULL);
	c2_app.options_default_mime = gnome_config_get_int_with_default
									("/cronosII/Options/default_mime=" DEFAULT_OPTIONS_DEFAULT_MIME, NULL);	
	
	c2_app.interface_title = gnome_config_get_string_with_default
									("/cronosII/Interface/title=" DEFAULT_INTERFACE_TITLE, NULL);
	c2_app.interface_toolbar = gnome_config_get_int_with_default
									("/cronosII/Interface/toolbar=" DEFAULT_INTERFACE_TOOLBAR, NULL);
	c2_app.interface_date_fmt = gnome_config_get_string_with_default
									("/cronosII/Interface/date_fmt=" DEFAULT_INTERFACE_DATE_FMT, NULL);

	c2_app.fonts_message_body = gnome_config_get_string_with_default
									("/cronosII/Fonts/message_body=" DEFAULT_FONTS_MESSAGE_BODY, NULL);
	c2_app.fonts_unreaded_message = gnome_config_get_string_with_default
									("/cronosII/Fonts/unreaded_message=" DEFAULT_FONTS_UNREADED_MESSAGE, NULL);
	c2_app.fonts_readed_message = gnome_config_get_string_with_default
									("/cronosII/Fonts/readed_message=" DEFAULT_FONTS_READED_MESSAGE, NULL);
	c2_app.fonts_source = gnome_config_get_int_with_default
									("/cronosII/Fonts/source=" DEFAULT_FONTS_SOURCE, NULL);
	
	c2_app.colors_replying_original_message.red = gnome_config_get_int_with_default
			("/cronosII/Colors/replying_original_message_red=" DEFAULT_COLORS_REPLYING_ORIGINAL_MESSAGE_RED, NULL);
	c2_app.colors_replying_original_message.green = gnome_config_get_int_with_default
		("/cronosII/Colors/replying_original_message_green=" DEFAULT_COLORS_REPLYING_ORIGINAL_MESSAGE_GREEN, NULL);
	c2_app.colors_replying_original_message.blue = gnome_config_get_int_with_default
			("/cronosII/Colors/replying_original_message_blue=" DEFAULT_COLORS_REPLYING_ORIGINAL_MESSAGE_BLUE, NULL);
	c2_app.colors_message_bg.red = gnome_config_get_int_with_default
				("/cronosII/Colors/message_bg_red=" DEFAULT_COLORS_MESSAGE_BG_RED, NULL);
	c2_app.colors_message_bg.green = gnome_config_get_int_with_default
				("/cronosII/Colors/replying_original_message_green=" DEFAULT_COLORS_MESSAGE_BG_GREEN, NULL);
	c2_app.colors_message_bg.blue = gnome_config_get_int_with_default
				("/cronosII/Colors/message_bg_blue=" DEFAULT_COLORS_MESSAGE_BG_BLUE, NULL);	
	c2_app.colors_message_fg.red = gnome_config_get_int_with_default
				("/cronosII/Colors/message_fg_red=" DEFAULT_COLORS_MESSAGE_FG_RED, NULL);
	c2_app.colors_message_fg.green = gnome_config_get_int_with_default
				("/cronosII/Colors/message_fg_green=" DEFAULT_COLORS_MESSAGE_FG_GREEN, NULL);
	c2_app.colors_message_fg.blue = gnome_config_get_int_with_default
				("/cronosII/Colors/message_fg_blue=" DEFAULT_COLORS_MESSAGE_FG_BLUE, NULL);
	c2_app.colors_message_source = gnome_config_get_int_with_default
									("/cronosII/Colors/message_source=" DEFAULT_COLORS_MESSAGE_SOURCE, NULL);

	buf = g_strconcat ("/cronosII/Paths/saving=", g_get_home_dir (), NULL);
	c2_app.paths_saving = gnome_config_get_string_with_default
									(buf, NULL);
	g_free (buf);
	buf = g_strconcat ("/cronosII/Paths/download=", g_get_home_dir (), NULL);
	c2_app.paths_download = gnome_config_get_string_with_default
									(buf, NULL);
	g_free (buf);
	buf = g_strconcat ("/cronosII/Paths/get=", g_get_home_dir (), NULL);
	c2_app.paths_get = gnome_config_get_string_with_default
									(buf, NULL);
	g_free (buf);
	c2_app.paths_always_use = gnome_config_get_int_with_default
									("/cronosII/Paths/always_use=" DEFAULT_PATHS_ALWAYS_USE, NULL);

	c2_app.advanced_http_proxy_addr = gnome_config_get_string_with_default
									("/cronosII/Advanced/http_proxy_addr=" DEFAULT_ADVANCED_HTTP_PROXY_ADDR, NULL);
	c2_app.advanced_http_proxy_port = gnome_config_get_int_with_default
									("/cronosII/Advanced/http_proxy_port=" DEFAULT_ADVANCED_HTTP_PROXY_PORT, NULL);
	c2_app.advanced_http_proxy = gnome_config_get_int_with_default
									("/cronosII/Advanced/http_proxy=" DEFAULT_ADVANCED_HTTP_PROXY, NULL);
	c2_app.advanced_ftp_proxy_addr = gnome_config_get_string_with_default
									("/cronosII/Advanced/ftp_proxy_addr=" DEFAULT_ADVANCED_FTP_PROXY_ADDR, NULL);
	c2_app.advanced_ftp_proxy_port = gnome_config_get_int_with_default
									("/cronosII/Advanced/ftp_proxy_port=" DEFAULT_ADVANCED_FTP_PROXY_PORT, NULL);
	c2_app.advanced_ftp_proxy = gnome_config_get_int_with_default
									("/cronosII/Advanced/ftp_proxy=" DEFAULT_ADVANCED_FTP_PROXY, NULL);
	c2_app.advanced_persistent_smtp_addr = gnome_config_get_string_with_default
							("/cronosII/Advanced/persistent_smtp_addr=" DEFAULT_ADVANCED_PERSISTENT_SMTP_ADDR, NULL);
	c2_app.advanced_persistent_smtp_port = gnome_config_get_int_with_default
							("/cronosII/Advanced/persistent_smtp_port=" DEFAULT_ADVANCED_PERSISTENT_SMTP_PORT, NULL);
	c2_app.advanced_persistent_smtp = gnome_config_get_int_with_default
									("/cronosII/Advanced/persistent_smtp=" DEFAULT_ADVANCED_PERSISTENT_SMTP, NULL);
	c2_app.advanced_use_internal_browser = gnome_config_get_int_with_default
							("/cronosII/Advanced/use_internal_browser=" DEFAULT_ADVANCED_USE_INTERNAL_BROWSER, NULL);

	c2_app.rc_hpan = gnome_config_get_int_with_default ("/cronosII/Rc/hpan=" DEFAULT_RC_HPAN, NULL);
	c2_app.rc_vpan = gnome_config_get_int_with_default ("/cronosII/Rc/vpan=" DEFAULT_RC_VPAN, NULL);
	c2_app.rc_clist[0] = gnome_config_get_int_with_default ("/cronosII/Rc/clist[0]=" DEFAULT_RC_CLIST_0, NULL);
	c2_app.rc_clist[1] = gnome_config_get_int_with_default ("/cronosII/Rc/clist[1]=" DEFAULT_RC_CLIST_1, NULL);
	c2_app.rc_clist[2] = gnome_config_get_int_with_default ("/cronosII/Rc/clist[2]=" DEFAULT_RC_CLIST_2, NULL);
	c2_app.rc_clist[3] = gnome_config_get_int_with_default ("/cronosII/Rc/clist[3]=" DEFAULT_RC_CLIST_3, NULL);
	c2_app.rc_clist[4] = gnome_config_get_int_with_default ("/cronosII/Rc/clist[4]=" DEFAULT_RC_CLIST_4, NULL);
	c2_app.rc_clist[5] = gnome_config_get_int_with_default ("/cronosII/Rc/clist[5]=" DEFAULT_RC_CLIST_5, NULL);
	c2_app.rc_clist[6] = gnome_config_get_int_with_default ("/cronosII/Rc/clist[6]=" DEFAULT_RC_CLIST_6, NULL);
	c2_app.rc_clist[7] = gnome_config_get_int_with_default ("/cronosII/Rc/clist[7]=" DEFAULT_RC_CLIST_7, NULL);
	c2_app.rc_width = gnome_config_get_int_with_default ("/cronosII/Rc/width=" DEFAULT_RC_WIDTH, NULL);
	c2_app.rc_height = gnome_config_get_int_with_default ("/cronosII/Rc/height=" DEFAULT_RC_HEIGHT, NULL);
	c2_app.rc_showable_headers[C2_SHOWABLE_HEADERS_PREVIEW] = gnome_config_get_int_with_default
								("/cronosII/Rc/showable_headers_preview=" DEFAULT_RC_SHOWABLE_HEADERS_PREVIEW, NULL);
	c2_app.rc_showable_headers[C2_SHOWABLE_HEADERS_MESSAGE] = gnome_config_get_int_with_default
								("/cronosII/Rc/showable_headers_message=" DEFAULT_RC_SHOWABLE_HEADERS_MESSAGE, NULL);
	c2_app.rc_showable_headers[C2_SHOWABLE_HEADERS_COMPOSE] = gnome_config_get_int_with_default
								("/cronosII/Rc/showable_headers_compose=" DEFAULT_RC_SHOWABLE_HEADERS_COMPOSE, NULL);
	c2_app.rc_showable_headers[C2_SHOWABLE_HEADERS_SAVE] = gnome_config_get_int_with_default
								("/cronosII/Rc/showable_headers_save=" DEFAULT_RC_SHOWABLE_HEADERS_SAVE, NULL);
	c2_app.rc_showable_headers[C2_SHOWABLE_HEADERS_PRINT] = gnome_config_get_int_with_default
								("/cronosII/Rc/showable_headers_print=" DEFAULT_RC_SHOWABLE_HEADERS_PRINT, NULL);
	
	return 0;
}

static void
load_mailboxes (void)
{
	gint quantity = gnome_config_get_int_with_default ("/cronosII/Mailboxes/quantity=0", NULL);
	gint i;

	for (c2_app.mailbox = NULL, i = 1; i <= quantity; i++)
	{
		gchar *name;
		gchar *id;
		C2MailboxType type;
		C2MailboxSortBy sort_by;
		GtkSortType sort_type;
		gchar *host, *user, *pass, *path;
		gint port;
		gchar *query = g_strdup_printf ("/cronosII/Mailbox %d/", i);
		C2Mailbox *mbox;
		
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
				mbox = c2_mailbox_new (name, id, type, sort_by, sort_type);
				break;
			case C2_MAILBOX_IMAP:
				host = gnome_config_get_string ("host");
				port = gnome_config_get_int ("port");
				user = gnome_config_get_string ("user");
				pass = gnome_config_get_string ("pass");
				path = gnome_config_get_string ("path");
				mbox = c2_mailbox_new (name, id, type, sort_by, sort_type, host, port, user, pass, path);
				g_free (host);
				g_free (user);
				g_free (pass);
				break;
		}
		g_free (name);
		g_free (id);
		gnome_config_pop_prefix ();
		g_free (query);

		/* We have to connect to the "db_loaded" signal */
		gtk_signal_connect (GTK_OBJECT (mbox), "db_loaded",
						GTK_SIGNAL_FUNC (on_mailbox_db_loaded), NULL);
	}

	c2_app.mailbox = c2_mailbox_get_head ();
}
