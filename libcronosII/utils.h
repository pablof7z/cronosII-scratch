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
#ifndef __LIBCRONOSII_UTILS_H__
#define __LIBCRONOSII_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <glib.h>
#include <stdio.h>
#include <pthread.h>
#include <gtk/gtk.h>
#if defined (HAVE_CONFIG_H) && defined (BUILDING_C2)
#	include <config.h>
#else
#	include <cronosII.h>
#endif

#define C2_HOME								/*$HOME*/ G_DIR_SEPARATOR_S ".c2" G_DIR_SEPARATOR_S

#define C2_CHAR(x)							((gchar*)x)

#define c2_strne(x,y)						(!(c2_streq (x, y)))
#define c2_strnne(x,y,z)					(!(c2_strneq (x, y, z)))

typedef void *(*C2PthreadFunc)				(void*);
#define C2_PTHREAD_FUNC(x)					((C2PthreadFunc)x)

typedef struct
{
	gpointer v1, v2;
} C2Pthread2;

typedef struct
{
	gpointer v1, v2, v3;
} C2Pthread3;

typedef struct
{
	gpointer v1, v2, v3, v4;
} C2Pthread4;

#ifdef USE_DEBUG
#	define L								g_print ("%s:%d:%s\n", __FILE__, __LINE__, \
													__PRETTY_FUNCTION__);
#	define C2_DEBUG(x)						g_print ("%s:%d:%s:%s: %s\n", __FILE__, __LINE__,\
													__PRETTY_FUNCTION__, #x, x)
#	define C2_DEBUG_(x)						{ x }
#	define C2_TODO							g_print ("%s:%d:%s: TODO\n" __FILE__, __LINE__, \
													__PRETTY_FUNCTION__)

#	define C2_PRINTD(mod, args...)			if (DMOD) g_print (mod " -- " __PRETTY_FUNCTION__ "() -- " ##args)

#else
#	define L								;
#	define C2_PRINTD(x)						;
#	define C2_DEBUG(x)						;
#	define C2_DEBUG_(x)						;
#	define C2_TODO							;
#	define C2_PRINTD(mod, args...)			
#endif

#define unless(x)							if (FAIL(x))
	
gboolean
c2_strcaseeq								(const gchar *fst, const gchar *snd);

gboolean
c2_strncaseeq								(const gchar *fst, const gchar *snd, gint length);

gboolean
c2_streq									(const gchar *fst, const gchar *snd);

gboolean
c2_strneq									(const gchar *fst, const gchar *snd, gint length);

const gchar *
c2_strstr_case_insensitive					(const gchar *haystack, const gchar *needle);

gchar *
c2_str_replace_all							(const gchar *or_string, const gchar *se_string,
											 const gchar *re_string);

gchar *
c2_str_strip_enclosed						(const gchar *str, gchar open, gchar close);

gchar *
c2_str_get_enclosed_text					(const gchar *str, gchar enc1, gchar enc2, guint args, ...);

gchar *
c2_str_get_enclosed_text_backward			(const gchar *str, gchar enc1, gchar enc2, guint args, ...);

gint
c2_str_count_lines							(const gchar *str);
   
gchar *
c2_str_get_line								(const gchar *str);

gchar *
c2_str_get_word								(guint8 word_n, const gchar *str, gchar ch);

gchar *
c2_str_wrap									(const gchar *str, guint8 position);

gchar *
c2_str_text_to_html							(const gchar *str, gboolean proc_email);

gchar *
c2_str_get_striped_subject					(const gchar *subject);

GList *
c2_str_get_emails							(const gchar *string);

gchar *
c2_str_get_email							(const gchar *email);

gchar *
c2_str_get_sender							(const gchar *string);

gboolean
c2_str_is_email								(const gchar *email);

gboolean
c2_str_are_emails							(GList *list);

gchar *
c2_str_decode_iso_8859_1					(const gchar *string);

gchar *
c2_get_tmp_file								(const gchar *template);

gint
c2_get_file									(const gchar *path, gchar **string);

char *
c2_fd_get_line								(FILE *fd);

gchar *
c2_fd_get_word								(FILE *fd);

gint
c2_file_binary_copy							(const gchar *from_path, const gchar *target_path);

gint
c2_file_binary_move							(const gchar *from_path, const gchar *target_path);

gboolean
c2_file_exists								(const gchar *file);

gboolean
c2_file_is_directory						(const gchar *file);

gboolean
c2_fd_move_to								(FILE *fp, gchar c, guint8 cant,
										 	 gboolean forward, gboolean next);

void
c2_marshal_NONE__INT_INT_INT				(GtkObject *object, GtkSignalFunc func,
											 gpointer func_data, GtkArg * args);

void
c2_marshal_INT__POINTER_POINTER_POINTER		(GtkObject *object, GtkSignalFunc func,
											 gpointer func_data, GtkArg * args);

#ifdef __cplusplus
}
#endif

#endif
