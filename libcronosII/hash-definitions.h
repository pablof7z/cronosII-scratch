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
#ifndef __LIBCRONOSII_HASH_DEFINITIONS_H__
#define __LIBCRONOSII_HASH_DEFINITIONS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* If you want to register something with the C2 Hash API
 * you must first declare here the UNIQUE key you are going
 * to use.
 * The way to declare this is by defining the name of the key,
 * checking that there's no other key with this name and putting
 * the number of the upper key plus 1 and a short description next
 * to it.
 */
#define C2_HASH_NULL	0	/* Null key */
#define C2_HASH_TEST	1	/* Test key */
#define C2_HASH_IMAP	2	/* DB IMAP module access key */
#define C2_HASH_DKEY	3	/* Dynamic Key Creation key */

#ifdef __cplusplus
}
#endif

#endif
