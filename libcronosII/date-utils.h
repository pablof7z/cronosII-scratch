#ifndef __LIBMODULES_DATE_UTILS_H__
#define __LIBMODULES_DATE_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>

gint
c2_date_get_month									(const gchar *strmnt);

time_t
c2_date_parse										(const gchar *strtime);

time_t
c2_date_parse_fmt2									(const gchar *strtime);

#ifdef __cplusplus
}
#endif

#endif
