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
#include "widget-mailbox-list.h"

static void
class_init									(C2MailboxListClass *klass);

static void
init										(C2MailboxList *mlist);

static void
on_tree_select_row							(GtkCTree *ctree, GtkCTreeNode *row, gint column,
											 C2MailboxList *mlist);

static void
on_tree_unselect_row						(GtkCTree *ctree, GtkCTreeNode *row, gint column,
											 C2MailboxList *mlist);

static void
on_application_reload_mailboxes				(C2Application *application, C2MailboxList *mlist);

static C2Mailbox *
get_mailbox_from_node						(C2MailboxList *mlist, GtkCTreeNode *node);

static void
tree_fill									(C2Application *application, C2MailboxList *mlist,
											 C2Mailbox *mailbox, GtkCTreeNode *node);

enum
{
	MAILBOX_SELECTED,
	MAILBOX_UNSELECTED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static GtkCTreeClass *parent_class = NULL;

GtkType
c2_mailbox_list_get_type (void)
{
	static GtkType type = 0;

	if (!type)
	{
		static GtkTypeInfo info =
		{
			"C2MailboxList",
			sizeof (C2MailboxList),
			sizeof (C2MailboxListClass),
			(GtkClassInitFunc) class_init,
			(GtkObjectInitFunc) init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL
		};

		type = gtk_type_unique (gtk_ctree_get_type (), &info);
	}

	return type;
}

static void
class_init (C2MailboxListClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *) klass;

	parent_class = gtk_type_class (gtk_ctree_get_type ());

	signals[MAILBOX_SELECTED] =
		gtk_signal_new ("mailbox_selected",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2MailboxListClass, mailbox_selected),
						gtk_marshal_NONE__POINTER, GTK_TYPE_NONE, 1,
						GTK_TYPE_POINTER);
	signals[MAILBOX_UNSELECTED] =
		gtk_signal_new ("mailbox_unselected",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2MailboxListClass, mailbox_unselected),
						gtk_marshal_NONE__NONE, GTK_TYPE_NONE, 0);
	gtk_object_class_add_signals(object_class, signals, LAST_SIGNAL);

	klass->mailbox_selected = NULL;
	klass->mailbox_unselected = NULL;
}

static void
init (C2MailboxList *mlist)
{
	gchar *titles[] =
	{
		N_("Mailbox"),
		NULL,
		NULL
	};
	
	mlist->selected_node = NULL;
	gtk_ctree_construct (GTK_CTREE (mlist), 1, 0, titles);
}

GtkWidget *
c2_mailbox_list_new (C2Application *application)
{
	C2MailboxList *mlist;

	mlist = gtk_type_new (c2_mailbox_list_get_type ());

	tree_fill (application, mlist, application->mailbox, NULL);

	gtk_signal_connect (GTK_OBJECT (mlist), "tree_select_row",
							GTK_SIGNAL_FUNC (on_tree_select_row), mlist);

	gtk_signal_connect (GTK_OBJECT (mlist), "tree_unselect_row",
							GTK_SIGNAL_FUNC (on_tree_unselect_row), mlist);

	gtk_signal_connect (GTK_OBJECT (application), "reload_mailboxes",
							GTK_SIGNAL_FUNC (on_application_reload_mailboxes), mlist);

	return GTK_WIDGET (mlist);
}

C2Mailbox *
c2_mailbox_list_get_selected_mailbox (C2MailboxList *mlist)
{
	return get_mailbox_from_node (mlist, mlist->selected_node);
}

void
c2_mailbox_list_set_selected_mailbox (C2MailboxList *mlist, C2Mailbox *mailbox)
{
	GtkCTreeNode *node;
	
	node = gtk_ctree_find_by_row_data (GTK_CTREE (mlist), gtk_ctree_node_nth (GTK_CTREE (mlist), 0), mailbox);

	if (!node)
		return;

	gtk_ctree_select (GTK_CTREE (mlist), node);

	/* [TODO] Check if the selected signal is emitted, if not, emit it. */
}

static void
on_tree_select_row (GtkCTree *ctree, GtkCTreeNode *row, gint column, C2MailboxList *mlist)
{
	mlist->selected_node = row;

	gtk_signal_emit (GTK_OBJECT (mlist), signals[MAILBOX_SELECTED], get_mailbox_from_node (mlist, row));
}

static void
on_tree_unselect_row (GtkCTree *ctree, GtkCTreeNode *row, gint column, C2MailboxList *mlist)
{
	mlist->selected_node = NULL;

	gtk_signal_emit (GTK_OBJECT (mlist), signals[MAILBOX_UNSELECTED]);
}

static void
on_application_reload_mailboxes (C2Application *application, C2MailboxList *mlist)
{
	tree_fill (application, mlist, application->mailbox, NULL);
}

static C2Mailbox *
get_mailbox_from_node (C2MailboxList *mlist, GtkCTreeNode *node)
{
	return C2_MAILBOX (gtk_ctree_node_get_row_data (GTK_CTREE (mlist), node));
}

static void
tree_fill (C2Application *application, C2MailboxList *mlist, C2Mailbox *mailbox, GtkCTreeNode *node)
{
	C2Mailbox *l;
	GtkWidget *pixmap_closed;
	GtkWidget *pixmap_opened;
	GtkCTree *ctree = GTK_CTREE (mlist);
	GtkCTreeNode *nnode;
	GtkStyle *style;
	gint unreaded;
	gchar *row[1];
	C2Db *db;
	
	if (!node)
	{
		gtk_clist_freeze (GTK_CLIST (mlist));
		gtk_clist_clear (GTK_CLIST (mlist));
	}

	for (l = mailbox; l; l = l->next)
	{
		if (c2_streq (l->name, C2_MAILBOX_INBOX))
		{
			pixmap_closed = gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/inbox.png");
			pixmap_opened = pixmap_closed;
		} else if (c2_streq (l->name, C2_MAILBOX_OUTBOX))
		{
			pixmap_closed = gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/outbox.png");
			pixmap_opened = pixmap_closed;
		} else if (c2_streq (l->name, C2_MAILBOX_QUEUE)) /* 2941761 */
		{
			pixmap_closed = gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/queue.png");
			pixmap_opened = pixmap_closed;
		} else if (c2_streq (l->name, C2_MAILBOX_GARBAGE))
		{
			pixmap_closed = gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/garbage.png");
			pixmap_opened = pixmap_closed;
		} else if (c2_streq (l->name, C2_MAILBOX_DRAFTS))
		{
			pixmap_closed = gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/drafts.png");
			pixmap_opened = pixmap_closed;
		} else
		{
			if (l->child)
			{
				pixmap_closed = gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/folder-closed.png");
				pixmap_opened = gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/folder-opened.png");
			} else
			{
				pixmap_closed = gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/mailbox.png");
				pixmap_opened = pixmap_closed;
			}
		}

		unreaded = 0;
		db = l->db;

		if (db)
		{
			do
			{
				if (db->state == C2_MESSAGE_UNREADED)
					unreaded++;
			} while (c2_db_lineal_next (db));
		}

		if (unreaded)
			row[0] = g_strdup_printf ("%s (%d)", l->name, unreaded);
		else
			row[0] = g_strdup (l->name);

		nnode = gtk_ctree_insert_node (ctree, node, NULL, row, 4, GNOME_PIXMAP (pixmap_closed)->pixmap,
									GNOME_PIXMAP (pixmap_closed)->mask,	GNOME_PIXMAP (pixmap_opened)->pixmap,
									GNOME_PIXMAP (pixmap_opened)->mask,	FALSE, TRUE);
		gtk_ctree_node_set_row_data (ctree, nnode, (gpointer) l);

		if (unreaded)
		{
			style = gtk_ctree_node_get_row_style (ctree, nnode);
			if (!style)
				style = gtk_style_copy (GTK_WIDGET (ctree)->style);
			style->font = application->fonts_gdk_unreaded_mailbox;
			gtk_ctree_node_set_row_style (ctree, nnode, style);
		}

		if (l->child)
			tree_fill (application, mlist, l, nnode);
	}

	if (!node)
		gtk_clist_thaw (GTK_CLIST (mlist));
}
