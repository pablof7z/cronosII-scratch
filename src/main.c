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

#include "widget-transfer-list.h"
#include "widget-window-main.h"

static void
on_flag_compose								(void);

static void
on_flag_check								(void);

static struct {
	gboolean open_composer;
	
	gchar *account;
	gchar *to;
	gchar *cc;
	gchar *bcc;
	gchar *subject;
	gchar *body;

	gchar *mailto;

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

	TRUE,
	TRUE
};

static void
c2_init (gint argc, gchar **argv)
{
	static struct poptOption options[] = {
		{
			"compose", 'm', POPT_ARG_NONE,
			on_flag_compose, 0,
			N_("Compose a new email."), NULL
		},
		{
			"account", 'a', POPT_ARG_STRING,
			NULL, 0,
			N_("Set the Account field."),
			N_("Account")
		},
		{
			"to", 't', POPT_ARG_STRING,
			NULL, 0,
			N_("Set the To field."),
			N_("Address")
		},
		{
			"cc", 'c', POPT_ARG_STRING,
			NULL, 0,
			N_("Set the CC field."),
			N_("Address")
		},
		{
			"bcc", 'b', POPT_ARG_STRING,
			NULL, 0,
			N_("Set the BCC field."),
			N_("Address")
		},
		{
			"subject", 's', POPT_ARG_STRING,
			NULL, 0,
			N_("Set the Subject field."),
			N_("Subject")
		},
		{
			"body", 'o', POPT_ARG_STRING,
			NULL, 0,
			N_("Set the Body."),
			N_("Text")
		},
		{
			"mailto", 'l', POPT_ARG_STRING,
			NULL, 0,
			N_("Compose a new email decoding the argument as a mailto: link"),
			"mailto:email@somewhere."
		},
		{
			"check", 'f', POPT_ARG_NONE,
			on_flag_check, 0,
			N_("Check account for mail."), NULL
		},
		{
			"wmain", 'w', POPT_ARG_NONE,
			NULL, 0,
			N_("Open the main window (default)"), NULL
		}
	};
	gnome_init_with_popt_table ("Cronos II", VERSION, argc, argv, options, 0, NULL);
	glade_gnome_init ();
	c2_hash_init ();
}

gint
main (gint argc, gchar **argv)
{
	GtkWidget *main_window;
	GtkWidget *transfer_list;
	C2TransferItem *transfer_item;

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

	application = c2_application_new (PACKAGE);

	if (flags.open_main_window)
	{
		main_window = c2_window_main_new (application);
		gtk_widget_show (main_window);
	}
	if (flags.check)
	{
		C2Account *account =
			c2_account_new ("Cronos II", "Pablo Fernández Navarro",
							"Cronos II", "cronosII@users.sourceforge.net",
							NULL, TRUE, NULL, NULL,
							C2_ACCOUNT_POP3, C2_SMTP_REMOTE,
							"pop3.yahoo.com.ar", 110, "pablo_viajando",
							getenv ("POP_PASS"), 0, "smtp.arnet.com.ar", 25,
							FALSE, NULL, NULL);
		
		transfer_list = c2_transfer_list_new (application);
		gtk_widget_show (transfer_list);

		transfer_item = c2_transfer_item_new (account, C2_TRANSFER_ITEM_RECEIVE);
		c2_transfer_list_add_item (C2_TRANSFER_LIST (transfer_list), transfer_item);

		transfer_item = c2_transfer_item_new (account, C2_TRANSFER_ITEM_RECEIVE);
		c2_transfer_list_add_item (C2_TRANSFER_LIST (transfer_list), transfer_item);

		transfer_item = c2_transfer_item_new (account, C2_TRANSFER_ITEM_SEND, NULL, NULL);
		c2_transfer_list_add_item (C2_TRANSFER_LIST (transfer_list), transfer_item);

		account->name = g_strdup ("Sourceforge");

		transfer_item = c2_transfer_item_new (account, C2_TRANSFER_ITEM_RECEIVE);
		c2_transfer_list_add_item (C2_TRANSFER_LIST (transfer_list), transfer_item);

		transfer_item = c2_transfer_item_new (account, C2_TRANSFER_ITEM_RECEIVE);
		c2_transfer_list_add_item (C2_TRANSFER_LIST (transfer_list), transfer_item);

		transfer_item = c2_transfer_item_new (account, C2_TRANSFER_ITEM_SEND, NULL, NULL);
		c2_transfer_list_add_item (C2_TRANSFER_LIST (transfer_list), transfer_item);
	}
	
	gdk_threads_enter ();
	gtk_main ();
	gdk_threads_leave ();
	
	gnome_config_sync ();
	c2_hash_destroy ();

	return 0;
}

static void
on_flag_compose (void)
{
	L
}

static void
on_flag_check (void)
{
L	flags.check = TRUE;
}
