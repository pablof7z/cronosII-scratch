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
#ifndef __LIBCRONOSII_IMAP_H__
#define __LIBCRONOSII_IMAP_H__

/* This module is in charge of Pablo and Bosko.
 * Let's go Bosko! We can do it!
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <glib.h>
#include <pthread.h>

#define C2_TYPE_IMAP						(c2_imap_get_type ())
#define C2_IMAP(obj)						(GTK_CHECK_CAST (obj, C2_TYPE_IMAP, C2IMAP))
#define C2_IMAP_CLASS(klass)				(GTK_CHECK_CLASS (klass, C2_TYPE_IMAP, C2IMAP))
#define C2_IS_IMAP(obj)						(GTK_CHECK_TYPE (obj, C2_TYPE_IMAP))
#define C2_IS_IMAP_CLASS(klass)				(GTK_CHECK_CLASS_TYPE (klass, C2_TYPE_IMAP))

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
# include "utils-mutex.h"
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

	/* [TODO] */
	C2IMAPAuthenticationType auth;

	gchar *user;
	gchar *pass;
	gchar *path;

	tag_t cmnd : 10;

	gint auth_remember :1;		/* %TRUE = Store password */

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
	void (*login_failed) (C2IMAP *imap);
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
c2_imap_new									(const gchar *host, guint port, const gchar *user,
											 const gchar *pass, const gchar *path,
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
c2_imap_load_mailbox						(C2IMAP *imap, C2Mailbox *mailbox);

gchar *
c2_imap_get_full_folder_name (C2IMAP *imap, C2Mailbox *mailbox);	
	
#ifdef __cplusplus
}
#endif

#endif
