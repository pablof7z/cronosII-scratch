/*  Cronos II Mail Client
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
#ifndef __LIBCRONOSII_I18N_H__
#define __LIBCRONOSII_I18N_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <config.h>

#ifdef ENABLE_NLS
#	include <libintl.h>
#	ifndef _
#		define _(String) gettext (String)
#	endif
#	ifndef N_
#		ifdef gettext_noop
#			define N_(String) gettext_noop (String)
#		else
#			define N_(String) (String)
#		endif
#	endif
#else
#  ifndef _
#		define _(String) (String)
#	endif
#	ifndef N_
#		define N_(String) (String)
#	endif
#endif

#ifdef __cplusplus
}
#endif

#endif
