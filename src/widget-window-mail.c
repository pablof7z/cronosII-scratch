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
#include "widget-mail.h"
#include "widget-toolbar.h"
#include "widget-window-mail.h"
#include "preferences.h"

#define TITLE "Message Viewer"

static void
class_init					(C2WindowMailClass *klass);

static void
init						(C2WindowMail *wmail);

static void
destroy						(C2WindowMail *wmail);

static void
on_toolbar_close_clicked					(GtkWidget *widget, C2WindowMail *wmail);

static void
on_toolbar_copy_clicked						(GtkWidget *widget, C2WindowMail *wmail);

static void
on_toolbar_delete_clicked					(GtkWidget *widget, C2WindowMail *wmail);

static void
on_toolbar_forward_clicked					(GtkWidget *widget, C2WindowMail *wmail);

static void
on_toolbar_move_clicked						(GtkWidget *widget, C2WindowMail *wmail);

static void
on_toolbar_next_clicked						(GtkWidget *widget, C2WindowMail *wmail);

static void
on_toolbar_previous_clicked					(GtkWidget *widget, C2WindowMail *wmail);

static void
on_toolbar_print_clicked					(GtkWidget *widget, C2WindowMail *wmail);

static void
on_toolbar_reply_clicked					(GtkWidget *widget, C2WindowMail *wmail);

static void
on_toolbar_reply_all_clicked				(GtkWidget *widget, C2WindowMail *wmail);

static void
on_toolbar_save_clicked						(GtkWidget *widget, C2WindowMail *wmail);

static void
on_toolbar_search_clicked					(GtkWidget *widget, C2WindowMail *wmail);

static void
set_sensitive								(C2WindowMail *wmail, gpointer data);

enum
{
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static C2WindowClass *parent_class = NULL;

static C2ToolbarItem toolbar_items[] = 
{
	{
		C2_TOOLBAR_BUTTON,
		"toolbar_search",
		N_("Search"), PKGDATADIR "/pixmaps/find.png",
		N_("Search a message in existent mailboxes"), TRUE,
		GTK_SIGNAL_FUNC (on_toolbar_search_clicked), NULL,
		NULL, NULL
	},
	{
		C2_TOOLBAR_SPACE, NULL, NULL, NULL, NULL, FALSE, NULL, NULL, NULL
	},
	{
		C2_TOOLBAR_BUTTON,
		"toolbar_save",
		N_("Save"), PKGDATADIR "/pixmaps/save.png",
		N_("Save selected message"), TRUE,
		GTK_SIGNAL_FUNC (on_toolbar_save_clicked), NULL,
		NULL, NULL
	},
	{
		C2_TOOLBAR_BUTTON,
		"toolbar_print",
		N_("Print"), PKGDATADIR "/pixmaps/print.png",
		N_("Print selected message"), TRUE,
		GTK_SIGNAL_FUNC (on_toolbar_print_clicked), NULL,
		NULL, NULL
	},
	{
		C2_TOOLBAR_SPACE, NULL, NULL, NULL, NULL, FALSE, NULL, NULL, NULL
	},
	{
		C2_TOOLBAR_BUTTON,
		"toolbar_delete",
		N_("Delete"), PKGDATADIR "/pixmaps/delete.png",
		N_("Delete selected mails"), TRUE,
		GTK_SIGNAL_FUNC (on_toolbar_delete_clicked), NULL,
		NULL, NULL
	},
	{
		C2_TOOLBAR_SPACE, NULL, NULL, NULL, NULL, FALSE, NULL, NULL, NULL
	},
	{
		C2_TOOLBAR_BUTTON,
		"toolbar_copy",
		N_("Copy"), PKGDATADIR "/pixmaps/copy-message.png",
		N_("Copy selected mails"), TRUE,
		GTK_SIGNAL_FUNC (on_toolbar_copy_clicked), NULL,
		NULL, NULL
	},
	{
		C2_TOOLBAR_BUTTON,
		"toolbar_move",
		N_("Move"), PKGDATADIR "/pixmaps/move-message.png",
		N_("Move selected mails"), TRUE,
		GTK_SIGNAL_FUNC (on_toolbar_move_clicked), NULL,
		NULL, NULL
	},
	{
		C2_TOOLBAR_SPACE, NULL, NULL, NULL, NULL, FALSE, NULL, NULL, NULL
	},
	{
		C2_TOOLBAR_BUTTON,
		"toolbar_reply",
		N_("Reply"), PKGDATADIR "/pixmaps/reply.png",
		N_("Reply selected message"), TRUE,
		GTK_SIGNAL_FUNC (on_toolbar_reply_clicked), NULL,
		NULL, NULL
	},
	{
		C2_TOOLBAR_BUTTON,
		"toolbar_reply_all",
		N_("Reply All"), PKGDATADIR "/pixmaps/reply-all.png",
		N_("Reply selected message to all recipients"), TRUE,
		GTK_SIGNAL_FUNC (on_toolbar_reply_all_clicked), NULL,
		NULL, NULL,
	},
	{
		C2_TOOLBAR_BUTTON,
		"toolbar_forward",
		N_("Forward"), PKGDATADIR "/pixmaps/forward.png",
		N_("Forward selected message"), TRUE,
		GTK_SIGNAL_FUNC (on_toolbar_forward_clicked), NULL,
		NULL, NULL
	},
	{
		C2_TOOLBAR_SPACE, NULL, NULL, NULL, NULL, FALSE, NULL, NULL, NULL
	},
	{
		C2_TOOLBAR_BUTTON,
		"toolbar_previous",
		N_("Previous"), PKGDATADIR "/pixmaps/prev.png",
		N_("Select previous message"), FALSE,
		GTK_SIGNAL_FUNC (on_toolbar_previous_clicked), NULL,
		NULL, NULL
	},
	{
		C2_TOOLBAR_BUTTON,
		"toolbar_next",
		N_("Next"), PKGDATADIR "/pixmaps/next.png",
		N_("Select next message"), FALSE,
		GTK_SIGNAL_FUNC (on_toolbar_next_clicked), NULL,
		NULL, NULL
	},
	{
		C2_TOOLBAR_SPACE, NULL, NULL, NULL, NULL, FALSE, NULL, NULL, NULL
	},
	{
		C2_TOOLBAR_BUTTON,
		"toolbar_close",
		N_("Close"), PKGDATADIR "/pixmaps/close.png",
		
		N_("Close the main window of Cronos II"), TRUE,
		GTK_SIGNAL_FUNC (on_toolbar_close_clicked), NULL,
		NULL, NULL
	}
};

GtkType
c2_window_mail_get_type (void)
{
	static GtkType type = 0;

	if (!type)
	{
		static GtkTypeInfo info =
		{
			"C2WindowMail",
			sizeof (C2WindowMail),
			sizeof (C2WindowMailClass),
			(GtkClassInitFunc) class_init,
			(GtkObjectInitFunc) init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL
		};

		type = gtk_type_unique (c2_window_get_type (), &info);
	}

	return type;
}

static void
class_init (C2WindowMailClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *) klass;
	
	parent_class = gtk_type_class (c2_window_get_type ());

	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);
}

static void
init (C2WindowMail *wmail)
{
	wmail->toolbar = NULL;
	wmail->db = NULL;
	wmail->message = NULL;
	wmail->read_only = 0;
}

static void
destroy (C2WindowMail *wmail)
{
	if (C2_IS_DB (wmail->db))
		gtk_object_unref (GTK_OBJECT (wmail->db));

	if (C2_IS_MESSAGE (wmail->message))
		gtk_object_unref (GTK_OBJECT (wmail->message));
}

GtkWidget *
c2_window_mail_new (C2Application *application)
{
	C2WindowMail *wmail;

	wmail = gtk_type_new (c2_window_mail_get_type ());
	
	c2_window_mail_construct (wmail, application);

	return GTK_WIDGET (wmail);
}

void
c2_window_mail_construct (C2WindowMail *wmail, C2Application *application)
{
	GladeXML *xml;
	GtkWidget *widget;
	int i;
	
	c2_window_construct (C2_WINDOW (wmail), application, _("Message Viewer"), "window_mail",
							PKGDATADIR "/pixmaps/mail.png");

	xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("mail"), "wnd_mail_contents");
	C2_WINDOW (wmail)->xml = xml;

	c2_window_set_contents_from_glade (C2_WINDOW (wmail), "wnd_mail_contents");

	/* Toolbar */
	widget = glade_xml_get_widget (xml, "toolbar_container");
	wmail->toolbar = c2_toolbar_new (C2_TOOLBAR_TEXT_BESIDE_ICON);
	gtk_box_pack_start (GTK_BOX (widget), wmail->toolbar, TRUE, TRUE, 0);
	gtk_widget_show (wmail->toolbar);
	/* Set the buttons of the toolbar */
	c2_toolbar_freeze (C2_TOOLBAR (wmail->toolbar));
	for (i = 0; toolbar_items[i].type >= 0 && toolbar_items[i].type <= 2; i++)
	{
		switch (toolbar_items[i].type)
		{
			case C2_TOOLBAR_BUTTON:
				widget = c2_toolbar_append_button (C2_TOOLBAR (wmail->toolbar),
										toolbar_items[i].name,
										toolbar_items[i].button_pixmap,
										toolbar_items[i].button_label,
										toolbar_items[i].button_tooltip,
										toolbar_items[i].button_force_label);
				if (toolbar_items[i].button_func)
					gtk_signal_connect (GTK_OBJECT (widget), "clicked",
										toolbar_items[i].button_func,
										toolbar_items[i].button_data ? toolbar_items[i].button_data :
										wmail);
				break;
			case C2_TOOLBAR_WIDGET:
				c2_toolbar_append_widget (C2_TOOLBAR (wmail->toolbar), toolbar_items[i].name,
										toolbar_items[i].widget, toolbar_items[i].widget_tooltip);
				break;
			case C2_TOOLBAR_SPACE:
				c2_toolbar_append_space (C2_TOOLBAR (wmail->toolbar));
				break;
		}
	}
	c2_toolbar_thaw (C2_TOOLBAR (wmail->toolbar));
	c2_toolbar_set_tooltips (C2_TOOLBAR (wmail->toolbar), gnome_config_get_bool_with_default
								("/"PACKAGE"/Toolbar::Window Main/tooltips=true", NULL));
	gtk_widget_show (wmail->toolbar);
	

	widget = glade_xml_get_widget (xml, "mail_container");

	wmail->mail = c2_mail_new (application);
	gtk_box_pack_start (GTK_BOX (widget), wmail->mail, TRUE, TRUE, 0);
	gtk_widget_show (wmail->mail);

	gtk_widget_set_usize (GTK_WIDGET (wmail), c2_preferences_get_window_main_width (),
											c2_preferences_get_window_main_height ());

	set_sensitive (wmail, NULL);
}

static void
on_db_destroy (C2Db *db, C2WindowMail *wmail)
{
	gdk_threads_enter ();
	
	if (!c2_db_is_last (db))
		c2_window_mail_set_db (wmail, db->next);
	else if (!c2_db_is_first (db))
		c2_window_mail_set_db (wmail, db->prev);
	else
		set_sensitive (wmail, NULL);

	gdk_threads_leave ();
}

static void
on_message_destroy (C2Message *message, C2WindowMail *wmail)
{
	gdk_threads_enter ();
	
	set_sensitive (wmail, NULL);

	gdk_threads_leave ();
}

static void
mark_db_as_readed (C2Db *db)
{
	c2_db_message_set_state (db, C2_MESSAGE_READED);
}

void
c2_window_mail_set_db (C2WindowMail *wmail, C2Db *db)
{
	pthread_t thread;
	gchar *subject;
	
	c2_return_if_fail (C2_IS_WINDOW_MAIL (wmail), C2EDATA);
	c2_return_if_fail ((C2_IS_DB (db) || db == NULL), C2EDATA);

	if (C2_IS_DB (wmail->db))
		gtk_signal_disconnect_by_func (GTK_OBJECT (wmail->db),
										GTK_SIGNAL_FUNC (on_db_destroy),
										wmail);
	if (C2_IS_MESSAGE (wmail->message))
		gtk_signal_disconnect_by_func (GTK_OBJECT (wmail->message),
										GTK_SIGNAL_FUNC (on_message_destroy),
										wmail);
	
	wmail->db = db;
	wmail->message = NULL;

	if (C2_IS_DB (db))
	{
		if (c2_db_load_message (db) < 0)
			return;

		gtk_signal_connect (GTK_OBJECT (db), "destroy",
							GTK_SIGNAL_FUNC (on_db_destroy), wmail);
		
		c2_mail_set_message (C2_MAIL (wmail->mail), db->message);
		subject = c2_message_get_header_field (db->message, "Subject:");

		if (!subject || !strlen (subject))
			subject = g_strdup (_("«No Subject»"));
		
		gtk_window_set_title (GTK_WINDOW (wmail), subject);

		g_free (subject);

		/* Mark as readed if required */
		if (db->state == C2_MESSAGE_UNREADED)
		{
			pthread_create (&thread, NULL, C2_PTHREAD_FUNC (mark_db_as_readed), db);
		}

		gtk_object_ref (GTK_OBJECT (db));
	} else
	{
		c2_mail_set_message (C2_MAIL (wmail->mail), NULL);
		gtk_window_set_title (GTK_WINDOW (wmail), TITLE);
	}

	set_sensitive (wmail, wmail->db);

	gtk_signal_emit_by_name (GTK_OBJECT (C2_WINDOW (wmail)->application), "window_changed",
					c2_application_open_windows (C2_WINDOW (wmail)->application));
}

void
c2_window_mail_set_message (C2WindowMail *wmail, C2Message *message)
{
	gchar *subject;

	c2_return_if_fail (C2_IS_WINDOW_MAIL (wmail), C2EDATA);
	c2_return_if_fail ((C2_IS_MESSAGE (message) || message == NULL), C2EDATA);

	if (C2_IS_DB (wmail->db))
		gtk_signal_disconnect_by_func (GTK_OBJECT (wmail->db),
										GTK_SIGNAL_FUNC (on_db_destroy),
										wmail);
	if (C2_IS_MESSAGE (wmail->message))
		gtk_signal_disconnect_by_func (GTK_OBJECT (wmail->message),
										GTK_SIGNAL_FUNC (on_message_destroy),
										wmail);
	
	wmail->db = NULL;
	wmail->message = message;

	if (C2_IS_MESSAGE (message))
	{
		gtk_signal_connect (GTK_OBJECT (message), "destroy",
							GTK_SIGNAL_FUNC (on_message_destroy), wmail);
		
		c2_mail_set_message (C2_MAIL (wmail->mail), message);

		subject = c2_message_get_header_field (message, "Subject:");

		if (!subject || !strlen (subject))
			subject = g_strdup (_("«No Subject»"));
		
		gtk_window_set_title (GTK_WINDOW (wmail), subject);

		g_free (subject);

		gtk_object_ref (GTK_OBJECT (message));
	} else
	{
		c2_mail_set_message (C2_MAIL (wmail->mail), NULL);
		gtk_window_set_title (GTK_WINDOW (wmail), TITLE);
	}

	set_sensitive (wmail, wmail->message);

	gtk_signal_emit_by_name (GTK_OBJECT (C2_WINDOW (wmail)->application), "window_changed",
					c2_application_open_windows (C2_WINDOW (wmail)->application));
}

static void
on_toolbar_close_clicked (GtkWidget *widget, C2WindowMail *wmail)
{
	if (C2_IS_DB (wmail->db))
		gtk_object_unref (GTK_OBJECT (wmail->db));
	if (C2_IS_MESSAGE (wmail->message))
		gtk_object_unref (GTK_OBJECT (wmail->message));

	gtk_object_destroy (GTK_OBJECT (wmail->toolbar));
	gtk_object_destroy (GTK_OBJECT (wmail));
}

static void
on_toolbar_copy_clicked (GtkWidget *widget, C2WindowMail *wmail)
{
}

static void
on_toolbar_delete_clicked (GtkWidget *widget, C2WindowMail *wmail)
{
	C2Application *application = C2_WINDOW (wmail)->application;
	GList *list;

	c2_return_if_fail (C2_IS_WINDOW_MAIL (wmail), C2EDATA);
	c2_return_if_fail (C2_IS_DB (wmail->db), C2EDATA);
	
	list = g_list_append (NULL, wmail->db);
	C2_APPLICATION_CLASS_FW (application)->delete (application, list, C2_WINDOW (wmail));
	g_list_free (list);
}

static void
on_toolbar_forward_clicked (GtkWidget *widget, C2WindowMail *wmail)
{
}

static void
on_toolbar_move_clicked (GtkWidget *widget, C2WindowMail *wmail)
{
}

static void
on_toolbar_next_clicked (GtkWidget *widget, C2WindowMail *wmail)
{
	c2_return_if_fail (C2_IS_WINDOW_MAIL (wmail), C2EDATA);
	c2_return_if_fail (C2_IS_DB (wmail->db), C2EDATA);
	
	c2_window_mail_set_db (wmail, wmail->db->next);
}

static void
on_toolbar_previous_clicked (GtkWidget *widget, C2WindowMail *wmail)
{
	c2_return_if_fail (C2_IS_WINDOW_MAIL (wmail), C2EDATA);
	c2_return_if_fail (C2_IS_DB (wmail->db), C2EDATA);

	c2_window_mail_set_db (wmail, wmail->db->prev);
}

static void
on_toolbar_print_clicked (GtkWidget *widget, C2WindowMail *wmail)
{
}

static void
on_toolbar_reply_clicked (GtkWidget *widget, C2WindowMail *wmail)
{
}

static void
on_toolbar_reply_all_clicked (GtkWidget *widget, C2WindowMail *wmail)
{
}

static void
on_toolbar_save_clicked (GtkWidget *widget, C2WindowMail *wmail)
{
}

static void
on_toolbar_search_clicked (GtkWidget *widget, C2WindowMail *wmail)
{
}

static void
set_sensitive (C2WindowMail *wmail, gpointer data)
{
	GtkWidget *widget;
	
	if (C2_IS_DB (data))
	{
		widget = c2_toolbar_get_item (C2_TOOLBAR (wmail->toolbar), "toolbar_search");
		if (GTK_IS_WIDGET (widget))
			gtk_widget_set_sensitive (widget, TRUE);
		
		widget = c2_toolbar_get_item (C2_TOOLBAR (wmail->toolbar), "toolbar_save");
		if (GTK_IS_WIDGET (widget))
			gtk_widget_set_sensitive (widget, TRUE);

		widget = c2_toolbar_get_item (C2_TOOLBAR (wmail->toolbar), "toolbar_print");
		if (GTK_IS_WIDGET (widget))
			gtk_widget_set_sensitive (widget, TRUE);

		widget = c2_toolbar_get_item (C2_TOOLBAR (wmail->toolbar), "toolbar_delete");
		if (GTK_IS_WIDGET (widget))
			gtk_widget_set_sensitive (widget, TRUE);

		widget = c2_toolbar_get_item (C2_TOOLBAR (wmail->toolbar), "toolbar_copy");
		if (GTK_IS_WIDGET (widget))
			gtk_widget_set_sensitive (widget, TRUE);

		widget = c2_toolbar_get_item (C2_TOOLBAR (wmail->toolbar), "toolbar_move");
		if (GTK_IS_WIDGET (widget))
			gtk_widget_set_sensitive (widget, TRUE);

		widget = c2_toolbar_get_item (C2_TOOLBAR (wmail->toolbar), "toolbar_reply");
		if (GTK_IS_WIDGET (widget))
			gtk_widget_set_sensitive (widget, TRUE);

		widget = c2_toolbar_get_item (C2_TOOLBAR (wmail->toolbar), "toolbar_reply_all");
		if (GTK_IS_WIDGET (widget))
			gtk_widget_set_sensitive (widget, TRUE);

		widget = c2_toolbar_get_item (C2_TOOLBAR (wmail->toolbar), "toolbar_forward");
		if (GTK_IS_WIDGET (widget))
			gtk_widget_set_sensitive (widget, TRUE);

		widget = c2_toolbar_get_item (C2_TOOLBAR (wmail->toolbar), "toolbar_previous");
		if (GTK_IS_WIDGET (widget))
			gtk_widget_set_sensitive (widget, !c2_db_is_first (C2_DB (data)));

		widget = c2_toolbar_get_item (C2_TOOLBAR (wmail->toolbar), "toolbar_next");
		if (GTK_IS_WIDGET (widget))
			gtk_widget_set_sensitive (widget, !c2_db_is_last (C2_DB (data)));

		widget = c2_toolbar_get_item (C2_TOOLBAR (wmail->toolbar), "toolbar_close");
		if (GTK_IS_WIDGET (widget))
			gtk_widget_set_sensitive (widget, TRUE);
	} else if (C2_IS_MESSAGE (data))
	{
		widget = c2_toolbar_get_item (C2_TOOLBAR (wmail->toolbar), "toolbar_search");
		if (GTK_IS_WIDGET (widget))
			gtk_widget_set_sensitive (widget, TRUE);
		
		widget = c2_toolbar_get_item (C2_TOOLBAR (wmail->toolbar), "toolbar_save");
		if (GTK_IS_WIDGET (widget))
			gtk_widget_set_sensitive (widget, TRUE);

		widget = c2_toolbar_get_item (C2_TOOLBAR (wmail->toolbar), "toolbar_print");
		if (GTK_IS_WIDGET (widget))
			gtk_widget_set_sensitive (widget, TRUE);

		widget = c2_toolbar_get_item (C2_TOOLBAR (wmail->toolbar), "toolbar_delete");
		if (GTK_IS_WIDGET (widget))
			gtk_widget_set_sensitive (widget, TRUE);

		widget = c2_toolbar_get_item (C2_TOOLBAR (wmail->toolbar), "toolbar_copy");
		if (GTK_IS_WIDGET (widget))
			gtk_widget_set_sensitive (widget, TRUE);

		widget = c2_toolbar_get_item (C2_TOOLBAR (wmail->toolbar), "toolbar_move");
		if (GTK_IS_WIDGET (widget))
			gtk_widget_set_sensitive (widget, FALSE);

		widget = c2_toolbar_get_item (C2_TOOLBAR (wmail->toolbar), "toolbar_reply");
		if (GTK_IS_WIDGET (widget))
			gtk_widget_set_sensitive (widget, TRUE);

		widget = c2_toolbar_get_item (C2_TOOLBAR (wmail->toolbar), "toolbar_reply_all");
		if (GTK_IS_WIDGET (widget))
			gtk_widget_set_sensitive (widget, TRUE);

		widget = c2_toolbar_get_item (C2_TOOLBAR (wmail->toolbar), "toolbar_forward");
		if (GTK_IS_WIDGET (widget))
			gtk_widget_set_sensitive (widget, TRUE);

		widget = c2_toolbar_get_item (C2_TOOLBAR (wmail->toolbar), "toolbar_previous");
		if (GTK_IS_WIDGET (widget))
			gtk_widget_set_sensitive (widget, FALSE);

		widget = c2_toolbar_get_item (C2_TOOLBAR (wmail->toolbar), "toolbar_next");
		if (GTK_IS_WIDGET (widget))
			gtk_widget_set_sensitive (widget, FALSE);

		widget = c2_toolbar_get_item (C2_TOOLBAR (wmail->toolbar), "toolbar_close");
		if (GTK_IS_WIDGET (widget))
			gtk_widget_set_sensitive (widget, TRUE);
	} else
	{
		widget = c2_toolbar_get_item (C2_TOOLBAR (wmail->toolbar), "toolbar_search");
		if (GTK_IS_WIDGET (widget))
			gtk_widget_set_sensitive (widget, FALSE);
		
		widget = c2_toolbar_get_item (C2_TOOLBAR (wmail->toolbar), "toolbar_save");
		if (GTK_IS_WIDGET (widget))
			gtk_widget_set_sensitive (widget, FALSE);

		widget = c2_toolbar_get_item (C2_TOOLBAR (wmail->toolbar), "toolbar_print");
		if (GTK_IS_WIDGET (widget))
			gtk_widget_set_sensitive (widget, FALSE);

		widget = c2_toolbar_get_item (C2_TOOLBAR (wmail->toolbar), "toolbar_delete");
		if (GTK_IS_WIDGET (widget))
			gtk_widget_set_sensitive (widget, FALSE);

		widget = c2_toolbar_get_item (C2_TOOLBAR (wmail->toolbar), "toolbar_copy");
		if (GTK_IS_WIDGET (widget))
			gtk_widget_set_sensitive (widget, FALSE);

		widget = c2_toolbar_get_item (C2_TOOLBAR (wmail->toolbar), "toolbar_move");
		if (GTK_IS_WIDGET (widget))
			gtk_widget_set_sensitive (widget, FALSE);

		widget = c2_toolbar_get_item (C2_TOOLBAR (wmail->toolbar), "toolbar_reply");
		if (GTK_IS_WIDGET (widget))
			gtk_widget_set_sensitive (widget, FALSE);

		widget = c2_toolbar_get_item (C2_TOOLBAR (wmail->toolbar), "toolbar_reply_all");
		if (GTK_IS_WIDGET (widget))
			gtk_widget_set_sensitive (widget, FALSE);

		widget = c2_toolbar_get_item (C2_TOOLBAR (wmail->toolbar), "toolbar_forward");
		if (GTK_IS_WIDGET (widget))
			gtk_widget_set_sensitive (widget, FALSE);
		
		widget = c2_toolbar_get_item (C2_TOOLBAR (wmail->toolbar), "toolbar_previous");
		if (GTK_IS_WIDGET (widget))
			gtk_widget_set_sensitive (widget, FALSE);

		widget = c2_toolbar_get_item (C2_TOOLBAR (wmail->toolbar), "toolbar_next");
		if (GTK_IS_WIDGET (widget))
			gtk_widget_set_sensitive (widget, FALSE);

		widget = c2_toolbar_get_item (C2_TOOLBAR (wmail->toolbar), "toolbar_close");
		if (GTK_IS_WIDGET (widget))
			gtk_widget_set_sensitive (widget, TRUE);
	}
}
