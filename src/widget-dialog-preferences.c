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

static GtkWidget *
on_general_accounts_add_clicked				(GtkWidget *pwidget, C2DialogPreferences *preferences);

static void
on_general_paths_save_changed				(GtkWidget *widget, C2DialogPreferences *preferences);

static void
on_general_paths_get_changed				(GtkWidget *widget, C2DialogPreferences *preferences);

static void
on_interface_composer_editor_internal_toggled	(GtkWidget *widget, C2DialogPreferences *preferences);

static void
on_interface_composer_editor_external_toggled	(GtkWidget *widget, C2DialogPreferences *preferences);

static void
on_identity_name_changed					(GtkWidget *widget, C2Window *window);

static void
on_identity_email_changed					(GtkWidget *widget, C2Window *window);

static void
on_incoming_host_changed					(GtkWidget *widget, C2Window *window);

static void
on_incoming_user_changed					(GtkWidget *widget, C2Window *window);

static void
on_general_accounts_incoming_protocol_selection_done	(GtkWidget *pwidget, C2Window *window);

static void
on_general_accounts_outgoing_server_protocol_selection_done	(GtkWidget *pwidget, C2Window *window);

static gboolean
on_general_accounts_druid_page0_next		(GnomeDruidPage *druid_page, GtkWidget *druid,
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

	widget = glade_xml_get_widget (xml, "general_accounts_add");
	gtk_signal_connect (GTK_OBJECT (widget), "clicked",
						GTK_SIGNAL_FUNC (on_general_accounts_add_clicked), preferences);

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


	
	buf = g_strdup_printf ("/"PACKAGE"/General-Paths/save=%s", g_get_home_dir ());
	charv = gnome_config_get_string_with_default (buf, NULL);
	widgetv = glade_xml_get_widget (xml, "general_paths_save");
	gtk_entry_set_text (GTK_ENTRY (gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (widgetv))),
						charv);
	g_free (buf);
	g_free (charv);
	
	buf = g_strdup_printf ("/"PACKAGE"/General-Paths/get=%s", g_get_home_dir ());
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
	
	c2_dialog_construct (C2_DIALOG (preferences), application, _("Preferences"), buttons);
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

static GtkWidget *
on_general_accounts_add_clicked (GtkWidget *pwidget, C2DialogPreferences *preferences)
{
	/* This function returns the window so it can be used as a construct function */
	GtkWidget *window, *widget;
	GladeXML *xml;
	gchar *buf, *organization;
	C2Account *account;

	C2_DEBUG (C2_APPLICATION_GLADE_FILE ("preferences"));
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

	widget = glade_xml_get_widget (xml, "identity_name");
	gtk_signal_connect (GTK_OBJECT (widget), "changed",
						GTK_SIGNAL_FUNC (on_identity_name_changed), window);

	widget = glade_xml_get_widget (xml, "identity_email");
	gtk_signal_connect (GTK_OBJECT (widget), "changed",
						GTK_SIGNAL_FUNC (on_identity_email_changed), window);

	widget = glade_xml_get_widget (xml, "incoming_server_hostname");
	gtk_signal_connect (GTK_OBJECT (widget), "changed",
						GTK_SIGNAL_FUNC (on_incoming_host_changed), window);



	widget = GTK_OPTION_MENU (glade_xml_get_widget (xml, "incoming_protocol"))->menu;
	gtk_signal_connect (GTK_OBJECT (widget), "selection_done",
						GTK_SIGNAL_FUNC (on_general_accounts_incoming_protocol_selection_done), window);

	widget = GTK_OPTION_MENU (glade_xml_get_widget (xml, "outgoing_server_protocol"))->menu;
	gtk_signal_connect (GTK_OBJECT (widget), "selection_done",
						GTK_SIGNAL_FUNC (on_general_accounts_outgoing_server_protocol_selection_done), window);

	widget = glade_xml_get_widget (xml, "page0");
	gtk_signal_connect (GTK_OBJECT (widget), "next",
						GTK_SIGNAL_FUNC (on_general_accounts_druid_page0_next), window);

	widget = glade_xml_get_widget (xml, "page2");
	gtk_signal_connect (GTK_OBJECT (widget), "next",
						GTK_SIGNAL_FUNC (on_general_accounts_druid_page2_next), window);

	widget = glade_xml_get_widget (xml, "page3");
	gtk_signal_connect (GTK_OBJECT (widget), "next",
						GTK_SIGNAL_FUNC (on_general_accounts_druid_page3_next), window);

	widget = glade_xml_get_widget (xml, "page4");
	gtk_signal_connect (GTK_OBJECT (widget), "next",
						GTK_SIGNAL_FUNC (on_general_accounts_druid_page4_next), window);

	widget = glade_xml_get_widget (xml, "page5");
	gtk_signal_connect (GTK_OBJECT (widget), "finish",
						GTK_SIGNAL_FUNC (on_general_accounts_druid_page5_finish), window);

	gtk_widget_show (window);
	
	return window;
}

static void
on_general_paths_save_changed (GtkWidget *widget, C2DialogPreferences *preferences)
{
	gchar *value = gtk_entry_get_text (GTK_ENTRY (widget));

	gnome_config_set_string ("/"PACKAGE"/General-Paths/save", value);
	gtk_signal_emit (GTK_OBJECT (preferences), signals[CHANGED],
					C2_DIALOG_PREFERENCES_KEY_GENERAL_PATHS_SAVE, value);
}

static void
on_general_paths_get_changed (GtkWidget *widget, C2DialogPreferences *preferences)
{
	gchar *value = gtk_entry_get_text (GTK_ENTRY (widget));

	gnome_config_set_string ("/"PACKAGE"/General-Paths/get", value);
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
		goto set_sensitive;
	}

	widget = glade_xml_get_widget (xml, "identity_email");
	buf = gtk_entry_get_text (GTK_ENTRY (widget));
	if (!strlen (buf) || !(buf2 = strstr (buf, "@")) || strlen (buf2) == 1)
	{
		sensitive = FALSE;
		goto set_sensitive;
	}

set_sensitive:
	gtk_widget_set_sensitive (next, sensitive);
}

static void
on_identity_name_changed (GtkWidget *widget, C2Window *window)
{
	process_page_identity (window->xml);
}

static void
on_identity_email_changed (GtkWidget *widget, C2Window *window)
{
	GtkWidget *incoming_host, *incoming_user;
	gchar *buf, *buf2, *ptr;
	gboolean edit_host = TRUE;
	gboolean edit_user = TRUE;

	incoming_host = glade_xml_get_widget (window->xml, "incoming_server_hostname");
	incoming_user = glade_xml_get_widget (window->xml, "incoming_server_username");

	buf = gtk_object_get_data (GTK_OBJECT (incoming_host), "changed");
	edit_host = GPOINTER_TO_INT ((gpointer) buf);
	buf = gtk_object_get_data (GTK_OBJECT (incoming_user), "changed");
	edit_user = GPOINTER_TO_INT ((gpointer) buf);
	
	buf = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (window->xml, "identity_email")));
	ptr = strstr (buf, "@");
	if (!edit_user)
	{
		if (ptr)
			buf2 = g_strndup (buf, ptr-buf);
		else
			buf2 = g_strdup (buf);
		gtk_entry_set_text (GTK_ENTRY (incoming_user), buf2);
		g_free (buf2);
	}

	if (ptr && edit_host)
		gtk_entry_set_text (GTK_ENTRY (incoming_host), ptr+1);
	process_page_identity (window->xml);
}

static void
on_incoming_host_changed (GtkWidget *widget, C2Window *window)
{
	L
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
			}
		}
	}
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
}

static gboolean
on_general_accounts_druid_page0_next (GnomeDruidPage *druid_page, GtkWidget *druid,
										C2Window *window)
{
	GladeXML *xml;
	GnomeDruidPage *page1;

	xml = window->xml;
	page1 = GNOME_DRUID_PAGE (glade_xml_get_widget (xml, "page1"));
	gnome_druid_set_page (GNOME_DRUID (druid), page1);
	process_page_identity (xml);
	
	return TRUE;
}

static gboolean
on_general_accounts_druid_page2_next (GnomeDruidPage *druid_page, GtkWidget *druid,
										C2Window *window)
{
	GtkWidget *widget;
	GladeXML *xml;
	gchar *buf, *buf2;

	xml = C2_WINDOW (window)->xml;

	widget = glade_xml_get_widget (xml, "incoming_server_hostname");
	if (!strlen (gtk_entry_get_text (GTK_ENTRY (widget))))
	{
		GladeXML *exml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("preferences"),
									"dlg_account_error_invalid_host");
		GtkWidget *edialog = glade_xml_get_widget (exml, "dlg_account_error_invalid_host");

		gnome_dialog_run_and_close (GNOME_DIALOG (edialog));
		gtk_object_unref (GTK_OBJECT (exml));

		return TRUE;
	}

	return FALSE;
}

static gboolean
on_general_accounts_druid_page3_next (GnomeDruidPage *druid_page, GtkWidget *druid,
										C2Window *window)
{
	GtkWidget *widget;
	GladeXML *xml;
	GtkWidget *menu;
	gchar *buf, *buf2, *selection;

	xml = C2_WINDOW (window)->xml;

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

				widget = glade_xml_get_widget (xml, "outgoing_server_hostname");
				if (!strlen (gtk_entry_get_text (GTK_ENTRY (widget))))
				{
					GladeXML *exml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("preferences"),
											"dlg_account_error_invalid_host");
					GtkWidget *edialog = glade_xml_get_widget (exml, "dlg_account_error_invalid_host");
					
					gnome_dialog_run_and_close (GNOME_DIALOG (edialog));
					gtk_object_unref (GTK_OBJECT (exml));
					
					return TRUE;
				}
			} else if (c2_streq (selection, _("Sendmail")))
			{
				widget = glade_xml_get_widget (xml, "outgoing_server_frame");
				gtk_widget_show (widget);
				widget = glade_xml_get_widget (xml, "outgoing_auth_frame");
				gtk_widget_show (widget);
			}
		}
	}	

	return FALSE;
}

static gboolean
on_general_accounts_druid_page4_next (GnomeDruidPage *druid_page, GtkWidget *druid,
										C2Window *window)
{
	GtkWidget *widget;
	GladeXML *xml;
	GtkWidget *menu;
	gchar *buf, *buf2, *selection;

	xml = C2_WINDOW (window)->xml;

	widget = glade_xml_get_widget (xml, "options_account_name");
	buf = gtk_entry_get_text (GTK_ENTRY (widget));

	if (!strlen (buf))
	{
		GladeXML *exml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("preferences"),
										"dlg_account_error_invalid_account");
		GtkWidget *edialog = glade_xml_get_widget (exml, "dlg_account_error_invalid_account");
					
		gnome_dialog_run_and_close (GNOME_DIALOG (edialog));
		gtk_object_unref (GTK_OBJECT (exml));
					
		return TRUE;
	}

	if (c2_account_get_by_name (C2_WINDOW (window)->application->account, buf))
	{
		GladeXML *exml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("preferences"),
										"dlg_account_error_used_account");
		GtkWidget *edialog = glade_xml_get_widget (exml, "dlg_account_error_used_account");
					
		gnome_dialog_run_and_close (GNOME_DIALOG (edialog));
		gtk_object_unref (GTK_OBJECT (exml));
					
		return TRUE;
	}

	return FALSE;
}

static void
on_general_accounts_druid_page5_finish (GnomeDruidPage *druid_page, GtkWidget *druid,
										C2Window *window)
{
	GtkWidget *widget;
	GladeXML *xml;
	GtkWidget *menu;
	gchar *buf, *buf2, *selection;
	gchar *full_name, *mail, *organization, *reply_to, *incoming_hostname, *incoming_user, *incoming_pass,
			*smtp_hostname, *name, *signature_plain, *signature_html;
	gint incoming_port, outgoing_port;
	gboolean active, leave_copy, incoming_ssl, outgoing_ssl;
	C2AccountType incoming_protocol;

	xml = C2_WINDOW (window)->xml;

	widget = glade_xml_get_widget (xml, "identity_name");
	full_name = gtk_entry_get_text (GTK_ENTRY (widget));
	
	widget = glade_xml_get_widget (xml, "identity_email");
	mail = gtk_entry_get_text (GTK_ENTRY (widget));
	
	widget = glade_xml_get_widget (xml, "identity_organization");
	organization = gtk_entry_get_text (GTK_ENTRY (widget));

	widget = glade_xml_get_widget (xml, "identity_reply_to");
	reply_to= gtk_entry_get_text (GTK_ENTRY (widget));

	widget = glade_xml_get_widget (xml, "incoming_protocol");
	if (GTK_BIN (widget)->child)
	{
		GtkWidget *child = GTK_BIN (widget)->child;

		if (GTK_LABEL (child))
		{
			gtk_label_get (GTK_LABEL (child), &selection);

			if (c2_streq (selection, _("POP3")))
			{
				incoming_protocol = C2_ACCOUNT_POP3;
				
				widget = glade_xml_get_widget (xml, "incoming_server_hostname");
				incoming_hostname = gtk_entry_get_text (GTK_ENTRY (widget));
				
				widget = glade_xml_get_widget (xml, "incoming_server_port");
				incoming_port = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (widget));
			} else if (c2_streq (selection, _("IMAP")))
			{
				incoming_protocol = C2_ACCOUNT_IMAP;
				
				widget = glade_xml_get_widget (xml, "incoming_server_hostname");
				incoming_hostname = gtk_entry_get_text (GTK_ENTRY (widget));
				
				widget = glade_xml_get_widget (xml, "incoming_server_port");
				incoming_port = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (widget));

				widget = glade_xml_get_widget (xml, "incoming_server_ssl");
				incoming_ssl = GTK_TOGGLE_BUTTON (widget)->active;

				
			}
		}
	}
	

	C2_DEBUG (full_name);
	C2_DEBUG (mail);
	C2_DEBUG (organization);
	C2_DEBUG (incoming_hostname);
	printf ("Port: %d\n", incoming_port);
}
