/*  Cronos II - The GNOME mail client
 *  Copyright (C) 2000-2001 Pablo Fernández
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
/**
 * Maintainer(s) of this file:
 * 		* Pablo Fernández
 * Code of this file by:
 * 		* Pablo Fernández
 */
#include <gnome.h>
#include <ctype.h>
#include <config.h>

#ifdef USE_GTKHTML
#	include <gtkhtml/gtkhtml.h>
#	include <gtkhtml/gtkhtml-embedded.h>
#	include <gtkhtml/gtkhtml-stream.h>
#endif

#include <libcronosII/mailbox.h>
#include <libcronosII/error.h>
#include <libcronosII/utils.h>

#include "main.h"
#include "preferences.h"
#include "widget-HTML.h"
#include "widget-mail.h"
#include "widget-part.h"
#include "widget-dialog.h"
#include "widget-select-list.h"
#include "widget-application-utils.h"

/* [TODO]
 * 010920 Interpret some HTML symbols too.
 * 010920 The quote symbol coloring is not working
 *        correctly, the rest of the message after
 *        the quote symbol the rest of the message
 *        is colored even if its not quoted.
 */
 
#define MOD "[Widget] Mail"
#define DMOD FALSE

static void
on_body_button_press_event					(GtkWidget *widget, GdkEventButton *event);

static void
c2_mail_class_init							(C2MailClass *klass);

static void
c2_mail_init								(C2Mail *mail);

static void
c2_mail_destroy								(GtkObject *object);

static void
on_box_size_allocate						(GtkWidget *widget, GtkAllocation *a, C2Mail *mail);

static void
on_attachments_clicked						(GtkWidget *btn, C2Mail *mail);

static void
on_mail_parent_set							(GtkWidget *widget, GtkWidget *prev, C2Mail *mail);

#ifdef USE_GTKHTML
void
html_link_manager_cid						(C2HTML *html, const gchar *url, GtkHTMLStream *stream);

static gboolean
html_object_requested						(GtkHTML *html, GtkHTMLEmbedded *e, C2Mail *mail);
#elif defined USE_GTKXMHTML
void
html_link_manager_cid						(C2HTML *html, const gchar *url, C2Pthread2 *data);
#endif

static gchar *
interpret_text_plain_symbols				(const gchar *plain);

static gchar *
get_word									(const gchar *cptr, gchar **extra, gboolean *new_line);

static void
make_quote_color							(gint level, gshort *red, gshort *green, gshort *blue);

enum
{
	LAST_SIGNAL
};

static gint c2_mail_signals[LAST_SIGNAL] = { 0 };

guint
c2_mail_get_type (void)
{
	static guint c2_mail_type = 0;

	if (!c2_mail_type)
	{
		GtkTypeInfo c2_mail_info =
		{
			"C2Mail",
			sizeof (C2Mail),
			sizeof (C2MailClass),
			(GtkClassInitFunc) c2_mail_class_init,
			(GtkObjectInitFunc) c2_mail_init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL
		};

		c2_mail_type = gtk_type_unique (gtk_vbox_get_type (), &c2_mail_info);
	}

	return c2_mail_type;
}

static void
c2_mail_class_init (C2MailClass *klass)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;
	GtkContainerClass *container_class;

	object_class = (GtkObjectClass *) klass;
	widget_class = (GtkWidgetClass *) klass;
	container_class = (GtkContainerClass *) klass;

	gtk_object_class_add_signals (object_class, c2_mail_signals, LAST_SIGNAL);

	object_class->destroy = c2_mail_destroy;
}

static void
c2_mail_init (C2Mail *mail)
{
	mail->window = NULL;
	mail->message = NULL;
	mail->headers_visible = 1;
}

static void
c2_mail_destroy (GtkObject *object)
{
	if (C2_MAIL (GTK_WIDGET (object))->message)
		gtk_object_unref (GTK_OBJECT (C2_MAIL (GTK_WIDGET (object))->message));
}

GtkWidget *
c2_mail_new (C2Application *application)
{
	C2Mail *mail;
	mail = gtk_type_new (c2_mail_get_type ());
	GTK_BOX (mail)->spacing = 3;
	GTK_BOX (mail)->homogeneous = 0;
	c2_mail_construct (mail, application);
	return GTK_WIDGET (mail);
}

void
c2_mail_construct (C2Mail *mail, C2Application *application)
{
	GtkWidget *parent;
	GtkWidget *scroll;
	GtkWidget *hbox, *image;
	GtkStyle *style;
	GtkTooltips *tooltips;

	mail->application = application;

	gtk_signal_connect (GTK_OBJECT (mail), "parent_set",
						GTK_SIGNAL_FUNC (on_mail_parent_set), mail);

	tooltips = gtk_tooltips_new ();

	/* Headers */
	hbox = gtk_hbox_new (FALSE, 2);
	gtk_box_pack_start (GTK_BOX (mail), hbox, FALSE, TRUE, 0);
	gtk_widget_show (hbox);

	mail->attachments_button = gtk_button_new ();
	gtk_box_pack_end (GTK_BOX (hbox), mail->attachments_button, FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (mail->attachments_button), 2);
	gtk_button_set_relief (GTK_BUTTON (mail->attachments_button), GTK_RELIEF_NONE);
	GTK_WIDGET_UNSET_FLAGS (mail->attachments_button, GTK_CAN_FOCUS);
	gtk_tooltips_set_tip (tooltips, mail->attachments_button,
							_("Click here for the \"Attachments Tool\" window."), NULL);
	gtk_signal_connect (GTK_OBJECT (mail->attachments_button), "clicked",
						GTK_SIGNAL_FUNC (on_attachments_clicked), mail);

	image = gnome_pixmap_new_from_file (mail->headers_visible ?
								PKGDATADIR "/pixmaps/attachments36.png" :
								PKGDATADIR "/pixmaps/attachments24.png");
	gtk_container_add (GTK_CONTAINER (mail->attachments_button), image);
	gtk_widget_show (image);

	mail->headers = gtk_vbox_new (FALSE, 2);
	gtk_box_pack_start (GTK_BOX (hbox), mail->headers, FALSE, FALSE, 0);
	if (mail->headers_visible)
		gtk_widget_show (mail->headers);

	hbox = gtk_hbox_new (FALSE, 3);
	gtk_box_pack_start (GTK_BOX (mail->headers), hbox, TRUE, TRUE, 0);
	gtk_widget_show (hbox);
	
	mail->from_label[0] = gtk_label_new (_("From:"));
	gtk_box_pack_start (GTK_BOX (hbox), mail->from_label[0], FALSE, FALSE, 0);
	style = gtk_style_copy (gtk_widget_get_style (mail->from_label[0]));
	style->font = gdk_font_load (c2_font_bold);
	gtk_widget_set_style (mail->from_label[0], style);
	gtk_widget_show (mail->from_label[0]);

	mail->from_label[1] = gtk_label_new ("");
	gtk_box_pack_start (GTK_BOX (hbox), mail->from_label[1], TRUE, TRUE, 0);
	gtk_widget_show (mail->from_label[1]);
	gtk_misc_set_alignment (GTK_MISC (mail->from_label[1]), 0, 0.5);

	mail->to_label[0] = gtk_label_new (_("   To:"));
	gtk_box_pack_start (GTK_BOX (hbox), mail->to_label[0], FALSE, FALSE, 0);
	style->font = gdk_font_load (c2_font_bold);
	gtk_widget_set_style (mail->to_label[0], style);
	gtk_widget_show (mail->to_label[0]);

	mail->to_label[1] = gtk_label_new ("");
	gtk_box_pack_start (GTK_BOX (hbox), mail->to_label[1], TRUE, TRUE, 0);
	gtk_widget_show (mail->to_label[1]);
	gtk_misc_set_alignment (GTK_MISC (mail->to_label[1]), 0, 0.5);

	mail->cc_label[0] = gtk_label_new (_("   CC:"));
	gtk_box_pack_start (GTK_BOX (hbox), mail->cc_label[0], FALSE, FALSE, 0);
	style->font = gdk_font_load (c2_font_bold);
	gtk_widget_set_style (mail->cc_label[0], style);

	mail->cc_label[1] = gtk_label_new ("");
	gtk_box_pack_start (GTK_BOX (hbox), mail->cc_label[1], TRUE, TRUE, 0);
	gtk_widget_show (mail->cc_label[1]);
	gtk_misc_set_alignment (GTK_MISC (mail->cc_label[1]), 0, 0.5);

	hbox = gtk_hbox_new (FALSE, 3);
	gtk_box_pack_start (GTK_BOX (mail->headers), hbox, FALSE, TRUE, 0);
	gtk_widget_show (hbox);
	
	mail->subject_label[0] = gtk_label_new (_("Subject:"));
	gtk_box_pack_start (GTK_BOX (hbox), mail->subject_label[0], FALSE, FALSE, 0);
	style = gtk_style_copy (gtk_widget_get_style (mail->subject_label[0]));
	style->font = gdk_font_load (c2_font_bold);
	gtk_widget_set_style (mail->subject_label[0], style);
	gtk_widget_show (mail->subject_label[0]);

	mail->subject_label[1] = gtk_label_new ("");
	gtk_box_pack_start (GTK_BOX (hbox), mail->subject_label[1], TRUE, TRUE, 0);
	gtk_widget_show (mail->subject_label[1]);
	gtk_misc_set_alignment (GTK_MISC (mail->subject_label[1]), 0, 0.5);

	/* Body */
	hbox = gtk_hbox_new (FALSE, 4);
	gtk_box_pack_start (GTK_BOX (mail), hbox, TRUE, TRUE, 0);
	gtk_widget_show (hbox);
	
#ifdef USE_GTKHTML
	scroll = gtk_viewport_new (NULL, NULL);
	gtk_box_pack_start (GTK_BOX (hbox), scroll, TRUE, TRUE, 0);
	gtk_widget_show (scroll);
	gtk_viewport_set_shadow_type (GTK_VIEWPORT (scroll), GTK_SHADOW_IN);
	
	parent = gtk_scrolled_window_new (NULL, NULL);
	gtk_container_add (GTK_CONTAINER (scroll), parent);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (parent), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
#elif defined (USE_GTKXMHTML)
	parent = gtk_viewport_new (NULL, NULL);
	gtk_box_pack_start (GTK_BOX (hbox), parent, TRUE, TRUE, 0);
#else
	parent = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (parent), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
	gtk_box_pack_start (GTK_BOX (hbox), parent, TRUE, TRUE, 0);
#endif
	gtk_widget_show (parent);

	mail->body = c2_html_new (application);
	gtk_container_add (GTK_CONTAINER (parent), mail->body);
	gtk_widget_show (mail->body);

#ifdef USE_GTKHTML
#elif defined (USE_GTKXMHTML)
#else
	c2_html_set_font_default (C2_HTML (mail->body), application->fonts_gdk_message_body, NULL, 0);
#endif

	mail->attachments_scroll = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (mail->attachments_scroll),
									GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
	gtk_box_pack_start (GTK_BOX (mail), mail->attachments_scroll, FALSE, TRUE, 0);

	mail->attachments_list = gnome_icon_list_new (16, NULL, GNOME_ICON_LIST_TEXT_RIGHT);
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (mail->attachments_scroll),
											mail->attachments_list);
	gtk_widget_show (mail->attachments_list);
	
#ifdef USE_GTKHTML
	c2_html_set_link_manager (C2_HTML (mail->body), "cid", html_link_manager_cid);
//	gtk_object_set_data (GTK_OBJECT (mail->body), "attachments_object", (gpointer) c2_attachment_list_new);
#endif
}

static void
on_mail_parent_set (GtkWidget *widget, GtkWidget *prev, C2Mail *mail)
{
	GtkWidget *w;

	if (!GTK_IS_WIDGET (GTK_WIDGET (mail)->parent))
		return;
	
	/* Find the window where the mail widget is to be packed */
	for (w = GTK_WIDGET (mail)->parent; GTK_IS_WIDGET (w->parent); w = w->parent)
		;

	if (GTK_IS_WINDOW (w))
		mail->window = w;
	else
		mail->window = NULL;

	gtk_signal_connect (GTK_OBJECT (GTK_WIDGET (mail)->parent), "size_allocate",
						GTK_SIGNAL_FUNC (on_box_size_allocate), mail);
}

static void
on_attachments_open_cfg_clicked (void)
{
	system ("gnomecc --capplet=file-types-capplet");
}

static void
on_attachments_save_toggled (GtkWidget *btn, GladeXML *xml)
{
	GtkWidget *open_cmnd = glade_xml_get_widget (xml, "open_cmnd");
	GtkWidget *save_file = glade_xml_get_widget (xml, "save_file");

	gtk_widget_set_sensitive (open_cmnd, FALSE);
	gtk_widget_set_sensitive (save_file, TRUE);
}

static void
on_attachments_open_toggled (GtkWidget *btn, GladeXML *xml)
{
	GtkWidget *open_cmnd = glade_xml_get_widget (xml, "open_cmnd");
	GtkWidget *save_file = glade_xml_get_widget (xml, "save_file");

	gtk_widget_set_sensitive (open_cmnd, TRUE);
	gtk_widget_set_sensitive (save_file, FALSE);
}

static void
on_attachments_mime_activate (GtkWidget *btn, C2Mail *mail)
{
	C2Mime *mime;
	gchar *filename;
	gchar *program;
	gchar *buf, *dir, *mime_type;
	GladeXML *xml;
	GtkWidget *widget;

	mime = C2_MIME (gtk_object_get_data (GTK_OBJECT (btn), "mime"));
	if (!C2_IS_MIME (mime))
		return;

	/* We will handle "message/*" attachments */
	if (c2_streq (mime->type, "message"))
	{
		C2Message *message = c2_message_new ();

		c2_message_set_message (message, c2_mime_get_part (mime));
		message->mime = c2_mime_new (message);
		c2_mail_set_message (mail, message);
		
		return;
	}

	/* Find the program */
	mime_type = g_strdup_printf ("%s/%s", mime->type, mime->subtype);
	program = gnome_mime_program (mime_type);

	/* Find the filename */
	filename = c2_mime_get_parameter_value (mime->disposition, "filename");

	if (!program && filename)
	{
		/* Try to get the program by the filename */
		program = gnome_mime_program (gnome_mime_type (filename));
	}

	xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("dialogs"), "dlg_attachment_operation");

	widget = glade_xml_get_widget (xml, "save_file");
	c2_preferences_get_general_paths_save (dir);
	buf = g_strdup_printf ("%s%s%s", dir,
							(*(dir+(strlen (dir)-1)) == G_DIR_SEPARATOR) ? "" : G_DIR_SEPARATOR_S, filename);
	gnome_file_entry_set_default_path (GNOME_FILE_ENTRY (widget), buf);
	C2_DEBUG (buf);
	g_free (buf);
	g_free (dir);

	widget = glade_xml_get_widget (xml, "open_cmnd");
	gtk_entry_set_text (GTK_ENTRY (widget), program);

	widget = glade_xml_get_widget (xml, "save");
	gtk_signal_connect (GTK_OBJECT (widget), "toggled",
						GTK_SIGNAL_FUNC (on_attachments_save_toggled), xml);
	if (!program)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), TRUE);

	widget = glade_xml_get_widget (xml, "open");
	gtk_signal_connect (GTK_OBJECT (widget), "toggled",
						GTK_SIGNAL_FUNC (on_attachments_open_toggled), xml);
	if (program)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), TRUE);

	widget = glade_xml_get_widget (xml, "open_cfg");
	gtk_signal_connect (GTK_OBJECT (widget), "clicked",
						GTK_SIGNAL_FUNC (on_attachments_open_cfg_clicked), NULL);

	widget = glade_xml_get_widget (xml, "dlg_attachment_operation");
	c2_application_window_add (mail->application, GTK_WINDOW (widget));
	
rerun:
	switch (gnome_dialog_run (GNOME_DIALOG (widget)))
	{
		case 0:
			widget = glade_xml_get_widget (xml, "save");
			if (GTK_TOGGLE_BUTTON (widget)->active)
			{
				gchar *path;
				FILE *fd;

				widget = glade_xml_get_widget (xml, "save_file");
				path = gnome_file_entry_get_full_path (GNOME_FILE_ENTRY (widget), FALSE);

				if (!path)
					goto rerun;
				
				if (!(fd = fopen (path, "wb")))
				{
					c2_error_set (-errno);
					g_warning (_("Unable to write to %s: %s"), path, c2_error_get ());
				} else
				{
					fwrite (c2_mime_get_part (mime), sizeof (gchar), mime->length, fd);
					fclose (fd);
					g_free (path);
				}
			} else
			{
				gchar *cmnd;
				gchar *path;
				gchar *buf2;
				FILE *fd;

				widget = glade_xml_get_widget (xml, "open_cmnd");
				cmnd = gtk_entry_get_text (GTK_ENTRY (widget));

				if (!cmnd)
					goto rerun;

				path = c2_get_tmp_file (NULL);
				
				if (!(fd = fopen (path, "wb")))
				{
					c2_error_set (-errno);
					g_warning (_("Unable to write to %s: %s"), path, c2_error_get ());
				} else
				{
					fwrite (c2_mime_get_part (mime), sizeof (gchar), mime->length, fd);
					fclose (fd);

					buf = c2_str_replace_all (cmnd, "%f", path);
					if (!(gnome_mime_needsterminal (mime_type, program) || c2_strne (program, cmnd)))
						buf2 = g_strdup_printf ("%s &", buf);
					else
						buf2 = g_strdup_printf ("gnome-terminal -x %s &", buf);
					system (buf2);
					g_free (buf);
					g_free (buf2);
					g_free (path);
				}
			}
			
		case 1:
			widget = glade_xml_get_widget (xml, "dlg_attachment_operation");
			c2_application_window_remove (mail->application, GTK_WINDOW (widget));
			gnome_dialog_close (GNOME_DIALOG (widget));
			gtk_object_destroy (GTK_OBJECT (xml));
	}

	g_free (mime_type);
}

static void
on_attachments_attachments_tool_activate (GtkWidget *btn, C2Mail *mail)
{
	c2_mail_attachments_tool_new (mail);
}

static void
on_attachments_clicked (GtkWidget *btn, C2Mail *mail)
{
	GtkWidget *menu, *mi;
	C2Message *message;
	C2Mime *mime;
	gchar *label;
	gchar *filename;

	message = mail->message;
	if (!C2_IS_MESSAGE (message))
		return;
	
	/* Create the attachments menu */
	menu = gtk_menu_new ();

	for (mime = message->mime; mime; mime = mime->next)
	{
		if (!(filename = c2_mime_get_parameter_value (mime->disposition, "filename")))
		{
			/* This part doesn't have a filename, use the type/subtype instead */
			filename = g_strdup_printf ("%s/%s", mime->type, mime->subtype);
		}

		if (mime->length < 1024)
			label = g_strdup_printf ("%s (%d bytes)", filename, mime->length);
		else if (mime->length < 1024*1024)
			label = g_strdup_printf ("%s (%d Kb)", filename, mime->length/1024);
		else
			label = g_strdup_printf ("%s (%d Mb)", filename, mime->length/(1024*1024));

		mi = gtk_menu_item_new_with_label (label);

		g_free (label);
		g_free (filename);

		gtk_menu_append (GTK_MENU (menu), mi);
		gtk_widget_show (mi);
		gtk_object_set_data (GTK_OBJECT (mi), "mime", mime);
		gtk_signal_connect (GTK_OBJECT (mi), "activate",
							GTK_SIGNAL_FUNC (on_attachments_mime_activate), mail);
	}

	mi = gtk_menu_item_new ();
	gtk_menu_append (GTK_MENU (menu), mi);
	gtk_widget_show (mi);
	gtk_widget_set_sensitive (mi, FALSE);

	mi = gtk_menu_item_new_with_label (_("Attachments Tool..."));
	gtk_menu_append (GTK_MENU (menu), mi);
	gtk_widget_show (mi);
	gtk_signal_connect (GTK_OBJECT (mi), "activate",
							GTK_SIGNAL_FUNC (on_attachments_attachments_tool_activate), mail);

	gnome_popup_menu_do_popup (menu,
								NULL, NULL, NULL, NULL);
}

static void
on_message_destroy (GtkObject *obj, C2Mail *mail)
{
	mail->message = NULL;
}

static void
set_headers_attachments (C2Mail *mail, C2Message *message)
{
	gchar *buf;

	buf = c2_message_get_header_field (message, "Content-Type:");
	if (c2_strneq (buf, "multipart/", 10))
		gtk_widget_show (mail->attachments_button);
	else
		gtk_widget_hide (mail->attachments_button);
	gtk_widget_queue_resize (mail->attachments_button->parent);
	g_free (buf);
}

static void
set_headers (C2Mail *mail, C2Message *message)
{
	gchar *buf, *buf2;
	guint16 available_width;
	guint16 av_w_subject;
	guint16 av_w_dest; /* This agroups From:, To: and CC:, since they will all messure the same */
	guint16 av_w_dest_each;
	GtkStyle *style;
	GdkFont *bold_font, *common_font;
	const gchar *text;
	gboolean display_cc;

	if (!C2_IS_MESSAGE (message))
	{
		gtk_label_set_text (GTK_LABEL (mail->subject_label[1]), "");
		gtk_label_set_text (GTK_LABEL (mail->from_label[1]), "");
		gtk_label_set_text (GTK_LABEL (mail->to_label[1]), "");
		gtk_label_set_text (GTK_LABEL (mail->cc_label[1]), "");
		gtk_widget_hide (mail->attachments_button);
		gtk_widget_hide (mail->headers);
		gtk_widget_queue_resize (mail->attachments_button->parent);
		return;
	}

	if (mail->headers_visible)
		gtk_widget_show (mail->headers);

	/* Get the fonts */
	bold_font = gdk_font_load (c2_font_bold);
	style = gtk_widget_get_style (mail->subject_label[1]);
	common_font = style->font;

	/* Calculate how much available width we have for the headers */
	available_width = GTK_WIDGET (mail)->parent->allocation.width - 30;

	buf = c2_message_get_header_field (message, "Content-Type:");
	if (c2_strneq (buf, "multipart/", 10))
		available_width -= 40;

	/* Calculate space for the subject */
	gtk_label_get (GTK_LABEL (mail->subject_label[0]), &buf);
	av_w_subject = available_width - gdk_string_width (bold_font, buf);

	/* Subject */
	buf = c2_message_get_header_field (message, "Subject:");
	if (!buf || !strlen (buf))
		text = _("«No Subject»");
	else
		text = buf;

	/* Get the text that will fit in the space we have */
	buf2 = c2_application_cut_text (application, common_font, text, av_w_subject);
	gtk_label_set_text (GTK_LABEL (mail->subject_label[1]), buf2);
	g_free (buf);
	g_free (buf2);

	/* Calculate space for the destinataries */
	/* Check if we display CC */
	buf = c2_message_get_header_field (message, "CC:");
	if (buf && strlen (buf))
		display_cc = TRUE;
	else
		display_cc = FALSE;
	
	av_w_dest = available_width;
	
	gtk_label_get (GTK_LABEL (mail->from_label[0]), &buf);
	av_w_dest -= gdk_string_width (bold_font, buf);

	gtk_label_get (GTK_LABEL (mail->to_label[0]), &buf);
	av_w_dest -= gdk_string_width (bold_font, buf);

	if (display_cc)
	{
		gtk_label_get (GTK_LABEL (mail->cc_label[0]), &buf);
		av_w_dest -= gdk_string_width (bold_font, buf);
	}

	/* We have the space we have for the From:, To: and CC: labels, divide */
	if (display_cc)
		av_w_dest_each = av_w_dest/3;
	else
		av_w_dest_each = av_w_dest/2;
	
	/* Get what will be the From: label */
	buf = c2_message_get_header_field (message, "From:");
	if (buf)
	{
		gchar *buf3 = c2_str_get_senders (buf);

		if (buf3)
		{
			g_free (buf);
			buf = buf3;
		}
		text = buf;
	} else
		text = _("(nobody)");

	/* Check if From: fits in the space given */
	if (gdk_string_width (common_font, text) < av_w_dest_each)
	{
		/* Recalculate the space we have for the other headers */
		av_w_dest -= gdk_string_width (common_font, text);
		if (display_cc)
			av_w_dest_each = av_w_dest/2;
		else
			av_w_dest_each = av_w_dest/1;

		gtk_label_set_text (GTK_LABEL (mail->from_label[1]), text);
	} else
	{
		/* It doesn't, cut it */
		buf2 = c2_application_cut_text (application, common_font, text, av_w_dest_each);
		gtk_label_set_text (GTK_LABEL (mail->from_label[1]), buf2);
		g_free (buf2);
	}
		
	g_free (buf);

	/* Get what will be the To: label */
	buf = c2_message_get_header_field (message, "To:");
	if (buf)
	{
		gchar *buf3 = c2_str_get_senders (buf);

		if (buf3)
		{
			g_free (buf);
			buf = buf3;
		}
		
		text = buf;
	} else
		text = "";

	/* Check if To: fits in the space given */
	if (gdk_string_width (common_font, text) < av_w_dest_each)
	{
		/* Recalculate the space we have for the other headers */
		av_w_dest -= gdk_string_width (common_font, text);
		if (display_cc)
			av_w_dest_each = av_w_dest/1;

		gtk_label_set_text (GTK_LABEL (mail->to_label[1]), text);
	} else
	{
		/* It doesn't, cut it */
		buf2 = c2_application_cut_text (application, common_font, text, av_w_dest_each);
		gtk_label_set_text (GTK_LABEL (mail->to_label[1]), buf2);
		g_free (buf2);
	}
	
	g_free (buf);

	/* Get what will be the CC: label */
	if (display_cc)
	{
		buf = c2_message_get_header_field (message, "CC:");
		if (buf && strlen (buf))
		{
			gchar *buf3 = c2_str_get_senders (buf);
	
			if (buf3)
			{
				g_free (buf);
				buf = buf3;
			}
	
			gtk_widget_show (mail->cc_label[0]);
			gtk_widget_show (mail->cc_label[1]);
			text = buf;

			buf2 = c2_application_cut_text (application, common_font, text, av_w_dest_each);
			gtk_label_set_text (GTK_LABEL (mail->cc_label[1]), buf2);
			g_free (buf2);
		}
		g_free (buf);
	} else
	{
		gtk_label_set_text (GTK_LABEL (mail->cc_label[1]), "");
		gtk_widget_hide (mail->cc_label[0]);
		gtk_widget_hide (mail->cc_label[1]);
	}

	/* Attachments */
	set_headers_attachments (mail, message);
}

static void
on_box_size_allocate (GtkWidget *widget, GtkAllocation *a, C2Mail *mail)
{
	if (C2_IS_MESSAGE (mail->message))
		set_headers (mail, mail->message);
}

void
c2_mail_set_file (C2Mail *mail, const gchar *path)
{
	gchar *string;
	
	c2_return_if_fail (C2_IS_MAIL (mail), C2EDATA);

	if (c2_get_file (path, &string) < 0)
		return;
	
	c2_mail_set_string (mail, string);

	g_free (string);
}

void
c2_mail_set_string (C2Mail *mail, const gchar *string)
{
	if (!string)
	{
		gtk_widget_hide ((GtkWidget*) mail);
		set_headers (mail, NULL);
		return;
	} else
		gtk_widget_show ((GtkWidget*) mail);
	
	if (C2_IS_MESSAGE (mail->message))
	{
		gtk_object_unref (GTK_OBJECT (mail->message));
	}
	mail->message = NULL;

	gtk_object_set_data (GTK_OBJECT (mail->body), "message", NULL);
	
	set_headers (mail, NULL);
	c2_html_set_content_from_string (C2_HTML (mail->body), string);
}

void
c2_mail_set_message (C2Mail *mail, C2Message *message)
{
	C2Mime *mime;
	gboolean text_plain = TRUE, default_is_text_plain;
	gchar *string, *buf, *default_mime;

	if (!C2_IS_MESSAGE (message))
	{
		gtk_widget_hide ((GtkWidget*) mail);
		return;
	} else
	{
		gtk_widget_show ((GtkWidget*) mail);
	}

	if (C2_IS_MESSAGE (mail->message) && message != mail->message)
	{
		gtk_signal_disconnect_by_func (GTK_OBJECT (mail->message),
									GTK_SIGNAL_FUNC (on_message_destroy), mail);
		gtk_object_unref (GTK_OBJECT (mail->message));
	}
	mail->message = message;
	gtk_signal_connect (GTK_OBJECT (message), "destroy",
						GTK_SIGNAL_FUNC (on_message_destroy), mail);

#ifdef USE_MESSAGE_CACHE
	/* Perhaps your machine has 10Gb of RAM memory and you want to waste 'em in Cronos II... */
	gtk_object_ref (GTK_OBJECT (mail->message));
#endif

	/* Get the part that should be displayed */
	default_mime = c2_preferences_get_interface_misc_attachments_default ();
	if (c2_streq (default_mime, "text/plain"))
		default_is_text_plain = TRUE;
	else
		default_is_text_plain = FALSE;
	g_free (default_mime);
	string = message->body;
	
	
	if (default_is_text_plain)
	{
		if ((mime = c2_mime_get_part_by_content_type (message->mime, "text/plain")))
			string = mime->part;
	} else
	{
		if (!(mime = c2_mime_get_part_by_content_type (message->mime, "text/html")))
		{
			if ((mime = c2_mime_get_part_by_content_type (message->mime, "text/plain")))
				string = mime->part;
		} else
		{
			string = mime->part;
			text_plain = FALSE;
		}
	}

#if defined (USE_GTKHTML) || defined (USE_GTKXMHTML)
	if (text_plain && mime)
		string = interpret_text_plain_symbols (mime->part);
	else if (text_plain && !mime)
		string = interpret_text_plain_symbols (message->mime ?
											c2_mime_get_part (message->mime) :
											message->body);
#else
	if (C2_IS_MIME (mime))
		string = c2_mime_get_part (mime);
	else
		string = message->mime ? c2_mime_get_part (message->mime) : message->body;
#endif

	gtk_object_set_data (GTK_OBJECT (mail->body), "message", message);
	
#if defined (USE_GTKHTML) || defined (USE_GTKXMHTML)
	if (text_plain)
	{
		buf = c2_str_wrap (string, 75);
		g_free (string);
		string = buf;
	}
#endif

	set_headers (mail, message);
	c2_html_set_content_from_string (C2_HTML (mail->body), string);

#if defined (USE_GTKHTML) || defined (USE_GTKXMHTML)
	if (text_plain)
		g_free (string);
#endif
}

C2Message *
c2_mail_get_message (C2Mail *mail)
{
	return mail->message;
}

void
c2_mail_set_headers_visible (C2Mail *mail, gboolean show)
{
	GtkWidget *pixmap;
	
	mail->headers_visible = show ? 1 : 0;

	if (show)
		gtk_widget_show (mail->headers);
	else
		gtk_widget_hide (mail->headers);
	
	gtk_container_remove (GTK_CONTAINER (mail->attachments_button),
						GTK_WIDGET (gtk_container_children (
						GTK_CONTAINER (mail->attachments_button))->data));
	
	pixmap = gnome_pixmap_new_from_file (show ?
								PKGDATADIR "/pixmaps/attachments36.png" :
								PKGDATADIR "/pixmaps/attachments24.png");
	gtk_container_add (GTK_CONTAINER (mail->attachments_button), pixmap);
	gtk_widget_show (pixmap);
	
	gtk_widget_queue_resize (mail->headers->parent);
}

gboolean
c2_mail_get_headers_visible (C2Mail *mail)
{
	return ((mail->headers_visible) ? TRUE : FALSE);
}

void
c2_mail_install_hints (C2Mail *mail, GtkWidget *appbar, C2Mutex *lock)
{
	c2_html_install_hints (C2_HTML (mail->body), appbar, lock);
}

static void
on_body_button_press_event (GtkWidget *widget, GdkEventButton *event)
{
}

#ifdef USE_GTKHTML
void
html_link_manager_cid (C2HTML *html, const gchar *url, GtkHTMLStream *stream)
{
	C2Message *message = C2_MESSAGE (gtk_object_get_data (GTK_OBJECT (html), "message"));
	C2Mime *mime;

	if (!message)
	{
		g_print ("Internal error, unable to get message from Widget.\n");
		return;
	}

	for (mime = message->mime; mime; mime = mime->next)
		if (c2_streq (mime->id, url+4))
		{
			printf ("%s %s (%s/%s)\n", mime->id, url+4, mime->type, mime->subtype);
			break;
		}

	if (!mime)
		return;

	gtk_html_stream_write (stream, c2_mime_get_part (mime), mime->length);
}

static gboolean
html_object_requested (GtkHTML *html, GtkHTMLEmbedded *e, C2Mail *mail)
{
	GtkWidget *widget;

	if (c2_strnne (e->classid, "mail::", 6))
		return FALSE;

	if (c2_streq (e->classid+6, "attachments"))
	{
		widget = gtk_hbox_new (TRUE, 2);
		gtk_widget_show (widget);
		gtk_container_add (GTK_CONTAINER (e), widget);

		return TRUE;
	}

	return FALSE;
}
#elif defined (USE_GTKXMHTML)
void
html_link_manager_cid (C2HTML *html, const gchar *url, C2Pthread2 *data)
{
	C2Message *message = C2_MESSAGE (gtk_object_get_data (GTK_OBJECT (html), "message"));
	C2Mime *mime;
	gchar *tmpfile;
	FILE *fd;
	GtkWidget *widget = GTK_WIDGET (data->v1);
	XmImageInfo *image = (XmImageInfo*) data->v2;
	XmImageInfo *new_image;

	if (!message)
	{
		g_print ("Internal error, unable to get message from Widget.\n");
		return;
	}

	for (mime = message->mime; mime; mime = mime->next)
		if (c2_streq (mime->id, url+4))
			break;

	if (!mime)
		return;

	tmpfile = c2_get_tmp_file (NULL);
	if (!(fd = fopen (tmpfile, "w")))
	{
		c2_error_set (-errno);
		g_warning ("Unable to write to tmpfile %s: %d\n", tmpfile, c2_error_get ());
		g_free (tmpfile);
	} else
	{
		fwrite (c2_mime_get_part (mime), sizeof (gchar), mime->length, fd);
		fclose (fd);
		new_image = XmHTMLImageDefaultProc (widget, tmpfile, NULL, 0);
	}

	gtk_xmhtml_freeze (GTK_XMHTML (widget));
	XmHTMLImageReplace (widget, image, new_image);
	XmHTMLRedisplay (widget);
	gtk_xmhtml_thaw (GTK_XMHTML (widget));
}
#endif

#define KBYTESBUFFER 1
#define BUFFER 1024*KBYTESBUFFER
static gchar *
_commit_buffer (gchar *val, gchar *append)
{
	size_t offset;

	if (!append || !strlen (append))
		return val;

	offset = (val?strlen (val):0)*sizeof (char);

	val = g_realloc (val, offset+(strlen (append)*sizeof (char)));
	strcpy (val+offset, append);

	memset (append, 0, BUFFER*sizeof (char));

	return val;
}

static gchar *
_get_next_word (const gchar *ptr)
{
	if (*ptr == ' ' || *ptr == '\t')
		return g_strndup (ptr, 1);

	return NULL; /* TEMP FIX (JUST TO AVOID COMPILATION TIME WARNINGS) */
}

#if 0
static gchar *
interpret_text_plain_symbols (const gchar *plain)
{
	gchar *retval = NULL;
	gchar buffer[BUFFER];
	const gchar *ptr;
	gint buffer_pos;

	guint line_offset = 0; /* Current location of the pointer within the current line. */
	gchar *word;
	gchar last_char = 0;
	guint indent_level = 0;
	gboolean in_indentation;
	gchar *dbuffer;
	gchar *bptr;
	gint i;

	/* Null the buffer */
	memset (buffer, 0, sizeof (buffer));
	
	for (buffer_pos = 0, ptr = plain; *ptr != '\0'; ptr++)
	{
		/* Check if we filled out our buffer */
		if (buffer_pos >= BUFFER)
		{
			/* Commit the current buffer to the return value */
			retval = _commit_buffer (retval, buffer);
		}

		/* Get the next word */
		word = _get_next_word (ptr);

		if (!line_offset)
			in_indentation = TRUE;
		
		if (strlen (word) == 1)
		{
			if (*word == ' ' && last_char == ' ')
				retval = _add_buffer (retval, buffer, "&nbsp;", &buffer_pos);
			else if (*word == '>' && in_indentation)
			{
				indent_level++;
				retval = _add_buffer (retval, buffer, ">", &buffer_pos);
			} else
				retval = _add_buffer (retval, buffer, word, &buffer_pos);

			last_char = *word;
		} else
		{
			if (in_indentation && indent_level)
			{
				dbuffer = _get_indentation_color_tag (indent_level);

				/* We have to put the color of this line.
				 * Go to the begin of the line and insert
				 * the HTML code.
				 */

				for (bptr = buffer+strlen (buffer)-1; *bptr != '\n' && bptr != buffer; bptr--)
					;

				if (*bptr == buffer)
				{
					/* We didn't find a \n in the current buffer,
					 * this can mean either this is the very first line
					 * or the buffer has been commited and the \n is
					 * in the retval variable.
					 */
					if (retval)
					{
						/* We have something in retval, check if there's a \n in there. */
						for (bptr = retval+strlen (retval)-1; *bptr != '\n' && bptr != retval; bptr--)
							;
						
						if (*bptr == retval)
						{
							/* We didn't find a \n here either, anyway, we will prepend in here the
							 * code. */
							gchar *nretval;

							nretval = g_malloc0 ((strlen (retval)*sizeof (char))+
															(strlen (dbuffer)*sizeof (char)));
							strcpy (nretval, dbuffer);
							strcpy (nretval+(strlen (dbuffer)*sizeof (char)), retval);
							g_free (retval);
							retval = nretval;
						} else
						{
							/* We found the \n, insert there */
							gchar *nretval;

							nretval = g_malloc0 ((strlen (retval)+strlen (dbuffer))*sizeof (char));
							strncpy (nretval, retval, bptr-nretval);
							strcpy (nretval+(bptr-nretval), dbuffer);
							strcpy (nretval+(bptr-nretval)+strlen (dbuffer), bptr);
							g_free (retval);
							retval = nretval;
						}
					} else
					{
						/* We don't have other buffer, this is the first one,
						 * insert in the beggining of this buffer.
						 */
						gchar *nretval;

						nretval = g_malloc0 ((strlen (retval)*sizeof (char))+
														(strlen (dbuffer)*sizeof (char)));
						strcpy (nretval, dbuffer);
						strcpy (nretval+(strlen (dbuffer)*sizeof (char)), retval);
						g_free (retval);
						retval = nretval;
					}
				} else
				{
					
				}
					
			}
			
			in_indentation = FALSE;
		}

		if (line_offset == 0)
		{
		}

		/* Update the line offset indicator */
		if (*ptr == '\n')
			line_offset = 0;
		else
			line_offset++;
	}

	/* Commit the changes to the buffer to the return value */
	if (!(retval = _commit_buffer (retval, buffer)))
		retval = g_strdup (""); /* We can't return %NULL */

	return retval;
}
#undef KBYTESBUFFER
#undef BUFFER

#else
/* [TODO]
 * Can this be optimized as we usually do?
 */
static gchar *
interpret_text_plain_symbols (const gchar *plain)
{
	GString *string = g_string_new (NULL);
	const gchar *ptr;
	gchar *word, *extra, *buf;
	gboolean quote_line = FALSE, new_line;
	gint length, elength;
	gint quote_level = 0;
	gshort red, green, blue;
	gchar *font;
	
	font = c2_preferences_get_interface_fonts_message_body ();
	g_string_sprintf (string, "<html>\n"
					 "<body bgcolor=#ffffff>\n"
					 "<table border=0><tr><td><pre><font face=\"%s\">\n", font);
	g_free (font);
	
	for (ptr = plain;;)
	{
		word = get_word (ptr, &extra, &new_line);
		if (!strlen (word) && !strlen (extra))
			break;
		
		g_string_append (string, extra);
		length = strlen (word);
		elength = strlen (extra);
		ptr += length+elength;
		g_free (extra);

		if (*(ptr-length-elength) == '\n')
			new_line = TRUE;

		/* First do a simple check if we have to act in this word. */
		if (word[0] == word[length-1])
		{
			/* _Word_ */
			if (word[0] == '_')
			{
				gchar *buf2 = c2_str_strip_enclosed (word, '_', '_');
				
				buf = g_strdup_printf ("<u>%s</u>", buf2);
				g_free (buf2);
			} else if (word[0] == '*')
			/* *Bold*/
			{
				gchar *buf2 = c2_str_strip_enclosed (word, '*', '*');
				
				buf = g_strdup_printf ("<b>%s</b>", buf2);
				g_free (buf2);
			} else if (length > 1 && word[0] == '-' && *(ptr-length-elength) == '\n')
			/* -- (HR) */
			{
				const gchar *lptr;

				for (lptr = word; *lptr != '\0'; lptr++)
					if (*lptr != '-')
						goto avoid_interpret;

				for (lptr = ptr; *lptr != '\0' && *lptr != '\n'; lptr++)
				{
					if (!isblank (*lptr))
						goto avoid_interpret;
				}
				
				buf = g_strdup ("<hr noshade=noshade>");
			} else if (word[0] == '>' && new_line)
			/* > Quote */
			{
				const gchar *lptr;
				gshort rec;

				for (lptr = ptr-length, rec = 0; *lptr != '\0' && *lptr != '\n'; lptr++)
				{
					if (*lptr == '>')
						rec++;
					else if (!isblank (*lptr))
						break;
				}

				if (rec == quote_level)
					buf = g_strdup (word);
				else
				{
					quote_level = rec;

					if (quote_line)
						g_string_append (string, "</font>\n");
					else
						quote_line = TRUE;

					make_quote_color (rec, &red, &green, &blue);
					buf = g_strdup_printf ("<font color=#%02x%02x%02x>%s", red, green, blue, word);
				}
			}
			/* Italic: What's the text/plain symbol???
			 * [TODO] Ask Pete, he probably knows... */
			else
			{
avoid_interpret:
				if (quote_line && new_line)
					g_string_append (string, "</font>");
				buf = g_strdup (word);
			}
			
			g_string_append (string, buf);
			g_free (buf);
		} else
		{
			if (c2_streq (word, ":)") ||
				c2_streq (word, ":-)"))
				g_string_append (string, "<img src=\"c2dist://pixmaps/emoticons/emoticon-smiley.png\" width=16 height=16 alt=\":)\">");
			else if (c2_streq (word, ":D") ||
					 c2_streq (word, ":-D"))
				g_string_append (string, "<img src=\"c2dist://pixmaps/emoticons/emoticon-laugh.png\" width=16 height=16 alt=\":D\">");
			else if (c2_streq (word, ":P"))
				g_string_append (string, "<img src=\"c2dist://pixmaps/emoticons/emoticon-tounge.png\" width=16 height=16 alt=\":P\">");
			else if (c2_streq (word, ":(") ||
					 c2_streq (word, ":-("))
				g_string_append (string, "<img src=\"c2dist://pixmaps/emoticons/emoticon-sad.png\" width=16 height=16 alt=\":(\">");
			else if (c2_streq (word, ":0") ||
					 c2_streq (word, ":-0"))
				g_string_append (string, "<img src=\"c2dist://pixmaps/emoticons/emoticon-surprise.png\" width=16 height=16 alt=\":0\">");
			else if (c2_strneq (word, "http://", 7)		||
					 c2_strneq (word, "https://", 8)	||
					 c2_strneq (word, "ftp://", 6)		||
					 c2_strneq (word, "file://", 7)		||
					 c2_strneq (word, "mailto:", 7))
			{
				buf = g_strdup_printf ("<a href=\"%s\">%s</a>", word, word);
				g_string_append (string, buf);
				g_free (buf);
			}
			else if (c2_str_is_email (word))
			{
				buf = g_strdup_printf ("<a href=\"mailto:%s\">%s</a>", word, word);
				g_string_append (string, buf);
				g_free (buf);
			} else
				g_string_append (string, word);
		}

		g_free (word);
	}

	g_string_append (string, " </font></pre></td></tr></table></body></html>");

	buf = string->str;
	g_string_free (string, FALSE);

	return buf;
}

static gchar *
get_word (const gchar *cptr, gchar **extra, gboolean *new_line)
{
	GString *sextra = g_string_new (NULL);
	const gchar *ptr, *end;
	gchar *word;
	
	*new_line = FALSE;
	for (ptr = cptr; ptr && (ptr[0] == ' ' ||
							ptr[0] == '\t' ||
							ptr[0] == '\n' ||
							ptr[0] == ','); ptr++)
		g_string_append_c (sextra, ptr[0]);
	
	for (end = ptr; end[0] != '\0' && (end[0] != ' ' &&
							end[0] != '\t' &&
							end[0] != ','); end++)
	{
		if (*end == '\n')
		{
			*new_line = TRUE;
			break;
		}
	}
	
	if (end)
		word = g_strndup (ptr, end-ptr);
	else
		word = g_strdup (ptr);
	
	*extra = sextra->str;
	
	g_string_free (sextra, FALSE);
	
	return word;
}

#endif

/*
static gchar *
get_word (const gchar *cptr, gchar **extra)
{
	GString *sextra = g_string_new (NULL);
	const gchar *ptr, *end;
	gchar *word;

	for (ptr = cptr; ptr &&	(); ptr++)
		g_string_append_c (sextra, ptr[0]);
	
	for (end = ptr; end && (); end++)
		printf ("'%c' IS digit or alpha\n", *end);
	L

	if (end)
		word = g_strndup (ptr, end-ptr);
	else
		word = g_strdup (ptr);

	*extra = sextra->str;
	
	g_string_free (sextra, FALSE);

	return word;
}
*/

static void
make_quote_color (gint level, gshort *red, gshort *green, gshort *blue)
{
	gshort vect[3];
	gshort a[3] = { 0x55, 0x77, 0x99};
	gshort i, ii, iii;
	gint j = 0;

	switch (level)
	{
		case 1:
			*red = *blue = *green = 0x77;
			return;
		case 2:
			*red = *blue = *green = 0x99;
			return;
		case 3:
			*red = *blue = *green = 0xaa;
			return;
	}

	level--;

	for (i = 0; i < 3; i++)
	{
		vect[0] = a[i];
		
		for (ii = 0; ii < 3; ii++)
		{
			vect[1] = a[ii];

			for (iii = 0; iii < 3; iii++)
			{
				vect[2] = a[iii];
				if (++j > level)
					goto out;
			}
		}
	}
out:

	*red = vect[0];
	*green = vect[1];
	*blue = vect[2];
}

static void
attachments_tool_process_save (C2Mime *mime, const gchar *path, gint nth)
{
	gchar *filename;
	gchar *fullname;
	FILE *fd;

	if (!(filename = c2_mime_get_parameter_value (mime->disposition, "filename")))
		filename = g_strdup_printf ("%s_%s.%03d", mime->type, mime->subtype, nth);

	fullname = g_strconcat (path,
							(*(path+strlen (path)-1) == G_DIR_SEPARATOR) ? "": G_DIR_SEPARATOR_S,
							filename, NULL);

	if (!(fd = fopen (fullname, "w")))
	{
		c2_error_set (-errno);
		return;
	}
	fwrite (c2_mime_get_part (mime), sizeof (gchar), mime->length, fd);
	fclose (fd);
	g_free (filename);
}

static void
attachments_tool_process (C2Mail *mail)
{
	GtkWidget *dialog;
	GtkWidget *widget;
	GladeXML *xml;
	gchar *path;
	gint i;
	GSList *list, *l;

	dialog = GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (mail), "attachments_tool"));
	xml = C2_DIALOG (dialog)->xml;
	
	widget = glade_xml_get_widget (xml, "select_list");
	list = c2_select_list_get_active_items_data (C2_SELECT_LIST (widget));
	
	widget = glade_xml_get_widget (xml, "save_entry");
	path = gtk_entry_get_text (GTK_ENTRY (gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (widget))));
	if (c2_preferences_get_general_paths_smart ())
	{
		c2_preferences_set_general_paths_save (path);
		c2_preferences_commit ();
	}

	for (l = list, i = 0; l; l = g_slist_next (l))
		attachments_tool_process_save (C2_MIME (l->data), path, i++);
}

static void
on_attachments_tool_cancel_btn_clicked (GtkWidget *widget, C2Mail *mail)
{
	GtkWidget *dialog;

	dialog = GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (mail), "attachments_tool"));
	if (!dialog)
		return;
	gtk_window_set_modal (GTK_WINDOW (dialog), FALSE);
	gtk_object_destroy (GTK_OBJECT (C2_DIALOG (dialog)->xml));
	gtk_widget_destroy (dialog);
	gtk_object_remove_data (GTK_OBJECT (mail), "attachments_tool");
}

static void
on_attachments_tool_ok_btn_clicked (GtkWidget *widget, C2Mail *mail)
{
	attachments_tool_process (mail);
	on_attachments_tool_cancel_btn_clicked (widget, mail);
}

static void
on_attachments_tool_apply_btn_clicked (GtkWidget *widget, C2Mail *mail)
{
	attachments_tool_process (mail);
}

GtkWidget *
c2_mail_attachments_tool_new (C2Mail *mail)
{
	GtkWidget *dialog;
	GtkWidget *contents;
	GtkWidget *widget;
	GtkWidget *icon;
	C2Mime *mime;
	gchar *buf;

	c2_return_val_if_fail (C2_IS_MAIL (mail), NULL, C2EDATA);
	c2_return_val_if_fail (C2_IS_MESSAGE (mail->message), NULL, C2EDATA);

	dialog = c2_dialog_new (mail->application, _("Attachments Tool"), "attachments_tool",
							PKGDATADIR "/pixmaps/attachments24.png",
							GNOME_STOCK_BUTTON_OK,
							GNOME_STOCK_BUTTON_APPLY, GNOME_STOCK_BUTTON_CANCEL, NULL);
	C2_DIALOG (dialog)->xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("dialogs"),
											 "dlg_attachments_tool_container");
	gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
	gtk_widget_set_usize (dialog, 500, 350);
	
	contents = glade_xml_get_widget (C2_DIALOG (dialog)->xml, "dlg_attachments_tool_container");
	gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (dialog)->vbox), contents, TRUE, TRUE, 0);

	widget = glade_xml_get_widget (C2_DIALOG (dialog)->xml, "save_entry");
	c2_preferences_get_general_paths_save (buf);
	gnome_file_entry_set_default_path (GNOME_FILE_ENTRY (widget), buf);
	g_free (buf);

	/* Set contents */
	widget = glade_xml_get_widget (C2_DIALOG (dialog)->xml, "select_list");

	for (mime = mail->message->mime; mime; mime = mime->next)
	{
		gchar *row[] = { NULL, NULL, mime->description, NULL };
		gchar *type;
		const gchar *path;
	
		c2_select_list_append_item (C2_SELECT_LIST (widget), row, mime);

		/* Icon */
		type = g_strdup_printf ("%s/%s", mime->type, mime->subtype);
		path = gnome_mime_get_value (type, "icon-filename");
		if (!path || !g_file_exists (path))
			path = PKGDATADIR "/pixmaps/unknown-file.png";
		icon = gnome_pixmap_new_from_file_at_size (path, 16, 16);
		gtk_clist_set_pixmap (GTK_CLIST (widget), GTK_CLIST (widget)->rows-1, 1,
								GNOME_PIXMAP (icon)->pixmap, GNOME_PIXMAP (icon)->mask);

		/* Name */
		if (mime->disposition)
		{
			gchar *filename;
			
			if ((filename = c2_mime_get_parameter_value (mime->disposition, "filename")))
			{
				g_free (type);
				type = filename;
			}
		}
		C2_DEBUG (type);
		printf ("Wrinting to %d, %d\n", c2_select_list_last (C2_SELECT_LIST (widget)), 2);
		gtk_clist_set_text (GTK_CLIST (widget), c2_select_list_last (C2_SELECT_LIST (widget)), 2, type);
		g_free (type);
		
		if (c2_strneq (mime->disposition, "attachment", 10))
			c2_select_list_set_active (C2_SELECT_LIST (widget),
										c2_select_list_last (C2_SELECT_LIST (widget)), TRUE);
	}

	gnome_dialog_button_connect (GNOME_DIALOG (dialog), 0,
									GTK_SIGNAL_FUNC (on_attachments_tool_ok_btn_clicked), mail);
	gnome_dialog_button_connect (GNOME_DIALOG (dialog), 1,
									GTK_SIGNAL_FUNC (on_attachments_tool_apply_btn_clicked), mail);
	gnome_dialog_button_connect (GNOME_DIALOG (dialog), 2,
									GTK_SIGNAL_FUNC (on_attachments_tool_cancel_btn_clicked), mail);
	gtk_object_set_data (GTK_OBJECT (mail), "attachments_tool", dialog);

	gtk_widget_show (dialog);
	return dialog;
}
