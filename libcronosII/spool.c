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
#include <glib.h>
#include <config.h>

#include "error.h"
#include "spool.h"

#define DEFAULT_FLAGS C2_SPOOL_DONT_KEEP_COPY

C2Spool *
c2_spool_new (const gchar *file)
{
	C2Spool *spool;
	
	c2_return_val_if_fail (file, NULL, C2EDATA);
	
	spool = g_new0 (C2Spool, 1);
	spool->file = g_strdup (file);
	spool->flags = DEFAULT_FLAGS;

	return spool;
}

void
c2_spool_set_flags (C2Spool *spool, gint flags)
{
	c2_return_if_fail (spool, C2EDATA);
	
	spool->flags = flags;
}

void
c2_spool_free (C2Spool *spool)
{
	c2_return_if_fail (spool, C2EDATA);

	g_free (spool->file);
	g_free (spool);
}
