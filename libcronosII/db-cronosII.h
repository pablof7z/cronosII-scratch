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
#ifndef __LIBCRONOSII_DB_CRONOSII_H__
#define __LIBCRONOSII_DB_CRONOSII_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <glib.h>

#include "db.h"
#include "mailbox.h"

C2Db *
c2_db_cronosII_load								(C2Mailbox *mailbox);

C2Db *
c2_db_cronosII_message_add						(C2Db *db, const C2Message *message, gint row);

gint
c2_db_cronosII_message_remove					(C2Db *db, gint row);

C2Message *
c2_db_cronosII_message_get						(C2Db *db, gint row);

#ifdef __cplusplus
}
#endif

#endif
