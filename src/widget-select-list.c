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

#include <libcronosII/error.h>
#include <libcronosII/utils.h>

#include "widget-select-list.h"

#define PIXMAP_ON	PKGDATADIR "/pixmaps/check-on.png"
#define PIXMAP_OFF	PKGDATADIR "/pixmaps/check-off.png"

static void
class_init									(C2SelectListClass *klass);

static void
init										(C2SelectList *sl);

static void
on_select_row								(GtkCList *clist, gint row, gint column);

static gboolean
row_is_activated							(C2SelectList *sl, gint row);

enum
{
	SELECT_ITEM,
	UNSELECT_ITEM,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static GtkCListClass *parent_class = NULL;

GtkType
c2_select_list_get_type (void)
{
	static GtkType type = 0;

	if (!type)
	{
		static GtkTypeInfo info =
		{
			"C2SelectList",
			sizeof (C2SelectList),
			sizeof (C2SelectListClass),
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
class_init (C2SelectListClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *) klass;
	
	parent_class = gtk_type_class (gtk_clist_get_type ());

	signals[SELECT_ITEM] =
		gtk_signal_new ("select_item",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2SelectListClass, select_item),
						gtk_marshal_NONE__INT_POINTER, GTK_TYPE_NONE, 2,
						GTK_TYPE_INT, GTK_TYPE_POINTER);
	signals[UNSELECT_ITEM] =
		gtk_signal_new ("unselect_item",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2SelectListClass, unselect_item),
						gtk_marshal_NONE__INT_POINTER, GTK_TYPE_NONE, 2,
						GTK_TYPE_INT, GTK_TYPE_POINTER);
	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);

	klass->select_item = NULL;
	klass->unselect_item = NULL;
}

static void
init (C2SelectList *sl)
{
	sl->pixmap_on = NULL;
	sl->pixmap_off = NULL;
}

GtkWidget *
c2_select_list_new_from_glade (const gchar *str1, const gchar *str2, gint n1, gint n2)
{
	return c2_select_list_new (n2);
}

GtkWidget *
c2_select_list_new (gint columns)
{
	C2SelectList *sl;
	gint c;

	sl = gtk_type_new (c2_select_list_get_type ());
	gtk_clist_construct (GTK_CLIST (sl), columns+1, NULL);
	gtk_clist_column_titles_hide (GTK_CLIST (sl));

	sl->pixmap_on = gnome_pixmap_new_from_file (PIXMAP_ON);
	sl->pixmap_off = gnome_pixmap_new_from_file (PIXMAP_OFF);

	for (c = 0; c < GTK_CLIST (sl)->columns; c++)
		gtk_clist_set_column_auto_resize (GTK_CLIST (sl), c, TRUE);

	gtk_signal_connect (GTK_OBJECT (sl), "select-row",
						GTK_SIGNAL_FUNC (on_select_row), NULL);

	return GTK_WIDGET (sl);
}

void
c2_select_list_insert_item (C2SelectList *sl, gint row, gchar **cont, gpointer data)
{
	GtkCList *clist = GTK_CLIST (sl);
	gint c;
	gchar *tcont[] = { NULL };

	c2_return_if_fail (C2_IS_SELECT_LIST (sl), C2EDATA);
	
	gtk_clist_freeze (clist);
	gtk_clist_insert (clist, row, tcont);
	
	for (c = 0; c < clist->columns; c++)
	{
		switch (c)
		{
			case 0:
				gtk_clist_set_pixmap (clist, row, c, GNOME_PIXMAP (sl->pixmap_off)->pixmap,
										GNOME_PIXMAP (sl->pixmap_off)->mask);
				break;
			default:
				gtk_clist_set_text (clist, row, c, cont[c-1]);
		}
	}

	if (data)
		gtk_clist_set_row_data (clist, row, data);
	
	gtk_clist_set_selectable (clist, row, FALSE);
	gtk_clist_thaw (clist);
}

static void
on_select_row (GtkCList *clist, gint row, gint column)
{
	gboolean active = row_is_activated (C2_SELECT_LIST (clist), row);

	if (active)
		gtk_clist_set_pixmap (clist, row, 0,
								GNOME_PIXMAP (C2_SELECT_LIST (clist)->pixmap_off)->pixmap,
								GNOME_PIXMAP (C2_SELECT_LIST (clist)->pixmap_off)->mask);
	else
		gtk_clist_set_pixmap (clist, row, 0,
								GNOME_PIXMAP (C2_SELECT_LIST (clist)->pixmap_on)->pixmap,
								GNOME_PIXMAP (C2_SELECT_LIST (clist)->pixmap_on)->mask);
}

static gboolean
row_is_activated (C2SelectList *sl, gint row)
{
	GdkPixmap *pixmap;
	GdkBitmap *mask;
	gboolean active;

	gtk_clist_get_pixmap (GTK_CLIST (sl), row, 0, &pixmap, &mask);

	if (pixmap == GNOME_PIXMAP (sl->pixmap_on)->pixmap)
		active = TRUE;
	else if (pixmap == GNOME_PIXMAP (sl->pixmap_off)->pixmap)
		active = FALSE;
	else
	{
		g_warning ("Error in C2SelectList, the clicked row is not ON or OFF.\n");
		return FALSE;
	}

	return active;
}

void
c2_select_list_set_active (C2SelectList *sl, gint row, gboolean active)
{
	gboolean ma, ia;

	ma = active ? TRUE : FALSE;
	ia = row_is_activated (sl, row) ? TRUE : FALSE;
	
	if (ma != ia)
		on_select_row (GTK_CLIST (sl), row, 0);
}

GSList *
c2_select_list_get_active_items (C2SelectList *sl)
{
	GSList *head = NULL;
	gint i;
	GtkCList *clist;

	clist = GTK_CLIST (sl);
	for (i = 0; i < clist->rows; i++)
		if (row_is_activated (sl, i))
			head = g_slist_append (head, (gpointer) i);

	return head;
}

GSList *
c2_select_list_get_active_items_data (C2SelectList *sl)
{
	GSList *head = NULL;
	gint i;
	GtkCList *clist;

	clist = GTK_CLIST (sl);
	for (i = 0; i < clist->rows; i++)
		if (row_is_activated (sl, i))
			head = g_slist_append (head,
							gtk_clist_get_row_data (clist, i));

	return head;
}
