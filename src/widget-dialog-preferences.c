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
#include <glade/glade.h>

#include <libcronosII/pop3.h>
#include <libcronosII/imap.h>
#include <libcronosII/smtp.h>
#include <libcronosII/utils.h>

#include "widget-application.h"
#include "widget-dialog-preferences.h"
#include "widget-sidebar.h"

static void
class_init									(C2DialogPreferencesClass *klass);

static void
init										(C2DialogPreferences *preferences);

static void
set_signals									(C2DialogPreferences *preferences);

static void
set_values									(C2DialogPreferences *preferences);

static void
on_sidebar_subsection_selected				(C2Sidebar *sidebar, const gchar *section, const gchar *subsection,
											 C2DialogPreferences *preferences);

static void
on_general_options_start_check_toggled		(GtkWidget *widget, C2DialogPreferences *preferences);
	
static void
on_general_options_start_load_toggled		(GtkWidget *widget, C2DialogPreferences *preferences);
	
static void
on_general_options_exit_expunge_toggled		(GtkWidget *widget, C2DialogPreferences *preferences);
	
static gint
on_general_options_timeout_mark_focus_out_event	(GtkWidget *widget, GdkEventFocus *event,
											C2DialogPreferences *preferences);

static void
on_general_options_timeout_mark_activate	(GtkWidget *widget, C2DialogPreferences *preferences);
	
static gint
on_general_options_timeout_check_focus_out_event	(GtkWidget *widget, GdkEventFocus *event,
											C2DialogPreferences *preferences);

static void
on_general_options_timeout_check_activate	(GtkWidget *widget, C2DialogPreferences *preferences);
	
static void
on_general_options_incoming_warn_toggled	(GtkWidget *widget, C2DialogPreferences *preferences);

static gint
on_general_options_outgoing_wrap_focus_out_event	(GtkWidget *widget, GdkEventFocus *event,
											C2DialogPreferences *preferences);

static void
on_general_options_outgoing_wrap_activate	(GtkWidget *widget, C2DialogPreferences *preferences);

static void
on_general_options_outgoing_outbox_toggled	(GtkWidget *widget, C2DialogPreferences *preferences);

static void
on_general_accounts_clist_select_row		(GtkWidget *widget, gint row, gint column,
											 GdkEvent *event, C2DialogPreferences *preferences);

static void
on_general_accounts_clist_unselect_row		(GtkWidget *widget, gint row, gint column,
											 GdkEvent *event, C2DialogPreferences *preferences);

static void
general_accounts_update_list				(C2DialogPreferences *preferences);

static GtkWidget *
on_general_accounts_add_clicked				(GtkWidget *pwidget, C2DialogPreferences *preferences);

static void
on_general_accounts_remove_clicked			(GtkWidget *pwidget, C2DialogPreferences *preferences);

static void
on_general_accounts_up_clicked				(GtkWidget *pwidget, C2DialogPreferences *preferences);

static void
on_general_accounts_down_clicked			(GtkWidget *pwidget, C2DialogPreferences *preferences);

static void
on_general_paths_save_changed				(GtkWidget *widget, C2DialogPreferences *preferences);

static void
on_general_paths_get_changed				(GtkWidget *widget, C2DialogPreferences *preferences);

static void
on_interface_composer_editor_internal_toggled	(GtkWidget *widget, C2DialogPreferences *preferences);

static void
on_interface_composer_editor_external_toggled	(GtkWidget *widget, C2DialogPreferences *preferences);

static void
on_interface_composer_editor_external_cmnd_changed	(GtkWidget *widget, C2DialogPreferences *preferences);

static void
on_general_accounts_identity_name_changed		(GtkWidget *widget, C2Window *window);

static void
on_general_accounts_identity_email_changed		(GtkWidget *widget, C2Window *window);

static void
on_general_accounts_incoming_host_changed		(GtkWidget *widget, C2Window *window);

static void
on_general_accounts_incoming_user_changed		(GtkWidget *widget, C2Window *window);

static void
on_general_accounts_outgoing_host_changed		(GtkWidget *widget, C2Window *window);

static void
on_general_accounts_outgoing_user_changed		(GtkWidget *widget, C2Window *window);

static void
on_general_accounts_outgoing_auth_required_toggled (GtkWidget *button, C2Window *window);

static void
on_general_accounts_options_account_changed		(GtkWidget *widget, C2Window *window);

static void
on_general_accounts_incoming_protocol_selection_done	(GtkWidget *pwidget, C2Window *window);

static void
on_general_accounts_outgoing_server_protocol_selection_done	(GtkWidget *pwidget, C2Window *window);

static void
on_general_accounts_options_multiple_access_leave_toggled	(GtkWidget *button, C2Window *window);

static void
on_general_accounts_options_multiple_access_remove_toggled	(GtkWidget *button, C2Window *window);

static gboolean
on_general_accounts_druid_page0_next		(GnomeDruidPage *druid_page, GtkWidget *druid,
												C2Window *window);

static gint
general_accounts_get_next_account_number	(void);

static gboolean
on_general_accounts_druid_page1_next		(GnomeDruidPage *druid_page, GtkWidget *druid,
												C2Window *window);

static gboolean
on_general_accounts_druid_page2_next		(GnomeDruidPage *druid_page, GtkWidget *druid,
												C2Window *window);

static gboolean
on_general_accounts_druid_page3_next		(GnomeDruidPage *druid_page, GtkWidget *druid,
												C2Window *window);

static gboolean
on_general_accounts_druid_page4_next		(GnomeDruidPage *druid_page, GtkWidget *druid,
												C2Window *window);

static void
on_general_accounts_druid_page5_finish		(GnomeDruidPage *druid_page, GtkWidget *druid,
												C2Window *window);

enum
{
	CHANGED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static C2DialogClass *parent_class = NULL;

GtkType
c2_dialog_preferences_get_type (void)
{
	static GtkType type = 0;

	if (!type)
	{
		static GtkTypeInfo info =
		{
			"C2DialogPreferences",
			sizeof (C2DialogPreferences),
			sizeof (C2DialogPreferencesClass),
			(GtkClassInitFunc) class_init,
			(GtkObjectInitFunc) init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL
		};

		type = gtk_type_unique (c2_dialog_get_type (), &info);
	}

	return type;
}

static void
class_init (C2DialogPreferencesClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *) klass;

	parent_class = gtk_type_class (c2_dialog_get_type ());

	signals[CHANGED] =
		gtk_signal_new ("changed",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2DialogPreferencesClass, changed),
						gtk_marshal_NONE__INT_POINTER, GTK_TYPE_NONE, 2,
						GTK_TYPE_ENUM, GTK_TYPE_POINTER);
	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);

	klass->changed = NULL;
}

static void
init (C2DialogPreferences *preferences)
{
}

GtkWidget *
c2_dialog_preferences_new (C2Application *application)
{
	C2DialogPreferences *preferences;

	preferences = gtk_type_new (c2_dialog_preferences_get_type ());

	c2_dialog_preferences_construct (preferences, application);
	gtk_widget_set_usize (GTK_WIDGET (preferences), 670, 400);

	set_signals (preferences);
	set_values (preferences);

	return GTK_WIDGET (preferences);
}

static void
set_signals (C2DialogPreferences *preferences)
{
	GladeXML *xml;
	GtkWidget *widget;

	xml = C2_DIALOG (preferences)->xml;

	widget = glade_xml_get_widget (xml, "general_options_start_check");
	gtk_signal_connect (GTK_OBJECT (widget), "toggled",
						GTK_SIGNAL_FUNC (on_general_options_start_check_toggled), preferences);
	
	widget = glade_xml_get_widget (xml, "general_options_start_load");
	gtk_signal_connect (GTK_OBJECT (widget), "toggled",
						GTK_SIGNAL_FUNC (on_general_options_start_load_toggled), preferences);
	
	widget = glade_xml_get_widget (xml, "general_options_exit_expunge");
	gtk_signal_connect (GTK_OBJECT (widget), "toggled",
						GTK_SIGNAL_FUNC (on_general_options_exit_expunge_toggled), preferences);
	
	widget = glade_xml_get_widget (xml, "general_options_timeout_mark");
	gtk_signal_connect (GTK_OBJECT (widget), "focus_out_event",
						GTK_SIGNAL_FUNC (on_general_options_timeout_mark_focus_out_event), preferences);
	gtk_signal_connect (GTK_OBJECT (widget), "activate",
						GTK_SIGNAL_FUNC (on_general_options_timeout_mark_activate), preferences);
	
	widget = glade_xml_get_widget (xml, "general_options_timeout_check");
	gtk_signal_connect (GTK_OBJECT (widget), "focus_out_event",
						GTK_SIGNAL_FUNC (on_general_options_timeout_check_focus_out_event), preferences);
	gtk_signal_connect (GTK_OBJECT (widget), "activate",
						GTK_SIGNAL_FUNC (on_general_options_timeout_check_activate), preferences);
	
	widget = glade_xml_get_widget (xml, "general_options_incoming_warn");
	gtk_signal_connect (GTK_OBJECT (widget), "toggled",
						GTK_SIGNAL_FUNC (on_general_options_incoming_warn_toggled), preferences);

	widget = glade_xml_get_widget (xml, "general_options_outgoing_wrap");
	gtk_signal_connect (GTK_OBJECT (widget), "focus_out_event",
						GTK_SIGNAL_FUNC (on_general_options_outgoing_wrap_focus_out_event), preferences);
	gtk_signal_connect (GTK_OBJECT (widget), "activate",
						GTK_SIGNAL_FUNC (on_general_options_outgoing_wrap_activate), preferences);

	widget = glade_xml_get_widget (xml, "general_options_outgoing_outbox");
	gtk_signal_connect (GTK_OBJECT (widget), "toggled",
						GTK_SIGNAL_FUNC (on_general_options_outgoing_outbox_toggled), preferences);

	widget = glade_xml_get_widget (xml, "general_accounts_clist");
	gtk_signal_connect (GTK_OBJECT (widget), "select_row",
						GTK_SIGNAL_FUNC (on_general_accounts_clist_select_row), preferences);
	gtk_signal_connect (GTK_OBJECT (widget), "unselect_row",
						GTK_SIGNAL_FUNC (on_general_accounts_clist_unselect_row), preferences);

	widget = glade_xml_get_widget (xml, "general_accounts_add");
	gtk_signal_connect (GTK_OBJECT (widget), "clicked",
						GTK_SIGNAL_FUNC (on_general_accounts_add_clicked), preferences);

	widget = glade_xml_get_widget (xml, "general_accounts_remove");
	gtk_signal_connect (GTK_OBJECT (widget), "clicked",
						GTK_SIGNAL_FUNC (on_general_accounts_remove_clicked), preferences);

	widget = glade_xml_get_widget (xml, "general_accounts_up");
	gtk_signal_connect (GTK_OBJECT (widget), "clicked",
						GTK_SIGNAL_FUNC (on_general_accounts_up_clicked), preferences);

	widget = glade_xml_get_widget (xml, "general_accounts_down");
	gtk_signal_connect (GTK_OBJECT (widget), "clicked",
						GTK_SIGNAL_FUNC (on_general_accounts_down_clicked), preferences);

	widget = gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (glade_xml_get_widget (xml, "general_paths_save")));
	gtk_signal_connect (GTK_OBJECT (widget), "changed",
						GTK_SIGNAL_FUNC (on_general_paths_save_changed), preferences);

	widget = gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (glade_xml_get_widget (xml, "general_paths_get")));
	gtk_signal_connect (GTK_OBJECT (widget), "changed",
						GTK_SIGNAL_FUNC (on_general_paths_get_changed), preferences);

	widget = glade_xml_get_widget (xml, "interface_composer_editor_internal");
	gtk_signal_connect (GTK_OBJECT (widget), "toggled",
						GTK_SIGNAL_FUNC (on_interface_composer_editor_internal_toggled), preferences);

	widget = glade_xml_get_widget (xml, "interface_composer_editor_external");
	gtk_signal_connect (GTK_OBJECT (widget), "toggled",
						GTK_SIGNAL_FUNC (on_interface_composer_editor_external_toggled), preferences);

	widget = glade_xml_get_widget (xml, "interface_composer_editor_external_cmnd");
	gtk_signal_connect (GTK_OBJECT (widget), "changed",
						GTK_SIGNAL_FUNC (on_interface_composer_editor_external_cmnd_changed), preferences);
}

#define SET_BOOLEAN(section, subsection, key, value, wkey)	\
	boolv = gnome_config_get_bool_with_default \
				("/"PACKAGE"/"section"-"subsection"/"key"="value, NULL); \
	widgetv = glade_xml_get_widget (xml, wkey"_"key); \
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widgetv), boolv)
#define SET_FLOAT(section, subsection, key, value, wkey)	\
	floatv = gnome_config_get_float_with_default \
				("/"PACKAGE"/"section"-"subsection"/"key"="value, NULL); \
	widgetv = glade_xml_get_widget (xml, wkey"_"key); \
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (widgetv), floatv)
#define SET_INT(section, subsection, key, value, wkey)	\
	intv = gnome_config_get_float_with_default \
				("/"PACKAGE"/"section"-"subsection"/"key"="value, NULL); \
	widgetv = glade_xml_get_widget (xml, wkey"_"key); \
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (widgetv), intv)
#define SET_STRING(section, subsection, key, value, wkey)	\
	charv = gnome_config_get_string_with_default \
				("/"PACKAGE"/"section"-"subsection"/"key"="value, NULL); \
	widgetv = glade_xml_get_widget (xml, wkey"_"key); \
	gtk_entry_set_text (GTK_ENTRY (widgetv), charv); \
	g_free (charv)
#define SET_COLOR(section, subsection, key, red, green, blue, wkey)	\
	widgetv = glade_xml_get_widget (xml, wkey"_"key); \
	gnome_color_picker_set_i16 (GNOME_COLOR_PICKER (widgetv), \
		gnome_config_get_int_with_default ("/"PACKAGE"/"section"-"key".red="red, NULL), \
		gnome_config_get_int_with_default ("/"PACKAGE"/"section"-"key".green="green, NULL), \
		gnome_config_get_int_with_default ("/"PACKAGE"/"section"-"key".blue="blue, NULL), 0);\
	

static void
set_values_accounts (C2DialogPreferences *preferences)
{
	C2Account *account;
	GtkWidget *widgetv;
	GladeXML *xml;
	
	xml = C2_DIALOG (preferences)->xml;
	widgetv = glade_xml_get_widget (xml, "general_accounts_clist");
	gtk_clist_freeze (GTK_CLIST (widgetv));
	gtk_clist_clear (GTK_CLIST (widgetv));
	for (account = C2_DIALOG (preferences)->application->account; account; account = c2_account_next (account))
	{
		gchar *row[] =
		{
			NULL, NULL
		};

		if (account == C2_DIALOG (preferences)->application->account)
			row[0] = g_strdup_printf (_("%s (default)"), account->name);
		else
			row[0] = account->name;
		
		if (account->type == C2_ACCOUNT_POP3)
			row[1] = "POP3";
		else if (account->type == C2_ACCOUNT_IMAP)
			row[1] = "IMAP";

		gtk_clist_append (GTK_CLIST (widgetv), row);
		gtk_clist_set_row_data (GTK_CLIST (widgetv), GTK_CLIST (widgetv)->rows-1, account);
	}
	gtk_clist_thaw (GTK_CLIST (widgetv));
}

static void
set_values (C2DialogPreferences *preferences)
{
	gint intv;
	gfloat floatv;
	gboolean boolv;
	gchar *charv;
	GtkWidget *widgetv;
	GladeXML *xml;
	gchar *buf;

	xml = C2_DIALOG (preferences)->xml;
	
	SET_BOOLEAN ("General", "Options", "start_check", "false", "general_options");
	SET_BOOLEAN ("General", "Options", "start_load", "true", "general_options");
	SET_BOOLEAN ("General", "Options", "exit_expunge", "false", "general_options");
	SET_FLOAT ("General", "Options", "timeout_mark", "1.5", "general_options");	
	SET_INT ("General", "Options", "timeout_check", "20", "general_options");
	SET_BOOLEAN ("General", "Options", "incoming_warn", "false", "general_options");
	SET_INT ("General", "Options", "outgoing_wrap", "72", "general_options");
	SET_BOOLEAN ("General", "Options", "outgoing_outbox", "true", "general_options");

	widgetv = glade_xml_get_widget (xml, "general_accounts_clist");
	gtk_clist_set_column_auto_resize (GTK_CLIST (widgetv), 0, TRUE);
	set_values_accounts (preferences);
	
	buf = g_strdup_printf ("/"PACKAGE"/General-Paths/save=%s/", g_get_home_dir ());
	charv = gnome_config_get_string_with_default (buf, NULL);
	widgetv = glade_xml_get_widget (xml, "general_paths_save");
	gtk_entry_set_text (GTK_ENTRY (gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (widgetv))),
						charv);
	g_free (buf);
	g_free (charv);
	
	buf = g_strdup_printf ("/"PACKAGE"/General-Paths/get=%s/", g_get_home_dir ());
	charv = gnome_config_get_string_with_default (buf, NULL);
	widgetv = glade_xml_get_widget (xml, "general_paths_get");
	gtk_entry_set_text (GTK_ENTRY (gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (widgetv))),
						charv);
	g_free (buf);
	g_free (charv);
	
	SET_BOOLEAN ("General", "Paths", "smart", "true", "general_paths");


	
	charv = gnome_config_get_string_with_default ("/"PACKAGE"/Interface-Fonts/readed_mails="
							"-adobe-helvetica-medium-r-normal-*-*-120-*-*-p-*-iso8859-1", NULL);
	widgetv = glade_xml_get_widget (xml, "interface_fonts_readed_mails");
	gnome_font_picker_set_font_name (GNOME_FONT_PICKER (widgetv), charv);
	g_free (charv);
	
	charv = gnome_config_get_string_with_default ("/"PACKAGE"/Interface-Fonts/unreaded_mails="
							"-adobe-helvetica-bold-r-normal-*-*-120-*-*-p-*-iso8859-1", NULL);
	widgetv = glade_xml_get_widget (xml, "interface_fonts_unreaded_mails");
	gnome_font_picker_set_font_name (GNOME_FONT_PICKER (widgetv), charv);
	g_free (charv);
	
	charv = gnome_config_get_string_with_default ("/"PACKAGE"/Interface-Fonts/unreaded_mails="
							"-adobe-helvetica-bold-r-normal-*-*-120-*-*-p-*-iso8859-1", NULL);
	widgetv = glade_xml_get_widget (xml, "interface_fonts_unreaded_mailbox");
	gnome_font_picker_set_font_name (GNOME_FONT_PICKER (widgetv), charv);
	g_free (charv);


	
	charv = gnome_config_get_string_with_default ("/"PACKAGE"/Interface-HTML/images=full_download", NULL);
	if (c2_streq (charv, "full_download"))
		widgetv = glade_xml_get_widget (xml, "interface_html_images_full_download");
	else if (c2_streq (charv, "partial_download"))
		widgetv = glade_xml_get_widget (xml, "interface_html_images_partial_download");
	else if (c2_streq (charv, "user_download"))
		widgetv = glade_xml_get_widget (xml, "interface_html_images_user_download");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widgetv), TRUE);
	g_free (charv);
	
	SET_BOOLEAN ("Interface", "HTML", "images_confirm", "false", "interface_html");
	
	charv = gnome_config_get_string_with_default ("/"PACKAGE"/Interface-HTML/links=default", NULL);
	if (c2_streq (charv, "default"))
		widgetv = glade_xml_get_widget (xml, "interface_html_links_default");
	else if (c2_streq (charv, "internal"))
		widgetv = glade_xml_get_widget (xml, "interface_html_links_internal");
	else if (c2_streq (charv, "mailto"))
		widgetv = glade_xml_get_widget (xml, "interface_html_links_mailto");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widgetv), TRUE);
	g_free (charv);
	
	SET_BOOLEAN ("Interface", "HTML", "links_confirm", "false", "interface_html");
	SET_BOOLEAN ("Interface", "HTML", "interpret_text_plain", "false", "interface_html");


	SET_STRING ("Interface", "Composer", "quote_character", "> ", "interface_composer");
	SET_COLOR ("Interface", "Composer", "quote_color", "52428", "52428", "52428", "interface_composer");

	charv = gnome_config_get_string_with_default ("/"PACKAGE"/Interface-Composer/editor=internal", NULL);
	if (c2_streq (charv, "internal"))
		widgetv = glade_xml_get_widget (xml, "interface_composer_editor_internal");
	else if (c2_streq (charv, "external"))
		widgetv = glade_xml_get_widget (xml, "interface_composer_editor_external");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widgetv), TRUE);
	gtk_signal_emit_by_name (GTK_OBJECT (widgetv), "toggled");

	SET_STRING ("Interface", "Composer", "editor_external_cmnd", "gnome-edit", "interface_composer");


	
	SET_STRING ("Interface", "Misc", "title", "%M - %a %v", "interface_misc");
	SET_STRING ("Interface", "Misc", "date", "%d.%m.%Y %H:%M %z", "interface_misc");

	charv = gnome_config_get_string_with_default
				("/"PACKAGE"/Interface-Misc/attachments_default=text/html", NULL);
	widgetv = glade_xml_get_widget (xml, "interface_misc_attachments_default");
	if (c2_streq (charv, "text/plain"))
		gtk_option_menu_set_history (GTK_OPTION_MENU (widgetv), 0);
	else if (c2_streq (charv, "text/html"))
		gtk_option_menu_set_history (GTK_OPTION_MENU (widgetv), 1);


	SET_BOOLEAN ("Advanced", "Misc", "proxy_http", "false", "advanced_misc");
	SET_STRING ("Advanced", "Misc", "proxy_http_host", "", "advanced_misc");
	SET_INT ("Advanced", "Misc", "proxy_http_port", "8080", "advanced_misc");
	SET_BOOLEAN ("Advanced", "Misc", "proxy_ftp", "false", "advanced_misc");
	SET_STRING ("Advanced", "Misc", "proxy_ftp_host", "", "advanced_misc");
	SET_INT ("Advanced", "Misc", "proxy_ftp_port", "8080", "advanced_misc");
}

#define GENERAL_OPTIONS		"Options"
#define GENERAL_ACCOUNTS	"Accounts"
#define GENERAL_PATHS		"Paths"
#define GENERAL_PLUGINS		"Plugins"
#define INTERFACE_FONTS		"Fonts"
#define INTERFACE_HTML		"HTML"
#define INTERFACE_COMPOSER	"Composer"
#define INTERFACE_MISC		"Misc"
#define ADVANCED_MISC		"Misc"

void
c2_dialog_preferences_construct (C2DialogPreferences *preferences, C2Application *application)
{
	GladeXML *xml;
	const gchar *buttons[] =
	{
		GNOME_STOCK_BUTTON_HELP,
		GNOME_STOCK_BUTTON_CLOSE,
		NULL
	};
	
	static C2SidebarSubSection general_icons[] =
	{
		{ N_(GENERAL_OPTIONS), PKGDATADIR "/pixmaps/general_options.png" },
		{ N_(GENERAL_ACCOUNTS), PKGDATADIR "/pixmaps/general_accounts.png" },
		{ N_(GENERAL_PATHS), PKGDATADIR "/pixmaps/general_paths.png" },
		{ N_(GENERAL_PLUGINS),	PKGDATADIR "/pixmaps/general_plugins.png" },
		{ NULL, NULL }
	};

	static C2SidebarSubSection interface_icons[] =
	{
		{ N_(INTERFACE_FONTS), PKGDATADIR "/pixmaps/interface_fonts.png" },
		{ N_(INTERFACE_HTML), PKGDATADIR "/pixmaps/interface_html.png" },
		{ N_(INTERFACE_COMPOSER), PKGDATADIR "/pixmaps/interface_composer.png" },
		{ N_(INTERFACE_MISC),	PKGDATADIR "/pixmaps/interface_misc.png" },
		{ NULL, NULL }
	};

	static C2SidebarSubSection advanced_icons[] =
	{
		{ N_(ADVANCED_MISC), PKGDATADIR "/pixmaps/advanced_misc.png" },
		{ NULL, NULL }
	};

	static C2SidebarSection sidebar_info[] =
	{
		{ N_("General"), general_icons, NULL },
		{ N_("Interface"), interface_icons, NULL },
		{ N_("Advanced"), advanced_icons, NULL },
		{ NULL, NULL, NULL }
	};
	GtkWidget *sidebar, *widget, *container;
	GtkStyle *style;

	xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("preferences"), "dlg_preferences_contents");
	
	c2_dialog_construct (C2_DIALOG (preferences), application, _("Preferences"), "preferences", buttons);
	C2_DIALOG (preferences)->xml = xml;
	gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (preferences)->vbox), glade_xml_get_widget (xml,
							"dlg_preferences_contents"), TRUE, TRUE, 0);

	/* Construct the pages */
	widget = gnome_stock_pixmap_widget_at_size (GTK_WIDGET (preferences), GNOME_STOCK_PIXMAP_ADD, 18, 18);
	container = glade_xml_get_widget (xml, "general_accounts_add_pixmap");
	gtk_box_pack_start (GTK_BOX (container), widget, TRUE, TRUE, 0);
	gtk_widget_show (widget);

	widget = gnome_stock_pixmap_widget_at_size(GTK_WIDGET(preferences), GNOME_STOCK_PIXMAP_PROPERTIES, 18, 18);
	container = glade_xml_get_widget (xml, "general_accounts_edit_pixmap");
	gtk_box_pack_start (GTK_BOX (container), widget, TRUE, TRUE, 0);
	gtk_widget_show (widget);

	widget = gnome_stock_pixmap_widget_at_size (GTK_WIDGET (preferences), GNOME_STOCK_PIXMAP_REMOVE, 18, 18);
	container = glade_xml_get_widget (xml, "general_accounts_remove_pixmap");
	gtk_box_pack_start (GTK_BOX (container), widget, TRUE, TRUE, 0);
	gtk_widget_show (widget);

	widget = gnome_stock_pixmap_widget_at_size (GTK_WIDGET (preferences), GNOME_STOCK_PIXMAP_UP, 18, 18);
	container = glade_xml_get_widget (xml, "general_accounts_up_pixmap");
	gtk_box_pack_start (GTK_BOX (container), widget, TRUE, TRUE, 0);
	gtk_widget_show (widget);

	widget = gnome_stock_pixmap_widget_at_size (GTK_WIDGET (preferences), GNOME_STOCK_PIXMAP_DOWN, 18, 18);
	container = glade_xml_get_widget (xml, "general_accounts_down_pixmap");
	gtk_box_pack_start (GTK_BOX (container), widget, TRUE, TRUE, 0);
	gtk_widget_show (widget);

	widget = gnome_stock_pixmap_widget_at_size (GTK_WIDGET (preferences), GNOME_STOCK_PIXMAP_ADD, 18, 18);
	container = glade_xml_get_widget (xml, "general_plugins_add_pixmap");
	gtk_box_pack_start (GTK_BOX (container), widget, TRUE, TRUE, 0);
	gtk_widget_show (widget);

	widget = gnome_stock_pixmap_widget_at_size (GTK_WIDGET (preferences), GNOME_STOCK_PIXMAP_REMOVE, 18, 18);
	container = glade_xml_get_widget (xml, "general_plugins_remove_pixmap");
	gtk_box_pack_start (GTK_BOX (container), widget, TRUE, TRUE, 0);
	gtk_widget_show (widget);

	widget = gnome_stock_pixmap_widget_at_size (GTK_WIDGET (preferences), GNOME_STOCK_PIXMAP_PREFERENCES, 18, 18);
	container = glade_xml_get_widget (xml, "general_plugins_configure_pixmap");
	gtk_box_pack_start (GTK_BOX (container), widget, TRUE, TRUE, 0);
	gtk_widget_show (widget);

	/* Sidebar */
	sidebar = glade_xml_get_widget (xml, "sidebar");
	c2_sidebar_set_contents (C2_SIDEBAR (sidebar), sidebar_info);
	gtk_widget_show (sidebar);

	gtk_signal_connect (GTK_OBJECT (sidebar), "subsection_selected",
							GTK_SIGNAL_FUNC (on_sidebar_subsection_selected), preferences);
}

static void
on_sidebar_subsection_selected (C2Sidebar *sidebar, const gchar *section, const gchar *subsection,
								C2DialogPreferences *preferences)
{
	GtkNotebook *notebook = GTK_NOTEBOOK (glade_xml_get_widget (C2_DIALOG (preferences)->xml, "notebook"));
	gint page = 0;
	
	if (c2_streq (section, _("General")))
	{
		if (c2_streq (subsection, _(GENERAL_OPTIONS)))
			page = C2_DIALOG_PREFERENCES_SUBSECTION_GENERAL_OPTIONS;
		else if (c2_streq (subsection, _(GENERAL_ACCOUNTS)))
			page = C2_DIALOG_PREFERENCES_SUBSECTION_GENERAL_ACCOUNTS;
		else if (c2_streq (subsection, _(GENERAL_PATHS)))
			page = C2_DIALOG_PREFERENCES_SUBSECTION_GENERAL_PATHS;
		else if (c2_streq (subsection, _(GENERAL_PLUGINS)))
			page = C2_DIALOG_PREFERENCES_SUBSECTION_GENERAL_PLUGINS;
	} else if (c2_streq (section, _("Interface")))
	{
		if (c2_streq (subsection, _(INTERFACE_FONTS)))
			page = C2_DIALOG_PREFERENCES_SUBSECTION_INTERFACE_FONTS;
		else if (c2_streq (subsection, _(INTERFACE_HTML)))
			page = C2_DIALOG_PREFERENCES_SUBSECTION_INTERFACE_HTML;
		else if (c2_streq (subsection, _(INTERFACE_COMPOSER)))
			page = C2_DIALOG_PREFERENCES_SUBSECTION_INTERFACE_COMPOSER;
		else if (c2_streq (subsection, _(INTERFACE_MISC)))
			page = C2_DIALOG_PREFERENCES_SUBSECTION_INTERFACE_MISC;
	} else if (c2_streq (section, _("Advanced")))
	{
		if (c2_streq (subsection, _(ADVANCED_MISC)))
			page = C2_DIALOG_PREFERENCES_SUBSECTION_ADVANCED_MISC;
	}

	gtk_notebook_set_page (notebook, page);
}

static void
on_general_options_start_check_toggled (GtkWidget *widget, C2DialogPreferences *preferences)
{
	gboolean bool = GTK_TOGGLE_BUTTON (widget)->active;
	
	gtk_signal_emit (GTK_OBJECT (preferences), signals[CHANGED],
						C2_DIALOG_PREFERENCES_KEY_GENERAL_OPTIONS_START_CHECK, bool);
	gnome_config_set_bool ("/"PACKAGE"/General-Options/start_check", bool);
}
	
static void
on_general_options_start_load_toggled (GtkWidget *widget, C2DialogPreferences *preferences)
{
	gboolean bool = GTK_TOGGLE_BUTTON (widget)->active;

	gtk_signal_emit (GTK_OBJECT (preferences), signals[CHANGED],
						C2_DIALOG_PREFERENCES_KEY_GENERAL_OPTIONS_START_LOAD, bool);
	gnome_config_set_bool ("/"PACKAGE"/General-Options/start_load", bool);
}
	
static void
on_general_options_exit_expunge_toggled (GtkWidget *widget, C2DialogPreferences *preferences)
{
	gboolean bool = GTK_TOGGLE_BUTTON (widget)->active;
	
	gtk_signal_emit (GTK_OBJECT (preferences), signals[CHANGED],
						C2_DIALOG_PREFERENCES_KEY_GENERAL_OPTIONS_EXIT_EXPUNGE, bool);
	gnome_config_set_bool ("/"PACKAGE"/General-Options/exit_expunge", bool);
}
	
static gint
on_general_options_timeout_mark_focus_out_event (GtkWidget *widget, GdkEventFocus *event,
											C2DialogPreferences *preferences)
{
	gfloat value = gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (widget));
	
	gtk_signal_emit (GTK_OBJECT (preferences), signals[CHANGED],
						C2_DIALOG_PREFERENCES_KEY_GENERAL_OPTIONS_TIMEOUT_MARK, value);
	gnome_config_set_float ("/"PACKAGE"/General-Options/timeout_mark", value);
	return FALSE;
}

static void
on_general_options_timeout_mark_activate (GtkWidget *widget, C2DialogPreferences *preferences)
{
	gfloat value = gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (widget));
	
	gtk_signal_emit (GTK_OBJECT (preferences), signals[CHANGED],
						C2_DIALOG_PREFERENCES_KEY_GENERAL_OPTIONS_TIMEOUT_MARK, value);
	gnome_config_set_float ("/"PACKAGE"/General-Options/timeout_mark", value);
}
	
static gint
on_general_options_timeout_check_focus_out_event (GtkWidget *widget, GdkEventFocus *event,
											C2DialogPreferences *preferences)
{
	gint value = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (widget));
	gtk_signal_emit (GTK_OBJECT (preferences), signals[CHANGED],
						C2_DIALOG_PREFERENCES_KEY_GENERAL_OPTIONS_TIMEOUT_CHECK, value);
	gnome_config_set_int ("/"PACKAGE"/General-Options/timeout_check", value);
	return FALSE;
}

static void
on_general_options_timeout_check_activate (GtkWidget *widget, C2DialogPreferences *preferences)
{
	gint value = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (widget));
	
	gtk_signal_emit (GTK_OBJECT (preferences), signals[CHANGED],
						C2_DIALOG_PREFERENCES_KEY_GENERAL_OPTIONS_TIMEOUT_CHECK, value);
	gnome_config_set_int ("/"PACKAGE"/General-Options/timeout_check", value);
}
	
static void
on_general_options_incoming_warn_toggled (GtkWidget *widget, C2DialogPreferences *preferences)
{
	gboolean bool = GTK_TOGGLE_BUTTON (widget)->active;
	
	gtk_signal_emit (GTK_OBJECT (preferences), signals[CHANGED],
						C2_DIALOG_PREFERENCES_KEY_GENERAL_OPTIONS_INCOMING_WARN, bool);
	gnome_config_set_bool ("/"PACKAGE"/General-Options/incoming_warn", bool);
}

static gint
on_general_options_outgoing_wrap_focus_out_event (GtkWidget *widget, GdkEventFocus *event,
											C2DialogPreferences *preferences)
{
	gint value = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (widget));
	
	gtk_signal_emit (GTK_OBJECT (preferences), signals[CHANGED],
						C2_DIALOG_PREFERENCES_KEY_GENERAL_OPTIONS_OUTGOING_WRAP, value);
	gnome_config_set_int ("/"PACKAGE"/General-Options/outgoing_wrap", value);
	return FALSE;
}

static void
on_general_options_outgoing_wrap_activate (GtkWidget *widget, C2DialogPreferences *preferences)
{
	gint value = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (widget));
	
	gtk_signal_emit (GTK_OBJECT (preferences), signals[CHANGED],
						C2_DIALOG_PREFERENCES_KEY_GENERAL_OPTIONS_OUTGOING_WRAP, value);
	gnome_config_set_int ("/"PACKAGE"/General-Options/outgoing_wrap", value);
}

static void
on_general_options_outgoing_outbox_toggled (GtkWidget *widget, C2DialogPreferences *preferences)
{
	gboolean bool = GTK_TOGGLE_BUTTON (widget)->active;
	
	gtk_signal_emit (GTK_OBJECT (preferences), signals[CHANGED],
						C2_DIALOG_PREFERENCES_KEY_GENERAL_OPTIONS_OUTGOING_OUTBOX, bool);
	gnome_config_set_bool ("/"PACKAGE"/General-Options/outgoing_outbox", bool);
}

static void
on_general_accounts_clist_select_row (GtkWidget *widget, gint row, gint column,
GdkEvent *event, C2DialogPreferences *preferences)
{
	GladeXML *xml;
	GtkWidget *edit, *remove, *up, *down;

	xml = C2_DIALOG (preferences)->xml;
	edit = glade_xml_get_widget (xml, "general_accounts_edit");
	remove = glade_xml_get_widget (xml, "general_accounts_remove");
	up = glade_xml_get_widget (xml, "general_accounts_up");
	down = glade_xml_get_widget (xml, "general_accounts_down");

	gtk_widget_set_sensitive (edit, TRUE);
	gtk_widget_set_sensitive (remove, TRUE);
	if (row == 0)
		gtk_widget_set_sensitive (up, FALSE);
	else
		gtk_widget_set_sensitive (up, TRUE);

	if (row+1 == GTK_CLIST (widget)->rows)
		gtk_widget_set_sensitive (down, FALSE);
	else
		gtk_widget_set_sensitive (down, TRUE);
}

static void
on_general_accounts_clist_unselect_row (GtkWidget *widget, gint row, gint column,
GdkEvent *event, C2DialogPreferences *preferences)
{
	GladeXML *xml;
	GtkWidget *edit, *remove, *up, *down;

	xml = C2_DIALOG (preferences)->xml;
	edit = glade_xml_get_widget (xml, "general_accounts_edit");
	remove = glade_xml_get_widget (xml, "general_accounts_remove");
	up = glade_xml_get_widget (xml, "general_accounts_up");
	down = glade_xml_get_widget (xml, "general_accounts_down");

	gtk_widget_set_sensitive (edit, FALSE);
	gtk_widget_set_sensitive (remove, FALSE);
	gtk_widget_set_sensitive (up, FALSE);
	gtk_widget_set_sensitive (down, FALSE);
}

static void
general_accounts_update_list (C2DialogPreferences *preferences)
{
	C2Application *application = C2_DIALOG (preferences)->application;
	C2Account *account;
	GtkWidget *clist;
	gint i;

	clist = glade_xml_get_widget (C2_DIALOG (preferences)->xml, "general_accounts_clist");

	application->account = NULL;
	for (i = 0; i < GTK_CLIST (clist)->rows; i++)
	{
		gpointer data = gtk_clist_get_row_data (GTK_CLIST (clist), i);

		if (!data)
			continue;

		account = C2_ACCOUNT (data);
		account->next = NULL;
		application->account = c2_account_append (application->account, account);
	}

	set_values_accounts (preferences);
}

#define general_accounts_update_conf_string(num,str)	\
	charv = (gchar*) c2_account_get_extra_data (account, num, NULL);	\
	gnome_config_set_string (str, charv);
#define general_accounts_update_conf_bool(num,str)	\
	boolv = (gboolean) c2_account_get_extra_data (account, num, NULL);	\
	gnome_config_set_bool (str, boolv);

static void
general_accounts_update_conf (C2DialogPreferences *preferences)
{
	C2Application *application;
	C2Account *account;
	C2AccountType type;
	C2SMTP *smtp;
	gchar *charv, *buf;
	gboolean boolv;
	gint i;

	for (i = 1;; i++)
	{
		buf = g_strdup_printf ("/"PACKAGE"/Account %d", i);
		if (!gnome_config_has_section (buf))
		{
			g_free (buf);
			break;
		}
		gnome_config_clean_section (buf);
		g_free (buf);
	}

	application = C2_DIALOG (preferences)->application;
	for (account = application->account, i = 1; account; account = c2_account_next (account), i++)
	{
		buf = g_strdup_printf ("/"PACKAGE"/Account %d/", i);
		gnome_config_push_prefix (buf);

		gnome_config_set_int ("type", account->type);
		gnome_config_set_string ("account_name", account->name);
		gnome_config_set_string ("identity_email", account->email);

		general_accounts_update_conf_string (C2_ACCOUNT_KEY_FULL_NAME, "identity_name");
		general_accounts_update_conf_string (C2_ACCOUNT_KEY_ORGANIZATION, "identity_organization");
		general_accounts_update_conf_string (C2_ACCOUNT_KEY_REPLY_TO, "identity_reply_to");
		
		if (account->type == C2_ACCOUNT_POP3)
		{
			C2POP3 *pop3;
			
			pop3 = C2_POP3 (c2_account_get_extra_data (account, C2_ACCOUNT_KEY_INCOMING, NULL));
			
			if (!pop3)
				goto skip_pop3;

			gnome_config_set_string ("incoming_server_hostname", C2_NET_OBJECT (pop3)->host);
			gnome_config_set_int ("incoming_server_port", C2_NET_OBJECT (pop3)->port);
			gnome_config_set_string ("incoming_server_username", pop3->user);
			gnome_config_set_string ("incoming_server_password", pop3->pass);
			gnome_config_set_bool ("incoming_server_ssl", C2_NET_OBJECT (pop3)->ssl);
			gnome_config_set_int ("incoming_auth_method", pop3->auth_method);
			if (pop3->flags & C2_POP3_DO_NOT_LOSE_PASSWORD)
				gnome_config_set_bool ("incoming_auth_remember", TRUE);
			else
				gnome_config_set_bool ("incoming_auth_remember", FALSE);
			if (pop3->flags & C2_POP3_DO_NOT_KEEP_COPY)
				gnome_config_set_bool ("options_multiple_access_leave", TRUE);
			else
				gnome_config_set_bool ("options_multiple_access_leave", FALSE);
			if (pop3->copies_in_server_life_time)
				gnome_config_set_bool ("options_multiple_access_remove", TRUE);
			else
				gnome_config_set_bool ("options_multiple_access_remove", FALSE);
			gnome_config_set_int ("options_multiple_access_remove", pop3->copies_in_server_life_time);
skip_pop3:
		} else if (type == C2_ACCOUNT_IMAP)
		{
			C2IMAP *imap;
			
			imap = C2_IMAP (c2_account_get_extra_data (account, C2_ACCOUNT_KEY_INCOMING, NULL));
			
			if (!imap)
				goto skip_imap;

			gnome_config_set_string ("incoming_server_hostname", C2_NET_OBJECT (imap)->host);
			gnome_config_set_int ("incoming_server_port", C2_NET_OBJECT (imap)->port);
			gnome_config_set_string ("incoming_server_username", imap->user);
			gnome_config_set_string ("incoming_server_password", imap->pass);
			gnome_config_set_bool ("incoming_server_ssl", C2_NET_OBJECT (imap)->ssl);
			gnome_config_set_bool ("incoming_auth_remember", imap->auth_remember);
skip_imap:
		}
		
		smtp = (C2SMTP*) c2_account_get_extra_data (account, C2_ACCOUNT_KEY_OUTGOING, NULL);
		if (!smtp)
			goto skip_smtp;
		
		gnome_config_set_int ("outgoing_server_protocol", smtp->type);
		gnome_config_set_string ("outgoing_server_hostname", smtp->host);
		gnome_config_set_int ("outgoing_server_port", smtp->port);
		gnome_config_set_bool ("outgoing_server_ssl", smtp->ssl);
		gnome_config_set_bool ("outgoing_auth_required", smtp->authentication);
		gnome_config_set_string ("outgoing_server_username", smtp->user);
		gnome_config_set_string ("outgoing_server_password", smtp->pass);
		if (smtp->flags & C2_SMTP_DO_NOT_LOSE_PASSWORD)
			gnome_config_set_bool ("outgoing_auth_remember", TRUE);
		else
			gnome_config_set_bool ("outgoing_auth_remember", FALSE);
skip_smtp:
		general_accounts_update_conf_string (C2_ACCOUNT_KEY_SIGNATURE_PLAIN, "options_signature_plain");
		general_accounts_update_conf_string (C2_ACCOUNT_KEY_SIGNATURE_HTML, "options_signature_html");
		general_accounts_update_conf_bool (C2_ACCOUNT_KEY_ACTIVE, "options_auto_check");

		gnome_config_pop_prefix ();
		g_free (buf);
	}
	
	gnome_config_sync ();
}

static GtkWidget *
on_general_accounts_add_clicked (GtkWidget *pwidget, C2DialogPreferences *preferences)
{
	/* This function returns the window so it can be used as a construct function */
	GtkWidget *window, *widget;
	GladeXML *xml;
	gchar *buf, *organization;
	C2Account *account;

	xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("preferences"), "dlg_account_editor_contents");
	account = C2_DIALOG (preferences)->application->account;
	window = c2_window_new (C2_DIALOG (preferences)->application, _("Account Editor"), "account_editor");
	C2_WINDOW (window)->xml = xml;
	c2_window_set_contents_from_glade (C2_WINDOW (window), "dlg_account_editor_contents");
	gtk_object_set_data (GTK_OBJECT (window), "preferences", preferences);

	widget = glade_xml_get_widget (xml, "identity_name");
	if ((buf = g_get_real_name ()))
		gtk_entry_set_text (GTK_ENTRY (widget), buf);

	organization = (gchar *) c2_account_get_extra_data (account, C2_ACCOUNT_KEY_ORGANIZATION, NULL);
	if (account && organization)
	{
		widget = glade_xml_get_widget (xml, "identity_organization");
		gtk_entry_set_text (GTK_ENTRY (widget), organization);
	}

#ifndef USE_SSL
	widget = glade_xml_get_widget (xml, "incoming_server_ssl");
	gtk_widget_set_sensitive (widget, FALSE);
#endif

	if (account)
	{
		C2SMTP *smtp = (C2SMTP *) c2_account_get_extra_data (account, C2_ACCOUNT_KEY_OUTGOING, NULL);
		
		widget = glade_xml_get_widget (xml, "outgoing_server_protocol");
		gtk_option_menu_set_history (GTK_OPTION_MENU (widget), smtp->type);

		if (smtp->type == C2_SMTP_REMOTE)
		{
			widget = glade_xml_get_widget (xml, "outgoing_server_hostname");
			gtk_entry_set_text (GTK_ENTRY (widget), smtp->host);
			widget = glade_xml_get_widget (xml, "outgoing_server_port");
			gtk_spin_button_set_value (GTK_SPIN_BUTTON (widget), smtp->port);
		}
	}

#ifndef USE_SSL
	widget = glade_xml_get_widget (xml, "outgoing_server_ssl");
	gtk_widget_set_sensitive (widget, FALSE);
#endif

	widget = glade_xml_get_widget (xml, "options_signature_plain");
	buf = gnome_config_get_string ("/"PACKAGE"/General-Paths/get");
	gnome_file_entry_set_default_path (GNOME_FILE_ENTRY (widget), buf);
	g_free (buf);

	widget = glade_xml_get_widget (xml, "identity_name");
	gtk_signal_connect (GTK_OBJECT (widget), "changed",
						GTK_SIGNAL_FUNC (on_general_accounts_identity_name_changed), window);

	widget = glade_xml_get_widget (xml, "identity_email");
	gtk_signal_connect (GTK_OBJECT (widget), "changed",
						GTK_SIGNAL_FUNC (on_general_accounts_identity_email_changed), window);

	widget = glade_xml_get_widget (xml, "incoming_server_hostname");
	gtk_signal_connect (GTK_OBJECT (widget), "changed",
						GTK_SIGNAL_FUNC (on_general_accounts_incoming_host_changed), window);

	widget = glade_xml_get_widget (xml, "incoming_server_username");
	gtk_signal_connect (GTK_OBJECT (widget), "changed",
						GTK_SIGNAL_FUNC (on_general_accounts_incoming_user_changed), window);

	widget = GTK_OPTION_MENU (glade_xml_get_widget (xml, "incoming_protocol"))->menu;
	gtk_signal_connect (GTK_OBJECT (widget), "selection_done",
						GTK_SIGNAL_FUNC (on_general_accounts_incoming_protocol_selection_done), window);
	gtk_signal_emit_by_name (GTK_OBJECT (widget), "selection_done");

	widget = glade_xml_get_widget (xml, "outgoing_server_hostname");
	gtk_signal_connect (GTK_OBJECT (widget), "changed",
						GTK_SIGNAL_FUNC (on_general_accounts_outgoing_host_changed), window);

	widget = glade_xml_get_widget (xml, "outgoing_server_username");
	gtk_signal_connect (GTK_OBJECT (widget), "changed",
						GTK_SIGNAL_FUNC (on_general_accounts_outgoing_user_changed), window);

	widget = GTK_OPTION_MENU (glade_xml_get_widget (xml, "outgoing_server_protocol"))->menu;
	gtk_signal_connect (GTK_OBJECT (widget), "selection_done",
						GTK_SIGNAL_FUNC (on_general_accounts_outgoing_server_protocol_selection_done), window);
	gtk_signal_emit_by_name (GTK_OBJECT (widget), "selection_done");

	widget = glade_xml_get_widget (xml, "outgoing_auth_required");
	gtk_signal_connect (GTK_OBJECT (widget), "toggled",
						GTK_SIGNAL_FUNC (on_general_accounts_outgoing_auth_required_toggled), window);
	gtk_signal_emit_by_name (GTK_OBJECT (widget), "toggled");

	widget = glade_xml_get_widget (xml, "options_account_name");
	gtk_signal_connect (GTK_OBJECT (widget), "changed",
						GTK_SIGNAL_FUNC (on_general_accounts_options_account_changed), window);

	widget = glade_xml_get_widget (xml, "options_multiple_access_leave");
	gtk_signal_connect (GTK_OBJECT (widget), "toggled",
						GTK_SIGNAL_FUNC (on_general_accounts_options_multiple_access_leave_toggled), window);

	widget = glade_xml_get_widget (xml, "options_multiple_access_remove");
	gtk_signal_connect (GTK_OBJECT (widget), "toggled",
						GTK_SIGNAL_FUNC (on_general_accounts_options_multiple_access_remove_toggled), window);

	widget = glade_xml_get_widget (xml, "page0");
	gtk_signal_connect (GTK_OBJECT (widget), "next",
						GTK_SIGNAL_FUNC (on_general_accounts_druid_page0_next), window);

	widget = glade_xml_get_widget (xml, "page1");
	gtk_signal_connect (GTK_OBJECT (widget), "next",
						GTK_SIGNAL_FUNC (on_general_accounts_druid_page1_next), window);

	widget = glade_xml_get_widget (xml, "page2");
	gtk_signal_connect (GTK_OBJECT (widget), "next",
						GTK_SIGNAL_FUNC (on_general_accounts_druid_page2_next), window);

	widget = glade_xml_get_widget (xml, "page3");
	gtk_signal_connect (GTK_OBJECT (widget), "next",
						GTK_SIGNAL_FUNC (on_general_accounts_druid_page3_next), window);

	widget = glade_xml_get_widget (xml, "page5");
	gtk_signal_connect (GTK_OBJECT (widget), "finish",
						GTK_SIGNAL_FUNC (on_general_accounts_druid_page5_finish), window);


	widget = GNOME_DRUID (glade_xml_get_widget (xml, "dlg_account_editor_contents"))->next;
	gtk_widget_set_sensitive (widget, TRUE);
	gtk_widget_show (window);
	
	return window;
}

static void
on_general_paths_save_changed (GtkWidget *widget, C2DialogPreferences *preferences)
{
	gchar *value = gtk_entry_get_text (GTK_ENTRY (widget));

	gnome_config_set_string ("/"PACKAGE"/General-Paths/save", value);
	gnome_config_sync ();
	gtk_signal_emit (GTK_OBJECT (preferences), signals[CHANGED],
					C2_DIALOG_PREFERENCES_KEY_GENERAL_PATHS_SAVE, value);
}

static void
on_general_paths_get_changed (GtkWidget *widget, C2DialogPreferences *preferences)
{
	gchar *value = gtk_entry_get_text (GTK_ENTRY (widget));

	gnome_config_set_string ("/"PACKAGE"/General-Paths/get", value);
	gnome_config_sync ();
	gtk_signal_emit (GTK_OBJECT (preferences), signals[CHANGED],
					C2_DIALOG_PREFERENCES_KEY_GENERAL_PATHS_GET, value);
}


static void
on_interface_composer_editor_internal_toggled (GtkWidget *widget, C2DialogPreferences *preferences)
{
	GladeXML *xml = C2_DIALOG (preferences)->xml;
	GtkWidget *combo = glade_xml_get_widget (xml, "interface_composer_editor_combo");

	if (!GTK_TOGGLE_BUTTON (widget)->active)
		gtk_widget_set_sensitive (combo, TRUE);
	else
		gtk_widget_set_sensitive (combo, FALSE);

	gnome_config_set_string ("/"PACKAGE"/Interface-Composer/editor", "internal");
	gnome_config_sync ();
	gtk_signal_emit (GTK_OBJECT (preferences), signals[CHANGED],
					C2_DIALOG_PREFERENCES_KEY_INTERFACE_COMPOSER_EDITOR, "internal");
}

static void
on_interface_composer_editor_external_toggled (GtkWidget *widget, C2DialogPreferences *preferences)
{
	GladeXML *xml = C2_DIALOG (preferences)->xml;
	GtkWidget *combo = glade_xml_get_widget (xml, "interface_composer_editor_combo");

	if (GTK_TOGGLE_BUTTON (widget)->active)
		gtk_widget_set_sensitive (combo, TRUE);
	else
		gtk_widget_set_sensitive (combo, FALSE);
	gnome_config_set_string ("/"PACKAGE"/Interface-Composer/editor", "external");
	gnome_config_sync ();
	gtk_signal_emit (GTK_OBJECT (preferences), signals[CHANGED],
					C2_DIALOG_PREFERENCES_KEY_INTERFACE_COMPOSER_EDITOR, "external");
}

static void
on_interface_composer_editor_external_cmnd_changed (GtkWidget *widget, C2DialogPreferences *preferences)
{
	gchar *cmnd = gtk_entry_get_text (GTK_ENTRY (widget));

	gnome_config_set_string ("/"PACKAGE"/Interface-Composer/editor_external_cmnd", cmnd);
	gnome_config_sync ();
	gtk_signal_emit (GTK_OBJECT (preferences), signals[CHANGED],
					C2_DIALOG_PREFERENCES_KEY_INTERFACE_COMPOSER_EDITOR_CMND, cmnd);
}

static void
process_page_identity (GladeXML *xml)
{
	GtkWidget *next, *widget;
	gboolean sensitive = TRUE;
	gchar *buf, *buf2;

	next = GNOME_DRUID (glade_xml_get_widget (xml, "dlg_account_editor_contents"))->next;

	widget = glade_xml_get_widget (xml, "identity_name");
	if (!strlen (gtk_entry_get_text (GTK_ENTRY (widget))))
	{
		sensitive = FALSE;
		goto identity_set_sensitive;
	}

	widget = glade_xml_get_widget (xml, "identity_email");
	buf = gtk_entry_get_text (GTK_ENTRY (widget));
	if (!strlen (buf) || !(buf2 = strstr (buf, "@")) || strlen (buf2) == 1)
	{
		sensitive = FALSE;
		goto identity_set_sensitive;
	}

identity_set_sensitive:
	gtk_widget_set_sensitive (next, sensitive);
}

static void
on_general_accounts_identity_name_changed (GtkWidget *widget, C2Window *window)
{
	process_page_identity (window->xml);
}

static void
on_general_accounts_identity_email_changed (GtkWidget *widget, C2Window *window)
{
	GtkWidget *incoming_host, *incoming_user;
	gchar *buf, *buf2, *ptr;
	gboolean edit_host = TRUE;
	gboolean edit_user = TRUE;

	incoming_host = glade_xml_get_widget (window->xml, "incoming_server_hostname");
	incoming_user = glade_xml_get_widget (window->xml, "incoming_server_username");

	buf = gtk_object_get_data (GTK_OBJECT (incoming_host), "changed");
	if (buf)
		edit_host = FALSE;
	buf = gtk_object_get_data (GTK_OBJECT (incoming_user), "changed");
	if (buf)
		edit_user = FALSE;
	
	buf = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (window->xml, "identity_email")));
	ptr = strstr (buf, "@");
	if (edit_user)
	{
		if (ptr)
			buf2 = g_strndup (buf, ptr-buf);
		else
			buf2 = g_strdup (buf);
		gtk_signal_handler_block_by_data (GTK_OBJECT (incoming_user), window);
		gtk_entry_set_text (GTK_ENTRY (incoming_user), buf2);
		gtk_signal_handler_unblock_by_data (GTK_OBJECT (incoming_user), window);
		g_free (buf2);
	}

	if (ptr && edit_host)
	{
		gtk_signal_handler_block_by_data (GTK_OBJECT (incoming_host), window);
		gtk_entry_set_text (GTK_ENTRY (incoming_host), ptr+1);
		gtk_signal_handler_unblock_by_data (GTK_OBJECT (incoming_host), window);
	}
	process_page_identity (window->xml);
}

static void
process_page_incoming (GladeXML *xml)
{
	GtkWidget *next, *widget;
	gboolean sensitive = TRUE;
	gchar *buf, *user, *host;

	next = GNOME_DRUID (glade_xml_get_widget (xml, "dlg_account_editor_contents"))->next;

	widget = glade_xml_get_widget (xml, "incoming_server_hostname");
	host = gtk_entry_get_text (GTK_ENTRY (widget));
	if (!strlen (host))
	{
		sensitive = FALSE;
		goto incoming_set_sensitive;
	}
	
	widget = glade_xml_get_widget (xml, "incoming_server_username");
	user = gtk_entry_get_text (GTK_ENTRY (widget));
	if (!strlen (user))
	{
		sensitive = FALSE;
		goto incoming_set_sensitive;
	}

	widget = glade_xml_get_widget (xml, "options_account_name");
	buf = g_strdup_printf ("%s@%s", user, host);
	gtk_entry_set_text (GTK_ENTRY (widget), buf);
	g_free (buf);

incoming_set_sensitive:
	gtk_widget_set_sensitive (next, sensitive);
}

static void
on_general_accounts_incoming_host_changed (GtkWidget *widget, C2Window *window)
{
	gtk_object_set_data (GTK_OBJECT (widget), "changed", (gpointer) "changed");
	process_page_incoming (window->xml);
}

static void
on_general_accounts_incoming_user_changed (GtkWidget *widget, C2Window *window)
{
	gtk_object_set_data (GTK_OBJECT (widget), "changed", (gpointer) "changed");
	process_page_incoming (window->xml);
}

static void
on_general_accounts_incoming_protocol_selection_done (GtkWidget *pwidget, C2Window *window)
{
	GladeXML *xml = C2_WINDOW (window)->xml;
	GtkWidget *menu = glade_xml_get_widget (xml, "incoming_protocol");
	GtkWidget *widget;
	gchar *selection;
	
	if (GTK_BIN (menu)->child)
	{
		GtkWidget *child = GTK_BIN (menu)->child;

		if (GTK_LABEL (child))
		{
			gtk_label_get (GTK_LABEL (child), &selection);

			if (c2_streq (selection, _("POP3")) || c2_streq (selection, _("IMAP")))
			{
				widget = glade_xml_get_widget (xml, "incoming_server_host_label");
				gtk_widget_show (widget);
				widget = glade_xml_get_widget (xml, "incoming_server_hostname");
				gtk_widget_show (widget);
				widget = glade_xml_get_widget (xml, "incoming_server_port_label");
				gtk_widget_show (widget);
				widget = glade_xml_get_widget (xml, "incoming_server_port");
				gtk_widget_show (widget);

				if (c2_streq (selection, _("POP3")))
				{
					gtk_spin_button_set_value (GTK_SPIN_BUTTON (widget), 110);

					widget = glade_xml_get_widget (xml, "options_multiple_access_leave");
					gtk_widget_set_sensitive (widget, TRUE);
				} else if (c2_streq (selection, _("IMAP")))
				{
					gtk_spin_button_set_value (GTK_SPIN_BUTTON (widget), 143);

					widget = glade_xml_get_widget (xml, "options_multiple_access_leave");
					gtk_widget_set_sensitive (widget, FALSE);

					widget = glade_xml_get_widget (xml, "options_multiple_access_remove");
					gtk_widget_set_sensitive (widget, FALSE);

					widget = glade_xml_get_widget (xml, "options_multiple_access_remove_value");
					gtk_widget_set_sensitive (widget, FALSE);
				}
			}
		}
	}
}

static void
process_page_outgoing (GladeXML *xml)
{
	GtkWidget *next, *widget;
	gboolean sensitive = TRUE;
	gchar *selection;

	widget = glade_xml_get_widget (xml, "outgoing_server_protocol");
	if (GTK_BIN (widget)->child)
	{
		GtkWidget *child = GTK_BIN (widget)->child;

		if (GTK_LABEL (child))
		{
			gtk_label_get (GTK_LABEL (child), &selection);

			if (c2_streq (selection, _("SMTP")))
			{
				widget = glade_xml_get_widget (xml, "outgoing_server_hostname");
				if (!strlen (gtk_entry_get_text (GTK_ENTRY (widget))))
				{
					sensitive = FALSE;
					goto outgoing_set_sensitive;
				}
			}
		}
	}

	widget = glade_xml_get_widget (xml, "outgoing_auth_required");
	if (GTK_TOGGLE_BUTTON (widget)->active)
	{
		widget = glade_xml_get_widget (xml, "outgoing_server_username");
		if (!strlen (gtk_entry_get_text (GTK_ENTRY (widget))))
		{
			sensitive = FALSE;
			goto outgoing_set_sensitive;
		}
	}

outgoing_set_sensitive:
	next = GNOME_DRUID (glade_xml_get_widget (xml, "dlg_account_editor_contents"))->next;
	gtk_widget_set_sensitive (next, sensitive);
}

static void
on_general_accounts_outgoing_host_changed (GtkWidget *widget, C2Window *window)
{
	gtk_object_set_data (GTK_OBJECT (widget), "changed", (gpointer) "changed");
	process_page_outgoing (window->xml);
}

static void
on_general_accounts_outgoing_user_changed (GtkWidget *widget, C2Window *window)
{
	gtk_object_set_data (GTK_OBJECT (widget), "changed", (gpointer) "changed");
	process_page_outgoing (window->xml);
}

static void
on_general_accounts_outgoing_server_protocol_selection_done (GtkWidget *pwidget, C2Window *window)
{
	GladeXML *xml = C2_WINDOW (window)->xml;
	GtkWidget *menu = glade_xml_get_widget (xml, "outgoing_server_protocol");
	GtkWidget *widget;
	gchar *selection;

	widget = glade_xml_get_widget (xml, "outgoing_server_protocol");
	if (GTK_BIN (widget)->child)
	{
		GtkWidget *child = GTK_BIN (widget)->child;

		if (GTK_LABEL (child))
		{
			gtk_label_get (GTK_LABEL (child), &selection);

			if (c2_streq (selection, _("SMTP")))
			{
				widget = glade_xml_get_widget (xml, "outgoing_server_frame");
				gtk_widget_show (widget);
				widget = glade_xml_get_widget (xml, "outgoing_auth_frame");
				gtk_widget_show (widget);
			} else if (c2_streq (selection, _("Sendmail")))
			{
				widget = glade_xml_get_widget (xml, "outgoing_server_frame");
				gtk_widget_hide (widget);
				widget = glade_xml_get_widget (xml, "outgoing_auth_frame");
				gtk_widget_hide (widget);
			}
		}
	}

	process_page_outgoing (window->xml);
}

static void
on_general_accounts_outgoing_auth_required_toggled (GtkWidget *button, C2Window *window)
{
	GtkWidget *auth_type;
	GtkWidget *auth_remember;
	GtkWidget *user_label;
	GtkWidget *user;

	auth_type = glade_xml_get_widget (window->xml, "outgoing_auth_type");
	auth_remember = glade_xml_get_widget (window->xml, "outgoing_auth_remember");
	user_label = glade_xml_get_widget (window->xml, "outgoing_server_username_label");
	user = glade_xml_get_widget (window->xml, "outgoing_server_username");
	
	if (GTK_TOGGLE_BUTTON (button)->active)
	{
		gtk_widget_set_sensitive (auth_type, TRUE);
		gtk_widget_set_sensitive (auth_remember, TRUE);
		gtk_widget_set_sensitive (user_label, TRUE);
		gtk_widget_set_sensitive (user, TRUE);
	} else
	{
		gtk_widget_set_sensitive (auth_type, FALSE);
		gtk_widget_set_sensitive (auth_remember, FALSE);
		gtk_widget_set_sensitive (user_label, FALSE);
		gtk_widget_set_sensitive (user, FALSE);
	}

	process_page_outgoing (window->xml);
}

static void
process_page_options (C2Window *window)
{
	GladeXML *xml = window->xml;
	C2Account *account;
	GtkWidget *next, *widget;
	gchar *buf;
	gboolean sensitive = TRUE;

	next = GNOME_DRUID (glade_xml_get_widget (xml, "dlg_account_editor_contents"))->next;

	account = window->application->account;
	
	widget = glade_xml_get_widget (xml, "options_account_name");
	buf = gtk_entry_get_text (GTK_ENTRY (widget));
	if (!strlen (buf) || c2_account_get_by_name (account, buf))
	{
		sensitive = FALSE;
		goto options_set_sensitive;
	}
	
options_set_sensitive:
	gtk_widget_set_sensitive (next, sensitive);
}

static void
on_general_accounts_options_account_changed (GtkWidget *widget, C2Window *window)
{
	gtk_object_set_data (GTK_OBJECT (widget), "changed", (gpointer) "changed");
	process_page_options (window);
}

static void
on_general_accounts_options_multiple_access_leave_toggled (GtkWidget *button, C2Window *window)
{
	GtkWidget *multiple_access_remove;
	GtkWidget *multiple_access_remove_value;

	multiple_access_remove = glade_xml_get_widget (window->xml, "options_multiple_access_remove");
	multiple_access_remove_value = glade_xml_get_widget (window->xml, "options_multiple_access_remove_value");
	
	if (GTK_TOGGLE_BUTTON (button)->active)
	{
		gtk_widget_set_sensitive (multiple_access_remove, TRUE);
		if (GTK_TOGGLE_BUTTON (multiple_access_remove)->active)
			gtk_widget_set_sensitive (multiple_access_remove_value, TRUE);
	} else
	{
		gtk_widget_set_sensitive (multiple_access_remove, FALSE);
		gtk_widget_set_sensitive (multiple_access_remove_value, FALSE);
	}
}

static void
on_general_accounts_options_multiple_access_remove_toggled (GtkWidget *button, C2Window *window)
{
	GtkWidget *multiple_access_remove_value;

	multiple_access_remove_value = glade_xml_get_widget (window->xml, "options_multiple_access_remove_value");
	
	if (GTK_TOGGLE_BUTTON (button)->active)
		gtk_widget_set_sensitive (multiple_access_remove_value, TRUE);
	else
		gtk_widget_set_sensitive (multiple_access_remove_value, FALSE);
}

static gboolean
on_general_accounts_druid_page0_next (GnomeDruidPage *druid_page, GtkWidget *druid,
										C2Window *window)
{
	GladeXML *xml;
	GnomeDruidPage *page;

	xml = window->xml;
	page = GNOME_DRUID_PAGE (glade_xml_get_widget (xml, "page1"));
	gnome_druid_set_page (GNOME_DRUID (druid), page);
	process_page_identity (xml);
	
	return TRUE;
}

static gboolean
on_general_accounts_druid_page1_next (GnomeDruidPage *druid_page, GtkWidget *druid,
										C2Window *window)
{
	GladeXML *xml;
	GnomeDruidPage *page;

	xml = window->xml;
	page = GNOME_DRUID_PAGE (glade_xml_get_widget (xml, "page2"));
	gnome_druid_set_page (GNOME_DRUID (druid), page);
	process_page_incoming (xml);
	
	return TRUE;
}

static gboolean
on_general_accounts_druid_page2_next (GnomeDruidPage *druid_page, GtkWidget *druid,
										C2Window *window)
{
	GladeXML *xml;
	GnomeDruidPage *page;

	xml = window->xml;
	page = GNOME_DRUID_PAGE (glade_xml_get_widget (xml, "page3"));
	gnome_druid_set_page (GNOME_DRUID (druid), page);
	process_page_outgoing (xml);
	
	return TRUE;
}

static gboolean
on_general_accounts_druid_page3_next (GnomeDruidPage *druid_page, GtkWidget *druid,
										C2Window *window)
{
	GladeXML *xml;
	GnomeDruidPage *page;

	xml = window->xml;
	page = GNOME_DRUID_PAGE (glade_xml_get_widget (xml, "page4"));
	gnome_druid_set_page (GNOME_DRUID (druid), page);
	process_page_options (window);
	
	return TRUE;
}

static void
on_general_accounts_druid_page5_finish (GnomeDruidPage *druid_page, GtkWidget *druid,
										C2Window *window)
{
	C2DialogPreferences *preferences = C2_DIALOG_PREFERENCES (gtk_object_get_data (GTK_OBJECT (window), "preferences"));
	GtkWidget *widget;
	GladeXML *xml;
	C2Account *account;
	GtkWidget *menu;
	gchar *buf, *buf2, *selection;
	gint integer;
	gboolean boolean, boolean2;
	C2AccountType type;
	C2SMTPType outgoing_type;
	C2SMTP *smtp;
	gint nth;

	xml = C2_WINDOW (window)->xml;

	nth = general_accounts_get_next_account_number ();

	widget = glade_xml_get_widget (xml, "incoming_protocol");
	if (GTK_BIN (widget)->child)
	{
		GtkWidget *child = GTK_BIN (widget)->child;

		if (GTK_LABEL (child))
		{
			gtk_label_get (GTK_LABEL (child), &selection);

			if (c2_streq (selection, _("POP3")))
				type = C2_ACCOUNT_POP3;
			else if (c2_streq (selection, _("IMAP")))
				type = C2_ACCOUNT_IMAP;
		}
	}
	gnome_config_set_int ("type", type);

	widget = glade_xml_get_widget (xml, "options_account_name");
	buf = gtk_entry_get_text (GTK_ENTRY (widget));
	gnome_config_set_string ("account_name", buf);
	
	widget = glade_xml_get_widget (xml, "identity_email");
	buf2 = gtk_entry_get_text (GTK_ENTRY (widget));
	gnome_config_set_string ("identity_email", buf);

	account = c2_account_new (type, buf, buf2);
	C2_DIALOG (preferences)->application->account =
			c2_account_append (C2_DIALOG (preferences)->application->account, account);

	widget = glade_xml_get_widget (xml, "identity_name");
	buf = gtk_entry_get_text (GTK_ENTRY (widget));
	c2_account_set_extra_data (account, C2_ACCOUNT_KEY_FULL_NAME, GTK_TYPE_STRING, buf);
	gnome_config_set_string ("identity_name", buf);

	widget = glade_xml_get_widget (xml, "identity_organization");
	buf = gtk_entry_get_text (GTK_ENTRY (widget));
	c2_account_set_extra_data (account, C2_ACCOUNT_KEY_ORGANIZATION, GTK_TYPE_STRING, buf);
	gnome_config_set_string ("identity_organization", buf);

	widget = glade_xml_get_widget (xml, "identity_reply_to");
	buf = gtk_entry_get_text (GTK_ENTRY (widget));
	c2_account_set_extra_data (account, C2_ACCOUNT_KEY_REPLY_TO, GTK_TYPE_STRING, buf);
	gnome_config_set_string ("identity_reply_to", buf);

	if (type == C2_ACCOUNT_POP3)
	{
		C2POP3 *pop3;
		gint flags = 0;
		
		widget = glade_xml_get_widget (xml, "incoming_server_hostname");
		buf = gtk_entry_get_text (GTK_ENTRY (widget));
		gnome_config_set_string ("incoming_server_hostname", buf);

		widget = glade_xml_get_widget (xml, "incoming_server_port");
		integer = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (widget));
		gnome_config_set_int ("incoming_server_port", integer);

		widget = glade_xml_get_widget (xml, "incoming_server_username");
		buf2 = gtk_entry_get_text (GTK_ENTRY (widget));
		gnome_config_set_string ("incoming_server_username", buf2);
		
		widget = glade_xml_get_widget (xml, "incoming_server_ssl");
		boolean = GTK_TOGGLE_BUTTON (widget)->active;
		gnome_config_set_bool ("incoming_server_ssl", boolean);

		pop3 = c2_pop3_new (buf, integer, buf2, NULL, boolean);
		c2_account_set_extra_data (account, C2_ACCOUNT_KEY_INCOMING, GTK_TYPE_OBJECT, pop3);

		widget = glade_xml_get_widget (xml, "incoming_auth_method");
		if (GTK_BIN (widget)->child)
		{
			GtkWidget *child = GTK_BIN (widget)->child;
			
			if (GTK_LABEL (child))
			{
				gtk_label_get (GTK_LABEL (child), &selection);
				
				if (c2_streq (selection, _("Password")))
				{
					c2_pop3_set_auth_method (pop3, C2_POP3_AUTHENTICATION_PASSWORD);
					gnome_config_set_int ("incoming_auth_method", C2_POP3_AUTHENTICATION_PASSWORD);
				} else if (c2_streq (selection, _("APOP")))
				{
					c2_pop3_set_auth_method (pop3, C2_POP3_AUTHENTICATION_APOP);
					gnome_config_set_int ("incoming_auth_method", C2_POP3_AUTHENTICATION_APOP);
				}
			}
		}

		widget = glade_xml_get_widget (xml, "incoming_auth_remember");
		boolean = GTK_TOGGLE_BUTTON (widget)->active;
		gnome_config_set_bool ("incoming_auth_remember", boolean);

		if (boolean)
			c2_pop3_set_flags (pop3, pop3->flags | C2_POP3_DO_NOT_LOSE_PASSWORD);
		else
			c2_pop3_set_flags (pop3, pop3->flags | C2_POP3_DO_LOSE_PASSWORD);

		widget = glade_xml_get_widget (xml, "options_multiple_access_leave");
		boolean = GTK_TOGGLE_BUTTON (widget)->active;
		gnome_config_set_bool ("options_multiple_access_leave", boolean);

		widget = glade_xml_get_widget (xml, "options_multiple_access_remove");
		gnome_config_set_bool ("options_multiple_access_remove", GTK_TOGGLE_BUTTON (widget)->active);
		
		if (GTK_TOGGLE_BUTTON (widget)->active && boolean)
		{
			widget = glade_xml_get_widget (xml, "options_multiple_access_remove_value");
			integer = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (widget));
			gnome_config_set_int ("options_multiple_access_remove_value", integer);
		} else
		{
			integer = 0;
			gnome_config_set_int ("options_multiple_access_remove_value", 0);
		}
		
		c2_pop3_set_leave_copy (pop3, boolean, integer);
	} else if (type == C2_ACCOUNT_IMAP)
	{
		C2IMAP *imap;
		
		widget = glade_xml_get_widget (xml, "incoming_server_hostname");
		buf = gtk_entry_get_text (GTK_ENTRY (widget));
		gnome_config_set_string ("incoming_server_hostname", buf);

		widget = glade_xml_get_widget (xml, "incoming_server_port");
		integer = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (widget));
		gnome_config_set_int ("incoming_server_port", integer);

		widget = glade_xml_get_widget (xml, "incoming_server_username");
		buf2 = gtk_entry_get_text (GTK_ENTRY (widget));
		gnome_config_set_string ("incoming_server_username", buf2);
		
		widget = glade_xml_get_widget (xml, "incoming_server_ssl");
		boolean = GTK_TOGGLE_BUTTON (widget)->active;
		gnome_config_set_bool ("incoming_server_ssl", boolean);

		imap = c2_imap_new (buf, integer, buf2, NULL, boolean);
		c2_account_set_extra_data (account, C2_ACCOUNT_KEY_INCOMING, GTK_TYPE_OBJECT, imap);
	}

	widget = glade_xml_get_widget (xml, "outgoing_server_protocol");
	if (GTK_BIN (widget)->child)
	{
		GtkWidget *child = GTK_BIN (widget)->child;

		if (GTK_LABEL (child))
		{
			gtk_label_get (GTK_LABEL (child), &selection);

			if (c2_streq (selection, _("SMTP")))
			{
				outgoing_type = C2_SMTP_REMOTE;
				gnome_config_set_int ("outgoing_server_protocol", C2_SMTP_REMOTE);
			} else if (c2_streq (selection, _("Sendmail")))
			{
				outgoing_type = C2_SMTP_LOCAL;
				gnome_config_set_int ("outgoing_server_protocol", C2_SMTP_LOCAL);
			}
		}
	}

	widget = glade_xml_get_widget (xml, "outgoing_server_hostname");
	buf = gtk_entry_get_text (GTK_ENTRY (widget));
	gnome_config_set_string ("outgoing_server_hostname", buf);

	widget = glade_xml_get_widget (xml, "outgoing_server_port");
	integer = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (widget));
	gnome_config_set_int ("outgoing_server_port", integer);

	widget = glade_xml_get_widget (xml, "outgoing_server_ssl");
	boolean = GTK_TOGGLE_BUTTON (widget)->active;
	gnome_config_set_bool ("outgoing_server_ssl", boolean);

	widget = glade_xml_get_widget (xml, "outgoing_auth_required");
	boolean2 = GTK_TOGGLE_BUTTON (widget)->active;
	gnome_config_set_bool ("outgoing_server_required", boolean2);

	widget = glade_xml_get_widget (xml, "outgoing_server_username");
	buf2 = gtk_entry_get_text (GTK_ENTRY (widget));
	gnome_config_set_string ("outgoing_server_username", buf2);

	smtp = c2_smtp_new (outgoing_type, buf, integer, boolean, boolean2, buf2, NULL);

	widget = glade_xml_get_widget (xml, "outgoing_auth_remember");
	boolean = GTK_TOGGLE_BUTTON (widget)->active;
	gnome_config_set_bool ("outgoing_auth_remember", boolean);
	if (boolean)
		c2_smtp_set_flags (smtp, smtp->flags | C2_SMTP_DO_NOT_LOSE_PASSWORD);
	else
		c2_smtp_set_flags (smtp, smtp->flags | C2_SMTP_DO_LOSE_PASSWORD);
	c2_account_set_extra_data (account, C2_ACCOUNT_KEY_OUTGOING, GTK_TYPE_OBJECT, smtp);

	widget = gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (
							glade_xml_get_widget (xml, "options_signature_plain")));
	buf = gtk_entry_get_text (GTK_ENTRY (widget));
	c2_account_set_extra_data (account, C2_ACCOUNT_KEY_SIGNATURE_PLAIN, GTK_TYPE_STRING, buf);
	gnome_config_set_string ("options_signature_plain", buf);

	widget = gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (
							glade_xml_get_widget (xml, "options_signature_html")));
	buf = gtk_entry_get_text (GTK_ENTRY (widget));
	c2_account_set_extra_data (account, C2_ACCOUNT_KEY_SIGNATURE_HTML, GTK_TYPE_STRING, buf);
	gnome_config_set_string ("options_signature_html", buf);

	widget = glade_xml_get_widget (xml, "options_auto_check");
	boolean = GTK_TOGGLE_BUTTON (widget)->active;
	c2_account_set_extra_data (account, C2_ACCOUNT_KEY_ACTIVE, GTK_TYPE_BOOL, &boolean);
	gnome_config_set_bool ("options_auto_check", boolean);

	gnome_config_sync ();

	set_values_accounts (preferences);
	gtk_widget_destroy (GTK_WIDGET (window));
}

static gint
general_accounts_get_next_account_number (void)
{
	gint i;
	gchar *buf;
	gchar *name;

	for (i = 1;; i++)
	{
		buf = g_strdup_printf ("/"PACKAGE"/Account %d/", i);
		gnome_config_push_prefix (buf);
		name = gnome_config_get_string ("account_name");
		g_free (name);
		g_free (buf);
		if (!name)
			return i;
		gnome_config_pop_prefix ();
	}
}

static void
on_general_accounts_remove_clicked (GtkWidget *pwidget, C2DialogPreferences *preferences)
{
	GladeXML *xml;
	GtkWidget *confirm_dialog;

	xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("preferences"), "dlg_account_remove_confirm");
	confirm_dialog = glade_xml_get_widget (xml, "dlg_account_remove_confirm");
	
	switch (gnome_dialog_run_and_close (GNOME_DIALOG (confirm_dialog)))
	{
		case 0:
			{
				C2Account *prev, *account, *next;
				GtkWidget *widget;
				gint row;

				widget = glade_xml_get_widget (C2_DIALOG (preferences)->xml, "general_accounts_clist");

				row = GPOINTER_TO_INT (GTK_CLIST (widget)->selection->data);
				account = C2_ACCOUNT (gtk_clist_get_row_data (GTK_CLIST (widget), row));

				gtk_clist_set_row_data (GTK_CLIST (widget), row, NULL);
				general_accounts_update_list (preferences);
				gtk_clist_select_row (GTK_CLIST (widget), row, 0);
				gtk_object_unref (GTK_OBJECT (account));
				general_accounts_update_conf (preferences);
			}
			break;
	}
	
	gtk_object_destroy (GTK_OBJECT (xml));
}

static void
on_general_accounts_up_clicked (GtkWidget *pwidget, C2DialogPreferences *preferences)
{
	GladeXML *xml = C2_DIALOG (preferences)->xml;
	GtkWidget *clist = glade_xml_get_widget (xml, "general_accounts_clist");
	gint row = GPOINTER_TO_INT (GTK_CLIST (clist)->selection->data);

	gtk_clist_swap_rows (GTK_CLIST (clist), row, row-1);
	general_accounts_update_list (preferences);
	gtk_clist_select_row (GTK_CLIST (clist), row-1, 0);
	general_accounts_update_conf (preferences);
}

static void
on_general_accounts_down_clicked (GtkWidget *pwidget, C2DialogPreferences *preferences)
{
	GladeXML *xml = C2_DIALOG (preferences)->xml;
	GtkWidget *clist = glade_xml_get_widget (xml, "general_accounts_clist");
	gint row = GPOINTER_TO_INT (GTK_CLIST (clist)->selection->data);

	gtk_clist_swap_rows (GTK_CLIST (clist), row, row+1);
	general_accounts_update_list (preferences);
	gtk_clist_select_row (GTK_CLIST (clist), row+1, 0);
	general_accounts_update_conf (preferences);
}
