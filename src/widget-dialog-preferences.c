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

#include <libcronosII/utils.h>

#include "widget-dialog-preferences.h"
#include "widget-sidebar.h"

static void
class_init									(C2DialogPreferencesClass *klass);

static void
init										(C2DialogPreferences *preferences);

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

	return GTK_WIDGET (preferences);
}

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
		{ N_("Options"), PKGDATADIR "/pixmaps/general_options.png" },
		{ N_("Accounts"), PKGDATADIR "/pixmaps/general_accounts.png" },
		{ N_("Paths"), PKGDATADIR "/pixmaps/general_paths.png" },
		{ N_("Plugins"),	PKGDATADIR "/pixmaps/general_plugins.png" },
		{ NULL, NULL }
	};

	static C2SidebarSubSection interface_icons[] =
	{
		{ N_("Fonts"), PKGDATADIR "/pixmaps/interface_fonts.png" },
		{ N_("HTML"), PKGDATADIR "/pixmaps/interface_html.png" },
		{ N_("Composer"), PKGDATADIR "/pixmaps/interface_composer.png" },
		{ N_("Misc"),	PKGDATADIR "/pixmaps/interface_misc.png" },
		{ NULL, NULL }
	};

	static C2SidebarSubSection advanced_icons[] =
	{
		{ N_("Misc"), PKGDATADIR "/pixmaps/advanced_misc.png" },
		{ NULL, NULL }
	};

	static C2SidebarSection sidebar_info[] =
	{
		{ N_("General"), general_icons, NULL },
		{ N_("Interface"), interface_icons, NULL },
		{ N_("Advanced"), advanced_icons, NULL },
		{ NULL, NULL, NULL }
	};
	GtkWidget *sidebar;
	GtkStyle *style;

	xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("preferences"), "dlg_preferences_contents");
	
	c2_dialog_construct (C2_DIALOG (preferences), application, _("Preferences"), buttons);
	C2_DIALOG (preferences)->xml = xml;
	gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (preferences)->vbox), glade_xml_get_widget (xml,
							"dlg_preferences_contents"), TRUE, TRUE, 0);

	/* Sidebar */
	sidebar = glade_xml_get_widget (xml, "sidebar");
	c2_sidebar_set_contents (C2_SIDEBAR (sidebar), sidebar_info);
	gtk_widget_show (sidebar);
}
