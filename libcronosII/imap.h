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
 * 		* Bosko Blagojevic
 * Code of this file by:
 * 		* Bosko Blagojevic
 * 		* Pablo Fernández
 */
#ifndef __LIBCRONOSII_IMAP_H__
#define __LIBCRONOSII_IMAP_H__

/* This module is in charge of Bosko.
 * Let's go Bosko! We can do it!
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <glib.h>
#include <pthread.h>

#define C2_TYPE_IMAP						(c2_imap_get_type ())
#define C2_IMAP(obj)						(GTK_CHECK_CAST (obj, C2_TYPE_IMAP, C2IMAP))
#define C2_IMAP_CLASS(klass)				(GTK_CHECK_CLASS_CAST (klass, C2_TYPE_IMAP, C2IMAPClass))
#define C2_IS_IMAP(obj)						(GTK_CHECK_TYPE (obj, C2_TYPE_IMAP))
#define C2_IS_IMAP_CLASS(klass)				(GTK_CHECK_CLASS_TYPE (klass, C2_TYPE_IMAP))
#define C2_IMAP_CLASS_FW(obj)				(C2_IMAP_CLASS (((GtkObject*) (obj))->klass))

typedef struct _C2IMAP C2IMAP;
typedef struct _C2IMAPServerReply C2IMAPServerReply;
typedef struct _C2IMAPClass C2IMAPClass;
typedef struct _C2IMAPFolder C2IMAPFolder;
typedef enum _C2IMAPAuthenticationType C2IMAPAuthenticationType;
typedef unsigned int tag_t;
typedef struct _C2IMAPPending C2IMAPPending;
typedef unsigned int C2IMAPState;
	
#ifdef BUILDING_C2
#	include "net-object.h"
#	include "mailbox.h"
#	include "message.h"
#	include "utils-mutex.h"
#	include "account.h"
#	include "error.h"
#else
#	include <cronosII.h>
#endif

enum _C2IMAPAuthenticationType
{
	C2_IMAP_AUTHENTICATION_PLAINTEXT
};
	
enum C2IMAPState
{
	C2IMAPDisconnected,
	C2IMAPNonAuthenticated,
	C2IMAPAuthenticated,
	C2IMAPSelected
};

struct _C2IMAP
{
	C2NetObject object;

	C2Account *account;
	
	/* [TODO] */
	C2IMAPAuthenticationType auth;

	gchar *user;
	gchar *pass;
	gchar *path;

	tag_t cmnd : 10;
	gint input_tag;

	gint auth_remember   :1;	 /* %TRUE = Store password */
	gint only_subscribed :1;   /* %TRUE = list only subscribed mailboxes */

	gint (*login) (C2IMAP *imap);
	
	GList *hash;      /* hash to store server replies...*/
	
	GSList *pending;   /* linked list to store process mutex locks	for processes
										 * that are expecting a tagged response from the server */
	
	C2IMAPState state;
	C2Mutex lock;
	
	C2Mailbox *mailboxes;
	C2Mailbox *selected_mailbox;
};

struct _C2IMAPClass
{
	C2NetObjectClass parent_class;

	void (*login) (C2IMAP *imap);
	gboolean (*login_failed) (C2IMAP *imap, const gchar *error, gchar **user, gchar **pass, C2Mutex *lock);
	void (*mailbox_list) (C2IMAP *imap, C2Mailbox *head);
	void (*incoming_mail) (C2IMAP *imap);
	void (*logout) (C2IMAP *imap);
	void (*net_error)  (C2IMAP *imap);
};
	
struct _C2IMAPServerReply
{
	tag_t tag;
	gchar *value;
};
	
struct _C2IMAPPending
{
	tag_t tag;
	C2Mutex lock;
};

GtkType
c2_imap_get_type							(void);

C2IMAP *
c2_imap_new						(C2Account *account, const gchar *host, guint port, 
											 const gchar *user, const gchar *pass, const gchar *path,
											 C2IMAPAuthenticationType auth, gboolean ssl);

gint
c2_imap_init								(C2IMAP *imap);

gint
c2_imap_populate_folders					(C2IMAP *imap);

gboolean
c2_imap_create_mailbox					(C2IMAP *imap, C2Mailbox *parent, const gchar *name);

gboolean
c2_imap_delete_mailbox						(C2IMAP *imap, C2Mailbox *mailbox);
	
gint
c2_imap_rename_mailbox					(C2IMAP *imap, C2Mailbox *mailbox, gchar *name);

gboolean
c2_imap_subscribe_mailbox				(C2IMAP *imap, C2Mailbox *mailbox);
	
gboolean
c2_imap_unsubscribe_mailbox			(C2IMAP *imap, C2Mailbox *mailbox);
	
gboolean
c2_imap_mailbox_is_subscribed		 (C2IMAP *imap, C2Mailbox *mailbox);
	
gboolean
c2_imap_load_mailbox						(C2IMAP *imap, C2Mailbox *mailbox);

gint
c2_imap_message_remove (C2IMAP *imap, GList *list);
	
gint
c2_imap_message_add (C2IMAP *imap, C2Mailbox *mailbox, C2Db *db);

C2Message *
c2_imap_load_message (C2IMAP *imap, C2Db *db);

gint
c2_imap_message_set_state (C2IMAP *imap, C2Db *db, C2MessageState state);

#ifdef __cplusplus
}
#endif

#endif
