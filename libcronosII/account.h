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
#ifndef __LIBMODULES_ACCOUNT_H__
#define __LIBMODULES_ACCOUNT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <glib.h>

#ifdef HAVE_CONFIG_H
#	include "mailbox.h"
#	include "pop.h"
#	include "smtp.h"
#	include "spool.h"
#else
#	include <cronosII.h>
#endif

typedef enum
{
	C2_ACCOUNT_POP,
	C2_ACCOUNT_SPOOL
} C2AccountType;

typedef struct _C2Account
{
	gchar *name;

	gchar *per_name;
	gchar *email;

	C2AccountType type;
	
	union
	{
		C2Pop *pop;
		C2Spool *spool;
	} protocol;

	C2Smtp smtp;

	struct
	{
		gboolean activated;
	} options;

	struct
	{
		gchar *string;
		gboolean automatically;
	} signature;

	C2Mailbox *mbox;

	struct _C2Account *next;
} C2Account;

C2Account *
c2_account_new										(const gchar *name, const gchar *per_name,
													 const gchar *email, const gchar *smtp_address,
													 gint smtp_port, gboolean activated,
													 const gchar *signature, gboolean autosign,
													 gint mailbox, C2AccountType type, ...);

void
c2_account_free										(C2Account *account);

void
c2_account_free_all									(C2Account *head);

C2Account *
c2_account_append									(C2Account *head);

#ifdef __cplusplus
}
#endif

#endif
