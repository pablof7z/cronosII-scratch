/*  Cronos II - The GNOME mail client
 *  Copyright (C) 2000-2001 Pablo Fernández
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
 * 		* Pablo Fernández
 * Code of this file by:
 * 		* Pablo Fernández
 */
#ifndef __LIBCRONOSII_UTILS_DEBUGGING_H__
#define __LIBCRONOSII_UTILS_DEBUGGING_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef USE_DEBUG
/* ALL THE DEBUGGING SYMBOLS FOR /LIBCRONOSII ARE IN HERE */
short _debug_db;
short _debug_db_cronosII;
short _debug_db_imap;
short _debug_db_spool;
short _debug_imap;
short _debug_mailbox;
short _debug_message;
short _debug_mime;
short _debug_net_object;
short _debug_pop3;
short _debug_request;
short _debug_smtp;
#endif

#ifdef USE_DEBUG
#	define L								g_print ("%s:%d:%s\n", __FILE__, __LINE__, \
													__PRETTY_FUNCTION__);
#	define C2_DEBUG(x)						g_print ("%s:%d:%s:%s: %s\n", __FILE__, __LINE__,\
													__PRETTY_FUNCTION__, #x, x)
#	define C2_DEBUG_(x)						{ x }
#	define C2_TODO							g_print ("%s:%d:%s: TODO\n" __FILE__, __LINE__, \
													__PRETTY_FUNCTION__)

#	define C2_PRINTD(mod, fmt, args...)		if (DMOD) g_print ("[ " mod " ] -- " __PRETTY_FUNCTION__ "() -- " fmt, ##args)

#else
#	define L								;
#	define C2_PRINTD(x)						;
#	define C2_DEBUG(x)						;
#	define C2_DEBUG_(x)						;
#	define C2_TODO							;
#	define C2_PRINTD(mod, args...)			
#endif

#define unless(x)							if (FAIL(x))

#ifdef __cplusplus
}
#endif

#endif
