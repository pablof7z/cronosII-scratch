/*  Cronos II Mail Client /libcronosII/db-spool.h
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

#ifdef BUILDING_C2
#	include "mailbox.h"
#else
#	include <cronosII.h>
#endif

gint
c2_db_spool_load								(C2Mailbox *mailbox);

gint
c2_db_spool_create_structure					(C2Mailbox *mailbox);

gint
c2_db_spool_update_structure					(C2Mailbox *mailbox);

gint
c2_db_spool_remove_structure					(C2Mailbox *mailbox);

#ifdef __cplusplus
}
#endif

#endif
