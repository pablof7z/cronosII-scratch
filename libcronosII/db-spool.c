/*  Cronos II Mail Client /libcronosII/db-spool.c
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
#include <glib.h>
#include <config.h>

#include "error.h"
#include "db-spool.h"
/*hmmm this is almost beautifull in its minimalism -pete ;)*/
/*yeah, is the shortest module I ever see! but it doesn't have any bugs! :) -pablo */
/*Ah you coders bugs bugs bugs. No code =nobugs. Code bad code use memory. :(*/

gint
c2_db_spool_load (C2Mailbox *mailbox)
{
	return 0;
}

gint
c2_db_spool_create_structure (C2Mailbox *mailbox)
{
	/* This is already done, this function doesn't need to do anything but
	 * return an ok status.
	 */
	return 0;
}

void
c2_db_spool_remove_structure (C2Mailbox *mailbox)
{
	unlink (mailbox->protocol.spool.path);
}

/*should this return anything when done ?? -pete..(I would add it if I knew how) */
