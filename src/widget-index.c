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
#include <time.h>

#include <libcronosII/error.h>

#include "widget-application.h"
#include "widget-index.h"

static void
class_init									(C2IndexClass *klass);

static void
init										(C2Index *index);

static void
on_index_click_column						(GtkCList *clist, gint column, gpointer data);

static void
sort										(C2Index *index);

static void
select_row									(C2Index *index, gint row, gint column, GdkEvent *event);

static void
on_resize_column							(C2Index *index, gint column, gint width);

/* signals */
enum
{
	SELECT_MESSAGE,			/* Click in a row */
	OPEN_MESSAGE,			/* Double click in a row */
	DELETE_MESSAGE,			/* Drop Down menu >> delete */
	EXPUNGE_MESSAGE,		/* Drop Down menu >> expunge */
	MOVE_MESSAGE,			/* Drop Down menu >> move */
	COPY_MESSAGE,			/* Drop Down menu >> copy */
	REPLY_MESSAGE,			/* Drop Down menu >> reply */
	REPLY_ALL_MESSAGE,		/* Drop Down menu >> reply all */
	FORWARD_MESSAGE,		/* Drop Down menu >> forward */
	PRINT_MESSAGE,			/* Drop Down menu >> print */
	SAVE_MESSAGE,			/* Drop Down menu >> save */
	ADD_CONTACT,			/* Drop Down menu >> add contact */
	LAST_SIGNAL
};

static gint signals[LAST_SIGNAL] = { 0 };

static GtkCListClass *parent_class = NULL;

guint
c2_index_get_type (void)
{
    static guint type = 0;
	
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
    signals[OPEN_MESSAGE] =
		gtk_signal_new("open_message",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET(C2IndexClass, open_message),
						gtk_marshal_NONE__POINTER, GTK_TYPE_NONE, 1,
						GTK_TYPE_POINTER);
	signals[DELETE_MESSAGE] =
		gtk_signal_new("delete_message",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET(C2IndexClass, delete_message),
						gtk_marshal_NONE__POINTER, GTK_TYPE_NONE, 1,
						GTK_TYPE_POINTER);
	signals[EXPUNGE_MESSAGE] =
		gtk_signal_new("expunge_message",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET(C2IndexClass, expunge_message),
						gtk_marshal_NONE__POINTER, GTK_TYPE_NONE, 1,
						GTK_TYPE_POINTER);
	signals[MOVE_MESSAGE] =
		gtk_signal_new("move_message",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET(C2IndexClass, move_message),
						gtk_marshal_NONE__POINTER, GTK_TYPE_NONE, 1,
						GTK_TYPE_POINTER);
	signals[COPY_MESSAGE] =
		gtk_signal_new("copy_message",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET(C2IndexClass, copy_message),
						gtk_marshal_NONE__POINTER, GTK_TYPE_NONE, 1,
						GTK_TYPE_POINTER);
	signals[REPLY_MESSAGE] =
		gtk_signal_new("reply_message",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET(C2IndexClass, reply_message),
						gtk_marshal_NONE__POINTER, GTK_TYPE_NONE, 1,
						GTK_TYPE_POINTER);
	signals[REPLY_ALL_MESSAGE] =
		gtk_signal_new("reply_all_message",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET(C2IndexClass, reply_all_message),
						gtk_marshal_NONE__POINTER, GTK_TYPE_NONE, 1,
						GTK_TYPE_POINTER);
	signals[FORWARD_MESSAGE] =
		gtk_signal_new("forward_message",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET(C2IndexClass, forward_message),
						gtk_marshal_NONE__POINTER, GTK_TYPE_NONE, 1,
						GTK_TYPE_POINTER);
	signals[PRINT_MESSAGE] =
		gtk_signal_new("print_message",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET(C2IndexClass, print_message),
						gtk_marshal_NONE__POINTER_POINTER, GTK_TYPE_NONE, 2,
						GTK_TYPE_ENUM, GTK_TYPE_POINTER);
	signals[SAVE_MESSAGE] =
		gtk_signal_new("save_message",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET(C2IndexClass, save_message),
						gtk_marshal_NONE__POINTER, GTK_TYPE_NONE, 1,
						GTK_TYPE_POINTER);
	signals[ADD_CONTACT] =
		gtk_signal_new("add_contact",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET(C2IndexClass, add_contact),
						gtk_marshal_NONE__POINTER, GTK_TYPE_NONE, 1,
						GTK_TYPE_POINTER);
	gtk_object_class_add_signals(object_class, signals, LAST_SIGNAL);

    klass->select_message = NULL;
    klass->open_message = NULL;
	klass->delete_message = NULL;
	klass->expunge_message = NULL;
	klass->move_message = NULL;
	klass->copy_message = NULL;
	klass->reply_message = NULL;
	klass->reply_all_message = NULL;
	klass->forward_message = NULL;
	klass->print_message = NULL;
	klass->save_message = NULL;
	klass->add_contact = NULL;
}

static void
init (C2Index *index)
{
	GtkWidget *pixmap;
	GtkCList *clist;
	gint i;
	static gchar *titles[] = {
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	};

    /* create the clist */
    gtk_clist_construct (GTK_CLIST (index), 7, titles);
    clist = GTK_CLIST (index);

	/* Create the columns */
	for (i = 0; i < 8; i++)
	{
		GtkWidget *hbox = gtk_hbox_new (FALSE, GNOME_PAD_SMALL);
		GtkWidget *label;

		switch (i)
		{
			case 0:
				i++;
			case 1:
				label = gtk_label_new (NULL); break;
			case 2:
				label = gtk_label_new (NULL); break;
			case 3:
				label = gtk_label_new (_("Subject")); break;
			case 4:
				label = gtk_label_new (_("From")); break;
			case 5:
				label = gtk_label_new (_("Date")); break;
			case 6:
				label = gtk_label_new (_("Account")); break;
			case 7:
				label = gtk_label_new (NULL); break;
		}

		gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);
		gtk_widget_show (label);

		index->clist_titles_arrow[i] = gtk_arrow_new (GTK_ARROW_DOWN, GTK_SHADOW_OUT);
		gtk_box_pack_start (GTK_BOX (hbox), index->clist_titles_arrow[i], FALSE, FALSE, 0);
		
		gtk_clist_set_column_widget (clist, i, hbox);
		gtk_widget_show (hbox);
	}
	
    gtk_signal_connect (GTK_OBJECT (index), "click_column",
		       GTK_SIGNAL_FUNC(on_index_click_column), NULL);

	pixmap = gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/unread.png");
	gtk_clist_set_column_widget (clist, 0, pixmap);
	
	gtk_clist_columns_autosize (clist);
	gtk_clist_set_column_visibility (clist, 1, FALSE);
	gtk_clist_set_column_visibility (clist, 2, FALSE);
	gtk_clist_set_column_visibility (clist, 7, FALSE);
	gtk_clist_column_titles_show (clist);
    gtk_clist_set_row_height (clist, 16);
	gtk_clist_set_selection_mode (clist, GTK_SELECTION_EXTENDED);

    gtk_signal_connect (GTK_OBJECT (clist), "select-row",
						GTK_SIGNAL_FUNC (select_row), NULL);
/*	gtk_signal_connect (GTK_OBJECT (clist), "unselect-row",
						GTK_SIGNAL_FUNC (unselect_row), NULL);
	gtk_signal_connect (GTK_OBJECT (clist), "button_press_event",
						GTK_SIGNAL_FUNC (button_press_event), NULL); */
	gtk_signal_connect (GTK_OBJECT (clist), "resize_column",
						GTK_SIGNAL_FUNC (on_resize_column), NULL);

    gtk_widget_show (GTK_WIDGET (clist));
    gtk_widget_ref (GTK_WIDGET (clist));

	index->unreaded_messages = 0;
	index->total_messages = 0;
	index->mbox = NULL;
}

GtkWidget *
c2_index_new (C2Application *application)
{
    C2Index *index;
	GtkCList *clist;
	
    index = gtk_type_new (c2_index_get_type());
	clist = GTK_CLIST (index);

	index->application = application;

	gtk_clist_set_column_width (clist, 0, atoi (DEFAULT_RC_CLIST_0));
	gtk_clist_set_column_width (clist, 1, atoi (DEFAULT_RC_CLIST_1));
	gtk_clist_set_column_width (clist, 2, atoi (DEFAULT_RC_CLIST_2));
	gtk_clist_set_column_width (clist, 3, atoi (DEFAULT_RC_CLIST_3));
	gtk_clist_set_column_width (clist, 4, atoi (DEFAULT_RC_CLIST_4));
	gtk_clist_set_column_width (clist, 5, atoi (DEFAULT_RC_CLIST_5));
	gtk_clist_set_column_width (clist, 6, atoi (DEFAULT_RC_CLIST_6));
	gtk_clist_set_column_width (clist, 7, atoi (DEFAULT_RC_CLIST_7));
	
	
	
    return GTK_WIDGET (index);
}

void
c2_index_add_mailbox (C2Index *index, C2Mailbox *mbox)
{
	C2Db *db;
	GtkCList *clist = GTK_CLIST (GTK_WIDGET (index));
	C2Application *application = index->application;
	
	c2_return_if_fail (mbox, C2EDATA);

	db = mbox->db;

	gtk_clist_freeze (clist);
	if (db)
	{
		do
		{
			struct tm *tm;
			gchar *row[] = {
				NULL, NULL, NULL, db->subject, db->from, NULL, db->account, g_strdup_printf ("%d", db->position)
			};
			gchar *tmp = g_strdup_printf ("%d", db->mid);
			gchar *info[] = {
				NULL, NULL, NULL, db->subject, db->from, NULL, db->account, tmp
			};
			tm = localtime (&db->date);
			row[5] = g_new (gchar, 128);
			strftime (row[5], 128, application->interface_date_fmt, tm);
			
			gtk_clist_append (clist, row);
			
			/* Set the state */
			switch (db->state)
			{
				case C2_MESSAGE_READED:
					gtk_clist_set_pixmap (clist, clist->rows-1, 0, application->pixmap_read, application->mask_read);
					break;
				case C2_MESSAGE_UNREADED:
					gtk_clist_set_pixmap (clist, clist->rows-1, 0, application->pixmap_unread, application->mask_unread);
					break;
				case C2_MESSAGE_FORWARDED:
					gtk_clist_set_pixmap (clist, clist->rows-1, 0, application->pixmap_forward, application->mask_forward);
					break;
				case C2_MESSAGE_REPLIED:
					gtk_clist_set_pixmap (clist, clist->rows-1, 0, application->pixmap_reply, application->mask_reply);
					break;
				default:
					gtk_clist_set_pixmap (clist, clist->rows-1, 0, application->pixmap_read, application->mask_read);
			}
			
			gtk_clist_set_row_data (clist, clist->rows-1, db);
		} while (c2_db_lineal_next (db));
	}
	
	gtk_clist_thaw (clist);
	index->mbox = mbox;
}

void
c2_index_sort (C2Index *index)
{
	gtk_clist_freeze (GTK_CLIST (index));
	sort (index);
	gtk_clist_thaw (GTK_CLIST (index));
}

void
c2_index_remove_mailbox (C2Index *index)
{
	GtkCList *clist = GTK_CLIST (GTK_WIDGET (index));

	gtk_clist_freeze (clist);
	gtk_clist_clear (clist);
	gtk_clist_thaw (clist);
}

static void
select_row (C2Index *index, gint row, gint column, GdkEvent *event)
{
	C2Db *node;
	
	/* Get the Db node */
	if ((node = C2_DB (gtk_clist_get_row_data (GTK_CLIST (GTK_WIDGET (index)), row))))
		gtk_signal_emit (GTK_OBJECT (index), signals[SELECT_MESSAGE], node);
}

static void
on_resize_column (C2Index *index, gint column, gint width)
{
	gchar *key = g_strdup_printf ("/Cronos II/Rc/clist[%d]", column);

	index->application->rc_clist[column] = width;

	gnome_config_set_int (key, width);
	g_free (key);
}

static gint
date_compare (GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2)
{
	return 0;
}

static gint
string_compare (GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2)
{
	return 0;
}

static void
sort (C2Index *index)
{
	GtkCList *clist = GTK_CLIST (index);

	gtk_clist_set_sort_type (clist, index->mbox->sort_type);

	switch (index->mbox->sort_by)
	{
		case C2_MAILBOX_SORT_STATUS:
#ifdef USE_DEBUG
			g_print ("Sorting by status has not been coded yet.\n");
#endif
			break;
		case C2_MAILBOX_SORT_DATE:
			gtk_clist_set_compare_func (clist, date_compare);
			break;
		default:
			gtk_clist_set_compare_func(clist, string_compare);
	}
	
	gtk_clist_sort (clist);
}

static void
on_index_click_column (GtkCList *clist, gint column, gpointer data)
{
	if (column == clist->sort_column)
	{
		clist->sort_type = (clist->sort_type == GTK_SORT_ASCENDING) ?
			GTK_SORT_DESCENDING : GTK_SORT_ASCENDING;
	} else
		gtk_clist_set_sort_column (clist, column);

	switch (column)
	{
/*		case 0:
			gtk_clist_set_compare_func(clist, state_compare);
			break;
		case 5:
			gtk_clist_set_compare_func(clist, date_compare);
			break;
*/		default:
			gtk_clist_set_compare_func(clist, NULL);
    }

	gtk_clist_sort (clist);
}
