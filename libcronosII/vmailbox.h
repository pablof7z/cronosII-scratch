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
#ifndef __LIBCRONOSII_VMAILBOX_H__
#define __LIBCRONOSII_VMAILBOX_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <gtk/gtk.h>

#define C2_VMAILBOX(obj)					(GTK_CHECK_CAST (obj, c2_vmailbox_get_type (), C2VMailbox))
#define C2_VMAILBOX_CLASS(klass)			(GTK_CHECK_CLASS_CAST (klass, c2_vmailbox_get_type (), C2VMailboxClass))
#define C2_IS_VMAILBOX(obj)					(GTK_CHECK_TYPE (obj, c2_vmailbox_get_type ())
#define C2_IS_VMAILBOX_CLASS(obj)			(GTK_CHECK_CLASS_TYPE (klass, c2_vmailbox_get_type ())

typedef struct _C2VMailbox C2VMailbox;
typedef struct _C2VMailboxClass C2VMailboxClass;

#include <libcronosII/vdb.h>

struct _C2VMailbox
{
	GtkObject object;
	
	C2VDb *vdb;

	GSList *rules;
};

struct _C2VMailboxClass
{
	GtkObjectClass parent_class;
};

GtkType
c2_vmailbox_get_type						(void);

C2VMailbox *
c2_vmailbox_new								(gchar *name, ...);

#ifdef __cplusplus
}
#endif

#endif
