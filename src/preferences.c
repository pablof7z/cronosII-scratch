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
#include <libcronosII/smtp.h>

#include "c2-app.h"
#include "main-window.h"

#define OPTIONS_DEFAULT_MIME_STRING_PLAIN_TEXT _("Plain text")
#define OPTIONS_DEFAULT_MIME_STRING_HTML_TEXT _("HTML formatted")
#define INTERFACE_TOOLBAR_STRING_ICONS_ONLY _("Icons only")
#define INTERFACE_TOOLBAR_STRING_TEXT_ONLY _("Text only")
#define INTERFACE_TOOLBAR_STRING_BOTH _("Icons & Text")
#define FONTS_SOURCE_STRING_USE_DOCUMENT _("Use document specified fonts")
#define FONTS_SOURCE_STRING_USE_MY _("Always use my fonts")
#define ACCOUNTS_NEW_PROTOCOL_TYPE_STRING_POP3 _("Post Office Protocol (POP3)")
#define ACCOUNTS_NEW_SIGNATURE_TYPE_STRING_STATIC _("Static Signature")
#define ACCOUNTS_NEW_SIGNATURE_TYPE_STRING_DYNAMIC _("Dynamic Signature (output of commands)")

static void
on_ok_btn_clicked								(void);

static void
on_apply_btn_clicked							(void);

static void
on_cancel_btn_clicked							(void);

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

static void
on_ctree_tree_select_row						(GtkCTree *ctree, GtkCTreeNode *node);

enum
{
	PAGE_GENERAL,
	PAGE_OPTIONS,
	PAGE_PATHS,
	PAGE_ACCOUNTS,
	PAGE_INTERFACE,
	PAGE_FONTS,
	PAGE_COLORS,
	PAGE_ADVANCED
};

static GladeXML *preferences_xml = NULL;

void
c2_preferences_new (void)
{
	GtkWidget *dialog;
	GtkWidget *pixmap;
	
	GtkCTree *ctree;
	GtkCTreeNode *ctree_general;
	GtkCTreeNode *ctree_options;
	GtkCTreeNode *ctree_paths;
	GtkCTreeNode *ctree_accounts;
	GtkCTreeNode *ctree_interface;
	GtkCTreeNode *ctree_fonts;
	GtkCTreeNode *ctree_colors;
	GtkCTreeNode *ctree_advanced;
	gchar *ctree_string[] = { NULL, NULL };

	GtkWidget *options_check_timeout;
	GtkWidget *options_mark_timeout;
	GtkWidget *options_prepend_character;
	GtkWidget *options_empty_garbage;
	GtkWidget *options_use_outbox;
	GtkWidget *options_check_at_start;
	GtkWidget *options_default_mime;
	GtkWidget *menu;

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
	GtkWidget *paths_always_use;
	
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
		gtk_clist_clear (GTK_CLIST (glade_xml_get_widget (preferences_xml, "accounts_clist")));
		gtk_widget_show (dialog);
		return;
	}
	
	preferences_xml = glade_xml_new (C2_APP_GLADE_FILE ("preferences"), "dlg_preferences");
	
	/* Dialog */
	dialog = glade_xml_get_widget (preferences_xml, "dlg_preferences");
	gnome_dialog_button_connect (GNOME_DIALOG (dialog), 0,
								GTK_SIGNAL_FUNC (on_ok_btn_clicked), NULL);
	gnome_dialog_button_connect (GNOME_DIALOG (dialog), 1,
								GTK_SIGNAL_FUNC (on_apply_btn_clicked), NULL);
	gnome_dialog_button_connect (GNOME_DIALOG (dialog), 2,
								GTK_SIGNAL_FUNC (on_cancel_btn_clicked), NULL);

	/* Tree */
	ctree = GTK_CTREE (glade_xml_get_widget (preferences_xml, "tree"));
	gtk_signal_connect (GTK_OBJECT (GTK_WIDGET (ctree)), "tree_select_row",
						GTK_SIGNAL_FUNC (on_ctree_tree_select_row), NULL);
	ctree_string[0] = _("General");
	ctree_general = gtk_ctree_insert_node (ctree, NULL, NULL, ctree_string, 0,
											NULL, NULL, NULL, NULL, FALSE, TRUE);
	gtk_ctree_node_set_row_data (ctree, ctree_general, "General");
	ctree_string[0] = _("Options");
	ctree_options = gtk_ctree_insert_node (ctree, ctree_general, NULL, ctree_string, 0,
											NULL, NULL, NULL, NULL, FALSE, TRUE);
	gtk_ctree_node_set_row_data (ctree, ctree_options, "Options");
	ctree_string[0] = _("Paths");
	ctree_paths = gtk_ctree_insert_node (ctree, ctree_general, NULL, ctree_string, 0,
											NULL, NULL, NULL, NULL, FALSE, TRUE);
	gtk_ctree_node_set_row_data (ctree, ctree_paths, "Paths");
	ctree_string[0] = _("Accounts");
	ctree_accounts = gtk_ctree_insert_node (ctree, NULL, NULL, ctree_string, 0, 
											NULL, NULL, NULL, NULL, FALSE, TRUE);
	gtk_ctree_node_set_row_data (ctree, ctree_accounts, "Accounts");
	ctree_string[0] = _("Interface");
	ctree_interface = gtk_ctree_insert_node (ctree, NULL, NULL, ctree_string, 0, 
											NULL, NULL, NULL, NULL, FALSE, TRUE);
	gtk_ctree_node_set_row_data (ctree, ctree_interface, "Interface");
	ctree_string[0] = _("Fonts");
	ctree_fonts = gtk_ctree_insert_node (ctree, ctree_interface, NULL, ctree_string, 0, 
											NULL, NULL, NULL, NULL, FALSE, TRUE);
	gtk_ctree_node_set_row_data (ctree, ctree_fonts, "Fonts");
	ctree_string[0] = _("Colors");
	ctree_colors = gtk_ctree_insert_node (ctree, ctree_interface, NULL, ctree_string, 0, 
											NULL, NULL, NULL, NULL, FALSE, TRUE);
	gtk_ctree_node_set_row_data (ctree, ctree_colors, "Colors");
	gtk_ctree_set_expander_style (ctree, GTK_CTREE_EXPANDER_TRIANGLE);
	gtk_ctree_set_line_style (ctree, GTK_CTREE_LINES_NONE);
	
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
	menu = gtk_menu_item_new_with_label (OPTIONS_DEFAULT_MIME_STRING_PLAIN_TEXT);
	gtk_widget_show (menu);
	gtk_menu_append (GTK_MENU (GTK_OPTION_MENU (options_default_mime)->menu), menu);
	
	menu = gtk_menu_item_new_with_label (OPTIONS_DEFAULT_MIME_STRING_HTML_TEXT);
	gtk_widget_show (menu);
	gtk_menu_append (GTK_MENU (GTK_OPTION_MENU (options_default_mime)->menu), menu);
	gtk_option_menu_set_history (GTK_OPTION_MENU (options_default_mime), c2_app.options_default_mime);

	/* accounts_clist */
	accounts_clist = GTK_CLIST (glade_xml_get_widget (preferences_xml, "accounts_clist"));
	gtk_signal_connect (GTK_OBJECT (accounts_clist), "select_row",
						GTK_SIGNAL_FUNC (on_accounts_clist_select_row), NULL);
	gtk_signal_connect (GTK_OBJECT (accounts_clist), "unselect_row",
						GTK_SIGNAL_FUNC (on_accounts_clist_unselect_row), NULL);
	for (account = c2_app.account; account != NULL; account = account->next)
	{
		gchar *row[] =
		{
			account->name,
			(account->type == C2_ACCOUNT_POP3) ? "POP" : "Spool",
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
	pixmap = gnome_stock_pixmap_widget_at_size (dialog, GNOME_STOCK_PIXMAP_ADD, 16, 16);
	gtk_box_pack_start (GTK_BOX (accounts_new_pixmap), pixmap, TRUE, TRUE, 0);
	gtk_widget_show (pixmap);

	/* accounts_edit_btn */
	accounts_edit_btn = glade_xml_get_widget (preferences_xml, "accounts_edit_btn");
	gtk_signal_connect (GTK_OBJECT (accounts_edit_btn), "clicked",
						GTK_SIGNAL_FUNC (on_accounts_edit_btn_clicked), NULL);

	/* accounts_edit_pixmap */
	accounts_edit_pixmap = glade_xml_get_widget (preferences_xml, "accounts_edit_pixmap");
	pixmap = gnome_stock_pixmap_widget_at_size (dialog, GNOME_STOCK_PIXMAP_INDEX, 16, 16);
	gtk_box_pack_start (GTK_BOX (accounts_edit_pixmap), pixmap, TRUE, TRUE, 0);
	gtk_widget_show (pixmap);

	/* accounts_delete_btn */
	accounts_delete_btn = glade_xml_get_widget (preferences_xml, "accounts_delete_btn");
	gtk_signal_connect (GTK_OBJECT (accounts_delete_btn), "clicked",
						GTK_SIGNAL_FUNC (on_accounts_delete_btn_clicked), NULL);

	/* accounts_delete_pixmap */
	accounts_delete_pixmap = glade_xml_get_widget (preferences_xml, "accounts_delete_pixmap");
	pixmap = gnome_stock_pixmap_widget_at_size (dialog, GNOME_STOCK_PIXMAP_REMOVE, 16, 16);
	gtk_box_pack_start (GTK_BOX (accounts_delete_pixmap), pixmap, TRUE, TRUE, 0);
	gtk_widget_show (pixmap);

	/* accounts_up_btn */
	accounts_up_btn = glade_xml_get_widget (preferences_xml, "accounts_up_btn");
	gtk_signal_connect (GTK_OBJECT (accounts_up_btn), "clicked",
						GTK_SIGNAL_FUNC (on_accounts_up_btn_clicked), NULL);

	/* accounts_up_pixmap */
	accounts_up_pixmap = glade_xml_get_widget (preferences_xml, "accounts_up_pixmap");
	pixmap = gnome_stock_pixmap_widget_at_size (dialog, GNOME_STOCK_PIXMAP_UP, 16, 16);
	gtk_box_pack_start (GTK_BOX (accounts_up_pixmap), pixmap, TRUE, TRUE, 0);
	gtk_widget_show (pixmap);

	/* accounts_down_btn */
	accounts_down_btn = glade_xml_get_widget (preferences_xml, "accounts_down_btn");
	gtk_signal_connect (GTK_OBJECT (accounts_down_btn), "clicked",
						GTK_SIGNAL_FUNC (on_accounts_down_btn_clicked), NULL);

	/* accounts_down_pixmap */
	accounts_down_pixmap = glade_xml_get_widget (preferences_xml, "accounts_down_pixmap");
	pixmap = gnome_stock_pixmap_widget_at_size (dialog, GNOME_STOCK_PIXMAP_DOWN, 16, 16);
	gtk_box_pack_start (GTK_BOX (accounts_down_pixmap), pixmap, TRUE, TRUE, 0);
	gtk_widget_show (pixmap);

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
	menu = gtk_menu_item_new_with_label (FONTS_SOURCE_STRING_USE_DOCUMENT);
	gtk_widget_show (menu);
	gtk_menu_append (GTK_MENU (GTK_OPTION_MENU (fonts_source)->menu), menu);
	menu = gtk_menu_item_new_with_label (FONTS_SOURCE_STRING_USE_MY);
	gtk_widget_show (menu);
	gtk_menu_append (GTK_MENU (GTK_OPTION_MENU (fonts_source)->menu), menu);
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
	gtk_entry_set_text (GTK_ENTRY (paths_saving_entry), c2_app.paths_saving);

	/* paths_saving_btn */
	paths_saving_btn = glade_xml_get_widget (preferences_xml, "paths_saving_btn");
	gtk_signal_connect (GTK_OBJECT (paths_saving_btn), "clicked",
						GTK_SIGNAL_FUNC (on_paths_btn_clicked), paths_saving_entry);

	/* paths_download_entry */
	paths_download_entry = glade_xml_get_widget (preferences_xml, "paths_download_entry");
	gtk_entry_set_text (GTK_ENTRY (paths_download_entry), c2_app.paths_download);

	/* paths_download_btn */
	paths_download_btn = glade_xml_get_widget (preferences_xml, "paths_download_btn");
	gtk_signal_connect (GTK_OBJECT (paths_download_btn), "clicked",
						GTK_SIGNAL_FUNC (on_paths_btn_clicked), paths_download_entry);

	/* paths_get_entry */
	paths_get_entry = glade_xml_get_widget (preferences_xml, "paths_get_entry");
	gtk_entry_set_text (GTK_ENTRY (paths_get_entry), c2_app.paths_get);

	/* paths_get_btn */
	paths_get_btn = glade_xml_get_widget (preferences_xml, "paths_get_btn");
	gtk_signal_connect (GTK_OBJECT (paths_get_btn), "clicked",
						GTK_SIGNAL_FUNC (on_paths_btn_clicked), paths_get_entry);

	/* paths_always_use */
	paths_always_use = glade_xml_get_widget (preferences_xml, "paths_always_use");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (paths_always_use), c2_app.paths_always_use);
	
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
on_ok_btn_clicked (void)
{
	on_apply_btn_clicked ();
	on_cancel_btn_clicked ();
}
	
static void
on_apply_btn_clicked (void)
{
	{ /* Options page */
		GtkWidget *check_timeout = glade_xml_get_widget (preferences_xml, "options_check_timeout");
		GtkWidget *mark_timeout = glade_xml_get_widget (preferences_xml, "options_mark_timeout");
		GtkWidget *prepend_character = glade_xml_get_widget (preferences_xml, "options_prepend_character");
		GtkWidget *empty_garbage = glade_xml_get_widget (preferences_xml, "options_empty_garbage");
		GtkWidget *use_outbox = glade_xml_get_widget (preferences_xml, "options_use_outbox");
		GtkWidget *check_at_start = glade_xml_get_widget (preferences_xml, "options_check_at_start");
		GtkWidget *default_mime = glade_xml_get_widget (preferences_xml, "options_default_mime");
		gchar *selection;

		c2_app.options_check_timeout = gtk_spin_button_get_value_as_int (
										GTK_SPIN_BUTTON (check_timeout));
		c2_app.options_mark_timeout = gtk_spin_button_get_value_as_int (
										GTK_SPIN_BUTTON (mark_timeout));

		g_free (c2_app.options_prepend_character);
		c2_app.options_prepend_character = g_strdup (gtk_entry_get_text (
											GTK_ENTRY (GTK_COMBO (prepend_character)->entry)));
		
		c2_app.options_empty_garbage = GTK_TOGGLE_BUTTON (empty_garbage)->active;
		c2_app.options_use_outbox = GTK_TOGGLE_BUTTON (use_outbox)->active;
		c2_app.options_check_at_start = GTK_TOGGLE_BUTTON (check_at_start)->active;

		if (GTK_BIN (default_mime)->child)
		{
			GtkWidget *child = GTK_BIN (default_mime)->child;
			
			if (GTK_LABEL (child))
			{
				gtk_label_get (GTK_LABEL (child), &selection);
				if (c2_streq (selection, OPTIONS_DEFAULT_MIME_STRING_PLAIN_TEXT))
					c2_app.options_default_mime = C2_DEFAULT_MIME_PLAIN;
				else
					c2_app.options_default_mime = C2_DEFAULT_MIME_HTML;
			}
		}

		gnome_config_set_int ("/cronosII/Options/check_timeout", c2_app.options_check_timeout);
		gnome_config_set_int ("/cronosII/Options/mark_timeout", c2_app.options_mark_timeout);
		gnome_config_set_string ("/cronosII/Options/prepend_character", c2_app.options_prepend_character);
		gnome_config_set_int ("/cronosII/Options/empty_garbage", c2_app.options_empty_garbage);
		gnome_config_set_int ("/cronosII/Options/use_outbox", c2_app.options_use_outbox);
		gnome_config_set_int ("/cronosII/Options/check_at_start", c2_app.options_check_at_start);
		gnome_config_set_int ("/cronosII/Options/default_mime", c2_app.options_default_mime);
	}
	{ /* Accounts */
		GtkCList *accounts_clist = GTK_CLIST (glade_xml_get_widget (preferences_xml, "accounts_clist"));
		C2Account *account;
		gint i;
		gchar *buf;

		/* Free the c2_app.account list */
		c2_account_free_all (c2_app.account);
		c2_app.account = NULL;
		
		for (i = 0; i < accounts_clist->rows; i++)
		{
			account = gtk_clist_get_row_data (accounts_clist, i);
			account->next = gtk_clist_get_row_data (accounts_clist, i+1);
			
			if (!c2_app.account)
				c2_app.account = account;

			buf = g_strdup_printf ("/cronosII/Account %d/", i);
			gnome_config_push_prefix (buf);
			gnome_config_set_string ("name", account->name);
			gnome_config_set_string ("per_name", account->per_name);
			gnome_config_set_string ("email", account->email);
			gnome_config_set_int ("options.active", account->options.active),
			gnome_config_set_string ("signature.string", account->signature.string);
			gnome_config_set_int ("signature.automatically", account->signature.automatic);
			gnome_config_set_int ("type", account->type);

			switch (account->type)
			{
				case C2_ACCOUNT_POP3:
					gnome_config_set_string ("pop3.username", account->protocol.pop3->user);
					gnome_config_set_string ("pop3.password", account->protocol.pop3->pass);
					gnome_config_set_string ("pop3.hostname", account->protocol.pop3->host);
					gnome_config_set_int ("pop3.port", account->protocol.pop3->port);
					gnome_config_set_int ("pop3.flags", account->protocol.pop3->flags);
					break;
			}
			gnome_config_pop_prefix ();
			g_free (buf);
		}
	}
	{ /* Interface */
		GtkWidget *title = glade_xml_get_widget (preferences_xml, "interface_title");
		GtkWidget *toolbar = glade_xml_get_widget (preferences_xml, "interface_toolbar");
		GtkWidget *date_fmt = glade_xml_get_widget (preferences_xml, "interface_date_fmt");
		gchar *selection;

		c2_app.interface_title = gtk_entry_get_text (GTK_ENTRY (title));

		if (GTK_BIN (toolbar)->child)
		{
			GtkWidget *child = GTK_BIN (toolbar)->child;
			
			if (GTK_LABEL (child))
			{
				gtk_label_get (GTK_LABEL (child), &selection);
				if (c2_streq (selection, INTERFACE_TOOLBAR_STRING_ICONS_ONLY))
					c2_app.interface_toolbar = GTK_TOOLBAR_ICONS;
				else if (c2_streq (selection, INTERFACE_TOOLBAR_STRING_TEXT_ONLY))
					c2_app.interface_toolbar = GTK_TOOLBAR_TEXT;
				else
					c2_app.interface_toolbar = GTK_TOOLBAR_BOTH;
			}
		}

		g_free (c2_app.interface_date_fmt);
		c2_app.interface_date_fmt = gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (date_fmt)->entry));

		gtk_toolbar_set_style (GTK_TOOLBAR (glade_xml_get_widget (WMain.xml, "toolbar")),
								c2_app.interface_toolbar);
		gtk_widget_queue_resize (glade_xml_get_widget (WMain.xml, "wnd_main"));

		gnome_config_set_string ("/cronosII/Interface/title", c2_app.interface_title);
		gnome_config_set_int ("/cronosII/Interface/toolbar", c2_app.interface_toolbar);
		gnome_config_set_string ("/cronosII/Interface/date_fmt", c2_app.interface_date_fmt);
	}
	{ /* Fonts */
		GtkWidget *message_body = glade_xml_get_widget (preferences_xml, "fonts_message_body");
		GtkWidget *unreaded_message = glade_xml_get_widget (preferences_xml, "fonts_unreaded_message");
		GtkWidget *readed_message = glade_xml_get_widget (preferences_xml, "fonts_readed_message");
		GtkWidget *source = glade_xml_get_widget (preferences_xml, "fonts_source");
		gchar *selection;
		
		g_free (c2_app.fonts_message_body);
		g_free (c2_app.fonts_unreaded_message);
		g_free (c2_app.fonts_readed_message);

		c2_app.fonts_message_body = gnome_font_picker_get_font_name (GNOME_FONT_PICKER (message_body));
		c2_app.fonts_unreaded_message = gnome_font_picker_get_font_name (GNOME_FONT_PICKER (unreaded_message));
		c2_app.fonts_readed_message = gnome_font_picker_get_font_name (GNOME_FONT_PICKER (readed_message));

		gdk_font_unref (c2_app.fonts_gdk_message_body);
		c2_app.fonts_gdk_message_body = gnome_font_picker_get_font (GNOME_FONT_PICKER (message_body));
		gdk_font_unref (c2_app.fonts_gdk_unreaded_message);
		c2_app.fonts_gdk_unreaded_message = gnome_font_picker_get_font (GNOME_FONT_PICKER (unreaded_message));
		gdk_font_unref (c2_app.fonts_gdk_readed_message);
		c2_app.fonts_gdk_readed_message = gnome_font_picker_get_font (GNOME_FONT_PICKER (readed_message));

		if (GTK_BIN (source)->child)
		{
			GtkWidget *child = GTK_BIN (source)->child;
			
			if (GTK_LABEL (child))
			{
				gtk_label_get (GTK_LABEL (child), &selection);
				if (c2_streq (selection, FONTS_SOURCE_STRING_USE_DOCUMENT))
					c2_app.fonts_source = C2_FONT_USE_DOCUMENTS_FONT;
				else
					c2_app.fonts_source = C2_FONT_USE_MY_FONT;
			}
		}

		gnome_config_set_string ("/cronosII/Fonts/message_body", c2_app.fonts_message_body);
		gnome_config_set_string ("/cronosII/Fonts/unreaded_message", c2_app.fonts_unreaded_message);
		gnome_config_set_string ("/cronosII/Fonts/readed_message", c2_app.fonts_readed_message);
		gnome_config_set_int ("/cronosII/Fonts/source", c2_app.fonts_source);
	}
	{ /* Colors */
		GtkWidget *replying_original_message = glade_xml_get_widget
												(preferences_xml, "colors_replying_original_message");
		GtkWidget *message_bg = glade_xml_get_widget (preferences_xml, "colors_message_bg");
		GtkWidget *message_fg = glade_xml_get_widget (preferences_xml, "colors_message_fg");
		GtkWidget *message_source = glade_xml_get_widget (preferences_xml, "colors_message_source");

		gnome_color_picker_get_i16 (GNOME_COLOR_PICKER (replying_original_message),
									&c2_app.colors_replying_original_message.red,
									&c2_app.colors_replying_original_message.green,
									&c2_app.colors_replying_original_message.blue,
									(gushort*) &c2_app.colors_replying_original_message.pixel);
		gdk_color_alloc (gdk_colormap_get_system (), &c2_app.colors_replying_original_message);
		
		gnome_color_picker_get_i16 (GNOME_COLOR_PICKER (message_bg),
									&c2_app.colors_message_bg.red,
									&c2_app.colors_message_bg.green,
									&c2_app.colors_message_bg.blue,
									(gushort*) &c2_app.colors_message_bg.pixel);
		gdk_color_alloc (gdk_colormap_get_system (), &c2_app.colors_message_bg);
		
		gnome_color_picker_get_i16 (GNOME_COLOR_PICKER (message_fg),
									&c2_app.colors_message_fg.red,
									&c2_app.colors_message_fg.green,
									&c2_app.colors_message_fg.blue,
									(gushort*) &c2_app.colors_message_fg.pixel);
		gdk_color_alloc (gdk_colormap_get_system (), &c2_app.colors_message_fg);
	}
	{ /* Paths */
		GtkWidget *saving_entry = glade_xml_get_widget (preferences_xml, "paths_saving_entry");
		GtkWidget *download_entry = glade_xml_get_widget (preferences_xml, "paths_download_entry");
		GtkWidget *get_entry = glade_xml_get_widget (preferences_xml, "paths_get_entry");
		GtkWidget *always_use = glade_xml_get_widget (preferences_xml, "paths_always_use");

		g_free (c2_app.paths_saving);
		g_free (c2_app.paths_download);
		g_free (c2_app.paths_get);

		c2_app.paths_saving = gtk_entry_get_text (GTK_ENTRY (saving_entry));
		c2_app.paths_download = gtk_entry_get_text (GTK_ENTRY (download_entry));
		c2_app.paths_get = gtk_entry_get_text (GTK_ENTRY (get_entry));
		c2_app.paths_always_use = GTK_TOGGLE_BUTTON (always_use)->active;

		gnome_config_set_string ("/cronosII/Paths/saving", c2_app.paths_saving);
		gnome_config_set_string ("/cronosII/Paths/download", c2_app.paths_download);
		gnome_config_set_string ("/cronosII/Paths/get", c2_app.paths_get);
		gnome_config_set_int ("/cronosII/Paths/always_use", c2_app.paths_always_use);
	}
	{ /* Advanced */
		GtkWidget *http_proxy_btn = glade_xml_get_widget (preferences_xml, "advanced_http_proxy_btn");
		GtkWidget *http_proxy_addr = glade_xml_get_widget (preferences_xml, "advanced_http_proxy_addr");
		GtkWidget *http_proxy_port = glade_xml_get_widget (preferences_xml, "advanced_http_proxy_port");
		GtkWidget *ftp_proxy_btn = glade_xml_get_widget (preferences_xml, "advanced_ftp_proxy_btn");
		GtkWidget *ftp_proxy_addr = glade_xml_get_widget (preferences_xml, "advanced_ftp_proxy_addr");
		GtkWidget *ftp_proxy_port = glade_xml_get_widget (preferences_xml, "advanced_ftp_proxy_port");
		GtkWidget *persistent_smtp_btn = glade_xml_get_widget(preferences_xml, "advanced_persistent_smtp_btn");
		GtkWidget *persistent_smtp_addr=glade_xml_get_widget(preferences_xml,"advanced_persistent_smtp_addr");
		GtkWidget *persistent_smtp_port=glade_xml_get_widget(preferences_xml,"advanced_persistent_smtp_port");
		GtkWidget *use_internal_browser=glade_xml_get_widget(preferences_xml,"advanced_use_internal_browser");

		c2_app.advanced_http_proxy_addr = gtk_entry_get_text (GTK_ENTRY (http_proxy_addr));
		c2_app.advanced_http_proxy_port = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (http_proxy_port));
		c2_app.advanced_http_proxy = GTK_TOGGLE_BUTTON (http_proxy_btn)->active;
		c2_app.advanced_ftp_proxy_addr = gtk_entry_get_text (GTK_ENTRY (ftp_proxy_addr));
		c2_app.advanced_ftp_proxy_port = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (ftp_proxy_port));
		c2_app.advanced_ftp_proxy = GTK_TOGGLE_BUTTON (ftp_proxy_btn)->active;
		c2_app.advanced_persistent_smtp_addr = gtk_entry_get_text (GTK_ENTRY (persistent_smtp_addr));
		c2_app.advanced_persistent_smtp_port = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (persistent_smtp_port));
		c2_app.advanced_persistent_smtp = GTK_TOGGLE_BUTTON (persistent_smtp_btn)->active;
		c2_app.advanced_use_internal_browser = GTK_TOGGLE_BUTTON (use_internal_browser)->active;


		gnome_config_set_int ("/cronosII/Advanced/http_proxy", c2_app.advanced_http_proxy);
		gnome_config_set_string ("/cronosII/Advanced/http_proxy_addr", c2_app.advanced_http_proxy_addr);
		gnome_config_set_int ("/cronosII/Advanced/http_proxy_port", c2_app.advanced_http_proxy_port);
		gnome_config_set_int ("/cronosII/Advanced/ftp_proxy", c2_app.advanced_ftp_proxy);
		gnome_config_set_string ("/cronosII/Advanced/ftp_proxy_addr", c2_app.advanced_ftp_proxy_addr);
		gnome_config_set_int ("/cronosII/Advanced/ftp_proxy_port", c2_app.advanced_ftp_proxy_port);
		gnome_config_set_int ("/cronosII/Advanced/persistent_smtp", c2_app.advanced_persistent_smtp);
		gnome_config_set_string ("/cronosII/Advanced/persistent_smtp_addr", c2_app.advanced_persistent_smtp_addr);
		gnome_config_set_int ("/cronosII/Advanced/persistent_smtp_port", c2_app.advanced_persistent_smtp_port);
		gnome_config_set_int ("/cronosII/Advanced/use_internal_browser", c2_app.advanced_use_internal_browser);
		
	}
	
	gnome_config_sync ();
}

static void
on_cancel_btn_clicked (void)
{
	gnome_dialog_close (GNOME_DIALOG (glade_xml_get_widget (preferences_xml, "dlg_preferences")));
}

static void
on_accounts_clist_select_row (GtkCList *clist)
{
	GtkWidget *accounts_edit_btn = glade_xml_get_widget (preferences_xml, "accounts_edit_btn");
	GtkWidget *accounts_delete_btn = glade_xml_get_widget (preferences_xml, "accounts_delete_btn");
	GtkWidget *accounts_up_btn = glade_xml_get_widget (preferences_xml, "accounts_up_btn");
	GtkWidget *accounts_down_btn = glade_xml_get_widget (preferences_xml, "accounts_down_btn");
L	
	gtk_widget_set_sensitive (accounts_edit_btn, TRUE);
	gtk_widget_set_sensitive (accounts_delete_btn, TRUE);

	if (GPOINTER_TO_INT (clist->selection->data) == 0)
		gtk_widget_set_sensitive (accounts_up_btn, FALSE);
	else
		gtk_widget_set_sensitive (accounts_up_btn, TRUE);

	if (GPOINTER_TO_INT (clist->selection->data) == clist->rows-1)
		gtk_widget_set_sensitive (accounts_down_btn, FALSE);
	else
		gtk_widget_set_sensitive (accounts_down_btn, TRUE);
}

static void
on_accounts_clist_unselect_row (GtkCList *clist)
{
	GtkWidget *accounts_edit_btn = glade_xml_get_widget (preferences_xml, "accounts_edit_btn");
	GtkWidget *accounts_delete_btn = glade_xml_get_widget (preferences_xml, "accounts_delete_btn");
	GtkWidget *accounts_up_btn = glade_xml_get_widget (preferences_xml, "accounts_up_btn");
	GtkWidget *accounts_down_btn = glade_xml_get_widget (preferences_xml, "accounts_down_btn");
	
	gtk_widget_set_sensitive (accounts_edit_btn, FALSE);
	gtk_widget_set_sensitive (accounts_delete_btn, FALSE);
	gtk_widget_set_sensitive (accounts_up_btn, FALSE);
	gtk_widget_set_sensitive (accounts_down_btn, FALSE);
}

static gboolean
on_accounts_new_druidpagestandard1_next (GtkWidget *druid_page, GtkWidget *druid, GladeXML *xml)
{
	GtkWidget *w_name = glade_xml_get_widget (xml, "account_name");
	GtkWidget *w_email = glade_xml_get_widget (xml, "email");
	GtkWidget *w_clist = glade_xml_get_widget (preferences_xml, "accounts_clist");
	gchar *name = gtk_entry_get_text (GTK_ENTRY (w_name));
	gchar *email = gtk_entry_get_text (GTK_ENTRY (w_email));
	gint i;
	C2Account *account;
	
	/* 1. Consist the name of the account */
	/* 2. Consist the email address */

	/* (1) */
	for (i = 0; i < GTK_CLIST (w_clist)->rows; i++)
	{
		account = gtk_clist_get_row_data (GTK_CLIST (w_clist), i);
		if (c2_streq (account->name, name))
		{
			/* Taken name */
			GladeXML *tmp_xml = glade_xml_new (C2_APP_GLADE_FILE ("preferences"),
											"dlg_account_new_error_wrong_name");
			gnome_dialog_run_and_close (GNOME_DIALOG (glade_xml_get_widget (tmp_xml,
											"dlg_account_new_error_wrong_name")));
			return TRUE;
		}
	}

	if (!strlen (name))
	{
		GladeXML *tmp_xml = glade_xml_new (C2_APP_GLADE_FILE ("preferences"),
											"dlg_account_new_error_wrong_name");
		gnome_dialog_run_and_close (GNOME_DIALOG (glade_xml_get_widget (tmp_xml,
											"dlg_account_new_error_wrong_name")));
		return TRUE;
	}

	/* (2) */
	if (!strlen (email))
	{
		GladeXML *tmp_xml = glade_xml_new (C2_APP_GLADE_FILE ("preferences"),
											"dlg_account_new_error_wrong_email");
		gnome_dialog_run_and_close (GNOME_DIALOG (glade_xml_get_widget (tmp_xml,
											"dlg_account_new_error_wrong_email")));
		return TRUE;
	}
	return FALSE;
}

static gboolean
on_accounts_new_druidpagestandard2_next (GtkWidget *druid_page, GtkWidget *druid, GladeXML *xml)
{
	GtkWidget *protocol_type = glade_xml_get_widget (xml, "protocol_type");
	GtkWidget *pop3_server_addr = glade_xml_get_widget (xml, "pop3_server_addr");
	GtkWidget *pop3_username = glade_xml_get_widget (xml, "pop3_username");
	GtkWidget *smtp_remote_btn = glade_xml_get_widget (xml, "smtp_remote_btn");
	GtkWidget *smtp_remote_server_addr = glade_xml_get_widget (xml, "smtp_remote_server_addr");
	GtkWidget *smtp_remote_server_port = glade_xml_get_widget (xml, "smtp_remote_server_port");
	gchar *selection;
	C2AccountType account_type;

	/* Get the type of account */
	if (GTK_BIN (protocol_type)->child)
	{
		GtkWidget *child = GTK_BIN (protocol_type)->child;
		
		if (GTK_LABEL (child))
		{
			gtk_label_get (GTK_LABEL (child), &selection);
			if (c2_streq (selection, ACCOUNTS_NEW_PROTOCOL_TYPE_STRING_POP3))
				account_type = C2_ACCOUNT_POP3;
		}
	}

	switch (account_type)
	{
		case C2_ACCOUNT_POP3:
			/* Check that the user entered some host and username */
			if (!strlen (gtk_entry_get_text (GTK_ENTRY (pop3_server_addr))))
			{
				GladeXML *tmpxml = glade_xml_new (C2_APP_GLADE_FILE ("preferences"),
													"dlg_account_new_error_pop3_no_host");
				
				gnome_dialog_run_and_close (GNOME_DIALOG (glade_xml_get_widget (tmpxml,
													"dlg_account_new_error_pop3_no_host")));
				return TRUE;
			}

			if (!strlen (gtk_entry_get_text (GTK_ENTRY (pop3_username))))
			{
				GladeXML *tmpxml = glade_xml_new (C2_APP_GLADE_FILE ("preferences"),
													"dlg_account_new_error_pop3_no_user");
				
				gnome_dialog_run_and_close (GNOME_DIALOG (glade_xml_get_widget (tmpxml,
													"dlg_account_new_error_pop3_no_user")));
				return TRUE;
			}
			break;
	}

	if (GTK_TOGGLE_BUTTON (smtp_remote_btn)->active)
	{
		/* Check that the information of the remote SMTP server is complete */
		if (!strlen (gtk_entry_get_text (GTK_ENTRY (smtp_remote_server_addr))))
		{
			GladeXML *tmpxml = glade_xml_new (C2_APP_GLADE_FILE ("preferences"),
												"dlg_account_new_error_no_smtp_info");
			
			gnome_dialog_run_and_close (GNOME_DIALOG (glade_xml_get_widget (tmpxml,
												"dlg_account_new_error_no_smtp_info")));
			return TRUE;
		}
	}

	return FALSE;
}

static void
on_accounts_new_smtp_btn_clicked (GtkWidget *widget, GladeXML *xml)
{
	GtkWidget *smtp_remote_btn = glade_xml_get_widget (xml, "smtp_remote_btn");
	GtkWidget *smtp_remote_information_btn = glade_xml_get_widget (xml, "smtp_remote_information_btn");

	gtk_widget_set_sensitive (smtp_remote_information_btn, GTK_TOGGLE_BUTTON (smtp_remote_btn)->active);
}

static void
on_accounts_new_smtp_remote_information_btn_clicked_authentication_btn_clicked (GtkWidget *widget, GladeXML *xml)
{
	GtkWidget *authentication_frame = glade_xml_get_widget (xml, "authentication_frame");
	
	if (GTK_TOGGLE_BUTTON (widget)->active)
		gtk_widget_show (authentication_frame);
	else
		gtk_widget_hide (authentication_frame);
}

static void
on_accounts_new_smtp_remote_information_btn_clicked (GtkWidget *widget, GladeXML *xml)
{
	GladeXML *tmpxml = glade_xml_new (C2_APP_GLADE_FILE ("preferences"), "dlg_smtp_server");
	GtkWidget *dialog = glade_xml_get_widget (tmpxml, "dlg_smtp_server");
	GtkWidget *smtp_remote_server_addr = glade_xml_get_widget (xml, "smtp_remote_server_addr");
	GtkWidget *smtp_remote_server_port = glade_xml_get_widget (xml, "smtp_remote_server_port");
	GtkWidget *smtp_remote_authentication = glade_xml_get_widget (xml, "smtp_remote_authentication");
	GtkWidget *smtp_remote_server_username = glade_xml_get_widget (xml, "smtp_remote_username");
	GtkWidget *smtp_remote_server_password = glade_xml_get_widget (xml, "smtp_remote_password");
	GtkWidget *hostname = glade_xml_get_widget (tmpxml, "hostname");
	GtkWidget *port = glade_xml_get_widget (tmpxml, "port");
	GtkWidget *authentication = glade_xml_get_widget (tmpxml, "authentication");
	GtkWidget *username = glade_xml_get_widget (tmpxml, "username");
	GtkWidget *password = glade_xml_get_widget (tmpxml, "password");

	gtk_signal_connect (GTK_OBJECT (authentication), "toggled",
						GTK_SIGNAL_FUNC (on_accounts_new_smtp_remote_information_btn_clicked_authentication_btn_clicked), tmpxml);
	gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (hostname)->entry),
						gtk_entry_get_text (GTK_ENTRY (smtp_remote_server_addr)));
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (port),
						gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (smtp_remote_server_port)));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (authentication),
						GTK_TOGGLE_BUTTON (smtp_remote_authentication)->active);
	gtk_entry_set_text (GTK_ENTRY (username),
						gtk_entry_get_text (GTK_ENTRY (smtp_remote_server_username)));
	gtk_entry_set_text (GTK_ENTRY (password),
						gtk_entry_get_text (GTK_ENTRY (smtp_remote_server_password)));
	
	switch (GTK_TOGGLE_BUTTON (smtp_remote_authentication)->active)
	{
		case 0:
			gtk_widget_hide (glade_xml_get_widget (tmpxml, "authentication_frame"));
			break;
		case 1:
			gtk_widget_show (glade_xml_get_widget (tmpxml, "authentication_frame"));
			break;
	}
			

	switch (gnome_dialog_run (GNOME_DIALOG (dialog)))
	{
		case 0:
			gtk_entry_set_text (GTK_ENTRY (smtp_remote_server_addr),
								gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (hostname)->entry)));
			gtk_spin_button_set_value (GTK_SPIN_BUTTON (smtp_remote_server_port),
								gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (port)));
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (smtp_remote_authentication),
								GTK_TOGGLE_BUTTON (authentication)->active);
			gtk_entry_set_text (GTK_ENTRY (smtp_remote_server_username),
								gtk_entry_get_text (GTK_ENTRY (username)));
			gtk_entry_set_text (GTK_ENTRY (smtp_remote_server_password),
								gtk_entry_get_text (GTK_ENTRY (password)));
		case 1:
			gnome_dialog_close (GNOME_DIALOG (dialog));
	}
}

static void
on_accounts_new_druidpagefinish1_finish (GnomeDruidPage *druid_page, GtkWidget *druid, GladeXML *xml)
{
	gchar *account_name, *person_name, *organization, *email, *replyto, *pop3_host,
			*pop3_user, *pop3_pass, *smtp_host, *smtp_user, *smtp_pass,
			*signature;
	gint pop3_port, pop3_flags, smtp_port;
	gboolean active, smtp_authentication, keep_copy, signature_automatic;
	C2AccountType account_type;
	C2AccountSignatureType signature_type;
	C2SMTPType smtp_type;
	C2Account *account;
	gchar *selection;
	GtkWidget *buf;
	gchar *clist_row[3];
	
	/* Collect the data */
	account_name = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (xml, "account_name")));
	person_name = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (xml, "person_name")));
	organization = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (xml, "organization")));
	email = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (xml, "email")));
	replyto = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (xml, "reply_to")));
	active = GTK_TOGGLE_BUTTON (glade_xml_get_widget (xml, "include_account"))->active;

	buf = glade_xml_get_widget (xml, "protocol_type");
	if (GTK_BIN (buf)->child)
	{
		GtkWidget *child = GTK_BIN (buf)->child;
		
		if (GTK_LABEL (child))
		{
			gtk_label_get (GTK_LABEL (child), &selection);
			if (c2_streq (selection, ACCOUNTS_NEW_PROTOCOL_TYPE_STRING_POP3))
				account_type = C2_ACCOUNT_POP3;
		}
	}

	if (account_type == C2_ACCOUNT_POP3)
	{
		pop3_host = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (xml, "pop3_server_addr")));
		pop3_port = gtk_spin_button_get_value_as_int (
								GTK_SPIN_BUTTON (glade_xml_get_widget (xml, "pop3_server_port")));
		pop3_user = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (xml, "pop3_username")));
		pop3_pass = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (xml, "pop3_password")));
	}
	
	if (GTK_TOGGLE_BUTTON (glade_xml_get_widget (xml, "smtp_remote_btn"))->active)
		smtp_type = C2_SMTP_REMOTE;
	else
		smtp_type = C2_SMTP_LOCAL;

	if (smtp_type == C2_SMTP_REMOTE)
	{
		smtp_host = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (xml, "smtp_remote_server_addr")));
		smtp_port = gtk_spin_button_get_value_as_int (
								GTK_SPIN_BUTTON (glade_xml_get_widget (xml, "smtp_remote_server_port")));
		smtp_authentication = GTK_TOGGLE_BUTTON (glade_xml_get_widget
										(xml, "smtp_remote_authentication"))->active;
		smtp_user = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (xml, "smtp_remote_username")));
		smtp_pass = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (xml, "smtp_remote_password")));
	}

	if (GTK_TOGGLE_BUTTON (glade_xml_get_widget (xml, "leave_messages"))->active)
		pop3_flags = C2_POP3_DO_KEEP_COPY;
	else
		pop3_flags = C2_POP3_DONT_KEEP_COPY;
	

	buf = glade_xml_get_widget (xml, "signature_type");
	if (GTK_BIN (buf)->child)
	{
		GtkWidget *child = GTK_BIN (buf)->child;
		
		if (GTK_LABEL (child))
		{
			gtk_label_get (GTK_LABEL (child), &selection);
			if (c2_streq (selection, ACCOUNTS_NEW_SIGNATURE_TYPE_STRING_STATIC))
				signature_type = C2_ACCOUNT_SIGNATURE_STATIC;
			else
				signature_type = C2_ACCOUNT_SIGNATURE_DYNAMIC;
		}
	}

	signature = gtk_editable_get_chars (GTK_EDITABLE (glade_xml_get_widget (xml, "signature_text")), 0, -1);
	signature_automatic = GTK_TOGGLE_BUTTON (glade_xml_get_widget (xml, "signature_automatic"))->active;

	switch (smtp_type)
	{
		case C2_SMTP_REMOTE:
			account = c2_account_new (account_name, person_name, organization, email, replyto,
										active, account_type, smtp_type, signature_type, signature,
										signature_automatic, pop3_host, pop3_port, pop3_user, pop3_pass,
										pop3_flags,
										smtp_host, smtp_port, smtp_authentication, smtp_user, smtp_pass);
			break;
		case C2_SMTP_LOCAL:
			account = c2_account_new (account_name, person_name, organization, email, replyto,
										active, account_type, smtp_type, signature_type, signature,
										pop3_flags,
										signature_automatic, pop3_host, pop3_port, pop3_user, pop3_pass);
			break;
	}

	gtk_widget_destroy (glade_xml_get_widget (xml, "dlg_account_new"));

	buf = glade_xml_get_widget (preferences_xml, "accounts_clist");
	clist_row[0] = account->name;
	clist_row[1] = account->per_name;
	clist_row[2] = account->email;

	gtk_clist_append (GTK_CLIST (buf), clist_row);
	gtk_clist_set_row_data (GTK_CLIST (buf), GTK_CLIST (buf)->rows-1, account);
}

static void
on_accounts_new_btn_clicked (void)
{
	GladeXML *xml = glade_xml_new (C2_APP_GLADE_FILE ("preferences"), "dlg_account_new");
	GtkWidget *dialog = glade_xml_get_widget (xml, "dlg_account_new");
	GtkWidget *druidpagestandard1 = glade_xml_get_widget (xml, "druidpagestandard1");
	GtkWidget *druidpagestandard2 = glade_xml_get_widget (xml, "druidpagestandard2");
	GtkWidget *druidpagefinish1 = glade_xml_get_widget (xml, "druidpagefinish1");
	GtkWidget *smtp_remote_btn = glade_xml_get_widget (xml, "smtp_remote_btn");
	GtkWidget *smtp_remote_information_btn = glade_xml_get_widget (xml, "smtp_remote_information_btn");
	GtkWidget *smtp_local = glade_xml_get_widget (xml, "smtp_local");
	GtkWidget *protocol_type = glade_xml_get_widget (xml, "protocol_type");
	GtkWidget *signature_type = glade_xml_get_widget (xml, "signature_type");
	GtkWidget *menu;

	menu = gtk_menu_item_new_with_label (ACCOUNTS_NEW_PROTOCOL_TYPE_STRING_POP3);
	gtk_widget_show (menu);
	gtk_menu_append (GTK_MENU (GTK_OPTION_MENU (protocol_type)->menu), menu);
	gtk_option_menu_set_history (GTK_OPTION_MENU (protocol_type), 0);

	menu = gtk_menu_item_new_with_label (ACCOUNTS_NEW_SIGNATURE_TYPE_STRING_STATIC);
	gtk_widget_show (menu);
	gtk_menu_append (GTK_MENU (GTK_OPTION_MENU (signature_type)->menu), menu);
	menu = gtk_menu_item_new_with_label (ACCOUNTS_NEW_SIGNATURE_TYPE_STRING_DYNAMIC);
	gtk_widget_show (menu);
	gtk_menu_append (GTK_MENU (GTK_OPTION_MENU (signature_type)->menu), menu);
	gtk_option_menu_set_history (GTK_OPTION_MENU (signature_type), 0);
	
	gtk_signal_connect (GTK_OBJECT (druidpagestandard1), "next",
						GTK_SIGNAL_FUNC (on_accounts_new_druidpagestandard1_next), xml);
	gtk_signal_connect (GTK_OBJECT (druidpagestandard2), "next",
						GTK_SIGNAL_FUNC (on_accounts_new_druidpagestandard2_next), xml);
	gtk_signal_connect (GTK_OBJECT (druidpagefinish1), "finish",
						GTK_SIGNAL_FUNC (on_accounts_new_druidpagefinish1_finish), xml);
	gtk_signal_connect (GTK_OBJECT (smtp_remote_btn), "clicked",
						GTK_SIGNAL_FUNC (on_accounts_new_smtp_btn_clicked), xml);
	gtk_signal_connect (GTK_OBJECT (smtp_remote_information_btn), "clicked",
						GTK_SIGNAL_FUNC (on_accounts_new_smtp_remote_information_btn_clicked), xml);
	gtk_signal_connect (GTK_OBJECT (smtp_local), "clicked",
						GTK_SIGNAL_FUNC (on_accounts_new_smtp_btn_clicked), xml);
	gtk_widget_show (dialog);
}

static gboolean
on_accounts_edit_druidpagestandard1_next (GtkWidget *druid_page, GtkWidget *druid, GladeXML *xml)
{
	GtkWidget *w_name = glade_xml_get_widget (xml, "account_name");
	GtkWidget *w_email = glade_xml_get_widget (xml, "email");
	GtkWidget *w_clist = glade_xml_get_widget (preferences_xml, "accounts_clist");
	gchar *name = gtk_entry_get_text (GTK_ENTRY (w_name));
	gchar *email = gtk_entry_get_text (GTK_ENTRY (w_email));
	gint i;
	C2Account *account;
	
	/* (2) */
	if (!strlen (email))
	{
		GladeXML *tmp_xml = glade_xml_new (C2_APP_GLADE_FILE ("preferences"),
											"dlg_account_new_error_wrong_email");
		gnome_dialog_run_and_close (GNOME_DIALOG (glade_xml_get_widget (tmp_xml,
											"dlg_account_new_error_wrong_email")));
		return TRUE;
	}
	return FALSE;
}

static gboolean
on_accounts_edit_druidpagestandard2_next (GtkWidget *druid_page, GtkWidget *druid, GladeXML *xml)
{
	GtkWidget *protocol_type = glade_xml_get_widget (xml, "protocol_type");
	GtkWidget *pop3_server_addr = glade_xml_get_widget (xml, "pop3_server_addr");
	GtkWidget *pop3_username = glade_xml_get_widget (xml, "pop3_username");
	GtkWidget *smtp_remote_btn = glade_xml_get_widget (xml, "smtp_remote_btn");
	GtkWidget *smtp_remote_server_addr = glade_xml_get_widget (xml, "smtp_remote_server_addr");
	GtkWidget *smtp_remote_server_port = glade_xml_get_widget (xml, "smtp_remote_server_port");
	gchar *selection;
	C2AccountType account_type;

	/* Get the type of account */
	if (GTK_BIN (protocol_type)->child)
	{
		GtkWidget *child = GTK_BIN (protocol_type)->child;
		
		if (GTK_LABEL (child))
		{
			gtk_label_get (GTK_LABEL (child), &selection);
			if (c2_streq (selection, ACCOUNTS_NEW_PROTOCOL_TYPE_STRING_POP3))
				account_type = C2_ACCOUNT_POP3;
		}
	}

	switch (account_type)
	{
		case C2_ACCOUNT_POP3:
			/* Check that the user entered some host and username */
			if (!strlen (gtk_entry_get_text (GTK_ENTRY (pop3_server_addr))))
			{
				GladeXML *tmpxml = glade_xml_new (C2_APP_GLADE_FILE ("preferences"),
													"dlg_account_new_error_pop3_no_host");
				
				gnome_dialog_run_and_close (GNOME_DIALOG (glade_xml_get_widget (tmpxml,
													"dlg_account_new_error_pop3_no_host")));
				return TRUE;
			}

			if (!strlen (gtk_entry_get_text (GTK_ENTRY (pop3_username))))
			{
				GladeXML *tmpxml = glade_xml_new (C2_APP_GLADE_FILE ("preferences"),
													"dlg_account_new_error_pop3_no_user");
				
				gnome_dialog_run_and_close (GNOME_DIALOG (glade_xml_get_widget (tmpxml,
													"dlg_account_new_error_pop3_no_user")));
				return TRUE;
			}
			break;
	}

	if (GTK_TOGGLE_BUTTON (smtp_remote_btn)->active)
	{
		/* Check that the information of the remote SMTP server is complete */
		if (!strlen (gtk_entry_get_text (GTK_ENTRY (smtp_remote_server_addr))))
		{
			GladeXML *tmpxml = glade_xml_new (C2_APP_GLADE_FILE ("preferences"),
												"dlg_account_new_error_no_smtp_info");
			
			gnome_dialog_run_and_close (GNOME_DIALOG (glade_xml_get_widget (tmpxml,
												"dlg_account_new_error_no_smtp_info")));
			return TRUE;
		}
	}

	return FALSE;
}

static void
on_accounts_edit_druidpagefinish1_finish (GnomeDruidPage *druid_page, GtkWidget *druid, GladeXML *xml)
{
	gchar *account_name, *person_name, *organization, *email, *replyto, *pop3_host,
			*pop3_user, *pop3_pass, *smtp_host, *smtp_user, *smtp_pass,
			*signature;
	gint pop3_port, pop3_flags, smtp_port;
	gboolean active, smtp_authentication, keep_copy, signature_automatic;
	C2AccountType account_type;
	C2AccountSignatureType signature_type;
	C2SMTPType smtp_type;
	C2Account *account;
	gchar *selection;
	GtkWidget *buf;
	gint row;
	
	/* Collect the data */
	account_name = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (xml, "account_name")));
	person_name = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (xml, "person_name")));
	organization = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (xml, "organization")));
	email = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (xml, "email")));
	replyto = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (xml, "reply_to")));
	active = GTK_TOGGLE_BUTTON (glade_xml_get_widget (xml, "include_account"))->active;

	buf = glade_xml_get_widget (xml, "protocol_type");
	if (GTK_BIN (buf)->child)
	{
		GtkWidget *child = GTK_BIN (buf)->child;
		
		if (GTK_LABEL (child))
		{
			gtk_label_get (GTK_LABEL (child), &selection);
			if (c2_streq (selection, ACCOUNTS_NEW_PROTOCOL_TYPE_STRING_POP3))
				account_type = C2_ACCOUNT_POP3;
		}
	}

	if (account_type == C2_ACCOUNT_POP3)
	{
		pop3_host = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (xml, "pop3_server_addr")));
		pop3_port = gtk_spin_button_get_value_as_int (
								GTK_SPIN_BUTTON (glade_xml_get_widget (xml, "pop3_server_port")));
		pop3_user = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (xml, "pop3_username")));
		pop3_pass = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (xml, "pop3_password")));
	}
	
	if (GTK_TOGGLE_BUTTON (glade_xml_get_widget (xml, "smtp_remote_btn"))->active)
		smtp_type = C2_SMTP_REMOTE;
	else
		smtp_type = C2_SMTP_LOCAL;

	if (smtp_type == C2_SMTP_REMOTE)
	{
		smtp_host = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (xml, "smtp_remote_server_addr")));
		smtp_port = gtk_spin_button_get_value_as_int (
								GTK_SPIN_BUTTON (glade_xml_get_widget (xml, "smtp_remote_server_port")));
		smtp_authentication = GTK_TOGGLE_BUTTON (glade_xml_get_widget
										(xml, "smtp_remote_authentication"))->active;
		smtp_user = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (xml, "smtp_remote_username")));
		smtp_pass = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (xml, "smtp_remote_password")));
	}

	if (GTK_TOGGLE_BUTTON (glade_xml_get_widget (xml, "leave_messages"))->active)
		pop3_flags = C2_POP3_DO_KEEP_COPY;
	else
		pop3_flags = C2_POP3_DONT_KEEP_COPY;
	

	buf = glade_xml_get_widget (xml, "signature_type");
	if (GTK_BIN (buf)->child)
	{
		GtkWidget *child = GTK_BIN (buf)->child;
		
		if (GTK_LABEL (child))
		{
			gtk_label_get (GTK_LABEL (child), &selection);
			if (c2_streq (selection, ACCOUNTS_NEW_SIGNATURE_TYPE_STRING_STATIC))
				signature_type = C2_ACCOUNT_SIGNATURE_STATIC;
			else
				signature_type = C2_ACCOUNT_SIGNATURE_DYNAMIC;
		}
	}

	signature = gtk_editable_get_chars (GTK_EDITABLE (glade_xml_get_widget (xml, "signature_text")), 0, -1);
	signature_automatic = GTK_TOGGLE_BUTTON (glade_xml_get_widget (xml, "signature_automatic"))->active;

	buf = glade_xml_get_widget (preferences_xml, "accounts_clist");
	row = GPOINTER_TO_INT (GTK_CLIST (buf)->selection->data);
	account = C2_ACCOUNT (gtk_clist_get_row_data (GTK_CLIST (buf), row));
	gtk_object_unref (GTK_OBJECT (account));
	switch (smtp_type)
	{
		case C2_SMTP_REMOTE:
			account = c2_account_new (account_name, person_name, organization, email, replyto,
										active, account_type, smtp_type, signature_type, signature,
										signature_automatic, pop3_host, pop3_port, pop3_user, pop3_pass,
										pop3_flags,
										smtp_host, smtp_port, smtp_authentication, smtp_user, smtp_pass);
			break;
		case C2_SMTP_LOCAL:
			account = c2_account_new (account_name, person_name, organization, email, replyto,
										active, account_type, smtp_type, signature_type, signature,
										signature_automatic, pop3_host, pop3_port, pop3_user, pop3_pass,
										pop3_flags);
			break;
	}

	gtk_widget_destroy (glade_xml_get_widget (xml, "dlg_account_new"));

	gtk_clist_set_text (GTK_CLIST (buf), row, 0, account->name);
	gtk_clist_set_text (GTK_CLIST (buf), row, 1, account->per_name);
	gtk_clist_set_text (GTK_CLIST (buf), row, 1, account->email);
	gtk_clist_set_row_data (GTK_CLIST (buf), row, account);
}

static void
on_accounts_edit_btn_clicked (void)
{
	GladeXML *xml = glade_xml_new (C2_APP_GLADE_FILE ("preferences"), "dlg_account_new");
	GtkWidget *clist = glade_xml_get_widget (preferences_xml, "accounts_clist");
	C2Account *account = C2_ACCOUNT (gtk_clist_get_row_data (GTK_CLIST (clist),
								GPOINTER_TO_INT (GTK_CLIST (clist)->selection->data)));
	
	
	GtkWidget *dialog = glade_xml_get_widget (xml, "dlg_account_new");
	GtkWidget *druidpagestandard1 = glade_xml_get_widget (xml, "druidpagestandard1");
	GtkWidget *druidpagestandard2 = glade_xml_get_widget (xml, "druidpagestandard2");
	GtkWidget *druidpagefinish1 = glade_xml_get_widget (xml, "druidpagefinish1");
	GtkWidget *account_name = glade_xml_get_widget (xml, "account_name");
	GtkWidget *person_name = glade_xml_get_widget (xml, "person_name");
	GtkWidget *organization = glade_xml_get_widget (xml, "organization");
	GtkWidget *email = glade_xml_get_widget (xml, "email");
	GtkWidget *reply_to = glade_xml_get_widget (xml, "reply_to");
	GtkWidget *include_account = glade_xml_get_widget (xml, "include_account");
	GtkWidget *pop3_server_addr = glade_xml_get_widget (xml, "pop3_server_addr");
	GtkWidget *pop3_server_port = glade_xml_get_widget (xml, "pop3_server_port");
	GtkWidget *pop3_username = glade_xml_get_widget (xml, "pop3_username");
	GtkWidget *pop3_password = glade_xml_get_widget (xml, "pop3_password");
	GtkWidget *leave_messages = glade_xml_get_widget (xml, "leave_messages");
	GtkWidget *smtp_remote_btn = glade_xml_get_widget (xml, "smtp_remote_btn");
	GtkWidget *smtp_remote_server_addr = glade_xml_get_widget (xml, "smtp_remote_server_addr");
	GtkWidget *smtp_remote_server_port = glade_xml_get_widget (xml, "smtp_remote_server_port");
	GtkWidget *smtp_remote_authentication = glade_xml_get_widget (xml, "smtp_remote_authentication");
	GtkWidget *smtp_remote_username = glade_xml_get_widget (xml, "smtp_remote_username");
	GtkWidget *smtp_remote_password = glade_xml_get_widget (xml, "smtp_remote_password");
	GtkWidget *smtp_local_btn = glade_xml_get_widget (xml, "smtp_local_btn");
	GtkWidget *signature_text = glade_xml_get_widget (xml, "signature_text");
	GtkWidget *signature_automatic = glade_xml_get_widget (xml, "signature_automatic");
	GtkWidget *smtp_remote_information_btn = glade_xml_get_widget (xml, "smtp_remote_information_btn");
	GtkWidget *smtp_local = glade_xml_get_widget (xml, "smtp_local");
	GtkWidget *protocol_type = glade_xml_get_widget (xml, "protocol_type");
	GtkWidget *signature_type = glade_xml_get_widget (xml, "signature_type");
	GtkWidget *menu;

	menu = gtk_menu_item_new_with_label (ACCOUNTS_NEW_PROTOCOL_TYPE_STRING_POP3);
	gtk_widget_show (menu);
	gtk_menu_append (GTK_MENU (GTK_OPTION_MENU (protocol_type)->menu), menu);
	gtk_option_menu_set_history (GTK_OPTION_MENU (protocol_type), account->type);

	menu = gtk_menu_item_new_with_label (ACCOUNTS_NEW_SIGNATURE_TYPE_STRING_STATIC);
	gtk_widget_show (menu);
	gtk_menu_append (GTK_MENU (GTK_OPTION_MENU (signature_type)->menu), menu);
	menu = gtk_menu_item_new_with_label (ACCOUNTS_NEW_SIGNATURE_TYPE_STRING_DYNAMIC);
	gtk_widget_show (menu);
	gtk_menu_append (GTK_MENU (GTK_OPTION_MENU (signature_type)->menu), menu);
	gtk_option_menu_set_history (GTK_OPTION_MENU (signature_type), account->signature.type);
	
	gtk_signal_connect (GTK_OBJECT (druidpagestandard1), "next",
						GTK_SIGNAL_FUNC (on_accounts_edit_druidpagestandard1_next), xml);
	gtk_signal_connect (GTK_OBJECT (druidpagestandard2), "next",
						GTK_SIGNAL_FUNC (on_accounts_edit_druidpagestandard2_next), xml);
	gtk_signal_connect (GTK_OBJECT (druidpagefinish1), "finish",
						GTK_SIGNAL_FUNC (on_accounts_edit_druidpagefinish1_finish), xml);
	gtk_signal_connect (GTK_OBJECT (smtp_remote_btn), "clicked",
						GTK_SIGNAL_FUNC (on_accounts_new_smtp_btn_clicked), xml);
	gtk_signal_connect (GTK_OBJECT (smtp_remote_information_btn), "clicked",
						GTK_SIGNAL_FUNC (on_accounts_new_smtp_remote_information_btn_clicked), xml);
	gtk_signal_connect (GTK_OBJECT (smtp_local), "clicked",
						GTK_SIGNAL_FUNC (on_accounts_new_smtp_btn_clicked), xml);

	gtk_entry_set_editable (GTK_ENTRY (account_name), FALSE);
	gtk_entry_set_text (GTK_ENTRY (account_name), account->name);

	gtk_entry_set_text (GTK_ENTRY (person_name), account->per_name);
	gtk_entry_set_text (GTK_ENTRY (organization), account->organization);
	gtk_entry_set_text (GTK_ENTRY (email), account->email);
	gtk_entry_set_text (GTK_ENTRY (reply_to), account->reply_to);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (include_account), account->options.active);

	switch (account->type)
	{
		case C2_ACCOUNT_POP3:
			gtk_entry_set_text (GTK_ENTRY (pop3_server_addr), account->protocol.pop3->host);
			gtk_spin_button_set_value (GTK_SPIN_BUTTON (pop3_server_port), account->protocol.pop3->port);
			gtk_entry_set_text (GTK_ENTRY (pop3_username), account->protocol.pop3->user);
			gtk_entry_set_text (GTK_ENTRY (pop3_password), account->protocol.pop3->pass);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (leave_messages), account->protocol.pop3->flags &
											C2_POP3_DO_KEEP_COPY);
			break;
	}
	
	switch (account->smtp->type)
	{
		case C2_SMTP_REMOTE:
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (smtp_remote_btn), 1);
			gtk_entry_set_text (GTK_ENTRY (smtp_remote_server_addr), account->smtp->host);
			gtk_spin_button_set_value (GTK_SPIN_BUTTON (smtp_remote_server_port), account->smtp->port);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (smtp_remote_authentication),
									account->smtp->authentication);
			gtk_entry_set_text (GTK_ENTRY (smtp_remote_username), account->smtp->user);
			gtk_entry_set_text (GTK_ENTRY (smtp_remote_password), account->smtp->pass);
			break;
		case C2_SMTP_LOCAL:
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (smtp_local_btn), 1);
			break;
	}

	gtk_text_insert (GTK_TEXT (signature_text), NULL, NULL, NULL,
									account->signature.string, -1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (signature_automatic), account->signature.automatic);
	
	gtk_widget_show (dialog);
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
	GtkWidget *advanced_http_proxy_btn  =  glade_xml_get_widget (preferences_xml, "advanced_http_proxy_btn");
	GtkWidget *advanced_http_proxy_addr = glade_xml_get_widget (preferences_xml, "advanced_http_proxy_addr");
	GtkWidget *advanced_http_proxy_port = glade_xml_get_widget (preferences_xml, "advanced_http_proxy_port");

	gtk_widget_set_sensitive (advanced_http_proxy_addr, GTK_TOGGLE_BUTTON (advanced_http_proxy_btn)->active);
	gtk_widget_set_sensitive (advanced_http_proxy_port, GTK_TOGGLE_BUTTON (advanced_http_proxy_btn)->active);
}

static void
on_advanced_ftp_proxy_btn_toggled (void)
{
	GtkWidget *advanced_ftp_proxy_btn  =  glade_xml_get_widget (preferences_xml, "advanced_ftp_proxy_btn");
	GtkWidget *advanced_ftp_proxy_addr = glade_xml_get_widget (preferences_xml, "advanced_ftp_proxy_addr");
	GtkWidget *advanced_ftp_proxy_port = glade_xml_get_widget (preferences_xml, "advanced_ftp_proxy_port");

	gtk_widget_set_sensitive (advanced_ftp_proxy_addr, GTK_TOGGLE_BUTTON (advanced_ftp_proxy_btn)->active);
	gtk_widget_set_sensitive (advanced_ftp_proxy_port, GTK_TOGGLE_BUTTON (advanced_ftp_proxy_btn)->active);
}

static void
on_advanced_persistent_smtp_btn_toggled (void)
{
}

static void
on_ctree_tree_select_row (GtkCTree *ctree, GtkCTreeNode *node)
{
	gchar *name = (gchar *) gtk_ctree_node_get_row_data (ctree, node);
	GtkWidget *notebook = glade_xml_get_widget (preferences_xml, "notebook");
	gint page = 0;

	if (!name)
		return;

	if (c2_streq (name, "General"))
		page = PAGE_GENERAL;
	else if (c2_streq (name, "Options"))
		page = PAGE_OPTIONS;
	else if (c2_streq (name, "Paths"))
		page = PAGE_PATHS;
	else if (c2_streq (name, "Accounts"))
		page = PAGE_ACCOUNTS;
	else if (c2_streq (name, "Interface"))
		page = PAGE_INTERFACE;
	else if (c2_streq (name, "Fonts"))
		page = PAGE_FONTS;
	else if (c2_streq (name, "Colors"))
		page = PAGE_COLORS;
	else if (c2_streq (name, "Advanced"))
		page = PAGE_ADVANCED;
	
	gtk_notebook_set_page (GTK_NOTEBOOK (notebook), page);
}
