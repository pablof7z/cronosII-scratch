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
#include <libcronosII/request.h>

#include "preferences.h"
#include "widget-HTML.h"
#include "widget-application.h"
#include "widget-dialog-preferences.h"
#include "widget-mailbox-list.h"
#include "widget-network-traffic.h"
#include "widget-select-list.h"

/* TODO
 * 20011208 There's still a lack for the c2_application_dialog_select_file_get
 */

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
on_dialog_add_features_thread_disconnect (C2Request *request, C2NetObjectByte *byte, gboolean success,
											GladeXML *xml)
{
	GtkWidget *widget;
	const gchar *source, *ptr;

	if (!success)
	{
		gchar *buf;
		
		gdk_threads_enter ();
		buf = g_strdup_printf (_("Failed: %s"), c2_error_object_get (GTK_OBJECT (request)));
		widget = glade_xml_get_widget (xml, "status_label");
		gtk_label_set_text (GTK_LABEL (widget), buf);
		g_free (buf);
		gdk_threads_leave ();

		return;
	}

	source = c2_request_get_source (request);

	widget = glade_xml_get_widget (xml, "select_list");

	gdk_threads_enter ();
	gtk_widget_hide (glade_xml_get_widget (xml, "status_label"));
	gdk_threads_leave ();

	for (ptr = source; *ptr != '\0';)
	{
		gchar *name, *size, *desc, *url;
		gint s, len;

		name = c2_str_get_word (0, ptr, '\r');
		size = c2_str_get_word (1, ptr, '\r');
		desc = c2_str_get_word (2, ptr, '\r');
		url  = c2_str_get_word (3, ptr, '\r');

		len = strlen (size);
		
		s = atoi (size);
		g_free (size);
		if ((s/1024) > 1024)
			size = g_strdup_printf ("%.2f Mb", (gfloat) (s/1024)/1024);
		else
			size = g_strdup_printf ("%.2f Kb", (gfloat) (s/1024));

		ptr += strlen (name) + len + strlen (desc) + strlen (url) + 4;

		if (*ptr != '\0')
			ptr++;

		{
			gchar *row[] = { name, size, desc };
			
			gdk_threads_enter ();
			c2_select_list_append_item (C2_SELECT_LIST (widget), row, g_strndup (url, strlen (url)-1));
			gdk_threads_leave ();
		}

		g_free (name);
		g_free (size);
		g_free (desc);
		g_free (url);
	}	

	gtk_object_destroy (GTK_OBJECT (request));
}

static void
dialog_add_features_thread (GladeXML *xml)
{
	C2Request *request;

	request = c2_request_new (FEATURES_URL);
	gtk_signal_connect (GTK_OBJECT (request), "disconnect",
						GTK_SIGNAL_FUNC (on_dialog_add_features_thread_disconnect), xml);
	c2_request_run (request);
}

void
c2_application_dialog_add_features (C2Application *application)
{
	GtkWidget *widget;
	GladeXML *xml;
	pthread_t thread;

	xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("cronosII"), "dlg_add_features");

	pthread_create (&thread, NULL, C2_PTHREAD_FUNC (dialog_add_features_thread), xml);

	widget = glade_xml_get_widget (xml, "dlg_add_features");
	c2_application_window_add (application, GTK_WINDOW (widget));

	switch (gnome_dialog_run (GNOME_DIALOG (widget)))
	{
		case 0:
			break;
		case 1:
			break;
		default:
		case 2:
			c2_application_window_remove (application, (GTK_WINDOW (widget)));
			gnome_dialog_close (GNOME_DIALOG (widget));
			break;
	}
	
	gtk_object_destroy (GTK_OBJECT (xml));
}

static void
on_dialog_network_traffic_close_clicked (GtkWidget *widget, GtkWidget *dialog)
{
	gtk_widget_destroy (dialog);
}

void
c2_application_dialog_network_traffic (C2Application *application)
{
	GtkWidget *dialog;
	GtkWidget *nt;

	dialog = c2_dialog_new (application, _("Network Traffic"), "network-traffic",
							PKGDATADIR "/pixmaps/send-receive16.png",
							GNOME_STOCK_BUTTON_CLOSE, NULL);
	gnome_dialog_button_connect (GNOME_DIALOG (dialog), 0,
								on_dialog_network_traffic_close_clicked, dialog);
	
	nt = c2_network_traffic_new (application);
	gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (dialog)->vbox), nt, TRUE, TRUE, 0);
	gtk_widget_show (nt);

	gtk_widget_show (dialog);
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
			default:
				id = file = NULL;
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

		html = c2_html_new (application);
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

	html = c2_html_new (application);
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

static gboolean
on_dialog_getting_in_touch_window_delete_event (GtkWidget *widget, GdkEvent *e, GladeXML *xml)
{
	C2Application *application = C2_APPLICATION (
							gtk_object_get_data (GTK_OBJECT (xml), "application"));
	c2_application_window_remove (application, GTK_WINDOW (widget));
	gtk_object_destroy (GTK_OBJECT (xml));

	return TRUE;
}

static void
on_dialog_getting_in_touch_close_clicked (GtkWidget *widget, GladeXML *xml)
{
	C2Application *application = C2_APPLICATION (
							gtk_object_get_data (GTK_OBJECT (xml), "application"));
	GtkWidget *window = glade_xml_get_widget (xml, "dlg_getting_in_touch");
	
	c2_application_window_remove (application, GTK_WINDOW (window));
	gtk_object_destroy (GTK_OBJECT (window));
	gtk_object_destroy (GTK_OBJECT (xml));
}

void
c2_application_dialog_getting_in_touch (C2Application *application)
{
	GladeXML *xml;
	GtkWidget *widget, *html, *scroll, *parent;
	gchar *buf;
	
	xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("cronosII"), "dlg_getting_in_touch");

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

	html = c2_html_new (application);
	gtk_container_add (GTK_CONTAINER (parent), html);
	gtk_widget_show (html);

	/* Load the file */
	if (c2_get_file (PKGDATADIR G_DIR_SEPARATOR_S "touch.html", &buf) > 0)
		c2_html_set_content_from_string (C2_HTML (html), buf);
	g_free (buf);

	widget = glade_xml_get_widget (xml, "dlg_getting_in_touch");
	gtk_signal_connect (GTK_OBJECT (widget), "delete_event",
						GTK_SIGNAL_FUNC (on_dialog_getting_in_touch_window_delete_event), xml);
	gnome_dialog_button_connect (GNOME_DIALOG (widget), 0,
						GTK_SIGNAL_FUNC (on_dialog_getting_in_touch_close_clicked), xml);
	c2_application_window_add (application, GTK_WINDOW (widget));

	gtk_widget_show (widget);
}

/**
 * c2_application_dialog_send_unsent_mails
 * @application: Application object.
 *
 * This function will popup a dialog asking the
 * user if s/he wants to send the unsent mails
 * in the «Outbox» mailbox.
 *
 * Return Value:
 * %TRUE if mails should be sent or %FALSE.
 **/
gboolean
c2_application_dialog_send_unsent_mails (C2Application *application)
{
	GtkWidget *widget;
	C2Mailbox *outbox;
	gint mails;
	gchar *label;
	gint reply;

	outbox = c2_mailbox_get_by_name (application->mailbox, C2_MAILBOX_OUTBOX);
	mails = c2_db_length (outbox);
	if (!mails)
		return FALSE;

	if (mails > 1)
		label = g_strdup_printf (_("There are %d unsent mails.\n"
								   "\n"
								   "Do you want to send them now?"), mails);
	else
		label = g_strdup (_("There is an unsent message.\n"
							"\n"
							"Do you want to send it now?"));
	
	widget = gnome_question_dialog_modal (label, NULL, NULL);
	c2_application_window_add (application, GTK_WINDOW (widget));
	gnome_dialog_close_hides (GNOME_DIALOG (widget), TRUE);
	
	reply = gnome_dialog_run (GNOME_DIALOG (widget));
	
	c2_application_window_remove (application, GTK_WINDOW (widget));
	gtk_widget_destroy (widget);
	g_free (label);
	
	return !reply;
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

	if (c2_preferences_get_general_paths_smart ())
	{
		gchar *dir, *buf2;

		buf2 = g_dirname (buf);
		dir = g_strdup_printf ("%s" G_DIR_SEPARATOR_S, buf2);
		g_free (buf2);
		c2_preferences_set_general_paths_save (dir);
		c2_preferences_commit ();
		g_free (dir);
	}
	c2_application_window_remove (application, GTK_WINDOW (filesel));
	gtk_widget_destroy (filesel);

	return fd;
}

/**
 * c2_application_dialog_mail_source
 * @application: Application object.
 * @message: Message to display.
 *
 * This function will popup a dialog which will show
 * the source code of @message.
 **/
void
c2_application_dialog_mail_source (C2Application *application, C2Message *message)
{
	GladeXML *xml;
	GtkWidget *widget;
	GdkColor green = { 0, 0x2d00, 0x8a00, 0x5700 };
	GdkColor red = { 0, 0xa500, 0x2900, 0x2900 };
	GdkFont *font_normal, *font_bold;
	gchar *ptr, *line = NULL;
	GtkText *text;

	gdk_color_alloc (gdk_colormap_get_system (), &green);
	gdk_color_alloc (gdk_colormap_get_system (), &red);
	font_normal = gdk_font_load ("-adobe-courier-medium-r-normal-*-*-140-*-*-m-*-iso8859-1");
	font_bold = gdk_font_load ("-adobe-courier-bold-r-normal-*-*-140-*-*-m-*-iso8859-1");
	
	xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("cronosII"), "dlg_mail_source");

	/* Set the header */
	widget = glade_xml_get_widget (xml, "text");
	text = GTK_TEXT (widget);
	
	for (ptr = message->header; *ptr != '\0';)
	{
		/* Get the current line */
		line = c2_str_get_line (ptr);
		ptr += strlen (line);

		if (*line == ' ' || *line == '\t')
			gtk_text_insert (text, font_normal, NULL, NULL, line, -1);
		else
		{
			gchar *word = c2_str_get_word (0, line, ' ');
			gint length = strlen (word);
			
			gtk_text_insert (text,
							*(word+length-1) == ':' ? font_bold : font_normal,
							c2_strneq (word, "X-", 2) ? &red : &green,
							NULL, word, length);
			gtk_text_insert (text, font_normal, NULL, NULL, line+length, -1);
			g_free (word);
		}
		g_free (line);
	}

	/* Set the body */
	gtk_text_insert (text, font_normal, NULL, NULL, "\n\n", 2);
	gtk_text_insert (text, font_normal, NULL, NULL, message->body, -1);
	
	widget = glade_xml_get_widget (xml, "dlg_mail_source");
	gtk_widget_set_usize (widget, c2_preferences_get_window_main_width (),
								  c2_preferences_get_window_main_height ());
	gnome_dialog_run_and_close (GNOME_DIALOG (widget));
	gtk_object_destroy (GTK_OBJECT (xml));
}
