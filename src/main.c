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

#include <libcronosII/mailbox.h>
#include <libcronosII/error.h>
#include <libcronosII/hash.h>

#include "main.h"
#include "preferences.h"
#include "widget-composer.h"
#include "widget-window-main.h"

static gint
idle										(C2Application *application);

gchar *error_list[] =
{
	N_("Failed to load message: %s."),
	N_("Failed to save message: %s."),
	N_("Failed to load mailbox «%s»: %s."),
	N_("Failed to create mailbox «%s»: %s."),

	N_("Message saved successfully."),

	N_("Action cancelled by user."),
	N_("Unknown reason")
};

static struct {
	gboolean open_composer;
	
	gchar *account;
	gchar *to;
	gchar *cc;
	gchar *bcc;
	gchar *subject;
	gchar *body;

	gchar *mailto;

	gchar *mailbox;

	gboolean check;
	gboolean open_main_window;
} flags =
{
	FALSE,
	
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	
	NULL,

	NULL,

	FALSE,
	FALSE
};

static void
c2_init (gint argc, gchar **argv)
{
	static struct poptOption options[] = {
		{
			"compose", 'c', POPT_ARG_NONE,
			&(flags.open_composer), 0,
			N_("Compose a new email."), NULL
		},
		{
			"account", 'a', POPT_ARG_STRING,
			&(flags.account), 0,
			N_("Set the Account field."),
			N_("Account")
		},
		{
			"to", 't', POPT_ARG_STRING,
			&(flags.to), 0,
			N_("Set the To field."),
			N_("Address")
		},
		{
			"cc", 0, POPT_ARG_STRING,
			&(flags.cc), 0,
			N_("Set the CC field."),
			N_("Address")
		},
		{
			"bcc", 0, POPT_ARG_STRING,
			&(flags.bcc), 0,
			N_("Set the BCC field."),
			N_("Address")
		},
		{
			"subject", 's', POPT_ARG_STRING,
			&(flags.subject), 0,
			N_("Set the Subject field."),
			N_("Subject")
		},
		{
			"body", 'b', POPT_ARG_STRING,
			&(flags.body), 0,
			N_("Set the Body."),
			N_("Text")
		},
		{
			"link", 'l', POPT_ARG_STRING,
			&(flags.mailto), 0,
			N_("Compose a new email decoding the argument as a mailto: link"),
			"mailto:email@somewhere."
		},
		{
			"mailbox", 'm', POPT_ARG_STRING,
			&(flags.mailbox), 0,
			N_("Set the active mailbox at start (default=Inbox)"),
			N_("Inbox")
		},
		{
			"check", 'c', POPT_ARG_NONE,
			&(flags.check), 0,
			N_("Check account for mail."), NULL
		},
		{
			"wmain", 0, POPT_ARG_NONE,
			&(flags.open_main_window), 0,
			N_("Open the main window (default)"), NULL
		}
	};

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
	
	gnome_init_with_popt_table ("Cronos II", VERSION, argc, argv, options, 0, NULL);
	
#ifdef USE_GCONF
	gconf_init (argc, argv, NULL);
#endif
	
	glade_gnome_init ();
	c2_hash_init ();
}

#define CREATE_WINDOW_MAIN \
	{ \
		C2Mailbox *mailbox; \
		 \
		widget = c2_window_main_new (application); \
		mailbox = c2_mailbox_get_by_name (application->mailbox, \
									flags.mailbox ? flags.mailbox : C2_MAILBOX_INBOX); \
		if (!mailbox) \
			c2_window_report (C2_WINDOW (widget), C2_WINDOW_REPORT_WARNING, \
								_("The mailbox \"%s\", specified in command line, " \
								  "does not exist."), flags.mailbox); \
		else \
			c2_window_main_set_mailbox (C2_WINDOW_MAIN (widget), mailbox); \
		gtk_widget_show (widget); \
		something_opened = TRUE; \
	}

gint
main (gint argc, gchar **argv)
{
	GtkWidget *widget;
	gboolean something_opened = FALSE;

	/* Initialization of GNOME and Glade */
	c2_init (argc, argv);

	gdk_threads_enter ();
	application = c2_application_new (PACKAGE);

	if (flags.open_main_window)
		CREATE_WINDOW_MAIN;

	if (flags.open_composer || flags.account || flags.to || flags.cc ||
		flags.bcc || flags.subject || flags.body || flags.mailto)
	{
		widget = c2_composer_new (application);

		if (flags.mailto)
			c2_composer_set_contents_from_link (C2_COMPOSER (widget), flags.mailto);
		else if (flags.account)
			c2_composer_set_extra_field (C2_COMPOSER (widget), C2_COMPOSER_ACCOUNT,
										flags.account);
		gtk_widget_show (widget);
		something_opened = TRUE;
	}

	if (!something_opened)
		CREATE_WINDOW_MAIN;

	/* Release Information Dialog */
	if (c2_preferences_get_extra_release_information_show ())
		gtk_idle_add (idle, application);
	
	gtk_main ();
	gdk_threads_leave ();
	
	gnome_config_sync ();
	c2_hash_destroy ();

	return 0;
}

static gint
idle (C2Application *application)
{
	c2_application_dialog_release_information (application);

	return FALSE;
}
