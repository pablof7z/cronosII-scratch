/*  Cronos II - A GNOME mail client
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
#ifndef __OBJECT_UI_DATA_H__
#define __OBJECT_UI_DATA_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <config.h>
#include <gnome.h>
#include <pthread.h>

#ifdef BUILDING_C2
#else
#	include <cronosII.h>
#endif
	
#define C2_UI_DATA(obj)							(GTK_CHECK_CAST(obj, c2_ui_data_get_type (), C2UIData))
#define C2_UI_DATA_CLASS(klass)					(GTK_CHECK_CLASS_CAST (klass, c2_ui_data_get_type, C2UIDataClass))
#define C2_IS_UI_DATA(obj)						(GTK_CHECK_TYPE (obj, c2_ui_data_get_type ())

#define C2_UI_DATA_KEY							"UIdata"

typedef struct _C2UIData C2UIData;
typedef struct _C2UIDataClass C2UIDataClass;

struct _C2UIData
{
	GtkObject object;

	gchar *source;

	FILE *file;

	pthread_mutex_t lock;
};

struct _C2UIDataClass
{
	GtkObjectClass parent_class;
};

GtkType
c2_ui_data_get_type								(void);

C2UIData *
c2_ui_data_new									(GtkObject *object);

C2UIData *
c2_ui_data_new_with_source						(GtkObject *object, const gchar *source);

C2UIData *
gtk_object_get_ui_data							(GtkObject *object);

void
c2_ui_data_set_source							(C2UIData *uidata, const gchar *source);

const gchar *
c2_ui_data_get_source							(C2UIData *uidata);

gchar *
c2_ui_data_query								(C2UIData *uidata, const gchar *query);

#ifdef __cplusplus
}
#endif

#endif
