#include <gnome.h>
#include <time.h>

#include <libmodules/error.h>

#include "c2-app.h"
#include "widget-index.h"
#include "main-window.h"

static void
c2_index_class_init									(C2IndexClass * klass);

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

static gint c2_index_signals[LAST_SIGNAL] = {
	0
};

static GtkCListClass *parent_class = NULL;

void
c2_index_add_mailbox (C2Index *index, C2Mailbox *mbox)
{
	C2Mailbox *tmp;
	C2DB *db;
	C2DBNode *node;
	GList *l;
	GtkStyle *style = NULL, *style2;
	
	c2_return_if_fail (index, C2EDATA);
	c2_return_if_fail (mbox, C2EDATA);
	
	if (GTK_CLIST (index)->selection)
	{
		/* Get the mailbox that was loaded */
		tmp = WMain.selected_mbox;
		if (tmp)
		{
			if (GTK_CLIST (index)->selection)
				tmp->last_row = GPOINTER_TO_INT (GTK_CLIST (index)->selection->data);
		}
	}

	gtk_clist_freeze (GTK_CLIST (index));
	for (l = mbox->db->head; l != NULL; l = g_list_next (l))
	{
		C2DBNode *node = l->data;
		struct tm *tm;
		gchar *tmp = g_strdup_printf ("%d", node->mid);
		gchar *info[] = {
			NULL, NULL, NULL, node->headers[0], node->headers[1], NULL,
			node->headers[2], tmp
		};
		tm = localtime (&node->date);
		info[5] = g_new0 (gchar, 128);
		strftime (info[5], 128, DATE_FORMAT, tm);
		gtk_clist_append (GTK_CLIST (index), info);
		if (!style)
			style = gtk_widget_get_style (GTK_WIDGET (index));
		style2 = gtk_style_copy (style);
		if (node->status == C2_DB_NODE_UNREADED)
		{
			style2->font = c2_app.gdk_font_unread;
			mbox->new_messages++;
		} else
			style2->font = c2_app.gdk_font_read;
		gtk_clist_set_row_style (GTK_CLIST (index), GTK_CLIST (index)->rows-1, style2);
	}
	gtk_clist_thaw (GTK_CLIST (index));
}

void
c2_index_remove_mailbox (C2Index *index)
{
	c2_return_if_fail (C2_IS_INDEX (index), C2EDATA);

	gtk_clist_freeze (GTK_CLIST (index));
	gtk_clist_clear (GTK_CLIST (index));
	gtk_clist_thaw (GTK_CLIST (index));
}

static void
select_message (GtkWidget *widget, gint row, gint column, GdkEventButton *event, gpointer data)
{
}

static void
open_message (GtkWidget *widget, gint row, gint column, GdkEventButton *event, gpointer data)
{
}

static void
delete_message (GtkWidget *widget, gint row, gint column, GdkEventButton *event, gpointer data)
{
}

static void
expunge_message (GtkWidget *widget, gint row, gint column, GdkEventButton *event, gpointer data)
{
}

static void
move_message (GtkWidget *widget, gint row, gint column, GdkEventButton *event, gpointer data)
{
}

static void
copy_message (GtkWidget *widget, gint row, gint column, GdkEventButton *event, gpointer data)
{
}

static void
reply_message (GtkWidget *widget, gint row, gint column, GdkEventButton *event, gpointer data)
{
}

static void
reply_all_message (GtkWidget *widget, gint row, gint column, GdkEventButton *event, gpointer data)
{
}

static void
forward_message (GtkWidget *widget, gint row, gint column, GdkEventButton *event, gpointer data)
{
}

static void
print_message (GtkWidget *widget, gint row, gint column, GdkEventButton *event, gpointer data)
{
}

static void
save_message (GtkWidget *widget, gint row, gint column, GdkEventButton *event, gpointer data)
{
}

static void
add_contact (GtkWidget *widget, gint row, gint column, GdkEventButton *event, gpointer data)
{
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

static void
c2_index_init (C2Index *index)
{
	GtkWidget *pixmap;
	GtkCList *clist;
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

	titles[3] = _("Subject");
	titles[4] = _("From");
	titles[5] = _("Date");
	titles[6] = _("Account");

    /* create the clist */
    gtk_clist_construct (GTK_CLIST (index), 8, titles);
    clist = GTK_CLIST(index);

    gtk_signal_connect (GTK_OBJECT (index), "click_column",
		       GTK_SIGNAL_FUNC(on_index_click_column), NULL);

	pixmap = gtk_pixmap_new (c2_app.pixmap_unread, c2_app.mask_unread);
	gtk_clist_set_column_widget (clist, 0, pixmap);
	
    gtk_clist_set_column_width (clist, 0, c2_app.wm_clist[0]);
	gtk_clist_set_column_width (clist, 1, c2_app.wm_clist[1]);
	gtk_clist_set_column_width (clist, 2, c2_app.wm_clist[2]);
	gtk_clist_set_column_width (clist, 3, c2_app.wm_clist[3]);
	gtk_clist_set_column_width (clist, 4, c2_app.wm_clist[4]);
	gtk_clist_set_column_width (clist, 5, c2_app.wm_clist[5]);
	gtk_clist_set_column_width (clist, 6, c2_app.wm_clist[6]);
	gtk_clist_set_column_width (clist, 7, c2_app.wm_clist[7]);
	gtk_clist_set_column_visibility (clist, 1, FALSE);
	gtk_clist_set_column_visibility (clist, 2, FALSE);
	gtk_clist_set_column_visibility (clist, 7, FALSE);
	gtk_clist_column_titles_show (clist);
    gtk_clist_set_row_height (clist, 16);
	gtk_clist_set_selection_mode (clist, GTK_SELECTION_EXTENDED);

    /* Set default sorting behaviour */
/*    gtk_clist_set_sort_column(clist, 5);
    gtk_clist_set_compare_func(clist, date_compare);
    gtk_clist_set_sort_type(clist, GTK_SORT_DESCENDING);

    gtk_signal_connect(GTK_OBJECT(clist),
		       "select-row",
		       (GtkSignalFunc) select_message, (gpointer) bindex);

    gtk_signal_connect(GTK_OBJECT(clist),
		       "unselect-row",
		       (GtkSignalFunc) unselect_message,
		       (gpointer) bindex);

    gtk_signal_connect(GTK_OBJECT(clist),
		       "button_press_event",
		       (GtkSignalFunc) button_event_press_cb,
		       (gpointer) bindex);

    gtk_signal_connect(GTK_OBJECT(clist),
		       "button_release_event",
		       (GtkSignalFunc) button_event_release_cb,
		       (gpointer) bindex);
*/
    /* We want to catch column resize attempts to store the new value */
/*    gtk_signal_connect(GTK_OBJECT(clist),
		       "resize_column",
		       GTK_SIGNAL_FUNC(resize_column_event_cb), NULL);
*/
    gtk_widget_show (GTK_WIDGET (clist));
    gtk_widget_ref (GTK_WIDGET (clist));
}

guint
c2_index_get_type (void)
{
    static guint c2_index_type = 0;
	
	if (!c2_index_type) {
		GtkTypeInfo c2_index_info = {
			"C2Index",
			sizeof (C2Index),
			sizeof (C2IndexClass),
			(GtkClassInitFunc) c2_index_class_init,
			(GtkObjectInitFunc) c2_index_init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL
		};
		
		c2_index_type = gtk_type_unique (gtk_clist_get_type (), &c2_index_info);
	}

    return c2_index_type;
}

static void
c2_index_class_init (C2IndexClass *klass)
{
    GtkObjectClass *object_class;
    GtkWidgetClass *widget_class;
    GtkContainerClass *container_class;

    object_class = (GtkObjectClass *) klass;
    widget_class = (GtkWidgetClass *) klass;
    container_class = (GtkContainerClass *) klass;

    parent_class = gtk_type_class(gtk_widget_get_type());

    c2_index_signals[SELECT_MESSAGE] =
	gtk_signal_new("select_message",
		       GTK_RUN_FIRST,
		       object_class->type,
		       GTK_SIGNAL_OFFSET(C2IndexClass, select_message),
		       gtk_marshal_NONE__POINTER_POINTER,
		       GTK_TYPE_NONE, 2, GTK_TYPE_ENUM,
		       GTK_TYPE_POINTER);
    c2_index_signals[OPEN_MESSAGE] =
	gtk_signal_new("open_message",
		       GTK_RUN_FIRST,
		       object_class->type,
		       GTK_SIGNAL_OFFSET(C2IndexClass,
					 open_message),
		       gtk_marshal_NONE__POINTER_POINTER, GTK_TYPE_NONE, 2,
		       GTK_TYPE_ENUM, GTK_TYPE_POINTER);
	c2_index_signals[DELETE_MESSAGE] =
	gtk_signal_new("delete_message",
		       GTK_RUN_FIRST,
		       object_class->type,
		       GTK_SIGNAL_OFFSET(C2IndexClass,
					 delete_message),
		       gtk_marshal_NONE__POINTER_POINTER, GTK_TYPE_NONE, 2,
		       GTK_TYPE_ENUM, GTK_TYPE_POINTER);
	c2_index_signals[EXPUNGE_MESSAGE] =
	gtk_signal_new("expunge_message",
		       GTK_RUN_FIRST,
		       object_class->type,
		       GTK_SIGNAL_OFFSET(C2IndexClass,
					 expunge_message),
		       gtk_marshal_NONE__POINTER_POINTER, GTK_TYPE_NONE, 2,
		       GTK_TYPE_ENUM, GTK_TYPE_POINTER);
	c2_index_signals[MOVE_MESSAGE] =
	gtk_signal_new("move_message",
		       GTK_RUN_FIRST,
		       object_class->type,
		       GTK_SIGNAL_OFFSET(C2IndexClass,
					 move_message),
		       gtk_marshal_NONE__POINTER_POINTER, GTK_TYPE_NONE, 2,
		       GTK_TYPE_ENUM, GTK_TYPE_POINTER);
	c2_index_signals[COPY_MESSAGE] =
	gtk_signal_new("copy_message",
		       GTK_RUN_FIRST,
		       object_class->type,
		       GTK_SIGNAL_OFFSET(C2IndexClass,
					 copy_message),
		       gtk_marshal_NONE__POINTER_POINTER, GTK_TYPE_NONE, 2,
		       GTK_TYPE_ENUM, GTK_TYPE_POINTER);
	c2_index_signals[REPLY_MESSAGE] =
	gtk_signal_new("reply_message",
		       GTK_RUN_FIRST,
		       object_class->type,
		       GTK_SIGNAL_OFFSET(C2IndexClass,
					 reply_message),
		       gtk_marshal_NONE__POINTER_POINTER, GTK_TYPE_NONE, 2,
		       GTK_TYPE_ENUM, GTK_TYPE_POINTER);
	c2_index_signals[REPLY_ALL_MESSAGE] =
	gtk_signal_new("reply_all_message",
		       GTK_RUN_FIRST,
		       object_class->type,
		       GTK_SIGNAL_OFFSET(C2IndexClass,
					 reply_all_message),
		       gtk_marshal_NONE__POINTER_POINTER, GTK_TYPE_NONE, 2,
		       GTK_TYPE_ENUM, GTK_TYPE_POINTER);
	c2_index_signals[FORWARD_MESSAGE] =
	gtk_signal_new("forward_message",
		       GTK_RUN_FIRST,
		       object_class->type,
		       GTK_SIGNAL_OFFSET(C2IndexClass,
					 forward_message),
		       gtk_marshal_NONE__POINTER_POINTER, GTK_TYPE_NONE, 2,
		       GTK_TYPE_ENUM, GTK_TYPE_POINTER);
	c2_index_signals[PRINT_MESSAGE] =
	gtk_signal_new("print_message",
		       GTK_RUN_FIRST,
		       object_class->type,
		       GTK_SIGNAL_OFFSET(C2IndexClass,
					 print_message),
		       gtk_marshal_NONE__POINTER_POINTER, GTK_TYPE_NONE, 2,
		       GTK_TYPE_ENUM, GTK_TYPE_POINTER);
	c2_index_signals[SAVE_MESSAGE] =
	gtk_signal_new("save_message",
		       GTK_RUN_FIRST,
		       object_class->type,
		       GTK_SIGNAL_OFFSET(C2IndexClass,
					 save_message),
		       gtk_marshal_NONE__POINTER_POINTER, GTK_TYPE_NONE, 2,
		       GTK_TYPE_ENUM, GTK_TYPE_POINTER);
	c2_index_signals[ADD_CONTACT] =
	gtk_signal_new("add_contact",
		       GTK_RUN_FIRST,
		       object_class->type,
		       GTK_SIGNAL_OFFSET(C2IndexClass,
					 add_contact),
		       gtk_marshal_NONE__POINTER_POINTER, GTK_TYPE_NONE, 2,
		       GTK_TYPE_ENUM, GTK_TYPE_POINTER);
    gtk_object_class_add_signals(object_class, c2_index_signals,
				 LAST_SIGNAL);

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

GtkWidget *
c2_index_new(void)
{
    C2Index *index;
    index = gtk_type_new (c2_index_get_type());
    return GTK_WIDGET (index);
}
