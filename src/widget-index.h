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
#ifndef __CRONOSII_WIDGET_INDEX_H__
#define __CRONOSII_WIDGET_INDEX_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <gtk/gtkclist.h>

#if defined (HAVE_CONFIG_H) && defined (BUILDING_C2)
#	include <libcronosII/mailbox.h>
#	include <libcronosII/db.h>
#	include "widget-application.h"
#else
#	include <cronosII.h>
#endif

#define C2_INDEX(obj)						GTK_CHECK_CAST (obj, c2_index_get_type (), C2Index)
#define C2_INDEX_CLASS(klass)				GTK_CHECK_CLASS_CAST (klass, c2_index_get_type (), C2IndexClass)
#define C2_IS_INDEX(obj)				    GTK_CHECK_TYPE (obj, c2_index_get_type ())
#define C2_INDEX_CLASS_FW(obj)				C2_INDEX_CLASS (((GtkObject*)(obj))->klass)

typedef struct _C2Index C2Index;
typedef struct _C2IndexClass C2IndexClass;
typedef enum _C2IndexMode C2IndexMode;

enum _C2IndexMode
{
	C2_INDEX_READ_WRITE,
	C2_INDEX_READ_ONLY
};

struct _C2Index
{
	GtkCList clist;
	GtkWidget *clist_titles_label[C2_MAILBOX_SORT_LAST];
	GtkWidget *clist_titles_arrow[C2_MAILBOX_SORT_LAST];
	gint unreaded_messages;
	gint total_messages;

	C2Application *application;
	C2IndexMode mode;

	GtkWidget *menu;

	gint signal_id_for_marking_unread;

	C2Mailbox *mailbox;
};

struct _C2IndexClass
{
	GtkCListClass parent_class;

	void (*select_message) (C2Index *index, C2Db *node);
	void (*unselect_message) (C2Index *index, C2Db *node);
	gint (*open_message) (C2Index *index, C2Db *node);

	/* Not functions */
	void (*reload) (C2Index *index);
	void (*sort) (C2Index *index, C2MailboxSortBy sort_by, GtkSortType sort_type);
};

/* Widget crap */
GtkType
c2_index_get_type							(void);

GtkWidget *
c2_index_new								(C2Application *application, C2IndexMode mode);

void
c2_index_construct							(C2Index *index, C2Application *application, C2IndexMode mode);

/* Content handling */
void
c2_index_set_mailbox						(C2Index *index, C2Mailbox *mailbox);

void
c2_index_add_mail							(C2Index *index, C2Db *db_node);

void
c2_index_clear								(C2Index *index);

void
c2_index_select_message						(C2Index *index, C2Db *db);

void
c2_index_select_next_message				(C2Index *index);

void
c2_index_select_previous_message			(C2Index *index);

/* Content Querying */
C2Db *
c2_index_selection_main						(C2Index *index);

GList *
c2_index_selection							(C2Index *index);

/* Sorting */
void
c2_index_sort								(C2Index *index, C2MailboxSortBy sort_by, GtkSortType sort_type);

/* Misc */
void
c2_index_install_hints						(C2Index *index, GtkWidget *appbar, C2Mutex *lock);

#ifdef __cplusplus
}
#endif

#endif
