/*  Cronos II
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
#ifndef __WIDGET_INDEX_H__
#define __WIDGET_INDEX_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <gtk/gtkclist.h>

#ifdef HAVE_CONFIG_H
#	include <libmodules/mailbox.h>
#	include <libmodules/db.h>
#else
#	include <cronosII.h>
#endif

#define C2_INDEX(obj)						GTK_CHECK_CAST (obj, c2_index_get_type (), C2Index)
#define C2_INDEX_CLASS(klass)				GTK_CHECK_CLASS_CAST (klass, c2_index_get_type (), C2IndexClass)
#define C2_IS_INDEX(obj)				    GTK_CHECK_TYPE (obj, c2_index_get_type ())

typedef struct
{
	GtkCList clist;

	C2Mailbox *mbox;
} C2Index;

typedef struct
{
	GtkCListClass parent_class;

	void (*select_message) (C2Index *index, C2Method access_method, C2Message *message);
	void (*open_message) (C2Index *index, C2Method access_method, C2Message *message);

	void (*delete_message) (C2Index *index, C2Method access_method, C2Message *message);
	void (*expunge_message) (C2Index *index, C2Method access_method, C2Message *message);
	void (*move_message) (C2Index *index, C2Method access_method, C2Message *message);
	void (*copy_message) (C2Index *index, C2Method access_method, C2Message *message);

	void (*reply_message) (C2Index *index, C2Method access_method, C2Message *message);
	void (*reply_all_message) (C2Index *index, C2Method access_method, C2Message *message);
	void (*forward_message) (C2Index *index, C2Method access_method, C2Message *message);

	void (*print_message) (C2Index *index, C2Method access_method, C2Message *message);
	void (*save_message) (C2Index *index, C2Method access_method, C2Message *message);

	void (*add_contact) (C2Index *index, C2Method access_method, C2Message *message);
} C2IndexClass;

guint
c2_index_get_type									(void);

GtkWidget *
c2_index_new										(void);

void
c2_index_add_mailbox								(C2Index *index, C2Mailbox *mbox);

void
c2_index_remove_mailbox								(C2Index *index);

void
c2_index_add_message								(C2Index *index, const C2Message *message);

void
c2_index_remove_message								(C2Index *index, const C2Message *message);

void
c2_index_select_previous_message					(C2Index *index);

void
c2_index_select_next_message						(C2Index *index);

gboolean
c2_index_exists_previous_message					(C2Index *index);

gboolean
c2_index_exists_next_message						(C2Index *index);

#ifdef __cplusplus
}
#endif

#endif
