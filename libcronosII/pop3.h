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
#ifndef __LIBCRONOSII_POP_H__
#define __LIBCRONOSII_POP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <glib.h>

enum
{
	C2_POP3_DO_KEEP_COPY			= 1 << 0,	/* Will get a mail and leave a copy on server */
	C2_POP3_DONT_KEEP_COPY			= 1 << 1,	/* Will get a mail and delete it on server */
	C2_POP3_DO_LOSE_PASSWORD		= 1 << 2,	/* Will delete the password once it sended it correctly */
	C2_POP3_DONT_LOSE_PASSWORD		= 1 << 3	/* Will keep the password unless its wrong */
};

typedef struct _C2Pop3 C2Pop3;

typedef gchar *(*C2Pop3GetPass) 				(C2Pop3 *pop3, const gchar *error);

#define C2_POP3_GET_PASS_FUNC(x)				((C2Pop3GetPass)x)

struct _C2Pop3
{
	gchar *user;
	gchar *pass;
	gchar *host;
	gint port;

	gint flags;

	C2Pop3GetPass wrong_pass_cb;

	guint sock;
};

C2Pop3 *
c2_pop3_new (const gchar *user, const gchar *pass, const gchar *host, gint port);

void
c2_pop3_set_flags (C2Pop3 *pop3, gint flags);

void
c2_pop3_set_wrong_pass_cb (C2Pop3 *pop3, C2Pop3GetPass func);

void
c2_pop3_free (C2Pop3 *pop3);

gint
c2_pop3_fetchmail (C2Pop3 *pop3);

#ifdef __cplusplus
}
#endif

#endif
