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

#include "main.h"
#include "preferences.h"
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

static void
check										(C2WindowMain *wmain);

static void
close_										(C2WindowMain *wmain);

static void
compose										(C2WindowMain *wmain);

static void
contacts									(C2WindowMain *wmain);

static void
copy										(C2WindowMain *wmain);

static void
delete										(C2WindowMain *wmain);

static void
exit_										(C2WindowMain *wmain);

static void
expunge										(C2WindowMain *wmain);

static void
forward										(C2WindowMain *wmain);

static void
move										(C2WindowMain *wmain);

static void
next										(C2WindowMain *wmain);

static void
previous									(C2WindowMain *wmain);

static void
print										(C2WindowMain *wmain);

static void
reply										(C2WindowMain *wmain);

static void
reply_all									(C2WindowMain *wmain);

static void
save										(C2WindowMain *wmain);

static void
search										(C2WindowMain *wmain);

static void
send_										(C2WindowMain *wmain);

static gint
on_delete_event								(GtkWidget *widget, GdkEventAny *event, gpointer data);

static void
on_size_allocate							(C2WindowMain *wmain, GtkAllocation *alloc);

static void
on_docktoolbar_button_press_event			(GtkWidget *widget, GdkEventButton *event, C2WindowMain *wmain);

static void
on_toolbar_changed							(GtkWidget *widget, C2WindowMain *wmain);

static void
on_toolbar_check_clicked					(GtkWidget *widget, C2WindowMain *wmain);

static void
on_toolbar_close_clicked					(GtkWidget *widget, C2WindowMain *wmain);

static void
on_toolbar_compose_clicked					(GtkWidget *widget, C2WindowMain *wmain);

static void
on_toolbar_contacts_clicked					(GtkWidget *widget, C2WindowMain *wmain);

static void
on_toolbar_copy_clicked						(GtkWidget *widget, C2WindowMain *wmain);

static void
on_toolbar_delete_clicked					(GtkWidget *widget, C2WindowMain *wmain);

static void
on_toolbar_exit_clicked						(GtkWidget *widget, C2WindowMain *wmain);

static void
on_toolbar_forward_clicked					(GtkWidget *widget, C2WindowMain *wmain);

static void
on_toolbar_move_clicked						(GtkWidget *widget, C2WindowMain *wmain);

static void
on_toolbar_next_clicked						(GtkWidget *widget, C2WindowMain *wmain);

static void
on_toolbar_previous_clicked					(GtkWidget *widget, C2WindowMain *wmain);

static void
on_toolbar_print_clicked					(GtkWidget *widget, C2WindowMain *wmain);

static void
on_toolbar_reply_clicked					(GtkWidget *widget, C2WindowMain *wmain);

static void
on_toolbar_reply_all_clicked				(GtkWidget *widget, C2WindowMain *wmain);

static void
on_toolbar_save_clicked						(GtkWidget *widget, C2WindowMain *wmain);

static void
on_toolbar_search_clicked					(GtkWidget *widget, C2WindowMain *wmain);

static void
on_toolbar_send_clicked						(GtkWidget *widget, C2WindowMain *wmain);

static void
on_index_select_message						(GtkWidget *index, C2Db *node, C2WindowMain *wmain);

static void
on_mlist_object_selected					(C2MailboxList *mlist, GtkObject *object, C2WindowMain *wmain);

static void
on_mlist_object_unselected					(C2MailboxList *mlist, C2WindowMain *wmain);

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
on_help_release_information_activate		(GtkWidget *widget, C2WindowMain *wmain);

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

static gint
dlg_confirm_delete_message					(C2WindowMain *wmain);

static gboolean
dlg_confirm_expunge_message					(C2WindowMain *wmain);

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
		GTK_SIGNAL_FUNC (on_toolbar_send_clicked), NULL,
		NULL, NULL
	},
	{
		C2_TOOLBAR_BUTTON,
		N_("Search"), PKGDATADIR "/pixmaps/find.png",
		N_("Search a message in existent mailboxes"), TRUE,
		GTK_SIGNAL_FUNC (on_toolbar_search_clicked), NULL,
		NULL, NULL
	},
	{
		C2_TOOLBAR_BUTTON,
		N_("Save"), PKGDATADIR "/pixmaps/save.png",
		N_("Save selected message"), TRUE,
		GTK_SIGNAL_FUNC (on_toolbar_save_clicked), NULL,
		NULL, NULL
	},
	{
		C2_TOOLBAR_BUTTON,
		N_("Print"), PKGDATADIR "/pixmaps/print.png",
		N_("Print selected message"), TRUE,
		GTK_SIGNAL_FUNC (on_toolbar_print_clicked), NULL,
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
		GTK_SIGNAL_FUNC (on_toolbar_copy_clicked), NULL,
		NULL, NULL
	},
	{
		C2_TOOLBAR_BUTTON,
		N_("Move"), PKGDATADIR "/pixmaps/move-message.png",
		N_("Move selected mails"), TRUE,
		GTK_SIGNAL_FUNC (on_toolbar_move_clicked), NULL,
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
		GTK_SIGNAL_FUNC (on_toolbar_reply_all_clicked), NULL,
		NULL, NULL,
	},
	{
		C2_TOOLBAR_BUTTON,
		N_("Forward"), PKGDATADIR "/pixmaps/forward.png",
		N_("Forward selected message"), TRUE,
		GTK_SIGNAL_FUNC (on_toolbar_forward_clicked), NULL,
		NULL, NULL
	},
	{
		C2_TOOLBAR_BUTTON,
		N_("Previous"), PKGDATADIR "/pixmaps/prev.png",
		N_("Select previous message"), FALSE,
		GTK_SIGNAL_FUNC (on_toolbar_previous_clicked), NULL,
		NULL, NULL
	},
	{
		C2_TOOLBAR_BUTTON,
		N_("Next"), PKGDATADIR "/pixmaps/next.png",
		N_("Select next message"), FALSE,
		GTK_SIGNAL_FUNC (on_toolbar_next_clicked), NULL,
		NULL, NULL
	},
	{
		C2_TOOLBAR_BUTTON,
		N_("Contacts"), PKGDATADIR "/pixmaps/contacts.png",
		N_("Open the Contacts Address Book"), TRUE,
		GTK_SIGNAL_FUNC (on_toolbar_contacts_clicked), NULL,
		NULL, NULL
	},
	{
		C2_TOOLBAR_BUTTON,
		N_("Close"), PKGDATADIR "/pixmaps/close.png",
		N_("Close the main window of Cronos II"), TRUE,
		GTK_SIGNAL_FUNC (on_toolbar_close_clicked), NULL,
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

	klass->check = check;
	klass->close = close_;
	klass->compose = compose;
	klass->contacts = contacts;
	klass->copy = copy;
	klass->delete = delete;
	klass->exit = exit_;
	klass->expunge = expunge;
	klass->forward = forward;
	klass->move = move;
	klass->next = next;
	klass->previous = previous;
	klass->print = print;
	klass->reply = reply;
	klass->reply_all = reply_all;
	klass->save = save;
	klass->search = search;
	klass->send = send_;
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
	gtk_widget_set_usize (GTK_WIDGET (wmain),
						c2_preferences_get_window_main_width (),
						c2_preferences_get_window_main_height ());
	gtk_window_set_policy (GTK_WINDOW (wmain), TRUE, TRUE, FALSE);
	gtk_signal_connect (GTK_OBJECT (wmain), "delete_event",
							GTK_SIGNAL_FUNC (on_delete_event), NULL);
	gtk_signal_connect (GTK_OBJECT (wmain), "size_allocate",
							GTK_SIGNAL_FUNC (on_size_allocate), NULL);

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
	gtk_paned_set_position (GTK_PANED (hpaned),
						c2_preferences_get_window_main_hpaned ());

	/* Mailbox List */
	wmain->mlist = c2_mailbox_list_new (application);
	scroll = glade_xml_get_widget (xml, "mlist_scroll");
	gtk_container_add (GTK_CONTAINER (scroll), wmain->mlist);
	gtk_widget_show (wmain->mlist);

	gtk_signal_connect (GTK_OBJECT (wmain->mlist), "object_selected",
								GTK_SIGNAL_FUNC (on_mlist_object_selected), wmain);
	gtk_signal_connect (GTK_OBJECT (wmain->mlist), "object_unselected",
								GTK_SIGNAL_FUNC (on_mlist_object_unselected), wmain);
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
		g_free (key);

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
	gtk_paned_set_position (GTK_PANED (vpaned), 
						c2_preferences_get_window_main_vpaned ());

	appbar = glade_xml_get_widget (xml, "appbar");

	/* Index */
	index_scroll = glade_xml_get_widget (xml, "index_scroll");
	wmain->index = c2_index_new (application, C2_INDEX_READ_WRITE);
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
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "help_release_information")), "activate",
							GTK_SIGNAL_FUNC (on_help_release_information_activate), wmain);

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

static void
check (C2WindowMain *wmain)
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

		if (!GPOINTER_TO_INT (data) || account->type == C2_ACCOUNT_IMAP)
			continue;

		wti = c2_transfer_item_new (application, account, C2_TRANSFER_ITEM_RECEIVE);
		c2_transfer_list_add_item (C2_TRANSFER_LIST (wtl), wti);
		c2_transfer_item_start (wti);
	}
}

static void
close_ (C2WindowMain *wmain)
{
	gtk_object_destroy (GTK_OBJECT (wmain));
}

static void
compose (C2WindowMain *wmain)
{
	GtkWidget *composer;
	
	composer = c2_composer_new (C2_WINDOW (wmain)->application);
	gtk_widget_show (composer);
}

static void
contacts (C2WindowMain *wmain)
{
}

static void
copy_thread (C2Pthread4 *data)
{
	C2WindowMain *wmain = C2_WINDOW_MAIN (data->v1);
	C2Mailbox *fmailbox = C2_MAILBOX (data->v2);
	C2Mailbox *tmailbox = C2_MAILBOX (data->v3);
	GList *list = (GList*) data->v4;
	GList *l;
	GtkWidget *widget;
	GtkProgress *progress;
	gint length, off;
	gboolean progress_ownership;
	gboolean status_ownership;
	
	g_free (data);

	/* Get the length of our list */
	length = g_list_length (list);

	widget = glade_xml_get_widget (C2_WINDOW (wmain)->xml, "appbar");

	/* Try to reserve ownership over the progress bar */
	if (!pthread_mutex_trylock (&C2_WINDOW (wmain)->progress_lock))
		progress_ownership = TRUE;
	else
		progress_ownership = FALSE;

	/* Try to reserver ownership over the status bar */
	if (!pthread_mutex_trylock (&C2_WINDOW (wmain)->status_lock))
		status_ownership = TRUE;
	else
		status_ownership = FALSE;

	gdk_threads_enter ();
	
	if (progress_ownership)
	{
		/* Configure the progress bar */
		progress = GTK_PROGRESS (GNOME_APPBAR (widget)->progress);
		gtk_progress_configure (progress, 0, 0, length);
	}

	if (status_ownership)
	{
		/* Configure the status bar */
		gnome_appbar_push (GNOME_APPBAR (widget), _("Copying..."));
	}

	gdk_threads_leave ();
	
	c2_db_freeze (tmailbox);
	for (l = list, off = 0; l; l = g_list_next (l), off++)
	{
		C2Db *db;
		
		/* Now do the actual copy */
		db = c2_db_get_node (fmailbox, GPOINTER_TO_INT (l->data));

		if (!db->message)
			c2_db_load_message (db);
		
		gtk_object_ref (GTK_OBJECT (db->message));
		c2_db_message_add (tmailbox, db->message);
		gtk_object_unref (GTK_OBJECT (db->message));

		if (progress_ownership)
		{
			gdk_threads_enter ();
			gtk_progress_set_value (progress, off);
			gdk_threads_leave ();
		}
	}
	c2_db_thaw (tmailbox);

	gdk_threads_enter ();

	if (status_ownership)
	{
		gnome_appbar_pop (GNOME_APPBAR (widget));
		pthread_mutex_unlock (&C2_WINDOW (wmain)->status_lock);
	}

	if (progress_ownership)
	{
		gtk_progress_set_percentage (progress, 1.0);
		pthread_mutex_unlock (&C2_WINDOW (wmain)->progress_lock);
	}

	gdk_threads_leave ();
}

static void
copy (C2WindowMain *wmain)
{
	GtkWidget *widget;
	C2Mailbox *fmailbox, *tmailbox;
	C2Pthread4 *data;
	pthread_t thread;

	if (!GTK_CLIST (wmain->index)->selection)
		return;

	fmailbox = c2_mailbox_list_get_selected_mailbox (C2_MAILBOX_LIST (wmain->mlist));
	if (!fmailbox)
	{
		c2_window_report (C2_WINDOW (wmain), C2_WINDOW_REPORT_WARNING, error_list[C2_NO_MAILBOX_SELECTED]);
		return;
	}
	
	if (!(tmailbox = c2_application_dialog_select_mailbox (
									C2_WINDOW (wmain)->application, GTK_WINDOW (wmain))))
	{
		c2_window_report (C2_WINDOW (wmain), C2_WINDOW_REPORT_MESSAGE, error_list[C2_CANCEL_USER]);
		return;
	}

	data = g_new0 (C2Pthread4, 1);
	data->v1 = wmain;
	data->v2 = fmailbox;
	data->v3 = tmailbox;
	data->v4 = GTK_CLIST (wmain->index)->selection;
	pthread_create (&thread, NULL, C2_PTHREAD_FUNC (copy_thread), data);
}

static void
delete_thread (C2Pthread3 *data)
{
	C2WindowMain *wmain = C2_WINDOW_MAIN (data->v1);
	C2Mailbox *fmailbox = C2_MAILBOX (data->v2);
	C2Mailbox *tmailbox = c2_mailbox_get_by_name (C2_WINDOW (wmain)->application->mailbox,
													C2_MAILBOX_TRASH);
	GList *list = (GList*) data->v3;
	GList *l;
	GtkWidget *widget;
	GtkProgress *progress;
	gint length, off;
	gboolean progress_ownership;
	gboolean status_ownership;
	
	g_free (data);

	/* Get the length of our list */
	length = g_list_length (list);

	widget = glade_xml_get_widget (C2_WINDOW (wmain)->xml, "appbar");

	/* Try to reserve ownership over the progress bar */
	if (!pthread_mutex_trylock (&C2_WINDOW (wmain)->progress_lock))
		progress_ownership = TRUE;
	else
		progress_ownership = FALSE;

	/* Try to reserver ownership over the status bar */
	if (!pthread_mutex_trylock (&C2_WINDOW (wmain)->status_lock))
		status_ownership = TRUE;
	else
		status_ownership = FALSE;

	gdk_threads_enter ();
	
	if (progress_ownership)
	{
		/* Configure the progress bar */
		progress = GTK_PROGRESS (GNOME_APPBAR (widget)->progress);
		gtk_progress_configure (progress, 0, 0, length);
	}

	if (status_ownership)
	{
		/* Configure the status bar */
		gnome_appbar_push (GNOME_APPBAR (widget), _("Deleting..."));
	}

	gdk_threads_leave ();
	
	c2_db_freeze (fmailbox);
	c2_db_freeze (tmailbox);
	for (l = list, off = 0; l; l = g_list_next (l))
	{
		C2Db *db;
		
		/* Now do the actual copy */
		db = c2_db_get_node (fmailbox, GPOINTER_TO_INT (l->data));

		if (!db->message)
		{
			if (c2_db_load_message (db) < 0)
			{
				c2_window_report (C2_WINDOW (wmain), C2_WINDOW_REPORT_WARNING,
									error_list[C2_FAIL_MESSAGE_LOAD], c2_error_get ());
				continue;
			}
		}
		
		gtk_object_ref (GTK_OBJECT (db->message));
		if (!(c2_db_message_add (tmailbox, db->message) < 0))
			c2_db_message_remove (fmailbox, GPOINTER_TO_INT (l->data)-(off++));
		gtk_object_unref (GTK_OBJECT (db->message));

		if (progress_ownership)
		{
			gdk_threads_enter ();
			gtk_progress_set_value (progress, off);
			gdk_threads_leave ();
		}
	}
	c2_db_thaw (tmailbox);
	c2_db_thaw (fmailbox);

	g_list_free (list);

	gdk_threads_enter ();

	if (status_ownership)
	{
		gnome_appbar_pop (GNOME_APPBAR (widget));
		pthread_mutex_unlock (&C2_WINDOW (wmain)->status_lock);
	}

	if (progress_ownership)
	{
		gtk_progress_set_percentage (progress, 1.0);
		pthread_mutex_unlock (&C2_WINDOW (wmain)->progress_lock);
	}

	gdk_threads_leave ();
}

static void
delete (C2WindowMain *wmain)
{
	C2Mailbox *fmailbox = c2_mailbox_list_get_selected_mailbox (C2_MAILBOX_LIST (wmain->mlist));
	C2Pthread3 *data;
	pthread_t thread;

	/* If there's nothing selected there's nothing to delete */
	if (!GTK_CLIST (wmain->index)->selection)
		return;

	if (c2_preferences_get_general_options_delete_use_trash ())
	{
		/* We have to save in «Trash» */
		if (c2_streq (fmailbox->name, C2_MAILBOX_TRASH))
			/* This is already «Trash», we have to expunge */
			goto expunge;
		
		/* Ask for confirmation (if we are supposed to) */
		if (c2_preferences_get_general_options_delete_confirmation ())
		{
			if (!dlg_confirm_delete_message (wmain))
			{
				c2_window_report (C2_WINDOW (wmain), C2_WINDOW_REPORT_MESSAGE,
									error_list[C2_CANCEL_USER]);
				return;
			}
		}

		/* Ok, we are ready to move everything to «Trash» */
		data = g_new0 (C2Pthread3, 1);
		data->v1 = wmain;
		data->v2 = fmailbox;
		data->v3 = g_list_copy (GTK_CLIST (wmain->index)->selection);
		pthread_create (&thread, NULL, C2_PTHREAD_FUNC (delete_thread), data);
	} else
	{ /* We have to expunge */
expunge:
		C2_WINDOW_MAIN_CLASS_FW (wmain)->expunge (wmain);
	}
}

static void
exit_ (C2WindowMain *wmain)
{
/* TODO	c2_application_finish (C2_WINDOW (wmain)->application); */
	gtk_object_destroy (GTK_OBJECT (wmain));
}

static void
expunge_thread (C2WindowMain *wmain)
{
	C2Mailbox *mailbox = c2_mailbox_list_get_selected_mailbox (C2_MAILBOX_LIST (wmain->mlist));
	
	c2_db_message_remove (mailbox, GTK_CLIST (wmain->index)->selection);
}

static void
expunge (C2WindowMain *wmain)
{
	pthread_t thread;

	if (!GTK_CLIST (wmain->index)->selection)
		return;
	
	/* Ask for confirmation */
	if (!dlg_confirm_expunge_message (wmain))
	{
		c2_window_report (C2_WINDOW (wmain), C2_WINDOW_REPORT_MESSAGE,
							error_list[C2_CANCEL_USER]);
		return;
	}

	pthread_create (&thread, NULL, C2_PTHREAD_FUNC (expunge_thread), wmain);
}

static void
forward (C2WindowMain *wmain)
{
	GtkWidget *composer;
	GtkWidget *mail;
	C2Message *message;
	
	mail = glade_xml_get_widget (C2_WINDOW (wmain)->xml, "mail");
	message = c2_mail_get_message (C2_MAIL (mail));
	composer = c2_composer_new (C2_WINDOW (wmain)->application);
	c2_composer_set_message_as_forward (C2_COMPOSER (composer), message);
	gtk_widget_show (composer);
}

static void
move_thread (C2Pthread4 *data)
{
	C2WindowMain *wmain = C2_WINDOW_MAIN (data->v1);
	C2Mailbox *fmailbox = C2_MAILBOX (data->v2);
	C2Mailbox *tmailbox = C2_MAILBOX (data->v3);
	GList *list = (GList*) data->v4;
	GList *l;
	GtkWidget *widget;
	GtkProgress *progress;
	gint length, off;
	gboolean progress_ownership;
	gboolean status_ownership;
	
	g_free (data);

	/* Get the length of our list */
	length = g_list_length (list);

	widget = glade_xml_get_widget (C2_WINDOW (wmain)->xml, "appbar");

	/* Try to reserve ownership over the progress bar */
	if (!pthread_mutex_trylock (&C2_WINDOW (wmain)->progress_lock))
		progress_ownership = TRUE;
	else
		progress_ownership = FALSE;

	/* Try to reserver ownership over the status bar */
	if (!pthread_mutex_trylock (&C2_WINDOW (wmain)->status_lock))
		status_ownership = TRUE;
	else
		status_ownership = FALSE;

	gdk_threads_enter ();
	
	if (progress_ownership)
	{
		/* Configure the progress bar */
		progress = GTK_PROGRESS (GNOME_APPBAR (widget)->progress);
		gtk_progress_configure (progress, 0, 0, length);
	}

	if (status_ownership)
	{
		/* Configure the status bar */
		gnome_appbar_push (GNOME_APPBAR (widget), _("Moveing..."));
	}

	gdk_threads_leave ();
	
	c2_db_freeze (fmailbox);
	c2_db_freeze (tmailbox);
	for (l = list, off = 0; l; l = g_list_next (l), off++)
	{
		C2Db *db;
		
		/* Now do the actual copy */
		db = c2_db_get_node (fmailbox, GPOINTER_TO_INT (l->data));

		if (!db->message)
		{
			if (c2_db_load_message (db) < 0)
			{
				c2_window_report (C2_WINDOW (wmain), C2_WINDOW_REPORT_WARNING,
									error_list[C2_FAIL_MESSAGE_LOAD], c2_error_get ());
				continue;
			}
		}
		
		gtk_object_ref (GTK_OBJECT (db->message));
		if (!(c2_db_message_add (tmailbox, db->message) < 0))
			c2_db_message_remove (fmailbox, GPOINTER_TO_INT (l->data));
		gtk_object_unref (GTK_OBJECT (db->message));

		if (progress_ownership)
		{
			gdk_threads_enter ();
			gtk_progress_set_value (progress, off);
			gdk_threads_leave ();
		}
	}
	c2_db_thaw (tmailbox);
	c2_db_thaw (fmailbox);

	g_list_free (list);

	gdk_threads_enter ();

	if (status_ownership)
	{
		gnome_appbar_pop (GNOME_APPBAR (widget));
		pthread_mutex_unlock (&C2_WINDOW (wmain)->status_lock);
	}

	if (progress_ownership)
	{
		gtk_progress_set_percentage (progress, 1.0);
		pthread_mutex_unlock (&C2_WINDOW (wmain)->progress_lock);
	}

	gdk_threads_leave ();
}

static void
move (C2WindowMain *wmain)
{
	GtkWidget *widget;
	C2Mailbox *fmailbox, *tmailbox;
	C2Pthread4 *data;
	pthread_t thread;

	if (!GTK_CLIST (wmain->index)->selection)
		return;

	fmailbox = c2_mailbox_list_get_selected_mailbox (C2_MAILBOX_LIST (wmain->mlist));
	if (!(tmailbox = c2_application_dialog_select_mailbox (
									C2_WINDOW (wmain)->application, GTK_WINDOW (wmain))))
	{
		c2_window_report (C2_WINDOW (wmain), C2_WINDOW_REPORT_MESSAGE, error_list[C2_CANCEL_USER]);
		return;
	}

	data = g_new0 (C2Pthread4, 1);
	data->v1 = wmain;
	data->v2 = fmailbox;
	data->v3 = tmailbox;
	data->v4 = g_list_copy (GTK_CLIST (wmain->index)->selection);
	pthread_create (&thread, NULL, C2_PTHREAD_FUNC (move_thread), data);
}

static void
next (C2WindowMain *wmain)
{
}

static void
previous (C2WindowMain *wmain)
{
}

static void
print (C2WindowMain *wmain)
{
}

static void
reply (C2WindowMain *wmain)
{
	GtkWidget *composer;
	GtkWidget *mail;
	C2Message *message;
	
	mail = glade_xml_get_widget (C2_WINDOW (wmain)->xml, "mail");
	message = c2_mail_get_message (C2_MAIL (mail));
	composer = c2_composer_new (C2_WINDOW (wmain)->application);
	c2_composer_set_message_as_reply (C2_COMPOSER (composer), message);
	gtk_widget_show (composer);
}

static void
reply_all (C2WindowMain *wmain)
{
	GtkWidget *composer;
	GtkWidget *mail;
	C2Message *message;
	
	mail = glade_xml_get_widget (C2_WINDOW (wmain)->xml, "mail");
	message = c2_mail_get_message (C2_MAIL (mail));
	composer = c2_composer_new (C2_WINDOW (wmain)->application);
	c2_composer_set_message_as_reply_all (C2_COMPOSER (composer), message);
	gtk_widget_show (composer);
}

static void
on_save_ok_clicked (GtkWidget *widget, gint *button)
{
	*button = 1;
	gtk_main_quit ();
}

static void
on_save_cancel_clicked (GtkWidget *widget, gint *button)
{
	*button = 0;
	gtk_main_quit ();
}

static void
on_save_delete_event (GtkWidget *widget, GdkEvent *e, gint *button)
{
	*button = 0;
	gtk_main_quit ();
}

static void
save (C2WindowMain *wmain)
{
	C2Message *message;
	GtkWidget *mail;
	GtkWidget *dialog;
	gchar *save_path;
	gint button;

	mail = glade_xml_get_widget (C2_WINDOW (wmain)->xml, "mail");
	message = c2_mail_get_message (C2_MAIL (mail));
	gtk_object_ref (GTK_OBJECT (message));

	c2_preferences_get_general_paths_save (save_path);
	dialog = gtk_file_selection_new (NULL);
	gtk_file_selection_set_filename (GTK_FILE_SELECTION (dialog), save_path);
	g_free (save_path);

	c2_application_window_add (C2_WINDOW (wmain)->application, GTK_WINDOW (dialog));

	gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (dialog)->ok_button), "clicked",
						GTK_SIGNAL_FUNC (on_save_ok_clicked), &button);
	gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (dialog)->cancel_button), "clicked",
						GTK_SIGNAL_FUNC (on_save_cancel_clicked), &button);
	gtk_signal_connect (GTK_OBJECT (dialog), "delete_event",
						GTK_SIGNAL_FUNC (on_save_delete_event), &button);
	
	gtk_widget_show (dialog);
	gtk_main ();

	if (!button)
		c2_window_report (C2_WINDOW (wmain), C2_WINDOW_REPORT_WARNING,
							error_list[C2_CANCEL_USER]);
	else
	{
		const gchar *file;
		FILE *fd;

		file = gtk_file_selection_get_filename (GTK_FILE_SELECTION (dialog));

		if (!strlen (file))
			goto no_name;

		if (gnome_config_get_bool_with_default ("/"PACKAGE"/Paths/smart=true", NULL))
		{
			gchar *dir, *buf;

			buf = g_dirname (file);
			dir = g_strdup_printf ("%s" G_DIR_SEPARATOR_S, buf);
			g_free (buf);
			c2_preferences_set_general_paths_save (dir);
		}	

		if (!message)
		{
			c2_window_report (C2_WINDOW (wmain), C2_WINDOW_REPORT_WARNING,
								error_list[C2_FAIL_MESSAGE_SAVE], _("Unable to find message."));
			goto no_name;
		}

		if (!(fd = fopen (file, "w")))
		{
			c2_error_set (-errno);

			c2_window_report (C2_WINDOW (wmain), C2_WINDOW_REPORT_WARNING,
								error_list[C2_FAIL_MESSAGE_SAVE], c2_error_get ());
			goto no_name;
		}

		fprintf (fd, "%s\n\n%s", message->header, message->body);
		fclose (fd);

		c2_window_report (C2_WINDOW (wmain), C2_WINDOW_REPORT_MESSAGE,
								error_list[C2_SUCCESS_MESSAGE_SAVE]);
	}
no_name:
	c2_application_window_remove (C2_WINDOW (wmain)->application, GTK_WINDOW (dialog));
	gtk_widget_destroy (dialog);
	gtk_object_unref (GTK_OBJECT (message));
}

static void
search (C2WindowMain *wmain)
{
}

static void
send_ (C2WindowMain *wmain)
{
	C2Mailbox *outbox;
	C2Db *db;

	outbox = c2_mailbox_get_by_name (C2_WINDOW (wmain)->application->mailbox,
										C2_MAILBOX_OUTBOX);
	if (!C2_IS_MAILBOX (outbox))
		return;

	db = outbox->db;
	if (db)
	{
		GtkWidget *tl;
		
		tl = c2_application_window_get (application, C2_WIDGET_TRANSFER_LIST_TYPE);
		if (!tl || !C2_IS_TRANSFER_LIST (tl))
			tl = c2_transfer_list_new (application);

		gtk_widget_show (tl);
		gdk_window_raise (tl->window);
		
		do
		{
			C2Message *message;
			C2Account *account;
			C2SMTP *smtp;
			C2TransferItem *ti;
			gchar *buf;

			c2_db_load_message (db);
			
			buf = c2_message_get_header_field (db->message, "\nX-CronosII-Account:");
			account = c2_account_get_by_name (application->account, buf);
			g_free (buf);
			if (!account)
				continue;

			smtp = (C2SMTP*) c2_account_get_extra_data (account, C2_ACCOUNT_KEY_OUTGOING, NULL);

			ti = c2_transfer_item_new (application, account, C2_TRANSFER_ITEM_SEND, smtp, db);
			c2_transfer_list_add_item (C2_TRANSFER_LIST (tl), ti);
			c2_transfer_item_start (ti);
		} while (c2_db_lineal_next (db));
	}
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
on_size_allocate (C2WindowMain *wmain, GtkAllocation *alloc)
{
	c2_preferences_set_window_main_width (alloc->width);
	c2_preferences_set_window_main_height (alloc->height);
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

	c2_mailbox_list_set_selected_object (C2_MAILBOX_LIST (wmain->mlist), GTK_OBJECT (mailbox));
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
	C2_WINDOW_MAIN_CLASS_FW (wmain)->check (wmain);
}

static void
on_toolbar_close_clicked (GtkWidget *widget, C2WindowMain *wmain)
{
	C2_WINDOW_MAIN_CLASS_FW (wmain)->close (wmain);
}

static void
on_toolbar_compose_clicked (GtkWidget *widget, C2WindowMain *wmain)
{
	C2_WINDOW_MAIN_CLASS_FW (wmain)->compose (wmain);
}

static void
on_toolbar_contacts_clicked (GtkWidget *widget, C2WindowMain *wmain)
{
	C2_WINDOW_MAIN_CLASS_FW (wmain)->contacts (wmain);
}

static void
on_toolbar_copy_clicked (GtkWidget *widget, C2WindowMain *wmain)
{
	C2_WINDOW_MAIN_CLASS_FW (wmain)->copy (wmain);
}

static void
on_toolbar_delete_clicked (GtkWidget *widget, C2WindowMain *wmain)
{
	C2_WINDOW_MAIN_CLASS_FW (wmain)->delete (wmain);
}

static void
on_toolbar_exit_clicked (GtkWidget *widget, C2WindowMain *wmain)
{
	C2_WINDOW_MAIN_CLASS_FW (wmain)->exit (wmain);
}

static void
on_toolbar_forward_clicked (GtkWidget *widget, C2WindowMain *wmain)
{
	C2_WINDOW_MAIN_CLASS_FW (wmain)->forward (wmain);
}

static void
on_toolbar_move_clicked (GtkWidget *widget, C2WindowMain *wmain)
{
	C2_WINDOW_MAIN_CLASS_FW (wmain)->move (wmain);
}

static void
on_toolbar_next_clicked (GtkWidget *widget, C2WindowMain *wmain)
{
	C2_WINDOW_MAIN_CLASS_FW (wmain)->next (wmain);
}

static void
on_toolbar_previous_clicked (GtkWidget *widget, C2WindowMain *wmain)
{
	C2_WINDOW_MAIN_CLASS_FW (wmain)->previous (wmain);
}

static void
on_toolbar_print_clicked (GtkWidget *widget, C2WindowMain *wmain)
{
	C2_WINDOW_MAIN_CLASS_FW (wmain)->print (wmain);
}

static void
on_toolbar_reply_clicked (GtkWidget *widget, C2WindowMain *wmain)
{
	C2_WINDOW_MAIN_CLASS_FW (wmain)->reply (wmain);
}

static void
on_toolbar_reply_all_clicked (GtkWidget *widget, C2WindowMain *wmain)
{
	C2_WINDOW_MAIN_CLASS_FW (wmain)->reply_all (wmain);
}

static void
on_toolbar_save_clicked (GtkWidget *widget, C2WindowMain *wmain)
{
	C2_WINDOW_MAIN_CLASS_FW (wmain)->save (wmain);
}

static void
on_toolbar_search_clicked (GtkWidget *widget, C2WindowMain *wmain)
{
	C2_WINDOW_MAIN_CLASS_FW (wmain)->search (wmain);
}

static void
on_toolbar_send_clicked (GtkWidget *widget, C2WindowMain *wmain)
{
	C2_WINDOW_MAIN_CLASS_FW (wmain)->send (wmain);
}

static void
on_index_select_message (GtkWidget *index, C2Db *node, C2WindowMain *wmain)
{
	GladeXML *xml;
	GtkWidget *widget;

	if (g_list_length (GTK_CLIST (index)->selection) > 1)
		return;

	if (!C2_IS_MESSAGE (node->message))
	{
		/* [TODO] This should be in a separated thread */
		c2_db_load_message (node);

		if (!C2_IS_MESSAGE (node->message))
		{
			/* Something went wrong */
			const gchar *error;
		
			error = c2_error_object_get (GTK_OBJECT (node));
			if (error)
				c2_window_report (C2_WINDOW (wmain), C2_WINDOW_REPORT_WARNING,
								error_list[C2_FAIL_MESSAGE_LOAD], error);
			else
				c2_window_report (C2_WINDOW (wmain), C2_WINDOW_REPORT_WARNING,
								error_list[C2_FAIL_MESSAGE_LOAD], error_list[C2_UNKNOWN_REASON]);

			return;
		}
	}

	
	
	c2_mail_set_message (C2_MAIL (glade_xml_get_widget (C2_WINDOW (wmain)->xml, "mail")), node->message);

	/* Set some widgets sensivity */
}

static gint
on_mlist_object_selected_pthread (C2WindowMain *wmain)
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
							error_list[C2_FAIL_MAILBOX_LOAD], mailbox->name,
							c2_error_object_get (GTK_OBJECT (mailbox)));
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
on_mlist_object_selected (C2MailboxList *mlist, GtkObject *object, C2WindowMain *wmain)
{
	pthread_t thread;

	if (!C2_IS_MAILBOX (object))
	{
		/* If this is not a mailbox we don't want to show anything */
		c2_index_remove_mailbox (C2_INDEX (wmain->index));
		return;
	}

	pthread_create (&thread, NULL, C2_PTHREAD_FUNC (on_mlist_object_selected_pthread), wmain);
}

static void
on_mlist_object_unselected (C2MailboxList *mlist, C2WindowMain *wmain)
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
			c2_mailbox_list_set_selected_object (C2_MAILBOX_LIST (wmain->mlist), GTK_OBJECT (mailbox));
			gtk_ctree_select (GTK_CTREE (widget), node);
		} else
			c2_mailbox_list_set_selected_object (C2_MAILBOX_LIST (wmain->mlist), NULL);

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
on_help_release_information_activate (GtkWidget *widget, C2WindowMain *wmain)
{
	c2_application_dialog_release_information (C2_WINDOW (wmain)->application);
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
	gchar *get_path;

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

	c2_preferences_get_general_paths_get (get_path);
	gnome_file_entry_set_default_path (GNOME_FILE_ENTRY (glade_xml_get_widget (xml, "epath_spool")),
									get_path);
	g_free (get_path);

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
										error_list[C2_FAIL_MAILBOX_CREATE], name);
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

				if (c2_preferences_get_general_options_start_load ())
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





#if 1 /* Delete Mails Confirmation Dialog */
static void
dlg_confirm_delete_message_confirmation_btn_toggled (GtkWidget *widget)
{
	c2_preferences_set_general_options_delete_confirmation (
						!GTK_TOGGLE_BUTTON (widget)->active);
	c2_preferences_commit ();
}

static gboolean
dlg_confirm_delete_message (C2WindowMain *wmain)
{
	C2Application *application;
	GtkWidget *dialog;
	GtkWidget *pixmap;
	GtkWidget *toggle;
	GladeXML *xml;
	gboolean retval;
	
	c2_return_val_if_fail (C2_IS_WINDOW_MAIN (wmain), 0, C2EDATA);

	application = C2_WINDOW (wmain)->application;

	xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("cronosII"), "dlg_confirm_delete_message");

	dialog = glade_xml_get_widget (xml, "dlg_confirm_delete_message");
	c2_application_window_add (application, GTK_WINDOW (dialog));

	pixmap = glade_xml_get_widget (xml, "pixmap");
	gnome_pixmap_load_file (GNOME_PIXMAP (pixmap), gnome_pixmap_file ("gnome-question.png"));

	toggle = glade_xml_get_widget (xml, "confirmation_btn");
	gtk_signal_connect (GTK_OBJECT (toggle), "toggled",
						GTK_SIGNAL_FUNC (dlg_confirm_delete_message_confirmation_btn_toggled), NULL);

	gtk_window_set_position (GTK_WINDOW (dialog),
					gnome_preferences_get_dialog_position ());
	gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (wmain));
	gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
	gnome_dialog_close_hides (GNOME_DIALOG (dialog), TRUE);
	if (gnome_dialog_run (GNOME_DIALOG (dialog)) == 0)
		retval = TRUE;
	else
		retval = FALSE;

	c2_application_window_remove (application, GTK_WINDOW (dialog));
	gtk_widget_destroy (dialog);
	gtk_object_destroy (GTK_OBJECT (xml));

	return retval;
}
#endif /* Delete Mails Confirmation Dialog */






#if 1 /* Expunge Mails Confirmation Dialog */
static gboolean
dlg_confirm_expunge_message (C2WindowMain *wmain)
{
	C2Application *application;
	GtkWidget *dialog;
	GtkWidget *pixmap;
	GtkWidget *toggle;
	GladeXML *xml;
	gboolean retval;
	
	c2_return_val_if_fail (C2_IS_WINDOW_MAIN (wmain), 0, C2EDATA);

	application = C2_WINDOW (wmain)->application;

	xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("cronosII"), "dlg_confirm_expunge_message");

	dialog = glade_xml_get_widget (xml, "dlg_confirm_expunge_message");
	c2_application_window_add (application, GTK_WINDOW (dialog));

	gtk_window_set_position (GTK_WINDOW (dialog),
					gnome_preferences_get_dialog_position ());
	gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (wmain));
	gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
	gnome_dialog_close_hides (GNOME_DIALOG (dialog), TRUE);
	if (gnome_dialog_run (GNOME_DIALOG (dialog)) == 0)
		retval = TRUE;
	else
		retval = FALSE;

	c2_application_window_remove (application, GTK_WINDOW (dialog));
	gtk_widget_destroy (dialog);
	gtk_object_destroy (GTK_OBJECT (xml));

	return retval;
}
#endif /* Expunge Mails Confirmation Dialog */






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
