/*  Cronos II
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

#include <libmodules/error.h>

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
	gnome_init_with_popt_table (PACKAGE, VERSION, argc, argv, options, 0, NULL);
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

	/* Initialization of GNOME */
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

	return 0;
}

static void
load_mailboxes (void)
{
	int i;

	for (c2_app.mailboxes = NULL, i = 0;; i++)
	{
		C2Mailbox *mbox = g_new0 (C2Mailbox, 1);
		gchar *query = g_strdup_printf ("/cronosII/Mailboxes/%d", i);
		gchar *tmp = gnome_config_get_string (query);
		if (!tmp)
		{
			g_free (query);
			g_free (mbox);
			break;
		}

		gnome_config_push_prefix (query);
		mbox->name = gnome_config_get_string ("::Name");
		mbox->id = gnome_config_get_int ("::Id");
		mbox->parent_id = gnome_config_get_int ("::Parent Id");
		gnome_config_pop_prefix ();
		g_free (query);

		c2_app.mailboxes = c2_mailbox_append (c2_app.mailboxes, mbox);
	}
}
