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
#ifndef __LIBCRONOSII_SMTP_H__
#define __LIBCRONOSII_SMTP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <glib.h>
#include <pthread.h>

#if HAVE_CONFIG_H
#	include "db.h"
#else
#	include <cronosII.h>
#endif

typedef enum _C2SMTPType C2SMTPType;

enum _C2SMTPType
{
	C2_SMTP_REMOTE,
	C2_SMTP_LOCAL
};

enum
{
	C2_SMTP_DO_PERSIST			= 1 << 0,	/* Will keep the connection alive for future requests */
	C2_SMTP_DONT_PERSIST		= 1 << 1	/* Will not keep the connection alive */
};

typedef struct
{
	gchar *address;
	gint port;

	gint flags;

	pthread_mutex_t lock;

	guint sock;
} C2Smtp;

C2Smtp *
c2_smtp_new (const gchar *address, gint port);

void
c2_smtp_set_flags (C2Smtp *smtp, gint flags);

gint
c2_smtp_send_message (C2Smtp *smtp, C2Message *message);

void
c2_smtp_free (C2Smtp *smtp);

#ifdef __cplusplus
}
#endif

#endif
