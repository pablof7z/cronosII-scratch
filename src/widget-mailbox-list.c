/*  Cronos II - The GNOME mail client
 *  Copyright (C) 2000-2001 Pablo Fernández López
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
/**
 * Maintainer(s) of this file:
 * 		* Pablo Fernández López
 * Code of this file by:
 * 		* Pablo Fernández López
 * 		* Bosko Blagojevic
 **/
#include <libcronosII/imap.h>
#include <libcronosII/error.h>

#include "widget-application.h"
#include "widget-application-utils.h"
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
on_mlist_button_press_event					(GtkWidget *mlist, GdkEvent *event);

static void
on_menu_new_mailbox_activate				(GtkWidget *widget, C2MailboxList *mlist);

static void
on_menu_delete_mailbox_activate				(GtkWidget *widget, C2MailboxList *mlist);

static void
on_menu_properties_activate					(GtkWidget *widget, C2MailboxList *mlist);

static void
on_menu_use_as_inbox_toggled				(GtkWidget *widget, C2MailboxList *mlist);

static void
on_menu_use_as_outbox_toggled				(GtkWidget *widget, C2MailboxList *mlist);

static void
on_menu_use_as_sent_items_toggled			(GtkWidget *widget, C2MailboxList *mlist);

static void
on_menu_use_as_trash_toggled				(GtkWidget *widget, C2MailboxList *mlist);

static void
on_menu_use_as_drafts_toggled				(GtkWidget *widget, C2MailboxList *mlist);

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
account_node_fill							(GtkCTree *ctree, GtkCTreeNode *cnode,
											 C2Account *account);

static void
mailbox_node_fill							(GtkCTree *ctree, GtkCTreeNode *cnode,
											 C2Mailbox *mailbox, C2Db *start_db, gint *unreaded);

static void
tree_fill									(C2MailboxList *mlist, C2Mailbox *mailbox,
											 C2Account *account, GtkCTreeNode *node);

static GtkWidget *
get_pixmap									(C2Mailbox *mailbox, gboolean open);

static void
on_imap_mailbox_list						(C2IMAP *imap, C2Mailbox *head, C2Pthread2 *data);

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
	GladeXML *xml;
	gchar *titles[] =
	{
		N_("Mailboxes")
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
	gtk_signal_connect (GTK_OBJECT (mlist), "button_press_event",
      			GTK_SIGNAL_FUNC (on_mlist_button_press_event), NULL);
	
	xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("cronosII"), "mnu_mlist");
			
	gtk_object_set_data (GTK_OBJECT (mlist), "menu", xml);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "new_mailbox")), "activate",
						GTK_SIGNAL_FUNC (on_menu_new_mailbox_activate), mlist);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "delete_mailbox")), "activate",
						GTK_SIGNAL_FUNC (on_menu_delete_mailbox_activate), mlist);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "properties")), "activate",
						GTK_SIGNAL_FUNC (on_menu_properties_activate), mlist);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "use_as_inbox")), "toggled",
						GTK_SIGNAL_FUNC (on_menu_use_as_inbox_toggled), mlist);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "use_as_outbox")), "toggled",
						GTK_SIGNAL_FUNC (on_menu_use_as_outbox_toggled), mlist);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "use_as_sent_items")), "toggled",
						GTK_SIGNAL_FUNC (on_menu_use_as_sent_items_toggled), mlist);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "use_as_trash")), "toggled",
						GTK_SIGNAL_FUNC (on_menu_use_as_trash_toggled), mlist);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "use_as_drafts")), "toggled",
						GTK_SIGNAL_FUNC (on_menu_use_as_drafts_toggled), mlist);
						

	gtk_signal_connect (GTK_OBJECT (application), "reload_mailboxes",
							GTK_SIGNAL_FUNC (on_application_reload_mailboxes), mlist);
	gtk_signal_connect (GTK_OBJECT (application), "application_preferences_changed",
							GTK_SIGNAL_FUNC (on_application_preferences_changed), mlist);
	
	widget = GTK_WIDGET (mlist);
	GTK_WIDGET_UNSET_FLAGS (widget, GTK_CAN_FOCUS);
	gtk_clist_column_titles_passive (GTK_CLIST (mlist));
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
on_mlist_button_press_event (GtkWidget *mlist, GdkEvent *event)
{
	GladeXML *xml;
	
	if (event->button.button == 3)
	{
		GdkEventButton *e = (GdkEventButton *) event;
		GtkCTreeNode *node;
		gint mbox_n, row, column;
		GtkObject *object;
		
		xml = GLADE_XML (gtk_object_get_data (GTK_OBJECT (mlist), "menu"));

		mbox_n = gtk_clist_get_selection_info (GTK_CLIST (mlist), e->x, e->y, &row, &column);

		if (mbox_n)
		{
			node = gtk_ctree_node_nth (GTK_CTREE (mlist), row);
			
			object = gtk_ctree_node_get_row_data (GTK_CTREE (mlist), node);
			c2_mailbox_list_set_selected_object (C2_MAILBOX_LIST (mlist), object);
			gtk_ctree_select (GTK_CTREE (mlist), node);

		} else
		{
			c2_mailbox_list_set_selected_object (C2_MAILBOX_LIST (mlist), NULL);
			object = NULL;
		}

		/* Set sensitivy */
		if (C2_IS_MAILBOX (object))
		{
			gtk_widget_show (glade_xml_get_widget (xml, "delete_mailbox"));
			gtk_widget_show (glade_xml_get_widget (xml, "properties"));
			gtk_widget_show (glade_xml_get_widget (xml, "use_as_inbox"));
			gtk_widget_show (glade_xml_get_widget (xml, "use_as_outbox"));
			gtk_widget_show (glade_xml_get_widget (xml, "use_as_sent_items"));
			gtk_widget_show (glade_xml_get_widget (xml, "use_as_trash"));
			gtk_widget_show (glade_xml_get_widget (xml, "use_as_drafts"));
			gtk_widget_show (glade_xml_get_widget (xml, "sep01"));
			gtk_widget_show (glade_xml_get_widget (xml, "sep02"));
		} else
		{
			gtk_widget_hide (glade_xml_get_widget (xml, "delete_mailbox"));
			gtk_widget_hide (glade_xml_get_widget (xml, "properties"));
			gtk_widget_hide (glade_xml_get_widget (xml, "use_as_inbox"));
			gtk_widget_hide (glade_xml_get_widget (xml, "use_as_outbox"));
			gtk_widget_hide (glade_xml_get_widget (xml, "use_as_sent_items"));
			gtk_widget_hide (glade_xml_get_widget (xml, "use_as_trash"));
			gtk_widget_hide (glade_xml_get_widget (xml, "use_as_drafts"));
			gtk_widget_hide (glade_xml_get_widget (xml, "sep01"));
			gtk_widget_hide (glade_xml_get_widget (xml, "sep02"));
		}

		/* Set data */
		if (C2_IS_MAILBOX (object))
		{
			C2Mailbox *mailbox = C2_MAILBOX (object);
				
			gtk_check_menu_item_set_active (glade_xml_get_widget (xml, "use_as_inbox"),
											mailbox->use_as & C2_MAILBOX_USE_AS_INBOX);
			gtk_check_menu_item_set_active (glade_xml_get_widget (xml, "use_as_outbox"),
											mailbox->use_as & C2_MAILBOX_USE_AS_OUTBOX);
			gtk_check_menu_item_set_active (glade_xml_get_widget (xml, "use_as_sent_items"),
											mailbox->use_as & C2_MAILBOX_USE_AS_SENT_ITEMS);
			gtk_check_menu_item_set_active (glade_xml_get_widget (xml, "use_as_trash"),
											mailbox->use_as & C2_MAILBOX_USE_AS_TRASH);
			gtk_check_menu_item_set_active (glade_xml_get_widget (xml, "use_as_drafts"),
											mailbox->use_as & C2_MAILBOX_USE_AS_DRAFTS);
		}

		gnome_popup_menu_do_popup (glade_xml_get_widget (xml, "mnu_mlist"),
										NULL, NULL, e, NULL);
	}
}

static void
on_menu_new_mailbox_activate (GtkWidget *widget, C2MailboxList *mlist)
{
	C2Application *application;

	application = C2_APPLICATION (gtk_object_get_data (GTK_OBJECT (mlist), "application"));

	c2_application_dialog_add_mailbox (application);
}

static void
on_menu_delete_mailbox_activate (GtkWidget *widget, C2MailboxList *mlist)
{
}

static void
on_menu_properties_activate (GtkWidget *widget, C2MailboxList *mlist)
{
}

static void
on_menu_use_as_inbox_toggled (GtkWidget *widget, C2MailboxList *mlist)
{
	C2Mailbox *mailbox;
	gchar *path;
	gint id;

	/* Get the selected mailbox */
	if (!C2_IS_MAILBOX ((mailbox = C2_MAILBOX (c2_mailbox_list_get_selected_object (mlist)))))
		return;

	/* Update the mark */
	if (mailbox->type == C2_MAILBOX_IMAP)
		c2_mailbox_set_use_as (mailbox->protocol.IMAP.imap->mailboxes, mailbox,
								mailbox->use_as | C2_MAILBOX_USE_AS_INBOX);
	else
		c2_mailbox_set_use_as (C2_APPLICATION (gtk_object_get_data (GTK_OBJECT (mlist),
													"application"))->mailbox, mailbox,
								mailbox->use_as | C2_MAILBOX_USE_AS_INBOX);

	/* Update the CheckMenuItem */
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (widget),
									mailbox->use_as & C2_MAILBOX_USE_AS_INBOX);

	/* Update the configuration */
	if ((id = c2_application_get_mailbox_configuration_id_by_name (mailbox->name)) < 0)
		return;

	path = g_strdup_printf (PACKAGE "/Mailbox %d/use_as", id);
	gnome_config_set_int (path, mailbox->use_as);
	gnome_config_sync ();
	g_free (path);
}

static void
on_menu_use_as_outbox_toggled (GtkWidget *widget, C2MailboxList *mlist)
{
	C2Mailbox *mailbox;
	gchar *path;
	gint id;

	/* Get the selected mailbox */
	if (!C2_IS_MAILBOX ((mailbox = C2_MAILBOX (c2_mailbox_list_get_selected_object (mlist)))))
		return;

	/* Update the mark */
	if (mailbox->type == C2_MAILBOX_IMAP)
		c2_mailbox_set_use_as (mailbox->protocol.IMAP.imap->mailboxes, mailbox,
								mailbox->use_as | C2_MAILBOX_USE_AS_OUTBOX);
	else
		c2_mailbox_set_use_as (C2_APPLICATION (gtk_object_get_data (GTK_OBJECT (mlist),
													"application"))->mailbox, mailbox,
								mailbox->use_as | C2_MAILBOX_USE_AS_OUTBOX);

	/* Update the CheckMenuItem */
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (widget),
									mailbox->use_as & C2_MAILBOX_USE_AS_OUTBOX);

	/* Update the configuration */
	if ((id = c2_application_get_mailbox_configuration_id_by_name (mailbox->name)) < 0)
		return;

	path = g_strdup_printf (PACKAGE "/Mailbox %d/use_as", id);
	gnome_config_set_int (path, mailbox->use_as);
	gnome_config_sync ();
	g_free (path);
}

static void
on_menu_use_as_sent_items_toggled (GtkWidget *widget, C2MailboxList *mlist)
{
	C2Mailbox *mailbox;
	gchar *path;
	gint id;

	/* Get the selected mailbox */
	if (!C2_IS_MAILBOX ((mailbox = C2_MAILBOX (c2_mailbox_list_get_selected_object (mlist)))))
		return;

	/* Update the mark */
	if (mailbox->type == C2_MAILBOX_IMAP)
		c2_mailbox_set_use_as (mailbox->protocol.IMAP.imap->mailboxes, mailbox,
								mailbox->use_as | C2_MAILBOX_USE_AS_SENT_ITEMS);
	else
		c2_mailbox_set_use_as (C2_APPLICATION (gtk_object_get_data (GTK_OBJECT (mlist),
													"application"))->mailbox, mailbox,
								mailbox->use_as | C2_MAILBOX_USE_AS_SENT_ITEMS);

	/* Update the CheckMenuItem */
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (widget),
									mailbox->use_as & C2_MAILBOX_USE_AS_SENT_ITEMS);

	/* Update the configuration */
	if ((id = c2_application_get_mailbox_configuration_id_by_name (mailbox->name)) < 0)
		return;

	path = g_strdup_printf (PACKAGE "/Mailbox %d/use_as", id);
	gnome_config_set_int (path, mailbox->use_as);
	gnome_config_sync ();
	g_free (path);
}

static void
on_menu_use_as_trash_toggled (GtkWidget *widget, C2MailboxList *mlist)
{
	C2Mailbox *mailbox;
	gchar *path;
	gint id;

	/* Get the selected mailbox */
	if (!C2_IS_MAILBOX ((mailbox = C2_MAILBOX (c2_mailbox_list_get_selected_object (mlist)))))
		return;

	/* Update the mark */
	if (mailbox->type == C2_MAILBOX_IMAP)
		c2_mailbox_set_use_as (mailbox->protocol.IMAP.imap->mailboxes, mailbox,
								mailbox->use_as | C2_MAILBOX_USE_AS_TRASH);
	else
		c2_mailbox_set_use_as (C2_APPLICATION (gtk_object_get_data (GTK_OBJECT (mlist),
													"application"))->mailbox, mailbox,
								mailbox->use_as | C2_MAILBOX_USE_AS_TRASH);

	/* Update the CheckMenuItem */
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (widget),
									mailbox->use_as & C2_MAILBOX_USE_AS_TRASH);

	/* Update the configuration */
	if ((id = c2_application_get_mailbox_configuration_id_by_name (mailbox->name)) < 0)
		return;

	path = g_strdup_printf (PACKAGE "/Mailbox %d/use_as", id);
	gnome_config_set_int (path, mailbox->use_as);
	gnome_config_sync ();
	g_free (path);
}

static void
on_menu_use_as_drafts_toggled (GtkWidget *widget, C2MailboxList *mlist)
{
	C2Mailbox *mailbox;
	gchar *path;
	gint id;

	/* Get the selected mailbox */
	if (!C2_IS_MAILBOX ((mailbox = C2_MAILBOX (c2_mailbox_list_get_selected_object (mlist)))))
		return;

	/* Update the mark */
	if (mailbox->type == C2_MAILBOX_IMAP)
		c2_mailbox_set_use_as (mailbox->protocol.IMAP.imap->mailboxes, mailbox,
								mailbox->use_as | C2_MAILBOX_USE_AS_DRAFTS);
	else
		c2_mailbox_set_use_as (C2_APPLICATION (gtk_object_get_data (GTK_OBJECT (mlist),
													"application"))->mailbox, mailbox,
								mailbox->use_as | C2_MAILBOX_USE_AS_DRAFTS);

	/* Update the CheckMenuItem */
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (widget),
									mailbox->use_as & C2_MAILBOX_USE_AS_DRAFTS);

	/* Update the configuration */
	if ((id = c2_application_get_mailbox_configuration_id_by_name (mailbox->name)) < 0)
		return;

	path = g_strdup_printf (PACKAGE "/Mailbox %d/use_as", id);
	gnome_config_set_int (path, mailbox->use_as);
	gnome_config_sync ();
	g_free (path);
}

static void
on_tree_expand (GtkCTree *ctree, GtkCTreeNode *node)
{
	C2Mailbox *mailbox;
	C2Account *account;
	gint unreaded;

	if (C2_IS_MAILBOX (mailbox = (C2Mailbox*) gtk_ctree_node_get_row_data (ctree, node)))
		mailbox_node_fill (ctree, node, mailbox, NULL, &unreaded);
	else if (C2_IS_ACCOUNT (account = (C2Account*) gtk_ctree_node_get_row_data (ctree, node)) || !account)
		account_node_fill (ctree, node, account);
}

static void
on_tree_collapse (GtkCTree *ctree, GtkCTreeNode *node)
{
	C2Mailbox *mailbox;
	C2Account *account;
	gint unreaded;

	if (C2_IS_MAILBOX (mailbox = (C2Mailbox*) gtk_ctree_node_get_row_data (ctree, node)))
		mailbox_node_fill (ctree, node, mailbox, NULL, &unreaded);
	else if (C2_IS_ACCOUNT (account = (C2Account*) gtk_ctree_node_get_row_data (ctree, node)) || !account)
		account_node_fill (ctree, node, account);
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
	if (key == C2_DIALOG_PREFERENCES_KEY_INTERFACE_FONTS_UNREADED_MAILBOX ||
		key == C2_DIALOG_PREFERENCES_KEY_INTERFACE_FONTS_READED_MAILBOX)
	{
		tree_fill (mlist, application->mailbox, application->account, NULL);
	}
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
		case C2_MAILBOX_CHANGE_STATE:
			gdk_threads_enter ();
			mailbox_node_fill (ctree, cnode, mailbox, NULL, &unreaded);
			gdk_threads_leave ();
			break;
	}
}

static void
account_node_fill (GtkCTree *ctree, GtkCTreeNode *cnode, C2Account *account)
{
	GtkStyle *style;
	GtkWidget *pixmap;
	C2Application *application;

	application = C2_APPLICATION (gtk_object_get_data (GTK_OBJECT (ctree), "application"));
	
	if (!C2_IS_ACCOUNT (account))
	{
		gchar *text;
		gchar *buf1, *buf2;

		
		buf1 = g_getenv ("HOSTNAME");
		buf2 = g_get_user_name ();
		text = g_strdup_printf ("%s@%s", buf2, buf1);
		if (!text || !strlen (text))
			text = g_strdup (_("Local Mailboxes"));
		
		pixmap = get_pixmap (NULL, TRUE);
		gtk_ctree_node_set_pixtext (ctree, cnode, 0, text, 2, GNOME_PIXMAP (pixmap)->pixmap,
									GNOME_PIXMAP (pixmap)->mask);

		g_free (text);
	} else
	{
		pixmap = get_pixmap (NULL, TRUE);
		gtk_ctree_node_set_pixtext (ctree, cnode, 0, account->name, 2, GNOME_PIXMAP (pixmap)->pixmap,
									GNOME_PIXMAP (pixmap)->mask);
	}

	style = gtk_ctree_node_get_row_style (ctree, cnode);
	if (!style)
		style = gtk_style_copy (GTK_WIDGET (ctree)->style);
	
	style->font = application->fonts_gdk_unreaded_mailbox;
	
	gtk_ctree_node_set_row_style (ctree, cnode, style);
	gtk_ctree_node_set_row_data (ctree, cnode, (gpointer) account);
}

static void
mailbox_node_fill (GtkCTree *ctree, GtkCTreeNode *cnode, C2Mailbox *mailbox, C2Db *start_db, gint *unreaded)
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

	style = gtk_ctree_node_get_row_style (ctree, cnode);
	if (!style)
		style = gtk_style_copy (GTK_WIDGET (ctree)->style);
	
	if ((*unreaded))
	{
		style->font = application->fonts_gdk_unreaded_mailbox;
	} else
	{
		style->font = application->fonts_gdk_readed_mailbox;
	}
	
	gtk_ctree_node_set_row_style (ctree, cnode, style);
}

static void
init_imap (C2IMAP *imap)
{
	if (c2_imap_init (imap) < 0)
		return;
	
	c2_imap_populate_folders (imap);
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
	gboolean toplevel = node?FALSE:TRUE;

	if (toplevel)
	{
		gtk_clist_freeze (GTK_CLIST (mlist));
		gtk_clist_clear (GTK_CLIST (mlist));

		node = gtk_ctree_insert_node (ctree, NULL, NULL, NULL, 4, NULL, NULL, NULL, NULL, FALSE, TRUE);
		account_node_fill (ctree, node, NULL);
	}

	for (l = mailbox; l; l = l->next)
	{
		data = g_new0 (C2Pthread2, 1);
		data->v1 = (gpointer) ctree;
		cnode = gtk_ctree_insert_node (ctree, node, NULL, NULL, 4, NULL, NULL, NULL, NULL, FALSE, TRUE);
		gtk_ctree_node_set_row_data (ctree, cnode, (gpointer) l);
		data->v2 = (gpointer) cnode;

		mailbox_node_fill (ctree, cnode, l, NULL, &unreaded);

		gtk_signal_connect (GTK_OBJECT (l), "changed_mailbox",
							GTK_SIGNAL_FUNC (on_mailbox_changed_mailbox), data);
		mlist->data_list = g_list_prepend (mlist->data_list, data);
		mlist->mailbox_list = g_list_prepend (mlist->mailbox_list, l);
		
		if (l->child)
			tree_fill (mlist, l->child, account, cnode);
	}

	if (toplevel)
	{
		C2Account *la;
		C2Application *application;

		application = C2_APPLICATION (gtk_object_get_data (GTK_OBJECT (mlist), "application"));
		
		for (la = account; la; la = c2_account_next (la))
		{
			if (la->type == C2_ACCOUNT_IMAP)
			{
				C2IMAP *imap;
				C2Pthread2 *data;
				pthread_t thread;
				
				/* Create the node */
				cnode = gtk_ctree_insert_node (ctree, NULL, NULL, NULL, 4, NULL, NULL, NULL, NULL,
												FALSE, TRUE);
				account_node_fill (ctree, cnode, la);

				/* Get the IMAP object */
				imap = C2_IMAP (c2_account_get_extra_data (la, C2_ACCOUNT_KEY_INCOMING, NULL));
				
				if (!imap->mailboxes)
				{
					data = g_new0 (C2Pthread2, 1);
					data->v1 = mlist;
					data->v2 = cnode;
					gtk_signal_connect (GTK_OBJECT (imap), "mailbox_list",
										GTK_SIGNAL_FUNC (on_imap_mailbox_list), data);

					/* Initializate it */
					pthread_create (&thread, NULL, C2_PTHREAD_FUNC (init_imap), imap);
				} else
				{
					tree_fill (mlist, imap->mailboxes, NULL, cnode);
				}
			}
		}
		
		gtk_clist_thaw (GTK_CLIST (mlist));
		/* FIXME - Here's where The Fucking Problem is.
		 * TFP is (no, is not FTP..) that when the indicator
		 * of new-mails in the mailbox-list is switched (that is
		 * bold is used or not used) the font is not updated until
		 * a new configure_event is signaled. */
	}
}

static GtkWidget *
get_pixmap (C2Mailbox *mailbox, gboolean open)
{
	if (C2_IS_MAILBOX (mailbox))
	{
		if (mailbox->use_as & C2_MAILBOX_USE_AS_INBOX)
			return gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/inbox.png");
		else if (mailbox->use_as & C2_MAILBOX_USE_AS_SENT_ITEMS)
			return gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/outbox.png");
		else if (mailbox->use_as & C2_MAILBOX_USE_AS_OUTBOX)
			return gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/queue.png");
		else if (mailbox->use_as & C2_MAILBOX_USE_AS_TRASH)
			return gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/garbage.png");
		else if (mailbox->use_as & C2_MAILBOX_USE_AS_DRAFTS)
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
	} else
	{
		if (!open)
				return gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/mails.png");
			else
				return gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/mails.png");
	}

	return gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/mailbox.png");
}

static void
on_imap_mailbox_list (C2IMAP *imap, C2Mailbox *head, C2Pthread2 *data)
{
	C2MailboxList *mlist = C2_MAILBOX_LIST (data->v1);
	GtkCTreeNode *cnode = (GtkCTreeNode*) data->v2;
	C2Mailbox *l;

	g_free (data);
	
	gdk_threads_enter ();
	tree_fill (mlist, head, NULL, cnode);
	gdk_threads_leave ();
}
