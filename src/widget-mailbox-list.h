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
#ifndef __WIDGET_MAILBOX_LIST_H__
#define __WIDGET_MAILBOX_LIST_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <gtk/gtk.h>

#ifdef BUILDING_C2
#	include <libcronosII/mailbox.h>

#	include "widget-application.h"
#else
#	include <cronosII.h>
#endif

#define C2_MAILBOX_LIST(obj)				(GTK_CHECK_CAST (obj, c2_mailbox_list_get_type (), C2MailboxList))
#define C2_MAILBOX_LIST_CLASS(klass)		(GTK_CHECK_CLASS_CAST (klass, c2_mailbox_list_get_type (), C2MailboxListClass))

typedef struct _C2MailboxList C2MailboxList;
typedef struct _C2MailboxListClass C2MailboxListClass;

struct _C2MailboxList
{
	GtkCTree ctree;

	GtkCTreeNode *selected_node;

	GList *mailbox_list;
	GList *data_list;
};

struct _C2MailboxListClass
{
	GtkCTreeClass parent_class;

	void (*object_selected) (C2MailboxList *mlist, GtkObject *object);
	void (*object_unselected) (C2MailboxList *mlist);
};

GtkType
c2_mailbox_list_get_type				(void);

GtkWidget *
c2_mailbox_list_new						(C2Application *application);

C2Mailbox *
c2_mailbox_list_get_selected_mailbox	(C2MailboxList *mlist);

GtkObject *
c2_mailbox_list_get_selected_object		(C2MailboxList *mlist);

void
c2_mailbox_list_set_selected_object		(C2MailboxList *mlist, GtkObject *object);

#ifdef __cplusplus
}
#endif

#endif
