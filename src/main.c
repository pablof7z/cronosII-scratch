/*  Cronos II Mail Client
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
#include "main-window.h"

static gint
c2_config_init											(void);

static void
c2_init (gint argc, gchar **argv)
{
	static struct poptOption options[] = {
		{"checkmail", 'c', POPT_ARG_NONE,
			NULL, 0,
			N_("Get new mail on startup"), NULL},
		{"compose", 'm', POPT_ARG_STRING,
			NULL, 0,
			N_("Compose a new email to EMAIL@ADDRESS"), "EMAIL@ADDRESS"}
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
	gchar *tmp;
	
	c2_app.tooltips = gtk_tooltips_new ();

	/* Check if the configuration exists */
	tmp = gnome_config_get_string ("/cronosII/CronosII/Version");
	if (!tmp)
	{
		gdk_threads_enter ();
		c2_install_new ();
		gdk_threads_leave ();
	}
	
	/* Get mailboxes */
	load_mailboxes ();

	c2_app.wm_hpan = gnome_config_get_int_with_default ("/cronosII/Appareance/hpan=150", NULL);
	c2_app.wm_vpan = gnome_config_get_int_with_default ("/cronosII/Appareance/vpan=170", NULL);
	c2_app.wm_width = gnome_config_get_int_with_default ("/cronosII/Appareance/width=600", NULL);
	c2_app.wm_height = gnome_config_get_int_with_default ("/cronosII/Appareance/height=440", NULL);
	c2_app.wm_clist[0] = gnome_config_get_int_with_default ("/cronosII/Appareance/wm_clist::0=40", NULL);
	c2_app.wm_clist[1] = gnome_config_get_int_with_default ("/cronosII/Appareance/wm_clist::1=200", NULL);
	c2_app.wm_clist[2] = gnome_config_get_int_with_default ("/cronosII/Appareance/wm_clist::2=180", NULL);
	c2_app.wm_clist[3] = gnome_config_get_int_with_default ("/cronosII/Appareance/wm_clist::3=70", NULL);
	c2_app.wm_clist[4] = gnome_config_get_int_with_default ("/cronosII/Appareance/wm_clist::4=70", NULL);
	c2_app.wm_clist[5] = gnome_config_get_int_with_default ("/cronosII/Appareance/wm_clist::5=70", NULL);
	c2_app.wm_clist[6] = gnome_config_get_int_with_default ("/cronosII/Appareance/wm_clist::6=70", NULL);
	c2_app.wm_clist[7] = gnome_config_get_int_with_default ("/cronosII/Appareance/wm_clist::7=70", NULL);

	c2_app.font_read = gnome_config_get_string_with_default ("/cronosII/Fonts/font_read=-b&h-lucida-medium-r-normal-*-*-100-*-*-p-*-iso8859-1", NULL);
	c2_app.font_unread = gnome_config_get_string_with_default ("/cronosII/Fonts/font_unread=-b&h-lucida-bold-r-normal-*-*-100-*-*-p-*-iso8859-1", NULL);
	c2_app.font_body = gnome_config_get_string_with_default ("/cronosII/Fonts/font_body=-adobe-courier-medium-r-normal-*-*-120-*-*-m-*-iso8859-1", NULL);
	c2_app.date_fmt = gnome_config_get_string_with_default ("/cronosII/Interface/Date fmt=" DATE_FORMAT, NULL);

	return 0;
}

static void
load_mailboxes (void)
{
	int i;

	for (c2_app.mailboxes = NULL, i = 0;; i++)
	{
		gchar *name;
		gchar *id;
		C2MailboxType type;
		C2MailboxSortBy sort_by;
		GtkSortType sort_type;
		gchar *host, *user, *pass;
		gint port;
		gchar *db;
		
		gchar *query = g_strdup_printf ("/cronosII/Mailboxes/%d", i);
		
		gnome_config_push_prefix (query);
		if (!(name = gnome_config_get_string ("::Name")))
		{
			gnome_config_pop_prefix ();
			c2_app.mailboxes = c2_mailbox_get_head ();
			g_free (query);
			break;
		}

		id = gnome_config_get_string ("::Id");
		type = gnome_config_get_int ("::Type");
		sort_by = gnome_config_get_int ("::Sort By");
		sort_type = gnome_config_get_int ("::Sort Type");

		switch (type)
		{
			case C2_MAILBOX_CRONOSII:
				c2_mailbox_new (name, id, type, sort_by, sort_type);
				break;
			case C2_MAILBOX_IMAP:
				host = gnome_config_get_string ("::Host");
				port = gnome_config_get_int ("::Port");
				user = gnome_config_get_string ("::User");
				pass = gnome_config_get_string ("::Pass");	
				c2_mailbox_new (name, id, type, sort_by, sort_type, host, port, user, pass);
				g_free (host);
				g_free (user);
				g_free (pass);
				break;
#ifdef USE_MYSQL
			case C2_MAILBOX_MYSQL:
				host = gnome_config_get_string ("::Host");
				port = gnome_config_get_int ("::Port");
				db = gnome_config_get_string ("::Db");
				user = gnome_config_get_string ("::User");
				pass = gnome_config_get_string ("::Pass");	
				c2_mailbox_new (name, id, type, sort_by, sort_type, host, port, db, user, pass);
				g_free (host);
				g_free (db);
				g_free (user);
				g_free (pass);
				break;
#endif
		}
		g_free (name);
		g_free (id);
		gnome_config_pop_prefix ();
		g_free (query);
	}
}
