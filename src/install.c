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
#include <gnome.h>
#include <glade/glade.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#include <libcronosII/error.h>
#include <libcronosII/utils.h>

#include "preferences.h"
#include "widget-application.h"

static void
on_start_btn_clicked						(GtkWidget *btn, GladeXML *xml);

static void
on_druid_cancel								(GtkWidget *druid, GladeXML *xml);

static void
on_druid_page1_prepare						(GtkWidget *druid_page, GtkWidget *druid, GladeXML *xml);

static void
on_druid_finish_prepare						(GtkWidget *druid_page, GtkWidget *druid, GladeXML *xml);

static void
on_druid_page_finish_finish					(GtkWidget *druid_page, GtkWidget *druid, GladeXML *xml);

static gboolean
on_wnd_install_delete_event					(GtkWidget *wnd, GdkEvent *e, GladeXML *xml);

/* Actions */
typedef struct _C2InstallAction C2InstallAction;
typedef enum _C2InstallActionType C2InstallActionType;
#define C2_INSTALL_ACTION_END C2_INSTALL_ACTION_LAST, NULL, NULL, NULL, FALSE

enum _C2InstallActionType
{
	C2_INSTALL_ACTION_DIR_RM,
	C2_INSTALL_ACTION_DIR_MK,
	C2_INSTALL_ACTION_MAILBOX_MK,
	C2_INSTALL_ACTION_FILE_RM,
	C2_INSTALL_ACTION_FILE_MK,
	C2_INSTALL_ACTION_FILE_CP,
	C2_INSTALL_ACTION_LAST
};

struct C2InstallAction
{
	C2InstallActionType type;
	const gchar *argv1;
	const gchar *argv2;
	const gchar *argv3;
	gboolean required;
} actions[] =
{
	{
		C2_INSTALL_ACTION_DIR_RM,
		"~/.c2", NULL, NULL,
		FALSE
	},
	{
		C2_INSTALL_ACTION_DIR_MK,
		"~/.c2", NULL, NULL,
		TRUE
	},
	{
		C2_INSTALL_ACTION_DIR_MK,
		"~/.c2/uidl", NULL, NULL,
		TRUE
	},
	{
		C2_INSTALL_ACTION_FILE_RM,
		"~/.gnome_private/"PACKAGE, NULL, NULL,
		FALSE
	},
	{
		C2_INSTALL_ACTION_FILE_CP,
		PKGDATADIR "/default/cronosII.gnome",
		"~/.gnome_private/"PACKAGE,
		NULL, TRUE
	},
	{
		C2_INSTALL_ACTION_MAILBOX_MK,
		C2_MAILBOX_N_INBOX, NULL, NULL,
		TRUE
	},
	{
		C2_INSTALL_ACTION_MAILBOX_MK,
		C2_MAILBOX_N_OUTBOX, NULL, NULL,
		TRUE
	},
	{
		C2_INSTALL_ACTION_MAILBOX_MK,
		C2_MAILBOX_N_SENT_ITEMS, NULL, NULL,
		TRUE
	},
	{
		C2_INSTALL_ACTION_MAILBOX_MK,
		C2_MAILBOX_N_TRASH, NULL, NULL,
		TRUE
	},
	{
		C2_INSTALL_ACTION_MAILBOX_MK,
		C2_MAILBOX_N_DRAFTS, NULL, NULL,
		TRUE
	},
	{
		C2_INSTALL_ACTION_FILE_CP,
		PKGDATADIR "/default/default.elm",
		/* Translators: Change here just the name
		 * of the Inbox mailbox to whatever name
		 * your language uses, note that it MUST
		 * be the same name as when you translated
		 * before.
		 */
		N_("~/.c2/Inbox.mbx/1"),
		NULL, TRUE
	},
	{
		C2_INSTALL_ACTION_FILE_CP,
		PKGDATADIR "/default/default.index",
		/* Translators: Same here, just change the
		 * Inbox name to whatever name you used
		 * before.
		 */
		N_("~/.c2/Inbox.mbx/index"),
		NULL, TRUE
	},
	{
		C2_INSTALL_ACTION_END
	}
};

/* This function WILL run on its own gtk_main() */
void
c2_install_new (void)
{
	GladeXML *xml;
	GtkWidget *widget;
	GtkStyle *style;
	GdkColor dark_blue = { 0, 0x1100, 0x1400, 0x7c00 };
	GdkColor light_blue = { 0, 0x1e00, 0x0c00, 0xbc00 };
	GdkColor white = { 0, 0xffff, 0xffff, 0xffff };
	GdkColor orange = { 0, 0xf400, 0xa000, 0x1600 };

	xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("install"), "wnd_install");

	widget = glade_xml_get_widget (xml, "start_btn");
	style = gtk_style_copy (gtk_widget_get_style (widget));
	gdk_color_alloc (gdk_colormap_get_system (), &dark_blue);
	gdk_color_alloc (gdk_colormap_get_system (), &light_blue);
	gdk_color_alloc (gdk_colormap_get_system (), &white);
	style->bg[GTK_STATE_NORMAL] = dark_blue;
	style->bg[GTK_STATE_ACTIVE] = light_blue;
	style->bg[GTK_STATE_PRELIGHT] = light_blue;
	style->bg[GTK_STATE_SELECTED] = dark_blue;
	style->bg[GTK_STATE_INSENSITIVE] = dark_blue;
	style->fg[0] = white;
	style->fg[1] = white;
	style->fg[2] = white;
	style->fg[3] = white;
	style->fg[4] = white;
	style->font = gdk_font_load ("-adobe-helvetica-bold-r-normal-*-*-120-*-*-p-*-iso8859-1");
	gtk_widget_set_style (widget, style);
	gtk_widget_set_style (GTK_BIN (widget)->child, style);
	gtk_signal_connect (GTK_OBJECT (widget), "clicked",
						GTK_SIGNAL_FUNC (on_start_btn_clicked), xml);

	widget = glade_xml_get_widget (xml, "druid");
	gtk_signal_connect (GTK_OBJECT (widget), "cancel",
						GTK_SIGNAL_FUNC (on_druid_cancel), xml);

	widget = glade_xml_get_widget (xml, "druid_page1");
	gtk_signal_connect (GTK_OBJECT (widget), "prepare",
						GTK_SIGNAL_FUNC (on_druid_page1_prepare), xml);

	widget = glade_xml_get_widget (xml, "druid_page_finish");
	gtk_signal_connect (GTK_OBJECT (widget), "prepare",
						GTK_SIGNAL_FUNC (on_druid_finish_prepare), xml);
	gtk_signal_connect (GTK_OBJECT (widget), "finish",
						GTK_SIGNAL_FUNC (on_druid_page_finish_finish), xml);

	widget = glade_xml_get_widget (xml, "wnd_install");
	gtk_signal_connect (GTK_OBJECT (widget), "delete_event",
						GTK_SIGNAL_FUNC (on_wnd_install_delete_event), xml);
	style = gtk_style_copy (gtk_widget_get_style (widget));
	gdk_color_alloc (gdk_colormap_get_system (), &orange);
	style->bg[GTK_STATE_NORMAL] = orange;
	gtk_widget_set_style (widget, style);
	
	if (gnome_preferences_get_dialog_centered ())
		gtk_window_set_position (GTK_WINDOW (widget), GTK_WIN_POS_CENTER);

	gtk_widget_show (widget);
}

static gint
dir_rm (const gchar *directory)
{
	DIR *dir;
	struct dirent *dentry;

	if (!(dir = opendir (directory)))
	{
		c2_error_set (-errno);
		return -1;
	}

	while ((dentry = readdir (dir)))
	{
		gchar *name;

		if (c2_streq (dentry->d_name, ".") ||
			c2_streq (dentry->d_name, ".."))
			continue;
		
		name = g_strdup_printf ("%s/%s", directory, dentry->d_name);

		if (c2_file_is_directory (name))
		{
			if (dir_rm (name) < 0)
			{
				g_free (name);
				return -1;
			}
		} else
		{
			if (unlink (name) < 0)
			{
				c2_error_set (-errno);
				g_free (name);
				return -1;
			}
		}
		g_free (name);
	}

	closedir (dir);

	if (rmdir (directory) < 0)
	{
		c2_error_set (-errno);
		return -1;
	}

	return 0;
}

static gint
dir_mk (const gchar *directory)
{
	if (mkdir (directory, 0700) < 0)
	{
		c2_error_set (-errno);
		return -1;
	}

	return 0;
}

static gint
mailbox_mk (const gchar *name, C2Mailbox **head)
{
	C2Mailbox *mailbox;
	gint config_id;
	gchar *query;
	
	if (!(mailbox = c2_mailbox_new_with_parent (head, name, NULL,
								C2_MAILBOX_CRONOSII, C2_MAILBOX_SORT_DATE, GTK_SORT_ASCENDING)))
		return -1;

	config_id = gnome_config_private_get_int_with_default ("/"PACKAGE"/Mailboxes/quantity=0", NULL)+1;
	query = g_strdup_printf ("/"PACKAGE"/Mailbox %d/", config_id);
	gnome_config_push_prefix (query);
				
	gnome_config_private_set_string ("name", mailbox->name);
	gnome_config_private_set_string ("id", mailbox->id);
	gnome_config_private_set_int ("type", mailbox->type);
	gnome_config_private_set_int ("sort_by", mailbox->sort_by);
	gnome_config_private_set_int ("sort_type", mailbox->sort_type);
	gnome_config_pop_prefix ();
	g_free (query);
				
	gnome_config_private_set_int ("/"PACKAGE"/Mailboxes/quantity", config_id);
	gnome_config_sync ();
	
	return 0;
}

static gint
file_rm (const gchar *file)
{
	if (unlink (file) < 0)
	{
		c2_error_set (-errno);
		return -1;
	}

	return 0;
}

static gint
file_cp (const gchar *from, const gchar *to)
{
	if (c2_file_binary_copy (from, to) < 0)
		return -1;
	return 0;
}

static void
on_start_btn_clicked_thread (GladeXML *xml)
{
	C2Mailbox *head = NULL;
	GtkWidget *log_clist;
	GtkWidget *error_label;
	GtkWidget *progress;
	GtkWidget *status_label;
	GtkWidget *druid, *error_page, *finish_page;
	gint i, retval, length;
	gchar *row[] = { NULL, NULL };
	gchar *argv1, *argv2, *argv3, *action_str, *error_str;

	gdk_threads_enter ();

	log_clist = glade_xml_get_widget (xml, "log_clist");
	error_label = glade_xml_get_widget (xml, "error_label");
	progress = glade_xml_get_widget (xml, "progress");
	status_label = glade_xml_get_widget (xml, "status_label");
	druid = glade_xml_get_widget (xml, "druid");
	error_page = glade_xml_get_widget (xml, "druid_page_error");
	finish_page = glade_xml_get_widget (xml, "druid_page_finish");

	/* Get the length */
	for (i = 0; actions[i].type != C2_INSTALL_ACTION_LAST; i++)
		;
	length = i;

	/* Configure the progress bar */
	gtk_progress_configure (GTK_PROGRESS (progress), 0, 0, length);
	gtk_progress_set_show_text (GTK_PROGRESS (progress), TRUE);
	gtk_progress_set_format_string (GTK_PROGRESS (progress), "%p%%");

	gtk_clist_freeze (GTK_CLIST (log_clist));
	for (i = 0;; i++)
	{
		/* Append an empty row */
		gtk_clist_append (GTK_CLIST (log_clist), row);

		/* Convert all args */
		if (actions[i].argv1)
			argv1 = c2_str_replace_all (actions[i].argv1, "~", g_get_home_dir ());
		else
			argv1 = NULL;
		if (actions[i].argv2)
			argv2 = c2_str_replace_all (actions[i].argv2, "~", g_get_home_dir ());
		else
			argv2 = NULL;
		if (actions[i].argv3)
			argv3 = c2_str_replace_all (actions[i].argv3, "~", g_get_home_dir ());
		else
			argv3 = NULL;
		
		/* Specific actions */
		switch (actions[i].type)
		{
			case C2_INSTALL_ACTION_DIR_RM:
				action_str = g_strdup_printf (_("Remove directory %s"), argv1);
				gtk_clist_set_text (GTK_CLIST (log_clist), GTK_CLIST (log_clist)->rows-1,
									0, action_str);
				gtk_label_set_text (GTK_LABEL (status_label), action_str);
				
				if ((retval = dir_rm (argv1) < 0))
				{
					/* Some error occur */
					if (c2_errno)
						error_str = g_strdup_printf (_("Failed: %s\n"), c2_error_get ());
					else
						error_str = g_strdup (_("Failed"));

					gtk_clist_set_text (GTK_CLIST (log_clist), GTK_CLIST (log_clist)->rows-1,
										1, error_str);

					if (actions[i].required)
						goto out_of_for;
					else
						g_free (error_str);
				} else
					gtk_clist_set_text (GTK_CLIST (log_clist), GTK_CLIST (log_clist)->rows-1,
										1, _("Success"));
				g_free (action_str);
				break;

			case C2_INSTALL_ACTION_DIR_MK:
				action_str = g_strdup_printf (_("Create directory %s"), argv1);
				gtk_clist_set_text (GTK_CLIST (log_clist), GTK_CLIST (log_clist)->rows-1,
									0, action_str);
				gtk_label_set_text (GTK_LABEL (status_label), action_str);
				
				if ((retval = dir_mk (argv1) < 0))
				{
					/* Some error occur */
					if (c2_errno)
						error_str = g_strdup_printf (_("Failed: %s\n"), c2_error_get ());
					else
						error_str = g_strdup (_("Failed"));

					gtk_clist_set_text (GTK_CLIST (log_clist), GTK_CLIST (log_clist)->rows-1,
										1, error_str);

					if (actions[i].required)
						goto out_of_for;
					else
						g_free (error_str);
				} else
					gtk_clist_set_text (GTK_CLIST (log_clist), GTK_CLIST (log_clist)->rows-1,
										1, _("Success"));
				g_free (action_str);
				break;

				
			case C2_INSTALL_ACTION_MAILBOX_MK:
				action_str = g_strdup_printf (_("Create mailbox «%s»"), argv1);
				gtk_clist_set_text (GTK_CLIST (log_clist), GTK_CLIST (log_clist)->rows-1,
									0, action_str);
				gtk_label_set_text (GTK_LABEL (status_label), action_str);
				
				retval = mailbox_mk (argv1, &head);
				if (retval < 0)
				{
					/* Some error occur */
					if (c2_errno)
						error_str = g_strdup_printf (_("Failed: %s\n"), c2_error_get ());
					else
						error_str = g_strdup (_("Failed"));

					gtk_clist_set_text (GTK_CLIST (log_clist), GTK_CLIST (log_clist)->rows-1,
										1, error_str);

					if (actions[i].required)
						goto out_of_for;
					else
						g_free (error_str);
				} else
					gtk_clist_set_text (GTK_CLIST (log_clist), GTK_CLIST (log_clist)->rows-1,
										1, _("Success"));
				g_free (action_str);
				break;

				
			case C2_INSTALL_ACTION_FILE_RM:
				action_str = g_strdup_printf (_("Remove file %s"), argv1);
				gtk_clist_set_text (GTK_CLIST (log_clist), GTK_CLIST (log_clist)->rows-1,
									0, action_str);
				gtk_label_set_text (GTK_LABEL (status_label), action_str);
				
				if ((retval = file_rm (argv1) < 0))
				{
					/* Some error occur */
					if (c2_errno)
						error_str = g_strdup_printf (_("Failed: %s\n"), c2_error_get ());
					else
						error_str = g_strdup (_("Failed"));

					gtk_clist_set_text (GTK_CLIST (log_clist), GTK_CLIST (log_clist)->rows-1,
										1, error_str);

					if (actions[i].required)
						goto out_of_for;
					else
						g_free (error_str);
				} else
					gtk_clist_set_text (GTK_CLIST (log_clist), GTK_CLIST (log_clist)->rows-1,
										1, _("Success"));
				g_free (action_str);
				break;


			case C2_INSTALL_ACTION_FILE_CP:
				action_str = g_strdup_printf (_("Copy %s to %s"), argv1, argv2);
				gtk_clist_set_text (GTK_CLIST (log_clist), GTK_CLIST (log_clist)->rows-1,
									0, action_str);
				gtk_label_set_text (GTK_LABEL (status_label), action_str);
				
				if ((retval = file_cp (argv1, argv2) < 0))
				{
					/* Some error occur */
					if (c2_errno)
						error_str = g_strdup_printf (_("Failed: %s\n"), c2_error_get ());
					else
						error_str = g_strdup (_("Failed"));

					gtk_clist_set_text (GTK_CLIST (log_clist), GTK_CLIST (log_clist)->rows-1,
										1, error_str);

					if (actions[i].required)
						goto out_of_for;
					else
						g_free (error_str);
				} else
					gtk_clist_set_text (GTK_CLIST (log_clist), GTK_CLIST (log_clist)->rows-1,
										1, _("Success"));
				g_free (action_str);
				break;
				
				
			case C2_INSTALL_ACTION_LAST:
				gtk_label_set_text (GTK_LABEL (status_label), _("Done."));
				goto out_of_for_with_no_error;
		}

		gtk_progress_set_value (GTK_PROGRESS (progress), i);

		gdk_threads_leave ();
		usleep (500000);
		gdk_threads_enter ();
	}
out_of_for_with_no_error:
	retval = 0;
	
out_of_for:
	gtk_progress_set_value (GTK_PROGRESS (progress), i);
	gtk_clist_thaw (GTK_CLIST (log_clist));

	if (retval < 0)
	{
		gchar *lblerror = g_strdup_printf ("%s: %s", action_str, error_str);

		gtk_label_set_text (GTK_LABEL (error_label), lblerror);
		g_free (lblerror);
		gnome_druid_set_page (GNOME_DRUID (druid), GNOME_DRUID_PAGE (error_page));
	} else
		gnome_druid_set_page (GNOME_DRUID (druid), GNOME_DRUID_PAGE (finish_page));

	gdk_threads_leave ();
}

static void
on_start_btn_clicked (GtkWidget *btn, GladeXML *xml)
{
	pthread_t thread;

	pthread_create (&thread, NULL, C2_PTHREAD_FUNC (on_start_btn_clicked_thread), xml);
}

static void
on_druid_cancel (GtkWidget *druid, GladeXML *xml)
{
	GtkWidget *wnd;

	wnd = glade_xml_get_widget (xml, "wnd_install");
	on_wnd_install_delete_event (wnd, NULL, xml);
}

static void
on_druid_page1_prepare (GtkWidget *druid_page, GtkWidget *druid, GladeXML *xml)
{
	gtk_widget_set_sensitive (GNOME_DRUID (druid)->next, FALSE);
}

static void
on_druid_finish_prepare (GtkWidget *druid_page, GtkWidget *druid, GladeXML *xml)
{
	gtk_widget_set_sensitive (GNOME_DRUID (druid)->back, FALSE);
	gtk_widget_set_sensitive (GNOME_DRUID (druid)->cancel, FALSE);
}

static void
on_druid_page_finish_finish (GtkWidget *druid_page, GtkWidget *druid, GladeXML *xml)
{
	GtkWidget *window;

	window = glade_xml_get_widget (xml, "wnd_install");

	gtk_widget_destroy (window);
	gtk_object_destroy (GTK_OBJECT (xml));

	gtk_main_quit ();
}

static gboolean
on_wnd_install_delete_event (GtkWidget *wnd, GdkEvent *e, GladeXML *xml)
{
	GladeXML *_xml;
	GtkWidget *dialog;
	gboolean retval = FALSE;

	_xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("install"), "dlg_confirm_exit");
	
	dialog = glade_xml_get_widget (_xml, "dlg_confirm_exit");

	gnome_dialog_set_parent (GNOME_DIALOG (dialog), GTK_WINDOW (wnd));
	gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
	
	switch (gnome_dialog_run_and_close (GNOME_DIALOG (dialog)))
	{
		case 1:
			retval = TRUE;
			break;
		case 0:
			retval = FALSE;
			break;		
	}

	gtk_object_destroy (GTK_OBJECT (_xml));
	
	if (!retval)
	{
		gtk_object_destroy (GTK_OBJECT (xml));
		exit (0);
	}
	
	return retval;
}
