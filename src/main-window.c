/*  Cronos II
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

#include <libmodules/error.h>
#include <libmodules/utils.h>

#include "c2-app.h"
#include "main-window.h"
#include "c2-main-window.h"
#include "widget-index.h"

#include "xpm/read.xpm"
#include "xpm/unread.xpm"
#include "xpm/reply.xpm"
#include "xpm/forward.xpm"

#define TITLES(a, b, c, d, e, f, g, h, i, j, k) \
	WMain.header_titles[a][0] = gtk_label_new (_(b)); \
	gtk_table_attach (GTK_TABLE (table), WMain.header_titles[a][0], d, e, f, g, \
					(GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0); \
	gtk_misc_set_alignment (GTK_MISC (WMain.header_titles[a][0]), 1, 0.5); \
	style = gtk_widget_get_style (WMain.header_titles[a][0]); \
	style2 = gtk_style_copy (style); \
	style2->font = gdk_font_load ("-adobe-helvetica-bold-r-normal-*-*-120-*-*-p-*-iso8859-1"); \
	gtk_widget_set_style (WMain.header_titles[a][0], style2); \
\
	WMain.header_titles[a][1] = gtk_label_new (""); \
	gtk_table_attach (GTK_TABLE (table), WMain.header_titles[a][1], h, i, j, k, \
			(GtkAttachOptions) (GTK_EXPAND) | (GTK_FILL), (GtkAttachOptions) (0), 0, 0); \
	gtk_misc_set_alignment (GTK_MISC (WMain.header_titles[a][1]), 7.45058e-09, 0.5); \
	if (c2_app.showable_headers[C2_SHOWABLE_HEADERS_PREVIEW] & c) \
	{ \
		gtk_widget_show (WMain.header_titles[a][0]); \
		gtk_widget_show (WMain.header_titles[a][1]); \
	} \

static void
on_ctree_tree_select_row							(GtkCTree *ctree, GtkCTreeNode *row, gint column);

static void
on_ctree_tree_unselect_row							(GtkCTree *ctree, GtkCTreeNode *row, gint column);

static void
on_quit												(void);

static GnomeUIInfo menu_ctree[] =
{
	GNOMEUIINFO_SEPARATOR,
	{
		GNOME_APP_UI_ITEM, N_("_New Mailbox"),
		N_("Create a new mailbox"),
		NULL, NULL, NULL,
		GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_HOME
	},
	GNOMEUIINFO_SEPARATOR,
	{
		GNOME_APP_UI_ITEM, N_("_Delete"),
		N_("Delete selected mailbox"),
		NULL, NULL, NULL,
		GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_TRASH_FULL,
	},
	GNOMEUIINFO_MENU_PROPERTIES_ITEM (NULL, NULL),
	GNOMEUIINFO_SEPARATOR,
	GNOMEUIINFO_END
};

void
c2_window_new (void)
{
	GtkWidget *scroll;
	GtkWidget *pixmap;
	GtkWidget *vbox, *table, *hbox, *viewport, *vbox2, *button;
	GtkStyle *style, *style2;

	pthread_mutex_init (&WMain.index_lock, NULL);
	pthread_mutex_init (&WMain.text_lock, NULL);
	pthread_mutex_init (&WMain.appbar_lock, NULL);

	/* Window */
	WMain.window = gnome_app_new (PACKAGE, "Cronos II");
	gtk_widget_realize (WMain.window);
	gtk_widget_set_usize (GTK_WIDGET (WMain.window), c2_app.wm_width, c2_app.wm_height);
	gtk_window_set_policy (GTK_WINDOW (WMain.window), TRUE, TRUE, FALSE);
	gtk_signal_connect (GTK_OBJECT (WMain.window), "delete_event",
							GTK_SIGNAL_FUNC (on_quit), NULL);

	style = gtk_widget_get_default_style ();
	c2_app.pixmap_read = gdk_pixmap_create_from_xpm_d (WMain.window->window, &c2_app.mask_read,
			&style->bg[GTK_STATE_NORMAL],
			read_xpm);
	c2_app.pixmap_unread = gdk_pixmap_create_from_xpm_d (WMain.window->window, &c2_app.mask_unread,
			&style->bg[GTK_STATE_NORMAL],
			unread_xpm);
	c2_app.pixmap_reply = gdk_pixmap_create_from_xpm_d (WMain.window->window, &c2_app.mask_reply,
			&style->bg[GTK_STATE_NORMAL],
			reply_xpm);
	c2_app.pixmap_forward = gdk_pixmap_create_from_xpm_d (WMain.window->window, &c2_app.mask_forward,
			&style->bg[GTK_STATE_NORMAL],
			forward_xpm);

	/* Menubar */

	/* Register the window */
	c2_app_register_window (GTK_WINDOW (WMain.window));

	/* Toolbar */

	/* Hpaned */
	WMain.hpaned = gtk_hpaned_new ();
	gnome_app_set_contents (GNOME_APP (WMain.window), WMain.hpaned);
	gtk_widget_show (WMain.hpaned);
	gtk_paned_set_position (GTK_PANED (WMain.hpaned), c2_app.wm_hpan);

	/* Mbox Scroll */
	scroll = gtk_scrolled_window_new (NULL, NULL);
	gtk_paned_add1 (GTK_PANED (WMain.hpaned), scroll);
	gtk_widget_show (scroll);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	/* CTree */
	WMain.ctree = gtk_ctree_new (1, 0);
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scroll), WMain.ctree);
	gtk_widget_show (WMain.ctree);
	gtk_clist_set_column_title (GTK_CLIST (WMain.ctree), 0, _("Mailboxes"));
	gtk_clist_set_column_justification (GTK_CLIST(WMain.ctree), 0, GTK_JUSTIFY_CENTER);
	gtk_clist_column_titles_passive (GTK_CLIST (WMain.ctree));
	gtk_clist_column_titles_show (GTK_CLIST (WMain.ctree));
	gnome_popup_menu_attach (gnome_popup_menu_new (menu_ctree), WMain.ctree, NULL);
	c2_mailbox_tree_fill (c2_app.mailboxes, NULL, WMain.ctree, WMain.window);
	gtk_signal_connect (GTK_OBJECT (WMain.ctree), "tree_select_row",
								GTK_SIGNAL_FUNC (on_ctree_tree_select_row), NULL);
	gtk_signal_connect (GTK_OBJECT (WMain.ctree), "tree_unselect_row",
								GTK_SIGNAL_FUNC (on_ctree_tree_unselect_row), NULL);

	/* Vpaned */
	WMain.vpaned = gtk_vpaned_new ();
	gtk_paned_add2 (GTK_PANED (WMain.hpaned), WMain.vpaned);
	gtk_widget_show (WMain.vpaned);
	gtk_paned_set_position (GTK_PANED(WMain.vpaned), c2_app.wm_vpan);

	scroll = gtk_scrolled_window_new (NULL, NULL);
	gtk_paned_add1 (GTK_PANED (WMain.vpaned), scroll);
	gtk_widget_show (scroll);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);

	/* Index */
	WMain.index = c2_index_new ();
	gtk_container_add (GTK_CONTAINER (scroll), WMain.index);
	gtk_widget_show (WMain.index);

	/* Vbox */
	vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
	gtk_paned_add2 (GTK_PANED (WMain.vpaned), vbox);
	gtk_widget_show (vbox);

	/* Table */
	table = gtk_table_new (6, 4, FALSE);
	gtk_box_pack_start (GTK_BOX (vbox), table, FALSE, FALSE, 0);
	gtk_widget_show (table);
	gtk_table_set_col_spacings (GTK_TABLE (table), 2);
	WMain.header_table = table;

	TITLES (C2_HEADER_TITLES_FROM, _("From:"), C2_SHOWABLE_HEADER_FIELD_FROM, 0, 1, 0, 1, 1, 2, 0, 1);
	TITLES (C2_HEADER_TITLES_ACCOUNT, _("Account:"), C2_SHOWABLE_HEADER_FIELD_ACCOUNT,2, 3, 0, 1, 3, 4, 0, 1);
	TITLES (C2_HEADER_TITLES_TO, _("To:"), C2_SHOWABLE_HEADER_FIELD_TO, 0, 1, 1, 2, 1, 2, 1, 2);
	TITLES (C2_HEADER_TITLES_CC, _("CC:"), C2_SHOWABLE_HEADER_FIELD_CC, 0, 1, 2, 3, 1, 2, 2, 3);
	TITLES (C2_HEADER_TITLES_BCC, _("BCC:"), C2_SHOWABLE_HEADER_FIELD_BCC, 0, 1, 3, 4, 1, 2, 3, 4);
	TITLES (C2_HEADER_TITLES_DATE, _("Date:"), C2_SHOWABLE_HEADER_FIELD_DATE, 0, 1, 4, 5, 1, 2, 4, 5);
	TITLES (C2_HEADER_TITLES_PRIORITY, _("Priority:"), C2_SHOWABLE_HEADER_FIELD_PRIORITY,2,3,4,5, 3, 4, 4, 5);
	TITLES (C2_HEADER_TITLES_SUBJECT, _("Subject:"), C2_SHOWABLE_HEADER_FIELD_SUBJECT,0, 1, 5, 6, 1, 2, 5, 6);
	
	/* Hbox */
	hbox = gtk_hbox_new (FALSE, 2);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);
	gtk_widget_show (hbox);
	
	/* Text */
	WMain.text = gtk_text_new (NULL, NULL);
	gtk_box_pack_start (GTK_BOX (hbox), WMain.text, TRUE, TRUE, 0);
	gtk_widget_show (WMain.text);
	
	/* Vbox */
	vbox2 = gtk_vbox_new (FALSE, 2);
	gtk_box_pack_end (GTK_BOX (hbox), vbox2, FALSE, TRUE, 0);
	gtk_widget_show (vbox2);
	
	/* VScroll */
	scroll = gtk_vscrollbar_new (GTK_TEXT (WMain.text)->vadj);
	gtk_box_pack_start (GTK_BOX (vbox2), scroll, TRUE, TRUE, 0);
	gtk_widget_show (scroll);
	
	/* Mime Left */
	button = gtk_button_new ();
	pixmap = gnome_stock_pixmap_widget_at_size (WMain.window, GNOME_STOCK_PIXMAP_BACK, 10, 14);
	gtk_container_add (GTK_CONTAINER (button), pixmap);
	gtk_box_pack_end (GTK_BOX (vbox2), button, FALSE, FALSE, 0);
	gtk_widget_show (pixmap);
	gtk_widget_show (button);
	WMain.mime_left = button;
	gtk_tooltips_set_tip (c2_app.tooltips, button, _("Show the attachment list of this message"), NULL);
	
	/* Mime Right */
	button = gtk_button_new ();
	pixmap = gnome_stock_pixmap_widget_at_size (WMain.window, GNOME_STOCK_PIXMAP_FORWARD, 10, 14);
	gtk_container_add (GTK_CONTAINER (button), pixmap);
	gtk_box_pack_end (GTK_BOX (vbox2), button, FALSE, FALSE, 0);
	gtk_widget_show (pixmap);
	WMain.mime_right = button;
	gtk_tooltips_set_tip (c2_app.tooltips, button, _("Hide the attachment list of this message"), NULL);
	
	/* Hbox */
	WMain.mime_scroll = gtk_hbox_new (FALSE, 2);
	gtk_box_pack_start (GTK_BOX (vbox), WMain.mime_scroll, FALSE, FALSE, 0);
	
	/* Viewport */
	viewport = gtk_viewport_new (NULL, NULL);
	gtk_box_pack_start (GTK_BOX (WMain.mime_scroll), viewport, TRUE, TRUE, 0);
	gtk_widget_set_usize (viewport, -1, 70);
	gtk_widget_show (viewport);
	
	/* Icon List */
	WMain.icon_list = gnome_icon_list_new (70, NULL, 0);
	gtk_container_add (GTK_CONTAINER (viewport), WMain.icon_list);
	gtk_widget_show (WMain.icon_list);
	gnome_icon_list_set_selection_mode (GNOME_ICON_LIST (WMain.icon_list), GTK_SELECTION_MULTIPLE);
	
	/* Vbox */
	vbox2 = gtk_vbox_new (FALSE, 2);
	gtk_box_pack_end (GTK_BOX (WMain.mime_scroll), vbox2, FALSE, TRUE, 0);
	gtk_widget_show (vbox2);
	
	/* VScroll */
	scroll = gtk_vscrollbar_new (GNOME_ICON_LIST (WMain.icon_list)->adj);
	gtk_box_pack_start (GTK_BOX (vbox2), scroll, TRUE, TRUE, 0);
	gtk_widget_show (scroll);
	
	/* Button */
	button = gtk_toggle_button_new ();
	pixmap = gnome_stock_pixmap_widget_at_size (WMain.window, GNOME_STOCK_PIXMAP_ATTACH, 10, 14);
	gtk_container_add (GTK_CONTAINER (button), pixmap);
	gtk_box_pack_end (GTK_BOX (vbox2), button, FALSE, FALSE, 0);
	gtk_widget_show (pixmap);
	gtk_widget_show (button);
	gtk_tooltips_set_tip (c2_app.tooltips, button, _("Set/Unset the attachment list as sticky"), NULL);
	if (c2_app.mime_window == C2_MIME_WINDOW_STICKY)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
	
	/* Status HBox */
	hbox = gtk_hbox_new (FALSE, 1);
	gtk_box_pack_start (GTK_BOX (GNOME_APP (WMain.window)->vbox), hbox, FALSE, TRUE, 1);
	gtk_widget_show (hbox);
	
	/* Appbar */
	WMain.appbar = gnome_appbar_new (TRUE, TRUE, GNOME_PREFERENCES_NEVER);
	gtk_box_pack_start (GTK_BOX (hbox), WMain.appbar, TRUE, TRUE, 0);
	gtk_widget_show (WMain.appbar);
	
	/* Button */
	button = gtk_button_new ();
	pixmap = gnome_stock_pixmap_widget_at_size (WMain.window, GNOME_STOCK_PIXMAP_UP, 10, 14);
	gtk_container_add (GTK_CONTAINER (button), pixmap);
	gtk_box_pack_end (GTK_BOX (hbox), button, FALSE, FALSE, 0);
	gtk_widget_show (pixmap);
	gtk_widget_show (button);
	gtk_tooltips_set_tip (c2_app.tooltips, button, _("Show the Checking Window."), NULL);
	
	gtk_widget_show (WMain.window);
}

static void
pthread_ctree_tree_select_row (C2Mailbox *mbox)
{
	c2_return_if_fail (mbox, C2EDATA);
	if (pthread_mutex_trylock (&WMain.index_lock))
		return;

	if (!mbox->db)
	{
		/* Load the database */
		if (!(mbox->db = c2_db_load (mbox->name, C2_METHOD_CRONOSII)))
		{
			gchar *string;

			string = g_strdup_printf (_("Couldn't load db: %s"), c2_error_get (c2_errno));
			gdk_threads_enter ();
			c2_app_report (string, C2_REPORT_ERROR);
			gdk_threads_leave ();
			g_free (string);
			return;
		}
	}
	
	gdk_threads_enter ();
	c2_index_add_mailbox (C2_INDEX (WMain.index), mbox);
	gtk_widget_queue_draw (WMain.index);
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
		pthread_create (&thread, NULL, PTHREAD_FUNC (pthread_ctree_tree_select_row), mbox);
}

static void
on_ctree_tree_unselect_row (GtkCTree *ctree, GtkCTreeNode *row, gint column)
{
	if (!pthread_mutex_trylock (&WMain.index_lock))
	{
		c2_index_remove_mailbox (C2_INDEX (WMain.index));
		pthread_mutex_unlock (&WMain.index_lock);
	}
}

static void
on_quit (void)
{
	GList *l;
	for (l = c2_app.open_windows; l != NULL; l = l->next)
	{
		if (!GTK_IS_WINDOW (l->data) || (GtkWindow*)l->data == GTK_WINDOW (WMain.window))
			continue;
		gtk_signal_emit_by_name (GTK_OBJECT ((GtkWindow*)l->data), "delete_event");
	}
	gtk_widget_destroy (WMain.window);
	gtk_main_quit ();
}
