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
#ifndef __MAIN_H__
#define __MAIN_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef BUILDING_C2
#	include "widget-application.h"
#else
#	include <cronosII.h>
#endif

/* Global variables */
C2Application *application;
extern gchar *error_list[];

enum
{
	C2_FAIL_MESSAGE_LOAD,
	C2_FAIL_MESSAGE_SAVE,
	C2_FAIL_MAILBOX_LOAD,
	C2_FAIL_MAILBOX_CREATE,
	C2_FAIL_FILE_SAVE,

	C2_SUCCESS_MESSAGE_SAVE,
	C2_SUCCESS_FILE_SAVE,
	
	C2_CANCEL_USER,
	C2_NO_MAILBOX_SELECTED,
	C2_UNKNOWN_REASON
};

#ifdef __cplusplus
}
#endif

#endif
