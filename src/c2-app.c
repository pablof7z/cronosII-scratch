#include <config.h>
#include <gnome.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <libmodules/error.h>
#include <libmodules/utils.h>

#include "c2-app.h"

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
c2_app_init (void) {

}

void
c2_mailbox_tree_fill (C2Mailbox *head, GtkCTreeNode *node, GtkWidget *ctree, GtkWidget *window) {
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
