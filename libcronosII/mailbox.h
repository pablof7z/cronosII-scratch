/*  Cronos II - The GNOME mail client
 *  Copyright (C) 2000-2001 Pablo Fernández
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
 * 		* Pablo Fernández
 * Code of this file by:
 * 		* Pablo Fernández
 */
#ifndef __LIBCRONOSII_MAILBOX_H__
#define __LIBCRONOSII_MAILBOX_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef BUILDING_C2
#	include <config.h>
#else
#	include <cronosII.h>
#endif

#include <glib.h>
#include <gtk/gtk.h>
#include <string.h>
#include <pthread.h>
	
#define C2_TYPE_MAILBOX						(c2_mailbox_get_type ())
#ifdef USE_DEBUG
#	define C2_MAILBOX(obj)					(GTK_CHECK_CAST (obj, C2_TYPE_MAILBOX, C2Mailbox))
#	define C2_MAILBOX_CLASS(klass)			(GTK_CHECK_CLASS_CAST (klass, C2_TYPE_MAILBOX, C2MailboxClass))
#else
#	define C2_MAILBOX(obj)					((C2Mailbox*)obj)
#	define C2_MAILBOX_CLASS(klass)			((C2MailboxClass)klass)
#endif

#define C2_IS_MAILBOX(obj)					(GTK_CHECK_TYPE (obj, C2_TYPE_MAILBOX))
#define C2_IS_MAILBOX_CLASS(klass)			(GTK_CHECK_CLASS_TYPE (klass, C2_TYPE_MAILBOX))

typedef struct _C2Mailbox C2Mailbox;
typedef struct _C2MailboxClass C2MailboxClass;
typedef enum _C2MailboxSortBy C2MailboxSortBy;
typedef enum _C2MailboxType C2MailboxType;
typedef enum _C2MailboxIMAPEvent C2MailboxIMAPEvent;
typedef enum _C2MailboxChangeType C2MailboxChangeType;
typedef enum _C2MailboxUseAs C2MailboxUseAs;

#if defined (HAVE_CONFIG_H) && defined (BUILDING_C2)
#	include "db.h"
#	include "imap.h"
#	include "utils-mutex.h"
#else
#	include <cronosII.h>
#endif

#define C2_MAILBOX_IS_TOPLEVEL(mbox)		(strstr (mbox->id, "-") ? 0 : 1)

enum _C2MailboxSortBy
{
	C2_MAILBOX_SORT_STATUS,
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
	C2_MAILBOX_IMAP,
	C2_MAILBOX_SPOOL
};

enum _C2MailboxChangeType
{
	C2_MAILBOX_CHANGE_ADD,
	C2_MAILBOX_CHANGE_REMOVE,
	C2_MAILBOX_CHANGE_STATE,

	C2_MAILBOX_CHANGE_ANY
};

enum _C2MailboxUseAs
{
	C2_MAILBOX_USE_AS_INBOX			= 1 << 0,
	C2_MAILBOX_USE_AS_OUTBOX		= 1 << 1,
	C2_MAILBOX_USE_AS_SENT_ITEMS	= 1 << 2,
	C2_MAILBOX_USE_AS_TRASH			= 1 << 3,
	C2_MAILBOX_USE_AS_DRAFTS		= 1 << 4
};

struct _C2Mailbox
{
	GtkObject object;
	
	gchar *name;
	gchar *id;
	C2MailboxType type;

	/* Here we will store information about the
	 * mailbox, specific information according
	 * to the type of mailbox.
	 */

	union
	{
		struct
		{
			FILE *fd;
			gint mid; /* Mid in line where we are */
			C2Mutex lock;
		} cronosII;
		struct
		{
			C2IMAP *imap;
			gboolean marked;
			gboolean noinferiors;
			gboolean noselect;
		} IMAP;
		struct
		{
			gchar *path;
		} spool;
	} protocol;
	
	gint freezed : 1;
	gint signals_queued : 1;

	C2Mutex lock;
	
	C2MailboxUseAs use_as;
	C2MailboxSortBy sort_by;
	GtkSortType sort_type;

	gint selection;

	gint db_is_loaded : 1;
	C2Db *db;
	gint last_mid;

	struct _C2Mailbox *next;
	struct _C2Mailbox *child;
};

struct _C2MailboxClass
{
	GtkObjectClass parent_class;

	void (*compacted) (C2Mailbox *mailbox, size_t cbytes, size_t tbytes);

	/* This signal represents a new mailbox */
	void (*changed_mailboxes) (C2Mailbox *mailbox);

	/* This signal will be emitted when the mailbox has changed the number
	 * of mails,  db_node is the node where changes start,
	 * from that node ahead there had been changes.
	 */
	void (*changed_mailbox) (C2Mailbox *mailbox, C2MailboxChangeType type, C2Db *db_node);
	void (*db_loaded) (C2Mailbox *mailbox, gboolean success);
};

GtkType
c2_mailbox_get_type								(void);

/* Tree handling */
#define c2_mailbox_new(head, a, b, c, d, e, args...)	_c2_mailbox_new (head, a, b, TRUE, c, d, e, ##args)

C2Mailbox *
_c2_mailbox_new									(C2Mailbox **head, const gchar *name, const gchar *id,
												 gboolean independent, C2MailboxType type,
												 C2MailboxSortBy sort_by, GtkSortType sort_type, ...);

C2Mailbox *
c2_mailbox_new_with_parent						(C2Mailbox **head, const gchar *name, const gchar *parent_id,
												 C2MailboxType type, C2MailboxSortBy sort_by,
												 GtkSortType sort_type, ...);

void
c2_mailbox_destroy_tree							(C2Mailbox *head);

void
c2_mailbox_set_use_as							(C2Mailbox *head, C2Mailbox *mailbox, C2MailboxUseAs use_as);

C2MailboxUseAs
c2_mailbox_get_use_as							(C2Mailbox *mailbox);

void
c2_mailbox_update								(C2Mailbox *mailbox, const gchar *name, const gchar *id,
												 C2MailboxType type, ...);

gboolean
c2_mailbox_remove								(C2Mailbox **head, C2Mailbox *mailbox);

#define c2_mailbox_get_parent_id(x)				c2_mailbox_get_complete_id (x, c2_mailbox_get_level (x)-1)

gchar *
c2_mailbox_create_id_from_parent				(C2Mailbox *head, C2Mailbox *parent);

gint
c2_mailbox_get_level							(const gchar *id);

gchar *
c2_mailbox_get_complete_id						(const gchar *id, guint number);

gint
c2_mailbox_get_id								(const gchar *id, gint number);

C2Mailbox *
c2_mailbox_get_by_name							(C2Mailbox *head, const gchar *name);

C2Mailbox *
c2_mailbox_get_by_usage							(C2Mailbox *head, C2MailboxUseAs use_as);
	
C2Mailbox *
c2_mailbox_get_by_id (C2Mailbox *head, const gchar *id);

gboolean
c2_mailbox_load_db								(C2Mailbox *mailbox);

#ifdef __cplusplus
}
#endif

#endif
