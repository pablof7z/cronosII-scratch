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
#include <sys/stat.h>

#include <glade/glade.h>

#include <libcronosII/error.h>
#include <libcronosII/mime.h>
#include <libcronosII/utils.h>

#include "widget-composer.h"
#include "widget-dialog-preferences.h"
#include "widget-mailbox-list.h"
#include "widget-mail.h"
#include "widget-HTML.h"
#include "widget-toolbar.h"
#include "widget-transfer-list.h"
#include "widget-index.h"
#include "widget-window-main.h"

#define MAILBOX_TYPE_CRONOSII				"Cronos II"
#define MAILBOX_TYPE_IMAP					"IMAP"
#define MAILBOX_TYPE_SPOOL					_("Spool (local)")

static void
class_init									(C2WindowMainClass *klass);

static void
init										(C2WindowMain *wmain);

static gint
on_delete_event								(GtkWidget *widget, GdkEventAny *event, gpointer data);

static void
on_docktoolbar_button_press_event			(GtkWidget *widget, GdkEventButton *event, C2WindowMain *wmain);

static void
on_toolbar_changed							(GtkWidget *widget, C2WindowMain *wmain);

static void
on_toolbar_check_clicked					(GtkWidget *widget, C2WindowMain *wmain);

static void
on_toolbar_delete_clicked					(GtkWidget *widget, C2WindowMain *wmain);

static void
on_toolbar_compose_clicked					(GtkWidget *widget, C2WindowMain *wmain);

static void
on_toolbar_reply_clicked					(GtkWidget *widget, C2WindowMain *wmain);

static void
on_toolbar_exit_clicked						(GtkWidget *widget, C2WindowMain *wmain);

static void
on_index_select_message						(GtkWidget *index, C2Db *node, C2WindowMain *wmain);

static void
on_mlist_mailbox_selected					(C2MailboxList *mlist, C2Mailbox *mailbox, C2WindowMain *wmain);

static void
on_mlist_mailbox_unselected					(C2MailboxList *mlist, C2WindowMain *wmain);

static void
on_mlist_button_press_event					(GtkWidget *widget, GdkEvent *event, C2WindowMain *wmain);

static void
on_file_new_mailbox_activate				(GtkWidget *widget, C2WindowMain *wmain);

static void
on_eastern_egg_separator_activate			(GtkWidget *widget, C2WindowMain *wmain);

static void
on_settings_preferences_activate			(GtkWidget *widget, C2WindowMain *wmain);

static void
on_about_activate							(GtkWidget *widget, C2WindowMain *wmain);

static void
on_mnu_toolbar_text_beside_icons_activate	(GtkWidget *widget, C2WindowMain *wmain);

static void
on_mnu_toolbar_text_under_icons_activate	(GtkWidget *widget, C2WindowMain *wmain);

static void
on_mnu_toolbar_icons_only_activate			(GtkWidget *widget, C2WindowMain *wmain);

static void
on_mnu_toolbar_text_only_activate			(GtkWidget *widget, C2WindowMain *wmain);

static void
on_mnu_toolbar_tooltips_toggled				(GtkWidget *widget, C2WindowMain *wmain);

static void
on_mnu_toolbar_edit_toolbar_activate		(GtkWidget *widget, C2WindowMain *wmain);

/* in widget-application.c */
extern void
on_mailbox_changed_mailboxes				(C2Mailbox *mailbox, C2Application *application);

enum
{
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static C2WindowClass *parent_class = NULL;

C2ToolbarItem toolbar_items[] =
{
	{
		C2_TOOLBAR_BUTTON,
		N_("Check"), PKGDATADIR "/pixmaps/receive.png",
		N_("Check for incoming mails"), TRUE,
		GTK_SIGNAL_FUNC (on_toolbar_check_clicked), NULL,
		NULL, NULL
	},
	{
		C2_TOOLBAR_BUTTON,
		N_("Send"), PKGDATADIR "/pixmaps/send.png",
		N_("Send outgoing mails"), TRUE,
		NULL, NULL,
		NULL, NULL
	},
	{
		C2_TOOLBAR_BUTTON,
		N_("Search"), PKGDATADIR "/pixmaps/find.png",
		N_("Search a message in existent mailboxes"), TRUE,
		NULL, NULL,
		NULL, NULL
	},
	{
		C2_TOOLBAR_BUTTON,
		N_("Save"), PKGDATADIR "/pixmaps/save.png",
		N_("Save selected message"), TRUE,
		NULL, NULL,
		NULL, NULL
	},
	{
		C2_TOOLBAR_BUTTON,
		N_("Print"), PKGDATADIR "/pixmaps/print.png",
		N_("Print selected message"), TRUE,
		NULL, NULL,
		NULL, NULL
	},
	{
		C2_TOOLBAR_BUTTON,
		N_("Delete"), PKGDATADIR "/pixmaps/delete.png",
		N_("Delete selected mails"), TRUE,
		GTK_SIGNAL_FUNC (on_toolbar_delete_clicked), NULL,
		NULL, NULL
	},
	{
		C2_TOOLBAR_BUTTON,
		N_("Copy"), PKGDATADIR "/pixmaps/copy-message.png",
		N_("Copy selected mails"), TRUE,
		NULL, NULL,
		NULL, NULL
	},
	{
		C2_TOOLBAR_BUTTON,
		N_("Move"), PKGDATADIR "/pixmaps/move-message.png",
		N_("Move selected mails"), TRUE,
		NULL, NULL,
		NULL, NULL
	},
	{
		C2_TOOLBAR_BUTTON,
		N_("Compose"), PKGDATADIR "/pixmaps/mail-write.png",
		N_("Compose a new message"), TRUE,
		GTK_SIGNAL_FUNC (on_toolbar_compose_clicked), NULL,
		NULL, NULL
	},
	{
		C2_TOOLBAR_BUTTON,
		N_("Reply"), PKGDATADIR "/pixmaps/reply.png",
		N_("Reply selected message"), TRUE,
		GTK_SIGNAL_FUNC (on_toolbar_reply_clicked), NULL,
		NULL, NULL
	},
	{
		C2_TOOLBAR_BUTTON,
		N_("Reply All"), PKGDATADIR "/pixmaps/reply-all.png",
		N_("Reply selected message to all recipients"), TRUE,
		NULL, NULL,
		NULL, NULL,
	},
	{
		C2_TOOLBAR_BUTTON,
		N_("Forward"), PKGDATADIR "/pixmaps/forward.png",
		N_("Forward selected message"), TRUE,
		NULL, NULL,
		NULL, NULL
	},
	{
		C2_TOOLBAR_BUTTON,
		N_("Previous"), PKGDATADIR "/pixmaps/prev.png",
		N_("Select previous message"), FALSE,
		NULL, NULL,
		NULL, NULL
	},
	{
		C2_TOOLBAR_BUTTON,
		N_("Next"), PKGDATADIR "/pixmaps/next.png",
		N_("Select next message"), FALSE,
		NULL, NULL,
		NULL, NULL
	},
	{
		C2_TOOLBAR_BUTTON,
		N_("Contacts"), PKGDATADIR "/pixmaps/contacts.png",
		N_("Open the Contacts Address Book"), TRUE,
		NULL, NULL,
		NULL, NULL
	},
	{
		C2_TOOLBAR_BUTTON,
		N_("Close"), PKGDATADIR "/pixmaps/close.png",
		N_("Close the main window of Cronos II"), TRUE,
		NULL, NULL,
		NULL, NULL
	},
	{
		C2_TOOLBAR_BUTTON,
		N_("Exit"), PKGDATADIR "/pixmaps/exit.png",
		N_("Exit Cronos II"), TRUE,
		GTK_SIGNAL_FUNC (on_toolbar_exit_clicked), NULL,
		NULL, NULL
	},
	{
		C2_TOOLBAR_SPACE, NULL, NULL, NULL, FALSE, NULL, NULL, NULL
	}
};

GtkType
c2_window_main_get_type (void)
{
	static GtkType type = 0;
	
	if (!type)
	{
		static GtkTypeInfo info =
		{
			"C2WindowMain",
			sizeof (C2WindowMain),
			sizeof (C2WindowMainClass),
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
class_init (C2WindowMainClass *klass)
{
	parent_class = gtk_type_class (c2_window_get_type ());
}

static void
init (C2WindowMain *wmain)
{
	pthread_mutex_init (&wmain->index_lock, NULL);
	pthread_mutex_init (&wmain->body_lock, NULL);
}

GtkWidget *
c2_window_main_new (C2Application *application)
{
	C2WindowMain *wmain;

	wmain = gtk_type_new (c2_window_main_get_type ());

	c2_window_main_construct (wmain, application);

	return GTK_WIDGET (wmain);
}

void
c2_window_main_construct (C2WindowMain *wmain, C2Application *application)
{
	GladeXML *xml;
	GtkWidget *window;
	GtkWidget *widget;
	GtkWidget *toolbar;
	GtkWidget *hpaned;
	GtkWidget *vpaned;
	GtkWidget *ctree;
	GtkCList *clist;
	GtkWidget *index_scroll;
	GtkWidget *mail;
	GtkWidget *button;
	GtkWidget *pixmap;
	GtkWidget *appbar;
	GtkWidget *scroll;
	GtkStyle *style;
	gint toolbar_style;
	gint i;

	c2_window_construct (C2_WINDOW (wmain), application, "Cronos II", "Main");

	C2_WINDOW (wmain)->xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("cronosII"), "wnd_main_contents");
	c2_window_set_contents_from_glade (C2_WINDOW (wmain), "wnd_main_contents");
	
	wmain->ctree_menu = glade_xml_new (C2_APPLICATION_GLADE_FILE ("cronosII"), "mnu_ctree");
	wmain->toolbar_menu = glade_xml_new (C2_APPLICATION_GLADE_FILE ("cronosII"), "mnu_toolbar");

	xml = C2_WINDOW (wmain)->xml;

	if (!C2_WINDOW (wmain)->xml)
	{
#ifdef USE_DEBUG
		g_warning ("Unable to find Glade files, check that you 'make install' "
			   "the directories /xml and /images in the source distribution.\n"
			   "The file that failed to load was '%s'.\n", C2_APPLICATION_GLADE_FILE ("cronosII"));
#else
		g_print (_("Cronos II was not able to load the UI, you should check "
			   "your installation.\nAborting.\n"));
#endif
		gtk_object_destroy (GTK_OBJECT (application));
	}

	gtk_widget_realize (GTK_WIDGET (wmain));
	gtk_widget_set_usize (GTK_WIDGET (wmain), application->rc_width, application->rc_height);
	gtk_window_set_policy (GTK_WINDOW (wmain), TRUE, TRUE, FALSE);
	gtk_signal_connect (GTK_OBJECT (wmain), "delete_event",
							GTK_SIGNAL_FUNC (on_delete_event), NULL);

	style = gtk_widget_get_default_style ();
	pixmap = gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/read.png");
	application->pixmap_read = GNOME_PIXMAP (pixmap)->pixmap;
	application->mask_read = GNOME_PIXMAP (pixmap)->mask;
	pixmap = gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/unread.png");
	application->pixmap_unread = GNOME_PIXMAP (pixmap)->pixmap;
	application->mask_unread = GNOME_PIXMAP (pixmap)->mask;
	pixmap = gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/reply.png");
	application->pixmap_reply = GNOME_PIXMAP (pixmap)->pixmap;
	application->mask_reply = GNOME_PIXMAP (pixmap)->mask;
	pixmap = gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/forward.png");
	application->pixmap_forward = GNOME_PIXMAP (pixmap)->pixmap;
	application->mask_forward = GNOME_PIXMAP (pixmap)->mask;

	/* Hpaned */
	hpaned = glade_xml_get_widget (xml, "hpaned");
	gtk_paned_set_position (GTK_PANED (hpaned), application->rc_hpan);

	/* Mailbox List */
	wmain->mlist = c2_mailbox_list_new (application);
	scroll = glade_xml_get_widget (xml, "mlist_scroll");
	gtk_container_add (GTK_CONTAINER (scroll), wmain->mlist);
	gtk_widget_show (wmain->mlist);
	GTK_WIDGET_UNSET_FLAGS (wmain->mlist, GTK_CAN_FOCUS);

	gtk_signal_connect (GTK_OBJECT (wmain->mlist), "mailbox_selected",
								GTK_SIGNAL_FUNC (on_mlist_mailbox_selected), wmain);
	gtk_signal_connect (GTK_OBJECT (wmain->mlist), "mailbox_unselected",
								GTK_SIGNAL_FUNC (on_mlist_mailbox_unselected), wmain);
	gtk_signal_connect (GTK_OBJECT (wmain->mlist), "button_press_event",
      			GTK_SIGNAL_FUNC (on_mlist_button_press_event), wmain);

	/* Toolbar */
	widget = glade_xml_get_widget (xml, "docktoolbar");
	toolbar_style = gnome_config_get_int_with_default ("/"PACKAGE"/Toolbar::Window Main/type=0", NULL);
	wmain->toolbar = c2_toolbar_new (toolbar_style);
	gtk_container_add (GTK_CONTAINER (widget), wmain->toolbar);
	
	gtk_signal_connect (GTK_OBJECT (wmain->toolbar), "changed",
						GTK_SIGNAL_FUNC (on_toolbar_changed), wmain);

	/* Set the buttons of the toolbar */
	c2_toolbar_freeze (C2_TOOLBAR (wmain->toolbar));
	for (i = 1;; i++)
	{
		gchar *key;
		gint val, df;

		key = g_strdup_printf ("/"PACKAGE"/Toolbar::Window Main/item %d=-1", i);
		val = gnome_config_get_int_with_default (key, &df);

		if (val < 0)
			break;

		switch (toolbar_items[val].type)
		{
			case C2_TOOLBAR_BUTTON:
				widget = c2_toolbar_append_button (C2_TOOLBAR (wmain->toolbar),
										toolbar_items[val].button_pixmap,
										toolbar_items[val].button_label,
										toolbar_items[val].button_tooltip,
										toolbar_items[val].button_force_label);
				if (toolbar_items[val].button_func)
					gtk_signal_connect (GTK_OBJECT (widget), "clicked",
										toolbar_items[val].button_func,
										toolbar_items[val].button_data ? toolbar_items[val].button_data :
										wmain);
				break;
			case C2_TOOLBAR_WIDGET:
				c2_toolbar_append_widget (C2_TOOLBAR (wmain->toolbar), toolbar_items[val].widget,
										toolbar_items[val].widget_tooltip);
				break;
			case C2_TOOLBAR_SPACE:
				c2_toolbar_append_space (C2_TOOLBAR (wmain->toolbar));
				break;
		}
	}
	c2_toolbar_thaw (C2_TOOLBAR (wmain->toolbar));
	c2_toolbar_set_tooltips (C2_TOOLBAR (wmain->toolbar), gnome_config_get_bool_with_default
								("/"PACKAGE"/Toolbar::Window Main/tooltips=true", NULL));
	gtk_widget_show (wmain->toolbar);

	/* Vpaned */
	vpaned = glade_xml_get_widget (xml, "vpaned");
	gtk_paned_set_position (GTK_PANED (vpaned), application->rc_vpan);

	appbar = glade_xml_get_widget (xml, "appbar");

	/* Index */
	index_scroll = glade_xml_get_widget (xml, "index_scroll");
	wmain->index = c2_index_new (application);
	gtk_container_add (GTK_CONTAINER (index_scroll), wmain->index);
	gtk_widget_show (wmain->index);
	gtk_signal_connect (GTK_OBJECT (wmain->index), "select_message",
							GTK_SIGNAL_FUNC (on_index_select_message), wmain);

	/* Mail */
	mail = glade_xml_get_widget (xml, "mail");
	c2_mail_install_hints (C2_MAIL (mail), appbar, &C2_WINDOW (wmain)->status_lock);

	/* Button */
	button = glade_xml_get_widget (xml, "appbar_button");
	pixmap = gnome_stock_pixmap_widget_at_size (window, GNOME_STOCK_PIXMAP_UP, 10, 14);
	gtk_container_add (GTK_CONTAINER (button), pixmap);
	gtk_widget_show (pixmap);

	/* Connect all signals: menues, toolbar, buttons, etc. */
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "docktoolbar")), "button_press_event",
							GTK_SIGNAL_FUNC (on_docktoolbar_button_press_event), wmain);
	
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "file_new_mailbox")), "activate",
							GTK_SIGNAL_FUNC (on_file_new_mailbox_activate), wmain);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "file_egg_separator")), "activate",
							GTK_SIGNAL_FUNC (on_eastern_egg_separator_activate), wmain);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "settings_preferences")), "activate",
							GTK_SIGNAL_FUNC (on_settings_preferences_activate), wmain);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "help_about")), "activate",
							GTK_SIGNAL_FUNC (on_about_activate), wmain);

	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (wmain->toolbar_menu, "text_beside_icons")), "activate",
							GTK_SIGNAL_FUNC (on_mnu_toolbar_text_beside_icons_activate), wmain);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (wmain->toolbar_menu, "text_under_icons")), "activate",
							GTK_SIGNAL_FUNC (on_mnu_toolbar_text_under_icons_activate), wmain);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (wmain->toolbar_menu, "text_only")), "activate",
							GTK_SIGNAL_FUNC (on_mnu_toolbar_text_only_activate), wmain);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (wmain->toolbar_menu, "icons_only")), "activate",
							GTK_SIGNAL_FUNC (on_mnu_toolbar_icons_only_activate), wmain);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (wmain->toolbar_menu, "tooltips")), "toggled",
							GTK_SIGNAL_FUNC (on_mnu_toolbar_tooltips_toggled), wmain);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (wmain->toolbar_menu, "edit_toolbar")), "activate",
							GTK_SIGNAL_FUNC (on_mnu_toolbar_edit_toolbar_activate), wmain);
	
//	c2_main_window_build_dynamic_menu_accounts ();
}

/**
 * c2_window_main_set_mailbox
 * @wmain: Object where to act.
 * @mailbox: Mailbox to set.
 *
 * This function sets the active mailbox in the main window.
 **/
void
c2_window_main_set_mailbox (C2WindowMain *wmain, C2Mailbox *mailbox)
{
	c2_return_if_fail_obj (mailbox, C2EDATA, GTK_OBJECT (wmain));

	c2_mailbox_list_set_selected_mailbox (C2_MAILBOX_LIST (wmain->mlist), mailbox);
}

static gint
on_delete_event (GtkWidget *widget, GdkEventAny *event, gpointer data)
{
	C2WindowMain *wmain = C2_WINDOW_MAIN (widget);
	
	/* Anything we should do when the main window
	 * is closed (not the application, just the
	 * main window):
	 * i.e. unref any loaded message.
	 */
	return FALSE;
}

static void
on_docktoolbar_button_press_event (GtkWidget *widget, GdkEventButton *event, C2WindowMain *wmain)
{
	C2ToolbarStyle style;
	gboolean tooltips;
	GtkWidget *widgetb;
	GladeXML *xml;

	c2_return_if_fail (event, C2EDATA);
	
	if (event->button == 3)
	{
		xml = wmain->toolbar_menu;
		style = gnome_config_get_int_with_default ("/"PACKAGE"/Toolbar::Window Main/type=0", NULL);
		tooltips = gnome_config_get_bool_with_default ("/"PACKAGE"/Toolbar::Window Main/tooltips=true", NULL);

		widgetb = glade_xml_get_widget (xml, "text_beside_icons");
		GTK_CHECK_MENU_ITEM (widgetb)->active = style == C2_TOOLBAR_TEXT_BESIDE_ICON ? 1 : 0;
		widgetb = glade_xml_get_widget (xml, "text_under_icons");
		GTK_CHECK_MENU_ITEM (widgetb)->active = style == C2_TOOLBAR_TEXT_UNDER_ICON ? 1 : 0;
		widgetb = glade_xml_get_widget (xml, "text_only");
		GTK_CHECK_MENU_ITEM (widgetb)->active = style == C2_TOOLBAR_JUST_TEXT ? 1 : 0;
		widgetb = glade_xml_get_widget (xml, "icons_only");
		GTK_CHECK_MENU_ITEM (widgetb)->active = style == C2_TOOLBAR_JUST_ICON ? 1 : 0;
		
		widgetb = glade_xml_get_widget (xml, "tooltips");
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (widgetb), tooltips);
		
		gnome_popup_menu_do_popup (glade_xml_get_widget (wmain->toolbar_menu, "mnu_toolbar"),
								NULL, NULL, event, NULL);
	}
}

static void
on_toolbar_changed (GtkWidget *widget, C2WindowMain *wmain)
{
	gtk_widget_queue_resize (GTK_WIDGET (wmain));
}

static void
on_toolbar_check_clicked (GtkWidget *widget, C2WindowMain *wmain)
{
	C2Application *application;
	C2Account *account;
	GtkWidget *wtl;
	C2TransferItem *wti;

	application = C2_WINDOW (wmain)->application;
	wtl = c2_application_window_get (application, C2_WIDGET_TRANSFER_LIST_TYPE);

	if (!wtl || !C2_IS_TRANSFER_LIST (wtl))
		wtl = c2_transfer_list_new (application);

	gtk_widget_show (wtl);
	gdk_window_raise (wtl->window);

	for (account = application->account; account; account = c2_account_next (account))
	{
		gpointer data = c2_account_get_extra_data (account, C2_ACCOUNT_KEY_ACTIVE, NULL);

		if (!GPOINTER_TO_INT (data))
			continue;

		wti = c2_transfer_item_new (application, account, C2_TRANSFER_ITEM_RECEIVE);
		c2_transfer_list_add_item (C2_TRANSFER_LIST (wtl), wti);
		c2_transfer_item_start (wti);
	}
}

static void
on_toolbar_delete_clicked_thread (C2WindowMain *wmain)
{
	C2Mailbox *mailbox = c2_mailbox_list_get_selected_mailbox (C2_MAILBOX_LIST (wmain->mlist));
	C2Mailbox *trash = c2_mailbox_get_by_name (C2_WINDOW (wmain)->application->mailbox,
												C2_MAILBOX_GARBAGE);
	GList *list;

	list = GTK_CLIST (wmain->index)->selection;

	if (gnome_config_get_bool_with_default ("/"PACKAGE"/General-Options/delete_use_trash=false", NULL))
		;
//		cd_db_message_move (mailbox, trash, list);
	else
		c2_db_message_remove (mailbox, list);
}

static void
on_toolbar_delete_clicked (GtkWidget *widget, C2WindowMain *wmain)
{
	pthread_t thread;

	pthread_create (&thread, NULL, C2_PTHREAD_FUNC (on_toolbar_delete_clicked_thread), wmain);
}

static void
on_toolbar_compose_clicked (GtkWidget *widget, C2WindowMain *wmain)
{
	GtkWidget *composer;
	
	composer = c2_composer_new (C2_WINDOW (wmain)->application);
	gtk_widget_show (composer);
}

static void
on_toolbar_reply_clicked (GtkWidget *widget, C2WindowMain *wmain)
{
	GtkWidget *composer;
	GtkWidget *mail;
	C2Message *message;
	
	mail = glade_xml_get_widget (C2_WINDOW (wmain)->xml, "mail");
	message = c2_mail_get_message (C2_MAIL (mail));
	composer = c2_composer_new (C2_WINDOW (wmain)->application);
	c2_composer_set_message_as_quote (C2_COMPOSER (composer), message);
	gtk_widget_show (composer);
}

static void
on_toolbar_exit_clicked (GtkWidget *widget, C2WindowMain *wmain)
{
	gtk_object_destroy (GTK_OBJECT (wmain));
}

static void
on_index_select_message (GtkWidget *index, C2Db *node, C2WindowMain *wmain)
{
	GladeXML *xml;
	GtkWidget *widget;

	if (g_list_length (GTK_CLIST (index)->selection) > 1)
		return;
	
	if (!node->message)
	{
		c2_db_load_message (node);
		if (!node->message)
		{
			/* Something went wrong */
			const gchar *error;
			
			error = c2_error_object_get (GTK_OBJECT (node));
			if (error)
				c2_window_report (C2_WINDOW (wmain), C2_WINDOW_REPORT_WARNING,
								_("Message failed to load: %s."), error);
			else
				c2_window_report (C2_WINDOW (wmain), C2_WINDOW_REPORT_WARNING,
								_("Message failed to load."));
		}
	}

	c2_mail_set_message (C2_MAIL (glade_xml_get_widget (C2_WINDOW (wmain)->xml, "mail")), node->message);

	/* Set some widgets sensivity */
	xml = C2_WINDOW (wmain)->xml;
	gtk_widget_set_sensitive (glade_xml_get_widget (xml, "toolbar_save"), TRUE);
	gtk_widget_set_sensitive (glade_xml_get_widget (xml, "toolbar_print"), TRUE);
	gtk_widget_set_sensitive (glade_xml_get_widget (xml, "toolbar_delete"), TRUE);
	gtk_widget_set_sensitive (glade_xml_get_widget (xml, "toolbar_reply"), TRUE);
	gtk_widget_set_sensitive (glade_xml_get_widget (xml, "toolbar_reply_all"), TRUE);
	gtk_widget_set_sensitive (glade_xml_get_widget (xml, "toolbar_forward"), TRUE);
	gtk_widget_set_sensitive (glade_xml_get_widget (xml, "file_save"), TRUE);
	gtk_widget_set_sensitive (glade_xml_get_widget (xml, "file_print"), TRUE);
	gtk_widget_set_sensitive (glade_xml_get_widget (xml, "message_reply"), TRUE);
	gtk_widget_set_sensitive (glade_xml_get_widget (xml, "message_reply_all"), TRUE);
	gtk_widget_set_sensitive (glade_xml_get_widget (xml, "message_forward"), TRUE);
	gtk_widget_set_sensitive (glade_xml_get_widget (xml, "message_copy"), TRUE);
	gtk_widget_set_sensitive (glade_xml_get_widget (xml, "message_move"), TRUE);
	gtk_widget_set_sensitive (glade_xml_get_widget (xml, "message_delete"), TRUE);
	gtk_widget_set_sensitive (glade_xml_get_widget (xml, "message_expunge"), TRUE);
	gtk_widget_set_sensitive (glade_xml_get_widget (xml, "message_mark_important"), TRUE);
	gtk_widget_set_sensitive (glade_xml_get_widget (xml, "message_mark_unread"), TRUE);
	gtk_widget_set_sensitive (glade_xml_get_widget (xml, "message_mark_read"), TRUE);
	gtk_widget_set_sensitive (glade_xml_get_widget (xml, "message_mark_replied"), TRUE);
	gtk_widget_set_sensitive (glade_xml_get_widget (xml, "message_mark_forward"), TRUE);
}

static gint
on_mlist_mailbox_selected_pthread (C2WindowMain *wmain)
{
	C2Mailbox *mailbox = c2_mailbox_list_get_selected_mailbox (C2_MAILBOX_LIST (wmain->mlist));
	C2Index *index = C2_INDEX (wmain->index);
	gchar *buf;
	
	gdk_threads_enter ();
	c2_window_set_activity (C2_WINDOW (wmain), TRUE);
	gdk_threads_leave ();
	
	if (!mailbox->db)
	{
		if (!c2_mailbox_load_db (mailbox))
		{
			/* Something went wrong... */
			c2_window_report (C2_WINDOW (wmain), C2_WINDOW_REPORT_WARNING,
							_("The mailbox '%s' couldn't be loaded"), mailbox->name);
			c2_index_remove_mailbox (index);
			c2_window_set_activity (C2_WINDOW (wmain), FALSE);
			gtk_widget_queue_draw (GTK_WIDGET (index));
			return 1;
		}

		/* Check if the user didn't select other
		 * mailbox.
		 */
		if (mailbox != c2_mailbox_list_get_selected_mailbox (C2_MAILBOX_LIST (wmain->mlist)))
			return 0;
	}
	
	if (pthread_mutex_trylock (&wmain->index_lock))
		return 0;
	
	gdk_threads_enter ();
	c2_index_remove_mailbox (index);
	c2_index_add_mailbox (index, mailbox);
	c2_window_set_activity (C2_WINDOW (wmain), FALSE);
	gtk_widget_queue_draw (GTK_WIDGET (index));
	c2_index_sort (index);
	c2_window_report (C2_WINDOW (wmain), C2_WINDOW_REPORT_MESSAGE,
						_("%d messages, %d new."), c2_db_length (mailbox),
						c2_db_length_type (mailbox, C2_MESSAGE_UNREADED));
	gdk_threads_leave ();

	pthread_mutex_unlock (&wmain->index_lock);
	return 0;
}

static void
on_mlist_mailbox_selected (C2MailboxList *mlist, C2Mailbox *mailbox, C2WindowMain *wmain)
{
	pthread_t thread;

	pthread_create (&thread, NULL, C2_PTHREAD_FUNC (on_mlist_mailbox_selected_pthread), wmain);
}

static void
on_mlist_mailbox_unselected (C2MailboxList *mlist, C2WindowMain *wmain)
{
	if (!pthread_mutex_trylock (&wmain->index_lock))
	{
		GtkWidget *index = wmain->index;

		c2_index_remove_mailbox (C2_INDEX (index));
		pthread_mutex_unlock (&wmain->index_lock);
	}
}

static void
on_mlist_button_press_event (GtkWidget *widget, GdkEvent *event, C2WindowMain *wmain)
{
	if (event->button.button == 3)
	{
		/* Right Click */
		GdkEventButton *e = (GdkEventButton *) event;
		GtkCTreeNode *node;
		gint mbox_n, row, column;

		mbox_n = gtk_clist_get_selection_info (GTK_CLIST (widget), e->x, e->y, &row, &column);

		if (mbox_n)
		{
			C2Mailbox *mailbox;
			
			node = gtk_ctree_node_nth (GTK_CTREE (widget), row);
			
			mailbox = gtk_ctree_node_get_row_data (GTK_CTREE (widget), node);
			c2_mailbox_list_set_selected_mailbox (C2_MAILBOX_LIST (wmain->mlist), mailbox);
			gtk_ctree_select (GTK_CTREE (widget), node);
		} else
			c2_mailbox_list_set_selected_mailbox (C2_MAILBOX_LIST (wmain->mlist), NULL);

		gnome_popup_menu_do_popup (glade_xml_get_widget (wmain->ctree_menu, "mnu_ctree"),
										NULL, NULL, e, NULL);
	}
}

static void
on_file_new_mailbox_activate (GtkWidget *widget, C2WindowMain *wmain)
{
	c2_window_main_add_mailbox_dialog (wmain);
}

static gint
eastern_egg_timeout (C2Pthread3 *data)
{
	/* Que onda si uso GdkPixbuf (como lo de su ejemplo)
	 * para hacer alguna precentación onda copada?
	 */
}

static void
on_eastern_egg_separator_activate (GtkWidget *widget, C2WindowMain *wmain)
{
	C2Window *window = C2_WINDOW (wmain);
	GtkWidget *dialog;
	GladeXML *xml;
	C2Pthread3 *data;

	dialog = c2_dialog_new (window->application, _("You found the Eastern Egg!"),
							_("Neat, baby!"), NULL);
	xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("cronosII"), "dlg_eastern_egg_contents");
	C2_DIALOG (dialog)->xml = xml;
	
	gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (dialog)->vbox), glade_xml_get_widget (xml,
							"dlg_eastern_egg_contents"), TRUE, TRUE, 0);
	gtk_widget_set_usize (dialog, 500, 480);

	data = g_new0 (C2Pthread3, 1);
	data->v1 = (gpointer) dialog;
	data->v2 = (gpointer) g_new (gint, 1);
	data->v3 = (gpointer) g_new (gint, 1);

	GPOINTER_TO_INT (data->v3) = gtk_timeout_add (100, (GtkFunction) eastern_egg_timeout, data);

	gtk_widget_show (dialog);
}

static void
on_settings_preferences_activate (GtkWidget *widget, C2WindowMain *wmain)
{
	GtkWidget *preferences;

	preferences = c2_dialog_preferences_new (C2_WINDOW (wmain)->application);
	gtk_widget_show (preferences);	
}

static void
on_about_activate (GtkWidget *widget, C2WindowMain *wmain)
{
	C2HTML *html = C2_HTML (C2_MAIL (glade_xml_get_widget (C2_WINDOW (wmain)->xml, "mail"))->body);
	gchar *string;

	if (c2_get_file (PKGDATADIR G_DIR_SEPARATOR_S "about.html", &string) < 0)
	{
		c2_window_report (C2_WINDOW (wmain), C2_WINDOW_REPORT_WARNING,
							_("Unable to open about file"));
		return;
	}

	c2_html_set_content_from_string (html, string);
	g_free (string);
}

static void
on_new_mail_activate (GtkWidget *widget, C2WindowMain *wmain)
{
	GtkWidget *composer;
	GtkWidget *window;

	if (!c2_application_check_account_exists (C2_WINDOW (wmain)->application))
		return;

	composer = c2_composer_new (C2_WINDOW (wmain)->application);

	gtk_widget_show (window);
}

static void
on_getting_in_touch_activated (GtkWidget *widget)
{
	GladeXML *xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("cronosII"), "dlg_getting_in_touch");
	GtkWidget *dialog = glade_xml_get_widget (xml, "dlg_getting_in_touch");

	gnome_dialog_run_and_close (GNOME_DIALOG (dialog));

	gtk_object_unref (GTK_OBJECT (xml));
}

static void
on_mnu_toolbar_text_beside_icons_activate (GtkWidget *widget, C2WindowMain *wmain)
{
	GtkWidget *toolbar;

	toolbar = wmain->toolbar;
	c2_toolbar_set_style (C2_TOOLBAR (toolbar), C2_TOOLBAR_TEXT_BESIDE_ICON);
	gnome_config_set_int ("/"PACKAGE"/Toolbar::Window Main/type", C2_TOOLBAR_TEXT_BESIDE_ICON);
	gnome_config_sync ();
}

static void
on_mnu_toolbar_text_under_icons_activate (GtkWidget *widget, C2WindowMain *wmain)
{
	GtkWidget *toolbar;

	toolbar = wmain->toolbar;
	c2_toolbar_set_style (C2_TOOLBAR (toolbar), C2_TOOLBAR_TEXT_UNDER_ICON);
	gnome_config_set_int ("/"PACKAGE"/Toolbar::Window Main/type", C2_TOOLBAR_TEXT_UNDER_ICON);
	gnome_config_sync ();
}

static void
on_mnu_toolbar_icons_only_activate (GtkWidget *widget, C2WindowMain *wmain)
{
	GtkWidget *toolbar;

	toolbar = wmain->toolbar;
	c2_toolbar_set_style (C2_TOOLBAR (toolbar), C2_TOOLBAR_JUST_ICON);
	gnome_config_set_int ("/"PACKAGE"/Toolbar::Window Main/type", C2_TOOLBAR_JUST_ICON);
	gnome_config_sync ();
}

static void
on_mnu_toolbar_text_only_activate (GtkWidget *widget, C2WindowMain *wmain)
{
	GtkWidget *toolbar;

	toolbar = wmain->toolbar;
	c2_toolbar_set_style (C2_TOOLBAR (toolbar), C2_TOOLBAR_JUST_TEXT);
	gnome_config_set_int ("/"PACKAGE"/Toolbar::Window Main/type", C2_TOOLBAR_JUST_TEXT);
	gnome_config_sync ();
}

static void
on_mnu_toolbar_tooltips_toggled (GtkWidget *widget, C2WindowMain *wmain)
{
	GtkWidget *toolbar;
	gboolean active;

	toolbar = wmain->toolbar;
	active = GTK_CHECK_MENU_ITEM (widget)->active;
	c2_toolbar_set_tooltips (C2_TOOLBAR (toolbar), active);
	gnome_config_set_int ("/"PACKAGE"/Toolbar::Window Main/tooltips", active);
	gnome_config_sync ();
}

static void
on_mnu_toolbar_edit_toolbar_activate (GtkWidget *widget, C2WindowMain *wmain)
{
	c2_window_main_toolbar_configuration_dialog (wmain);
}

/***************************************************
 *                  [ DIALOGS ]                    *
 ***************************************************/
#if 1 /* Add Mailbox dialog */
static void
add_mailbox_dialog_type_selection_done (GtkWidget *widget, C2WindowMain *wmain)
{
	GladeXML *xml = GLADE_XML (gtk_object_get_data (GTK_OBJECT (wmain), "add_mailbox_dialog::xml"));
	GtkWidget *edata = glade_xml_get_widget (xml, "edata");
	GtkWidget *ehostl = glade_xml_get_widget (xml, "ehostl");
	GtkWidget *eportl = glade_xml_get_widget (xml, "eportl");
	GtkWidget *euserl = glade_xml_get_widget (xml, "euserl");
	GtkWidget *epassl = glade_xml_get_widget (xml, "epassl");
	GtkWidget *epathl = glade_xml_get_widget (xml, "epathl");
	GtkWidget *ehost = glade_xml_get_widget (xml, "ehost");
	GtkWidget *eport = glade_xml_get_widget (xml, "eport");
	GtkWidget *euser = glade_xml_get_widget (xml, "euser");
	GtkWidget *epass = glade_xml_get_widget (xml, "epass");
	GtkWidget *epath_imap = glade_xml_get_widget (xml, "epath_imap");
	GtkWidget *epath_spool = glade_xml_get_widget (xml, "epath_spool");
	GtkWidget *type = glade_xml_get_widget (xml, "type");
	gchar *selection;

	/* Get the selected type */
	if (GTK_BIN (type)->child)
	{
		GtkWidget *child = GTK_BIN (type)->child;

		if (GTK_LABEL (child))
		{
			gtk_label_get (GTK_LABEL (child), &selection);

			if (c2_streq (selection, MAILBOX_TYPE_CRONOSII))
				gtk_widget_hide (edata);
			else if (c2_streq (selection, MAILBOX_TYPE_IMAP))
			{
				gtk_widget_show (edata);
				gtk_widget_show (ehostl);
				gtk_widget_show (eportl);
				gtk_widget_show (euserl);
				gtk_widget_show (epassl);
				gtk_widget_show (epathl);
				gtk_widget_show (ehost);
				gtk_widget_show (eport);
				gtk_widget_show (euser);
				gtk_widget_show (epass);
				gtk_widget_show (epath_imap);
				gtk_widget_hide (epath_spool);
			} else if (c2_streq (selection, MAILBOX_TYPE_SPOOL))
			{
				gtk_widget_show (edata);
				gtk_widget_hide (ehostl);
				gtk_widget_hide (eportl);
				gtk_widget_hide (euserl);
				gtk_widget_hide (epassl);
				gtk_widget_show (epathl);
				gtk_widget_hide (ehost);
				gtk_widget_hide (eport);
				gtk_widget_hide (euser);
				gtk_widget_hide (epass);
				gtk_widget_hide (epath_imap);
				gtk_widget_show (epath_spool);
			}
		}
	}
}

void
c2_window_main_add_mailbox_dialog (C2WindowMain *wmain)
{
	GtkWidget *dialog;
	GtkWidget *menuitem, *wbuf;
	GtkWidget *menu;
	GtkOptionMenu *option_menu;
	GladeXML *xml;

	dialog = c2_dialog_new (C2_WINDOW (wmain)->application, _("New mailbox"), "new_mailbox", GNOME_STOCK_BUTTON_HELP,
							GNOME_STOCK_BUTTON_OK, GNOME_STOCK_BUTTON_CANCEL, NULL);
	xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("cronosII"), "dlg_mailbox_properties_contents");
	C2_DIALOG (dialog)->xml = xml;
	gtk_widget_set_usize (dialog, 400, -1);

	gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (dialog)->vbox), glade_xml_get_widget (xml,
							"dlg_mailbox_properties_contents"), TRUE, TRUE, 0);

	menu = gtk_menu_new ();
	option_menu = GTK_OPTION_MENU (glade_xml_get_widget (xml, "type"));
	gtk_signal_connect (GTK_OBJECT (menu), "selection_done",
						GTK_SIGNAL_FUNC (add_mailbox_dialog_type_selection_done), wmain);
	
	menuitem = gtk_menu_item_new_with_label (MAILBOX_TYPE_CRONOSII);
	gtk_menu_append (GTK_MENU (menu), menuitem);
	gtk_widget_show (menuitem);

	menuitem = gtk_menu_item_new_with_label (MAILBOX_TYPE_IMAP);
	gtk_menu_append (GTK_MENU (menu), menuitem);
	gtk_widget_show (menuitem);

	menuitem = gtk_menu_item_new_with_label (MAILBOX_TYPE_SPOOL);
	gtk_menu_append (GTK_MENU (menu), menuitem);
	gtk_widget_show (menuitem);

	gtk_option_menu_set_menu (option_menu, menu);
	gtk_option_menu_set_history (option_menu, 0);

	gtk_widget_grab_focus (glade_xml_get_widget (xml, "name"));

	gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (wmain));
	gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

	gtk_object_set_data (GTK_OBJECT (wmain), "add_mailbox_dialog::xml", xml);

	gnome_file_entry_set_default_path (GNOME_FILE_ENTRY (glade_xml_get_widget (xml, "epath_spool")),
									C2_WINDOW (wmain)->application->paths_get);

re_run_add_mailbox_dialog:
	switch (gnome_dialog_run (GNOME_DIALOG (dialog)))
	{
		case 1:
			{
				C2MailboxType type;
				gchar *name, *host, *user, *pass, *path;
				C2Mailbox *parent, *mailbox;
				gint port, config_id;
				gchar *query;

				name = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (xml, "name")));

				/* [TODO] Hehe, a little eastern egg :) */
				if (c2_streq (name, ""))
					;

				/* Check if the name is valid */
				if (!name || !strlen (name) ||
					c2_mailbox_get_by_name (C2_WINDOW (wmain)->application->mailbox, name))
				{
					GladeXML *err_xml;
					GtkWidget *err_dialog;
					
					err_xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("cronosII"), "dlg_mailbox_err");
					err_dialog = glade_xml_get_widget (err_xml, "dlg_mailbox_err");

					gtk_window_set_modal (GTK_WINDOW (err_dialog), TRUE);
					gnome_dialog_run_and_close (GNOME_DIALOG (err_dialog));

					gtk_object_destroy (GTK_OBJECT (err_xml));

					goto re_run_add_mailbox_dialog;
				}

				/* Check if the data is enough for the type */
				wbuf = glade_xml_get_widget (xml, "type");

				if (GTK_BIN (wbuf)->child)
				{
					GtkWidget *child = GTK_BIN (wbuf)->child;
					gchar *query;
					
					if (GTK_LABEL (child))
					{
						gtk_label_get (GTK_LABEL (child), &query);
						
						if (c2_streq (query, MAILBOX_TYPE_CRONOSII))
							type = C2_MAILBOX_CRONOSII;
						else if (c2_streq (query, MAILBOX_TYPE_IMAP))
						{
							type = C2_MAILBOX_IMAP;
							host = gtk_entry_get_text (GTK_ENTRY (
													glade_xml_get_widget (xml, "ehost")));
							user = gtk_entry_get_text (GTK_ENTRY (
													glade_xml_get_widget (xml, "euser")));
							pass = gtk_entry_get_text (GTK_ENTRY (
													glade_xml_get_widget (xml, "epass")));
							path = gtk_entry_get_text (GTK_ENTRY (
													glade_xml_get_widget (xml, "epath_imap")));
							port = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (
													glade_xml_get_widget (xml, "eport")));

							if (!strlen (host) ||
								!strlen (user) ||
								!strlen (path) ||
								!port)
							{
								GladeXML *err_xml;
								GtkWidget *err_dialog;

								err_xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("cronosII"),
															"dlg_mailbox_not_enough_data");
								err_dialog = glade_xml_get_widget (err_xml, "dlg_mailbox_not_enough_data");
								
								gtk_window_set_modal (GTK_WINDOW (err_dialog), TRUE);
								gnome_dialog_run_and_close (GNOME_DIALOG (err_dialog));
								
								gtk_object_destroy (GTK_OBJECT (err_xml));
								
								goto re_run_add_mailbox_dialog;
							}
						} else if (c2_streq (query, MAILBOX_TYPE_SPOOL))
						{
							type = C2_MAILBOX_SPOOL;
							path = gtk_entry_get_text (GTK_ENTRY (
													gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (
													glade_xml_get_widget (xml, "epath_spool")))));

							if (!strlen (path))
							{
								GladeXML *err_xml;
								GtkWidget *err_dialog;

								err_xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("cronosII"),
															"dlg_mailbox_not_enough_data");
								err_dialog = glade_xml_get_widget (err_xml, "dlg_mailbox_not_enough_data");
								
								gtk_window_set_modal (GTK_WINDOW (err_dialog), TRUE);
								gnome_dialog_run_and_close (GNOME_DIALOG (err_dialog));
								
								gtk_object_destroy (GTK_OBJECT (err_xml));
								
								goto re_run_add_mailbox_dialog;
							}
						}
					}
				}

				/* Get parent mailbox */
				parent = c2_mailbox_list_get_selected_mailbox (C2_MAILBOX_LIST (wmain->mlist));

				switch (type)
				{
					case C2_MAILBOX_CRONOSII:
						mailbox = c2_mailbox_new_with_parent (
											&C2_WINDOW (wmain)->application->mailbox,
											name, parent ? parent->id : NULL, type,
											C2_MAILBOX_SORT_DATE, GTK_SORT_ASCENDING);
						break;
					case C2_MAILBOX_IMAP:
						mailbox = c2_mailbox_new_with_parent (
											&C2_WINDOW (wmain)->application->mailbox,
											name, parent ? parent->id : NULL, type,
											C2_MAILBOX_SORT_DATE, GTK_SORT_ASCENDING,
											host, port, user, pass, path);
						break;
					case C2_MAILBOX_SPOOL:
						mailbox = c2_mailbox_new_with_parent (
											&C2_WINDOW (wmain)->application->mailbox,
											name, parent ? parent->id : NULL, type,
											C2_MAILBOX_SORT_DATE, GTK_SORT_ASCENDING, path);
						break;
				}

				if (!mailbox)
				{
					c2_window_report (C2_WINDOW (wmain), C2_WINDOW_REPORT_WARNING,
										_("Unable to create mailbox"));
					switch (type)
					{
						case C2_MAILBOX_IMAP:
							g_free (host);
							g_free (user);
							g_free (pass);
						case C2_MAILBOX_SPOOL:
							g_free (path);
						case C2_MAILBOX_CRONOSII:
							g_free (name);
					}

					return;
				}

				if (C2_WINDOW (wmain)->application->advanced_load_mailboxes_at_start)
					c2_mailbox_load_db (mailbox);

				/* If this is the first mailbox we need
				 * to connect the application to the
				 * signal changed_mailboxes and we
				 * also have to reemit the signal,
				 * so the application knows about it.
				 */
				if (!parent)
				{
					gtk_signal_connect (GTK_OBJECT (mailbox), "changed_mailboxes",
									GTK_SIGNAL_FUNC (on_mailbox_changed_mailboxes),
										C2_WINDOW (wmain)->application);
					gtk_signal_emit_by_name (GTK_OBJECT (mailbox), "changed_mailboxes");
				}
				
				config_id = gnome_config_get_int_with_default ("/"PACKAGE"/Mailboxes/quantity=0", NULL)+1;
				query = g_strdup_printf ("/"PACKAGE"/Mailbox %d/", config_id);
				gnome_config_push_prefix (query);
				
				gnome_config_set_string ("name", mailbox->name);
				gnome_config_set_string ("id", mailbox->id);
				gnome_config_set_int ("type", mailbox->type);
				gnome_config_set_int ("sort_by", mailbox->sort_by);
				gnome_config_set_int ("sort_type", mailbox->sort_type);
				
				switch (mailbox->type)
				{
					case C2_MAILBOX_SPOOL:
						gnome_config_set_string ("path", mailbox->protocol.spool.path);
						break;
				}
				gnome_config_pop_prefix ();
				g_free (query);
				
				gnome_config_set_int ("/"PACKAGE"/Mailboxes/quantity", config_id);
				gnome_config_sync ();
			}
		case 2:
			gtk_window_set_modal (GTK_WINDOW (dialog), FALSE);
			gtk_object_destroy (GTK_OBJECT (dialog));
			break;
		case 0:
			/* [TODO]
			 * c2_application_help_show (wmain->application, "c2help://add_mailbox_dialog");
			 */
			break;
	}		
}
#endif /* Add Mailbox Dialog */





#if 1 /* Toolbar configuration */

#define APPEND_SEPARATOR(__widget__, n)	\
	{ \
		gchar *__text [] = { "---", N_("Separator") }; \
		gtk_clist_append (GTK_CLIST (__widget__), __text); \
		gtk_clist_set_row_data (GTK_CLIST (__widget__), GTK_CLIST (__widget__)->rows-1, (gpointer) n);\
	}

static gint
get_separator_n								(void);

static void
on_toolbar_configuration_dialog_clist_select_row (GtkCList *clist, gint row, gint column,
				GdkEvent *e, GladeXML *xml)
{
	GtkWidget *widget;
	gboolean up_btn, down_btn, left_btn, right_btn;

	if (GTK_CLIST (glade_xml_get_widget (xml, "available_clist")) == clist)
	{
		up_btn = down_btn = left_btn = 0;
		right_btn = 1;
		gtk_clist_unselect_all (GTK_CLIST (glade_xml_get_widget (xml, "current_clist")));
	} else
	{
		gint n = GPOINTER_TO_INT (clist->selection->data);
		
		up_btn = n ? 1 : 0;
		down_btn = n != clist->rows-1 ? 1 : 0;
		right_btn = 0;
		left_btn = 1;
		gtk_clist_unselect_all (GTK_CLIST (glade_xml_get_widget (xml, "available_clist")));
	}

	widget = glade_xml_get_widget (xml, "up_btn");
	gtk_widget_set_sensitive (widget, up_btn);
	widget = glade_xml_get_widget (xml, "down_btn");
	gtk_widget_set_sensitive (widget, down_btn);
	widget = glade_xml_get_widget (xml, "left_btn");
	gtk_widget_set_sensitive (widget, left_btn);
	widget = glade_xml_get_widget (xml, "right_btn");
	gtk_widget_set_sensitive (widget, right_btn);
}

static void
on_toolbar_configuration_dialog_up_btn_clicked (GtkWidget *widget, GladeXML *xml)
{
	GtkCList *clist = GTK_CLIST (glade_xml_get_widget (xml, "current_clist"));
	gint n = GPOINTER_TO_INT (clist->selection->data);

	gtk_clist_swap_rows (clist, n, n-1);
	gtk_signal_emit_by_name (GTK_OBJECT (clist), "select_row", n-1, 0);
}

static void
on_toolbar_configuration_dialog_down_btn_clicked (GtkWidget *widget, GladeXML *xml)
{
	GtkCList *clist = GTK_CLIST (glade_xml_get_widget (xml, "current_clist"));
	gint n = GPOINTER_TO_INT (clist->selection->data);

	gtk_clist_swap_rows (clist, n, n+1);
	gtk_signal_emit_by_name (GTK_OBJECT (clist), "select_row", n+1, 0);
}

static void
on_toolbar_configuration_dialog_left_btn_clicked (GtkWidget *widget, GladeXML *xml)
{
	GtkCList *current_clist = GTK_CLIST (glade_xml_get_widget (xml, "current_clist"));
	GtkCList *available_clist = GTK_CLIST (glade_xml_get_widget (xml, "available_clist"));
	gpointer data;
	gint n = GPOINTER_TO_INT (current_clist->selection->data), sep;
	GdkPixmap *pixmap;
	GdkBitmap *mask;
	gchar *text;
	gchar *row[] = { NULL, NULL };

	data = gtk_clist_get_row_data (current_clist, n);
	sep = get_separator_n ();
	
	if (GPOINTER_TO_INT (data) == sep)
	{
		gtk_clist_remove (current_clist, n);
		return;
	}

	gtk_clist_get_pixmap (current_clist, n, 0, &pixmap, &mask);
	gtk_clist_get_text (current_clist, n, 1, &text);
	gtk_clist_set_pixmap (available_clist, available_clist->rows-1, 0, pixmap, mask);
	gtk_clist_set_text (available_clist, available_clist->rows-1, 1, text);
	gtk_clist_set_row_data (available_clist, available_clist->rows-1, data);
	APPEND_SEPARATOR (available_clist, sep);
	gtk_clist_remove (current_clist, n);
}

static void
on_toolbar_configuration_dialog_right_btn_clicked (GtkWidget *widget, GladeXML *xml)
{
	GtkCList *current_clist = GTK_CLIST (glade_xml_get_widget (xml, "current_clist"));
	GtkCList *available_clist = GTK_CLIST (glade_xml_get_widget (xml, "available_clist"));
	gpointer data;
	gint n = GPOINTER_TO_INT (available_clist->selection->data), sep;
	GdkPixmap *pixmap;
	GdkBitmap *mask;
	gchar *text;
	gchar *row[] = { NULL, NULL };

	data = gtk_clist_get_row_data (available_clist, n);
	sep = get_separator_n ();

	if (GPOINTER_TO_INT (data) == sep)
	{
		APPEND_SEPARATOR (current_clist, sep);
		return;
	}

	gtk_clist_get_pixmap (available_clist, n, 0, &pixmap, &mask);
	gtk_clist_get_text (available_clist, n, 1, &text);
	gtk_clist_append (current_clist, row);
	gtk_clist_set_pixmap (current_clist, current_clist->rows-1, 0, pixmap, mask);
	gtk_clist_set_text (current_clist, current_clist->rows-1, 1, text);
	gtk_clist_set_row_data (current_clist, current_clist->rows-1, data);
	gtk_clist_remove (available_clist, n);
}

void
c2_window_main_toolbar_configuration_dialog (C2WindowMain *wmain)
{
	GladeXML *xml;
	GtkWidget *dialog;
	GtkWidget *widget;
	C2ToolbarItem *item;
	gint list_length;
	GList *available = NULL;
	GList *current = NULL;
	GList *l;
	gint i;

	dialog = c2_dialog_new (C2_WINDOW (wmain)->application, _("Toolbar Configuration"),
							"toolbar_configuration", GNOME_STOCK_BUTTON_HELP,
							GNOME_STOCK_BUTTON_OK, GNOME_STOCK_BUTTON_CANCEL, NULL);
	xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("cronosII"), "dlg_toolbar_configuration_contents");
	C2_DIALOG (dialog)->xml = xml;
	gtk_widget_set_usize (dialog, 500, 385);

	gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (dialog)->vbox), glade_xml_get_widget (xml,
							"dlg_toolbar_configuration_contents"), TRUE, TRUE, 0);

	/* Load data (backend) */
	for (i = 0; toolbar_items[i].button_label || toolbar_items[i].button_pixmap; i++)
		available = g_list_append (available, (gpointer) i);

	for (i = 1;; i++)
	{
		gchar *key;
		gint val;

		key = g_strdup_printf ("/"PACKAGE"/Toolbar::Window Main/item %d=-1", i);
		val = gnome_config_get_int_with_default (key, NULL);

		if (val < 0)
			break;

		for (l = available; l; l = g_list_next (l))
			if (GPOINTER_TO_INT (l->data) == val)
				available = g_list_remove_link (available, l);

		current = g_list_append (current, (gpointer) val);
	}

	/* Load data (UI) */
	widget = glade_xml_get_widget (xml, "available_clist");
	gtk_clist_set_row_height (GTK_CLIST (widget), 26);
	gtk_clist_set_column_width (GTK_CLIST (widget), 0, 26);
	gtk_clist_set_column_width (GTK_CLIST (widget), 1, 80);
	GTK_WIDGET_UNSET_FLAGS (widget, GTK_CAN_FOCUS);

	gtk_clist_freeze (GTK_CLIST (widget));
	for (l = available; l; l = g_list_next (l))
	{
		GtkWidget *pixmap;
		gint n = GPOINTER_TO_INT (l->data);
		gchar *text[] = { NULL, NULL };
		
		gtk_clist_append (GTK_CLIST (widget), text);
		gtk_clist_set_row_data (GTK_CLIST (widget), GTK_CLIST (widget)->rows-1, (gpointer) n);
		
		if (toolbar_items[n].button_pixmap)
		{
			pixmap = gnome_pixmap_new_from_file_at_size (toolbar_items[n].button_pixmap, 24, 24);
			gtk_clist_set_pixmap (GTK_CLIST (widget), GTK_CLIST (widget)->rows-1,
									0, GNOME_PIXMAP (pixmap)->pixmap, GNOME_PIXMAP (pixmap)->mask);
		}

		if (toolbar_items[n].button_label)
		{
			gtk_clist_set_text (GTK_CLIST (widget), GTK_CLIST (widget)->rows-1, 1,
								toolbar_items[n].button_label);
		}
	}

	APPEND_SEPARATOR (widget, get_separator_n ());	
	gtk_clist_thaw (GTK_CLIST (widget));
	gtk_signal_connect (GTK_OBJECT (widget), "select_row",
						GTK_SIGNAL_FUNC (on_toolbar_configuration_dialog_clist_select_row), xml);

	widget = glade_xml_get_widget (xml, "current_clist");
	gtk_clist_set_row_height (GTK_CLIST (widget), 26);
	gtk_clist_set_column_width (GTK_CLIST (widget), 0, 26);
	gtk_clist_set_column_width (GTK_CLIST (widget), 1, 80);
	GTK_WIDGET_UNSET_FLAGS (widget, GTK_CAN_FOCUS);

	gtk_clist_freeze (GTK_CLIST (widget));
	for (l = current; l; l = g_list_next (l))
	{
		GtkWidget *pixmap;
		gint n = GPOINTER_TO_INT (l->data);
		gchar *text[] = { NULL, NULL };
		
		gtk_clist_append (GTK_CLIST (widget), text);
		gtk_clist_set_row_data (GTK_CLIST (widget), GTK_CLIST (widget)->rows-1, (gpointer) n);
		
		if (toolbar_items[n].button_pixmap)
		{
			pixmap = gnome_pixmap_new_from_file_at_size (toolbar_items[n].button_pixmap, 24, 24);
			gtk_clist_set_pixmap (GTK_CLIST (widget), GTK_CLIST (widget)->rows-1,
									0, GNOME_PIXMAP (pixmap)->pixmap, GNOME_PIXMAP (pixmap)->mask);
		}

		if (toolbar_items[n].button_label)
		{
			gtk_clist_set_text (GTK_CLIST (widget), GTK_CLIST (widget)->rows-1, 1,
								toolbar_items[n].button_label);
		}

		if (!toolbar_items[n].button_label && !toolbar_items[n].button_pixmap)
		{
			/* This is a separator */
			gtk_clist_set_text (GTK_CLIST (widget), GTK_CLIST (widget)->rows-1, 0,
								"---");
			gtk_clist_set_text (GTK_CLIST (widget), GTK_CLIST (widget)->rows-1, 1,
								_("Separator"));
			
		}
	}
	gtk_clist_thaw (GTK_CLIST (widget));
	gtk_signal_connect (GTK_OBJECT (widget), "select_row",
						GTK_SIGNAL_FUNC (on_toolbar_configuration_dialog_clist_select_row), xml);

	widget = glade_xml_get_widget (xml, "up_btn");
	gtk_signal_connect (GTK_OBJECT (widget), "clicked",
						GTK_SIGNAL_FUNC (on_toolbar_configuration_dialog_up_btn_clicked), xml);
	widget = glade_xml_get_widget (xml, "down_btn");
	gtk_signal_connect (GTK_OBJECT (widget), "clicked",
						GTK_SIGNAL_FUNC (on_toolbar_configuration_dialog_down_btn_clicked), xml);
	widget = glade_xml_get_widget (xml, "left_btn");
	gtk_signal_connect (GTK_OBJECT (widget), "clicked",
						GTK_SIGNAL_FUNC (on_toolbar_configuration_dialog_left_btn_clicked), xml);
	widget = glade_xml_get_widget (xml, "right_btn");
	gtk_signal_connect (GTK_OBJECT (widget), "clicked",
						GTK_SIGNAL_FUNC (on_toolbar_configuration_dialog_right_btn_clicked), xml);

	switch (gnome_dialog_run (GNOME_DIALOG (dialog)))
	{
		case 0:
			break;
		case 1:
			{ /* Save the configuration */
				gint n;

				widget = glade_xml_get_widget (xml, "current_clist");
				
				for (n = 1; n <= GTK_CLIST (widget)->rows; n++)
				{
					gchar *key = g_strdup_printf ("/"PACKAGE"/Toolbar::Window Main/item %d", n);

					gnome_config_set_int (key, GPOINTER_TO_INT (
											gtk_clist_get_row_data (GTK_CLIST (widget), n-1)));
				}

				/* Delete any other key */
				for (;; n++)
				{
					gchar *key = g_strdup_printf ("/"PACKAGE"/Toolbar::Window Main/item %d=-1", n);
					gint k = gnome_config_get_int_with_default (key, NULL);
					
					if (k < 0)
					{
						g_free (key);
						break;
					}
					gnome_config_clean_key (key);
					g_free (key);
				}

				gnome_config_sync ();
			}

			{
				c2_toolbar_clear (C2_TOOLBAR (wmain->toolbar));
				for (i = 1;; i++)
				{
					gchar *key;
					gint val, df;
					
					key = g_strdup_printf ("/"PACKAGE"/Toolbar::Window Main/item %d=-1", i);
					val = gnome_config_get_int_with_default (key, &df);
					
					if (val < 0)
						break;
					
					switch (toolbar_items[val].type)
					{
						case C2_TOOLBAR_BUTTON:
							widget = c2_toolbar_append_button (C2_TOOLBAR (wmain->toolbar),
													toolbar_items[val].button_pixmap,
													toolbar_items[val].button_label,
													toolbar_items[val].button_tooltip,
													toolbar_items[val].button_force_label);
							if (toolbar_items[val].button_func)
								gtk_signal_connect (GTK_OBJECT (widget), "clicked",
											toolbar_items[val].button_func,
											toolbar_items[val].button_data ? toolbar_items[val].button_data :
											wmain);
							break;
						case C2_TOOLBAR_WIDGET:
							c2_toolbar_append_widget (C2_TOOLBAR (wmain->toolbar), toolbar_items[val].widget,
											toolbar_items[val].widget_tooltip);
							break;
						case C2_TOOLBAR_SPACE:
							c2_toolbar_append_space (C2_TOOLBAR (wmain->toolbar));
							break;
					}
				}
			}
		case 2:
			gnome_dialog_close (GNOME_DIALOG (dialog));
			break;

	}
	gtk_object_destroy (GTK_OBJECT (xml));
}

static gint
get_separator_n (void)
{
	gint i;

	for (i = 0;; i++)
		if (toolbar_items[i].type == C2_TOOLBAR_SPACE)
			return i;

	return 0;
}

#endif /* Toolbar configuration */
