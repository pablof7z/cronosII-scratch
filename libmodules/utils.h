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
#ifndef __LIBMODULES_UTILS_H__
#define __LIBMODULES_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <glib.h>
#include <stdio.h>

#define CHAR(x)										((gchar*)x)

typedef void *(*PthreadFunc)						(void*);
#define PTHREAD_FUNC(x)								((PthreadFunc)x)

gboolean
c2_strcaseeq									(const gchar *fst, const gchar *snd);

gboolean
c2_strncaseeq									(const gchar *fst, const gchar *snd, gint length);

gboolean
c2_streq										(const gchar *fst, const gchar *snd);

gboolean
c2_strneq										(const gchar *fst, const gchar *snd, gint length);

gchar *
c2_str_replace_all								(const gchar *or_string, const gchar *se_string,
												 const gchar *re_string);

gchar *
c2_str_get_line									(const gchar *str);

gchar *
c2_str_get_word									(guint8 word_n, const gchar *str, gchar ch);

gchar *
c2_fd_get_line									(FILE *fd);

gchar *
c2_fd_get_word									(FILE *fd);

gboolean
c2_file_exists									(const gchar *file);

gboolean
c2_fd_move_to									(FILE *fp, gchar c, guint8 cant,
										 		 gboolean forward, gboolean next);

#ifdef __cplusplus
}
#endif

#endif
