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
#include <gnome.h>
#include <time.h>
#include <config.h>

#include <glade/glade.h>

#include <libcronosII/db.h>
#include <libcronosII/error.h>
#include <libcronosII/utils.h>

#include "main-window.h"
#include "c2-main-window.h"
#include "c2-app.h"

#define CRONOSII_TYPE_STRING		"Cronos II"
#define IMAP_TYPE_STRING			"IMAP"
#ifdef USE_MYSQL
#	define MYSQL_TYPE_STRING		"MySQL"
#endif

void
c2_main_window_set_sensitivity (void)
{
	if (GTK_CLIST (glade_xml_get_widget (WMain.xml, "ctree"))->selection)
	{
		/* There's a mailbox opened */
		gtk_widget_show (glade_xml_get_widget (WMain.ctree_menu, "delete_mailbox"));
		gtk_widget_show (glade_xml_get_widget (WMain.ctree_menu, "properties"));
		gtk_widget_show (glade_xml_get_widget (WMain.ctree_menu, "separator"));
	} else
	{
		/* There's no mailbox opened */
		gtk_widget_hide (glade_xml_get_widget (WMain.ctree_menu, "separator"));
		gtk_widget_hide (glade_xml_get_widget (WMain.ctree_menu, "delete_mailbox"));
		gtk_widget_hide (glade_xml_get_widget (WMain.ctree_menu, "properties"));
	}
}

/* Section: New Mailbox Dialog */

/* Defined in main-window.c */
extern void
on_ctree_changed_mailboxes						(C2Mailbox *mailbox);

static void
on_new_mailbox_dlg_type_changed (GtkWidget *widget, GladeXML *gxml)
{
	gchar *type = gtk_entry_get_text (GTK_ENTRY (widget));
	GtkWidget *extra_data_frame = glade_xml_get_widget (gxml, "extra_data_frame");
	GtkWidget *extra_data_db_label = glade_xml_get_widget (gxml, "extra_data_db_label");
	GtkWidget *extra_data_db = glade_xml_get_widget (gxml, "extra_data_db");
	
	if (c2_streq (type, CRONOSII_TYPE_STRING))
		gtk_widget_hide (extra_data_frame);
	else if (c2_streq (type, IMAP_TYPE_STRING))
	{
		gtk_widget_show (extra_data_frame);
		gtk_widget_hide (extra_data_db);
		gtk_widget_hide (extra_data_db_label);
#ifdef USE_MYSQL
	} else if (c2_streq (type, MYSQL_TYPE_STRING))
	{
		gtk_widget_show (extra_data_frame);
		gtk_widget_show (extra_data_db);
		gtk_widget_show (extra_data_db_label);
#endif
	}
}

static void
on_new_mailbox_dlg_ok_clicked (GladeXML *gxml, gboolean first_mailbox)
{
	gchar *name = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (gxml, "name")));
	gchar *tmp;
	GtkWidget *ctree = glade_xml_get_widget (WMain.xml, "ctree");
	C2Mailbox *parent;
	C2MailboxType type;
	C2Mailbox *retval;
	gchar *query;
	gint config_id;
	
	c2_return_if_fail (gxml, C2EDATA);

	/* Consistency of the name */
	if (!name || !strlen (name))
		return;
	
	if (c2_mailbox_get_by_name (c2_mailbox_get_head (), name))
	{
		GladeXML *err_xml =
			glade_xml_new (DATADIR "/cronosII/cronosII.glade", "dlg_mailbox_err");
		GtkWidget *err_dialog = glade_xml_get_widget (err_xml, "dlg_mailbox_err");
		gnome_dialog_run_and_close (GNOME_DIALOG (err_dialog));
		return;
	}
	
	/* Get the parent */
	if (GTK_CLIST (ctree)->selection)
		parent = WMain.selected_mbox;
	else
		parent = NULL;

	/* Get the type */
	tmp = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (gxml, "type-entry")));
	if (c2_streq (tmp, CRONOSII_TYPE_STRING))
		type = C2_MAILBOX_CRONOSII;
	else if (c2_streq (tmp, IMAP_TYPE_STRING))
		type = C2_MAILBOX_IMAP;
#ifdef USE_MYSQL
	else if (c2_streq (tmp, MYSQL_TYPE_STRING))
		type = C2_MAILBOX_MYSQL;
#endif
	
	retval = c2_mailbox_new_with_parent (name, parent ? parent->id : NULL, type,
			C2_MAILBOX_SORT_DATE, GTK_SORT_ASCENDING);
	C2_DEBUG (retval->id);
	C2_DEBUG (retval->name);
	
	/* Since this is the first mailbox we will connect to the signal
	 * and emit it again.
	 */
	if (first_mailbox)
	{
		gtk_signal_connect (GTK_OBJECT (retval), "changed_mailboxes",
				GTK_SIGNAL_FUNC (on_ctree_changed_mailboxes), NULL);
		gtk_signal_emit_by_name (GTK_OBJECT (retval), "changed_mailboxes");
	}

	/* Now we have to write to the config file the new mailbox */
	config_id = gnome_config_get_int_with_default ("/cronosII/Mailboxes/quantity=0", NULL)+1;
	query = g_strdup_printf ("/cronosII/Mailbox %d/", config_id);
	gnome_config_push_prefix (query);

	gnome_config_set_string ("name", retval->name);
	gnome_config_set_string ("id", retval->id);
	gnome_config_set_int ("type", retval->type);
	gnome_config_set_int ("sort_by", retval->sort_by);
	gnome_config_set_int ("sort_type", retval->sort_type);

	switch (retval->sort_type)
	{
		case C2_MAILBOX_IMAP:
			gnome_config_set_string ("user", retval->protocol.imap.user);
			gnome_config_set_string ("pass", retval->protocol.imap.pass);
			gnome_config_set_string ("host", retval->protocol.imap.server);
			gnome_config_set_int ("port", retval->protocol.imap.port);
			break;
#ifdef USE_MYSQL
		case C2_MAILBOX_MYSQL:
			gnome_config_set_string ("user", retval->protocol.mysql.user);
			gnome_config_set_string ("pass", retval->protocol.mysql.pass);
			gnome_config_set_string ("host", retval->protocol.mysql.server);
			gnome_config_set_int ("port", retval->protocol.mysql.port);
			gnome_config_set_string ("db", retval->protocol.mysql.db);
			break;
#endif
	}
	gnome_config_pop_prefix ();
	g_free (query);

	gnome_config_set_int ("/cronosII/Mailboxes/quantity", config_id);
	gnome_config_sync ();
}

static void
on_new_mailbox_dlg_name_activate (GtkWidget *widget, GladeXML *gxml)
{
	on_new_mailbox_dlg_ok_clicked (gxml, c2_mailbox_get_head () ? FALSE : TRUE);
	gnome_dialog_close (GNOME_DIALOG (glade_xml_get_widget (gxml, "dlg_new_mailbox")));
}

void
on_new_mailbox_dlg (void)
{
	GladeXML *gxml;
	GtkWidget *dialog;
	GtkWidget *entry;
	GList *types = NULL;

	gxml = glade_xml_new (DATADIR "/cronosII/cronosII.glade", "dlg_new_mailbox");
	
	dialog = glade_xml_get_widget (gxml, "dlg_new_mailbox");
	gnome_dialog_set_default (GNOME_DIALOG (dialog), 0);

	entry = glade_xml_get_widget (gxml, "name");
	gtk_widget_grab_focus (entry);
	gtk_signal_connect (GTK_OBJECT (entry),	"activate",
						GTK_SIGNAL_FUNC (on_new_mailbox_dlg_name_activate), gxml);
	
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (gxml, "type-entry")),
						"changed", GTK_SIGNAL_FUNC (on_new_mailbox_dlg_type_changed), gxml);
	
	types = g_list_append (types, CRONOSII_TYPE_STRING);
	types = g_list_append (types, IMAP_TYPE_STRING);
#ifdef USE_MYSQL
	types = g_list_append (types, MYSQL_TYPE_STRING);
#endif
	gtk_combo_set_popdown_strings (GTK_COMBO (glade_xml_get_widget (gxml, "type")), types);
	g_list_free (types);
	
	switch (gnome_dialog_run (GNOME_DIALOG (dialog)))
	{
		case 0:
			on_new_mailbox_dlg_ok_clicked (gxml, c2_mailbox_get_head () ? FALSE : TRUE);
		case 1:
			gnome_dialog_close (GNOME_DIALOG (dialog));
	}
}

static void
on_properties_mailbox_dlg_type_menu_selection_done (GtkWidget *widget, GladeXML *xml)
{
	GtkWidget *type = glade_xml_get_widget (xml, "type");
	GtkWidget *extra_data = glade_xml_get_widget (xml, "extra_data");
	GtkWidget *db_label = glade_xml_get_widget (xml, "db_label");
	GtkWidget *db = glade_xml_get_widget (xml, "db");
	gchar *selection;

	/* Get the selected type */
	if (GTK_BIN (type)->child)
	{
		GtkWidget *child = GTK_BIN (type)->child;

		if (GTK_LABEL (child))
		{
			gtk_label_get (GTK_LABEL (child), &selection);

			if (c2_streq (selection, CRONOSII_TYPE_STRING))
				gtk_widget_hide (extra_data);
			else if (c2_streq (selection, IMAP_TYPE_STRING))
			{
				gtk_widget_show (extra_data);
				gtk_widget_hide (db);
				gtk_widget_hide (db_label);
#ifdef USE_MYSQL
			} else if (c2_streq (selection, MYSQL_TYPE_STRING))
			{
				gtk_widget_show (extra_data);
				gtk_widget_show (db);
				gtk_widget_show (db_label);
#endif
			}
		}
	}
}

/* Section: Mailbox Properties Dialog */
static void
on_properties_mailbox_dlg_ok_btn_clicked (GtkWidget *widget, GladeXML *xml)
{
	L
}

void
on_properties_mailbox_dlg (void)
{
	GladeXML *xml = glade_xml_new (DATADIR "/cronosII/cronosII.glade", "dlg_mailbox_properties");
	GtkWidget *dialog = glade_xml_get_widget (xml, "dlg_mailbox_properties");
	GnomePixmap *pixmap = GNOME_PIXMAP (glade_xml_get_widget (xml, "pixmap"));
	GtkEntry *name = GTK_ENTRY (glade_xml_get_widget (xml, "name"));
	GtkWidget *type = glade_xml_get_widget (xml, "type");
	GtkFrame *extra_data = GTK_FRAME (glade_xml_get_widget (xml, "extra_data"));
	GtkEntry *host = GTK_ENTRY (glade_xml_get_widget (xml, "host"));
	GtkWidget *port = glade_xml_get_widget (xml, "port");
	GtkEntry *user = GTK_ENTRY (glade_xml_get_widget (xml, "user"));
	GtkEntry *pass = GTK_ENTRY (glade_xml_get_widget (xml, "pass"));
	GtkWidget *db_label = glade_xml_get_widget (xml, "db_label");
	GtkEntry *db = GTK_ENTRY (glade_xml_get_widget (xml, "db"));

	C2Mailbox *parent;

	GtkWidget *menu;

	/* Create the pixmap */
	gnome_pixmap_load_file (pixmap, DATADIR "/pixmaps/gnome-warning.png");
	
	/* Create the drop down list */
	gtk_signal_connect (GTK_OBJECT (GTK_OPTION_MENU (type)->menu), "selection_done",
						GTK_SIGNAL_FUNC (on_properties_mailbox_dlg_type_menu_selection_done), xml);
	menu = gtk_menu_item_new_with_label (CRONOSII_TYPE_STRING);
	gtk_widget_show (menu);
	gtk_menu_append (GTK_MENU (GTK_OPTION_MENU (type)->menu), menu);

	menu = gtk_menu_item_new_with_label (IMAP_TYPE_STRING);
	gtk_widget_show (menu);
	gtk_menu_append (GTK_MENU (GTK_OPTION_MENU (type)->menu), menu);
	
#ifdef USE_MYSQL
	menu = gtk_menu_item_new_with_label (MYSQL_TYPE_STRING);
	gtk_widget_show (menu);
	gtk_menu_append (GTK_MENU (GTK_OPTION_MENU (type)->menu), menu);
#endif

	/* Get the parent */
	if (GTK_CLIST (glade_xml_get_widget (WMain.xml, "ctree"))->selection)
		parent = WMain.selected_mbox;
	else
		parent = NULL;

	gtk_entry_set_text (name, parent->name);
	gtk_option_menu_set_history (GTK_OPTION_MENU (type), parent->type);
	

	switch (gnome_dialog_run (GNOME_DIALOG (dialog)))
	{
		case 0:
			on_properties_mailbox_dlg_ok_btn_clicked (dialog, xml);
		case 1:
			gnome_dialog_close (GNOME_DIALOG (dialog));
	}
}
