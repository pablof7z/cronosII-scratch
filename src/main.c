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
#include <config.h>
#include <gnome.h>
#include <signal.h>

#ifdef USE_GCONF
#	include <gconf/gconf.h>
#endif

#include <libcronosII/mailbox.h>
#include <libcronosII/error.h>
#include <libcronosII/hash.h>

#include "main.h"
#include "preferences.h"
#include "widget-application-utils.h"
#include "widget-composer.h"
#include "widget-window.h"
#include "widget-window-main.h"

static gint
on_release_information_idle					(gpointer);

#ifdef USE_DEBUG
static gint
on_intensive_test_timeout					(gpointer data);
#endif

C2Application *
global_application = NULL;

gchar *error_list[] =
{
	N_("Failed to load message: %s."),
	N_("Failed to save message: %s."),
	N_("Failed to load mailbox «%s»: %s."),
	N_("Failed to create mailbox «%s»: %s."),
	N_("Failed to save file: %s."),

	N_("Message saved successfully."),
	N_("File saved successfully."),

	N_("Action cancelled by user."),
	N_("There is no selected mailbox."),
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
	gboolean be_server;
	
	gboolean hide_wmain;
	gboolean raise_wmain;

	gboolean exit;

#ifdef USE_DEBUG
	gchar *intensive_test;
	gboolean intensive_test_list;
#endif
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
	FALSE,
	FALSE,
	FALSE,
	FALSE,
	FALSE,

#ifdef USE_DEBUG
	NULL,
	FALSE
#endif
};

void on_sigsegv (int signal)
{
	GtkWidget *dialog;

	dialog = gnome_error_dialog (_("An internal error has crashed Cronos II!\n"
 							       "\n"
								   "Your unsaved data is going to be saved\n"
								   "so you do not lose any information.\n"
								   "\n"
								   "We apologise for this inconvenient.\n"));
	gnome_dialog_run_and_close (dialog);

	gtk_signal_emit_by_name (GTK_OBJECT (global_application), "emergency_data_save");
	c2_preferences_set_application_crashed (TRUE);
	abort ();
}

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
			"subject", 'u', POPT_ARG_STRING,
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
			N_("mailto:email@somewhere.")
		},
		{
			"mailbox", 'm', POPT_ARG_STRING,
			&(flags.mailbox), 0,
			N_("Set the active mailbox at start (default=Inbox)"),
			N_("Inbox")
		},
		{
			"check", 'f', POPT_ARG_NONE,
			&(flags.check), 0,
			N_("Check account for mail."), NULL
		},
		{
			"wmain", 0, POPT_ARG_NONE,
			&(flags.open_main_window), 0,
			N_("Open the main window (default)"), NULL
		},
		{
			"server", 's', POPT_ARG_NONE,
			&(flags.be_server), 0,
			N_("Don't open any windows; instead act as a server for quick startup of new Cronos II instances"),
			NULL
		},
		{
			"hide_wmain", 0, POPT_ARG_NONE,
			&(flags.hide_wmain), 0,
			N_("Hides the main window."), NULL
		},
		{
			"raise_wmain", 0, POPT_ARG_NONE,
			&(flags.raise_wmain), 0,
			N_("Shows the main window."), NULL
		},
		{
			"exit", 0, POPT_ARG_NONE,
			&(flags.exit), 0,
			N_("Finishes Cronos II and its multisession server"),
			NULL
		}
#ifdef USE_DEBUG
		, {
			"intensive-test", 0, POPT_ARG_STRING,
			&(flags.intensive_test), 0,
			N_("Runs an intensive test"),
			NULL
		},
		{
			"intensive-test-list", 0, POPT_ARG_NONE,
			&(flags.intensive_test_list), 0,
			N_("Shows a list of available intensive tests and quit"),
			NULL
		}
#endif
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
	
	glade_gnome_init ();
	c2_hash_init ();
	
	signal (SIGSEGV, on_sigsegv);
}

#define CREATE_WINDOW_MAIN \
	{ \
		c2_application_command (application, C2_COMMAND_WINDOW_MAIN_NEW, flags.mailbox); \
		something_opened = TRUE; \
	}

gint
main (gint argc, gchar **argv)
{
	gboolean something_opened = FALSE;
	gchar *version;

#ifdef USE_DEBUG
	/* Redirect the error output */
	setbuf (stdout, NULL);
#endif

	/* Initialization of GNOME and Glade */
	c2_init (argc, argv);

#ifdef USE_DEBUG
	if (flags.intensive_test_list)
	{
			printf ("\nAvailable intensive tests are:\n"
					"%18s\t\t%s.\n"
					"\n",
					"mailbox", "Makes a hard tests creating, manipulating and deleting mailboxes");
			exit (0);
	}
#endif

	gdk_threads_enter ();

	c2_font_bold	= "-adobe-helvetica-bold-r-normal-*-*-120-*-*-p-*-iso8859-1";
	c2_font_italic	= "-adobe-helvetica-medium-o-normal-*-*-120-*-*-p-*-iso8859-1";

	/* Get the Version of the Application */
	version = c2_preferences_get_application_version ();
	if (!version)
	{
		c2_install_new ();
		gtk_main ();
	}

	/* Create the Application object */
	if (!(application = c2_application_new (PACKAGE, flags.be_server)))			
		return 1;
	global_application = application;

	if (flags.be_server)
	{
		something_opened = TRUE;
	}
	
	/* Open specified windows */
	if (flags.open_main_window)
	{
		CREATE_WINDOW_MAIN;
	}
	
	/* Composer */
	if (flags.open_composer || flags.account || flags.to || flags.cc ||
		flags.bcc || flags.subject || flags.body || flags.mailto)
	{
		gchar *headers = NULL;
		gchar *values = NULL;
		gchar *buf = NULL;
		gboolean interpret_as_link = FALSE;
		
		if (flags.mailto)
		{
			interpret_as_link = TRUE;
			headers = flags.mailto;
		} else
		{
			if (flags.account)
			{
				if (headers)
				{
					buf = g_strdup_printf ("%s\r%s", headers, C2_COMPOSER_ACCOUNT);
					g_free (headers);
				} else
					buf = g_strdup (C2_COMPOSER_ACCOUNT);
				headers = buf;
				
				if (values)
				{
					buf = g_strdup_printf ("%s\r%s", values, flags.account);
					g_free (values);
				} else
					buf = g_strdup (flags.account);
				values = buf;
			}
					
			if (flags.to)
			{
				if (headers)
				{
					buf = g_strdup_printf ("%s\r%s", headers, C2_COMPOSER_TO);
					g_free (headers);
				} else
					buf = g_strdup (C2_COMPOSER_TO);
				headers = buf;
				
				if (values)
				{
					buf = g_strdup_printf ("%s\r%s", values, flags.to);
					g_free (values);
				} else
					buf = g_strdup (flags.to);
				values = buf;
			}
			
			if (flags.cc)
			{
				if (headers)
				{
					buf = g_strdup_printf ("%s\r%s", headers, C2_COMPOSER_CC);
					g_free (headers);
				} else
					buf = g_strdup (C2_COMPOSER_CC);
				headers = buf;
				
				if (values)
				{
					buf = g_strdup_printf ("%s\r%s", values, flags.cc);
					g_free (values);
				} else
					buf = g_strdup (flags.cc);
				values = buf;
				
			}
			
			if (flags.bcc)
			{
				if (headers)
				{
					buf = g_strdup_printf ("%s\r%s", headers, C2_COMPOSER_BCC);
					g_free (headers);
				} else
					buf = g_strdup (C2_COMPOSER_BCC);
				headers = buf;
				
				if (values)
				{
					buf = g_strdup_printf ("%s\r%s", values, flags.bcc);
					g_free (values);
				} else
					buf = g_strdup (flags.bcc);
				values = buf;
			}
			
			if (flags.subject)
			{
				if (headers)
				{
					buf = g_strdup_printf ("%s\r%s", headers, C2_COMPOSER_SUBJECT);
					g_free (headers);
				} else
					buf = g_strdup (C2_COMPOSER_SUBJECT);
				headers = buf;
				
				if (values)
				{
					buf = g_strdup_printf ("%s\r%s", values, flags.subject);
					g_free (values);
				} else
					buf = g_strdup (flags.subject);
				values = buf;
			}
			
			if (flags.body)
			{
				if (headers)
				{
					buf = g_strdup_printf ("%s\r%s", headers, C2_COMPOSER_BODY);
					g_free (headers);
				} else
					buf = g_strdup (C2_COMPOSER_BODY);
				headers = buf;
				
				if (values)
				{
					buf = g_strdup_printf ("%s\r%s", values, flags.body);
					g_free (values);
				} else
					buf = g_strdup (flags.body);
				values = buf;
			}
		}

		c2_application_command (application, C2_COMMAND_COMPOSER_NEW, interpret_as_link, headers, values);
			
		something_opened = TRUE;
	}

	/* Check mail */
	if (flags.check)
	{
		c2_application_command (application, C2_COMMAND_CHECK_MAIL);
		something_opened = TRUE;
	}

	if (flags.hide_wmain)
	{
		c2_application_command (application, C2_COMMAND_WINDOW_MAIN_HIDE);
		something_opened = TRUE;
	}

	if (flags.raise_wmain)
	{
		c2_application_command (application, C2_COMMAND_WINDOW_MAIN_RAISE);
		something_opened = TRUE;
	}

	if (flags.exit)
	{
		c2_application_command (application, C2_COMMAND_EXIT);
		something_opened = TRUE;
	}
	
	/* If nothing opened we will open the defaults window */
	if (!something_opened)
	{
		CREATE_WINDOW_MAIN;
	}

#ifdef USE_DEBUG
	if (flags.intensive_test)
	{
		printf ("Starting intensive test '%s' in 2 seconds\n", flags.intensive_test);
		gtk_timeout_add (2000, on_intensive_test_timeout, application);
	}
#endif

	if (application->acting_as_server)
	{
		/* Release Information Dialog */
		if (c2_preferences_get_extra_release_information_show ())
			gtk_idle_add (on_release_information_idle, application);
	
		gtk_main ();
		gdk_threads_leave ();
	
		gnome_config_sync ();
		c2_hash_destroy ();
	}

	return 0;
}

static gint
on_release_information_idle (gpointer data)
{
	C2Application *application = C2_APPLICATION (data);
	c2_application_dialog_release_information (application);

	return FALSE;
}

#ifdef USE_DEBUG
static void
intensive_test_mailbox (C2Application *application)
{
	C2Mailbox *test1, *test2;
	gint config_id;
	gchar *query;
	C2Message *message;
	gint i;
	GList *mlist = NULL;
	
/*	printf ("Checking if mailbox 'test1' exists: ");
	fflush (stdout);

	test1 = c2_mailbox_get_by_name (application->mailbox, "test1");
	if (C2_IS_MAILBOX (test1))
	{
		printf ("yes\n");
		printf ("Removing it...\n");
		if (c2_mailbox_remove (application->mailbox, test1))
			printf ("Success\n");
		else
			printf ("Failed: %s\n", c2_error_object_get (GTK_OBJECT (test1)));
	} else
		printf ("no\n");

	sleep (2);

	printf ("Creating mailbox 'test1'\n");
	
	gdk_threads_enter ();
	test1 = c2_mailbox_new_with_parent (&application->mailbox, "test1", NULL, C2_MAILBOX_CRONOSII,
								C2_MAILBOX_SORT_DATE, GTK_SORT_ASCENDING);
	gdk_threads_leave ();
	
	config_id = gnome_config_get_int_with_default ("/"PACKAGE"/Mailboxes/quantity=0", NULL)+1;
	query = g_strdup_printf ("/"PACKAGE"/Mailbox %d/", config_id);
	gnome_config_push_prefix (query);
	gnome_config_set_string ("name", test1->name);
	gnome_config_set_string ("id", test1->id);
	gnome_config_set_int ("type", test1->type);
	gnome_config_set_int ("sort_by", test1->sort_by);
	gnome_config_set_int ("sort_type", test1->sort_type);
	gnome_config_pop_prefix ();
	g_free (query);
	gnome_config_set_int ("/"PACKAGE"/Mailboxes/quantity", config_id);

*/	/* Do the same for test2 */
/*	test2 = c2_mailbox_get_by_name (application->mailbox, "test2");
	if (C2_IS_MAILBOX (test2))
	{
		printf ("yes\n");
		printf ("Removing it...\n");
		if (c2_mailbox_remove (application->mailbox, test2))
			printf ("Success\n");
		else
			printf ("Failed: %s\n", c2_error_object_get (GTK_OBJECT (test2)));
	} else
		printf ("no\n");

	sleep (1);

	printf ("Creating mailbox 'test2'\n");
	
	gdk_threads_enter ();
	test2 = c2_mailbox_new_with_parent (&application->mailbox, "test2", NULL, C2_MAILBOX_CRONOSII,
								C2_MAILBOX_SORT_DATE, GTK_SORT_ASCENDING);
	gdk_threads_leave ();
	
	config_id = gnome_config_get_int_with_default ("/"PACKAGE"/Mailboxes/quantity=0", NULL)+1;
	query = g_strdup_printf ("/"PACKAGE"/Mailbox %d/", config_id);
	gnome_config_push_prefix (query);
	gnome_config_set_string ("name", test1->name);
	gnome_config_set_string ("id", test1->id);
	gnome_config_set_int ("type", test1->type);
	gnome_config_set_int ("sort_by", test1->sort_by);
	gnome_config_set_int ("sort_type", test1->sort_type);
	gnome_config_pop_prefix ();
	g_free (query);
	gnome_config_set_int ("/"PACKAGE"/Mailboxes/quantity", config_id);

*/	/* We are going to add a message */
	message = c2_message_new ();
	c2_message_set_message (message, "From: Intensive Test <intensive@test.org>\n"
									 "To: You <y@u.net>\n"
									 "Subject: Just an intensive test\n"
									 "\n"
									 "This is a mail produced by an intensive test.\n");

	printf ("Making a list of 2000 messages\n");
	/* Add this message 2000 times */
	for (i = 0; i < 2000; i++)
		mlist = g_list_prepend (mlist, message);
	
	test1 = c2_mailbox_get_by_name (application->mailbox, "Drafts");
	printf ("Freezing the test1 mailbox\n");
	c2_db_freeze (test1);
	printf ("Adding the messages\n");
	c2_db_message_add_list (test1, mlist);
	printf ("Thawing the test1 mailbox\n");
	c2_db_thaw (test1);
	printf ("Done adding the 2000 messages to test1\n");
	
	
	
	c2_preferences_commit ();
}

static gint
on_intensive_test_timeout (gpointer data)
{
	pthread_t thread;
	
	printf ("Starting intensive test '%s'\n\n\n\n", flags.intensive_test);

	if (c2_streq (flags.intensive_test, "mailbox"))
		pthread_create (&thread, NULL, C2_PTHREAD_FUNC (intensive_test_mailbox), data);
	else
		printf ("Intensive test error: '%s' is not a valid intensive test.\n", flags.intensive_test);
	
	return FALSE;
}
#endif
