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

#include <libmodules/error.h>
#include <libmodules/utils.h>

#include "c2-app.h"
#include "main-window.h"

#include "xpm/drafts.xpm"
#include "xpm/inbox.xpm"
#include "xpm/outbox.xpm"
#include "xpm/trash.xpm"
#include "xpm/queue_mbox.xpm"
#include "xpm/folder.xpm"

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

	DEBUG(c2_app.font_body);
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
		gnome_appbar_pop (GNOME_APPBAR (WMain.appbar));
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
	c2_return_if_fail (msg, C2EDATA);

	/* TODO Here we should ask which way to make a report: Statusbar, dialog, etc. TODO 
	 * We assume statusbar */
	if (!pthread_mutex_trylock (&WMain.appbar_lock))
	{
		gnome_appbar_push (GNOME_APPBAR (WMain.appbar), msg);
		gtk_timeout_add (5000, on_report_expirate, NULL);
		pthread_mutex_unlock (&WMain.appbar_lock);
	}
}

void
c2_mailbox_tree_fill (C2Mailbox *head, GtkCTreeNode *node, GtkWidget *ctree, GtkWidget *window)
{
	C2Mailbox *current;
	GdkPixmap *xpm;
	GdkBitmap *msk;
	GtkCTreeNode *_node;
	gchar *buf;
  
	c2_return_if_fail (head || ctree, C2EDATA);

	current = head;
	
	for (; current; current = current->next)
	{  
		if (c2_streq (current->name, MAILBOX_INBOX))
		{
			xpm = gdk_pixmap_create_from_xpm_d (window->window, &msk,
					&window->style->bg[GTK_STATE_NORMAL],
					inbox_xpm);
		} else if (!strcmp ((char *) current->name, MAILBOX_OUTBOX))
		{
			xpm = gdk_pixmap_create_from_xpm_d (window->window, &msk,
					&window->style->bg[GTK_STATE_NORMAL],
					outbox_xpm);
		} else if (!strcmp ((char *) current->name, MAILBOX_QUEUE))
		{
			xpm = gdk_pixmap_create_from_xpm_d (window->window, &msk,
					&window->style->bg[GTK_STATE_NORMAL],
					queue_mbox_xpm);
		} else if (!strcmp ((char *) current->name, MAILBOX_GARBAGE))
		{
			xpm = gdk_pixmap_create_from_xpm_d (window->window, &msk,
					&window->style->bg[GTK_STATE_NORMAL],
					trash_xpm);
		} else if (!strcmp ((char *) current->name, MAILBOX_DRAFTS))
		{
			xpm = gdk_pixmap_create_from_xpm_d (window->window, &msk,
					&window->style->bg[GTK_STATE_NORMAL],
					drafts_xpm);
		} else
		{
			xpm = gdk_pixmap_create_from_xpm_d (window->window, &msk,
					&window->style->bg[GTK_STATE_NORMAL],
					folder_xpm);
		}
		
		buf = g_strdup (current->name);
		_node = gtk_ctree_insert_node (GTK_CTREE (ctree), node, NULL, (gchar **) &buf, 4, xpm, msk,
				xpm, msk, FALSE, TRUE);
		gtk_ctree_node_set_row_data(GTK_CTREE(ctree), _node, (gpointer) current);
		if (current->child) c2_mailbox_tree_fill (current->child, _node, ctree, window);
	}
}
