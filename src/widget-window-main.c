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
 **/
#include <config.h>
#include <gnome.h>
#include <sys/stat.h>

#include <glade/glade.h>

#include <libcronosII/error.h>
#include <libcronosII/mime.h>
#include <libcronosII/utils.h>

#include "main.h"
#include "preferences.h"
#include "widget-application-utils.h"
#include "widget-composer.h"
#include "widget-network-traffic.h"
#include "widget-dialog-preferences.h"
#include "widget-mailbox-list.h"
#include "widget-mail.h"
#include "widget-network-traffic.h"
#include "widget-HTML.h"
#include "widget-toolbar.h"
#include "widget-transfer-list.h"
#include "widget-index.h"
#include "widget-window-main.h"

static void
class_init									(C2WindowMainClass *klass);

static void
init										(C2WindowMain *wmain);

static void
destroy										(GtkObject *obj);

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
on_application_window_changed				(C2Application *application, GSList *list, C2WindowMain *wmain);

static void
on_application_application_preferences_changed	(C2Application *application, gint key, gpointer value,
													C2WindowMain *wmain);

static void
on_outbox_mailbox_changed_mailbox			(C2Mailbox *mailbox, C2MailboxChangeType type, C2Db *db_node,
												C2WindowMain *wmain);

static void
on_docktoolbar_button_press_event			(GtkWidget *widget, GdkEventButton *event, C2WindowMain *wmain);

static void
on_menubar_file_check_mail_all_accounts_activate	(GtkWidget *widget, C2WindowMain *wmain);

static void
on_menubar_file_check_mail_account_activate	(GtkWidget *widget, C2WindowMain *wmain);

static void
on_menubar_file_new_message_activate		(GtkWidget *widget, C2WindowMain *wmain);

static void
on_menubar_file_new_mailbox_activate		(GtkWidget *widget, C2WindowMain *wmain);

static void
on_menubar_file_new_window_activate			(GtkWidget *widget, C2WindowMain *wmain);

static void
on_menubar_file_send_unsent_mails_activate	(GtkWidget *widget, C2WindowMain *wmain);

static void
on_menubar_file_save_activate				(GtkWidget *widget, C2WindowMain *wmain);

static void
on_menubar_file_print_activate				(GtkWidget *widget, C2WindowMain *wmain);

static void
on_menubar_file_exit_activate				(GtkWidget *widget, C2WindowMain *wmain);

static void
on_menubar_view_toolbar_activate			(GtkWidget *widget, C2WindowMain *wmain);

static void
on_menubar_view_mail_preview_activate		(GtkWidget *widget, C2WindowMain *wmain);

static void
on_menubar_view_headers_activate			(GtkWidget *widget, C2WindowMain *wmain);

static void
on_menubar_view_network_traffic_activate	(GtkWidget *widget, C2WindowMain *wmain);

static void
on_menubar_view_mail_source_activate		(GtkWidget *widget, C2WindowMain *wmain);

static void
on_menubar_view_dialog_network_traffic_activate	(GtkWidget *widget, C2WindowMain *wmain);

static void
on_menubar_view_dialog_send_receive_activate	(GtkWidget *widget, C2WindowMain *wmain);

static void
on_menubar_mailbox_speed_up_activate		(GtkWidget *widget, C2WindowMain *wmain);

static void
on_menubar_message_reply_activate			(GtkWidget *widget, C2WindowMain *wmain);

static void
on_menubar_message_reply_all_activate		(GtkWidget *widget, C2WindowMain *wmain);

static void
on_menubar_message_forward_activate			(GtkWidget *widget, C2WindowMain *wmain);

static void
on_menubar_message_copy_activate			(GtkWidget *widget, C2WindowMain *wmain);

static void
on_menubar_message_move_activate			(GtkWidget *widget, C2WindowMain *wmain);

static void
on_menubar_message_delete_activate			(GtkWidget *widget, C2WindowMain *wmain);

static void
on_menubar_message_mark_important_activate	(GtkWidget *widget, C2WindowMain *wmain);

static void
on_menubar_message_mark_unreaded_activate	(GtkWidget *widget, C2WindowMain *wmain);

static void
on_menubar_message_mark_readed_activate		(GtkWidget *widget, C2WindowMain *wmain);

static void
on_menubar_message_mark_replied_activate	(GtkWidget *widget, C2WindowMain *wmain);

static void
on_menubar_message_mark_forwarded_activate	(GtkWidget *widget, C2WindowMain *wmain);

static void
on_menubar_message_next_activate			(GtkWidget *widget, C2WindowMain *wmain);

static void
on_menubar_message_previous_activate		(GtkWidget *widget, C2WindowMain *wmain);

static void
on_menubar_settings_features_download_activate	(GtkWidget *widget, C2WindowMain *wmain);

static void
on_menubar_settings_preferences_activate	(GtkWidget *widget, C2WindowMain *wmain);

static void
on_menubar_help_getting_in_touch_activate	(GtkWidget *widget, C2WindowMain *wmain);

static void
on_menubar_help_release_information_activate	(GtkWidget *widget, C2WindowMain *wmain);

static void
on_menubar_help_about_activate				(GtkWidget *widget, C2WindowMain *wmain);

static void
on_menubar_help_donations_activate			(GtkWidget *widget, C2WindowMain *wmain);

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
on_index_open_message						(GtkWidget *index, C2Db *node, C2WindowMain *wmain);

static void
on_index_select_message						(GtkWidget *index, C2Db *node, C2WindowMain *wmain);

static void
on_index_unselect_message					(C2Index *index, C2Db *node, C2WindowMain *wmain);

static void
on_mlist_object_selected					(C2MailboxList *mlist, GtkObject *object, C2WindowMain *wmain);

static void
on_mlist_object_unselected					(C2MailboxList *mlist, C2WindowMain *wmain);

static void
on_eastern_egg_separator_activate			(GtkWidget *widget, C2WindowMain *wmain);

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

static gboolean
dlg_confirm_expunge_message					(C2WindowMain *wmain);

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
		"toolbar_check",
		N_("Check"), PKGDATADIR "/pixmaps/receive.png",
		N_("Check for incoming mails"), TRUE,
		GTK_SIGNAL_FUNC (on_toolbar_check_clicked), NULL,
		NULL, NULL
	},
	{
		C2_TOOLBAR_BUTTON,
		"toolbar_send",
		N_("Send"), PKGDATADIR "/pixmaps/send.png",
		N_("Send outgoing mails"), TRUE,
		GTK_SIGNAL_FUNC (on_toolbar_send_clicked), NULL,
		NULL, NULL
	},
	{
		C2_TOOLBAR_BUTTON,
		"toolbar_search",
		N_("Search"), PKGDATADIR "/pixmaps/find.png",
		N_("Search a message in existent mailboxes"), TRUE,
		GTK_SIGNAL_FUNC (on_toolbar_search_clicked), NULL,
		NULL, NULL
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
		C2_TOOLBAR_BUTTON,
		"toolbar_delete",
		N_("Delete"), PKGDATADIR "/pixmaps/delete.png",
		N_("Delete selected mails"), TRUE,
		GTK_SIGNAL_FUNC (on_toolbar_delete_clicked), NULL,
		NULL, NULL
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
		C2_TOOLBAR_BUTTON,
		"toolbar_compose",
		N_("Compose"), PKGDATADIR "/pixmaps/mail-write.png",
		N_("Compose a new message"), TRUE,
		GTK_SIGNAL_FUNC (on_toolbar_compose_clicked), NULL,
		NULL, NULL
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
		C2_TOOLBAR_BUTTON,
		"toolbar_previous",
		N_("Previous"), PKGDATADIR "/pixmaps/prev.png",
		N_("Select previous message"), TRUE,
		GTK_SIGNAL_FUNC (on_toolbar_previous_clicked), NULL,
		NULL, NULL
	},
	{
		C2_TOOLBAR_BUTTON,
		"toolbar_next",
		N_("Next"), PKGDATADIR "/pixmaps/next.png",
		N_("Select next message"), TRUE,
		GTK_SIGNAL_FUNC (on_toolbar_next_clicked), NULL,
		NULL, NULL
	},
	{
		C2_TOOLBAR_BUTTON,
		"toolbar_contacts",
		N_("Contacts"), PKGDATADIR "/pixmaps/contacts.png",
		N_("Open the Contacts Address Book"), TRUE,
		GTK_SIGNAL_FUNC (on_toolbar_contacts_clicked), NULL,
		NULL, NULL
	},
	{
		C2_TOOLBAR_BUTTON,
		"toolbar_close",
		N_("Close"), PKGDATADIR "/pixmaps/close.png",
		
		N_("Close the main window of Cronos II"), TRUE,
		GTK_SIGNAL_FUNC (on_toolbar_close_clicked), NULL,
		NULL, NULL
	},
	{
		C2_TOOLBAR_BUTTON,
		"toolbar_exit",
		N_("Exit"), PKGDATADIR "/pixmaps/exit.png",
		N_("Close all windows of Cronos II"), TRUE,
		GTK_SIGNAL_FUNC (on_toolbar_exit_clicked), NULL,
		NULL, NULL
	},
	{
		C2_TOOLBAR_SPACE, NULL, NULL, NULL, NULL, FALSE, NULL, NULL, NULL
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
	c2_mutex_init (&wmain->index_lock);
	c2_mutex_init (&wmain->body_lock);
}

static void
destroy (GtkObject *obj)
{
	C2_WINDOW_MAIN_CLASS_FW (obj)->close (C2_WINDOW_MAIN (obj));
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
	GtkWidget *window = NULL;
	GtkWidget *widget;
	GtkWidget *menuitem;
	GtkWidget *hpaned;
	GtkWidget *vpaned;
	GtkWidget *index_scroll;
	GtkWidget *button;
	GtkWidget *pixmap;
	GtkWidget *appbar;
	GtkWidget *scroll;
	GtkStyle *style;
	gint toolbar_style;
	gint i;
	GList *l;

	c2_window_construct (C2_WINDOW (wmain), application, "Cronos II", "wmain", NULL);
	gtk_signal_connect (GTK_OBJECT (application), "window_changed",
						GTK_SIGNAL_FUNC (on_application_window_changed), wmain);
	gtk_signal_connect (GTK_OBJECT (wmain), "destroy",
						GTK_SIGNAL_FUNC (destroy), wmain);

	C2_WINDOW (wmain)->xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("cronosII"), "wnd_main_contents");
	c2_window_set_contents_from_glade (C2_WINDOW (wmain), "wnd_main_contents");

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

	/* Menubar */
	widget = glade_xml_get_widget (xml, "file_check_mail_all_accounts");
	for (l = gtk_container_children (GTK_CONTAINER (widget)); l; l = g_list_next (l))
	{
		if (GTK_IS_LABEL (l->data))
		{
			widget = (GtkWidget*) l->data;
			style = gtk_style_copy (gtk_widget_get_style (widget));
			style->font = gdk_font_load (c2_font_bold);
			gtk_widget_set_style (widget, style);
			break;
		}
	}

	/* Toolbar */
	widget = glade_xml_get_widget (xml, "toolbar_container");
	menuitem = glade_xml_get_widget (xml, "view_toolbar");
	toolbar_style = gnome_config_get_int_with_default ("/"PACKAGE"/Toolbar::Window Main/type=0", NULL);
	wmain->toolbar = c2_toolbar_new (toolbar_style);
	gtk_box_pack_start (GTK_BOX (widget), wmain->toolbar, TRUE, TRUE, 0);
	gtk_widget_show (wmain->toolbar);
	widget = glade_xml_get_widget (xml, "dockitem_toolbar");
	if (c2_preferences_get_window_main_toolbar_visible ())
	{
		gtk_widget_show (widget);

		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menuitem), TRUE);
	} else
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menuitem), FALSE);
	
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
										toolbar_items[val].name,
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
				c2_toolbar_append_widget (C2_TOOLBAR (wmain->toolbar), toolbar_items[val].name,
										toolbar_items[val].widget, toolbar_items[val].widget_tooltip);
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

	/* Filters Toolbar */
	widget = glade_xml_get_widget (xml, "filter_pixmap");
	gnome_pixmap_load_file (GNOME_PIXMAP (widget), PKGDATADIR "/pixmaps/filter.png");

	/* Vpaned */
	vpaned = glade_xml_get_widget (xml, "vpaned");
	menuitem = glade_xml_get_widget (xml, "view_mail_preview");
	if (c2_preferences_get_window_main_mail_preview_visible ())
	{
		gtk_paned_set_position (GTK_PANED (vpaned), 
							c2_preferences_get_window_main_vpaned ());
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menuitem), TRUE);
	} else
	{
		gtk_paned_set_position (GTK_PANED (vpaned), gdk_screen_height ());
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menuitem), FALSE);
	}

	appbar = glade_xml_get_widget (xml, "appbar");

	/* Index */
	index_scroll = glade_xml_get_widget (xml, "index_scroll");
	wmain->index = c2_index_new (application, C2_INDEX_READ_WRITE);
	gtk_container_add (GTK_CONTAINER (index_scroll), wmain->index);
	gtk_widget_show (wmain->index);
	gtk_signal_connect (GTK_OBJECT (wmain->index), "open_message",
							GTK_SIGNAL_FUNC (on_index_open_message), wmain);
	gtk_signal_connect (GTK_OBJECT (wmain->index), "select_message",
							GTK_SIGNAL_FUNC (on_index_select_message), wmain);
	gtk_signal_connect (GTK_OBJECT (wmain->index), "unselect_message",
							GTK_SIGNAL_FUNC (on_index_unselect_message), wmain);
	
	/* Mail VBox */
	wmain->mail_vbox = gtk_vbox_new (FALSE, 4);
	widget = glade_xml_get_widget (xml, "vpaned");
	gtk_paned_add2 (GTK_PANED (widget), wmain->mail_vbox);
	gtk_widget_show (wmain->mail_vbox);

	/* Mail */
	wmain->mail = c2_mail_new (application);
	gtk_box_pack_start (GTK_BOX (wmain->mail_vbox), wmain->mail, TRUE, TRUE, 0);
	if (c2_preferences_get_window_main_mail_preview_visible ())
		gtk_widget_show (wmain->mail);
	c2_mail_install_hints (C2_MAIL (wmain->mail), appbar, &C2_WINDOW (wmain)->status_lock);
	menuitem = glade_xml_get_widget (xml, "view_headers");
	if (c2_preferences_get_widget_mail_headers_visible ())
	{
		c2_mail_set_headers_visible (C2_MAIL (wmain->mail), TRUE);
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menuitem), TRUE);
	} else
	{
		c2_mail_set_headers_visible (C2_MAIL (wmain->mail), FALSE);
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menuitem), FALSE);
	}

	/* Network Traffic */
	menuitem = glade_xml_get_widget (xml, "view_network_traffic");
	if (c2_preferences_get_window_main_network_traffic_visible ())
	{
		wmain->nt = c2_network_traffic_new (application);
		gtk_box_pack_end (GTK_BOX (wmain->mail_vbox), wmain->nt, FALSE, TRUE, 0);
		gtk_widget_set_usize (wmain->nt, -1, 100);
		gtk_widget_show (wmain->nt);
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menuitem), TRUE);
	} else
	{
		wmain->nt = NULL;
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menuitem), FALSE);
	}

	/* Button */
	button = glade_xml_get_widget (xml, "appbar_button");
	pixmap = gnome_stock_pixmap_widget_at_size (window, GNOME_STOCK_PIXMAP_UP, 10, 14);
	gtk_container_add (GTK_CONTAINER (button), pixmap);
	gtk_widget_show (pixmap);

	/* Connect all signals: menues, toolbar, buttons, etc. */
	gtk_signal_connect (GTK_OBJECT (application), "application_preferences_changed",
							GTK_SIGNAL_FUNC (on_application_application_preferences_changed), wmain);
	gtk_signal_connect (GTK_OBJECT (c2_mailbox_get_by_usage (application->mailbox, C2_MAILBOX_USE_AS_OUTBOX)),
													"changed_mailbox",
							GTK_SIGNAL_FUNC (on_outbox_mailbox_changed_mailbox), wmain);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "dockitem_toolbar")), "button_press_event",
							GTK_SIGNAL_FUNC (on_docktoolbar_button_press_event), wmain);
	
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "file_check_mail_all_accounts")), "activate",
							GTK_SIGNAL_FUNC (on_menubar_file_check_mail_all_accounts_activate), wmain);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "file_new_message")), "activate",
							GTK_SIGNAL_FUNC (on_menubar_file_new_message_activate), wmain);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "file_new_mailbox")), "activate",
							GTK_SIGNAL_FUNC (on_menubar_file_new_mailbox_activate), wmain);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "file_new_window")), "activate",
							GTK_SIGNAL_FUNC (on_menubar_file_new_window_activate), wmain);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "file_send_unsent_mails")), "activate",
							GTK_SIGNAL_FUNC (on_menubar_file_send_unsent_mails_activate), wmain);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "file_save")), "activate",
							GTK_SIGNAL_FUNC (on_menubar_file_save_activate), wmain);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "file_print")), "activate",
							GTK_SIGNAL_FUNC (on_menubar_file_print_activate), wmain);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "file_exit")), "activate",
							GTK_SIGNAL_FUNC (on_menubar_file_exit_activate), wmain);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "file_egg_separator")), "activate",
							GTK_SIGNAL_FUNC (on_eastern_egg_separator_activate), wmain);

	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "view_toolbar")), "activate",
							GTK_SIGNAL_FUNC (on_menubar_view_toolbar_activate), wmain);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "view_mail_preview")), "activate",
							GTK_SIGNAL_FUNC (on_menubar_view_mail_preview_activate), wmain);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "view_headers")), "activate",
							GTK_SIGNAL_FUNC (on_menubar_view_headers_activate), wmain);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "view_network_traffic")), "activate",
							GTK_SIGNAL_FUNC (on_menubar_view_network_traffic_activate), wmain);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "view_mail_source")), "activate",
							GTK_SIGNAL_FUNC (on_menubar_view_mail_source_activate), wmain);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "view_dialog_network_traffic")), "activate",
							GTK_SIGNAL_FUNC (on_menubar_view_dialog_network_traffic_activate), wmain);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "view_dialog_send_receive")), "activate",
							GTK_SIGNAL_FUNC (on_menubar_view_dialog_send_receive_activate), wmain);

	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "mailbox_speed_up")), "activate",
							GTK_SIGNAL_FUNC (on_menubar_mailbox_speed_up_activate), wmain);

	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "message_reply")), "activate",
							GTK_SIGNAL_FUNC (on_menubar_message_reply_activate), wmain);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "message_reply_all")), "activate",
							GTK_SIGNAL_FUNC (on_menubar_message_reply_all_activate), wmain);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "message_forward")), "activate",
							GTK_SIGNAL_FUNC (on_menubar_message_forward_activate), wmain);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "message_copy")), "activate",
							GTK_SIGNAL_FUNC (on_menubar_message_copy_activate), wmain);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "message_move")), "activate",
							GTK_SIGNAL_FUNC (on_menubar_message_move_activate), wmain);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "message_delete")), "activate",
							GTK_SIGNAL_FUNC (on_menubar_message_delete_activate), wmain);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "message_mark_important")), "activate",
							GTK_SIGNAL_FUNC (on_menubar_message_mark_important_activate), wmain);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "message_mark_unreaded")), "activate",
							GTK_SIGNAL_FUNC (on_menubar_message_mark_unreaded_activate), wmain);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "message_mark_readed")), "activate",
							GTK_SIGNAL_FUNC (on_menubar_message_mark_readed_activate), wmain);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "message_mark_replied")), "activate",
							GTK_SIGNAL_FUNC (on_menubar_message_mark_replied_activate), wmain);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "message_mark_forwarded")), "activate",
							GTK_SIGNAL_FUNC (on_menubar_message_mark_forwarded_activate), wmain);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "message_next")), "activate",
							GTK_SIGNAL_FUNC (on_menubar_message_next_activate), wmain);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "message_previous")), "activate",
							GTK_SIGNAL_FUNC (on_menubar_message_previous_activate), wmain);
	
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "settings_preferences")), "activate",
							GTK_SIGNAL_FUNC (on_menubar_settings_preferences_activate), wmain);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "settings_features_download")), "activate",
							GTK_SIGNAL_FUNC (on_menubar_settings_features_download_activate), wmain);
	
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "help_getting_in_touch")), "activate",
							GTK_SIGNAL_FUNC (on_menubar_help_getting_in_touch_activate), wmain);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "help_release_information")), "activate",
							GTK_SIGNAL_FUNC (on_menubar_help_release_information_activate), wmain);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "help_about")), "activate",
							GTK_SIGNAL_FUNC (on_menubar_help_about_activate), wmain);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "help_donations")), "activate",
							GTK_SIGNAL_FUNC (on_menubar_help_donations_activate), wmain);	

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
	

	gtk_signal_emit_by_name (GTK_OBJECT (application), "window_changed",
							c2_application_open_windows (application));

	/* Build the menu Accounts */
	on_application_application_preferences_changed (application, C2_DIALOG_PREFERENCES_KEY_GENERAL_ACCOUNTS,
													application->account, wmain);

	/* Set the sensitivity of the Send button */
	on_outbox_mailbox_changed_mailbox (c2_mailbox_get_by_usage (
									application->mailbox, C2_MAILBOX_USE_AS_OUTBOX), C2_MAILBOX_CHANGE_ANY,
									NULL, wmain);
}

static void
check (C2WindowMain *wmain)
{
	C2Application *application;
	
	application = C2_WINDOW (wmain)->application;
	C2_APPLICATION_CLASS_FW (application)->check (application);
}

static void
close_ (C2WindowMain *wmain)
{
	C2Application *application = C2_WINDOW (wmain)->application;
	
	/* Disconnect to signals connected to object that might not be destroyed */
	gtk_signal_disconnect_by_func (GTK_OBJECT (application),
									GTK_SIGNAL_FUNC (on_application_window_changed), wmain);
	gtk_signal_disconnect_by_func (GTK_OBJECT (application),
									GTK_SIGNAL_FUNC (on_application_application_preferences_changed), wmain);

	/* Destroy objects */
	gtk_object_destroy (GTK_OBJECT (wmain->toolbar));
	gtk_object_destroy (GTK_OBJECT (wmain->toolbar_menu));
	gtk_object_destroy (GTK_OBJECT (wmain->mlist));
	gtk_object_destroy (GTK_OBJECT (wmain));
}

static void
compose (C2WindowMain *wmain)
{
	GtkWidget *composer;
	
	if ((composer = c2_composer_new (C2_WINDOW (wmain)->application)))
		gtk_widget_show (composer);
}

static void
contacts (C2WindowMain *wmain)
{
}

static void
copy (C2WindowMain *wmain)
{
	C2Application *application = C2_WINDOW (wmain)->application;
	GList *list;

	list = c2_index_selection (C2_INDEX (wmain->index));
	C2_APPLICATION_CLASS_FW (application)->copy (application, list, C2_WINDOW (wmain));
	g_list_free (list);
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
L	
	g_free (data);

	/* Get the length of our list */
	length = g_list_length (list);

	widget = glade_xml_get_widget (C2_WINDOW (wmain)->xml, "appbar");

	/* Try to reserve ownership over the progress bar */
	if (!c2_mutex_trylock (&C2_WINDOW (wmain)->progress_lock))
		progress_ownership = TRUE;
	else
		progress_ownership = FALSE;

	/* Try to reserve ownership over the status bar */
	if (!c2_mutex_trylock (&C2_WINDOW (wmain)->status_lock))
		status_ownership = TRUE;
	else
		status_ownership = FALSE;

	gdk_threads_enter ();
	
	if (progress_ownership)
	{
		/* Configure the progress bar */
		progress = GTK_PROGRESS (GNOME_APPBAR (widget)->progress);
		gtk_progress_configure (progress, 0, 0, length);
	} else
		progress = NULL;

	if (status_ownership)
	{
		/* Configure the status bar */
		gnome_appbar_push (GNOME_APPBAR (widget), _("Deleting..."));
	}

	gdk_threads_leave ();
	
	c2_db_freeze (fmailbox);
	c2_db_freeze (tmailbox);
L	for (l = list, off = 0; l; l = g_list_next (l))
	{
		C2Db *db;
		
		/* Now do the actual copy */
L		db = (C2Db*) l->data;

		if (!db->message)
		{
			if (!c2_db_load_message (db))
			{
				c2_window_report (C2_WINDOW (wmain), C2_WINDOW_REPORT_WARNING,
									error_list[C2_FAIL_MESSAGE_LOAD], c2_error_get ());
				continue;
			}
		}
		
		gtk_object_ref (GTK_OBJECT (db->message));
		if (c2_db_message_add (tmailbox, db->message))
		{
			c2_db_message_remove (fmailbox, GPOINTER_TO_INT (l->data)-(off++));
		}
		gtk_object_unref (GTK_OBJECT (db->message));

		if (progress_ownership)
		{
			gdk_threads_enter ();
			gtk_progress_set_value (progress, off);
			gdk_threads_leave ();
		}
L	}
	c2_db_thaw (tmailbox);
	c2_db_thaw (fmailbox);
L
	g_list_free (list);
L
	gdk_threads_enter ();

	if (status_ownership)
	{
		gnome_appbar_pop (GNOME_APPBAR (widget));
		c2_mutex_unlock (&C2_WINDOW (wmain)->status_lock);
	}

	if (progress_ownership)
	{
		gtk_progress_set_percentage (progress, 1.0);
		c2_mutex_unlock (&C2_WINDOW (wmain)->progress_lock);
	}

	gdk_threads_leave ();
L}

static void
delete (C2WindowMain *wmain)
{
	C2Application *application = C2_WINDOW (wmain)->application;
	GList *list;
L
L	list = c2_index_selection (C2_INDEX (wmain->index));
L	C2_APPLICATION_CLASS_FW (application)->delete (application, list, C2_WINDOW (wmain));
L	g_list_free (list);
L}

static void
exit_ (C2WindowMain *wmain)
{
	C2Application *application = C2_WINDOW (wmain)->application;
	
/* TODO	c2_application_finish (C2_WINDOW (wmain)->application); */
	
	C2_WINDOW_MAIN_CLASS_FW (wmain)->close (wmain);

	if (C2_IS_APPLICATION (application))
		gtk_object_destroy (GTK_OBJECT (application));
}

static void
expunge_thread (C2WindowMain *wmain)
{
/*	C2Mailbox *mailbox = c2_mailbox_list_get_selected_mailbox (C2_MAILBOX_LIST (wmain->mlist));
	
	c2_db_message_remove (mailbox, GTK_CLIST (wmain->index)->selection);*/
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
	C2Application *application = C2_WINDOW (wmain)->application;
	GList *list, *l;
	C2Db *db;

	list = c2_index_selection (C2_INDEX (wmain->index));
	
	for (l = list; l; l = g_list_next (l))
	{
		db = C2_DB (l->data);

		gdk_threads_leave ();
		if (!c2_db_load_message (db))
		{
			gdk_threads_enter ();
			continue;
		}
		gdk_threads_enter ();
		
		gtk_object_ref (GTK_OBJECT (db));
		C2_APPLICATION_CLASS_FW (application)->forward (application, db, db->message);
		gtk_object_unref (GTK_OBJECT (db));
	}
	
	g_list_free (list);
}

static void
move (C2WindowMain *wmain)
{
	C2Application *application = C2_WINDOW (wmain)->application;
	GList *list;

	list = c2_index_selection (C2_INDEX (wmain->index));
	C2_APPLICATION_CLASS_FW (application)->move (application, list, C2_WINDOW (wmain));
	g_list_free (list);
}

static void
next (C2WindowMain *wmain)
{
	c2_index_select_next_message (C2_INDEX (wmain->index));
}

static void
previous (C2WindowMain *wmain)
{
	c2_index_select_previous_message (C2_INDEX (wmain->index));
}

static void
print (C2WindowMain *wmain)
{
}

static void
reply (C2WindowMain *wmain)
{
	C2Application *application = C2_WINDOW (wmain)->application;
	GList *list, *l;
	C2Db *db;

	list = c2_index_selection (C2_INDEX (wmain->index));
	
	for (l = list; l; l = g_list_next (l))
	{
		db = C2_DB (l->data);

		gdk_threads_leave ();
		if (!c2_db_load_message (db))
		{
			gdk_threads_enter ();
			continue;
		}
		gdk_threads_enter ();
		
		gtk_object_ref (GTK_OBJECT (db));
		C2_APPLICATION_CLASS_FW (application)->reply (application, db, db->message);
		gtk_object_unref (GTK_OBJECT (db));
	}
	
	g_list_free (list);
}

static void
reply_all (C2WindowMain *wmain)
{
	C2Application *application = C2_WINDOW (wmain)->application;
	GList *list, *l;
	C2Db *db;

	list = c2_index_selection (C2_INDEX (wmain->index));
	
	for (l = list; l; l = g_list_next (l))
	{
		db = C2_DB (l->data);

		gdk_threads_leave ();
		if (!c2_db_load_message (db))
		{
			gdk_threads_enter ();
			continue;
		}
		gdk_threads_enter ();
		
		gtk_object_ref (GTK_OBJECT (db));
		C2_APPLICATION_CLASS_FW (application)->reply_all (application, db, db->message);
		gtk_object_unref (GTK_OBJECT (db));
	}
	
	g_list_free (list);
}

static void
save (C2WindowMain *wmain)
{
	C2Application *application = C2_WINDOW (wmain)->application;
	GList *list, *l;
	C2Db *db;

	list = c2_index_selection (C2_INDEX (wmain->index));
	
	for (l = list; l; l = g_list_next (l))
	{
		db = C2_DB (l->data);

		gdk_threads_leave ();
		if (!c2_db_load_message (db))
		{
			gdk_threads_enter ();
			continue;
		}
		gdk_threads_enter ();
		
		gtk_object_ref (GTK_OBJECT (db));
		C2_APPLICATION_CLASS_FW (application)->save (application, db, db->message);
		gtk_object_unref (GTK_OBJECT (db));
	}
	
	g_list_free (list);
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
			C2Account *account;
			C2SMTP *smtp;
			C2TransferItem *ti;
			gchar *buf;

			c2_db_load_message (db);
			
			buf = c2_message_get_header_field (db->message, "X-CronosII-Account:");
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

static void
on_window_item_activate (GtkWidget *widget, GtkWidget *window)
{
	gtk_widget_show (window);
	gdk_window_raise (window->window);
}

static void
on_application_window_changed (C2Application *application, GSList *list, C2WindowMain *wmain)
{
	GtkWidget *widget, *menu, *item;
	GSList *l;
	const gchar *title;

	widget = glade_xml_get_widget (C2_WINDOW (wmain)->xml, "windows");

	menu = gtk_menu_new ();
	
	for (l = list; l; l = g_slist_next (l))
	{
		GtkWidget *hbox;
		GtkWidget *label;
		GtkWidget *image;
		const gchar *file = PKGDATADIR "/pixmaps/window.png";
		gchar *tlabel;

		hbox = gtk_hbox_new (FALSE, 4);
		gtk_widget_show (hbox);

		if (C2_IS_WINDOW (l->data) || C2_IS_DIALOG (l->data))
		{
			gchar *_file;

			_file = (gchar*) gtk_object_get_data (GTK_OBJECT (l->data), "icon");
			if (_file)
				file = _file;
		}
		
		image = gnome_pixmap_new_from_file (file);
		gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);
		gtk_widget_show (image);

		title = ((GtkWindow*)l->data)->title;
		if (title && strlen (title) > 40)
		{
			gchar *buf;

			buf = g_strndup (((GtkWindow*)l->data)->title, 36);
			tlabel = g_strdup_printf ("%s...", buf);
			g_free (buf);
		} else
			tlabel = g_strdup (((GtkWindow*)l->data)->title);
		
		label = gtk_label_new (tlabel);
		gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);
		gtk_widget_show (label);
		gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
		g_free (tlabel);
		
		item = gtk_menu_item_new ();
		gtk_container_add (GTK_CONTAINER (item), hbox);
		gtk_menu_append (GTK_MENU (menu), item);
		gtk_signal_connect (GTK_OBJECT (item), "activate",
							GTK_SIGNAL_FUNC (on_window_item_activate), l->data);
		gtk_widget_show (item);
	}

	gtk_menu_item_remove_submenu (GTK_MENU_ITEM (widget));
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (widget), menu);
}

static void
on_application_application_preferences_changed (C2Application *application, gint key, gpointer value,
												C2WindowMain *wmain)
{
	GtkWidget *widget;
	
	switch (key)
	{
		case C2_DIALOG_PREFERENCES_KEY_GENERAL_ACCOUNTS:
		{
			GtkWidget *sep;
			GtkWidget *submenu;
			GtkWidget *item, *label, *pixmap, *hbox;
			GList *children, *l;
			C2Account *account;
			
			widget = glade_xml_get_widget (C2_WINDOW (wmain)->xml, "file_check_mail");
			submenu = GTK_MENU_ITEM (widget)->submenu;
			children = gtk_container_children (GTK_CONTAINER (submenu));
			
			/* Find the separator */
			sep = glade_xml_get_widget (C2_WINDOW (wmain)->xml, "file_check_mail_sep");
			for (l = children; l; l = g_list_next (l))
			{
				if (GTK_WIDGET (l->data) == sep)
					break;
			}
			
			/* And remove the rest */
			for (l = g_list_next (l); l; l = g_list_next (l))
				if (GTK_IS_WIDGET (l->data))
					gtk_widget_destroy (GTK_WIDGET (l->data));
			g_list_free (children);
			
			for (account = application->account; account; account = account->next)
			{
				if (!c2_account_is_checkeable (account))
					continue;

				item = gtk_pixmap_menu_item_new ();
				gtk_object_set_data (GTK_OBJECT (item), "account", account);
				gtk_signal_connect (GTK_OBJECT (item), "activate",
									GTK_SIGNAL_FUNC (on_menubar_file_check_mail_account_activate), wmain);

				pixmap = gnome_stock_pixmap_widget_at_size (GTK_WIDGET (wmain)->window,
															PKGDATADIR "/ui/mail.xpm", 16, 16);
				gtk_pixmap_menu_item_set_pixmap (GTK_PIXMAP_MENU_ITEM (item), pixmap);
				gtk_widget_show (pixmap);

				label = gtk_label_new (account->name);
				gtk_misc_set_alignment (GTK_MISC (label), 0, .5);

				hbox = gtk_hbox_new (FALSE, 0);
				gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);
				gtk_container_add (GTK_CONTAINER (item), hbox);
				gtk_widget_show (hbox);
				gtk_widget_show (label);

				gtk_menu_append (GTK_MENU (submenu), item);
				gtk_widget_show (item);
			}
		}

		/* Now do the Check button in the toolbar */
		widget = c2_toolbar_get_item (C2_TOOLBAR (wmain->toolbar), "toolbar_check");
		if (GTK_IS_WIDGET (widget))
			gtk_widget_set_sensitive (widget, c2_application_check_checkeable_account_exists (application));
		
		break;
	}
}

static void
on_outbox_mailbox_changed_mailbox (C2Mailbox *mailbox, C2MailboxChangeType type, C2Db *db_node,
									C2WindowMain *wmain)
{
	GtkWidget *widget;
	
	widget = c2_toolbar_get_item (C2_TOOLBAR (wmain->toolbar), "toolbar_send");
	if (GTK_IS_WIDGET (widget))
		gtk_widget_set_sensitive (widget, c2_db_length (mailbox));
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

/**
 * c2_window_main_get_mlist_selection
 * @wmain: Main window where to get the selection from.
 *
 * This function will return the selected object (if any)
 * in the main window @wmain in the Mailbox List.
 *
 * Return Value:
 * The selected object or %NULL.
 **/
GtkObject *
c2_window_main_get_mlist_selection (C2WindowMain *wmain)
{
	c2_return_val_if_fail (C2_IS_WINDOW_MAIN (wmain), NULL, C2EDATA);
	
	return c2_mailbox_list_get_selected_object (C2_MAILBOX_LIST (wmain->mlist));
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
on_menubar_file_check_mail_all_accounts_activate (GtkWidget *widget, C2WindowMain *wmain)
{
	C2_WINDOW_MAIN_CLASS_FW (wmain)->check (wmain);
}

static void
on_menubar_file_check_mail_account_activate (GtkWidget *widget, C2WindowMain *wmain)
{
	C2Application *application;
	C2Account *account;
	GtkWidget *wtl;
	C2TransferItem *wti;

	account = (C2Account*) gtk_object_get_data (GTK_OBJECT (widget), "account");
	application = C2_WINDOW (wmain)->application;
	wtl = c2_application_window_get (application, C2_WIDGET_TRANSFER_LIST_TYPE);

	if (!wtl || !C2_IS_TRANSFER_LIST (wtl))
		wtl = c2_transfer_list_new (application);

	gtk_widget_show (wtl);
	gdk_window_raise (wtl->window);

	wti = c2_transfer_item_new (application, account, C2_TRANSFER_ITEM_RECEIVE);
	c2_transfer_list_add_item (C2_TRANSFER_LIST (wtl), wti);
	c2_transfer_item_start (wti);
}

static void
on_menubar_file_new_message_activate (GtkWidget *widget, C2WindowMain *wmain)
{
	C2_WINDOW_MAIN_CLASS_FW (wmain)->compose (wmain);
}

static void
on_menubar_file_new_mailbox_activate (GtkWidget *widget, C2WindowMain *wmain)
{
	c2_application_dialog_add_mailbox (C2_WINDOW (wmain)->application);
}

static void
on_menubar_file_new_window_activate (GtkWidget *widget, C2WindowMain *wmain)
{
	GtkWidget *window;

	window = c2_window_main_new (C2_WINDOW (wmain)->application);
	gtk_widget_show (window);
}

static void
on_menubar_file_send_unsent_mails_activate (GtkWidget *widget, C2WindowMain *wmain)
{
	C2_WINDOW_MAIN_CLASS_FW (wmain)->send (wmain);
}

static void
on_menubar_file_save_activate (GtkWidget *widget, C2WindowMain *wmain)
{
	C2_WINDOW_MAIN_CLASS_FW (wmain)->save (wmain);
}

static void
on_menubar_file_print_activate (GtkWidget *widget, C2WindowMain *wmain)
{
	C2_WINDOW_MAIN_CLASS_FW (wmain)->print (wmain);
}

static void
on_menubar_file_exit_activate (GtkWidget *widget, C2WindowMain *wmain)
{
	C2_WINDOW_MAIN_CLASS_FW (wmain)->exit (wmain);
}

static void
on_menubar_view_toolbar_activate (GtkWidget *widget, C2WindowMain *wmain)
{
	GtkWidget *toolbar;
	gboolean active = GTK_CHECK_MENU_ITEM (widget)->active;

	toolbar = glade_xml_get_widget (C2_WINDOW (wmain)->xml, "dockitem_toolbar");
	
	if (active)
		gtk_widget_show (toolbar);
	else
		gtk_widget_hide (toolbar);

	c2_preferences_set_window_main_toolbar_visible (active);
	c2_preferences_commit ();

	gtk_widget_queue_resize (GTK_WIDGET (wmain));
}

static void
on_menubar_view_mail_preview_activate (GtkWidget *widget, C2WindowMain *wmain)
{
	GtkWidget *vpaned = glade_xml_get_widget (C2_WINDOW (wmain)->xml, "vpaned");
	GtkCList *clist = GTK_CLIST (wmain->index);
	gint row = clist->selection ? GPOINTER_TO_INT (clist->selection->data) : -1;
	gboolean active = GTK_CHECK_MENU_ITEM (widget)->active;
	
	if (active)
	{
		gtk_widget_show (wmain->mail);
		gtk_paned_set_position (GTK_PANED (vpaned), 
						c2_preferences_get_window_main_vpaned ());
	} else
	{
		gtk_widget_hide (wmain->mail);
		gtk_paned_set_position (GTK_PANED (vpaned), gdk_screen_height ());
	}

	c2_preferences_set_window_main_mail_preview_visible (active);
	c2_preferences_commit ();

	/* FIXME I have no idea why this shit doesn't work... */
	
	if (row > 0 && gtk_clist_row_is_visible (clist, row) != GTK_VISIBILITY_FULL)
		gtk_clist_moveto (clist, row, 0, 0, 0);
}

static void
on_menubar_view_headers_activate (GtkWidget *widget, C2WindowMain *wmain)
{
	gboolean active = GTK_CHECK_MENU_ITEM (widget)->active;

	c2_preferences_set_widget_mail_headers_visible (active);
	c2_mail_set_headers_visible (C2_MAIL (wmain->mail), active);
}

static void
on_menubar_view_network_traffic_activate (GtkWidget *widget, C2WindowMain *wmain)
{
	gboolean active = GTK_CHECK_MENU_ITEM (widget)->active;

	if (active)
	{
		wmain->nt = c2_network_traffic_new (application);
		gtk_box_pack_end (GTK_BOX (wmain->mail_vbox), wmain->nt, FALSE, TRUE, 0);
		gtk_widget_set_usize (wmain->nt, -1, 100);
		gtk_widget_show (wmain->nt);
	} else
	{
		gtk_widget_destroy (wmain->nt);
		wmain->nt = NULL;
	}

	c2_preferences_set_window_main_network_traffic_visible (active);
}

static void
on_menubar_view_mail_source_activate (GtkWidget *widget, C2WindowMain *wmain)
{
	C2Message *message;

	if (!C2_IS_MESSAGE ((message = c2_mail_get_message (C2_MAIL (wmain->mail)))))
		return;
	
	c2_application_dialog_mail_source (C2_WINDOW (wmain)->application, message);
}

static void
on_menubar_view_dialog_network_traffic_activate (GtkWidget *widget, C2WindowMain *wmain)
{
	c2_application_dialog_network_traffic (C2_WINDOW (wmain)->application);
}

static void
on_menubar_view_dialog_send_receive_activate (GtkWidget *widget, C2WindowMain *wmain)
{
	GtkWidget *wtl;
	
	wtl = c2_application_window_get (application, C2_WIDGET_TRANSFER_LIST_TYPE);

	if (!wtl || !C2_IS_TRANSFER_LIST (wtl))
		wtl = c2_transfer_list_new (application);
	
	/* Show the dialog */
	gtk_widget_show (wtl);
	gdk_window_raise (wtl->window);
}

static void
on_menubar_mailbox_speed_up_activate (GtkWidget *widget, C2WindowMain *wmain)
{
	C2_APPLICATION_CLASS_FW (C2_WINDOW (wmain)->application)->compact_mailboxes (
								C2_WINDOW (wmain)->application);
}

static void
on_menubar_message_reply_activate (GtkWidget *widget, C2WindowMain *wmain)
{
	C2_WINDOW_MAIN_CLASS_FW (wmain)->reply (wmain);
}

static void
on_menubar_message_reply_all_activate (GtkWidget *widget, C2WindowMain *wmain)
{
	C2_WINDOW_MAIN_CLASS_FW (wmain)->reply_all (wmain);
}

static void
on_menubar_message_forward_activate (GtkWidget *widget, C2WindowMain *wmain)
{
	C2_WINDOW_MAIN_CLASS_FW (wmain)->forward (wmain);
}

static void
on_menubar_message_copy_activate (GtkWidget *widget, C2WindowMain *wmain)
{
	C2_WINDOW_MAIN_CLASS_FW (wmain)->copy (wmain);
}

static void
on_menubar_message_move_activate (GtkWidget *widget, C2WindowMain *wmain)
{
	C2_WINDOW_MAIN_CLASS_FW (wmain)->move (wmain);
}

static void
on_menubar_message_delete_activate (GtkWidget *widget, C2WindowMain *wmain)
{
	C2_WINDOW_MAIN_CLASS_FW (wmain)->delete (wmain);
}

static void
on_menubar_message_mark_important_activate (GtkWidget *widget, C2WindowMain *wmain)
{
	GList *list, *l;

	list = c2_index_selection (C2_INDEX (wmain->index));

	gdk_threads_leave ();
	for (l = list; l; l = g_list_next (l))
	{
		C2Db *db = (C2Db*) l->data;
		
		c2_db_message_set_mark (db, !db->mark);
	}
	gdk_threads_enter ();

	g_list_free (list);
}

static void
on_menubar_message_mark_unreaded_activate (GtkWidget *widget, C2WindowMain *wmain)
{
	GList *list, *l;

	list = c2_index_selection (C2_INDEX (wmain->index));

	gdk_threads_leave ();
	for (l = list; l; l = g_list_next (l))
	{
		C2Db *db = (C2Db*) l->data;
		
		c2_db_message_set_state (db, C2_MESSAGE_UNREADED);
	}
	gdk_threads_enter ();

	g_list_free (list);
}

static void
on_menubar_message_mark_readed_activate (GtkWidget *widget, C2WindowMain *wmain)
{
	GList *list, *l;

	list = c2_index_selection (C2_INDEX (wmain->index));

	gdk_threads_leave ();
	for (l = list; l; l = g_list_next (l))
	{
		C2Db *db = (C2Db*) l->data;
		
		c2_db_message_set_state (db, C2_MESSAGE_READED);
	}
	gdk_threads_enter ();

	g_list_free (list);
}

static void
on_menubar_message_mark_replied_activate (GtkWidget *widget, C2WindowMain *wmain)
{
	GList *list, *l;

	list = c2_index_selection (C2_INDEX (wmain->index));

	gdk_threads_leave ();
	for (l = list; l; l = g_list_next (l))
	{
		C2Db *db = (C2Db*) l->data;
		
		c2_db_message_set_state (db, C2_MESSAGE_REPLIED);
	}
	gdk_threads_enter ();

	g_list_free (list);
}

static void
on_menubar_message_mark_forwarded_activate (GtkWidget *widget, C2WindowMain *wmain)
{
	GList *list, *l;

	list = c2_index_selection (C2_INDEX (wmain->index));

	gdk_threads_leave ();
	for (l = list; l; l = g_list_next (l))
	{
		C2Db *db = (C2Db*) l->data;
		
		c2_db_message_set_state (db, C2_MESSAGE_FORWARDED);
	}
	gdk_threads_enter ();

	g_list_free (list);
}

static void
on_menubar_message_next_activate (GtkWidget *widget, C2WindowMain *wmain)
{
	C2_WINDOW_MAIN_CLASS_FW (wmain)->next (wmain);
}

static void
on_menubar_message_previous_activate (GtkWidget *widget, C2WindowMain *wmain)
{
	C2_WINDOW_MAIN_CLASS_FW (wmain)->previous (wmain);
}

static void
on_menubar_settings_preferences_activate (GtkWidget *widget, C2WindowMain *wmain)
{
	GtkWidget *preferences;

	preferences = c2_dialog_preferences_new (C2_WINDOW (wmain)->application);
	gtk_widget_show (preferences);	
}

static void
on_menubar_settings_features_download_activate (GtkWidget *widget, C2WindowMain *wmain)
{
	c2_application_dialog_add_features (C2_WINDOW (wmain)->application);
}

static void
on_menubar_help_getting_in_touch_activate (GtkWidget *widget, C2WindowMain *wmain)
{
	c2_application_dialog_getting_in_touch (C2_WINDOW (wmain)->application);
}

static void
on_menubar_help_release_information_activate (GtkWidget *widget, C2WindowMain *wmain)
{
	c2_application_dialog_release_information (C2_WINDOW (wmain)->application);
}

static void
on_menubar_help_about_activate (GtkWidget *widget, C2WindowMain *wmain)
{
	c2_application_dialog_about (C2_WINDOW (wmain)->application);
}

static void
on_menubar_help_donations_activate (GtkWidget *widget, C2WindowMain *wmain)
{
	gnome_url_show (URL "site/donations.php");
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
on_index_open_message (GtkWidget *index, C2Db *node, C2WindowMain *wmain)
{
	C2Mailbox *mailbox;

	mailbox = c2_mailbox_list_get_selected_mailbox (C2_MAILBOX_LIST (wmain->mlist));
	if (!C2_IS_MAILBOX (mailbox))
		g_assert_not_reached ();

	if (mailbox->use_as & C2_MAILBOX_USE_AS_DRAFTS)
	{
		GtkWidget *composer;

		if ((composer = c2_composer_new (C2_WINDOW (wmain)->application)))
		{
			if (!c2_db_load_message (node))
			{
				gtk_widget_destroy (composer);
				return;
			}
			
			gtk_object_ref (GTK_OBJECT (node->message));
			c2_composer_set_message_as_draft (C2_COMPOSER (composer), node, node->message);
			gtk_widget_show (composer);
			gtk_object_unref (GTK_OBJECT (node->message));
		}
	} else
	{
		/* Open a window message */
	}
}

static void
on_index_select_message_thread (C2Pthread3 *data)
{
	GtkWidget *index = GTK_WIDGET (data->v1);
	C2Db *node = C2_DB (data->v2);
	C2WindowMain *wmain = C2_WINDOW_MAIN (data->v3);
	GtkWidget *widget;
	GladeXML *xml;

	g_free (data);

	if (!C2_IS_MESSAGE (node->message))
	{
		/* [TODO] This should be in a separated thread */
		c2_db_load_message (node);

		if (!C2_IS_MESSAGE (node->message))
		{
			/* Something went wrong */
			const gchar *error;
	
			gdk_threads_enter ();
			c2_mail_set_file (C2_MAIL (wmain->mail), PKGDATADIR "/message_404.html");
			error = c2_error_object_get (GTK_OBJECT (node));
			if (error)
				c2_window_report (C2_WINDOW (wmain), C2_WINDOW_REPORT_WARNING,
								error_list[C2_FAIL_MESSAGE_LOAD], error);
			else
				c2_window_report (C2_WINDOW (wmain), C2_WINDOW_REPORT_WARNING,
								error_list[C2_FAIL_MESSAGE_LOAD], error_list[C2_UNKNOWN_REASON]);

			gdk_threads_leave ();
			return;
		}
	}

	gdk_threads_enter ();
	
	c2_mail_set_message (C2_MAIL (wmain->mail), node->message);

	/* Set some widgets sensivity */
	xml = C2_WINDOW (wmain)->xml;
	
	widget = glade_xml_get_widget (xml, "message_previous");
	gtk_widget_set_sensitive (widget, !c2_db_is_first (node));
	
	widget = c2_toolbar_get_item (C2_TOOLBAR (wmain->toolbar), "toolbar_previous");
	if (GTK_IS_WIDGET (widget))
		gtk_widget_set_sensitive (widget, !c2_db_is_first (node));
	
	widget = glade_xml_get_widget (xml, "message_next");
	gtk_widget_set_sensitive (widget, !c2_db_is_last (node));
	
	widget = c2_toolbar_get_item (C2_TOOLBAR (wmain->toolbar), "toolbar_next");
	if (GTK_IS_WIDGET (widget))
		gtk_widget_set_sensitive (widget, !c2_db_is_last (node));

	widget = glade_xml_get_widget (xml, "message_copy");
	gtk_widget_set_sensitive (widget, TRUE);
	
	widget = c2_toolbar_get_item (C2_TOOLBAR (wmain->toolbar), "toolbar_copy");
	if (GTK_IS_WIDGET (widget))
		gtk_widget_set_sensitive (widget, TRUE);

	widget = glade_xml_get_widget (xml, "message_move");
	gtk_widget_set_sensitive (widget, TRUE);
	
	widget = c2_toolbar_get_item (C2_TOOLBAR (wmain->toolbar), "toolbar_move");
	if (GTK_IS_WIDGET (widget))
		gtk_widget_set_sensitive (widget, TRUE);

	widget = glade_xml_get_widget (xml, "file_save");
	gtk_widget_set_sensitive (widget, TRUE);
	
	widget = c2_toolbar_get_item (C2_TOOLBAR (wmain->toolbar), "toolbar_save");
	if (GTK_IS_WIDGET (widget))
		gtk_widget_set_sensitive (widget, TRUE);

	widget = glade_xml_get_widget (xml, "file_print");
	gtk_widget_set_sensitive (widget, TRUE);
	
	widget = c2_toolbar_get_item (C2_TOOLBAR (wmain->toolbar), "toolbar_print");
	if (GTK_IS_WIDGET (widget))
		gtk_widget_set_sensitive (widget, TRUE);

	widget = glade_xml_get_widget (xml, "message_delete");
	gtk_widget_set_sensitive (widget, TRUE);
	
	widget = c2_toolbar_get_item (C2_TOOLBAR (wmain->toolbar), "toolbar_delete");
	if (GTK_IS_WIDGET (widget))
		gtk_widget_set_sensitive (widget, TRUE);

	widget = glade_xml_get_widget (xml, "message_expunge");
	gtk_widget_set_sensitive (widget, TRUE);

	widget = glade_xml_get_widget (xml, "message_reply");
	gtk_widget_set_sensitive (widget, TRUE);
	
	widget = c2_toolbar_get_item (C2_TOOLBAR (wmain->toolbar), "toolbar_reply");
	if (GTK_IS_WIDGET (widget))
		gtk_widget_set_sensitive (widget, TRUE);

	widget = glade_xml_get_widget (xml, "message_reply_all");
	gtk_widget_set_sensitive (widget, TRUE);
	
	widget = c2_toolbar_get_item (C2_TOOLBAR (wmain->toolbar), "toolbar_reply_all");
	if (GTK_IS_WIDGET (widget))
		gtk_widget_set_sensitive (widget, TRUE);

	widget = glade_xml_get_widget (xml, "message_forward");
	gtk_widget_set_sensitive (widget, TRUE);
	
	widget = c2_toolbar_get_item (C2_TOOLBAR (wmain->toolbar), "toolbar_forward");
	if (GTK_IS_WIDGET (widget))
		gtk_widget_set_sensitive (widget, TRUE);

	gdk_threads_leave ();
L}

static void
on_index_select_message (GtkWidget *index, C2Db *node, C2WindowMain *wmain)
{
	C2Pthread3 *data;
	pthread_t thread;
	
	if (g_list_length (GTK_CLIST (index)->selection) > 1)
		return;

	data = g_new0 (C2Pthread3, 1);
	data->v1 = (gpointer) index;
	data->v2 = (gpointer) node;
	data->v3 = (gpointer) wmain;

	pthread_create (&thread, NULL, C2_PTHREAD_FUNC (on_index_select_message_thread), data);
}

static void
on_index_unselect_message (C2Index *index, C2Db *node, C2WindowMain *wmain)
{
	GtkWidget *widget;
	GladeXML *xml;
L
	/* Set some widgets sensivity */
	xml = C2_WINDOW (wmain)->xml;
	
	widget = glade_xml_get_widget (xml, "message_previous");
	gtk_widget_set_sensitive (widget, FALSE);
	
	widget = c2_toolbar_get_item (C2_TOOLBAR (wmain->toolbar), "toolbar_previous");
	if (GTK_IS_WIDGET (widget))
		gtk_widget_set_sensitive (widget, FALSE);
	
	widget = glade_xml_get_widget (xml, "message_next");
	gtk_widget_set_sensitive (widget, FALSE);
	
	widget = c2_toolbar_get_item (C2_TOOLBAR (wmain->toolbar), "toolbar_next");
	if (GTK_IS_WIDGET (widget))
		gtk_widget_set_sensitive (widget, FALSE);

	widget = glade_xml_get_widget (xml, "message_copy");
	gtk_widget_set_sensitive (widget, FALSE);
	
	widget = c2_toolbar_get_item (C2_TOOLBAR (wmain->toolbar), "toolbar_copy");
	if (GTK_IS_WIDGET (widget))
		gtk_widget_set_sensitive (widget, FALSE);

	widget = glade_xml_get_widget (xml, "message_move");
	gtk_widget_set_sensitive (widget, FALSE);
	
	widget = c2_toolbar_get_item (C2_TOOLBAR (wmain->toolbar), "toolbar_move");
	if (GTK_IS_WIDGET (widget))
		gtk_widget_set_sensitive (widget, FALSE);

	widget = glade_xml_get_widget (xml, "file_save");
	gtk_widget_set_sensitive (widget, FALSE);
	
	widget = c2_toolbar_get_item (C2_TOOLBAR (wmain->toolbar), "toolbar_save");
	if (GTK_IS_WIDGET (widget))
		gtk_widget_set_sensitive (widget, FALSE);

	widget = glade_xml_get_widget (xml, "file_print");
	gtk_widget_set_sensitive (widget, FALSE);
	
	widget = c2_toolbar_get_item (C2_TOOLBAR (wmain->toolbar), "toolbar_print");
	if (GTK_IS_WIDGET (widget))
		gtk_widget_set_sensitive (widget, FALSE);

	widget = glade_xml_get_widget (xml, "message_delete");
	gtk_widget_set_sensitive (widget, FALSE);
	
	widget = c2_toolbar_get_item (C2_TOOLBAR (wmain->toolbar), "toolbar_delete");
	if (GTK_IS_WIDGET (widget))
		gtk_widget_set_sensitive (widget, FALSE);

	widget = glade_xml_get_widget (xml, "message_expunge");
	gtk_widget_set_sensitive (widget, FALSE);

	widget = glade_xml_get_widget (xml, "message_reply");
	gtk_widget_set_sensitive (widget, FALSE);
	
	widget = c2_toolbar_get_item (C2_TOOLBAR (wmain->toolbar), "toolbar_reply");
	if (GTK_IS_WIDGET (widget))
		gtk_widget_set_sensitive (widget, FALSE);

	widget = glade_xml_get_widget (xml, "message_reply_all");
	gtk_widget_set_sensitive (widget, FALSE);
	
	widget = c2_toolbar_get_item (C2_TOOLBAR (wmain->toolbar), "toolbar_reply_all");
	if (GTK_IS_WIDGET (widget))
		gtk_widget_set_sensitive (widget, FALSE);

	widget = glade_xml_get_widget (xml, "message_forward");
	gtk_widget_set_sensitive (widget, FALSE);
	
	widget = c2_toolbar_get_item (C2_TOOLBAR (wmain->toolbar), "toolbar_forward");
	if (GTK_IS_WIDGET (widget))
		gtk_widget_set_sensitive (widget, FALSE);
L}

static gint
on_mlist_object_selected_pthread (C2WindowMain *wmain)
{
	C2Mailbox *mailbox = c2_mailbox_list_get_selected_mailbox (C2_MAILBOX_LIST (wmain->mlist));
	C2Index *index = C2_INDEX (wmain->index);
	
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
			c2_index_clear (index);
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
	
	if (c2_mutex_trylock (&wmain->index_lock))
		return 0;
	
	gdk_threads_enter ();
	c2_index_clear (index);
	c2_index_set_mailbox (index, mailbox);
	c2_window_set_activity (C2_WINDOW (wmain), FALSE);
	gtk_widget_queue_draw (GTK_WIDGET (index));
	c2_index_sort (index, mailbox->sort_by, mailbox->sort_type);
	c2_window_report (C2_WINDOW (wmain), C2_WINDOW_REPORT_MESSAGE,
						_("%d messages, %d new."), c2_db_length (mailbox),
						c2_db_length_type (mailbox, C2_MESSAGE_UNREADED));
	gdk_threads_leave ();

	c2_mutex_unlock (&wmain->index_lock);
	return 0;
}

static void
on_mlist_object_selected (C2MailboxList *mlist, GtkObject *object, C2WindowMain *wmain)
{
	static GtkWidget *index_scroll = NULL;
	static GtkWidget *view_mail_preview = NULL;
	static GtkWidget *view_headers = NULL;
	static GtkWidget *vpaned = NULL;
	static C2WindowMain *static_wmain = NULL;
	static gint pan_size = -1;
	static unsigned char showing_mailbox = TRUE;
	pthread_t thread;

	if (static_wmain != wmain)
	{
		index_scroll = glade_xml_get_widget (C2_WINDOW (wmain)->xml, "index_scroll");
		view_mail_preview = glade_xml_get_widget (C2_WINDOW (wmain)->xml, "view_mail_preview");
		view_headers = glade_xml_get_widget (C2_WINDOW (wmain)->xml, "view_headers");
		vpaned = glade_xml_get_widget (C2_WINDOW (wmain)->xml, "vpaned");
	}

	if (C2_IS_MAILBOX (object))
	{
		if (!showing_mailbox)
		{
			gtk_widget_show (index_scroll);
			if (GTK_CHECK_MENU_ITEM (view_mail_preview)->active)
				gtk_widget_show (wmain->mail);
	
			c2_mail_set_headers_visible (C2_MAIL (wmain->mail),
									GTK_CHECK_MENU_ITEM (view_headers)->active);
				
			gtk_paned_set_position (GTK_PANED (vpaned), pan_size);
		}
		
		showing_mailbox = TRUE;
		pthread_create (&thread, NULL, C2_PTHREAD_FUNC (on_mlist_object_selected_pthread), wmain);
	} else
	{
		gchar *string = NULL;
		
		if (showing_mailbox)
		{
			gtk_widget_hide (index_scroll);
			gtk_widget_show (wmain->mail);
			c2_index_clear (C2_INDEX (wmain->index));

			c2_mail_set_headers_visible (C2_MAIL (wmain->mail), FALSE);

			if (showing_mailbox)
				pan_size = GTK_PANED (vpaned)->child1_size;

			showing_mailbox = FALSE;
			gtk_paned_set_position (GTK_PANED (vpaned), 0);
		}

		if (C2_IS_ACCOUNT (object))
		{
		} else
		{
			gchar *buf1, *buf2, *text;
			
			buf1 = g_getenv ("HOSTNAME");
			buf2 = g_get_user_name ();
			text = g_strdup_printf ("%s@%s", buf2, buf1);
			
#if defined (USE_GTKHTML) || defined (USE_GTKXMHTML)
			string = g_strdup_printf (
"<html>
<body bgcolor=#ffffff>
<h1>%s</h1>

<table cellspacing=4>
  <tr>
    <td>
	
	  <table bgcolor=#ff9000 cellspacing=0 cellpadding=1 border=0 width=150>
	    <tr>
		  <td align=\"center\">
		    <font color=#ffffff face=\"Verdana, Helvetica\" size=\"-1\"><b>%s</b></font>
	      </td>
        </tr>
	    <tr>
		  <td width=150>
	        <table bgcolor=#ffd050 cellspacing=0 height=\"100%%\" valign=\"top\">
			  <tr>
			    <td>
				  <font face=\"Verdana, Helvetica\" size=\"-1\" color=#000000>%s</font>
				</td>
			  </tr>
			</table>
		  </td>
		</tr>
	  </table>

	</td>
    <td>
	  <font face=\"Verdana, Helvetica\">%s</font>
	</td>
  </tr>
</table>

</body>
</html>
", text, _("Tip of the day"), _("There are no available tips yet."),
_("This folder parents all the local mailboxes that <b>Cronos II</b> handles for you."));
#else
		
#endif
		}

		c2_mail_set_string (C2_MAIL (wmain->mail), string);
		g_free (string);
	}
}

static void
on_mlist_object_unselected (C2MailboxList *mlist, C2WindowMain *wmain)
{
	if (!c2_mutex_trylock (&wmain->index_lock))
	{
		GtkWidget *index = wmain->index;

		c2_index_clear (C2_INDEX (index));
		printf ("[ DOING ON_MLIST_OBJECT_UNSELECTED ]\n");
		c2_mail_set_message (C2_MAIL (wmain->mail), NULL);
		c2_mutex_unlock (&wmain->index_lock);
	}
}

static gint
eastern_egg_timeout (C2Pthread3 *data)
{
	/* Que onda si uso GdkPixbuf (como lo de su ejemplo)
	 * para hacer alguna precentación onda copada?
	 */
	return FALSE;
}

static void
on_eastern_egg_separator_activate (GtkWidget *widget, C2WindowMain *wmain)
{
	C2Window *window = C2_WINDOW (wmain);
	GtkWidget *dialog;
	GladeXML *xml;
	C2Pthread3 *data;

	dialog = c2_dialog_new (window->application, _("You found the Eastern Egg!"), NULL,
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
#if 1 /* Expunge Mails Confirmation Dialog */
static gboolean
dlg_confirm_expunge_message (C2WindowMain *wmain)
{
	C2Application *application;
	GtkWidget *dialog;
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
	GList *available = NULL;
	GList *current = NULL;
	GList *l;
	gint i;

	dialog = c2_dialog_new (C2_WINDOW (wmain)->application, _("Toolbar Configuration"),
							"toolbar_configuration", NULL, GNOME_STOCK_BUTTON_HELP,
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
													toolbar_items[val].name,
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
							c2_toolbar_append_widget (C2_TOOLBAR (wmain->toolbar), toolbar_items[val].name,
											toolbar_items[val].widget, toolbar_items[val].widget_tooltip);
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
