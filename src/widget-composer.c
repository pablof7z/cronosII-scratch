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
on_application_preferences_changed_account	(C2Application *application, gint key, gpointer value,
											 GtkCombo *combo);

static void
on_application_preferences_changed_editor_external_cmnd	(C2Application *application, gint key,
											gpointer value, GtkLabel *label);

static void
on_run_external_editor_clicked				(GtkWidget *widget, C2Composer *composer);

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
	gint i;
	GladeXML *xml;
	
	c2_window_construct (C2_WINDOW (composer), application, _("Composer"), "composer");
	C2_WINDOW (composer)->xml = glade_xml_new (C2_APPLICATION_GLADE_FILE (XML_FILE),
								"wnd_composer_contents");
	xml = C2_WINDOW (composer)->xml;
#ifdef USE_GNOME_WINDOW_ICON
	gnome_window_icon_set_from_file (GTK_WINDOW (composer), PKGDATADIR "/pixmaps/mail-write.png");
#endif

	c2_window_set_contents_from_glade (C2_WINDOW (composer), "wnd_composer_contents");

	for (i = 0; i <= 9; i++)
	{
		switch (i)
		{
			case 0:
				str = "file_send_now";
				break;
			case 1:
				str = "file_send_later";
				break;
			case 2:
				str = "edit_undo";
				break;
			case 3:
				str = "edit_redo";
				break;
			case 4:
				str = "insert_image";
				break;
			case 5:
				str = "insert_link";
				break;
			case 6:
				str = "send_now_btn";
				break;
			case 7:
				str = "send_later_btn";
				break;
			case 8:
				str = "undo_btn";
				break;
			case 9:
				str = "redo_btn";
				break;
		}
		
		if ((widget = glade_xml_get_widget (xml, str)))
			gtk_widget_set_sensitive (widget, FALSE);
	}

	/* Fill the account combo */
	widget = glade_xml_get_widget (xml, "account");
	on_application_preferences_changed_account (application,
					C2_DIALOG_PREFERENCES_KEY_GENERAL_ACCOUNTS, NULL, GTK_COMBO (widget));
	gtk_signal_connect (GTK_OBJECT (application), "application_preferences_changed",
						GTK_SIGNAL_FUNC (on_application_preferences_changed_account),
						GTK_COMBO (widget));




	/* Create the actual editor widget */
	scroll = glade_xml_get_widget (xml, "scroll");

	buf = gnome_config_get_string ("/"PACKAGE"/Interface-Composer/editor");
	
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

		label = gtk_label_new (_("You selected to use an external editor "
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

	gtk_signal_connect_object (GTK_OBJECT (composer), "destroy",
							GTK_SIGNAL_FUNC (destroy), NULL);
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
        execlp(cmnd, "cronosII-external-editor", filename, NULL);
		g_assert_not_reached ();
        _exit(-1);
	} else
	{
		fprintf(stderr, "Parent: forked a child with pid = %d\n", (int)pid);
	}
}
