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

#include <glade/glade.h>

#include <libcronosII/account.h>

#include "c2-app.h"

static void
on_accounts_clist_select_row					(GtkCList *clist);

static void
on_accounts_clist_unselect_row					(GtkCList *clist);

static void
on_accounts_new_btn_clicked						(void);

static void
on_accounts_edit_btn_clicked					(void);

static void
on_accounts_delete_btn_clicked					(void);

static void
on_accounts_up_btn_clicked						(void);

static void
on_accounts_down_btn_clicked					(void);

static void
on_interface_help_btn_clicked					(void);

static void
on_paths_btn_clicked							(GtkWidget *widget, GtkWidget *entry);

static void
on_advanced_http_proxy_btn_toggled				(void);

static void
on_advanced_ftp_proxy_btn_toggled				(void);

static void
on_advanced_persistent_smtp_btn_toggled			(void);

static GladeXML *preferences_xml = NULL;
void
c2_preferences_new (void)
{
	GtkWidget *dialog;
	GtkWidget *tree;

	GtkWidget *options_check_timeout;
	GtkWidget *options_mark_timeout;
	GtkWidget *options_prepend_character;
	GtkWidget *options_empty_garbage;
	GtkWidget *options_use_outbox;
	GtkWidget *options_check_at_start;
	GtkWidget *options_default_mime;

	C2Account *account;
	GtkCList *accounts_clist;
	GtkWidget *accounts_new_btn;
	GtkWidget *accounts_new_pixmap;
	GtkWidget *accounts_edit_btn;
	GtkWidget *accounts_edit_pixmap;
	GtkWidget *accounts_delete_btn;
	GtkWidget *accounts_delete_pixmap;
	GtkWidget *accounts_up_btn;
	GtkWidget *accounts_up_pixmap;
	GtkWidget *accounts_down_btn;
	GtkWidget *accounts_down_pixmap;
	
	GtkWidget *interface_title;
	GtkWidget *interface_toolbar;
	GtkWidget *interface_date_fmt;
	GtkWidget *interface_help_btn;

	GtkWidget *fonts_message_body;
	GtkWidget *fonts_unreaded_message;
	GtkWidget *fonts_readed_message;
	GtkWidget *fonts_source;
	
	GtkWidget *colors_replying_original_message;
	GtkWidget *colors_message_bg;
	GtkWidget *colors_message_fg;
	GtkWidget *colors_message_source;
	
	GtkWidget *paths_saving_entry;
	GtkWidget *paths_saving_btn;
	GtkWidget *paths_download_entry;
	GtkWidget *paths_download_btn;
	GtkWidget *paths_get_entry;
	GtkWidget *paths_get_btn;
	
	GtkWidget *advanced_http_proxy_btn;
	GtkWidget *advanced_http_proxy_addr;
	GtkWidget *advanced_http_proxy_port;
	GtkWidget *advanced_ftp_proxy_btn;
	GtkWidget *advanced_ftp_proxy_addr;
	GtkWidget *advanced_ftp_proxy_port;
	GtkWidget *advanced_persistent_smtp_btn;
	GtkWidget *advanced_persistent_smtp_addr;
	GtkWidget *advanced_persistent_smtp_port;
	GtkWidget *advanced_use_internal_browser;
	
	if (preferences_xml)
	{
		dialog = glade_xml_get_widget (preferences_xml, "dlg_preferences");
		gtk_widget_show (dialog);
		return;
	}
	
	preferences_xml = glade_xml_new (DATADIR "/cronosII/preferences.glade", "dlg_preferences");
	
	/* Dialog */
	dialog = glade_xml_get_widget (preferences_xml, "dlg_preferences");

	/* Tree */

	/* options_check_timeout */
	options_check_timeout = glade_xml_get_widget (preferences_xml, "options_check_timeout");
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (options_check_timeout),
								(gfloat) c2_app.options_check_timeout);

	/* options_mark_timeout */
	options_mark_timeout = glade_xml_get_widget (preferences_xml, "options_mark_timeout");
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (options_mark_timeout),
								(gfloat) c2_app.options_mark_timeout);

	/* options_prepend_character */
	options_prepend_character = glade_xml_get_widget (preferences_xml, "options_prepend_character");
	gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (options_prepend_character)->entry),
						c2_app.options_prepend_character);

	/* options_empty_garbage */
	options_empty_garbage = glade_xml_get_widget (preferences_xml, "options_empty_garbage");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (options_empty_garbage),
									c2_app.options_empty_garbage);

	/* options_use_outbox */
	options_use_outbox = glade_xml_get_widget (preferences_xml, "options_use_outbox");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (options_use_outbox),
									c2_app.options_use_outbox);

	/* options_check_at_start */
	options_check_at_start = glade_xml_get_widget (preferences_xml, "options_check_at_start");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (options_check_at_start),
									c2_app.options_check_at_start);

	/* options_default_mime */
	options_default_mime = glade_xml_get_widget (preferences_xml, "options_default_mime");
	gtk_option_menu_set_history (GTK_OPTION_MENU (options_default_mime), c2_app.options_default_mime);

	/* accounts_clist */
	accounts_clist = GTK_CLIST (glade_xml_get_widget (preferences_xml, "accounts_clist"));
	for (account = c2_app.account; account != NULL; account = account->next)
	{
		gchar *row[] =
		{
			account->name,
			(account->type == C2_ACCOUNT_POP) ? "POP" : "Spool",
			account->per_name,
			account->email,
			NULL
		};
		gtk_clist_append (accounts_clist, row);
		gtk_clist_set_row_data (accounts_clist, accounts_clist->rows-1, account);
	}

	/* accounts_new_btn */
	accounts_new_btn = glade_xml_get_widget (preferences_xml, "accounts_new_btn");
	gtk_signal_connect (GTK_OBJECT (accounts_new_btn), "clicked",
						GTK_SIGNAL_FUNC (on_accounts_new_btn_clicked), NULL);

	/* accounts_new_pixmap */
	accounts_new_pixmap = glade_xml_get_widget (preferences_xml, "accounts_new_pixmap");
	gnome_pixmap_load_file (GNOME_PIXMAP (accounts_new_pixmap), "GNOME_STOCK_PIXMAP_NEW");

	/* accounts_edit_btn */
	accounts_edit_btn = glade_xml_get_widget (preferences_xml, "accounts_edit_btn");
	gtk_signal_connect (GTK_OBJECT (accounts_edit_btn), "clicked",
						GTK_SIGNAL_FUNC (on_accounts_edit_btn_clicked), NULL);

	/* accounts_edit_pixmap */
	accounts_edit_pixmap = glade_xml_get_widget (preferences_xml, "accounts_edit_pixmap");
	gnome_pixmap_load_file (GNOME_PIXMAP (accounts_edit_pixmap), "GNOME_STOCK_PIXMAP_INDEX");

	/* accounts_delete_btn */
	accounts_delete_btn = glade_xml_get_widget (preferences_xml, "accounts_delete_btn");
	gtk_signal_connect (GTK_OBJECT (accounts_delete_btn), "clicked",
						GTK_SIGNAL_FUNC (on_accounts_delete_btn_clicked), NULL);

	/* accounts_delete_pixmap */
	accounts_delete_pixmap = glade_xml_get_widget (preferences_xml, "accounts_delete_pixmap");
	gnome_pixmap_load_file (GNOME_PIXMAP (accounts_delete_pixmap), "GNOME_STOCK_PIXMAP_TRASH");

	/* accounts_up_btn */
	accounts_up_btn = glade_xml_get_widget (preferences_xml, "accounts_up_btn");
	gtk_signal_connect (GTK_OBJECT (accounts_up_btn), "clicked",
						GTK_SIGNAL_FUNC (on_accounts_up_btn_clicked), NULL);

	/* accounts_up_pixmap */
	accounts_up_pixmap = glade_xml_get_widget (preferences_xml, "accounts_up_pixmap");
	gnome_pixmap_load_file (GNOME_PIXMAP (accounts_up_pixmap), "GNOME_STOCK_PIXMAP_UP");

	/* accounts_down_btn */
	accounts_down_btn = glade_xml_get_widget (preferences_xml, "accounts_down_btn");
	gtk_signal_connect (GTK_OBJECT (accounts_down_btn), "clicked",
						GTK_SIGNAL_FUNC (on_accounts_down_btn_clicked), NULL);

	/* accounts_down_pixmap */
	accounts_down_pixmap = glade_xml_get_widget (preferences_xml, "accounts_down_pixmap");
	gnome_pixmap_load_file (GNOME_PIXMAP (accounts_down_pixmap), "GNOME_STOCK_PIXMAP_DOWN");

	/* interface_title */
	interface_title = glade_xml_get_widget (preferences_xml, "interface_title");
	gtk_entry_set_text (GTK_ENTRY (interface_title), c2_app.interface_title);

	/* interface_toolbar */
	interface_toolbar = glade_xml_get_widget (preferences_xml, "interface_toolbar");
	gtk_option_menu_set_history (GTK_OPTION_MENU (interface_toolbar), c2_app.interface_toolbar);

	/* interface_date_fmt */
	interface_date_fmt = glade_xml_get_widget (preferences_xml, "interface_date_fmt");
	gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (interface_date_fmt)->entry), c2_app.interface_date_fmt);

	/* interface_help_btn */
	interface_help_btn = glade_xml_get_widget (preferences_xml, "interface_help_btn");
	gtk_signal_connect (GTK_OBJECT (interface_help_btn), "clicked",
						GTK_SIGNAL_FUNC (on_interface_help_btn_clicked), NULL);

	/* fonts_message_body */
	fonts_message_body = glade_xml_get_widget (preferences_xml, "fonts_message_body");
	gnome_font_picker_set_font_name (GNOME_FONT_PICKER (fonts_message_body), c2_app.fonts_message_body);

	/* fonts_unreaded_message */
	fonts_unreaded_message = glade_xml_get_widget (preferences_xml, "fonts_unreaded_message");
	gnome_font_picker_set_font_name (GNOME_FONT_PICKER (fonts_unreaded_message),
										c2_app.fonts_unreaded_message);

	/* fonts_readed_message */
	fonts_readed_message = glade_xml_get_widget (preferences_xml, "fonts_readed_message");
	gnome_font_picker_set_font_name (GNOME_FONT_PICKER (fonts_readed_message), c2_app.fonts_readed_message);
	
	/* fonts_source */
	fonts_source = glade_xml_get_widget (preferences_xml, "fonts_source");
	gtk_option_menu_set_history (GTK_OPTION_MENU (fonts_source), c2_app.fonts_source);

	/* colors_replying_original_message */
	colors_replying_original_message = glade_xml_get_widget (preferences_xml,
															"colors_replying_original_message");
	gnome_color_picker_set_i16 (GNOME_COLOR_PICKER (colors_replying_original_message),
								c2_app.colors_replying_original_message.red,
								c2_app.colors_replying_original_message.green,
								c2_app.colors_replying_original_message.blue, 0);

	/* colors_message_bg */
	colors_message_bg = glade_xml_get_widget (preferences_xml, "colors_message_bg");
	gnome_color_picker_set_i16 (GNOME_COLOR_PICKER (colors_message_bg),
								c2_app.colors_message_bg.red,
								c2_app.colors_message_bg.green,
								c2_app.colors_message_bg.blue, 0);

	/* colors_message_fg */
	colors_message_fg = glade_xml_get_widget (preferences_xml, "colors_message_fg");
	gnome_color_picker_set_i16 (GNOME_COLOR_PICKER (colors_message_fg),
								c2_app.colors_message_fg.red,
								c2_app.colors_message_fg.green,
								c2_app.colors_message_fg.blue, 0);

	/* colors_message_source */
	colors_message_source = glade_xml_get_widget (preferences_xml, "colors_message_source");
	gtk_option_menu_set_history (GTK_OPTION_MENU (colors_message_source), c2_app.colors_message_source);

	/* paths_saving_entry */
	paths_saving_entry = glade_xml_get_widget (preferences_xml, "paths_saving_entry");
	gtk_entry_set_text (GTK_ENTRY (paths_saving_entry), c2_app.paths_saving_entry);

	/* paths_saving_btn */
	paths_saving_btn = glade_xml_get_widget (preferences_xml, "paths_saving_btn");
	gtk_signal_connect (GTK_OBJECT (paths_saving_btn), "clicked",
						GTK_SIGNAL_FUNC (on_paths_btn_clicked), paths_saving_entry);

	/* paths_download_entry */
	paths_download_entry = glade_xml_get_widget (preferences_xml, "paths_download_entry");
	gtk_entry_set_text (GTK_ENTRY (paths_download_entry), c2_app.paths_download_entry);

	/* paths_download_btn */
	paths_download_btn = glade_xml_get_widget (preferences_xml, "paths_download_btn");
	gtk_signal_connect (GTK_OBJECT (paths_download_btn), "clicked",
						GTK_SIGNAL_FUNC (on_paths_btn_clicked), paths_download_entry);

	/* paths_get_entry */
	paths_get_entry = glade_xml_get_widget (preferences_xml, "paths_get_entry");
	gtk_entry_set_text (GTK_ENTRY (paths_get_entry), c2_app.paths_get_entry);

	/* paths_get_btn */
	paths_get_btn = glade_xml_get_widget (preferences_xml, "paths_get_btn");
	gtk_signal_connect (GTK_OBJECT (paths_get_btn), "clicked",
						GTK_SIGNAL_FUNC (on_paths_btn_clicked), paths_get_entry);
	
	/* advanced_http_proxy_addr */
	advanced_http_proxy_addr = glade_xml_get_widget (preferences_xml, "advanced_http_proxy_addr");
	gtk_entry_set_text (GTK_ENTRY (advanced_http_proxy_addr), c2_app.advanced_http_proxy_addr);

	/* advanced_http_proxy_port */
	advanced_http_proxy_port = glade_xml_get_widget (preferences_xml, "advanced_http_proxy_port");
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (advanced_http_proxy_port),
								(gfloat) c2_app.advanced_http_proxy_port);
	
	/* advanced_http_proxy_btn */
	advanced_http_proxy_btn = glade_xml_get_widget (preferences_xml, "advanced_http_proxy_btn");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (advanced_http_proxy_btn),
									c2_app.advanced_http_proxy);
	if (!c2_app.advanced_http_proxy)
	{
		gtk_widget_set_sensitive (advanced_http_proxy_addr, FALSE);
		gtk_widget_set_sensitive (advanced_http_proxy_port, FALSE);
	}
	gtk_signal_connect (GTK_OBJECT (advanced_http_proxy_btn), "toggled",
						GTK_SIGNAL_FUNC (on_advanced_http_proxy_btn_toggled), NULL);

	/* advanced_ftp_proxy_addr */
	advanced_ftp_proxy_addr = glade_xml_get_widget (preferences_xml, "advanced_ftp_proxy_addr");
	gtk_entry_set_text (GTK_ENTRY (advanced_ftp_proxy_addr), c2_app.advanced_ftp_proxy_addr);

	/* advanced_ftp_proxy_port */
	advanced_ftp_proxy_port = glade_xml_get_widget (preferences_xml, "advanced_ftp_proxy_port");
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (advanced_ftp_proxy_port),
								(gfloat) c2_app.advanced_ftp_proxy_port);
	
	/* advanced_ftp_proxy_btn */
	advanced_ftp_proxy_btn = glade_xml_get_widget (preferences_xml, "advanced_ftp_proxy_btn");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (advanced_ftp_proxy_btn),
									c2_app.advanced_ftp_proxy);
	if (!c2_app.advanced_ftp_proxy)
	{
		gtk_widget_set_sensitive (advanced_ftp_proxy_addr, FALSE);
		gtk_widget_set_sensitive (advanced_ftp_proxy_port, FALSE);
	}
	gtk_signal_connect (GTK_OBJECT (advanced_ftp_proxy_btn), "toggled",
						GTK_SIGNAL_FUNC (on_advanced_ftp_proxy_btn_toggled), NULL);
	
	/* advanced_persistent_smtp_addr */
	advanced_persistent_smtp_addr = glade_xml_get_widget (preferences_xml, "advanced_persistent_smtp_addr");
	gtk_entry_set_text (GTK_ENTRY (advanced_persistent_smtp_addr), c2_app.advanced_persistent_smtp_addr);

	/* advanced_persistent_smtp_port */
	advanced_persistent_smtp_port = glade_xml_get_widget (preferences_xml, "advanced_persistent_smtp_port");
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (advanced_persistent_smtp_port),
								(gfloat) c2_app.advanced_persistent_smtp_port);
	
	/* advanced_persistent_smtp_btn */
	advanced_persistent_smtp_btn = glade_xml_get_widget (preferences_xml, "advanced_persistent_smtp_btn");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (advanced_persistent_smtp_btn),
									c2_app.advanced_persistent_smtp);
	if (!c2_app.advanced_persistent_smtp)
	{
		gtk_widget_set_sensitive (advanced_persistent_smtp_addr, FALSE);
		gtk_widget_set_sensitive (advanced_persistent_smtp_port, FALSE);
	}
	gtk_signal_connect (GTK_OBJECT (advanced_persistent_smtp_btn), "toggled",
						GTK_SIGNAL_FUNC (on_advanced_persistent_smtp_btn_toggled), NULL);

	/* advanced_use_internal_browser */
	advanced_use_internal_browser = glade_xml_get_widget (preferences_xml, "advanced_use_internal_browser");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (advanced_use_internal_browser),
									c2_app.advanced_use_internal_browser);
	
	gtk_widget_show (dialog);
}

static void
on_accounts_clist_select_row (GtkCList *clist)
{
}

static void
on_accounts_clist_unselect_row (GtkCList *clist)
{
}

static void
on_accounts_new_btn_clicked (void)
{
}

static void
on_accounts_edit_btn_clicked (void)
{
}

static void
on_accounts_delete_btn_clicked (void)
{
}

static void
on_accounts_up_btn_clicked (void)
{
}

static void
on_accounts_down_btn_clicked (void)
{
}

static void
on_interface_help_btn_clicked (void)
{
}

static void
on_paths_btn_clicked (GtkWidget *widget, GtkWidget *entry)
{
}

static void
on_advanced_http_proxy_btn_toggled (void)
{
}

static void
on_advanced_ftp_proxy_btn_toggled (void)
{
}

static void
on_advanced_persistent_smtp_btn_toggled (void)
{
}
