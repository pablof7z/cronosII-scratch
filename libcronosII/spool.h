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
#ifndef __LIBCRONOSII_SPOOL_H__
#define __LIBCRONOSII_SPOOL_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _C2Spool C2Spool;

enum
{
	C2_SPOOL_DO_KEEP_COPY			= 1 << 0,	/* Will get a mail and leave a copy on server */
	C2_SPOOL_DONT_KEEP_COPY			= 1 << 1	/* Will get a mail and delete it on server */
};

struct _C2Spool
{
	gchar *file;

	gint flags;
};

C2Spool *
c2_spool_new (const gchar *file);

void
c2_spool_set_flags (C2Spool *spool, gint flags);

void
c2_spool_free (C2Spool *spool);

#ifdef __cplusplus
}
#endif

#endif
