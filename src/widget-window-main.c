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
on_toolbar_check_clicked					(GtkWidget *widget, C2WindowMain *wmain);

static void
on_index_select_message						(GtkWidget *index, C2Db *node, C2WindowMain *wmain);

static void
on_mlist_mailbox_selected					(C2MailboxList *mlist, C2Mailbox *mailbox, C2WindowMain *wmain);

static void
on_mlist_mailbox_unselected					(C2MailboxList *mlist, C2Mailbox *mailbox, C2WindowMain *wmain);

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
on_close_clicked							(GtkWidget *widget, C2WindowMain *wmain);

static void
on_preferences_changed						(C2DialogPreferences *preferences,
											 C2DialogPreferencesKey key, gpointer value, C2WindowMain *wmain);

static void
on_toolbar_menu_toolbar_style_item_toggled	(GtkWidget *object, C2WindowMain *wmain);

/* in widget-application.c */
extern void
on_mailbox_changed_mailboxes				(C2Mailbox *mailbox, C2Application *application);

enum
{
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static C2WindowClass *parent_class = NULL;

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

	/* Toolbar */
	toolbar = glade_xml_get_widget (xml, "toolbar");
	gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), application->interface_toolbar);

	/* Hpaned */
	hpaned = glade_xml_get_widget (xml, "hpaned");
	gtk_paned_set_position (GTK_PANED (hpaned), application->rc_hpan);

	/* Mailbox List */
	wmain->mlist = c2_mailbox_list_new (application);
	scroll = glade_xml_get_widget (xml, "mlist_scroll");
	gtk_container_add (GTK_CONTAINER (scroll), wmain->mlist);
	gtk_widget_show (wmain->mlist);

	gtk_signal_connect (GTK_OBJECT (wmain->mlist), "mailbox_selected",
								GTK_SIGNAL_FUNC (on_mlist_mailbox_selected), wmain);
	gtk_signal_connect (GTK_OBJECT (wmain->mlist), "mailbox_unselected",
								GTK_SIGNAL_FUNC (on_mlist_mailbox_unselected), wmain);
	gtk_signal_connect (GTK_OBJECT (wmain->mlist), "button_press_event",
      			GTK_SIGNAL_FUNC (on_mlist_button_press_event), wmain);

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
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "toolbar_check")), "clicked",
							GTK_SIGNAL_FUNC (on_toolbar_check_clicked), wmain);
/*	glade_xml_signal_connect (xml, "on_new_mail_activate", GTK_SIGNAL_FUNC (on_new_mail_activate));
	gtk_signal_connect_object (GTK_OBJECT (glade_xml_get_widget (xml, "file_exit")), "activate",
							GTK_SIGNAL_FUNC (on_quit), NULL);
	gtk_signal_connect_object (GTK_OBJECT(glade_xml_get_widget(xml,"settings_preferences")), "activate",
							GTK_SIGNAL_FUNC (on_preferences_activated), NULL);
*/	
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "file_new_mailbox")), "activate",
							GTK_SIGNAL_FUNC (on_file_new_mailbox_activate), wmain);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "file_egg_separator")), "activate",
							GTK_SIGNAL_FUNC (on_eastern_egg_separator_activate), wmain);
	
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "settings_preferences")), "activate",
							GTK_SIGNAL_FUNC (on_settings_preferences_activate), wmain);
	
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "help_about")), "activate",
							GTK_SIGNAL_FUNC (on_about_activate), wmain);

/*	gtk_signal_connect_object (GTK_OBJECT (glade_xml_get_widget (xml, "help_getting_in_touch")),
							"activate",	GTK_SIGNAL_FUNC (on_getting_in_touch_activated), NULL);
	gtk_signal_connect_object (GTK_OBJECT (glade_xml_get_widget (wmain->ctree_menu, "new_mailbox")), "activate",
							GTK_SIGNAL_FUNC (on_new_mailbox_dlg), NULL);
	gtk_signal_connect_object (GTK_OBJECT (glade_xml_get_widget (wmain->ctree_menu, "properties")), "activate",
							GTK_SIGNAL_FUNC (on_properties_mailbox_dlg), NULL);
	gtk_signal_connect_object (GTK_OBJECT (glade_xml_get_widget (wmain->ctree_menu, "delete_mailbox")),
							"activate",	GTK_SIGNAL_FUNC (on_delete_mailbox_dlg), NULL);

	gtk_signal_connect_object (GTK_OBJECT (glade_xml_get_widget (xml, "file_check_mail_all_accounts")), "activate",
							GTK_SIGNAL_FUNC (on_check_clicked), NULL);
	gtk_signal_connect_object (GTK_OBJECT (glade_xml_get_widget (xml, "toolbar_check")), "clicked",
							GTK_SIGNAL_FUNC (on_check_clicked), NULL);
*/	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "toolbar_exit")), "clicked",
							GTK_SIGNAL_FUNC (on_close_clicked), wmain);
	
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (wmain->toolbar_menu, "text")), "toggled",
							GTK_SIGNAL_FUNC (on_toolbar_menu_toolbar_style_item_toggled), wmain);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (wmain->toolbar_menu, "icons")), "toggled",
							GTK_SIGNAL_FUNC (on_toolbar_menu_toolbar_style_item_toggled), wmain);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (wmain->toolbar_menu, "both")), "toggled",
							GTK_SIGNAL_FUNC (on_toolbar_menu_toolbar_style_item_toggled), wmain);
	
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
}

static void
on_docktoolbar_button_press_event (GtkWidget *widget, GdkEventButton *event, C2WindowMain *wmain)
{
	C2Application *application;
	GtkWidget *toolbar;
	c2_return_if_fail (event, C2EDATA);
	
	switch (event->button)
	{
		case 3:
			application = C2_WINDOW (wmain)->application;
			switch (application->interface_toolbar)
			{
				case GTK_TOOLBAR_BOTH:
					toolbar = glade_xml_get_widget (wmain->toolbar_menu, "text");
					gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (toolbar), FALSE);
					toolbar = glade_xml_get_widget (wmain->toolbar_menu, "both");
					gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (toolbar), TRUE);
					toolbar = glade_xml_get_widget (wmain->toolbar_menu, "icons");
					gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (toolbar), FALSE);
					break;
				case GTK_TOOLBAR_ICONS:
					toolbar = glade_xml_get_widget (wmain->toolbar_menu, "icons");
					gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (toolbar), TRUE);
					break;
				case GTK_TOOLBAR_TEXT:
					toolbar = glade_xml_get_widget (wmain->toolbar_menu, "text");
					gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (toolbar), TRUE);
					break;
			}
			gnome_popup_menu_do_popup (glade_xml_get_widget (wmain->toolbar_menu, "mnu_toolbar"),
								NULL, NULL, event, NULL);
			break;
	}
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
on_index_select_message (GtkWidget *index, C2Db *node, C2WindowMain *wmain)
{
	c2_db_load_message (node);
	c2_mail_set_message (C2_MAIL (glade_xml_get_widget (C2_WINDOW (wmain)->xml, "mail")), node->message);
	c2_db_unload_message (node);
}

static gint
on_mlist_mailbox_selected_pthread (C2WindowMain *wmain)
{
	C2Mailbox *mailbox = c2_mailbox_list_get_selected_mailbox (C2_MAILBOX_LIST (wmain->mlist));
	C2Index *index = C2_INDEX (wmain->index);
	gchar *buf;
	
	gdk_threads_enter ();
L	c2_window_set_activity (C2_WINDOW (wmain), TRUE);
	gdk_threads_leave ();
	
	if (!mailbox->db)
	{
L		if (!c2_mailbox_load_db (mailbox))
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
	
	buf = g_strdup_printf (_("%d messages, %d new."), c2_db_length (mailbox),
							c2_db_length_type (mailbox, C2_MESSAGE_UNREADED));
	gdk_threads_enter ();
	c2_index_remove_mailbox (index);
	c2_index_add_mailbox (index, mailbox);
	c2_window_set_activity (C2_WINDOW (wmain), FALSE);
	gtk_widget_queue_draw (GTK_WIDGET (index));
	c2_index_sort (index);
	if (!pthread_mutex_trylock (&C2_WINDOW (wmain)->status_lock))
	{
		gnome_appbar_set_status (GNOME_APPBAR (glade_xml_get_widget (C2_WINDOW (wmain)->xml, "appbar")),
									buf);
		pthread_mutex_unlock (&C2_WINDOW (wmain)->status_lock);
	}
	gdk_threads_leave ();
	g_free (buf);

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
on_mlist_mailbox_unselected (C2MailboxList *mlist, C2Mailbox *mailbox, C2WindowMain *wmain)
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
	gtk_signal_connect (GTK_OBJECT (preferences), "changed",
							GTK_SIGNAL_FUNC (on_preferences_changed), wmain);
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
on_close_clicked (GtkWidget *widget, C2WindowMain *wmain)
{
#ifdef USE_DEBUG
	g_print ("on_close_clicked was reached (%s)\n", (gchar*) gtk_object_get_data (GTK_OBJECT (wmain), "type"));
#endif
	gtk_object_destroy (GTK_OBJECT (wmain));
}

static void
on_preferences_changed (C2DialogPreferences *preferences, C2DialogPreferencesKey key, gpointer value,
						C2WindowMain *wmain)
{
	gtk_signal_emit_by_name (GTK_OBJECT (C2_WINDOW (wmain)->application), "application_preferences_changed",
								key, value);
}

static void
on_toolbar_menu_toolbar_style_item_toggled (GtkWidget *object, C2WindowMain *wmain)
{
	GtkWidget *wicons;
	GtkWidget *wlabel;
	GtkWidget *wboth;
	GtkWidget *toolbar;
	gboolean icons;
	gboolean label;
	gboolean both;

	wicons = glade_xml_get_widget (wmain->toolbar_menu, "icons");
	wlabel = glade_xml_get_widget (wmain->toolbar_menu, "text");
	wboth = glade_xml_get_widget (wmain->toolbar_menu, "both");
	toolbar = glade_xml_get_widget (C2_WINDOW (wmain)->xml, "toolbar");

	icons = GTK_TOGGLE_BUTTON (wicons)->active;
	label = GTK_TOGGLE_BUTTON (wlabel)->active;
	both = GTK_TOGGLE_BUTTON (wboth)->active;

	if (icons)
		gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), GTK_TOOLBAR_ICONS);
	else if (label)
		gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), GTK_TOOLBAR_TEXT);
	else if (both)
		gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), GTK_TOOLBAR_BOTH);
}


static void
on_preferences_activated (GtkWidget *widget)
{
	c2_preferences_new ();
}

static void
on_check_clicked (GtkWidget *widget, C2WindowMain *wmain)
{
#if 0
	static GtkWidget *mt = NULL;
	C2Account *account;

	if (!mt)
	{
		mt = c2_message_transfer_new (C2_WINDOW (wmain)->application);
		gtk_widget_show (mt);
	} else
	{
		gtk_widget_show(mt);
		gdk_window_raise (mt->window);
	}
	
	c2_message_transfer_freeze (C2_MESSAGE_TRANSFER (mt));
	for (account = C2_WINDOW (wmain)->application->account; account != NULL;
				account = c2_account_next (account)) 
		if (account->options.active)
			c2_message_transfer_append (C2_MESSAGE_TRANSFER (mt), account, C2_MESSAGE_TRANSFER_MANUAL,
										C2_MESSAGE_TRANSFER_CHECK);
	
	c2_message_transfer_thaw (C2_MESSAGE_TRANSFER (mt));
#endif
}

static void
on_new_mail_activate (GtkWidget *widget, C2WindowMain *wmain)
{
	GtkWidget *composer;
	GtkWidget *window;

	if (!c2_application_check_account_exists (C2_WINDOW (wmain)->application))
		return;

	composer = c2_composer_new (C2_WINDOW (wmain)->application->interface_toolbar);

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

#if 0
static void
on_quit (void)
{
	GtkWidget *widget;
	GtkWidget *window = glade_xml_get_widget (WMain.xml, "wnd_main");
	GList *l;
	
	for (l = c2_app.open_windows; l != NULL; l = l->next)
	{
		if (!GTK_IS_WINDOW (l->data) || (GtkWindow*)l->data == GTK_WINDOW (window))
			continue;
		gtk_signal_emit_by_name (GTK_OBJECT ((GtkWindow*)l->data), "delete_event");
	}

	if ((widget = glade_xml_get_widget (WMain.xml, "hpaned")))
		gnome_config_set_int ("/Cronos II/Rc/hpan", GTK_PANED (widget)->child1_size);

	if ((widget = glade_xml_get_widget (WMain.xml, "vpaned")))
		gnome_config_set_int ("/Cronos II/Rc/vpan", GTK_PANED (widget)->child1_size);
	

	
	gtk_widget_destroy (window);
	gnome_config_sync ();
	gtk_main_quit ();
}
#endif

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

	dialog = c2_dialog_new (C2_WINDOW (wmain)->application, _("New mailbox"), GNOME_STOCK_BUTTON_HELP,
							GNOME_STOCK_BUTTON_OK, GNOME_STOCK_BUTTON_CANCEL, NULL);
	xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("cronosII"), "dlg_mailbox_properties_contents");
	C2_DIALOG (dialog)->xml = xml;

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

				/* Hehe, a little eastern egg :) */
				if (c2_streq (name, ""))


				if (!C2_WINDOW (wmain)->application)
					L

				C2_DEBUG (name);
L
				/* Check if the name is valid */
L				if (!name || !strlen (name) ||
					c2_mailbox_get_by_name (C2_WINDOW (wmain)->application->mailbox, name))
				{
					GladeXML *err_xml;
					GtkWidget *err_dialog;
L					
					err_xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("cronosII"), "dlg_mailbox_err");
					err_dialog = glade_xml_get_widget (err_xml, "dlg_mailbox_err");
L
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
L							user = gtk_entry_get_text (GTK_ENTRY (
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
								
L								gtk_window_set_modal (GTK_WINDOW (err_dialog), TRUE);
								gnome_dialog_run_and_close (GNOME_DIALOG (err_dialog));
								
								gtk_object_destroy (GTK_OBJECT (err_xml));
								
								goto re_run_add_mailbox_dialog;
							}
						} else if (c2_streq (query, MAILBOX_TYPE_SPOOL))
						{
L							type = C2_MAILBOX_SPOOL;
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
L								
								gtk_object_destroy (GTK_OBJECT (err_xml));
								
								goto re_run_add_mailbox_dialog;
							}
						}
					}
				}

				/* Get parent mailbox */
				parent = c2_mailbox_list_get_selected_mailbox (C2_MAILBOX_LIST (wmain->mlist));
L
				switch (type)
				{
					case C2_MAILBOX_CRONOSII:
L						mailbox = c2_mailbox_new_with_parent (name, parent ? parent->id : NULL, type,
													C2_MAILBOX_SORT_DATE, GTK_SORT_ASCENDING);
						break;
					case C2_MAILBOX_IMAP:
						mailbox = c2_mailbox_new_with_parent (name, parent ? parent->id : NULL, type,
													C2_MAILBOX_SORT_DATE, GTK_SORT_ASCENDING,
													host, port, user, pass, path);
						break;
					case C2_MAILBOX_SPOOL:
						mailbox = c2_mailbox_new_with_parent (name, parent ? parent->id : NULL, type,
													C2_MAILBOX_SORT_DATE, GTK_SORT_ASCENDING, path);
						break;
				}
L
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

L				if (C2_WINDOW (wmain)->application->advanced_load_mailboxes_at_start)
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
				
L				gnome_config_set_string ("name", mailbox->name);
				gnome_config_set_string ("id", mailbox->id);
				gnome_config_set_int ("type", mailbox->type);
				gnome_config_set_int ("sort_by", mailbox->sort_by);
				gnome_config_set_int ("sort_type", mailbox->sort_type);
				
L				switch (mailbox->type)
				{
					case C2_MAILBOX_IMAP:
						gnome_config_set_string ("host", mailbox->protocol.imap.host);
						gnome_config_set_string ("user", mailbox->protocol.imap.user);
L						gnome_config_set_string ("pass", mailbox->protocol.imap.pass);
						gnome_config_set_string ("path", mailbox->protocol.imap.path);
						gnome_config_set_int ("port", mailbox->protocol.imap.port);
						break;
					case C2_MAILBOX_SPOOL:
						gnome_config_set_string ("path", mailbox->protocol.spool.path);
						break;
				}
				gnome_config_pop_prefix ();
L				g_free (query);
				
				gnome_config_set_int ("/"PACKAGE"/Mailboxes/quantity", config_id);
L				gnome_config_sync ();
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
