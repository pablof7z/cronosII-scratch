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
#include <libcronosII/imap.h>
#include <libcronosII/error.h>

#include "widget-dialog-preferences.h"
#include "widget-mailbox-list.h"

static void
class_init									(C2MailboxListClass *klass);

static void
init										(C2MailboxList *mlist);

static void
destroy										(GtkObject *object);

static void
on_tree_select_row							(GtkCTree *ctree, GtkCTreeNode *row, gint column,
											 C2MailboxList *mlist);

static void
on_tree_unselect_row						(GtkCTree *ctree, GtkCTreeNode *row, gint column,
											 C2MailboxList *mlist);

static void
on_tree_expand								(GtkCTree *ctree, GtkCTreeNode *node);

static void
on_tree_collapse							(GtkCTree *ctree, GtkCTreeNode *node);

static void
on_application_reload_mailboxes				(C2Application *application, C2MailboxList *mlist);

static void
on_application_preferences_changed			(C2Application *application, gint key, gpointer value,
											 C2MailboxList *mlist);

static GtkObject *
get_object_from_node						(C2MailboxList *mlist, GtkCTreeNode *node);

static void
on_mailbox_changed_mailbox					(C2Mailbox *mailbox, C2MailboxChangeType type,
											 C2Db *db, C2Pthread2 *data);

static void
node_fill									(GtkCTree *ctree, GtkCTreeNode *cnode,
											 C2Mailbox *mailbox, C2Db *start_db, gint *unreaded);

static void
tree_fill									(C2MailboxList *mlist, C2Mailbox *mailbox,
											 C2Account *account, GtkCTreeNode *node);

static GtkWidget *
get_pixmap									(C2Mailbox *mailbox, gboolean open);

enum
{
	OBJECT_SELECTED,
	OBJECT_UNSELECTED,
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

	signals[OBJECT_SELECTED] =
		gtk_signal_new ("object_selected",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2MailboxListClass, object_selected),
						gtk_marshal_NONE__POINTER, GTK_TYPE_NONE, 1,
						GTK_TYPE_POINTER);
	signals[OBJECT_UNSELECTED] =
		gtk_signal_new ("object_unselected",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2MailboxListClass, object_unselected),
						gtk_marshal_NONE__NONE, GTK_TYPE_NONE, 0);
	gtk_object_class_add_signals(object_class, signals, LAST_SIGNAL);

	klass->object_selected = NULL;
	klass->object_unselected = NULL;
	object_class->destroy = destroy;
}

static void
init (C2MailboxList *mlist)
{	
	mlist->selected_node = NULL;
	mlist->mailbox_list = NULL;
	mlist->data_list = NULL;
}

static void
destroy (GtkObject *object)
{
	C2MailboxList *mlist;
	GList *ml, *dl;

	mlist = C2_MAILBOX_LIST (object);
	
	for (ml = mlist->mailbox_list, dl = mlist->data_list;
		 ml && dl;
		 ml = g_list_next (ml), dl = g_list_next (dl))
	{
		if (C2_IS_MAILBOX (ml->data))
			gtk_signal_disconnect_by_func (GTK_OBJECT (ml->data),
											GTK_SIGNAL_FUNC (on_mailbox_changed_mailbox),
											dl->data);
		g_free (dl->data);
	}

	g_list_free (mlist->mailbox_list);
	g_list_free (mlist->data_list);
}

GtkWidget *
c2_mailbox_list_new (C2Application *application)
{
	C2MailboxList *mlist;
	GtkWidget *widget;
	gchar *titles[] =
	{
		N_("Mailbox"),
		NULL,
		NULL
	};

	mlist = gtk_type_new (c2_mailbox_list_get_type ());

	gtk_object_set_data (GTK_OBJECT (mlist), "application", application);

	gtk_ctree_construct (GTK_CTREE (mlist), 1, 0, titles);
	tree_fill (mlist, application->mailbox, application->account, NULL);

	gtk_signal_connect (GTK_OBJECT (mlist), "tree_select_row",
							GTK_SIGNAL_FUNC (on_tree_select_row), mlist);
	gtk_signal_connect (GTK_OBJECT (mlist), "tree_unselect_row",
							GTK_SIGNAL_FUNC (on_tree_unselect_row), mlist);
	gtk_signal_connect (GTK_OBJECT (mlist), "tree_expand",
							GTK_SIGNAL_FUNC (on_tree_expand), mlist);
	gtk_signal_connect (GTK_OBJECT (mlist), "tree_collapse",
							GTK_SIGNAL_FUNC (on_tree_collapse), mlist);

	gtk_signal_connect (GTK_OBJECT (application), "reload_mailboxes",
							GTK_SIGNAL_FUNC (on_application_reload_mailboxes), mlist);
	gtk_signal_connect (GTK_OBJECT (application), "application_preferences_changed",
							GTK_SIGNAL_FUNC (on_application_preferences_changed), mlist);
	
	widget = GTK_WIDGET (mlist);
	GTK_WIDGET_UNSET_FLAGS (widget, GTK_CAN_FOCUS);
	return widget;
}

C2Mailbox *
c2_mailbox_list_get_selected_mailbox (C2MailboxList *mlist)
{
	GtkObject *object = get_object_from_node (mlist, mlist->selected_node);

	if (C2_IS_MAILBOX (object))
		return C2_MAILBOX (object);

	return NULL;
}

GtkObject *
c2_mailbox_list_get_selected_object (C2MailboxList *mlist)
{
	return get_object_from_node (mlist, mlist->selected_node);
}

void
c2_mailbox_list_set_selected_object (C2MailboxList *mlist, GtkObject *object)
{
	GtkCTreeNode *node;
	
	node = gtk_ctree_find_by_row_data (GTK_CTREE (mlist), gtk_ctree_node_nth (GTK_CTREE (mlist), 0), object);

	if (!node)
		return;

	gtk_ctree_select (GTK_CTREE (mlist), node);

	/* [TODO] Check if the selected signal is emitted, if not, emit it. */
}

static void
on_tree_select_row (GtkCTree *ctree, GtkCTreeNode *row, gint column, C2MailboxList *mlist)
{
	mlist->selected_node = row;

	gtk_signal_emit (GTK_OBJECT (mlist), signals[OBJECT_SELECTED], get_object_from_node (mlist, row));
}

static void
on_tree_unselect_row (GtkCTree *ctree, GtkCTreeNode *row, gint column, C2MailboxList *mlist)
{
	mlist->selected_node = NULL;

	gtk_signal_emit (GTK_OBJECT (mlist), signals[OBJECT_UNSELECTED]);
}

static void
on_tree_expand (GtkCTree *ctree, GtkCTreeNode *node)
{
	C2Mailbox *mailbox;
	gint unreaded;

	mailbox = C2_MAILBOX (gtk_ctree_node_get_row_data (ctree, node));
	node_fill (ctree, node, mailbox, NULL, &unreaded);
}

static void
on_tree_collapse (GtkCTree *ctree, GtkCTreeNode *node)
{
	C2Mailbox *mailbox;
	gint unreaded;

	mailbox = C2_MAILBOX (gtk_ctree_node_get_row_data (ctree, node));
	node_fill (ctree, node, mailbox, NULL, &unreaded);
}

static void
on_application_reload_mailboxes (C2Application *application, C2MailboxList *mlist)
{
	tree_fill (mlist, application->mailbox, application->account, NULL);
}

static void
on_application_preferences_changed (C2Application *application, gint key, gpointer value,
	 C2MailboxList *mlist)
{
	if (key == C2_DIALOG_PREFERENCES_KEY_INTERFACE_FONTS_UNREADED_MAILBOX)
		tree_fill (mlist, application->mailbox, application->account, NULL);
}

static GtkObject *
get_object_from_node (C2MailboxList *mlist, GtkCTreeNode *node)
{
	return GTK_OBJECT (gtk_ctree_node_get_row_data (GTK_CTREE (mlist), node));
}

static void
on_mailbox_changed_mailbox (C2Mailbox *mailbox, C2MailboxChangeType type, C2Db *db, C2Pthread2 *data)
{
	GtkCTree *ctree = GTK_CTREE (data->v1);
	GtkCTreeNode *cnode = GTK_CTREE_NODE (data->v2);
	gint unreaded;

	switch (type)
	{
		case C2_MAILBOX_CHANGE_ANY:
		case C2_MAILBOX_CHANGE_ADD:
		case C2_MAILBOX_CHANGE_REMOVE:
			gdk_threads_enter ();
			node_fill (ctree, cnode, mailbox, NULL, &unreaded);
			gdk_threads_leave ();
			break;
		case C2_MAILBOX_CHANGE_STATE:
			break;
	}
}

static void
node_fill (GtkCTree *ctree, GtkCTreeNode *cnode, C2Mailbox *mailbox, C2Db *start_db, gint *unreaded)
{
	C2Application *application;
	GtkWidget *pixmap;
	GtkStyle *style;
	C2Db *ldb;
	gchar *text;

	application = C2_APPLICATION (gtk_object_get_data (GTK_OBJECT (ctree), "application"));
	
	pixmap = get_pixmap (mailbox, GTK_CTREE_ROW (cnode)->expanded);

	ldb = start_db ? start_db : mailbox->db;

	*unreaded = c2_db_length_type (mailbox, C2_MESSAGE_UNREADED);
	
	if ((*unreaded))
		text = g_strdup_printf ("%s (%d)", mailbox->name, (*unreaded));
	else
		text = mailbox->name;

	gtk_ctree_node_set_pixtext (ctree, cnode, 0, text, 2, GNOME_PIXMAP (pixmap)->pixmap,
								GNOME_PIXMAP (pixmap)->mask);

	if ((*unreaded))
		g_free (text);

	if (GTK_CTREE_ROW (cnode)->expanded)
	{
		GTK_CTREE_ROW (cnode)->pixmap_opened = GNOME_PIXMAP (pixmap)->pixmap;
		GTK_CTREE_ROW (cnode)->mask_opened = GNOME_PIXMAP (pixmap)->mask;
		pixmap = get_pixmap (mailbox, !GTK_CTREE_ROW (cnode)->expanded);
		GTK_CTREE_ROW (cnode)->pixmap_closed = GNOME_PIXMAP (pixmap)->pixmap;
		GTK_CTREE_ROW (cnode)->mask_closed = GNOME_PIXMAP (pixmap)->mask;
	} else
	{
		GTK_CTREE_ROW (cnode)->pixmap_closed = GNOME_PIXMAP (pixmap)->pixmap;
		GTK_CTREE_ROW (cnode)->mask_closed = GNOME_PIXMAP (pixmap)->mask;
		pixmap = get_pixmap (mailbox, !GTK_CTREE_ROW (cnode)->expanded);
		GTK_CTREE_ROW (cnode)->pixmap_opened = GNOME_PIXMAP (pixmap)->pixmap;
		GTK_CTREE_ROW (cnode)->mask_opened = GNOME_PIXMAP (pixmap)->mask;
	}

	if ((*unreaded))
	{
		style = gtk_ctree_node_get_row_style (ctree, cnode);
		if (!style)
			style = gtk_style_copy (GTK_WIDGET (ctree)->style);
		style->font = application->fonts_gdk_unreaded_mailbox;
		gtk_ctree_node_set_row_style (ctree, cnode, style);
	}
}

static void
tree_fill (C2MailboxList *mlist, C2Mailbox *mailbox, C2Account *account,
			GtkCTreeNode *node)
{
	C2Pthread2 *data;
	C2Mailbox *l;
	GtkCTree *ctree = GTK_CTREE (mlist);
	GtkCTreeNode *cnode;
	gint unreaded;
	GtkStyle *style;
	C2Db *db;

	if (!node)
	{
		gtk_clist_freeze (GTK_CLIST (mlist));
		gtk_clist_clear (GTK_CLIST (mlist));
	}

	for (l = mailbox; l; l = l->next)
	{
		data = g_new0 (C2Pthread2, 1);
		data->v1 = (gpointer) ctree;
		cnode = gtk_ctree_insert_node (ctree, node, NULL, NULL, 4, NULL, NULL, NULL, NULL, FALSE, TRUE);
		gtk_ctree_node_set_row_data (ctree, cnode, (gpointer) l);
		data->v2 = (gpointer) cnode;

		node_fill (ctree, cnode, l, NULL, &unreaded);

		gtk_signal_connect (GTK_OBJECT (l), "changed_mailbox",
							GTK_SIGNAL_FUNC (on_mailbox_changed_mailbox), data);
		mlist->data_list = g_list_prepend (mlist->data_list, data);
		mlist->mailbox_list = g_list_prepend (mlist->mailbox_list, l);
		
		if (l->child)
			tree_fill (mlist, l->child, account, cnode);
	}

	if (!node)
	{
		C2Account *la;
		C2Application *application;

		application = C2_APPLICATION (gtk_object_get_data (GTK_OBJECT (mlist), "application"));
		
		for (la = account; la; la = c2_account_next (la))
		{
			if (la->type == C2_ACCOUNT_IMAP)
			{
				C2IMAP *imap;
				gchar *lrow[] =
				{
					la->name
				};
				
				cnode = gtk_ctree_insert_node (ctree, NULL, NULL, lrow, 4, NULL, NULL, NULL, NULL, FALSE,
												TRUE);
				gtk_ctree_node_set_row_data (ctree, cnode, (gpointer) la);
				style = gtk_ctree_node_get_row_style (ctree, cnode);
				if (!style)
					style = gtk_style_copy (GTK_WIDGET (ctree)->style);
				style->font = application->fonts_gdk_unreaded_mailbox;
				gtk_ctree_node_set_row_style (ctree, cnode, style);

				imap = C2_IMAP (c2_account_get_extra_data (la, C2_ACCOUNT_KEY_INCOMING, NULL));
				c2_imap_init (imap);
			}
		}
		
		gtk_clist_thaw (GTK_CLIST (mlist));
	}
}

static GtkWidget *
get_pixmap (C2Mailbox *mailbox, gboolean open)
{
	if (c2_streq (mailbox->name, C2_MAILBOX_INBOX))
		return gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/inbox.png");
	else if (c2_streq (mailbox->name, C2_MAILBOX_OUTBOX))
		return gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/outbox.png");
	else if (c2_streq (mailbox->name, C2_MAILBOX_SENT_ITEMS))
		return gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/queue.png");
	else if (c2_streq (mailbox->name, C2_MAILBOX_GARBAGE))
		return gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/garbage.png");
	else if (c2_streq (mailbox->name, C2_MAILBOX_DRAFTS))
		return gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/drafts.png");
	else
	{
		if (mailbox->child)
		{
			if (!open)
				return gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/folder-closed.png");
			else
				return gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/folder-opened.png");
		} else
		{
			return gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/mailbox.png");
		}
	}

	return gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/mailbox.png");
}
