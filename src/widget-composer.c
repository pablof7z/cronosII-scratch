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
#include <sys/stat.h>
#include <config.h>
#ifdef USE_GTKHTML
#	include <gtkhtml/gtkhtml.h>
#endif

#include <libcronosII/account.h>
#include <libcronosII/error.h>
#include <libcronosII/message.h>
#include <libcronosII/smtp.h>

#include "main.h"
#include "preferences.h"
#include "widget-application.h"
#include "widget-application-utils.h"
#include "widget-composer.h"
#include "widget-dialog-preferences.h"
#include "widget-editor.h"
#include "widget-transfer-list.h"
#include "widget-transfer-item.h"
#include "widget-window.h"

#define XML_FILE "composer"

#define GET_WINDOW_WIDTH	gnome_config_get_int_with_default ("/"PACKAGE"/Composer/width=640", NULL)
#define GET_WINDOW_HEIGHT	gnome_config_get_int_with_default ("/"PACKAGE"/Composer/height=480", NULL)
#define SET_WINDOW_WIDTH(x)	gnome_config_set_int ("/"PACKAGE"/Composer/width", x)
#define SET_WINDOW_HEIGHT(x)gnome_config_set_int ("/"PACKAGE"/Composer/height", x)

#define EXTERNAL_EDITOR_FILE	"external editor::file"
#define MESSAGE_CONTENT_TYPE	"message::content type"

#define ACCOUNT_ENTRY(account, str)	\
	{ \
		const gchar *__name__; \
		__name__ = c2_account_get_extra_data (account, C2_ACCOUNT_KEY_FULL_NAME, NULL); \
		 \
		if (__name__) \
			str = g_strdup_printf ("%s <%s> (%s)", __name__, account->email, account->name); \
		else \
			str = g_strdup_printf ("%s (%s)", account->email, account->name); \
	}

static void
class_init									(C2ComposerClass *klass);

static void
init										(C2Composer *composer);

static void
destroy										(GtkObject *object);

static void
add_attachment								(C2Composer *composer, gchar *file, gchar *description, gint nth);

static void
autosave									(C2Composer *composer);

static void
find										(C2Composer *composer);

static void
open_										(C2Composer *composer, const gchar *file, C2Message *message);

static void
open_draft									(C2Composer *composer);

static void
open_file									(C2Composer *composer);

static void
save										(C2Composer *composer);

static void
save_file									(C2Composer *composer, gchar *efile);

static void
save_draft									(C2Composer *composer);

static void
send_										(C2Composer *composer, C2ComposerSendType type);

static void
send_now									(C2Composer *composer);

static void
send_later									(C2Composer *composer);


static void
on_composer_size_allocate					(C2Composer *composer, GtkAllocation *alloc);

static const gchar *
get_pixmap									(const gchar *filename);

static void
on_to_changed								(GtkWidget *widget, C2Composer *composer);

static void
on_subject_changed							(GtkWidget *widget, C2Composer *composer);

static void
on_icon_list_button_press_event				(GtkWidget *widget, GdkEventButton *e, C2Composer *composer);

static void
on_save_clicked								(GtkWidget *widget, C2Composer *composer);

static void
on_save_draft_clicked						(GtkWidget *widget, C2Composer *composer);

static void
on_save_as_clicked							(GtkWidget *widget, C2Composer *composer);

static void
on_send_now_clicked							(GtkWidget *widget, C2Composer *composer);

static void
on_send_later_clicked						(GtkWidget *widget, C2Composer *composer);

static void
on_undo_clicked								(GtkWidget *widget, C2Composer *composer);

static void
on_redo_clicked								(GtkWidget *widget, C2Composer *composer);

static void
on_editor_undo_available					(GtkWidget *widget, gboolean available, C2Composer *composer);

static void
on_editor_redo_available					(GtkWidget *widget, gboolean available, C2Composer *composer);

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

static C2Account *
get_account									(C2Composer *composer);

static C2Message *
create_message								(C2Composer *composer);

enum
{
	CHANGED_TITLE,
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
	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);
	
	klass->changed_title = NULL;
	klass->add_attachment = add_attachment;
	klass->autosave = autosave;
	klass->save = save;
	klass->save_file = save_file;
	klass->save_draft = save_draft;
	klass->send_now = send_now;
	klass->send_later = send_later;
}

static void
init (C2Composer *composer)
{
	composer->type = 0;
	composer->cmnd = NULL;
	composer->action = 0;
	composer->autosave_id = -1;
	composer->file = NULL;
	composer->save_type = 0;
	composer->draft_id = -1;
	composer->eheaders = NULL;
}

static void
destroy (GtkObject *object)
{
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

	gtk_object_set_data (GTK_OBJECT (composer), MESSAGE_CONTENT_TYPE, "multipart/mixed");

	widget = glade_xml_get_widget (C2_WINDOW (composer)->xml, "attachments_label");
	gtk_widget_show (widget);
	widget = glade_xml_get_widget (C2_WINDOW (composer)->xml, "attachments_container");
	gtk_widget_show (widget);
}

static void
autosave (C2Composer *composer)
{
	gint id;
	
	/* This is how it will work
	 * is storing its mails in a spool mailbox in
	 * ~/.c2/autosave
	 */
}

static void
save (C2Composer *composer)
{
	if (composer->save_type == C2_COMPOSER_SAVE_DRAFT)
		C2_COMPOSER_CLASS_FW (composer)->save_draft (composer);
	else
		C2_COMPOSER_CLASS_FW (composer)->save_file (composer, composer->file);
}

static void
save_draft_thread (C2Composer *composer)
{
	C2Mailbox *mailbox;
	C2Message *message;

	composer->save_type = C2_COMPOSER_SAVE_DRAFT;

	mailbox = c2_mailbox_get_by_name (C2_WINDOW (composer)->application->mailbox, C2_MAILBOX_DRAFTS);
	if (!mailbox)
	{
		gdk_threads_enter ();
		c2_window_report (C2_WINDOW (composer), C2_WINDOW_REPORT_WARNING,
							error_list[C2_FAIL_MESSAGE_SAVE], _("Unable to find the proper mailbox"));
		gdk_threads_leave ();
		return;
	}

	gdk_threads_enter ();
	message = create_message (composer);
	gdk_threads_leave ();

	if (composer->draft_id >= 0)
		c2_db_message_remove_by_mid (mailbox, composer->draft_id);
	
	c2_db_message_add (mailbox, message);
	composer->draft_id = mailbox->db->prev->mid;

	gdk_threads_enter ();
	c2_window_report (C2_WINDOW (composer), C2_WINDOW_REPORT_MESSAGE, error_list[C2_SUCCESS_MESSAGE_SAVE]);
	gdk_threads_leave ();
}

static void
save_draft (C2Composer *composer)
{
	pthread_t thread;

	pthread_create (&thread, NULL, C2_PTHREAD_FUNC (save_draft_thread), composer);
}

static void
save_file (C2Composer *composer, gchar *efile)
{
	GtkWidget *dialog;
	gchar *buf, *file;
	C2Message *message;
	FILE *fd;

	if (!efile)
	{
		if (!(fd = c2_application_dialog_select_file_save (C2_WINDOW (composer)->application, &file)))
		{
			c2_window_report (C2_WINDOW (composer), C2_WINDOW_REPORT_WARNING,
								error_list[C2_FAIL_FILE_SAVE], c2_error_get ());
			return;
		}
	} else
	{
		file = efile;

		if (!(fd = fopen (efile, "w")))
		{
			c2_error_set (-errno);
			c2_window_report (C2_WINDOW (composer), C2_WINDOW_REPORT_WARNING,
								error_list[C2_FAIL_FILE_SAVE], c2_error_get ());
			return;
		}
	}

	composer->file = file;
	composer->save_type = C2_COMPOSER_SAVE_FILE;

	message = create_message (composer);

	fprintf (fd, "%s\n"
				 "\n"
				 "%s", message->header, message->body);
	fclose (fd);

	gtk_object_destroy (GTK_OBJECT (message));

	c2_window_report (C2_WINDOW (composer), C2_WINDOW_REPORT_MESSAGE, error_list[C2_SUCCESS_FILE_SAVE]);
}

static void
send_ (C2Composer *composer, C2ComposerSendType type)
{
	C2Message *message;
	C2Mailbox *mailbox;
	GladeXML *xml;
	gchar *buf;

	gdk_threads_enter ();
	message = create_message (composer);
	gdk_threads_leave ();

	/* Set the Send Now state of the message */
	buf = message->header;
	message->header = g_strdup_printf ("%s\n"
									   "X-CronosII-Send-Type: %d\n",
									   message->header, type);

	mailbox = c2_mailbox_get_by_name (C2_WINDOW (composer)->application->mailbox, C2_MAILBOX_OUTBOX);
	if (!mailbox)
		g_assert_not_reached ();

	c2_db_freeze (mailbox);
	if (!c2_db_message_add (mailbox, message))
	{
		c2_db_thaw (mailbox);

		gdk_threads_enter ();
		gtk_widget_destroy (GTK_WIDGET (composer));
		gdk_threads_leave ();
	} else
	{
		c2_db_thaw (mailbox);
		
		/* Make the send buttons sensitive again */
	}
}

static void
send_now_thread (C2Composer *composer)
{
	send_ (composer, C2_COMPOSER_SEND_NOW);
}

static void
send_now (C2Composer *composer)
{
	pthread_t thread;
	
	/* TODO Set all possible way to send this message (now or later)
	 * insensitive */

	pthread_create (&thread, NULL, C2_PTHREAD_FUNC (send_now_thread), composer);
}

static void
send_later_thread (C2Composer *composer)
{
	send_ (composer, C2_COMPOSER_SEND_LATER);
}

static void
send_later (C2Composer *composer)
{
	pthread_t thread;
	
	/* TODO Set all possible way to send this message (now or later)
	 * insensitive */

	pthread_create (&thread, NULL, C2_PTHREAD_FUNC (send_later_thread), composer);
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
	GtkWidget *editor_container;
	GtkWidget *widget;
	gchar *str, *buf;
	GladeXML *xml;
	
	c2_window_construct (C2_WINDOW (composer), application, _("Composer: Untitled"), "composer");
	C2_WINDOW (composer)->xml = glade_xml_new (C2_APPLICATION_GLADE_FILE (XML_FILE),
								"wnd_composer_contents");
	xml = C2_WINDOW (composer)->xml;
#ifdef USE_GNOME_WINDOW_ICON
	gnome_window_icon_set_from_file (GTK_WINDOW (composer), PKGDATADIR "/pixmaps/mail-write.png");
#endif
	gtk_widget_set_usize (GTK_WIDGET (composer), GET_WINDOW_WIDTH, GET_WINDOW_HEIGHT);
	gtk_window_set_policy (GTK_WINDOW (composer), TRUE, TRUE, FALSE);

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
	widget = glade_xml_get_widget (xml, "format_html");
#ifndef USE_ADVANCED_EDITOR
	gtk_widget_set_sensitive (widget, FALSE);
#endif

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
		composer->type = C2_COMPOSER_TYPE_EXTERNAL;

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
	} else
		composer->type = C2_COMPOSER_TYPE_INTERNAL;

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
	editor_container = glade_xml_get_widget (xml, "editor_container");

	if (c2_streq (buf, "external"))
	{
		GtkWidget *viewport;
		GtkWidget *alignment;
		GtkWidget *frame;
		GtkWidget *vbox, *label, *button, *hbox, *pixmap;
		
		viewport = gtk_viewport_new (NULL, NULL);
		gtk_box_pack_start (GTK_BOX (editor_container), viewport, TRUE, TRUE, 0);
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
	} else
	{ /* Create the internal widget */
		composer->editor = c2_editor_new ();
#ifdef USE_ADVANCED_EDITOR
#else
		C2_EDITOR (composer->editor)->font = application->fonts_gdk_composer_body;
#endif
		gtk_box_pack_start (GTK_BOX (editor_container), composer->editor, TRUE, TRUE, 0);
		gtk_widget_show (composer->editor);

		gtk_signal_connect (GTK_OBJECT (composer->editor), "undo_available",
							GTK_SIGNAL_FUNC (on_editor_undo_available), composer);
		gtk_signal_connect (GTK_OBJECT (composer->editor), "redo_available",
							GTK_SIGNAL_FUNC (on_editor_redo_available), composer);
	}

	g_free (buf);

	/* Connect signals */
	widget = glade_xml_get_widget (xml, "file_send_now");
	gtk_signal_connect (GTK_OBJECT (widget), "activate",
						GTK_SIGNAL_FUNC (on_send_now_clicked), composer);
	widget = glade_xml_get_widget (xml, "file_save_as");
	gtk_signal_connect (GTK_OBJECT (widget), "activate",
						GTK_SIGNAL_FUNC (on_save_as_clicked), composer);
	widget = glade_xml_get_widget (xml, "file_save_draft");
	gtk_signal_connect (GTK_OBJECT (widget), "activate",
						GTK_SIGNAL_FUNC (on_save_draft_clicked), composer);
	
	widget = glade_xml_get_widget (xml, "edit_undo");
	gtk_signal_connect (GTK_OBJECT (widget), "activate",
						GTK_SIGNAL_FUNC (on_undo_clicked), composer);
	widget = glade_xml_get_widget (xml, "edit_redo");
	gtk_signal_connect (GTK_OBJECT (widget), "activate",
						GTK_SIGNAL_FUNC (on_redo_clicked), composer);
	
	widget = glade_xml_get_widget (xml, "insert_attachment");
	gtk_signal_connect (GTK_OBJECT (widget), "activate",
						GTK_SIGNAL_FUNC (on_attachments_clicked), composer);
	

	
	widget = glade_xml_get_widget (xml, "send_now_btn");
	gtk_signal_connect (GTK_OBJECT (widget), "clicked",
						GTK_SIGNAL_FUNC (on_send_now_clicked), composer);
	widget = glade_xml_get_widget (xml, "send_later_btn");
	gtk_signal_connect (GTK_OBJECT (widget), "clicked",
						GTK_SIGNAL_FUNC (on_send_later_clicked), composer);

	widget = glade_xml_get_widget (xml, "save_btn");
	gtk_signal_connect (GTK_OBJECT (widget), "clicked",
						GTK_SIGNAL_FUNC (on_save_clicked), composer);
	
	widget = glade_xml_get_widget (xml, "undo_btn");
	gtk_signal_connect (GTK_OBJECT (widget), "clicked",
						GTK_SIGNAL_FUNC (on_undo_clicked), composer);
	widget = glade_xml_get_widget (xml, "redo_btn");
	gtk_signal_connect (GTK_OBJECT (widget), "clicked",
						GTK_SIGNAL_FUNC (on_redo_clicked), composer);
	
	widget = glade_xml_get_widget (xml, "attach_btn");
	gtk_signal_connect (GTK_OBJECT (widget), "clicked",
						GTK_SIGNAL_FUNC (on_attachments_clicked), composer);

	widget = glade_xml_get_widget (xml, "to");
	gtk_signal_connect (GTK_OBJECT (widget), "changed",
						GTK_SIGNAL_FUNC (on_to_changed), composer);
	
	/* Grab the focus */
	gtk_widget_grab_focus (widget);

	widget = GTK_COMBO (glade_xml_get_widget (xml, "subject"))->entry;
	gtk_signal_connect (GTK_OBJECT (widget), "changed",
						GTK_SIGNAL_FUNC (on_subject_changed), composer);
	
	gtk_signal_connect (GTK_OBJECT (composer), "size_allocate",
							GTK_SIGNAL_FUNC (on_composer_size_allocate), NULL);
	gtk_signal_connect_object (GTK_OBJECT (composer), "destroy",
							GTK_SIGNAL_FUNC (destroy), NULL);
}

static void
on_composer_size_allocate (C2Composer *composer, GtkAllocation *alloc)
{
	SET_WINDOW_WIDTH (alloc->width);
	SET_WINDOW_HEIGHT (alloc->height);
	gnome_config_sync ();
}

static gboolean
on_composer_delete_event (GtkWidget *widget, GdkEventAny *e, C2Composer *composer)
{
	return TRUE;
}

static const gchar *
get_pixmap (const gchar *filename)
{
	const gchar *type = gnome_mime_type_or_default (filename, "");
	const gchar *pixmap = type || !strlen (type) ? gnome_mime_get_value (type, "icon-filename") : NULL;

	return pixmap;
}

static void
on_to_changed (GtkWidget *widget, C2Composer *composer)
{
	GladeXML *xml;
	gboolean sensitive;
	GList *list, *l;

	/* Get emails */
	list = c2_str_get_emails (gtk_entry_get_text (GTK_ENTRY (widget)));
	if (!c2_str_are_emails (list))
		sensitive = FALSE;
	else
		sensitive = TRUE;
	
	/* Free no-longer-used data */
	for (l = list; l; l = g_list_next (l))
		g_free (l->data);
	g_list_free (list);
	
	xml = C2_WINDOW (composer)->xml;

	gtk_widget_set_sensitive (glade_xml_get_widget (xml, "file_send_now"), sensitive);
	gtk_widget_set_sensitive (glade_xml_get_widget (xml, "file_send_later"), sensitive);
	gtk_widget_set_sensitive (glade_xml_get_widget (xml, "send_now_btn"), sensitive);
	gtk_widget_set_sensitive (glade_xml_get_widget (xml, "send_later_btn"), sensitive);
}

static void
on_subject_changed (GtkWidget *widget, C2Composer *composer)
{
	const gchar *subject;

	subject = gtk_entry_get_text (GTK_ENTRY (widget));
	
	if (strlen (subject))
	{
		gchar *title;
		
		title = g_strdup_printf (_("Composer: %s"), subject);
		gtk_window_set_title (GTK_WINDOW (composer), title);
		g_free (title);
	} else
		gtk_window_set_title (GTK_WINDOW (composer), _("Composer: Untitled"));
}

static void
on_editor_undo_available (GtkWidget *widget, gboolean available, C2Composer *composer)
{
	widget = glade_xml_get_widget (C2_WINDOW (composer)->xml, "undo_btn");
	gtk_widget_set_sensitive (widget, available);
	widget = glade_xml_get_widget (C2_WINDOW (composer)->xml, "edit_undo");
	gtk_widget_set_sensitive (widget, available);
}

static void
on_editor_redo_available (GtkWidget *widget, gboolean available, C2Composer *composer)
{
	widget = glade_xml_get_widget (C2_WINDOW (composer)->xml, "redo_btn");
	gtk_widget_set_sensitive (widget, available);
	widget = glade_xml_get_widget (C2_WINDOW (composer)->xml, "edit_redo");
	gtk_widget_set_sensitive (widget, available);
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
		ACCOUNT_ENTRY (account, str);
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

	filename = gtk_object_get_data (GTK_OBJECT (composer), EXTERNAL_EDITOR_FILE);
	if (!filename)
	{
		filename = c2_get_tmp_file ("c2-editor-XXXXXX");
		gtk_object_set_data (GTK_OBJECT (composer), EXTERNAL_EDITOR_FILE, filename);
	}

	cmnd = gnome_config_get_string ("/"PACKAGE"/Interface-Composer/editor_external_cmnd");

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
on_save_clicked (GtkWidget *widget, C2Composer *composer)
{
	C2_COMPOSER_CLASS_FW (composer)->save (composer);
}

static void
on_save_draft_clicked (GtkWidget *widget, C2Composer *composer)
{
	C2_COMPOSER_CLASS_FW (composer)->save_draft (composer);
}

static void
on_save_as_clicked (GtkWidget *widget, C2Composer *composer)
{
	C2_COMPOSER_CLASS_FW (composer)->save_file (composer, NULL);
}

static void
on_send_now_clicked (GtkWidget *widget, C2Composer *composer)
{
	C2_COMPOSER_CLASS_FW (composer)->send_now (composer);
}

static void
on_send_later_clicked (GtkWidget *widget, C2Composer *composer)
{
	C2_COMPOSER_CLASS_FW (composer)->send_later (composer);
}

static void
on_undo_clicked (GtkWidget *widget, C2Composer *composer)
{
	c2_editor_operations_undo (C2_EDITOR (composer->editor));
}

static void
on_redo_clicked (GtkWidget *widget, C2Composer *composer)
{
	c2_editor_operations_redo (C2_EDITOR (composer->editor));
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
		gtk_object_remove_data (GTK_OBJECT (composer), MESSAGE_CONTENT_TYPE);
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

void
c2_composer_set_action (C2Composer *composer, C2ComposerAction action)
{
	composer->action = action;
}

static C2Account *
set_message_account (C2Composer *composer, C2Message *message)
{
	C2Account *account;
	GtkWidget *widget;
	gchar *buf, *buf2;

	buf = c2_message_get_header_field (message, "\nX-CronosII-Account:");
	account = c2_account_get_by_name (C2_WINDOW (composer)->application->account, buf);
	if (!account)
	{
		if (buf)
		{
			c2_window_report (C2_WINDOW (composer), C2_WINDOW_REPORT_WARNING,
								_("The account specified does not longer exist: %s"), buf);
			return NULL;
		} else
		{
			c2_window_report (C2_WINDOW (composer), C2_WINDOW_REPORT_WARNING,
							_("The message is not asociated to an account, default account will be used."));
			return C2_WINDOW (composer)->application->account;
		}
	} else
	{
		ACCOUNT_ENTRY (account, buf2);
		widget = glade_xml_get_widget (C2_WINDOW (composer)->xml, "account");
		gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (widget)->entry), buf2);
		g_free (buf2);
	}
	g_free (buf);

	return account;
}

static void
set_message_to (C2Composer *composer, C2Message *message)
{
	GtkWidget *widget;
	gchar *buf;
	
	switch (composer->action)
	{
		case C2_COMPOSER_ACTION_REPLY:
		case C2_COMPOSER_ACTION_REPLY_ALL:
			if (!(buf = c2_message_get_header_field (message, "\nReply-To:")))
				buf = c2_message_get_header_field (message, "\nFrom:");
			widget = glade_xml_get_widget (C2_WINDOW (composer)->xml, "to");
			gtk_entry_set_text (GTK_ENTRY (widget), buf);
			g_free (buf);
			break;
		case C2_COMPOSER_ACTION_DRAFT:
			buf = c2_message_get_header_field (message, "\nTo:");
			widget = glade_xml_get_widget (C2_WINDOW (composer)->xml, "to");
			gtk_entry_set_text (GTK_ENTRY (widget), buf);
			g_free (buf);
			break;
	}
}

static void
set_message_cc (C2Composer *composer, C2Message *message)
{
	GtkWidget *widget;
	gchar *buf;
	
	switch (composer->action)
	{
		case C2_COMPOSER_ACTION_REPLY_ALL:
		case C2_COMPOSER_ACTION_DRAFT:
			buf = c2_message_get_header_field (message, "\nCC:");
			widget = glade_xml_get_widget (C2_WINDOW (composer)->xml, "cc");
			gtk_entry_set_text (GTK_ENTRY (widget), buf);
			g_free (buf);
			break;
	}
}

static void
set_message_bcc (C2Composer *composer, C2Message *message)
{
	GtkWidget *widget;
	gchar *buf;
	
	switch (composer->action)
	{
		case C2_COMPOSER_ACTION_REPLY_ALL:
		case C2_COMPOSER_ACTION_DRAFT:
			buf = c2_message_get_header_field (message, "\nBCC:");
			widget = glade_xml_get_widget (C2_WINDOW (composer)->xml, "bcc");
			gtk_entry_set_text (GTK_ENTRY (widget), buf);
			g_free (buf);
			break;
	}
}

static void
set_message_subject (C2Composer *composer, C2Message *message)
{
	GtkWidget *widget;
	gchar *buf, *buf2;

	buf2 = c2_message_get_header_field (message, "\nSubject:");
	widget = glade_xml_get_widget (C2_WINDOW (composer)->xml, "subject");
	
	switch (composer->action)
	{
		case C2_COMPOSER_ACTION_REPLY:
		case C2_COMPOSER_ACTION_REPLY_ALL:
			buf = "Re: ";
			break;
		case C2_COMPOSER_ACTION_FORWARD:
			buf = "Fwd: ";
			break;
		case C2_COMPOSER_ACTION_DRAFT:
			buf = "";
			break;
	}

	buf = g_strdup_printf ("%s%s", buf, buf2 ? buf2 : "");
	gtk_entry_set_text (GTK_ENTRY (gnome_entry_gtk_entry (GNOME_ENTRY (widget))), buf);
	g_free (buf2);
	g_free (buf);
}

static void
set_message_in_reply_to (C2Composer *composer, C2Message *message)
{
	gchar *buf, *buf2;

	buf = c2_message_get_header_field (message, "\nMessage-ID:");
	if (!buf)
		return;

	c2_composer_set_extra_field (composer, "In-Reply-To", buf);
	g_free (buf);
}

static void
set_message_references (C2Composer *composer, C2Message *message)
{
	gchar *id, *ref, *buf;

	id = c2_message_get_header_field (message, "\nMessage-ID:");
	if (!id)
		return;
	
	ref = c2_message_get_header_field (message, "\nReferences:");

	/* I don't like the cutting of the references header that some
	 * mail clients seems to do, thus why this will not do it
	 */
	buf = g_strdup_printf ("%s%s%s", ref ? ref : "", ref ? " " : "", id);

	c2_composer_set_extra_field (composer, "References", buf);
	g_free (buf);
}

static void
set_message_attachments (C2Composer *composer, C2Message *message)
{
	C2Mime *mime;
	
	/* Attachments */
	switch (composer->action)
	{
		case C2_COMPOSER_ACTION_REPLY:
		case C2_COMPOSER_ACTION_REPLY_ALL:
		case C2_COMPOSER_ACTION_FORWARD:
		case C2_COMPOSER_ACTION_DRAFT:
				
			/* Go throught the different MIME parts */
			for (mime = message->mime; mime; mime = mime->next)
			{
				/* Save the part to a file */
				
				/* Attach the file */
			}

			break;
	}
}

static void
set_message_body_internal_freeze (C2Composer *composer)
{
	c2_editor_freeze (C2_EDITOR (composer->editor));
}

static void
set_message_body_external_freeze (C2Composer *composer)
{
	FILE *fd;
	gchar *file;

	file = c2_get_tmp_file ("c2-editor-XXXXXX");
	if (!(fd = fopen (file, "w")))
	{
		c2_error_object_set (GTK_OBJECT (composer), -errno);
		c2_window_report (C2_WINDOW (composer), C2_WINDOW_REPORT_WARNING,
							_("Unable to open temporal file for writing to external editor: %s"),
							c2_error_object_get (GTK_OBJECT (composer)));
		return;
	}

	gtk_object_set_data (GTK_OBJECT (composer), "composer::fd", fd);
}

static void
set_message_body_internal_append (C2Composer *composer, const gchar *body, gint r, gint g, gint b)
{
	c2_editor_append (C2_EDITOR (composer->editor), body, r, g, b);
}

static void
set_message_body_external_append (C2Composer *composer, const gchar *body, gint r, gint g, gint b)
{
	FILE *fd;

	fd = (FILE*) gtk_object_get_data (GTK_OBJECT (composer), "composer::fd");
	if (fd)
		fwrite (body, sizeof (gchar), strlen (body), fd);
}

static void
set_message_body_internal_thaw (C2Composer *composer)
{
	c2_editor_thaw (C2_EDITOR (composer->editor));
}

static void
set_message_body_external_thaw (C2Composer *composer)
{
	FILE *fd;

	fd = (FILE*) gtk_object_get_data (GTK_OBJECT (composer), "composer::fd");
	if (fd)
		fclose (fd);
}

static void
set_message_body (C2Composer *composer, C2Message *message)
{
	gchar *ptr, *partbody, *buf;
	gchar *quote_char = c2_preferences_get_interface_composer_quote_character ();
	C2Mime *part;
	gint r, g, b;
	void (*freeze) (C2Composer *);
	void (*append) (C2Composer *, const gchar *, gint, gint, gint);
	void (*thaw) (C2Composer *);

	if (composer->type == C2_COMPOSER_TYPE_INTERNAL)
	{
		freeze = set_message_body_internal_freeze;
		append = set_message_body_internal_append;
		thaw = set_message_body_internal_thaw;
	} else
	{
		freeze = set_message_body_external_freeze;
		append = set_message_body_external_append;
		thaw = set_message_body_external_thaw;
	}

	freeze (composer);
	c2_preferences_get_interface_composer_quote_color (r, g, b);

	/* Ground Zero: Let's write an empty line where the
	 * user might write something and then put the quote
	 * line
	 */
	/* TODO Signature should be appended somewhere around here */
	if (composer->action != C2_COMPOSER_ACTION_DRAFT)
	{
		struct tm *tm;
		time_t date;
		gchar tbuf[128];
		gchar *date_fmt, *from;
		
		append (composer, "\n\n", 0, 0, 0);
	
		if (!(buf = c2_message_get_header_field (message, "\nDate:")))
		{
			date = c2_date_parse (buf);
			g_free (buf);
		} else
			date = time (NULL);

		date_fmt = c2_preferences_get_interface_misc_date ();
		tm = localtime (&date);
		strftime (tbuf, 128, date_fmt, tm);
		g_free (date_fmt);

		from = c2_message_get_header_field (message, "\nFrom:");

		buf = g_strdup_printf (_("On %s, %s wrote:\n"), tbuf, from);
		append (composer, buf, 0, 0, 0);
		g_free (buf);
		g_free (from);
	}
	
	/* First learn which is the right body (HTML or PLAIN) */
	/* We will use the HTML part of the message to reply
	 * when the message has an HTML part and when the composer
	 * has support for HTML, if something of this is not true
	 * we use the PLAIN part */
#ifdef USE_ADVANCED_EDITOR
	part = c2_mime_get_part_by_content_type (message->mime, "text/html");
	if (!part)
		/* There's no HTML part */
		part = c2_mime_get_part_by_content_type (message->mime, "text/plain");
	
#else
	part = c2_mime_get_part_by_content_type (message->mime, "text/plain");
	/* TODO If there's no text/plain we should check if there's
	 * a text/html and use a function to convert HTML to PLAIN
	 */
	
#endif
	
	/* There is a chance that we don't have a part (there's no
	 * text/html or text/plain in the message), if we don't
	 * have a part yet, we will just use an empty body.
	 */
	if (part)
	{
		gchar cbuf;
		gboolean newline;

		partbody = c2_str_wrap (part->part, 75);
		g_free (part->part);
		part->part = NULL;

		/* TODO I think this would be better
		 * if it uses a buffer of some length, say 80,
		 * and fills that buffer before appending.
		 */

		newline = TRUE;
		for (ptr = partbody;; ptr++)
		{
			if (*ptr == '\0')
				break;

			/* If it is a new line we have to append the
			 * quote character.
			 */
			if (newline)
			{
				append (composer, quote_char, r, g, b);
				newline = FALSE;
			}

			if (*ptr == '\n')
				newline = TRUE;

			cbuf = *(ptr+1);
			*(ptr+1) = '\0';
			append (composer, ptr, r, g, b);
			*(ptr+1) = cbuf;
		}
		g_free (partbody);
	}

	g_free (quote_char);

	thaw (composer);
}

/**
 * c2_composer_set_message_as_reply
 * @composer: The composer object.
 * @message: Message to be used.
 *
 * This function will put the message
 * in the composer as a reply.
 **/
void
c2_composer_set_message_as_reply (C2Composer *composer, C2Message *message)
{
	C2Account *account;
	
	c2_return_if_fail (C2_IS_COMPOSER (composer), C2EDATA);
	c2_return_if_fail_obj (C2_IS_MESSAGE (message), C2EDATA, GTK_OBJECT (composer));
	
	composer->action = C2_COMPOSER_ACTION_REPLY;

	account = set_message_account (composer, message);
	set_message_to (composer, message);
	set_message_cc (composer, message);
	set_message_bcc (composer, message);
	set_message_subject (composer, message);
	set_message_in_reply_to (composer, message);
	set_message_references (composer, message);
	set_message_body (composer, message);

	gtk_widget_grab_focus (C2_EDITOR (composer->editor)->text);
}

/**
 * c2_composer_set_message_as_reply_all
 * @composer: The composer object.
 * @message: Message to be used.
 *
 * This function will put the message
 * in the composer as a reply all.
 **/
void
c2_composer_set_message_as_reply_all (C2Composer *composer, C2Message *message)
{
	C2Account *account;
	
	c2_return_if_fail (C2_IS_COMPOSER (composer), C2EDATA);
	c2_return_if_fail_obj (C2_IS_MESSAGE (message), C2EDATA, GTK_OBJECT (composer));
	
	composer->action = C2_COMPOSER_ACTION_REPLY_ALL;

	account = set_message_account (composer, message);
	set_message_to (composer, message);
	set_message_cc (composer, message);
	set_message_bcc (composer, message);
	set_message_subject (composer, message);
	set_message_in_reply_to (composer, message);
	set_message_references (composer, message);
	set_message_body (composer, message);

	gtk_widget_grab_focus (C2_EDITOR (composer->editor)->text);
}

/**
 * c2_composer_set_message_as_forward
 * @composer: The composer object.
 * @message: Message to be used.
 *
 * This function will put the message
 * in the composer as a forward.
 **/
void
c2_composer_set_message_as_forward (C2Composer *composer, C2Message *message)
{
	C2Account *account;
	
	c2_return_if_fail (C2_IS_COMPOSER (composer), C2EDATA);
	c2_return_if_fail_obj (C2_IS_MESSAGE (message), C2EDATA, GTK_OBJECT (composer));
	
	composer->action = C2_COMPOSER_ACTION_FORWARD;

	account = set_message_account (composer, message);
	set_message_cc (composer, message);
	set_message_bcc (composer, message);
	set_message_subject (composer, message);
	set_message_in_reply_to (composer, message);
	set_message_references (composer, message);
	set_message_body (composer, message);
}

void
c2_composer_set_message_as_quote (C2Composer *composer, C2Message *message)
{
	GtkWidget *widget;
	gchar *buf, *buf2, *body;
	C2Account *account;
	FILE *fd;
	
	c2_return_if_fail_obj (C2_IS_MESSAGE (message), C2EDATA, GTK_OBJECT (composer));
	
	
	
	/* Body */
	switch (composer->action)
	{
		case C2_COMPOSER_ACTION_REPLY:
		case C2_COMPOSER_ACTION_REPLY_ALL:
		case C2_COMPOSER_ACTION_FORWARD:
			
		case C2_COMPOSER_ACTION_DRAFT:
	}
	
	//string = create_quoted_message (message);
	
	if (composer->type == C2_COMPOSER_TYPE_INTERNAL)
	{
		
	} else
	{
		FILE *fd;
		gchar *file;

//		file = (gchar*) gtk_object_get_data (GTK_OBJECT (composer), );

		if (!(fd = fopen (file, "a")))
		{
			c2_window_report (C2_WINDOW (composer), C2_WINDOW_REPORT_WARNING, "%s: %s",
								g_strerror (errno), file);
			return;
		}
	}
}

static C2Message *
create_message (C2Composer *composer)
{
	GladeXML *xml;
	GtkWidget *widget;
	C2Message *message, *fmessage;
	C2Account *account;
	GString *header;
	C2Mime *mime;
	gboolean html, multipart;
	gchar *buf, *buf1, *buf2;
	gint i;

	/* Create message */
	message = c2_message_new ();

	xml = C2_WINDOW (composer)->xml;
	
	/* Prepare header */
	header = g_string_new (NULL);

	/* Account */
	widget = glade_xml_get_widget (xml, "account");
	account = get_account (composer);
	buf1 = (gchar*) c2_account_get_extra_data (account, C2_ACCOUNT_KEY_FULL_NAME, NULL);
	if (buf1)
		buf = g_strdup_printf ("From: %s <%s>\n", buf1, account->email);
	else
		buf = g_strdup (account->email);
	g_string_append (header, buf);
	g_free (buf);

	/* To */
	widget = glade_xml_get_widget (xml, "to");
	buf1 = gtk_entry_get_text (GTK_ENTRY (widget));
	buf = g_strdup_printf ("To: %s\n", buf1);
	g_string_append (header, buf);
	g_free (buf);

	/* CC */
	widget = glade_xml_get_widget (xml, "cc");
	buf1 = gtk_entry_get_text (GTK_ENTRY (widget));
	if (strlen (buf1))
	{
		buf = g_strdup_printf ("CC: %s\n", buf1);
		g_string_append (header, buf);
		g_free (buf);
	}

	/* BCC */
	widget = glade_xml_get_widget (xml, "bcc");
	buf1 = gtk_entry_get_text (GTK_ENTRY (widget));
	if (strlen (buf1))
	{
		buf = g_strdup_printf ("BCC: %s\n", buf1);
		g_string_append (header, buf);
		g_free (buf);
	}

	/* Subject */
	widget = glade_xml_get_widget (xml, "subject");
	buf1 = gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (widget)->entry));
	buf = g_strdup_printf ("Subject: %s\n", buf1);
	g_string_append (header, buf);
	g_free (buf);

	/* Reply-To */
	buf1 = (gchar*) c2_account_get_extra_data (account, C2_ACCOUNT_KEY_REPLY_TO, NULL);
	if (strlen (buf1))
	{
		buf = g_strdup_printf ("Reply-To: %s\n", buf1);
		g_string_append (header, buf);
		g_free (buf);
	}

	/* Organization */
	buf1 = (gchar*) c2_account_get_extra_data (account, C2_ACCOUNT_KEY_ORGANIZATION, NULL);
	if (strlen (buf1))
	{
		buf = g_strdup_printf ("Organization: %s\n", buf1);
		g_string_append (header, buf);
		g_free (buf);
	}

	/* Check if message is in HTML or PLAIN */
	widget = glade_xml_get_widget (C2_WINDOW (composer)->xml, "format_html");
	if (GTK_CHECK_MENU_ITEM (widget)->active)
		html = TRUE;
	else
		html = FALSE;

	/* MIME-Version */
	g_string_append (header, "MIME-Version: 1.0\n");
		
	/* Content-Type */
	buf1 = (gchar*) gtk_object_get_data (GTK_OBJECT (composer), MESSAGE_CONTENT_TYPE);
	if (!buf1 || !strlen (buf1))
	{
		/* This can be text/plain or multipart/alternative */
		if (html)
			buf1 = g_strdup ("Content-Type: multipart/alternative\n");
		else
			buf1 = g_strdup ("Content-Type: text/plain\n");
		g_string_append (header, buf1);
		g_free (buf1);
	} else
	{
		buf = g_strdup_printf ("Content-Type: %s\n", buf1);
		g_string_append (header, buf);
		g_free (buf);
	}

	/* X-Mailer
	 * User-Agent
	 */
	buf1 = g_strdup_printf ("X-Mailer: " USE_XMAILER "\n"
							"User-Agent: Cronos II " VERSION "\n");
	g_string_append (header, buf1);
	g_free (buf1);

	/* X-Priority */
	widget = glade_xml_get_widget (xml, "priority_very_high");
	if (GTK_CHECK_MENU_ITEM (glade_xml_get_widget (xml, "priority_very_high"))->active)
		buf1 = g_strdup_printf ("X-Priority: 1\n");
	else if (GTK_CHECK_MENU_ITEM (glade_xml_get_widget (xml, "priority_high"))->active)
		buf1 = g_strdup_printf ("X-Priority: 2\n");
	else if (GTK_CHECK_MENU_ITEM (glade_xml_get_widget (xml, "priority_normal"))->active)
		buf1 = g_strdup_printf ("X-Priority: 3\n");
	else if (GTK_CHECK_MENU_ITEM (glade_xml_get_widget (xml, "priority_low"))->active)
		buf1 = g_strdup_printf ("X-Priority: 4\n");
	else if (GTK_CHECK_MENU_ITEM (glade_xml_get_widget (xml, "priority_very_low"))->active)
		buf1 = g_strdup_printf ("X-Priority: 5\n");
	else
		buf1 = g_strdup_printf ("X-Priority: 3\n");
	g_string_append (header, buf1);
	g_free (buf1);

	/* X-CronosII-Account */
	buf1 = g_strdup_printf ("X-CronosII-Account: %s\n", account->name);
	g_string_append (header, buf1);
	g_free (buf1);

	/* X-CronosII-State */
	buf1 = g_strdup_printf ("X-CronosII-State: %d\n", C2_MESSAGE_UNREADED);
	g_string_append (header, buf1);
	g_free (buf1);


	/* Body */
	if (composer->type == C2_COMPOSER_TYPE_INTERNAL)
	{
		buf = c2_editor_get_text (C2_EDITOR (composer->editor));
	} else
	{
		struct stat stat_buf;
		FILE *fd;
		gint length;

		buf = (gchar*) gtk_object_get_data (GTK_OBJECT (composer), EXTERNAL_EDITOR_FILE);

		if (stat (buf, &stat_buf) < 0)
		{
			c2_error_object_set (GTK_OBJECT (composer), -errno);
			gtk_object_destroy (GTK_OBJECT (message));
			return NULL;
		}

		if (!(fd = fopen (buf, "r")))
		{
			c2_error_object_set (GTK_OBJECT (composer), -errno);
			gtk_object_destroy (GTK_OBJECT (message));
			return NULL;
		}

		length = ((gint) stat_buf.st_size * sizeof (gchar));

		buf = g_new0 (gchar, length+1);
		fread (buf, sizeof (gchar), length, fd);
		fclose (fd);
	}

	/* Add a '\n' to the end of the body */
	i = strlen (buf);
	buf = g_realloc (buf, i+1);
	buf[i] = '\n';
	buf[i+1] = '\0';
	
	/* The body is the first attachment */
	mime = c2_mime_new (NULL);
	mime->part = buf;
	mime->type = g_strdup ("text");
	mime->subtype = g_strdup (!html ? "plain" : "html");
	mime->disposition = g_strdup ("inline");
	mime->encoding = g_strdup ("8bit");
	message->mime = c2_mime_append (message->mime, mime);

	/* Attachments */
	widget = glade_xml_get_widget (C2_WINDOW (composer)->xml, "attachments_list");
	for (i = 0; i < GNOME_ICON_LIST (widget)->icons; i++)
	{
		C2ComposerAttachment *attach = (C2ComposerAttachment*)
						gnome_icon_list_get_icon_data (GNOME_ICON_LIST (widget), i);
		C2Mime *mime;
		struct stat stat_buf;
		FILE *fd;
		gint length;

		if (!attach)
			continue;

		/* Create the MIME object */
		mime = c2_mime_new (NULL);

		/* Load the file into the mime object */
		if (!(fd = fopen (attach->file, "r")))
		{
			c2_error_object_set (GTK_OBJECT (composer), -errno);
			gtk_object_unref (GTK_OBJECT (mime));
			c2_window_report (C2_WINDOW (composer), C2_WINDOW_REPORT_WARNING,
								_("Failed to open %s: %s"), attach->file,
								c2_error_object_get (GTK_OBJECT (composer)));
			c2_error_object_set (GTK_OBJECT (composer), C2SUCCESS);
			continue;
		}

		if (stat (attach->file, &stat_buf) < 0)
		{
			c2_error_object_set (GTK_OBJECT (composer), -errno);
			c2_window_report (C2_WINDOW (composer), C2_WINDOW_REPORT_WARNING,
								_("Failed to stat %s: %s"), attach->file,
								c2_error_object_get (GTK_OBJECT (composer)));
			gtk_object_destroy (GTK_OBJECT (mime));
			continue;
		}

		length = ((gint) stat_buf.st_size * sizeof (gchar));
		mime->part = g_new0 (gchar, length+1);
		fread (mime->part, sizeof (gchar), length, fd);
		fclose (fd);

		/* Set more info about the mime object */
		if (c2_strne (attach->type, _("unknown")))
			buf = g_strdup (attach->type );
		else
		{
autodefine_type:
			buf = g_strdup ("application/octet-stream");
		}
		
		if (!(buf1 = strchr (buf, '/')))
			goto autodefine_type;
		
		mime->type = g_strndup (buf, buf1-buf);
		mime->subtype = g_strdup (buf1);
		g_free (buf);
		
		buf = g_basename (attach->file);
		mime->disposition = g_strdup_printf ("attachment; filename=\"%s\"", buf);
		g_free (buf);

		mime->description = g_strdup (attach->description);
		mime->encoding = g_strdup ("base64");
		mime->length = length;

		message->mime = c2_mime_append (message->mime, mime);
	}

	message->header = header->str;
	g_string_free (header, FALSE);

	fmessage = c2_message_fix_broken_message (message);
	gtk_object_destroy (GTK_OBJECT (message));
	
	return fmessage;
}

static C2Account *
get_account (C2Composer *composer)
{
	C2Account *account;
	C2Application *application;
	GtkWidget *widget;
	gchar *text, *name;

	application = C2_WINDOW (composer)->application;

	widget = GTK_COMBO (glade_xml_get_widget (C2_WINDOW (composer)->xml, "account"))->entry;
	text = gtk_entry_get_text (GTK_ENTRY (widget));
	name = c2_str_get_enclosed_text_backward (text, '(', ')', 0);

	for (account = application->account; account; account = c2_account_next (account))
	{
		if (c2_streq (account->name, name))
			return account;
	}

	return NULL;
}

void
c2_composer_set_extra_field (C2Composer *composer, const gchar *field, const gchar *data)
{
	GtkWidget *widget;
	gchar *string;
	
	c2_return_if_fail_obj (C2_IS_COMPOSER (composer) || field, C2EDATA, GTK_OBJECT (composer));

	if (c2_streq (field, C2_COMPOSER_ACCOUNT))
	{
		C2Account *account;
		gchar *str;

		account = c2_account_get_by_name (C2_WINDOW (composer)->application->account, data);
		
		if (!account)
		{
			c2_window_report (C2_WINDOW (composer), C2_WINDOW_REPORT_WARNING,
								_("The account specified does not exist: %s"), data);
			return;
		}

		ACCOUNT_ENTRY (account, str);
		
		widget = glade_xml_get_widget (C2_WINDOW (composer)->xml, "account");
		gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (widget)->entry), str);
	} else if (c2_streq (field, C2_COMPOSER_TO))
	{
		widget = glade_xml_get_widget (C2_WINDOW (composer)->xml, "to");
		gtk_entry_set_text (GTK_ENTRY (widget), data);
	} else if (c2_streq (field, C2_COMPOSER_CC))
	{
		widget = glade_xml_get_widget (C2_WINDOW (composer)->xml, "cc");
		gtk_entry_set_text (GTK_ENTRY (widget), data);
	} else if (c2_streq (field, C2_COMPOSER_BCC))
	{
		widget = glade_xml_get_widget (C2_WINDOW (composer)->xml, "bcc");
		gtk_entry_set_text (GTK_ENTRY (widget), data);
	} else if (c2_streq (field, C2_COMPOSER_SUBJECT))
	{
		widget = glade_xml_get_widget (C2_WINDOW (composer)->xml, "subject");
		gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (widget)->entry), data);
	} else
	{
		gchar *string;
		
		string = g_strdup_printf ("%s: %s", field, data);
		composer->eheaders = g_list_append (composer->eheaders, string);
	}
}

/* This can be in the form of:
 * [mailto:]em@il[?[subject=Subject][&...]
 *               [?[cc=CC][&...]
 *               [?[bcc=BCC][&...]
 *               [?[contents=Body][&...]]
 */
void
c2_composer_set_contents_from_link (C2Composer *composer, const gchar *link)
{
	const gchar *ptr, *ptr2;
	gchar *buf, *subject;

	ptr = link;
	
	if (c2_strneq (link, "mailto:", 7))
		ptr += 7;

	ptr2 = strstr (ptr, "?");

	if (ptr2)
		buf = g_strndup (ptr, ptr2-ptr);
	else
		buf = g_strdup (ptr);

	c2_composer_set_extra_field (composer, C2_COMPOSER_TO, buf);
	g_free (buf);

	if (ptr2++)
		for (;;)
		{
			ptr = strstr (ptr2, "&");

			if (ptr)
				buf = g_strndup (ptr2, ptr-ptr2);
			else
				buf = g_strdup (ptr2);

			if (c2_strneq (buf, "subject=", 8))
				c2_composer_set_extra_field (composer, C2_COMPOSER_SUBJECT, buf+8);
			else if (c2_strneq (buf, "cc=", 3))
				c2_composer_set_extra_field (composer, C2_COMPOSER_CC, buf+3);
			else if (c2_strneq (buf, "bcc=", 4))
				c2_composer_set_extra_field (composer, C2_COMPOSER_BCC, buf+4);
			else if (c2_strneq (buf, "contents=", 9))
				c2_composer_set_extra_field (composer, C2_COMPOSER_BODY, buf+9);
			g_free (buf);
			
			if (!ptr)
				break;

			ptr2 = ptr+1;
		}
}
