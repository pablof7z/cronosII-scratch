#include <config.h>
#include <gnome.h>

#include "main-window.h"
#include "c2-app.h"

#include "xpm/read.xpm"
#include "xpm/unread.xpm"
#include "xpm/reply.xpm"
#include "xpm/forward.xpm"

void
c2_window_new (void) {
	GtkWidget *scroll;
	GtkWidget *pixmap;
	GtkWidget *vbox, *table;
	GtkStyle *style, *style2;

	pthread_mutex_init (&WMain.appbar_lock, NULL);

	/* Window */
	WMain.window = gnome_app_new (PACKAGE, "Cronos II");
	gtk_widget_realize (WMain.window);
	gtk_widget_set_usize (GTK_WIDGET (WMain.window), c2_app.wm_width, c2_app.wm_height);
	gtk_window_set_policy (GTK_WINDOW (WMain.window), TRUE, TRUE, FALSE);

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

	/* Appbar */
	WMain.appbar = gnome_appbar_new (TRUE, TRUE, GNOME_PREFERENCES_USER);
	gnome_app_set_statusbar(GNOME_APP(WMain.window), GTK_WIDGET(WMain.appbar));
	gnome_app_install_appbar_menu_hints(GNOME_APPBAR(WMain.appbar),
											NULL);

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
	c2_mailbox_tree_fill (c2_app.mailboxes, NULL, WMain.ctree, WMain.window);

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

	/* CList */
	WMain.clist = gtk_clist_new (8);
	gtk_container_add (GTK_CONTAINER (scroll), WMain.clist);
	gtk_widget_show (WMain.clist);
	gtk_clist_set_row_height (GTK_CLIST (WMain.clist), 16);
	pixmap = gtk_pixmap_new (c2_app.pixmap_unread, c2_app.mask_unread);
	gtk_clist_set_column_widget (GTK_CLIST (WMain.clist), 0, pixmap);
	gtk_clist_set_column_title (GTK_CLIST (WMain.clist), 3, _("Subject"));
	gtk_clist_set_column_title (GTK_CLIST (WMain.clist), 4, _("From"));
	gtk_clist_set_column_title (GTK_CLIST (WMain.clist), 5, _("Date"));
	gtk_clist_set_column_title (GTK_CLIST (WMain.clist), 6, _("Account"));
	gtk_clist_set_column_width (GTK_CLIST (WMain.clist), 0, c2_app.wm_clist[0]);
	gtk_clist_set_column_width (GTK_CLIST (WMain.clist), 1, c2_app.wm_clist[1]);
	gtk_clist_set_column_width (GTK_CLIST (WMain.clist), 2, c2_app.wm_clist[2]);
	gtk_clist_set_column_width (GTK_CLIST (WMain.clist), 3, c2_app.wm_clist[3]);
	gtk_clist_set_column_width (GTK_CLIST (WMain.clist), 4, c2_app.wm_clist[4]);
	gtk_clist_set_column_width (GTK_CLIST (WMain.clist), 5, c2_app.wm_clist[5]);
	gtk_clist_set_column_width (GTK_CLIST (WMain.clist), 6, c2_app.wm_clist[6]);
	gtk_clist_set_column_width (GTK_CLIST (WMain.clist), 7, c2_app.wm_clist[7]);
	gtk_clist_set_column_visibility (GTK_CLIST (WMain.clist), 7, FALSE);
	gtk_clist_set_column_visibility (GTK_CLIST (WMain.clist), 1, FALSE);
	gtk_clist_set_column_visibility (GTK_CLIST (WMain.clist), 2, FALSE);
	gtk_clist_column_titles_show (GTK_CLIST (WMain.clist));
	gtk_clist_set_selection_mode (GTK_CLIST (WMain.clist), GTK_SELECTION_EXTENDED);

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

	/* From */
	WMain.header_titles[C2_HEADER_TITLES_FROM][0] = gtk_label_new (_("From:"));
	gtk_table_attach (GTK_TABLE (table), WMain.header_titles[C2_HEADER_TITLES_FROM][0], 0, 1, 0, 1,
					(GtkAttachOptions) (GTK_FILL),
					(GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (WMain.header_titles[C2_HEADER_TITLES_FROM][0]), 1, 0.5);
	style = gtk_widget_get_style (WMain.header_titles[C2_HEADER_TITLES_FROM][0]);
	style2 = gtk_style_copy (style);
	style2->font = gdk_font_load ("-adobe-helvetica-bold-r-normal-*-*-120-*-*-p-*-iso8859-1");
	gtk_widget_set_style (WMain.header_titles[C2_HEADER_TITLES_FROM][0], style2);
	
	gtk_widget_show (WMain.window);
}
