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
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <libcronosII/error.h>
#include <libcronosII/utils.h>

#include "c2-app.h"
#include "main-window.h"

/**
 * c2_app_init
 * 
 * This function initializates the configuration
 * from the file.
 * 
 * Return Value:
 * 0 if success, 1 if there's an error.
 **/
gint
c2_app_init (void)
{
	c2_app.tooltips = gtk_tooltips_new ();
	c2_app.open_windows = NULL;

	c2_app.gdk_font_body = gdk_font_load (c2_app.font_body);
	c2_app.gdk_font_read = gdk_font_load (c2_app.font_read);
	c2_app.gdk_font_unread = gdk_font_load (c2_app.font_unread);

	return 0;
}

/**
 * c2_app_register_window
 * @window: A GtkWindow object.
 * 
 * This function will register a window
 * in the c2 internals.
 * This function MUST be called everytime a window
 * is created.
 **/
void
c2_app_register_window (GtkWindow *window)
{
	c2_return_if_fail (GTK_IS_WINDOW (window), C2EDATA);

	c2_app.open_windows = g_list_append (c2_app.open_windows, window);

	/* TODO Now we should update the Windows menu TODO */
}

/**
 * c2_app_unregister_window
 * @window: A GtkWindow object.
 * 
 * This function will unregister a window
 * in the c2 internals.
 * This function MUST be called everytime a window
 * is about to be destroied.
 **/
void
c2_app_unregister_window (GtkWindow *window)
{
	c2_return_if_fail (GTK_IS_WINDOW (window), C2EDATA);

	c2_app.open_windows = g_list_remove (c2_app.open_windows, window);

	/* TODO Now we should update the Windows menu TODO */
}

static gint
on_report_expirate (gpointer data)
{
	if (!pthread_mutex_trylock (&WMain.appbar_lock))
	{
		GtkWidget *appbar = glade_xml_get_widget (WMain.xml, "appbar");
		gnome_appbar_pop (GNOME_APPBAR (appbar));
		pthread_mutex_unlock (&WMain.appbar_lock);
	}

	return FALSE;
}
/**
 * c2_app_report
 * @msg: Message to display.
 * @severity: Severity of the report.
 *
 * This function reports the user of something
 * that happend. i.e. errors, warnings, etc.
 **/
void
c2_app_report (const gchar *msg, C2ReportSeverity severity)
{
	gchar *realmsg;
	
	c2_return_if_fail (msg, C2EDATA);

	/* TODO Here we should ask which way to make a report: Statusbar, dialog, etc. TODO 
	 * We assume statusbar */
	if (!pthread_mutex_trylock (&WMain.appbar_lock))
	{
		GtkWidget *appbar = glade_xml_get_widget (WMain.xml, "appbar");
		if (severity == C2_REPORT_WARNING || severity == C2_REPORT_ERROR)
			realmsg = g_strdup_printf ("%s: %s", msg, c2_error_get (c2_errno));
		else
			realmsg = g_strdup (msg);
		gnome_appbar_push (GNOME_APPBAR (appbar), msg);
		gtk_timeout_add (5000, on_report_expirate, NULL);
		g_free (realmsg);
		pthread_mutex_unlock (&WMain.appbar_lock);
	}
}

static gboolean
on_activity_update (void)
{
	if (!pthread_mutex_trylock (&WMain.appbar_lock))
	{
		pthread_mutex_unlock (&WMain.appbar_lock);
	} else
	{
		GtkWidget *appbar = glade_xml_get_widget (WMain.xml, "appbar");
		static gboolean adding = TRUE;
		gfloat value = gtk_progress_get_value (gnome_appbar_get_progress (GNOME_APPBAR (appbar)));
	
		if (adding)
			value += 0.05;
		else
			value -= 0.05;
		
		if (adding && value > 1)
		{
			value -= 0.05;
			adding = !adding;
		} else if (!adding && value < 0)
		{
			value += 0.05;
			adding = !adding;
		}
		gtk_progress_set_value (gnome_appbar_get_progress (GNOME_APPBAR (appbar)), value);
		return TRUE;
	}
	return FALSE;
}

void
c2_app_start_activity (GtkWidget *progress)
{
	if (!pthread_mutex_trylock (&WMain.appbar_lock))
	{
		GtkWidget *appbar = glade_xml_get_widget (WMain.xml, "appbar");
		gtk_progress_set_activity_mode (gnome_appbar_get_progress (GNOME_APPBAR (appbar)), TRUE);
		gtk_timeout_add (10, on_activity_update, NULL);
	}
}

void
c2_app_stop_activity (void)
{
	GtkWidget *appbar = glade_xml_get_widget (WMain.xml, "appbar");
	gtk_progress_set_activity_mode (gnome_appbar_get_progress (GNOME_APPBAR (appbar)), FALSE);
	gtk_progress_set_value (gnome_appbar_get_progress (GNOME_APPBAR (appbar)), 0);
	pthread_mutex_unlock (&WMain.appbar_lock);
}

void
c2_mailbox_tree_fill (C2Mailbox *head, GtkCTreeNode *node, GtkWidget *ctree, GtkWidget *window)
{
	C2Mailbox *current;
	GtkWidget *pixmap_closed;
	GtkWidget *pixmap_opened;
	GtkCTreeNode *_node;
	gchar *buf;
  
	c2_return_if_fail (head || ctree, C2EDATA);

	if (!node)
	{
		gtk_clist_freeze (GTK_CLIST (ctree));
		gtk_clist_clear (GTK_CLIST (ctree));
	}

	for (current = head; current; current = current->next)
	{
		if (c2_streq (current->name, MAILBOX_INBOX))
		{
			pixmap_closed = gnome_pixmap_new_from_file (DATADIR "/cronosII/pixmaps/inbox.png");
			pixmap_opened = pixmap_closed;
		} else if (c2_streq (current->name, MAILBOX_OUTBOX))
		{
			pixmap_closed = gnome_pixmap_new_from_file (DATADIR "/cronosII/pixmaps/outbox.png");
			pixmap_opened = pixmap_closed;
		} else if (c2_streq (current->name, MAILBOX_QUEUE))
		{
			pixmap_closed = gnome_pixmap_new_from_file (DATADIR "/cronosII/pixmaps/queue_mbox.png");
			pixmap_opened = pixmap_closed;
		} else if (c2_streq (current->name, MAILBOX_GARBAGE))
		{
			pixmap_closed = gnome_pixmap_new_from_file (DATADIR "/cronosII/pixmaps/garbage.png");
			pixmap_opened = pixmap_closed;
		} else if (c2_streq (current->name, MAILBOX_DRAFTS))
		{
			pixmap_closed = gnome_pixmap_new_from_file (DATADIR "/cronosII/pixmaps/drafts.png");
			pixmap_opened = pixmap_closed;
		} else
		{
			if (current->child)
			{
				pixmap_closed = gnome_pixmap_new_from_file (DATADIR "/cronosII/pixmaps/folder-closed.png");
				pixmap_opened = gnome_pixmap_new_from_file (DATADIR "/cronosII/pixmaps/folder-opened.png");
			} else
			{
				pixmap_closed = gnome_pixmap_new_from_file (DATADIR "/cronosII/pixmaps/mailbox.png");
				pixmap_opened = pixmap_closed;
			}
		}
		
		buf = g_strdup (current->name);
		_node = gtk_ctree_insert_node (GTK_CTREE (ctree), node, NULL, (gchar **) &buf, 4,
									GNOME_PIXMAP (pixmap_closed)->pixmap, GNOME_PIXMAP (pixmap_closed)->mask,
									GNOME_PIXMAP (pixmap_opened)->pixmap, GNOME_PIXMAP (pixmap_opened)->mask,
									FALSE, TRUE);
		gtk_ctree_node_set_row_data(GTK_CTREE(ctree), _node, (gpointer) current);
		if (current->child)
			c2_mailbox_tree_fill (current->child, _node, ctree, window);
	}

	if (!node)
		gtk_clist_thaw (GTK_CLIST (ctree));
}
