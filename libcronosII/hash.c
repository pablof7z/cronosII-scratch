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
#include <libcronosII/error.h>
#include <libcronosII/utils.h>
#include <stdlib.h>

#include "hash.h"

static void
init											(void);

static GHashTable *table = NULL;

static void
init (void)
{
	table = g_hash_table_new (g_int_hash, g_int_equal);
}

static void
destroy (void)
{
	
}

static void
forever (gpointer key, gpointer value, gpointer data)
{
	gint *ke = key;
	gint *val = value;
	printf ("%d %d\n", *ke, *val);
}

static GHashTable*
get_module_table (gint module)
{
	GHashTable *retval;
	
	if (!(retval = g_hash_table_lookup (table, (gpointer) &module)))
	{
		printf ("Unable to find %d\n", module);
		printf ("table is %d length\n", g_hash_table_size (table));
		g_hash_table_foreach (table, forever, NULL);
	}

	return retval;
}

void
c2_hash_init (void)
{
	if (!table)
		init ();
}

void
c2_hash_destroy (void)
{
	destroy ();
}

/**
 * c2_hash_new_key
 * @module: The module where the key will exist.
 *
 * This function will create a dynamic key for use with
 * the c2_hash_* family functions, which will be owned by
 * the module @module.
 *
 * Return Value:
 * The new dynamic key.
 **/
gint
c2_hash_new_key (gint module)
{
	static gint modid = C2_HASH_DKEY;
	gchar *strmod = g_strdup_printf ("%d", module);
	gchar *value, *new_value;
	gint new_key;

	value = c2_hash_search_key (modid, strmod);	

	if (value)
	{
		C2_DEBUG (value);
		new_key = atoi (value)+1;
		new_value = g_strdup_printf ("%d", new_key);
	} else
	{
L		new_key = 1;
		new_value = g_strdup ("1");
	}

	c2_hash_insert_key (modid, strmod, new_value);

	g_free (value);
	g_free (strmod);

	return new_key;
}

void
c2_hash_insert_key (gint module, const gchar *key, gpointer value)
{
	GHashTable *module_table;
	
	c2_return_if_fail (key, C2EDATA);
	c2_return_if_fail (value, C2EDATA);

	/* Check if the hash table for this module is loaded */
	if (!(module_table = get_module_table (module)))
	{
		printf ("Creating %d\n", module);
		module_table = g_hash_table_new (g_str_hash, g_str_equal);

		g_hash_table_insert (table, (gpointer) &module, (gpointer) module_table);
	}

	g_hash_table_insert (module_table, (gpointer) key, (gpointer) value);
}

gpointer
c2_hash_search_key (gint module, const gchar *key)
{
	GHashTable *module_table = get_module_table (module);

	if (!module_table)
		return NULL;
	
	return g_hash_table_lookup (module_table, key);
}

void
c2_hash_remove_key (gint module, const gchar *key)
{
	GHashTable *module_table = get_module_table (module);

	g_hash_table_remove (module_table, key);

	if (!g_hash_table_size (module_table))
		c2_hash_destroy_module (module);
}

void
c2_hash_destroy_module (gint module)
{
	GHashTable *module_table = get_module_table (module);	
	
	g_hash_table_destroy (module_table);
	g_hash_table_remove (table, &module);
}

GHashTable *
c2_hash_get_table (void)
{
	return table;
}

GHashTable *
c2_hash_get_module_table (gint module)
{
	return get_module_table (module);
}
