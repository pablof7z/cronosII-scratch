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
#include <gnome.h>
#include <config.h>

#include <libcronosII/error.h>
#include <libcronosII/utils.h>
#include <libcronosII/utils-str.h>

#include "object-ui-data.h"

static void
class_init										(C2UIDataClass *klass);

static void
init											(C2UIData *data);

static void
destroy											(GtkObject *obj);

static void
destroy_object									(GtkObject *object);

enum
{
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static GtkObjectClass *parent_class = NULL;

GtkType
c2_ui_data_get_type (void)
{
	static GtkType type = 0;

	if (!type)
	{
		GtkTypeInfo info =
		{
			"C2UIData",
			sizeof (C2UIData),
			sizeof (C2UIDataClass),
			(GtkClassInitFunc) class_init,
			(GtkObjectInitFunc) init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL,
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (gtk_object_get_type (), &info);
	}

	return type;
}

static void
class_init (C2UIDataClass *klass)
{
	GtkObjectClass *obj_klass;

	obj_klass = (GtkObjectClass *) klass;
	
	parent_class = gtk_type_class (gtk_object_get_type ());

	obj_klass->destroy = destroy;
}

static void
init (C2UIData *uidata)
{
	uidata->source = NULL;
	uidata->file = NULL;
	pthread_mutex_init (&uidata->lock, NULL);
}

static void
destroy (GtkObject *object)
{
	C2UIData *uidata = C2_UI_DATA (object);

	fclose (uidata->file);
	g_free (uidata->source);
	pthread_mutex_destroy (&uidata->lock);
}

static void
destroy_object (GtkObject *object)
{
	gtk_object_destroy (gtk_object_get_data (object, C2_UI_DATA_KEY));
}

C2UIData *
c2_ui_data_new (GtkObject *object)
{
	C2UIData *uidata;
	
	c2_return_val_if_fail (object, NULL, C2EDATA);

	if ((uidata = gtk_object_get_data (object, C2_UI_DATA_KEY)))
		return uidata;

	uidata = gtk_type_new (c2_ui_data_get_type ());

	gtk_object_set_data (object, C2_UI_DATA_KEY, uidata);
	
	gtk_signal_connect (object, "destroy",
							GTK_SIGNAL_FUNC (destroy_object), NULL);

	return uidata;
}

C2UIData *
c2_ui_data_new_with_source (GtkObject *object, const gchar *source)
{
	C2UIData *uidata = c2_ui_data_new (object);
	
	c2_ui_data_set_source (uidata, source);

	return uidata;
}

C2UIData *
c2_ui_get_data (GtkObject *object)
{
	return gtk_object_get_data (object, C2_UI_DATA_KEY);
}

void
c2_ui_data_set_source (C2UIData *uidata, const gchar *source)
{
	if (!(uidata->file = fopen (source, "rt")))
		return;
	
	uidata->source = g_strdup (source);
}

const gchar *
c2_ui_data_get_source (C2UIData *uidata)
{
	c2_return_val_if_fail (uidata, NULL, C2EDATA);
	
	return uidata->source;
}

gchar *
c2_ui_data_query (C2UIData *uidata, const gchar *query)
{
	gchar *retval = NULL;
	gchar *cmnd;

	cmnd = c2_str_get_word (0, query, ' ');

	if (c2_streq (cmnd, "GET"))
	{
		
	} else if (c2_streq (cmnd, "SET"))
	{

		
	}

	g_free (cmnd);

	return retval;
}

static void
query_set (const gchar *key, const gchar *text)
{
}
