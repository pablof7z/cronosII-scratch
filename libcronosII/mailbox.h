/*  Cronos II Mail Client
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
#ifndef __LIBCRONOSII_MAILBOX_H__
#define __LIBCRONOSII_MAILBOX_H__

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
#	include <config.h>
#	include "db.h"
#else
#	include <cronosII.h>
#endif

#define C2_MAILBOX_IS_TOPLEVEL(mbox)				(strstr (mbox->id, "-") ? 0 : 1)

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

	/* Here we will store information about the
	 * mailbox, specific information according
	 * to the type of mailbox.
	 */
	union
	{
		struct {
		} cronosII;
		struct {
			gchar *host;
			gint port;
			gchar *user;
			gchar *pass;
			gchar *path;
		} imap;
	} protocol;
	
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
												 C2MailboxSortBy sort_by, GtkSortType sort_type, ...);

C2Mailbox *
c2_mailbox_new_with_parent						(const gchar *name, const gchar *parent_id, C2MailboxType type,
												 C2MailboxSortBy sort_by, GtkSortType sort_type, ...);

void
c2_mailbox_destroy_tree							(void);

void
c2_mailbox_update								(C2Mailbox *mailbox, const gchar *name, const gchar *id,
												 C2MailboxType type, ...);

void
c2_mailbox_remove								(C2Mailbox *mailbox);

#define c2_mailbox_get_parent_id(x)				c2_mailbox_get_complete_id (x, c2_mailbox_get_level (x)-1)

gint
c2_mailbox_get_level							(const gchar *id);

gchar *
c2_mailbox_get_complete_id						(const gchar *id, guint number);

gint
c2_mailbox_get_id								(const gchar *id, gint number);

C2Mailbox *
c2_mailbox_get_head								(void);

C2Mailbox *
c2_mailbox_get_by_name							(C2Mailbox *head, const gchar *name);

#ifdef __cplusplus
}
#endif

#endif
