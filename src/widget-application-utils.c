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
#include "preferences.h"
#include "widget-HTML.h"
#include "widget-application.h"
#include "widget-dialog-preferences.h"

/**
 * c2_application_check_account_exists
 * @application: C2Application where to act.
 * 
 * This function checks if theres an account configured.
 *
 * Return Value:
 * %TRUE if there's an account, %FALSE if not.
 **/
gboolean
c2_application_check_account_exists (C2Application *application)
{
	GladeXML *xml;
	GtkWidget *window, *widget;
	
	if (application->account)
		return TRUE;

	xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("cronosII"), "dlg_no_accounts");
	window = glade_xml_get_widget (xml, "dlg_no_accounts");

	switch (gnome_dialog_run_and_close (GNOME_DIALOG (window)))
	{
		case 0:
			widget = c2_dialog_preferences_new (application);
			gtk_widget_show (widget);
	}

	return FALSE;
}

static void
on_dialog_release_information_show_at_start_btn_toggled (GtkWidget *button)
{
	c2_preferences_set_extra_release_information_show (GTK_TOGGLE_BUTTON (button)->active);
	c2_preferences_commit ();
}

static gboolean
on_dialog_release_information_window_delete_event (GtkWidget *widget, GdkEvent *e, GladeXML *xml)
{
	C2Application *application = C2_APPLICATION (
							gtk_object_get_data (GTK_OBJECT (xml), "application"));
L	c2_application_window_remove (application, GTK_WINDOW (widget));
	gtk_object_destroy (GTK_OBJECT (xml));

	return TRUE;
}

static gboolean
on_dialog_release_information_close_clicked (GtkWidget *widget, GladeXML *xml)
{
	C2Application *application = C2_APPLICATION (
							gtk_object_get_data (GTK_OBJECT (xml), "application"));
	GtkWidget *window = glade_xml_get_widget (xml, "dlg_release_information");
	
	c2_application_window_remove (application, GTK_WINDOW (window));
	gtk_object_destroy (GTK_OBJECT (window));
	gtk_object_destroy (GTK_OBJECT (xml));

	return TRUE;
}

/**
 * c2_application_dialog_release_information
 *
 * This function will show a dialog with information
 * about the current release.
 **/
void
c2_application_dialog_release_information (C2Application *application)
{
	GladeXML *xml;
	GtkWidget *widget, *html, *scroll, *parent;
	gint i;
	gchar *buf;
	
	xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("cronosII"), "dlg_release_information");
	gtk_object_set_data (GTK_OBJECT (xml), "application", application);

	for (i = 0; i < 3; i++)
	{
		gchar *id, *file;

		switch (i)
		{
			case 0:
				id = "welcome_box";
				file = PKGDATADIR "/welcome.html";
				break;
			case 1:
				id = "bugs_box";
				file = PKGDATADIR "/bugs.html";
				break;
			case 2:
				id = "features_box";
				file = PKGDATADIR "/features.html";
				break;
		}
		
		widget = glade_xml_get_widget (xml, id);
		
#ifdef USE_GTKHTML
		scroll = gtk_viewport_new (NULL, NULL);
		gtk_box_pack_start (GTK_BOX (widget), scroll, TRUE, TRUE, 0);
		gtk_widget_show (scroll);
		gtk_viewport_set_shadow_type (GTK_VIEWPORT (scroll), GTK_SHADOW_IN);
	
		parent = gtk_scrolled_window_new (NULL, NULL);
		gtk_container_add (GTK_CONTAINER (scroll), parent);
		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (parent), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
#elif defined (USE_GTKXMHTML)
		parent = gtk_viewport_new (NULL, NULL);
		gtk_box_pack_start (GTK_BOX (widget), parent, TRUE, TRUE, 0);
#else
		parent = gtk_scrolled_window_new (NULL, NULL);
		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (parent), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
		gtk_box_pack_start (GTK_BOX (widget), parent, TRUE, TRUE, 0);
#endif
		gtk_widget_show (parent);

		html = c2_html_new ();
		gtk_container_add (GTK_CONTAINER (parent), html);
		gtk_widget_show (html);

		/* Load the file */
		if (c2_get_file (file, &buf) > 0)
			c2_html_set_content_from_string (C2_HTML (html), buf);
//		g_free (buf);
	}

	widget = glade_xml_get_widget (xml, "show_at_start_btn");
	gtk_signal_connect (GTK_OBJECT (widget), "toggled",
						GTK_SIGNAL_FUNC (on_dialog_release_information_show_at_start_btn_toggled), NULL);
	
	widget = glade_xml_get_widget (xml, "dlg_release_information");
	gtk_signal_connect (GTK_OBJECT (widget), "delete_event",
						GTK_SIGNAL_FUNC (on_dialog_release_information_window_delete_event), xml);
	gnome_dialog_button_connect (GNOME_DIALOG (widget), 0,
						GTK_SIGNAL_FUNC (on_dialog_release_information_close_clicked), xml);
	c2_application_window_add (application, GTK_WINDOW (widget));

	gtk_widget_show (widget);
}
