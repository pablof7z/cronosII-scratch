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
#ifndef __WIDGET_INDEX_H__
#define __WIDGET_INDEX_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <gtk/gtkclist.h>

#define C2_INDEX(obj)						GTK_CHECK_CAST (obj, c2_index_get_type (), C2Index)
#define C2_INDEX_CLASS(klass)				GTK_CHECK_CLASS_CAST (klass, c2_index_get_type (), C2IndexClass)
#define C2_IS_INDEX(obj)				    GTK_CHECK_TYPE (obj, c2_index_get_type ())

typedef enum
{
	C2_INDEX_ACCESS_C2,
	C2_INDEX_ACCESS_IMAP /* TODO */
} C2IndexAccessType;

typedef struct
{
	GtkCList clist;

	C2IndexAccessType access_type;
} C2Index;

typedef struct
{
	GtkCListClass parent_class;

	

#ifdef __cplusplus
}
#endif

#endif
