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
/**
 * Maintainer(s) of this file:
 * 		* Bosko Blagojevic
 * Code of this file by:
 * 		* Bosko Blagojevic
 */

/* C2Mutex -- implemented by Bosko <falling@users.sourceforge.net> so we have
 *  one set of easy-to-use and not-so-POSIX mutexs for CronosII :-) 
 * 
 * Inspired by FreeBSD's less-than-kind mutexs ;-)
 */


#ifndef __LIBCRONOSII_UTILS_MUTEX_H__
#define __LIBCRONOSII_UTILS_MUTEX_H__

#ifdef __cplusplus
extern "C" {
#endif
		
#include <glib.h>

typedef struct _C2Mutex C2Mutex;

struct _C2Mutex
{
	gboolean dead;
	gboolean lock;
	GList *queue;
};

gint
c2_mutex_init								(C2Mutex *mutex);

gint
c2_mutex_lock								(C2Mutex *mutex);

gint
c2_mutex_trylock							(C2Mutex *mutex);

gint
c2_mutex_unlock								(C2Mutex *mutex);

gint
c2_mutex_destroy							(C2Mutex *mutex);

#ifdef __cplusplus
}
#endif

#endif
