#ifndef I18N_H
#define I18N_H

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
