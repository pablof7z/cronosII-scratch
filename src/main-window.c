/*  Cronos II Mail Client
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

#include <glade/glade.h>

#include <libcronosII/error.h>
#include <libcronosII/mime.h>
#include <libcronosII/utils.h>

#include "c2-app.h"
#include "main-window.h"
#include "c2-main-window.h"

#include "widget-mail.h"
#include "widget-index.h"

#include "xpm/read.xpm"
#include "xpm/unread.xpm"
#include "xpm/reply.xpm"
#include "xpm/forward.xpm"

void
on_ctree_changed_mailboxes							(C2Mailbox *mailbox);

static void
on_ctree_tree_select_row							(GtkCTree *ctree, GtkCTreeNode *row, gint column);

static void
on_ctree_tree_unselect_row							(GtkCTree *ctree, GtkCTreeNode *row, gint column);

static void
on_ctree_button_press_event							(GtkWidget *widget, GdkEvent *event);

static void
on_index_select_message								(GtkWidget *index, C2Db *node);

static void
on_preferences_activated							(GtkWidget *widget);

static void
on_about_activated									(GtkWidget *widget);

static void
on_quit												(void);

void
c2_window_new (void)
{
	GtkWidget *window;
	GtkWidget *hpaned;
	GtkWidget *vpaned;
	GtkWidget *ctree;
	GtkWidget *index;
	GtkWidget *mail;
	GtkWidget *button;
	GtkWidget *pixmap;
	GtkWidget *appbar;
	GtkStyle *style;

	pthread_mutex_init (&WMain.index_lock, NULL);
	pthread_mutex_init (&WMain.body_lock, NULL);
	pthread_mutex_init (&WMain.appbar_lock, NULL);
	
	WMain.xml = glade_xml_new (DATADIR "/cronosII/cronosII.glade", "wnd_main");
	WMain.ctree_menu = glade_xml_new (DATADIR "/cronosII/cronosII.glade", "mnu_ctree");

	window = glade_xml_get_widget (WMain.xml, "wnd_main");
	gtk_widget_realize (window);
	gtk_widget_set_usize (GTK_WIDGET (window), c2_app.rc_width, c2_app.rc_height);
	gtk_window_set_policy (GTK_WINDOW (window), TRUE, TRUE, FALSE);
	gtk_signal_connect (GTK_OBJECT (window), "delete_event",
							GTK_SIGNAL_FUNC (on_quit), NULL);

	style = gtk_widget_get_default_style ();
	c2_app.pixmap_read = gdk_pixmap_create_from_xpm_d (window->window, &c2_app.mask_read,
			&style->bg[GTK_STATE_NORMAL],
			read_xpm);
	c2_app.pixmap_unread = gdk_pixmap_create_from_xpm_d (window->window, &c2_app.mask_unread,
			&style->bg[GTK_STATE_NORMAL],
			unread_xpm);
	c2_app.pixmap_reply = gdk_pixmap_create_from_xpm_d (window->window, &c2_app.mask_reply,
			&style->bg[GTK_STATE_NORMAL],
			reply_xpm);
	c2_app.pixmap_forward = gdk_pixmap_create_from_xpm_d (window->window, &c2_app.mask_forward,
			&style->bg[GTK_STATE_NORMAL],
			forward_xpm);

	/* Register the window */
	c2_app_register_window (GTK_WINDOW (window));

	/* Hpaned */
	hpaned = glade_xml_get_widget (WMain.xml, "hpaned");
	gtk_paned_set_position (GTK_PANED (hpaned), c2_app.rc_hpan);

	/* CTree */
	ctree = glade_xml_get_widget (WMain.xml, "ctree");
	gtk_signal_connect (GTK_OBJECT (ctree), "tree_select_row",
								GTK_SIGNAL_FUNC (on_ctree_tree_select_row), NULL);
	gtk_signal_connect (GTK_OBJECT (ctree), "tree_unselect_row",
								GTK_SIGNAL_FUNC (on_ctree_tree_unselect_row), NULL);
	gtk_signal_connect (GTK_OBJECT (ctree), "button_press_event",
      			GTK_SIGNAL_FUNC (on_ctree_button_press_event), NULL);
	if (c2_app.mailbox)
		gtk_signal_connect (GTK_OBJECT (c2_app.mailbox), "changed_mailboxes",
							GTK_SIGNAL_FUNC (on_ctree_changed_mailboxes), NULL);

	/* Vpaned */
	vpaned = glade_xml_get_widget (WMain.xml, "vpaned");
	gtk_paned_set_position (GTK_PANED (vpaned), c2_app.rc_vpan);

	appbar = glade_xml_get_widget (WMain.xml, "appbar");

	/* Index */
	index = glade_xml_get_widget (WMain.xml, "index");
	gtk_signal_connect (GTK_OBJECT (index), "select_message",
							GTK_SIGNAL_FUNC (on_index_select_message), NULL);

	/* Mail */
	mail = glade_xml_get_widget (WMain.xml, "mail");
	c2_mail_install_hints (C2_MAIL (mail), appbar);

	/* Button */
	button = glade_xml_get_widget (WMain.xml, "appbar_button");
	pixmap = gnome_stock_pixmap_widget_at_size (window, GNOME_STOCK_PIXMAP_UP, 10, 14);
	gtk_container_add (GTK_CONTAINER (button), pixmap);
	gtk_widget_show (pixmap);

	/* Connect all signals, menues, toolbar, buttons, etc. */
	gtk_signal_connect_object (GTK_OBJECT (glade_xml_get_widget (WMain.xml, "file_exit")), "activate",
							GTK_SIGNAL_FUNC (gtk_main_quit), NULL);
	gtk_signal_connect_object (GTK_OBJECT(glade_xml_get_widget(WMain.xml,"settings_preferences")), "activate",
							GTK_SIGNAL_FUNC (on_preferences_activated), NULL);
	gtk_signal_connect_object (GTK_OBJECT (glade_xml_get_widget (WMain.xml, "help_about")), "activate",
							GTK_SIGNAL_FUNC (on_about_activated), NULL);
	gtk_signal_connect_object (GTK_OBJECT (glade_xml_get_widget (WMain.ctree_menu, "new_mailbox")), "activate",
							GTK_SIGNAL_FUNC (on_new_mailbox_dlg), NULL);
	gtk_signal_connect_object (GTK_OBJECT (glade_xml_get_widget (WMain.ctree_menu, "properties")), "activate",
							GTK_SIGNAL_FUNC (on_properties_mailbox_dlg), NULL);

	gtk_widget_show (window);
}

static void
pthread_ctree_tree_select_row (C2Mailbox *mbox)
{
	GtkWidget *index = glade_xml_get_widget (WMain.xml, "index");

	c2_return_if_fail (mbox, C2EDATA);
	if (pthread_mutex_trylock (&WMain.index_lock))
		return;

	if (!mbox->db)
	{
		gdk_threads_enter ();
		c2_app_start_activity (GTK_WIDGET (gnome_appbar_get_progress (GNOME_APPBAR (
										glade_xml_get_widget (WMain.xml, "appbar")))));
		gdk_threads_leave ();
		/* Load the database */
		if (!(mbox->db = c2_db_new (mbox)))
		{
			gchar *string;

			string = g_strdup_printf (_("Unable to load db: %s"), c2_error_get (c2_errno));
			gdk_threads_enter ();
			c2_app_stop_activity ();
			c2_app_report (string, C2_REPORT_ERROR);
			gdk_threads_leave ();
			g_free (string);
			WMain.selected_mbox = mbox;
			pthread_mutex_unlock (&WMain.index_lock);
			return;
		}
	}
	
	WMain.selected_mbox = mbox;

	gdk_threads_enter ();
	c2_index_remove_mailbox (C2_INDEX (index));
	c2_app_stop_activity ();
	c2_index_add_mailbox (C2_INDEX (index), mbox);
	gtk_widget_queue_draw (index);
	gdk_threads_leave ();
	pthread_mutex_unlock (&WMain.index_lock);
}

static void
on_ctree_tree_select_row (GtkCTree *ctree, GtkCTreeNode *row, gint column)
{
	C2Mailbox *mbox;
	pthread_t thread;
	
	mbox = gtk_ctree_node_get_row_data (ctree, row);
	if (!mbox)
		c2_app_report (_("Internal error in CTree listing."), C2_REPORT_ERROR);
	else
		pthread_create (&thread, NULL, C2_PTHREAD_FUNC (pthread_ctree_tree_select_row), mbox);
}

static void
on_ctree_tree_unselect_row (GtkCTree *ctree, GtkCTreeNode *row, gint column)
{
	if (!pthread_mutex_trylock (&WMain.index_lock))
	{
		GtkWidget *index = glade_xml_get_widget (WMain.xml, "index");
		
		c2_index_remove_mailbox (C2_INDEX (index));
		WMain.selected_mbox = NULL;
		pthread_mutex_unlock (&WMain.index_lock);
	}
}

static void
on_ctree_button_press_event (GtkWidget *widget, GdkEvent *event)
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
			node = gtk_ctree_node_nth (GTK_CTREE (widget), row);
			gtk_ctree_select (GTK_CTREE (widget), node);
		} else
			gtk_clist_unselect_all (GTK_CLIST (widget));

		c2_main_window_set_sensitivity ();
		gnome_popup_menu_do_popup (glade_xml_get_widget (WMain.ctree_menu, "mnu_ctree"),
									NULL, NULL, e, NULL);
	}
}

void
on_ctree_changed_mailboxes (C2Mailbox *mailbox)
{
	GtkWidget *ctree = glade_xml_get_widget (WMain.xml, "ctree");
	GtkWidget *window = glade_xml_get_widget (WMain.xml, "window");

	c2_app.mailbox = c2_mailbox_get_head ();
	c2_mailbox_tree_fill (c2_app.mailbox, NULL, ctree, window);
}

static void
on_index_select_message (GtkWidget *index, C2Db *node)
{
	c2_db_message_load (node);
	c2_mail_set_message (C2_MAIL (glade_xml_get_widget (WMain.xml, "mail")), C2_MESSAGE (node));
}

static void
on_preferences_activated (GtkWidget *widget)
{
	c2_preferences_new ();
}

static void
on_about_activated (GtkWidget *widget)
{
	GladeXML *xml = glade_xml_new (DATADIR "/cronosII/cronosII.glade", "dlg_about");
	GtkWidget *about = glade_xml_get_widget (xml, "dlg_about");
}

static void
on_quit (void)
{
	GtkWidget *window = glade_xml_get_widget (WMain.xml, "wnd_main");
	GList *l;
	
	for (l = c2_app.open_windows; l != NULL; l = l->next)
	{
		if (!GTK_IS_WINDOW (l->data) || (GtkWindow*)l->data == GTK_WINDOW (window))
			continue;
		gtk_signal_emit_by_name (GTK_OBJECT ((GtkWindow*)l->data), "delete_event");
	}
	gtk_widget_destroy (window);
	gtk_main_quit ();
}
