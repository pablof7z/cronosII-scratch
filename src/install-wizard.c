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
#include <gnome.h>
#include <glib.h>
#include <config.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <libmodules/mailbox.h>
#include <libmodules/utils.h>
#include <libmodules/error.h>
#include <libmodules/db.h>
#include <libmodules/date-utils.h>

#include "c2-app.h"

#include "xpm/mini_about.xpm"
#include "xpm/mini_error.xpm"

#if TRUE
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
#else
#define REPORT(x)
#define RESULT(status)
#define REPORT_RESULT(x, status)
#endif

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
create_new_configuration (void);

static gboolean
import_old_configuration (void);

static void
on_page1_next_clicked (void)
{
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
		create_new_configuration ();
	}

	process_done = TRUE;
}

static gboolean
detect_old_configuration (void)
{
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

enum
{
	SOURCE,
	TEMP
};

static gchar *
upgrade_database (const gchar *mbox)
{
	gchar *path[2];
	FILE *fd;
	GList *l;
	C2DB *db;
	gint i;

	c2_return_val_if_fail (mbox, _("Wrong upgrade_database parameters"), C2EDATA);
	
	/* Calculate paths */
	path[SOURCE] = g_strdup_printf ("%s" G_DIR_SEPARATOR_S ".CronosII" G_DIR_SEPARATOR_S
									"%s.mbx" G_DIR_SEPARATOR_S "index", g_get_home_dir (), mbox);
	path[TEMP] = c2_get_tmp_file ();

	/* Open files */
	if (!(fd = fopen (path[TEMP], "wt")))
	{
		g_free (path[SOURCE]);
		g_free (path[TEMP]);
		c2_error_set (-errno);
		return g_strdup_printf (_("Unable to open index file: %s"), c2_error_get (c2_errno));
	}

	/* Load db */
	if (!(db = c2_db_load (mbox, C2_METHOD_CRONOSII)))
	{
		g_free (path[SOURCE]);
		g_free (path[TEMP]);
		return g_strdup_printf (_("Unable to load db: %s"), c2_error_get (c2_errno));
	}

	/* Process */
	for (l = db->head, i = 0; l != NULL; l = g_list_next (l))
	{
		/* Get the message */
		C2Message *message = c2_db_message_get (db, i++);
		gchar *from, *strdate, *account;
		time_t date;
		C2DBNode *node = l->data;
		
		if (!message)
			continue;
		
		/* Get fields and write them */
		from = c2_message_get_header_field (message, NULL, "From:");
		account = c2_message_get_header_field (message, NULL, "X-CronosII-Account:");
		strdate = c2_message_get_header_field (message, NULL, "Date:");

		/* Get the date */
		if (strdate)
		{
			if ((date = c2_date_parse (strdate)) < 0)
				if ((date = c2_date_parse_fmt2 (strdate)) < 0)
					date = time (NULL);
		} else
			date = time (NULL);
		
		fprintf (fd, "%c\r%s\r%s\r%s\r%s\r%d\r%s\r%d\n",
					node->status, (node->marked) ? "MARK" : "", "", node->headers[0],
					node->headers[1], (gint) date, account, node->mid);

		g_free (from);
		g_free (account);
		g_free (strdate);
	}
	fclose (fd);

	/* Unload the database */
	c2_db_unload (db);

	/* Move the file to the right location */
	if (c2_file_binary_move (path[TEMP], path[SOURCE]))
	{
		gchar *err = g_strdup_printf (_("Unable to move the temporary file to the correct location: %s"),
										c2_error_get (c2_errno));
		return err;
	}

	g_free (path[TEMP]);
	g_free (path[SOURCE]);

	return NULL;
}

static gboolean
create_new_configuration (void)
{
	gchar *home = g_get_home_dir ();
	gchar *directory_paths[] = {
		g_strdup_printf (".CronosII"),
		g_strdup_printf (".CronosII" G_DIR_SEPARATOR_S "%s.mbx", MAILBOX_INBOX),
		g_strdup_printf (".CronosII" G_DIR_SEPARATOR_S "%s.mbx", MAILBOX_OUTBOX),
		g_strdup_printf (".CronosII" G_DIR_SEPARATOR_S "%s.mbx", MAILBOX_QUEUE),
		g_strdup_printf (".CronosII" G_DIR_SEPARATOR_S "%s.mbx", MAILBOX_GARBAGE),
		g_strdup_printf (".CronosII" G_DIR_SEPARATOR_S "%s.mbx", MAILBOX_DRAFTS),
		NULL
	};
	gchar *file_paths[] = {
		g_strdup_printf (".CronosII" G_DIR_SEPARATOR_S "%s.mbx" G_DIR_SEPARATOR_S "index",  MAILBOX_INBOX),
		g_strdup_printf (".CronosII" G_DIR_SEPARATOR_S "%s.mbx" G_DIR_SEPARATOR_S "index",  MAILBOX_OUTBOX),
		g_strdup_printf (".CronosII" G_DIR_SEPARATOR_S "%s.mbx" G_DIR_SEPARATOR_S "index",  MAILBOX_QUEUE),
		g_strdup_printf (".CronosII" G_DIR_SEPARATOR_S "%s.mbx" G_DIR_SEPARATOR_S "index",  MAILBOX_GARBAGE),
		g_strdup_printf (".CronosII" G_DIR_SEPARATOR_S "%s.mbx" G_DIR_SEPARATOR_S "index",  MAILBOX_DRAFTS),
		NULL
	};
	gchar *path;
	gint i;


	REPORT (N_("Creating internal directory structure"));
	for (i = 0; (path = directory_paths[i]) != NULL; i++)
	{
		path = g_strdup_printf ("%s" G_DIR_SEPARATOR_S "%s", home, path);
		if (mkdir (path, 0700) < 0)
		{
			c2_error_set (-errno);
			REPORT_RESULT (c2_error_get (c2_errno), FALSE);
			g_free (path);
			return -1;
		}
		g_free (path);
		g_free (directory_paths[i]);
	}
	REPORT_RESULT (N_("Success."), TRUE);

	REPORT (N_("Creating internal file structure"));
	for (i = 0; (path = file_paths[i]) != NULL; i++)
	{
		FILE *fd;
		path = g_strdup_printf ("%s" G_DIR_SEPARATOR_S "%s", home, path);
		if (!(fd = fopen (path, "wt")))
		{
			c2_error_set (-errno);
			REPORT_RESULT (c2_error_get (c2_errno), FALSE);
			g_free (path);
			return -1;
		}
		fclose (fd);
		g_free (path);
		g_free (file_paths[i]);
	}
	REPORT_RESULT (N_("Success."), TRUE);

	REPORT (N_("Configuring Cronos II"));
	gnome_config_set_string ("/cronosII/CronosII/Version", VERSION);
	gnome_config_set_string ("/cronosII/Mailboxes/0", "");
	gnome_config_set_string ("/cronosII/Mailboxes/0::Name", MAILBOX_INBOX);
	gnome_config_set_int ("/cronosII/Mailboxes/0::Id", 0);
	gnome_config_set_int ("/cronosII/Mailboxes/0::Parent Id", 0);
	gnome_config_set_string ("/cronosII/Mailboxes/1", "");
	gnome_config_set_string ("/cronosII/Mailboxes/1::Name", MAILBOX_OUTBOX);
	gnome_config_set_int ("/cronosII/Mailboxes/1::Id", 1);
	gnome_config_set_int ("/cronosII/Mailboxes/1::Parent Id", 1);
	gnome_config_set_string ("/cronosII/Mailboxes/2", "");
	gnome_config_set_string ("/cronosII/Mailboxes/2::Name", MAILBOX_QUEUE);
	gnome_config_set_int ("/cronosII/Mailboxes/2::Id", 2);
	gnome_config_set_int ("/cronosII/Mailboxes/2::Parent Id", 2);
	gnome_config_set_string ("/cronosII/Mailboxes/3", "");
	gnome_config_set_string ("/cronosII/Mailboxes/3::Name", MAILBOX_GARBAGE);
	gnome_config_set_int ("/cronosII/Mailboxes/3::Id", 3);
	gnome_config_set_int ("/cronosII/Mailboxes/3::Parent Id", 3);
	gnome_config_set_string ("/cronosII/Mailboxes/4", "");
	gnome_config_set_string ("/cronosII/Mailboxes/4::Name", MAILBOX_DRAFTS);
	gnome_config_set_int ("/cronosII/Mailboxes/4::Id", 4);
	gnome_config_set_int ("/cronosII/Mailboxes/4::Parent Id", 4);
	gnome_config_set_int ("/cronosII/Address Book/init", C2_INIT_ADDRBOOK_AT_START);
	gnome_config_set_bool ("/cronosII/Options/empty_garbage", FALSE);
	gnome_config_set_bool ("/cronosII/Options/check_at_start", FALSE);
	gnome_config_set_bool ("/cronosII/Options/use_outbox", TRUE);
	gnome_config_set_bool ("/cronosII/Persistent SMTP/use", FALSE);
	gnome_config_set_int ("/cronosII/Options/check_timeout", 10);
	gnome_config_set_int ("/cronosII/Options/message_size_limit", 0);
	gnome_config_set_int ("/cronosII/Timeout/net", 20);
	gnome_config_set_int ("/cronosII/Timeout/mark_as_read", 2);
	gnome_config_set_string ("/cronosII/Options/prepend_character", "> ");
	gnome_config_set_string ("/cronosII/Colors/reply_original_message", "0x0x65535");
	gnome_config_set_string ("/cronosII/Colors/misc_body", "0x0x0");
	gnome_config_set_int ("/cronosII/Appareance/mime_window", C2_MIME_WINDOW_AUTOMATIC);
	gnome_config_set_int ("/cronosII/Appareance/toolbar", GTK_TOOLBAR_BOTH);
	gnome_config_set_int ("/cronosII/Appareance/wm_hpan", 120);
	gnome_config_set_int ("/cronosII/Appareance/wm_vpan", 120);
	gnome_config_push_prefix ("/cronosII/Appareance/wm_clist::");
	gnome_config_set_int ("/cronosII/Appareance/wm_clist::0", 20);
	gnome_config_set_int ("/cronosII/Appareance/wm_clist::1", 10);
	gnome_config_set_int ("/cronosII/Appareance/wm_clist::2", 10);
	gnome_config_set_int ("/cronosII/Appareance/wm_clist::3", 150);
	gnome_config_set_int ("/cronosII/Appareance/wm_clist::4", 150);
	gnome_config_set_int ("/cronosII/Appareance/wm_clist::5", 100);
	gnome_config_set_int ("/cronosII/Appareance/wm_clist::6", 65);
	gnome_config_set_int ("/cronosII/Appareance/wm_clist::7", 15);
	gnome_config_set_int ("/cronosII/Appareance/wm_width", gdk_screen_width ()-40);
	gnome_config_set_int ("/cronosII/Appareance/wm_height", gdk_screen_height ()-40);
	gnome_config_set_int ("/cronosII/Appareance/showable_headers:preview", 12);
	gnome_config_set_int ("/cronosII/Appareance/showable_headers:message", 63);
	gnome_config_set_int ("/cronosII/Appareance/showable_headers:compose", 61);
	gnome_config_set_int ("/cronosII/Appareance/showable_headers:save", 0);
	gnome_config_set_int ("/cronosII/Appareance/showable_headers:print", 0);
	gnome_config_set_string ("/cronosII/Fonts/font_read", "-b&h-lucida-medium-r-normal-*-*-100-*-*-p-*-iso8859-1");
	gnome_config_set_string ("/cronosII/Fonts/font_unread", "-b&h-lucida-bold-r-normal-*-*-100-*-*-p-*-iso8859-1");
	gnome_config_set_string ("/cronosII/Fonts/font_body", "-adobe-times-medium-r-normal-*-*-140-*-*-p-*-iso8859-1");
	gnome_config_set_string ("/cronosII/Appareance/app_title", "%a v.%v - %M");
	gnome_config_sync ();
	REPORT_RESULT (N_("Success."), TRUE);
	return TRUE;
}

typedef struct _OldMailbox
{
	gchar *name;
	gint id;
	gint parent_id;
	struct _OldMailbox *next;
	struct _OldMailbox *child;
} OldMailbox;

static OldMailbox *
old_mailbox_parse (const gchar *info)
{
	OldMailbox *mbox;
	gchar *buf;

	c2_return_val_if_fail (info, NULL, C2EDATA);

	mbox = g_new0 (OldMailbox, 1);

	buf = c2_str_get_word (0, info, '\r');
	mbox->id = atoi (buf);
	g_free (buf);

	mbox->name = c2_str_get_word (1, info, '\r');

	buf = c2_str_get_word (2, info, '\r');
	mbox->parent_id = atoi (buf);
	g_free (buf);

	mbox->next = NULL;
	mbox->child = NULL;

	return mbox;
}

static OldMailbox *
old_mailbox_search_id (OldMailbox *head, gint id)
{
	OldMailbox *l, *s;
	gint i = 0;

	for (l = head; l != NULL; l = l->next, i++)
	{
		if (l->id == id)
			return l;
		if (l->child)
			if ((s = old_mailbox_search_id (l->child, id)) != NULL)
				return s;
	}

	return NULL;
}

static OldMailbox *
old_mailbox_append (OldMailbox *head, OldMailbox *mailbox)
{
	OldMailbox *l;
	
	c2_return_val_if_fail (mailbox, NULL, C2EDATA);

	if (!head)
		return mailbox;

	if (mailbox->id == mailbox->parent_id)
	{
		/* Insert must be done in the top of the tree */
		for (l = head; l->next != NULL; l = l->next);
		l->next = mailbox;
	} else
	{
		OldMailbox *parent = old_mailbox_search_id (head, mailbox->parent_id);

		if (parent)
		{
			if (parent->child)
			{
				OldMailbox *s;
				
				for (s = parent->child; s->next != NULL; s = s->next);
				s->next = mailbox;
			} else
				parent->child = mailbox;
		}
	}

	return head;
}

static C2Mailbox *
old_mailbox_upgrade_node (OldMailbox *old_node, C2Mailbox *new_head, C2Mailbox *new_parent)
{
	C2Mailbox *new_node;
	
	c2_return_val_if_fail (old_node, NULL, C2EDATA);
	
	new_node = g_new0 (C2Mailbox, 1);
	new_node->name = g_strdup (old_node->name);
	new_node->id = c2_mailbox_next_id (new_head, new_parent);
	new_node->child = NULL;
	new_node->next = NULL;
	
	return new_node;
}

static C2Mailbox *
old_mailbox_upgrade (OldMailbox *old_head, C2Mailbox *new_head, C2Mailbox *new_parent)
{
	C2Mailbox *new_l;
	OldMailbox *old_l;
	
	c2_return_val_if_fail (old_head, NULL, C2EDATA);

	for (old_l = old_head; old_l != NULL; old_l = old_l->next)
	{
		new_l = old_mailbox_upgrade_node (old_l, new_head, new_parent);
		new_head = c2_mailbox_append (new_head, new_l);
		if (old_l->child)
			old_mailbox_upgrade (old_l->child, new_head, new_l);
	}

	return new_head;
}

static void
new_mailbox_write (C2Mailbox *head)
{
	C2Mailbox *l;
	static gint i = 0;
	
	c2_return_if_fail (head, C2EDATA);

	for (l = head; l != NULL; l = l->next)
	{
		gchar *prefix = g_strdup_printf ("/cronosII/Mailboxes/%d", i++);
		
		gnome_config_push_prefix (prefix);
		gnome_config_set_string ("::Name", l->name);
		gnome_config_set_string ("::Id", l->id);
		gnome_config_pop_prefix ();
		g_free (prefix);
		if (l->child)
			new_mailbox_write (l->child);
	}
}

static gboolean
import_old_configuration (void)
{
	gchar *home = g_get_home_dir ();
	gchar *path = g_strdup_printf ("%s" G_DIR_SEPARATOR_S ".CronosII" G_DIR_SEPARATOR_S "cronos.conf", home);
	gchar *key, *val;
	FILE *fd;

	OldMailbox *mailboxes_head = NULL;
	C2Mailbox *new_mailbox_head = NULL;

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

	REPORT (N_("Registering in the new configuration 'Version'"));
	gnome_config_set_string ("/cronosII/CronosII/Version", VERSION);
	REPORT_RESULT (N_("Success."), TRUE);

	for (;;)
	{
		if ((key = c2_fd_get_word (fd)) == NULL) break;
		if ((val = c2_fd_get_word (fd)) == NULL) break;
		if (c2_fd_move_to (fd, '\n', 1, TRUE, TRUE) == EOF) fseek (fd, -1, SEEK_CUR);
		
		if (c2_streq (key, "mailbox"))
		{
			OldMailbox *mbox;
			gchar *err;
			
			REPORT (N_("Parsing a mailbox"));
			mbox = old_mailbox_parse (val);
			if (!mbox)
			{
				REPORT_RESULT (N_("Failed."), FALSE);
				g_free (val);
				continue;
			}
			mailboxes_head = old_mailbox_append (mailboxes_head, mbox);
			REPORT_RESULT (N_("Success."), TRUE);

			REPORT (N_("Upgrading database"));
			if ((err = upgrade_database (mbox->name)))
			{
				REPORT_RESULT (err, FALSE);
			} else
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
	
	REPORT (N_("Upgrading mailboxes tree."));
	new_mailbox_head = old_mailbox_upgrade (mailboxes_head, NULL, NULL);
	new_mailbox_write (new_mailbox_head);
	REPORT_RESULT (N_("Success."));
	

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
			gnome_config_push_prefix ("/cronosII/Appareance/wm_clist::");
			REPORT (N_("Registering in the new configuration 'wm_clist'"));
			gnome_config_set_int (key+6, atoi (val));
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
c2_install_new (void)
{
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
on_close (void)
{
	if (!process_done)
	{
		GtkWidget *_window = gnome_question_dialog (_(
					"The configuration wasn't generated.\n"
					"Quit anyway?"), NULL, NULL);
		if (gnome_dialog_run_and_close (GNOME_DIALOG (_window)) == 0)
		{
			gtk_widget_destroy (window);
			gtk_main_quit ();
			exit (0);
		} else
			return;
	}
	gtk_widget_destroy (window);
	gtk_main_quit ();
}
