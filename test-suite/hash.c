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
#include <libcronosII/hash.h>
#include <libcronosII/utils.h>

static void
foreach (gpointer key, gpointer value, gpointer data)
{
	gint **ke = key;
	gint *val = value;
	printf ("%d %d\n", *ke, *val);
}

gint
main (gint argc, gchar **argv)
{
	gchar *name;
	gint module = C2_HASH_TEST;
	
	gtk_init (&argc, &argv);

	c2_hash_init ();

	printf ("Inserting name=pablo\n");
	c2_hash_insert_key (module, "name", g_strdup ("pablo"));

	printf ("Checking if the insert was succesful\n");
	name = (gchar *) c2_hash_search_key (module, "name");
	if (name)
		printf ("Founded: %s\n", name);
	else
		printf ("Not founded!\n");

	printf ("Checking if the insert was succesful\n");
	name = (gchar *) c2_hash_search_key (module, "name");
	if (name)
		printf ("Founded: %s\n", name);
	else
		printf ("Not founded!\n");

	printf ("Test table is %d length\n", g_hash_table_size (c2_hash_get_module_table (module)));

	printf ("Removing name\n");
	c2_hash_remove_key (module, "name");

	printf ("Test table is %d length\n", g_hash_table_size (c2_hash_get_module_table (module)));

	return 0;
}

/*
 * table = g_hash_table_new (g_int_hash, g_int_equal);
 * retval = g_hash_table_lookup (table, (gpointer) &module)
 * module_table = g_hash_table_new (g_str_hash, g_str_equal)
 * g_hash_table_insert (table, (gpointer) &module, (gpointer) module_table)
 * g_hash_table_insert (module_table, (gpointer) key, (gpointer) value)
 * retval = g_hash_table_lookup (table, (gpointer) &module)
 * g_hash_table_lookup (module_table, key)
 * retval = g_hash_table_lookup (table, (gpointer) &module)
 * g_hash_table_remove (module_table, key)
*/
