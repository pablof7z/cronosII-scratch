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
#ifndef __LIBMODULES_SMTP_H__
#define __LIBMODULES_SMTP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <glib.h>

#if HAVE_CONFIG_H
#	include "db.h"
#else
#	include <cronosII.h>
#endif

typedef struct
{
	gchar *address;
	gint port;

	gint sock;
} C2Smtp;

C2Smtp *
c2_smtp_new (const gchar *address, gint port);

void
c2_smtp_new (C2Smtp *smtp);

gint
c2_smtp_connect (C2Smtp *smtp);

gint
c2_smtp_send_message (C2Smtp *smtp, C2Message *message);

gint
c2_smtp_disconnect (C2Smtp *smtp);

#ifdef __cplusplus
}
#endif

#endif
