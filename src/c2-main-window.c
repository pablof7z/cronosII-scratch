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
#include <time.h>

#include <libmodules/db.h>
#include <libmodules/error.h>

#include "c2-main-window.h"
#include "c2-app.h"

/**
 * c2_main_window_list_mails
 * @mbox: Mailbox to list.
 *
 * This function will list a mailbox
 * in the main window.
 **/
void
c2_main_window_list_mails (C2Mailbox *mbox)
{
	C2Mailbox *tmp;
	GList *l;
	GtkStyle *style = NULL, *style2;

	c2_return_if_fail (mbox, C2EDATA);
	if (pthread_mutex_trylock (&WMain.clist_lock))
		return;

	if (GTK_CLIST (WMain.clist)->selection)
	{
		/* Get the mailbox that was loaded */
		tmp = WMain.selected_mbox;
		if (tmp)
		{
			if (GTK_CLIST (WMain.clist)->selection)
				tmp->last_row = GPOINTER_TO_INT (GTK_CLIST (WMain.clist)->selection->data);
		}
	}

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
	gtk_clist_freeze (GTK_CLIST (WMain.clist));
	gtk_clist_clear (GTK_CLIST (WMain.clist));
	for (l = mbox->db->head; l != NULL; l = g_list_next (l))
	{
		C2DBNode *node = l->data;
		struct tm *tm;
		gchar *tmp = g_strdup_printf ("%d", node->mid);
		gchar *info[] = {
			NULL, NULL, NULL, node->headers[0], node->headers[1], NULL,
			node->headers[2], tmp
		};
		tm = node->date;
		info[5] = g_new0 (gchar, 128);
		strftime (info[5], 128, DATE_FORMAT, tm);
		gtk_clist_append (GTK_CLIST (WMain.clist), info);
		if (!style)
			style = gtk_widget_get_style (WMain.clist);
		style2 = gtk_style_copy (style);
		if (node->status == C2_DB_NODE_UNREADED)
		{
			style2->font = c2_app.gdk_font_unread;
			mbox->new_messages++;
		} else
			style2->font = c2_app.gdk_font_read;
		gtk_clist_set_row_style (GTK_CLIST (WMain.clist), GTK_CLIST (WMain.clist)->rows-1, style2);
	}
	gtk_clist_thaw (GTK_CLIST (WMain.clist));
	gtk_widget_queue_draw (WMain.clist);
	gdk_threads_leave ();

	pthread_mutex_unlock (&WMain.clist_lock);
}
