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
#include <glade/glade.h>
#include <unistd.h>

#include <libcronosII/account.h>

#include "widget-application.h"
#include "widget-composer.h"
#include "widget-dialog-preferences.h"
#include "widget-window.h"

#define XML_FILE "cronosII-composer"

static void
class_init									(C2ComposerClass *klass);

static void
init										(C2Composer *composer);

static void
destroy										(GtkObject *object);

static void
on_to_changed								(GtkWidget *widget, C2Composer *composer);

static void
on_icon_list_button_press_event				(GtkWidget *widget, GdkEventButton *e, C2Composer *composer);

static void
add_attachment								(C2Composer *composer, gchar *file, gchar *description, gint nth);

static void
on_application_preferences_changed_account	(C2Application *application, gint key, gpointer value,
											 GtkCombo *combo);

static void
on_application_preferences_changed_editor_external_cmnd	(C2Application *application, gint key,
											gpointer value, GtkLabel *label);

static void
on_mnu_attachments_remove_activate			(GtkWidget *widget, C2Composer *composer);

static void
on_run_external_editor_clicked				(GtkWidget *widget, C2Composer *composer);

static void
on_attachments_clicked						(GtkWidget *widgetv, C2Composer *composer);

static void
on_mnu_attachments_edit_activate			(GtkWidget *widget, C2Composer *composer);

enum
{
	CHANGED_TITLE,
	SEND_NOW,
	SEND_LATER,
	SAVE_DRAFT,
	LAST_SIGNAL
};

static gint signals[LAST_SIGNAL] = { 0 };

static C2WindowClass *parent_class = NULL;

GtkType
c2_composer_get_type (void)
{
	static GtkType type = 0;

	if (!type)
	{
		GtkTypeInfo info =
		{
			"C2Composer",
			sizeof (C2Composer),
			sizeof (C2ComposerClass),
			(GtkClassInitFunc) class_init,
			(GtkObjectInitFunc) init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL,
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (c2_window_get_type (), &info);
	}

	return type;
}

static void
class_init (C2ComposerClass *klass)
{
	GtkObjectClass *object_class;

	object_class = (GtkObjectClass *) klass;

	parent_class = gtk_type_class (gtk_widget_get_type ());

	signals[CHANGED_TITLE] =
		gtk_signal_new ("changed_title",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2ComposerClass, changed_title),
						gtk_marshal_NONE__STRING, GTK_TYPE_NONE, 1,
						GTK_TYPE_STRING);
	signals[SEND_NOW] =
		gtk_signal_new ("send_now",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2ComposerClass, send_now),
						gtk_marshal_NONE__NONE, GTK_TYPE_NONE, 0);
	signals[SEND_LATER] =
		gtk_signal_new ("send_later",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2ComposerClass, send_later),
						gtk_marshal_NONE__NONE, GTK_TYPE_NONE, 0);
	signals[SAVE_DRAFT] =
		gtk_signal_new ("save_draft",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2ComposerClass, save_draft),
						gtk_marshal_NONE__INT, GTK_TYPE_NONE, 1,
						GTK_TYPE_INT);

	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);
	
	klass->changed_title = NULL;
	klass->send_now = NULL;
	klass->send_later = NULL;
	klass->save_draft = NULL;
	klass->add_attachment = add_attachment;
}

static void
init (C2Composer *composer)
{
	composer->type = 0;
	composer->cmnd = NULL;
	composer->file = NULL;
	composer->draft_id = -1;
	composer->operations = NULL;
	composer->operations_ptr = NULL;
}

static void
destroy (GtkObject *object)
{
}

GtkWidget *
c2_composer_new (C2Application *application)
{
	C2Composer *composer;

	composer = gtk_type_new (c2_composer_get_type ());
	c2_composer_construct (composer, application);

	return GTK_WIDGET (composer);
}

void
c2_composer_construct (C2Composer *composer, C2Application *application)
{
	GtkWidget *scroll;
	GtkWidget *widget;
	gchar *str, *buf;
	GladeXML *xml;
	
	c2_window_construct (C2_WINDOW (composer), application, _("Composer"), "composer");
	C2_WINDOW (composer)->xml = glade_xml_new (C2_APPLICATION_GLADE_FILE (XML_FILE),
								"wnd_composer_contents");
	xml = C2_WINDOW (composer)->xml;
#ifdef USE_GNOME_WINDOW_ICON
	gnome_window_icon_set_from_file (GTK_WINDOW (composer), PKGDATADIR "/pixmaps/mail-write.png");
#endif

	c2_window_set_contents_from_glade (C2_WINDOW (composer), "wnd_composer_contents");

	widget = glade_xml_get_widget (xml, "file_send_now");
	gtk_widget_set_sensitive (widget, FALSE);
	widget = glade_xml_get_widget (xml, "file_send_later");
	gtk_widget_set_sensitive (widget, FALSE);
	widget = glade_xml_get_widget (xml, "edit_cut");
	gtk_widget_set_sensitive (widget, FALSE);
	widget = glade_xml_get_widget (xml, "edit_copy");
	gtk_widget_set_sensitive (widget, FALSE);
	widget = glade_xml_get_widget (xml, "edit_paste");
	gtk_widget_set_sensitive (widget, FALSE);
	widget = glade_xml_get_widget (xml, "edit_undo");
	gtk_widget_set_sensitive (widget, FALSE);
	widget = glade_xml_get_widget (xml, "edit_redo");
	gtk_widget_set_sensitive (widget, FALSE);
	widget = glade_xml_get_widget (xml, "insert_image");
	gtk_widget_set_sensitive (widget, FALSE);
	widget = glade_xml_get_widget (xml, "insert_link");
	gtk_widget_set_sensitive (widget, FALSE);
	widget = glade_xml_get_widget (xml, "send_now_btn");
	gtk_widget_set_sensitive (widget, FALSE);
	widget = glade_xml_get_widget (xml, "send_later_btn");
	gtk_widget_set_sensitive (widget, FALSE);
	widget = glade_xml_get_widget (xml, "undo_btn");
	gtk_widget_set_sensitive (widget, FALSE);
	widget = glade_xml_get_widget (xml, "redo_btn");
	gtk_widget_set_sensitive (widget, FALSE);

	buf = gnome_config_get_string ("/"PACKAGE"/Interface-Composer/editor");

	if (c2_streq (buf, "external"))
	{
		widget = glade_xml_get_widget (xml, "edit_select_all");
		gtk_widget_set_sensitive (widget, FALSE);
		widget = glade_xml_get_widget (xml, "edit_clear");
		gtk_widget_set_sensitive (widget, FALSE);
		widget = glade_xml_get_widget (xml, "edit_find");
		gtk_widget_set_sensitive (widget, FALSE);
		widget = glade_xml_get_widget (xml, "edit_replace");
		gtk_widget_set_sensitive (widget, FALSE);
		widget = glade_xml_get_widget (xml, "edit_spell_check");
		gtk_widget_set_sensitive (widget, FALSE);
	}

	/* Fill the account combo */
	widget = glade_xml_get_widget (xml, "account");
	on_application_preferences_changed_account (application,
					C2_DIALOG_PREFERENCES_KEY_GENERAL_ACCOUNTS, NULL, GTK_COMBO (widget));
	gtk_signal_connect (GTK_OBJECT (application), "application_preferences_changed",
						GTK_SIGNAL_FUNC (on_application_preferences_changed_account),
						GTK_COMBO (widget));

	/* Set options for the attachments list */
	widget = glade_xml_get_widget (xml, "attachments_list");
	GNOME_ICON_LIST (widget)->pad10 = GNOME_ICON_LIST_TEXT_RIGHT;
	gtk_signal_connect_after (GTK_OBJECT (widget), "button_press_event",
						GTK_SIGNAL_FUNC (on_icon_list_button_press_event), composer);

	/* Create the actual editor widget */
	scroll = glade_xml_get_widget (xml, "scroll");

	if (c2_streq (buf, "external"))
	{
		GtkWidget *viewport;
		GtkWidget *alignment;
		GtkWidget *frame;
		GtkWidget *vbox, *label, *button, *hbox, *pixmap;
		
		viewport = gtk_viewport_new (NULL, NULL);
		gtk_container_add (GTK_CONTAINER (scroll), viewport);
		gtk_widget_show (viewport);

		alignment = gtk_alignment_new (0.5, 0.5, 0.220001, 0.0100004);
		gtk_widget_show (alignment);
		gtk_container_add (GTK_CONTAINER (viewport), alignment);

		frame = gtk_frame_new (NULL);
		gtk_container_add (GTK_CONTAINER (alignment), frame);
		gtk_widget_show (frame);
		
		vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
		gtk_container_add (GTK_CONTAINER (frame), vbox);
		gtk_widget_show (vbox);
		gtk_container_set_border_width (GTK_CONTAINER (vbox), GNOME_PAD_SMALL);
		gtk_widget_set_usize (vbox, 70, -1);

		label = gtk_label_new (_("You have choosen to use an external editor "
								 "instead of the built-in with Cronos II.\n"
								 "\n"
								 "Click the button below to run the editor "
								 "you selected in the Preferences dialog.\n"
								 "\n"
								 "Once you finish writting the message in "
								 "the editor save the file and close it.\n"
								 "After that you can manipulate the message "
								 "using the tools provided in this window.\n"));
		gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, TRUE, 0);
		gtk_widget_show (label);
		gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
		gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
		gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

		alignment = gtk_alignment_new (0.5, 0.5, 0.01, 0.0100004);
		gtk_box_pack_start (GTK_BOX (vbox), alignment, FALSE, FALSE, 0);
		gtk_widget_show (alignment);
		
		button = gtk_button_new ();
		gtk_container_add (GTK_CONTAINER (alignment), button);
		gtk_widget_show (button);

		hbox = gtk_hbox_new (FALSE, GNOME_PAD_SMALL);
		gtk_container_add (GTK_CONTAINER (button), hbox);
		gtk_widget_show (hbox);

		pixmap = gnome_stock_pixmap_widget (GTK_WIDGET (composer), GNOME_STOCK_PIXMAP_EXEC);
		gtk_box_pack_start (GTK_BOX (hbox), pixmap, FALSE, FALSE, 0);
		gtk_widget_show (pixmap);

		label = gtk_label_new ("");
		gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);
		gtk_widget_show (label);
		gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
		gtk_signal_connect (GTK_OBJECT (application), "application_preferences_changed",
						GTK_SIGNAL_FUNC (on_application_preferences_changed_editor_external_cmnd),
						GTK_LABEL (label));
		on_application_preferences_changed_editor_external_cmnd (application,
									C2_DIALOG_PREFERENCES_KEY_INTERFACE_COMPOSER_EDITOR_CMND,
									NULL, GTK_LABEL (label));
									
		
		gtk_signal_connect (GTK_OBJECT (button), "clicked",
							GTK_SIGNAL_FUNC (on_run_external_editor_clicked), composer);
	}

	g_free (buf);

	/* Connect signals */
	widget = glade_xml_get_widget (xml, "insert_attachment");
	gtk_signal_connect (GTK_OBJECT (widget), "activate",
						GTK_SIGNAL_FUNC (on_attachments_clicked), composer);
	
	widget = glade_xml_get_widget (xml, "attach_btn");
	gtk_signal_connect (GTK_OBJECT (widget), "clicked",
						GTK_SIGNAL_FUNC (on_attachments_clicked), composer);

	widget = glade_xml_get_widget (xml, "to");
	gtk_signal_connect (GTK_OBJECT (widget), "changed",
						GTK_SIGNAL_FUNC (on_to_changed), composer);
	

	gtk_signal_connect_object (GTK_OBJECT (composer), "destroy",
							GTK_SIGNAL_FUNC (destroy), NULL);
}

static const gchar *
get_pixmap (const gchar *filename)
{
	const gchar *type = gnome_mime_type_or_default (filename, "");
	const gchar *pixmap = type || !strlen (type) ? gnome_mime_get_value (type, "icon-filename") : NULL;

	return pixmap;
}

static void
add_attachment (C2Composer *composer, gchar *file, gchar *description, gint nth)
{
	GtkWidget *widget;
	
	if (c2_file_is_directory (file))
	{
	} else
	{
		/* Add attachment as file */
		C2ComposerAttachment *attach;
		gchar *filename;
		const gchar *pixmapfile;
		gchar *dynpixmap;
		
		attach = g_new0 (C2ComposerAttachment, 1);
		attach->file = file;
		attach->type = gnome_mime_type_or_default (file, "application/octet-stream");
		attach->description = description;

		filename = g_basename (file);
		pixmapfile = get_pixmap (filename);
		
		if (!pixmapfile || !c2_file_exists (pixmapfile))
			dynpixmap = g_strdup (PKGDATADIR "/pixmaps/unknown-file.png");

		widget = glade_xml_get_widget (C2_WINDOW (composer)->xml, "attachments_list");

		if (nth < 0)
		{
			gnome_icon_list_append (GNOME_ICON_LIST (widget),
									pixmapfile ? pixmapfile : dynpixmap, filename);
			gnome_icon_list_set_icon_data (GNOME_ICON_LIST (widget), GNOME_ICON_LIST (widget)->icons-1,
									attach);
		} else
		{
			gnome_icon_list_insert (GNOME_ICON_LIST (widget), nth,
									pixmapfile ? pixmapfile : dynpixmap, filename);
			gnome_icon_list_set_icon_data (GNOME_ICON_LIST (widget), nth, attach);
		}

		if (!pixmapfile)
			g_free (dynpixmap);
	}

	widget = glade_xml_get_widget (C2_WINDOW (composer)->xml, "attachments_label");
	gtk_widget_show (widget);
	widget = glade_xml_get_widget (C2_WINDOW (composer)->xml, "attachments_container");
	gtk_widget_show (widget);
}

static void
on_to_changed (GtkWidget *widget, C2Composer *composer)
{
	GladeXML *xml;
	gboolean sensitive;

	if (!c2_str_is_email (gtk_entry_get_text (GTK_ENTRY (widget))))
		sensitive = FALSE;
	else
		sensitive = TRUE;
	
	xml = C2_WINDOW (composer)->xml;

	gtk_widget_set_sensitive (glade_xml_get_widget (xml, "file_send_now"), sensitive);
	gtk_widget_set_sensitive (glade_xml_get_widget (xml, "file_send_later"), sensitive);
	gtk_widget_set_sensitive (glade_xml_get_widget (xml, "send_now_btn"), sensitive);
	gtk_widget_set_sensitive (glade_xml_get_widget (xml, "send_later_btn"), sensitive);
}

static void
on_application_preferences_changed_account (C2Application *application, gint key, gpointer value,
				GtkCombo *combo)
{
	C2Account *account;
	GList *popdown = NULL;
	gchar *str;

	if (key != C2_DIALOG_PREFERENCES_KEY_GENERAL_ACCOUNTS)
		return;
	
	for (account = application->account; account; account = c2_account_next (account))
	{
		const gchar *name;
		name = c2_account_get_extra_data (account, C2_ACCOUNT_KEY_FULL_NAME, NULL);
		
		if (name)
			str = g_strdup_printf ("\"%s\" <%s> (%s)", name, account->email, account->name);
		else
			str = g_strdup_printf ("%s (%s)", account->email, account->name);

		popdown = g_list_append (popdown, str);
	}
	gtk_combo_set_popdown_strings (combo, popdown);
}

static void
on_application_preferences_changed_editor_external_cmnd	(C2Application *application, gint key,
gpointer value, GtkLabel *label)
{
	gchar *cmnd, *buf2;
	
	if (key != C2_DIALOG_PREFERENCES_KEY_INTERFACE_COMPOSER_EDITOR_CMND)
		return;
	
	cmnd = gnome_config_get_string ("/"PACKAGE"/Interface-Composer/editor_external_cmnd");
	buf2 = g_strdup_printf (_("Run external editor: %s"), cmnd);
	gtk_label_set_text (label, buf2);
	g_free (buf2);
	g_free (cmnd);
}

static void
on_run_external_editor_clicked (GtkWidget *widget, C2Composer *composer)
{
	gchar *cmnd;
	gchar *filename;
	pid_t pid;

	cmnd = gnome_config_get_string ("/"PACKAGE"/Interface-Composer/editor_external_cmnd");
	filename = c2_get_tmp_file ("c2-editor-XXXXXX");

	gtk_object_set_data (GTK_OBJECT (composer), "external editor::file", filename);

	pid = fork ();

	if (pid == -1)
	{
		g_assert_not_reached ();
	}
	if (!pid)
	{
        execlp (cmnd, "cronosII-external-editor", filename, NULL);
		g_assert_not_reached ();
        _exit(-1);
	} else
	{
		fprintf (stderr, "Parent: forked a child with pid = %d\n", (int)pid);
	}
}

static void
on_attachments_dialog_file_changed (GtkWidget *widget, GladeXML *xml)
{
	GtkWidget *wbuf;
	gchar *file;
	
	file = gtk_entry_get_text (GTK_ENTRY (widget));
	wbuf = glade_xml_get_widget (xml, "type");
	gtk_label_set_text (GTK_LABEL (wbuf), gnome_mime_type_or_default (file, _("unknown")));

	wbuf = glade_xml_get_widget (xml, "pixmap");
	gnome_pixmap_load_file (GNOME_PIXMAP (wbuf), get_pixmap (file));
}

static gint
add_attachment_dialog (C2Composer *composer, gchar **file,
		gchar **description)
{
	GladeXML *xml;
	GtkWidget *widget, *wbuf;

	xml = glade_xml_new (C2_APPLICATION_GLADE_FILE (XML_FILE), "dlg_insert_attachment");
	widget = glade_xml_get_widget (xml, "dlg_insert_attachment");

	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "file")), "changed",
						GTK_SIGNAL_FUNC (on_attachments_dialog_file_changed), xml);

	wbuf = glade_xml_get_widget (xml, "file");
	if (*file)
		gtk_entry_set_text (GTK_ENTRY (wbuf), *file);

	wbuf = glade_xml_get_widget (xml, "description");
	if (*file)
		gtk_entry_set_text (GTK_ENTRY (wbuf), *description);

	c2_application_window_add (C2_WINDOW (composer)->application, GTK_WINDOW (widget));

run:
	switch (gnome_dialog_run (GNOME_DIALOG (widget)))
	{
		case 0:
			/* [TODO]
			 * Open the help window
			 */
			goto run;
		case 1:
			g_free (*file);
			g_free (*description);
			
			wbuf = glade_xml_get_widget (xml, "file");
			*file = g_strdup (gtk_entry_get_text (GTK_ENTRY (wbuf)));
			wbuf = glade_xml_get_widget (xml, "description");
			*description = g_strdup (gtk_entry_get_text (GTK_ENTRY (wbuf)));
			
			c2_application_window_remove (C2_WINDOW (composer)->application, GTK_WINDOW (widget));
			gnome_dialog_close (GNOME_DIALOG (widget));
			gtk_object_destroy (GTK_OBJECT (xml));
			return 0;
		case 2:
			c2_application_window_remove (C2_WINDOW (composer)->application, GTK_WINDOW (widget));
			gnome_dialog_close (GNOME_DIALOG (widget));
			gtk_object_destroy (GTK_OBJECT (xml));
			return -1;
	}

	return -1;
}

static void
on_attachments_clicked (GtkWidget *widgetv, C2Composer *composer)
{
	gchar *file, *description;

	file = description = NULL;

	if (add_attachment_dialog (composer, &file, &description))
		return;

	C2_COMPOSER_CLASS_FW (composer)->add_attachment (composer, file, description, -1);
}

static void
on_icon_list_button_press_event (GtkWidget *widget, GdkEventButton *e, C2Composer *composer)
{
	GladeXML *xml;
	GtkWidget *wbuf, *il;
	
	switch (e->button)
	{
		case 3:
			il = glade_xml_get_widget (C2_WINDOW (composer)->xml, "attachments_list");
			
			if (!g_list_length (GNOME_ICON_LIST (il)->selection))
				return;
			xml = glade_xml_new (C2_APPLICATION_GLADE_FILE (XML_FILE), "mnu_attachments");
			wbuf = glade_xml_get_widget (xml, "mnu_attachments");
			gtk_widget_show (wbuf);

			gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "remove_attachments")), "activate",
								GTK_SIGNAL_FUNC (on_mnu_attachments_remove_activate), composer);
			gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "properties")), "activate",
								GTK_SIGNAL_FUNC (on_mnu_attachments_edit_activate), composer);

			gnome_popup_menu_do_popup (wbuf, NULL, NULL, e, NULL);
			break;
	}
}

static void
on_mnu_attachments_remove_activate (GtkWidget *widget, C2Composer *composer)
{
	GtkWidget *il;
	GList *l, *s;
	
	il = glade_xml_get_widget (C2_WINDOW (composer)->xml, "attachments_list");

	s = g_list_copy (GNOME_ICON_LIST (il)->selection);

	gnome_icon_list_freeze (GNOME_ICON_LIST (il));
	for (l = s; l; l = g_list_next (l))
	{
		gint nth = GPOINTER_TO_INT (l->data);
		C2ComposerAttachment *attach = (C2ComposerAttachment*)
							gnome_icon_list_get_icon_data (GNOME_ICON_LIST (il), nth);

		g_free (attach->file);
		g_free (attach->description);
		g_free (attach);

		gnome_icon_list_remove (GNOME_ICON_LIST (il), nth);
	}
	gnome_icon_list_thaw (GNOME_ICON_LIST (il));
	g_list_free (s);

	if (!GNOME_ICON_LIST (il)->icons)
	{
		il = glade_xml_get_widget (C2_WINDOW (composer)->xml, "attachments_label");
		gtk_widget_hide (il);
		il = glade_xml_get_widget (C2_WINDOW (composer)->xml, "attachments_container");
		gtk_widget_hide (il);
		gtk_widget_queue_resize (GTK_WIDGET (composer));
	}
}

static void
on_mnu_attachments_edit_activate (GtkWidget *widget, C2Composer *composer)
{
	GtkWidget *il;
	GList *l, *s;

	il = glade_xml_get_widget (C2_WINDOW (composer)->xml, "attachments_list");

	s = g_list_copy (GNOME_ICON_LIST (il)->selection);

	for (l = s; l; l = g_list_next (l))
	{
		gint nth = GPOINTER_TO_INT (l->data);
		C2ComposerAttachment *attach = (C2ComposerAttachment*)
							gnome_icon_list_get_icon_data (GNOME_ICON_LIST (il), nth);
		gchar *file = attach->file, *description = attach->description;

		if (add_attachment_dialog (composer, &file, &description))
			return;

		gnome_icon_list_freeze (GNOME_ICON_LIST (il));
		gnome_icon_list_remove (GNOME_ICON_LIST (il), nth);
		C2_COMPOSER_CLASS_FW (composer)->add_attachment (composer, file, description, nth);
		gnome_icon_list_thaw (GNOME_ICON_LIST (il));
	}
	g_list_free (s);
}
