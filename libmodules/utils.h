#ifndef UTILS_H
#define UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <glib.h>
#include <stdio.h>

#define CHAR(x)								((gchar*)x)

gboolean
c2_strcaseeq							(const gchar *fst, const gchar *snd);

gboolean
c2_strncaseeq							(const gchar *fst, const gchar *snd, gint length);

gboolean
c2_streq								(const gchar *fst, const gchar *snd);

gboolean
c2_strneq								(const gchar *fst, const gchar *snd, gint length);

gchar *
c2_str_replace_all						(const gchar *or_string, const gchar *se_string,
										 const gchar *re_string);

gchar *
c2_str_get_line							(const gchar *str);

gchar *
c2_str_get_word							(guint8 word_n, const gchar *str, gchar ch);

gchar *
c2_fd_get_line							(FILE *fd);

gchar *
c2_fd_get_word							(FILE *fd);

gboolean
c2_file_exists							(const gchar *file);

gboolean
c2_fd_move_to							(FILE *fp, gchar c, guint8 cant,
										 gboolean forward, gboolean next);

#ifdef __cplusplus
}
#endif

#endif
