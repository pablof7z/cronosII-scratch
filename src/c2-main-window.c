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
/* XXX XXX review line 602!!! */
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
#define SPOOL_TYPE_STRING			"Spool (local)"

void
c2_main_window_set_sensitivity (void)
{
	GtkWidget *ctree_menu_delete_mailbox = glade_xml_get_widget (WMain.ctree_menu, "delete_mailbox");
	GtkWidget *ctree_menu_properties = glade_xml_get_widget (WMain.ctree_menu, "properties");
	
	if (WMain.selected_mbox)
	{
		/* There's a mailbox opened */
		gtk_widget_set_sensitive (ctree_menu_delete_mailbox, TRUE);
		gtk_widget_set_sensitive (ctree_menu_properties, TRUE);
	
		if (c2_streq (WMain.selected_mbox->name, MAILBOX_INBOX) ||
			c2_streq (WMain.selected_mbox->name, MAILBOX_QUEUE) ||
			c2_streq (WMain.selected_mbox->name, MAILBOX_GARBAGE) ||
			c2_streq (WMain.selected_mbox->name, MAILBOX_DRAFTS))
		{
			gtk_widget_set_sensitive (ctree_menu_delete_mailbox, FALSE);
			gtk_widget_set_sensitive (ctree_menu_properties, FALSE);
		}
	} else
	{
		/* There's no mailbox opened */
		gtk_widget_set_sensitive (ctree_menu_delete_mailbox, FALSE);
		gtk_widget_set_sensitive (ctree_menu_properties, FALSE);
	}
}



/***********************************/
/* Section: Dynamic Menu Functions */
/***********************************/
void
c2_main_window_build_dynamic_menu_accounts (void)
{
	GtkWidget *item;
	GtkWidget *pixmap;
	GtkWidget *box;
	GtkWidget *label;
	GtkWidget *submenu;
	GList *l, *s;
	C2Account *account;
	
	/* First remove everything after the separator */
	submenu = GTK_MENU_ITEM (glade_xml_get_widget (WMain.xml, "file_check_mail"))->submenu;
	l = gtk_container_children (GTK_CONTAINER (submenu));
	
	/* Find the separator... */
	for (s = l; s != NULL; s = s->next)
		if (s->data == glade_xml_get_widget (WMain.xml, "file_check_mail_sep"))
			break;
	
	/* ... and remove */
	for (s = g_list_next (s); s != NULL; s = s->next)
		if (GTK_IS_WIDGET (s->data))
			gtk_widget_destroy (GTK_WIDGET (s->data));
	g_list_free (l);
	
	if (!submenu)
	{
		submenu = gtk_menu_new ();
		gtk_menu_item_set_submenu (GTK_MENU_ITEM (glade_xml_get_widget (WMain.xml, "file_check_mail")),
									submenu);
	}
	
	for (account = c2_app.account; account != NULL; account = account->next)
	{
		item = gtk_pixmap_menu_item_new ();
		
		pixmap = gnome_stock_pixmap_widget_at_size (submenu, GNOME_STOCK_PIXMAP_MAIL, 18, 18);
		box = gtk_hbox_new (FALSE, 0);
		if (strlen (account->name) > 40)
		{
			char *s = g_strndup (account->name, 40);
			char *t = g_strconcat (s, "...", NULL);
			label = gtk_label_new (t);
			g_free (s);
			g_free (t);
		} else
			label = gtk_label_new (account->name);
		
		gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);
		gtk_container_add (GTK_CONTAINER (item), box);
		gtk_pixmap_menu_item_set_pixmap (GTK_PIXMAP_MENU_ITEM (item), pixmap);
		gtk_widget_show (label);
		gtk_widget_show (box);
		if (gnome_preferences_get_menus_have_icons ())
			gtk_widget_show (pixmap);
		gtk_menu_append (GTK_MENU (submenu), item);
		gtk_widget_show (item);
	}
	
	item = gtk_menu_item_new ();
	gtk_menu_append (GTK_MENU (submenu), item);
	gtk_widget_set_sensitive (GTK_WIDGET (item), FALSE);
	gtk_widget_show (item);
}

void
c2_main_window_build_dynamic_menu_windows (void)
{
	GtkWidget *submenu, *item, *box, *pixmap, *label;
	GList *list, *s;
	GtkWindow *window;

	/* First clean whatever was before */
	submenu = GTK_MENU_ITEM (glade_xml_get_widget (WMain.xml, "windows"))->submenu;
	list = gtk_container_children (GTK_CONTAINER (submenu));
	for (s = list; s != NULL; s = s->next)
		if (GTK_IS_WIDGET (s->data))
			gtk_widget_destroy (GTK_WIDGET (s->data));
	g_list_free (list);

	if (!submenu)
	{
		submenu = gtk_menu_new ();
		gtk_menu_item_set_submenu (GTK_MENU_ITEM (glade_xml_get_widget (WMain.xml, "windows")), submenu);
	}

	for (list = c2_app.open_windows; list != NULL; list = list->next)
	{
		if (!GTK_IS_WINDOW (list->data))
			continue;
		window = GTK_WINDOW (list->data);

		item = gtk_pixmap_menu_item_new ();
		pixmap = gnome_stock_pixmap_widget_at_size (submenu, GNOME_STOCK_PIXMAP_MAIL, 18, 18);
		box = gtk_hbox_new (FALSE, 0);
		
		if (strlen (window->title) > 40) {
			char *s = g_strndup (window->title, 40);
			char *t = g_strconcat (s, "...", NULL);
			label = gtk_label_new (t);
			g_free (s);
			g_free (t);
		} else
			label = gtk_label_new (window->title);
		
		gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);
		gtk_container_add (GTK_CONTAINER (item), box);
		gtk_pixmap_menu_item_set_pixmap (GTK_PIXMAP_MENU_ITEM (item), pixmap);
		gtk_widget_show (label);
		gtk_widget_show (box);
		
		if (gnome_preferences_get_menus_have_icons ())
			gtk_widget_show (pixmap);
		gtk_menu_append (GTK_MENU (submenu), item);
		gtk_widget_show (item);
	}
}

/*******************************/
/* Section: New Mailbox Dialog */
/*******************************/

/* Defined in main-window.c */
extern void
on_ctree_changed_mailboxes						(C2Mailbox *mailbox);

static void
on_properties_mailbox_dlg_type_menu_selection_done (GtkWidget *widget, GladeXML *xml)
{
	GtkWidget *type = glade_xml_get_widget (xml, "type");
	GtkWidget *extra_data = glade_xml_get_widget (xml, "extra_data");
	GtkWidget *hostname_label = glade_xml_get_widget (xml, "hostname_label");
	GtkWidget *hostname = glade_xml_get_widget (xml, "extra_data_host");
	GtkWidget *port_label = glade_xml_get_widget (xml, "port_label");
	GtkWidget *port = glade_xml_get_widget (xml, "extra_data_port");
	GtkWidget *username_label = glade_xml_get_widget (xml, "username_label");
	GtkWidget *username = glade_xml_get_widget (xml, "extra_data_user");
	GtkWidget *password_label = glade_xml_get_widget (xml, "password_label");
	GtkWidget *password = glade_xml_get_widget (xml, "extra_data_pass");
	GtkWidget *path_label = glade_xml_get_widget (xml, "path_label");
	GtkWidget *path = glade_xml_get_widget (xml, "extra_data_path");
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
				gtk_widget_show (hostname_label);
				gtk_widget_show (hostname);
				gtk_widget_show (port_label);
				gtk_widget_show (port);
				gtk_widget_show (username_label);
				gtk_widget_show (username);
				gtk_widget_show (password_label);
				gtk_widget_show (password);
				gtk_widget_show (path_label);
				gtk_widget_show (path);
			} else if (c2_streq (selection, SPOOL_TYPE_STRING))
			{
				gtk_widget_show (extra_data);
				gtk_widget_hide (hostname_label);
				gtk_widget_hide (hostname);
				gtk_widget_hide (port_label);
				gtk_widget_hide (port);
				gtk_widget_hide (username_label);
				gtk_widget_hide (username);
				gtk_widget_hide (password_label);
				gtk_widget_hide (password);
				gtk_widget_show (path_label);
				gtk_widget_show (path);
			}
		}
	}
}

static void
on_new_mailbox_dlg_ok_clicked (GladeXML *gxml, gboolean first_mailbox)
{
	GtkWidget *w_type = glade_xml_get_widget (gxml, "type");
	gchar *name = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (gxml, "name")));
	C2MailboxType type;
	gchar *host, *user, *pass, *path;
	gint port;
	gchar *tmp;
	GtkWidget *ctree = glade_xml_get_widget (WMain.xml, "ctree");
	C2Mailbox *parent;
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
			glade_xml_new (C2_APP_GLADE_FILE ("cronosII"), "dlg_mailbox_err");
		GtkWidget *err_dialog = glade_xml_get_widget (err_xml, "dlg_mailbox_err");
		gnome_dialog_run_and_close (GNOME_DIALOG (err_dialog));
		return;
	}

	/* Get the type */
	if (GTK_BIN (w_type)->child)
	{
		GtkWidget *child = GTK_BIN (w_type)->child;

		if (GTK_LABEL (child))
		{
			gtk_label_get (GTK_LABEL (child), &query);

			if (c2_streq (query, CRONOSII_TYPE_STRING))
				type = C2_MAILBOX_CRONOSII;
			else if (c2_streq (query, IMAP_TYPE_STRING))
			{
				type = C2_MAILBOX_IMAP;
				host = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (gxml, "extra_data_host")));
				user = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (gxml, "extra_data_user")));
				pass = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (gxml, "extra_data_pass")));
				path = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (gxml, "extra_data_path")));
				port = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (
											glade_xml_get_widget (gxml, "extra_data_port")));
			} else if (c2_streq (query, SPOOL_TYPE_STRING))
			{
				type = C2_MAILBOX_SPOOL;
				path = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (gxml, "extra_data_path")));
			}
		}
	}
	
	/* Get the parent */
	if (GTK_CLIST (ctree)->selection)
		parent = WMain.selected_mbox;
	else
		parent = NULL;

	switch (type)
	{
		case C2_MAILBOX_CRONOSII:
			retval = c2_mailbox_new_with_parent (name, parent ? parent->id : NULL, type,
							C2_MAILBOX_SORT_DATE, GTK_SORT_ASCENDING);
			break;
		case C2_MAILBOX_IMAP:
			retval = c2_mailbox_new_with_parent (name, parent ? parent->id : NULL, type,
							C2_MAILBOX_SORT_DATE, GTK_SORT_ASCENDING, host, port, user, pass, path);
			break;
		case C2_MAILBOX_SPOOL:
			retval = c2_mailbox_new_with_parent (name, parent ? parent->id : NULL, type,
							C2_MAILBOX_SORT_DATE, GTK_SORT_ASCENDING, path);
			break;
	}

	gtk_signal_connect (GTK_OBJECT (retval), "db_loaded",
						GTK_SIGNAL_FUNC (on_mailbox_db_loaded), NULL);
	
	/* Since this is the first mailbox we will connect to the signal
	 * and emit it again.
	 */
	if (first_mailbox)
	{
		gtk_signal_connect (GTK_OBJECT (retval), "changed_mailboxes",
				GTK_SIGNAL_FUNC (on_ctree_changed_mailboxes), NULL);
		gtk_signal_emit_by_name (GTK_OBJECT (retval), "changed_mailboxes");
	}

	/* Create the structure */
	if (c2_db_create_structure (retval) < 0)
		return;

	/* Now we have to write to the config file the new mailbox */
	config_id = gnome_config_get_int_with_default ("/cronosII/Mailboxes/quantity=0", NULL)+1;
	query = g_strdup_printf ("/cronosII/Mailbox %d/", config_id);
	gnome_config_push_prefix (query);

	gnome_config_set_string ("name", retval->name);
	gnome_config_set_string ("id", retval->id);
	gnome_config_set_int ("type", retval->type);
	gnome_config_set_int ("sort_by", retval->sort_by);
	gnome_config_set_int ("sort_type", retval->sort_type);

	switch (retval->type)
	{
		case C2_MAILBOX_IMAP:
			gnome_config_set_string ("host", retval->protocol.imap.host);
			gnome_config_set_string ("user", retval->protocol.imap.user);
			gnome_config_set_string ("pass", retval->protocol.imap.pass);
			gnome_config_set_string ("path", retval->protocol.imap.path);
			gnome_config_set_int ("port", retval->protocol.imap.port);
			break;
	}
	gnome_config_pop_prefix ();
	g_free (query);

	gnome_config_set_int ("/cronosII/Mailboxes/quantity", config_id);
	gnome_config_sync ();
}

static void
on_new_mailbox_dlg_name_activate (GtkWidget *widget, GladeXML *gxml)
{
	GtkWidget *dialog = glade_xml_get_widget (gxml, "dlg_mailbox_properties");
	
	on_new_mailbox_dlg_ok_clicked (gxml, c2_mailbox_get_head () ? FALSE : TRUE);
	c2_app_unregister_window (GTK_WINDOW (dialog));
	gnome_dialog_close (GNOME_DIALOG (dialog));
}

void
on_new_mailbox_dlg (void)
{
	GladeXML *gxml = glade_xml_new (C2_APP_GLADE_FILE ("cronosII"), "dlg_mailbox_properties");
	GtkWidget *dialog = glade_xml_get_widget (gxml, "dlg_mailbox_properties");
	GtkWidget *entry = glade_xml_get_widget (gxml, "name");
	GtkWidget *type = glade_xml_get_widget (gxml, "type");
	GtkWidget *menu;

	/* Register the window */
	c2_app_register_window (GTK_WINDOW (dialog));

	/* Create the drop down list */
	gtk_signal_connect (GTK_OBJECT (GTK_OPTION_MENU (type)->menu), "selection_done",
						GTK_SIGNAL_FUNC (on_properties_mailbox_dlg_type_menu_selection_done), gxml);
	menu = gtk_menu_item_new_with_label (CRONOSII_TYPE_STRING);
	gtk_widget_show (menu);
	gtk_menu_append (GTK_MENU (GTK_OPTION_MENU (type)->menu), menu);

	menu = gtk_menu_item_new_with_label (IMAP_TYPE_STRING);
	gtk_widget_show (menu);
	gtk_menu_append (GTK_MENU (GTK_OPTION_MENU (type)->menu), menu);
	gtk_option_menu_set_history (GTK_OPTION_MENU (type), 0);

	menu = gtk_menu_item_new_with_label (SPOOL_TYPE_STRING);
	gtk_widget_show (menu);
	gtk_menu_append (GTK_MENU (GTK_OPTION_MENU (type)->menu), menu);
	gtk_option_menu_set_history (GTK_OPTION_MENU (type), 0);

	gnome_dialog_set_default (GNOME_DIALOG (dialog), 0);

	gtk_widget_grab_focus (entry);
	gtk_signal_connect (GTK_OBJECT (entry),	"activate",
						GTK_SIGNAL_FUNC (on_new_mailbox_dlg_name_activate), gxml);
	
	switch (gnome_dialog_run (GNOME_DIALOG (dialog)))
	{
		case 0:
			on_new_mailbox_dlg_ok_clicked (gxml, c2_mailbox_get_head () ? FALSE : TRUE);
		case 1:
			c2_app_unregister_window (GTK_WINDOW (dialog));
			gnome_dialog_close (GNOME_DIALOG (dialog));
	}
}






/**************************************/
/* Section: Mailbox Properties Dialog */
/**************************************/
static void
on_properties_mailbox_dlg_ok_btn_clicked (GtkWidget *widget, GladeXML *xml)
{
	GtkWidget *dialog = glade_xml_get_widget (xml, "dlg_mailbox_properties");
	GtkEntry *w_name = GTK_ENTRY (glade_xml_get_widget (xml, "name"));
	GtkBin *w_type = GTK_BIN (glade_xml_get_widget (xml, "type"));
	GtkEntry *w_host = GTK_ENTRY (glade_xml_get_widget (xml, "extra_data_host"));
	GtkSpinButton *w_port = GTK_SPIN_BUTTON (glade_xml_get_widget (xml, "extra_data_port"));
	GtkEntry *w_user = GTK_ENTRY (glade_xml_get_widget (xml, "extra_data_user"));
	GtkEntry *w_pass = GTK_ENTRY (glade_xml_get_widget (xml, "extra_data_pass"));
	GtkEntry *w_path = GTK_ENTRY (glade_xml_get_widget (xml, "extra_data_path"));
	gchar *selection;
	
	gint configuration_id;
	gchar *name = NULL, *id = NULL, *host = NULL, *user = NULL, *pass = NULL, *path = NULL;
	gint port;
	C2MailboxType type;
	C2Mailbox *mailbox;
	gchar *query;

	if (!(mailbox = C2_MAILBOX (gtk_object_get_data (GTK_OBJECT (dialog), "mailbox"))))
		return;

	/* Get the data */
	name = g_strdup (gtk_entry_get_text (w_name));
	id = g_strdup (mailbox->id);
	
	if (w_type->child)
	{
		GtkLabel *child = GTK_LABEL (w_type->child);

		if (child)
		{
			gtk_label_get (child, &selection);

			if (c2_streq (selection, CRONOSII_TYPE_STRING))
			{
				type = C2_MAILBOX_CRONOSII;
			} else if (c2_streq (selection, IMAP_TYPE_STRING))
			{
				type = C2_MAILBOX_IMAP;
				host = g_strdup (gtk_entry_get_text (w_host));
				user = g_strdup (gtk_entry_get_text (w_user));
				pass = g_strdup (gtk_entry_get_text (w_pass));
				path = g_strdup (gtk_entry_get_text (w_path));
				port = gtk_spin_button_get_value_as_int (w_port);
			} else if (c2_streq (selection, SPOOL_TYPE_STRING))
			{
				type = C2_MAILBOX_SPOOL;
				path = g_strdup (gtk_entry_get_text (w_path));
			}
		}
	}

	if ((configuration_id = c2_app_get_mailbox_configuration_id_by_name (mailbox->name)) < 0)
	{
		gchar error[] = _("Unable to update the mailbox: Unable to find the configuration ID");
		c2_app_report (error, C2_REPORT_WARNING);
		return;
	}
	
	c2_mailbox_update (mailbox, name, id, type, host, port, user, pass, path);

	query = g_strdup_printf ("/cronosII/Mailbox %d/", configuration_id);
	gnome_config_push_prefix (query);

	gnome_config_set_string ("name", mailbox->name);
	gnome_config_set_string ("id", mailbox->id);
	gnome_config_set_int ("type", mailbox->type);
	gnome_config_set_int ("sort_by", mailbox->sort_by);
	gnome_config_set_int ("sort_type", mailbox->sort_type);

	switch (mailbox->type)
	{
		case C2_MAILBOX_IMAP:
			gnome_config_set_string ("host", mailbox->protocol.imap.host);
			gnome_config_set_string ("user", mailbox->protocol.imap.user);
			gnome_config_set_string ("pass", mailbox->protocol.imap.pass);
			gnome_config_set_string ("path", mailbox->protocol.imap.path);
			gnome_config_set_int ("port", mailbox->protocol.imap.port);
			break;
		case C2_MAILBOX_SPOOL:
			gnome_config_set_string ("path", mailbox->protocol.spool.path);
			break;
	}
	gnome_config_pop_prefix ();
	g_free (query);

	gnome_config_sync ();

	g_free (name);
	g_free (id);
	g_free (host);
	g_free (user);
	g_free (pass);
	g_free (path);
}

void
on_properties_mailbox_dlg (void)
{
	GladeXML *xml = glade_xml_new (C2_APP_GLADE_FILE ("cronosII"), "dlg_mailbox_properties");
	GtkWidget *dialog = glade_xml_get_widget (xml, "dlg_mailbox_properties");
	GnomePixmap *pixmap = GNOME_PIXMAP (glade_xml_get_widget (xml, "pixmap"));
	GtkEntry *name = GTK_ENTRY (glade_xml_get_widget (xml, "name"));
	GtkWidget *type = glade_xml_get_widget (xml, "type");
	GtkFrame *warning_frame = GTK_FRAME (glade_xml_get_widget (xml, "warning_frame"));
	GtkFrame *extra_data = GTK_FRAME (glade_xml_get_widget (xml, "extra_data"));
	GtkEntry *host = GTK_ENTRY (glade_xml_get_widget (xml, "extra_data_host"));
	GtkSpinButton *port = GTK_SPIN_BUTTON (glade_xml_get_widget (xml, "extra_data_port"));
	GtkEntry *user = GTK_ENTRY (glade_xml_get_widget (xml, "extra_data_user"));
	GtkEntry *pass = GTK_ENTRY (glade_xml_get_widget (xml, "extra_data_pass"));
	GtkEntry *path = GTK_ENTRY (glade_xml_get_widget (xml, "extra_data_path"));

	C2Mailbox *parent;

	GtkWidget *menu;

	/* Create the pixmap */
	gtk_widget_show (GTK_WIDGET (warning_frame));
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

	menu = gtk_menu_item_new_with_label (SPOOL_TYPE_STRING);
	gtk_widget_show (menu);
	gtk_menu_append (GTK_MENU (GTK_OPTION_MENU (type)->menu), menu);
	
	/* Get the parent */
	if (GTK_CLIST (glade_xml_get_widget (WMain.xml, "ctree"))->selection)
		parent = WMain.selected_mbox;
	else
		parent = NULL;

	gtk_entry_set_text (name, parent->name);
	gtk_option_menu_set_history (GTK_OPTION_MENU (type), parent->type);
	gtk_object_set_data (GTK_OBJECT (dialog), "mailbox", parent);

	switch (parent->type)
	{
		case C2_MAILBOX_IMAP:
			gtk_entry_set_text (host, parent->protocol.imap.host);
			gtk_entry_set_text (user, parent->protocol.imap.user);
			gtk_entry_set_text (pass, parent->protocol.imap.pass);
			gtk_entry_set_text (path, parent->protocol.imap.path);
			gtk_spin_button_set_value (port, parent->protocol.imap.port);
			break;
		case C2_MAILBOX_SPOOL:
			gtk_entry_set_text (path, parent->protocol.spool.path);
			break;
	}

	/* Register the window */
	c2_app_register_window (GTK_WINDOW (dialog));

	on_properties_mailbox_dlg_type_menu_selection_done (dialog, xml);
	switch (gnome_dialog_run (GNOME_DIALOG (dialog)))
	{
		case 0:
			on_properties_mailbox_dlg_ok_btn_clicked (dialog, xml);
		case 1:
			/* Unregister the window */
			c2_app_unregister_window (GTK_WINDOW (dialog));
			gnome_dialog_close (GNOME_DIALOG (dialog));
			C2_DEBUG (parent->protocol.spool.path);
	}
}



/**********************************/
/* Section: Delete Mailbox Dialog */
/**********************************/
void
on_delete_mailbox_dlg (void)
{
	GladeXML *xml = glade_xml_new (C2_APP_GLADE_FILE ("cronosII"), "dlg_mailbox_confirm_deletion");
	GtkWidget *dialog = glade_xml_get_widget (xml, "dlg_mailbox_confirm_deletion");
	C2Mailbox *mailbox;
	gint configuration_id;
	gint max_configuration_id;
	gchar *buffer;

	switch (gnome_dialog_run_and_close (GNOME_DIALOG (dialog)))
	{
		case 0:
			/* Get the mailbox */
			if (GTK_CLIST (glade_xml_get_widget (WMain.xml, "ctree"))->selection)
				mailbox = WMain.selected_mbox;
			else
			{
				g_assert_not_reached ();
				break;
			}
			
			if ((configuration_id = c2_app_get_mailbox_configuration_id_by_name (mailbox->name)) < 0)
			{
				gchar error[] = _("Unable to delete the mailbox: Unable to find the configuration ID");
				c2_app_report (error, C2_REPORT_WARNING);
				return;
			}
			
			buffer = g_strdup_printf ("/cronosII/Mailbox %d", configuration_id);
			gnome_config_clean_section (buffer);
			g_free (buffer);

			max_configuration_id = gnome_config_get_int ("/cronosII/Mailboxes/quantity");
			
			if (max_configuration_id == configuration_id)
				/* If this is the last mailbox in the configuration file
				 * we have to update the /Mailboxes/quantity variable
				 */
				gnome_config_set_int ("/cronosII/Mailboxes/quantity", --max_configuration_id);

			c2_mailbox_remove (mailbox, FALSE); /* <---- This is just for testing!!!!!! XXX XXX XXX XXX XXX */
	}
}
