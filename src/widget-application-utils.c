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
#include <libcronosII/error.h>

#include "preferences.h"
#include "widget-HTML.h"
#include "widget-application.h"
#include "widget-dialog-preferences.h"
#include "widget-mailbox-list.h"

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
	c2_application_window_remove (application, GTK_WINDOW (widget));
	gtk_object_destroy (GTK_OBJECT (xml));

	return TRUE;
}

static void
on_dialog_release_information_close_clicked (GtkWidget *widget, GladeXML *xml)
{
	C2Application *application = C2_APPLICATION (
							gtk_object_get_data (GTK_OBJECT (xml), "application"));
	GtkWidget *window = glade_xml_get_widget (xml, "dlg_release_information");
	
	c2_application_window_remove (application, GTK_WINDOW (window));
	gtk_object_destroy (GTK_OBJECT (window));
	gtk_object_destroy (GTK_OBJECT (xml));
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
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget),
								c2_preferences_get_extra_release_information_show ());
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

static gboolean
on_dialog_about_window_delete_event (GtkWidget *widget, GdkEvent *e, GladeXML *xml)
{
	C2Application *application = C2_APPLICATION (
							gtk_object_get_data (GTK_OBJECT (xml), "application"));
	c2_application_window_remove (application, GTK_WINDOW (widget));
	gtk_object_destroy (GTK_OBJECT (xml));

	return TRUE;
}

static void
on_dialog_about_web_site_clicked (GtkWidget *widget, GladeXML *xml)
{
	gnome_url_show (URL);
}

static void
on_dialog_about_close_clicked (GtkWidget *widget, GladeXML *xml)
{
	C2Application *application = C2_APPLICATION (
							gtk_object_get_data (GTK_OBJECT (xml), "application"));
	GtkWidget *window = glade_xml_get_widget (xml, "dlg_about");
	
	c2_application_window_remove (application, GTK_WINDOW (window));
	gtk_object_destroy (GTK_OBJECT (window));
	gtk_object_destroy (GTK_OBJECT (xml));
}

void
c2_application_dialog_about (C2Application *application)
{
	GladeXML *xml;
	GtkWidget *widget, *html, *scroll, *parent;
	gint i;
	gchar *buf;
	
	xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("cronosII"), "dlg_about");

	widget = glade_xml_get_widget (xml, "contents_box");
	gtk_object_set_data (GTK_OBJECT (xml), "application", application);
		
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
	if (c2_get_file (PKGDATADIR G_DIR_SEPARATOR_S "about.html", &buf) > 0)
		c2_html_set_content_from_string (C2_HTML (html), buf);
	g_free (buf);

	widget = glade_xml_get_widget (xml, "dlg_about");
	gtk_signal_connect (GTK_OBJECT (widget), "delete_event",
						GTK_SIGNAL_FUNC (on_dialog_about_window_delete_event), xml);
	gnome_dialog_button_connect (GNOME_DIALOG (widget), 0,
						GTK_SIGNAL_FUNC (on_dialog_about_web_site_clicked), xml);
	gnome_dialog_button_connect (GNOME_DIALOG (widget), 1,
						GTK_SIGNAL_FUNC (on_dialog_about_close_clicked), xml);
	c2_application_window_add (application, GTK_WINDOW (widget));

	gtk_widget_show (widget);
}

C2Mailbox *
c2_application_dialog_select_mailbox (C2Application *application, GtkWindow *parent)
{
	GladeXML *xml;
	C2Mailbox *mailbox = NULL;
	GtkWidget *widget;
	GtkWidget *scroll;
	GtkWidget *mlist;

	xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("cronosII"), "dlg_select_mailbox");
	
	scroll = glade_xml_get_widget (xml, "scroll");
	mlist = c2_mailbox_list_new (application);
	gtk_container_add (GTK_CONTAINER (scroll), mlist);
	gtk_widget_show (mlist);

	widget = glade_xml_get_widget (xml, "dlg_select_mailbox");

	c2_application_window_add (application, GTK_WINDOW (widget));
	gtk_object_ref (GTK_OBJECT (widget));

	if (GTK_IS_WINDOW (parent))
		gtk_window_set_transient_for (GTK_WINDOW (widget), parent);

	switch (gnome_dialog_run (GNOME_DIALOG (widget)))
	{
		case 0:
			mailbox = c2_mailbox_list_get_selected_mailbox (C2_MAILBOX_LIST (mlist));
			break;
		case 1:
			mailbox = NULL;
			break;
	}

	c2_application_window_remove (application, GTK_WINDOW (widget));
	gtk_object_destroy (GTK_OBJECT (widget));
	gtk_object_unref (GTK_OBJECT (xml));

	return mailbox;
}

static void
on_application_dialog_select_file_save_ok_clicked (GtkWidget *widget, gint *button)
{
	*button = 1;
	gtk_main_quit ();
}

static void
on_application_dialog_select_file_save_cancel_clicked (GtkWidget *widget, gint *button)
{
	*button = 0;
	gtk_main_quit ();
}

static void
on_application_dialog_select_file_save_delete_event (GtkWidget *widget, GdkEvent *e, gint *button)
{
	*button = 0;
	gtk_main_quit ();
}

FILE *
c2_application_dialog_select_file_save (C2Application *application, gchar **file)
{
	GtkWidget *filesel;
	gint button;
	gchar *buf;
	FILE *fd;

	if (file)
		*file = NULL;

	filesel = gtk_file_selection_new (NULL);
	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
	
	c2_application_window_add (application, GTK_WINDOW (filesel));

	c2_preferences_get_general_paths_save (buf);
	gtk_file_selection_set_filename (GTK_FILE_SELECTION (filesel), buf);
	g_free (buf);
	
	gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (filesel)->ok_button), "clicked",
						GTK_SIGNAL_FUNC (on_application_dialog_select_file_save_ok_clicked), &button);
	gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (filesel)->cancel_button), "clicked",
						GTK_SIGNAL_FUNC (on_application_dialog_select_file_save_cancel_clicked), &button);
	gtk_signal_connect (GTK_OBJECT (filesel), "delete_event",
						GTK_SIGNAL_FUNC (on_application_dialog_select_file_save_delete_event), &button);

	gtk_widget_show (filesel);
rerun:
	gtk_main ();

	if (!button)
	{
		c2_error_set (C2USRCNCL);
		c2_application_window_remove (application, GTK_WINDOW (filesel));
		gtk_widget_destroy (filesel);

		return NULL;
	}

	buf = gtk_file_selection_get_filename (GTK_FILE_SELECTION (filesel));

	/* Check if a directory was selected */
	if (c2_file_is_directory (buf))
	{
		GladeXML *xml;
		GtkWidget *dialog;
		gint ret;
		
		xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("cronosII"), "dlg_directory_selected");
		
		dialog = glade_xml_get_widget (xml, "dlg_directory_selected");

		ret = gnome_dialog_run_and_close (GNOME_DIALOG (dialog));

		gtk_object_destroy (GTK_OBJECT (xml));

		if (ret == 1)
			goto rerun;
	}

	/* Check that the file doesn't exist yet */
	if (!c2_file_exists (buf))
	{
		GladeXML *xml;
		GtkWidget *dialog;
		gint ret;
		
		xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("cronosII"), "dlg_confirm_overwrite");
		
		dialog = glade_xml_get_widget (xml, "dlg_confirm_overwrite");

		ret = gnome_dialog_run_and_close (GNOME_DIALOG (dialog));

		gtk_object_destroy (GTK_OBJECT (xml));

		if (ret == 1)
			goto rerun;
	}

	gtk_widget_hide (filesel);
	if (file)
		*file = g_strdup (buf);

	if (!(fd = fopen (buf, "w")))
		c2_error_set (-errno);

	c2_preferences_set_general_paths_save (buf);
	c2_preferences_commit ();
	c2_application_window_remove (application, GTK_WINDOW (filesel));
	gtk_widget_destroy (filesel);

	return fd;
}
