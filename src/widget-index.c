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

#define SELECTED_MAIL	"mailbox::selected mail"

static void
class_init									(C2IndexClass *klass);

static void
init										(C2Index *index);

static void
add_message									(C2Application *application, GtkCList *clist,
											 C2Db *db, const gchar *date_fmt);

static void
reload										(C2Index *index);

static gint
get_selected_mail							(C2Index *index);

static void
set_selected_mail							(C2Index *index, gint i);

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
	LAST_SIGNAL
};

static gint signals[LAST_SIGNAL] = { 0 };

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
    signals[OPEN_MESSAGE] =
		gtk_signal_new("open_message",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET(C2IndexClass, open_message),
						gtk_marshal_NONE__POINTER, GTK_TYPE_NONE, 1,
						GTK_TYPE_POINTER);
	gtk_object_class_add_signals(object_class, signals, LAST_SIGNAL);

    klass->select_message = NULL;
    klass->open_message = NULL;
	klass->reload = reload;
}

static void
init (C2Index *index)
{
	index->unreaded_messages = 0;
	index->total_messages = 0;
	index->mailbox = NULL;
}

GtkWidget *
c2_index_new (C2Application *application)
{
    C2Index *index;
	
    index = gtk_type_new (c2_index_get_type());
	c2_index_construct (index, application);
	
    return GTK_WIDGET (index);
}

void
c2_index_construct (C2Index *index, C2Application *application)
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

	index->application = application;

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

	gtk_clist_set_column_width (clist, 0, atoi (DEFAULT_RC_CLIST_0));
	gtk_clist_set_column_width (clist, 1, atoi (DEFAULT_RC_CLIST_1));
	gtk_clist_set_column_width (clist, 2, atoi (DEFAULT_RC_CLIST_2));
	gtk_clist_set_column_width (clist, 3, atoi (DEFAULT_RC_CLIST_3));
	gtk_clist_set_column_width (clist, 4, atoi (DEFAULT_RC_CLIST_4));
	gtk_clist_set_column_width (clist, 5, atoi (DEFAULT_RC_CLIST_5));
	gtk_clist_set_column_width (clist, 6, atoi (DEFAULT_RC_CLIST_6));
	gtk_clist_set_column_width (clist, 7, atoi (DEFAULT_RC_CLIST_7));

/*    gtk_widget_show (GTK_WIDGET (clist));
    gtk_widget_ref (GTK_WIDGET (clist));*/
}

static void
add_message (C2Application *application, GtkCList *clist, C2Db *db, const gchar *date_fmt)
{
	struct tm *tm;
	GtkStyle *style;
	gchar *row[] = {
		NULL, NULL, NULL, db->subject, db->from, NULL, db->account, g_strdup_printf ("%d", db->position)
	};
	gchar *tmp = g_strdup_printf ("%d", db->mid);
	gchar *info[] = {
		NULL, NULL, NULL, db->subject, db->from, NULL, db->account, tmp
	};
	
	tm = localtime (&db->date);
	row[5] = g_new (gchar, 128);
	strftime (row[5], 128, date_fmt, tm);
	
	gtk_clist_append (clist, row);
	
	style = gtk_style_copy ((style = gtk_clist_get_row_style (clist, clist->rows-1)) ?
						style : gtk_widget_get_style (GTK_WIDGET (clist)));
	/* Set the state */
	switch (db->state)
	{
		case C2_MESSAGE_READED:
			gtk_clist_set_pixmap (clist, clist->rows-1, 0, application->pixmap_read, application->mask_read);
			style->font = application->fonts_gdk_readed_message;
			break;
		case C2_MESSAGE_UNREADED:
			gtk_clist_set_pixmap (clist, clist->rows-1, 0, application->pixmap_unread, application->mask_unread);
			style->font = application->fonts_gdk_unreaded_message;
			break;
		case C2_MESSAGE_FORWARDED:
			gtk_clist_set_pixmap (clist, clist->rows-1, 0, application->pixmap_forward, application->mask_forward);
			style->font = application->fonts_gdk_readed_message;
			break;
		case C2_MESSAGE_REPLIED:
			gtk_clist_set_pixmap (clist, clist->rows-1, 0, application->pixmap_reply, application->mask_reply);
			style->font = application->fonts_gdk_readed_message;
			break;
		default:
			gtk_clist_set_pixmap (clist, clist->rows-1, 0, application->pixmap_read, application->mask_read);
			style->font = application->fonts_gdk_readed_message;
	}

	/* [TODO] Get the BG color for this row */
	
	gtk_clist_set_row_style (clist, clist->rows-1, style);
	gtk_clist_set_row_data (clist, clist->rows-1, db);
}

static void
reload (C2Index *index)
{
	C2Db *db;
	GtkCList *clist = GTK_CLIST (GTK_WIDGET (index));
	C2Application *application = index->application;
	gchar *date_fmt;
	gint selected_mail;

	if (!index->mailbox)
	{
		gtk_clist_clear (clist);
		return;
	}
	
	date_fmt = gnome_config_get_string_with_default ("/"PACKAGE"/Interface-Misc/date_fmt="
									DEFAULT_INTERFACE_DATE_FMT, NULL);
	db = index->mailbox->db;

	gtk_clist_freeze (clist);
	if (db)
	{
		do
			add_message (index->application, clist, db, date_fmt);
		while (c2_db_lineal_next (db));
	}
	g_free (date_fmt);

	selected_mail = get_selected_mail (index);
	if (selected_mail < 0)
		selected_mail = clist->rows-1;

	gtk_clist_thaw (clist);
	gtk_clist_select_row (clist, selected_mail, 5);
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

	gtk_object_set_data (GTK_OBJECT (mailbox), SELECTED_MAIL, (gpointer) i+1);
}

void
c2_index_sort (C2Index *index)
{
	gtk_clist_freeze (GTK_CLIST (index));
	sort (index);
	gtk_clist_thaw (GTK_CLIST (index));
}

static void
select_row (C2Index *index, gint row, gint column, GdkEvent *event)
{
	GtkVisibility visibility;
	GtkCList *clist = GTK_CLIST (index);
	C2Db *node;
	gint top;
	
	/* Get the Db node */
	if ((node = C2_DB (gtk_clist_get_row_data (GTK_CLIST (GTK_WIDGET (index)), row))))
		gtk_signal_emit (GTK_OBJECT (index), signals[SELECT_MESSAGE], node);
	set_selected_mail (index, row);

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
}

static void
on_resize_column (C2Index *index, gint column, gint width)
{
	gchar *key = g_strdup_printf ("/"PACKAGE"/Rc/clist[%d]", column);

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

	gtk_clist_set_sort_type (clist, index->mailbox->sort_type);

	switch (index->mailbox->sort_by)
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

void
c2_index_add_mailbox (C2Index *index, C2Mailbox *mailbox)
{
	c2_return_if_fail_obj (mailbox, C2EDATA, GTK_OBJECT (index));

	index->mailbox = mailbox;
	C2_INDEX_CLASS_FW (index)->reload (index);
}

void
c2_index_remove_mailbox (C2Index *index)
{
	index->mailbox = NULL;
	
	C2_INDEX_CLASS_FW (index)->reload (index);
}

void
c2_index_add_message (C2Index *index, C2Db *db)
{
	gchar *date_fmt;
	
	c2_return_if_fail (db, C2EDATA);

	gnome_config_get_string_with_default ("/"PACKAGE"/Interface-Misc/date_fmt="
									DEFAULT_INTERFACE_DATE_FMT, NULL);
	add_message (index->application, GTK_CLIST (index), db, date_fmt);
	g_free (date_fmt);
}

void
c2_index_remove_message (C2Index *index, C2Db *db)
{
}
