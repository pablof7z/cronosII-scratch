/*  Cronos II - The GNOME mail client
 *  Copyright (C) 2000-2001 Pablo Fernández López
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
/**
 * Maintainer(s) of this file:
 * 		* Pablo Fernández López
 * Code of this file by:
 * 		* Pablo Fernández López
 */
#ifndef __LIBCRONOSII_VDB_H__
#define __LIBCRONOSII_VDB_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <gtk/gtk.h>
#include <libcronosII/db.h>

#define C2_VDB(obj)							(GTK_CHECK_CAST (obj, c2_vdb_get_type (), C2VDb))
#define C2_VDB_CLASS(klass)					(GTK_CHECK_CLASS_CAST (klass, c2_vdb_get_type (), C2VDbClass))
#define C2_IS_VDB(obj)						(GTK_CHECK_TYPE (obj, c2_vdb_get_type ())
#define C2_IS_VDB_CLASS(obj)				(GTK_CHECK_CLASS_TYPE (klass, c2_vdb_get_type ())

typedef struct _C2VDb C2VDb;
typedef struct _C2VDbClass C2VDbClass;

#include <libcronosII/vmailbox.h>

struct _C2VDb
{
	GtkObject object;
	
	C2Db *db;

	gint position;

	C2VMailbox *mailbox;
	C2VDb *prev;
	C2VDb *next;
};

struct _C2VDbClass
{
	GtkObjectClass parent_class;
};

GtkType
c2_vdb_get_type								(void);

C2VDb *
c2_vdb_new									(C2Db *db);

C2VDb *
c2_vdb_append								(C2VDb *head, C2VDb *item);

C2Message *
c2_vdb_get_message							(C2VDb *vdb);

#ifdef __cplusplus
}
#endif

#endif
