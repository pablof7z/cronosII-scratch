#ifndef ERROR_H
#define ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <glib.h>
#include <errno.h>
	
#define c2_return_if_fail(condition, errnum) if (!(condition)) { \
		c2_error_set (errnum); \
		g_return_if_fail (condition); \
	}

#define c2_return_val_if_fail(condition, value, errnum) if (!(condition)) { \
		c2_error_set (errnum); \
		g_return_val_if_fail (condition, value); \
	}

enum
{
	C2SUCCESS,
	C2EDATA,
	C2ENOMSG,

	C2ELAST
};

/* Own errno variable to keep track of our errors */
int c2_errno;

const gchar *
c2_error_get									(gint err);

void
c2_error_set									(gint err);

#ifdef __cplusplus
}
#endif

#endif
