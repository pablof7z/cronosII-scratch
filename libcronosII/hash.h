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
#ifndef __LIBCRONOSII_HASH_H__
#define __LIBCRONOSII_HASH_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <glib.h>

#include "hash-definitions.h"

/* This should be called before any other c2_hash function */
void
c2_hash_init									(void);

/* This should be called when the program is about to quit */
void
c2_hash_destroy									(void);

void
c2_hash_insert_key								(gint module, const gchar *key, gpointer value);

gpointer
c2_hash_search_key								(gint module, const gchar *key);

void
c2_hash_remove_key								(gint module, const gchar *key);

void
c2_hash_destroy_module							(gint module);

/* I don't know why you might want to use this function, but
 * I'll provide it anyway...
 */
GHashTable *
c2_hash_get_table								(void);

GHashTable *
c2_hash_get_module_table						(gint module);

#ifdef __cplusplus
}
#endif

#endif
