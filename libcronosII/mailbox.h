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
#ifndef __LIBC2_MAILBOX_H__
#define __LIBC2_MAILBOX_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <glib.h>
#include <gtk/gtk.h>

#define C2_TYPE_MAILBOX							(c2_mailbox_get_type ())
#define C2_MAILBOX(obj)							(GTK_CHECK_CAST (obj, C2_TYPE_MAILBOX, C2Mailbox))
#define C2_MAILBOX_CLASS(klass)					(GTK_CHECK_CLASS_CAST (klass, C2_TYPE_MAILBOX, C2MailboxClass))
#define C2_IS_MAILBOX(obj)						(GTK_CHECK_TYPE (obj, C2_TYPE_MAILBOX))
#define C2_IS_MAILBOX_CLASS(klass)				(GTK_CHECK_CLASS_TYPE (klass, C2_TYPE_MAILBOX))

typedef struct _C2Mailbox C2Mailbox;
typedef struct _C2MailboxClass C2MailboxClass;
typedef enum _C2MailboxSortBy C2MailboxSortBy;
typedef enum _C2MailboxType C2MailboxType;

#if defined (HAVE_CONFIG_H) && defined (BUILDING_C2)
#	include "db.h"
#else
#	include <cronosII.h>
#endif

#define C2_MAILBOX_IS_IN_TOPLEVEL(mbox)				(strlen (mbox) == 1)

enum _C2MailboxSortBy
{
	C2_MAILBOX_SORT_STATUS,
	C2_MAILBOX_SORT_UNUSED1,
	C2_MAILBOX_SORT_UNUSED2,
	C2_MAILBOX_SORT_SUBJECT,
	C2_MAILBOX_SORT_FROM,
	C2_MAILBOX_SORT_DATE,
	C2_MAILBOX_SORT_ACCOUNT,
	C2_MAILBOX_SORT_MID,
	C2_MAILBOX_SORT_LAST
};

enum _C2MailboxType
{
	C2_MAILBOX_CRONOSII,
	C2_MAILBOX_IMAP
};

struct _C2Mailbox
{
	GtkObject object;
	
	gchar *name;
	gchar *id;
	C2MailboxType type;
	
	C2MailboxSortBy sort_by;
	GtkSortType sort_type;

	gint selection;

	C2Db *db;
	gint last_mid;

	struct _C2Mailbox *next;
	struct _C2Mailbox *child;
};

struct _C2MailboxClass
{
	GtkObjectClass parent_class;

	void (*changed_mailboxes) (C2Mailbox *mailbox);
	void (*changed_mailbox) (C2Mailbox *mailbox, C2Db *db_node);
};

GtkType
c2_mailbox_get_type								(void);

C2Mailbox *
c2_mailbox_new									(const gchar *name, const gchar *id, C2MailboxType type,
												 C2MailboxSortBy sort_by, GtkSortType sort_type);

C2Mailbox *
c2_mailbox_append								(C2Mailbox *head, C2Mailbox *mailbox);

gchar *
c2_mailbox_next_id								(const C2Mailbox *head, const C2Mailbox *parent);

C2Mailbox *
c2_mailbox_search_id							(C2Mailbox *head, const gchar *id);

C2Mailbox *
c2_mailbox_search_name							(C2Mailbox *head, const gchar *name);

gint
c2_mailbox_length								(const C2Mailbox *mbox);

gint
c2_mailbox_next_mid								(C2Mailbox *mbox);

#ifdef __cplusplus
}
#endif

#endif
