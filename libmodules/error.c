#include <stdio.h>

#include "error.h"
#include "i18n.h"

static const gchar *err_list[] = {
	/* C2SUCCESS */	N_("Success"),
	/* C2EDATA */		N_("Data exception"),
	/* C2ENOMSG */		N_("No such message")
};

/**
 * c2_error_set
 * @err: Error number.
 *
 * Will set the error code in the internal errno.
 **/
void
c2_error_set (gint err) {
	c2_errno = err;
}

/**
 * c2_error_get
 * @err: Error number.
 *
 * Will get the error string according to the code.
 *
 * Return Value:
 * A non freeable string describing the error.
 **/
const gchar *
c2_error_get (gint err) {
	if (err >= 0) return err_list[err];
	else return g_strerror (err*(-1));
}
