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
 **/
#include <gnome.h>
#include <time.h>

#include <libcronosII/error.h>

#include "main.h"
#include "preferences.h"
#include "widget-application.h"
#include "widget-dialog-preferences.h"
#include "widget-index.h"

#define SELECTED_MAIL	"mailbox::selected mail"

enum
{
	COLUMN_STYLE,
	COLUMN_SUBJECT,
	COLUMN_FROM,
	COLUMN_DATE,
	COLUMN_ACCOUNT,
	COLUMN_LAST
};

/*
 * TODO Sorting
 * TODO Right Click menu
 */

/***************************************
 * NOTES                               *
 ***************************************
 * The columns of the index widget
 * (which, I hope, will be soon
 * ported to use our own unparented
 * widget, completly unrelated of
 * GtkCList), are the following:
 * 1. Status (visible),
 * 2. Subject (visible),
 * 3. From (visible
 * 4. Date (visible),
 * 5. Account (visible),
 *
 * The C2Db is set as the data of each row,
 * that way is easy to get it.
 */

static void
class_init									(C2IndexClass *klass);

static void
init										(C2Index *index);

static void
reload										(C2Index *index);

static void
add_message									(C2Application *application, GtkCList *clist,
											 C2Db *db, const gchar *date_fmt);

static void
reload_node									(C2Application *application, GtkCList *clist, gint row, C2Db *db);

static gint
get_selected_mail							(C2Index *index);

static void
set_selected_mail							(C2Index *index, gint i);

static void
sort										(C2Index *index, C2MailboxSortBy sort_by, GtkSortType sort_type);

static void
on_clist_select_row							(C2Index *index, gint row, gint column, GdkEvent *event);

static void
on_clist_unselect_row						(C2Index *index, gint row, gint column, GdkEvent *event);

static void
on_clist_button_press_event					(C2Index *index, GdkEvent *e);

static void
on_resize_column							(C2Index *index, gint column, gint width);

static void
on_index_click_column						(GtkCList *clist, gint column, gpointer data);

static void
on_mnu_open_message_activate				(GtkWidget *widget, C2Index *index);

static void
on_mnu_reply_activate						(GtkWidget *widget, C2Index *index);

static void
on_mnu_reply_all_activate					(GtkWidget *widget, C2Index *index);

static void
on_mnu_forward_activate						(GtkWidget *widget, C2Index *index);

static void
on_mnu_delete_activate						(GtkWidget *widget, C2Index *index);

static void
on_mnu_expunge_activate						(GtkWidget *widget, C2Index *index);

static void
on_mnu_mark_important_activate				(GtkWidget *widget, C2Index *index);

static void
on_mnu_mark_unreaded_activate				(GtkWidget *widget, C2Index *index);

static void
on_mnu_mark_readed_activate					(GtkWidget *widget, C2Index *index);

static void
on_mnu_mark_replied_activate				(GtkWidget *widget, C2Index *index);

static void
on_mnu_mark_forwarded_activate				(GtkWidget *widget, C2Index *index);

static void
on_mnu_copy_activate						(GtkWidget *widget, C2Index *index);

static void
on_mnu_move_activate						(GtkWidget *widget, C2Index *index);

static void
on_mnu_previous_activate					(GtkWidget *widget, C2Index *index);

static void
on_mnu_next_activate						(GtkWidget *widget, C2Index *index);

static void
on_mnu_save_activate						(GtkWidget *widget, C2Index *index);

static void
on_mnu_print_activate						(GtkWidget *widget, C2Index *index);

static void
on_application_preferences_changed			(C2Application *application, gint key, gpointer value,
											 C2Index *index);

static void
disconnect									(C2Index *index);

static void
_connect									(C2Index *index);

/* signals */
enum
{
	SELECT_MESSAGE,			/* Click in a row */
	UNSELECT_MESSAGE,
	OPEN_MESSAGE,			/* Double click in a row */
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static GtkCListClass *parent_class = NULL;

GtkType
c2_index_get_type (void)
{
    static GtkType type = 0;
	
	if (!type) {
		static GtkTypeInfo info =
		{
			"C2Index",
			sizeof (C2Index),
			sizeof (C2IndexClass),
			(GtkClassInitFunc) class_init,
			(GtkObjectInitFunc) init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL
		};
		
		type = gtk_type_unique (gtk_clist_get_type (), &info);
	}

    return type;
}

static void
class_init (C2IndexClass *klass)
{
    GtkObjectClass *object_class;
    GtkWidgetClass *widget_class;
    GtkContainerClass *container_class;

    object_class = (GtkObjectClass *) klass;
    widget_class = (GtkWidgetClass *) klass;
    container_class = (GtkContainerClass *) klass;

    parent_class = gtk_type_class(gtk_widget_get_type());

    signals[SELECT_MESSAGE] =
		gtk_signal_new("select_message",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET(C2IndexClass, select_message),
						gtk_marshal_NONE__POINTER, GTK_TYPE_NONE, 1,
						GTK_TYPE_POINTER);
	signals[UNSELECT_MESSAGE] =
		gtk_signal_new ("unselect_message",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2IndexClass, unselect_message),
						gtk_marshal_NONE__POINTER, GTK_TYPE_NONE, 1,
						GTK_TYPE_POINTER);
    signals[OPEN_MESSAGE] =
		gtk_signal_new("open_message",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET(C2IndexClass, open_message),
						gtk_marshal_NONE__POINTER, GTK_TYPE_NONE, 1,
						GTK_TYPE_POINTER);
	gtk_object_class_add_signals(object_class, signals, LAST_SIGNAL);

    klass->select_message = NULL;
	klass->unselect_message = NULL;
    klass->open_message = NULL;
	klass->reload = reload;
	klass->sort = sort;
}

static void
init (C2Index *index)
{
	index->application = NULL;
	index->mode = 0;
	index->unreaded_messages = 0;
	index->total_messages = 0;
	index->signal_id_for_marking_unread = -1;
	index->menu = NULL;
}

static void
reload (C2Index *index)
{
	C2Db *db;
	GtkCList *clist = (GtkCList*) index;
	gchar *date_fmt;
	gint selected_mail;

	if (!index->mailbox)
	{
		gtk_clist_clear (clist);
		return;
	}
	
	date_fmt = c2_preferences_get_interface_misc_date ();
	db = index->mailbox->db;

	gtk_clist_freeze (clist);
	gtk_clist_clear (clist);
	if (db)
	{
		do
			add_message (index->application, clist, db, date_fmt);
		while (c2_db_lineal_next (db));
	} else
		gtk_signal_emit (GTK_OBJECT (index), signals[UNSELECT_MESSAGE], NULL);
	g_free (date_fmt);

	selected_mail = get_selected_mail (index);
	if (selected_mail < 0 || selected_mail > clist->rows-1)
		selected_mail = clist->rows-1;

	gtk_clist_thaw (clist);
	gtk_clist_select_row (clist, selected_mail, 1);
}

static gint
sort_status (C2Db *db1, C2Db *db2)
{
	if ((!db1->mark && !db2->mark) ||
		(db1->mark && db2->mark))
	{
		return db1->state - db2->state;
	} else if (db1->mark && !db2->mark)
		return 1;
	else if (!db1->mark && db2->mark)
		return -1;

	return 0;
}

static gint
sort_subject (C2Db *db1, C2Db *db2)
{
	gchar *db1_subject;
	gchar *db2_subject;
	gint retval;

	db1_subject = c2_str_get_striped_subject (db1->subject);
	db2_subject = c2_str_get_striped_subject (db2->subject);

	retval = strcmp (db1_subject, db2_subject);

	g_free (db1_subject);
	g_free (db2_subject);

	return retval;
}

static gint
sort_from (C2Db *db1, C2Db *db2)
{
	return strcmp (db1->from, db2->from);
}

static gint
sort_date (C2Db *db1, C2Db *db2)
{
	return db1->date - db2->date;
}

static gint
sort_account (C2Db *db1, C2Db *db2)
{
	return strcmp (db1->account, db2->account);
}

static gint
sort_mid (C2Db *db1, C2Db *db2)
{
	return 0;
}

static void
sort (C2Index *index, C2MailboxSortBy sort_by, GtkSortType sort_type)
{
	gint (*sort_func) (C2Db *db1, C2Db *db2);
	GtkCList *clist = (GtkCList *) index;
	C2Db *db1, *db2;
	gint i, length;

	if (!index->mailbox)
		return;

	for (i = 0; i < C2_MAILBOX_SORT_LAST; i++)
	{
		if (i == sort_by)
		{
			gtk_arrow_set (GTK_ARROW (index->clist_titles_arrow[i]),
									(sort_type == GTK_SORT_ASCENDING) ?
									GTK_ARROW_UP : GTK_ARROW_DOWN, GTK_SHADOW_OUT);
			gtk_widget_show (index->clist_titles_arrow[i]);
		} else if (i != C2_MAILBOX_SORT_MID)
			gtk_widget_hide (index->clist_titles_arrow[i]);
	}
	
	switch (sort_by)
	{
		case C2_MAILBOX_SORT_STATUS:
			sort_func = sort_status;
			break;
		case C2_MAILBOX_SORT_SUBJECT:
			sort_func = sort_subject;
			break;
		case C2_MAILBOX_SORT_FROM:
			sort_func = sort_from;
			break;
		default:
		case C2_MAILBOX_SORT_DATE:
			sort_func = sort_date;
			break;
		case C2_MAILBOX_SORT_ACCOUNT:
			sort_func = sort_account;
			break;
		case C2_MAILBOX_SORT_MID:
			sort_func = sort_mid;
			break;
	}

	/* Sort */
	length = c2_db_length (index->mailbox);
	
	gtk_clist_freeze (clist);
	if (sort_type == GTK_SORT_DESCENDING)
	{ /* From > to < */
		for (i = length-1; i >= 0; i--)
		{
			gint retval;
			
			db1 = gtk_clist_get_row_data (clist, i-1);
			db2 = gtk_clist_get_row_data (clist, i);
	
			if (!db1 || !db2)
				continue;
	
			retval = sort_func (db1, db2);

			if (retval < 0)
			{
				gint l, pos = -1;
				
				gtk_clist_swap_rows (clist, i, i-1);

				for (l = i+1; l < length; l++)
				{
					db2 = gtk_clist_get_row_data (clist, l);
					
					retval = sort_func (db1, db2);

					if (retval < 0)
						pos = l;
					else
						break;
				}

				if (pos >= 0)
					gtk_clist_row_move (clist, i, pos);
			}
		}
	} else
	{ /* From < to > */
		for (i = length-1; i >= 0; i--)
		{
			gint retval;
			
			db1 = gtk_clist_get_row_data (clist, i-1);
			db2 = gtk_clist_get_row_data (clist, i);
	
			if (!db1 || !db2)
				continue;
	
			retval = sort_func (db1, db2);

			if (retval > 0)
			{
				gint l, pos = -1;
				
				gtk_clist_swap_rows (clist, i, i-1);

				for (l = i+1; l < length; l++)
				{
					db2 = gtk_clist_get_row_data (clist, l);
					
					retval = sort_func (db1, db2);

					if (retval > 0)
						pos = l;
					else
						break;
				}

				if (pos >= 0)
					gtk_clist_row_move (clist, i, pos);
			}
		}
	}
	gtk_clist_thaw (clist);
}

GtkWidget *
c2_index_new (C2Application *application, C2IndexMode mode)
{
    C2Index *index;
	
    index = gtk_type_new (c2_index_get_type());
	c2_index_construct (index, application, mode);

	gtk_signal_connect (GTK_OBJECT (application), "application_preferences_changed",
							GTK_SIGNAL_FUNC (on_application_preferences_changed), index);
	
    return GTK_WIDGET (index);
}

void
c2_index_construct (C2Index *index, C2Application *application, C2IndexMode mode)
{
	GtkWidget *pixmap = NULL, *widget;
	GtkCList *clist;
	gint i;
	static gchar *titles[COLUMN_LAST];
	GladeXML *xml;
	GList *l;

	index->application = application;
	index->mode = mode;

	/* create the clist */
    gtk_clist_construct (GTK_CLIST (index), COLUMN_LAST, titles);
    clist = GTK_CLIST (index);

	/* Create the columns */
	for (i = 0; i < COLUMN_LAST; i++)
	{
		GtkWidget *hbox = gtk_hbox_new (FALSE, GNOME_PAD_SMALL), *hbox2;
		GtkWidget *label;

		switch (i)
		{
			case COLUMN_STYLE:
				label = NULL;
				pixmap = gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/mark-unreaded.png");
				break;
			case COLUMN_SUBJECT:
				label = gtk_label_new (_("Subject")); break;
			case COLUMN_FROM:
				label = gtk_label_new (_("From")); break;
			case COLUMN_DATE:
				label = gtk_label_new (_("Date")); break;
			case COLUMN_ACCOUNT:
				label = gtk_label_new (_("Account")); break;
			default:
				g_assert_not_reached ();
				return;
		}

		if (label)
		{
			gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);
			gtk_widget_show (label);
			gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
		} else
		{
			gtk_box_pack_start (GTK_BOX (hbox), pixmap, TRUE, TRUE, 0);
			gtk_widget_show (pixmap);
		}

		index->clist_titles_arrow[i] = gtk_arrow_new (GTK_ARROW_DOWN, GTK_SHADOW_OUT);
		gtk_box_pack_start (GTK_BOX (hbox), index->clist_titles_arrow[i], TRUE, TRUE, 0);
		
		hbox2 = gtk_hbox_new (TRUE, 0);
		gtk_box_pack_start (GTK_BOX (hbox2), hbox, FALSE, FALSE, 0);
		gtk_clist_set_column_widget (clist, i, hbox2);
		gtk_widget_show (hbox2);
		gtk_widget_show (hbox);
	}
	
    gtk_signal_connect (GTK_OBJECT (index), "click_column",
		       GTK_SIGNAL_FUNC(on_index_click_column), NULL);

	gtk_clist_columns_autosize (clist);
	gtk_clist_column_titles_show (clist);
    gtk_clist_set_row_height (clist, 16);
	gtk_clist_set_selection_mode (clist, GTK_SELECTION_EXTENDED);

    gtk_signal_connect (GTK_OBJECT (clist), "select-row",
						GTK_SIGNAL_FUNC (on_clist_select_row), NULL);
	gtk_signal_connect (GTK_OBJECT (clist), "unselect-row",
						GTK_SIGNAL_FUNC (on_clist_unselect_row), NULL);
	gtk_signal_connect (GTK_OBJECT (clist), "button_press_event",
						GTK_SIGNAL_FUNC (on_clist_button_press_event), NULL);
	gtk_signal_connect (GTK_OBJECT (clist), "resize_column",
						GTK_SIGNAL_FUNC (on_resize_column), NULL);

	gtk_clist_set_column_width (clist, COLUMN_STYLE, c2_preferences_get_window_main_index_width_column_style ());
	gtk_clist_set_column_width (clist, COLUMN_SUBJECT, c2_preferences_get_window_main_index_width_column_subject ());
	gtk_clist_set_column_width (clist, COLUMN_FROM, c2_preferences_get_window_main_index_width_column_from ());
	gtk_clist_set_column_width (clist, COLUMN_DATE, c2_preferences_get_window_main_index_width_column_date ());
	gtk_clist_set_column_width (clist, COLUMN_ACCOUNT, c2_preferences_get_window_main_index_width_column_account ());

	/* Create the menu */
	xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("cronosII"), "mnu_index");
	index->menu = glade_xml_get_widget (xml, "mnu_index");
	gtk_object_set_data (GTK_OBJECT (index->menu), "xml", xml);

	widget = glade_xml_get_widget (xml, "open_message");
	for (l = gtk_container_children (GTK_CONTAINER (widget)); l; l = g_list_next (l))
	{
		if (GTK_IS_LABEL (l->data))
		{
			GtkStyle *style;
			
			widget = (GtkWidget*) l->data;
			style = gtk_style_copy (gtk_widget_get_style (widget));
			style->font = gdk_font_load (c2_font_bold);
			gtk_widget_set_style (widget, style);
			break;
		}
	}

	widget = glade_xml_get_widget (xml, "open_message");
	gtk_signal_connect (GTK_OBJECT (widget), "activate",
						GTK_SIGNAL_FUNC (on_mnu_open_message_activate), index);

	widget = glade_xml_get_widget (xml, "reply");
	gtk_signal_connect (GTK_OBJECT (widget), "activate",
						GTK_SIGNAL_FUNC (on_mnu_reply_activate), index);
	widget = glade_xml_get_widget (xml, "reply_all");
	gtk_signal_connect (GTK_OBJECT (widget), "activate",
						GTK_SIGNAL_FUNC (on_mnu_reply_all_activate), index);
	widget = glade_xml_get_widget (xml, "forward");
	gtk_signal_connect (GTK_OBJECT (widget), "activate",
						GTK_SIGNAL_FUNC (on_mnu_forward_activate), index);

	widget = glade_xml_get_widget (xml, "delete");
	gtk_signal_connect (GTK_OBJECT (widget), "activate",
						GTK_SIGNAL_FUNC (on_mnu_delete_activate), index);
	widget = glade_xml_get_widget (xml, "expunge");
	gtk_signal_connect (GTK_OBJECT (widget), "activate",
						GTK_SIGNAL_FUNC (on_mnu_expunge_activate), index);
	widget = glade_xml_get_widget (xml, "mark_important");
	gtk_signal_connect (GTK_OBJECT (widget), "activate",
						GTK_SIGNAL_FUNC (on_mnu_mark_important_activate), index);
	widget = glade_xml_get_widget (xml, "mark_unreaded");
	gtk_signal_connect (GTK_OBJECT (widget), "activate",
						GTK_SIGNAL_FUNC (on_mnu_mark_unreaded_activate), index);
	widget = glade_xml_get_widget (xml, "mark_readed");
	gtk_signal_connect (GTK_OBJECT (widget), "activate",
						GTK_SIGNAL_FUNC (on_mnu_mark_readed_activate), index);
	widget = glade_xml_get_widget (xml, "mark_replied");
	gtk_signal_connect (GTK_OBJECT (widget), "activate",
						GTK_SIGNAL_FUNC (on_mnu_mark_replied_activate), index);
	widget = glade_xml_get_widget (xml, "mark_forwarded");
	gtk_signal_connect (GTK_OBJECT (widget), "activate",
						GTK_SIGNAL_FUNC (on_mnu_mark_forwarded_activate), index);
	
	widget = glade_xml_get_widget (xml, "copy");
	gtk_signal_connect (GTK_OBJECT (widget), "activate",
						GTK_SIGNAL_FUNC (on_mnu_copy_activate), index);
	widget = glade_xml_get_widget (xml, "move");
	gtk_signal_connect (GTK_OBJECT (widget), "activate",
						GTK_SIGNAL_FUNC (on_mnu_move_activate), index);
	
	widget = glade_xml_get_widget (xml, "previous");
	gtk_signal_connect (GTK_OBJECT (widget), "activate",
						GTK_SIGNAL_FUNC (on_mnu_previous_activate), index);
	widget = glade_xml_get_widget (xml, "next");
	gtk_signal_connect (GTK_OBJECT (widget), "activate",
						GTK_SIGNAL_FUNC (on_mnu_next_activate), index);
	
	widget = glade_xml_get_widget (xml, "save");
	gtk_signal_connect (GTK_OBJECT (widget), "activate",
						GTK_SIGNAL_FUNC (on_mnu_save_activate), index);
	widget = glade_xml_get_widget (xml, "print");
	gtk_signal_connect (GTK_OBJECT (widget), "activate",
						GTK_SIGNAL_FUNC (on_mnu_print_activate), index);
}

void
c2_index_set_mailbox (C2Index *index, C2Mailbox *mailbox)
{
	c2_return_if_fail_obj (C2_IS_MAILBOX (mailbox), C2EDATA, GTK_OBJECT (index));

    disconnect (index);
    index->mailbox = mailbox;
    _connect (index);

    C2_INDEX_CLASS_FW (index)->reload (index);
}

void
c2_index_add_mail (C2Index *index, C2Db *db_node)
{
	C2_TODO;
}

void
c2_index_clear (C2Index *index)
{
	GtkCList *clist = (GtkCList*) index;

	disconnect (index);
	index->mailbox = NULL;
	
	gtk_clist_freeze (clist);
	gtk_clist_clear (clist);
	gtk_clist_thaw (clist);
}

static void
select_row (GtkCList *clist, gint row, gint column)
{
	gtk_clist_freeze (clist);
	gtk_clist_unselect_all (clist);
	gtk_clist_select_row (clist, row, column);
	gtk_clist_thaw (clist);
}

void
c2_index_select_message (C2Index *index, C2Db *db)
{
	GtkCList *clist = (GtkCList*) index;
	gint row;

	/* Search for the row with this node */
	if ((row = gtk_clist_find_row_from_data (clist, db)) < 0)
		return;

	select_row (clist, row, 3);
}

void
c2_index_select_next_message (C2Index *index)
{
	GtkCList *clist = (GtkCList*) index;
	gint sr;

	sr = clist->selection ? GPOINTER_TO_INT (clist->selection->data) : -1;

	if (sr < 0 || sr >= clist->rows)
		return;
	
	select_row (clist, sr+1, 3);
}

void
c2_index_select_previous_message (C2Index *index)
{
	GtkCList *clist = (GtkCList*) index;
	gint sr;

	sr = clist->selection ? GPOINTER_TO_INT (clist->selection->data) : -1;

	if (sr <= 0)
		return;

	select_row (clist, sr-1, 3);
}

C2Db *
c2_index_selection_main (C2Index *index)
{
	GtkCList *clist = (GtkCList*) index;
	gint row;

	if (!clist->selection)
		return NULL;

	row = GPOINTER_TO_INT (clist->selection->data);

	return gtk_clist_get_row_data (clist, row);
}

GList *
c2_index_selection (C2Index *index)
{
	GtkCList *clist = (GtkCList*) index;
	GList *l, *list = NULL;

	for (l = clist->selection; l; l = g_list_next (l))
		list = g_list_append (list,
						gtk_clist_get_row_data (clist, GPOINTER_TO_INT (l->data)));

	return list;
}

void
c2_index_sort (C2Index *index, C2MailboxSortBy sort_by, GtkSortType sort_type)
{
	gtk_clist_freeze (GTK_CLIST (index));
	C2_INDEX_CLASS_FW (index)->sort (index, sort_by, sort_type);
	gtk_clist_thaw (GTK_CLIST (index));
}

static void
add_message (C2Application *application, GtkCList *clist, C2Db *db, const gchar *date_fmt)
{
	struct tm *tm;
	gchar *row[COLUMN_LAST];
	
	tm = localtime (&db->date);
	row[COLUMN_SUBJECT] = db->subject;
	row[COLUMN_FROM] = db->from;
	row[COLUMN_ACCOUNT] = db->account;
	row[COLUMN_DATE] = g_new (gchar, 128);
	strftime (row[COLUMN_DATE], 128, date_fmt, tm);
	
	gtk_clist_append (clist, row);
	
	reload_node (application, clist, clist->rows-1, db);
	
	gtk_clist_set_row_data (clist, clist->rows-1, db);

	g_free (row[COLUMN_DATE]);
}

static void
reload_node (C2Application *application, GtkCList *clist, gint row, C2Db *db)
{
	GtkStyle *style;

	style = gtk_style_copy ((style = gtk_clist_get_row_style (clist, row)) ?
						style : gtk_widget_get_style (GTK_WIDGET (clist)));

	/* Set the font */
	if (db->mark)
		style->font = application->fonts_gdk_unreaded_mails;
	else
	{
		switch (db->state)
		{
			case C2_MESSAGE_READED:
				style->font = application->fonts_gdk_readed_mails;
				break;
			case C2_MESSAGE_UNREADED:
				gtk_clist_set_pixmap (clist, row, 0, application->pixmap_unread, application->mask_unread);
				style->font = application->fonts_gdk_unreaded_mails;
				break;
			case C2_MESSAGE_FORWARDED:
				gtk_clist_set_pixmap (clist, row, 0, application->pixmap_forward, application->mask_forward);
				style->font = application->fonts_gdk_readed_mails;
				break;
			case C2_MESSAGE_REPLIED:
				gtk_clist_set_pixmap (clist, row, 0, application->pixmap_reply, application->mask_reply);
				style->font = application->fonts_gdk_readed_mails;
				break;
			default:
				gtk_clist_set_pixmap (clist, row, 0, application->pixmap_read, application->mask_read);
				style->font = application->fonts_gdk_readed_mails;
		}
	}

	/* Set the pixmap */
	switch (db->state)
	{
		case C2_MESSAGE_READED:
			if (db->mark)
				gtk_clist_set_pixmap (clist, row, 0, application->pixmap_i_read,
										application->mask_i_read);
			else
				gtk_clist_set_pixmap (clist, row, 0, application->pixmap_read,
										application->mask_read);
			break;
		case C2_MESSAGE_UNREADED:
			if (db->mark)
				gtk_clist_set_pixmap (clist, row, 0, application->pixmap_i_unread,
										application->mask_i_unread);
			else
				gtk_clist_set_pixmap (clist, row, 0, application->pixmap_unread,
										application->mask_unread);
			break;
		case C2_MESSAGE_REPLIED:
			if (db->mark)
				gtk_clist_set_pixmap (clist, row, 0, application->pixmap_i_reply,
										application->mask_i_reply);
			else
				gtk_clist_set_pixmap (clist, row, 0, application->pixmap_reply,
										application->mask_reply);
			break;
		case C2_MESSAGE_FORWARDED:
			if (db->mark)
				gtk_clist_set_pixmap (clist, row, 0, application->pixmap_i_forward,
										application->mask_i_forward);
			else
				gtk_clist_set_pixmap (clist, row, 0, application->pixmap_forward,
										application->mask_forward);
			break;
	}

	/* Set the color (if applicable */
	if (db->mark)
	{
		GdkColor dark_red = { 0, 0xcd00, 0, 0 };

		gdk_color_alloc (gdk_colormap_get_system (), &dark_red);
		
		style->fg[GTK_STATE_NORMAL] = dark_red;
	}
	
	gtk_clist_set_row_style (clist, row, style);
}

static gint
get_selected_mail (C2Index *index)
{
	C2Mailbox *mailbox = index->mailbox;
	gpointer i;

	i = gtk_object_get_data (GTK_OBJECT (mailbox), SELECTED_MAIL);

	if (!i)
		return -1;

	return GPOINTER_TO_INT (i)-1;
}

static void
set_selected_mail (C2Index *index, gint i)
{
	C2Mailbox *mailbox = index->mailbox;

	gtk_object_set_data (GTK_OBJECT (mailbox), SELECTED_MAIL, (gpointer) (i+1));
}

static void
on_db_unreaded_timeout_thread (C2Db *db)
{
	c2_db_message_set_state (db, C2_MESSAGE_READED);
}

static gboolean
on_db_unreaded_timeout (C2Db *db)
{
	pthread_t thread;

	pthread_create (&thread, NULL, C2_PTHREAD_FUNC (on_db_unreaded_timeout_thread), db);
	
	return FALSE;
}

static void
on_clist_select_row (C2Index *index, gint row, gint column, GdkEvent *event)
{
	GtkVisibility visibility;
	GtkCList *clist = GTK_CLIST (index);
	C2Db *node;
	gint top;

	/* Check if there's a timeout for marking a message
	 * as unread running in the background.
	 */
	if (index->signal_id_for_marking_unread >= 0)
	{
		gtk_timeout_remove (index->signal_id_for_marking_unread);
		index->signal_id_for_marking_unread = -1;
	}
	
	/* Get the Db node */
	if ((node = C2_DB (gtk_clist_get_row_data (GTK_CLIST (GTK_WIDGET (index)), row))))
		gtk_signal_emit (GTK_OBJECT (index), signals[SELECT_MESSAGE], node);
	set_selected_mail (index, row);

	if (g_list_length (clist->selection) == 1)
	{
		/* Now scroll */
		visibility = gtk_clist_row_is_visible (clist, row);

		switch (visibility)
		{
			case GTK_VISIBILITY_NONE:
				gtk_clist_moveto (clist, row, -1, 0.5, 0);
				break;
			case GTK_VISIBILITY_PARTIAL:
				top = ((clist->row_height * row) + ((row + 1) * 1) + clist->voffset);
				
				if ((top < 0))
					gtk_clist_moveto (clist, row, -1, 0.2, 0);
				else
					gtk_clist_moveto (clist, row, -1, 0.8, 0);
		}


		/* Connect a timeout for the marking of messages as readed */
		if (index->mode == C2_INDEX_READ_WRITE)
		{
			if (C2_IS_DB (node) && node->state == C2_MESSAGE_UNREADED)
				index->signal_id_for_marking_unread =
						gtk_timeout_add (c2_preferences_get_general_options_timeout_mark ()*1000,
									on_db_unreaded_timeout, node);
		}
	}
}

static void
on_clist_unselect_row (C2Index *index, gint row, gint column, GdkEvent *event)
{
	C2Db *node;

	node = C2_DB (gtk_clist_get_row_data (GTK_CLIST (GTK_WIDGET (index)), row));
	gtk_signal_emit (GTK_OBJECT (index), signals[UNSELECT_MESSAGE], node);
	set_selected_mail (index, -1);
}

static void
on_clist_button_press_event (C2Index *index, GdkEvent *e)
{
	GdkEventButton *button = (GdkEventButton*) e;
	GtkCList *clist = (GtkCList*) index;
	gint row, col;
	C2Db *db;
	GladeXML *xml;
	GtkWidget *widget;

	gtk_clist_get_selection_info ((GtkCList*) index, button->x, button->y, &row, &col);
	if (row < 0)
		return;

	select_row ((GtkCList*) index, row, col);
	
	switch (button->button)
	{
		case 2:
			/* Button 2 toggles the important mark */
			if (index->mode == C2_INDEX_READ_ONLY)
				return;

			db = (C2Db*) gtk_clist_get_row_data ((GtkCList*) index, row);
			if (!db)
				return;
		
			gdk_threads_leave ();
			c2_db_message_set_mark (db, !db->mark);
			gdk_threads_enter ();
			break;
			
		case 3:
			/* Button 3 popups a menu */
			xml = gtk_object_get_data (GTK_OBJECT (index->menu), "xml");
			
			widget = glade_xml_get_widget (xml, "previous");
			gtk_widget_set_sensitive (widget, row);

			widget = glade_xml_get_widget (xml, "next");
			gtk_widget_set_sensitive (widget, clist->rows-1 != row);
			
			gnome_popup_menu_do_popup (index->menu, NULL, NULL, button, NULL);
			break;
	}
}

static void
on_resize_column (C2Index *index, gint column, gint width)
{
	switch (column)
	{
		case COLUMN_STYLE:
			c2_preferences_set_window_main_index_width_column_style (width);
			break;
		case COLUMN_SUBJECT:
			c2_preferences_set_window_main_index_width_column_subject (width);
			break;
		case COLUMN_FROM:
			c2_preferences_set_window_main_index_width_column_from (width);
			break;
		case COLUMN_DATE:
			c2_preferences_set_window_main_index_width_column_date (width);
			break;
		case COLUMN_ACCOUNT:
			c2_preferences_set_window_main_index_width_column_account (width);
			break;
	}

	c2_preferences_commit ();
}

static void
on_index_click_column (GtkCList *clist, gint column, gpointer data)
{
	C2Index *index = C2_INDEX (clist);

	if (!index->mailbox)
		return;
	
	if (clist->sort_column == column)
		index->mailbox->sort_type = (index->mailbox->sort_type == GTK_SORT_ASCENDING) ?
							GTK_SORT_DESCENDING : GTK_SORT_ASCENDING;
	gtk_clist_set_sort_column (clist, column);
	switch (column)
	{
		case 0: index->mailbox->sort_by = C2_MAILBOX_SORT_STATUS; break;
		case 1: index->mailbox->sort_by = C2_MAILBOX_SORT_SUBJECT; break;
		case 2: index->mailbox->sort_by = C2_MAILBOX_SORT_FROM; break;
		default: case 3: index->mailbox->sort_by = C2_MAILBOX_SORT_DATE; break;
		case 4: index->mailbox->sort_by = C2_MAILBOX_SORT_ACCOUNT; break;
	}
	
	C2_INDEX_CLASS_FW (index)->sort (index, index->mailbox->sort_by, index->mailbox->sort_type);
}

static void
on_mnu_open_message_activate (GtkWidget *widget, C2Index *index)
{
	C2Application *application = index->application;
	C2Db *db = c2_index_selection_main (index);

	if (!db)
		return;
	
	C2_APPLICATION_CLASS_FW (application)->open_message (application, db, NULL);
}

static void
on_mnu_reply_activate (GtkWidget *widget, C2Index *index)
{
	C2Application *application = index->application;
	C2Db *db = c2_index_selection_main (index);

	if (!db)
		return;

	gdk_threads_leave ();
	c2_db_load_message (db);
	gdk_threads_enter ();
	
	C2_APPLICATION_CLASS_FW (application)->reply (application, db, db->message);
}

static void
on_mnu_reply_all_activate (GtkWidget *widget, C2Index *index)
{
	C2Application *application = index->application;
	C2Db *db = c2_index_selection_main (index);

	if (!db)
		return;

	gdk_threads_leave ();
	c2_db_load_message (db);
	gdk_threads_enter ();
	
	C2_APPLICATION_CLASS_FW (application)->reply_all (application, db, db->message);
}

static void
on_mnu_forward_activate (GtkWidget *widget, C2Index *index)
{
	C2Application *application = index->application;
	C2Db *db = c2_index_selection_main (index);

	if (!db)
		return;

	gdk_threads_leave ();
	c2_db_load_message (db);
	gdk_threads_enter ();
	
	C2_APPLICATION_CLASS_FW (application)->forward (application, db, db->message);
}

static void
on_mnu_delete_activate (GtkWidget *widget, C2Index *index)
{
	C2Application *application = index->application;
	C2Db *db = c2_index_selection_main (index);

	if (!db)
		return;
	
	C2_APPLICATION_CLASS_FW (application)->delete (application, g_list_prepend (NULL, db), NULL);
}

static void
on_mnu_expunge_activate (GtkWidget *widget, C2Index *index)
{
	C2Application *application = index->application;
	C2Db *db = c2_index_selection_main (index);

	if (!db)
		return;
	
	C2_APPLICATION_CLASS_FW (application)->expunge (application, g_list_prepend (NULL, db), NULL);
}

static void
on_mnu_mark_important_activate (GtkWidget *widget, C2Index *index)
{
	C2Db *db = c2_index_selection_main (index);

	if (!db)
		return;
	
	gdk_threads_leave ();
	c2_db_message_set_mark (db, !db->mark);
	gdk_threads_enter ();
}

static void
on_mnu_mark_unreaded_activate (GtkWidget *widget, C2Index *index)
{
	C2Db *db = c2_index_selection_main (index);

	if (!db)
		return;

	gdk_threads_leave ();
	c2_db_message_set_state (db, C2_MESSAGE_UNREADED);
	gdk_threads_enter ();
}

static void
on_mnu_mark_readed_activate (GtkWidget *widget, C2Index *index)
{
	C2Db *db = c2_index_selection_main (index);

	if (!db)
		return;

	gdk_threads_leave ();
	c2_db_message_set_state (db, C2_MESSAGE_READED);
	gdk_threads_enter ();
}

static void
on_mnu_mark_replied_activate (GtkWidget *widget, C2Index *index)
{
	C2Db *db = c2_index_selection_main (index);

	if (!db)
		return;

	gdk_threads_leave ();
	c2_db_message_set_state (db, C2_MESSAGE_REPLIED);
	gdk_threads_enter ();
}

static void
on_mnu_mark_forwarded_activate (GtkWidget *widget, C2Index *index)
{
	C2Db *db = c2_index_selection_main (index);

	if (!db)
		return;

	gdk_threads_leave ();
	c2_db_message_set_state (db, C2_MESSAGE_FORWARDED);
	gdk_threads_enter ();
}

static void
on_mnu_copy_activate (GtkWidget *widget, C2Index *index)
{
	C2Application *application = index->application;
	C2Db *db = c2_index_selection_main (index);

	if (!db)
		return;
	
	C2_APPLICATION_CLASS_FW (application)->copy (application, g_list_prepend (NULL, db), NULL);
}

static void
on_mnu_move_activate (GtkWidget *widget, C2Index *index)
{
	C2Application *application = index->application;
	C2Db *db = c2_index_selection_main (index);

	if (!db)
		return;
	
	C2_APPLICATION_CLASS_FW (application)->move (application, g_list_prepend (NULL, db), NULL);
}

static void
on_mnu_previous_activate (GtkWidget *widget, C2Index *index)
{
	c2_index_select_previous_message (index);
}

static void
on_mnu_next_activate (GtkWidget *widget, C2Index *index)
{
	c2_index_select_next_message (index);
}

static void
on_mnu_save_activate (GtkWidget *widget, C2Index *index)
{
	C2Application *application = index->application;
	C2Db *db = c2_index_selection_main (index);

	if (!db)
		return;
	
	gdk_threads_leave ();
	c2_db_load_message (db);
	gdk_threads_enter ();

	gtk_object_ref (GTK_OBJECT (db->message));
	C2_APPLICATION_CLASS_FW (application)->save (application, db->message, NULL);
	gtk_object_unref (GTK_OBJECT (db->message));
}

static void
on_mnu_print_activate (GtkWidget *widget, C2Index *index)
{
	C2Application *application = index->application;
	C2Db *db = c2_index_selection_main (index);

	if (!db)
		return;

	gdk_threads_leave ();
	c2_db_load_message (db);
	gdk_threads_enter ();

	gtk_object_ref (GTK_OBJECT (db->message));
	C2_APPLICATION_CLASS_FW (application)->print (application, db->message);
	gtk_object_unref (GTK_OBJECT (db->message));
}

static void
on_application_preferences_changed (C2Application *application, gint key, gpointer value,
	 C2Index *index)
{
	if (key == C2_DIALOG_PREFERENCES_KEY_INTERFACE_FONTS_UNREADED_MAILS ||
		key == C2_DIALOG_PREFERENCES_KEY_INTERFACE_FONTS_READED_MAILS)
		reload (index);
}

static void
on_mailbox_changed_mailbox (C2Mailbox *mailbox, C2MailboxChangeType type, C2Db *db_node, C2Index *index)
{
	/* TODO This is a temp fix */
	/* WARN This function assumes it is being called
	 * by a separated thread since all DB action is
	 * supposed to run in other thread.
	 */
	switch (type)
	{
		case C2_MAILBOX_CHANGE_STATE:
			{
				gint row;
				
				if ((row = gtk_clist_find_row_from_data ((GtkCList*) index, db_node)) < 0)
					return;

				gdk_threads_enter ();
				reload_node (index->application, (GtkCList*) index, row, db_node);
				gtk_widget_queue_draw ((GtkWidget*) index);
				gdk_threads_leave ();
			}
			break;
			
		default:
			gdk_threads_enter ();
			C2_INDEX_CLASS_FW (index)->reload (index);
			gdk_threads_leave ();
	}
}

static void
_connect (C2Index *index)
{
	if (!C2_IS_MAILBOX (index->mailbox))
		return;

	gtk_signal_connect (GTK_OBJECT (index->mailbox), "changed_mailbox",
						GTK_SIGNAL_FUNC (on_mailbox_changed_mailbox), index);
}

static void
disconnect (C2Index *index)
{
	if (C2_IS_MAILBOX (index->mailbox))
		gtk_signal_disconnect_by_func (GTK_OBJECT (index->mailbox),
							GTK_SIGNAL_FUNC (on_mailbox_changed_mailbox), index);
}
