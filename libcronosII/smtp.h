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

/* C2SMTP -- Cronos II's Object for sending messages via SMTP */
/*                                                            */
/*                  -- Public Interface --                    */
/*                                                            */
/* Notes: This object inherits its network functionality from */
/*       the Cronos II Net-Object (see net-object.h in the C2 */
/*       source files). It also uses Gtk+ 1.2 for its signals */
/*       and is, in turn, a GtkObject.                        */
/*                                                            */
/* c2_smtp_new(C2_SMTP_REMOTE, gchar *host, gint port,        */
/*             gboolean ssl, gboolean auth, gchar *user,      */
/*             gchar *pass) -- Returns a new C2SMTP object    */
/*             that will send messages via SMTP server at host*/
/*             on port number port, and use ssl or auth       */
/*             optionally.                                    */
/*                                                            */
/* TODO: Finish me!                                           */
/*                                                            */

#ifndef __LIBCRONOSII_SMTP_H__
#define __LIBCRONOSII_SMTP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <glib.h>
#include <pthread.h>

#ifdef BUILDING_C2
#	include "db.h"
#	include "net-object.h"
#else
#	include <cronosII.h>
#endif

#define C2_TYPE_SMTP						(c2_smtp_get_type ())
#define C2_SMTP(obj)						(GTK_CHECK_CAST (obj, C2_TYPE_SMTP, C2SMTP))
#define C2_SMTP_CLASS(klass)				(GTK_CHECK_CLASS (klass, C2_TYPE_SMTP, C2SMTP))
#define C2_IS_SMTP(obj)						(GTK_CHECK_TYPE (obj, C2_TYPE_SMTP))
#define C2_IS_SMTP_CLASS(klass)				(GTK_CHECK_CLASS_TYPE (klass, C2_TYPE_SMTP))
	
typedef enum _C2SMTPType C2SMTPType;

enum _C2SMTPType
{
	C2_SMTP_REMOTE,
	C2_SMTP_LOCAL
};

enum
{
	C2_SMTP_DO_PERSIST			= 1 << 1,	/* Will keep the connection alive for future requests */
	C2_SMTP_DO_NOT_PERSIST		= 0 << 1,	/* Will not keep the connection alive */
	C2_SMTP_DO_LOSE_PASSWORD	= 1 << 2,	/* Will delete the password once it sended it correctly */
	C2_SMTP_DO_NOT_LOSE_PASSWORD= 0 << 2	/* Will keep the password unless its wrong */
};

typedef struct _C2SMTP C2SMTP;
typedef struct _C2SMTPClass C2SMTPClass;
	
struct _C2SMTP
{
	C2NetObject object;
	
	C2SMTPType type;

	gchar *host;
	gint port;
	gboolean ssl;
	gboolean authentication;
	gchar *user, *pass;
	gchar *smtp_local_cmd;

	gint flags;
	
	pthread_mutex_t lock;
};
	
struct _C2SMTPClass
{
	C2NetObjectClass parent_class;
	
	void (*smtp_update) (C2SMTP *smtp, C2Message *message, guint length, guint bytes);
	/*gboolean (*login_failed) (C2SMTP *smtp, const gchar *error, gchar **user, gchar **pass);*/
};

GtkType
c2_smtp_get_type							(void);

C2SMTP *
c2_smtp_new									(C2SMTPType type, ...);

void
c2_smtp_set_flags							(C2SMTP *smtp, gint flags);

gint
c2_smtp_send_message						(C2SMTP *smtp, C2Message *message);

void
c2_smtp_free								(C2SMTP *smtp);

#ifdef __cplusplus
}
#endif

#endif
