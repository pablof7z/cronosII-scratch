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
 *  GNU General Public License for more details.
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
#include <gnome.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>

#include <libcronosII/account.h>
#include <libcronosII/pop3.h>
#include <libcronosII/imap.h>
#include <libcronosII/smtp.h>
#include <libcronosII/error.h>
#include <libcronosII/utils.h>
#include <libcronosII/utils-mutex.h>
#include <libcronosII/utils-str.h>

#include "main.h"
#include "preferences.h"
#include "widget-application.h"
#include "widget-application-utils.h"
#include "widget-composer.h"
#include "widget-dialog.h"
#include "widget-dialog-preferences.h"
#include "widget-HTML.h"
#include "widget-transfer-item.h"
#include "widget-transfer-list.h"
#include "widget-window.h"
#include "widget-window-mail.h"
#include "widget-window-main.h"

/* TODO
 * 20011208 We have to send a delete_event to all open window when
 *          destroying the app and there must be a clean up.
 */
 
#define MOD "Widget Application"
#ifdef USE_DEBUG
#	define DMOD TRUE
#endif

static void
class_init									(C2ApplicationClass *klass);

static void
init										(C2Application *application);

static void
destroy										(GtkObject *object);

static void
on_server_read								(C2Application *application, gint sock, GdkInputCondition cond);

static void
_check										(C2Application *application);

static void
_compact_mailboxes							(C2Application *application);

static void
_copy										(C2Application *application, GList *list, C2Window *window);

static void
_delete										(C2Application *application, GList *list, C2Window *window);

static void
_empty_trash								(C2Application *application, C2Window *window);

static void
_expunge									(C2Application *application, GList *list, C2Window *window);

static void
_forward									(C2Application *application, C2Db *db, C2Message *message);

static void
_move										(C2Application *application, GList *list, C2Window *window);

static void
_open_message								(C2Application *application, C2Db *db, C2Message *message,
											 const gchar *file);

static void
_print										(C2Application *application, C2Message *message);

static void
_reply										(C2Application *application, C2Db *db, C2Message *message);

static void
_reply_all									(C2Application *application, C2Db *db, C2Message *message);

static void
_save										(C2Application *application, C2Message *message, C2Window *window);

static void
_send										(C2Application *application);

void
on_mailbox_changed_mailboxes				(C2Mailbox *mailbox, C2Application *application);

static gboolean
on_application_timeout_check				(C2Application *application);

static void
on_outbox_changed_mailbox					(C2Mailbox *mailbox, C2MailboxChangeType change, C2Db *db,
											 C2Application *application);

static void
on_preferences_changed						(C2DialogPreferences *preferences,
											 C2DialogPreferencesKey key, gpointer value);

static gboolean
dlg_confirm_delete_message					(C2Application *application, GtkWindow *window, gboolean expunge);

static void
command_wmain_new							(C2Application *application, va_list args);

static void
command_wmain_raise							(C2Application *application, va_list args);

static void
command_wmain_hide							(C2Application *application, va_list args);

static void
command_composer_new						(C2Application *application, va_list args);

static void
command_open_file							(C2Application *application, va_list args);

static void
command_check_mail							(C2Application *application, va_list args);

static void
command_exit								(C2Application *application, va_list args);

typedef gpointer (*C2CommandFunc)			(C2Application *application, va_list args);

#define C2_COMMAND_FUNC(f)					(C2CommandFunc) f

#define AVAILABLE_ARGS 8
struct {
	const gchar *cmnd;
	C2CommandFunc func;
	size_t params;
	GtkFundamentalType arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8;
} commands[] =
{
	{
		C2_COMMAND_WINDOW_MAIN_NEW,
		C2_COMMAND_FUNC (command_wmain_new),
		1*sizeof (gchar),
		GTK_TYPE_STRING, GTK_TYPE_NONE, GTK_TYPE_NONE, GTK_TYPE_NONE,
		GTK_TYPE_NONE, GTK_TYPE_NONE, GTK_TYPE_NONE, GTK_TYPE_NONE
	},
	{
		C2_COMMAND_WINDOW_MAIN_RAISE,
		C2_COMMAND_FUNC (command_wmain_raise),
		0,
		GTK_TYPE_NONE, GTK_TYPE_NONE, GTK_TYPE_NONE, GTK_TYPE_NONE,
		GTK_TYPE_NONE, GTK_TYPE_NONE, GTK_TYPE_NONE, GTK_TYPE_NONE
	},
	{
		C2_COMMAND_WINDOW_MAIN_HIDE,
		C2_COMMAND_FUNC (command_wmain_hide),
		0,
		GTK_TYPE_NONE, GTK_TYPE_NONE, GTK_TYPE_NONE, GTK_TYPE_NONE,
		GTK_TYPE_NONE, GTK_TYPE_NONE, GTK_TYPE_NONE, GTK_TYPE_NONE
	},
	{
		C2_COMMAND_COMPOSER_NEW,
		C2_COMMAND_FUNC (command_composer_new),
		1*sizeof(gboolean)+2*sizeof(gchar),
		GTK_TYPE_BOOL, GTK_TYPE_STRING, GTK_TYPE_STRING, GTK_TYPE_NONE,
		GTK_TYPE_NONE, GTK_TYPE_NONE, GTK_TYPE_NONE, GTK_TYPE_NONE
	},
	{
		C2_COMMAND_OPEN_FILE,
		C2_COMMAND_FUNC (command_open_file),
		1*sizeof(gchar*),
		GTK_TYPE_STRING, GTK_TYPE_NONE, GTK_TYPE_NONE, GTK_TYPE_NONE,
		GTK_TYPE_NONE, GTK_TYPE_NONE, GTK_TYPE_NONE, GTK_TYPE_NONE
	},
	{
		C2_COMMAND_CHECK_MAIL,
		C2_COMMAND_FUNC (command_check_mail),
		0,
		GTK_TYPE_NONE, GTK_TYPE_NONE, GTK_TYPE_NONE, GTK_TYPE_NONE,
		GTK_TYPE_NONE, GTK_TYPE_NONE, GTK_TYPE_NONE, GTK_TYPE_NONE
	},
	{
		C2_COMMAND_EXIT,
		C2_COMMAND_FUNC (command_exit),
		0,
		GTK_TYPE_NONE, GTK_TYPE_NONE, GTK_TYPE_NONE, GTK_TYPE_NONE,
		GTK_TYPE_NONE, GTK_TYPE_NONE, GTK_TYPE_NONE, GTK_TYPE_NONE
	},
	{
		NULL,
		NULL,
		0,
		GTK_TYPE_NONE, GTK_TYPE_NONE, GTK_TYPE_NONE, GTK_TYPE_NONE,
		GTK_TYPE_NONE, GTK_TYPE_NONE, GTK_TYPE_NONE, GTK_TYPE_NONE
	}
};

static const gchar *check_files[] =
{
	C2_APPLICATION_GLADE_FILE ("cronosII"),
	C2_APPLICATION_GLADE_FILE ("dialogs"),
	C2_APPLICATION_GLADE_FILE ("preferences"),
	C2_APPLICATION_GLADE_FILE ("composer"),
	PKGDATADIR "/about.html",
	PKGDATADIR "/bugs.html",
	PKGDATADIR "/features.html",
	PKGDATADIR "/header.png",
	PKGDATADIR "/message_404.html",
	PKGDATADIR "/touch.html",
	PKGDATADIR "/welcome.html",
	PKGDATADIR "/default/cronosII.gnome",
	PKGDATADIR "/default/default.elm",
	PKGDATADIR "/default/default.index",
	NULL
};

enum
{
	EMERGENCY_DATA_SAVE,
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

	signals[EMERGENCY_DATA_SAVE] =
		gtk_signal_new ("emergency_data_save",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2ApplicationClass, emergency_data_save),
						gtk_marshal_NONE__NONE, GTK_TYPE_NONE, 0);
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
						gtk_marshal_NONE__POINTER, GTK_TYPE_NONE, 1,
						GTK_TYPE_POINTER);
	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);

	klass->emergency_data_save = NULL;
	klass->application_preferences_changed = NULL;
	klass->reload_mailboxes = NULL;
	klass->window_changed = NULL;

	klass->check = _check;
	klass->compact_mailboxes = _compact_mailboxes;
	klass->copy = _copy;
	klass->delete = _delete;
	klass->empty_trash = _empty_trash;
	klass->expunge = _expunge;
	klass->forward = _forward;
	klass->move = _move;
	klass->open_message = _open_message;
	klass->print = _print;
	klass->reply = _reply;
	klass->reply_all = _reply_all;
	klass->save = _save;
	klass->send = _send;
}

static void
load_mailbox (C2Mailbox *mailbox)
{
	c2_mailbox_load_db (mailbox);
}

static void
init (C2Application *application)
{
	gchar *tmp, *buf, *path = NULL;
	gint quantity = gnome_config_get_int_with_default ("/"PACKAGE"/Mailboxes/quantity=0", NULL);
	gint i;
	GtkStyle *style;
	GtkWidget *pixmap;
	gboolean load_mailboxes_at_start;
	struct sockaddr_un sa;
	struct stat stbuf;

#ifdef USE_DEBUG
	if (_debug_widget_application)
		C2_PRINTD (MOD, "Initializating the Application\n");
#endif

	application->open_windows = NULL;
	application->tmp_files = NULL;
	application->account = NULL;
	application->mailbox = NULL;
	application->check_timeout = 0;

	/* MULTISESSION CODE */
	if ((application->server_socket = socket (AF_UNIX, SOCK_STREAM, 0)) < 0)
	{
		perror ("socket");
		application->acting_as_server = 0;
		goto _init;
	}

	memset (&sa, 0, sizeof (sa));
	sa.sun_family = AF_UNIX;
	strncpy (sa.sun_path, C2_UNIX_SOCKET, sizeof (sa.sun_path) -1);

	path = g_strdup_printf ("%s" C2_HOME, g_get_home_dir ());
	chdir (path);
	g_free (path);

	path = g_strdup_printf ("%s" C2_HOME C2_UNIX_SOCKET, g_get_home_dir ());
	
	if (connect (application->server_socket, (struct sockaddr*) &sa, sizeof (sa)) < 0)
	{
		perror ("connect");
		/* We couldn't connect, we'll act as server */
		unlink (path);

		if (bind (application->server_socket, (struct sockaddr*) &sa, sizeof (sa)) < 0)
		{
			application->acting_as_server = 0;
			perror ("bind");
			goto _init;
		}

		if (listen (application->server_socket, 10) < 0)
		{
			printf ("Not acting as server\n");
			application->acting_as_server = 0;
			perror ("listen");
			goto _init;
		}

		application->server_lock = g_new0 (C2Mutex, 1);
		c2_mutex_init (application->server_lock);
		gdk_input_add (application->server_socket, GDK_INPUT_READ, (GdkInputFunction)on_server_read, application);
		application->acting_as_server = 1;
		printf ("Acting as server\n");
	} else
	{
		printf ("Not acting as server\n");
		application->acting_as_server = 0;
		return;
	}

_init:
	g_free (path);
	
	/* LOAD ACCOUNTS */
	for (i = 1;; i++)
	{
		C2Account *account;
		gchar *name, *email, *buf;
		C2AccountType account_type;
		gint type, outgoing, l;
		gpointer value = NULL;
		
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
					buf = "options_signature_plain";
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
					gint port, flags = 0, days = 0;
					gboolean ssl;
					C2POP3AuthenticationMethod auth_method;
					
					host = gnome_config_get_string ("incoming_server_hostname");
					port = gnome_config_get_int ("incoming_server_port");
					user = gnome_config_get_string ("incoming_server_username");
					pass = gnome_config_get_string ("incoming_server_password");
					ssl = gnome_config_get_bool ("incoming_server_ssl");
					auth_method = gnome_config_get_int ("incoming_auth_method");
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
						c2_pop3_set_save_password (pop3, gnome_config_get_bool ("incoming_auth_remember"));
						c2_account_set_extra_data (account, C2_ACCOUNT_KEY_INCOMING, GTK_TYPE_OBJECT, pop3);
					} else if (account_type == C2_ACCOUNT_IMAP)
					{
						C2IMAP *imap;

						imap = c2_imap_new (account, host, port, user, pass, NULL,
											auth_method, ssl);

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

	
	/* LOAD MAILBOXES */
	load_mailboxes_at_start = c2_preferences_get_general_options_start_load ();
	for (application->mailbox = NULL, i = 1; i <= quantity; i++)
	{
		gchar *name;
		gchar *id;
		C2MailboxType type;
		C2MailboxSortBy sort_by;
		GtkSortType sort_type;
		C2MailboxUseAs use_as;
		gchar *path;
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
		use_as = gnome_config_get_int ("use_as");

		switch (type)
		{
			case C2_MAILBOX_CRONOSII:
				mailbox = c2_mailbox_new (&application->mailbox, name, id, type, sort_by, sort_type);
				break;
			case C2_MAILBOX_SPOOL:
				path = gnome_config_get_string ("path");
				mailbox = c2_mailbox_new (&application->mailbox, name, id, type, sort_by, sort_type, path);
				g_free (path);
				break;
			default:
				mailbox = NULL;
		}

		c2_mailbox_set_use_as (application->mailbox, mailbox, use_as);

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
		if (c2_mailbox_get_use_as (mailbox) & C2_MAILBOX_USE_AS_OUTBOX)
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

	/* LOAD PIXMAPS */
	style = gtk_widget_get_default_style ();
	pixmap = gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/mark-i-readed.png");
	application->pixmap_i_read = GNOME_PIXMAP (pixmap)->pixmap;
	application->mask_i_read = GNOME_PIXMAP (pixmap)->mask;
	pixmap = gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/mark-i-unreaded.png");
	application->pixmap_i_unread = GNOME_PIXMAP (pixmap)->pixmap;
	application->mask_i_unread = GNOME_PIXMAP (pixmap)->mask;
	pixmap = gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/mark-i-replied.png");
	application->pixmap_i_reply = GNOME_PIXMAP (pixmap)->pixmap;
	application->mask_i_reply = GNOME_PIXMAP (pixmap)->mask;
	pixmap = gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/mark-i-forwarded.png");
	application->pixmap_i_forward = GNOME_PIXMAP (pixmap)->pixmap;
	application->mask_i_forward = GNOME_PIXMAP (pixmap)->mask;
	pixmap = gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/mark-readed.png");
	application->pixmap_read = GNOME_PIXMAP (pixmap)->pixmap;
	application->mask_read = GNOME_PIXMAP (pixmap)->mask;
	pixmap = gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/mark-unreaded.png");
	application->pixmap_unread = GNOME_PIXMAP (pixmap)->pixmap;
	application->mask_unread = GNOME_PIXMAP (pixmap)->mask;
	pixmap = gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/mark-replied.png");
	application->pixmap_reply = GNOME_PIXMAP (pixmap)->pixmap;
	application->mask_reply = GNOME_PIXMAP (pixmap)->mask;
	pixmap = gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/mark-forwarded.png");
	application->pixmap_forward = GNOME_PIXMAP (pixmap)->pixmap;
	application->mask_forward = GNOME_PIXMAP (pixmap)->mask;

	/* LOAD FONTS */
	buf = c2_preferences_get_interface_fonts_readed_mails ();
	application->fonts_gdk_readed_mails = gdk_font_load (buf);
	g_free (buf);
	buf = c2_preferences_get_interface_fonts_unreaded_mails ();
	application->fonts_gdk_unreaded_mails = gdk_font_load (buf);
	g_free (buf);
	buf = c2_preferences_get_interface_fonts_readed_mailbox ();
	application->fonts_gdk_readed_mailbox = gdk_font_load (buf);
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

	/* Check that we have an Inbox, Outbox, Sent Items, Trash and Drafts */
	if (!C2_IS_MAILBOX (c2_mailbox_get_by_usage (application->mailbox, C2_MAILBOX_USE_AS_INBOX)))
		c2_application_dialog_missing_mailbox_inform (application, C2_MAILBOX_INBOX);
	if (!C2_IS_MAILBOX (c2_mailbox_get_by_usage (application->mailbox, C2_MAILBOX_USE_AS_OUTBOX)))
		c2_application_dialog_missing_mailbox_inform (application, C2_MAILBOX_OUTBOX);
	if (!C2_IS_MAILBOX (c2_mailbox_get_by_usage (application->mailbox, C2_MAILBOX_USE_AS_SENT_ITEMS)))
		c2_application_dialog_missing_mailbox_inform (application, C2_MAILBOX_SENT_ITEMS);
	if (!C2_IS_MAILBOX (c2_mailbox_get_by_usage (application->mailbox, C2_MAILBOX_USE_AS_TRASH)))
		c2_application_dialog_missing_mailbox_inform (application, C2_MAILBOX_TRASH);
	if (!C2_IS_MAILBOX (c2_mailbox_get_by_usage (application->mailbox, C2_MAILBOX_USE_AS_DRAFTS)))
		c2_application_dialog_missing_mailbox_inform (application, C2_MAILBOX_DRAFTS);

	/* Check that the package has been installed */
	for (i = 0; check_files[i]; i++)
	{
		if (stat (check_files[i], &stbuf) < 0)
		{
			GtkWidget *dialog;
			gchar *str;

			c2_error_set (-errno);
			str = g_strdup_printf (_("The package Cronos II hasn't been installed "
									 "properly.\n"
									 "Please, reinstall the package to fix this problem.\n\n"
									 "The error is that the file %s couldn't be load because %s."),
									 check_files[i], c2_error_get ());
			
			dialog = gnome_error_dialog (str);
			g_free (str);
			gnome_dialog_run_and_close (GNOME_DIALOG (dialog));
			abort ();
		}
	}

#ifdef USE_DEBUG
	if (_debug_widget_application)
		C2_PRINTD (MOD, "Application initializated\n");
#endif
}

static void
destroy (GtkObject *object)
{
	C2Application *application = C2_APPLICATION (object);
	GSList *l;

	close (application->server_socket);

	if (application->acting_as_server)
	{
		gchar *path;

		path = g_strdup_printf ("%s" C2_HOME, g_get_home_dir ());
		chdir (path);

		unlink (C2_UNIX_SOCKET);
	}

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

#if 0
	FD_ZERO (&rset);
	FD_SET (application->server_socket, &rset);

	if (select (application->server_socket+1, &rset, NULL, NULL, NULL) < 0)
	{
		perror ("select");
		return;
	}
	
	memset (&sa, 0, sizeof (sa));
	sa.sun_family = AF_UNIX;
	strncpy (sa.sun_path, C2_UNIX_SOCKET, sizeof (sa.sun_path) -1);

	path = g_strdup_printf ("%s" C2_HOME, g_get_home_dir ());
	chdir (path);
	g_free (path);

	size = sizeof (sa);
	
	if ((sock = accept (application->server_socket, (struct sockaddr*) &sa, &size)) < 0)
		perror ("accept");
	g_free (path);
}
#endif

static void
on_server_read (C2Application *application, gint sock, GdkInputCondition cond)
{
	gchar *buffer = NULL;
	size_t size;
	gchar *cmnd;
	gint i;

	c2_mutex_lock (application->server_lock);

#ifdef USE_DEBUG
	if (_debug_widget_application)
		C2_PRINTD (MOD, "Reciving a command\n");
#endif
	
start:
	if (c2_net_read (sock, &buffer) < 0)
	{
		if (errno == EINVAL)
		{
			struct sockaddr_un sa;
			size_t size;
			gchar *path;
			
			memset (&sa, 0, sizeof (sa));
			sa.sun_family = AF_UNIX;
			strncpy (sa.sun_path, C2_UNIX_SOCKET, sizeof (sa.sun_path) -1);

			path = g_strdup_printf ("%s" C2_HOME, g_get_home_dir ());
			chdir (path);
			g_free (path);
	
			size = sizeof (sa);
	
			if ((sock = accept (application->server_socket, (struct sockaddr*) &sa, &size)) < 0)
				perror ("accept");
			else
				goto start;
		} else
			perror ("read");
		return;
	}

	/* Get the command */
	size = strlen (buffer);
	if (buffer[size-1] == '\n')
		buffer[size-1] = '\0';
	cmnd = c2_str_get_word (0, buffer, ' ');

	for (i = 0; commands[i].func; i++)
	{
		if (c2_streq (commands[i].cmnd, cmnd))
		{
			gpointer data;

			data = g_malloc0 (commands[i].params);
			
			if (c2_streq (commands[i].cmnd, C2_COMMAND_WINDOW_MAIN_NEW))
			{
				gchar *arg1;

				arg1 = c2_str_get_word (1, buffer, '\n'); /* Usar en lugar de \n algun caracter q no sea ni \r ni \0 ni \n */
				if (!strlen (arg1))
				{
					g_free (arg1);
					arg1 = NULL;
				}

				c2_application_remote_command (application, C2_COMMAND_WINDOW_MAIN_NEW, arg1);
			} else if (c2_streq (commands[i].cmnd, C2_COMMAND_COMPOSER_NEW))
			{
//				buf = c2_str_get_word (1, buffer, 
			} else if (c2_streq (commands[i].cmnd, C2_COMMAND_CHECK_MAIL))
			{
				c2_application_remote_command (application, C2_COMMAND_CHECK_MAIL);
			} else if (c2_streq (commands[i].cmnd, C2_COMMAND_WINDOW_MAIN_RAISE))
			{
				gchar *arg1;

				arg1 = c2_str_get_word (1, buffer, ' ');
				if (c2_streq (arg1, "(MTAnull)"))
				{
					g_free (arg1);
					arg1 = NULL;
				}
				
				c2_application_remote_command (application, C2_COMMAND_WINDOW_MAIN_RAISE, arg1);
				
				g_free (arg1);
			} else if (c2_streq (commands[i].cmnd, C2_COMMAND_WINDOW_MAIN_HIDE))
			{
				c2_application_remote_command (application, C2_COMMAND_WINDOW_MAIN_HIDE);
			} else if (c2_streq (commands[i].cmnd, C2_COMMAND_EXIT))
			{
				c2_application_remote_command (application, C2_COMMAND_EXIT);
			} else
			{
				g_warning (_("Recived unknown command: '%s'\n"), buffer);
			}
		}
	}

	g_free (buffer);

#ifdef USE_DEBUG
	if (_debug_widget_application)
		C2_PRINTD (MOD, "Command received: %s\n");
#endif

	c2_mutex_unlock (application->server_lock);
}

static void
on_transfer_list_finish (C2TransferList *tl, C2Application *application)
{
#ifdef USE_DEBUG
	if (_debug_widget_application)
		C2_PRINTD (MOD, "Reconnecting background checking timeout\n");
#endif
	
	application->check_timeout = gtk_timeout_add (
							c2_preferences_get_general_options_timeout_check () * 6000,
							(GtkFunction) on_application_timeout_check, application);
}

static gboolean
_check_real (C2Application *application)
{
	C2Account *account;
	GtkWidget *wtl;
	C2TransferItem *wti;
	GSList *plist = NULL, *l;
	gboolean silent;
	
	c2_return_val_if_fail (C2_IS_APPLICATION (application), FALSE, C2EDATA);

#ifdef USE_DEBUG
	if (_debug_widget_application)
		C2_PRINTD (MOD, "Checking the accounts\n");
#endif

	/* Get the silent option */
	silent = gtk_object_get_data (GTK_OBJECT (application), "check::silent") ? TRUE : FALSE;

	wtl = c2_application_window_get (application, C2_WIDGET_TRANSFER_LIST_TYPE);

	if (!wtl || !C2_IS_TRANSFER_LIST (wtl))
		wtl = c2_transfer_list_new (application);

	/* First put all the actions we have to process */
	for (account = application->account; account; account = c2_account_next (account))
	{
		gpointer data = c2_account_get_extra_data (account, C2_ACCOUNT_KEY_ACTIVE, NULL);

		if (!GPOINTER_TO_INT (data) || account->type == C2_ACCOUNT_IMAP)
			continue;

		wti = c2_transfer_item_new (application, account, C2_TRANSFER_ITEM_RECEIVE);
		c2_transfer_list_add_item (C2_TRANSFER_LIST (wtl), wti);
		plist = g_slist_append (plist, (gpointer) wti);
	}

	/* If it is not "check::silent" we remove the timeout
	 * and connect to the "finish" signal of the transfer-list,
	 * so we can add the timeout again.
	 */
	if (c2_preferences_get_general_options_timeout_check ())
	{
		gtk_timeout_remove (application->check_timeout);
		gtk_signal_connect (GTK_OBJECT (wtl), "finish",
							GTK_SIGNAL_FUNC (on_transfer_list_finish), application);
	}
	
	if (!silent)
	{
		/* Show the dialog */
		gtk_widget_show (wtl);
		gdk_window_raise (wtl->window);
	}

	/* And now fire the processes */
	for (l = plist; l; l = g_slist_next (l))
	{
		c2_transfer_item_start ((C2TransferItem*) l->data);
	}
	g_slist_free (plist);
	plist = NULL;

#ifdef USE_DEBUG
	if (_debug_widget_application)
		C2_PRINTD (MOD, "Account checking fired\n");
#endif

	return FALSE;
}

/**
 * COMMANDS:
 * "check::wait_idle" (gboolean wait) = Will check on IDLE.
 * "check::silent" (gboolean silent) = Will run in the background (no UI).
 **/
static void
_check (C2Application *application)
{
	gboolean wait_idle = FALSE;
	gboolean exists_account = c2_application_check_checkeable_account_exists (application);

	if (gtk_object_get_data (GTK_OBJECT (application), "check::wait_idle"))
		wait_idle = TRUE;

	if (!exists_account && !gtk_object_get_data (GTK_OBJECT (application), "check::silent"))
	{
		if (!wait_idle)
			return;
		
		if (!c2_application_check_account_exists (application))
			return;
	} else if (!exists_account)
		return;
	
	if (wait_idle)
		gtk_idle_add ((GtkFunction) (_check_real), application);
	else
		_check_real (application);
}

static void
compact_mailboxes_on_mailbox_compacted (C2Mailbox *mailbox, gint cbytes, gint tbytes, GladeXML *xml)
{
	gint *ccbytes, *ctbytes;

	ccbytes = (gint*) gtk_object_get_data (GTK_OBJECT (xml), "cbytes");
	ctbytes = (gint*) gtk_object_get_data (GTK_OBJECT (xml), "tbytes");

	printf ("cbytes = %d -- tbytes = %d\n", *ccbytes, *ctbytes);

	*ccbytes += cbytes;
	*ctbytes += tbytes;
}

static gboolean
compact_mailboxes_on_timeout (GtkWidget *progress)
{
	gfloat value;

	gdk_threads_enter ();
	value = gtk_progress_get_current_percentage (GTK_PROGRESS (progress));
	gdk_threads_leave ();
	
	if (value < 0.50)
		value += 0.03;
	else if (value < 0.60)
		value += 0.01;
	else if (value < 0.80)
		value += 0.001;
	else
		return FALSE;

	gdk_threads_enter ();
	gtk_progress_set_percentage (GTK_PROGRESS (progress), value);
	gdk_threads_leave ();
	return TRUE;
}

static void
compact_mailboxes_thread (GladeXML *xml)
{
	C2Mailbox *mailbox;
	C2Application *application;
	GtkWidget *label, *progress, *hbox;
	gchar *buf;
	guint16 timeout;
	gint *cbytes, *tbytes, i;

	application = C2_APPLICATION (gtk_object_get_data (GTK_OBJECT (xml), "application"));
	cbytes = g_new0 (gint, 1);
	tbytes = g_new0 (gint, 1);
	*cbytes = 0;
	*tbytes = 0;
	gtk_object_set_data (GTK_OBJECT (xml), "cbytes", (gpointer) cbytes);
	gtk_object_set_data (GTK_OBJECT (xml), "tbytes", (gpointer) tbytes);
	
	hbox = glade_xml_get_widget (xml, "hbox");
	label = glade_xml_get_widget (xml, "mailbox_label");
	progress = glade_xml_get_widget (xml, "progress");

	gtk_widget_show (hbox);

	for (mailbox = application->mailbox, i = 0; mailbox; mailbox = mailbox->next, i++)
	{
		gtk_signal_connect (GTK_OBJECT (mailbox), "compacted",
							GTK_SIGNAL_FUNC (compact_mailboxes_on_mailbox_compacted), xml);

		gdk_threads_enter ();
		buf = g_strdup_printf ("«%s»", mailbox->name);
		gtk_label_set_text (GTK_LABEL (label), buf);
		g_free (buf);
		gtk_progress_set_percentage (GTK_PROGRESS (progress), 0);
		gdk_threads_leave ();

		timeout = gtk_timeout_add (10, (GtkFunction)compact_mailboxes_on_timeout, progress);

		c2_db_compact (mailbox);

		gtk_timeout_remove (timeout);

		gdk_threads_enter ();
		gtk_progress_set_percentage (GTK_PROGRESS (progress), 1);
		gdk_threads_leave ();
	}

	/* Show the information dialog */
	{
		GladeXML *ixml;
		GtkWidget *iwidget;
		gchar *ibuf, *ibuf2, *ibuf3;

		gdk_threads_enter ();
		
		ixml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("dialogs"), "dlg_compact_inform");

		/* Mailboxes Label */
		iwidget = glade_xml_get_widget (ixml, "mailboxes_label");
		gtk_label_get (GTK_LABEL (iwidget), &ibuf);
		ibuf2 = g_strdup_printf ("%d", i);
		ibuf3 = c2_str_replace_all (ibuf, _("%MAILBOXES%"), ibuf2);
		g_free (ibuf);
		g_free (ibuf2);
		ibuf = ibuf3;
		gtk_label_set_text (GTK_LABEL (iwidget), ibuf);

		/* Cspace Label */
		iwidget = glade_xml_get_widget (ixml, "cspace_label");
		gtk_label_get (GTK_LABEL (iwidget), &ibuf);

		if (*cbytes < 1024)
			ibuf2 = g_strdup_printf ("%d bytes", *cbytes);
		else
			ibuf2 = g_strdup_printf ("%.1f Mb", (*cbytes)/1024);

		ibuf3 = c2_str_replace_all (ibuf, _("%CSPACE%"), ibuf2);
		g_free (ibuf);
		g_free (ibuf2);
		ibuf = ibuf3;
		gtk_label_set_text (GTK_LABEL (iwidget), ibuf);

		/* Tspace Label */
		iwidget = glade_xml_get_widget (ixml, "tspace_label");
		gtk_label_get (GTK_LABEL (iwidget), &ibuf);

		if (*tbytes < 1024)
			ibuf2 = g_strdup_printf ("%d bytes", *tbytes);
		else
			ibuf2 = g_strdup_printf ("%.1f Mb", (*tbytes)/1024);

		ibuf3 = c2_str_replace_all (ibuf, _("%TSPACE%"), ibuf2);
		g_free (ibuf);
		g_free (ibuf2);
		ibuf = ibuf3;
		gtk_label_set_text (GTK_LABEL (iwidget), ibuf);

		iwidget = glade_xml_get_widget (ixml, "dlg_compact_inform");

		gtk_widget_show (iwidget);

		gtk_object_destroy (GTK_OBJECT (ixml));

		gdk_threads_leave ();
	}

	/* Free everything */
	g_free (cbytes);
	g_free (tbytes);
}

static void
_compact_mailboxes (C2Application *application)
{
	GladeXML *xml;
	GtkWidget *window, *widget, *html;
	GtkStyle *style;
	pthread_t thread;

	xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("dialogs"), "dlg_compact");
	gtk_object_set_data (GTK_OBJECT (xml), "application", application);

	window = glade_xml_get_widget (xml, "dlg_compact");
	
	widget = glade_xml_get_widget (xml, "speed_up_label");
	style = gtk_style_copy (gtk_widget_get_style (widget));
	style->font = gdk_font_load ("-adobe-helvetica-bold-r-normal-*-*-240-*-*-p-*-iso8859-1");
	style->fg[GTK_STATE_NORMAL] = style->white;
	gtk_widget_set_style (widget, style);

	widget = glade_xml_get_widget (xml, "speed_up_event");
	style = gtk_style_copy (gtk_widget_get_style (widget));
	style->bg[GTK_STATE_NORMAL] = style->black;
	gtk_widget_set_style (widget, style);

	widget = glade_xml_get_widget (xml, "container");
	html = c2_html_new (application);
	gtk_container_add (GTK_CONTAINER (widget), html);
	gtk_widget_show (html);
	c2_html_set_content_from_string (C2_HTML (html), _(
"<html><body bgcolor=#ffffff>
<p>This tool will compact your mailboxes <b>and</b> speed them up.
This means that mailboxes which reside in your file system
(such as the Cronos II mailboxes) will use <u>less disk space</u>
and will be <u>accessed much faster</u>.</p>

<p>You should run this tool whenever you notice that your mailboxes
are getting slow, or when you are running out of disk space.</p>

<p>You can now run this tool by clicking the <b>Start</b> button
below.</p>
</body></html>"
));
	

	switch (gnome_dialog_run (GNOME_DIALOG (window)))
	{
		case 0:
			widget = glade_xml_get_widget (xml, "btn_close");
			gtk_widget_set_sensitive (widget, FALSE);
			pthread_create (&thread, NULL, C2_PTHREAD_FUNC (compact_mailboxes_thread), xml);
			break;
		case 1:
			gnome_dialog_close (GNOME_DIALOG (window));
			gtk_object_destroy (GTK_OBJECT (xml));
			break;
	}
}


static void
_copy_thread (C2Pthread3 *data)
{
	C2Mailbox *mailbox;
	GList *list, *l;
	C2Window *window;
	GtkWidget *widget = NULL;
	GtkProgress *progress = NULL;
	gint length, off;
	gboolean progress_ownership = FALSE,
			 status_ownership = FALSE;

	/* Load the data */
	mailbox = C2_MAILBOX (data->v1);
	list = (GList*) data->v2;
	window = (C2Window*) (data->v3);
	g_free (data);

	/* Get the length of the list to copy */
	length = g_list_length (list);

	/* Get the appbar */
	if (window)
	{
		widget = glade_xml_get_widget (window->xml, "appbar");

		if (GNOME_IS_APPBAR (widget))
		{
			/* Try to reserve ownership over the progress bar */
			if (!c2_mutex_trylock (&window->progress_lock))
				progress_ownership = TRUE;

			/* Try to reserve ownership over the status bar */
			if (!c2_mutex_trylock (&window->status_lock))
				status_ownership = TRUE;
		}
	}

	gdk_threads_enter ();

	if (progress_ownership)
	{
		/* Configure the progressbar */
		progress = (GtkProgress*) ((GnomeAppBar*) widget)->progress;
		gtk_progress_configure (progress, 0, 0, length);
	}
	
	if (status_ownership)
	{
		/* Configure the statusbar */
		gnome_appbar_push (GNOME_APPBAR (widget), _("Copying..."));
	}

	gdk_threads_leave ();

	/* Start copying */
	c2_db_freeze (mailbox);
	for (l = list, off = 0; l; l = g_list_next (l), off++)
	{
		C2Db *db;
		
		/* Now do the actual copy */
		db = C2_DB (l->data);

		if (!db->message)
			c2_db_load_message (db);
		
		gtk_object_ref (GTK_OBJECT (db->message));
		c2_db_message_add (mailbox, db->message);
		gtk_object_unref (GTK_OBJECT (db->message));

		if (progress_ownership)
		{
			gdk_threads_enter ();
			gtk_progress_set_value (progress, off);
			gdk_threads_leave ();
		}
	}
	c2_db_thaw (mailbox);

	g_list_free (list);

	gdk_threads_enter ();
	
	if (status_ownership)
	{
		gnome_appbar_pop (GNOME_APPBAR (widget));
		c2_mutex_unlock (&window->status_lock);
	}

	if (progress_ownership)
	{
		gtk_progress_set_percentage (progress, 1.0);
		c2_mutex_unlock (&window->progress_lock);
	}
	
	gdk_threads_leave ();
}

static void
_copy (C2Application *application, GList *list, C2Window *window)
{
	C2Mailbox *mailbox;
	C2Pthread3 *data;
	pthread_t thread;
	
	c2_return_if_fail (C2_IS_APPLICATION (application), C2EDATA);
	c2_return_if_fail (list, C2EDATA);

	/* Get the mailbox where to copy to */
	if (!(mailbox = c2_application_dialog_select_mailbox (application, (GtkWindow*) window)))
	{
		if (window)
			c2_window_report (window, C2_WINDOW_REPORT_MESSAGE, error_list[C2_CANCEL_USER]);
		return;
	}

	/* Fire the thread */
	data = g_new0 (C2Pthread3, 1);
	data->v1 = mailbox;
	data->v2 = g_list_copy (list);
	data->v3 = window;
	pthread_create (&thread, NULL, C2_PTHREAD_FUNC (_copy_thread), data);
}

static void
_delete_thread (C2Pthread3 *data)
{
	C2Mailbox *fmailbox, *tmailbox;
	GList *list, *l;
	C2Window *window;
	GtkWidget *widget = NULL;
	GtkProgress *progress = NULL;
	gint length, off;
	gboolean progress_ownership = FALSE,
			 status_ownership = FALSE;

	/* Load the data */
	tmailbox = C2_MAILBOX (data->v1);
	list = (GList*) data->v2;
	window = (C2Window*) (data->v3);
	fmailbox = C2_DB (list->data)->mailbox;
	g_free (data);

	/* Get the length of the list to move */
	length = g_list_length (list);

	/* Get the appbar */
	if (window)
	{
		widget = glade_xml_get_widget (window->xml, "appbar");

		if (GNOME_IS_APPBAR (widget))
		{
			/* Try to reserve ownership over the progress bar */
			if (!c2_mutex_trylock (&window->progress_lock))
				progress_ownership = TRUE;

			/* Try to reserve ownership over the status bar */
			if (!c2_mutex_trylock (&window->status_lock))
				status_ownership = TRUE;
		}
	}

	gdk_threads_enter ();

	if (progress_ownership)
	{
		/* Configure the progressbar */
		progress = (GtkProgress*) ((GnomeAppBar*) widget)->progress;
		gtk_progress_configure (progress, 0, 0, length);
	}
	
	if (status_ownership)
	{
		/* Configure the statusbar */
		gnome_appbar_push (GNOME_APPBAR (widget), _("Deleting..."));
	}

	gdk_threads_leave ();

	/* Start moving */
	c2_db_freeze (fmailbox);
	c2_db_freeze (tmailbox);
	for (l = list, off = 0; l; l = g_list_next (l), off++)
	{
		C2Db *db;
		
		/* Now do the actual copy */
		db = C2_DB (l->data);

		if (!db->message)
			c2_db_load_message (db);
		
		gtk_object_ref (GTK_OBJECT (db->message));
		gtk_object_set_data (GTK_OBJECT (db->message), "state", (gpointer) db->state);
		if (c2_db_message_add (tmailbox, db->message))
			c2_db_message_remove (fmailbox, db);
		gtk_object_unref (GTK_OBJECT (db->message));

		if (progress_ownership)
		{
			gdk_threads_enter ();
			gtk_progress_set_value (progress, off);
			gdk_threads_leave ();
		}
	}
	c2_db_thaw (fmailbox);
	c2_db_thaw (tmailbox);

	g_list_free (list);

	gdk_threads_enter ();
	
	if (status_ownership)
	{
		gnome_appbar_pop (GNOME_APPBAR (widget));
		c2_mutex_unlock (&window->status_lock);
	}

	if (progress_ownership)
	{
		gtk_progress_set_percentage (progress, 1.0);
		c2_mutex_unlock (&window->progress_lock);
	}
	
	gdk_threads_leave ();
}

static void
_delete (C2Application *application, GList *list, C2Window *window)
{
	C2Pthread3 *data;
	pthread_t thread;
	
	c2_return_if_fail (C2_IS_APPLICATION (application), C2EDATA);
	c2_return_if_fail (list, C2EDATA);

	if (c2_preferences_get_general_options_delete_use_trash ())
	{
		/* We have to save in «Trash» */
		if (c2_streq (C2_DB (list->data)->mailbox->name, C2_MAILBOX_TRASH))
			/* This is already «Trash», we have to expunge */
			goto expunge;
		
		/* Ask for confirmation (if we are supposed to) */
		if (c2_preferences_get_general_options_delete_confirmation ())
		{
			if (!dlg_confirm_delete_message (application, (GtkWindow*) window, FALSE))
			{
				if (window)
					c2_window_report (window, C2_WINDOW_REPORT_MESSAGE,
										error_list[C2_CANCEL_USER]);
				return;
			}
		}

		/* Ok, we are ready to move everything to «Trash» */
		/* Fire the thread */
		data = g_new0 (C2Pthread3, 1);
		data->v1 = c2_mailbox_get_by_usage (application->mailbox, C2_MAILBOX_USE_AS_TRASH);
		data->v2 = g_list_copy (list);
		data->v3 = window;

		/* Check that the «Trash» mailbox was fetched correctly */
		if (!C2_IS_MAILBOX (data->v1))
		{
			if (C2_IS_WINDOW (window))
			{
				c2_window_report (window, C2_WINDOW_REPORT_WARNING,
								_("Unable to find mailbox «%s»: Deletion is not possible."), C2_MAILBOX_TRASH);
				g_list_free (data->v2);
				g_free (data);
			}

			return;
		}
		
		pthread_create (&thread, NULL, C2_PTHREAD_FUNC (_delete_thread), data);
	} else
	{ /* We have to expunge */
expunge:
		C2_APPLICATION_CLASS_FW (application)->expunge (application, list, window);
	}
}

static void
_empty_trash_thread (C2Pthread2 *data)
{
	C2Window *window = (C2Window *) data->v1;
	C2Mailbox *mailbox = C2_MAILBOX (data->v2);
	C2Db *db;
	GtkWidget *widget = NULL;
	GtkProgress *progress = NULL;
	gint length, off = 0;
	gboolean progress_ownership = FALSE,
			 status_ownership = FALSE;

	/* Get the length */
	length = c2_db_length (mailbox);

	/* Get the appbar */
	if (window)
	{
		widget = glade_xml_get_widget (window->xml, "appbar");

		if (GNOME_IS_APPBAR (widget))
		{
			/* Try to reserve ownership over the progress bar */
			if (!c2_mutex_trylock (&window->progress_lock))
				progress_ownership = TRUE;

			/* Try to reserve ownership over the status bar */
			if (!c2_mutex_trylock (&window->status_lock))
				status_ownership = TRUE;
		}
	}

	gdk_threads_enter ();

	if (progress_ownership)
	{
		/* Configure the progressbar */
		progress = (GtkProgress*) ((GnomeAppBar*) widget)->progress;
		gtk_progress_configure (progress, 0, 0, length);
	}
	
	if (status_ownership)
	{
		/* Configure the statusbar */
		gnome_appbar_push (GNOME_APPBAR (widget), _("Cleaning..."));
	}

	gdk_threads_leave ();

	c2_db_freeze (mailbox);
	for (db = mailbox->db;;)
	{
		c2_db_message_remove (mailbox, db);

		if (progress_ownership)
		{
			gdk_threads_enter ();
			gtk_progress_set_value (progress, off);
			gdk_threads_leave ();
		}
		
		if (c2_db_is_last (db))
			break;
		
		c2_db_lineal_next (db);
	}
	c2_db_thaw (mailbox);

	gdk_threads_enter ();
	if (status_ownership)
	{
		gnome_appbar_pop (GNOME_APPBAR (widget));
		c2_mutex_unlock (&window->status_lock);
	}
	gdk_threads_leave ();
}

static void
_empty_trash (C2Application *application, C2Window *window)
{
	GladeXML *xml;
	GtkWidget *dialog;
	gboolean empty_trash;
	C2Mailbox *mailbox;
	C2Pthread2 *data;
	pthread_t thread;
	
	/* Confirm */
	xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("dialogs"), "dlg_empty_trash");
	dialog = glade_xml_get_widget (xml, "dlg_empty_trash");

	c2_application_window_add (application, GTK_WINDOW (dialog));
	switch (gnome_dialog_run (GNOME_DIALOG (dialog)))
	{
		case 0:
			empty_trash = TRUE;
			break;
		default:
		case 1:
			empty_trash = FALSE;
	}
	c2_application_window_remove (application, GTK_WINDOW (dialog));
	gtk_widget_destroy (dialog);
	gtk_object_destroy (GTK_OBJECT (xml));
	

	if (!empty_trash)
		return;

	mailbox = c2_mailbox_get_by_usage (application->mailbox, C2_MAILBOX_USE_AS_TRASH);
	if (!C2_IS_MAILBOX (mailbox))
	{
		dialog = gnome_error_dialog (_("There is no mailbox marked to be used as Trash!"));
		gnome_dialog_run_and_close (GNOME_DIALOG (dialog));
		return;
	}

	data = g_new0 (C2Pthread2, 1);
	data->v1 = (gpointer) window;
	data->v2 = (gpointer) mailbox;

	pthread_create (&thread, NULL, C2_PTHREAD_FUNC (_empty_trash_thread), data);
}

static void
_expunge_thread (C2Pthread2 *data)
{
	C2Mailbox *fmailbox;
	GList *list, *l;
	C2Window *window;
	GtkWidget *widget = NULL;
	GtkProgress *progress = NULL;
	gint length, off;
	gboolean progress_ownership = FALSE,
			 status_ownership = FALSE;

	/* Load the data */
	list = (GList*) data->v1;
	window = (C2Window*) (data->v2);
	fmailbox = C2_DB (list->data)->mailbox;
	g_free (data);

	/* Get the length of the list to move */
	length = g_list_length (list);

	/* Get the appbar */
	if (window)
	{
		widget = glade_xml_get_widget (window->xml, "appbar");

		if (GNOME_IS_APPBAR (widget))
		{
			/* Try to reserve ownership over the progress bar */
			if (!c2_mutex_trylock (&window->progress_lock))
				progress_ownership = TRUE;

			/* Try to reserve ownership over the status bar */
			if (!c2_mutex_trylock (&window->status_lock))
				status_ownership = TRUE;
		}
	}

	gdk_threads_enter ();

	if (progress_ownership)
	{
		/* Configure the progressbar */
		progress = (GtkProgress*) ((GnomeAppBar*) widget)->progress;
		gtk_progress_configure (progress, 0, 0, length);
	}
	
	if (status_ownership)
	{
		/* Configure the statusbar */
		gnome_appbar_push (GNOME_APPBAR (widget), _("Deleting..."));
	}

	gdk_threads_leave ();

L	/* Start deleting */
	c2_db_freeze (fmailbox);
	for (l = list, off = 0; l; l = g_list_next (l), off++)
	{
		C2Db *db;
		
		db = C2_DB (l->data);

		c2_db_message_remove (fmailbox, db);

		if (progress_ownership)
		{
			gdk_threads_enter ();
			gtk_progress_set_value (progress, off);
			gdk_threads_leave ();
		}
	}
	c2_db_thaw (fmailbox);

L	g_list_free (list);

	gdk_threads_enter ();
L	
	if (status_ownership)
	{
		gnome_appbar_pop (GNOME_APPBAR (widget));
		c2_mutex_unlock (&window->status_lock);
	}

	if (progress_ownership)
	{
		gtk_progress_set_percentage (progress, 1.0);
		c2_mutex_unlock (&window->progress_lock);
	}
	
	gdk_threads_leave ();
}

static void
_expunge (C2Application *application, GList *list, C2Window *window)
{
	C2Pthread2 *data;
	pthread_t thread;
	
	c2_return_if_fail (C2_IS_APPLICATION (application), C2EDATA);
	c2_return_if_fail (list, C2EDATA);

	/* Ask for confirmation */
	if (!dlg_confirm_delete_message (application, (GtkWindow*) window, TRUE))
	{
		if (window)
			c2_window_report (window, C2_WINDOW_REPORT_MESSAGE,
						error_list[C2_CANCEL_USER]);
		return;
	}

	/* Ok, we are ready to move everything to «Trash» */
	/* Fire the thread */
	data = (C2Pthread2*)g_new0 (C2Pthread3, 1);
	data->v1 = g_list_copy (list);
	data->v2 = window;

	pthread_create (&thread, NULL, C2_PTHREAD_FUNC (_expunge_thread), data);
}

static void
_forward (C2Application *application, C2Db *db, C2Message *message)
{
	GtkWidget *composer;
	
	c2_return_if_fail (C2_IS_APPLICATION (application), C2EDATA);
	c2_return_if_fail (!(!C2_IS_DB (db) && !C2_IS_MESSAGE (message)), C2EDATA);

	if ((composer = c2_composer_new (application)))
	{
		c2_composer_set_message_as_forward (C2_COMPOSER (composer), db, message);
		gtk_widget_show (composer);
	}
}

static void
_move_thread (C2Pthread3 *data)
{
	C2Mailbox *fmailbox, *tmailbox;
	GList *list, *l;
	C2Window *window;
	GtkWidget *widget = NULL;
	GtkProgress *progress = NULL;
	gint length, off;
	gboolean progress_ownership = FALSE,
			 status_ownership = FALSE;

	/* Load the data */
	tmailbox = C2_MAILBOX (data->v1);
	list = (GList*) data->v2;
	window = C2_WINDOW (data->v3);
	fmailbox = C2_DB (list->data)->mailbox;
	g_free (data);

	/* Get the length of the list to move */
	length = g_list_length (list);

	/* Get the appbar */
	if (window)
	{
		widget = glade_xml_get_widget (window->xml, "appbar");

		if (GNOME_IS_APPBAR (widget))
		{
			/* Try to reserve ownership over the progress bar */
			if (!c2_mutex_trylock (&window->progress_lock))
				progress_ownership = TRUE;

			/* Try to reserve ownership over the status bar */
			if (!c2_mutex_trylock (&window->status_lock))
				status_ownership = TRUE;
		}
	}

	gdk_threads_enter ();

	if (progress_ownership)
	{
		/* Configure the progressbar */
		progress = (GtkProgress*) ((GnomeAppBar*) widget)->progress;
		gtk_progress_configure (progress, 0, 0, length);
	}
	
	if (status_ownership)
	{
		/* Configure the statusbar */
		gnome_appbar_push (GNOME_APPBAR (widget), _("Moving..."));
	}

	gdk_threads_leave ();

	/* Start moving */
	c2_db_freeze (fmailbox);
	c2_db_freeze (tmailbox);
	for (l = list, off = 0; l; l = g_list_next (l), off++)
	{
		C2Db *db;
		
		/* Now do the actual copy */
		db = C2_DB (l->data);

		if (!db->message)
			c2_db_load_message (db);
		
		gtk_object_ref (GTK_OBJECT (db->message));
		if (c2_db_message_add (tmailbox, db->message))
			c2_db_message_remove (fmailbox, db);
		gtk_object_unref (GTK_OBJECT (db->message));

		if (progress_ownership)
		{
			gdk_threads_enter ();
			gtk_progress_set_value (progress, off);
			gdk_threads_leave ();
		}
	}
	c2_db_thaw (fmailbox);
	c2_db_thaw (tmailbox);

	g_list_free (list);

	gdk_threads_enter ();
	
	if (status_ownership)
	{
		gnome_appbar_pop (GNOME_APPBAR (widget));
		c2_mutex_unlock (&window->status_lock);
	}

	if (progress_ownership)
	{
		gtk_progress_set_percentage (progress, 1.0);
		c2_mutex_unlock (&window->progress_lock);
	}
	
	gdk_threads_leave ();
}

static void
_move (C2Application *application, GList *list, C2Window *window)
{
	C2Mailbox *mailbox;
	C2Pthread3 *data;
	pthread_t thread;
	
	c2_return_if_fail (C2_IS_APPLICATION (application), C2EDATA);
	c2_return_if_fail (list, C2EDATA);

	/* Get the mailbox where to move to */
	if (!(mailbox = c2_application_dialog_select_mailbox (application, (GtkWindow*) window)))
	{
		if (window)
			c2_window_report (window, C2_WINDOW_REPORT_MESSAGE, error_list[C2_CANCEL_USER]);
		return;
	}

	/* Fire the thread */
	printf ("MOVE %d MAILS\n", g_list_length (list));
	data = g_new0 (C2Pthread3, 1);
	data->v1 = mailbox;
	data->v2 = g_list_copy (list);
	data->v3 = window;
	pthread_create (&thread, NULL, C2_PTHREAD_FUNC (_move_thread), data);
}

static void
_open_message (C2Application *application, C2Db *db, C2Message *message, const gchar *file)
{
	GtkWidget *wmail = NULL;
	
	c2_return_if_fail (C2_IS_APPLICATION (application), C2EDATA);
	c2_return_if_fail (!(!C2_IS_DB (db) && !C2_IS_MESSAGE (message) && !file), C2EDATA);

	if (C2_IS_DB (db))
	{
		wmail = c2_window_mail_new (application);
		c2_window_mail_set_db (C2_WINDOW_MAIL(wmail), db);
	} else if (C2_IS_MESSAGE (message))
	{
		wmail = c2_window_mail_new (application);
		c2_window_mail_set_message (C2_WINDOW_MAIL(wmail), message);
	} else if (file)
	{
		C2Message *message;

		wmail = c2_window_mail_new (application);
		
		message = c2_message_new ();

		if (!c2_message_set_message_from_file (message, file))
		{
			/* Failed */
			GtkWidget *dialog;
			gchar *str;

			str = g_strdup_printf (_("The file couldn't be loaded: %s"),
										c2_error_object_get (GTK_OBJECT (message)));
			dialog = gnome_warning_dialog (str);

			gtk_widget_show (dialog);

			g_free (str);
		} else
		{
			c2_window_mail_set_message (C2_WINDOW_MAIL(wmail), message);
		}
	}

	if (C2_IS_WINDOW_MAIL (wmail))
		gtk_widget_show (wmail);
}

static void
_print (C2Application *application, C2Message *message)
{
	c2_return_if_fail (C2_IS_APPLICATION (application), C2EDATA);
	c2_return_if_fail (C2_IS_MESSAGE (message), C2EDATA);
}

static void
_reply (C2Application *application, C2Db *db, C2Message *message)
{
	GtkWidget *composer;
	
	c2_return_if_fail (C2_IS_APPLICATION (application), C2EDATA);
	c2_return_if_fail (!(!C2_IS_DB (db) && !C2_IS_MESSAGE (message)), C2EDATA);

	if ((composer = c2_composer_new (application)))
	{
		c2_composer_set_message_as_reply (C2_COMPOSER (composer), db, message);
		gtk_widget_show (composer);
	}
}

static void
_reply_all (C2Application *application, C2Db *db, C2Message *message)
{
	GtkWidget *composer;
	
	c2_return_if_fail (C2_IS_APPLICATION (application), C2EDATA);
	c2_return_if_fail (!(!C2_IS_DB (db) && !C2_IS_MESSAGE (message)), C2EDATA);

	if ((composer = c2_composer_new (application)))
	{
		c2_composer_set_message_as_reply_all (C2_COMPOSER (composer), db, message);
		gtk_widget_show (composer);
	}
}

static void
_save (C2Application *application, C2Message *message, C2Window *window)
{
	gchar *file;
	gchar *buf;
	FILE *fd;
	
	c2_return_if_fail (C2_IS_APPLICATION (application), C2EDATA);
	c2_return_if_fail (C2_IS_MESSAGE (message), C2EDATA);

	gtk_object_ref (GTK_OBJECT (message));
	if (!message)
	{
		if (window)
			c2_window_report (window, C2_WINDOW_REPORT_WARNING,
								error_list[C2_FAIL_MESSAGE_SAVE], _("Unable to find message."));
		goto no_name;
	}

	buf = c2_message_get_header_field (message, "Subject:");
	file = g_strdup_printf ("%s.eml", buf);
	g_free (buf);

	fd = c2_application_dialog_select_file_save (application, &file);
	
	if (!fd)
	{
		if (window)
			c2_window_report (window, C2_WINDOW_REPORT_WARNING,
								error_list[C2_FAIL_MESSAGE_SAVE], c2_error_get ());
		goto no_name;
	}

	fprintf (fd, "%s\n\n%s", message->header, message->body);
	fclose (fd);

	if (window)
		c2_window_report (window, C2_WINDOW_REPORT_MESSAGE,
							error_list[C2_SUCCESS_MESSAGE_SAVE]);
no_name:
	gtk_object_unref (GTK_OBJECT (message));
}

static void
_send (C2Application *application)
{
	C2Mailbox *outbox;
	C2Db *db;

	c2_return_if_fail (C2_IS_APPLICATION (application), C2EDATA);
	
	outbox = c2_mailbox_get_by_usage (application->mailbox, C2_MAILBOX_USE_AS_OUTBOX);
	if (!C2_IS_MAILBOX (outbox))
		return;

	db = outbox->db;
	if (db)
	{
		GtkWidget *tl;
		
		tl = c2_application_window_get (application, C2_WIDGET_TRANSFER_LIST_TYPE);
		if (!tl || !C2_IS_TRANSFER_LIST (tl))
			tl = c2_transfer_list_new (application);

		gtk_widget_show (tl);
		gdk_window_raise (tl->window);
		
		do
		{
			C2Account *account;
			C2SMTP *smtp;
			C2TransferItem *ti;
			gchar *buf;

			c2_db_load_message (db);
			
			buf = c2_message_get_header_field (db->message, "X-CronosII-Account:");
			account = c2_account_get_by_name (application->account, buf);
			g_free (buf);
			if (!account)
				continue;

			smtp = (C2SMTP*) c2_account_get_extra_data (account, C2_ACCOUNT_KEY_OUTGOING, NULL);

			ti = c2_transfer_item_new (application, account, C2_TRANSFER_ITEM_SEND, smtp, db);
			c2_transfer_list_add_item (C2_TRANSFER_LIST (tl), ti);
			c2_transfer_item_start (ti);
		} while (c2_db_lineal_next (db));
	}
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
	 * to the outbox mailbox when the mailbox is frozen
	 */
	if (change != C2_MAILBOX_CHANGE_ANY)
		return;

	db = db->next;

	c2_db_load_message (db);

	buf = c2_message_get_header_field (db->message, "X-CronosII-Send-Type:");
	if (!buf || (((C2ComposerSendType) atoi (buf)) != C2_COMPOSER_SEND_NOW))
	{
		if(buf)
			g_free (buf);
		gtk_object_unref (GTK_OBJECT (db->message));
		return;
	}
	g_free (buf);

	buf = c2_message_get_header_field (db->message, "X-CronosII-Account:");
	account = c2_account_get_by_name (application->account, buf);
	g_free (buf);
	if (!account)
		return;

	smtp = (C2SMTP*) c2_account_get_extra_data (account, C2_ACCOUNT_KEY_OUTGOING, NULL);

	gdk_threads_enter ();
	tl = c2_application_window_get (application, C2_WIDGET_TRANSFER_LIST_TYPE);
	if (!tl || !C2_IS_TRANSFER_LIST (tl))
		tl = c2_transfer_list_new (application);
	
	ti = c2_transfer_item_new (application, account, C2_TRANSFER_ITEM_SEND, smtp, db);
	c2_transfer_list_add_item (C2_TRANSFER_LIST (tl), ti);
	c2_transfer_item_start (ti);

	gtk_widget_show (tl);
	gdk_window_raise (tl->window);
	gdk_threads_leave ();
}

static gboolean
on_application_timeout_check (C2Application *application)
{
	printf ("Ejecutando on_application_timeout_check\n");
	/* If there's no account configured we will wait for the next timeout */
	if (!c2_application_check_checkeable_account_exists (application))
		return TRUE;
	
	gtk_object_set_data (GTK_OBJECT (application), "check::silent", 1);
	C2_APPLICATION_CLASS_FW (application)->check (application);
	gtk_object_set_data (GTK_OBJECT (application), "check::silent", NULL);
	printf ("Terminó de ejecutar %s\n", __PRETTY_FUNCTION__);

	return FALSE;
}

C2Application *
c2_application_new (const gchar *name, gboolean running_as_server)
{
	C2Application *application;
	gint timeout_check;

	application = gtk_type_new (c2_application_get_type ());

	/* Set the name of the application */
	application->name = g_strdup (name);
	application->running_as_server = running_as_server;

	if (application->acting_as_server)
	{
		timeout_check = c2_preferences_get_general_options_timeout_check ();
		
		/* Check at start */
		if (!running_as_server && c2_preferences_get_general_options_start_check () &&
				c2_application_check_checkeable_account_exists (application))
		{
			gtk_object_set_data (GTK_OBJECT (application), "check::wait_idle", 1);
			C2_APPLICATION_CLASS_FW (application)->check (application);
			gtk_object_set_data (GTK_OBJECT (application), "check::wait_idle", NULL);
		} else if (timeout_check)
		{
			printf ("Conectando timeout a %d\n", timeout_check);
			application->check_timeout = gtk_timeout_add (timeout_check * 60000, /* 60000 = 60 (seconds) * 1000 */
								(GtkFunction) on_application_timeout_check, application);
		}
	}

	gtk_signal_connect (GTK_OBJECT (application), "destroy",
						GTK_SIGNAL_FUNC (destroy), NULL);

	return application;
}




#if 1 /* WINDOWS */
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
	c2_return_if_fail (GTK_IS_WINDOW (window), C2EDATA);

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

	gtk_signal_emit (GTK_OBJECT (application), signals[WINDOW_CHANGED], application->open_windows);
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
	static gint asked_send_unsent_mails = 0;

	application->open_windows = g_slist_remove (application->open_windows, window);
	gtk_object_unref (GTK_OBJECT (application));

	if (asked_send_unsent_mails == 1)
		return;

	if (!application->running_as_server && GTK_OBJECT (application)->ref_count <= 1)
	{
		C2Mailbox *mailbox;

		mailbox = c2_mailbox_get_by_usage (application->mailbox, C2_MAILBOX_USE_AS_OUTBOX);
		
		asked_send_unsent_mails++;
		if (asked_send_unsent_mails == 1 &&
			c2_db_length (mailbox) &&
			c2_application_dialog_send_unsent_mails (application))
		{
			C2_APPLICATION_CLASS_FW (application)->send (application);
			asked_send_unsent_mails++;
			return;
		} else
		{
			asked_send_unsent_mails++;
			gtk_object_destroy (GTK_OBJECT (application));
		}
	} else
		gtk_signal_emit (GTK_OBJECT (application), signals[WINDOW_CHANGED], application->open_windows);
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

	return NULL;
}

GSList *
c2_application_open_windows (C2Application *application)
{
	return application->open_windows;
}

#endif





#if 1 /* COMMANDS */
/**
 * _c2_application_command
 * @application: C2Application object.
 * @local: If the command was originated from a LOCAL instance of Cronos II set to %TRUE.
 *         Command will be local ALWAYS but when passed through the server pipe.
 * @cmnd: Command to execute.
 * @...: Parameters for the command. Check widget-application.h for more information.
 *
 * This function will execute a command in the application pointed by @application.
 **/
void
_c2_application_command (C2Application *application, gboolean local, const gchar *cmnd, ...)
{
	gint i;
	va_list args;
	gchar *rcmnd = NULL;

	va_start (args, cmnd);

	if (application->acting_as_server)
	{
		for (i = 0; commands[i].func; i++)
			if (c2_streq (commands[i].cmnd, cmnd))
			{
#ifdef USE_DEBUG
				if (_debug_widget_application)
					C2_PRINTD (MOD, "Executing command: %s\n", cmnd);
#endif

				if (!local)
					gdk_threads_enter ();
				commands[i].func (application, args);
				
				if (!local)
					gdk_threads_leave ();
				break;
			}
	} else
	{
		for (i = 0; commands[i].func; i++)
		{
			if (c2_streq (commands[i].cmnd, cmnd))
			{
				gint l;

				for (l = 0; l < AVAILABLE_ARGS; l++)
				{
					gchar *buf = NULL, *buf2;
					GtkFundamentalType type;

					switch (l)
					{
						case 0:
							type = commands[i].arg1;
							break;
						case 1:
							type = commands[i].arg2;
							break;
						case 2:
							type = commands[i].arg3;
							break;
						case 3:
							type = commands[i].arg4;
							break;
						case 4:
							type = commands[i].arg5;
							break;
						case 5:
							type = commands[i].arg6;
							break;
						case 6:
							type = commands[i].arg7;
							break;
						case 7:
							type = commands[i].arg8;
							break;
						default:
							type = GTK_TYPE_NONE;
					}

					switch (type)
					{
						case GTK_TYPE_UCHAR:
						case GTK_TYPE_CHAR:
							buf = g_strdup_printf ("%c", va_arg (args, gint));
							break;
						case GTK_TYPE_BOOL:
							buf = g_strdup_printf ("%d", va_arg (args, gboolean));
							break;
						case GTK_TYPE_ENUM:
						case GTK_TYPE_UINT:
						case GTK_TYPE_INT:
							buf = g_strdup_printf ("%d", va_arg (args, gint));
							break;
						case GTK_TYPE_ULONG:
						case GTK_TYPE_LONG:
							buf = g_strdup_printf ("%ld", va_arg (args, glong));
							break;
						case GTK_TYPE_FLOAT:
							buf = g_strdup_printf ("%f", va_arg (args, gdouble));
							break;
						case GTK_TYPE_DOUBLE:
							buf = g_strdup_printf ("%lf", va_arg (args, gdouble));
							break;
						case GTK_TYPE_STRING:
							buf = g_strdup_printf ("%s", va_arg (args, gchar*));
							break;
					}

					if (type == GTK_TYPE_NONE)
						break;

					if (rcmnd)
					{
						buf2 = g_strdup_printf ("%s %s", rcmnd, buf);
						g_free (rcmnd);
						rcmnd = buf2;
					} else
					{
						rcmnd = g_strdup_printf ("%s %s", cmnd, buf);
					}
					g_free (buf);
				}

				break;
			}
		}

		if (!rcmnd)
			rcmnd = g_strdup (cmnd);

		c2_net_send (application->server_socket, "%s\n", rcmnd);
		g_free (rcmnd);
	}

	va_end (args);
}

static void
on_command_wmain_new_window_main_show (GtkWidget *widget, C2WindowMain *wmain)
{
	C2Mailbox *mailbox;
	gchar *sel_mailbox;

	sel_mailbox = gtk_object_get_data (GTK_OBJECT (wmain), "command_wmain_new::mailbox");

	if (sel_mailbox)
	{
		mailbox = c2_mailbox_get_by_name (application->mailbox, sel_mailbox);
	} else
	{
		mailbox = c2_mailbox_get_by_usage (application->mailbox, C2_MAILBOX_USE_AS_INBOX);
		if (!mailbox)
			if (!(mailbox = c2_mailbox_get_by_usage (application->mailbox, C2_MAILBOX_USE_AS_INBOX)))
				sel_mailbox = C2_MAILBOX_INBOX;
	}
	
	if (!mailbox)
		c2_window_report (C2_WINDOW (wmain), C2_WINDOW_REPORT_WARNING,
							_("The mailbox \"%s\", specified in command line, "
							  "does not exist."), sel_mailbox);
	else
		c2_window_main_set_mailbox (wmain, mailbox);
	
	return;
}

static void
command_wmain_new (C2Application *application, va_list args)
{
	GtkWidget *widget;
	gchar *mailbox;

	mailbox = va_arg (args, gchar*);

	widget = c2_window_main_new (application);
#ifdef USE_DEBUG
	if (_debug_widget_application)
		C2_PRINTD (MOD, "Creating main window: %s\n", C2_IS_WINDOW_MAIN (widget) ? "Success" : "Failed");
#endif

	gtk_object_set_data (GTK_OBJECT (widget), "command_wmain_new::mailbox", mailbox);
	gtk_signal_connect (GTK_OBJECT (widget), "show",
							GTK_SIGNAL_FUNC (on_command_wmain_new_window_main_show), widget); \
	gtk_widget_show (widget);
}

static void
command_wmain_raise_new (C2Application *application, ...)
{
	va_list args;

	va_start (args, application);
	command_wmain_new (application, args);
	va_end (args);
}

static void
command_wmain_raise (C2Application *application, va_list args)
{
	GtkWidget *widget;

	widget = c2_application_window_get (application, "wmain");

	if (!C2_IS_WINDOW_MAIN (widget))
		command_wmain_raise_new (application, NULL);
	else
	{
		gtk_widget_show (widget);
		gdk_window_raise (widget->window);
	}
}

static void
command_wmain_hide (C2Application *application, va_list args)
{
	GtkWidget *widget;

	widget = c2_application_window_get (application, "wmain");

	if (C2_IS_WINDOW_MAIN (widget))
		gtk_widget_hide (widget);
}

/**
 * if (interpret_as_link)
 *      ... = @gchar* = String containing a mailto: link
 * else
 *      ... = @gchar*1 = String containing the list of headers to be passed, list
 *                       separated by \r.
 *            @gchar*2 = String containing the list of value of each header, list
 *                       separated by \r.
 **/
static void
command_composer_new (C2Application *application, va_list args)
{
	GtkWidget *widget;
	gchar *arg1, *arg2;
	gboolean interpret_as_link;

	if (!(widget = c2_composer_new (application)))
		return;

	interpret_as_link = va_arg (args, gboolean);
	
	if (interpret_as_link)
	{
		arg1 = va_arg (args, gchar*);

		if (!arg1)
		{
			g_warning ("command_composer_new: Caller said it was going to be interpreted "
					   "as link but didn't give any parameter\n");
		} else
		{
			c2_composer_set_contents_from_link (C2_COMPOSER (widget), arg1);
		}
	} else
	{
		if ((arg1 = va_arg (args, gchar*)))
		{
			gint i;
			
			if (!(arg2 = va_arg (args, gchar*)))
			{
				g_warning ("command_composer_new: Caller specify a header (%s) but didn't "
						   "give any value for it.\n", arg1);
				g_free (arg1);
				goto end;
			}

			for (i = 0;; i++)
			{
				gchar *header, *data;

				if (!(header = c2_str_get_word (i, arg1, '\r')) || !strlen (header))
					break;
				if (!(data = c2_str_get_word (i, arg2, '\r')))
				{
					g_free (header);
					break;
				}

				c2_composer_set_extra_field (C2_COMPOSER (widget), header, data);
				g_free (header);
				g_free (data);
			}

			g_free (arg1);
			g_free (arg2);
		}
	}

end:
	gtk_widget_show (widget);
}

static void
command_open_file (C2Application *application, va_list args)
{
	const gchar *file;

	file = va_arg (args, gchar*);
	
	C2_APPLICATION_CLASS_FW (application)->open_message (application, NULL, NULL, file);
}

static void
command_check_mail (C2Application *application, va_list args)
{
	C2_APPLICATION_CLASS_FW (application)->check (application);
}

static void
command_exit (C2Application *application, va_list args)
{
	gtk_object_destroy (GTK_OBJECT (application));
}

#endif



static void
on_preferences_changed (C2DialogPreferences *preferences, C2DialogPreferencesKey key, gpointer value)
{
	C2Application *application = C2_DIALOG (preferences)->application;

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

#if 1 /* Delete Mails Confirmation Dialog */
static void
dlg_confirm_delete_message_confirmation_btn_toggled (GtkWidget *widget)
{
	c2_preferences_set_general_options_delete_confirmation (
						!GTK_TOGGLE_BUTTON (widget)->active);
	c2_preferences_commit ();
}

static gboolean
dlg_confirm_delete_message (C2Application *application, GtkWindow *window, gboolean expunge)
{
	GtkWidget *dialog;
	GtkWidget *pixmap;
	GtkWidget *toggle;
	GladeXML *xml;
	gboolean retval;
	
	c2_return_val_if_fail (C2_IS_APPLICATION (application), 0, C2EDATA);

	application = application;

	xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("dialogs"), "dlg_confirm_delete_message");

	dialog = glade_xml_get_widget (xml, "dlg_confirm_delete_message");
	c2_application_window_add (application, GTK_WINDOW (dialog));

	pixmap = glade_xml_get_widget (xml, "pixmap");
	gnome_pixmap_load_file (GNOME_PIXMAP (pixmap), gnome_pixmap_file ("gnome-question.png"));

	toggle = glade_xml_get_widget (xml, "confirmation_btn");
	gtk_signal_connect (GTK_OBJECT (toggle), "toggled",
						GTK_SIGNAL_FUNC (dlg_confirm_delete_message_confirmation_btn_toggled), NULL);

	if (expunge)
		gtk_widget_hide (toggle);

	gtk_window_set_position (GTK_WINDOW (dialog),
					gnome_preferences_get_dialog_position ());
	if (window)
		gtk_window_set_transient_for (GTK_WINDOW (dialog), window);
	gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
	gnome_dialog_close_hides (GNOME_DIALOG (dialog), TRUE);
	if (gnome_dialog_run (GNOME_DIALOG (dialog)) == 0)
		retval = TRUE;
	else
		retval = FALSE;

	c2_application_window_remove (application, GTK_WINDOW (dialog));
	gtk_widget_destroy (dialog);
	gtk_object_destroy (GTK_OBJECT (xml));

	return retval;
}
#endif /* Delete Mails Confirmation Dialog */
