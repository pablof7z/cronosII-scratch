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
#ifndef __C2_MAIN_WINDOW_H__
#define __C2_MAIN_WINDOW_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <glib.h>
	
#if defined (HAVE_CONFIG_H) && defined (BUILDING_C2)
#	include <libcronosII/mailbox.h>
#	include "main-window.h"
#else
#	include <cronosII.h>
#endif

void
c2_main_window_set_sensitivity					(void);
	
void
on_new_mailbox_dlg								(void);

void
on_properties_mailbox_dlg						(void);
	
#ifdef __cplusplus
}
#endif

#endif
