#include <config.h>
#include <gnome.h>

#include "main-window.h"
#include "c2-app.h"

#include "xpm/sendqueue.xpm"

static void
main_window_menu_clist (void);

static void
main_window_menu_attach (void);

void main_window_menubar (void);

void main_window_toolbar (void);

static void
main_window_install_menu_hints (void);

void
c2_window_main_new (void) {
	GtkWidget *vbox, *vbox2;
	GtkWidget *hbox;
	GtkWidget *pixmap, *attach_pixmap;
	GtkWidget *scroll;
	GtkWidget *table;
	GtkWidget *button;
	GtkWidget *viewport;
	GtkStyle *style;
	GtkStyle *style2;
	
	WMain.selected_mbox = NULL;
	WMain.selected_row = -1;
	pthread_mutex_init (&WMain.appbar_lock, NULL);
	
	/* Window */
	WMain.window = gnome_app_new (PACKAGE, "Cronos II");
	gtk_widget_realize (WMain.window);
	gtk_widget_set_usize (GTK_WIDGET (WMain.window), c2_app.wm_width, c2_app.wm_height);
	gtk_window_set_policy (GTK_WINDOW (WMain.window), TRUE, TRUE, FALSE);
	
	main_window_menubar ();
//	main_window_make_account_menu ();
	main_window_toolbar ();
	
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
	gtk_ctree_set_expander_style (GTK_CTREE (WMain.ctree), GTK_CTREE_EXPANDER_CIRCULAR);
	gtk_clist_set_column_title (GTK_CLIST (WMain.ctree), 0, _("Mailboxes"));
	gtk_clist_set_column_justification (GTK_CLIST(WMain.ctree), 0, GTK_JUSTIFY_CENTER);
	gtk_clist_column_titles_passive (GTK_CLIST (WMain.ctree));
	gtk_clist_column_titles_show (GTK_CLIST (WMain.ctree));
	
	/** Vpaned **/
	WMain.vpaned = gtk_vpaned_new ();
	gtk_paned_add2 (GTK_PANED (WMain.hpaned), WMain.vpaned);
	gtk_widget_show (WMain.vpaned);
	gtk_paned_set_position (GTK_PANED(WMain.vpaned), c2_app.wm_vpan);
	
	scroll = gtk_scrolled_window_new (NULL, NULL);
	gtk_paned_add1 (GTK_PANED (WMain.vpaned), scroll);
	gtk_widget_show (scroll);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);

  /** CList **/
  WMain.clist = gtk_clist_new (8);
  gtk_container_add (GTK_CONTAINER (scroll), WMain.clist);
  gtk_widget_show (WMain.clist);
  gtk_clist_set_row_height (GTK_CLIST (WMain.clist), 16);
  
/*  pixmap = gtk_pixmap_new (pixmap_unread, mask_unread);
  gtk_clist_set_column_widget (GTK_CLIST (WMain.clist), 0, pixmap);
  pixmap = gtk_pixmap_new (pixmap_mark, mask_mark);
  gtk_clist_set_column_widget (GTK_CLIST (WMain.clist), 1, pixmap);
  gtk_clist_set_column_widget (GTK_CLIST (WMain.clist), 2, attach_pixmap);*/
  gtk_clist_set_column_title (GTK_CLIST (WMain.clist), 3, _("Subject"));
  gtk_clist_set_column_title (GTK_CLIST (WMain.clist), 4, _("From"));
  gtk_clist_set_column_title (GTK_CLIST (WMain.clist), 5, _("Date"));
  gtk_clist_set_column_title (GTK_CLIST (WMain.clist), 6, _("Account"));
  gtk_clist_set_column_title (GTK_CLIST (WMain.clist), 7, "#");
  gtk_clist_set_column_width (GTK_CLIST (WMain.clist), 0, c2_app.wm_clist[0]);
  gtk_clist_set_column_width (GTK_CLIST (WMain.clist), 3, c2_app.wm_clist[1]);
  gtk_clist_set_column_width (GTK_CLIST (WMain.clist), 4, c2_app.wm_clist[2]);
  gtk_clist_set_column_width (GTK_CLIST (WMain.clist), 5, c2_app.wm_clist[3]);
  gtk_clist_set_column_width (GTK_CLIST (WMain.clist), 6, c2_app.wm_clist[4]);
  gtk_clist_set_column_width (GTK_CLIST (WMain.clist), 7, c2_app.wm_clist[5]);
  gtk_clist_set_column_visibility (GTK_CLIST (WMain.clist), 7, FALSE);
  gtk_clist_set_column_visibility (GTK_CLIST (WMain.clist), 1, FALSE);
  gtk_clist_set_column_visibility (GTK_CLIST (WMain.clist), 2, FALSE);
  gtk_clist_column_titles_show (GTK_CLIST (WMain.clist));
  gtk_clist_set_selection_mode (GTK_CLIST (WMain.clist), GTK_SELECTION_EXTENDED);
  main_window_menu_clist ();

  /** Vbox **/
  vbox = gtk_vbox_new (FALSE, 2);
  gtk_paned_add2 (GTK_PANED (WMain.vpaned), vbox);
  gtk_widget_show (vbox);

  /** Table **/
  table = gtk_table_new (6, 4, FALSE);
  gtk_box_pack_start (GTK_BOX (vbox), table, FALSE, FALSE, 0);
  gtk_widget_show (table);
  gtk_table_set_col_spacings (GTK_TABLE (table), 2);
  WMain.header_table = table;

  /** From **/
  WMain.header_titles[C2_HEADER_TITLES_FROM][0] = gtk_label_new (_("From:"));
  gtk_table_attach (GTK_TABLE (table), WMain.header_titles[C2_HEADER_TITLES_FROM][0], 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (WMain.header_titles[C2_HEADER_TITLES_FROM][0]), 1, 0.5);
  style = gtk_widget_get_style (WMain.header_titles[C2_HEADER_TITLES_FROM][0]);
  style2 = gtk_style_copy (style);
  style2->font = gdk_font_load ("-adobe-helvetica-bold-r-normal-*-*-120-*-*-p-*-iso8859-1");
  gtk_widget_set_style (WMain.header_titles[C2_HEADER_TITLES_FROM][0], style2);
  
  WMain.header_titles[C2_HEADER_TITLES_FROM][1] = gtk_label_new ("");
  gtk_table_attach (GTK_TABLE (table), WMain.header_titles[C2_HEADER_TITLES_FROM][1], 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND) | (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (WMain.header_titles[C2_HEADER_TITLES_FROM][1]), 7.45058e-09, 0.5);
  if (c2_app.showable_headers[C2_SHOWABLE_HEADERS_PREVIEW] & 1 << 2) {
    gtk_widget_show (WMain.header_titles[C2_HEADER_TITLES_FROM][0]);
    gtk_widget_show (WMain.header_titles[C2_HEADER_TITLES_FROM][1]);
  }

  /** To **/
  WMain.header_titles[C2_HEADER_TITLES_TO][0] = gtk_label_new (_("To:"));
  gtk_table_attach (GTK_TABLE (table), WMain.header_titles[C2_HEADER_TITLES_TO][0], 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (WMain.header_titles[C2_HEADER_TITLES_TO][0]), 1, 0.5);
  gtk_widget_set_style (WMain.header_titles[C2_HEADER_TITLES_TO][0], style2);

  WMain.header_titles[C2_HEADER_TITLES_TO][1] = gtk_label_new ("");
  gtk_table_attach (GTK_TABLE (table), WMain.header_titles[C2_HEADER_TITLES_TO][1], 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND) | (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (WMain.header_titles[C2_HEADER_TITLES_TO][1]), 7.45058e-09, 0.5);
  if (c2_app.showable_headers[C2_SHOWABLE_HEADERS_PREVIEW] & 1 << 0) {
    gtk_widget_show (WMain.header_titles[C2_HEADER_TITLES_TO][0]);
    gtk_widget_show (WMain.header_titles[C2_HEADER_TITLES_TO][1]);
  }

  /** CC **/
  WMain.header_titles[C2_HEADER_TITLES_CC][0] = gtk_label_new (_("CC:"));
  gtk_table_attach (GTK_TABLE (table), WMain.header_titles[C2_HEADER_TITLES_CC][0], 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (WMain.header_titles[C2_HEADER_TITLES_CC][0]), 1, 0.5);
  gtk_widget_set_style (WMain.header_titles[C2_HEADER_TITLES_CC][0], style2);

  WMain.header_titles[C2_HEADER_TITLES_CC][1] = gtk_label_new ("");
  gtk_table_attach (GTK_TABLE (table), WMain.header_titles[C2_HEADER_TITLES_CC][1], 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND) | (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (WMain.header_titles[C2_HEADER_TITLES_CC][1]), 7.45058e-09, 0.5);
  if (c2_app.showable_headers[C2_SHOWABLE_HEADERS_PREVIEW] & 1 << 5) {
    gtk_widget_show (WMain.header_titles[C2_HEADER_TITLES_CC][0]);
    gtk_widget_show (WMain.header_titles[C2_HEADER_TITLES_CC][1]);
  }

  /** BCC **/
  WMain.header_titles[C2_HEADER_TITLES_BCC][0] = gtk_label_new (_("BCC:"));
  gtk_table_attach (GTK_TABLE (table), WMain.header_titles[C2_HEADER_TITLES_BCC][0], 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (WMain.header_titles[C2_HEADER_TITLES_BCC][0]), 1, 0.5);
  gtk_widget_set_style (WMain.header_titles[C2_HEADER_TITLES_BCC][0], style2);

  WMain.header_titles[C2_HEADER_TITLES_BCC][1] = gtk_label_new ("");
  gtk_table_attach (GTK_TABLE (table), WMain.header_titles[C2_HEADER_TITLES_BCC][1], 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_EXPAND) | (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (WMain.header_titles[C2_HEADER_TITLES_BCC][1]), 7.45058e-09, 0.5);
  if (c2_app.showable_headers[C2_SHOWABLE_HEADERS_PREVIEW] & 1 << 6) {
    gtk_widget_show (WMain.header_titles[C2_HEADER_TITLES_BCC][0]);
    gtk_widget_show (WMain.header_titles[C2_HEADER_TITLES_BCC][1]);
  }

  /** SUBJECT **/
  WMain.header_titles[C2_HEADER_TITLES_SUBJECT][0] = gtk_label_new (_("Subject:"));
  gtk_table_attach (GTK_TABLE (table), WMain.header_titles[C2_HEADER_TITLES_SUBJECT][0], 0, 1, 5, 6,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (WMain.header_titles[C2_HEADER_TITLES_SUBJECT][0]), 1, 0.5);
  gtk_widget_set_style (WMain.header_titles[C2_HEADER_TITLES_SUBJECT][0], style2);

  WMain.header_titles[C2_HEADER_TITLES_SUBJECT][1] = gtk_label_new ("");
  gtk_table_attach (GTK_TABLE (table), WMain.header_titles[C2_HEADER_TITLES_SUBJECT][1], 1, 2, 5, 6,
                    (GtkAttachOptions) (GTK_EXPAND) | (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (WMain.header_titles[C2_HEADER_TITLES_SUBJECT][1]), 7.45058e-09, 0.5);
  if (c2_app.showable_headers[C2_SHOWABLE_HEADERS_PREVIEW] & 1 << 3) {
    gtk_widget_show (WMain.header_titles[C2_HEADER_TITLES_SUBJECT][0]);
    gtk_widget_show (WMain.header_titles[C2_HEADER_TITLES_SUBJECT][1]);
  }

  /** ACCOUNT **/
  WMain.header_titles[C2_HEADER_TITLES_ACCOUNT][0] = gtk_label_new (_("Account:"));
  gtk_table_attach (GTK_TABLE (table), WMain.header_titles[C2_HEADER_TITLES_ACCOUNT][0], 2, 3, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (WMain.header_titles[C2_HEADER_TITLES_ACCOUNT][0]), 1, 0.5);
  gtk_widget_set_style (WMain.header_titles[C2_HEADER_TITLES_ACCOUNT][0], style2);

  WMain.header_titles[C2_HEADER_TITLES_ACCOUNT][1] = gtk_label_new ("");
  gtk_table_attach (GTK_TABLE (table), WMain.header_titles[C2_HEADER_TITLES_ACCOUNT][1], 3, 4, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (WMain.header_titles[C2_HEADER_TITLES_ACCOUNT][1]), 7.45058e-09, 0.5);
  gtk_widget_set_usize (WMain.header_titles[C2_HEADER_TITLES_ACCOUNT][1], 100, -1);
  if (c2_app.showable_headers[C2_SHOWABLE_HEADERS_PREVIEW] & 1 << 4) {
    gtk_widget_show (WMain.header_titles[C2_HEADER_TITLES_ACCOUNT][0]);
    gtk_widget_show (WMain.header_titles[C2_HEADER_TITLES_ACCOUNT][1]);
  }

  /** DATE **/
  WMain.header_titles[C2_HEADER_TITLES_DATE][0] = gtk_label_new (_("Date:"));
  gtk_table_attach (GTK_TABLE (table), WMain.header_titles[C2_HEADER_TITLES_DATE][0], 0, 1, 4, 5,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (WMain.header_titles[C2_HEADER_TITLES_DATE][0]), 1, 0.5);
  gtk_widget_set_style (WMain.header_titles[C2_HEADER_TITLES_DATE][0], style2);

  WMain.header_titles[C2_HEADER_TITLES_DATE][1] = gtk_label_new ("");
  gtk_table_attach (GTK_TABLE (table), WMain.header_titles[C2_HEADER_TITLES_DATE][1], 1, 2, 4, 5,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (WMain.header_titles[C2_HEADER_TITLES_DATE][1]), 7.45058e-09, 0.5);
  if (c2_app.showable_headers[C2_SHOWABLE_HEADERS_PREVIEW] & 1 << 1) {
    gtk_widget_show (WMain.header_titles[C2_HEADER_TITLES_DATE][0]);
    gtk_widget_show (WMain.header_titles[C2_HEADER_TITLES_DATE][1]);
  }

  /** PRIORITY **/
  WMain.header_titles[C2_HEADER_TITLES_PRIORITY][0] = gtk_label_new (_("Priority:"));
  gtk_table_attach (GTK_TABLE (table), WMain.header_titles[C2_HEADER_TITLES_PRIORITY][0], 2, 3, 4, 5,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (WMain.header_titles[C2_HEADER_TITLES_PRIORITY][0]), 1, 0.5);
  gtk_widget_set_style (WMain.header_titles[C2_HEADER_TITLES_PRIORITY][0], style2);

  WMain.header_titles[C2_HEADER_TITLES_PRIORITY][1] = gtk_label_new ("");
  gtk_table_attach (GTK_TABLE (table), WMain.header_titles[C2_HEADER_TITLES_PRIORITY][1], 3, 4, 4, 5,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (WMain.header_titles[C2_HEADER_TITLES_PRIORITY][1]), 7.45058e-09, 0.5);
  if (c2_app.showable_headers[C2_SHOWABLE_HEADERS_PREVIEW] & 1 << 7) {
    gtk_widget_show (WMain.header_titles[C2_HEADER_TITLES_PRIORITY][0]);
    gtk_widget_show (WMain.header_titles[C2_HEADER_TITLES_PRIORITY][1]);
  }
  
  /** Hbox **/
  hbox = gtk_hbox_new (FALSE, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);
  gtk_widget_show (hbox);

  /** Text **/
  WMain.text = gtk_text_new (NULL, NULL);
  gtk_box_pack_start (GTK_BOX (hbox), WMain.text, TRUE, TRUE, 0);
  gtk_widget_show (WMain.text);
  
  /** Vbox **/
  vbox2 = gtk_vbox_new (FALSE, 2);
  gtk_box_pack_end (GTK_BOX (hbox), vbox2, FALSE, TRUE, 0);
  gtk_widget_show (vbox2);
  
  /** VScroll **/
  scroll = gtk_vscrollbar_new (GTK_TEXT (WMain.text)->vadj);
  gtk_box_pack_start (GTK_BOX (vbox2), scroll, TRUE, TRUE, 0);
  gtk_widget_show (scroll);

  /** Mime Left **/
  button = gtk_button_new ();
  pixmap = gnome_stock_pixmap_widget_at_size (WMain.window, GNOME_STOCK_PIXMAP_BACK, 10, 14);
  gtk_container_add (GTK_CONTAINER (button), pixmap);
  gtk_box_pack_end (GTK_BOX (vbox2), button, FALSE, FALSE, 0);
  gtk_widget_show (pixmap);
  gtk_widget_show (button);
  WMain.mime_left = button;
  gtk_tooltips_set_tip (c2_app.tooltips, button, _("Show the attachment list of this message"), NULL);

  /** Mime Right **/
  button = gtk_button_new ();
  pixmap = gnome_stock_pixmap_widget_at_size (WMain.window, GNOME_STOCK_PIXMAP_FORWARD, 10, 14);
  gtk_container_add (GTK_CONTAINER (button), pixmap);
  gtk_box_pack_end (GTK_BOX (vbox2), button, FALSE, FALSE, 0);
  gtk_widget_show (pixmap);
  WMain.mime_right = button;
  gtk_tooltips_set_tip (c2_app.tooltips, button, _("Hide the attachment list of this message"), NULL);

  /** Hbox **/
  WMain.mime_scroll = gtk_hbox_new (FALSE, 2);
  gtk_box_pack_start (GTK_BOX (vbox), WMain.mime_scroll, FALSE, FALSE, 0);

  /** Viewport **/
  viewport = gtk_viewport_new (NULL, NULL);
  gtk_box_pack_start (GTK_BOX (WMain.mime_scroll), viewport, TRUE, TRUE, 0);
  gtk_widget_set_usize (viewport, -1, 70);
  gtk_widget_show (viewport);

  /** Icon List **/
  WMain.icon_list = gnome_icon_list_new (70, NULL, 0);
  gtk_container_add (GTK_CONTAINER (viewport), WMain.icon_list);
  gtk_widget_show (WMain.icon_list);
  gnome_icon_list_set_selection_mode (GNOME_ICON_LIST (WMain.icon_list), GTK_SELECTION_MULTIPLE);
  main_window_menu_attach ();

  /** Vbox **/
  vbox2 = gtk_vbox_new (FALSE, 2);
  gtk_box_pack_end (GTK_BOX (WMain.mime_scroll), vbox2, FALSE, TRUE, 0);
  gtk_widget_show (vbox2);
  
  /** VScroll **/
  scroll = gtk_vscrollbar_new (GNOME_ICON_LIST (WMain.icon_list)->adj);
  gtk_box_pack_start (GTK_BOX (vbox2), scroll, TRUE, TRUE, 0);
  gtk_widget_show (scroll);

  /** Button **/
  button = gtk_toggle_button_new ();
  pixmap = gnome_stock_pixmap_widget_at_size (WMain.window, GNOME_STOCK_PIXMAP_ATTACH, 10, 14);
  gtk_container_add (GTK_CONTAINER (button), pixmap);
  gtk_box_pack_end (GTK_BOX (vbox2), button, FALSE, FALSE, 0);
  gtk_widget_show (pixmap);
  gtk_widget_show (button);
  gtk_tooltips_set_tip (c2_app.tooltips, button, _("Set/Unset the attachment list as sticky"), NULL);
  if (c2_app.mime_window == C2_MIME_WINDOW_STICKY) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
  
  /** Status HBox **/
  hbox = gtk_hbox_new (FALSE, 1);
  gtk_box_pack_start (GTK_BOX (GNOME_APP (WMain.window)->vbox), hbox, FALSE, TRUE, 1);
  gtk_widget_show (hbox);

  /** Appbar **/
  WMain.appbar = gnome_appbar_new (TRUE, TRUE, GNOME_PREFERENCES_NEVER);
  gtk_box_pack_start (GTK_BOX (hbox), WMain.appbar, TRUE, TRUE, 0);
  gtk_widget_show (WMain.appbar);

  /** Button **/
  button = gtk_button_new ();
  pixmap = gnome_stock_pixmap_widget_at_size (WMain.window, GNOME_STOCK_PIXMAP_UP, 10, 14);
  gtk_container_add (GTK_CONTAINER (button), pixmap);
  gtk_box_pack_end (GTK_BOX (hbox), button, FALSE, FALSE, 0);
  gtk_widget_show (pixmap);
  gtk_widget_show (button);
  gtk_tooltips_set_tip (c2_app.tooltips, button, _("Show the Checking Window."), NULL);

#if USE_PLUGINS
#endif

  /*********************************/

  main_window_install_menu_hints ();
  gtk_widget_show (WMain.window);
}

/******************************************
 *********** MARK MENU ********************
 ******************************************/
static GnomeUIInfo mark_menu[] =
{
  {
    GNOME_APP_UI_ITEM, N_("_Unread"),
    N_("Mark the selected mails as unread"),
    NULL, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_MAIL,
    GDK_U, GDK_MOD1_MASK, NULL
  },
  {
    GNOME_APP_UI_ITEM, N_("Rea_d"),
    N_("Mark the selected mails as read"),
    NULL, NULL, NULL,
    GNOME_APP_PIXMAP_NONE, NULL,
    GDK_V, GDK_MOD1_MASK, NULL
  },
  {
    GNOME_APP_UI_ITEM, N_("_Replied"),
    N_("Mark the selected mails as replied"),
    NULL, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_MAIL_RPL,
    GDK_R, GDK_MOD1_MASK, NULL
  },
  {
    GNOME_APP_UI_ITEM, N_("_Forwarded"),
    N_("Mark the selected mails as forwarded"),
    NULL, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_MAIL_FWD,
    GDK_F, GDK_MOD1_MASK, NULL
  },
  GNOMEUIINFO_SEPARATOR,
  GNOMEUIINFO_END
};

/*******************************************
 *********** CLIST MENU ********************
 *******************************************/
static GnomeUIInfo menu_clist[] =
{
  GNOMEUIINFO_SEPARATOR,
  {
    GNOME_APP_UI_ITEM, N_("_Previous"),
    N_("Show the previous message in the current mailbox"),
    NULL, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_BACK,
    0, 0, NULL
  },
  {
    GNOME_APP_UI_ITEM, N_("_Next"),
    N_("Show the next message in the current mailbox"),
    NULL, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_FORWARD,
    0, 0, NULL
  },
  GNOMEUIINFO_SEPARATOR,
  GNOMEUIINFO_MENU_SAVE_ITEM (NULL, NULL),
  GNOMEUIINFO_MENU_PRINT_ITEM (NULL, NULL),
  GNOMEUIINFO_SEPARATOR,
  {
    GNOME_APP_UI_ITEM, N_("_Reply"),
    NULL,
    NULL, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_MAIL_RPL,
    0, 0, NULL
  },
  {
    GNOME_APP_UI_ITEM, N_("Reply _all"),
    NULL,
    NULL, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_MAIL_RPL,
    0, 0, NULL
  },
  {
    GNOME_APP_UI_ITEM, N_("_Forward"),
    NULL,
    NULL, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_MAIL_FWD,
    0, 0, NULL
  },
  GNOMEUIINFO_SEPARATOR,
  {
    GNOME_APP_UI_ITEM, N_("_Delete"),
    NULL,
    NULL, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_TRASH,
    0, 0, NULL
  },
  {
    GNOME_APP_UI_ITEM, N_("_Expunge"),
    NULL,
    NULL, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_TRASH_FULL,
    0, 0, NULL
  },
  GNOMEUIINFO_SEPARATOR,
  {
    GNOME_APP_UI_ITEM, N_("_Copy"),
    NULL,
    NULL, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_COPY,
    0, 0, NULL
  },
  {
    GNOME_APP_UI_ITEM, N_("_Move"),
    NULL,
    NULL, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_JUMP_TO,
    0, 0, NULL
  },
  GNOMEUIINFO_SEPARATOR,
  {
    GNOME_APP_UI_SUBTREE, N_("Mar_k"),
    NULL, mark_menu, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_INDEX,
    0, 0, NULL
  },
  GNOMEUIINFO_SEPARATOR,
  GNOMEUIINFO_END
};

static void main_window_menu_clist (void) {
  GtkWidget *menu;

  menu = gnome_popup_menu_new (menu_clist);
  gnome_popup_menu_attach (menu, WMain.clist, NULL);
}

/********************************************
 *********** ATTACH MENU ********************
 ********************************************/
static GnomeUIInfo menu_attach[] = {
  GNOMEUIINFO_SEPARATOR,
  {
    GNOME_APP_UI_ITEM, N_("Open"),
    NULL, NULL, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_OPEN,
    0, 0, NULL
  },
  {
    GNOME_APP_UI_ITEM, N_("_View"),
    NULL, NULL, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_NEW,
    0, 0, NULL
  },
  {
    GNOME_APP_UI_ITEM, N_("_Save"),
    NULL, NULL, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_SAVE,
    0, 0, NULL
  },
  GNOMEUIINFO_SEPARATOR,
  GNOMEUIINFO_END
};

static void
main_window_menu_attach (void) {
  WMain.mb_w.attach_menu = gnome_popup_menu_new (menu_attach);
  gnome_popup_menu_attach (WMain.mb_w.attach_menu, WMain.icon_list, NULL);
  WMain.mb_w.attach_menu_sep = menu_attach[4].widget;
}

GtkWidget *
main_window_menu_attach_add_item (const char *label, const char *pixmap, GtkSignalFunc func, gpointer data) {
  GtkWidget *item;
  GtkWidget *submenu;
  GtkWidget *wpixmap, *hbox, *wlabel;

  if (WMain.mb_w.attach_menu)
    submenu = WMain.mb_w.attach_menu;
  else {
    submenu = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (WMain.mb_w.attach_menu), submenu);
  }

  item = gtk_pixmap_menu_item_new ();
  gtk_signal_connect (GTK_OBJECT (item), "activate",
      			GTK_SIGNAL_FUNC (func), data);

  if (label && pixmap)
    wpixmap = gnome_stock_pixmap_widget_at_size (WMain.window, pixmap, 18, 18);

  if (label)
    hbox = gtk_hbox_new (FALSE, 0);
  if (label && strlen (label) > 40) {
    char *s = g_strndup (label, 40);
    char *t = g_strconcat (s, "...", NULL);
    wlabel = gtk_label_new (t);
    g_free (s);
    g_free (t);
  } else if (label) wlabel = gtk_label_new (label);

  if (label) {
    gtk_box_pack_start (GTK_BOX (hbox), wlabel, FALSE, FALSE, 0);
    gtk_container_add (GTK_CONTAINER (item), hbox);
  }

  if (label && pixmap)
    gtk_pixmap_menu_item_set_pixmap (GTK_PIXMAP_MENU_ITEM (item), wpixmap);

  if (label) {
    gtk_widget_show (wlabel);
    gtk_widget_show (hbox);
  }
  if (gnome_preferences_get_menus_have_icons () && pixmap) gtk_widget_show (wpixmap);
  gtk_menu_append (GTK_MENU (submenu), item);
  gtk_widget_show (item);

  if (!label) gtk_widget_set_sensitive (item, FALSE);

  return item;
}

void
main_window_menu_attach_clear (void) {
  GList *l, *s;
  
  /* Remove all after the separator */
  l = gtk_container_children (GTK_CONTAINER (WMain.mb_w.attach_menu));

  /* Find the separator */
  for (s = l; s != NULL; s = s->next) {
    if (s->data == WMain.mb_w.attach_menu_sep) break;
  }

  /* And remove */
  for (s = g_list_next (s); s != NULL; s = s->next) {
    if (GTK_IS_WIDGET (s->data)) gtk_widget_destroy (GTK_WIDGET (s->data));
  }
  g_list_free (l);
}

/*********************************************
 *********** ACCOUNT MENU ********************
 *********************************************/
static GnomeUIInfo account_menu[] = {
  {
    GNOME_APP_UI_ITEM, N_("_All Accounts"),
    N_("Check all accounts"),
    NULL, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_MAIL_RCV,
    'M', GDK_CONTROL_MASK, NULL
  },
  GNOMEUIINFO_SEPARATOR,
  GNOMEUIINFO_END
};

/******************************************
 *********** PERSISTENT SMTP MENU *********
 ******************************************/
static GnomeUIInfo persistent_smtp_menu[] = {
  {
    GNOME_APP_UI_ITEM, N_("_Connect"),
    N_("Connect to the SMTP server"),
    NULL, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_HOME,
    0, 0, NULL
  },
  {
    GNOME_APP_UI_ITEM, N_("_Disconnect"),
    N_("Disconnect from the SMTP server"),
    NULL, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_STOP,
    0, 0, NULL
  },
  GNOMEUIINFO_END
};

/******************************************
 *********** FILE MENU ********************
 ******************************************/
static GnomeUIInfo file_menu[] = {
  { /* Get New Mail */
    GNOME_APP_UI_SUBTREE, N_("_Get New Mail"),
    NULL, account_menu, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_MAIL_RCV,
    0, 0, NULL
  },
  GNOMEUIINFO_SEPARATOR,
  { /* Send queue messages */
    GNOME_APP_UI_ITEM, N_("_Send Queued Messages"),
    N_("Send mail from the Queue mailbox now"), NULL, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_MAIL_SND,
    0, 0, NULL
  },     
  GNOMEUIINFO_SEPARATOR,
  {
    GNOME_APP_UI_SUBTREE, N_("_Persistent SMTP"),
    NULL, persistent_smtp_menu, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_REFRESH,
    0, 0, NULL
  },
  GNOMEUIINFO_SEPARATOR,
  GNOMEUIINFO_MENU_EXIT_ITEM (NULL, NULL),
  GNOMEUIINFO_SEPARATOR,
  GNOMEUIINFO_END
};

/******************************************
 *********** EDIT MENU ********************
 ******************************************/
static GnomeUIInfo edit_menu[] =
{
  GNOMEUIINFO_MENU_FIND_ITEM (NULL, NULL),
  GNOMEUIINFO_SEPARATOR,
  GNOMEUIINFO_END
};

/******************************************
 *********** VIEW MENU ********************
 ******************************************/
static GnomeUIInfo view_menu[] = {
  {
    GNOME_APP_UI_TOGGLEITEM, N_("From"),
    NULL, NULL, NULL, NULL,
    GNOME_APP_PIXMAP_NONE, NULL,
    0, 0, NULL
  },
  {
    GNOME_APP_UI_TOGGLEITEM, N_("Account"),
    NULL, NULL, NULL, NULL,
    GNOME_APP_PIXMAP_NONE, NULL,
    0, 0, NULL
  },
  {
    GNOME_APP_UI_TOGGLEITEM, N_("To"),
    NULL, NULL, NULL, NULL,
    GNOME_APP_PIXMAP_NONE, NULL,
    0, 0, NULL
  },
  {
    GNOME_APP_UI_TOGGLEITEM, N_("CC"),
    NULL, NULL, NULL, NULL,
    GNOME_APP_PIXMAP_NONE, NULL,
    0, 0, NULL
  },
  {
    GNOME_APP_UI_TOGGLEITEM, N_("BCC"),
    NULL, NULL, NULL, NULL,
    GNOME_APP_PIXMAP_NONE, NULL,
    0, 0, NULL
  },
  {
    GNOME_APP_UI_TOGGLEITEM, N_("Date"),
    NULL, NULL, NULL, NULL,
    GNOME_APP_PIXMAP_NONE, NULL,
    0, 0, NULL
  },
  {
    GNOME_APP_UI_TOGGLEITEM, N_("Subject"),
    NULL, NULL, NULL, NULL,
    GNOME_APP_PIXMAP_NONE, NULL,
    0, 0, NULL
  },
  {
    GNOME_APP_UI_TOGGLEITEM, N_("Priority"),
    NULL, NULL, NULL, NULL,
    GNOME_APP_PIXMAP_NONE, NULL,
    0, 0, NULL
  },
  GNOMEUIINFO_SEPARATOR,
  GNOMEUIINFO_END
};

/*********************************************
 *********** MAILBOX MENU ********************
 *********************************************/
static GnomeUIInfo mailbox_menu[] = {
#ifndef USE_OLD_MBOX_HANDLERS
  {
    GNOME_APP_UI_ITEM, N_("_Speed Up"),
    N_("Easily speed up the mailbox handling"),
    NULL, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_TIMER, 0, 0, NULL
  },
  GNOMEUIINFO_SEPARATOR,
#endif
  {
    GNOME_APP_UI_ITEM, N_("_Import"),
    N_("Import a mailbox from a different format"),
    NULL, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_HOME, 0, GDK_CONTROL_MASK, NULL
  },
  {
    GNOME_APP_UI_ITEM, N_("_Export"),
    N_("Export a mailbox to a different format"),
    NULL, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_CONVERT, 0, GDK_CONTROL_MASK, NULL
  },
  GNOMEUIINFO_SEPARATOR,
  GNOMEUIINFO_END
};


/*********************************************
 *********** MESSAGE MENU ********************
 *********************************************/
static GnomeUIInfo message_menu[] =
{
  {
    GNOME_APP_UI_ITEM, N_("_Compose"),
    N_("Open the Composer window"),
    NULL, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_MAIL_NEW,
    GDK_N, GDK_CONTROL_MASK, NULL
  },
  GNOMEUIINFO_SEPARATOR,
  GNOMEUIINFO_MENU_SAVE_ITEM (NULL, NULL),
  GNOMEUIINFO_MENU_PRINT_ITEM (NULL, NULL),
  GNOMEUIINFO_SEPARATOR,
  {
    GNOME_APP_UI_ITEM, N_("_Reply"),
    N_("Reply to sender of the selected message"),
    NULL, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_MAIL_RPL,
    GDK_R, GDK_CONTROL_MASK, NULL
  },
  {
    GNOME_APP_UI_ITEM, N_("Reply _all"),
    N_("Reply to all recipients of the selected message"),
    NULL, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_MAIL_RPL,
    GDK_A, GDK_CONTROL_MASK, NULL
  },
  {
    GNOME_APP_UI_ITEM, N_("_Forward"),
    N_("Forward the selected message"),
    NULL, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_MAIL_FWD,
    GDK_F, GDK_CONTROL_MASK, NULL
  },
  GNOMEUIINFO_SEPARATOR,
  {
    GNOME_APP_UI_ITEM, N_("_Copy"),
    NULL,
    NULL, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_COPY,
    GDK_C, GDK_MOD1_MASK, NULL
  },
  {
    GNOME_APP_UI_ITEM, N_("_Move"),
    NULL,
    NULL, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_JUMP_TO,
    GDK_M, GDK_MOD1_MASK, NULL
  },
  GNOMEUIINFO_SEPARATOR,
  {
    GNOME_APP_UI_ITEM, N_("_Delete"),
    N_("Delete the selected messages"),
    NULL, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_TRASH,
    GDK_D, 0, NULL
  },
  {
    GNOME_APP_UI_ITEM, N_("_Expunge"),
    N_("Expunge the selected messages"),
    NULL, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_TRASH_FULL,
    GDK_E, GDK_CONTROL_MASK, NULL
  },
  GNOMEUIINFO_SEPARATOR,
  {
    GNOME_APP_UI_SUBTREE, N_("Mar_k"),
    NULL, mark_menu, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_INDEX,
    0, 0, NULL
  },
  GNOMEUIINFO_SEPARATOR,
  {
    GNOME_APP_UI_ITEM, N_("_Previous"),
    N_("Show the previous message in the current mailbox"),
    NULL, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_BACK,
    GDK_Z, 0, NULL
  },
  {
    GNOME_APP_UI_ITEM, N_("_Next"),
    N_("Show the next message in the current mailbox"),
    NULL, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_FORWARD,
    GDK_X, 0, NULL
  },
  GNOMEUIINFO_SEPARATOR,
  GNOMEUIINFO_END
};

/**********************************************
 *********** SETTINGS MENU ********************
 **********************************************/
static GnomeUIInfo settings_menu[] =
{
  GNOMEUIINFO_MENU_PREFERENCES_ITEM (NULL, NULL),
  GNOMEUIINFO_SEPARATOR,
  GNOMEUIINFO_END
};

#ifdef BUILD_ADDRESS_BOOK
/**********************************************
 ********** WINDOWS MENU **********************
 **********************************************/
static GnomeUIInfo windows_menu[] =
{
  {
    GNOME_APP_UI_ITEM, N_("Address Book"),
    N_("Open the Cronos II Address Book"),
    NULL, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_BOOK_GREEN,
    0, 0, NULL
  },
  GNOMEUIINFO_SEPARATOR,
  GNOMEUIINFO_END
};
#endif
/******************************************
 *********** HELP MENU ********************
 ******************************************/
static GnomeUIInfo help_menu[] =
{
  {
    GNOME_APP_UI_ITEM, N_("Bug Report"),
    N_("Open the bug report window"),
    NULL, NULL, NULL,
    GNOME_APP_PIXMAP_NONE, NULL,
    0, 0, NULL
  },
  GNOMEUIINFO_SEPARATOR,
  GNOMEUIINFO_HELP(PACKAGE),
  GNOMEUIINFO_SEPARATOR,
  GNOMEUIINFO_MENU_ABOUT_ITEM (NULL, NULL),
  GNOMEUIINFO_SEPARATOR,
  GNOMEUIINFO_END
};

/******************************************
 *********** MAIN MENU ********************
 ******************************************/
static GnomeUIInfo menu_bar[] = {
  GNOMEUIINFO_MENU_FILE_TREE (file_menu),
  GNOMEUIINFO_MENU_EDIT_TREE (edit_menu),
  GNOMEUIINFO_MENU_VIEW_TREE (view_menu),
  {
    GNOME_APP_UI_SUBTREE, N_("M_ailbox"),
    NULL, mailbox_menu, NULL, NULL,
    GNOME_APP_PIXMAP_NONE, N_("M_ailbox"),
    0, 0, NULL
  },
  {
    GNOME_APP_UI_SUBTREE, N_("_Message"),
    NULL, message_menu, NULL, NULL,
    GNOME_APP_PIXMAP_NONE, N_("_Message"),
    0, 0, NULL
  },
  GNOMEUIINFO_MENU_SETTINGS_TREE (settings_menu),
#ifdef BUILD_ADDRESS_BOOK
  GNOMEUIINFO_MENU_WINDOWS_TREE (windows_menu),
#endif
  GNOMEUIINFO_MENU_HELP_TREE (help_menu),
  GNOMEUIINFO_END
};

#if FALSE
void main_window_make_account_menu (void) {
  /* This function is inspired in Galeon 0.8.1 */
  GtkWidget *item;
  GtkWidget *pixmap;
  GtkWidget *box;
  GtkWidget *label;
  GtkWidget *submenu;
  GList *l, *s;
  Account *account;
  /*int i;
  GtkAccelGroup *grp;*/

  /* First remove all after the separator */
  l = gtk_container_children (GTK_CONTAINER (GTK_MENU_ITEM (WMain.mb_w.get_new_mail)->submenu));

  /* Find the separator */
  for (s = l; s != NULL; s = s->next) {
    if (s->data == WMain.mb_w.get_new_mail_sep) break;
  }
  /* And remove */
  for (s = g_list_next (s); s != NULL; s = s->next) {
    if (GTK_IS_WIDGET (s->data)) gtk_widget_destroy (GTK_WIDGET (s->data));
  }
  g_list_free (l);
  
  if (GTK_MENU_ITEM (WMain.mb_w.get_new_mail)->submenu)
    submenu = GTK_MENU_ITEM (WMain.mb_w.get_new_mail)->submenu;
  else {
    submenu = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (WMain.mb_w.get_new_mail), submenu);
  }

  for (account = c2_app.account_head/*, i = 1*/; account != NULL; account = account->next/*, i++*/) {
    item = gtk_pixmap_menu_item_new ();

    pixmap = gnome_stock_pixmap_widget_at_size (WMain.window, GNOME_STOCK_PIXMAP_MAIL, 18, 18);
    box = gtk_hbox_new (FALSE, 0);
    if (strlen (account->acc_name) > 40) {
      char *s = g_strndup (account->acc_name, 40);
      char *t = g_strconcat (s, "...", NULL);
      label = gtk_label_new (t);
      c2_free (s);
      c2_free (t);
    } else label = gtk_label_new (account->acc_name);
    
    gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);
    gtk_container_add (GTK_CONTAINER (item), box);
    gtk_pixmap_menu_item_set_pixmap (GTK_PIXMAP_MENU_ITEM (item), pixmap);
    gtk_widget_show (label);
    gtk_widget_show (box);
    if (gnome_preferences_get_menus_have_icons ()) gtk_widget_show (pixmap);
    gtk_menu_append (GTK_MENU (submenu), item);
    gtk_widget_show (item);
  }

  item = gtk_menu_item_new ();
  gtk_menu_append (GTK_MENU (submenu), item);
  gtk_widget_set_sensitive (GTK_WIDGET (item), FALSE);
  gtk_widget_show (item);
}
#endif

void main_window_menubar (void) {
  GtkWidget *widget;
  
  gnome_app_create_menus (GNOME_APP (WMain.window), menu_bar);
  WMain.mb_w.file_menu		= menu_bar[0].widget;
  WMain.mb_w.get_new_mail	= file_menu[0].widget;
  WMain.mb_w.get_new_mail_sep	= account_menu[1].widget;
  WMain.mb_w.menu_sendqueue	= file_menu[2].widget;
  WMain.mb_w.persistent_smtp_options = file_menu[4].widget;
  WMain.mb_w.persistent_smtp_options_connect = persistent_smtp_menu[0].widget;
  WMain.mb_w.persistent_smtp_options_disconnect = persistent_smtp_menu[1].widget;
  WMain.mb_w.quit		= file_menu[4].widget;
  WMain.mb_w.edit_menu		= menu_bar[1].widget;
  WMain.mb_w.search		= edit_menu[0].widget;
  WMain.mb_w.message_menu	= menu_bar[2].widget;
  WMain.mb_w.compose		= message_menu[0].widget;
  WMain.mb_w.save		= message_menu[2].widget;
  WMain.mb_w.print		= message_menu[3].widget;
  WMain.mb_w.reply		= message_menu[5].widget;
  WMain.mb_w.reply_all		= message_menu[6].widget;
  WMain.mb_w.forward		= message_menu[7].widget;
  WMain.mb_w.copy		= message_menu[9].widget;
  WMain.mb_w.move		= message_menu[10].widget;
  WMain.mb_w._delete		= message_menu[12].widget;
  WMain.mb_w.expunge		= message_menu[13].widget;
  WMain.mb_w.mark		= message_menu[15].widget;
  WMain.mb_w.previous		= message_menu[17].widget;
  WMain.mb_w.next		= message_menu[18].widget;
  WMain.mb_w.settings		= menu_bar[3].widget;
  WMain.mb_w.preferences	= settings_menu[0].widget;
  WMain.mb_w.about		= help_menu[0].widget;

  /* Activated here because they can't dynamically change */
  gtk_widget_set_sensitive (WMain.mb_w.quit, TRUE);

  if (!c2_app.use_persistent_smtp) {
    gtk_widget_set_sensitive (WMain.mb_w.persistent_smtp_options, FALSE);
    gtk_widget_set_sensitive (WMain.mb_w.persistent_smtp_options_connect, FALSE);
    gtk_widget_set_sensitive (WMain.mb_w.persistent_smtp_options_disconnect, FALSE);
  }
  
  /*if (!c2_app.queue_state)
  	gtk_widget_set_sensitive (WMain.mb_w.menu_sendqueue, FALSE);
  else
  	gtk_widget_set_sensitive (WMain.mb_w.menu_sendqueue, TRUE);*/
	
  widget = view_menu[0].widget;
  if (c2_app.showable_headers[C2_SHOWABLE_HEADERS_PREVIEW] & 1 << 2)
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (widget), TRUE);
  widget = view_menu[1].widget;
  if (c2_app.showable_headers[C2_SHOWABLE_HEADERS_PREVIEW] & 1 << 4)
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (widget), TRUE);
  widget = view_menu[2].widget;
  if (c2_app.showable_headers[C2_SHOWABLE_HEADERS_PREVIEW] & 1 << 0)
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (widget), TRUE);
  widget = view_menu[3].widget;
  if (c2_app.showable_headers[C2_SHOWABLE_HEADERS_PREVIEW] & 1 << 5)
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (widget), TRUE);
  widget = view_menu[4].widget;
  if (c2_app.showable_headers[C2_SHOWABLE_HEADERS_PREVIEW] & 1 << 6)
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (widget), TRUE);
  widget = view_menu[5].widget;
  if (c2_app.showable_headers[C2_SHOWABLE_HEADERS_PREVIEW] & 1 << 1)
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (widget), TRUE);
  widget = view_menu[6].widget;
  if (c2_app.showable_headers[C2_SHOWABLE_HEADERS_PREVIEW] & 1 << 3)
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (widget), TRUE);
  widget = view_menu[7].widget;
  if (c2_app.showable_headers[C2_SHOWABLE_HEADERS_PREVIEW] & 1 << 7)
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (widget), TRUE);
}

void main_window_toolbar (void) {
  GtkWidget *toolbar;
  GtkWidget *btn;
  GtkWidget *tmp_toolbar_icon;
  GdkPixmap *icon;
  GdkBitmap *mask;  

  toolbar = gtk_toolbar_new (GTK_ORIENTATION_HORIZONTAL, c2_app.toolbar);

  tmp_toolbar_icon = gnome_stock_pixmap_widget (WMain.window, GNOME_STOCK_PIXMAP_MAIL_RCV);
  WMain.tb_w.get_new_mail = gtk_toolbar_append_element (GTK_TOOLBAR (toolbar),
                                GTK_TOOLBAR_CHILD_BUTTON,
                                NULL,
                                N_("Check"),
                                N_("Get New Mail"), NULL,
                                tmp_toolbar_icon, NULL, NULL);
//  if (!c2_app.account_head) gtk_widget_set_sensitive (WMain.tb_w.get_new_mail, FALSE);

  icon = gdk_pixmap_create_from_xpm_d(WMain.window->window,
  					 &mask, &WMain.window->style->white,
					 sendqueue_xpm);
  tmp_toolbar_icon = gtk_pixmap_new(icon, mask);

  WMain.tb_w.sendqueue = gtk_toolbar_append_element (GTK_TOOLBAR (toolbar),
                                GTK_TOOLBAR_CHILD_BUTTON,
                                NULL,
                                N_("Send"),
                                N_("Send Queued Messages"), NULL,
                                tmp_toolbar_icon, NULL, NULL);
//  if (!c2_app.queue_state) gtk_widget_set_sensitive (WMain.tb_w.sendqueue, FALSE);

  tmp_toolbar_icon = gnome_stock_pixmap_widget (WMain.window, GNOME_STOCK_PIXMAP_MAIL_NEW);
  WMain.tb_w.compose = gtk_toolbar_append_element (GTK_TOOLBAR (toolbar),
                                GTK_TOOLBAR_CHILD_BUTTON,
                                NULL,
                                N_("Compose"),
                                N_("Compose a new mail"), NULL,
                                tmp_toolbar_icon, NULL, NULL);
//  if (!c2_app.account_head) gtk_widget_set_sensitive (WMain.tb_w.compose, FALSE);

  gtk_toolbar_append_space (GTK_TOOLBAR (toolbar));

  tmp_toolbar_icon = gnome_stock_pixmap_widget (WMain.window, GNOME_STOCK_PIXMAP_SAVE_AS);
  WMain.tb_w.save = gtk_toolbar_append_element (GTK_TOOLBAR (toolbar),
                                GTK_TOOLBAR_CHILD_BUTTON,
                                NULL,
                                N_("Save"),
                                N_("Save the selected mails to a file"), NULL,
                                tmp_toolbar_icon, NULL, NULL);

  tmp_toolbar_icon = gnome_stock_pixmap_widget (WMain.window, GNOME_STOCK_PIXMAP_PRINT);
  WMain.tb_w.print = gtk_toolbar_append_element (GTK_TOOLBAR (toolbar),
                                GTK_TOOLBAR_CHILD_BUTTON,
                                NULL,
                                N_("Print"),
                                N_("Print the selected mails"), NULL,
                                tmp_toolbar_icon, NULL, NULL);
  
  gtk_toolbar_append_space (GTK_TOOLBAR (toolbar));

  tmp_toolbar_icon = gnome_stock_pixmap_widget (WMain.window, GNOME_STOCK_PIXMAP_SEARCH);
  WMain.tb_w.search = gtk_toolbar_append_element (GTK_TOOLBAR (toolbar),
                                GTK_TOOLBAR_CHILD_BUTTON,
                                NULL,
                                N_("Search"),
                                N_("Search for a message"), NULL,
                                tmp_toolbar_icon, NULL, NULL);

  gtk_toolbar_append_space (GTK_TOOLBAR (toolbar));
  
  tmp_toolbar_icon = gnome_stock_pixmap_widget (WMain.window, GNOME_STOCK_PIXMAP_TRASH);
  WMain.tb_w._delete = gtk_toolbar_append_element (GTK_TOOLBAR (toolbar),
                                GTK_TOOLBAR_CHILD_BUTTON,
                                NULL,
                                N_("Delete"),
                                N_("Delete the selected mails"), NULL,
                                tmp_toolbar_icon, NULL, NULL);

  gtk_toolbar_append_space (GTK_TOOLBAR (toolbar));

  tmp_toolbar_icon = gnome_stock_pixmap_widget (WMain.window, GNOME_STOCK_PIXMAP_MAIL_RPL);
  WMain.tb_w.reply = gtk_toolbar_append_element (GTK_TOOLBAR (toolbar),
                                GTK_TOOLBAR_CHILD_BUTTON,
                                NULL,
                                N_("Reply"),
                                N_("Reply the selected mails"), NULL,
                                tmp_toolbar_icon, NULL, NULL);

  tmp_toolbar_icon = gnome_stock_pixmap_widget (WMain.window, GNOME_STOCK_PIXMAP_MAIL_RPL);
  WMain.tb_w.reply_all = gtk_toolbar_append_element (GTK_TOOLBAR (toolbar),
                                GTK_TOOLBAR_CHILD_BUTTON,
                                NULL,
                                N_("Reply All"),
                                N_("Reply to all recipients the selected mails"), NULL,
                                tmp_toolbar_icon, NULL, NULL);
  
  tmp_toolbar_icon = gnome_stock_pixmap_widget (WMain.window, GNOME_STOCK_PIXMAP_MAIL_FWD);
  WMain.tb_w.forward = gtk_toolbar_append_element (GTK_TOOLBAR (toolbar),
                                GTK_TOOLBAR_CHILD_BUTTON,
                                NULL,
                                N_("Forward"),
                                N_("Forward the selected mails"), NULL,
                                tmp_toolbar_icon, NULL, NULL);

  gtk_toolbar_append_space (GTK_TOOLBAR (toolbar));
  
  tmp_toolbar_icon = gnome_stock_pixmap_widget (WMain.window, GNOME_STOCK_PIXMAP_BACK);
  WMain.tb_w.previous = gtk_toolbar_append_element (GTK_TOOLBAR (toolbar),
                                GTK_TOOLBAR_CHILD_BUTTON,
                                NULL,
                                N_("Previous"),
                                N_("Show previous message"), NULL,
                                tmp_toolbar_icon, NULL, NULL);

  tmp_toolbar_icon = gnome_stock_pixmap_widget (WMain.window, GNOME_STOCK_PIXMAP_FORWARD);
  WMain.tb_w.next = gtk_toolbar_append_element (GTK_TOOLBAR (toolbar),
                                GTK_TOOLBAR_CHILD_BUTTON,
                                NULL,
                                N_("Next"),
                                N_("Show next message"), NULL,
                                tmp_toolbar_icon, NULL, NULL);
  
  gtk_toolbar_append_space (GTK_TOOLBAR (toolbar));

#ifdef BUILD_ADDRESS_BOOK
  tmp_toolbar_icon = gnome_stock_pixmap_widget (WMain.window, GNOME_STOCK_PIXMAP_BOOK_GREEN);
  btn = gtk_toolbar_append_element (GTK_TOOLBAR (toolbar),
                                GTK_TOOLBAR_CHILD_BUTTON,
                                NULL,
                                N_("Contacts"),
                                N_("Open the Cronos II Address Book"), NULL,
                                tmp_toolbar_icon, NULL, NULL);
  
  gtk_toolbar_append_space (GTK_TOOLBAR (toolbar));
#endif

  tmp_toolbar_icon = gnome_stock_pixmap_widget (WMain.window, GNOME_STOCK_PIXMAP_EXIT);
  WMain.tb_w.quit = gtk_toolbar_append_element (GTK_TOOLBAR (toolbar),
                                GTK_TOOLBAR_CHILD_BUTTON,
                                NULL,
                                N_("Quit"),
                                N_("Quit Cronos II"), NULL,
                                tmp_toolbar_icon, NULL, NULL);

  WMain.toolbar = toolbar;
  gnome_app_set_toolbar (GNOME_APP (WMain.window), GTK_TOOLBAR (toolbar));
  gtk_toolbar_set_space_size (GTK_TOOLBAR (toolbar), 5);
  gtk_toolbar_set_tooltips (GTK_TOOLBAR (toolbar), TRUE);
  gtk_widget_show (toolbar);

  gtk_widget_set_sensitive (WMain.tb_w.quit, TRUE);
}

static void
main_window_install_menu_hints (void) {
  gnome_app_install_menu_hints (GNOME_APP (WMain.window), menu_bar);
}
