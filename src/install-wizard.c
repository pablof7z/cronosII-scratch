#include <gnome.h>
#include <glib.h>

#include <libmodules/utils.h>
#include <libmodules/error.h>

#include "c2-app.h"

#include "xpm/mini_about.xpm"
#include "xpm/mini_error.xpm"

#define REPORT(x) { \
	gchar *n[] = { "", x, NULL }; \
	gtk_clist_freeze (GTK_CLIST (clist)); \
	gtk_clist_append (GTK_CLIST (clist), n); \
	gtk_clist_thaw (GTK_CLIST (clist)); \
}

#define RESULT(status) { \
	if (status) \
		gtk_clist_set_pixmap (GTK_CLIST (clist), GTK_CLIST (clist)->rows-1, 0, success_xpm, success_mask); \
	else \
		gtk_clist_set_pixmap (GTK_CLIST (clist), GTK_CLIST (clist)->rows-1, 0, error_xpm, error_mask); \
}

#define REPORT_RESULT(x, status) { \
	gchar *n, *g; \
	gtk_clist_get_text (GTK_CLIST (clist), GTK_CLIST (clist)->rows-1, 1, &g); \
	n = g_strdup_printf ("%s: %s", g, x); \
	gtk_clist_set_text (GTK_CLIST (clist), GTK_CLIST (clist)->rows-1, 1, n); \
	RESULT (status); \
}

static void
on_close (void);

GtkWidget *window;
GtkWidget *druid;
GtkWidget *druidpage1;
GtkWidget *druidpage2;
GtkWidget *druidpage3;
GtkWidget *clist;

gboolean process_done = FALSE;

GdkPixmap *success_xpm, *error_xpm;
GdkBitmap *success_mask, *error_mask;

static gboolean
detect_old_configuration (void);

static gboolean
import_old_configuration (void);

static void
on_page1_next_clicked (void) {
	if (process_done)
	{
		gnome_druid_set_page (GNOME_DRUID (druid), GNOME_DRUID_PAGE (druidpage2));
		return;
	}
	gnome_druid_set_page (GNOME_DRUID (druid), GNOME_DRUID_PAGE (druidpage2));

	/* Detect previous configuration */
	if (detect_old_configuration ())
	{
		REPORT (N_("Old configuration found, importing it..."));
		RESULT (TRUE);
		if (!import_old_configuration ())
		{
			REPORT (N_("Creating new configuration due to previous error..."));
			RESULT (TRUE);
			goto import_failed;
		}
	} else
	{
		REPORT (N_("No previous configuration found, creating new..."));
		RESULT (TRUE);
import_failed:
	}

	process_done = TRUE;
}

static gboolean
detect_old_configuration (void) {
	gchar *home = g_get_home_dir ();
	gchar *path = g_strdup_printf ("%s" G_DIR_SEPARATOR_S ".CronosII" G_DIR_SEPARATOR_S "cronos.conf", home);
	gboolean val = FALSE;

	REPORT (N_("Detecting old configuration..."));
	if (c2_file_exists (path))
		val = TRUE;
	g_free (path);

	RESULT (TRUE);

	return val;
}

static gboolean
import_old_configuration (void) {
	gchar *home = g_get_home_dir ();
	gchar *path = g_strdup_printf ("%s" G_DIR_SEPARATOR_S ".CronosII" G_DIR_SEPARATOR_S "cronos.conf", home);
	gchar *key, *val;
	FILE *fd;

	gint mailboxes = 0;

	REPORT (N_("Loading old configuration"));
	if (!(fd = fopen (path, "r")))
	{
		const gchar *err;
		c2_error_set (-errno);
		err = c2_error_get (c2_errno);
		REPORT_RESULT (err, FALSE);
		return FALSE;
	}
	RESULT (TRUE);
	g_free (path);

	for (;;)
	{
		if ((key = c2_fd_get_word (fd)) == NULL) break;
		if ((val = c2_fd_get_word (fd)) == NULL) break;
		if (c2_fd_move_to (fd, '\n', 1, TRUE, TRUE) == EOF) fseek (fd, -1, SEEK_CUR);
		
		if (c2_streq (key, "mailbox"))
		{
			C2Mailbox *mbox;
			gchar *tmp;
			
			REPORT (N_("Registering in the new configuration 'mailbox'"));
			mbox = c2_mailbox_parse (val);
			if (!mbox)
			{
				REPORT_RESULT (N_("Failed."), FALSE);
				g_free (val);
				continue;
			}

			tmp = g_strdup_printf ("/cronosII/Mailboxes/%d", mailboxes);
			gnome_config_push_prefix (tmp);
			gnome_config_set_string ("", "");
			gnome_config_set_string ("::Name", mbox->name);
			gnome_config_set_int ("::Id", mbox->id);
			gnome_config_set_int ("::Parent Id", mbox->parent_id);
			gnome_config_pop_prefix ();
			
			g_free (tmp);
			g_free (val);
			c2_mailbox_free (mbox);
			
			mailboxes++;
			REPORT_RESULT (N_("Success."), TRUE);
		} else if (c2_streq (key, "addrbook_init"))
		{
			REPORT (N_("Registering in the new configuration 'addrbook_init'"));
			gnome_config_set_int ("/cronosII/Address Book/init", atoi (val));
			g_free (val);
			REPORT_RESULT (N_("Success."), TRUE);
		} else if (c2_streq (key, "empty_garbage"))
		{
			REPORT (N_("Registering in the new configuration 'empty_garbage'"));
			gnome_config_set_bool ("/cronosII/Options/empty_garbage", atoi (val));
			g_free (val);
			REPORT_RESULT (N_("Success."), TRUE);
		} else if (c2_streq (key, "check_at_start"))
		{
			REPORT (N_("Registering in the new configuration 'check_at_start'"));
			gnome_config_set_bool ("/cronosII/Options/check_at_start", atoi (val));
			g_free (val);
			REPORT_RESULT (N_("Success."), TRUE);
		} else if (c2_streq (key, "use_outbox"))
		{
			REPORT (N_("Registering in the new configuration 'use_outbox'"));
			gnome_config_set_bool ("/cronosII/Options/use_outbox", atoi (val));
			g_free (val);
			REPORT_RESULT (N_("Success."), TRUE);
		} else if (c2_streq (key, "use_persistent_smtp_connection"))
		{
			REPORT (N_("Registering in the new configuration 'use_persistent_smtp_connection'"));
			gnome_config_set_bool ("/cronosII/Persistent SMTP/use", atoi (val));
			g_free (val);
			REPORT_RESULT (N_("Success."), TRUE);
		} else if (c2_streq (key, "check_timeout"))
		{
			REPORT (N_("Registering in the new configuration 'check_timeout'"));
			gnome_config_set_int ("/cronosII/Options/check_timeout", atoi (val));
			g_free (val);
			REPORT_RESULT (N_("Success."), TRUE);
		} else if (c2_streq (key, "message_bigger"))
		{
			REPORT (N_("Registering in the new configuration 'message_size_limit'"));
			gnome_config_set_int ("/cronosII/Options/message_size_limit", atoi (val));
			g_free (val);
			REPORT_RESULT (N_("Success."), TRUE);
		} else if (c2_streq (key, "timeout"))
		{
			REPORT (N_("Registering in the new configuration 'net_timeout'"));
			gnome_config_set_int ("/cronosII/Timeout/net", atoi (val));
			g_free (val);
			REPORT_RESULT (N_("Success."), TRUE);
		} else if (c2_streq (key, "mark_as_read"))
		{
			REPORT (N_("Registering in the new configuration 'mark_as_read_timeout'"));
			gnome_config_set_int ("/cronosII/Timeout/mark_as_read", atoi (val));
			g_free (val);
			REPORT_RESULT (N_("Success."), TRUE);
		} else if (c2_streq (key, "persistent_smtp_port"))
		{
			REPORT (N_("Registering in the new configuration 'persistent_smtp_port'"));
			gnome_config_set_int ("/cronosII/Persistent SMTP/port", atoi (val));
			g_free (val);
			REPORT_RESULT (N_("Success."), TRUE);
		} else if (c2_streq (key, "prepend_char_on_re"))
		{
			REPORT (N_("Registering in the new configuration 'prepend_character'"));
			gnome_config_set_string ("/cronosII/Options/prepend_character", val);
			g_free (val);
			REPORT_RESULT (N_("Success."), TRUE);
		} else if (c2_streq (key, "persistent_smtp_address"))
		{
			REPORT (N_("Registering in the new configuration 'persistent_smtp_host'"));
			gnome_config_set_string ("/cronosII/Persistent SMTP/host", val);
			g_free (val);
			REPORT_RESULT (N_("Success."), TRUE);
		} else if (c2_streq (key, "color_reply_original_message"))
		{
			REPORT (N_("Registering in the new configuration 'color_reply_original_message'"));
			gnome_config_set_string ("/cronosII/Colors/reply_original_message", val);
			g_free (val);
			REPORT_RESULT (N_("Success."), TRUE);
		} else if (c2_streq (key, "color_misc_body"))
		{
			REPORT (N_("Registering in the new configuration 'color_misc_body'"));
			gnome_config_set_string ("/cronosII/Colors/misc_body", val);
			g_free (val);
			REPORT_RESULT (N_("Success."), TRUE);
		}
		g_free (key);
	}

	/* Load the RC file */
	path = g_strdup_printf ("%s" G_DIR_SEPARATOR_S ".CronosII" G_DIR_SEPARATOR_S "cronos.rc", home);
	
	REPORT (N_("Loading old rc configuration"));
	if (!(fd = fopen (path, "r")))
	{
		const gchar *err;
		c2_error_set (-errno);
		err = c2_error_get (c2_errno);
		REPORT_RESULT (err, FALSE);
		return FALSE;
	}
	RESULT (TRUE);
	g_free (path);

	for (;;)
	{
		if ((key = c2_fd_get_word (fd)) == NULL) break;
		if ((val = c2_fd_get_word (fd)) == NULL) break;
		if (c2_fd_move_to (fd, '\n', 1, TRUE, TRUE) == EOF) fseek (fd, -1, SEEK_CUR);
		
		if (c2_streq (key, "mime_win_mode"))
		{
			REPORT (N_("Registering in the new configuration 'mime_window'"));
			gnome_config_set_int ("/cronosII/Appareance/mime_window", atoi (val));
			g_free (val);
			REPORT_RESULT (N_("Success."), TRUE);
		} else if (c2_streq (key, "toolbar"))
		{
			REPORT (N_("Registering in the new configuration 'toolbar'"));
			gnome_config_set_int ("/cronosII/Appareance/toolbar", atoi (val));
			g_free (val);
			REPORT_RESULT (N_("Success."), TRUE);
		} else if (c2_streq (key, "h_pan"))
		{
			REPORT (N_("Registering in the new configuration 'wm_hpan'"));
			gnome_config_set_int ("/cronosII/Appareance/wm_hpan", atoi (val));
			g_free (val);
			REPORT_RESULT (N_("Success."), TRUE);
		} else if (c2_streq (key, "v_pan"))
		{
			REPORT (N_("Registering in the new configuration 'wm_vpan'"));
			gnome_config_set_int ("/cronosII/Appareance/wm_hpan", atoi (val));
			g_free (val);
			REPORT_RESULT (N_("Success."), TRUE);
		} else if (c2_strneq (key, "clist_", 6))
		{
			gnome_config_push_prefix ("/cronosII/Appareance/wm_");
			REPORT (N_("Registering in the new configuration 'wm_clist'"));
			gnome_config_set_int (key, atoi (val));
			REPORT_RESULT (N_("Success."), TRUE);
			gnome_config_pop_prefix ();
			g_free (val);
		} else if (c2_streq (key, "main_window_width"))
		{
			REPORT (N_("Registering in the new configuration 'wm_width'"));
			gnome_config_set_int ("/cronosII/Appareance/wm_width", atoi (val));
			g_free (val);
			REPORT_RESULT (N_("Success."), TRUE);
		} else if (c2_streq (key, "main_window_height"))
		{
			REPORT (N_("Registering in the new configuration 'wm_height'"));
			gnome_config_set_int ("/cronosII/Appareance/wm_height", atoi (val));
			g_free (val);
			REPORT_RESULT (N_("Success."), TRUE);
		} else if (c2_streq (key, "showable_headers:preview"))
		{
			REPORT (N_("Registering in the new configuration 'showable_headers:preview'"));
			gnome_config_set_int ("/cronosII/Appareance/showable_headers:preview", atoi (val));
			g_free (val);
			REPORT_RESULT (N_("Success."), TRUE);
		} else if (c2_streq (key, "showable_headers:message"))
		{
			REPORT (N_("Registering in the new configuration 'showable_headers:message'"));
			gnome_config_set_int ("/cronosII/Appareance/showable_headers:message", atoi (val));
			g_free (val);
			REPORT_RESULT (N_("Success."), TRUE);
		} else if (c2_streq (key, "showable_headers:compose"))
		{
			REPORT (N_("Registering in the new configuration 'showable_headers:compose'"));
			gnome_config_set_int ("/cronosII/Appareance/showable_headers:compose", atoi (val));
			g_free (val);
			REPORT_RESULT (N_("Success."), TRUE);
		} else if (c2_streq (key, "showable_headers:save"))
		{
			REPORT (N_("Registering in the new configuration 'showable_headers:save'"));
			gnome_config_set_int ("/cronosII/Appareance/showable_headers:save", atoi (val));
			g_free (val);
			REPORT_RESULT (N_("Success."), TRUE);
		} else if (c2_streq (key, "showable_headers:print"))
		{
			REPORT (N_("Registering in the new configuration 'showable_headers:print'"));
			gnome_config_set_int ("/cronosII/Appareance/showable_headers:print", atoi (val));
			g_free (val);
			REPORT_RESULT (N_("Success."), TRUE);
		} else if (c2_streq (key, "font_read"))
		{
			REPORT (N_("Registering in the new configuration 'font_read'"));
			gnome_config_set_string ("/cronosII/Fonts/font_read", val);
			g_free (val);
			REPORT_RESULT (N_("Success."), TRUE);
		} else if (c2_streq (key, "font_unread"))
		{
			REPORT (N_("Registering in the new configuration 'font_unread'"));
			gnome_config_set_string ("/cronosII/Fonts/font_unread", val);
			g_free (val);
			REPORT_RESULT (N_("Success."), TRUE);
		} else if (c2_streq (key, "font_body"))
		{
			REPORT (N_("Registering in the new configuration 'font_body'"));
			gnome_config_set_string ("/cronosII/Fonts/font_body", val);
			g_free (val);
			REPORT_RESULT (N_("Success."), TRUE);
		} else if (c2_streq (key, "title"))
		{
			REPORT (N_("Registering in the new configuration 'app_title'"));
			gnome_config_set_string ("/cronosII/Appareance/app_title", val);
			g_free (val);
			REPORT_RESULT (N_("Success."), TRUE);
		}
		g_free (key);
	}
	
	REPORT (N_("Saving the new configuration"));
	gnome_config_sync ();
	REPORT_RESULT (N_("Success."), TRUE);

	return TRUE;
}

void
c2_install_new (void) {
	GtkWidget *vbox, *scroll;
	GtkStyle *style;
	GdkColor druidpage1_bg_color = { 0, 0, 0, 257 };
	GdkColor druidpage1_textbox_color = { 0, 17219, 17219, 17219 };
	GdkColor druidpage1_logo_bg_color = { 0, 257, 257, 257 };
	GdkColor druidpage1_title_color = { 0, 65535, 37265, 14649 };
	GdkColor druidpage1_text_color = { 0, 65535, 51143, 10023 };
  
	window = gtk_window_new (GTK_WINDOW_DIALOG);
	gtk_widget_realize (window);
	gtk_window_set_title (GTK_WINDOW (window), _("Cronos II Configuration Kit"));
	gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
	gtk_signal_connect (GTK_OBJECT (window), "delete_event",
						GTK_SIGNAL_FUNC (on_close), NULL);

	style = gtk_widget_get_default_style ();
	error_xpm = gdk_pixmap_create_from_xpm_d (window->window, &error_mask,
			&style->bg[GTK_STATE_NORMAL],
			mini_error_xpm);
	success_xpm = gdk_pixmap_create_from_xpm_d (window->window, &success_mask,
			&style->bg[GTK_STATE_NORMAL],
			mini_about_xpm);

	druid = gnome_druid_new ();
	gtk_container_add (GTK_CONTAINER (window), druid);
	gtk_widget_show (druid);
	gtk_signal_connect (GTK_OBJECT (druid), "cancel",
						GTK_SIGNAL_FUNC (on_close), NULL);

	druidpage1 = gnome_druid_page_start_new ();
	gnome_druid_append_page (GNOME_DRUID (druid), GNOME_DRUID_PAGE (druidpage1));
	gtk_widget_show (druidpage1);
	gnome_druid_set_page (GNOME_DRUID (druid), GNOME_DRUID_PAGE (druidpage1));
	gnome_druid_page_start_set_bg_color (GNOME_DRUID_PAGE_START (druidpage1), &druidpage1_bg_color);
	gnome_druid_page_start_set_textbox_color (GNOME_DRUID_PAGE_START (druidpage1), &druidpage1_textbox_color);
	gnome_druid_page_start_set_logo_bg_color (GNOME_DRUID_PAGE_START (druidpage1), &druidpage1_logo_bg_color);
	gnome_druid_page_start_set_title_color (GNOME_DRUID_PAGE_START (druidpage1), &druidpage1_title_color);
	gnome_druid_page_start_set_text_color (GNOME_DRUID_PAGE_START (druidpage1), &druidpage1_text_color);
	gnome_druid_page_start_set_title (GNOME_DRUID_PAGE_START (druidpage1), _("Welcome to Cronos II 0.3.0"));
	gnome_druid_page_start_set_text (GNOME_DRUID_PAGE_START (druidpage1),
			_("Since this is the first time you ever used Cronos II 0.3.0\n"
			  "your configuration must be created, in order to be able\n"
			  "to use all of the new functions Cronos II has.\n"
			  "\n"
			  "Through this simple wizard you will be able to finish the\n"
			  "installation of Cronos II in your system.\n"
			  "\n"
			  "To start the process, press the 'Next' button."));
	gtk_signal_connect (GTK_OBJECT (druidpage1), "next",
						GTK_SIGNAL_FUNC (on_page1_next_clicked), NULL);

	druidpage2 = gnome_druid_page_standard_new_with_vals ("", NULL);
	gtk_widget_show (druidpage2);
	gnome_druid_append_page (GNOME_DRUID (druid), GNOME_DRUID_PAGE (druidpage2));
	gnome_druid_page_standard_set_bg_color (GNOME_DRUID_PAGE_STANDARD (druidpage2), &druidpage1_bg_color);
	gnome_druid_page_standard_set_logo_bg_color (GNOME_DRUID_PAGE_STANDARD (druidpage2), &druidpage1_logo_bg_color);
	gnome_druid_page_standard_set_title_color (GNOME_DRUID_PAGE_STANDARD (druidpage2), &druidpage1_title_color);
	gnome_druid_page_standard_set_title (GNOME_DRUID_PAGE_STANDARD (druidpage2), _("Cronos II 0.3.0"));

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox);
	gtk_box_pack_start (GTK_BOX (GNOME_DRUID_PAGE_STANDARD (druidpage2)->vbox), vbox, TRUE, TRUE, 0);

	scroll = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_show (scroll);
	gtk_box_pack_start (GTK_BOX (vbox), scroll, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (scroll), 10);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

	clist = gtk_clist_new (2);
	gtk_widget_show (clist);
	gtk_container_add (GTK_CONTAINER (scroll), clist);
	gtk_clist_set_column_width (GTK_CLIST (clist), 0, 30);
	gtk_clist_set_column_width (GTK_CLIST (clist), 1, 200);
	gtk_clist_column_titles_hide (GTK_CLIST (clist));

	druidpage3 = gnome_druid_page_finish_new ();
	gtk_widget_show (druidpage3);
	gnome_druid_append_page (GNOME_DRUID (druid), GNOME_DRUID_PAGE (druidpage3));
	gnome_druid_page_finish_set_bg_color (GNOME_DRUID_PAGE_FINISH (druidpage3), &druidpage1_bg_color);
	gnome_druid_page_finish_set_textbox_color (GNOME_DRUID_PAGE_FINISH (druidpage3), &druidpage1_textbox_color);
	gnome_druid_page_finish_set_logo_bg_color (GNOME_DRUID_PAGE_FINISH (druidpage3), &druidpage1_logo_bg_color);
	gnome_druid_page_finish_set_title_color (GNOME_DRUID_PAGE_FINISH (druidpage3), &druidpage1_title_color);
	gnome_druid_page_finish_set_text_color (GNOME_DRUID_PAGE_FINISH (druidpage3), &druidpage1_text_color);
	gnome_druid_page_finish_set_title (GNOME_DRUID_PAGE_FINISH (druidpage3), _("Installation complete"));
	gnome_druid_page_finish_set_text (GNOME_DRUID_PAGE_FINISH (druidpage3),
				_("The installation of Cronos II in your system is complete, you can\n"
				  "now start it by clicking the 'Finish' button or by accessing your\n"
				  "Gnome menu in the Net menu.\n"
				  "\n"
				  "Thanks for choosing Cronos II.\n"
				  "The Cronos II Developers Team."));
	gtk_signal_connect (GTK_OBJECT (druidpage3), "finish",
						GTK_SIGNAL_FUNC (on_close), NULL);

	gtk_widget_show (window);
	
	gtk_main ();
}

static void
on_close (void) {
	gtk_widget_destroy (window);
	gtk_main_quit ();
}
