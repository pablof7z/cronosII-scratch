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

#ifdef BUILDING_C2
#	include "net-object.h"
#	include "mailbox.h"
#else
#	include <cronosII.h>
#endif

#define C2_TYPE_IMAP						(c2_imap_get_type ())
#define C2_IMAP(obj)						(GTK_CHECK_CAST (obj, C2_TYPE_IMAP, C2IMAP))
#define C2_IMAP_CLASS(klass)				(GTK_CHECK_CLASS (klass, C2_TYPE_IMAP, C2IMAP))
#define C2_IS_IMAP(obj)						(GTK_CHECK_TYPE (obj, C2_TYPE_IMAP))
#define C2_IS_IMAP_CLASS(klass)				(GTK_CHECK_CLASS_TYPE (klass, C2_TYPE_IMAP))

typedef struct _C2IMAP C2IMAP;
typedef struct _C2IMAPClass C2IMAPClass;
typedef enum _C2IMAPAuthenticationType C2IMAPAuthenticationType;

enum _C2IMAPAuthenticationType
{
	C2_IMAP_AUTHENTICATION_PASSWORD
};

struct _C2IMAP
{
	C2NetObject object;

	/* [TODO] */
	C2IMAPAuthenticationType auth;

	gchar *user;
	gchar *pass;

	gint cmnd;

	gint auth_remember :1;		/* %TRUE = Store password */

	pthread_mutex_t lock;
	
	C2Mailbox *mailboxes;
	C2Mailbox *selected_mailbox;
};

struct _C2IMAPClass
{
	C2NetObjectClass parent_class;

	gchar* (*login_failed) (C2IMAP *imap, const gchar *error);
	void (*mailbox_list) (C2IMAP *imap, C2Mailbox *head);
	void (*incoming_mail) (C2IMAP *imap);
	void (*logout) (C2IMAP *imap);
};

GtkType
c2_imap_get_type							(void);

C2IMAP *
c2_imap_new									(gchar *host, gint port, gchar *user, gchar *pass, gboolean ssl);

void
c2_imap_init								(C2IMAP *imap);

#ifdef __cplusplus
}
#endif

#endif
